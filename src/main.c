/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: main.c
 *    This file is part of fontopia.
 *
 *    fontopia is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    fontopia is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fontopia.  If not, see <http://www.gnu.org/licenses/>.
 */    

#include <locale.h>
#include <ncurses.h>
#include "defs.h"
#include "view.h"
#include "glyph.h"
#include "metadata.h"
#include "modules/cp.h"
#include "menu.h"

extern int fcloseall (void);    /* stdio.h */

//char *readme_file = "/usr/share/doc/fontopia/README";
//char *keys_file   = "/usr/share/doc/fontopia/READMEkeys";

extern char *fontopia_ver;    /* args.c */
char *about_str = "Fontopia for GNU/Linux, "
              "Version 0.0  \n"
              "By Mohammed Isam, 2015, 2016, 2017, 2018, 2024";
/* global flag to indicate if we need to exit */
int end = 0;

/* This buffer will hold the copied/cut glyph,
 * and is as big as the maximal width & height.
 */
unsigned char copy_buffer[(MAX_WIDTH/8)*MAX_HEIGHT];
/* flag to indicate whether buffer mode is on */
char buffer_mode_on = 0;
char buffer_is_empty = 1;

int big_endian;

/*
 * Exit gracefully, restoring the terminal, clearing
 * screen, showing cursor, and closing all files.
 */
void exit_gracefully()
{
    setScreenColors(WHITE, BGDEFAULT);
    clearScreen(); 
    setScreenColors(WHITE, BGDEFAULT);
    reset_attribs();
    showCursor();
    restoreTerminal(); 
    fcloseall();
    exit(0);
}

/***************************************
 * sighandler(): 
 * Procedure that sets trap to the system
 * signals that are issued when user 
 * presses CTRL-C or CTRL-Z...
 * *************************************/
void sighandler(int signo)
{
    if(signo == SIGTSTP) 
    { //CTRL-Z pressed
        int i = msgBox("Are you sure you want to exit?", 
                        BUTTON_YES | BUTTON_NO, CONFIRM);

        if(i == BUTTON_YES) 
        {     //exit gracefully
            exit_gracefully();
        }
        else 
        {
            //refresh_view();
        }
    }
    else 
    {
        end = 1;
    }
}//end sighandler


int main(int argc, char **argv)
{
    struct font_s *font;

    setlocale(LC_ALL, "");
    init(argc, argv);

    /* clear the screen */
    clearScreen();

    if(font_file_name)
    {
        if(!(font = load_font_file(font_file_name)))
        {
            refresh();
            if(msgBox("Continue with a new empty font?", 
                        BUTTON_YES|BUTTON_NO, ERROR) == BUTTON_NO)
                exit_gracefully();
            if(!(font = create_empty_font())) exit_gracefully();
            if(font_file_name) free(font_file_name);
            font_file_name = NULL;
        }
    }
    else
    {
        if(!(font = create_empty_font())) exit_gracefully();
    }
    //refresh_view_status_msg(font_file_name, font);

    // we don't need those after start up
    startup_rawfont_width = 0;
    startup_rawfont_height = 0;
  
    /*
     * TODO: We should use this information to add
     *       support for big-endian machines.
     */
    if(is_big_endian()) big_endian = 1;
    else big_endian = 0;
  
    int i;
    int ch = 0;
    char *abt;

    while(!end) 
    {    //infinite program loop//
        ch = getKey();
        //printf("%d", ch);

        switch(ch) 
        {
            // CTRL-H (^H) is read as CTRL-Backspace
            case(BACKSPACE_KEY):
                if(CTRL)
                {
                    //show_readme(readme_file, NULL, 0);
                    fontopia_show_readme(readme_text, " README ");
                    refresh_view_status_msg(font_file_name, font);
                }
                break;

            case(UP_KEY):
                do_up(font);
                break;

            case(DOWN_KEY):
                do_down(font);
                break;

            case(LEFT_KEY):
                do_left(font);
                break;

            case(RIGHT_KEY):
                do_right(font);
                break;

            case('a'):
            case('A'):
                abt = (char *)malloc(strlen(about_str)+1);
                if(!abt)
                {
                    status_error("Insufficient memory!");
                    break;
                }

                strcpy(abt, about_str);
                for(i = 0; i < (int)strlen(fontopia_ver); i++) 
                    abt[32+i] = fontopia_ver[i];

                showAbout(abt);
                free(abt);
                refresh_left_window(font);
                refresh_right_window(font);
                hideCursor();
                break;

            case('b'):
            case('B'):
                if(!CTRL)
                {
                    draw_shape(font, 'b');
                    break;
                }
                buffer_mode_on = !buffer_mode_on;
                if(buffer_mode_on) status_msg("Buffering is ON");
                else               status_msg("Buffering is OFF");
                break;

            case('c'):
            case('C'):
                if(CTRL)
                {
                    copy_glyph(font, copy_buffer);
                    buffer_is_empty = 0;
                }
                else
                {
                    clear_glyph(font);
                    refresh_left_window(font);
                }
                break;

            case('d'):
            case('D'):
                if(CTRL)
                {
                    /* force saving with new file name */
                    font = save_font_file(font, 1);
                    refresh_left_window(font);
                    refresh_right_window(font);
                    hideCursor();
                }
                else
                {
                    invert_glyph(font);
                    refresh_left_window(font);
                }
                break;

            case('e'):
            case('E'):
                export_unitab(font);
                break;

            case('g'):
            case('G'):
                show_glyph_info(font);
                refresh_left_window(font);
                refresh_right_window(font);
                break;

            case('h'):
            case('H'):
                if(CTRL)
                {
                    //show_readme(readme_file, NULL, 0);
                    fontopia_show_readme(readme_text, " README ");
                    refresh_view_status_msg(font_file_name, font);
                }
                else
                {
                    flip_glyph_horizontally(font);
                    refresh_left_window(font);
                }
                break;

            case('i'):
            case('I'):
                import_unitab(font);
                refresh_left_window(font);
                refresh_right_window(font);
                hideCursor();
                break;

            case('j'):
            case('J'):
                if(!CTRL)
                {
                    /*
                     * TODO: Add routine for clockwise rotation.
                     */
                }
                break;

            case('k'):
            case('K'):
                //show_readme(keys_file, " KEYS ", 0);
                fontopia_show_readme(keys_text, " KEYS ");
                refresh_view_status_msg(font_file_name, font);
                break;

            case('l'):
            case('L'):
                if(!CTRL)
                {
                    draw_shape(font, 'l');
                }
                break;

            case('m'):
            case('M'):
                show_font_metrics(font);
                refresh_left_window(font);
                refresh_right_window(font);
                hideCursor();
                break;

            case('n'):
            case('N'):
                if(CTRL)
                {
                    struct font_s *old_font = font;
                    font = new_font_file(font);
                    if(old_font != font)
                    {
                        refresh_view_status_msg(font_file_name, font);
                    }
                    else
                    {
                        refresh_left_window(font);
                        refresh_right_window(font);
                    }
                    hideCursor();
                }
                else
                {
                    /*
                     * TODO: Add routine for counter-clockwise rotation.
                     */
                }
                break;

            case('o'):
            case('O'):
                if(CTRL)
                {
                    struct font_s *old_font = font;
                    font = open_font_file(font);
                    if(old_font == font)
                    {
                        refresh_left_window(font);
                        refresh_right_window(font);
                    }
                    hideCursor();
                }
                else
                {
                    show_ext_glyph_operations(font);
                }
                break;

            case('p'):
            case('P'):
                if(font->version == VER_CP)
                {
                    cp_change_codepage(font);
                }
                else if(font->has_metadata)
                {
                    show_metadata(font);
                }
                refresh_left_window(font);
                refresh_right_window(font);
                hideCursor();
                break;

            case('q'):
            case('Q'):
                //if(CTRL) quit(font);
                quit(font);
                break;

            case('r'):
            case('R'):
                if(!CTRL)
                {
                    /*
                     * TODO: Add circle (round) drawing routine.
                     */
                }
                else
                {
                    if(remove_unitab(font))
                        status_msg("Font unicode table removed successfully");
                    else
                        status_msg("Error: Font has no unicode table");
                }
                refresh_left_window(font);
                refresh_right_window(font);
                hideCursor();
                break;

            case('s'):
            case('S'):
                if(CTRL)
                {
                    if(SHIFT)    /* force saving with new file name */
                         font = save_font_file(font, 1);
                    else font = save_font_file(font, 0);    /* save with -probably- old name */
                    refresh_left_window(font);
                    refresh_right_window(font);
                    hideCursor();
                }
                else
                {
                    set_glyph(font);
                    refresh_left_window(font);
                }
                break;

            case('t'):
            case('T'):
                if(!CTRL)
                {
                    /*
                     * TODO: Add triangle drawing routine.
                     */
                }
                break;

            case('u'):
            case('U'):
                open_unicode_table(font);
                refresh_left_window(font);
                refresh_right_window(font);
                hideCursor();
                break;

            case('v'):
            case('V'):
                if(CTRL)
                {
                    if(!buffer_is_empty)
                    {
                        paste_glyph(font, copy_buffer);
                        refresh_left_window(font);
                    }
                    else
                    {
                        status_error("No glyph is selected for copying");
                    }
                }
                else
                {
                    flip_glyph_vertically(font);
                    refresh_left_window(font);
                }
                break;

            case('w'):
            case('W'):
                if(CTRL) export_glyphs(font, 1);    // export as C source file
                else if(!ALT) export_glyphs(font, 0); // export as plain text
                break;

            case('x'):
            case('X'):
                if(CTRL)
                {
                    cut_glyph(font, copy_buffer);
                    buffer_is_empty = 0;
                    refresh_left_window(font);
                    break;
                }
                zoom_out(font); break;

            case('z'):
            case('Z'):
                zoom_in(font); break;

            case(SPACE_KEY):
            case(ENTER_KEY):
                if(active_window == &right_window)
                {
                    active_window = &left_window;
                }
                else
                {
                    font_toggle_active_bit(font);
                }
                refresh_left_window(font);
                break;

            case(TAB_KEY):
                active_window = active_window->next;
                refresh_left_window(font);
                break;

            case(ESC_KEY):
                if(active_window == &left_window)
                {
                    active_window = &right_window;
                    refresh_left_window(font);
                }
                break;

            default:
                if(ch >= '1' && ch <= '9') 
                    cp_change_active_font(font, ch-'0');
                break;
        }
    }

    exit(0);
}


/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: args.c
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <asm/types.h>
#include <limits.h>
#include <sys/stat.h>
#include "defs.h"
#include "view.h"
#include "modules/modules.h"

char *fontopia_ver = "2.0";

char *font_file_name = (char *)NULL;
FILE *font_file = NULL;
char *file_data = (char *)NULL;

// in case the user has supplied -w or -h options for a raw font
int startup_rawfont_width = 0;
int startup_rawfont_height = 0;

extern struct window_s left_window, right_window;
extern struct window_s *active_window;

void parse_args(int argc, char **argv)
{
    if(argc == 1) return;

    ///////////////////////////////////////
    //parse command line arguments
    ///////////////////////////////////////
    int c;
    static struct option long_options[] =
    {
        { "help",         no_argument,            0,  'h' },
        { "version",      no_argument,            0,  'v' },
        { "width",        required_argument,      0,  'w' },
        { "height",       required_argument,      0,  'i' },
        { 0, 0, 0, 0}
    };

    int seeni = 0, seenw = 0;

    while(1)
    {
        int option_index=0;
        c = getopt_long(argc, argv, "vhw:i:", long_options, &option_index);
        if(c == -1) break;    //end of options

        switch(c)
        {
            case 0:
                break;

            case 'w':
                startup_rawfont_width = atoi(optarg);
                seenw = 1;
                break;

            case 'i':
                startup_rawfont_height = atoi(optarg);
                seeni = 1;
                break;

            case 'v':    //show version & exit
                printf("%s\n", fontopia_ver);
                exit(0);

            case 'h':    //show help & exit
              printf("Fontopia for GNU/Linux, Version %s\n"
                     "By Mohammed Isam, 2015, 2016, 2017, 2018, 2024\n"
                     "Fontopia is a GNU software\n"
                     "\nUsage: %s [options] [file-name]\n"
                     "\nOptions:\n"
                     "  [-h, --help]         show this help and exit\n"
                     "  [-i, --height X]     if passed with a filename, the file is opened as a raw\n"
                     "                         font file, assuming glyphs are of height X\n"
                     "  [-v, --version]      show version and exit\n"
                     "  [-w, --width X]      if passed with a filename, the file is opened as a raw\n"
                     "                         font file, assuming glyphs are of width X\n"
                     "\n"
                     , fontopia_ver, argv[0]);
              exit(0);

            case '?':
                break;

            default:
                abort();
        }
    }

    if(seeni != seenw)
    {
        printf("\nYou passed the -%c option without the -%c option."
               "\nIf you are trying to load a raw font file and want to specify the"
               "\nglyph dimensions, you have to pass BOTH -w and -h."
               "\n\n",
               seeni ? 'i' : 'w',
               seeni ? 'w' : 'i');
        exit(1);
    }

    ///////////////////////////////////////
    //parse the remaining arguments
    ///////////////////////////////////////
    if(optind < argc)
    {
        font_file_name = (char *)malloc(strlen(argv[optind])+1);
        if(!font_file_name) { printf("Insufficient memory\n"); exit(1); }
        strcpy(font_file_name, argv[optind]);
    }
}

void init(int argc, char **argv)
{
    parse_args(argc, argv);

    if(!initTerminal())
    {
        printf("Failed to initialize terminal: Quitting\n");
        exit(1);
    }

    getScreenSize();

    if(!SCREEN_H || !SCREEN_W)
    {
        restoreTerminal(); 
        setScreenColors(WHITE, BGDEFAULT);
        printf("Error: Failed to get terminal size\n");
        exit(1);
    }

    FG_COLOR[COLOR_WINDOW]         = 37;
    FG_COLOR[COLOR_HIGHLIGHT_TEXT] = 34;
    FG_COLOR[COLOR_MENU_BAR]       = 34;
    FG_COLOR[COLOR_STATUS_BAR]     = 34;
    FG_COLOR[COLOR_BUTTONS]        = 37;
    FG_COLOR[COLOR_HBUTTONS]       = 32;
    BG_COLOR[COLOR_WINDOW]         = 49;
    BG_COLOR[COLOR_HIGHLIGHT_TEXT] = 47;
    BG_COLOR[COLOR_MENU_BAR]       = 47;
    BG_COLOR[COLOR_STATUS_BAR]     = 47;
    BG_COLOR[COLOR_BUTTONS]        = 41;
    BG_COLOR[COLOR_HBUTTONS]       = 41;

    left_window.min_width = 30;
    right_window.min_width = 16;
    left_window.min_height = 20;
    right_window.min_height = 20;

    int total_width  = left_window.min_width  + right_window.min_width;
    int total_height = left_window.min_height;

    if(total_height > SCREEN_H-2 || total_width  > SCREEN_W-3)
    {
        restoreTerminal(); 
        setScreenColors(WHITE, BGDEFAULT);
        printf("Error: Terminal size is too small. Required size is at least 80x25\n");
        exit(1);
    }
  
    right_window.width = right_window.min_width;
    right_window.height = SCREEN_H-3;
    right_window.cursor.row = 0;
    right_window.cursor.col = 0;
    right_window.old_cursor.row = 0;
    right_window.old_cursor.col = 0;
    right_window.next = &left_window;
    right_window.start_col = SCREEN_W-right_window.width-1;
    right_window.start_row = 2;
    right_window.first_vis_row = 0;
    right_window.cols_per_row = right_window.width;
    right_window.zoom = 1;
  
    left_window.width = SCREEN_W-right_window.min_width-3;
    left_window.height = SCREEN_H-3;
    left_window.cursor.row = 0;
    left_window.cursor.col = 0;
    left_window.old_cursor.row = 0;
    left_window.old_cursor.col = 0;
    left_window.next = &right_window;
    left_window.start_col = 2;
    left_window.start_row = 2;
    left_window.zoom = 1;
  
    active_window = &right_window;

    /* this will init modules and register necessary modules */
    init_modules();
  
    hideCursor();
}

void quit(struct font_s *font)
{
    int res;

    if(font->state == MODIFIED || font->state == NEW_MODIFIED)
    {
        res = msgBox("Font is not saved. Save?", BUTTON_YES|BUTTON_NO, INFO);
        if(res == BUTTON_ABORT) return;
        if(res == BUTTON_NO) exit_gracefully();
    }

    buffer_mode_on = 0;
    if(check_font_saved(font, 0) == 0) exit_gracefully();
}


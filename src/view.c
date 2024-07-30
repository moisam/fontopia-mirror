/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: view.c
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

#include <ncurses.h>
#include "defs.h"
#include "view.h"
#include "glyph.h"
#include "modules/psf.h"

struct window_s left_window = { 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, }, { 0, }, NULL };
struct window_s right_window = { 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, }, { 0, }, NULL };
struct window_s *active_window;

unsigned short utf_mask[] = { 192, 224, 240 };

/*
 * Code for this function is adopted from code written by Jeff Bezanson,
 * who kindly placed this in the public domain in 2005. Source is available from:
 *    http://www.cprogramming.com/tutorial/unicode.html.
 */
int make_utf8(unsigned char *dest, unsigned int ch)
{
    if (ch < 0x80)
    {
        dest[0] = (char)ch;
        return 1;
    }

    if (ch < 0x800)
    {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }

    if (ch < 0x10000)
    {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }

    if (ch < 0x110000)
    {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }

    return 0;
}

int make_utf16(unsigned int *res, unsigned char *utf8);    /* font_ops.c */


unsigned int get_next_utf(struct font_s *font)
{
    unsigned char *unicode = font->unicode_info;
    int array_index = font->unicode_array_index;
    int index = font->unicode_index;
    unsigned short val = 0;
    unsigned int val2 = 0;
  
    /*
     * Unicode in PSF1 is stored as 16-bit UTF.
     */
    if(font->utf_version == VER_PSF1)
    {
        /*
         * WARNING: there should be some error checking scheme to ensure
         *          we don't trespass on other people's memory, i.e. cheking for
         *          index-out-of-bounds error.
         */
        val = (unicode[array_index] | (unsigned short)(unicode[array_index+1] << 8));
        array_index += 2;
        font->unicode_array_index = array_index;
        font->unicode_index = index;
        return (unsigned int)val;
    }
    /*
     * Unicode in PSF2 is stored as 8-bit-coded UTF.
     */
    else if(font->utf_version == VER_PSF2)
    {
        if(unicode[array_index] == PSF2_SEPARATOR)
        {
            val2 = (unsigned int)PSF1_SEPARATOR;
            array_index++;
        }
        else if(unicode[array_index] == PSF2_STARTSEQ)
        {
            val2 = (unsigned int)PSF1_STARTSEQ;
            array_index++;
        }
        else
        {
            int bytes = 1;
            bytes = make_utf16(&val2, &unicode[array_index]);
            array_index += bytes;
        }

        font->unicode_array_index = array_index;
        font->unicode_index = index;
        return (unsigned int)val2;
    }

    return 0;
}

void refresh_right_window(struct font_s *font)
{
    int row = 0;
    int col = 0;
    int i = 0;
    unsigned short c = 0;
    unsigned char wc[5];

    font->unicode_array_index = 0;
    font->unicode_index = 0;

    /* first clear old canvas */
    setScreenColors(WHITE, BGDEFAULT);

    for(i = 0; i < right_window.height; i++)
    {
        locate(right_window.start_row+i, right_window.start_col);
        printw("%*s ", right_window.width, " ");
    }

    i = 0;
    if(row > right_window.height || col > right_window.width) { row = 0; col = 0; }
    locate(right_window.start_row+row, right_window.start_col+col);

    i = right_window.first_vis_row*right_window.cols_per_row;

    while(i < (int)font->length)
    {
        if(right_window.cursor.row == row && right_window.cursor.col == col)
            setScreenColors(BLACK, BGWHITE);
        else
            setScreenColors(WHITE, BGDEFAULT);
    
        if(!font->has_unicode_table)
        {
            printw("%04x ", i++);
            col++;

            if(col >= right_window.cols_per_row)
            {
                row++;
                if(row == right_window.height) break;
                col = 0;
                locate(right_window.start_row+row, right_window.start_col+col);
            }
            continue;
        }

        /* font has unicode table */
        if(font->unicode_table_index[i] == 0xFFFF)
        {
            unsigned int *arr = 0;
            get_unitab_entry(font, i, &arr);
            c = arr[0];
        }
        else
        {
            c = font->unicode_table[i*2];
        }

        memset((void *)wc, 0, 5);
        make_utf8(wc, c);

        if(wc[0] == 0xa0 || wc[0] < 0x20)
        {
            make_utf8(wc, 0xfffd);
            printw("%s", wc);
        }
        else printw("%s", wc);

        i++; col++;

        if(col == right_window.cols_per_row)
        {
            row++;
            if(row == right_window.height) break;
            col = 0;
            locate(right_window.start_row+row, right_window.start_col+col);
        }
    }

    refresh();
}

void refresh_left_window(struct font_s *font)
{
    int bail_out = 0;
    if((left_window.width < (int)font->width+6) ||
       (left_window.height < (int)font->height+2))
    {
        msgBox("Can't display glyph. Font metrics\n"
               "(width or height) larger than screen\n"
               "size.", BUTTON_OK, ERROR);
        bail_out = 1;
    }

    int col_midpoint = (left_window.width-(font->width*font->current_zoom)) >> 1;
    int row_midpoint = (left_window.height-(font->height*font->current_zoom)) >> 1;
    int col = left_window.start_col + col_midpoint;
    int row = left_window.start_row + row_midpoint;
    int zoom = (int)font->current_zoom;
    int j;
    unsigned char *data = font->data;
    char X[10], O[10];

    data += get_glyph_index(font);
    setScreenColors(WHITE, BGDEFAULT);

    for(j = 0; j < zoom; j++) { X[j] = 'X'; O[j] = '-'; }
    X[j] = '\0'; O[j] = '\0';
  
    /* first clear old canvas */
    for(j = 0; j < left_window.height; j++)
    {
        locate(left_window.start_row+j, left_window.start_col);
        printw("%*s", left_window.width, " ");
    }

    if(bail_out) return;
  
    int actual_row = row;
    unsigned int base_index = (1 << (font->width - 1));

    if(font->width < 8) base_index <<= (8-font->width);
    else if(font->width > 8 && font->width < 16) base_index <<= (16-font->width);
    else if(font->width > 16 && font->width < 32) base_index <<= (32-font->width);

    /* Then draw the glyph */
    for(j = 0; j < (int)font->height; j++)
    {
        unsigned int line, index;
        index = base_index;

        if(font->width <= 8)
        {
            line = (unsigned int)data[0]; data++;
        }
        else if(font->width <= 16)
        {
            line = (unsigned int)(data[0] | (unsigned int)(data[1] << 8));
            data += 2;
        }
        else if(font->width <= 32)
        {
            line = (unsigned int)(data[0] | ((unsigned int)(data[1]) << 8) | 
                   ((unsigned int)(data[2]) << 16) | ((unsigned int)(data[3] << 24)));
            data += 4;
        }
        /* 
         * TODO: We must accommodate other font widths 
         */
        else
        {
            return;
        }
    
        /* print the hex presentation of this col, only
         * if there is enough space to the left.
         */
        setScreenColors(WHITE, BGDEFAULT);

        if(col > 6)
        {
            locate(row, col-6);
            if(font->width <= 16) printw("%04x", line);
            else if(font->width <= 32) printw("%08x", line);
        }
    
        /* now print individual bits */
        int k;

        for(k = 0; k < (int)font->width; k++)
        {
            if(active_window == &left_window)
            {
                if(left_window.cursor.row == j && left_window.cursor.col == k)
                    setScreenColors(BLACK, BGWHITE);
                else
                    setScreenColors(WHITE, BGDEFAULT);
            }

            int i = 0;

            while(i < zoom)
            {
                locate(row+i, col);
                if((line & index)) printw("%s", X);
                else printw("%s", O);
                i++;
            }

            index >>= 1;
            col += zoom;
        }

        row += zoom;
        col = left_window.start_col + col_midpoint;
        actual_row = row;
        if(actual_row > SCREEN_H) break;
    }

    refresh();
}

void create_status_msg(char *smsg, char *msg, struct font_s *font)
{
    char *help_msg = "| ^H help | ^Q quit";
    char m[(MAX_UNICODE_TABLE_ENTRIES*7)+10];
    char t[12];

    smsg[0] = '\0';
    m[0] = '\0';

    if(font->version == VER_CP)
    {
        sprintf(m, "[%d/%d] ", font->cp_active_font+1, font->cp_total_fonts);
    }
  
    if(font->has_unicode_table)
    {
        int j = get_glyph_index(font)/font->charsize;
        /* this specific glyph has multiple unicode values, they are
         * stored in a second array, pointed to by unicode_table_index.
         */
        if(font->unicode_table_index[j] == 0xFFFF)
        {
            unsigned int *arr = 0;
            get_unitab_entry(font, j, &arr);
            sprintf(t, "U+%04x..", arr[0]);
            strcat(m, t);
        }
        else
        {
            sprintf(t, "U+%04x ", font->unicode_table[j*2]);
            strcat(m, t);
        }
    }
  
    int l = strlen(help_msg) + strlen(m);
    int l2 = msg ? strlen(msg) : 0;
    int s = SCREEN_W-l-l2-2;
    s -= 4;    /* to accommodate zoom */

    if(msg)
    {
        if(s < 0)
        {
            /* we need to truncate this msg */
            int s2 = (-s) + 2;
            l = l2-s2;
            char c = msg[l];
            msg[l] = '\0';
            strcpy(smsg, msg);
            strcat(smsg, "..[x");
            smsg[l+4] = font->current_zoom + '0';
            smsg[l+5] = ']';
            smsg[l+6] = '\0';
            strcat(smsg, m);
            strcat(smsg, help_msg);
            msg[l] = c;
        }
        else
            sprintf(smsg, "%s[x%d]%*s%s%s", msg, font->current_zoom, s, " ", m, help_msg);
    }
    else
    {
        sprintf(smsg, "[x%d]%*s%s%s", font->current_zoom, s, " ", m, help_msg);
    }
}

void _refresh_view(char *msg, int status, struct font_s *font)
{
    /* clear the screen */
    setScreenColors(WHITE, BGDEFAULT);

    /* draw main window */
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Fontopia for GNU/Linux ", 0);
    refresh_right_window(font);
    refresh_left_window(font);

    if(status == STATUS_ERROR)
    {
        status_error(msg);
        refresh();
        return;
    }
  
    char smsg[SCREEN_W+2];
    create_status_msg(smsg, msg, font);
    status_msg(smsg);
    refresh();
}

void refresh_view(struct font_s *font)
{
    if(font_file_name) _refresh_view(font_file_name, 0, font);
    else _refresh_view((char *)NULL, 0, font);
}

void refresh_view_status_msg(char *msg, struct font_s *font)
{
    _refresh_view(msg, STATUS_REGULAR, font);
}

void refresh_view_status_error(char *msg, struct font_s *font)
{
    _refresh_view(msg, STATUS_ERROR, font);
}

void calc_max_zoom(struct font_s *font)
{
    int zoom_w = left_window.width/font->width;
    int zoom_h = left_window.height/font->height;

    if(zoom_h == 0) zoom_h = 1;
    if(zoom_w == 0) zoom_w = 1;
    if(zoom_h < zoom_w) font->max_zoom = zoom_h;
    else font->max_zoom = zoom_w;

    font->current_zoom = 1;
    right_window.first_vis_row = 0;

    if(font->has_unicode_table)
        right_window.cols_per_row = right_window.width;
    else
        right_window.cols_per_row = right_window.width/5;
}

void zoom_in(struct font_s *font)
{
    if(font->current_zoom >= font->max_zoom) return;
    font->current_zoom++;
    refresh_view_status_msg(font_file_name, font);
    refresh();
}

void zoom_out(struct font_s *font)
{
    if(font->current_zoom <= 1) return;
    font->current_zoom--;
    refresh_view_status_msg(font_file_name, font);
    refresh();
}

void reset_window_cursor(struct window_s *win)
{
    win->cursor.row = 0;
    win->cursor.col = 0;
}

void reset_all_cursors()
{
    right_window.cursor.col = 0;
    right_window.cursor.row = 0;
    left_window.cursor.col = 0;
    left_window.cursor.row = 0;
}


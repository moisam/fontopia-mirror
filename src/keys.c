/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: keys.c
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

#include "defs.h"
#include "view.h"

void do_up(struct font_s *font)
{
    if(active_window == &right_window)
    {
        if(right_window.cursor.row == 0)
        {
            if(right_window.first_vis_row == 0) return;
            right_window.first_vis_row--;
        }
        else
        {
            right_window.cursor.row--;
        }
        refresh_view_status_msg(font_file_name, font);
    }
    else
    {
        if(left_window.cursor.row == 0) return;
        left_window.cursor.row--;
        refresh_left_window(font);
    }
}

void do_down(struct font_s *font)
{
    if(active_window == &right_window)
    {
        int h = 0;
        h = font->length / right_window.cols_per_row;
        if(font->length % right_window.cols_per_row) h++;
        if(right_window.cursor.row >= h-1) return;
        right_window.cursor.row++;

        if(right_window.cursor.row >= right_window.height)
        {
            right_window.cursor.row--;
            if(right_window.cursor.row+right_window.first_vis_row >= h-1) return;
            right_window.first_vis_row++;
        }

        /* watch out for the short last line */
        if(h-1 == right_window.first_vis_row+right_window.cursor.row)
        {
            int l = font->length % right_window.cols_per_row;
            if(l && right_window.cursor.col >= l) right_window.cursor.col = l-1;
        }

        refresh_view_status_msg(font_file_name, font);
    }
    else
    {
        int h = font->height;

        if(left_window.cursor.row == h-1) return;
        left_window.cursor.row++;
        refresh_left_window(font);
    }
}

void do_right(struct font_s *font)
{
    if(active_window == &right_window)
    {
        int h = 0;
        h = font->length / right_window.cols_per_row;
        if(font->length % right_window.cols_per_row) h++;

        int w = right_window.cols_per_row;
        if(right_window.cursor.col < w-1)
        {
            if(h-1 == right_window.first_vis_row+right_window.cursor.row)
            {
                int l = font->length % right_window.cols_per_row;
                if(l && right_window.cursor.col == l-1) return;
            }

            right_window.cursor.col++;
        }
        else return;

        refresh_view_status_msg(font_file_name, font);
    }
    else
    {
        int h = font->width;
        if(left_window.cursor.col == h-1)
        {
            if(left_window.cursor.row == (int)font->height-1) return;
            left_window.cursor.row++;
            left_window.cursor.col = 0;
        }
        else
        {
            left_window.cursor.col++;
        }

        refresh_left_window(font);
    }
}

void do_left(struct font_s *font)
{
    if(active_window == &right_window)
    {
        if(right_window.cursor.col == 0)
        {
            return;
        }
        else
        {
            right_window.cursor.col--;
        }

        refresh_view_status_msg(font_file_name, font);
    }
    else
    {
        if(left_window.cursor.col == 0)
        {
            if(left_window.cursor.row == 0) return;
            left_window.cursor.col = font->width-1;
            left_window.cursor.row--;
        }
        else
        {
            left_window.cursor.col--;
        }

        refresh_left_window(font);
    }
}


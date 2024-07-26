/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: view.h
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

#ifndef VIEW_H
#define VIEW_H

struct cursor_s
{
  int row;
  int col;
};

struct window_s
{
  int start_row, start_col;
  int first_vis_row;	/* if rows are more than window height */
  int cols_per_row;
  int width, height;
  int min_width, min_height;
  int zoom;
  struct cursor_s cursor;
  struct cursor_s old_cursor;
  struct window_s *next;
};

extern struct window_s left_window, right_window;
extern struct window_s *active_window;

void calc_max_zoom(struct font_s *font);
void refresh_right_window(struct font_s *font);
void refresh_left_window(struct font_s *font);
unsigned int get_next_utf(struct font_s *font);
int make_utf8(unsigned char* dest, unsigned int ch);

void zoom_in(struct font_s *font);
void zoom_out(struct font_s *font);
void reset_window_cursor(struct window_s *win);
void reset_all_cursors();

#endif

/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: defs.h
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

#ifndef FONTOPIA_H
#define FONTOPIA_H

#include "font_ops.h"
#include <string.h>
#include <console/dialogs.h>

/* args.c */
void parse_args(int argc, char **argv);
void init(int argc, char **argv);
void quit(struct font_s *font);

extern char *font_file_name;
extern FILE *font_file;
extern char *file_data;
extern int startup_rawfont_width;
extern int startup_rawfont_height;

/* status.c */
void status_error(char *msg);
void status_msg(char *msg);
void status_clear();

#define STATUS_ERROR		1
#define STATUS_REGULAR		2

/* main.c */
void exit_gracefully();
extern char buffer_mode_on;

/* view.c */
void refresh_view(struct font_s *font);
void refresh_view_status_msg(char* msg, struct font_s *font);
void refresh_view_status_error(char *msg, struct font_s *font);

/* keys.c */
void do_up(struct font_s *font);
void do_down(struct font_s *font);
void do_left(struct font_s *font);
void do_right(struct font_s *font);

/* metrics.c */
int show_font_metrics(struct font_s *font);

/* unitab.c */
int open_unicode_table(struct font_s *font);
void export_unitab(struct font_s *font);
void import_unitab(struct font_s *font);
int remove_unitab(struct font_s *font);

#define MAX_UNICODE_TABLE_ENTRIES	(20)

/* opensave.c */
/* options to be passed to open/save dialog box */
#define OPEN			(1)
#define SAVE			(2)
/* shows open/save dialog box */
int show_opensave(char* _path, int OPEN_SAVE, char** file, int max_len);
/* same as show_opensave() but returns file name only, without parent name
 * (hence the _np in the name).
 */
int show_opensave_np(char *_path, int OPEN_SAVE, char **file, int max_len);
/* possible return values from open/save dialog box functions */
#define OPENSAVE_SUCCESS	(0)
#define OPENSAVE_ERROR		(-1)
#define OPENSAVE_CANCEL		(1)

/* readme.c */
int fontopia_show_readme(char* readme, char* title);
extern char *readme_text;
extern char *keys_text;

#endif

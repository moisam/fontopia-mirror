/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: cp.h
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

#ifndef CP_H
#define CP_H

#include <stdio.h>
#include <stdlib.h>
#include "../font_ops.h"
#include "psf.h"

/*
 * Header for Code Page format.
 */
struct cp_entry_header
{
  short cpeh_size;
  int next_cpeh_offset;
  short device_type;	/* 1 screen, 2 printer */
  char device_name[8];
  short codepage;
  char res[6];		/* zeroes */
  int cpih_offset;
}__attribute__((packed));

struct cp_info_header
{
  short version;	/* 1 font, 2 DRFont */
  short num_fonts;	/* 1, 3, 4 */
  short size;		/* data length for each font */
}__attribute__((packed));

struct screen_font_header
{
  char height;		/* 6, 8, 14, 16 */
  char width;		/* 8 */
  short res;		/* zeroes */
  short num_chars;	/* 256 */
}__attribute__((packed));

struct cp_header
{
  struct cp_entry_header entry_hdr;		/* 28 bytes */
  struct cp_info_header info_hdr;		/* 6 bytes */
}__attribute__((packed));

struct font_s *cp_create_empty_font();
struct font_s *cp_load_font_file(char *file_name);
struct font_s *cp_load_font(char *file_name, unsigned char *file_data, long file_size);
int cp_write_to_file(FILE *file, struct font_s *font);
void cp_change_active_font(struct font_s *font, char cp_index);
void cp_handle_hw_change(struct font_s *font, char *newdata, long new_datasize);
void cp_handle_version_change(struct font_s *font, char old_version);
void cp_export_unitab(struct font_s *font, FILE *f);
int cp_create_unitab(struct font_s *font);
void cp_kill_unitab(struct font_s *font);
void cp_convert_to_psf(struct font_s *font);
void cp_change_codepage(struct font_s *font);
void cp_init_module();
void cp_handle_unicode_table_change(struct font_s *font);

//extern char *cp_files_prefix;
extern int cp_files_count;
extern char *cp_files[];
extern short cp_ids[];
//extern char *cp_str[];

#endif

/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: raw.h
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

#ifndef RAW_H
#define RAW_H

#include <stdio.h>
#include <stdlib.h>
#include "../font_ops.h"
#include "psf.h"

struct font_s *raw_create_empty_font();
struct font_s *raw_load_font_file(char *file_name);
struct font_s *raw_load_font(char *file_name, unsigned char *file_data, long file_size);
int raw_write_to_file(FILE *file, struct font_s *font);
void raw_handle_hw_change(struct font_s *font, char *newdata, long new_datasize);
void raw_handle_version_change(struct font_s *font, char old_version);
void raw_export_unitab(struct font_s *font, FILE *f);
int raw_create_unitab(struct font_s *font);
void raw_kill_unitab(struct font_s *font);
void raw_convert_to_psf(struct font_s *font);
void raw_shrink_glyphs(struct font_s *font, int old_length);
void raw_expand_glyphs(struct font_s *font, int old_length, int option);
void raw_init_module();
int raw_acceptable_filesize(long file_size);

#endif

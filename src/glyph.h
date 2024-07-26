/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: glyph.h
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

#ifndef GLYPH_H
#define GLYPH_H

/* glyph.c */
void copy_glyph(struct font_s *font, unsigned char buffer[]);
void cut_glyph(struct font_s *font, unsigned char buffer[]);
void paste_glyph(struct font_s *font, unsigned char buffer[]);
void clear_glyph(struct font_s *font);
void set_glyph(struct font_s *font);
void flip_glyph_horizontally(struct font_s *font);
void flip_glyph_vertically(struct font_s *font);
void export_glyphs(struct font_s *font, int as_c_file);
void invert_glyph(struct font_s *font);
int get_glyph_index(struct font_s *font);
void draw_shape(struct font_s *font, char break_key);

/* glyphext.c */
void show_ext_glyph_operations(struct font_s *font);
void glyphop_center_horz(struct font_s *font);
void glyphop_center_vert(struct font_s *font);
void glyphop_center_both(struct font_s *font);
void glyphop_rotate_cw(struct font_s *font);
void glyphop_rotate_ccw(struct font_s *font);

/* glyphinfo.c */
void show_glyph_info(struct font_s *font);
int glyph_index(struct font_s *font);

#endif

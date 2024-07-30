/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: bdf.h
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

#ifndef BDF_H
#define BDF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../font_ops.h"
#include "psf.h"

/*
 * Type of possible argument(s) to each keyword:
 */
enum arg_type_e
{
    NUMBER,
    INTEGER,
    STRING
};

#define BDF_MAX_KEYWORD_LEN     (30)
#define BDF_MAX_ARGS            (10)
#define BDF_TOTAL_KEYWORDS      (45)

/*
 * EACH KEYWORD STRUCT HAS THE FOLLOWING COMPONENTS:
 *   Keyword, Required (Vs Optional), Scope, 
 *   Number of arguments, Type of args.
 */
struct bdf_keyword_s
{
  char name[BDF_MAX_KEYWORD_LEN];    /* the keyword itself */
#define BDF_REQ_YES             (1)
#define BDF_REQ_NO              (0)
  char required;            /* is it required? we should use this for error checking */
#define BDF_ANY_SCOPE           (255)
#define BDF_PROPERTIES_SCOPE    (4)
#define BDF_GLOBAL_SCOPE        (2)
#define BDF_CHARS_SCOPE         (1)
#define BDF_GLYPH_SCOPE         (0)
  int scope;                /* we should use this also for error checking */
  char argc;                /* acceptable arguments to this keyword */
  enum arg_type_e args[BDF_MAX_ARGS];
};

/*
 * Keywords and their positions in the array.
 */
#define STARTFONT_KEYWORD           (0)
#define COMMENT_KEYWORD             (1)
#define CONTENTVERSION_KEYWORD      (2)
#define FONT_KEYWORD                (3)
#define SIZE_KEYWORD                (4)
#define FONTBOUNDINGBOX_KEYWORD     (5)
#define METRICSSET_KEYWORD          (6)
#define SWIDTH_KEYWORD              (7)
#define DWIDTH_KEYWORD              (8)
#define SWIDTH1_KEYWORD             (9)
#define DWIDTH1_KEYWORD             (10)
#define VVECTOR_KEYWORD             (11)
#define STARTPROPERTIES_KEYWORD     (12)
#define ENDPROPERTIES_KEYWORD       (13)
#define CHARS_KEYWORD               (14)
#define ENDFONT_KEYWORD             (15)
#define COPYRIGHT_KEYWORD           (16)
#define FOUNDRY_KEYWORD             (17)
#define FAMILY_NAME_KEYWORD         (18)
#define WEIGHT_NAME_KEYWORD         (19)
#define SLANT_KEYWORD               (20)
#define SETWIDTH_NAME_KEYWORD       (21)
#define FONT_VERSION_KEYWORD        (22)
#define FONT_TYPE_KEYWORD           (23)
#define PIXEL_SIZE_KEYWORD          (24)
#define POINT_SIZE_KEYWORD          (25)
#define RESOLUTION_X_KEYWORD        (26)
#define RESOLUTION_Y_KEYWORD        (27)
#define SPACING_KEYWORD             (28)
#define AVERAGE_WIDTH_KEYWORD       (29)
#define CHARSET_REGISTRY_KEYWORD    (30)
#define CHARSET_ENCODING_KEYWORD    (31)
#define UNDERLINE_POSITION_KEYWORD  (32)
#define UNDERLINE_THICKNESS_KEYWORD (33)
#define CAP_HEIGHT_KEYWORD          (34)
#define X_HEIGHT_KEYWORD            (35)
#define FONT_ASCENT_KEYWORD         (36)
#define FONT_DESCENT_KEYWORD        (37)
#define DEFAULT_CHAR_KEYWORD        (38)
#define ADD_STYLE_NAME_KEYWORD      (39)

#define STARTCHAR_KEYWORD           (40)
#define ENCODING_KEYWORD            (41)
#define BBX_KEYWORD                 (42)
#define BITMAP_KEYWORD              (43)
#define ENDCHAR_KEYWORD             (44)


struct font_s *bdf_create_empty_font();
struct font_s *bdf_load_font_file(char *file_name);
struct font_s *bdf_load_font(char *file_name, unsigned char *file_data, long file_size);
int bdf_write_to_file(FILE *file, struct font_s *font);
void bdf_handle_hw_change(struct font_s *font, char *newdata, long new_datasize);
void bdf_handle_version_change(struct font_s *font, char old_version);
//void bdf_export_unitab(struct font_s *font, FILE *f);
void bdf_convert_to_psf(struct font_s *font);
void bdf_shrink_glyphs(struct font_s *font, int old_length);
void bdf_expand_glyphs(struct font_s *font, int old_length, int option);
void bdf_init_module();

struct hash_elem {
    int count;
    char *strval[6];
    size_t unival[6];
};
/* bdf_hash.c */
extern struct hash_elem hashtab[];
extern size_t hashtab_sz;
/* bdf_helper.c */
//unsigned int postscript_to_unicode(char *s);
int postscript_to_unicode(char *s, unsigned int *res);
unsigned int codepoint_to_unicode(char *s);

#endif

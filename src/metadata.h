/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: metadata.h
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

#ifndef METADATA_H
#define METADATA_H

#include <stdio.h>
#include <stdlib.h>
#include "font_ops.h"

#define metadata_table_len        32

/* indices into metadata table */
#define METADATA_FONT                   0
#define METADATA_COPYRIGHT              1
#define METADATA_FONT_VERSION           2
#define METADATA_FONT_TYPE              3
#define METADATA_FOUNDRY                4
#define METADATA_FAMILY_NAME            5
#define METADATA_WEIGHT_NAME            6
#define METADATA_SLANT                  7
#define METADATA_SETWIDTH_NAME          8
#define METADATA_ADD_STYLE_NAME         9
#define METADATA_PIXEL_SIZE             10
#define METADATA_POINT_SIZE             11
#define METADATA_RESOLUTION_X           12
#define METADATA_RESOLUTION_Y           13
#define METADATA_SPACING                14
#define METADATA_AVERAGE_WIDTH          15
#define METADATA_CHARSET_REGISTRY       16
#define METADATA_CHARSET_ENCODING       17
#define METADATA_UNDERLINE_POSITION     18
#define METADATA_UNDERLINE_THICKNESS    19
#define METADATA_CAP_HEIGHT             20
#define METADATA_X_HEIGHT               21
#define METADATA_WEIGHT                 22
#define METADATA_RESOLUTION             23
#define METADATA_QUAD_WIDTH             24
#define METADATA_FONT_ASCENT            25
#define METADATA_FONT_DESCENT           26
#define METADATA_DEFAULT_CHAR           27
#define METADATA_FONTBOUNDINGBOX_X      28
#define METADATA_FONTBOUNDINGBOX_Y      29
#define METADATA_FONTBOUNDINGBOX_XOFF   30
#define METADATA_FONTBOUNDINGBOX_YOFF   31

#define DEFAULT_SWIDTH                  1000

struct metadata_item_s
{
  char  name[31];
  char  is_str;    /* is value a string? */
  int   value;     /* for integer values */
  char *value2;    /* pointer to string values */
};

extern struct metadata_item_s metadata_table[metadata_table_len];

int save_metadata_str(struct font_s *font, int index, char *value);
void show_metadata(struct font_s *font);

#endif

/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: pcf.h
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

#ifndef PCF_H
#define PCF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../font_ops.h"
#include "bdf.h"
#include <sys/types.h>

/*
 * Check the spec at:
 *    https://fontforge.github.io/en-US/documentation/reference/pcf-format/
 */

// type field
#define PCF_PROPERTIES              (1<<0)
#define PCF_ACCELERATORS            (1<<1)
#define PCF_METRICS                 (1<<2)
#define PCF_BITMAPS                 (1<<3)
#define PCF_INK_METRICS             (1<<4)
#define PCF_BDF_ENCODINGS           (1<<5)
#define PCF_SWIDTHS                 (1<<6)
#define PCF_GLYPH_NAMES             (1<<7)
#define PCF_BDF_ACCELERATORS        (1<<8)

// format field
#define PCF_DEFAULT_FORMAT          0x00000000
#define PCF_INKBOUNDS               0x00000200
#define PCF_ACCEL_W_INKBOUNDS       0x00000100
#define PCF_COMPRESSED_METRICS      0x00000100
#define PCF_GLYPH_PAD_MASK          (3<<0)        /* See the bitmap table for explanation */
#define PCF_BYTE_MASK               (1<<2)        /* If set then Most Sig Byte First */
#define PCF_BIT_MASK                (1<<3)        /* If set then Most Sig Bit First */
#define PCF_SCAN_UNIT_MASK          (3<<4)        /* See the bitmap table for explanation */

struct pcf_header
{
	char      header[4];          /* always "\1fcp" */
	u_int32_t table_count;
}__attribute__((packed));

struct pcf_toc_entry
{
	u_int32_t type;      /* See below, indicates which table */
	u_int32_t format;        /* See below, indicates how the data are formatted in the table */
	u_int32_t size;      /* In bytes */
	u_int32_t offset;        /* from start of file */
}__attribute__((packed));

// properties table entry
struct props
{
	u_int32_t name_offset;      /* Offset into the following string table */
	char      isStringProp;
	u_int32_t value;        /* The value for integer props, the offset for string props */
}__attribute__((packed));

struct compressed_metrics
{
	unsigned char left_side_bearing, right_side_bearing;
	unsigned char character_width;
	unsigned char character_ascent;
	unsigned char character_descent;
	/* character_attributes implied to be 0 */
}__attribute__((packed));


struct uncompressed_metrics
{
	int16_t left_side_bearing, right_side_bearing;
	int16_t character_width;
	int16_t character_ascent;
	int16_t character_descent;
	u_int16_t character_attributes;
}__attribute__((packed));

struct accel_table
{
	u_int32_t     format;
	unsigned char no_overlap;
	unsigned char const_metrics;
	unsigned char terminal_font;
	unsigned char const_width;
	unsigned char ink_inside;
	unsigned char ink_metrics;
	unsigned char draw_direction;
	unsigned char padding;
	int32_t       font_ascent, font_descent;
	int32_t       max_overlap;
	struct uncompressed_metrics minbounds, maxbounds;
}__attribute__((packed));

struct dimensions
{
	int h, w;
};


int get_properties_table(char *table_data, struct font_s *font);
int get_metrics_table(char *table_data, struct font_s *font);
int get_ink_metrics_table(char *table_data, struct font_s *font);
int get_accel_table(char *table_data, struct font_s *font);
int get_bitmap_table(char *table_data, struct font_s *font);
int get_encodings_table(char *table_data, struct font_s *font);
int get_swidths_table(char *table_data, struct font_s *font);

struct font_s *pcf_create_empty_font();
struct font_s *pcf_load_font_file(char *file_name);
struct font_s *pcf_load_font(char *file_name, unsigned char *file_data, long file_size);
int pcf_write_to_file(FILE *file, struct font_s *font);
void pcf_handle_hw_change(struct font_s *font, char *newdata, long new_datasize);
int pcf_create_unitab(struct font_s *font);
void bdf_kill_unitab(struct font_s *font);
void pcf_kill_unitab(struct font_s *font);
void pcf_handle_unicode_table_change(struct font_s *font, char old_has_unicode_table);
void pcf_handle_version_change(struct font_s *font, char old_version);
void pcf_convert_to_psf(struct font_s *font);
void pcf_shrink_glyphs(struct font_s *font, int old_length);
void pcf_expand_glyphs(struct font_s *font, int old_length, int option);
long pcf_make_utf16_unitab(struct font_s *new_font, unsigned short **_unicode_table);
void pcf_init_module();

#endif

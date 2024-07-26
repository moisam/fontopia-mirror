/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: font_ops.h
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

#ifndef FONTOPS_H
#define FONTOPS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include "modules/modules.h"

//check endianness of the system
static inline int is_big_endian()
{
    int x = 1;
    return ((((char *)&x)[0]) == 0);
}

extern int big_endian;

//This function will swap the bytes from big-endian to little-endian
//and vice versa, for 32-bit dwords.
static inline uint32_t swap_dword(uint32_t x)
{
    return ((x>>24)&0xff) |      // move byte 3 to byte 0
               ((x<<8)&0xff0000) |   // move byte 1 to byte 2
               ((x>>8)&0xff00) |     // move byte 2 to byte 1
               ((x<<24)&0xff000000); // byte 0 to byte 3
}

//This function will swap the bytes from big-endian to little-endian
//and vice versa, for 16-bit words.
static inline uint16_t swap_word(uint16_t x)
{
    return ((x>>8)&0xff) |      // move byte 1 to byte 0
               ((x<<8)&0xff00);    // move byte 0 to byte 1
}

static inline char reverse_char(char c)
{
    char c2 = 0;
    int j = 7;

    for( ; j >= 0; j--)
    {
        c2 |= ((c & 0x1) << j);
        c >>= 1;
    }

    return c2;
}


enum file_state
{
  MODIFIED, NEW, NEW_MODIFIED, IDLE, OPENED,
};

struct font_s
{
    unsigned int length;            /* # of glyphs in font */
    unsigned char has_unicode_table;
#define MAX_WIDTH       64
#define MAX_HEIGHT      64
    unsigned int height,            /* height of glyph */
             width,            /* width of glyph in bits */
             charsize;            /* # of bytes occupied by glyph */
#define VER_PSF1        0x1
#define VER_PSF2        0x2
#define VER_CP          0x3
#define VER_RAW         0x4
#define VER_BDF         0x5
#define VER_PCF         0x6
    unsigned char version;            /* PSF version */
    unsigned int data_size;            /* total size of glyph data */
    unsigned char *data;            /* pointer to glyph data */
    unsigned char *unicode_info;        /* pointer to unicode info at end of font file */
    unsigned int unicode_info_size;
    int unicode_index, unicode_array_index;    /* used internally by viewing routines in view.c */
    char current_zoom;
    char max_zoom;
    enum file_state state;
    void *raw_data;                /* data as is read/written to disk, not used by PSF or RAW */
    long raw_data_size;
    /* we need fast access to the unicode table, without the overhead of unicode_info table,
     * as it contains unicode data as found exactly in the file, which include control sequences
     * like 0xFFFF and 0xFFFE which we don't need regularly. If you need to know just what
     * unicode chars are included in this font, use the table below.
     */
    unsigned int *unicode_table;        /* list of unicode entries of this font */
    unsigned short *unicode_table_index;    /* index to unicode_table */
    unsigned char utf_version;        /* format of unicode table: 1=UTF16, 2=UTF8 */
    /* these fields are for use with Code Page (CP) and Windows FON files. */
    char cp_active_font;
    char cp_total_fonts;
    /* which module is handling this font? */
    struct module_s *module;
    void *file_hdr;
    unsigned int header_size;
    char has_metadata;    /* flag to indicate if font has metadata (properties)
                                 * e.g. like BDF & PCF
                                 */
    void *metadata;        /* pointer to metadata table if font has_metadata */
    void *char_info;        /* pointer to auxiliary char info array (BDF only) */
    unsigned int char_info_size;
};


struct char_info_s {
    // for BDF fonts
    int encoding;
    int swidthX, swidthY;
    int dwidthX, dwidthY;
    int BBw, BBh, BBXoff, BBYoff;
    // for PCF fonts
    int rBearing, lBearing;
    int charAscent, charDescent;
};


static inline void get_unitab_entry(struct font_s *font, int i, unsigned int **arr)
{
    if(sizeof(unsigned int *) == 8)        /* take care of 64-bit CPUs */
    {
        unsigned long p = 
            (unsigned long)((font->unicode_table[(i)*2]) |
            ((unsigned long)(font->unicode_table[((i)*2)+1]) << 32));
        *arr = (unsigned int *)p;
    }
    else
    {
        unsigned long p = (unsigned long)(font->unicode_table[(i)*2]);
        *arr = (unsigned int *)p;
    }
}


//void display_font_data(struct font_s *font, unsigned char *data);
//void display_unicode_info(struct font_s *font);

void get_font_unicode_table(struct font_s *font);
void font_toggle_active_bit(struct font_s *font);
int write_to_file(FILE *file, struct font_s *font);
void free_unicode_table(struct font_s *font);
void kill_font(struct font_s *font);
struct font_s *new_font_file(struct font_s *font);
struct font_s *open_font_file(struct font_s *font);
struct font_s *save_font_file(struct font_s *font, int force_new);
int check_font_saved(struct font_s *font, int alert_user);

struct font_s *load_font_file(char* file_name);
struct font_s *create_empty_font();
void force_font_dirty(struct font_s *font);
int create_empty_unitab(struct font_s *font);
int make_utf16(unsigned int *res, unsigned char *utf8);
int create_char_info(struct font_s *font);

#endif

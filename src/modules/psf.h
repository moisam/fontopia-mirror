/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: psf.h
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

#ifndef PSF_H
#define PSF_H

#include "../defs.h"
#include "../view.h"
#include "../glyph.h"
#include "modules.h"

#define PSF1_MAGIC0	0x36
#define PSF1_MAGIC1	0x04

#define PSF1_MODE512	0x01
#define PSF1_MODEHASTAB	0x02
#define PSF1_MODEHASSEQ	0x04
#define PSF1_MAXMOD	0x05

#define PSF1_SEPARATOR	0xFFFF
#define PSF1_STARTSEQ	0xFFFE

struct psf1_header
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
};

#define PSF2_MAGIC0	0x72
#define PSF2_MAGIC1	0xb5
#define PSF2_MAGIC2	0x4a
#define PSF2_MAGIC3	0x86

#define PSF2_HAS_UNICODE_TABLE	0x01
#define PSF2_MAXVERSION	0

#define PSF2_SEPARATOR	0xFF
#define PSF2_STARTSEQ	0xFE

struct psf2_header
{
	unsigned char magic[4];
	unsigned int version;
	unsigned int headersize;
	unsigned int flags;
	unsigned int length;		/* # of glyphs */
	unsigned int charsize;		/* char size in bytes */
	unsigned int height, width;	/* glyph size */
	/* charsize = height * ((width + 7) / 8) */
};


static inline int verify_psf2_hdr(struct psf2_header *hdr)
{
	if(hdr->magic[0] == PSF2_MAGIC0 &&
	   hdr->magic[1] == PSF2_MAGIC1 &&
	   hdr->magic[2] == PSF2_MAGIC2 &&
	   hdr->magic[3] == PSF2_MAGIC3)
		return 1;
	return 0;
}

static inline int verify_psf1_hdr(struct psf1_header *hdr)
{
	if(hdr->magic[0] == PSF1_MAGIC0 &&
	   hdr->magic[1] == PSF1_MAGIC1)
		return 1;
	return 0;
}


struct font_s *psf_create_empty_font();
int psf_write_to_file(FILE *file, struct font_s *font);
struct font_s *psf_load_font_file(char *file_name);
struct font_s *psf_load_font(char *file_name, unsigned char *file_data, long file_size);
void psf_init_module();
void psf_handle_hw_change(struct font_s *font, char *newdata, long new_datasize);
void psf_shrink_glyphs(struct font_s *font, int old_length);
void psf_expand_glyphs(struct font_s *font, int old_length, int option);
void psf_handle_unicode_table_change(struct font_s *font, char old_has_unicode_table);
void psf_kill_unitab(struct font_s *font);
void psf_update_font_hdr(struct font_s *font);
long psf_make_utf16_unitab(struct font_s *new_font, short unsigned int** unicode_table);

#endif

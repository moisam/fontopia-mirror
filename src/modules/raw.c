/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: raw.c
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

#include "raw.h"
#include "../defs.h"
#include "../view.h"

int raw_acceptable_filesize(long file_size)
{
    return (file_size == 2048 || file_size == 3072 || file_size == 3584 ||
            file_size == 4096 || file_size == 6144 || file_size == 7168 ||
            file_size == 8192 || file_size == 1536 || file_size == 2560);
}

void reverse_glyph_rows(struct font_s *font)
{
    int i, j, k;
    int line_bytes = (font->width + 7) / 8;
    char *data = (char *)font->data, *d;
    char buf[font->charsize], *p;

    if(big_endian) return;

    for(i = 0; i < (int)font->length; i++)
    {
        for(d = data, p = buf, j = 0; j < (int)font->height; j++)
        {
            for(k = 0; k < line_bytes; k++)
            {
                p[k] = reverse_char(d[line_bytes-k-1]);
            }

            d += line_bytes;
            p += line_bytes;
        }

        memcpy(data, buf, font->charsize);
        data += font->charsize;
    }
}

struct font_s *raw_create_empty_font()
{
    struct font_s *font = (struct font_s *)NULL;
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    
    font->length = 256;
    font->has_unicode_table = 0;
    font->height   = 8;
    font->width    = 8;
    font->charsize = font->height;
    font->version = get_version("RAW");
    font->file_hdr = (void *)NULL;
    font->header_size = 0;
    font->utf_version = 0;
    font->data_size = font->height*font->length;
    font->data = (unsigned char *)malloc(font->data_size);
    if(!font->data) goto memory_error;
    memset((void *)font->data, 0, font->data_size);
    font->state = NEW;
    calc_max_zoom(font);

    font->module = get_module_by_name("raw");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;

    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error creating new font");
    goto end;

memory_error:

    status_error("Insufficient memory");

end:

    kill_font(font);
    return (struct font_s *)NULL;
}


struct font_s *raw_load_font_file(char *file_name)
{
    FILE *font_file = (FILE *)NULL;
    char *file_data = (char *)NULL;
    struct font_s *font = (struct font_s *)NULL;
    if(!file_name) return (struct font_s *)NULL;

    if(!(font_file = fopen(file_name, "rb")))
    {
        status_error("Error opening file");
        return (struct font_s *)NULL;
    }
    
    long i;
    i = fseek(font_file, 0, SEEK_END);
    long file_size = ftell(font_file);

    if(!file_size)
    {
        status_error("Error: empty file!");
        fclose(font_file);
        return (struct font_s *)NULL;
    }

    if(!raw_acceptable_filesize(file_size))
    {
        status_error("Error: Invalid file size");
        fclose(font_file);
        return (struct font_s *)NULL;
    }
    
    rewind(font_file);
    file_data = (char *)malloc(file_size);
    if(!file_data) goto memory_error;
    i = fread(file_data, 1, file_size, font_file);
    if(i != file_size) goto file_read_error;
    fclose(font_file);
    font = raw_load_font(file_name, (unsigned char *)file_data, file_size);
    free(file_data);
    return font;
    
file_read_error:

    status_error("Error reading font file");
    goto end;

memory_error:

    status_error("Not enough memory to load font file");

end:

    if(file_data) free(file_data);
    fclose(font_file);
    font_file = (FILE *)NULL;
    return (struct font_s *)NULL;
}

struct filesize_s
{
    long filesize;
    char *namepart;
    unsigned int width, height;
    unsigned int charsize;
} file_candidates[] =
{
    { 1536, "4x6", 4, 6, 6 },
    { 2048, "5x8", 5, 8, 8 },
    { 2048, "6x8", 6, 8, 8 },
    { 2048, "7x8", 7, 8, 8 },
    { 2048, "8x8", 8, 8, 8 },
    { 2560, "5x10", 5, 10, 10 },
    { 2560, "6x10", 6, 10, 10 },
    { 2560, "7x10", 7, 10, 10 },
    { 2560, "8x10", 8, 10, 10 },
    { 3072, "5x12", 5, 12, 12 },
    { 3072, "6x12", 6, 12, 12 },
    { 3072, "7x12", 7, 12, 12 },
    { 3072, "8x12", 8, 12, 12 },
    { 3584, "5x14", 5, 14, 14 },
    { 3584, "6x14", 6, 14, 14 },
    { 3584, "7x14", 7, 14, 14 },
    { 3584, "8x14", 8, 14, 14 },
    { 4096, "5x16", 5, 16, 16 },
    { 4096, "6x16", 6, 16, 16 },
    { 4096, "7x16", 7, 16, 16 },
    { 4096, "8x16", 8, 16, 16 },
    { 6144, "10x12", 10, 12, 24 },
    { 7168, "10x14", 10, 14, 28 },
    { 7168, "12x14", 12, 14, 28 },
    { 8192, "10x16", 10, 16, 32 },
    { 8192, "12x16", 12, 16, 32 },
    { 8192, "14x16", 14, 16, 32 },
    { 8192, "16x16", 16, 16, 32 },
    { 0, NULL, 0, 0, 0 }
};

struct font_s *raw_load_font(char *file_name, unsigned char *file_data, long file_size)
{
    int i;
    struct font_s *font = (struct font_s *)NULL;
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    
    // try to guess glyph size from file size and file name
    for(i = 0; file_candidates[i].namepart; i++)
    {
        if(file_candidates[i].filesize != file_size) continue;

        if(strstr(file_name, file_candidates[i].namepart))
        {
            font->height = file_candidates[i].height;
            font->width = file_candidates[i].width;
            font->charsize = file_candidates[i].charsize;
            break;
        }
    }
    
    // nothing found, so try to guess from the file size
    if(file_candidates[i].namepart == NULL)
    {
        if(startup_rawfont_width >= 4 && startup_rawfont_width <= 128 &&
           startup_rawfont_height >= 4 && startup_rawfont_height <= 128)
        {
            font->height   = startup_rawfont_height;
            font->width    = startup_rawfont_width;
            font->charsize = ((font->width+7)/8)*font->height;
        }
        else
        {
            if(file_size == 2048)
            {
                font->height   = 8;
                font->width    = 8;
                font->charsize = 8;
            }
            else if(file_size == 4096)
            {
                font->height   = 16;
                font->width    = 8;
                font->charsize = 16;
            }
            else if(file_size == 8192)
            {
                font->height   = 16;
                font->width    = 16;
                font->charsize = 32;
            }
            else
            {
                status_error("Error: Invalid file size");
                goto end;
            }
        }
    }

    font->length = 256;
    font->has_unicode_table = 0;
    //font->height   = (file_size == 2048) ? 8 : 16;
    //font->width    = 8;
    //font->charsize = font->height;
    font->version = get_version("RAW"); //VER_RAW;
    font->file_hdr = (void *)NULL;
    font->header_size = 0;
    font->utf_version = 0;
    font->data = (unsigned char *)malloc(file_size);
    if(!font->data) goto memory_error;
    memcpy((void *)font->data, (void *)file_data, file_size);
    font->data_size = file_size;
    font->state = OPENED;
    calc_max_zoom(font);
    reverse_glyph_rows(font);

    font->module = get_module_by_name("raw");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;

    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error loading font file");
    goto end;

memory_error:

    status_error("Insufficient memory");
    goto end;

end:

    kill_font(font);
    return (struct font_s *)NULL;
}


int raw_write_to_file(FILE *file, struct font_s *font)
{
    if(!file || !font) return 1;

    int i, j, k;
    int line_bytes = (font->width + 7) / 8;
    char *data = (char *)font->data;
    char buf[font->charsize], *p;

    for(i = 0; i < (int)font->length; i++)
    {
        for(p = buf, j = 0; j < (int)font->height; j++)
        {
            for(k = 0; k < line_bytes; k++)
            {
                p[k] = reverse_char(data[line_bytes-k-1]);
            }

            data += line_bytes;
            p += line_bytes;
        }

        if(fwrite(buf, 1, font->charsize, file) != font->charsize) return 1;
    }

    //int res;
    //res = fwrite(font->data, 1, font->data_size, file);
    //if(res != font->data_size) return 1;

    return 0;
}


void raw_handle_hw_change(struct font_s *font, char *newdata, long new_datasize)
{
    long old_datasize = font->data_size;

    /* width changed but did not affect data length - nothing to do here */
    if(!newdata) return;

    /* shrinking data - easy one, no new memory allocation */
    if(new_datasize < old_datasize)
    {
        memcpy((void *)font->data, (void *)newdata, new_datasize);
        font->data_size = new_datasize;
    }
    /* expanding data - we need memory reallocation */
    else
    {
        void *new_rawdata = (void *)malloc(new_datasize);
        /* FIXME: Handle this error more decently */
        if(!new_rawdata) return;
        memcpy((void *)new_rawdata, (void *)newdata, new_datasize);
        free(font->data);
        font->data = new_rawdata;
        font->data_size = new_datasize;
    }
}

/*
void raw_export_unitab(struct font_s *font, FILE *f)
{
  status_error("Raw font has no unicode table");
  return;
}
*/

int raw_create_unitab(struct font_s *font __attribute__((unused)))
{
    status_error("Raw fonts have no unicode table");
    return 0;
}

void raw_kill_unitab(struct font_s *font)
{
    free_unicode_table(font);
    font->unicode_info_size = 0;
    if(font->unicode_info) free(font->unicode_info);
    font->unicode_info = 0;
    font->has_unicode_table = 0;
}


void raw_handle_unicode_table_change(struct font_s *font)
{
    /***********************/
    /* remove unicode info */
    /***********************/
    if(!font->has_unicode_table)
    {
        //raw_kill_unitab(font);
    }
    else
    {
        status_error("Raw fonts have no Unicode tables");
    }
}


void raw_handle_version_change(struct font_s *font, 
                               char old_version __attribute__((unused)))
{
    font->header_size = 0;
    if(font->file_hdr) free(font->file_hdr);
    font->file_hdr = 0;
    raw_kill_unitab(font);
}


void raw_convert_to_psf(struct font_s *font)
{
    if(font->version == VER_PSF1)
    {
        struct psf1_header hdr;
        hdr.magic[0] = PSF1_MAGIC0;
        hdr.magic[1] = PSF1_MAGIC1;
        hdr.mode = 0;
        hdr.charsize = font->charsize;
        /* shift font structure */
        long sz = sizeof(struct psf1_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr, sizeof(struct psf1_header));
        font->file_hdr = new_hdr;
        font->header_size = sz;
    }
    else if(font->version == VER_PSF2)
    {
        struct psf2_header hdr2;
        hdr2.magic[0] = PSF2_MAGIC0;
        hdr2.magic[1] = PSF2_MAGIC1;
        hdr2.magic[2] = PSF2_MAGIC2;
        hdr2.magic[3] = PSF2_MAGIC3;
        hdr2.version = 0;
        hdr2.length = font->length;
        hdr2.charsize = font->charsize;
        hdr2.height = font->height;
        hdr2.width = font->width;
        hdr2.headersize = sizeof(struct psf2_header);
        hdr2.flags = 0;
        /* shift font structure */
        long sz = sizeof(struct psf2_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr2, sizeof(struct psf2_header));
        font->file_hdr = new_hdr;
        font->header_size = sz;
    }

    force_font_dirty(font);
}

/*
void raw_shrink_glyphs(struct font_s *font, int old_length)
{
  psf_shrink_glyphs(font, old_length);
}

void raw_expand_glyphs(struct font_s *font, int old_length, int option)
{
  psf_expand_glyphs(font, old_length, option);
}
*/

int raw_is_acceptable_width(struct font_s *font)
{
    return (font->width >= 4 && font->width <= 16);
}

int raw_next_acceptable_width(struct font_s *font)
{
    if(font->width < 16) return font->width + 1;
    else return 4;
}

int raw_is_acceptable_height(struct font_s *font)
{
    return (font->height >= 6 && font->height <= 16);
}

int raw_next_acceptable_height(struct font_s *font)
{
    if(font->height < 16) return font->height + 1;
    else return 6;
}

/********************************
 * ******************************
 * ******************************/
struct module_s raw_module;

void raw_init_module()
{
    strcpy(raw_module.mod_name, "raw");
    raw_module.max_width = 8;
    raw_module.max_height = 16;
    raw_module.max_length = 256;
    raw_module.create_empty_font = raw_create_empty_font;
    raw_module.write_to_file = raw_write_to_file;
    raw_module.load_font = raw_load_font;
    raw_module.load_font_file = raw_load_font_file;
    raw_module.handle_hw_change = raw_handle_hw_change;
    raw_module.shrink_glyphs = NULL;//raw_shrink_glyphs;
    raw_module.expand_glyphs = NULL;//raw_expand_glyphs;
    raw_module.update_font_hdr = NULL;
    raw_module.handle_version_change = raw_handle_version_change;
    raw_module.handle_unicode_table_change = raw_handle_unicode_table_change;
    raw_module.export_unitab = NULL; //raw_export_unitab;
    //raw_module.create_unitab = raw_create_unitab;
    //raw_module.kill_unitab = raw_kill_unitab;
    raw_module.convert_to_psf = raw_convert_to_psf;
    raw_module.make_utf16_unitab = NULL;
    raw_module.is_acceptable_width = raw_is_acceptable_width;
    raw_module.next_acceptable_width = raw_next_acceptable_width;
    raw_module.is_acceptable_height = raw_is_acceptable_height;
    raw_module.next_acceptable_height = raw_next_acceptable_height;
    register_module(&raw_module);
    //add_file_extension("fnt", "raw");
    //add_file_extension("", "raw");
}


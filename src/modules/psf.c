/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: psf.c
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

#include "psf.h"
#include "modules.h"

extern char buffer_mode_on;    /* main.c */

/* used to create empty new fonts */
unsigned short default_unicode_table[] =
{
  0xfffd, 0x2248, 0x0152, 0x0153, 0x25c6, 0x2409, 0x240c, 0x240d, 0x240a,
  0x2591, 0x2592, 0x2593, 0x2588, 0x2584, 0x2580, 0x258c, 0x2590, 0x2424,
  0x240b, 0x2264, 0x2265, 0x2260, 0x25c0, 0x25b6, 0x2191, 0x2193, 0x2192,
  0x2190, 0x2195, 0x2194, 0x21b5, 0x03c0, 0x0020, 0x0021, 0x0022, 0x0023,
  0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c,
  0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
  0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e,
  0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
  0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050,
  0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
  0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062,
  0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b,
  0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
  0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
  0x007e, 0x0178, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6,
  0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
  0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8,
  0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 0x2423, 0x00a1,
  0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa,
  0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3,
  0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc,
  0x00bd, 0x00be, 0x00bf, 0xf801, 0x2575, 0x2576, 0x2514, 0x2577, 0x2502,
  0x250c, 0x251c, 0x2574, 0x2518, 0x2500, 0x2534, 0x2510, 0x2524, 0x252c,
  0x253c, 0xf803, 0x2579, 0x257a, 0x2517, 0x257b, 0x2503, 0x250f, 0x2523,
  0x2578, 0x251b, 0x2501, 0x253b, 0x2513, 0x2563, 0x2533, 0x254b, 0x00e0,
  0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9,
  0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 0x00f0, 0x00f1, 0x00f2,
  0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb,
  0x00fc, 0x00fd, 0x00fe, 0x00ff
};

/* these are borrowed from modules.c for module init'ing */
extern struct module_s first_module;
extern struct file_ext_s file_extensions[MAX_FILE_EXTENSIONS];
extern struct file_sig_s first_sig;


struct font_s *psf_create_empty_font()
{
    struct font_s *font = (struct font_s *)NULL;
    
    /* create in-memory PSF1 header */
    struct psf1_header *hdr = (struct psf1_header *)malloc(sizeof(struct psf1_header));
    if(!hdr) goto memory_error;
    hdr->magic[0] = PSF1_MAGIC0;
    hdr->magic[1] = PSF1_MAGIC1;
    hdr->mode = PSF1_MODEHASTAB;
    hdr->charsize = 16;
    
    /* create our font structure */
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));

    font->length = 256;
    font->has_unicode_table = 1;
    font->utf_version = VER_PSF1;
    font->height   = 16;
    font->width    = 8;
    font->charsize = font->height;
    font->version  = get_version("PSF1");
    font->data_size = font->length * font->charsize;
    font->data = (unsigned char *)malloc(font->data_size);
    if(!font->data) goto memory_error;
    memset(font->data, 0, font->data_size);
    font->file_hdr = hdr;
    font->header_size = sizeof(struct psf1_header);
    font->unicode_info_size = (256 * 4);
    font->unicode_info = (unsigned char *)malloc(font->unicode_info_size);
    if(!font->unicode_info) goto memory_error;

    int i, j = 0;
    unsigned short *data = (unsigned short *)font->unicode_info;
    for(i = 0; i < (int)font->length; i++)
    {
        data[j  ] = default_unicode_table[i];
        data[j+1] = 0xFFFF;
        j += 2;
    }

    create_empty_unitab(font);
    get_font_unicode_table(font);
    calc_max_zoom(font);
    font->state = NEW;
    font->module = get_module_by_name("psf");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;
    
    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error creating new font");
    goto go;

memory_error:

    status_error("Not enough memory to create new font");

go:

    kill_font(font);
    return (struct font_s *)NULL;
}


int psf_write_to_file(FILE *file, struct font_s *font)
{
    int res;

    /* 1- write the header */
    res = fwrite(font->file_hdr, 1, font->header_size, file);
    if(res != (int)font->header_size) return 1;

    /* 2- write glyph data */
    if(font->width <= 8)
    {
        res = fwrite(font->data, 1, font->data_size, file);
        if(res != (int)font->data_size) return 1;
    }
    else
    {
        /* swap glyph data if width is more than 1 byte */
        if(font->width <= 16)
        {
            int i;
            for(i = 0; i < (int)font->data_size; i += 2)
            {
                res = fputc(font->data[i+1], file);
                res = fputc(font->data[i  ], file);
            }
        }
        else if(font->width <= 32)
        {
            int i;
            for(i = 0; i < (int)font->data_size; i += 4)
            {
                res = fputc(font->data[i+3], file);
                res = fputc(font->data[i+2], file);
                res = fputc(font->data[i+1], file);
                res = fputc(font->data[i  ], file);
            }
        }
    }

    /* 3- write unicode table */
    if(font->has_unicode_table)
    {
        if(font->utf_version == VER_PSF1)
        {
            long unicode_size = font->unicode_info_size;
            if(font->version == VER_PSF1)
            {
                res = fwrite(font->unicode_info, 1, unicode_size, file);
                if(res != unicode_size) return 1;
            }
            /* we need conversion as the file was PSF1 but user changed it to PSF2 */
            else if(font->version == VER_PSF2)
            {
                /* convert from PSF1 (UTF-16) to PSF2 (UTF-8) */
                int i = 0, j = 0;
                unsigned short *u = (unsigned short *)font->unicode_info;
                unsigned short term = 0xFF;
                unsigned short ss = 0xFE;
                unsigned char c[5];

                while(i < (int)font->length)
                {
                    if(u[j] == 0xFFFF)
                    {
                        res = fwrite(&term, 1, 1, file);
                        if(res != 1) return 1;
                        i++;
                    }
                    else if(u[j] == 0xFFFE)
                    {
                        res = fwrite(&ss, 1, 1, file);
                        if(res != 1) return 1;
                    }
                    else
                    {
                        memset(c, 0, 5);
                        unsigned int c2 = (unsigned int)(u[j]);
                        int k = make_utf8(c, c2);
                        res = fwrite(c, 1, k, file);
                        if(res != k) return 1;
                    }
                    j++;
                }
            }
        }
        /////////////////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        else if(font->utf_version == VER_PSF2)
        {
            long unicode_size = font->unicode_info_size;
            if(font->version == VER_PSF2)
            {
                res = fwrite(font->unicode_info, 1, unicode_size, file);
                if(res != unicode_size) return 1;
            }
            /* we need conversion as the file was PSF2 but user changed it to PSF1 */
            else if(font->version == VER_PSF1)
            {
                /* convert from PSF2 (UTF-8) to PSF1 (UTF-16) */
                int i = 0, j = 0;
                unsigned char *u = (unsigned char *)font->unicode_info;
                unsigned short term = 0xFFFF;
                unsigned short ss = 0xFFFE;

                while(i < (int)font->length)
                {
                    if(u[j] == 0xFF)
                    {
                        res = fwrite(&term, 1, 2, file);
                        if(res != 2) return 1;
                        i++; j++;
                    }
                    else if(u[j] == 0xFE)
                    {
                        res = fwrite(&ss, 1, 2, file);
                        if(res != 2) return 1;
                        j++;
                    }
                    else
                    {
                        int bytes = 1;
                        unsigned int r;
                        bytes = make_utf16(&r, &u[j]);
                        j += bytes;
                        int res2 = fwrite(&r, 1, 2, file);
                        if(res2 != 2) return 1;
                    }
                }
            }
        }
    }

    return 0;
}

struct font_s *psf_load_font_file(char *file_name)
{
    long i;
    FILE *font_file = (FILE *)NULL;
    struct font_s *font = (struct font_s *)NULL;
    unsigned char *file_data = (unsigned char *)NULL;
    if(!(font_file = fopen(file_name, "rb"))) return (struct font_s *)NULL;
    i = fseek(font_file, 0, SEEK_END);
    long file_size = ftell(font_file);
    if(!file_size) goto file_read_error;
    /* File size should be AT LEAST as expected, or more. */
    //if(file_size < expected_file_size)
    //  goto file_corrupt;
    rewind(font_file);
    file_data = (unsigned char *)malloc(file_size);
    if(!file_data) goto memory_error;
    i = fread(file_data, 1, file_size, font_file);
    if(i != file_size) goto file_read_error;
    fclose(font_file);
    font = psf_load_font(file_name, file_data, file_size);
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

struct font_s *psf_load_font(char *file_name, unsigned char *file_data, long file_size)
{
    struct font_s *font = (struct font_s *)NULL;
    struct psf2_header *hdr = (struct psf2_header *)file_data;
    struct psf1_header *hdr_old = (struct psf1_header *)hdr;

    (void)file_name;
    
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    long expected_file_size = 0;
    
    /* Is it PSF2? */
    if(verify_psf2_hdr(hdr))
    {
        font->length = hdr->length;
        font->has_unicode_table = (hdr->flags & PSF2_HAS_UNICODE_TABLE);
        font->height   = hdr->height;
        font->width    = hdr->width;
        font->charsize = hdr->charsize;
        font->version = get_version("PSF2"); //VER_PSF2;
        if(!hdr->headersize) goto file_corrupt;
        struct psf2_header *file_hdr = (struct psf2_header *)malloc(sizeof(struct psf2_header));
        if(!file_hdr) goto memory_error;
        memcpy((void *)file_hdr, (void *)hdr, sizeof(struct psf2_header));
        font->file_hdr = file_hdr;
        font->header_size = hdr->headersize;
        font->utf_version = VER_PSF2;
        expected_file_size = hdr->headersize;
    }
    /* Is it PSF1? */
    else if(verify_psf1_hdr(hdr_old))
    {
        if(hdr_old->mode & PSF1_MODE512)
            font->length = 512;
        else    font->length = 256;
        font->has_unicode_table = (hdr_old->mode & PSF1_MODEHASTAB);
        font->height   = hdr_old->charsize;
        font->width    = 8;
        font->charsize = hdr_old->charsize;
        font->version  = get_version("PSF1"); //VER_PSF1;
        struct psf1_header *file_hdr = (struct psf1_header *)malloc(sizeof(struct psf1_header));
        if(!file_hdr) goto memory_error;
        memcpy((void *)file_hdr, (void *)hdr_old, sizeof(struct psf1_header));
        font->file_hdr = file_hdr;
        font->header_size = sizeof(struct psf1_header);
        font->utf_version = VER_PSF1;
        expected_file_size = sizeof(struct psf1_header);
    }
    else
    {
        //msgBox("This is not a valid PSF font file!", OK, ERROR);
        goto file_read_error;
    }
    
    /* basic error checking */
    if(!font->length || !font->height || !font->width || !font->charsize)
    {
        goto file_corrupt;
    }

    expected_file_size += (font->length * font->charsize);

    if(font->version == VER_PSF1 && font->has_unicode_table)
    {
      expected_file_size += (font->length * 4);
    }
    else
    {
      /* TODO: Add calculation of expected Unicode info size
       *       for PSF2.
       */
    }

    font->data_size = font->length * font->charsize;
    font->data = (unsigned char *)malloc(font->data_size);
    if(!font->data) goto memory_error;
    if(font->version == VER_PSF1)
        memcpy((void *)font->data, (void *)(file_data+sizeof(struct psf1_header)), font->data_size);
    else
        memcpy((void *)font->data, (void *)(file_data+hdr->headersize), font->data_size);
    
    /* swap glyph data if width is more than 1 byte */
    if(font->width > 8)
    {
        if(font->width <= 16)
        {
            int i;
            for(i = 0; i < (int)font->data_size; i += 2)
            {
                unsigned char c = font->data[i];
                font->data[i] = font->data[i+1];
                font->data[i+1] = c;
            }
        }
        else if(font->width <= 32)
        {
            int i;
            for(i = 0; i < (int)font->data_size; i += 4)
            {
                unsigned char c = font->data[i];
                font->data[i] = font->data[i+3];
                font->data[i+3] = c;
                c = font->data[i+1];
                font->data[i+1] = font->data[i+2];
                font->data[i+2] = c;
            }
        }
    }
    
    if(font->has_unicode_table)
    {
        unsigned int unicode_size = file_size-font->data_size;
        if(font->version == VER_PSF1)
            unicode_size -= sizeof(struct psf1_header);
        else
            unicode_size -= font->header_size;
        font->unicode_info_size = unicode_size;
        font->unicode_info = (unsigned char *)malloc(font->unicode_info_size);
        if(!font->unicode_info) goto memory_error;
        memcpy((void *)font->unicode_info,
               (void *)(file_data+font->header_size+font->data_size),
               unicode_size);
    }
    else font->unicode_info = 0;

    if(!create_empty_unitab(font)) goto memory_error;
    get_font_unicode_table(font);

    calc_max_zoom(font);
    font->state = OPENED;
    font->module = get_module_by_name("psf");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;

    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error loading font file");
    goto end;

file_corrupt:

    status_error("Font file is corrput!");
    goto end;

file_read_error:

    status_error("Error reading font file");
    goto end;

memory_error:

    status_error("Not enough memory to load font file");

end:

    kill_font(font);
    return (struct font_s *)NULL;
}


void psf_handle_hw_change(struct font_s *font, char *newdata, long new_datasize)
{
    /********************/
    /* Handle PSF 1 & 2 */
    /********************/

    if(newdata)
    {
        long new_rawdata_size = new_datasize;
        void *new_rawdata = (void *)malloc(new_rawdata_size);
        /* FIXME: Handle this error more decently */
        if(!new_rawdata) return;
        memcpy((void *)(new_rawdata), (void *)newdata, new_datasize);
        free(font->data);
        font->data = (unsigned char *)(new_rawdata);
        font->data_size = new_datasize;
    }

    /* update font header */
    if(font->version == 1)
    {
        struct psf1_header *hdr = (struct psf1_header *)font->file_hdr;
        hdr->charsize = font->charsize;
    }
    else if(font->version == 2)
    {
        struct psf2_header *hdr = (struct psf2_header *)font->file_hdr;
        hdr->charsize = font->charsize;
        hdr->height = font->height;
        hdr->width = font->width;
    }
}


void psf_shrink_glyphs(struct font_s *font, int old_length __attribute__((unused)))
{
    long new_datasize = font->length * font->charsize;
    unsigned int unicode_size = 0;
  
    if(font->has_unicode_table)
    {
        unsigned char *old_unicode = font->unicode_info;
        int cnt = font->length;
        long bytes = 0;

        /* PSF1 Unicode entry is fixed at 4 bytes, but we may have multiple entries
           per glyph. */
        if(font->utf_version == VER_PSF1)
        {
            int i = 0;
            unsigned short *info = (unsigned short *)old_unicode;

            while(i < cnt)
            {
                while(info[bytes] != PSF1_SEPARATOR) bytes++;
                bytes++; i++;
            }

            /* double the bytes as PSF1 uses 16-bit UTF */
            bytes <<= 1;  
        }
        /* PSF2 is another story. We have to manually count bytes as it is UTF-8. */
        else if(font->utf_version == VER_PSF2)
        {
            int i = 0;
            while(i < cnt)
            {
                while(old_unicode[bytes] != PSF2_SEPARATOR) bytes++;
                bytes++; i++;
            }  
        }

        unicode_size = bytes;
        unsigned char *new_unicode = (unsigned char *)malloc(unicode_size);
        if(!new_unicode) return;
        memcpy((void *)new_unicode, (void *)old_unicode, unicode_size);
        if(font->unicode_info) free(font->unicode_info);
        font->unicode_info = new_unicode;
        font->unicode_info_size = unicode_size;
    }

    font->data_size = new_datasize;
    force_font_dirty(font);
}


void psf_kill_unitab(struct font_s *font)
{
    free_unicode_table(font);
    font->unicode_info_size = 0;
    if(font->unicode_info) free(font->unicode_info);
    font->unicode_info = 0;
    font->has_unicode_table = 0;
}


void psf_expand_glyphs(struct font_s *font, int old_length, int option)
{
    long new_datasize = font->length * font->charsize;
    long old_datasize = old_length * font->charsize;
    unsigned char *old_unicodeinfo = (unsigned char *)(font->unicode_info);
    unsigned int old_unicode_size = font->unicode_info_size;
    int unicode_additional_bytes = 0;

    /* we need to calculate the size of additional Unicode info */
    if(font->has_unicode_table)
    {
        int cnt = font->length - old_length;
        long bytes = 0;

        /* Roll over */
        if(option == 2)
        {
            /* PSF1 Unicode entry is fixed at 4 bytes, but we may have multiple entries
               per glyph. */
            if(font->utf_version == VER_PSF1)
            {
                int i = 0;
                unsigned short *info = (unsigned short *)font->unicode_info;

                while(i < cnt)
                {
                    while(info[bytes] != PSF1_SEPARATOR) bytes++;
                    bytes++; i++;
                }

                /* double the bytes as PSF1 uses 16-bit UTF */
                bytes <<= 1;
            }
            /* PSF2 is another story. We have to manually count bytes as it is UTF-8. */
            else if(font->utf_version == VER_PSF2)
            {
                int i = 0;
                while(i < cnt)
                {
                    while(font->unicode_info[bytes] != PSF2_SEPARATOR) bytes++;
                    bytes++; i++;
                }
            }
        }
        /* Add empty glyph Unicode info entries */
        else
        {
            if(font->utf_version == VER_PSF1)      bytes = cnt*4;
            else if(font->utf_version == VER_PSF2) bytes = cnt*2;
        }

        unicode_additional_bytes = bytes;
    }

    /* now to copy the data */
    void *new_rawdata = (void *)malloc(new_datasize);

    /* FIXME: Handle this error more decently */
    if(!new_rawdata)
    {
        status_error("Error modifying font length (in psf_expand_glyphs function)");
        return;
    }
  
    //memcpy((void *)new_rawdata, (void *)font->raw_data, font->header_size);
    memcpy((void *)(new_rawdata), (void *)font->data, old_datasize);
    free(font->data);
    font->data = (unsigned char *)(new_rawdata);

    unsigned char *data = (unsigned char *)new_rawdata + old_datasize;
    int count = new_datasize-old_datasize;

    /* Roll over */
    if(option == 2)
    {
        memcpy((void *)data, (void *)font->data, count);
    }
    /* Add empty glyphs */
    else
    {
        memset((void *)data, 0, count);
    }
  
    if(font->has_unicode_table)
    {
        unsigned char *new_unicodeinfo = malloc(old_unicode_size+
                                                unicode_additional_bytes);

        /* FIXME: Handle this error more decently */
        if(!new_unicodeinfo)
        {
            status_error("Error modifying font length (in expand_glyphs function)");
            return;
        }

        memcpy((void *)new_unicodeinfo, (void *)old_unicodeinfo, old_unicode_size);

        /* Roll over */
        if(option == 2)
        {
            memcpy((void *)(new_unicodeinfo+old_unicode_size), 
                   (void *)old_unicodeinfo, unicode_additional_bytes);
        }
        /* Add empty values */
        else
        {
            memset((void *)(new_unicodeinfo+old_unicode_size), 
                   0, unicode_additional_bytes);

            if(font->utf_version == VER_PSF1)
            {
                unsigned short *ndata = 
                        (unsigned short *)(new_unicodeinfo+old_unicode_size);
                int i;

                for(i = 1; i < unicode_additional_bytes/2; i += 2)
                {
                    ndata[i] = 0xFFFF;
                }
            }
            else if(font->utf_version == VER_PSF2)
            {
                int i;
                for(i = 1; i < unicode_additional_bytes/2; i += 2)
                {
                    data[i] = 0xFF;
                }
            }
        }

        free(font->unicode_info);
        font->unicode_info = new_unicodeinfo;
        font->unicode_info_size = old_unicode_size+unicode_additional_bytes;
    }

    font->data_size = new_datasize;
    force_font_dirty(font);
}


void psf_handle_unicode_table_change(struct font_s *font)
{
    /***********************/
    /* remove unicode info */
    /***********************/
    if(!font->has_unicode_table)
    {
        /* update header */
        psf_update_font_hdr(font);
    }
    /***************************/
    /* make empty unicode info */
    /***************************/
    else
    {
        if(font->version == VER_PSF1)
        {
            struct psf1_header *hdr = (struct psf1_header *)font->file_hdr;
            hdr->mode |= PSF1_MODEHASTAB;
        }
        else if(font->version == VER_PSF2)
        {
            struct psf2_header *hdr = (struct psf2_header *)font->file_hdr;
            hdr->flags |= PSF2_HAS_UNICODE_TABLE;
        }
    }
}


void psf_update_font_hdr(struct font_s *font)
{
    if(font->version == 1)
    {
        struct psf1_header *hdr = (struct psf1_header *)font->file_hdr;
        if(font->length == 512) hdr->mode |= PSF1_MODE512;
        else hdr->mode &= ~PSF1_MODE512;
        if(font->has_unicode_table) hdr->mode |= PSF1_MODEHASTAB;
        else hdr->mode &= ~PSF1_MODEHASTAB;
    }
    else if(font->version == 2)
    {
        struct psf2_header *hdr = (struct psf2_header *)font->file_hdr;
        hdr->length = font->length;
        if(font->has_unicode_table) hdr->flags |= PSF2_HAS_UNICODE_TABLE;
        else hdr->flags &= ~PSF2_HAS_UNICODE_TABLE;
    }
}


void psf_handle_version_change(struct font_s *font, 
                               char old_version __attribute__((unused)))
{
    if(font->version == VER_PSF1)
    {
        struct psf1_header hdr;
        hdr.magic[0] = PSF1_MAGIC0;
        hdr.magic[1] = PSF1_MAGIC1;
        hdr.mode = 0;
        if(font->length == 512) hdr.mode |= PSF1_MODE512;
        if(font->has_unicode_table) hdr.mode |= PSF1_MODEHASTAB;
        hdr.charsize = font->charsize;
        long sz = sizeof(struct psf1_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr, sizeof(struct psf1_header));
        if(font->file_hdr) free(font->file_hdr);
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
        if(font->has_unicode_table) hdr2.flags |= PSF2_HAS_UNICODE_TABLE;

        long sz = sizeof(struct psf2_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr2, sizeof(struct psf2_header));
        if(font->file_hdr) free(font->file_hdr);
        font->file_hdr = new_hdr;
        font->header_size = sz;
    }
}


void psf_convert_to_psf(struct font_s *font)
{
    psf_handle_version_change(font, font->version);
}


long psf_make_utf16_unitab(struct font_s *new_font, unsigned short **_unicode_table)
{
    long unicode_table_len = 0;
    unsigned short *unicode_table = *_unicode_table;

    if(new_font->utf_version != VER_PSF2)
    {
        unicode_table = (unsigned short *)malloc(new_font->unicode_info_size);
        if(!unicode_table) goto error;
        memcpy((void *)unicode_table, (void *)new_font->unicode_info, new_font->unicode_info_size);
        unicode_table_len = new_font->unicode_info_size;
    }
    else
    {
        long draft_index = 0;
        long draft_bytes = new_font->length*sizeof(unsigned short)*2;
        unicode_table = (unsigned short *)malloc(draft_bytes);
        if(!unicode_table) goto error;
        unsigned int i = 0;
        int cnt = 0;

        unsigned int c = 0;
        new_font->unicode_array_index = 0;
        new_font->unicode_index = 0;
        do
        {
            c = get_next_utf(new_font);
            unicode_table[draft_index++] = c;

            if(c == PSF1_SEPARATOR)
            {
                i++; cnt = 0;
                continue;
            }
            else if(c == PSF1_STARTSEQ)
            {
                /* we didn't take sequences into account when allocating
                 * memory to our draft array. we will need to realloc it.
                 */
                int bytes = 0;
                while(new_font->unicode_info
                        [new_font->unicode_array_index+bytes] 
                            != PSF2_SEPARATOR) bytes++;

                /* add extra byte for the separator */
                bytes = (bytes+1) * sizeof(unsigned short);
                unicode_table = 
                   (unsigned short *)realloc(unicode_table, draft_bytes+bytes);
                if(!unicode_table) goto error;
                draft_bytes += bytes;

                do
                {
                    c = get_next_utf(new_font);
                    unicode_table[draft_index++] = c;
                } while(c != PSF1_SEPARATOR);

                i++; cnt = 0;
                continue;
            }
            else
            {
                cnt++;
                if(cnt > 1)
                {
                    int bytes = 0;
                    int m = new_font->unicode_array_index;
                    while(new_font->unicode_info[m+bytes] != PSF2_SEPARATOR) bytes++;
                    bytes += 2;
                    bytes *= sizeof(unsigned short);

                    if(bytes)
                    {
                        unicode_table = 
                            (unsigned short *)realloc(unicode_table, draft_bytes+bytes);
                        if(!unicode_table) goto error;
                        draft_bytes += bytes;

                        do
                        {
                            c = get_next_utf(new_font);
                            unicode_table[draft_index++] = c;
                        } while(c != PSF1_SEPARATOR);
                        i++; cnt = 0;
                    }
                }
            }  
        } while(i < new_font->length);

        unicode_table_len = draft_bytes;
    }

    *_unicode_table = unicode_table;
    return unicode_table_len;

error:

    return 0;
}

int psf_is_acceptable_width(struct font_s *font)
{
    if(font->version == VER_PSF1) return (font->width == 8);
    else return (font->width >= 4 && font->width <= 32);
}

int psf_next_acceptable_width(struct font_s *font)
{
    if(font->version == VER_PSF1) return 8;
    else if(font->width < 32) return font->width + 1;
    else return 4;
}

int psf_is_acceptable_height(struct font_s *font)
{
    return (font->height >= 4 && font->height <= 32);
}

int psf_next_acceptable_height(struct font_s *font)
{
    if(font->height < 32) return font->height + 1;
    else return 4;
}

/********************************
 * ******************************
 * ******************************/
struct file_sig_s psf1_sig = { 0, 2, { PSF1_MAGIC0, PSF1_MAGIC1, 0 }, 
                               NULL, NULL, NULL };
struct file_sig_s psf2_sig = { 0, 4, { PSF2_MAGIC0, PSF2_MAGIC1, 
                                       PSF2_MAGIC2, PSF2_MAGIC3, 0 },
                               NULL, NULL, NULL };

void psf_init_module()
{
    strcpy(first_module.mod_name, "psf");
    first_module.max_width = 32;
    first_module.max_height = 32;
    first_module.max_length = 512;
    first_module.create_empty_font = psf_create_empty_font;
    first_module.load_font_file = psf_load_font_file;
    first_module.load_font = psf_load_font;
    first_module.write_to_file = psf_write_to_file;
    first_module.handle_hw_change = psf_handle_hw_change;
    first_module.shrink_glyphs = psf_shrink_glyphs;
    first_module.expand_glyphs = psf_expand_glyphs;
    first_module.update_font_hdr = psf_update_font_hdr;
    first_module.handle_unicode_table_change = psf_handle_unicode_table_change;
    first_module.handle_version_change = psf_handle_version_change;
    first_module.export_unitab = NULL; //psf_export_unitab;
    //first_module.create_unitab = NULL; //psf_create_unitab;
    //first_module.kill_unitab = psf_kill_unitab;
    first_module.convert_to_psf = psf_convert_to_psf;
    first_module.make_utf16_unitab = psf_make_utf16_unitab;
    first_module.is_acceptable_width = psf_is_acceptable_width;
    first_module.next_acceptable_width = psf_next_acceptable_width;
    first_module.is_acceptable_height = psf_is_acceptable_height;
    first_module.next_acceptable_height = psf_next_acceptable_height;
    add_file_extension("psf", "psf");
    add_file_extension("psfu", "psf");
    struct file_sig_s *f = &first_sig;
    memcpy((void *)f, (void *)&psf1_sig, sizeof(struct file_sig_s));
    f->module = &first_module;
    //add_file_signature(&psf1_sig, "psf");
    add_file_signature(&psf2_sig, "psf");
}


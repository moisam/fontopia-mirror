/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: font_ops.c
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

#include "defs.h"
#include "view.h"
#include "glyph.h"
#include "font_ops.h"
#include "modules/cp.h"
#include "modules/raw.h"
#include "modules/psf.h"
#include "modules/modules.h"
#include "metadata.h"
#include "menu.h"

struct font_s *create_empty_font()
{
    struct font_s *font;

    if((font = psf_create_empty_font()))
    {
        refresh_view_status_msg("New file", font);
    }

    return font;
}

int make_utf16(unsigned int *res, unsigned char *utf8)
{
    unsigned char ch = *utf8;
    unsigned int utf16 = 0;

    if((ch & 0xF0) == 0xF0)
    {
        utf16 = (unsigned int)(ch & ~0xF0) << 18;
        ch = utf8[1];
        utf16 |= (unsigned int)(ch & ~0x80) << 12;
        ch = utf8[2];
        utf16 |= (unsigned int)(ch & ~0x80) << 6;
        ch = utf8[3];
        utf16 |= (unsigned int)(ch & ~0x80);
        *res = utf16;
        return 4;
    }

    if((ch & 0xE0) == 0xE0)
    {
        utf16 = (unsigned int)(ch & ~0xE0) << 12;
        ch = utf8[1];
        utf16 |= (unsigned int)(ch & ~0x80) << 6;
        ch = utf8[2];
        utf16 |= (unsigned int)(ch & ~0x80);
        *res = utf16;
        return 3;
    }

    if((ch & 0xC0) == 0xC0)
    {
        utf16 = (unsigned int)(ch & ~0xC0) << 6;
        ch = utf8[1];
        utf16 |= (unsigned int)(ch & ~0x80);
        *res = utf16;
        return 2;
    }
    else
    {
        utf16 = (unsigned int)ch;
        *res = utf16;
        return 1;
    }

    return utf16;
}

extern unsigned short utf_mask[];    /* view.c */


int write_to_file(FILE *file, struct font_s *font)
{
    if(!font->module->write_to_file)
    {
        return psf_write_to_file(file, font);
    }

    return font->module->write_to_file(file, font);
}

struct font_s *load_font_file(char *file_name)
{
    FILE *font_file = (FILE *)NULL;
    char *file_data = (char *)NULL;
    struct font_s *font = (struct font_s *)NULL;
    if(!file_name) return (struct font_s *)NULL;

    /* 
     * try with file extension first
     */
    struct module_s *mod = check_file_ext(file_name);
    if(mod)
    {
        font = mod->load_font_file(file_name);

        if(font && left_window.width < (int)font->width)
        {
            status_error("Error: Font width is larger than screen width");
            //msgBox("Font width is larger than screen width.", OK, ERROR);
            goto end;
        }

        if(font && left_window.height < (int)font->height)
        {
            status_error("Error: Font height is larger than screen height");
            //msgBox("Font height is larger than screen height.", OK, ERROR);
            goto end;
        }

        if(font) refresh_view_status_msg(file_name, font);
        return font;
    }

    /*
     * try to brute-force read the file
     */
    if(!(font_file = fopen(file_name, "rb")))
    {
        status_error("Error opening file");
        return (struct font_s *)NULL;
    }

    long i;
    i = fseek(font_file, 0, SEEK_END);
    long file_size = ftell(font_file);
    if(!file_size) goto file_read_error;
    rewind(font_file);
    file_data = (char *)malloc(file_size);
    if(!file_data) goto memory_error;

    i = fread(file_data, 1, file_size, font_file);
    if(i != file_size) goto file_read_error;

    mod = check_file_signature((unsigned char *)file_data);
    if(mod)
    {
        font = mod->load_font(file_name, (unsigned char *)file_data, file_size);
        fclose(font_file);
        free(file_data);
        if(font) refresh_view_status_msg(file_name, font);
        return font;
    }

    /*
     * Try to open it as a RAW font file.
     */
    if(raw_acceptable_filesize(file_size))
    //if(file_size == 2048 || file_size == 4096)
    {
        mod = get_module_by_name("raw");
        if(mod)
        {
            font = mod->load_font(file_name, (unsigned char *)file_data, file_size);
            if(font)
            {
                fclose(font_file);
                free(file_data);
                setScreenColors(WHITE, BGDEFAULT);
                drawBox(1, 1, SCREEN_H, SCREEN_W, " Fontopia for GNU/Linux ", 0);
                /* not an error per se, but to get user's attention!! */
                status_error("Assuming a RAW font. If it is NOT, close it NOW!");
                refresh_left_window(font);
                refresh_right_window(font);
                return font;
            }
        }
    }

    /*
     * nope. we don't know this file type
     */
    status_error("Unknown font format");
    //return (struct font_s *)NULL;
    goto end;
    
file_read_error:

    status_error("Error reading font file");
    goto end;

memory_error:

    status_error("Not enough memory to load font file");
end:

    if(file_data) free(file_data);
    if(font_file)
    {
      fclose(font_file);
      font_file = (FILE *)NULL;
    }

    return (struct font_s *)NULL;
}

void get_font_unicode_table(struct font_s *font)
{
    if(!font->has_unicode_table)
    {
        return;
    }

    int i = 0;
    unsigned int c = 0;
    font->unicode_array_index = 0;
    font->unicode_index = 0;

    do
    {
        /* multiple entries for the same Unicode value will need
         * separate allocation outside these tables. In this case,
         * unicode_table_index will be 0xFFFFFFFF, and unicode_table
         * entry will be a pointer to malloc'ed memory containing this info.
         */
        c = get_next_utf(font);

        if(c == PSF1_SEPARATOR)
        {
            i++;
            continue;
        }
        else if(c == PSF1_STARTSEQ)
        {
            /*
             * NOTE: we are discarding unicode sequences.
             * TODO: fix this!.
             */
            do
            {
                c = get_next_utf(font);
            } while(c != PSF1_SEPARATOR);
            i++;
            continue;
        }
        else
        {
            /* normal storage */
            if(font->unicode_table_index[i] == 0)
            {
                font->unicode_table_index[i] = (unsigned short)i;
                font->unicode_table[(i)*2] = c;
                continue;
            }

            if(font->unicode_table_index[i] != 0xFFFF)
            {
                font->unicode_table_index[i] = 0xFFFF;
                /* this is actually a pointer! to a one-dimensional 
                 * MAX_UNICODE_TABLE_ENTRIES member array of unsigned ints.
                 */
                unsigned int *ptr = malloc(MAX_UNICODE_TABLE_ENTRIES * 
                                                sizeof(unsigned int));

                /* WARNING: we should handle this error properly */
                if(!ptr) break;

                memset((void *)ptr, 0, MAX_UNICODE_TABLE_ENTRIES * sizeof(unsigned int));

                unsigned long p = (unsigned long)ptr;
                /* save the first item */
                ptr[0] = font->unicode_table[(i)*2];
                font->unicode_table[(i)*2] = (unsigned int)(p & 0xFFFFFFFF);

                if(sizeof(unsigned int *) == 8)        /* take care of 64-bit CPUs */
                {
                    font->unicode_table[((i)*2)+1] = 
                            (unsigned int)((p >> 32) & 0xFFFFFFFF);
                }
            }

            unsigned int *arr = 0;
            get_unitab_entry(font, i, &arr);

            int j = 0;
            do
            {
                if(arr[j] == 0)
                {
                    arr[j] = c; break;
                }
            } while(j++ < MAX_UNICODE_TABLE_ENTRIES);
            //i--;
        }
    } while(i < (int)font->length);
}


void font_toggle_active_bit(struct font_s *font)
{
    int linebytes = font->width/8;
    if(font->width%8) linebytes++;
    int byteindex = left_window.cursor.row*linebytes;
    int glyph = get_glyph_index(font);
    int w = linebytes * 8;
    byteindex += glyph;
    byteindex += (int)((w-left_window.cursor.col-1)/8);
    int bitindex  = 1 << ((w - left_window.cursor.col - 1) % 8);
    font->data[byteindex] ^= (char)bitindex;
    if(font->state == NEW || font->state == NEW_MODIFIED) font->state = NEW_MODIFIED;
    else font->state = MODIFIED;
}


void free_unicode_table(struct font_s *font)
{
    if(!font->unicode_table_index) return;
    int i = 0;

    while(i < (int)font->length)
    {
        if(font->unicode_table_index[i] == 0xFFFF)
        {
            unsigned int *arr = 0;
            get_unitab_entry(font, i, &arr);
            free(arr);
        }
        i++;
    }

    free(font->unicode_table_index);
    free(font->unicode_table);
    font->unicode_table = 0;
    font->unicode_table_index = 0;
}

int check_font_saved(struct font_s *font, int alert_user)
{
    FILE *save;
    int res;
    char *buf;

    if(!font) goto cancelled;
    if(alert_user == 2) goto as_new;

    if(font->state == MODIFIED)
    {
        if(!alert_user) goto save_only;
        res = msgBox("Font is not saved. Save?", BUTTON_YES|BUTTON_NO, INFO);
        if(res == BUTTON_ABORT) goto cancelled;
        if(res == BUTTON_YES) goto save_only;
        if(res == BUTTON_NO) goto ret;
    }
    else if(font->state == NEW_MODIFIED)
    {
        if(font_file_name) goto save_only;
        res = msgBox("Font is not saved. Save?", BUTTON_YES|BUTTON_NO, INFO);
        if(res == BUTTON_ABORT) goto cancelled;
        if(res == BUTTON_NO) goto ret;

as_new:

        res = show_opensave(".", SAVE, &buf, 0);
        if(res == OPENSAVE_ERROR) goto error;
        if(res == OPENSAVE_CANCEL) goto cancelled;
        if(font_file_name) free(font_file_name);
        font_file_name = buf;

save_only:

        /* save to buffer? */
        if(buffer_mode_on)
        {
            int len = strlen(font_file_name);
            char *file_name2 = (char *)malloc(len+2);
            if(!file_name2) goto error;
            strcpy(file_name2, font_file_name);
            file_name2[len] = '~';
            file_name2[len+1] = '\0';

            if(!(save = fopen(file_name2, "wb+")))
            {
                free(file_name2);
                goto error;
            }

            res = write_to_file(save, font);
            fclose(save);
            free(file_name2);
            if(res) goto error;
            status_msg("Saved into buffer");
            goto ret;
        }

        /* write to original file */
        if(!(save = fopen(font_file_name, "wb+"))) goto error;
        res = write_to_file(save, font);
        fclose(save);
        if(res) goto error;
    }
    //else if(buffer_mode_on) goto save_only;

    hideCursor();
    return 0;

ret:

    hideCursor();
    return -2;
  
cancelled:

    hideCursor();
    return -1;

error:

    refresh_view_status_error("Error saving file. Use ^S to save or ^Q to quit.", font);
    hideCursor();
    return 1;
}

void kill_font(struct font_s *font)
{
    if(font_file_name) free(font_file_name);
    font_file_name = (char *)NULL;

    if(font)
    {
        //free(font->raw_data);
        free_unicode_table(font);
        if(font->file_hdr) free(font->file_hdr);
        if(font->unicode_info) free(font->unicode_info);
        if(font->raw_data) free(font->raw_data);

        if(font->version != VER_CP)
        {
            if(font->data) free(font->data);
        }

        if(font->has_metadata)
        {
            struct metadata_item_s *meta = (struct metadata_item_s *)font->metadata;
            if(meta)
            {
                int i;
                for(i = 0; i < metadata_table_len; i++)
                {
                    if(meta[i].is_str)
                    if(meta[i].value2) free(meta[i].value2);
                }
            }
        }

        if(font->char_info) free(font->char_info);
        free(font);
    }
}

int create_empty_unitab(struct font_s *font)
{
    if(font->unicode_table) free_unicode_table(font);
    font->unicode_table = (unsigned int *)malloc(font->length * sizeof(unsigned int) * 2);
    if(!font->unicode_table) return 0;
    font->unicode_table_index = (unsigned short *)malloc(font->length * sizeof(unsigned short));
    if(!font->unicode_table_index) return 0;

    long s = font->length * sizeof(unsigned short);
    memset((void *)font->unicode_table_index, 0, s);
    s = font->length * sizeof(unsigned int) * 2;
    memset((void *)font->unicode_table, 0, s);
    return 1;
}

int create_char_info(struct font_s* font)
{
    unsigned int sz = font->length*sizeof(struct char_info_s);
    if(font->char_info) free(font->char_info);
    struct char_info_s *char_info = (struct char_info_s *)malloc(sz);
    if(!char_info) return 0;
    memset(char_info, 0, sz);
    font->char_info = (void *)char_info;
    font->char_info_size = sz;

    for(sz = 0; sz < font->length; sz++)
    {
        char_info[sz].encoding    = sz;
        char_info[sz].dwidthX     = font->width;
        char_info[sz].lBearing    = 0;
        char_info[sz].rBearing    = font->width;
        char_info[sz].charAscent  = font->height;
        char_info[sz].charDescent = 0;
        char_info[sz].BBw         = font->width;
        char_info[sz].BBh         = font->height;
        char_info[sz].BBXoff      = 0;
        char_info[sz].BBYoff      = 0;
        char_info[sz].swidthX     = 1000;
        char_info[sz].swidthY     = 0;
    }

    return 1;
}

struct font_s *new_font_file(struct font_s *font)
{
    int res = check_font_saved(font, 1);
    if(res == 1 || res == -1) return font;    /* error or cancelled */
    struct font_s *f;
  
    int mod_count = get_registered_modules()+1;
    char *mod_names[mod_count];
    int m;

    for(m = 0; m < mod_count; m++)
    {
        mod_names[m] = get_version_str(m+1);
    }

    m = show_menu(mod_names, mod_count, "Choose font format", 0, 0, 0, 0);
    if(m < 0) return font;
    struct module_s *mod = (m < 2) ? get_module_by_name("psf") :
                           get_module_by_name(mod_names[m]);

    if(!mod) return font;
    if(!mod->create_empty_font)
    {
        status_error("Error in font module!");
        return font;
    }

    f = mod->create_empty_font();

    if(!f) return (struct font_s *)NULL;

    kill_font(font);
    if(font_file_name) free(font_file_name);
    font_file_name = 0;
    return f;
}

struct font_s *open_font_file(struct font_s *font)
{
    int res;
    res = check_font_saved(font, 1);
    if(res == 1 || res == -1) return font;    /* error or cancelled */

    char *buf;
    res = show_opensave(".", OPEN, &buf, 0);

    if(res == OPENSAVE_ERROR)
    {
        refresh_view_status_error("Error opening font.", font);
        return font;
    }
    else if(res == OPENSAVE_CANCEL) return font;
  
    struct font_s *f = load_font_file(buf);

    if(f)
    {
        kill_font(font);
        if(font_file_name) free(font_file_name);
        font_file_name = buf;
        return f;
    }

    /* there was an error */
    //status_error("Error reading font file");
    return font;
}

struct font_s *save_font_file(struct font_s *font, int force_new)
{
    enum file_state old_state = font->state;
    if(font->state == NEW) font->state = NEW_MODIFIED;
    //if(force_new) font->state = NEW_MODIFIED;
    int res;

    if(force_new || !font_file_name)
         res = check_font_saved(font, 2);
    else res = check_font_saved(font, 0);

    if(res == 0)
    {
        font->state = IDLE;
        char *st = (char *)malloc(strlen(font_file_name)+8);
        if(!st) status_msg(font_file_name);
        else
        {
            strcpy(st, "Saved: ");
            strcat(st, font_file_name);
            status_msg(st);
            free(st);
        }
    }
    else font->state = old_state;

    return font;
}

void force_font_dirty(struct font_s *font)
{
    if(font->state == NEW || font->state == NEW_MODIFIED) font->state = NEW_MODIFIED;
    else font->state = MODIFIED;
}


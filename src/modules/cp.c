/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: cp.c
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

#include <ncurses.h>
#include "cp.h"
#include "cp_include.h"
#include "../defs.h"
#include "../view.h"

int cp_files_count = 33+49;
int cp_files_original_ones = 33;    /* the others are mapped to these "original" ones */

char *cp_files[] =
{
    NULL, "667.fcp",  "668.fcp",  "720.fcp",  "737.fcp",
    "770.fcp",  "771.fcp",  "772.fcp",  "773.fcp",
    "774.fcp",  "775.fcp",  "850.fcp",  "851.fcp",
    "852.fcp",  "853.fcp",  "855.fcp",  "856.fcp",
    "857.fcp",  "859.fcp",  "860.fcp",  "861.fcp",
    "862.fcp",  "863.fcp",  "864.fcp",  "865.fcp",
    "866.fcp",  "867.fcp",  "869.fcp",  "874.fcp",
    "895.fcp",  "912.fcp",  "915.fcp",  "1117.fcp",
    /* non-IBM */
    "851.fcp",  "857.fcp",  "912.fcp",  "720.fcp",
    "720.fcp",  "720.fcp",  "720.fcp",  "720.fcp",
    "720.fcp",  NULL,       "720.fcp",  "720.fcp",
    "720.fcp",  "720.fcp",  "770.fcp",  "771.fcp",
    "775.fcp",  "667.fcp",  "855.fcp",  "855.fcp",
    "855.fcp",  "850.fcp",  NULL,       "855.fcp",
    "850.fcp",  "850.fcp",  "850.fcp",  "850.fcp",
    "850.fcp",  NULL,       "855.fcp",  NULL,
    NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,
    NULL,       "720.fcp",  "667.fcp",  NULL,
    "1117.fcp", "774.fcp",  "774.fcp",  "855.fcp",  "855.fcp",
    "",
};

short cp_ids[] =
{
    437, 667,  668,  720,  737,
    770,  771,  772,  773,
    774,  775,  850,  851,
    852,  853,  855,  856,
    857,  859,  860,  861,
    862,  863,  864,  865,
    866,  867,  869,  874,
    895,  912,  915,  1117,
    /* non-IBM */
    111,  112,  113,  151,
    161,  162,  163,  164,
    165,  220,  708,  709,
    710,  711,  776,  777,
    778,  790,  808,  848,
    849,  858,  868,  872,
    881,  882,  883,  884,
    885,  891,  900,  932,
    934,  936,  938,  942,
    943,  944,  948,  949,
    950,  966,  991,  1098,
    1116, 1118, 1119, 1125, 1131,
    -1,
};

int show_cp_selection_dialog();


struct font_s *cp_create_empty_font()
{
    struct font_s *font = (struct font_s *)NULL;
    struct cp_header *hdr = (struct cp_header *)NULL;
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    hdr = (struct cp_header *)malloc(sizeof(struct cp_header));
    if(!hdr) goto memory_error;

    font->length = 256;
    font->has_unicode_table = 0;
    font->height   = 8;
    font->width    = 8;
    font->charsize = font->height;
    font->version = get_version("CP");
    font->utf_version = 0;
    font->data_size = font->height*font->length;
  
    struct screen_font_header fhdr;
    hdr->entry_hdr.cpeh_size = sizeof(struct cp_entry_header)+sizeof(struct cp_info_header);
    hdr->entry_hdr.next_cpeh_offset = 0;
    hdr->entry_hdr.device_type = 1;    /* 1 screen, 2 printer */
    hdr->entry_hdr.device_name[0] = 'V';
    hdr->entry_hdr.device_name[1] = 'I';
    hdr->entry_hdr.device_name[2] = 'D';
    hdr->entry_hdr.device_name[3] = 'E';
    hdr->entry_hdr.device_name[4] = 'O';
    hdr->entry_hdr.device_name[5] = ' ';
    hdr->entry_hdr.device_name[6] = ' ';
    hdr->entry_hdr.device_name[7] = ' ';
    short codepage = cp_ids[show_cp_selection_dialog()];
    hdr->entry_hdr.codepage = codepage;
    hdr->entry_hdr.res[0] = 0;
    hdr->entry_hdr.res[1] = 0;
    hdr->entry_hdr.res[2] = 0;
    hdr->entry_hdr.res[3] = 0;
    hdr->entry_hdr.res[4] = 0;
    hdr->entry_hdr.res[5] = 0;
    hdr->entry_hdr.cpih_offset = 0;

    hdr->info_hdr.version = 0;
    hdr->info_hdr.num_fonts = 1;
    hdr->info_hdr.size = font->length*font->charsize;
  
    fhdr.height = font->height;
    fhdr.width = font->width;
    fhdr.res = 0;
    fhdr.num_chars = font->length;

    font->file_hdr = hdr;
    font->header_size = sizeof(struct cp_header);
    long sz = sizeof(struct screen_font_header)+font->data_size;
    unsigned char *data = (unsigned char *)malloc(sz);
    if(!data) goto memory_error;
    memset((void *)data, 0, sz);
    memcpy((void *)(data), (void *)&fhdr, sizeof(struct screen_font_header));
    font->data = (data+sizeof(struct screen_font_header));
    font->raw_data = data;
    font->raw_data_size = sz;
    font->cp_active_font = 0;
    font->cp_total_fonts = 1;
    font->state = NEW;
    calc_max_zoom(font);
    font->module = get_module_by_name("cp");

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

    if(hdr) free(hdr);
    if(font) kill_font(font);
    return (struct font_s *)NULL;
}


int cp_write_to_file(FILE *file, struct font_s *font)
{
    if(!file || !font) return 1;
    size_t res;
    /* 1- write the header */
    res = fwrite(font->file_hdr, 1, font->header_size, file);
    if(res != font->header_size) return 1;
    /* 2- write glyph data */
    res = fwrite(font->raw_data, 1, font->raw_data_size, file);
    if(res != (size_t)font->raw_data_size) return 1;
    return 0;
}


struct font_s *cp_load_font_file(char *file_name)
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
    
    rewind(font_file);
    file_data = (char *)malloc(file_size);
    if(!file_data) goto memory_error;
    i = fread(file_data, 1, file_size, font_file);
    if(i != file_size) goto file_read_error;
    fclose(font_file);
    font = cp_load_font(file_name, (unsigned char *)file_data, file_size);
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


struct font_s *cp_load_font(char *file_name, unsigned char *file_data, long file_size)
{
    struct font_s *font = (struct font_s *)NULL;
    struct cp_header *hdr = (struct cp_header *)file_data;
    int i;

    (void)file_name;

    /* Basic error checking */
    if(!hdr->entry_hdr.cpeh_size) goto corrupt_file;
    if(hdr->entry_hdr.device_type != 1 && hdr->entry_hdr.device_type != 2)
        goto corrupt_file;
    for(i = 0; i < 6; i++)
        if(hdr->entry_hdr.res[i]) goto corrupt_file;
    if(hdr->info_hdr.version < 0 && hdr->info_hdr.version > 2)
        goto corrupt_file;
    if(hdr->info_hdr.num_fonts != 1 && hdr->info_hdr.num_fonts != 3 &&
       hdr->info_hdr.num_fonts != 4)
        goto corrupt_file;
    
    struct screen_font_header *first_font_hdr = 
        (struct screen_font_header *)(file_data+sizeof(struct cp_header));
    char height = first_font_hdr->height;
    if(height != 6 && height != 8 && height != 14 && height != 16)
      goto corrupt_file;

    if(first_font_hdr->width != 8) goto corrupt_file;
    if(first_font_hdr->res != 0) goto corrupt_file;
    if(first_font_hdr->num_chars != 256) goto corrupt_file;

    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    
    font->length = first_font_hdr->num_chars;
    font->has_unicode_table = 0;
    font->height   = first_font_hdr->height;
    font->width    = first_font_hdr->width;
    font->charsize = first_font_hdr->height;
    font->version = get_version("CP"); //VER_CP;
    struct cp_header *cphdr = (struct cp_header *)malloc(sizeof(struct cp_header));
    if(!cphdr) goto memory_error;
    memcpy((void *)cphdr, (void *)file_data, sizeof(struct cp_header));
    font->file_hdr = cphdr;
    font->header_size = sizeof(struct cp_header);

    font->raw_data_size = file_size-sizeof(struct cp_header);
    font->raw_data = (void *)malloc(font->raw_data_size);
    if(!font->raw_data) goto memory_error;
    memcpy((void *)font->raw_data, (void *)(file_data+sizeof(struct cp_header)), font->raw_data_size);
    //font->raw_data = file_data;
    //font->data = (unsigned char *)(first_font_hdr);
    font->data = (unsigned char *)font->raw_data;
    font->data += sizeof(struct screen_font_header);
    font->data_size = font->charsize*font->length;
    font->cp_active_font = 0;
    font->cp_total_fonts = hdr->info_hdr.num_fonts;
    
    font->has_unicode_table = 1;
    font->utf_version = VER_PSF1;
    cp_handle_unicode_table_change(font);
    
    font->state = OPENED;
    calc_max_zoom(font);
    
    font->module = get_module_by_name("cp");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;

    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error creating new font");
    //msgBox("Error creating new font", OK, ERROR);
    goto end;

memory_error:

    status_error("Insufficient memory");
    //msgBox("Insufficient memory", OK, ERROR);
    goto end;

corrupt_file:

    status_error("Invalid or corrupt file");
    //msgBox("Invalid or corrupt file", OK, ERROR);
    goto end;

end:

    kill_font(font);
    return (struct font_s *)NULL;
}


void cp_change_active_font(struct font_s *font, char cp_index)
{
    if(font->version != VER_CP)
    {
        status_error("Not a CP font");
        return;
    }

    if(cp_index < 1 || cp_index > font->cp_total_fonts) goto index_error;
  
    char index = 1;
    struct screen_font_header *first_font_hdr = 
                (struct screen_font_header *)(font->raw_data);
    char height = 0;

check:

    height = first_font_hdr->height;
    if(height != 6 && height != 8 && height != 14 && height != 16)
        goto corrupt_data;
    if(first_font_hdr->width != 8) goto corrupt_data;
    if(first_font_hdr->res != 0) goto corrupt_data;
    if(first_font_hdr->num_chars != 256) goto corrupt_data;

    if(index < cp_index)
    {
        index++;
        unsigned char *ptr = (unsigned char *)first_font_hdr;
        long sz = (first_font_hdr->num_chars * first_font_hdr->height) + 
                                        sizeof(struct screen_font_header);
        ptr += sz;
        first_font_hdr = (struct screen_font_header *)ptr;
        goto check;
    }
  
    font->length = first_font_hdr->num_chars;
    font->height   = first_font_hdr->height;
    font->width    = first_font_hdr->width;
    font->charsize = first_font_hdr->height;
    font->data = (unsigned char *)(first_font_hdr);
    font->data += sizeof(struct screen_font_header);
    font->data_size = font->charsize*font->length;
    font->cp_active_font = index-1;
    calc_max_zoom(font);
    refresh_view_status_msg(font_file_name, font);
    return;
  
corrupt_data:

    status_error("Corrupt font data");
    return;

index_error:

    status_error("Code page does not exist");
    return;
}


void cp_handle_hw_change(struct font_s *font, char *newdata, long new_datasize)
{
    if(newdata)
    {
        unsigned char *rest_of_data = (unsigned char *)(font->data+font->data_size);
        long sz = rest_of_data - (unsigned char *)font->raw_data;
        long rest_of_data_size = 0;
        long old_datasize = font->data_size;

        if(sz < font->raw_data_size)
        {
            rest_of_data_size = font->raw_data_size - sz;
        }
  
        /* shrinking data - easy one, no new memory allocation */
        if(new_datasize < old_datasize)
        {
            memcpy((void *)font->data, (void *)newdata, new_datasize);
            font->data_size = new_datasize;

            if(rest_of_data_size)
            {
                memcpy((void *)(font->data+new_datasize), (void *)rest_of_data, rest_of_data_size);
            }

            font->raw_data_size -= (old_datasize-new_datasize);  
        }
        /* expanding data - we need memory reallocation */
        else
        {
            long new_rawdata_size = font->raw_data_size+new_datasize-old_datasize;
            void *new_rawdata = (void *)malloc(new_rawdata_size);
            /* FIXME: Handle this error more decently */
            if(!new_rawdata) return;
            //memcpy((void *)new_rawdata, (void *)font->raw_data, font->header_size);
            sz = (font->data - (unsigned char *)font->raw_data);
            memcpy((void *)new_rawdata, (void *)font->raw_data, sz);
            memcpy((void *)((unsigned char *)new_rawdata+sz), (void *)newdata, new_datasize);

            if(rest_of_data_size)
            {
                memcpy((void *)((unsigned char *)new_rawdata+sz+new_datasize), 
                                        (void *)rest_of_data, rest_of_data_size);
            }

            free(font->raw_data);
            font->raw_data = new_rawdata;
            font->raw_data_size = new_rawdata_size;
        }
    }
  
    /* adjust header */
    char index = 0;
    struct screen_font_header *first_font_hdr = 
                (struct screen_font_header *)((unsigned char *)font->raw_data);

check:

    if(index < font->cp_active_font)
    {
        index++;
        unsigned char *ptr = (unsigned char *)first_font_hdr;
        long sz = (first_font_hdr->num_chars * first_font_hdr->height) + 
                                            sizeof(struct screen_font_header);
        ptr += sz;
        first_font_hdr = (struct screen_font_header *)ptr;
        goto check;
    }

    first_font_hdr->height = font->height;
    first_font_hdr->width = font->width;
    first_font_hdr->num_chars = font->length;
    /* adjust pointers */
    font->charsize = font->height*((font->width+7)/8);
    font->data = (unsigned char *)(first_font_hdr);
    font->data += sizeof(struct screen_font_header);
    font->data_size = font->charsize*font->length;
    //cp_change_active_font(font, font->cp_active_font+1);
}

/*
void cp_update_font_hdr(struct font_s *font)
{
}
*/

void cp_export_unitab(struct font_s *font, FILE *f)
{
    struct cp_header *hdr = (struct cp_header *)font->file_hdr;
    short codepage = hdr->entry_hdr.codepage;
    int i, index = 0;

    for(i = 0; i < cp_files_count; i++)
    {
        if(codepage == cp_ids[i])
        {
            index = i; break;
        }
    }

    if(i == cp_files_count) index = 0;
  
    if(index >= cp_files_original_ones)
    {
        unsigned short j = atoi(cp_files[index]);
        for(i = 0; i < cp_files_count; i++)
            if(j == cp_ids[i])
            {
                fprintf(f, "#Mapped Codepage: %s\n", code_pages[i].string); break;
            }
        fprintf(f, "#Original file codepage: CP%d\n", codepage);
    }
    else
    {
        fprintf(f, "#Codepage: %s\n", code_pages[index].string);
        if(codepage != cp_ids[index])
            fprintf(f, "#Original file codepage: CP%d\n", codepage);
    }
  
    if(index == 0 || cp_files[index] == NULL)
    {
        for(i = 0; i < 256; i++)
        {
            fprintf(f, "0x%04x: ", i);
            fprintf(f, "U+%04x\n", code_pages[0].values[i]);
        }
    }
    else
    {
        int file_index = -1;
        for(i = 0; i < cp_files_original_ones; i++)
        {
            if(strcmp(code_pages[i].file_name, cp_files[index]) == 0)
            {
                file_index = i; break;
            }
        }

        if(file_index == -1)
        {
            fprintf(f, "Unable to read fontopia codepage file");
            status_error("Unable to read fontopia codepage file");
            return;
        }
    
        for(i = 0; i < 256; i++)
        {
            fprintf(f, "0x%04x: ", i);
            if(i < 128) fprintf(f, "U+%04x\n", code_pages[0].values[i]);
            else        fprintf(f, "U+%04x\n", code_pages[file_index].values[i-128]);
        }
    }

    status_msg("Unicode table saved to file with .tab extension");
}


int cp_create_unitab(struct font_s *font)
{
    if(!create_empty_unitab(font)) goto memory_error;
    struct cp_header *hdr = (struct cp_header *)font->file_hdr;
    short codepage = hdr->entry_hdr.codepage;
    int i, index = 0;

    for(i = 0; i < cp_files_count; i++)
    {
        if(codepage == cp_ids[i])
        {
            index = i; break;
        }
    }

    if(i == cp_files_count) index = 0;

    if(index == 0 || cp_files[index] == NULL)
    {
        for(i = 0; i < 256; i++)
        {
            font->unicode_table_index[i] = i;
            //font->unicode_table[i*2] = code_page_437[i];
            font->unicode_table[i*2] = code_pages[0].values[i];
        }
    }
    else
    {
        int file_index = -1;
        for(i = 0; i < cp_files_original_ones; i++)
        {
            if(strcmp(code_pages[i].file_name, cp_files[index]) == 0)
            {
                file_index = i; break;
            }
        }

        if(file_index == -1)
        {
            status_error("Unable to read fontopia codepage file");
            cp_kill_unitab(font);
            return 0;
        }
    
        for(i = 0; i < 256; i++)
        {
            font->unicode_table_index[i] = i;
            if(i < 128) font->unicode_table[i*2] = code_pages[0].values[i];
            else        font->unicode_table[i*2] = code_pages[file_index].values[i-128];
        }
    }

    return 1;
  
memory_error:

    status_error("Insufficient memory");
    return 0;
}

void cp_kill_unitab(struct font_s *font)
{
    free_unicode_table(font);
    font->unicode_info_size = 0;
    if(font->unicode_info) free(font->unicode_info);
    font->unicode_info = 0;
    font->has_unicode_table = 0;
}


void cp_handle_unicode_table_change(struct font_s *font)
{
    /***********************/
    /* remove unicode info */
    /***********************/
    if(!font->has_unicode_table)
    {
        cp_kill_unitab(font);
    }
    else
    {
        if(!cp_create_unitab(font))
        {
            font->has_unicode_table = 0;
            return;
        }

        font->unicode_info_size = (256 * 4);
        font->unicode_info = (unsigned char *)malloc(font->unicode_info_size);
        if(!font->unicode_info) return;

        int i, j = 0;
        unsigned short *data = (unsigned short *)font->unicode_info;

        for(i = 0; i < (int)font->length; i++)
        {
            unsigned int d = font->unicode_table[i*2];
            data[j  ] = (unsigned short)d;
            data[j+1] = 0xFFFF;
            j += 2;
        }

        font->utf_version = VER_PSF1;
        //status_error("CP fonts have no Unicode tables");
    }

    calc_max_zoom(font);
}


int show_cp_selection_dialog(int first_selection)
{
    int h = 12;
    int w = 32;
    int x = 1, y = 1;

    if(h > SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w > SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;
  
    if(first_selection < 0) first_selection = 0;
    if(first_selection >= cp_files_original_ones) first_selection = cp_files_original_ones-1;
  
    int vis_entries = h-1;
    int selected_entry = 0;
    int first_vis_entry = 0;

    if(first_selection >= vis_entries-1)
    {
        selected_entry = vis_entries-1;
        first_vis_entry = first_selection-selected_entry;
    }
  
draw_win:

    setScreenColors(WHITE, BGDEFAULT);
    drawBox(x, y, h+x, w+y," - Select codepage - ", 1);
  
    int i;
    for(i = 0; i < vis_entries; i++)
    {
        if(i == selected_entry) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+1+i, y+2);
        printw("%s", code_pages[first_vis_entry+i].string);
    }
  
    int ch;
    while(1)
    {
        ch = getKey();

        switch(ch)
        {
            case(UP_KEY):
            case(LEFT_KEY):
                if(selected_entry == 0)
                {
                    if(first_vis_entry == 0) break;
                    first_vis_entry--;
                }
                else selected_entry--;
                goto draw_win;

            case(DOWN_KEY):
            case(RIGHT_KEY):
                if(selected_entry == vis_entries-1)
                {
                    if(selected_entry+first_vis_entry >= cp_files_original_ones-1) break;
                    first_vis_entry++;
                }
                else selected_entry++;
                goto draw_win;

            case(ENTER_KEY):
            case(SPACE_KEY):
                return first_vis_entry+selected_entry;

            case(ESC_KEY):
                return -1;
        }
    }
}


void cp_change_codepage(struct font_s *font)
{
    if(font->version != VER_CP)
    {
        status_error("This is not a CP font");
        return;
    }

    struct cp_header *hdr = (struct cp_header *)font->file_hdr;
    int index = show_cp_selection_dialog(hdr->entry_hdr.codepage);
    /* user cancelled */
    if(index == -1) return;
    short codepage = cp_ids[index];

    hdr->entry_hdr.codepage = codepage;
    cp_kill_unitab(font);
    font->has_unicode_table = 1;
    cp_handle_unicode_table_change(font);
}


void cp_handle_version_change(struct font_s *font, 
                              char old_version __attribute__((unused)))
{
    struct cp_header *hdr = (struct cp_header *)malloc(sizeof(struct cp_header));
    if(!hdr)
    {
        status_error("Insufficient memory");
        return;
    }

    struct screen_font_header fhdr;
    hdr->entry_hdr.cpeh_size = sizeof(struct cp_entry_header)+sizeof(struct cp_info_header);
    hdr->entry_hdr.next_cpeh_offset = 0;
    hdr->entry_hdr.device_type = 1;    /* 1 screen, 2 printer */
    hdr->entry_hdr.device_name[0] = 'V';
    hdr->entry_hdr.device_name[1] = 'I';
    hdr->entry_hdr.device_name[2] = 'D';
    hdr->entry_hdr.device_name[3] = 'E';
    hdr->entry_hdr.device_name[4] = 'O';
    hdr->entry_hdr.device_name[5] = ' ';
    hdr->entry_hdr.device_name[6] = ' ';
    hdr->entry_hdr.device_name[7] = ' ';
    short codepage = cp_ids[show_cp_selection_dialog(0)];
    hdr->entry_hdr.codepage = codepage;
    hdr->entry_hdr.res[0] = 0;
    hdr->entry_hdr.res[1] = 0;
    hdr->entry_hdr.res[2] = 0;
    hdr->entry_hdr.res[3] = 0;
    hdr->entry_hdr.res[4] = 0;
    hdr->entry_hdr.res[5] = 0;
    hdr->entry_hdr.cpih_offset = 0;

    hdr->info_hdr.version = 0;
    hdr->info_hdr.num_fonts = 1;
    hdr->info_hdr.size = font->length*font->charsize;
  
    fhdr.height = font->height;
    fhdr.width = font->width;
    fhdr.res = 0;
    fhdr.num_chars = font->length;

    if(font->file_hdr) free(font->file_hdr);
    font->file_hdr = hdr;
    font->header_size = sizeof(struct cp_header);
    long sz = sizeof(struct screen_font_header)+font->data_size;

    unsigned char *data = (unsigned char *)malloc(sz);
    if(!data)
    {
        status_error("Insufficient memory");
        return;
    }

    memcpy((void *)(data), (void *)&fhdr, sizeof(struct screen_font_header));
    memcpy((void *)(data+sizeof(struct screen_font_header)), font->data, font->data_size);
    free(font->data);
    font->data = (data+sizeof(struct screen_font_header));
    if(font->raw_data) free(font->raw_data);
    font->raw_data = data;
    font->raw_data_size = sz;
    cp_kill_unitab(font);
    font->has_unicode_table = 1;
    cp_handle_unicode_table_change(font);
    font->cp_active_font = 0;
    font->cp_total_fonts = 1;
}


void cp_convert_to_psf(struct font_s *font)
{
    /* create unicode table from code page */
    int u = 0;
    if(!font->has_unicode_table) u = cp_create_unitab(font);
    long draft_bytes = font->length*sizeof(unsigned short)*2;
  
    if(font->version == VER_PSF1)
    {
        struct psf1_header hdr;
        hdr.magic[0] = PSF1_MAGIC0;
        hdr.magic[1] = PSF1_MAGIC1;
        hdr.mode = 0;
        if(font->has_unicode_table) hdr.mode |= PSF1_MODEHASTAB;
        hdr.charsize = font->charsize;
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
        if(font->has_unicode_table) hdr2.flags |= PSF2_HAS_UNICODE_TABLE;
        long sz = sizeof(struct psf2_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr2, sizeof(struct psf2_header));
        font->file_hdr = new_hdr;
        font->header_size = sz;
    }

    unsigned char *new_data = (unsigned char *)malloc(font->data_size);
    if(!new_data) { status_error("Insufficient memory"); return; }
    memcpy((void *)new_data, (void *)font->data, font->data_size);
    free(font->raw_data);
    font->raw_data = 0;
    font->raw_data_size = 0;
    font->data = new_data;
  
    if(!u)
    {
        force_font_dirty(font);
        return;
    }

    long draft_index = 0;
    unsigned short *unicode_table = (unsigned short *)malloc(draft_bytes);
    if(!unicode_table) { return; }

    int i;
    for(i = 0; i < 256; i++)
    {
        unicode_table[draft_index++] = font->unicode_table[i*2];
        unicode_table[draft_index++] = 0xFFFF;
    }

    font->utf_version = VER_PSF1;
    font->unicode_info_size = draft_bytes;
    font->has_unicode_table = 1;
    font->unicode_info = (unsigned char *)unicode_table;
  
    force_font_dirty(font);
}


long cp_make_utf16_unitab(struct font_s *new_font, unsigned short **_unicode_table)
{
    return psf_make_utf16_unitab(new_font, _unicode_table);
}

int cp_is_acceptable_width(struct font_s *font)
{
    return (font->width == 8);
}

int cp_next_acceptable_width(struct font_s *font __attribute__((unused)))
{
    return 8;
}

int cp_is_acceptable_height(struct font_s *font)
{
    return (font->height == 6 || font->height == 8 ||
            font->height == 14 || font->height == 16);
}

int cp_next_acceptable_height(struct font_s *font)
{
    if(font->height == 6) return 8;
    else if(font->height == 8) return 14;
    else if(font->height == 14) return 16;
    else return 6;
}

/********************************
 * ******************************
 * ******************************/
struct module_s cp_module;

void cp_init_module()
{
    strcpy(cp_module.mod_name, "cp");
    cp_module.max_width = 8;
    cp_module.max_height = 16;
    cp_module.max_length = 256;
    cp_module.create_empty_font = cp_create_empty_font;
    cp_module.write_to_file = cp_write_to_file;
    cp_module.load_font = cp_load_font;
    cp_module.load_font_file = cp_load_font_file;
    cp_module.handle_hw_change = cp_handle_hw_change;
    cp_module.update_font_hdr = NULL;
    cp_module.shrink_glyphs = NULL;
    cp_module.expand_glyphs = NULL;
    cp_module.handle_version_change = cp_handle_version_change;
    cp_module.handle_unicode_table_change = cp_handle_unicode_table_change;
    cp_module.export_unitab = cp_export_unitab;
    //cp_module.create_unitab = cp_create_unitab;
    //cp_module.kill_unitab = cp_kill_unitab;
    cp_module.convert_to_psf = cp_convert_to_psf;
    cp_module.make_utf16_unitab = cp_make_utf16_unitab;
    cp_module.is_acceptable_width = cp_is_acceptable_width;
    cp_module.next_acceptable_width = cp_next_acceptable_width;
    cp_module.is_acceptable_height = cp_is_acceptable_height;
    cp_module.next_acceptable_height = cp_next_acceptable_height;
    register_module(&cp_module);
    add_file_extension("cp", "cp");
}


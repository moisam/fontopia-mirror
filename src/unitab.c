/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: unitab.c
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
#include "defs.h"
#include "view.h"
#include "modules/cp.h"

int patch_font_unicode(struct font_s *font, unsigned short *new_unicode,
               int new_font_length, long new_unicode_bytes);



int remove_unitab(struct font_s *font)
{
    if(!font) return 0;
    if(!font->has_unicode_table) return 0;

    free_unicode_table(font);
    font->has_unicode_table = 0;
    font->unicode_table_index = 0;
    font->unicode_table = 0;
    free(font->unicode_info);
    font->unicode_info = 0;
    font->unicode_info_size = 0;
    if(font->module->update_font_hdr) font->module->update_font_hdr(font);
    force_font_dirty(font);
    calc_max_zoom(font);
    reset_all_cursors();
    return 1;
}

int _update_cache(struct font_s *font)
{
    /* update cache */
    if(!create_empty_unitab(font))
    {
        free_unicode_table(font);
        status_error("Insufficient memory");
        font->has_unicode_table = 0;
        return 0;
    }

    get_font_unicode_table(font);
    return 1;
}

int import_unitab_from_file(struct font_s *font, char *file_name)
{
    struct font_s *new_font = (struct font_s *)NULL;
    unsigned short *unicode_table = (unsigned short *)NULL;
    //int len = 0;
    long unicode_table_len = 0;

    if(file_name)
    {
        if(!(new_font = load_font_file(file_name))) return 0;
    }
  
    if(!new_font->has_unicode_table)
    {
        if(new_font->version != VER_CP)
        {
            status_error("Font has no unicode table");
            goto cancelled;
        }
    }
  
    if(new_font->module->make_utf16_unitab)
    {
        unicode_table_len = 
            new_font->module->make_utf16_unitab(new_font, &unicode_table);
        if(unicode_table_len == 0) goto memory_error;
    }
    else
    {
        status_error("Error reading Unicode table");
        goto cancelled;
    }

    unsigned int len = new_font->length;
    if(!patch_font_unicode(font, unicode_table, len, unicode_table_len))
    {
        status_error("Insufficient memory");
        goto cancelled;
    }

    /* update cache */
    if(!_update_cache(font)) goto cancelled;
    status_msg("Font unicode table updated");

    /* this was malloc'd by us, so free it */
    free(unicode_table);
    kill_font(new_font);
    return 1;
  
memory_error:

    status_error("Insufficient memory");
    goto cancelled;

cancelled:

    if(unicode_table) free(unicode_table);
    if(new_font) kill_font(new_font);
    calc_max_zoom(font);
    return 0;
}


int patch_font_unicode(struct font_s *font, unsigned short *new_unicode,
               int new_font_length, long new_unicode_bytes)
{
    void *old_unicode = font->unicode_info;
    long old_unicode_bytes = font->unicode_info_size;

    if(new_font_length == font->length)
    {
        if(new_unicode_bytes <= old_unicode_bytes)
        {
            //font->raw_data_size -= (old_unicode_bytes-new_unicode_bytes);
            /* make sure the pointer is valid. the font may have had
             * no unicode table before.
             */
            //font->unicode_info = font->data+font->data_size;
        }
        else
        {
            font->unicode_info = (void *)realloc(font->unicode_info, new_unicode_bytes);
            if(!font->unicode_info)
            {
                font->unicode_info = old_unicode;
                return 0;
            }
        }

        font->unicode_info_size = new_unicode_bytes;
        unsigned char *unicode = font->unicode_info;
        memcpy((void *)unicode, (void *)new_unicode, new_unicode_bytes);
        font->has_unicode_table = 1;
        font->utf_version = VER_PSF1;
        if(font->module->update_font_hdr) font->module->update_font_hdr(font);
        force_font_dirty(font);
        return 1;
    }
    //////////////////////////////////////////
    //////////////////////////////////////////
    else
    {
        long new_index = 0, old_index = 0;
        unsigned short *draft = (unsigned short *)NULL;
        long draft_index = 0;
        long draft_bytes = font->length*sizeof(unsigned short)*2;
        draft = (unsigned short *)malloc(draft_bytes);
        if(!draft) goto error;
        unsigned short u;
        int i = 0;
        int cnt = 0;

        /*
         * 1- Take unicode entries from the new table until font length is 
         *    reached (if font length is shorter than the new table, extra
         *    entries are discarded).
         * 2- If font length is longer than new table length, get remaining
         *    entries from the old font unicode table.
         */
        while(i < font->length)
        {
            /* this condition is true when the new unicode table
             * is shorther than the old one, e.g. patching a 256
             * entry unicode table on a 512 entry font.
             */
            if(i >= new_font_length) break;
            u = new_unicode[new_index++];
            draft[draft_index++] = u;
            if(u == PSF1_SEPARATOR)
            {
                i++; cnt = 0;
            }
            else if(u == PSF1_STARTSEQ)
            {
                /* we didn't take sequences into account when allocating
                 * memory to our draft array. we will need to realloc it.
                 */
                int bytes = 0;
                while(new_unicode[new_index+bytes] != PSF1_SEPARATOR) bytes++;
                /* add extra byte for the separator */
                bytes = (bytes+1) * sizeof(unsigned short);
                draft = (unsigned short *)realloc(draft, draft_bytes+bytes);
                if(!draft) goto error;
                draft_bytes += bytes;

                do
                {
                    u = new_unicode[new_index++];
                    draft[draft_index++] = u;
                } while(u != PSF1_SEPARATOR);

                draft[draft_index++] = u;
                i++; cnt = 0;
            }
            /* entries may have more than one unicode with them.
             * if so, calculate extra memory needed.
             */
            else
            {
                cnt++;

                if(cnt > 1)
                {
                    int bytes = 0;
                    while(new_unicode[new_index+bytes] != PSF1_SEPARATOR) bytes++;
                    /* count 2 more: one for the extra unicode, and one for 0xFFFF */
                    bytes += 2;

                    if(bytes)
                    {
                        //bytes++;
                        bytes *= sizeof(unsigned short);
                        draft = (unsigned short *)realloc(draft, draft_bytes+bytes);
                        if(!draft) goto error;
                        draft_bytes += bytes;

                        do
                        {
                            u = new_unicode[new_index++];
                            draft[draft_index++] = u;
                        } while(u != PSF1_SEPARATOR);

                        draft[draft_index++] = u;
                        i++; cnt = 0;
                    }
                }
            }
        } /* end while */

        /* get more entries from old table if needed */
        cnt = 0;
        if(i < font->length)
        {
            /* we will skip the entries we added from the new table */
            unsigned int c = 0;
            font->unicode_array_index = 0;
            font->unicode_index = 0;

            do
            {
                if(!font->has_unicode_table)
                {
                    draft[draft_index++] = 0;
                    draft[draft_index++] = 0xFFFF;
                    i++;
                    continue;
                }

                c = get_next_utf(font);

                if(c == PSF1_SEPARATOR)
                {
                    if(old_index < i) { old_index++; continue; }
                    draft[draft_index++] = c;
                    i++; old_index = i; cnt = 0;
                    continue;
                }
                else if(c == PSF1_STARTSEQ)
                {
                    /* we didn't take sequences into account when allocating
                     * memory to our draft array. we will need to realloc it.
                     */
                    int bytes = 0;
                    while(font->unicode_info[old_index+bytes] != PSF1_SEPARATOR) bytes++;
                    /* add extra byte for the separator */
                    bytes = (bytes+1) * sizeof(unsigned short);
                    draft = (unsigned short *)realloc(draft, draft_bytes+bytes);
                    if(!draft) goto error;
                    draft_bytes += bytes;

                    do
                    {
                        draft[draft_index++] = c;
                        c = get_next_utf(font);
                    } while(c != PSF1_SEPARATOR);

                    draft[draft_index++] = c;
                    i++; old_index = i; cnt = 0;
                    continue;
                }
                else
                {
                    if(old_index < i) { continue; }
                    draft[draft_index++] = c;
                    cnt++;

                    if(cnt > 1)
                    {
                        int bytes = 0;
                        int m = font->unicode_array_index;

                        if(font->utf_version == VER_PSF1)
                        {
                            while(font->unicode_info[m+bytes] != PSF2_SEPARATOR) bytes += 2;
                            bytes += 4;
                        }
                        else if(font->utf_version == VER_PSF2)
                        {
                            while(font->unicode_info[m+bytes] != PSF2_SEPARATOR) bytes++;
                            bytes += 2;
                            bytes *= sizeof(unsigned short);
                        }

                        if(bytes)
                        {
                            draft = (unsigned short *)realloc(draft, draft_bytes+bytes);
                            if(!draft) goto error;
                            draft_bytes += bytes;

                            do
                            {
                                c = get_next_utf(font);
                                draft[draft_index++] = c;
                            } while(c != PSF1_SEPARATOR);

                            draft[draft_index++] = c;
                            i++; old_index = i; cnt = 0;
                        }
                    }
                }
            } while(i < font->length);
        }

        //////////////////////////////////////
        /* Now save the new table into font */
        //////////////////////////////////////
        if(draft_bytes <= old_unicode_bytes)
        {
            //font->raw_data_size -= (old_unicode_bytes-draft_bytes);
            /* make sure the pointer is valid. the font may have had
             * no unicode table before.
             */
            //font->unicode_info = font->data+font->data_size;
        }
        else
        {
            font->unicode_info = (void *)realloc(font->unicode_info, draft_bytes);
            if(!font->unicode_info)
            {
                font->unicode_info = old_unicode;
                goto error;
            }
        }

        unsigned char *unicode = font->unicode_info;
        memcpy((void *)unicode, (void *)draft, draft_bytes);
        font->has_unicode_table = 1;
        font->utf_version = VER_PSF1;
        free(draft);

        /* update file header */
        if(font->module->update_font_hdr) font->module->update_font_hdr(font);
        force_font_dirty(font);
        return 1;
    
error:

        if(draft) free(draft);
        return 0;
    }

    return 0;
}

static inline unsigned char hex_to_decimal(char h)
{
    if(h >= '0' && h <= '9') return h-'0';
    if(h >= 'a' && h <= 'z') return h-'a'+10;
    if(h >= 'A' && h <= 'Z') return h-'A'+10;
    return 0;
}

int get_next_entry_in_line(char *line, unsigned int *res)
{
    char *orig_line = line;

    while(*line)
    {
        if(*line == ' ' || *line == '\n' || *line == '\t')
        {
            line++;
            continue;
        }

        /* assume file is corrupt */
        if(*line != 'U' && *line != 'u') return -1;
        if(line[1] != '+') return -1;
        *res = 0;
        line += 2;

        while(*line)
        {
            *res = ((*res) << 4) | (hex_to_decimal(*line));
            line++;
            if(*line == ' ' || *line == '\n' || *line == '\t') break;
        }

        while(*line == ' ' || *line == '\n' || *line == '\t') line++;
        //line++;
        break;
    }

    return line-orig_line;
}

void import_unitab(struct font_s *font)
{
    if(font->version == VER_CP)
    {
        status_error("Cannot import unicode table to CP font");
        return;
    }

    int res;
    char *unitab_file_name = (char *)NULL;
    char buf[1024];
    unsigned short *unicode_table = (unsigned short *)NULL;
    long unicode_table_len = 0;        /* length of this table */
    long unicode_table_index = 0;        /* # of bytes used in this table */

    res = show_opensave(".", OPEN, &unitab_file_name, 0);
    if(res == OPENSAVE_ERROR) goto error;
    if(res == OPENSAVE_CANCEL) goto cancelled;

    FILE *unitab_file = (FILE *)NULL;
    unitab_file = fopen(unitab_file_name, "r");
    if(!unitab_file) goto error;
  
    int i;
    int len = 0;
    /* we will count the entries first, then confirm from the user,
     * then read the file again if user chose to proceed. This flag
     * helps us know if we are in the first or the second run of reading.
     */
    char count_only = 1;

    if(!fgets(buf, 1024, unitab_file)) goto corrupt_file;
    if(strcmp(buf, "#fontopia unicode table file\n")) goto corrupt_file;
  
read:

    i = 0;

    while(fgets(buf, 1024, unitab_file))
    {
        if(buf[0] == '#')
        {
            /* ignore commented lines */
            continue;
        }

        /* Rudimentary error checking.
         * TODO: we need to do more strict error checking here.
         */
        if(!strchr(buf, ':')) goto corrupt_file;

        /* try to estimate # of entries in this line */
        if(count_only)
        {
            char *j = strchr(buf, ':');
            /* entries in file are 7 chars (" U+xxxx").
             * entries in font are 2 bytes each, plus 2 bytes for terminating 0xFFFF.
             * since we use (unsigned short *), this comes as follows:
             */
            int k = (strlen(j)/7) + 1;
            unicode_table_len += k;
        }
        /* extract information */
        else
        {
            char *j = strchr(buf, ':')+1;
            unsigned int l;
            int bytes = get_next_entry_in_line(j, &l);

            if(bytes == -1) goto corrupt_file;
            j += bytes;

            while(bytes)
            {
                unicode_table[unicode_table_index++] = (unsigned short)l;
                bytes = get_next_entry_in_line(j, &l);
                j += bytes;
            }
            unicode_table[unicode_table_index++] = 0xFFFF;
        }
        i++;
    }

    if(!count_only) goto process;
  
    /* now confirm from the user */
    sprintf(buf, "Found %d unicode entries in the file. Your font\n"
                 "contains %d glyph entries.", i, font->length);
    len = i;

    if(i == font->length)
    {
        if(font->has_unicode_table)
            strcat(buf, "\nNew entries will overwrite existing ones.");
        strcat(buf, "\nProceed?");
    }
    else if(i < font->length)
    {
        if(font->has_unicode_table)
            strcat(buf, "\nRemaining entries in your font will not be changed.");
        else
            strcat(buf, "\nEntries will be added up to table length."
                        "\nRest will be zeroed.");
        strcat(buf, "\nProceed?");
    }
    else
    {
        strcat(buf, "\nExtra entries from file will be ignored.\nProceed?");
    }

    if(msgBox(buf, BUTTON_YES|BUTTON_NO, CONFIRM) == BUTTON_YES)
    {
        /* go back to read the entries properly */
        rewind(unitab_file);
        count_only = 0;

        /* throw extra 12 unsigned shorts just in case!
         * NOTE: this is an arbitrary number only.
         */
        unicode_table_len += 12;
        unicode_table = (unsigned short *)malloc(unicode_table_len*sizeof(unsigned short));

        if(!unicode_table)
        {
            fclose(unitab_file);
            status_error("Insufficient memory");
            goto error;
        }
        goto read;
    }
    else
    {
        fclose(unitab_file);
        goto cancelled;
    }

process:

    fclose(unitab_file);

    if(!patch_font_unicode(font, unicode_table, len, 
                            unicode_table_index*sizeof(unsigned short)))
    {
        status_error("Insufficient memory");
        goto cancelled;
    }

    /* update cache */
    if(!_update_cache(font)) goto cancelled;
    status_msg("Font unicode table updated");
    goto cancelled;

error:

    status_error("Error opening unicode table file");
    goto cancelled;

corrupt_file:

    fclose(unitab_file);
    import_unitab_from_file(font, unitab_file_name);

cancelled:

    if(unitab_file_name) free(unitab_file_name);
    if(unicode_table) free(unicode_table);
    calc_max_zoom(font);
}


void export_unitab(struct font_s *font)
{
    if(!font->has_unicode_table && font->version != VER_CP)
    {
        status_error("Font has no unicode info");
        return;
    }

    char *fn = 0;

    if(!font_file_name)
    {
        fn = (char *)malloc(12);
        if(!fn) goto memory_error;
        strcpy(fn, "Unnamed.tab");
    }
    else
    {
        int len = strlen(font_file_name);
        fn = (char *)malloc(len+5);
        if(!fn) goto memory_error;
        strcpy(fn, font_file_name);
        strcat(fn, ".tab");
    }

    FILE *f = fopen(fn, "w");
    if(!f) goto file_error;

    fprintf(f, "#fontopia unicode table file\n");
    fprintf(f, "# you can edit this file in the following format\n");
    fprintf(f, "# position: unicode1 [unicode2 [...unicode%d]]\n#\n", MAX_UNICODE_TABLE_ENTRIES);
    fprintf(f, "# where:\n# - position: glyph position in hex (e.g. 0x001d)\n");
    fprintf(f, "# - the colon is used to separate position from coming unicode values\n");
    fprintf(f, "# - unicode values, upto %d. "
               "If this glyph is not having any unicode\n", 
               MAX_UNICODE_TABLE_ENTRIES);
    fprintf(f, "#   association, please put U+0000. Note that unicode values are UTF-16\n");
    fprintf(f, "#   only (work is in progress to support UTF-8. Be patient!).\n#\n");
    fprintf(f, "# ALL lines must end in a newline character '\\n', "
               "ALL including\n# the very last line!.\n");
    fprintf(f, "# Entries should be separated by one space only. For example:\n");
    fprintf(f, "# 0x001d: U+00a0 U+00bc U+a1c0\n#\n");
    fprintf(f, "# We know we are asking too much, but we are working on easing these\n");
    fprintf(f, "# restrictions in future versions. Bear with us!.\n#\n");
    fprintf(f, "# Please don't modify this header as it is the only way we can know\n");
    fprintf(f, "# for sure that this is a fontopia unicode table file!.\n");
    fprintf(f, "#\n");
  
    if(font->module->export_unitab)
    {
        font->module->export_unitab(font, f);
        fclose(f);
        free(fn);
        return;
    }
  
    int i;
    char t[16];
    char m[(MAX_UNICODE_TABLE_ENTRIES*7)+1];

    for(i = 0; i < font->length; i++)
    {
        m[0] = '\0';
        if(font->unicode_table_index[i] == 0xFFFF)
        {
            unsigned int *arr = 0;
            get_unitab_entry(font, i, &arr);

            int count = 0;
            while(count < MAX_UNICODE_TABLE_ENTRIES)
            {
                if(arr[count] == 0) break;
                sprintf(t, "U+%04x ", arr[count]);
                strcat(m, t);
                count++;
            }
        }
        else
        {
            sprintf(t, "U+%04x ", font->unicode_table[i*2]);
            strcat(m, t);
        }

        fprintf(f, "0x%04x: ", i);
        fprintf(f, "%s\n", m);
    }

    fclose(f);
    free(fn);
    status_msg("Unicode table saved to file with .tab extension");
    return;

file_error:  

    status_error("File I/O error!");
    return;

memory_error:

    if(fn) free(fn);
    status_error("Insufficient memory!");
}


int open_unicode_table(struct font_s *font)
{
    if(!font->has_unicode_table && font->version != VER_CP)
    {
        status_error("Font has no unicode info");
        return 1;
    }
  
    int h = 15;
    int w = 49;
    int x = 1, y = 1;

    if(h > SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w > SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;
  
    int res = 0;
    int i, j;
    int first_entry = 0;
    int selected_entry = 0;
    int vis_entries = h-2;
    char t[16];
    char m[(MAX_UNICODE_TABLE_ENTRIES*7)+10];
  
    if(font->version == VER_CP)
    {
        if(!cp_create_unitab(font)) return 1;
    }

draw_win:

    drawBox(x, y, h+x, w+y," - Font Unicode Info - ", 1);
    locate(x+h-1, y+1);
    setScreenColors(BLACK, BGWHITE);
    printw("ENTER: Edit | X: Export | S: Save | ESC: Cancel");

refresh:  

    for(i = 0; i < vis_entries; i++)
    {
        j = first_entry+i;
        m[0] = '\0';

        if(font->unicode_table_index[j] == 0xFFFF)
        {
            unsigned int *arr = 0;
            get_unitab_entry(font, j, &arr);

            int count = 0;
            while(count < MAX_UNICODE_TABLE_ENTRIES)
            {
                if(arr[count] == 0) break;
                sprintf(t, "U+%04x ", arr[count]);
                strcat(m, t);
                count++;
            }
        }
        else
        {
            sprintf(t, "U+%04x ", font->unicode_table[j*2]);
            strcat(m, t);
        }

        /* print that entry */
        if(i == selected_entry) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+1+i, y+1);
        printw("0x%04x: ", j);

        int len = strlen(m);
        if(len < w-10)
        {
            printw("%s", m);
            printw("%*s", w-10-len, " ");
        }
        else
        {
            int k;
            for(k = 0; k < w-12; k++) putchar(m[k]);
            printw("..");
        }
    }
    
    char ch;
    char end = 0;

    while(!end)
    {
        ch = getKey();

        switch(ch)
        {
            case('s'):
            case(ENTER_KEY):
                /*
                 * TODO: Implement this!.
                 */
                msgBox("Oops! This function is not implemented yet!\n"
                       "You can e[X]port the table to a file, edit it\n"
                       "manually, and reload it into fontopia. Sorry\n"
                       "for that pal!", BUTTON_OK, INFO);
                goto draw_win;

            case(HOME_KEY):
                selected_entry = 0;
                first_entry = 0;
                goto refresh;

            case(END_KEY):
                selected_entry = vis_entries-1;
                first_entry = font->length-selected_entry-1;
                goto refresh;

            case(LEFT_KEY):
            case(UP_KEY):
                if(selected_entry == 0)
                {
                    if(first_entry == 0) break;
                    first_entry--;
                }
                else selected_entry--;
                goto refresh;

            case(RIGHT_KEY):
            case(DOWN_KEY):
                if(selected_entry == vis_entries-1)
                {
                    if(selected_entry+first_entry == font->length-1) break;
                    first_entry++;
                }
                else selected_entry++;
                goto refresh;

            case(ESC_KEY):
                res = 1;
                end = 1;
                break;

            case('x'):
                export_unitab(font);
                res = 0;
                end = 1;
                break;
        }
    }
  
    /*
    if(font->version == VER_CP)
    {
        cp_kill_unitab(font);
    }
    */
  
    /* cancelled? */
    if(res == 1) return 1;
    return 0;
}


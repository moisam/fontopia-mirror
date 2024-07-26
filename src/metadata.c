/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: metadata.c
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

#include <string.h>
#include <ncurses.h>
#include "defs.h"
#include "metadata.h"

struct metadata_item_s metadata_table[metadata_table_len] =
{
  { "FONT",                1, 0,   0 },
  { "COPYRIGHT",           1, 0,   0 },
  { "FONT_VERSION",        1, 0,   0 },
  { "FONT_TYPE",           1, 0,   0 },
  { "FOUNDRY",             1, 0,   0 },
  { "FAMILY_NAME",         1, 0,   0 },
  { "WEIGHT_NAME",         1, 0,   0 },
  { "SLANT",               1, 0,   0 },
  { "SETWIDTH_NAME",       1, 0,   0 },
  { "ADD_STYLE_NAME",      1, 0,   0 },
  { "PIXEL_SIZE",          0, 16,  0 },
  { "POINT_SIZE",          0, 160, 0 },
  { "RESOLUTION_X",        0, 75,  0 },
  { "RESOLUTION_Y",        0, 75,  0 },
  { "SPACING",             1, 0,   0 },
  { "AVERAGE_WIDTH",       0, 80,  0 },
  { "CHARSET_REGISTRY",    1, 0,   0 },
  { "CHARSET_ENCODING",    1, 0,   0 },
  { "UNDERLINE_POSITION",  0, -2,  0 },
  { "UNDERLINE_THICKNESS", 0, 1,   0 },
  { "CAP_HEIGHT",          0, 10,  0 },
  { "X_HEIGHT",            0, 8,   0 },
  { "WEIGHT",              0, 10,  0 },
  { "RESOLUTION",          0, 100, 0 },
  { "QUAD_WIDTH",          0, 12,  0 },
  { "FONT_ASCENT",         0, 14,  0 },
  { "FONT_DESCENT",        0, 2,   0 },
  { "DEFAULT_CHAR",        0, 0,   0 },
  { "FONTBOUNDINGBOX X",   0, 16,  0 },
  { "FONTBOUNDINGBOX Y",   0, 16,  0 },
  { "FONTBOUNDINGBOX XOff",0, 0,   0 },
  { "FONTBOUNDINGBOX YOff",0, -2,  0 },
  //{ "", 1, 0, 0 },
};


int save_metadata_str(struct font_s *font, int index, char *value)
{
    struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;

    if(!metadata) return 0;
    if(metadata[index].is_str == 0) return 0;
    if(index < 0 || index >= metadata_table_len) return 0;
  
    /* discard old value if any */
    if(metadata[index].value2) free(metadata[index].value2);
    metadata[index].value2 = 0;
  
    /* make room for new value */
    int len = strlen(value);
    if(!len) return 0;
    metadata[index].value2 = (char *)malloc(len+1);
    //char *m = metadata[index].value2;
    if(!metadata[index].value2) return 0;
    strcpy(metadata[index].value2, value);
    return 1;
}


void show_metadata(struct font_s *font)
{
    if(!font->has_metadata) return;
    if(!font->metadata) return;
  
    int h = 15;
    int w = 49;
    int x = 1, y = 1;

    if(h > SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w > SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;
  
    int i, j;
    int first_entry = 0;
    int selected_entry = 0;
    int vis_entries = h-2;
    struct metadata_item_s *meta = (struct metadata_item_s *)font->metadata;
    char t[10];
    char m[80];

draw_win:

    drawBox(x, y, h+x, w+y," - Font Metadata/Properties - ", 1);
    setScreenColors(BLACK, BGWHITE);
    locate(x+h-1, y+1);
    printw("%*s", w-2, " ");
    locate(x+h-1, y+1);
    printw("ENTER: Edit | ESC: Close");

refresh:  

    for(i = 0; i < vis_entries; i++)
    {
        j = first_entry+i;
        m[0] = '\0';
        strcpy(m, meta[j].name);
        strcat(m, ": ");

        if(meta[j].is_str)
        {
            if(meta[j].value2)
            {
                int lenm = strlen(m);
                int lenv = strlen(meta[j].value2);

                if(lenm+lenv > w-2)
                {
                    strncat(m, meta[j].value2, w-lenm-4);
                    strcat(m, "..");
                }
                else
                {
                    strcat(m, meta[j].value2);
                }
            }
        }
        else
        {
            sprintf(t, "%d", meta[j].value);
            strcat(m, t);
        }

        /* print that entry */
        if(i == selected_entry) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+1+i, y+1);
        printw("%*s", w-2, " ");
        locate(x+1+i, y+1);
        printw("%s", m);
    }

    hideCursor();
    refresh();
    
    char ch;
    char end = 0;

    while(!end)
    {
        ch = getKey();

        switch(ch)
        {
            case('s'):
            case('S'):
            case(ENTER_KEY):
                i = first_entry+selected_entry;
                if(i == METADATA_FONTBOUNDINGBOX_X || 
                   i == METADATA_FONTBOUNDINGBOX_Y) break;

                char *r = 0;

                if(meta[i].is_str)
                {
                    if(meta[i].value2) 
                         r = inputBoxI("Enter new value:", meta[i].value2, meta[i].name);
                    else r = inputBox("Enter new value:", meta[i].name);
                }
                else
                {
                    sprintf(t, "%d", meta[i].value);
                    r = inputBoxI("Enter new value:", t, meta[i].name);
                }

                /* check result */
                if(r)
                {
                    if(meta[i].is_str) save_metadata_str(font, i, r);
                    else meta[i].value = atoi(r);
                }
                goto draw_win;

            case(HOME_KEY):
                selected_entry = 0;
                first_entry = 0;
                goto refresh;

            case(END_KEY):
                selected_entry = vis_entries-1;
                first_entry = metadata_table_len-selected_entry-1;
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
                    if(selected_entry+first_entry == metadata_table_len-1) break;
                    first_entry++;
                }
                else selected_entry++;
                goto refresh;

            case(ESC_KEY):
                //res = 1;
                end = 1;
                break;
        }
    }
}


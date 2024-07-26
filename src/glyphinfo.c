/* 
 *    Copyright 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: glyphinfo.c
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
#include "glyph.h"
#include "menu.h"

/***********************************
 * Show Glyph information
 ***********************************/

char *info_str[] =
{
    // for BDF fonts
    "Encoding", "SWidthX", "SWidthY",
    "DWidthX", "DWidthY",
    "BBw", "BBh", "BBXoff", "BBYoff",
    // for PCF fonts
    "Right bearing", "Left bearing",
    "Ascent", "Descent",
};
int info_str_count = sizeof(info_str)/sizeof(char *);


int glyph_index(struct font_s *font)
{
    int r = right_window.cursor.row+right_window.first_vis_row;
    return (r*right_window.cols_per_row + right_window.cursor.col);
}

int get_val(struct char_info_s *char_info, int index)
{
    switch(index)
    {
        case 0: return char_info->encoding;
        case 1: return char_info->swidthX;
        case 2: return char_info->swidthY;
        case 3: return char_info->dwidthX;
        case 4: return char_info->dwidthY;
        case 5: return char_info->BBw;
        case 6: return char_info->BBh;
        case 7: return char_info->BBXoff;
        case 8: return char_info->BBYoff;
        case 9: return char_info->rBearing;
        case 10: return char_info->lBearing;
        case 11: return char_info->charAscent;
        case 12: return char_info->charDescent;
        default: return -1;
    }
}


void show_glyph_info(struct font_s *font)
{
    if(!font->char_info)
    {
        msgBox("This font does not contain glyph info data.", BUTTON_OK, ERROR);
        return;
    }

    int index = glyph_index(font);
  
    int h = info_str_count;
    int w = 49;
    int x = 1, y = 1;
    if(h > SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w > SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;
  
    int i, j;
    int first_entry    = 0;
    int selected_entry = 0;
    int vis_entries    = h-2;
    struct char_info_s *char_info = (struct char_info_s *)font->char_info;
    char t[10];
    char m[80];
    char title[24];
    sprintf(title, " - Glyph Info (#%d) - ", index);

draw_win:

    drawBox(x, y, h+x, w+y, title, 1);
    setScreenColors(BLACK, BGWHITE);
    locate(x+h-1, y+1);
    printw("%*s", w-2, " ");
    locate(x+h-1, y+1);
    printw("ENTER: Edit | D: Default value | ESC: Close");

refresh:  

    for(i = 0; i < vis_entries; i++)
    {
        j = first_entry+i;
        sprintf(m, "%s: %d", info_str[j], get_val(&char_info[index], j));
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
            case('d'):
            case('D'):      // put the default value
                i = first_entry+selected_entry;
                switch(i)
                {
                    case 0: char_info->encoding     = -1; break;
                    case 1: char_info->swidthX      = 1000; break;
                    case 2: char_info->swidthY      = 0; break;
                    case 3: char_info->dwidthX      = font->width; break;
                    case 4: char_info->dwidthY      = 0; break;
                    case 5: char_info->BBw          = char_info[i].dwidthX; break;
                    case 6: char_info->BBh          = char_info[i].charAscent +
                                                      char_info[i].charDescent;
                            break;
                    case 7: char_info->BBXoff       = 0; break;
                    case 8: char_info->BBYoff       = 0; break;
                    case 9: char_info->rBearing     = char_info[i].dwidthX; break;
                    case 10: char_info->lBearing    = 0; break;
                    case 11: char_info->charAscent  = font->height; break;
                    case 12: char_info->charDescent = 0; break;
                }
                goto draw_win;
                break;

            case(ENTER_KEY):
                i = first_entry+selected_entry;
                char *r = 0;
                sprintf(t, "%d", get_val(&char_info[index], i));
                r = inputBoxI("Enter new value:", t, info_str[i]);
                /* check result */
                if(r)
                {
                    j = atoi(r);
                    switch(i)
                    {
                        case 0: char_info->encoding     = j; break;
                        case 1: char_info->swidthX      = j; break;
                        case 2: char_info->swidthY      = j; break;
                        case 3: char_info->dwidthX      = j; break;
                        case 4: char_info->dwidthY      = j; break;
                        case 5: char_info->BBw          = j; break;
                        case 6: char_info->BBh          = j; break;
                        case 7: char_info->BBXoff       = j; break;
                        case 8: char_info->BBYoff       = j; break;
                        case 9: char_info->rBearing     = j; break;
                        case 10: char_info->lBearing     = j; break;
                        case 11: char_info->charAscent   = j; break;
                        case 12: char_info->charDescent = j; break;
                    }
                }
                goto draw_win;
                break;

            case(HOME_KEY):
                selected_entry = 0;
                first_entry = 0;
                goto refresh;
                break;

            case(END_KEY):
                selected_entry = vis_entries-1;
                first_entry = info_str_count-selected_entry-1;
                goto refresh;
                break;

            case(LEFT_KEY):
            case(UP_KEY):
                if(selected_entry == 0)
                {
                    if(first_entry == 0) break;
                    first_entry--;
                }
                else selected_entry--;
                goto refresh;
                break;

            case(RIGHT_KEY):
            case(DOWN_KEY):
                if(selected_entry == vis_entries-1)
                {
                    if(selected_entry+first_entry == info_str_count-1) break;
                    first_entry++;
                }
                else selected_entry++;
                goto refresh;
                break;

            case(ESC_KEY):
                end = 1;
                break;
        }
    }
}


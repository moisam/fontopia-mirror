/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: menu_generic.c
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
#include <stdlib.h>
#include <ncurses.h>
#include "menu.h"

/*
 * Shows a menu for the user to select from.
 * The menu items to show are passed as an array of strings in menu_items.
 * Their count is passed in item_count.
 * If you want a title to be shown, pass it in title, otherwise the menu
 * will be shown as a box without any title (useful for menu bars & drop-down menus).
 * Where to position the menu on screen (row, col) are 1-based integers, if you want
 * the menu centered on screen, pass 0 for both.
 * Similarly, to make the menu as big as needed (in terms of height & width), pass
 * zeroes in both h and w. Otherwise we will respect the size you passed, even if it
 * means that menu items will be chopped!.
 */
int show_menu(char **menu_items, unsigned int item_count, char *title,
              unsigned int row, unsigned int col, unsigned int h, unsigned int w)
{
    if(!menu_items || !item_count) return MENU_ARG_ERROR;
    int longest_len = 0;
    int title_len = 0;
    unsigned int i, j;

    for(i = 0; i < item_count; i++)
    {
        int l = strlen(menu_items[i]);
        if(l > longest_len) longest_len = l;
    }

    if(!longest_len) return MENU_ARG_ERROR;
    if(title) title_len = strlen(title);
    if(title_len > longest_len) longest_len = title_len;
  
    /* try to guess a proper height */
    if(h == 0)
    {
        if(item_count < (unsigned)(SCREEN_H-1))
        {
            h = item_count+1;
        }
        else
        {
            h = SCREEN_H-1;
            row = 1;
        }
    }
    else
    {
        if(h > (unsigned)(SCREEN_H-1))
        {
            h = SCREEN_H-1;
            row = 1;
        }
    }

    /* try to guess a proper width */
    if(w == 0)
    {
        if(longest_len < SCREEN_W-2)
        {
            w = longest_len+2;
        }
        else
        {
            w = SCREEN_W-1;
            col = 1;
        }
    }
    else
    {
        if(w > (unsigned)(SCREEN_W-1))
        {
            w = SCREEN_W-1;
            col = 1;
        }
    }

    /* try to guess a proper row */
    if(row == 0)
    {
        row = (SCREEN_H-h)/2;
        if(row == 0) row = 1;
    }

    /* try to guess a proper col */
    if(col == 0)
    {
        col = (SCREEN_W-w)/2;
        if(col == 0) col = 1;
    }
  
    unsigned int selected_entry = 0;
    unsigned int first_vis_entry = 0;
    unsigned int vis_entries = h-1;
    unsigned int x = row+1, y = col+1;
  
    drawBox(row, col, h+row, w+col, title, 1);
  
draw_win:

    for(i = 0; i < vis_entries; i++)
    {
        j = first_vis_entry+i;
        if(j == item_count) break;

        if(i == selected_entry)
            setScreenColors(FG_COLOR[COLOR_HIGHLIGHT_TEXT], BG_COLOR[COLOR_HIGHLIGHT_TEXT]);
        else
            setScreenColors(FG_COLOR[COLOR_WINDOW], BG_COLOR[COLOR_WINDOW]);

        locate(x+i, y);
        printw("%*s", w-2, " ");
        locate(x+i, y);
        size_t len = strlen(menu_items[j]);

        if(len > w-2)
        {
            char *t = strndup(menu_items[j], w-4);
            if(!t) return MENU_MEM_ERROR;
            printw("%s..", t);
            free(t);
        }
        else
        {
            printw("%s", menu_items[j]);
        }
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
                    if(first_vis_entry == 0)
                    {
                        /* wrap over */
                        selected_entry = vis_entries-1;
                        if(item_count <= vis_entries) first_vis_entry = 0;
                        else first_vis_entry = item_count-selected_entry;
                        goto draw_win;
                    }
                    first_vis_entry--;
                }
                else selected_entry--;
                goto draw_win;

            case(DOWN_KEY):
            case(RIGHT_KEY):
                if(selected_entry+first_vis_entry >= item_count-1)
                {
                    /* wrap over */
                    selected_entry = 0;
                    first_vis_entry = 0;
                    goto draw_win;
                }

                if(selected_entry == vis_entries-1)
                {
                    first_vis_entry++;
                }
                else selected_entry++;
                goto draw_win;

            case(ENTER_KEY):
            case(SPACE_KEY):
                return first_vis_entry+selected_entry;

            case(ESC_KEY):
                return MENU_CANCELLED;
        }
    }

    return MENU_CANCELLED;
}


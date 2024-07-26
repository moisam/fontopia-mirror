/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: status.c
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

void _do_status(char *msg, int STATUS)
{
    int row = SCREEN_H-1;
    int col = 2;
    int w = SCREEN_W-2;
    int i;
    int len;

    if(!msg) len = 0;
    else len = strlen(msg);

    switch(STATUS)
    {
        case STATUS_ERROR:
            setScreenColors(WHITE, BGRED); break;

        case STATUS_REGULAR:
        default:
            setScreenColors(BLACK, BGWHITE); break;
    }
  
    locate(row, col);

    if(len > w)
    {
        for(i = 0; i < w; i++) putchar(msg[i]);
    }
    else
    {
        if(msg) printw("%s", msg);
        if(w-len) printw("%*s", w-len, " ");
    }

    hideCursor();
    refresh();
}

void status_error(char *msg)
{
    _do_status(msg, STATUS_ERROR);
}

void status_msg(char *msg)
{
    _do_status(msg, STATUS_REGULAR);
}

void status_clear()
{
    _do_status(" ", STATUS_REGULAR);
}


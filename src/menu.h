/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: menu.h
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

#ifndef MENU_GENERIC_H
#define MENU_GENERIC_H

#define MENU_ARG_ERROR	(-1)
#define MENU_MEM_ERROR	(-2)
#define MENU_CANCELLED	(-3)

#include <console/dialogs.h>

int show_menu(char **menu_items, unsigned int item_count, char *title,
	      unsigned int row, unsigned int col, unsigned int h, unsigned int w);

#endif

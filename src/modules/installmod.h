/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: installmod.h
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


/*
 * This file is NOT to be included by any file other than modules.c
 * As a safety measure, I didn't include header checking to make
 * sure compilation will fail if the file is included more than once.
 */

/* if you are adding a new module, follow these steps: */

/* 1- Add the header file of your module to the end of this list. */
#include "psf.h"
#include "cp.h"
#include "raw.h"
#include "bdf.h"
#include "pcf.h"

/* 2- bump this macro by one. */
#define INSTALLABLE_MODULES	5

/* 3- Add the name of your initializer function to the END of this list.
 *    The function should look like xxx_init_module().
 * 
 * WARNING: DO NOT - repeat - DO NOT! remove the initializer
 *          of psf module, as this module MUST be the first
 *          module on the system. Play with the others as you wish.
 */
static void *init_module_func[INSTALLABLE_MODULES] =
{ 
  psf_init_module,
  cp_init_module,
  raw_init_module,
  bdf_init_module,
  pcf_init_module,
};

/* 4- Add a string to identify your module here.
 *    This can be simply the file extension.
 */
static char *module_version_str[] =
{
  "PSF1", "PSF2",
  "CP",
  "RAW",
  "BDF",
  "PCF",
};

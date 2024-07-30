# 
#    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
# 
#    file: Makefile.am
#    This file is part of fontopia.
#
#    fontopia is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    fontopia is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with fontopia.  If not, see <http://www.gnu.org/licenses/>.
#    

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = info man

EXTRA_DIST = ChangeLog AUTHORS COPYING README READMEkeys THANKS

docdir = $(datarootdir)/doc/@PACKAGE@
doc_DATA = COPYING AUTHORS README NEWS ChangeLog READMEkeys THANKS

bin_PROGRAMS = fontopia
fontopia_SOURCES = src/main.c src/glyph.c src/metrics.c src/unitab.c src/metadata.c src/menu_generic.c
fontopia_SOURCES += src/font_ops.c src/status.c src/view.c src/args.c src/keys.c src/modules/installmod.h
fontopia_SOURCES += src/opensave.c src/defs.h src/font_ops.h src/view.h src/glyph.h src/metadata.h src/menu.h
fontopia_SOURCES += src/glyphext.c src/glyphinfo.c
fontopia_SOURCES += src/modules/cp.c src/modules/raw.c src/modules/modules.c 
fontopia_SOURCES += src/modules/psf.c src/modules/bdf.c
fontopia_SOURCES += src/modules/cp.h src/modules/raw.h src/modules/modules.h 
fontopia_SOURCES += src/modules/psf.h src/modules/bdf.h
fontopia_SOURCES += src/modules/cp_include.h src/readme.c
fontopia_SOURCES += src/modules/bdf_hash.c src/modules/bdf_helper.c
fontopia_SOURCES += src/modules/pcf.h src/modules/pcf.c
fontopia_LDADD = -lgnudos
fontopia_CFLAGS = -Wall -Wextra -pedantic

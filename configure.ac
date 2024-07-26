# 
#    Copyright 2015, 2016, 2017, 2018, 2024 (c)
#    Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
# 
#    file: configure.ac
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

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([fontopia], [2.0], [mohammed_isam1984@yahoo.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
AM_PROG_AR
LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdio.h stdlib.h getopt.h string.h asm/types.h limits.h unistd.h])

# Checks for GnuDOS header files
AC_DEFINE([gnudos_header], [], [this var will indicate if a GnuDOS header is missing])
gnudos_header=""
AC_CHECK_HEADER([console/dialogs.h], [], [gnudos_header=" dialogs.h"])
AC_CHECK_HEADER([console/kbd.h], [], [gnudos_header="$gnudos_header kbd.h"])
AC_CHECK_HEADER([console/screen.h], [], [gnudos_header="$gnudos_header screen.h"])

if test "$gnudos_header" != ""; then
	AC_MSG_ERROR([Couldn't find or include the following header file(s):$gnudos_header

            *** Fontopia needs the following header files in order to compile:
            ***     dialogs.h, kbd.h and screen.h
            ***
            *** Those files are part of the GnuDOS package. If you don't have
            *** GnuDOS installed on your system, you can download the sources
            *** from https://ftp.gnu.org/gnu/gnudos/, compile and install them,
            *** then retry compiling Fontopia.
            ***
            *** Alternatively, if you are using an RPM-based distro, you can download and
            *** install the RPM from places like: https://pkgs.org/download/gnudos.
            *** Additionally, Fedora, CentOS and RHEL users can use yum/dnf to install
            *** GnuDOS from the official repos.
])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([malloc getcwd memset strcasecmp strchr strndup strrchr])

AC_C_INLINE

AC_CONFIG_FILES([Makefile
		info/Makefile
		man/Makefile])

AC_OUTPUT

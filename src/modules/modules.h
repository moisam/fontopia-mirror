/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: modules.h
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

#ifndef MODULES_H
#define MODULES_H

#include "../font_ops.h"

#define MAX_MODULES                 20
#define MAX_FILE_EXTENSIONS         30
#define MAX_FILE_SIGNATURES         30
#define MAX_MODULE_NAME_LEN         10
#define MAX_FILE_EXTENSION_LEN      5

struct module_s
{
    char mod_name[MAX_MODULE_NAME_LEN+1];
    char max_width;                 /* max font width */
    char max_height;                /* max font height */
    unsigned int max_length;        /* max font length */
    struct font_s *(*create_empty_font)();
    int (*write_to_file)(FILE *file, struct font_s *font);
    struct font_s *(*load_font_file)(char *file_name);
    struct font_s *(*load_font)(char *file_name, unsigned char *file_data, long file_size);
    void (*handle_hw_change)(struct font_s *font, char *newdata, long new_datasize);
    void (*shrink_glyphs)(struct font_s *font, int old_length);
    void (*expand_glyphs)(struct font_s *font, int old_length, int option);
    void (*handle_version_change)(struct font_s *font, char old_version);
    void (*handle_unicode_table_change)(struct font_s *font);
    void (*export_unitab)(struct font_s *font, FILE *f);
    //int (*create_unitab)(struct font_s *font);
    //void (*kill_unitab)(struct font_s *font);
    void (*update_font_hdr)(struct font_s *font);
    void (*convert_to_psf)(struct font_s *font);
    long (*make_utf16_unitab)(struct font_s *new_font, unsigned short **unicode_table);
    int (*is_acceptable_width)(struct font_s *font);
    int (*next_acceptable_width)(struct font_s *font);
    int (*is_acceptable_height)(struct font_s *font);
    int (*next_acceptable_height)(struct font_s *font);
    /* links */
    struct module_s *prev, *next;
};

struct file_sig_s
{
    int sig_first_byte;
    int sig_length;
    unsigned char sig[MAX_MODULE_NAME_LEN+1];
    struct module_s *module;
    struct file_sig_s *prev, *next;
};

struct file_ext_s
{
    char file_ext[MAX_FILE_EXTENSION_LEN+1];
    struct module_s *module;
};

void init_modules();
int register_module(struct module_s *new_module);
int add_file_extension(char *ext, char *module_name);
int add_file_signature(struct file_sig_s *sig, char *module_name);

struct module_s *check_file_ext(char *file_name);
struct module_s *check_file_signature(unsigned char *file_data);
struct module_s *get_module_by_name(char *module_name);
int get_registered_modules();
int get_version(char *ver_str);
char *get_version_str(int i);

#endif

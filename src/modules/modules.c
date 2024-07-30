/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: modules.c
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
#include "modules.h"
#include "installmod.h"

/* keep track of registered modules */
struct module_s first_module;
static int registered_modules = 0;

/* keep track of known file extensions */
struct file_ext_s file_extensions[MAX_FILE_EXTENSIONS];
static int registered_extensions = 0;

/* keep track of registered file signatures */
struct file_sig_s first_sig;
static int registered_sigs = 0;

int get_registered_modules() { return registered_modules; }

int get_version(char *ver_str)
{
    /* we add one because PSF has two versions but one module */
    int max_ver = get_registered_modules()+1;
    int i;

    for(i = 0; i < max_ver; i++)
    {
        if(strcasecmp(ver_str, module_version_str[i]) == 0) return i+1;
    }

    return 0;
}

char *get_version_str(int i)
{
    return module_version_str[i-1];
}

void init_modules()
{
    memset((void *)&first_module, 0, sizeof(struct module_s));
    memset((void *)&first_sig, 0, sizeof(struct file_sig_s));
    registered_modules = 0;
    registered_extensions = 0;

    /* first module is always the PSF module */
    psf_init_module();
    registered_modules = 1;
    registered_sigs = 2;

    /* install other modules starting from second one */
    int i;
    for(i = 1; i < INSTALLABLE_MODULES; i++)
    {
        void (*f)() = init_module_func[i];
        f();
    }
}

int register_module(struct module_s *new_module)
{
    if(!new_module) return 0;
    if(strlen(new_module->mod_name) > MAX_MODULE_NAME_LEN) return 0;
    if(registered_modules >= MAX_MODULES) return 0;
  
    struct module_s *m = &first_module;
    struct module_s *lastm = &first_module;

    /* check for module duplication */
    do
    {
        if(strcmp(new_module->mod_name, m->mod_name) == 0) return 0;
        lastm = m;
        m = m->next;
    } while(m);

    m = lastm;
    m->next = new_module;
    new_module->prev = m;
    new_module->next = (struct module_s *)NULL;
    registered_modules++;
    return 1;
}

int add_file_extension(char *ext, char *module_name)
{
    if(!ext || !module_name) return 0;
    if(strlen(ext) > MAX_FILE_EXTENSION_LEN) return 0;
    if(registered_extensions >= MAX_FILE_EXTENSIONS) return 0;
    struct module_s *m = &first_module;

    do
    {
        if(strcmp(module_name, m->mod_name) == 0) break;
        m = m->next;
    } while(m);

    /* module not found */
    if(!m) return 0;
    strcpy(file_extensions[registered_extensions].file_ext, ext);
    file_extensions[registered_extensions].module = m;
    registered_extensions++;
    return 1;
}

int add_file_signature(struct file_sig_s *sig, char *module_name)
{
    if(!sig || !module_name) return 0;
    if(registered_sigs >= MAX_FILE_SIGNATURES) return 0;
  
    struct module_s *m = &first_module;

    do
    {
        if(strcmp(module_name, m->mod_name) == 0) break;
        m = m->next;
    } while(m);

    /* module not found */
    if(!m) return 0;

    struct file_sig_s *f = &first_sig;
    while(f->next) f = f->next;
    f->next = sig;
    sig->prev = f;
    sig->next = (struct file_sig_s *)NULL;
    sig->module = m;
    registered_sigs++;
    return 1;
}

/*
 * checks if the given file has a registered extension.
 * will return a pointer to the module, or a NULL ptr
 * if the extension is not registered with us.
 */
struct module_s *check_file_ext(char *file_name)
{
    if(!registered_extensions) return (struct module_s *)0;
    char *trail = strrchr(file_name, '.');
    if(!trail) return (struct module_s *)0;
    trail++;
    int i;
    size_t traillen = strlen(trail);

    for(i = 0; i < registered_extensions; i++)
    {
        if(strlen(file_extensions[i].file_ext) != traillen) continue;
        if(strcasecmp(trail, file_extensions[i].file_ext) == 0)
            return file_extensions[i].module;
    }

    return (struct module_s *)0;
}

/*
 * checks if the given file has a registered signature.
 * will return a pointer to the module, or a NULL ptr
 * if the signature is not identified.
 */
struct module_s *check_file_signature(unsigned char *file_data)
{
    if(!registered_sigs) return (struct module_s *)0;
    struct file_sig_s *s = &first_sig;
    int i, j;
    char match = 1;

check:

    for(i = s->sig_first_byte, j = 0; i < s->sig_first_byte+s->sig_length; i++, j++)
    {
        if(file_data[i] != s->sig[j])
        {
            match = 0; break;
        }
    }

    /* match found? */
    if(!match)
    {
        s = s->next;
        if(!s) return (struct module_s *)0;
        match = 1;
        goto check;
    }

    return s->module;
}

struct module_s *get_module_by_name(char *module_name)
{
    if(!module_name) return (struct module_s *)0;
    if(strlen(module_name) > MAX_MODULE_NAME_LEN) return (struct module_s *)0;
    struct module_s *m = &first_module;

    do
    {
        if(strcasecmp(module_name, m->mod_name) == 0) break;
        m = m->next;
    } while(m);

    /* module not found */
    if(!m) return (struct module_s *)0;
    return m;
}


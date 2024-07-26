/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: metrics.c
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
#include "modules/psf.h"
#include "modules/cp.h"
#include "modules/raw.h"

void handle_hw_change(struct font_s *font, int old_height, int old_width, int old_length)
{
    //status_msg("Applying changes to glyph size...");
    
    char hskip = old_height/font->height;
    char rhskip = font->height/old_height;
    char wskip = old_width/font->width;
    if(wskip && old_width%font->width) wskip++;
    char rwskip = font->width/old_width;
    if(rwskip && font->width%old_width) rwskip++;
    int new_width = (font->width)/8;
    if(font->width%8) new_width++;
    int new_bytes = new_width;
    int old_bytes = (old_width)/8;
    if(old_width%8) old_bytes++;
    int new_charsize = font->height * new_width;
    int old_charsize = old_height * old_bytes;

    /* we use old_length here because the user may have chosen another length,
     * we will not apply this until later when we call handle_length_change().
     */
    long new_datasize = old_length * new_charsize;

    /* if the data size did not change, the width might have still changed,
     * and some modules (psf, bdf, cp) need to know this so they can update their 
     * internal data structures to reflect the change.
     */
    if(new_datasize == font->data_size)
    {
        font->module->handle_hw_change(font, NULL, 0);
        status_msg("Font metrics updated successfully");
        force_font_dirty(font);
        return;
    }
    
    char *newdata = (char *)malloc(new_datasize);
    if(!newdata) return;
    memset((void *)newdata, 0, new_datasize);

    status_msg("Applying changes to glyph size...");
    
    int i = 0;
    int i2 = 0;
    unsigned int base_o1 = 1 << (old_width-1);
    if(old_width < 8) base_o1 <<= (8-old_width);
    else if(old_width > 8 && old_width < 16) base_o1 <<= (16-old_width);
    else if(old_width > 16 && old_width < 32) base_o1 <<= (32-old_width);
    unsigned int base_o2 = 1 << (font->width-1);
    if(font->width < 8) base_o2 <<= (8-font->width);
    else if(font->width > 8 && font->width < 16) base_o2 <<= (16-font->width);
    else if(font->width > 16 && font->width < 32) base_o2 <<= (32-font->width);

    while(i < font->data_size)
    {
        int j = 0;
        int j2 = 0;

        while(j < old_charsize)
        {
            unsigned int line = (unsigned int)(font->data[i+j]);
            if(old_width > 8) line |= (unsigned int)(font->data[i+j+1]) << 8;
            if(old_width > 16) line |= (unsigned int)(font->data[i+j+2]) << 16;
            if(old_width > 24) line |= (unsigned int)(font->data[i+j+3]) << 24;
            unsigned int line2 = 0;
            unsigned int o1 = base_o1;
            unsigned int o2 = base_o2;
            int k = 0;

            while(k < font->width)
            {
                if(wskip)    /* shrinking */
                {
                    if(line & o1) line2 |= o2;
                    o1 >>= wskip;
                    o2 >>= 1;
                    k++;
                }
                else        /* expanding */
                {
                    char rw = rwskip;
                    while(rw--)
                    {
                        if(line & o1) line2 |= o2;
                        o2 >>= 1;
                    }

                    o1 >>= 1;
                    k += rwskip;

                    if(o1 == 0)
                        while(k++ < font->width) ;
                }
            }

            newdata[i2+j2] = (char)(line2 & 0xFF);
            if(font->width > 8) newdata[i2+j2+1] = (char)((line2 >> 8) & 0xFF);
            if(font->width > 16) newdata[i2+j2+2] = (char)((line2 >> 16) & 0xFF);
            if(font->width > 24) newdata[i2+j2+3] = (char)((line2 >> 24) & 0xFF);

            if(hskip)    /* shriking */
            {
                j += (hskip*old_bytes);
                j2 += new_bytes;
            }
            else        /* expanding */
            {
                char rh = rhskip;
                while(rh--)
                {
                    newdata[i2+j2] = (char)(line2 & 0xFF);
                    if(font->width > 8) newdata[i2+j2+1] = (char)((line2 >> 8) & 0xFF);
                    if(font->width > 16) newdata[i2+j2+2] = (char)((line2 >> 16) & 0xFF);
                    if(font->width > 24) newdata[i2+j2+3] = (char)((line2 >> 24) & 0xFF);
                    j2 += new_bytes;
                }
                j += old_bytes;
            }

            /* make sure we don't write past this char's size */
            if(j2 >= new_charsize) break;
        }
        i += old_charsize;
        i2 += new_charsize;
    }

    font->charsize = new_charsize;
    font->module->handle_hw_change(font, newdata, new_datasize);
    status_msg("Font metrics updated successfully");
    free(newdata);
    force_font_dirty(font);
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
void shrink_glyphs(struct font_s *font, int old_length)
{
    status_msg("Applying changes to font length...");

    if(font->module->shrink_glyphs)
        font->module->shrink_glyphs(font, old_length);
    else
        psf_shrink_glyphs(font, old_length);
}

void expand_glyphs(struct font_s *font, int old_length, int option)
{
    status_msg("Applying changes to font length...");

    if(font->module->expand_glyphs)
        font->module->expand_glyphs(font, old_length, option);
    else
        psf_expand_glyphs(font, old_length, option);
}

int ok_to_change_length(struct font_s *font, int old_length)
{
    if(old_length == font->length) return 1;
    if(!font->module->shrink_glyphs && !font->module->expand_glyphs)
    {
        status_error("Font has a fixed length");
        font->length = old_length;
        return 0;
    }
    /*
    if(font->version == VER_CP || font->version == VER_RAW)
    {
        if(font->version == VER_CP)
            status_error("CP has a fixed length of 256");
        else if(font->version == VER_RAW)
            status_error("Raw FNT has a fixed length of 256");
        font->length = old_length;
        return;
    }
    */
  
    int h = 12;
    int w = 45;
    int x = 1, y = 1;

    if(h > SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w > SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;
  
    int res = 0;
    int sel = 0;
    int options = 0;

    setScreenColors(WHITE, BGDEFAULT);
    drawBox(x, y, h+x, w+y," Changing font length ", 1);

    setScreenColors(WHITE, BGDEFAULT);
    locate(x+1, y+1);
    printw("You chose to change this font's length,");
    locate(x+2, y+1);
    printw("From: %d chars", old_length);
    locate(x+3, y+1);
    printw("To: %d chars", font->length);

    if(old_length > font->length)
    {
        locate(x+5, y+1);
        printw("which means you will lose some glyphs.");
        locate(x+6, y+1);
        printw("What do you want to do?");
        options = 0;
    }
    else if(old_length < font->length)
    {
        locate(x+5, y+1);
        printw("which means we will add new glyphs to your");
        locate(x+6, y+1);
        printw("font. How do you want to do this?");
        options = 1;
    }
  
refresh:

    if(options == 0)
    {
        if(sel == 0) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+8, y+1);
        printw("Yes I know. Truncate the font already.");
        if(sel == 1) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+9, y+1);
        printw("Nope, I didn't mean that. Cancel.");
    }
    else
    {
        if(sel == 0) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+8, y+1);
        printw("Add empty glyphs.");
        if(sel == 1) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+9, y+1);
        printw("Roll over (i.e. copy from first glyphs)");
        if(sel == 2) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
        locate(x+10, y+1);
        printw("Nope, I didn't mean that. Cancel.");
    }

    refresh();
  
    char ch;
    char end = 0;

    while(!end)
    {
        ch = getKey();

        switch(ch)
        {
            case(ENTER_KEY):
            case(SPACE_KEY):
                /* user cancelled */
                if((options == 1 && sel == 2) || (options == 0 && sel == 1))
                {
                    font->length = old_length;
                    res = 1;
                }
                else
                {
                    /* user chose to truncate font length */
                    if(options == 0 && sel == 0) shrink_glyphs(font, old_length);
                    else expand_glyphs(font, old_length, sel);
                    res = 0;
                }
                end = 1;
                break;

            case(DOWN_KEY):
            case(RIGHT_KEY):
                if(options == 0)
                {
                    sel = !sel;
                }
                else
                {
                    if(sel < 2) sel++;
                    else sel = 0;
                }
                goto refresh;

            case(LEFT_KEY):
            case(UP_KEY):
                if(sel == 0)
                {
                    if(options == 0) sel = 1;
                    else sel = 2;
                }
                else sel--;
                goto refresh;
                break;

            case(ESC_KEY):
                font->length = old_length;
                res = 1;
                end = 1;
                break;
        }
    }

    /* cancelled? */
    return !(res == 1);
}

void handle_length_change(struct font_s *font, int old_length)
{
    if(old_length == font->length) return;

    if(font->has_unicode_table)
    {
        if(font->length > old_length)
        {
            if(!create_empty_unitab(font)) goto memory_error;
            get_font_unicode_table(font);
        }
    }

    /* update font header */
    if(font->module->update_font_hdr)
        font->module->update_font_hdr(font);
    goto end;

memory_error:

    status_error("Insufficient memory");

end:

    force_font_dirty(font);
}


void handle_unicode_table_change(struct font_s *font, char old_has_unicode_table)
{
    if(font->has_unicode_table == old_has_unicode_table) return;
    if(font->module->handle_unicode_table_change)
        font->module->handle_unicode_table_change(font, old_has_unicode_table);
}


void handle_version_change(struct font_s *font, char old_version)
{
    if(font->version == old_version) return;

    char ver = font->version;
    if(ver > VER_PSF2) font->version = VER_PSF1;
    font->module->convert_to_psf(font);
    if(ver > VER_PSF2) font->version = ver;

    struct module_s *newmod = (struct module_s *)NULL;
    if(font->version <= 2)
        newmod = get_module_by_name("psf");
    else
        newmod = get_module_by_name(get_version_str(font->version));

    if(!newmod) return;
    font->module = newmod;
    if(font->version > VER_PSF2)
        font->module->handle_version_change(font, old_version);
    force_font_dirty(font);
}


void refresh_metrics_window(struct font_s *font, int x, int y, int sel)
{
    /* print metrics */
    if(sel == 0) setScreenColors(BLACK, BGWHITE);
    else setScreenColors(WHITE, BGDEFAULT);
    locate(x+3, y+4);
    printw("Height:          %2d pixels", font->height);
  
    if(sel == 1) setScreenColors(BLACK, BGWHITE);
    else setScreenColors(WHITE, BGDEFAULT);
    locate(x+4, y+4);
    printw("Width:           %2d pixels", font->width);
  
    if(sel == 2) setScreenColors(BLACK, BGWHITE);
    else setScreenColors(WHITE, BGDEFAULT);
    locate(x+5, y+4);
    printw("Char-size:       %2d bytes", font->charsize);
  
    if(sel == 3) setScreenColors(BLACK, BGWHITE);
    else setScreenColors(WHITE, BGDEFAULT);
    locate(x+6, y+4);
    printw("Characters:          %4d", font->length);
  
    if(sel == 4) setScreenColors(BLACK, BGWHITE);
    else setScreenColors(WHITE, BGDEFAULT);
    locate(x+7, y+4);
    printw("Unicode table:        %s", font->has_unicode_table ? "Yes" : "No");
  
    if(sel == 5) setScreenColors(BLACK, BGWHITE);
    else setScreenColors(WHITE, BGDEFAULT);
    locate(x+8, y+4);
    printw("Version:             %s", get_version_str(font->version));
    /*
    if(font->version == VER_CP)
        printw("Version:             CP");
    else if(font->version == VER_RAW)
        printw("Version:             FNT");
    else
        printw("Version:             PSF%1d", font->version);
    */
    if(sel == 6) setScreenColors(GREEN, BGRED);
    else setScreenColors(WHITE, BGRED);
    locate(x+10, y+6);
    printw("   OK   ");

    if(sel == 7) setScreenColors(GREEN, BGRED);
    else setScreenColors(WHITE, BGRED);
    locate(x+10, y+20);
    printw(" CANCEL ");

    refresh();
}


int show_font_metrics(struct font_s *font)
{
    int h = 12;
    int w = 36;
    int x = 1, y = 1;

    if(h > SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w > SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;
  
    int res = 0;
    int sel = 0;
    int changed = 0;
    /* save these in case user cancelled */
    unsigned int old_height = font->height;
    unsigned int old_width = font->width;
    unsigned int old_charsize = font->charsize;
    unsigned int old_length = font->length;
    unsigned char old_has_unicode_table = font->has_unicode_table;
    unsigned char old_version = font->version;
  
    /* we add one because PSF has two versions but one module */
    int max_ver = get_registered_modules()+1;

draw_win:
 
    setScreenColors(WHITE, BGDEFAULT);
    drawBox(x, y, h+x, w+y," - Font Metrics - ", 1);
  
    /* print font name on top */
    setScreenColors(BROWN, BGWHITE);
    locate(x+1, y+1);
    printw("%*s", w-2, " ");
    if(!font_file_name) goto skip;

    int font_name_len = strlen(font_file_name);

    if(font_name_len <= w-2)
    {
        int mid = (w-font_name_len)/2;
        locate(x+1, y+mid);
        printw("%s", font_file_name);
    }
    else
    {
        int j = font_name_len-(w-3);
        locate(x+1, y+1);
        printw("..%s", font_file_name+j);
    }

skip:

    refresh_metrics_window(font, x, y, sel);
  
    char ch;
    char end = 0;

    while(!end)
    {
        ch = getKey();

        switch(ch)
        {
            case(LEFT_KEY):
                if(sel == 7)
                {
                    sel = 6;
                    refresh_metrics_window(font, x, y, sel);
                    break;
                }

            case(UP_KEY):
                if(sel == 3) sel = 1;
                else if(sel == 7) sel = 5;
                else if(sel > 0) sel--;
                else sel = 6;
                refresh_metrics_window(font, x, y, sel);
                break;

            case(TAB_KEY):
            case(RIGHT_KEY):
                if(sel == 6)
                {
                    sel = 7;
                    refresh_metrics_window(font, x, y, sel);
                    break;
                }

            case(DOWN_KEY):
                if(sel == 1) sel = 3;
                else if(sel >= 6) sel = 0;
                else sel++;
                refresh_metrics_window(font, x, y, sel);
                break;

            case(ENTER_KEY):
            case(SPACE_KEY):
                if(sel == 7)        /* cancel */
                {
                    res = 1;
                    end = 1;
                    break;
                }
                else if(sel == 6)    /* ok */
                {
                    res = 0;
                    end = 1;
                    break;
                }
                else
                {
                    if(sel == 0)
                    {
                        /*
                        if(font->height < 8) font->height = 8;
                        else if(font->height < 16) font->height = 16;
                        else if(font->height < 32) font->height = 32;
                        //else if(font->height < 64) font->height = 64;
                        else font->height = 8;
                        */

                        struct module_s *mod = 
                                get_module_by_name(font->version < 3 ? "psf" : 
                                              get_version_str(font->version));

                        //if(font->height > mod->max_height) 
                        //    font->height = mod->max_height;
                        int h = mod->next_acceptable_height(font);

                        if(h == font->height)
                        {
                            msgBox("This font format has only one height.", BUTTON_OK, INFO);
                        }
                        else
                        {
                            font->height = h;
                            font->charsize = font->height*((font->width+7)/8);
                        }

                    }
                    else if(sel == 1)
                    {
                        /*
                        if(font->width < 8) font->width = 8;
                        else if(font->width < 16) font->width = 16;
                        else if(font->width < 32) font->width = 32;
                        //else if(font->width < 64) font->width = 64;
                        else font->width = 8;
                        */

                        struct module_s *mod = 
                                get_module_by_name(font->version < 3 ? "psf" : 
                                              get_version_str(font->version));

                        //if(font->width > mod->max_width) 
                        //    font->width = mod->max_width;
                        /* any width larger than 8 is incompatible with PSF1 */
                        ////if(font->width > 8 && font->version == VER_PSF1) font->version = VER_PSF2;
                        //if(font->width > 8 && font->version == VER_PSF1) font->width = 8;
                        int w = mod->next_acceptable_width(font);

                        if(w == font->width)
                        {
                            msgBox("This font format has only one width.", BUTTON_OK, INFO);
                        }
                        else
                        {
                            font->width = w;
                            font->charsize = font->height*((font->width+7)/8);
                        }
                    }
                    else if(sel == 3)
                    {
                        if(font->version == VER_BDF)
                        {
                            char s[10], *input;
                            sprintf(s, "%u", font->length);

                            input = inputBoxI("Enter new glyph count:", 
                                                s, " Glyph count ");

                            if(strlen(input) == 0)
                            {
                                refresh_metrics_window(font, x, y, sel);
                                break;
                            }

                            int l = atoi(input);
                            if(l <= 0)
                            {
                                msgBox("Invalid glyph count.", BUTTON_OK, ERROR);
                            }
                            else font->length = l;
                        }
                        else if(font->length < 256) font->length = 256;
                        else if(font->length < 512) font->length = 512;
                        else font->length = 256;
        
                        struct module_s *mod = 
                            get_module_by_name(font->version < 3 ? "psf" :
                                               get_version_str(font->version));

                        if(font->length > mod->max_length)
                            font->length = mod->max_length;
                    }
                    else if(sel == 4)
                    {
                        font->has_unicode_table = !font->has_unicode_table;
                        if(font->version == VER_CP || font->version == VER_RAW)
                            font->has_unicode_table = 0;
                    }
                    else if(sel == 5)
                    {
                        /*
                        if(font->version < 4) font->version++;
                        else if(font->version == VER_RAW) font->version = VER_PSF1;
                        else font->version = VER_PSF1;
                        */
                        if(font->version < max_ver) font->version++;
                        else font->version = VER_PSF1;

                        struct module_s *mod = 
                            get_module_by_name(font->version < 3 ? "psf" : 
                            get_version_str(font->version));

                        if(font->length > mod->max_length) font->length = mod->max_length;
                        //if(font->width > mod->max_width) font->width = mod->max_width;
                        //if(font->height > mod->max_height) font->height = mod->max_height;

                        /* any width larger than 8 is incompatible with PSF1 & CP */
                        /* NOTE: we have to check width for PSF1 manually, as the
                         *       PSF module works for PSF2 limits which are bigger
                         *       than PSF1 limits.
                         */
                        if(!mod->is_acceptable_width(font))
                            font->width = mod->next_acceptable_width(font);
                        if(!mod->is_acceptable_height(font))
                            font->height = mod->next_acceptable_height(font);
                        //if(font->width != 8 && font->version == VER_PSF1) font->width = 8;
                        //if(font->width != 8 && font->version == VER_CP) font->width = 8;
                        /***
                        if(font->version != VER_PSF2 && font->width > 8) font->width = 8;
                        if(font->version == VER_CP || font->version == VER_RAW)
                        {
                            font->has_unicode_table = 0;
                            font->length = 256;
                            if(font->height > 16) font->height = 16;
                        }
                        ***/
                        font->charsize = font->height*((font->width+7)/8);
                    }
                    changed = 1;
                    goto draw_win;
                }
                break;

            case(ESC_KEY):
                res = 1;
                end = 1;
                break;
        }
    }
  
    /* user cancelled, or pressed OK but didn't make changes */
    if(res == 1 || !changed)
    {
        /* restore old settings */
        font->height = old_height;
        font->width = old_width;
        font->charsize = old_charsize;
        font->length = old_length;
        font->has_unicode_table = old_has_unicode_table;
        font->version = old_version;
        return 1;
    }

    /* check for uneven metrics */
    int rh = (old_height > font->height) ? old_height%font->height :
                                           font->height%old_height;
    int rw = (old_width > font->width) ? old_width%font->width :
                                         font->width%old_width;

    if(rh || rw)
    {
        int m = msgBox("Ratio between old and new metrics\n"
                       "is uneven. Some of the glyphs may appear\n"
                       "skewed or distorted. Continue?", 
                       BUTTON_YES|BUTTON_NO, CONFIRM);

        if(m == BUTTON_NO)
        {
            goto cancel;
        }
    }

    if(!ok_to_change_length(font, old_length))
    {
        goto cancel;
    }
  
    unsigned char tmp_has_unicode_table = font->has_unicode_table;
    unsigned char tmp_ver = font->version;
    font->has_unicode_table = old_has_unicode_table;
    font->version = old_version;

    handle_hw_change(font, old_height, old_width, old_length);
    handle_length_change(font, old_length);

    font->has_unicode_table = tmp_has_unicode_table;
    font->version = tmp_ver;
  
    handle_unicode_table_change(font, old_has_unicode_table);
    handle_version_change(font, old_version);
    calc_max_zoom(font);
    //status_msg("Font metrics updated successfully.");
    return 0;

cancel:

    /* restore old settings */
    font->height = old_height;
    font->width = old_width;
    font->charsize = old_charsize;
    font->length = old_length;
    font->has_unicode_table = old_has_unicode_table;
    font->version = old_version;
    return 1;
}


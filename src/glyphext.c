/* 
 *    Copyright 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: glyphext.c
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

#include "defs.h"
#include "view.h"
#include "glyph.h"
#include "menu.h"

/***********************************
 * Extended Glyph operations
 ***********************************/
char *glyphops_text[] =
{
    "Center horizontally",
    "Center vertically",
    "Center both",
    "Rotate 90* clockwise",
    "Rotate 90* counter-clockwise",
};
int glyphops_text_len = sizeof(glyphops_text)/sizeof(char *);

void (*glyphops_funcs[])(struct font_s *font) =
{
    glyphop_center_horz,
    glyphop_center_vert,
    glyphop_center_both,
    glyphop_rotate_cw,
    glyphop_rotate_ccw,
};


void show_ext_glyph_operations(struct font_s *font)
{
    int i = show_menu(glyphops_text, glyphops_text_len, 
                      "Extended Glyph Operations", 0, 0, 0, 0);
    if(i >= 0)
    {
        glyphops_funcs[i](font);
    }
    refresh_left_window(font);
    refresh_view_status_msg(font_file_name, font);
}

static inline unsigned int set_bit_index(unsigned int i)
{
    int j;

    for(j = 0; j < (int)(sizeof(unsigned int)*8); j++)
    {
        if(i & (1 << j)) return j;
    }

    return -1;
}

void glyphop_center_horz(struct font_s *font)
{
    // find the leftmost and rightmost set pixels in the glyph
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    unsigned char *data2 = data;
    unsigned int h = font->charsize/font->height;
    unsigned int i, j, k;
    unsigned int base_index = (1 << (font->width - 1));
    unsigned int index, line;
    unsigned int redge /* = font->width-1 */, ledge = 0;

    if(font->width < 8) base_index <<= (8-font->width);
    else if(font->width > 8 && font->width < 16) base_index <<= (16-font->width);
    else if(font->width > 16 && font->width < 32) base_index <<= (32-font->width);

    redge = base_index;

    for(i = 0; i < font->height; i++)
    {
        for(line = 0, j = 0; j < h; j++)
        {
            line |= (unsigned int)(data[j]) << (j*8);
        }

        data += h;
        index = base_index;

        for(j = 0; j < font->width; j++)
        {
            if(line & index)
            {
                //if(j < redge) redge = j;
                //if(j > ledge) ledge = j;
                if(index < redge) redge = index;
                if(index > ledge) ledge = index;
            }

            index >>= 1;
        }
    }

    ledge = set_bit_index(ledge);
    redge = set_bit_index(redge);

    // check glyph data is not stretching all the way from left to right edges
    if((ledge == font->width-1) && (redge == 0))
    {
        return;
    }

    data = data2;

    // find the glyph's horizontal center
    j  = ledge + redge;
    i  = j/2;
    
    // find where the horizontal center should actually be
    index = base_index;
    for(j = 0; j < font->width / 2; j++) index >>= 1;
    index = set_bit_index(index);
    
    // if i > index, we shift to the right
    // if i < index, we shift to the left
    // if i == index, do nothing
    int goright;

    if(i > index) { k = i - index; goright = 1; }
    else          { k = index - i; goright = 0; }

    if(k)
    {
        for(i = 0; i < font->height; i++)
        {
            unsigned int line = 0;

            for(j = 0; j < h; j++)
            {
                line |= (unsigned int)(data[j]) << (j*8);
            }

            if(goright) line >>= k;
            else line <<= k;

            for(j = 0; j < h; j++)
            {
                data[j] = line & 0xff;
                line >>= 8;
            }
            data += h;
        }
    }

    // update the character info struct
    struct char_info_s *char_info = (struct char_info_s *)font->char_info;
    if(char_info)
    {
        int gindex = glyph_index(font);
        char_info[gindex].dwidthX = font->width;
        char_info[gindex].lBearing = 0;
        char_info[gindex].rBearing = font->width;
    }
    force_font_dirty(font);
}

void glyphop_center_vert(struct font_s *font)
{
    // find the topmost and lowermost set pixels in the glyph
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    unsigned char *data2 = data;
    int h = font->charsize/font->height;
    int i, j, k;
    int uedge = font->height-1, ledge = 0;

    for(i = 0; i < (int)font->height; i++)
    {
        unsigned int line = 0;
        for(j = 0; j < h; j++)
        {
            line |= (unsigned int)(data[j]) << (j*8);
        }
        data += h;

        if(line)
        {
            if(i < uedge) uedge = i;
            if(i > ledge) ledge = i;
        }
    }
    
    // check glyph data is not stretching all the way from top to bottom edges
    if((uedge == (int)font->height-1) && (ledge == 0))
    {
        return;
    }

    data = data2;
    j  = uedge;
    j += font->height-1-ledge;
    i  = j/2;
    if(i > uedge)
    {
        k = i - uedge;
        j = k*h;
        data += font->charsize-h;
        for(i = font->height; i >= k; i--)
        {
            memcpy(data, data-j, h);
            data -= h;
        }
        for( ; i >= 0; i--)
        {
            memset(data, 0, h);
            data -= h;
        }
    }
    else
    {
        k = uedge - i;
        j = k*h;
        for(i = 0; i <= ledge-k; i++)
        {
            memcpy(data, data+j, h);
            data += h;
        }
        for( ; i < (int)font->height; i++)
        {
            memset(data, 0, h);
            data += h;
        }
    }
    // update the character info struct
    struct char_info_s *char_info = (struct char_info_s *)font->char_info;
    if(char_info)
    {
        int gindex = glyph_index(font);
        char_info[gindex].dwidthY = font->height;
        char_info[gindex].charAscent = font->height;
        char_info[gindex].charDescent = 0;
    }
    force_font_dirty(font);
}

void glyphop_center_both(struct font_s *font)
{
    glyphop_center_horz(font);
    glyphop_center_vert(font);
}

void glyphop_rotate_ccw(struct font_s *font)
{
    struct char_info_s *char_info = (struct char_info_s *)font->char_info;
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    unsigned char *data2 = data;
    int gindex = glyph_index(font);
    int gw = char_info ? char_info[gindex].dwidthX : (int)font->width;
    int gh = char_info ? char_info[gindex].charAscent+char_info[gindex].charDescent :
                         (int)font->height;
    int skipbytes = (font->width-gw+7)/8;
    int w = (gw+7)/8;
    int w2 = (font->width+7)/8;
    int l1 = (gh/2)-(gw/2);     // starting line
    int c1 = (gw/2)+(gh/2)-1;     // starting pixel
    unsigned char buffer[font->charsize];
    memset(buffer, 0, font->charsize);
    int i, j, l2, c2;
    for(i = 0; i < gh; i++)
    {
        unsigned int line;

        for(line = 0, j = 0; j < w; j++)
        {
            line |= (unsigned int)(*data) << (j*8);
            data++;
        }

        for(j = 0; j < skipbytes; j++) data++;

        for(j = 0; j < gw; j++)
        {
            l2 = l1+j;
            if(l2 < 0 || l2 >= (int)font->height) continue;
            c2 = c1-i;
            if(c2 < 0 || c2 >= (int)font->width) break;
            int byte = (l2*w2)+(c2/8);      // index to dest byte
            int bitindex  = 1 << (c2%8);
            if(line & (1 << j))
            {
                buffer[byte] |= (unsigned char)bitindex;
            }
            else
            {
                buffer[byte] &= (unsigned char)~bitindex;
            }
        }
    }

    memcpy(data2, buffer, font->charsize);
    force_font_dirty(font);
}

void glyphop_rotate_cw(struct font_s *font)
{
    struct char_info_s *char_info = (struct char_info_s *)font->char_info;
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    unsigned char *data2 = data;
    int gindex = glyph_index(font);
    int gw = char_info ? char_info[gindex].dwidthX : (int)font->width;
    int gh = char_info ? char_info[gindex].charAscent+char_info[gindex].charDescent :
                         (int)font->height;
    int skipbytes = (font->width-gw+7)/8;
    int w = (gw+7)/8;
    int w2 = (font->width+7)/8;
    int l1 = (gh/2)+(gw/2)-1;     // starting line
    int c1 = (gw/2)-(gh/2);     // starting pixel
    unsigned char buffer[font->charsize];
    memset(buffer, 0, font->charsize);
    int i, j, l2, c2;

    for(i = 0; i < gh; i++)
    {
        unsigned int line;

        for(line = 0, j = 0; j < w; j++)
        {
            line |= (unsigned int)(*data) << (j*8);
            data++;
        }

        for(j = 0; j < skipbytes; j++) data++;

        for(j = 0; j < gw; j++)
        {
            l2 = l1-j;
            if(l2 < 0 || l2 >= (int)font->height) continue;
            c2 = c1+i;
            if(c2 < 0 || c2 >= (int)font->width) break;
            int byte = (l2*w2)+(c2/8);      // index to dest byte
            int bitindex  = 1 << (c2%8);
            if(line & (1 << j))
            {
                buffer[byte] |= (unsigned char)bitindex;
            }
            else
            {
                buffer[byte] &= (unsigned char)~bitindex;
            }
        }
    }

    memcpy(data2, buffer, font->charsize);
    force_font_dirty(font);
}


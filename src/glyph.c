/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: glyph.c
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
#include <sys/types.h>

/***********************************
 * Glyph operations
 ***********************************/
void copy_glyph(struct font_s *font, unsigned char buffer[])
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memcpy((void *)buffer, (void *)data, font->charsize);
}

void cut_glyph(struct font_s *font, unsigned char buffer[])
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memcpy((void *)buffer, (void *)data, font->charsize);
    memset((void *)data, 0, font->charsize);
    force_font_dirty(font);
}

void paste_glyph(struct font_s *font, unsigned char buffer[])
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memcpy((void *)data, (void *)buffer, font->charsize);
    force_font_dirty(font);
}

void clear_glyph(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memset((void *)data, 0, font->charsize);
    force_font_dirty(font);
}

void set_glyph(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memset((void *)data, 255, font->charsize);
    force_font_dirty(font);
}

void flip_glyph_horizontally(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    int h = font->charsize/font->height;
    unsigned int i, j, k;

    /*
     * in the following operations we assume we are working with a maximum
     * font width of 32 (i.e. 4 bytes, the size of unsigned int).
     * TODO: handle more font widths properly.
     */
    for(i = 0; i < font->height; i++)
    {
        unsigned int line = 0, line2 = 0;

        for(j = 0; j < h; j++)
        {
            line |= (unsigned int)(data[j]) << (j*8);
        }

        /* flip */
        k = 1;
        for(j = (1 << 31); j > (1 << 15); j >>= 1, k <<= 1)
        {
            unsigned int tmp = line & j;
            if(line & k) line2 |= j;
            if(tmp) line2 |= k;
        }

        /* fix fonts with width not a multiple of 8 */
        if(font->width > 8 && font->width <= 16)
        {
            //line2 >>= 16;
            //line2 >>= (16-font->width);
        }

        /* copy back */
        for(j = 0; j < h; j++)
        {
            data[h-j-1] = (unsigned char)((line2 >> ((3-j)*8)) & 0xFF);
        }

        /* fix fonts with width not a multiple of 8 */
        if(font->width > 8 && font->width <= 16)
        {
            u_int16_t *l = (u_int16_t *)data;
            *l <<= (16-font->width);
        }

        data += h;
    }

    force_font_dirty(font);
}

void flip_glyph_vertically(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    int h = font->charsize/font->height;
    int i, j;

    for(i = 0; i < font->charsize/2; i += h)
    {
        unsigned int line = 0, line2 = 0;

        for(j = 0; j < h; j++)
        {
            line |= (unsigned int)(data[i+j]) << (j*8);
        }

        /* flip */
        line2 = font->charsize-i;
        unsigned char *flip_with = data+line2-h;

        for(j = 0; j < h; j++)
        {
            data[i+j] = flip_with[j];
            flip_with[j] = (unsigned char)((line >> (j*8)) & 0xFF);
        }
    }

    force_font_dirty(font);
}

void export_glyphs(struct font_s *font, int as_c_file)
{
    char *comment_start = "#";
    char *comment_end = "#";
    char *comment_chars = "#";
    char *fn = 0;
    int len = 0;

    if(font_file_name)
    {
        len = strlen(font_file_name);
        fn = (char *)malloc(len+7);
        if(!fn) goto memory_error;
        strcpy(fn, font_file_name);
    }
    else
    {
        len = 7;
        fn = (char *)malloc(len+7);
        if(!fn) goto memory_error;
        strcpy(fn, "Unnamed");
    }

    strcat(fn, as_c_file ? ".c" : ".glyph");

    FILE *f = fopen(fn, "w");
    if(!f) goto file_error;

    if(as_c_file)
    {
        comment_start = "/*";
        comment_end = " */";
        comment_chars = " *";
    }

    fprintf(f, "%s\n", comment_start);
    fprintf(f, "%s Fontopia glyph file.\n%s\n", comment_chars, comment_chars);
    fprintf(f, "%s This file contains a textual representation of the "
               "glyphs of the font:\n", comment_chars);
    fprintf(f, "%s    %s\n", comment_chars, font_file_name);
    fprintf(f, "%s\n", comment_chars);
    fprintf(f, "%s This file is informative only, as the actual font file "
               "may have been\n", comment_chars);
    fprintf(f, "%s modified in fontopia. Currently you can not open this file "
               "in fontopia for\n", comment_chars);
    fprintf(f, "%s editing, we will try to support this function in "
               "future versions.\n", comment_chars);
    fprintf(f, "%s\n", comment_chars);
    fprintf(f, "%s For now, this file is yours!. You are free to do whatever "
               "on heavenly earth\n", comment_chars);
    fprintf(f, "%s you want to do with it!.\n", comment_chars);
    fprintf(f, "%s\n", comment_chars);
    fprintf(f, "%s Some info about your font:\n", comment_chars);
    fprintf(f, "%s\n", comment_chars);
    fprintf(f, "%s Length=%d chars\n", comment_chars, font->length);
    fprintf(f, "%s Width=%d pixels\n", comment_chars, font->width);
    fprintf(f, "%s Height=%d pixels\n", comment_chars, font->height);
    fprintf(f, "%s Charsize=%d bytes\n", comment_chars, font->charsize);
    fprintf(f, "%s\n", comment_end);

    int i, j, k;
    int h = font->charsize/font->height;
    unsigned char *data = font->data;
    unsigned int line, mask;

    if(as_c_file)
    {
        fprintf(f, "\n#include <stdint.h>\n\n");
        fprintf(f, "#define CHAR_WIDTH      %d\n", font->width);
        fprintf(f, "#define CHAR_HEIGHT     %d\n", font->height);
        fprintf(f, "#define CHAR_BYTES      %d\n", font->charsize);
        fprintf(f, "#define TOTAL_CHARS     %d\n", font->length);
        fprintf(f, "\nuint8_t fontdata[] =\n{\n");
    }

    for(i = 0; i < font->length; i++)
    {
        if(as_c_file) fprintf(f, "  /* %4d */  ", i);
        else          fprintf(f, "\nchar %d:\n", i);

        for(j = 0; j < font->height; j++)
        {
            if(as_c_file)
            {
                for(k = 0; k < h; k++) fprintf(f, "0x%02x, ", data[(j*h)+k]);
            }
            else
            {
                line = 0;
                for(k = 0; k < h; k++) line |= (unsigned int)data[(j*h)+k] << (k*8);

                if(h == 1) fprintf(f, "0x%02x | ", line);
                else if(h == 2) fprintf(f, "0x%04x | ", line);
                else if(h == 3) fprintf(f, "0x%06x | ", line);
                else if(h == 4) fprintf(f, "0x%08x | ", line);
                else fprintf(f, "0x%0x | ", line);

                mask = (1 << ((h*8)-1));

                for(k = 0; k < h*8; k++)
                {
                    fprintf(f, "%c", (line & mask) ? 'X' : ' ');
                    mask >>= 1;
                }

                fprintf(f, "|\n");
                //data += h;
            }
        }

        if(as_c_file) fprintf(f, "\n");

        data += font->charsize;
    }

    if(as_c_file) fprintf(f, "};\n\n");
  
    fclose(f);
    free(fn);

    if(as_c_file)
        status_msg("Glyphs saved to file with .c extension");
    else
        status_msg("Glyphs saved to file with .glyph extension");
    return;

file_error:  

    status_error("File I/O error!");
    return;

memory_error:

    if(fn) free(fn);
    status_error("Insufficient memory!");
}

void invert_glyph(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    //int h = font->charsize/font->height;
    unsigned int i;
    unsigned char j;

    /*
     * in the following operations we assume we are working with a maximum
     * font width of 32 (i.e. 4 bytes, the size of unsigned int).
     * TODO: handle more font widths properly.
     */
    for(i = 0; i < font->charsize; i++)
    {
        unsigned char line = data[i];

        for(j = (1 << 7); j > 0; j >>= 1)
        {
            if(line & j) line &= ~j;
            else line |= j;
        }

        data[i] = line;
    }

    force_font_dirty(font);
}

int get_glyph_index(struct font_s *font)
{
    int r = right_window.cursor.row+right_window.first_vis_row;
    return (r*right_window.cols_per_row + right_window.cursor.col)*font->charsize;
}

/*******************************
 *******************************
 * Drawing routines.
 *******************************
 *******************************/
unsigned char draw_buffer[(MAX_WIDTH/8)*MAX_HEIGHT];
unsigned char draw_buffer_backup[(MAX_WIDTH/8)*MAX_HEIGHT];

void set_start_coords(int *row1, int *row2, int *col1, int *col2)
{
    if(active_window != &left_window)
    {
        active_window = &left_window;
        left_window.cursor.col = 0;
        left_window.cursor.row = 0;
    }

    *row1 = left_window.cursor.row;
    *col1 = left_window.cursor.col;
    *row2 = *row1;
    *col2 = *col1;
}

static inline void init_buffer(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memcpy((void *)draw_buffer, (void *)data, font->charsize);
    memcpy((void *)draw_buffer_backup, (void *)data, font->charsize);
}

static inline void invalidate_buffer(struct font_s *font)
{
    memcpy((void *)draw_buffer, (void *)draw_buffer_backup, font->charsize);
}

static inline void writeout_buffer(struct font_s *font)
{
    unsigned char *data = font->data;
    data += get_glyph_index(font);
    memcpy((void *)data, (void *)draw_buffer, font->charsize);
}

static inline void _draw_box(struct font_s *font,
            char start_row, char end_row,
            char start_col, char end_col,
            char is_hollow)
{
    int i, j;
    int linebytes = font->width/8;

    if(font->width%8) linebytes++;

    for(i = start_row; i <= end_row; i++)
    {
        int byteindex = i*linebytes;

        if(is_hollow)
        {
            if(i == start_row || i == end_row) goto solid_box;
            else
            {
                int byte = byteindex + (int)((font->width-start_col-1)/8);
                int bitindex  = 1 << ((font->width -start_col-1) % 8);
                draw_buffer[byte] |= (char)bitindex;
                byte = byteindex + (int)((font->width-end_col-1)/8);
                bitindex = 1 << ((font->width -end_col-1) % 8);
                draw_buffer[byte] |= (char)bitindex;
            }
        }
        /* solid */
        else
        {

solid_box:

            for(j = start_col; j <= end_col; j++)
            {
                int byte = byteindex + (int)((font->width-j-1)/8);
                int bitindex  = 1 << ((font->width - j - 1) % 8);
                draw_buffer[byte] |= (char)bitindex;
            }
        }
    }
}

static inline void _draw_line(struct font_s *font,
            char start_row, char end_row,
            char start_col, char end_col,
            char is_hollow)
{
    int i, j;
    int linebytes = font->width/8;

    if(font->width%8) linebytes++;

    /********check if the line is horizontal********/
    if(start_row == end_row)
    {
        int byteindex = start_row*linebytes;
        for(j = start_col; j < end_col; j++)
        {
            int byte = byteindex + (int)((font->width-j-1)/8);
            int bitindex  = 1 << ((font->width - j - 1) % 8);
            draw_buffer[byte] |= (char)bitindex;  
        }
        return;
    }

    /********check if the line is vertical********/
    if(start_col == end_col)
    {
        int bitindex  = 1 << ((font->width-start_col-1) % 8);
        for(i = start_row; i < end_row; i++)
        {
            int byteindex = i*linebytes;
            byteindex += (int)((font->width-start_col-1)/8);
            draw_buffer[byteindex] |= (char)bitindex;
        }
        return;
    }

    /********if we reached here, it means the line is oblique********/
    /*
     * FIXME: This technique is drawing broken lines. Needs serious fixing.
     */
    double ri = (end_col-start_col);
    ri /= (end_row-start_row);        //calculate the step increment in x
    double rj = (end_row-start_row);
    rj /= (end_col-start_col);        //calculate the step increment in y
    rj = 1;
    double k, l = start_row;

    for(k = start_col; k <= end_col; k += ri)
    {
        int byteindex = (int)l*linebytes;
        byteindex += (int)((font->width-(int)k-1)/8);
        int bitindex  = 1 << ((font->width-(int)k-1) % 8);
        draw_buffer[byteindex] |= (char)bitindex;
        l += rj;
    }
}


void handle_moves(struct font_s *font, char break_key)
{
    int ch;
    char h = font->height;
    char w = font->width;
    //int i, j;
    int row1, row2, col1, col2;
    set_start_coords(&row1, &row2, &col1, &col2);
    int linebytes = font->width/8;
    if(font->width%8) linebytes++;
    char is_hollow = 0;

draw_all:

    if(is_hollow)
        status_msg("Move around to resize. Hit ENTER to accept, H to make solid");
    else
        status_msg("Move around to resize. Hit ENTER to accept, H to make hollow");
  
draw:

    invalidate_buffer(font);
    char start_row, end_row;
    char start_col, end_col;
    if(row2 < row1)
    { start_row = row2; end_row = row1; }
    else
    { start_row = row1; end_row = row2; }
    if(col2 < col1)
    { start_col = col2; end_col = col1; }
    else
    { start_col = col1; end_col = col2; }
    left_window.cursor.row = row2;
    left_window.cursor.col = col2;
    
    /* draw box */
    if(break_key == 'b')
    {
        _draw_box(font, start_row, end_row, start_col, end_col, is_hollow);
    } /* box draw */
    else if(break_key == 'l')
    {
        _draw_line(font, start_row, end_row, start_col, end_col, is_hollow);
    } /* box draw */

    writeout_buffer(font);
    refresh_left_window(font);
  
    /* infinite program loop */
    while(1)
    {
        ch = getKey();

        if(ch == (int)break_key) break;
    
        switch(ch)
        {
            case('h'):
                is_hollow = !is_hollow;
                goto draw_all;

            case(UP_KEY):
                if(row2 == 0) break;
                row2--;
                goto draw;

            case(LEFT_KEY):
                if(col2 == 0) break;
                col2--;
                goto draw;

            case(DOWN_KEY):
                if(row2 == h-1) break;
                row2++;
                goto draw;

            case(RIGHT_KEY):
                if(col2 == w-1) break;
                col2++;
                goto draw;

            case(ENTER_KEY):
                return;

            case(ESC_KEY):
                invalidate_buffer(font);
                writeout_buffer(font);
                return;
        }
    }
}

void draw_shape(struct font_s *font, char break_key)
{
    init_buffer(font);
    handle_moves(font, break_key);
    refresh_left_window(font);
}


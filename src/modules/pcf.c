/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: pcf.c
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

#include <ctype.h>
#include "../defs.h"
#include "../view.h"
#include "../metadata.h"
#include "pcf.h"

extern unsigned short default_unicode_table[]; /* psf.c */
static char filesig[] = { 1, 'f', 'c', 'p' };


static inline int32_t get_int(unsigned char *d, int swapbits)
{
    int32_t d1, d2, d3, d4;

    if(swapbits)
    {
        d1 = reverse_char(d[0]); d2 = reverse_char(d[1]);
        d3 = reverse_char(d[2]); d4 = reverse_char(d[3]);
    }
    else
    {
        d1 = d[0]; d2 = d[1];
        d3 = d[2]; d4 = d[3];
    }

    return d1 | (d2 << 8) | (d3 << 16) | (d4 << 24);
}

/*
static inline int16_t get_short(unsigned char *d)
{
    int16_t res = 0;
    res = d[0] | ((int16_t)d[1] << 8);
    return res;
}
*/

static inline u_int16_t get_ushort(unsigned char *d, int swapbits)
{
    u_int16_t d1, d2;

    if(swapbits) { d1 = reverse_char(d[0]); d2 = reverse_char(d[1]); }
    else { d1 = d[0]; d2 = d[1]; }

    return d1 | (d2 << 8);
}

static inline int32_t pcf_get_lsbint(int32_t i)
{
    if(big_endian) return swap_dword(i);
    else return i;
}

/*
 * make sure the font has a name and other basic data, usually called before saving to file.
 */
void __pcf_check_font_meta(struct font_s *font, int index, char *value)
{
    struct metadata_item_s *meta = (struct metadata_item_s *)font->metadata;

    if(meta[index].value2 && meta[index].value2[0] != '\0')
    {
        return;
    }

    char *name;
    switch(index)
    {
        case METADATA_FONT:
        case METADATA_FAMILY_NAME:
            name = strrchr(font_file_name, '/');
            if(name) name++;
            else name = font_file_name;

            char *dot = strrchr(name, '.');
            if(dot)
            {
                int len = dot-name;
                char d[len+1];
                strncpy(d, name, len);
                d[len] = '\0';
                save_metadata_str(font, index, d);
            }
            else
            {
                save_metadata_str(font, index, name);
            }
            break;

        default:
            save_metadata_str(font, index, value);
            break;
    }
}

void pcf_check_font_meta(struct font_s *font)
{
    __pcf_check_font_meta(font, METADATA_FONT, NULL);
    __pcf_check_font_meta(font, METADATA_FAMILY_NAME, NULL);
    __pcf_check_font_meta(font, METADATA_FONT_TYPE, "Bitmap");
    __pcf_check_font_meta(font, METADATA_WEIGHT_NAME, "Medium");
    __pcf_check_font_meta(font, METADATA_SLANT, "R");
    __pcf_check_font_meta(font, METADATA_SETWIDTH_NAME, "Normal");
    __pcf_check_font_meta(font, METADATA_SPACING, "C");
    __pcf_check_font_meta(font, METADATA_CHARSET_REGISTRY, "ISO8859");
    __pcf_check_font_meta(font, METADATA_CHARSET_ENCODING, "1");
}

/////////////////////////////////////////////
/////////////////////////////////////////////

struct font_s *pcf_create_empty_font()
{
    struct font_s *font = (struct font_s *)NULL;
    
    /* create our font structure */
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    font->has_metadata = 1;
    font->metadata = (void *)malloc(sizeof(metadata_table));
    if(!font->metadata) goto memory_error;
    /* copy the standard metadata table, to fill it later */
    memcpy(font->metadata, (void *)&metadata_table, sizeof(metadata_table));
    //struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
    if(!create_char_info(font)) goto memory_error;

    font->length = 256;
    font->has_unicode_table = 0;
    font->height   = 8;
    font->width    = 8;
    font->charsize = font->height*2;
    font->version  = get_version("PCF");
    font->data_size = font->length * font->charsize;
    font->data = (unsigned char *)malloc(font->data_size);
    if(!font->data) goto memory_error;
    memset(font->data, 0, font->data_size);
    font->file_hdr = 0;
    font->header_size = 0;
    calc_max_zoom(font);
    font->state = NEW;
    font->module = get_module_by_name("pcf");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;
    
    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error creating new font");
    goto go;

memory_error:

    status_error("Not enough memory to create new font");

go:

    kill_font(font);
    return (struct font_s *)NULL;
}


struct font_s *pcf_load_font_file(char *file_name)
{
    FILE *font_file = (FILE *)NULL;
    char *file_data = (char *)NULL;
    struct font_s *font = (struct font_s *)NULL;
    if(!file_name) return (struct font_s *)NULL;

    if(!(font_file = fopen(file_name, "rb")))
    {
        status_error("Error opening file");
        return (struct font_s *)NULL;
    }
    
    long i;
    i = fseek(font_file, 0, SEEK_END);
    long file_size = ftell(font_file);

    if(!file_size)
    {
        status_error("Error: empty file!");
        fclose(font_file);
        return (struct font_s *)NULL;
    }
    
    rewind(font_file);
    file_data = (char *)malloc(file_size);
    if(!file_data) goto memory_error;
    i = fread(file_data, 1, file_size, font_file);
    if(i != file_size) goto file_read_error;
    fclose(font_file);
    font = pcf_load_font(file_name, (unsigned char *)file_data, file_size);
    free(file_data);
    return font;
    
file_read_error:

    status_error("Error reading font file");
    goto end;

memory_error:

    status_error("Not enough memory to load font file");

end:

    if(file_data) free(file_data);
    fclose(font_file);
    font_file = (FILE *)NULL;
    return (struct font_s *)NULL;
}

int alloc_font_charinfo(struct font_s *font, int fill_with_ff)
{
    struct char_info_s *char_info = font->char_info;

    if(!char_info)
    {
        int len = font->length*sizeof(struct char_info_s);
        char_info = (struct char_info_s *)malloc(len);
        if(!char_info) return 0;
        memset(char_info, 0, len);
        if(fill_with_ff)
        {
            int i;
            for(i = 0; i < (int)font->length; i++)
                char_info[i].encoding = 0xff;
        }
        font->char_info = (void *)char_info;
        font->char_info_size = len;
    }

    return 1;
}

int alloc_font_data(struct font_s *font)
{
    if(font->data) return 1;
    unsigned char *data = (unsigned char *)malloc(font->charsize*font->length);
    if(!data) return 0;
    memset((void *)data, 0, font->charsize*font->length);
    font->data = data;
    font->data_size = font->charsize*font->length;
    if(!alloc_font_charinfo(font, 1)) return 0;
    return 1;
}

struct font_s *pcf_load_font(char *file_name, unsigned char *file_data,
                             long file_size __attribute__((unused)))
{
    (void)file_name;

    struct font_s *font = (struct font_s *)NULL;
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;
    memset((void *)font, 0, sizeof(struct font_s));
    font->has_unicode_table = 0;
    font->has_metadata = 1;
    font->metadata = (void *)malloc(sizeof(metadata_table));
    if(!font->metadata) goto memory_error;
    /* copy the standard metadata table, to fill it later */
    memcpy(font->metadata, (void *)&metadata_table, sizeof(metadata_table));
    font->version = get_version("PCF");

    struct pcf_header *hdr = (struct pcf_header *)file_data;
    struct pcf_toc_entry *toc_entry = 
                (struct pcf_toc_entry *)(file_data+sizeof(struct pcf_header));
    int count = hdr->table_count;
    char *accelerators = NULL;
    char *bitmaps = NULL;
    int i;

    for(i = 0; i < 4; i++)
    {
        if(hdr->header[i] != filesig[i]) goto corrupt_file;
    }

    for(i = 0; i < count; i++)
    {
        int type = pcf_get_lsbint(toc_entry->type);
        int off  = pcf_get_lsbint(toc_entry->offset);
        char *e = (char *)(file_data+off);

        switch(type)
        {
            case PCF_PROPERTIES:
                if(!get_properties_table(e, font)) goto corrupt_file;
                break;

            case PCF_METRICS:
                if(!get_metrics_table(e, font)) goto memory_error;
                break;

            case PCF_INK_METRICS:
                if(!get_ink_metrics_table(e, font)) goto corrupt_file;
                break;

            case PCF_ACCELERATORS:
            case PCF_BDF_ACCELERATORS:
                accelerators = e;
                break;

            case PCF_BITMAPS:
                bitmaps = e;
                break;

            case PCF_BDF_ENCODINGS:
                if(!get_encodings_table(e, font)) goto corrupt_file;
                break;

            case PCF_SWIDTHS:
                if(!get_swidths_table(e, font)) goto corrupt_file;
                break;
        }

        toc_entry++;
    }

    // we need to parse accelerators table AFTER we get the glyph count.
    // this is because we might be having a 'constant metrics' situation.
    if(accelerators)
    {
        if(!get_accel_table(accelerators, font)) goto corrupt_file;
    }

    if(font->width == 0)
    {
        if(!font->char_info) goto corrupt_file;
        struct char_info_s *char_info = font->char_info;
        int maxw = 0;

        for(i = 0; i < (int)font->length; i++)
        {
            if(char_info[i].dwidthX > maxw) maxw = char_info[i].dwidthX;
        }

        if(!maxw) goto corrupt_file;
        font->width = maxw;
        font->charsize = ((font->width+7)/8)*font->height;
    }

    if(!alloc_font_data(font)) goto memory_error;
        
    if(bitmaps)
    {
        if(!get_bitmap_table(bitmaps, font)) goto corrupt_file;
    }
        
    calc_max_zoom(font);
    font->state = OPENED;
    font->module = get_module_by_name("pcf");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;
        
    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error loading font file");
    goto go;

corrupt_file:

    status_error("Invalid or corrupt file");
    goto go;

memory_error:
    status_error("Insufficient memory");

go:

    kill_font(font);
    return (struct font_s *)NULL;
}

int need_swap_bytes(int i)
{
    if((i & PCF_BYTE_MASK) == PCF_BYTE_MASK)
    {
        if(big_endian) return 0;
        return 1;
    }
    if(big_endian) return 1;
    return 0;
}

int need_swap_bits(int i)
{
    if((i & PCF_BIT_MASK) == PCF_BIT_MASK)
    {
        if(big_endian) return 0;
        return 1;
    }
    if(big_endian) return 1;
    return 0;
}

/*
unsigned char swap_bits(unsigned char n)
{
    unsigned int n2 = 0;
    int i = 0, j = 7;
    for( ; j >= 0; j--, i++)
    {
        unsigned int b = (n & 0x01);
        n >>= 1;
        n2 |= (b << j);
    }
    return (unsigned char)n2;
}
*/

int swap_bitsi(int n)
{
    int n2 = 0;
    int i = 0, j = 31;
    for( ; j >= 0; j--, i++)
    {
        int b = (n & 0x01);
        n >>= 1;
        n2 |= (b << j);
    }
    return n2;
}

int table_get_int(int n, int swapbytes, int swapbits)
{
    if(swapbytes)
    {
        if(swapbits)
        {
            int n2 = 0;
            int j = (sizeof(int)*8)-1;
            for( ; j >= 0; j--)
            {
                n2 |= ((n & 0x01) << j);
                n >>= 1;
            }
            return n2;
        }
        else
        {
            return swap_dword(n);
        }
    }
    else
    {
        if(swapbits)
        {
            int i;
            int n2 = 0;
            for(i = 0; i < (int)sizeof(int); i++)
            {
                n2 |= ((int)reverse_char(n & 0xff) << (i*8));
                n >>= 8;
                /*
                char b = (n & 0xff);
                n >>= 8;
                b = swap_bits(b);
                n2 |= ((int)b << (i*8));
                */
            }
            return n2;
        }
        else
        {
            return n;
        }
    }
}


int save_property(struct font_s *font, int index, int value, struct props *p, char *strings)
{
    if(p->isStringProp)
    {
        save_metadata_str(font, index, strings+value);
    }
    else
    {
        struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
        metadata[index].value = value;
    }
    return 1;
}

int get_properties_table(char *table_data, struct font_s *font)
{
    int *st = (int *)table_data;
    int format = pcf_get_lsbint(*st);
    int swapbytes = need_swap_bytes(format);
    int swapbits  = 0; //need_swap_bits (format);
    int nprops = table_get_int(st[1], swapbytes, swapbits);
    // calc the offset to the strings table
    int i = 12 + (sizeof(struct props)*nprops) + ((nprops & 3) == 0 ? 0 : (4-(nprops & 3)));
    char *strings = (table_data+i);
    struct props *p = (struct props *)(&st[2]);
    for(i = 0; i < nprops; i++)
    {
        int offset  = table_get_int(p->name_offset, swapbytes, swapbits);
        int value   = table_get_int(p->value      , swapbytes, swapbits);
        char *s = strings+offset;

        if(strcmp(s, "FONT") == 0)
            save_property(font, METADATA_FONT               , value, p, strings);
        else if(strcmp(s, "COPYRIGHT"          ) == 0)
            save_property(font, METADATA_COPYRIGHT          , value, p, strings);
        else if(strcmp(s, "FONT_VERSION"       ) == 0)
            save_property(font, METADATA_FONT_VERSION       , value, p, strings);
        else if(strcmp(s, "FONT_TYPE"          ) == 0)
            save_property(font, METADATA_FONT_TYPE          , value, p, strings);
        else if(strcmp(s, "FOUNDRY"            ) == 0)
            save_property(font, METADATA_FOUNDRY            , value, p, strings);
        else if(strcmp(s, "FAMILY_NAME"        ) == 0)
            save_property(font, METADATA_FAMILY_NAME        , value, p, strings);
        else if(strcmp(s, "WEIGHT_NAME"        ) == 0)
            save_property(font, METADATA_WEIGHT_NAME        , value, p, strings);
        else if(strcmp(s, "SLANT"              ) == 0)
            save_property(font, METADATA_SLANT              , value, p, strings);
        else if(strcmp(s, "SETWIDTH_NAME"      ) == 0)
            save_property(font, METADATA_SETWIDTH_NAME      , value, p, strings);
        else if(strcmp(s, "PIXEL_SIZE"         ) == 0)
            save_property(font, METADATA_PIXEL_SIZE         , value, p, strings);
        else if(strcmp(s, "POINT_SIZE"         ) == 0)
            save_property(font, METADATA_POINT_SIZE         , value, p, strings);
        else if(strcmp(s, "RESOLUTION_X"       ) == 0)
            save_property(font, METADATA_RESOLUTION_X       , value, p, strings);
        else if(strcmp(s, "RESOLUTION_Y"       ) == 0)
             save_property(font, METADATA_RESOLUTION_Y       , value, p, strings);
        else if(strcmp(s, "SPACING"            ) == 0)
             save_property(font, METADATA_SPACING            , value, p, strings);
        else if(strcmp(s, "AVERAGE_WIDTH"      ) == 0)
             save_property(font, METADATA_AVERAGE_WIDTH      , value, p, strings);
        else if(strcmp(s, "CHARSET_REGISTRY"   ) == 0)
             save_property(font, METADATA_CHARSET_REGISTRY   , value, p, strings);
        else if(strcmp(s, "CHARSET_ENCODING"   ) == 0)
             save_property(font, METADATA_CHARSET_ENCODING   , value, p, strings);
        else if(strcmp(s, "ADD_STYLE_NAME"     ) == 0)
             save_property(font, METADATA_ADD_STYLE_NAME     , value, p, strings);
        else if(strcmp(s, "UNDERLINE_POSITION" ) == 0)
             save_property(font, METADATA_UNDERLINE_POSITION , value, p, strings);
        else if(strcmp(s, "UNDERLINE_THICKNESS") == 0)
             save_property(font, METADATA_UNDERLINE_THICKNESS, value, p, strings);
        else if(strcmp(s, "CAP_HEIGHT"         ) == 0)
             save_property(font, METADATA_CAP_HEIGHT         , value, p, strings);
        else if(strcmp(s, "X_HEIGHT"           ) == 0)
             save_property(font, METADATA_X_HEIGHT           , value, p, strings);
        else if(strcmp(s, "FONT_ASCENT"        ) == 0)
             save_property(font, METADATA_FONT_ASCENT        , value, p, strings);
        else if(strcmp(s, "FONT_DESCENT"       ) == 0)
             save_property(font, METADATA_FONT_DESCENT       , value, p, strings);
        else if(strcmp(s, "DEFAULT_CHAR"       ) == 0)
             save_property(font, METADATA_DEFAULT_CHAR       , value, p, strings);
        p++;
    }

    return 1;
}

int get_metrics_table(char *table_data, struct font_s *font)
{
    int *st = (int *)table_data;
    int format = pcf_get_lsbint(*st);
    int swapbytes = need_swap_bytes(format);
    int swapbits  = 0; //need_swap_bits (format);

    if(format & PCF_COMPRESSED_METRICS)
    {
        unsigned int i = *(unsigned short *)(&table_data[4]);
        if(swapbytes) i = swap_word(i);
        font->length = i;
        // ensure data and charinfo structures are allocated
        if(!alloc_font_charinfo(font, 0)) return 0;
        struct compressed_metrics *metric = (struct compressed_metrics *)(table_data+6);
        struct char_info_s *ci = font->char_info;

        while(i-- > 0)
        {
            ci->lBearing    = metric->left_side_bearing-0x80;
            ci->rBearing    = metric->right_side_bearing-0x80;
            ci->dwidthX     = metric->character_width-0x80;
            ci->dwidthY     = 0;
            ci->charAscent  = metric->character_ascent-0x80;
            ci->charDescent = metric->character_descent-0x80;
            ci++;
            metric++;
        }

        return 1;
    }
    else
    {
        int i = table_get_int(st[1], swapbytes, swapbits);
        font->length = i;
        // ensure data and charinfo structures are allocated
        if(!alloc_font_charinfo(font, 0)) return 0;
        struct uncompressed_metrics *metric = (struct uncompressed_metrics *)(table_data+8);
        struct char_info_s *ci = font->char_info;

        while(i--)
        {
            ci->lBearing    = metric->left_side_bearing;
            ci->rBearing    = metric->right_side_bearing;
            ci->dwidthX     = metric->character_width;
            ci->dwidthY     = 0;
            ci->charAscent  = metric->character_ascent;
            ci->charDescent = metric->character_descent;
            ci++;
            metric++;
        }

        return 1;
    }

    return 0;
}

int get_ink_metrics_table(char *table_data, struct font_s *font)
{
    int *st = (int *)table_data;
    int format = pcf_get_lsbint(*st);
    int swapbytes = need_swap_bytes(format);
    int swapbits  = 0; //need_swap_bits (format);

    if(format & PCF_COMPRESSED_METRICS)
    {
        unsigned int i = *(unsigned short *)(&table_data[4]);
        if(swapbytes) i = swap_word(i);
        font->length = i;
        // ensure data and charinfo structures are allocated
        if(!alloc_font_charinfo(font, 0)) return 0;
        struct compressed_metrics *metric = (struct compressed_metrics *)(table_data+6);
                struct char_info_s *ci = font->char_info;

        while(i-- > 0)
        {
            ci->BBXoff = metric->left_side_bearing-0x80;
            ci->BBYoff = metric->right_side_bearing-0x80;
            ci->BBw    = metric->character_width-0x80;
            ci->BBh    = metric->character_ascent-0x80;
            ci++;
            metric++;
        }
        return 1;
    }
    else
    {
        int i = table_get_int(st[1], swapbytes, swapbits);
        font->length = i;
        // ensure data and charinfo structures are allocated
        if(!alloc_font_charinfo(font, 0)) return 0;
        struct uncompressed_metrics *metric = (struct uncompressed_metrics *)(table_data+8);
                struct char_info_s *ci = font->char_info;

        while(i--)
        {
            ci->BBXoff = metric->left_side_bearing;
            ci->BBYoff = metric->right_side_bearing;
            ci->BBw    = metric->character_width;
            ci->BBh    = metric->character_ascent;
            ci++;
            metric++;
        }

        return 1;
    }

    return 0;
}

int get_accel_table(char *table_data, struct font_s *font)
{
    struct accel_table *acc = (struct accel_table *)table_data;
    struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
    int *st = (int *)table_data;
    int format = pcf_get_lsbint(*st);
    int swapbytes = need_swap_bytes(format);
    int swapbits  = 0; //need_swap_bits (format);
    int ascent = table_get_int(acc->font_ascent, swapbytes, swapbits);
    metadata[METADATA_FONT_ASCENT].value = ascent;
    int descent = table_get_int(acc->font_descent, swapbytes, swapbits);
    metadata[METADATA_FONT_DESCENT].value = descent;
    int i;

    metadata[METADATA_FONTBOUNDINGBOX_X].value = acc->minbounds.left_side_bearing;
    metadata[METADATA_FONTBOUNDINGBOX_Y].value = acc->minbounds.right_side_bearing;
    metadata[METADATA_FONTBOUNDINGBOX_XOFF].value = acc->minbounds.character_width;
    metadata[METADATA_FONTBOUNDINGBOX_YOFF].value = acc->minbounds.character_ascent;

    if(acc->const_metrics)
    {
        if(!alloc_font_charinfo(font, 0)) return 0;
        struct char_info_s *char_info = font->char_info;
        int maxw = 0;

        for(i = 0; i < (int)font->length; i++)
        {
            char_info[i].lBearing = 0;
            char_info[i].rBearing = char_info[i].dwidthX;
            if(char_info[i].dwidthX > maxw) maxw = char_info[i].dwidthX;
            char_info[i].charAscent = ascent;
            char_info[i].charDescent = descent;
        }
        if(font->width == 0) font->width = maxw;
    }

    if(font->height == 0) font->height = ascent+descent;
    font->charsize = font->height*((font->width+7)/8);
    return 1;
}


int get_bitmap_table(char *table_data, struct font_s *font)
{
    if(!font->char_info) return 0;
    struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
    struct char_info_s *char_info = font->char_info;
    unsigned char *data = font->data;
    u_int32_t *st = (u_int32_t *)table_data;
    int format = pcf_get_lsbint(*st);
    int swapbytes = need_swap_bytes(format);
    int swapbits = !need_swap_bits(format);     // only swap if LSB first as we
                                                // need the leftmost pixel in
                                                // the MSB position
    int bcount = table_get_int(st[1], swapbytes, 0);
    font->length = bcount;
    u_int32_t *offsets = &st[2];
    u_int32_t *sizes = &st[2+bcount];
    unsigned char *bdata = (unsigned char *)(&st[6+bcount]);
    int line_bytes = (format >> 4) & 3;
    int padding = format & 3;
    int pad = 0;

    switch(padding)
    {
        case 0: pad = 1; break;
        case 1: pad = 2; break;
        case 2: pad = 4; break;
    }

    // check we have valid data length
    if(bcount < 0) return 0; 
    
    // check we have valid font height & width
    if(!font->charsize)
    {
        if(!metadata) return 0;
        font->height = metadata[METADATA_FONT_ASCENT ].value +
                       metadata[METADATA_FONT_DESCENT].value;
        font->width  = metadata[METADATA_AVERAGE_WIDTH].value;
        font->charsize = font->height*((font->width+7)/8);
        // still not getting valid width & height?
        if(!font->charsize) return 0;
    }
    if(!alloc_font_data(font)) return 0;

//restoreTerminal();
//printf("swapbytes %d, swapbits %d\n", swapbytes, swapbits);

    int j;
    int size = table_get_int(sizes[padding], swapbytes, 0);
    for(j = 0; j < bcount; j++)
    {
        u_int32_t off = (u_int32_t)table_get_int(offsets[j], swapbytes, 0);
        u_int32_t sz = (u_int32_t)size;
        if(j == bcount-1)
        {
            sz = sz-off;
        }
        else
        {
            u_int32_t off2 = (u_int32_t)table_get_int(offsets[j+1], swapbytes, 0);
            sz = off2-off;
        }

        int gw = (font->width+7)/8;
        int w = char_info[j].dwidthX;
        //int h = char_info[j].charAscent+char_info[j].charDescent;
        w = ((w+7)/8);
        unsigned char *d = &bdata[off];
        unsigned int k = 0;

        while(k < sz)
        {
            unsigned int l = 0;
            if(line_bytes == 0)
            {
                int m = w;
                int shift = 0;
                while(m--)
                {
                    unsigned int l2 = (swapbits ? reverse_char(*d) : *d) & 0xff;
                    //if(j == 0x24) printf("%x,", l2);
                    if(swapbytes) l = (l << 8) | l2;
                    else { l = l | (l2 << shift); shift += 8; }
                    d++;
                    k++;
                }
                //if(j == 0x24) printf(" [%x]\n", l);
            }    /* bytes */
            else if(line_bytes == 1)
            {
                l = get_ushort(d, swapbits);
                d += 2; k += 2;
                if(swapbytes) l = swap_word(l);
            }    /* shorts */
            else if(line_bytes == 2)
            {
                l = get_int(d, swapbits);
                d += 4; k += 4;
                if(swapbytes) l = swap_word(l);
            }    /* ints */
                else goto corrupt_file;
            if(pad)
            {
                int diff = pad-(w%pad);
                d += diff;
                k += diff;
            }
            unsigned int l2 = l;
            int w2 = 0;
            while(w2++ < w)
            {
                //*data = reverse_char(l2 & 0xff);
                *data = (unsigned char)(l2 & 0xff);
                l2 >>= 8;
                data++;
            }
            while(w2++ <= gw) data++;
        }
    }

//exit(0);

    return 1;

corrupt_file:

    return 0;
}

int get_encodings_table(char *table_data, struct font_s *font)
{
    short *st = (short *)table_data;
    int format = pcf_get_lsbint(*(int *)table_data);
    int swapbytes = need_swap_bytes(format);
    //int swapbits  = 0; //need_swap_bits (format);
    int min_char_or_byte2 = st[2];
    int max_char_or_byte2 = st[3];
    int min_byte1 = st[4];
    int max_byte1 = st[5];
    int default_char = st[6] & 0xffff;
    short *glyphindex = &st[7];

    if(swapbytes)
    {
        min_char_or_byte2 = swap_word(min_char_or_byte2);
        max_char_or_byte2 = swap_word(max_char_or_byte2);
        min_byte1 = swap_word(min_byte1);
        max_byte1 = swap_word(max_byte1);
        default_char = swap_word(default_char & 0xffff);
    }

    // make sure we have a charinfo struct
    if(!alloc_font_charinfo(font, 1)) return 0;
    struct char_info_s *char_info = font->char_info;
    struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
    metadata[METADATA_DEFAULT_CHAR].value = default_char;

    int ilen = (max_char_or_byte2-min_char_or_byte2+1)*(max_byte1-min_byte1+1);
    int i, j;

    if(!min_byte1 && !max_byte1)    // single byte encodings
    {
        for(i = 0; i < ilen; i++)
        {
            j = glyphindex[i];
            if(swapbytes) j = swap_word(j & 0xffff);
                        // 0xffff means no glyph for that encoding
            if(j != 0xffff) char_info[j].encoding = i;
        }
    }
    else                // double byte encoding
    {
        min_byte1 &= 0xff;
        min_char_or_byte2 &= 0xff;
        int min = (min_byte1 << 8) | (min_char_or_byte2);

        for(i = 0; i < ilen; i++)
        {
            j = glyphindex[i];
            if(swapbytes) j = swap_word(j & 0xffff);
                        // 0xffff means no glyph for that encoding
            if(j != 0xffff) char_info[j].encoding = min;
            min++;
        }
    }

    return 1;
}

int get_swidths_table(char *table_data, struct font_s *font)
{
    int *st = (int *)table_data;
    int format = pcf_get_lsbint(*st);
    int swapbytes = need_swap_bytes(format);
    //int swapbits  = 0; //need_swap_bits (format);
    int count = table_get_int(st[1], swapbytes, 0);

    font->length = count;
    // make sure we have a charinfo struct
    if(!alloc_font_charinfo(font, 0)) return 0;
    struct char_info_s *char_info = font->char_info;

    int *swidths = &st[2];
    int i, j;

    for(i = 0; i < count; i++)
    {
        j = table_get_int(swidths[i], swapbytes, 0);
        char_info[i].swidthX = j;
        char_info[i].swidthY = 0;
    }

    return 1;
}

int file_write_lsbint(u_int32_t n, FILE *file)
{
    int i;

    if(big_endian)
    {
        n = swap_dword(n);
    }

    for(i = 0; i < 4; i++)
    {
        putc((n & 0xff), file);
        n >>= 8;
    }

    return 0;
}

/*
 * Similar to the above function, except it reverses every char's bits.
 */
int file_write_lsbintr(u_int32_t n, FILE *file)
{
    int i;

    if(big_endian)
    {
        n = swap_dword(n);
    }

    for(i = 0; i < 4; i++)
    {
        putc(reverse_char(n & 0xff), file);
        n >>= 8;
    }

    return 0;
}

int file_write_lsbshort(u_int16_t n, FILE *file)
{
    int i;
    unsigned char c;

    if(big_endian)
    {
        n = swap_word(n);
    }

    for(i = 0; i < 2; i++)
    {
        c = n & 0xff;
        putc(c, file);
        n >>= 8;
    }

    return 0;
}

/*
 * l = how many bits to write from n (this will be converted to bytes).
 *     the rest of the 4 bytes are written as zeroes.
 */
int file_write_msbint(u_int32_t n, FILE *file, int l)
{
    unsigned char *c = (unsigned char *)&n;
    int i;
    l = (l+7)/8;

    if(big_endian)
    {
        for(i = 0; i < l; i++) putc(c[i], file);
        for( ; i < 4; i++) putc(0, file);
    }
    else
    {
        for(i = l-1; i >= 0; i--)
        {
            //unsigned char c2 = swap_bits(c[i]);
            //putc(c2, file);
            putc(reverse_char(c[i]), file);
        }
        for(i = 0; i < 4-l; i++) putc(0, file);
    }

    return 0;
}

int file_write_uncompressed_metrics(struct uncompressed_metrics *metrics, FILE *file)
{
    int res;
    res = file_write_lsbshort(metrics->left_side_bearing   , file);
    res = file_write_lsbshort(metrics->right_side_bearing  , file);
    res = file_write_lsbshort(metrics->character_width     , file);
    res = file_write_lsbshort(metrics->character_ascent    , file);
    res = file_write_lsbshort(metrics->character_descent   , file);
    res = file_write_lsbshort(metrics->character_attributes, file);
    return res;
}

int file_write_compressed_metrics(struct compressed_metrics *metrics, FILE *file)
{
    putc(metrics->left_side_bearing + 0x80, file);
    putc(metrics->right_side_bearing+ 0x80, file);
    putc(metrics->character_width   + 0x80, file);
    putc(metrics->character_ascent  + 0x80, file);
    putc(metrics->character_descent + 0x80, file);
    return 0;
}

int pcf_write_to_file(FILE *file, struct font_s *font)
{
    if(!file || !font) return 1;
    int32_t res, i, j;
    u_int32_t format = PCF_DEFAULT_FORMAT;
    struct metadata_item_s *meta = (struct metadata_item_s *)font->metadata;
    if(!font->char_info)
    {
        if(!create_char_info(font)) return 1;
    }
    pcf_check_font_meta(font);
    struct char_info_s *char_info = font->char_info;
    unsigned char *data = font->data;
    u_int32_t tabcount = 7;
    //struct pcf_toc_entry tables[tabcount];
    struct pcf_toc_entry *tables = malloc(tabcount*sizeof(struct pcf_toc_entry));
    if(!tables) return 1;
  
    // 0 - write the file header
    res = fwrite(filesig, 1, 4, file);
    res = file_write_lsbint(tabcount, file);

    // 1 - the properties table
    tables[0].type = PCF_PROPERTIES;
    tables[0].format = format;
    
    // calculate table offset in file
    i = (sizeof(struct pcf_toc_entry)*tabcount) + 8;
    tables[0].offset = i;
  
    // calculate table size
    u_int32_t string_size = 0;
    u_int32_t nprops = 0;

    for(i = 0; i < metadata_table_len; i++)
    {
        if(meta[i].is_str)
        {
            if(meta[i].value2)
            {
                string_size += strlen(meta[i].name)+1;
                string_size += strlen(meta[i].value2)+1;
                nprops++;
            }
        }
        else
        {
            //if(meta[i].value)
            //{
                string_size += strlen(meta[i].name)+1;
                nprops++;
            //}
        }
    }

    // calculate padding
    int pad1 = (nprops & 3) == 0 ? 0 : (4 - (nprops & 3));
    tables[0].size = 12+(9*nprops)+pad1+string_size;

    // calculate padding2
    int pad2 = (tables[0].size & 3) == 0 ? 0 : (4 - (tables[0].size & 3));
    tables[0].size += pad2;

    // 2 - the accelerators table
    tables[1].type   = PCF_ACCELERATORS;
    tables[1].format = format;
    tables[1].offset = tables[0].offset + tables[0].size;
    tables[1].size   = sizeof(struct accel_table);

    // 3 - the metrics table
    tables[2].type   = PCF_METRICS;
    tables[2].format = format;
    tables[2].offset = tables[1].offset + tables[1].size;
    tables[2].size   = sizeof(struct uncompressed_metrics) * font->length + 8;

    // 4 - the bitmaps table
    tables[3].type   = PCF_BITMAPS;
    tables[3].format = format | 2;  // 2 = rows padded to ints
    tables[3].offset = tables[2].offset + tables[2].size;
    tables[3].size   = 4*font->length + 24;

    // calculate storage size per scanline, padded to 4 bytes
    /*
     * TODO: this calculation assumes font width is <= 32 bits.
     *       we should test for the cases where width takes > 4 bytes.
     */
    int bitmap_length = 4*font->height*font->length;
    tables[3].size += bitmap_length;
    // calculate padding
    i = (tables[3].size & 3) == 0 ? 0 : (4 - (tables[3].size & 3));
    tables[3].size += i;

    // 5 - the ink metrics table -- same as the metrics table
    tables[4].type   = PCF_INK_METRICS;
    tables[4].format = tables[2].format;
    tables[4].offset = tables[3].offset + tables[3].size;
    tables[4].size   = tables[2].size;

    // 6 - the encodings table
    tables[5].type   = PCF_BDF_ENCODINGS;
    tables[5].format = format;
    tables[5].offset = tables[4].offset + tables[4].size;
    // get the minimum and maximum encodings
    int min = 0, max = 0;
    for(i = 0; i < (int)font->length; i++)
    {
        /*
         * TODO: This is an awful hack. Sometimes the font contains out-of-range
         *       encodings (e.g. when shrinking a lengthier font, or changing 
         *       format from BDF to PCF, etc). We solve this by truncating the
         *       encoding value to the max (otherwise we will write outside the
         *       encodings array boundaries and will probably SIGSEGV or SIGABRT).
         *       This needs to be FIXED!
         */
        if(char_info[i].encoding >= (int)font->length) 
            char_info[i].encoding = font->length-1;

        j = char_info[i].encoding;
        if(j < min) min = j;
        else if(j > max) max = j;
    }

    int max_char_or_byte2 = (max >> 8) & 0xff;
    int min_char_or_byte2 = (min >> 8) & 0xff;
    int max_byte1 = (max & 0xff);
    int min_byte1 = (min & 0xff);

    if(!big_endian)
    {
        j = max_char_or_byte2;
        max_char_or_byte2 = max_byte1;
        max_byte1 = j;
        j = min_char_or_byte2;
        min_char_or_byte2 = min_byte1;
        min_byte1 = j;
    }

    int enc_len = sizeof(u_int16_t)*
                    ((max_char_or_byte2-min_char_or_byte2+1)*
                        (max_byte1-min_byte1+1));
    tables[5].size   = enc_len + 14;

    // 7 - the scalable widths table
    tables[6].type   = PCF_SWIDTHS;
    tables[6].format = format;
    tables[6].offset = tables[5].offset + tables[5].size;
    tables[6].size   = 4*font->length + 8;

    // fix the numbers to be LSB
    for(i = 0; i < (int)tabcount; i++)
    {
        file_write_lsbint(tables[i].type  , file);
        file_write_lsbint(tables[i].format, file);
        file_write_lsbint(tables[i].size  , file);
        file_write_lsbint(tables[i].offset, file);
    }
    
    // now write the tables to file
    // write the properties table
    res = file_write_lsbint(tables[0].format, file);
    res = file_write_lsbint(nprops, file);
    //char strings[string_size];
    char *strings = (char *)malloc(string_size);
    if(!strings) return 1;
    char *sp = strings;

    for(i = 0; i < metadata_table_len; i++)
    {
        if(meta[i].is_str)
        {
            if(meta[i].value2)
            {
                j = sp-strings;
                res = file_write_lsbint((u_int32_t)j, file);
                strcpy(sp, meta[i].name);
                sp += strlen(meta[i].name)+1;
                putc(1, file);
                j = sp-strings;
                res = file_write_lsbint((u_int32_t)j, file);
                strcpy(sp, meta[i].value2);
                sp += strlen(meta[i].value2)+1;
            }
        }
        else
        {
            //if(meta[i].value)
            //{
                j = sp-strings;
                res = file_write_lsbint((u_int32_t)j, file);
                strcpy(sp, meta[i].name);
                sp += strlen(meta[i].name)+1;
                putc(0, file);
                j = meta[i].value;
                res = file_write_lsbint((u_int32_t)j, file);
            //}
        }
    }

    // write padding
    while(pad1-- > 0) putc(0, file);
    res = file_write_lsbint(string_size, file);
    res = fwrite(strings, 1, string_size, file);
    // write padding2
    while(pad2-- > 0) putc(0, file);

    // write the accelerators table
    res = file_write_lsbint(tables[1].format, file);
    putc(0, file);
    putc(1, file);
    putc(1, file);
    putc(1, file);
    putc(0, file);
    putc(0, file);
    putc(0, file);
    putc(0, file);
    res = file_write_lsbint(meta[METADATA_FONT_ASCENT].value, file);
    res = file_write_lsbint(meta[METADATA_FONT_DESCENT].value, file);
    j = 0;
    res = file_write_lsbint(j, file);
    struct uncompressed_metrics bounds = 
    { 
        meta[METADATA_FONTBOUNDINGBOX_X].value,
        meta[METADATA_FONTBOUNDINGBOX_Y].value,
        meta[METADATA_FONTBOUNDINGBOX_XOFF].value,
        meta[METADATA_FONTBOUNDINGBOX_YOFF].value,
        0, 0
    };
    res = file_write_uncompressed_metrics(&bounds, file);
    res = file_write_uncompressed_metrics(&bounds, file);

    // write the metrics table
    res = file_write_lsbint(tables[2].format, file);
    j = font->length;
    res = file_write_lsbint(j, file);
    for(i = 0; i < (int)font->length; i++)
    {
        bounds = (struct uncompressed_metrics)
                { 
                    char_info[i].lBearing, char_info[i].rBearing, char_info[i].dwidthX,
                    char_info[i].charAscent, char_info[i].charDescent, 0
                };
        res = file_write_uncompressed_metrics(&bounds, file);
    }

    // write the bitmaps table
    res = file_write_lsbint(tables[3].format, file);
    j = font->length;
    res = file_write_lsbint(j, file);
    j = 0;

    // write the offsets into bitmap data
    for(i = 0; i < (int)font->length; i++)
    {
        res = file_write_lsbint(j, file);
        j += (font->height << 2);
    }

    // now write the bitmap sizes array
    j = 0;
    res = file_write_lsbint(j, file);
    res = file_write_lsbint(j, file);
    res = file_write_lsbint(bitmap_length, file);
    res = file_write_lsbint(j, file);

//restoreTerminal();
    int m = (font->width+7)/8;
    for(i = 0; i < (int)font->length; i++)
    {
        for(j = 0; j < (int)font->height; j++)
        {
            unsigned int line = 0;
            int l;
            for(l = 0; l < m; l++)
            {
                line = (line) | (unsigned int)data[l] << (l*8);
            }
            data += l;
            /*
            l = char_info[i].dwidthX;
            if(l == 0) l = font->width;
            res = file_write_msbint(line, file, l);
            */
            res = file_write_lsbintr(line, file);
            //if(i == 0x24) printf("[%d] %x", j, line);
        }
    }
//exit(0);

    // write the ink metrics table
    // NOTE: this is the same code as for the metrics table above
    res = file_write_lsbint(tables[4].format, file);
    j = font->length;
    res = file_write_lsbint(j, file);
    for(i = 0; i < (int)font->length; i++)
    {
        bounds = (struct uncompressed_metrics)
            { 
                char_info[i].lBearing, char_info[i].rBearing, char_info[i].dwidthX,
                char_info[i].charAscent, char_info[i].charDescent, 0
            };
        res = file_write_uncompressed_metrics(&bounds, file);
    }

    // write the encodings table
    res = file_write_lsbint(tables[5].format, file);
    u_int16_t k;
    // do we need one byte or two bytes for the encodings?
    if(max < 256)
    {
        k = min & 0xffff;
        res = file_write_lsbshort(k, file);
        k = max & 0xffff;
        res = file_write_lsbshort(k, file);
        k = 0;
        res = file_write_lsbshort(k, file);
        res = file_write_lsbshort(k, file);
    }
    else
    {
        i = min;
        j = max;
        if(big_endian)
        {
            k = (i >> 8) & 0xff;
            res = file_write_lsbshort(k, file);
            k = (j >> 8) & 0xff;
            res = file_write_lsbshort(k, file);
            k = i & 0xff;
            res = file_write_lsbshort(k, file);
            k = j & 0xff;
            res = file_write_lsbshort(k, file);
        }
        else
        {
            k = i & 0xff;
            res = file_write_lsbshort(k, file);
            k = j & 0xff;
            res = file_write_lsbshort(k, file);
            k = (i >> 8) & 0xff;
            res = file_write_lsbshort(k, file);
            k = (j >> 8) & 0xff;
            res = file_write_lsbshort(k, file);
        }
    }
    k = meta[METADATA_DEFAULT_CHAR].value;
    res = file_write_lsbshort(k, file);
    
    // assemble the encodings table
    u_int16_t *encodings = (u_int16_t *)malloc(enc_len);
    if(!encodings) return 1;
    memset(encodings, 0xff, enc_len);
    for(i = 0; i < (int)font->length; i++)
    {
        int enc1 = (char_info[i].encoding >> 8) & 0xff;
        int enc2 = char_info[i].encoding & 0xff;
        if(big_endian)
        {
            j = enc1;
            enc1 = enc2;
            enc2 = j;
        }
        j = (enc1-min_byte1)*(max_char_or_byte2-min_char_or_byte2+1)+enc2-min_char_or_byte2;
        encodings[j] = i;
        //printf("j = %d, i = %d, enc_len/2 = %d, min = %d, max = %d\n", j, i, enc_len/2, min, max);
    }
    res = fwrite(encodings, 1, enc_len, file);

    // write the scalable widths table
    res = file_write_lsbint(tables[6].format, file);
    j = font->length;
    res = file_write_lsbint(j, file);
    for(i = 0; i < (int)font->length; i++)
    {
        if(char_info[i].swidthX == 0) char_info[i].swidthX = DEFAULT_SWIDTH;
        res = file_write_lsbint(char_info[i].swidthX, file);
    }

    free(strings);
    free(tables);
    free(encodings);
    if(res == EOF) return 1;
    return 0;
}


void pcf_handle_hw_change(struct font_s *font, char *newdata, long new_datasize)
{
    bdf_handle_hw_change(font, newdata, new_datasize);
}

/*
int pcf_create_unitab(struct font_s *font)
{
    return bdf_create_unitab(font);
}

void pcf_kill_unitab(struct font_s *font)
{
    bdf_kill_unitab(font);
}

void pcf_handle_unicode_table_change(struct font_s *font, char old_has_unicode_table)
{
    bdf_handle_unicode_table_change(font, old_has_unicode_table);
}
*/


void pcf_handle_version_change(struct font_s *font, char old_version)
{
    bdf_handle_version_change(font, old_version);
}


void pcf_convert_to_psf(struct font_s *font)
{
    bdf_convert_to_psf(font);
}

void pcf_shrink_glyphs(struct font_s *font, int old_length)
{
    psf_shrink_glyphs(font, old_length);
}

void pcf_expand_glyphs(struct font_s *font, int old_length, int option)
{
    psf_expand_glyphs(font, old_length, option);
}

long pcf_make_utf16_unitab(struct font_s *new_font, unsigned short **_unicode_table)
{
    return psf_make_utf16_unitab(new_font, _unicode_table);
}

int pcf_is_acceptable_width(struct font_s *font)
{
    return (font->width >= 4 && font->width <= 32);
}

int pcf_next_acceptable_width(struct font_s *font)
{
    if(font->width < 32) return font->width + 1;
    else return 4;
}

int pcf_is_acceptable_height(struct font_s *font)
{
    return (font->height >= 4 && font->height <= 32);
}

int pcf_next_acceptable_height(struct font_s *font)
{
    if(font->height < 32) return font->height + 1;
    else return 4;
}

/********************************
 * ******************************
 * ******************************/
struct module_s pcf_module;

struct file_sig_s pcf_sig = { 0, 4, { 1, 'f', 'c', 'p' }, NULL, NULL, NULL };

void pcf_init_module()
{
    strcpy(pcf_module.mod_name, "pcf");
    pcf_module.max_width = 32;
    pcf_module.max_height = 32;
    pcf_module.max_length = 512;
    pcf_module.create_empty_font = pcf_create_empty_font;
    pcf_module.write_to_file = pcf_write_to_file;
    pcf_module.load_font = pcf_load_font;
    pcf_module.load_font_file = pcf_load_font_file;
    pcf_module.handle_hw_change = pcf_handle_hw_change;
    pcf_module.shrink_glyphs = pcf_shrink_glyphs;
    pcf_module.expand_glyphs = pcf_expand_glyphs;
    pcf_module.update_font_hdr = NULL;
    pcf_module.handle_version_change = pcf_handle_version_change;
    pcf_module.handle_unicode_table_change = NULL; //pcf_handle_unicode_table_change;
    pcf_module.export_unitab = NULL; //pcf_export_unitab;
    //pcf_module.create_unitab = NULL; //pcf_create_unitab;
    //pcf_module.kill_unitab = NULL; //pcf_kill_unitab;
    pcf_module.convert_to_psf = pcf_convert_to_psf;
    pcf_module.is_acceptable_width = pcf_is_acceptable_width;
    pcf_module.next_acceptable_width = pcf_next_acceptable_width;
    pcf_module.is_acceptable_height = pcf_is_acceptable_height;
    pcf_module.next_acceptable_height = pcf_next_acceptable_height;
    register_module(&pcf_module);
    add_file_extension("pcf", "pcf");
    add_file_signature(&pcf_sig, "pcf");
}


/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: bdf.c
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
#include "bdf.h"

struct bdf_keyword_s bdf_keywords[] =
{
  /* Global keywords */
  { "STARTFONT",          BDF_REQ_YES, BDF_GLOBAL_SCOPE, 1, { NUMBER  } },
  { "COMMENT",            BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "CONTENTVERSION",     BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "FONT",               BDF_REQ_YES, BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "SIZE",               BDF_REQ_YES, BDF_GLOBAL_SCOPE, 3, { NUMBER, NUMBER, NUMBER } },
  { "FONTBOUNDINGBOX",    BDF_REQ_YES, BDF_GLOBAL_SCOPE, 4, { INTEGER, INTEGER, INTEGER, INTEGER } },
  { "METRICSSET",         BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "SWIDTH",             BDF_REQ_NO,  BDF_ANY_SCOPE,    2, { NUMBER, NUMBER } },
  { "DWIDTH",             BDF_REQ_NO,  BDF_ANY_SCOPE,    2, { NUMBER, NUMBER } },
  { "SWIDTH1",            BDF_REQ_NO,  BDF_ANY_SCOPE,    2, { NUMBER, NUMBER } },
  { "DWIDTH1",            BDF_REQ_NO,  BDF_ANY_SCOPE,    2, { NUMBER, NUMBER } },
  { "VVECTOR",            BDF_REQ_NO,  BDF_ANY_SCOPE,    2, { NUMBER, NUMBER } },
  { "STARTPROPERTIES",    BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "ENDPROPERTIES",      BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 0, { 0 } },
  { "CHARS",              BDF_REQ_YES, BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "ENDFONT",            BDF_REQ_YES, BDF_GLOBAL_SCOPE, 0, { 0 } },
  { "COPYRIGHT",          BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "FOUNDRY",            BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "FAMILY_NAME",        BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "WEIGHT_NAME",        BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "SLANT",              BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "SETWIDTH_NAME",      BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "FONT_VERSION",       BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "FONT_TYPE",          BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "PIXEL_SIZE",         BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "POINT_SIZE",         BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "RESOLUTION_X",       BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "RESOLUTION_Y",       BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "SPACING",            BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "AVERAGE_WIDTH",      BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "CHARSET_REGISTRY",   BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "CHARSET_ENCODING",   BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  { "UNDERLINE_POSITION", BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "UNDERLINE_THICKNESS",BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "CAP_HEIGHT",         BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "X_HEIGHT",           BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "FONT_ASCENT",        BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "FONT_DESCENT",       BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "DEFAULT_CHAR",       BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { INTEGER } },
  { "ADD_STYLE_NAME",     BDF_REQ_NO,  BDF_GLOBAL_SCOPE, 1, { STRING  } },
  /* Glyph keywords */
  { "STARTCHAR",          BDF_REQ_YES, BDF_GLYPH_SCOPE,  1, { STRING  } },
  { "ENCODING",           BDF_REQ_YES, BDF_GLYPH_SCOPE,  1, { INTEGER } },
  { "BBX",                BDF_REQ_YES, BDF_GLYPH_SCOPE,  4, { INTEGER, INTEGER, INTEGER, INTEGER } },
  { "BITMAP",             BDF_REQ_YES, BDF_GLYPH_SCOPE,  0, { 0 } },
  { "ENDCHAR",            BDF_REQ_YES, BDF_GLYPH_SCOPE,  0, { 0 } },
};

/* we will use this bitmap to check that all the required words
 * were found in the file, otherwise the file is considered corrupt.
 */
unsigned long long keyword_bitmap;

/* Helper function: skip all spaces to the next non-space char */
static inline void skip_spaces(char **s)
{
    char *s2 = *s;
    while(*s2 && *s2 == ' ') s2++;
    *s = s2;
}

char buf[512];

/* Helper function: get the next keyword */
static inline char *get_keyword(char **s)
{
    int counter = 0;
    char *s2 = *s;

    if(*s2 == '\"')
    {
        s2++;
        while(*s2 && *s2 != '\"') { buf[counter++] = *s2++; }
        s2++;
    }
    else
    {
        while(*s2 && *s2 != ' ') { buf[counter++] = *s2++; }
    }

    buf[counter] = '\0';
    *s = s2;

    return buf;
}

/* Helper function: get keyword index from the array */
static inline char get_keyword_index(char *keyword)
{
    int i;
    for(i = 0; i < BDF_TOTAL_KEYWORDS; i++)
    {
        if(strcmp(keyword, bdf_keywords[i].name) == 0)
        {
            //printf("%s\n", keyword);
            return i;
        }
    }

    return i;
}

/* Helper function: get keyword argument of type STRING */
static inline char *get_string(char **s)
{
    skip_spaces(s);
    return get_keyword(s);
}

/* Helper function: get keyword argument of type INTEGER */
static inline int get_integer(char **s)
{
    skip_spaces(s);
    return atoi(get_keyword(s));
}

/* Helper function: get keyword argument of type NUMBER */
static inline int get_number(char **s)
{
    skip_spaces(s);
    return atof(get_keyword(s));
}

/* Helper function: get hex number, used in parsing BITMAP structure */
static inline unsigned int get_hex(char *s, int nibbles)
{
    unsigned int res = 0;
    while(nibbles--)
    {
        unsigned char h = 0;
        if(*s >= 'a' && *s <= 'f') h = *s-'a'+10;
        else if(*s >= 'A' && *s <= 'F') h = *s-'A'+10;
        else if(*s >= '0' && *s <= '9') h = *s-'0';
        else break;
        res = (res << 4) | h;
        s++;
    }

    return res;
}

static inline int ishexdigit(char c)
{
    if(c >= 'a' && c <= 'f') return 1;
    if(c >= 'A' && c <= 'F') return 1;
    if(c >= '0' && c <= '9') return 1;
    return 0;
}


extern unsigned short default_unicode_table[]; /* psf.c */


struct font_s *bdf_create_empty_font()
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
    font->has_unicode_table = 1;
    font->utf_version = VER_PSF1;
    font->height   = 16;
    font->width    = 16;
    font->charsize = font->height*2;
    font->version  = get_version("BDF");
    font->data_size = font->length * font->charsize;
    font->data = (unsigned char *)malloc(font->data_size);
    if(!font->data) goto memory_error;
    memset(font->data, 0, font->data_size);
    font->file_hdr = 0;
    font->header_size = 0;
    font->unicode_info_size = (256 * 4);
    font->unicode_info = (unsigned char *)malloc(font->unicode_info_size);
    if(!font->unicode_info) goto memory_error;
    int i, j = 0;
    unsigned short *data = (unsigned short *)font->unicode_info;
    for(i = 0; i < (int)font->length; i++)
    {
        data[j  ] = default_unicode_table[i];
        data[j+1] = 0xFFFF;
        j += 2;
    }
    create_empty_unitab(font);
    get_font_unicode_table(font);
    calc_max_zoom(font);
    font->state = NEW;
    font->module = get_module_by_name("bdf");
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


struct font_s *bdf_load_font_file(char *file_name)
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
    font = bdf_load_font(file_name, (unsigned char *)file_data, file_size);
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


struct font_s *bdf_load_font(char *file_name, unsigned char *file_data, 
                             long file_size __attribute__((unused)))
{
    struct font_s *font = (struct font_s *)NULL;
    font = (struct font_s *)malloc(sizeof(struct font_s));
    if(!font) goto memory_error;

    (void)file_name;

    memset((void *)font, 0, sizeof(struct font_s));
    unsigned char *data = (unsigned char *)NULL;
    unsigned short *unicode_info = NULL;
    font->has_unicode_table = 1;
    font->utf_version = VER_PSF1;
    font->has_metadata = 1;
    font->metadata = (void *)malloc(sizeof(metadata_table));
    if(!font->metadata) goto memory_error;
    /* copy the standard metadata table, to fill it later */
    memcpy(font->metadata, (void *)&metadata_table, sizeof(metadata_table));
    struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
    font->version = get_version("BDF");
    /* to be assigned later when we know char count */
    struct char_info_s *char_info = NULL;

    char *s = strtok((char *)file_data, "\n");
    if(!s) goto file_read_error;
    keyword_bitmap = 0;

    /* there is no signature. just make sure the first line is legit. */
    /*
        if(strncmp(s, "STARTFONT", 9)) goto corrupt_file;
    // check the version. we support ver 2.1 //
    if(strcmp(s, "STARTFONT 2.1"))
    {
      status_error("BDF versions apart from 2.1 are not supported");
      goto go;
    }
    */

    int scope = BDF_GLOBAL_SCOPE;
    int w = 0, h = 0;
    int globalw = 0, globalh = 0;
    char globaln = 0;
    int xoff = 0, yoff = 0;
    int globalxoff = 0, globalyoff = 0;
    int i, chars = 0;
    unsigned int data_index = 0;
    
    do
    {
        skip_spaces(&s);
        char *keyword = get_keyword(&s);
        char index = get_keyword_index(keyword);

        if(index == BDF_TOTAL_KEYWORDS)
        {
            if(scope == BDF_GLOBAL_SCOPE)
            {
                continue;
            }
        }

        /* mark the keyword as found in our bitmap */
        keyword_bitmap |= (1 << index);

        /* now check the keyword */
        switch(index)
        {
            case STARTFONT_KEYWORD:
                skip_spaces(&s);
                /* check the version. we support ver 2.1 */
                if(strcmp(s, "2.1"))
                {
                    status_error("BDF versions apart from 2.1 are not supported");
                    goto go;
                }
                break;

            case STARTPROPERTIES_KEYWORD:
                scope = BDF_PROPERTIES_SCOPE;
                break;

            case ENDPROPERTIES_KEYWORD:
                scope = BDF_GLOBAL_SCOPE;
                break;

            case FONT_KEYWORD:
                skip_spaces(&s);
                int z = 0;
                while(*s != '\0') buf[z++] = *s++;
                save_metadata_str(font, METADATA_FONT, buf); //get_string(&s));
                break;

            case COPYRIGHT_KEYWORD:
                save_metadata_str(font, METADATA_COPYRIGHT, get_string(&s));
                break;

            case FONT_VERSION_KEYWORD:
                save_metadata_str(font, METADATA_FONT_VERSION, get_string(&s));
                break;

            case FONT_TYPE_KEYWORD:
                save_metadata_str(font, METADATA_FONT_TYPE, get_string(&s));
                break;

            case FOUNDRY_KEYWORD:
                save_metadata_str(font, METADATA_FOUNDRY, get_string(&s));
                break;

            case FAMILY_NAME_KEYWORD:
                save_metadata_str(font, METADATA_FAMILY_NAME, get_string(&s));
                break;

            case WEIGHT_NAME_KEYWORD:
                save_metadata_str(font, METADATA_WEIGHT_NAME, get_string(&s));
                break;

            case SLANT_KEYWORD:
                save_metadata_str(font, METADATA_SLANT, get_string(&s));
                break;

            case SETWIDTH_NAME_KEYWORD:
                save_metadata_str(font, METADATA_SETWIDTH_NAME, get_string(&s));
                break;

            case PIXEL_SIZE_KEYWORD:
                metadata[METADATA_PIXEL_SIZE].value = get_integer(&s);
                break;

            case POINT_SIZE_KEYWORD:
                metadata[METADATA_POINT_SIZE].value = get_integer(&s);
                break;

            case RESOLUTION_X_KEYWORD:
                metadata[METADATA_RESOLUTION_X].value = get_integer(&s);
                break;

            case RESOLUTION_Y_KEYWORD:
                metadata[METADATA_RESOLUTION_Y].value = get_integer(&s);
                break;

            case SPACING_KEYWORD:
                save_metadata_str(font, METADATA_SPACING, get_string(&s));
                break;

            case AVERAGE_WIDTH_KEYWORD:
                metadata[METADATA_AVERAGE_WIDTH].value = get_integer(&s);
                break;

            case CHARSET_REGISTRY_KEYWORD:
                save_metadata_str(font, METADATA_CHARSET_REGISTRY, get_string(&s));
                break;

            case CHARSET_ENCODING_KEYWORD:
                save_metadata_str(font, METADATA_CHARSET_ENCODING, get_string(&s));
                break;

            case ADD_STYLE_NAME_KEYWORD:
                save_metadata_str(font, METADATA_ADD_STYLE_NAME, get_string(&s));
                break;

            case UNDERLINE_POSITION_KEYWORD:
                metadata[METADATA_UNDERLINE_POSITION].value = get_integer(&s);
                break;

            case UNDERLINE_THICKNESS_KEYWORD:
                metadata[METADATA_UNDERLINE_THICKNESS].value = get_integer(&s);
                break;

            case CAP_HEIGHT_KEYWORD:
                metadata[METADATA_CAP_HEIGHT].value = get_integer(&s);
                break;

            case X_HEIGHT_KEYWORD:
                metadata[METADATA_X_HEIGHT].value = get_integer(&s);
                break;

            case FONT_ASCENT_KEYWORD:
                metadata[METADATA_FONT_ASCENT].value = get_integer(&s);
                break;

            case FONT_DESCENT_KEYWORD:
                metadata[METADATA_FONT_DESCENT].value = get_integer(&s);
                break;

            case DEFAULT_CHAR_KEYWORD:
                metadata[METADATA_DEFAULT_CHAR].value = get_integer(&s);
                break;

            case FONTBOUNDINGBOX_KEYWORD:
                if(scope != BDF_GLOBAL_SCOPE) goto corrupt_file;
                globalw = get_integer(&s);
                globalh = get_integer(&s);
                globalxoff = get_integer(&s);
                globalyoff = get_integer(&s);
                font->width = globalw;
                font->height = globalh;
                globaln = ((globalw+7)/8)*2;
                font->charsize = font->height*((font->width+7)/8);
                metadata[METADATA_FONTBOUNDINGBOX_X].value = globalw;
                metadata[METADATA_FONTBOUNDINGBOX_Y].value = globalh;
                metadata[METADATA_FONTBOUNDINGBOX_XOFF].value = globalxoff;
                metadata[METADATA_FONTBOUNDINGBOX_YOFF].value = globalyoff;
                break;

            case CHARS_KEYWORD:
                scope = BDF_CHARS_SCOPE;
                font->length = get_integer(&s);

                if(font->length > (unsigned int)-1)
                {
                    char buf[32];
                    sprintf(buf, "Invalid font length (%d).", font->length);
                    status_error(buf);
                    //msgBox("We don't support font length > 512 chars. Sorry!", OK, ERROR);
                    goto go;
                }

                font->data_size = font->charsize*font->length;
                data = (unsigned char *)malloc(font->data_size);
                if(!data) goto memory_error;
                memset((void *)data, 0, font->data_size);
                font->data = data;
                //font->unicode_info_size = (512 * 4);
                font->unicode_info_size = (font->length * 4);
                font->unicode_info = (unsigned char *)malloc(font->unicode_info_size);
                if(!font->unicode_info) goto memory_error;
                memset((void *)font->unicode_info, 0, font->unicode_info_size);
                unicode_info = (unsigned short *)font->unicode_info;
                if(!create_char_info(font)) goto memory_error;
                char_info = font->char_info;
                break;

            case STARTCHAR_KEYWORD:
                scope = BDF_GLYPH_SCOPE;
                skip_spaces(&s);
                /* check if it is Unicode-encoded */
                unsigned int chr = 0;
                int skip = 0;
                if(strncmp(s, "uni", 3) == 0)
                {
                    skip = 3;
                }
                else if(s[0] == 'U' || s[0] == 'u' || s[0] == 'C')
                {
                    if(ishexdigit(s[1]) || s[1] == '+')
                    {
                        skip++;
                        if(s[1] == '+') skip++;
                    }
                }
                  
                if(!skip)
                {
                    int found = 0;
                    if(*s >= '0' && *s <= '9')
                    {
                        chr = get_hex(s, 4);
                        if(chr) found = 1;
                    }
                    else
                    {
                        if(postscript_to_unicode(s, &chr)) found = 1;
                        else
                        {
                            //chr = codepoint_to_unicode(s);
                            if(strlen(s) == 4)
                            {
                                chr = get_hex(s, 4);
                                if(chr) found = 1;
                            }
                        }
                    }

                    if(!found)
                    {
                        char buf[113+strlen(s)];
                        strcpy(buf, "Char name '");
                        strcat(buf, s);
                        strcat(buf, "' not in Unicode, Adobe\n"
                                    "Standard Encoding, or Postscript char\n"
                                    "name lists. I Can't read this encoding!");
                        msgBox(buf, BUTTON_OK, ERROR);
                        goto go;
                    }
                }
                else
                {
                    s += skip;
                    chr = get_hex(s, 4);
                }

                unicode_info[chars*2] = (unsigned short)chr;
                unicode_info[(chars*2)+1] = 0xFFFF;
                break;

            case ENDCHAR_KEYWORD:
                scope = BDF_CHARS_SCOPE;
                chars++;
                break;

            case ENDFONT_KEYWORD:
                scope = BDF_GLOBAL_SCOPE;
                break;

            case BBX_KEYWORD:
                if(scope != BDF_GLYPH_SCOPE) goto corrupt_file;
                w = get_integer(&s);
                h = get_integer(&s);
                xoff = get_integer(&s);
                yoff = get_integer(&s);

                if(font->char_info)
                {
                    char_info[chars].BBw = w;
                    char_info[chars].BBh = h;
                    char_info[chars].BBXoff = xoff;
                    char_info[chars].BBYoff = yoff;
                }
                break;

            case SWIDTH_KEYWORD:
                if(scope != BDF_GLYPH_SCOPE) goto corrupt_file;
                if(font->char_info)
                {
                    int swx1 = get_integer(&s);
                    int swy1 = get_integer(&s);
                    char_info[chars].swidthX = swx1;
                    char_info[chars].swidthY = swy1;
                }
                break;

            case DWIDTH_KEYWORD:
                if(scope != BDF_GLYPH_SCOPE) goto corrupt_file;
                if(font->char_info)
                {
                    int dwx1 = get_integer(&s);
                    int dwy1 = get_integer(&s);
                    char_info[chars].dwidthX = dwx1;
                    char_info[chars].dwidthY = dwy1;
                }
                break;

            case ENCODING_KEYWORD:
                if(scope != BDF_GLYPH_SCOPE) goto corrupt_file;
                if(font->char_info)
                {
                    int encoding = get_integer(&s);
                    char_info[chars].encoding = encoding;
                }
                break;

            case BITMAP_KEYWORD:
                if(scope != BDF_GLYPH_SCOPE) goto corrupt_file;
                if(!font->data) goto corrupt_file;
                if(!w || !h)
                {
                    if(!globalw || !globalh) goto corrupt_file;
                    //w = globalw; h = globalh;
                }

                i = h;
                data_index = chars * font->charsize;

                while(i && (s = strtok(NULL, "\n")))
                {
                    /* how many nibbles do we have per line? */
                    unsigned int n = ((w+7)/8)*2;
                    skip_spaces(&s);
                    //printf("0x%s: ", s);
                    if(strlen(s) < n) goto corrupt_file;

                    unsigned int line = get_hex(s, n);
                    unsigned int j;

                    /* convert nibble count to byte count */
                    n >>= 1;
                    for(j = 0; j < n; j++)
                    {
                        data[data_index+j] = line & 0xFF;
                        line >>= 8;
                    }

                    n = globaln >> 1;
                    for( ; j < n; j++)
                    {
                        data[data_index+j] = 0;
                    }
                    data_index += n;
                    i--;
                }

                if(i) goto corrupt_file;
                /* fill the rest of lines (if any) with zeroes */
                i = globalh-h;

                while(i--)
                {
                    /* how many nibbles do we have per line? */
                    unsigned int n = ((globalw+7)/8)*2;
                    n >>= 1;
                    unsigned int j;

                    for(j = 0; j < n; j++)
                    {
                        data[data_index+j] = 0;
                    }
                    data_index += n;
                }

                break;
        }
    } while((s = strtok(NULL, "\n")));

    /* check we got all chars */
    if(chars != (int)font->length) goto corrupt_file;

    /* make sure no vital keyword is missing */
    for(i = 0; i < BDF_TOTAL_KEYWORDS; i++)
    {
        if(bdf_keywords[i].required)
        {
            if(!(keyword_bitmap & (1 << i))) goto corrupt_file;
        }
    }
    
    /* passed all tests? great! */
    create_empty_unitab(font);
    get_font_unicode_table(font);
    calc_max_zoom(font);
    font->state = OPENED;
    font->module = get_module_by_name("bdf");
    /* something REALLY WRONG happended here */
    if(!font->module) goto undefined_error;
        
    reset_all_cursors();
    return font;

undefined_error:

    status_error("Error loading font file");
    //msgBox("Error creating new font", OK, ERROR);
    goto go;

corrupt_file:

    status_error("Invalid or corrupt file");
    //msgBox("Invalid or corrupt file", OK, ERROR);
    goto go;

memory_error:

    status_error("Insufficient memory");
    //msgBox("Insufficient memory", OK, ERROR);
    goto go;

file_read_error:

    status_error("Error reading file");
    //msgBox("Error reading file", OK, ERROR);

go:

    kill_font(font);
    return (struct font_s *)NULL;
}


int bdf_write_to_file(FILE *file, struct font_s *font)
{
    if(!file || !font) return 1;
    int res;
    struct metadata_item_s *meta = (struct metadata_item_s *)font->metadata;
    res = fprintf(file, "STARTFONT 2.1\n");

    if(meta[METADATA_FONT].value2 && meta[METADATA_FONT].value2[0] != '\0')
    {
        res = fprintf(file, "FONT %s\n", meta[METADATA_FONT].value2);
    }
    else
    {
        char *name = strrchr(font_file_name, '/');
        if(name) name++;
        else name = font_file_name;

        char *dot = strrchr(name, '.');
        if(dot)
            res = fprintf(file, "FONT %.*s\n", (int)(dot-name), name);
        else
            res = fprintf(file, "FONT %s\n", name);
    }

    res = fprintf(file, "SIZE 16 75 75\n");
    res = fprintf(file, "FONTBOUNDINGBOX %d %d %d %d\n", 
                        meta[METADATA_FONTBOUNDINGBOX_X].value,
                        meta[METADATA_FONTBOUNDINGBOX_Y].value,
                        meta[METADATA_FONTBOUNDINGBOX_XOFF].value,
                        meta[METADATA_FONTBOUNDINGBOX_YOFF].value);

#define OPTIONAL_VAL(x)     meta[x].value2 ? meta[x].value2 : ""

    /* NOTE: change this number if we added more properties */
    res = fprintf(file, "STARTPROPERTIES 23\n");
    res = fprintf(file, "COPYRIGHT \"%s\"\n", OPTIONAL_VAL(METADATA_COPYRIGHT));
    res = fprintf(file, "FONT_TYPE \"%s\"\n", OPTIONAL_VAL(METADATA_FONT_TYPE));
    res = fprintf(file, "FONT_VERSION \"%s\"\n", OPTIONAL_VAL(METADATA_FONT_VERSION));
    res = fprintf(file, "FOUNDRY \"%s\"\n", OPTIONAL_VAL(METADATA_FOUNDRY));
    res = fprintf(file, "FAMILY_NAME \"%s\"\n", OPTIONAL_VAL(METADATA_FAMILY_NAME));
    res = fprintf(file, "WEIGHT_NAME \"%s\"\n", OPTIONAL_VAL(METADATA_WEIGHT_NAME));
    res = fprintf(file, "SLANT \"%s\"\n", OPTIONAL_VAL(METADATA_SLANT));
    res = fprintf(file, "SETWIDTH_NAME \"%s\"\n", OPTIONAL_VAL(METADATA_SETWIDTH_NAME));
    res = fprintf(file, "PIXEL_SIZE %d\n", meta[METADATA_PIXEL_SIZE].value);
    res = fprintf(file, "RESOLUTION_X %d\n", meta[METADATA_RESOLUTION_X].value);
    res = fprintf(file, "RESOLUTION_Y %d\n", meta[METADATA_RESOLUTION_Y].value);
    res = fprintf(file, "SPACING \"%s\"\n", OPTIONAL_VAL(METADATA_SPACING));
    res = fprintf(file, "ADD_STYLE_NAME %d\n", meta[METADATA_ADD_STYLE_NAME].value);
    res = fprintf(file, "AVERAGE_WIDTH %d\n", meta[METADATA_AVERAGE_WIDTH].value);
    res = fprintf(file, "CHARSET_REGISTRY \"%s\"\n", OPTIONAL_VAL(METADATA_CHARSET_REGISTRY));
    res = fprintf(file, "CHARSET_ENCODING \"%s\"\n", OPTIONAL_VAL(METADATA_CHARSET_ENCODING));
    res = fprintf(file, "UNDERLINE_POSITION %d\n", meta[METADATA_UNDERLINE_POSITION].value);
    res = fprintf(file, "UNDERLINE_THICKNESS %d\n", meta[METADATA_UNDERLINE_THICKNESS].value);
    res = fprintf(file, "CAP_HEIGHT %d\n", meta[METADATA_CAP_HEIGHT].value);
    res = fprintf(file, "X_HEIGHT %d\n", meta[METADATA_X_HEIGHT].value);
    res = fprintf(file, "FONT_ASCENT %d\n", meta[METADATA_FONT_ASCENT].value);
    res = fprintf(file, "FONT_DESCENT %d\n", meta[METADATA_FONT_DESCENT].value);
    res = fprintf(file, "DEFAULT_CHAR %d\n", meta[METADATA_DEFAULT_CHAR].value);
    res = fprintf(file, "ENDPROPERTIES\n");
    res = fprintf(file, "CHARS %d\n", font->length);

#undef OPTIONAL_VAL

    int i, j, l;
    unsigned char *data = font->data;
    struct char_info_s *char_info = (struct char_info_s *)font->char_info;

    for(i = 0; i < (int)font->length; i++)
    {
        res = fprintf(file, "STARTCHAR ");

        if(font->has_unicode_table)
        {
            /* this specific glyph has multiple unicode values, they are
             * stored in a second array, pointed to by unicode_table_index.
             */
            if(font->unicode_table_index[i] == 0xFFFF)
            {
                unsigned int *arr = 0;
                get_unitab_entry(font, i, &arr);
                fprintf(file, "U+%04x\n", arr[0]);
            }
            else
            {
                fprintf(file, "U+%04x\n", font->unicode_table[i*2]);
            }
        }
        else
        {
            fprintf(file, "U+%04X\n", i);
        }
    
        if(font->char_info)
        {
            res = fprintf(file, "ENCODING %d\n", char_info[i].encoding);
            res = fprintf(file, "SWIDTH %d %d\n", char_info[i].swidthX, char_info[i].swidthY);
            res = fprintf(file, "DWIDTH %d %d\n", char_info[i].dwidthX, char_info[i].dwidthY);
            res = fprintf(file, "BBX %d %d %d %d\n", char_info[i].BBw, char_info[i].BBh,
                             char_info[i].BBXoff, char_info[i].BBYoff);
        }
        else
        {
            res = fprintf(file, "ENCODING %d\n", i);
            res = fprintf(file, "SWIDTH 1000 0\n");
            res = fprintf(file, "DWIDTH %d 0\n", font->width);
            res = fprintf(file, "BBX %d %d %d %d\n", 
                                meta[METADATA_FONTBOUNDINGBOX_X].value,
                                meta[METADATA_FONTBOUNDINGBOX_Y].value,
                                meta[METADATA_FONTBOUNDINGBOX_XOFF].value,
                                meta[METADATA_FONTBOUNDINGBOX_YOFF].value);
        }

        res = fprintf(file, "BITMAP\n");

        for(j = 0; j < (int)font->height; j++)
        {
            unsigned int line2 = 0;
            unsigned int line = 0;

            for(l = 0; l < ((int)font->width+7)/8; l++)
            {
                line = (line) | (unsigned int)data[l] << (l*8);
            }

            line2 = line;
            l = (font->width+7)/8;

            if(l == 1) fprintf(file, "%02X\n", line2);
            else if(l == 2) fprintf(file, "%04X\n", line2);
            else if(l == 3) fprintf(file, "%06X\n", line2);
            else if(l == 4) fprintf(file, "%08X\n", line2);
            else fprintf(file, "%0X\n", line2);
            data += l;
        }

        res = fprintf(file, "ENDCHAR\n");
    }

    res = fprintf(file, "ENDFONT\n");

    if(res == EOF) return 1;
    return 0;
}


void bdf_handle_hw_change(struct font_s *font, char *newdata, long new_datasize)
{
    if(newdata)
    {
        void *new_rawdata = (void *)malloc(new_datasize);
        /* FIXME: Handle this error more decently */
        if(!new_rawdata) return;
        memcpy((void *)new_rawdata, (void *)newdata, new_datasize);
        free(font->data);
        font->data = new_rawdata;
        font->data_size = new_datasize;
    }

    if(font->has_metadata)
    {
        struct metadata_item_s *meta = (struct metadata_item_s *)font->metadata;
        meta[METADATA_FONTBOUNDINGBOX_X].value = font->width;
        meta[METADATA_FONTBOUNDINGBOX_Y].value = font->height;
    }

    if(font->char_info)
    {
        struct char_info_s *char_info = font->char_info;
        int i;
        for(i = 0; i < (int)font->length; i++)
        {
            char_info[i].dwidthX     = font->width;
            char_info[i].lBearing    = 0;
            char_info[i].rBearing    = font->width;
            char_info[i].charAscent  = font->height;
            char_info[i].charDescent = 0;
        }
    }
}

/*
void bdf_export_unitab(struct font_s *font, FILE *f)
{
  status_error("BDF is not fully supported yet");
  return;
}
*/


void bdf_handle_version_change(struct font_s *font, 
                               char old_version __attribute__((unused)))
{
    font->header_size = 0;
    if(font->file_hdr) free(font->file_hdr);
    font->file_hdr = 0;
    //bdf_kill_unitab(font);

    if(!font->has_metadata)
    {
        font->has_metadata = 1;
        font->metadata = (void *)malloc(sizeof(metadata_table));
        if(!font->metadata) return;
        /* copy the standard metadata table, to fill it later */
        memcpy(font->metadata, (void *)&metadata_table, sizeof(metadata_table));
    }

    if(!font->char_info)
    {
        create_char_info(font);
    }

    struct metadata_item_s *metadata = (struct metadata_item_s *)font->metadata;
    metadata[METADATA_FONTBOUNDINGBOX_X].value = font->width;
    metadata[METADATA_FONTBOUNDINGBOX_Y].value = font->height;
    metadata[METADATA_FONTBOUNDINGBOX_XOFF].value = 0;
    metadata[METADATA_FONTBOUNDINGBOX_YOFF].value = 0;
}


void bdf_convert_to_psf(struct font_s *font)
{
    if(font->version == VER_PSF1)
    {
        struct psf1_header hdr;
        hdr.magic[0] = PSF1_MAGIC0;
        hdr.magic[1] = PSF1_MAGIC1;
        hdr.mode = 0;
        if(font->has_unicode_table) hdr.mode |= PSF1_MODEHASTAB;
        if(font->length == 512) hdr.mode |= PSF1_MODE512;
        hdr.charsize = font->charsize;
        /* shift font structure */
        long sz = sizeof(struct psf1_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr, sizeof(struct psf1_header));
        if(font->file_hdr) free(font->file_hdr);
        font->file_hdr = new_hdr;
        font->header_size = sz;
    }
    else if(font->version == VER_PSF2)
    {
        struct psf2_header hdr2;
        hdr2.magic[0] = PSF2_MAGIC0;
        hdr2.magic[1] = PSF2_MAGIC1;
        hdr2.magic[2] = PSF2_MAGIC2;
        hdr2.magic[3] = PSF2_MAGIC3;
        hdr2.version = 0;
        hdr2.length = font->length;
        hdr2.charsize = font->charsize;
        hdr2.height = font->height;
        hdr2.width = font->width;
        hdr2.headersize = sizeof(struct psf2_header);
        hdr2.flags = 0;
        if(font->has_unicode_table) hdr2.flags |= PSF2_HAS_UNICODE_TABLE;
        /* shift font structure */
        long sz = sizeof(struct psf2_header);
        unsigned char *new_hdr = (unsigned char *)malloc(sz);
        if(!new_hdr) { status_error("Insufficient memory"); return; }
        memcpy((void *)new_hdr, (void *)&hdr2, sizeof(struct psf2_header));
        if(font->file_hdr) free(font->file_hdr);
        font->file_hdr = new_hdr;
        font->header_size = sz;
    }

    /*
    if(font->has_metadata)
    {
        font->has_metadata = 0;
        free(font->metadata);
    }
    */

    force_font_dirty(font);
}

void bdf_shrink_glyphs(struct font_s *font, int old_length)
{
    psf_shrink_glyphs(font, old_length);
}

void bdf_expand_glyphs(struct font_s *font, int old_length, int option)
{
    psf_expand_glyphs(font, old_length, option);
}

long bdf_make_utf16_unitab(struct font_s *new_font, unsigned short **_unicode_table)
{
    return psf_make_utf16_unitab(new_font, _unicode_table);
}

int bdf_is_acceptable_width(struct font_s *font)
{
    return (font->width >= 4 && font->width <= 32);
}

int bdf_next_acceptable_width(struct font_s *font)
{
    if(font->width < 32) return font->width + 1;
    else return 4;
}

int bdf_is_acceptable_height(struct font_s *font)
{
    return (font->height >= 4 && font->height <= 32);
}

int bdf_next_acceptable_height(struct font_s *font)
{
    if(font->height < 32) return font->height + 1;
    else return 4;
}

/********************************
 * ******************************
 * ******************************/
struct module_s bdf_module;

void bdf_init_module()
{
    strcpy(bdf_module.mod_name, "bdf");
    bdf_module.max_width = 32;
    bdf_module.max_height = 32;
    bdf_module.max_length = 65535;
    bdf_module.create_empty_font = bdf_create_empty_font;
    bdf_module.write_to_file = bdf_write_to_file;
    bdf_module.load_font = bdf_load_font;
    bdf_module.load_font_file = bdf_load_font_file;
    bdf_module.handle_hw_change = bdf_handle_hw_change;
    bdf_module.shrink_glyphs = bdf_shrink_glyphs;
    bdf_module.expand_glyphs = bdf_expand_glyphs;
    bdf_module.update_font_hdr = NULL;
    bdf_module.handle_version_change = bdf_handle_version_change;
    bdf_module.handle_unicode_table_change = NULL; //bdf_handle_unicode_table_change;
    bdf_module.export_unitab = NULL; //bdf_export_unitab;
    //bdf_module.create_unitab = NULL; //bdf_create_unitab;
    //bdf_module.kill_unitab = NULL; //bdf_kill_unitab;
    bdf_module.convert_to_psf = bdf_convert_to_psf;
    bdf_module.is_acceptable_width = bdf_is_acceptable_width;
    bdf_module.next_acceptable_width = bdf_next_acceptable_width;
    bdf_module.is_acceptable_height = bdf_is_acceptable_height;
    bdf_module.next_acceptable_height = bdf_next_acceptable_height;
    register_module(&bdf_module);
    add_file_extension("bdf", "bdf");
}


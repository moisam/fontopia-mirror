/* 
 *    Copyright 2015, 2016, 2017, 2018 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: bdf_hepler.c
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

/* D. J. Bernstein hash function */
/*
static size_t djb_hash(char *cp)
{
    //printf("%s():\n", __FUNCTION__);
    size_t hash = 5381;
    while (*cp)
        hash = 33 * hash ^ (unsigned char) *cp++;
    //printf("%s(): finished [%u]\n", __FUNCTION__, hash);
    return hash;
}
*/

/* Fowler/Noll/Vo (FNV) hash function, variant 1a */
static size_t fnv1a_hash(char *cp)
{
    //printf("%s():\n", __FUNCTION__);
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    //printf("%s(): finished [%u]\n", __FUNCTION__, hash);
    return hash;
}

/* Helper function: Postscript character name to Unicode */
int postscript_to_unicode(char *s, unsigned int *res)
{
    //size_t j = djb_hash(s);
    size_t j = fnv1a_hash(s);
    j %= hashtab_sz;
    if(hashtab[j].count == 0) return 0;
    int i = 0;
    do
    {
        if(strcmp(s, hashtab[j].strval[i]) == 0)
        {
            *res = hashtab[j].unival[i];
            return 1;
        }
    } while(++i < hashtab[j].count);
    return 0;
}

/* Helper function: convert Adobe Standard Encoding codepoint to Unicode */
unsigned int codepoint_to_unicode(char *s)
{
    char c[5] = { '0', 'x', s[0], s[1], '\0' };
    int codepoint = atoi(c);
    if(!codepoint) return 0;
    if(codepoint == 0x27) return 0x2019;
    if(codepoint == 0x60) return 0x2018;
    if(codepoint == 0xA4) return 0x2044;
    if(codepoint == 0xA6) return 0x0192;
    if(codepoint >= 0x20 && codepoint <= 0xA7) return codepoint;
    if(codepoint == 0xA8) return 0xA4;
    if(codepoint == 0xA9) return 0x27;
    if(codepoint == 0xAA) return 0x201C;
    if(codepoint == 0xAB) return 0xAB;
    if(codepoint == 0xAC) return 0x2039;
    if(codepoint == 0xAD) return 0x203A;
    if(codepoint == 0xAE) return 0xFB01;
    if(codepoint == 0xAF) return 0xFB02;
    if(codepoint == 0xB1) return 0x2013;
    if(codepoint == 0xB2) return 0x2020;
    if(codepoint == 0xB3) return 0x2021;
    if(codepoint == 0xB4) return 0xB7;
    if(codepoint == 0xB6) return 0xB6;
    if(codepoint == 0xB7) return 0x2022;
    if(codepoint == 0xB8) return 0x201A;
    if(codepoint == 0xB9) return 0x201E;
    if(codepoint == 0xBA) return 0x201D;
    if(codepoint == 0xBB) return 0xBB;
    if(codepoint == 0xBC) return 0x2026;
    if(codepoint == 0xBD) return 0x2030;
    if(codepoint == 0xBF) return 0xBF;
    if(codepoint == 0xC1) return 0x60;
    if(codepoint == 0xC2) return 0xB4;
    if(codepoint == 0xC3) return 0x02C6;
    if(codepoint == 0xC4) return 0x02DC;
    if(codepoint == 0xC5) return 0xAF;
    if(codepoint == 0xC6) return 0x02D8;
    if(codepoint == 0xC7) return 0x02D9;
    if(codepoint == 0xC8) return 0xA8;
    if(codepoint == 0xCA) return 0x02DA;
    if(codepoint == 0xCB) return 0xB8;
    if(codepoint == 0xCD) return 0x02DD;
    if(codepoint == 0xCE) return 0x02DB;
    if(codepoint == 0xCF) return 0x02C7;
    if(codepoint == 0xD0) return 0x2014;
    if(codepoint == 0xE1) return 0xC6;
    if(codepoint == 0xE3) return 0xAA;
    if(codepoint == 0xE8) return 0x0141;
    if(codepoint == 0xE9) return 0xD8;
    if(codepoint == 0xEA) return 0x0152;
    if(codepoint == 0xEB) return 0xBA;
    if(codepoint == 0xF1) return 0xE6;
    if(codepoint == 0xF5) return 0x0131;
    if(codepoint == 0xF8) return 0x0142;
    if(codepoint == 0xF9) return 0xF8;
    if(codepoint == 0xFA) return 0x0153;
    if(codepoint == 0xFB) return 0xDF;
    return 0;
}

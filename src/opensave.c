/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: opensave.c
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
#include <unistd.h>
#include <console/dialogs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ncurses.h>

#define MAX_FILENAME_LEN            255

struct dirent **eps;
struct stat st;

struct dialog_size
{
    int x, y;
    int w, h;
};

static int one(const struct dirent *unused __attribute__((unused))) 
{
    return 1;
}

static inline int valid_filename_char(char c)
{
    /* include all chars between ASCII 32 - 126 */
    if(c >= ' ' && c <= '~') return 1;
    return 0;
}

void update_name_file(int first_vis_char, int selected_char, char *entry, struct dialog_size *s)
{
    int len = strlen(entry);

    if(len)
    {
        locate(s->x+s->h-1, s->y+1);
        setScreenColors(BLACK, BGWHITE);

        if(len >= s->w-2)
        {
            int k;
            for(k = 0; k < s->w-2; k++)
            {
                char c = entry[first_vis_char+k];
                if(c == '\0') c = ' ';
                addch(c);
            }
        }
        else
        {
            printw("%s", entry);
            printw("%*s", s->w-2-len, " ");  
        } 
    }
    else
    {
        /* put empty field for file name */
        locate(s->x+s->h-1, s->y+1);
        setScreenColors(BLACK, BGWHITE);
        printw("%*s", s->w-2, " ");
    }

    locate(s->x+s->h-1, s->y+1+selected_char);
    refresh();
}

void free_entries(int count)
{
    if(!count) return;
    if(eps)
    {
        int n;
        for(n = 0; n < count; n++) free(eps[n]);
    }
}

int scan_dir(char *d)
{
    int n;

    if(strcmp(d, "."))
    {
        if(chdir(d) == -1) return -1;
        n = scandir(".", &eps, one, alphasort);
    }
    else
        n = scandir(d, &eps, one, alphasort);

    return n;
}

int get_direntry(int index, int cnt, char **entry)
{
    if(cnt >= 0 && index < cnt)
    {
        if(lstat(eps[index]->d_name, &st) == -1) return -1;
        *entry = eps[index]->d_name;
        if(S_ISDIR(st.st_mode)) return 1;
        else return 0;
    }

    return -1;
}

int first_starts_with(char c, int start_at, int entries_count)
{
    int i;

    for(i = start_at+1; i < entries_count; i++)
    {
        if(eps[i]->d_name[0] == c) return i;
    }

    // not found? try searching from the top
    for(i = 0; i <= start_at; i++)
    {
        if(eps[i]->d_name[0] == c) return i;
    }

    // nothing found
    return -1;
}

int check_file_exists(char *name)
{
    if(!name) return -1;
    if(lstat(name, &st) == -1) return -1;
    if(S_ISDIR(st.st_mode)) return 1;
    else return 0;
}


int __show_opensave(char *_path, int OPEN_SAVE, char **file, int max_len, int include_path)
{
    if(!file) return -1;
    if(OPEN_SAVE != OPEN && OPEN_SAVE != SAVE) return -1;

    char *path;
    int path_max_len = 0;
    char *file_name = (char *)malloc(256);

    if(!file_name) return -1;
    file_name[0] = '\0';

    /* try CWD if the caller didn't supply starting path */
    if(!_path)
    {
        path_max_len = 255;
        _path = ".";
    }
    else
    {
        path_max_len = strlen(_path)+1;
        if(path_max_len < 255) path_max_len = 255;
    }

    path = (char *)malloc(path_max_len);

    if(!path)
    {
        free(file_name);
        return -1;
    }

    strcpy(path, _path);
  
    int res = -1;    /* final result to be returned to caller, assume error */
    int count;

    if((count = scan_dir(path)) == -1)
    {
        /* try home */
        strcpy(path, "~");

        if((count = scan_dir(path)) == -1)
        {
            free(path);
            free(file_name);
            return -1;
        }
    }
  
    int h = 20;
    int w = 40;
    int x = 1, y = 1;

    if(h >= SCREEN_H) h = SCREEN_H-1;
    else x = (SCREEN_H-h)/2;
    if(w >= SCREEN_W) w = SCREEN_W-1;
    else y = (SCREEN_W-w)/2;

    /* for passing to update_name_file() function */
    struct dialog_size s = { x, y, w, h };
  
    int first_entry = 0;
    int highlighted_entry = 0;
    int total_vis_entries = h-3;

    /* Active field in the dialog. Can be:
     * 0 - Dir/File list
     * 1 - Filename field
     */
    char active_field = 0;
    int first_vis_char = 0, selected_char = 0;

    /* we use two counters because of the dot-case (.).
     * Here we don't want to list dot in the listing,
     * but it still counts (and maybe we find another use
     * for it in the future). So we use i as grand loop counter,
     * and j as dir counter.
     */
    int i, j;

    char *null_entry = "(No entries)";
    char *entry = (char *)NULL;

    /* flag to indicate if each entry is a file or dir, we assume
     * maximal height of 100 (can a terminal have higher values for height?).
     */
    char is_dir[100];
    char *cwd = (char *)NULL;
  
clear_window:

    setScreenColors(WHITE, BGDEFAULT);
    drawBox(x, y, h+x, w+y, (OPEN_SAVE == OPEN) ? " OPEN " : " SAVE AS ", 1);

    if(cwd) free(cwd);
    cwd = getcwd(NULL, 0);
    if(!cwd) goto bail_out;

    locate(x+1, y+1);
    setScreenColors(BROWN, BGDEFAULT);

    /* print cwd */
    int cwd_len = strlen(cwd);

    if(cwd_len > w-2)
    {
        j = cwd_len-(w-3);
        printw("..%s", cwd+j);
    }
    else printw("%s", cwd);

    /* put empty field for file name */
    locate(x+h-1, y+1);
    setScreenColors(BLACK, BGWHITE);
    printw("%*s", w-2, " ");
  
    /* check for the freaking dot */
    get_direntry(0, count, &entry);
    if(strcmp(entry, ".") == 0) first_entry = 1;

show_entries:

    hideCursor();
    if(entry == null_entry) goto loop;
  
    for(i = 0, j = 0; j < total_vis_entries; i++)
    {
        if(first_entry+j == count) break;
        locate(x+2+j, y+1);

        int _is_dir = get_direntry(first_entry+i, count, &entry);

        /*
         * FIXME: there was an error while accessing this entry. We choose to ignore that,
         *        but we can alert the user. This can be troublesome to the user if the error repeats,
         *        which is probable. Hints?.
         */
        if(_is_dir == -1) continue;

        if(!entry) entry = null_entry;
    
        if(j == highlighted_entry) setScreenColors(BLACK, BGWHITE);
        else setScreenColors(WHITE, BGDEFAULT);
    
        int len = strlen(entry);

        if(_is_dir)
        {
            if(strcmp(entry, ".") == 0) continue;
            len -= 2;
        }

        /* print it out */
        int l = w-2;
        if(_is_dir) l -= 2;
        if(len > l)
        {
            int k;
            l -= 2;    /* for the '..' */
            if(_is_dir) addch('[');
            for(k = 0; k < l; k++) addch(entry[k]) ;
            printw("..");
            if(_is_dir) addch(']');
        }
        else
        {
            printw("%*s", w-2, " ");
            locate(x+2+j, y+1);
            if(_is_dir) printw("[%s]", entry);
            else printw("%s", entry);
        }

        is_dir[j] = (char)_is_dir;

        /* if it's a file and is highlighted, put the name in the name field  */
        if(_is_dir == 0 && j == highlighted_entry)
        {
            locate(x+h-1, y+1);
            setScreenColors(BLACK, BGWHITE);

            if(len > w-2)
            {
                int k;
                    for(k = 0; k < w-2; k++) addch(entry[k]) ;
            }
            else
            {
                printw("%s", entry);
                printw("%*s", w-2-len, " ");
            }

            if(len > MAX_FILENAME_LEN)
            {
                strncpy(file_name, entry, MAX_FILENAME_LEN);
                file_name[MAX_FILENAME_LEN] = '\0';
            }
            else strcpy(file_name, entry);
            first_vis_char = 0; selected_char = 0;
        }
        j++;
    }

    refresh();
  
    char ch;
    char end = 0;
    int file_name_len, index;

loop:

    while(!end)
    {
        ch = getKey();

        switch(ch)
        {
            case(SPACE_KEY):
                if(active_field == 1) goto do_char;
                __attribute__((fallthrough));

            case(ENTER_KEY):
                if(active_field == 1)
                {
                    /* maybe they want to open a directory? */
                    int r = check_file_exists(file_name);
                    if(r == 1)
                    {
                        entry = file_name;
                        active_field = 0;
                        goto do_entry;
                    }
                    /* if saving, check if user understands they selected
                     * an existing file.
                     */
                    else if(r == 0)
                    {

do_save:

                        if(OPEN_SAVE == SAVE)
                        {
                            if(msgBox("File already exists. Overwrite?", 
                                            BUTTON_YES|BUTTON_NO, INFO) != BUTTON_YES)
                                goto clear_window;
                        }
                    }

                    /* nope. get out with the file name */
                    res = 0;
                    end = 1;
                    break;
                }

                if(is_dir[highlighted_entry] == 1)
                {
                    get_direntry(first_entry+highlighted_entry, count, &entry);

do_entry:

                    path_max_len = strlen(entry)+1;
                    int cur_path_len = strlen(path)+1;

                    if(path_max_len < cur_path_len)
                    {
                        strcpy(path, entry);
                    }
                    else
                    {
                        free(path);
                        path = (char *)malloc(path_max_len);
                        if(!path) goto bail_out;
                        strcpy(path, entry);
                    }

                    free_entries(count);
                    if((count = scan_dir(path)) == -1) goto bail_out;
                    entry = 0;
                    first_entry = 1;
                    highlighted_entry = 0;
                    file_name[0] = '\0';
                    first_vis_char = 0; selected_char = 0;
                    goto clear_window;
                }
                else
                {
                    /* user selected a file from the list */
                    goto do_save;
                }
                break;

            case(LEFT_KEY):
            case(UP_KEY):
                if(active_field == 1)
                {
                    if(selected_char == 0)
                    {
                        if(first_vis_char == 0) break;
                        first_vis_char--;
                    }
                    else selected_char--;
                    update_name_file(first_vis_char, selected_char, file_name, &s);
                    break;
                }

                if(highlighted_entry == 0)
                {
                    if(first_entry == 1) break;
                    first_entry--;
                }
                else highlighted_entry--;
                goto show_entries;

            case(RIGHT_KEY):
            case(DOWN_KEY):
                if(active_field == 1)
                {
                    file_name_len = strlen(file_name);
                    if(selected_char == w-3)
                    {
                        if(first_vis_char+selected_char == file_name_len) break;
                        first_vis_char++;
                    }
                    else
                    {
                        if(file_name_len <= w-2 && selected_char == file_name_len) break;
                        selected_char++;
                    }

                    update_name_file(first_vis_char, selected_char, file_name, &s);
                    break;
                }

                if(count <= total_vis_entries)
                {
                    if(highlighted_entry+first_entry < count-1) highlighted_entry++;
                    else break;
                }
                else
                {
                    if(highlighted_entry < total_vis_entries-1) highlighted_entry++;
                    else
                    {
                        if(first_entry+total_vis_entries < count) first_entry++;
                        else break;
                    }
                }
                goto show_entries;

            case(TAB_KEY):
                active_field = !active_field;
                if(active_field == 1)
                {
                    locate(x+h-1, y+1+first_vis_char);
                    showCursor();
                }
                else hideCursor();
                break;

            case(ESC_KEY):
                end = 1;
                res = 1;
                break;

            case(DEL_KEY):
                if(active_field == 0) break;
                file_name_len = strlen(file_name);
                index = first_vis_char+selected_char;
                if(file_name_len == 0) break;
                if(index > file_name_len) break;

                /* shift all chars to left by one */
                for(i = index; i < file_name_len; i++) file_name[i] = file_name[i+1];
                update_name_file(first_vis_char, selected_char, file_name, &s);
                break;

            case(BACKSPACE_KEY):
                if(active_field == 0) break;
                file_name_len = strlen(file_name);
                index = first_vis_char+selected_char;
                if(file_name_len == 0) break;
                if(index == 0) break;

                /* shift all chars to left by one */
                for(i = index-1; i < file_name_len; i++) file_name[i] = file_name[i+1];

                /* update cursor position */
                if(selected_char == 0) first_vis_char--;
                else selected_char--;
                update_name_file(first_vis_char, selected_char, file_name, &s);
                break;

            case(PGUP_KEY):
                if(active_field == 0)
                {
                    highlighted_entry -= total_vis_entries;
                    if(highlighted_entry < 0)
                    {
                        first_entry += highlighted_entry;
                        if(first_entry < 0) first_entry = 0;
                        highlighted_entry = 0;
                    }
                    goto show_entries;
                }

            case(HOME_KEY):
                if(active_field == 0)
                {
                    first_entry = 1;
                    highlighted_entry = 0;
                    goto show_entries;
                }
                else
                {
                    first_vis_char = 0;
                    selected_char = 0;
                    update_name_file(first_vis_char, selected_char, file_name, &s);
                }
                break;

            case(PGDOWN_KEY):
                if(active_field == 0)
                {
                    highlighted_entry += total_vis_entries;
                    if(count <= total_vis_entries)
                    {
                        highlighted_entry = count-first_entry-1;
                    }
                    else
                    {
                        if(highlighted_entry >= total_vis_entries)
                        {
                            first_entry += (highlighted_entry-total_vis_entries);
                            highlighted_entry = total_vis_entries-1;
                        }

                        if(first_entry+total_vis_entries >= count)
                        {
                            first_entry = count-total_vis_entries;
                            highlighted_entry = total_vis_entries-1;
                        }
                    }
                    goto show_entries;
                }

            case(END_KEY):
                if(active_field == 0)
                {
                    if(count <= total_vis_entries)
                    {
                        highlighted_entry = count-first_entry-1;
                    }
                    else
                    {
                        first_entry = count-total_vis_entries;
                        highlighted_entry = total_vis_entries-1;
                    }
                    goto show_entries;
                }
                else
                {
                    file_name_len = strlen(file_name);
                    if(file_name_len < w-3)
                    {
                        selected_char = file_name_len;
                    }
                    else
                    {
                        selected_char = w-3;
                        first_vis_char = file_name_len-selected_char;
                    }
                    update_name_file(first_vis_char, selected_char, file_name, &s);
                }
                break;

            ////////////////////////////////////
            ////////////////////////////////////
            default:

do_char:

                if(active_field == 0)
                {
                    index = first_entry+highlighted_entry;
                    index = first_starts_with(ch, index, count);
                    if(index == -1) break;
                    i = index-first_entry;

                    if(i < 0)
                    {
                        first_entry = index;
                        highlighted_entry = 0;
                    }
                    else if(i >= total_vis_entries)
                    {
                        highlighted_entry = total_vis_entries-1;
                        first_entry = index-highlighted_entry;
                    }
                    else
                    {
                        highlighted_entry = i;
                    }
                    goto show_entries;
                }

                if(valid_filename_char(ch))
                {
                    if(strlen(file_name+1) > MAX_FILENAME_LEN) break;
                    file_name_len = strlen(file_name);
                    index = first_vis_char+selected_char;

                    /* insert in the middle of string */
                    if(index < file_name_len)
                    {
                        /* shift all chars to right by one */
                        for(i = file_name_len+1; i > index; i--) file_name[i] = file_name[i-1];
                        file_name[index] = ch;
                    }
                    else
                    {
                        file_name[index] = ch;
                        file_name[index+1] = '\0';
                    }

                    /* update cursor position */
                    if(selected_char == w-3) first_vis_char++;
                    else selected_char++;
                    update_name_file(first_vis_char, selected_char, file_name, &s);
                    break;
                }
        } /* end switch */
    } /* end while */

    if(res == 1) goto really_go;    /* user cancelled by pressing ESC */
    goto go;
  
bail_out:

    res = -1;
    goto really_go;

go:

    /* did the caller specify max_len for result? */
    cwd_len = strlen(cwd);
    file_name_len = strlen(file_name);
    int total_len = file_name_len;

    if(include_path) total_len += cwd_len+1; /* possible missing '/' in path */
    if(max_len && total_len > max_len)
    {
        *file = (char *)malloc(max_len+1);
        if(!*file) goto really_go;
        if(include_path)
        {
            if(cwd_len >= max_len)
            {
                strncpy(*file, cwd, max_len);
                (*file)[max_len] = '\0';
            }
            else
            {
                strcpy(*file, cwd);
                if(cwd[cwd_len-1] != '/')
                {
                    strcat(*file, "/");
                    cwd_len++;
                }
                file_name_len = max_len-cwd_len-1;
                strncat(*file, file_name, file_name_len);
                (*file)[max_len] = '\0';
            }
        }
        else
        {
            strncpy(*file, file_name, max_len);
            if(file_name_len >= max_len) (*file)[max_len] = '\0';
            else (*file)[file_name_len] = '\0';
        }
    }
    else
    {
        if(include_path)
        {
            *file = (char *)malloc(total_len+2);    /* extra char for possible '/' */
            if(!*file) goto really_go;
            strcpy(*file, cwd);
            if(cwd[cwd_len-1] != '/') strcat(*file, "/");
            strcat(*file, file_name);
        }
    }

really_go:

    free_entries(count);
    if(file_name) free(file_name);
    if(path) free(path);
    if(cwd) free(cwd);
    showCursor();
    return res;
}

int show_opensave(char *_path, int OPEN_SAVE, char **file, int max_len)
{
    /* include path in result */
    return __show_opensave(_path, OPEN_SAVE, file, max_len, 1);
}

int show_opensave_np(char *_path, int OPEN_SAVE, char **file, int max_len)
{
    /* do not include path in result */
    return __show_opensave(_path, OPEN_SAVE, file, max_len, 0);
}


/* 
 *    Copyright 2015, 2016, 2017, 2018, 2024 (c) Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 * 
 *    file: readme.c
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

char *readme_text =
{
  "README file for fontopia - the console font editor\n"
  "Copyright (c) 2015, 2016, 2017, 2018, 2024 by Mohammed Isam\n"
  "Fontopia is a GNU software\n"
  "Released under GPL v3+\n"
  "\n"
  "About:\n"
  "======\n"
  "Fontopia is an easy-to-use, text-based, console font editor.\n"
  "What this means in simple English is that you can edit the fonts\n"
  "that your GNU/Linux kernel is using to display your text on\n"
  "text-based (vs graphical) terminals.\n"
  "\n"
  "Unlike other console text editors which usually work on one PSF\n"
  "version, or work on unicode tables only, or allow very minimal glyph\n"
  "editing, fontopia provides all these functions together:\n"
  "- It works on both PSF 1 & 2, you can even change file type and save\n"
  "  it to disk in the other version.\n"
  "- It allows exporting and importing of unicode tables from external\n"
  "  files.\n"
  "- It provides a user-friendly, easy-to-use glyph editor.\n"
  "- It can easily change font metrics, e.g. length, width, height...\n"
  "\n"
  "Console font files are commonly of PSF type (of which there are two\n"
  "versions), or of CP type (legacy fonts). Fontopia works with PSF files\n"
  "of both versions, as well as CP fonts, BDF files and raw font files.\n"
  "\n"
  "Supported formats:\n"
  "==================\n"
  "- PSF 1 and 2\n"
  "- Linux Code pages (CP)\n"
  "- Raw fonts\n"
  "- BDF fonts (beta)\n"
  "- PCF font and Windows FON files support are under development\n"
  "\n"
  "WARNING:\n"
  "========\n"
  "(1) There is a major downside currently: fontopia doesn't work with unicode\n"
  "    sequences properly (at least not in all cases). If you export a unicode\n"
  "    table, edit it, and then import it to a font file, you should be safe.\n"
  "(2) Note that if you changed the font version from CP to any other version,\n"
  "    only the ACTIVE font will be changed, as the other font formats (PSF,\n"
  "    Raw, BDF) don't support multiple fonts inside the same font file. In this\n"
  "    case, you will need to open the original CP file multiple times, every\n"
  "    time select a different font size (by using '1'-'4' number keys) and\n"
  "    convert to the new font version, then reopen the original CP file, select\n"
  "    another font size, convert it, and so on.\n"
  "\n"
  "Navigation:\n"
  "===========\n"
  "Using the editor is very easy:\n"
  "(1)  Navigate the glyphs on the right side window using the arrow keys. Edit\n"
  "     a single glyph using ENTER (or TAB).\n"
  "(2)  Navigate the bits that form a single bit (on the left side window)\n"
  "     using arrow keys. Press ENTER or SPACE to set/unset the desired bit. Go\n"
  "     back to right window (glyph list) using TAB.\n"
  "(3)  To zoom in (make the glyph on the left look bigger), press Z.\n"
  "(4)  To zoom out, press X.\n"
  "(5)  To open a new file, press CTRL+O.\n"
  "(6)  To create a new font from scratch, press CTRL+N.\n"
  "(7)  To save your work, press CTRL+S.\n"
  "(8)  To save with a new filename, press CTRL+D (aka Save As..).\n"
  "(9)  To quit the editor, press CTRL+Q.\n"
  "(10) To show help, press CTRL+H.\n"
  "(11) To copy a glyph, press CTRL+C.\n"
  "(12) To cut a glyph, press CTRL+X.\n"
  "(13) To paste a glyph, press CTRL+V.\n"
  "(14) To show about dialog box, press A.\n"
  "(15) To switch buffer mode (on/off), press CTRL+B.\n"
  "(16) To clear a glyph (i.e. remove all bits), press C.\n"
  "(17) To invert a glyph, press D.\n"
  "(18) To export font unicode table (by default to a file with same original\n"
  "     file name plus .tab extension), press E.\n"
  "(19) To flip a glyph horizontally, press H.\n"
  "(20) To import unicode table from another file, press I.\n"
  "(21) To show these keys without the extra jargon, press K.\n"
  "(22) To show font metrics window, press M.\n"
  "(23) To change codepage of a CP font, press P.\n"
  "     If font is in BDF format, P shows properties (or metadata).\n"
  "(24) To remove unicode table from font, press CTRL+R.\n"
  "(25) To set a glyph (i.e. set all bits), press S.\n"
  "(26) To show font unicode table, press U.\n"
  "(27) To flip a glyph vertically, press V.\n"
  "(28) To export font glyphs in textual format (by default to a file with\n"
  "     same original file name plus .glyph extension), press W.\n"
  "     You can alternatively export glyphs to a C source file by pressing CTRL+W."
  "(29) If editing a CP font and you want to change the active font (e.g.\n"
  "     edit the 8x8 font instead of the 16x8 font you are currently editing)\n"
  "     press any key from '1' to '4' and the respective font will be the\n"
  "     active font to be edited. You can go back and forth by using the\n"
  "     number keys.\n"
  "\n"
  "The unicode values that are mapped to a specific glyph are shown on the\n"
  "status bar (the bottom strip) to the right side.\n"
  "\n"
  "That should make you ready to move around and edit your fonts. Have fun!.\n"
};


char *keys_text =
{
  "Keys (in alphabetical order):\n"
  "=============================\n"
  "TAB:    Navigate to left/right window.\n"
  "ENTER:  Toggle bit set/unset (if in left window)\n"
  "        Edit glyph (if in right window)\n"
  "SPACE:  As ENTER above\n"
  "^B   :  Switch buffer mode (on/off)\n"
  "^C   :  Copy a glyph\n"
  "^D   :  Save with a new filename (aka Save As..)\n"
  "^H   :  Show help\n"
  "^N   :  Create a new font from scratch\n"
  "^O   :  Open a new file\n"
  "^Q   :  Quit the editor\n"
  "^R   :  Remove unicode table from font\n"
  "^S   :  Save your work\n"
  "^V   :  Paste a glyph\n"
  "^W   :  Export font glyphs to a C-style source file\n"
  "^X   :  Cut a glyph\n"
  "A    :  Show about dialog box\n"
  "C    :  Clear a glyph (i.e. remove all bits)\n"
  "D    :  Invert a glyph\n"
  "E    :  Export font unicode table\n"
  "I    :  Import unicode table from another file\n"
  "K    :  Show these keys without the extra jargon\n"
  "M    :  Show font metrics window\n"
  "P    :  Change codepage of a CP font\n"
  "        If font is in BDF format, P shows properties (or metadata)\n"
  "S    :  Set a glyph (i.e. set all bits)\n"
  "U    :  Show font unicode table\n"
  "W    :  Export font glyphs to a plain text file\n"
  "X    :  Zoom out\n"
  "Z    :  Zoom in\n"
  "1 to 4 (number keys):\n"
  "        Change the active font in a CP font file\n"
};


int fontopia_show_readme(char *readme, char *title)
{
    FILE *file;

    if(!(file = tmpfile()))
    {
        msgBoxH("Error opening temporary file. Aborting.", BUTTON_OK, ERROR);
        return 1;
    }

    if(fprintf(file, "%s", readme) <= 0)
    {
        fclose(file);
        msgBoxH("Error writing to temporary file. Aborting.", BUTTON_OK, ERROR);
        return 1;
    }

    fflush(file);
    setScreenColors(WHITE, BGDEFAULT);
    showReadme(file, title, 1);
    fclose(file);
    return 0;
}


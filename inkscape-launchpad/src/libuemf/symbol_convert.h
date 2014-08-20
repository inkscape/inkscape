/** @file
 * @brief Enhanced Metafile Input/Output
 */
/* Authors:
 *   David Mathog <mathog@caltech.edu>
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef __cplusplus
extern "C" {
#endif
 
#ifndef SEEN_UNICODE_CONVERT_H
#define SEEN_UNICODE_CONVERT_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

    enum cvt_to_font {CVTNON, CVTSYM, CVTZDG, CVTWDG};

    void msdepua(uint32_t *text);                 //translate down from Microsoft Private Use Area
    void msrepua(uint16_t *text);                 //translate up to Microsoft Private Use Area
    int isNon(char *font);                        //returns one of the cvt_to_font enum values
    char *FontName(int code);                     //returns the font name (or NULL) given the enum code
    int NonToUnicode(uint32_t *text, char *font); //nonunicode to Unicode translation
    int CanUTN(void);                             // 1 if tables exist for UnicodeToNon translation
    int SingleUnicodeToNon(uint16_t text);        //retuns the enum value for this translation
    void UnicodeToNon(uint16_t *text, int *ecount, int *edest); //translate Unicode to NonUnicode
    void TableGen(bool symb, bool wing, bool zdng, bool pua);

#ifdef __cplusplus
}
#endif

#endif /* SEEN_UNICODE_CONVERT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :



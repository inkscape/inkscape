/** \file
 * XML quoting routines.
 */

/* Based on Lauris' repr_quote_write in repr-io.cpp.
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * May be modified and/or redistributed under the terms of version 2
 * of the GNU General Public License: see the file `COPYING'.
 */

#include <cstring>
#include <glib/gmem.h>


/** \return strlen(xml_quote_strdup(\a val)) (without doing the malloc).
 *  \pre val != NULL
 */
static size_t
xml_quoted_strlen(char const *val)
{
    size_t ret = 0;
    for (; *val != '\0'; val++) {
        switch (*val) {
            case '"': ret += sizeof("&quot;") - 1; break;
            case '&': ret += sizeof("&amp;") - 1; break;
            case '<': ret += sizeof("&lt;") - 1; break;
            case '>': ret += sizeof("&gt;") - 1; break;
            default: ++ret; break;
        }
    }
    return ret;
}

/** Writes \a src (including the NUL byte) to \a dest, doing XML quoting as necessary.
 *
 *  \pre \a src != NULL.
 *  \pre \a dest must have enough space for (xml_quoted_strlen(src) + 1) bytes.
 */
static void
xml_quote(char *dest, char const *src)
{
#define COPY_LIT(_lit) do {	\
	    size_t cpylen = sizeof(_lit "") - 1; \
	    memcpy(dest, _lit, cpylen);	      \
	    dest += cpylen;	              \
	} while(0)

    for (; *src != '\0'; ++src) {
        switch (*src) {
            case '"': COPY_LIT("&quot;"); break;
            case '&': COPY_LIT("&amp;"); break;
            case '<': COPY_LIT("&lt;"); break;
            case '>': COPY_LIT("&gt;"); break;
            default: *dest++ = *src; break;
        }
    }
    *dest = '\0';

#undef COPY_LIT
}

/** \return A g_malloc'd buffer containing an XML-quoted version of \a src.
 *  \pre src != NULL.
 */
char *
xml_quote_strdup(char const *src)
{
    size_t const quoted_size = xml_quoted_strlen(src) + 1;
    char *ret = (char *) g_malloc(quoted_size);
    xml_quote(ret, src);
    return ret;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

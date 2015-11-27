/** \file
 * @brief XML quoting routines
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * This file is in the public domain.
 */

#include "xml/quote.h"
#include <cstring>
#include <glib.h>

/// Returns the length of the string after quoting the characters <code>"&amp;&lt;&gt;</code>.
size_t xml_quoted_strlen(char const *val)
{
    if (!val) return 0;
    size_t len = 0;

    for (char const *valp = val; *valp; ++valp) {
        switch (*valp) {
        case '"':
            len += 6; // &quot;
            break;
        case '&':
            len += 5; // &amp;
            break;
        case '<':
        case '>':
            len += 4; // &lt; or &gt;
            break;
        default:
            ++len;
            break;
        }
    }
    return len;
}

char *xml_quote_strdup(char const *src)
{
    size_t len = xml_quoted_strlen(src);
    char *result = static_cast<char*>(g_malloc(len + 1));
    char *resp = result;

    for (char const *srcp = src; *srcp; ++srcp) {
        switch(*srcp) {
        case '"':
            strcpy(resp, "&quot;");
            resp += 6;
            break;
        case '&':
            strcpy(resp, "&amp;");
            resp += 5;
            break;
        case '<':
            strcpy(resp, "&lt;");
            resp += 4;
            break;
        case '>':
            strcpy(resp, "&gt;");
            resp += 4;
            break;
        default:
            *resp++ = *srcp;
            break;
        }
    }
    *resp = 0;
    return result;
}

// quote: ", &, <, >


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

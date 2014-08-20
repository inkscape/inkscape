#include <string.h>
#include <glib.h>

#include "extract-uri.h"

// FIXME: kill this ugliness when we have a proper CSS parser

// Functions as per 4.3.4 of CSS 2.1
// http://www.w3.org/TR/CSS21/syndata.html#uri
gchar *extract_uri( gchar const *s, gchar const** endptr )
{
    if (!s)
        return NULL;

    gchar* result = NULL;
    gchar const *sb = s;
    if ( strlen(sb) < 4 || strncmp(sb, "url", 3) != 0 ) {
        return NULL;
    }

    sb += 3;

    if ( endptr ) {
        *endptr = 0;
    }

    // This first whitespace technically is not allowed.
    // Just left in for now for legacy behavior.
    while ( ( *sb == ' ' ) ||
            ( *sb == '\t' ) )
    {
        sb++;
    }

    if ( *sb == '(' ) {
        sb++;
        while ( ( *sb == ' ' ) ||
                ( *sb == '\t' ) )
        {
            sb++;
        }

        gchar delim = ')';
        if ( (*sb == '\'' || *sb == '"') ) {
            delim = *sb;
            sb++;
        }
        gchar const* se = sb + 1;
        while ( *se && (*se != delim) ) {
            se++;
        }

        // we found the delimiter
        if ( *se ) {
            if ( delim == ')' ) {
                if ( endptr ) {
                    *endptr = se + 1;
                }

                // back up for any trailing whitespace
                se--;
                while ( ( se[-1] == ' ' ) ||
                        ( se[-1] == '\t' ) )
                {
                    se--;
                }

                result = g_strndup(sb, se - sb + 1);
            } else {
                gchar const* tail = se + 1;
                while ( ( *tail == ' ' ) ||
                        ( *tail == '\t' ) )
                {
                    tail++;
                }
                if ( *tail == ')' ) {
                    if ( endptr ) {
                        *endptr = tail + 1;
                    }
                    result = g_strndup(sb, se - sb);
                }
            }
        }
    }

    return result;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

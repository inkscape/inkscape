#include <string.h>
#include <glib.h>

// FIXME: kill this ugliness when we have a proper CSS parser
gchar *extract_uri(gchar const *s)
{
    gchar const *sb = s;
    g_assert( strncmp(sb, "url", 3) == 0 );
    sb += 3;

    while ( ( *sb == ' ' ) ||
            ( *sb == '(' ) )
    {
        sb++;
    }

    gchar const *se = sb + strlen(sb);
    while ( ( se[-1] == ' ' ) ||
            ( se[-1] == ')' ) )
    {
        se--;
    }

    if ( sb < se ) {
        return g_strndup(sb, se - sb);
    } else {
        return NULL;
    }
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

/**
 * Helpers
 *
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <ctype.h>
#include <glib.h>

#include "gnome-utils.h"

/**
 * gnome_uri_list_extract_uris:
 * @uri_list: an uri-list in the standard format.
 *
 * Returns a GList containing strings allocated with g_malloc
 * that have been splitted from @uri-list.
 */
GList *gnome_uri_list_extract_uris(const gchar *uri_list)
{
    const gchar *p, *q;
    gchar *retval;
    GList *result = NULL;

    g_return_val_if_fail(uri_list != NULL, NULL);

    p = uri_list;

    /* We don't actually try to validate the URI according to RFC
     * 2396, or even check for allowed characters - we just ignore
     * comments and trim whitespace off the ends.  We also
     * allow LF delimination as well as the specified CRLF.
     */
    while (p) {
        if (*p != '#') {
            while (isspace(*p))
                p++;

            q = p;
            while (*q && (*q != '\n') && (*q != '\r'))
                q++;

            if (q > p) {
                q--;
                while (q > p && isspace(*q))
                    q--;

                retval = (gchar *)g_malloc(q - p + 2);
                strncpy(retval, p, q - p + 1);
                retval[q - p + 1] = '\0';

                result = g_list_prepend(result, retval);
            }
        }
        p = strchr(p, '\n');
        if (p)
            p++;
    }

    return g_list_reverse(result);
}

/**
 * gnome_uri_list_extract_filenames:
 * @uri_list: an uri-list in the standard format
 *
 * Returns a GList containing strings allocated with g_malloc
 * that contain the filenames in the uri-list.
 *
 * Note that unlike gnome_uri_list_extract_uris() function, this
 * will discard any non-file uri from the result value.
 */
GList *gnome_uri_list_extract_filenames(const gchar *uri_list)
{
    g_return_val_if_fail(uri_list != NULL, NULL);

    GList *result = gnome_uri_list_extract_uris(uri_list);

    GList *tmp_list = result;
    while (tmp_list) {
        gchar *s = (gchar *)tmp_list->data;

        GList *node = tmp_list;
        tmp_list = tmp_list->next;

        if (!strncmp(s, "file:", 5)) {
            node->data = g_filename_from_uri(s, NULL, NULL);
            /* not sure if this fallback is useful at all */
            if (!node->data)
                node->data = g_strdup(s + 5);
        } else {
            result = g_list_remove_link(result, node);
            g_list_free_1(node);
        }
        g_free(s);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

/*
 * Utility functions for reading and setting preferences
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "prefs-utils.h"
#include "inkscape.h"
#include "xml/repr.h"


void
prefs_set_recent_file(gchar const *uri, gchar const *name)
{
    unsigned const max_documents = prefs_get_int_attribute("options.maxrecentdocuments", "value", 20);

    if (uri != NULL) {
        Inkscape::XML::Node *recent = inkscape_get_repr(INKSCAPE, "documents.recent");
        if (recent) {
            // remove excess recent files
            if (recent->childCount() >= max_documents) {
                Inkscape::XML::Node *child = recent->firstChild();
                // count to the last
                for (unsigned i = 0; child && i + 1 < max_documents; ++i) {
                    child = child->next();
                }
                // remove all after the last
                while (child) {
                    Inkscape::XML::Node *next = child->next();
                    sp_repr_unparent(child);
                    child = next;
                }
            }

            if (max_documents > 0) {
                Inkscape::XML::Node *child = sp_repr_lookup_child(recent, "uri", uri);
                if (child) {
                    recent->changeOrder(child, NULL);
                } else {
                    child = recent->document()->createElement("document");
                    child->setAttribute("uri", uri);
                    recent->addChild(child, NULL);
                }
                child->setAttribute("name", name);
            }
        }
    }
}

gchar const **
prefs_get_recent_files()
{
    Inkscape::XML::Node *recent = inkscape_get_repr(INKSCAPE, "documents.recent");
    if (recent) {
        unsigned const docs = recent->childCount();
        gchar const **datalst = (gchar const **) g_malloc(sizeof(gchar *) * ((docs * 2) + 1));

        gint i;
        Inkscape::XML::Node *child;
        for (i = 0, child = recent->firstChild();
             child != NULL;
             child = child->next(), i += 2)
        {
            gchar const *uri = child->attribute("uri");
            gchar const *name = child->attribute("name");
            datalst[i]     = uri;
            datalst[i + 1] = name;
        }

        datalst[i] = NULL;
        return datalst;
    }

    return NULL;
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

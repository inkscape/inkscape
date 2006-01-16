#define __SP_REPR_C__

/** \file
 * A few non-inline functions of the C facade to Inkscape::XML::Node.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 1999-2003 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 * g++ port Copyright (C) 2003 Nathan Hurst
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noREPR_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "xml/repr.h"
#include "xml/text-node.h"
#include "xml/element-node.h"
#include "xml/comment-node.h"
#include "xml/simple-document.h"

using Inkscape::Util::SharedCStringPtr;

/// Returns new node.
Inkscape::XML::Node *
sp_repr_new(gchar const *name)
{
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(*name != '\0', NULL);

    return new Inkscape::XML::ElementNode(g_quark_from_string(name));
}

/// Returns new textnode with content. See Inkscape::XML::TextNode.
Inkscape::XML::Node *
sp_repr_new_text(gchar const *content)
{
    g_return_val_if_fail(content != NULL, NULL);
    return new Inkscape::XML::TextNode(SharedCStringPtr::copy(content));
}

/// Returns new commentnode with comment. See Inkscape::XML::CommentNode.
Inkscape::XML::Node *
sp_repr_new_comment(gchar const *comment)
{
    g_return_val_if_fail(comment != NULL, NULL);
    return new Inkscape::XML::CommentNode(SharedCStringPtr::copy(comment));
}

/// Returns new document having as first child a node named rootname.
Inkscape::XML::Document *
sp_repr_document_new(char const *rootname)
{
    Inkscape::XML::Document *doc = new Inkscape::XML::SimpleDocument(g_quark_from_static_string("xml"));
    if (!strcmp(rootname, "svg:svg")) {
        doc->setAttribute("version", "1.0");
        doc->setAttribute("standalone", "no");
        Inkscape::XML::Node *comment = sp_repr_new_comment(" Created with Inkscape (http://www.inkscape.org/) ");
        doc->appendChild(comment);
        Inkscape::GC::release(comment);
    }

    Inkscape::XML::Node *root = sp_repr_new(rootname);
    doc->appendChild(root);
    Inkscape::GC::release(root);

    return doc;
}

/// Returns new document having reprs as first child.
Inkscape::XML::Document *
sp_repr_document_new_list(GSList *reprs)
{
    g_assert(reprs != NULL);

    Inkscape::XML::Document *doc = sp_repr_document_new("void");
    doc->removeChild(doc->firstChild());

    for ( GSList *iter = reprs ; iter ; iter = iter->next ) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) iter->data;
        doc->appendChild(repr);
    }

    g_assert(sp_repr_document_root(doc) != NULL);

    return doc;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

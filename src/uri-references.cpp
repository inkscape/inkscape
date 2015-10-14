/**
 * Helper methods for resolving URI References
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iostream>
#include <cstring>
#include <string>

#include "document.h"
#include "sp-object.h"
#include "uri.h"
#include "uri-references.h"
#include "extract-uri.h"

#include <glibmm/miscutils.h>
#include <sigc++/functors/mem_fun.h>

namespace Inkscape {

URIReference::URIReference(SPObject *owner)
    : _owner(owner)
    , _owner_document(NULL)
    , _obj(NULL)
    , _uri(NULL)
{
    g_assert(_owner != NULL);
    /* FIXME !!! attach to owner's destroy signal to clean up in case */
}

URIReference::URIReference(SPDocument *owner_document)
    : _owner(NULL)
    , _owner_document(owner_document)
    , _obj(NULL)
    , _uri(NULL)
{
    g_assert(_owner_document != NULL);
}

URIReference::~URIReference() { detach(); }

/*
 * The main ideas here are:
 * (1) "If we are inside a clone, then we can accept if and only if our "original thing" can accept the reference"
 * (this caused problems when there are clones because a change in ids triggers signals for the object hrefing this id,
 *but also its cloned reprs
 * (descendants of <use> referencing an ancestor of the href'ing object)). The way it is done here is *atrocious*, but i
 *could not find a better way.
 * FIXME: find a better and safer way to find the "original object" of anyone with the flag ->cloned
 *
 * (2) Once we have an (potential owner) object, it can accept a href to obj, iff the graph of objects where directed
 *edges are
 * either parent->child relations , *** or href'ing to href'ed *** relations, stays acyclic.
 * We can go either from owner and up in the tree, or from obj and down, in either case this will be in the worst case
 *linear in the number of objects.
 * There are no easy objects allowing to do the second proposition, while "hrefList" is a "list of objects href'ing us",
 *so we'll take this.
 * Then we keep a set of already visited elements, and do a DFS on this graph. if we find obj, then BOOM.
 */

bool URIReference::_acceptObject(SPObject *obj) const
{
    // we go back following hrefList and parent to find if the object already references ourselves indirectly
    std::set<SPObject *> done;
    SPObject *owner = getOwner();
    if (!owner)
        return true;
    while (owner->cloned) {
        std::vector<int> positions;
        while (owner->cloned) {
            int position = 0;
            SPObject *c = owner->parent->firstChild();
            while (c != owner && dynamic_cast<SPObject *>(c)) {
                position++;
                c = c->next;
            }
            positions.push_back(position);
            owner = owner->parent;
        }
        owner = ((SPUse *)owner)->get_original();
        for (int i = positions.size() - 2; i >= 0; i--)
            owner = owner->childList(false)[positions[i]];
    }
    // once we have the "original" object (hopefully) we look at who is referencing it
    if (obj == owner)
        return false;
    std::list<SPObject *> todo(owner->hrefList);
    todo.push_front(owner->parent);
    while (!todo.empty()) {
        SPObject *e = todo.front();
        todo.pop_front();
        if (!dynamic_cast<SPObject *>(e))
            continue;
        if (done.insert(e).second) {
            if (e == obj) {
                return false;
            }
            todo.push_front(e->parent);
            todo.insert(todo.begin(), e->hrefList.begin(), e->hrefList.end());
        }
    }
    return true;
}



void URIReference::attach(const URI &uri) throw(BadURIException)
{
    SPDocument *document = NULL;

    // Attempt to get the document that contains the URI
    if (_owner) {
        document = _owner->document;
    } else if (_owner_document) {
        document = _owner_document;
    }

    // createChildDoc() assumes that the referenced file is an SVG.
    // PNG and JPG files are allowed (in the case of feImage).
    gchar *filename = uri.toString();
    bool skip = false;
    if (g_str_has_suffix(filename, ".jpg") || g_str_has_suffix(filename, ".JPG") ||
        g_str_has_suffix(filename, ".png") || g_str_has_suffix(filename, ".PNG")) {
        skip = true;
    }

    // The path contains references to separate document files to load.
    if (document && uri.getPath() && !skip) {
        std::string base = document->getBase() ? document->getBase() : "";
        std::string path = uri.getFullPath(base);
        if (!path.empty()) {
            document = document->createChildDoc(path);
        } else {
            document = NULL;
        }
    }
    if (!document) {
        g_warning("Can't get document for referenced URI: %s", filename);
        g_free(filename);
        return;
    }
    g_free(filename);

    gchar const *fragment = uri.getFragment();
    if (!uri.isRelative() || uri.getQuery() || !fragment) {
        throw UnsupportedURIException();
    }

    /* FIXME !!! real xpointer support should be delegated to document */
    /* for now this handles the minimal xpointer form that SVG 1.0
     * requires of us
     */
    gchar *id = NULL;
    if (!strncmp(fragment, "xpointer(", 9)) {
        /* FIXME !!! this is wasteful */
        /* FIXME: It looks as though this is including "))" in the id.  I suggest moving
           the strlen calculation and validity testing to before strdup, and copying just
           the id without the "))".  -- pjrm */
        if (!strncmp(fragment, "xpointer(id(", 12)) {
            id = g_strdup(fragment + 12);
            size_t const len = strlen(id);
            if (len < 3 || strcmp(id + len - 2, "))")) {
                g_free(id);
                throw MalformedURIException();
            }
        } else {
            throw UnsupportedURIException();
        }
    } else {
        id = g_strdup(fragment);
    }

    /* FIXME !!! validate id as an NCName somewhere */

    _connection.disconnect();
    delete _uri;
    _uri = new URI(uri);

    _setObject(document->getObjectById(id));
    _connection = document->connectIdChanged(id, sigc::mem_fun(*this, &URIReference::_setObject));

    g_free(id);
}

void URIReference::detach()
{
    _connection.disconnect();
    delete _uri;
    _uri = NULL;
    _setObject(NULL);
}

void URIReference::_setObject(SPObject *obj)
{
    if (obj && !_acceptObject(obj)) {
        obj = NULL;
    }

    if (obj == _obj)
        return;

    SPObject *old_obj = _obj;
    _obj = obj;

    _release_connection.disconnect();
    if (_obj) {
        sp_object_href(_obj, _owner);
        _release_connection = _obj->connectRelease(sigc::mem_fun(*this, &URIReference::_release));
    }
    _changed_signal.emit(old_obj, _obj);
    if (old_obj) {
        /* release the old object _after_ the signal emission */
        sp_object_hunref(old_obj, _owner);
    }
}

/* If an object is deleted, current semantics require that we release
 * it on its "release" signal, rather than later, when its ID is actually
 * unregistered from the document.
 */
void URIReference::_release(SPObject *obj)
{
    g_assert(_obj == obj);
    _setObject(NULL);
}

} /* namespace Inkscape */



SPObject *sp_css_uri_reference_resolve(SPDocument *document, const gchar *uri)
{
    SPObject *ref = NULL;

    if (document && uri && (strncmp(uri, "url(", 4) == 0)) {
        gchar *trimmed = extract_uri(uri);
        if (trimmed) {
            ref = sp_uri_reference_resolve(document, trimmed);
            g_free(trimmed);
        }
    }

    return ref;
}

SPObject *sp_uri_reference_resolve(SPDocument *document, const gchar *uri)
{
    SPObject *ref = NULL;

    if (uri && (*uri == '#')) {
        ref = document->getObjectById(uri + 1);
    }

    return ref;
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

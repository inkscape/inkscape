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
    : _owner(owner), _owner_document(NULL), _obj(NULL), _uri(NULL)
{
    g_assert(_owner != NULL);
    /* FIXME !!! attach to owner's destroy signal to clean up in case */
}

URIReference::URIReference(SPDocument *owner_document)
    : _owner(NULL), _owner_document(owner_document), _obj(NULL), _uri(NULL)
{
    g_assert(_owner_document != NULL);
}

URIReference::~URIReference()
{
    detach();
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
    if( g_str_has_suffix( filename, ".jpg" ) ||
        g_str_has_suffix( filename, ".JPG" ) ||
        g_str_has_suffix( filename, ".png" ) ||
        g_str_has_suffix( filename, ".PNG" ) ) {
        skip = true;
    }
 
    // The path contains references to seperate document files to load.
    if(document && uri.getPath() && !skip ) {
        std::string base = document->getBase() ? document->getBase() : "";
        std::string path = uri.getFullPath(base);
        if(!path.empty()) {
            document = document->createChildDoc(path);
        } else {
            document = NULL;
        }
    }
    if(!document) {
        g_warning("Can't get document for referenced URI: %s", filename);
        g_free( filename );
        return;
    }
    g_free( filename );

    gchar const *fragment = uri.getFragment();
    if ( !uri.isRelative() || uri.getQuery() || !fragment ) {
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
            id = g_strdup(fragment+12);
            size_t const len = strlen(id);
            if ( len < 3 || strcmp(id+len-2, "))") ) {
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
    if ( obj && !_acceptObject(obj) ) {
        obj = NULL;
    }

    if ( obj == _obj ) return;

    SPObject *old_obj=_obj;
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
    g_assert( _obj == obj );
    _setObject(NULL);
}

} /* namespace Inkscape */



SPObject* sp_css_uri_reference_resolve( SPDocument *document, const gchar *uri )
{
    SPObject* ref = NULL;

    if ( document && uri && ( strncmp(uri, "url(", 4) == 0 ) ) {
        gchar *trimmed = extract_uri( uri );
        if ( trimmed ) {
            ref = sp_uri_reference_resolve( document, trimmed );
            g_free( trimmed );
        }
    }

    return ref;
}

SPObject *
sp_uri_reference_resolve (SPDocument *document, const gchar *uri)
{
    SPObject* ref = NULL;

    if ( uri && (*uri == '#') ) {
        ref = document->getObjectById( uri + 1 );
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

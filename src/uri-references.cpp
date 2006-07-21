#define __SP_URI_REFERENCES_C__

/*
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

#include "document.h"
#include "sp-object.h"
#include "uri.h"
#include "uri-references.h"

#include <sigc++/functors/mem_fun.h>

static gchar *uri_to_id(SPDocument *document, const gchar *uri);

namespace Inkscape {

URIReference::URIReference(SPObject *owner)
: _owner(owner), _obj(NULL), _uri(NULL)
{
	g_assert(_owner != NULL);
	/* FIXME !!! attach to owner's destroy signal to clean up in case */
}

URIReference::~URIReference() {
	detach();
}

void URIReference::attach(const URI &uri) throw(BadURIException)
{
	SPDocument *document = SP_OBJECT_DOCUMENT(_owner);
	gchar const *fragment = uri.getFragment();
	if ( !uri.isRelative() || uri.getQuery() || !fragment ) {
		throw UnsupportedURIException();
	}

	/* FIXME !!! real xpointer support should be delegated to document */
	/* for now this handles the minimal xpointer form that SVG 1.0
	 * requires of us
	 */
	gchar *id;
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

	if (_uri) {
		delete _uri;
	}
	_uri = new URI(uri);

	_connection.disconnect();
	_setObject(document->getObjectById(id));
	_connection = document->connectIdChanged(id, sigc::mem_fun(*this, &URIReference::_setObject));

	g_free(id);
}

void URIReference::detach() {
	_connection.disconnect();
	delete _uri;
	_uri = NULL;
	_setObject(NULL);
}

void URIReference::_setObject(SPObject *obj) {
	if ( obj && !_acceptObject(obj) ) {
		obj = NULL;
	}

	if ( obj == _obj ) return;

	SPObject *old_obj=_obj;
	_obj = obj;

	if (_obj) {
		sp_object_href(_obj, _owner);
		_release_connection = _obj->connectRelease(sigc::mem_fun(*this, &URIReference::_release));
	}
	_changed_signal.emit(old_obj, _obj);
	if (old_obj) {
		_release_connection.disconnect();
		/* release the old object _after_ the signal emission */
		sp_object_hunref(old_obj, _owner);
	}
}

/* If an object is deleted, current semantics require that we release
 * it on its "release" signal, rather than later, when its ID is actually
 * unregistered from the document.
 */
void URIReference::_release(SPObject *obj) {
	g_assert( _obj == obj );
	_setObject(NULL);
}

} /* namespace Inkscape */

static gchar *
uri_to_id(SPDocument *document, const gchar *uri)
{
	const gchar *e;
	gchar *id;
	gint len;

	g_return_val_if_fail (document != NULL, NULL);

	if (!uri) return NULL;
	/* fixme: xpointer, everything */
	if (strncmp (uri, "url(#", 5)) return NULL;

	e = uri + 5;
	while (*e) {
		if (*e == ')') break;
		if (!isalnum (*e) && (*e != '_') && (*e != '-') && (*e != ':') && (*e != '.')) return NULL;
		e += 1;
		if (!*e) return NULL;
	}

	len = e - uri - 5;
	if (len < 1) return NULL;

	id = (gchar*)g_new(gchar, len + 1);
	memcpy (id, uri + 5, len);
	id[len] = '\0';

	return id;
}

SPObject *
sp_uri_reference_resolve (SPDocument *document, const gchar *uri)
{
	gchar *id;

	id = uri_to_id(document, uri);
	if (!id) return NULL;

	SPObject *ref;
	ref = document->getObjectById(id);
	g_free(id);
	return ref;
}


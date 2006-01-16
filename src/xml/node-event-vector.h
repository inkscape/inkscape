#ifndef SEEN_INKSCAPE_XML_SP_REPR_EVENT_VECTOR
#define SEEN_INKSCAPE_XML_SP_REPR_EVENT_VECTOR

/*
 * Fuzzy DOM-like tree implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski and Frank Felfe
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

#include "xml/node.h"

namespace Inkscape {
namespace XML {

struct NodeEventVector {
	/* Immediate signals */
	void (* child_added) (Node *repr, Node *child, Node *ref, void * data);
	void (* child_removed) (Node *repr, Node *child, Node *ref, void * data);
	void (* attr_changed) (Node *repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);
	void (* content_changed) (Node *repr, const gchar *oldcontent, const gchar *newcontent, void * data);
	void (* order_changed) (Node *repr, Node *child, Node *oldref, Node *newref, void * data);
};

}
}

inline void sp_repr_synthesize_events (Inkscape::XML::Node *repr, const Inkscape::XML::NodeEventVector *vector, void * data) {
	repr->synthesizeEvents(vector, data);
}
                                                                                
inline void sp_repr_add_listener (Inkscape::XML::Node *repr, const Inkscape::XML::NodeEventVector *vector, void * data) {
	repr->addListener(vector, data);
}
inline void sp_repr_remove_listener_by_data (Inkscape::XML::Node *repr, void * data) {
	repr->removeListenerByData(data);
}

#endif

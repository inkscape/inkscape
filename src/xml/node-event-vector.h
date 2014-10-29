/** @file
 * @brief Deprecated structure for a set of callbacks for node state changes
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski and Frank Felfe
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_XML_SP_REPR_EVENT_VECTOR
#define SEEN_INKSCAPE_XML_SP_REPR_EVENT_VECTOR

#include "xml/node.h"

namespace Inkscape {
namespace XML {
struct NodeEventVector;
}
}

/**
 * @brief Generate events corresponding to the node's state
 * @deprecated Use Node::synthesizeEvents(NodeObserver &) instead
 */
inline void sp_repr_synthesize_events (Inkscape::XML::Node *repr, const Inkscape::XML::NodeEventVector *vector, void* data) {
	repr->synthesizeEvents(vector, data);
}
/**
 * @brief Add a set of callbacks for node state changes and its associated data
 * @deprecated Use Node::addObserver() instead
 */                                                                                
inline void sp_repr_add_listener (Inkscape::XML::Node *repr, const Inkscape::XML::NodeEventVector *vector, void* data) {
	repr->addListener(vector, data);
}
/**
 * @brief Remove a set of callbacks based on associated data
 * @deprecated Use Node::removeObserver() instead
 */
inline void sp_repr_remove_listener_by_data (Inkscape::XML::Node *repr, void* data) {
	repr->removeListenerByData(data);
}

namespace Inkscape {
namespace XML {

/**
 * @brief Structure holding callbacks for node state changes
 * @deprecated Derive an observer object from the NodeObserver class instead
 */
struct NodeEventVector {
	/* Immediate signals */
	void (* child_added) (Node *repr, Node *child, Node *ref, void* data);
	void (* child_removed) (Node *repr, Node *child, Node *ref, void* data);
	void (* attr_changed) (Node *repr, char const *key, char const *oldval, char const *newval, bool is_interactive, void* data);
	void (* content_changed) (Node *repr, char const *oldcontent, char const *newcontent, void * data);
	void (* order_changed) (Node *repr, Node *child, Node *oldref, Node *newref, void* data);
};

}
}

#endif

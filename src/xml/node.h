/*
 * Node - interface for XML nodes
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_NODE_H
#define SEEN_INKSCAPE_XML_NODE_H

#include <glib/gtypes.h>
#include "gc-anchored.h"
#include "util/list.h"

namespace Inkscape {
namespace XML {

class AttributeRecord;
class Document;
class NodeEventVector;
class NodeObserver;

enum NodeType {
    DOCUMENT_NODE,
    ELEMENT_NODE,
    TEXT_NODE,
    COMMENT_NODE,
    PI_NODE
};

// careful; GC::Anchored should only appear once in the inheritance
// hierarcy; else there will be leaks
class Node : public Inkscape::GC::Anchored {
public:
    Node() {}
    
    virtual ~Node() {}

    virtual NodeType type() const=0;

    virtual gchar const *name() const=0;
    virtual int code() const=0;
    virtual void setCodeUnsafe(int code)=0;

    virtual Document *document()=0;
    virtual Document const *document() const=0;

    virtual Node *duplicate(Document *doc) const=0;

    virtual Node *root()=0;
    virtual Node const *root() const=0;

    virtual Node *parent()=0;
    virtual Node const *parent() const=0;

    virtual Node *next()=0;
    virtual Node const *next() const=0;

    virtual Node *firstChild()=0;
    virtual Node const *firstChild() const=0;
    virtual Node *lastChild()=0;
    virtual Node const *lastChild() const=0;

    virtual unsigned childCount() const=0;
    virtual Node *nthChild(unsigned index)=0;
    virtual Node const *nthChild(unsigned index) const=0;

    virtual void addChild(Node *child, Node *ref)=0;
    virtual void appendChild(Node *child)=0;
    virtual void removeChild(Node *child)=0;
    virtual void changeOrder(Node *child, Node *ref)=0;

    virtual unsigned position() const=0;
    virtual void setPosition(int pos)=0;

    virtual gchar const *attribute(gchar const *key) const=0;
    virtual void setAttribute(gchar const *key, gchar const *value, bool is_interactive=false)=0;
    virtual bool matchAttributeName(gchar const *partial_name) const=0;

    virtual gchar const *content() const=0;
    virtual void setContent(gchar const *value)=0;

    virtual void mergeFrom(Node const *src, gchar const *key)=0;

    virtual Inkscape::Util::List<AttributeRecord const> attributeList() const=0;

    virtual void synthesizeEvents(NodeEventVector const *vector, void *data)=0;
    virtual void synthesizeEvents(NodeObserver &observer)=0;
    virtual void addObserver(NodeObserver &observer)=0;
    virtual void addListener(NodeEventVector const *vector, void *data)=0;
    virtual void removeObserver(NodeObserver &observer)=0;
    virtual void removeListenerByData(void *data)=0;

    virtual void addSubtreeObserver(NodeObserver &observer)=0;
    virtual void removeSubtreeObserver(NodeObserver &observer)=0;

protected:
    Node(Node const &) : Anchored() {}
};

}
}

#endif
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

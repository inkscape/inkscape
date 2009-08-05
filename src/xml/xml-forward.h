#ifndef __SEEN_XML_FORWARD_H__
#define __SEEN_XML_FORWARD_H__

/** @file
 * @brief Forward declarations for the XML namespace.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

namespace Inkscape {
namespace XML {

/* Copied from the relevant Doxygen page */

struct AttributeRecord;
struct CommentNode;
class  CompositeNodeObserver;
class Document;
class  ElementNode;
class  Event;
class  EventAdd;
class  EventDel;
class  EventChgAttr;
class  EventChgContent;
class  EventChgOrder;
class  InvalidOperationException;
class  LogBuilder;
struct NodeEventVector;
struct NodeSiblingIteratorStrategy;
struct NodeParentIteratorStrategy;
class  NodeObserver;
class  Node;
struct PINode;
class  SimpleDocument;
class  SimpleNode;
class  Subtree;
struct TextNode;

} // namespace XML
} // namespace Inkscape

#endif // __SEEN_XML_FORWARD_H__

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

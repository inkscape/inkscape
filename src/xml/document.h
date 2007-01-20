/*
 * Inkscape::XML::Document - interface for XML documents
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

#ifndef SEEN_INKSCAPE_XML_SP_REPR_DOC_H
#define SEEN_INKSCAPE_XML_SP_REPR_DOC_H

#include "xml/node.h"

namespace Inkscape {
namespace XML {

class Event;
class NodeObserver;

struct Document : virtual public Node {
public:
    virtual NodeObserver *logger()=0;

    virtual bool inTransaction()=0;

    virtual void beginTransaction()=0;
    virtual void rollback()=0;
    virtual void commit()=0;
    virtual Inkscape::XML::Event *commitUndoable()=0;

    virtual Node *createElementNode(char const *name)=0;
    virtual Node *createTextNode(char const *content)=0;
    virtual Node *createCommentNode(char const *content)=0;
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

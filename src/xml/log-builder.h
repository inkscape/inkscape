/*
 * Inkscape::XML::LogBuilder - NodeObserver which builds an event log
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

#ifndef SEEN_INKSCAPE_XML_LOG_BUILDER_H
#define SEEN_INKSCAPE_XML_LOG_BUILDER_H

#include "gc-managed.h"
#include "xml/node-observer.h"

namespace Inkscape {
namespace XML {

class Event;

class LogBuilder {
public:
    LogBuilder() : _log(NULL) {}
    ~LogBuilder() { discard(); }

    void discard();
    Event *detach();

    void addChild(Node &node, Node &child, Node *prev);

    void removeChild(Node &node, Node &child, Node *prev);

    void setChildOrder(Node &node, Node &child,
                       Node *old_prev, Node *new_prev);

    void setContent(Node &node,
                    Util::ptr_shared<char> old_content,
                    Util::ptr_shared<char> new_content);

    void setAttribute(Node &node, GQuark name,
                      Util::ptr_shared<char> old_value,
                      Util::ptr_shared<char> new_value);

private:
    Event *_log;
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

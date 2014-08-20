/**
 * @file
 * Object building an event log.
 */
/* Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#include "xml/log-builder.h"
#include "xml/event.h"
#include "xml/event-fns.h"

namespace Inkscape {
namespace XML {

void LogBuilder::discard() {
    sp_repr_free_log(_log);
    _log = NULL;
}

Event *LogBuilder::detach() {
    Event *log=_log;
    _log = NULL;
    return log;
}

void LogBuilder::addChild(Node &node, Node &child, Node *prev) {
    _log = new Inkscape::XML::EventAdd(&node, &child, prev, _log);
    _log = _log->optimizeOne();
}

void LogBuilder::removeChild(Node &node, Node &child, Node *prev) {
    _log = new Inkscape::XML::EventDel(&node, &child, prev, _log);
    _log = _log->optimizeOne();
}

void LogBuilder::setChildOrder(Node &node, Node &child,
                               Node *old_prev, Node *new_prev)
{
    _log = new Inkscape::XML::EventChgOrder(&node, &child, old_prev, new_prev, _log);
    _log = _log->optimizeOne();
}

void LogBuilder::setContent(Node &node,
                            Util::ptr_shared<char> old_content,
                            Util::ptr_shared<char> new_content)
{
    _log = new Inkscape::XML::EventChgContent(&node, old_content, new_content, _log);
    _log = _log->optimizeOne();
}

void LogBuilder::setAttribute(Node &node, GQuark name,
                              Util::ptr_shared<char> old_value,
                              Util::ptr_shared<char> new_value)
{
    _log = new Inkscape::XML::EventChgAttr(&node, name, old_value, new_value, _log);
    _log = _log->optimizeOne();
}

}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

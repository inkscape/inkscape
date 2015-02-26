/** @file
 * @brief Object building an event log
 */
/* Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_XML_LOG_BUILDER_H
#define SEEN_INKSCAPE_XML_LOG_BUILDER_H

#include "inkgc/gc-managed.h"
#include "xml/node-observer.h"

namespace Inkscape {
namespace XML {

class Event;
class Node;

/**
 * @brief Event log builder
 *
 * This object records all events sent to it via the public methods in an internal event log.
 * Calling detach() then returns the built log. Calling discard() will clear all the events
 * recorded so far.
 */
class LogBuilder {
public:
    LogBuilder() : _log(NULL) {}
    ~LogBuilder() { discard(); }

    /** @name Manipulate the recorded event log
     * @{ */
    /**
     * @brief Clear the internal log
     */
    void discard();
    /**
     * @brief Get the internal event log
     * @return The recorded event chain
     */
    Event *detach();
    /*@}*/

    /** @name Record events in the log
     * @{ */
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
    /*@}*/

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

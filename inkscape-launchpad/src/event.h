#ifndef INKSCAPE_EVENT_H
#define INKSCAPE_EVENT_H

/*
 * Inkscape::Event -- Container for an XML::Event along with some additional information
*                     describing it.
 * 
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <glibmm/ustring.h>

#include "xml/event-fns.h"
#include "verbs.h"

namespace Inkscape {
namespace XML {
class Event;
}
}

namespace Inkscape {

struct Event {
     
    Event(XML::Event *_event, unsigned int _type=SP_VERB_NONE, Glib::ustring _description="")
        : event (_event), type (_type), description (_description)  { }

    virtual ~Event() { sp_repr_free_log (event); }

    XML::Event *event;
    const unsigned int type;
    Glib::ustring description;
};

} // namespace Inkscape

#endif // INKSCAPE_EVENT_H

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

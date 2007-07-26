/**
 * \brief Very basic interface for classes that control attributes
 *
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_ATTR_WIDGET_H
#define INKSCAPE_UI_WIDGET_ATTR_WIDGET_H

#include "attributes.h"
#include "sp-object.h"
#include "xml/node.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class AttrWidget
{
public:
    AttrWidget(const SPAttributeEnum a)
        : _attr(a)
    {}

    virtual ~AttrWidget()
    {}

    virtual Glib::ustring get_as_attribute() const = 0;
    virtual void set_from_attribute(SPObject*) = 0;

protected:
    const gchar* attribute_value(SPObject* o) const
    {
        const gchar* name = (const gchar*)sp_attribute_name(_attr);
        if(name && o) {
            const gchar* val = SP_OBJECT_REPR(o)->attribute(name);
            return val;
        }
        return 0;
    }

private:
    const SPAttributeEnum _attr;
};

}
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

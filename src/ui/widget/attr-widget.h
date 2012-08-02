/*
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *   Rodrigo Kumpera <kumpera@gmail.com>
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

enum DefaultValueType
{
    T_NONE,
    T_DOUBLE,
    T_VECT_DOUBLE,
    T_BOOL,
    T_UINT,
    T_CHARPTR
};

/**
 * Very basic interface for classes that control attributes.
 */
class DefaultValueHolder
{
    DefaultValueType type;
    union {
        double d_val;
        std::vector<double>* vt_val;
        bool b_val;
        unsigned int uint_val;
        char* cptr_val;
    } value;

    //FIXME remove copy ctor and assignment operator as private to avoid double free of the vector
public:
    DefaultValueHolder () {
        type = T_NONE;
    }

    DefaultValueHolder (double d) {
        type = T_DOUBLE;
        value.d_val = d;
    }

    DefaultValueHolder (std::vector<double>* d) {
        type = T_VECT_DOUBLE;
        value.vt_val = d;
    }

    DefaultValueHolder (char* c) {
        type = T_CHARPTR;
        value.cptr_val = c;
    }

    DefaultValueHolder (bool d) {
        type = T_BOOL;
        value.b_val = d;
    }

    DefaultValueHolder (unsigned int ui) {
        type = T_UINT;
        value.uint_val = ui;
    }

    ~DefaultValueHolder() {
        if (type == T_VECT_DOUBLE)
            delete value.vt_val;
    }

    unsigned int as_uint() {
        g_assert (type == T_UINT);
        return value.uint_val;
    }

    bool as_bool() {
        g_assert (type == T_BOOL);
        return value.b_val;
    }

    double as_double() {
        g_assert (type == T_DOUBLE);
        return value.d_val;
    }

    std::vector<double>* as_vector() {
        g_assert (type == T_VECT_DOUBLE);
        return value.vt_val;
    }

    char* as_charptr() {
        g_assert (type == T_CHARPTR);
        return value.cptr_val;
    }
};

class AttrWidget
{
public:
    AttrWidget(const SPAttributeEnum a, unsigned int value)
        : _attr(a),
          _default(value)
    {}

    AttrWidget(const SPAttributeEnum a, double value)
        : _attr(a),
          _default(value)
    {}

    AttrWidget(const SPAttributeEnum a, bool value)
        : _attr(a),
          _default(value)
    {}
    
    AttrWidget(const SPAttributeEnum a, char* value)
        : _attr(a),
          _default(value)
    {}
    
    AttrWidget(const SPAttributeEnum a)
        : _attr(a),
          _default()
    {}

    virtual ~AttrWidget()
    {}

    virtual Glib::ustring get_as_attribute() const = 0;
    virtual void set_from_attribute(SPObject*) = 0;

    SPAttributeEnum get_attribute() const
    {
        return _attr;
    }

    sigc::signal<void>& signal_attr_changed()
    {
        return _signal;
    }
protected:
    DefaultValueHolder* get_default() { return &_default; }
    const gchar* attribute_value(SPObject* o) const
    {
        const gchar* name = (const gchar*)sp_attribute_name(_attr);
        if(name && o) {
            const gchar* val = o->getRepr()->attribute(name);
            return val;
        }
        return 0;
    }

private:
    const SPAttributeEnum _attr;
    DefaultValueHolder _default;
    sigc::signal<void> _signal;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

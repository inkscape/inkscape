/**
 * \brief Simplified management of enumerations of svg items with UI labels
 *
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UTIL_ENUMS_H
#define INKSCAPE_UTIL_ENUMS_H

#include <glibmm/ustring.h>

namespace Inkscape {
namespace Util {

template<typename E> class EnumData
{
public:
    E id;
    const Glib::ustring label;
    const Glib::ustring key;
};

template<typename E> class EnumDataConverter
{
public:
    typedef EnumData<E> Data;

    EnumDataConverter(const EnumData<E>* cd, const int endval)
        : end(endval), _data(cd)
    {}

    E get_id_from_label(const Glib::ustring& label) const
    {
        for(int i = 0; i < end; ++i) {
            if(_data[i].label == label)
                return (E)i;
        }

        return (E)0;
    }

    E get_id_from_key(const Glib::ustring& key) const
    {
        for(int i = 0; i < end; ++i) {
            if(_data[i].key == key)
                return (E)i;
        }

        return (E)0;
    }

    bool is_valid_key(const Glib::ustring& key) const
    {
        for(int i = 0; i < end; ++i) {
            if(_data[i].key == key)
                return true;
        }

        return false;
    }

    const Glib::ustring& get_label(const E e) const
    {
        return _data[e].label;
    }

    const Glib::ustring& get_key(const E e) const
    {
        return _data[e].key;
    }

    const EnumData<E>& data(const int i) const
    {
        return _data[i];
    }

    const int end;
private:
    const EnumData<E>* _data;
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

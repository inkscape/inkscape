/*
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

/**
 * Simplified management of enumerations of svg items with UI labels.
 * IMPORTANT:
 *  When initializing the EnumData struct, you cannot use _(...) to translate strings.
 * Instead, one must use N_(...) and do the translation every time the string is retreived.
 */
template<typename E>
struct EnumData
{
    E id;
    const Glib::ustring label;
    const Glib::ustring key;
};

const Glib::ustring empty_string("");

/**
 * Simplified management of enumerations of svg items with UI labels.
 *
 * @note that get_id_from_key and get_id_from_label return 0 if it cannot find an entry for that key string.
 * @note that get_label and get_key return an empty string when the requested id is not in the list.
 */
template<typename E> class EnumDataConverter
{
public:
    typedef EnumData<E> Data;

    EnumDataConverter(const EnumData<E>* cd, const unsigned int length)
        : _length(length), _data(cd)
    {}

    E get_id_from_label(const Glib::ustring& label) const
    {
        for(unsigned int i = 0; i < _length; ++i) {
            if(_data[i].label == label)
                return _data[i].id;
        }

        return (E)0;
    }

    E get_id_from_key(const Glib::ustring& key) const
    {
        for(unsigned int i = 0; i < _length; ++i) {
            if(_data[i].key == key)
                return _data[i].id;
        }

        return (E)0;
    }

    bool is_valid_key(const Glib::ustring& key) const
    {
        for(unsigned int i = 0; i < _length; ++i) {
            if(_data[i].key == key)
                return true;
        }

        return false;
    }

    bool is_valid_id(const E id) const
    {
        for(unsigned int i = 0; i < _length; ++i) {
            if(_data[i].id == id)
                return true;
        }
        return false;
    }

    const Glib::ustring& get_label(const E id) const
    {
        for(unsigned int i = 0; i < _length; ++i) {
            if(_data[i].id == id)
                return _data[i].label;
        }

        return empty_string;
    }

    const Glib::ustring& get_key(const E id) const
    {
        for(unsigned int i = 0; i < _length; ++i) {
            if(_data[i].id == id)
                return _data[i].key;
        }

        return empty_string;
    }

    const EnumData<E>& data(const unsigned int i) const
    {
        return _data[i];
    }

    const unsigned int _length;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

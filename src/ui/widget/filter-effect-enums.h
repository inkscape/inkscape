/**
 * \brief Simplified management of enumerations for filter effects
 *
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_FILTER_EFFECT_ENUMS_H
#define INKSCAPE_UI_WIDGET_FILTER_EFFECT_ENUMS_H

#include "sp-fecomposite.h"
#include "display/nr-filter-blend.h"
#include "display/nr-filter-types.h"

namespace Inkscape {
namespace UI {
namespace Widget {

template<typename E> struct EnumData
{
    E id;
    const Glib::ustring label;
    const Glib::ustring name;
};

template<typename E> class Converter
{
public:
    typedef EnumData<E> Data;

    Converter(const EnumData<E>* cd, const int endval)
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

    E get_id_from_name(const Glib::ustring& name) const
    {
        for(int i = 0; i < end; ++i) {
            if(_data[i].name == name)
                return (E)i;
        }

        return (E)0;
    }

    const Glib::ustring& get_label(const E e) const
    {
        return _data[e].label;
    }

    const Glib::ustring& get_name(const E e) const
    {
        return _data[e].name;
    }

    const EnumData<E>& data(const int i) const
    {
        return _data[i];
    }

    const int end;
private:
    const EnumData<E>* _data;
};

template<typename E> class ComboBoxEnum : public Gtk::ComboBox
{
public:
    ComboBoxEnum(const Converter<E>& c)
        : _converter(c)
    {
        _model = Gtk::ListStore::create(_columns);
        set_model(_model);

        pack_start(_columns.label);

        // Initialize list
        for(int i = 0; i < _converter.end; ++i) {
            Gtk::TreeModel::Row row = *_model->append();
            const EnumData<E>* data = &_converter.data(i);
            row[_columns.data] = data;
            row[_columns.label] = _converter.get_label(data->id);
        }

        set_active(0);
    }
    
    const EnumData<E>* get_active_data()
    {
        Gtk::TreeModel::iterator i = this->get_active();
        if(i)
            return (*i)[_columns.data];
        return 0;
    }

    void add_row(const Glib::ustring& s)
    {
        Gtk::TreeModel::Row row = *_model->append();
        row[_columns.data] = 0;
        row[_columns.label] = s;
    }
private:
    class Columns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Columns()
        {
            add(data);
            add(label);
        }

        Gtk::TreeModelColumn<const EnumData<E>*> data;
        Gtk::TreeModelColumn<Glib::ustring> label;
    };

    Columns _columns;
    Glib::RefPtr<Gtk::ListStore> _model;
    const Converter<E>& _converter;
};

/*** Filter Primitives ***/
extern const EnumData<NR::FilterPrimitiveType> FPData[NR::NR_FILTER_ENDPRIMITIVETYPE];
extern const Converter<NR::FilterPrimitiveType> FPConverter;

/*** feBlend Mode ***/
extern const EnumData<NR::FilterBlendMode> BlendModeData[NR::BLEND_ENDMODE];
extern const Converter<NR::FilterBlendMode> BlendModeConverter;

/*** feComposite Operator ***/
extern const EnumData<FeCompositeOperator> CompositeOperatorData[COMPOSITE_ENDOPERATOR];
extern const Converter<FeCompositeOperator> CompositeOperatorConverter;

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

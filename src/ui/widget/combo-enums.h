/**
 * \brief Simplified management of enumerations in the UI as combobox.
 *
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_COMBO_ENUMS_H
#define INKSCAPE_UI_WIDGET_COMBO_ENUMS_H

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include "attr-widget.h"
#include "util/enums.h"
#include "ui/widget/labelled.h"

namespace Inkscape {
namespace UI {
namespace Widget {

template<typename E> class ComboBoxEnum : public Gtk::ComboBox, public AttrWidget
{
public:
    ComboBoxEnum(const Util::EnumDataConverter<E>& c, const SPAttributeEnum a = SP_ATTR_INVALID)
        : AttrWidget(a), _converter(c)
    {
        signal_changed().connect(signal_attr_changed().make_slot());

        _model = Gtk::ListStore::create(_columns);
        set_model(_model);

        pack_start(_columns.label);

        // Initialize list
        for(int i = 0; i < _converter.end; ++i) {
            Gtk::TreeModel::Row row = *_model->append();
            const Util::EnumData<E>* data = &_converter.data(i);
            row[_columns.data] = data;
            row[_columns.label] = _( _converter.get_label(data->id).c_str() );
        }

        set_active(0);
    }

    virtual Glib::ustring get_as_attribute() const
    {
        return get_active_data()->key;
    }

    virtual void set_from_attribute(SPObject* o)
    {
        const gchar* val = attribute_value(o);
        if(val)
            set_active(_converter.get_id_from_key(val));
        else
            set_active(0);
    }
    
    const Util::EnumData<E>* get_active_data() const
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

    void set_active_by_id(E id) {
        for(Gtk::TreeModel::iterator i = _model->children().begin();
            i != _model->children().end(); ++i) 
        {
            const Util::EnumData<E>* data = (*i)[_columns.data];
            if(data->id == id) {
                set_active(i);
                break;
            }
        }
    };

    void set_active_by_key(const Glib::ustring& key) {
        set_active_by_id( _converter.get_id_from_key(key) );
    };

private:
    class Columns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Columns()
        {
            add(data);
            add(label);
        }

        Gtk::TreeModelColumn<const Util::EnumData<E>*> data;
        Gtk::TreeModelColumn<Glib::ustring> label;
    };

    Columns _columns;
    Glib::RefPtr<Gtk::ListStore> _model;
    const Util::EnumDataConverter<E>& _converter;
};


template<typename E> class LabelledComboBoxEnum : public Labelled
{
public:
    LabelledComboBoxEnum( Glib::ustring const &label,
                          Glib::ustring const &tooltip,
                          const Util::EnumDataConverter<E>& c,
                          Glib::ustring const &suffix = "",
                          Glib::ustring const &icon = "",
                          bool mnemonic = true)
        : Labelled(label, tooltip, new ComboBoxEnum<E>(c), suffix, icon, mnemonic)
    { }

    ComboBoxEnum<E>* getCombobox() {
        return static_cast< ComboBoxEnum<E>* > (_widget);
    }
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

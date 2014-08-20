/*
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

#include "ui/widget/labelled.h"
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include "attr-widget.h"
#include "util/enums.h"

#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Simplified management of enumerations in the UI as combobox.
 */
template<typename E> class ComboBoxEnum : public Gtk::ComboBox, public AttrWidget
{
private:
    int on_sort_compare( const Gtk::TreeModel::iterator & a, const Gtk::TreeModel::iterator & b)
    {
        Glib::ustring an=(*a)[_columns.label];
        Glib::ustring bn=(*b)[_columns.label];
        return an.compare(bn);
    }

    bool _sort;

public:
    ComboBoxEnum(E default_value, const Util::EnumDataConverter<E>& c, const SPAttributeEnum a = SP_ATTR_INVALID, bool sort = true)
        : AttrWidget(a, (unsigned int)default_value), setProgrammatically(false), _converter(c)
    {
        _sort = sort;

        signal_changed().connect(signal_attr_changed().make_slot());

        _model = Gtk::ListStore::create(_columns);
        set_model(_model);

        pack_start(_columns.label);

        // Initialize list
        for(int i = 0; i < static_cast<int>(_converter._length); ++i) {
            Gtk::TreeModel::Row row = *_model->append();
            const Util::EnumData<E>* data = &_converter.data(i);
            row[_columns.data] = data;
            row[_columns.label] = _( _converter.get_label(data->id).c_str() );
        }
        set_active_by_id(default_value);

        // Sort the list
        if (sort) {
            _model->set_default_sort_func(sigc::mem_fun(*this, &ComboBoxEnum<E>::on_sort_compare));
            _model->set_sort_column(_columns.label, Gtk::SORT_ASCENDING);
        }
    }

    ComboBoxEnum(const Util::EnumDataConverter<E>& c, const SPAttributeEnum a = SP_ATTR_INVALID, bool sort = true)
        : AttrWidget(a, (unsigned int) 0), setProgrammatically(false), _converter(c)
    {
        _sort = sort;

        signal_changed().connect(signal_attr_changed().make_slot());

        _model = Gtk::ListStore::create(_columns);
        set_model(_model);

        pack_start(_columns.label);

        // Initialize list
        for(unsigned int i = 0; i < _converter._length; ++i) {
            Gtk::TreeModel::Row row = *_model->append();
            const Util::EnumData<E>* data = &_converter.data(i);
            row[_columns.data] = data;
            row[_columns.label] = _( _converter.get_label(data->id).c_str() );
        }
        set_active(0);

        // Sort the list
        if (_sort) {
            _model->set_default_sort_func(sigc::mem_fun(*this, &ComboBoxEnum<E>::on_sort_compare));
            _model->set_sort_column(_columns.label, Gtk::SORT_ASCENDING);
        }
    }

    virtual Glib::ustring get_as_attribute() const
    {
        return get_active_data()->key;
    }

    virtual void set_from_attribute(SPObject* o)
    {
        setProgrammatically = true;
        const gchar* val = attribute_value(o);
        if(val)
            set_active_by_id(_converter.get_id_from_key(val));
        else
            set_active(get_default()->as_uint());
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

    void remove_row(E id) {
        Gtk::TreeModel::iterator i;
        
        for(i = _model->children().begin(); i != _model->children().end(); ++i) {
            const Util::EnumData<E>* data = (*i)[_columns.data];

            if(data->id == id)
                break;
        }

        if(i != _model->children().end())
            _model->erase(i);
    }

    void set_active_by_id(E id) {
        setProgrammatically = true;
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
        setProgrammatically = true;
        set_active_by_id( _converter.get_id_from_key(key) );
    };

    bool setProgrammatically;

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


/**
 * Simplified management of enumerations in the UI as combobox.
 */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

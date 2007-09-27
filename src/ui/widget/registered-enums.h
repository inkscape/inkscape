/**
 * \brief Simplified management of enumerations in the UI as combobox.
 *
 * Authors:
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_REGISTERED_ENUMS_H
#define INKSCAPE_UI_WIDGET_REGISTERED_ENUMS_H

#include "ui/widget/combo-enums.h"
#include "ui/widget/registered-widget.h"

namespace Inkscape {
namespace UI {
namespace Widget {

template<typename E> class RegisteredEnum : public RegisteredWidget
{
public:
    RegisteredEnum() {
        labelled = NULL;
    }

    ~RegisteredEnum() {
        _changed_connection.disconnect();
        if (labelled)
            delete labelled;
    }

    void init ( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                const Util::EnumDataConverter<E>& c,
                Registry& wr,
                Inkscape::XML::Node* repr_in,
                SPDocument *doc_in)
    {
        init_parent(key, wr, repr_in, doc_in);

        labelled = new LabelledComboBoxEnum<E> (label, tip, c);

        _changed_connection = combobox()->signal_changed().connect (sigc::mem_fun (*this, &RegisteredEnum::on_changed));
    }

    inline void init ( const Glib::ustring& label,
                       const Glib::ustring& key,
                       Registry& wr)
    {
        init(label, key, wr, NULL, NULL);
    }

    void set_active_by_id (E id) {
        combobox()->set_active_by_id(id);
    };

    void set_active_by_key (const Glib::ustring& key) {
        combobox()->set_active_by_key(key);
    }

    inline const Util::EnumData<E>* get_active_data() {
        return combobox()->get_active_data();
    }

    ComboBoxEnum<E> * combobox() {
        if (labelled) {
            return labelled->getCombobox();
        } else {
            return NULL;
        }
    }

    LabelledComboBoxEnum<E> * labelled;
    sigc::connection _changed_connection;

protected:
    void on_changed() {
        if (combobox()->setProgrammatically) {
            combobox()->setProgrammatically = false;
            return;
        }

        if (_wr->isUpdating())
            return;
        _wr->setUpdating (true);

        const Util::EnumData<E>* data = combobox()->get_active_data();
        if (data) {
            write_to_xml(data->key.c_str());
        }

        _wr->setUpdating (false);
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

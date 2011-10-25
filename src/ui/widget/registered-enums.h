/*
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

/**
 * Simplified management of enumerations in the UI as combobox.
 */
template<typename E> class RegisteredEnum : public RegisteredWidget< LabelledComboBoxEnum<E> >
{
public:
    virtual ~RegisteredEnum() {
        _changed_connection.disconnect();
    }

    RegisteredEnum ( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                const Util::EnumDataConverter<E>& c,
                Registry& wr,
                Inkscape::XML::Node* repr_in = NULL,
                SPDocument *doc_in = NULL )
        : RegisteredWidget< LabelledComboBoxEnum<E> >(label, tip, c)
    {
        RegisteredWidget< LabelledComboBoxEnum<E> >::init_parent(key, wr, repr_in, doc_in);

        _changed_connection = combobox()->signal_changed().connect (sigc::mem_fun (*this, &RegisteredEnum::on_changed));
    }

    void set_active_by_id (E id) {
        combobox()->set_active_by_id(id);
    };

    void set_active_by_key (const Glib::ustring& key) {
        combobox()->set_active_by_key(key);
    }

    inline const Util::EnumData<E>* get_active_data() {
        combobox()->get_active_data();
    }

    ComboBoxEnum<E> * combobox() {
        return LabelledComboBoxEnum<E>::getCombobox();
    }

    sigc::connection _changed_connection;

protected:
    void on_changed() {
        if (combobox()->setProgrammatically) {
            combobox()->setProgrammatically = false;
            return;
        }

        if (RegisteredWidget< LabelledComboBoxEnum<E> >::_wr->isUpdating())
            return;

        RegisteredWidget< LabelledComboBoxEnum<E> >::_wr->setUpdating (true);

        const Util::EnumData<E>* data = combobox()->get_active_data();
        if (data) {
            RegisteredWidget< LabelledComboBoxEnum<E> >::write_to_xml(data->key.c_str());
        }

        RegisteredWidget< LabelledComboBoxEnum<E> >::_wr->setUpdating (false);
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

#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_ENUM_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_ENUM_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

#include "ui/widget/registry.h"
#include "ui/widget/registered-enums.h"
#include <gtkmm/tooltips.h>

#include "live_effects/parameter/parameter.h"
#include "verbs.h"

namespace Inkscape {

namespace LivePathEffect {

template<typename E> class EnumParam : public Parameter {
public:
    EnumParam(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                const Util::EnumDataConverter<E>& c,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                E defvalue)
        : Parameter(label, tip, key, wr, effect)
    {
        regenum = NULL;
        enumdataconv = &c;
        value = defvalue;
    };
    ~EnumParam() {
        if (regenum)
            delete regenum;
    };

    Gtk::Widget * param_getWidget() {
        if (!regenum) {
            regenum = new Inkscape::UI::Widget::RegisteredEnum<E>();
            regenum->init(param_label, param_tooltip, param_key, *enumdataconv, *param_wr, param_effect->getRepr(), param_effect->getSPDoc());
            regenum->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change enum parameter"));
            regenum->combobox()->set_active_by_id(value);
        }
        return dynamic_cast<Gtk::Widget *> (regenum->labelled);
    };

    bool param_readSVGValue(const gchar * strvalue) {
        if (!strvalue) return false;

        value = enumdataconv->get_id_from_key(Glib::ustring(strvalue));

        if (regenum)
            regenum->combobox()->set_active_by_id(value);

        return true;
    };
    gchar * param_writeSVGValue() const {
        gchar * str = g_strdup( enumdataconv->get_key(value).c_str() );
        return str;
    };

    E get_value() const {
        return value;
    }

private:
    EnumParam(const EnumParam&);
    EnumParam& operator=(const EnumParam&);

    UI::Widget::RegisteredEnum<E> * regenum;
    E value;

    const Util::EnumDataConverter<E> * enumdataconv;
};


}; //namespace LivePathEffect

}; //namespace Inkscape

#endif

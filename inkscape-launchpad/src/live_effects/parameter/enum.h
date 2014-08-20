#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_ENUM_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_ENUM_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-enums.h"
#include <glibmm/ustring.h>
#include "live_effects/effect.h"
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
                E default_value)
        : Parameter(label, tip, key, wr, effect)
    {
        enumdataconv = &c;
        defvalue = default_value;
        value = defvalue;
    };

    virtual ~EnumParam() { };

    virtual Gtk::Widget * param_newWidget() {
        Inkscape::UI::Widget::RegisteredEnum<E> *regenum = Gtk::manage ( 
            new Inkscape::UI::Widget::RegisteredEnum<E>( param_label, param_tooltip,
                       param_key, *enumdataconv, *param_wr, param_effect->getRepr(), param_effect->getSPDoc() ) );

        regenum->set_active_by_id(value);
        regenum->combobox()->setProgrammatically = false;
        regenum->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change enumeration parameter"));

        return dynamic_cast<Gtk::Widget *> (regenum);
    };

    bool param_readSVGValue(const gchar * strvalue) {
        if (!strvalue) {
            param_set_default();
            return true;
        }

        param_set_value( enumdataconv->get_id_from_key(Glib::ustring(strvalue)) );

        return true;
    };
    gchar * param_getSVGValue() const {
        gchar * str = g_strdup( enumdataconv->get_key(value).c_str() );
        return str;
    };

    E get_value() const {
        return value;
    }

    inline operator E() const {
        return value;
    };

    void param_set_default() {
        param_set_value(defvalue);
    }

    void param_set_value(E val) {
        value = val;
    }

private:
    EnumParam(const EnumParam&);
    EnumParam& operator=(const EnumParam&);

    E value;
    E defvalue;

    const Util::EnumDataConverter<E> * enumdataconv;
};


}; //namespace LivePathEffect

}; //namespace Inkscape

#endif

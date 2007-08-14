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
                Inkscape::UI::Widget::Registry* wr, Effect* effect)
        : Parameter(label, tip, key, wr, effect)
    {
        regenum = NULL;
        enumdataconv = &c;
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
        }
        return dynamic_cast<Gtk::Widget *> (regenum->labelled);
    };

    bool param_readSVGValue(const gchar * strvalue) {
        if (regenum)
            regenum->combobox()->set_active_by_key(Glib::ustring(strvalue));
        return true;
    };
    gchar * param_writeSVGValue() const {
        if (regenum) {
            gchar * str = g_strdup(regenum->combobox()->get_active_data()->key.c_str());
            return str;
        } else {
            return NULL;
        }
    };

    const Util::EnumData<E>* get_selected_data() {
        if (regenum) {
            return regenum->combobox()->get_active_data();
        } else {
            return NULL;
        }
    };

private:
    EnumParam(const EnumParam&);
    EnumParam& operator=(const EnumParam&);

    UI::Widget::RegisteredEnum<E> * regenum;

    const Util::EnumDataConverter<E> * enumdataconv;
};


}; //namespace LivePathEffect

}; //namespace Inkscape

#endif

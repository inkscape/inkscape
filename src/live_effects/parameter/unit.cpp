#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_UNIT_CPP

/*
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/unit.h"
#include "live_effects/effect.h"
#include "ui/widget/registered-widget.h"

namespace Inkscape {

namespace LivePathEffect {


UnitParam::UnitParam( const Glib::ustring& label, const Glib::ustring& tip,
                              const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                              Effect* effect, SPUnitId default_value)
    : Parameter(label, tip, key, wr, effect)
{
    defunit = &sp_unit_get_by_id(default_value);;
    unit = defunit;
}

UnitParam::~UnitParam()
{
}

bool
UnitParam::param_readSVGValue(const gchar * strvalue)
{
    SPUnit const *newval = sp_unit_get_by_abbreviation(strvalue);
    if (newval) {
        param_set_value(newval);
        return true;
    }
    return false;
}

gchar *
UnitParam::param_getSVGValue() const
{
    return g_strdup(sp_unit_get_abbreviation(unit));
}

void
UnitParam::param_set_default()
{
    param_set_value(defunit);
}

void
UnitParam::param_set_value(SPUnit const *val)
{
    unit = val;
}

const gchar *
UnitParam::get_abbreviation()
{
    return sp_unit_get_abbreviation(unit);
}

Gtk::Widget *
UnitParam::param_newWidget(Gtk::Tooltips * /*tooltips*/)
{
    Inkscape::UI::Widget::RegisteredUnitMenu* unit_menu = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredUnitMenu(param_label,
                                                     param_key,
                                                     *param_wr,
                                                     param_effect->getRepr(),
                                                     param_effect->getSPDoc()));

    unit_menu->setUnit(unit);
    unit_menu->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change unit parameter"));

    return dynamic_cast<Gtk::Widget *> (unit_menu);
}

} /* namespace LivePathEffect */
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

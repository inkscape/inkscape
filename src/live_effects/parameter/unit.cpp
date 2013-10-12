/*
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include <glibmm/i18n.h>

#include "live_effects/parameter/unit.h"
#include "live_effects/effect.h"
#include "verbs.h"
#include "util/units.h"

using Inkscape::Util::unit_table;

namespace Inkscape {

namespace LivePathEffect {


UnitParam::UnitParam( const Glib::ustring& label, const Glib::ustring& tip,
                              const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                              Effect* effect, Glib::ustring default_unit)
    : Parameter(label, tip, key, wr, effect)
{
    defunit = unit_table.getUnit(default_unit);
    unit = defunit;
}

UnitParam::~UnitParam()
{
}

bool
UnitParam::param_readSVGValue(const gchar * strvalue)
{
    if (strvalue) {
        param_set_value(*unit_table.getUnit(strvalue));
        return true;
    }
    return false;
}

gchar *
UnitParam::param_getSVGValue() const
{
    return g_strdup(unit->abbr.c_str());
}

void
UnitParam::param_set_default()
{
    param_set_value(*defunit);
}

void
UnitParam::param_set_value(Inkscape::Util::Unit const &val)
{
    unit = new Inkscape::Util::Unit(val);
}

const gchar *
UnitParam::get_abbreviation() const
{
    return unit->abbr.c_str();
}

Gtk::Widget *
UnitParam::param_newWidget()
{
    Inkscape::UI::Widget::RegisteredUnitMenu* unit_menu = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredUnitMenu(param_label,
                                                     param_key,
                                                     *param_wr,
                                                     param_effect->getRepr(),
                                                     param_effect->getSPDoc()));

    unit_menu->setUnit(unit->abbr);
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

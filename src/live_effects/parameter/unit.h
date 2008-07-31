#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_UNIT_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_UNIT_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/parameter.h"
#include <helper/units.h>

namespace Inkscape {

namespace LivePathEffect {

class UnitParam : public Parameter {
public:
    UnitParam(const Glib::ustring& label,
		  const Glib::ustring& tip,
		  const Glib::ustring& key, 
		  Inkscape::UI::Widget::Registry* wr,
		  Effect* effect,
		  SPUnitId default_value = SP_UNIT_PX);
    virtual ~UnitParam();

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;
    virtual void param_set_default();
    void param_set_value(SPUnit const *val);
    const gchar *get_abbreviation();

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    operator SPUnit const *() { return unit; }

private:
    SPUnit const *unit;
    SPUnit const *defunit;

    UnitParam(const UnitParam&);
    UnitParam& operator=(const UnitParam&);
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

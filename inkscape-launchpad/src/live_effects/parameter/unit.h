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

namespace Inkscape {

namespace Util {
    class Unit;
}

namespace LivePathEffect {

class UnitParam : public Parameter {
public:
    UnitParam(const Glib::ustring& label,
		  const Glib::ustring& tip,
		  const Glib::ustring& key, 
		  Inkscape::UI::Widget::Registry* wr,
		  Effect* effect,
		  Glib::ustring default_unit = "px");
    virtual ~UnitParam();

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;
    virtual void param_set_default();
    void param_set_value(Inkscape::Util::Unit const &val);
    const gchar *get_abbreviation() const;

    virtual Gtk::Widget * param_newWidget();

    operator Inkscape::Util::Unit const *() const { return unit; }

private:
    Inkscape::Util::Unit const *unit;
    Inkscape::Util::Unit const *defunit;

    UnitParam(const UnitParam&);
    UnitParam& operator=(const UnitParam&);
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

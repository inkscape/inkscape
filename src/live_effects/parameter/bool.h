#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_BOOL_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_BOOL_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"

#include "live_effects/parameter/parameter.h"

namespace Inkscape {

namespace LivePathEffect {


class BoolParam : public Parameter {
public:
    BoolParam( const Glib::ustring& label,
               const Glib::ustring& tip,
               const Glib::ustring& key,
               Inkscape::UI::Widget::Registry* wr,
               Effect* effect,
               bool default_value = false);
    ~BoolParam();

    Gtk::Widget * param_getWidget();

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    void param_setValue(bool newvalue);
    void param_set_default();

    bool get_value() { return value; };

private:
    BoolParam(const BoolParam&);
    BoolParam& operator=(const BoolParam&);

    Inkscape::UI::Widget::RegisteredCheckButton * checkwdg;

    bool value;
    bool defvalue;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

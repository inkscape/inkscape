#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_BOOL_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_BOOL_H

/*
 * Inkscape::LivePathEffectParameters
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

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
               bool default_value = false,
               bool no_widget = false);
    virtual ~BoolParam();

    virtual Gtk::Widget * param_newWidget();

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;

    void param_setValue(bool newvalue);
    virtual void param_set_default();

    bool get_value() const { return value; };

    inline operator bool() const { return value; };

private:
    BoolParam(const BoolParam&);
    BoolParam& operator=(const BoolParam&);

    bool value;
    bool defvalue;
    bool hide_widget;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

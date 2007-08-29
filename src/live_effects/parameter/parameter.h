#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include <2geom/point.h>
#include <2geom/path.h>

#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"

namespace Gtk {
    class Widget;
}

namespace Inkscape {

namespace LivePathEffect {

class Effect;

class Parameter {
public:
    Parameter(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect);
    virtual ~Parameter() {};

    virtual bool param_readSVGValue(const gchar * strvalue) = 0;   // returns true if new value is valid / accepted.
    virtual gchar * param_writeSVGValue() const = 0;

    virtual void param_set_default() = 0;

    // This returns pointer to the parameter's widget to be put in the live-effects dialog. Must also create the
    // necessary widget if it does not exist yet.
    virtual Gtk::Widget * param_getWidget() = 0;
    virtual Glib::ustring * param_getTooltip() { return &param_tooltip; };

    Glib::ustring param_key;
    Inkscape::UI::Widget::Registry * param_wr;
    Glib::ustring param_label;

protected:
    Glib::ustring param_tooltip;

    Effect* param_effect;

private:
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);
};


class ScalarParam : public Parameter {
public:
    ScalarParam(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key, 
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                gdouble default_value = 1.0);
    ~ScalarParam();

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    void param_set_default();
    void param_set_value(gdouble val);
    void param_make_integer(bool yes = true);
    void param_set_range(gdouble min, gdouble max);

    Gtk::Widget * param_getWidget();

    inline operator gdouble()
        { return value; };

private:
    ScalarParam(const ScalarParam&);
    ScalarParam& operator=(const ScalarParam&);

    gdouble value;
    gdouble min;
    gdouble max;
    bool integer;
    gdouble defvalue;
    Inkscape::UI::Widget::RegisteredScalar * rsu;
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

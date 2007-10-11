#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_POINT_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_POINT_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>
#include <2geom/point.h>

#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"
#include <gtkmm/tooltips.h>

#include "live_effects/parameter/parameter.h"

struct SPKnot;

namespace Inkscape {

namespace LivePathEffect {


class PointParam : public Geom::Point, public Parameter {
public:
    PointParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                Geom::Point default_value = Geom::Point(0,0));
    ~PointParam();

    Gtk::Widget * param_getWidget();

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    void param_setValue(Geom::Point newpoint);
    void param_set_default();

private:
    PointParam(const PointParam&);
    PointParam& operator=(const PointParam&);

    Gtk::Widget * _widget;
    Gtk::Tooltips * _tooltips;
    Inkscape::UI::Widget::RegisteredPoint * pointwdg;
    void on_button_click();

    SPKnot *knot;

    Geom::Point defvalue;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

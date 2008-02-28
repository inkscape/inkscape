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
    virtual ~PointParam();

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    void param_setValue(Geom::Point newpoint);
    void param_set_default();

    void param_set_and_write_new_value(Geom::Point newpoint);

    virtual void param_editOncanvas(SPItem * item, SPDesktop * dt);

    virtual void param_transform_multiply(Geom::Matrix const& /*postmul*/, bool /*set*/);

private:
    PointParam(const PointParam&);
    PointParam& operator=(const PointParam&);

    void on_button_click();

    SPKnot *knot;

    Geom::Point defvalue;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

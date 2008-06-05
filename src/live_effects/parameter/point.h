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

#include "knot-enums.h"

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

    virtual ParamType paramType() { return POINT_PARAM; }

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_getSVGValue() const;

    void param_setValue(Geom::Point newpoint);
    void param_set_default();

    void param_set_and_write_new_value(Geom::Point newpoint);

    virtual void param_editOncanvas(SPItem * item, SPDesktop * dt);

    virtual void param_transform_multiply(Geom::Matrix const& /*postmul*/, bool /*set*/);

    void set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);

private:
    PointParam(const PointParam&);
    PointParam& operator=(const PointParam&);

    void on_button_click();

    Geom::Point defvalue;

    SPKnotShapeType knot_shape;
    SPKnotModeType knot_mode;
    guint32 knot_color;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

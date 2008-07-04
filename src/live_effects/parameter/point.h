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

#include "knot-holder-entity.h"

namespace Inkscape {

namespace LivePathEffect {


class PointParam : public Geom::Point, public Parameter, public KnotHolderEntity {
public:
    PointParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                const gchar *handle_tip = NULL,
		Geom::Point default_value = Geom::Point(0,0) ); // tip for automatically associated on-canvas handle
    virtual ~PointParam();

    virtual ParamType paramType() { return POINT_PARAM; }

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_getSVGValue() const;
    inline const gchar *handleTip() const { return handle_tip ? handle_tip : param_tooltip.c_str(); }

    void param_setValue(Geom::Point newpoint);
    void param_set_default();

    void param_set_and_write_new_value(Geom::Point newpoint);

    virtual void param_editOncanvas(SPItem * item, SPDesktop * dt);

    virtual void param_transform_multiply(Geom::Matrix const& /*postmul*/, bool /*set*/);

    void set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);
    SPKnotShapeType knotShape() { return knot_shape; }
    SPKnotModeType knotMode() { return knot_mode; }
    guint32 knotColor() { return knot_color; }

    /* these are overloaded from KnotHolderEntity */
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
    virtual void knot_click(guint state);

    virtual bool isLPEParam() { return true; }

private:
    PointParam(const PointParam&);
    PointParam& operator=(const PointParam&);

    void on_button_click();

    Geom::Point defvalue;

    SPKnotShapeType knot_shape;
    SPKnotModeType knot_mode;
    guint32 knot_color;
    gchar *handle_tip;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

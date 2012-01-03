#ifndef INKSCAPE_LIVEPATHEFFECT_POWERSTROKE_POINT_ARRAY_H
#define INKSCAPE_LIVEPATHEFFECT_POWERSTROKE_POINT_ARRAY_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <2geom/point.h>

#include "live_effects/parameter/array.h"

#include "knot-holder-entity.h"

namespace Inkscape {

namespace LivePathEffect {

class PowerStrokePointArrayParamKnotHolderEntity;

class PowerStrokePointArrayParam : public ArrayParam<Geom::Point> {
public:
    PowerStrokePointArrayParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                const gchar *handle_tip = NULL); // tip for automatically associated on-canvas handle
    virtual ~PowerStrokePointArrayParam();

    virtual Gtk::Widget * param_newWidget();

    virtual void param_transform_multiply(Geom::Affine const& /*postmul*/, bool /*set*/);

    void set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);

    virtual bool providesKnotHolderEntities() { return true; }
    virtual void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

    void set_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in, Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_normal_in);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const & get_pwd2() { return last_pwd2; }
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const & get_pwd2_normal() { return last_pwd2_normal; }

    void recalculate_controlpoints_for_new_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    friend class PowerStrokePointArrayParamKnotHolderEntity;

private:
    PowerStrokePointArrayParam(const PowerStrokePointArrayParam&);
    PowerStrokePointArrayParam& operator=(const PowerStrokePointArrayParam&);

    SPKnotShapeType knot_shape;
    SPKnotModeType knot_mode;
    guint32 knot_color;
    gchar *handle_tip;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > last_pwd2;
    Geom::Piecewise<Geom::D2<Geom::SBasis> > last_pwd2_normal;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

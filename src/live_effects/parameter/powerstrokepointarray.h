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

class PowerStrokePointArrayParam : public ArrayParam<Geom::Point> {
public:
    PowerStrokePointArrayParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect);
    virtual ~PowerStrokePointArrayParam();

    virtual Gtk::Widget * param_newWidget();

    virtual void param_transform_multiply(Geom::Affine const& postmul, bool /*set*/);

    void set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);

    float median_width();

    virtual bool providesKnotHolderEntities() const { return true; }
    virtual void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

    void set_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in, Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_normal_in);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const & get_pwd2() const { return last_pwd2; }
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const & get_pwd2_normal() const { return last_pwd2_normal; }

    void recalculate_controlpoints_for_new_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    friend class PowerStrokePointArrayParamKnotHolderEntity;

private:
    PowerStrokePointArrayParam(const PowerStrokePointArrayParam&);
    PowerStrokePointArrayParam& operator=(const PowerStrokePointArrayParam&);

    SPKnotShapeType knot_shape;
    SPKnotModeType knot_mode;
    guint32 knot_color;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > last_pwd2;
    Geom::Piecewise<Geom::D2<Geom::SBasis> > last_pwd2_normal;
};

class PowerStrokePointArrayParamKnotHolderEntity : public KnotHolderEntity {
public:
    PowerStrokePointArrayParamKnotHolderEntity(PowerStrokePointArrayParam *p, unsigned int index);
    virtual ~PowerStrokePointArrayParamKnotHolderEntity() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
    virtual void knot_set_offset(Geom::Point offset);
    virtual void knot_click(guint state);

    /** Checks whether the index falls within the size of the parameter's vector */
    bool valid_index(unsigned int index) const {
        return (_pparam->_vector.size() > index);
    };

private:
    PowerStrokePointArrayParam *_pparam;
    unsigned int _index;
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

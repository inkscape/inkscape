#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_VECTOR_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_VECTOR_H

/*
 * Inkscape::LivePathEffectParameters
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <2geom/point.h>

#include "live_effects/parameter/parameter.h"

#include "knot-holder-entity.h"

namespace Inkscape {

namespace LivePathEffect {


class VectorParam : public Parameter {
public:
    VectorParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                Geom::Point default_vector = Geom::Point(1,0) );
    virtual ~VectorParam();

    virtual Gtk::Widget * param_newWidget();
    inline const gchar *handleTip() const { return param_tooltip.c_str(); }

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;

    Geom::Point getVector() const { return vector; };
    Geom::Point getOrigin() const { return origin; };
    void setValues(Geom::Point const &new_origin, Geom::Point const &new_vector) { setVector(new_vector); setOrigin(new_origin); };
    void setVector(Geom::Point const &new_vector) { vector = new_vector; };
    void setOrigin(Geom::Point const &new_origin) { origin = new_origin; };
    virtual void param_set_default();

    void set_and_write_new_values(Geom::Point const &new_origin, Geom::Point const &new_vector);

    virtual void param_transform_multiply(Geom::Affine const &postmul, bool set);

    void set_vector_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);
    void set_origin_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);
    void set_oncanvas_color(guint32 color);

    virtual bool providesKnotHolderEntities() const { return true; }
    virtual void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

private:
    VectorParam(const VectorParam&);
    VectorParam& operator=(const VectorParam&);

    Geom::Point defvalue;

    Geom::Point origin;
    Geom::Point vector;

    /// The looks of the vector and origin knots oncanvas
    SPKnotShapeType vec_knot_shape;
    SPKnotModeType  vec_knot_mode;
    guint32         vec_knot_color;
    SPKnotShapeType ori_knot_shape;
    SPKnotModeType  ori_knot_mode;
    guint32         ori_knot_color;

    friend class VectorParamKnotHolderEntity_Origin;
    friend class VectorParamKnotHolderEntity_Vector;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

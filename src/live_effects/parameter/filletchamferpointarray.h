#ifndef INKSCAPE_LIVEPATHEFFECT_FILLET_CHAMFER_POINT_ARRAY_H
#define INKSCAPE_LIVEPATHEFFECT_FILLET_CHAMFER_POINT_ARRAY_H

/*
 * Inkscape::LivePathEffectParameters
 * Copyright (C) Jabiertxo Arraiza Cenoz <jabier.arraiza@marker.es>
 * Special thanks to Johan Engelen for the base of the effect -powerstroke-
 * Also to ScislaC for point me to the idea
 * Also su_v for his construvtive feedback and time
 * and finaly to Liam P. White for his big help on coding, that save me a lot of
 * hours
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <2geom/point.h>

#include "live_effects/parameter/array.h"

#include "knot-holder-entity.h"

namespace Inkscape {

namespace LivePathEffect {

class FilletChamferPointArrayParamKnotHolderEntity;

class FilletChamferPointArrayParam : public ArrayParam<Geom::Point> {
public:
    FilletChamferPointArrayParam(const Glib::ustring &label,
                                 const Glib::ustring &tip,
                                 const Glib::ustring &key,
                                 Inkscape::UI::Widget::Registry *wr,
                                 Effect *effect);
    virtual ~FilletChamferPointArrayParam();

    virtual Gtk::Widget *param_newWidget();

    virtual void param_transform_multiply(Geom::Affine const &postmul,
                                          bool /*set*/);

    void set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode,
                            guint32 color);
    virtual double to_time(int index, double A);
    virtual double to_len(int index, double A);
    virtual double rad_to_len(int index, double rad);
    virtual double len_to_rad(int index, double len);
    virtual double len_to_time(int index, double len);
    virtual double time_to_len(int index, double time);
    virtual std::pair<std::size_t, std::size_t> get_positions(int index, Geom::PathVector subpaths);
    virtual int last_index(int index, Geom::PathVector subpaths);
    std::vector<double> get_times(int index, Geom::PathVector subpaths, bool last);
    virtual void set_helper_size(int hs);
    virtual void set_use_distance(bool use_knot_distance);
    virtual void set_chamfer_steps(int value_chamfer_steps);
    virtual void addCanvasIndicators(SPLPEItem const *lpeitem,
                                     std::vector<Geom::PathVector> &hp_vec);
    virtual bool providesKnotHolderEntities() const {
        return true;
    }
    virtual void updateCanvasIndicators();
    virtual void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop,
                                       SPItem *item);

    void set_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &pwd2_in,
                  Geom::Piecewise<Geom::D2<Geom::SBasis> > const &pwd2_normal_in);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const &get_pwd2() const {
        return last_pwd2;
    }
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const &get_pwd2_normal() const {
        return last_pwd2_normal;
    }

    void recalculate_controlpoints_for_new_pwd2(
        Geom::Piecewise<Geom::D2<Geom::SBasis> > const &pwd2_in);
    void recalculate_knots(
        Geom::Piecewise<Geom::D2<Geom::SBasis> > const &pwd2_in);
    friend class FilletChamferPointArrayParamKnotHolderEntity;

private:
    FilletChamferPointArrayParam(const FilletChamferPointArrayParam &);
    FilletChamferPointArrayParam &operator=(const FilletChamferPointArrayParam &);

    SPKnotShapeType knot_shape;
    SPKnotModeType knot_mode;
    guint32 knot_color;
    int helper_size;
    int chamfer_steps;
    bool use_distance;
    Geom::PathVector hp;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > last_pwd2;
    Geom::Piecewise<Geom::D2<Geom::SBasis> > last_pwd2_normal;
};

class FilletChamferPointArrayParamKnotHolderEntity : public KnotHolderEntity {
public:
    FilletChamferPointArrayParamKnotHolderEntity(FilletChamferPointArrayParam *p,
            unsigned int index);
    virtual ~FilletChamferPointArrayParamKnotHolderEntity() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin,
                          guint state);
    virtual Geom::Point knot_get() const;
    virtual void knot_click(guint state);
    virtual void knot_set_offset(Geom::Point offset);

    /*Checks whether the index falls within the size of the parameter's vector*/
    bool valid_index(unsigned int index) const {
        return (_pparam->_vector.size() > index);
    }
    ;

private:
    FilletChamferPointArrayParam *_pparam;
    unsigned int _index;
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

/** \file
 * LPE <copy_rotate> implementation
 */
/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *   Jabiertxo Arraiza Cenoz <jabier.arraiza@marker.es>
 * Copyright (C) Authors 2007-2012
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include <2geom/path-intersection.h>
#include <2geom/sbasis-to-bezier.h>
#include "live_effects/lpe-copy_rotate.h"

#include "knotholder.h"
// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

namespace Inkscape {
namespace LivePathEffect {

namespace CR {

class KnotHolderEntityStartingAngle : public LPEKnotHolderEntity {
public:
    KnotHolderEntityStartingAngle(LPECopyRotate *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

class KnotHolderEntityRotationAngle : public LPEKnotHolderEntity {
public:
    KnotHolderEntityRotationAngle(LPECopyRotate *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

} // namespace CR

bool 
pointInTriangle(Geom::Point const &p, Geom::Point const &p1, Geom::Point const &p2, Geom::Point const &p3)
{
    //http://totologic.blogspot.com.es/2014/01/accurate-point-in-triangle-test.html
    using Geom::X;
    using Geom::Y;
    double denominator = (p1[X]*(p2[Y] - p3[Y]) + p1[Y]*(p3[X] - p2[X]) + p2[X]*p3[Y] - p2[Y]*p3[X]);
    double t1 = (p[X]*(p3[Y] - p1[Y]) + p[Y]*(p1[X] - p3[X]) - p1[X]*p3[Y] + p1[Y]*p3[X]) / denominator;
    double t2 = (p[X]*(p2[Y] - p1[Y]) + p[Y]*(p1[X] - p2[X]) - p1[X]*p2[Y] + p1[Y]*p2[X]) / -denominator;
    double s = t1 + t2;

    return 0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1 && s <= 1;
}


LPECopyRotate::LPECopyRotate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    origin(_("Origin"), _("Origin of the rotation"), "origin", &wr, this),
    starting_angle(_("Starting:"), _("Angle of the first copy"), "starting_angle", &wr, this, 0.0),
    rotation_angle(_("Rotation angle:"), _("Angle between two successive copies"), "rotation_angle", &wr, this, 60.0),
    num_copies(_("Number of copies:"), _("Number of copies of the original path"), "num_copies", &wr, this, 6),
    copies_to_360(_("360º Copies"), _("No rotation angle, fixed to 360º"), "copies_to_360", &wr, this, true),
    fuse_paths(_("Fuse paths"), _("Fuse paths by helper line, use fill-rule: evenodd for best result"), "fuse_paths", &wr, this, false),
    dist_angle_handle(100.0)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;
    apply_to_clippath_and_mask = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter(&copies_to_360);
    registerParameter(&fuse_paths);
    registerParameter(&starting_angle);
    registerParameter(&rotation_angle);
    registerParameter(&num_copies);
    registerParameter(&origin);

    num_copies.param_make_integer(true);
    num_copies.param_set_range(0, 1000);
}

LPECopyRotate::~LPECopyRotate()
{

}

void
LPECopyRotate::doOnApply(SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);

    A = Point(boundingbox_X.min(), boundingbox_Y.middle());
    B = Point(boundingbox_X.middle(), boundingbox_Y.middle());
    origin.param_setValue(A);
    origin.param_update_default(A);
    dist_angle_handle = L2(B - A);
    dir = unit_vector(B - A);
}

void
LPECopyRotate::transform_multiply(Geom::Affine const& postmul, bool set)
{
    if(fuse_paths) {
        Geom::Coord angle  = Geom::deg_from_rad(atan(-postmul[1]/postmul[0]));
        angle += starting_angle;
        starting_angle.param_set_value(angle);
    }
    // cycle through all parameters. Most parameters will not need transformation, but path and point params do.

    for (std::vector<Parameter *>::iterator it = param_vector.begin(); it != param_vector.end(); ++it) {
        Parameter * param = *it;
        param->param_transform_multiply(postmul, set);
    }
}

void
LPECopyRotate::doBeforeEffect (SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);
    if (copies_to_360) {
        rotation_angle.param_set_value(360.0/(double)num_copies);
    }
    if (fuse_paths && rotation_angle * num_copies > 360 && rotation_angle > 0) {
        num_copies.param_set_value(floor(360/rotation_angle));
    }
    if (fuse_paths && copies_to_360) {
        num_copies.param_set_increments(2,2);
        if ((int)num_copies%2 !=0) {
            num_copies.param_set_value(num_copies+1);
            rotation_angle.param_set_value(360.0/(double)num_copies);
        }
    } else {
        num_copies.param_set_increments(1,1);
    }

    if (dist_angle_handle < 1.0) {
        dist_angle_handle = 1.0;
    }
    A = Point(boundingbox_X.min(), boundingbox_Y.middle());
    B = Point(boundingbox_X.middle(), boundingbox_Y.middle());
    dir = unit_vector(B - A);
    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    start_pos = origin + dir * Rotate(-rad_from_deg(starting_angle)) * dist_angle_handle;
    rot_pos = origin + dir * Rotate(-rad_from_deg(rotation_angle+starting_angle)) * dist_angle_handle;
    if ( fuse_paths || copies_to_360 ) {
        rot_pos = origin;
    }
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
}

void
LPECopyRotate::split(Geom::PathVector &path_on, Geom::Path const &divider)
{
    Geom::PathVector tmp_path;
    double time_start = 0.0;
    Geom::Path original = path_on[0];
    int position = 0;
    Geom::Crossings cs = crossings(original,divider);
    std::vector<double> crossed;
    for(unsigned int i = 0; i < cs.size(); i++) {
        crossed.push_back(cs[i].ta);
    }
    std::sort(crossed.begin(), crossed.end());
    for (unsigned int i = 0; i < crossed.size(); i++) {
        double time_end = crossed[i];
        if (time_start == time_end || time_end - time_start < Geom::EPSILON) {
            continue;
        }
        Geom::Path portion_original = original.portion(time_start,time_end);
        if (!portion_original.empty()) {
            Geom::Point side_checker = portion_original.pointAt(0.0001);
            position = Geom::sgn(Geom::cross(divider[1].finalPoint() - divider[0].finalPoint(), side_checker - divider[0].finalPoint()));
            if (rotation_angle != 180) {
                position = pointInTriangle(side_checker, divider.initialPoint(), divider[0].finalPoint(), divider[1].finalPoint());
            }
            if (position == 1) {
                tmp_path.push_back(portion_original);
            }
            portion_original.clear();
            time_start = time_end;
        }
    }
    position = Geom::sgn(Geom::cross(divider[1].finalPoint() - divider[0].finalPoint(), original.finalPoint() - divider[0].finalPoint()));
    if (rotation_angle != 180) {
        position = pointInTriangle(original.finalPoint(), divider.initialPoint(), divider[0].finalPoint(), divider[1].finalPoint());
    }
    if (cs.size() > 0 && position == 1) {
        Geom::Path portion_original = original.portion(time_start, original.size());
        if(!portion_original.empty()){
            if (!original.closed()) {
                tmp_path.push_back(portion_original);
            } else {
                if (tmp_path.size() > 0 && tmp_path[0].size() > 0 ) {
                    portion_original.setFinal(tmp_path[0].initialPoint());
                    portion_original.append(tmp_path[0]);
                    tmp_path[0] = portion_original;
                } else {
                    tmp_path.push_back(portion_original);
                }
            }
            portion_original.clear();
        }
    }
    if (cs.size()==0  && position == 1) {
        tmp_path.push_back(original);
    }
    path_on = tmp_path;
}

void
LPECopyRotate::setFusion(Geom::PathVector &path_on, Geom::Path divider, double size_divider)
{
    split(path_on,divider);
    Geom::PathVector tmp_path;
    Geom::Affine pre = Geom::Translate(-origin);
    for (Geom::PathVector::const_iterator path_it = path_on.begin(); path_it != path_on.end(); ++path_it) {
        Geom::Path original = *path_it;
        if (path_it->empty()) {
            continue;
        }
        Geom::PathVector tmp_path_helper;
        Geom::Path append_path = original;

        for (int i = 0; i < num_copies; ++i) {
            Geom::Rotate rot(-Geom::rad_from_deg(rotation_angle * (i)));
            Geom::Affine m = pre * rot * Geom::Translate(origin);
            if (i%2 != 0) {
                Geom::Point A = (Geom::Point)origin;
                Geom::Point B = origin + dir * Geom::Rotate(-Geom::rad_from_deg((rotation_angle*i)+starting_angle)) * size_divider;
                Geom::Translate m1(A[0], A[1]);
                double hyp = Geom::distance(A, B);
                double c = (B[0] - A[0]) / hyp; // cos(alpha)
                double s = (B[1] - A[1]) / hyp; // sin(alpha)

                Geom::Affine m2(c, -s, s, c, 0.0, 0.0);
                Geom::Scale sca(1.0, -1.0);

                Geom::Affine tmp_m = m1.inverse() * m2;
                m = tmp_m;
                m = m * sca;
                m = m * m2.inverse();
                m = m * m1;
            } else {
                append_path = original;
            }
            append_path *= m;
            if (tmp_path_helper.size() > 0) {
                if (Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].finalPoint(), append_path.finalPoint())) {
                    Geom::Path tmp_append = append_path.reversed();
                    tmp_append.setInitial(tmp_path_helper[tmp_path_helper.size()-1].finalPoint());
                    tmp_path_helper[tmp_path_helper.size()-1].append(tmp_append);
                } else if (Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].initialPoint(), append_path.initialPoint())) {
                    Geom::Path tmp_append = append_path;
                    tmp_path_helper[tmp_path_helper.size()-1] = tmp_path_helper[tmp_path_helper.size()-1].reversed();
                    tmp_append.setInitial(tmp_path_helper[tmp_path_helper.size()-1].finalPoint());
                    tmp_path_helper[tmp_path_helper.size()-1].append(tmp_append);
                } else if (Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].finalPoint(), append_path.initialPoint())) {
                    Geom::Path tmp_append = append_path;
                    tmp_append.setInitial(tmp_path_helper[tmp_path_helper.size()-1].finalPoint());
                    tmp_path_helper[tmp_path_helper.size()-1].append(tmp_append);
                } else if (Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].initialPoint(), append_path.finalPoint())) {
                    Geom::Path tmp_append = append_path.reversed();
                    tmp_path_helper[tmp_path_helper.size()-1] = tmp_path_helper[tmp_path_helper.size()-1].reversed();
                    tmp_append.setInitial(tmp_path_helper[tmp_path_helper.size()-1].finalPoint());
                    tmp_path_helper[tmp_path_helper.size()-1].append(tmp_append);
                } else if (Geom::are_near(tmp_path_helper[0].finalPoint(), append_path.finalPoint())) {
                    Geom::Path tmp_append = append_path.reversed();
                    tmp_append.setInitial(tmp_path_helper[0].finalPoint());
                    tmp_path_helper[0].append(tmp_append);
                } else if (Geom::are_near(tmp_path_helper[0].initialPoint(), append_path.initialPoint())) {
                    Geom::Path tmp_append = append_path;
                    tmp_path_helper[0] = tmp_path_helper[0].reversed();
                    tmp_append.setInitial(tmp_path_helper[0].finalPoint());
                    tmp_path_helper[0].append(tmp_append);
                } else {
                    tmp_path_helper.push_back(append_path);
                }
                if ( Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].finalPoint(),tmp_path_helper[tmp_path_helper.size()-1].initialPoint())) {
                    tmp_path_helper[tmp_path_helper.size()-1].close();
                }
            } else {
                tmp_path_helper.push_back(append_path);
            }
        }
        if (tmp_path_helper.size() > 0) {
            tmp_path_helper[tmp_path_helper.size()-1] = tmp_path_helper[tmp_path_helper.size()-1];
            tmp_path_helper[0] = tmp_path_helper[0];
            if (rotation_angle * num_copies != 360) {
                Geom::Ray base_a(divider.pointAt(1),divider.pointAt(0));
                double diagonal = Geom::distance(Geom::Point(boundingbox_X.min(),boundingbox_Y.min()),Geom::Point(boundingbox_X.max(),boundingbox_Y.max()));
                Geom::Rect bbox(Geom::Point(boundingbox_X.min(),boundingbox_Y.min()),Geom::Point(boundingbox_X.max(),boundingbox_Y.max()));
                double size_divider = Geom::distance(origin,bbox) + (diagonal * 2);
                Geom::Point base_point = origin + dir * Geom::Rotate(-Geom::rad_from_deg((rotation_angle * num_copies) + starting_angle)) * size_divider;
                Geom::Ray base_b(divider.pointAt(1), base_point);
                if (Geom::are_near(tmp_path_helper[0].initialPoint(),base_a) && 
                    Geom::are_near(tmp_path_helper[0].finalPoint(),base_a)) 
                {
                    tmp_path_helper[0].close();
                    if (tmp_path_helper.size() > 1) {
                        tmp_path_helper[tmp_path_helper.size()-1].close();
                    }
                } else if (Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].initialPoint(),base_b) && 
                           Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].finalPoint(),base_b)) 
                {
                    tmp_path_helper[0].close();
                    if (tmp_path_helper.size() > 1) {
                        tmp_path_helper[tmp_path_helper.size()-1].close();
                    }
                } else if ((Geom::are_near(tmp_path_helper[0].initialPoint(),base_a) && 
                           Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].finalPoint(),base_b)) ||
                           (Geom::are_near(tmp_path_helper[0].initialPoint(),base_b) && 
                           Geom::are_near(tmp_path_helper[tmp_path_helper.size()-1].finalPoint(),base_a))) 
                {
                    Geom::Path close_path = Geom::Path(tmp_path_helper[tmp_path_helper.size()-1].finalPoint());
                    close_path.appendNew<Geom::LineSegment>((Geom::Point)origin);
                    close_path.appendNew<Geom::LineSegment>(tmp_path_helper[0].initialPoint());
                    tmp_path_helper[0].append(close_path);
                }
            }

            if (Geom::are_near(tmp_path_helper[0].finalPoint(),tmp_path_helper[0].initialPoint())) {
                tmp_path_helper[0].close();
            }
        }
        tmp_path.insert(tmp_path.end(), tmp_path_helper.begin(), tmp_path_helper.end());
        tmp_path_helper.clear();
    }
    path_on = tmp_path;
    tmp_path.clear();
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPECopyRotate::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    if (num_copies == 1 && !fuse_paths) {
        return pwd2_in;
    }

    double diagonal = Geom::distance(Geom::Point(boundingbox_X.min(),boundingbox_Y.min()),Geom::Point(boundingbox_X.max(),boundingbox_Y.max()));
    Geom::Rect bbox(Geom::Point(boundingbox_X.min(),boundingbox_Y.min()),Geom::Point(boundingbox_X.max(),boundingbox_Y.max()));
    double size_divider = Geom::distance(origin,bbox) + (diagonal * 2);
    Geom::Point line_start  = origin + dir * Rotate(-rad_from_deg(starting_angle)) * size_divider;
    Geom::Point line_end = origin + dir * Rotate(-rad_from_deg(rotation_angle + starting_angle)) * size_divider;
    //Note:: beter way to do this
    //Whith AppendNew have problems whith the crossing order
    Geom::Path divider = Geom::Path(line_start);
    divider.appendNew<Geom::LineSegment>((Geom::Point)origin);
    divider.appendNew<Geom::LineSegment>(line_end);
    Piecewise<D2<SBasis> > output;
    Affine pre = Translate(-origin) * Rotate(-rad_from_deg(starting_angle));
    if (fuse_paths) {
        Geom::PathVector path_out;
        Geom::PathVector tmp_path;
        PathVector const original_pathv = path_from_piecewise(remove_short_cuts(pwd2_in, 0.1), 0.001);
        for (Geom::PathVector::const_iterator path_it = original_pathv.begin(); path_it != original_pathv.end(); ++path_it) {
            if (path_it->empty()) {
                continue;
            }
            bool end_open = false;
            if (path_it->closed()) {
                const Geom::Curve &closingline = path_it->back_closed();
                if (!are_near(closingline.initialPoint(), closingline.finalPoint())) {
                    end_open = true;
                }
            }
            Geom::Path original = (Geom::Path)(*path_it);
            if (end_open && path_it->closed()) {
                original.close(false);
                original.appendNew<Geom::LineSegment>( original.initialPoint() );
                original.close(true);
            }
            tmp_path.push_back(original);
            setFusion(tmp_path, divider, size_divider);
            path_out.insert(path_out.end(), tmp_path.begin(), tmp_path.end());
            tmp_path.clear();
        }
        if (path_out.size()>0) {
            output = paths_to_pw(path_out);
        }
    } else {
        for (int i = 0; i < num_copies; ++i) {
            Rotate rot(-rad_from_deg(rotation_angle * i));
            Affine t = pre * rot * Translate(origin);
            output.concat(pwd2_in * t);
        }
    }
    return output;
}

void
LPECopyRotate::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;
    hp_vec.clear();
    Geom::Path hp;
    hp.start(start_pos);
    hp.appendNew<Geom::LineSegment>((Geom::Point)origin);
    hp.appendNew<Geom::LineSegment>(origin + dir * Rotate(-rad_from_deg(rotation_angle+starting_angle)) * dist_angle_handle);
    Geom::PathVector pathv;
    pathv.push_back(hp);
    hp_vec.push_back(pathv);
}

void
LPECopyRotate::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);
    original_bbox(SP_LPE_ITEM(item));
}

void
LPECopyRotate::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    {
        KnotHolderEntity *e = new CR::KnotHolderEntityStartingAngle(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the starting angle"));
        knotholder->add(e);
    }
    {
        KnotHolderEntity *e = new CR::KnotHolderEntityRotationAngle(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the rotation angle"));
        knotholder->add(e);
    }
};

namespace CR {

using namespace Geom;

void
KnotHolderEntityStartingAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = dynamic_cast<LPECopyRotate *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->starting_angle.param_set_value(deg_from_rad(-angle_between(lpe->dir, s - lpe->origin)));
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRotationAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = dynamic_cast<LPECopyRotate *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->rotation_angle.param_set_value(deg_from_rad(-angle_between(lpe->dir, s - lpe->origin)) - lpe->starting_angle);
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityStartingAngle::knot_get() const
{
    LPECopyRotate const *lpe = dynamic_cast<LPECopyRotate const*>(_effect);
    return lpe->start_pos;
}

Geom::Point
KnotHolderEntityRotationAngle::knot_get() const
{
    LPECopyRotate const *lpe = dynamic_cast<LPECopyRotate const*>(_effect);
    return lpe->rot_pos;
}

} // namespace CR

/* ######################## */

} //namespace LivePathEffect
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

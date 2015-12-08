/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-patternalongpath.h"
#include "live_effects/lpeobject.h"
#include "sp-shape.h"
#include "display/curve.h"
#include "svg/svg.h"
#include "ui/widget/scalar.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include "knot-holder-entity.h"
#include "knotholder.h"

#include <algorithm>
using std::vector;


/* Theory in e-mail from J.F. Barraud
Let B be the skeleton path, and P the pattern (the path to be deformed).

P is a map t --> P(t) = ( x(t), y(t) ).
B is a map t --> B(t) = ( a(t), b(t) ).

The first step is to re-parametrize B by its arc length: this is the parametrization in which a point p on B is located by its distance s from start. One obtains a new map s --> U(s) = (a'(s),b'(s)), that still describes the same path B, but where the distance along B from start to
U(s) is s itself.

We also need a unit normal to the path. This can be obtained by computing a unit tangent vector, and rotate it by 90�. Call this normal vector N(s).

The basic deformation associated to B is then given by:

   (x,y) --> U(x)+y*N(x)

(i.e. we go for distance x along the path, and then for distance y along the normal)

Of course this formula needs some minor adaptations (as is it depends on the absolute position of P for instance, so a little translation is needed
first) but I think we can first forget about them.
*/

namespace Inkscape {
namespace LivePathEffect {
Geom::PathVector pap_helper_path;

namespace WPAP {
    class KnotHolderEntityWidthPatternAlongPath : public LPEKnotHolderEntity {
    public:
        KnotHolderEntityWidthPatternAlongPath(LPEPatternAlongPath * effect) : LPEKnotHolderEntity(effect) {}
        virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
        virtual Geom::Point knot_get() const;
    };
} // WPAP

static const Util::EnumData<PAPCopyType> PAPCopyTypeData[PAPCT_END] = {
    {PAPCT_SINGLE,               N_("Single"),               "single"},
    {PAPCT_SINGLE_STRETCHED,     N_("Single, stretched"),    "single_stretched"},
    {PAPCT_REPEATED,             N_("Repeated"),             "repeated"},
    {PAPCT_REPEATED_STRETCHED,   N_("Repeated, stretched"),  "repeated_stretched"}
};
static const Util::EnumDataConverter<PAPCopyType> PAPCopyTypeConverter(PAPCopyTypeData, PAPCT_END);

LPEPatternAlongPath::LPEPatternAlongPath(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    pattern(_("Pattern source:"), _("Path to put along the skeleton path"), "pattern", &wr, this, "M0,0 L1,0"),
    original_height(0),
    prop_scale(_("_Width:"), _("Width of the pattern"), "prop_scale", &wr, this, 1.0),
    copytype(_("Pattern copies:"), _("How many pattern copies to place along the skeleton path"),
        "copytype", PAPCopyTypeConverter, &wr, this, PAPCT_SINGLE_STRETCHED),
    scale_y_rel(_("Wid_th in units of length"),
        _("Scale the width of the pattern in units of its length"),
        "scale_y_rel", &wr, this, false),
    spacing(_("Spa_cing:"),
        // xgettext:no-c-format
        _("Space between copies of the pattern. Negative values allowed, but are limited to -90% of pattern width."),
        "spacing", &wr, this, 0),
    normal_offset(_("No_rmal offset:"), "", "normal_offset", &wr, this, 0),
    tang_offset(_("Tan_gential offset:"), "", "tang_offset", &wr, this, 0),
    prop_units(_("Offsets in _unit of pattern size"),
        _("Spacing, tangential and normal offset are expressed as a ratio of width/height"),
        "prop_units", &wr, this, false),
    vertical_pattern(_("Pattern is _vertical"), _("Rotate pattern 90 deg before applying"),
        "vertical_pattern", &wr, this, false),
    fuse_tolerance(_("_Fuse nearby ends:"), _("Fuse ends closer than this number. 0 means don't fuse."),
        "fuse_tolerance", &wr, this, 0)
{
    registerParameter( dynamic_cast<Parameter *>(&pattern) );
    registerParameter( dynamic_cast<Parameter *>(&copytype) );
    registerParameter( dynamic_cast<Parameter *>(&prop_scale) );
    registerParameter( dynamic_cast<Parameter *>(&scale_y_rel) );
    registerParameter( dynamic_cast<Parameter *>(&spacing) );
    registerParameter( dynamic_cast<Parameter *>(&normal_offset) );
    registerParameter( dynamic_cast<Parameter *>(&tang_offset) );
    registerParameter( dynamic_cast<Parameter *>(&prop_units) );
    registerParameter( dynamic_cast<Parameter *>(&vertical_pattern) );
    registerParameter( dynamic_cast<Parameter *>(&fuse_tolerance) );
    prop_scale.param_set_digits(3);
    prop_scale.param_set_increments(0.01, 0.10);

    _provides_knotholder_entities = true;

}

LPEPatternAlongPath::~LPEPatternAlongPath()
{

}

void
LPEPatternAlongPath::doBeforeEffect (SPLPEItem const* lpeitem)
{
    // get the pattern bounding box
    Geom::OptRect bbox = pattern.get_pathvector().boundsFast();
    if (bbox) {
        original_height = (*bbox)[Geom::Y].max() - (*bbox)[Geom::Y].min();
    }
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPatternAlongPath::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    // Don't allow empty path parameter:
    if ( pattern.get_pathvector().empty() ) {
        return pwd2_in;
    }

/* Much credit should go to jfb and mgsloan of lib2geom development for the code below! */
    Piecewise<D2<SBasis> > output;
    std::vector<Geom::Piecewise<Geom::D2<Geom::SBasis> > > pre_output;

    PAPCopyType type = copytype.get_value();

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(pattern.get_pwd2());
    Piecewise<SBasis> x0 = vertical_pattern.get_value() ? Piecewise<SBasis>(patternd2[1]) : Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y0 = vertical_pattern.get_value() ? Piecewise<SBasis>(patternd2[0]) : Piecewise<SBasis>(patternd2[1]);
    OptInterval pattBndsX = bounds_exact(x0);
    OptInterval pattBndsY = bounds_exact(y0);
    if (pattBndsX && pattBndsY) {
        x0 -= pattBndsX->min();
        y0 -= pattBndsY->middle();

        double xspace  = spacing;
        double noffset = normal_offset;
        double toffset = tang_offset;
        if (prop_units.get_value() && pattBndsY){
            xspace  *= pattBndsX->extent();
            noffset *= pattBndsY->extent();
            toffset *= pattBndsX->extent();
        }

        //Prevent more than 90% overlap...
        if (xspace < -pattBndsX->extent()*.9) {
            xspace = -pattBndsX->extent()*.9;
        }
        //TODO: dynamical update of parameter ranges?
        //if (prop_units.get_value()){
        //        spacing.param_set_range(-.9, Geom::infinity());
        //    }else{
        //        spacing.param_set_range(-pattBndsX.extent()*.9, Geom::infinity());
        //    }

        y0+=noffset;

        std::vector<Geom::Piecewise<Geom::D2<Geom::SBasis> > > paths_in;
        paths_in = split_at_discontinuities(pwd2_in);

        for (unsigned idx = 0; idx < paths_in.size(); idx++){
            Geom::Piecewise<Geom::D2<Geom::SBasis> > path_i = paths_in[idx];
            Piecewise<SBasis> x = x0;
            Piecewise<SBasis> y = y0;
            Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(path_i,2,.1);
            uskeleton = remove_short_cuts(uskeleton,.01);
            Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
            n = force_continuity(remove_short_cuts(n,.1));
            
            int nbCopies = 0;
            double scaling = 1;
            switch(type) {
                case PAPCT_REPEATED:
                    nbCopies = static_cast<int>(floor((uskeleton.domain().extent() - toffset + xspace)/(pattBndsX->extent()+xspace)));
                    pattBndsX = Interval(pattBndsX->min(),pattBndsX->max()+xspace);
                    break;
                    
                case PAPCT_SINGLE:
                    nbCopies = (toffset + pattBndsX->extent() < uskeleton.domain().extent()) ? 1 : 0;
                    break;
                    
                case PAPCT_SINGLE_STRETCHED:
                    nbCopies = 1;
                    scaling = (uskeleton.domain().extent() - toffset)/pattBndsX->extent();
                    break;
                    
                case PAPCT_REPEATED_STRETCHED:
                    // if uskeleton is closed:
                    if(path_i.segs.front().at0() == path_i.segs.back().at1()){
                        nbCopies = static_cast<int>(std::floor((uskeleton.domain().extent() - toffset)/(pattBndsX->extent()+xspace)));
                        pattBndsX = Interval(pattBndsX->min(),pattBndsX->max()+xspace);
                        scaling = (uskeleton.domain().extent() - toffset)/(((double)nbCopies)*pattBndsX->extent());
                        // if not closed: no space at the end
                    }else{
                        nbCopies = static_cast<int>(std::floor((uskeleton.domain().extent() - toffset + xspace)/(pattBndsX->extent()+xspace)));
                        pattBndsX = Interval(pattBndsX->min(),pattBndsX->max()+xspace);
                        scaling = (uskeleton.domain().extent() - toffset)/(((double)nbCopies)*pattBndsX->extent() - xspace);
                    }
                    break;
                    
                default:
                    return pwd2_in;
            };
            
            double pattWidth = pattBndsX->extent() * scaling;
            
            if (scaling != 1.0) {
                x*=scaling;
            }
            if ( scale_y_rel.get_value() ) {
                y*=(scaling*prop_scale);
            } else {
                if (prop_scale != 1.0) y *= prop_scale;
            }
            x += toffset;
            
            double offs = 0;
            for (int i=0; i<nbCopies; i++){
                if (fuse_tolerance > 0){        
                    Geom::Piecewise<Geom::D2<Geom::SBasis> > output_piece = compose(uskeleton,x+offs)+y*compose(n,x+offs);
                    std::vector<Geom::Piecewise<Geom::D2<Geom::SBasis> > > splited_output_piece = split_at_discontinuities(output_piece);
                    pre_output.insert(pre_output.end(), splited_output_piece.begin(), splited_output_piece.end() );
                }else{
                    output.concat(compose(uskeleton,x+offs)+y*compose(n,x+offs));
                }
                offs+=pattWidth;
            }
        }
        if (fuse_tolerance > 0){        
            pre_output = fuse_nearby_ends(pre_output, fuse_tolerance);
            for (unsigned i=0; i<pre_output.size(); i++){
                output.concat(pre_output[i]);
            }
        }
        return output;
    } else {
        return pwd2_in;
    }
}

void
LPEPatternAlongPath::transform_multiply(Geom::Affine const& postmul, bool set)
{
    // overriding the Effect class default method, disabling transform forwarding to the parameters.

    // only take translations into account
    if (postmul.isTranslation()) {
        pattern.param_transform_multiply(postmul, set);
    }
}

void
LPEPatternAlongPath::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    hp_vec.push_back(pap_helper_path);
}


void 
LPEPatternAlongPath::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    KnotHolderEntity *e = new WPAP::KnotHolderEntityWidthPatternAlongPath(this);
    e->create(desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN, _("Change the width"), SP_KNOT_SHAPE_CIRCLE);
    knotholder->add(e);
}

namespace WPAP {

void 
KnotHolderEntityWidthPatternAlongPath::knot_set(Geom::Point const &p, Geom::Point const& /*origin*/, guint state)
{
    LPEPatternAlongPath *lpe = dynamic_cast<LPEPatternAlongPath *> (_effect);
    
    Geom::Point const s = snap_knot_position(p, state);
    SPShape const *sp_shape = dynamic_cast<SPShape const *>(SP_LPE_ITEM(item));
    if (sp_shape) {
        Geom::Path const *path_in = sp_shape->getCurveBeforeLPE()->first_path();
        Geom::Point ptA = path_in->pointAt(Geom::PathTime(0, 0.0));
        lpe->prop_scale.param_set_value(Geom::distance(s , ptA)/(lpe->original_height/2.0));
    }
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point 
KnotHolderEntityWidthPatternAlongPath::knot_get() const
{
    LPEPatternAlongPath *lpe = dynamic_cast<LPEPatternAlongPath *> (_effect);

    SPShape const *sp_shape = dynamic_cast<SPShape const *>(SP_LPE_ITEM(item));
    if (sp_shape) {
        Geom::Path const *path_in = sp_shape->getCurveBeforeLPE()->first_path();
        Geom::Point ptA = path_in->pointAt(Geom::PathTime(0, 0.0));
        Geom::Point B = path_in->pointAt(Geom::PathTime(1, 0.0));
        Geom::Curve const *first_curve = &path_in->curveAt(Geom::PathTime(0, 0.0));
        Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const *>(&*first_curve);
        Geom::Ray ray(ptA, B);
        if (cubic) {
            ray.setPoints((*cubic)[1], ptA);
        }
        Geom::Angle first_curve_angle = ray.transformed(Geom::Rotate(Geom::deg_to_rad(90))).angle();
        Geom::Point result_point = Geom::Point::polar(first_curve_angle, (lpe->original_height * lpe->prop_scale)/2.0) + ptA;

        pap_helper_path.clear();
        Geom::Path hp(result_point);
        hp.appendNew<Geom::LineSegment>(ptA);
        pap_helper_path.push_back(hp);
        hp.clear();
        
        return result_point;
    }
    return Geom::Point();
}
} // namespace WPAP
} // namespace LivePathEffect
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

/**
 * @file
 * Taper Stroke path effect, provided as an alternative to Power Strokes
 * for otherwise constant-width paths.
 *
 * Authors:
 *   Liam P White <inkscapebrony@gmail.com>
 *
 * Copyright (C) 2014 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-taperstroke.h"

#include <2geom/path.h>
#include <2geom/path.h>
#include <2geom/circle.h>
#include <2geom/sbasis-to-bezier.h>

#include "helper/geom-nodetype.h"
#include "helper/geom-pathstroke.h"
#include "display/curve.h"
#include "sp-shape.h"
#include "style.h"
#include "xml/repr.h"
#include "sp-paint-server.h"
#include "svg/svg-color.h"
#include "desktop-style.h"
#include "svg/css-ostringstream.h"
#include "svg/svg.h"

#include "knot-holder-entity.h"
#include "knotholder.h"

#include <glibmm/i18n.h>

template<typename T>
inline bool withinRange(T value, T low, T high) {
    return (value > low && value < high);
}

namespace Inkscape {
namespace LivePathEffect {

namespace TpS {
    class KnotHolderEntityAttachBegin : public LPEKnotHolderEntity {
    public:
        KnotHolderEntityAttachBegin(LPETaperStroke * effect) : LPEKnotHolderEntity(effect) {}
        virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
        virtual Geom::Point knot_get() const;
    };
    
    class KnotHolderEntityAttachEnd : public LPEKnotHolderEntity {
    public:
        KnotHolderEntityAttachEnd(LPETaperStroke * effect) : LPEKnotHolderEntity(effect) {}
        virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
        virtual Geom::Point knot_get() const;
    };
} // TpS

static const Util::EnumData<unsigned> JoinType[] = {
    {JOIN_BEVEL,          N_("Beveled"),         "bevel"},
    {JOIN_ROUND,          N_("Rounded"),         "round"},
    {JOIN_MITER,          N_("Miter"),           "miter"},
    {JOIN_EXTRAPOLATE,    N_("Extrapolated"),    "extrapolated"},
};

static const Util::EnumDataConverter<unsigned> JoinTypeConverter(JoinType, sizeof (JoinType)/sizeof(*JoinType));

LPETaperStroke::LPETaperStroke(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    line_width(_("Stroke width:"), _("The (non-tapered) width of the path"), "stroke_width", &wr, this, 1.),
    attach_start(_("Start offset:"), _("Taper distance from path start"), "attach_start", &wr, this, 0.2),
    attach_end(_("End offset:"), _("The ending position of the taper"), "end_offset", &wr, this, 0.2),
    smoothing(_("Taper smoothing:"), _("Amount of smoothing to apply to the tapers"), "smoothing", &wr, this, 0.5),
    join_type(_("Join type:"), _("Join type for non-smooth nodes"), "jointype", JoinTypeConverter, &wr, this, JOIN_EXTRAPOLATE),
    miter_limit(_("Miter limit:"), _("Limit for miter joins"), "miter_limit", &wr, this, 100.)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;

    attach_start.param_set_digits(3);
    attach_end.param_set_digits(3);

    registerParameter(&line_width);
    registerParameter(&attach_start);
    registerParameter(&attach_end);
    registerParameter(&smoothing);
    registerParameter(&join_type);
    registerParameter(&miter_limit);
}

// from LPEPowerStroke -- sets fill if stroke color because we will
// be converting to a fill to make the new join.

void LPETaperStroke::doOnApply(SPLPEItem const* lpeitem)
{
    if (SP_IS_SHAPE(lpeitem)) {
        SPLPEItem* item = const_cast<SPLPEItem*>(lpeitem);
        double width = (lpeitem && lpeitem->style) ? lpeitem->style->stroke_width.computed : 1.;

        SPCSSAttr *css = sp_repr_css_attr_new ();
        if (lpeitem->style->stroke.isPaintserver()) {
            SPPaintServer * server = lpeitem->style->getStrokePaintServer();
            if (server) {
                Glib::ustring str;
                str += "url(#";
                str += server->getId();
                str += ")";
                sp_repr_css_set_property (css, "fill", str.c_str());
            }
        } else if (lpeitem->style->stroke.isColor()) {
            gchar c[64];
            sp_svg_write_color (c, sizeof(c), lpeitem->style->stroke.value.color.toRGBA32(SP_SCALE24_TO_FLOAT(lpeitem->style->stroke_opacity.value)));
            sp_repr_css_set_property (css, "fill", c);
        } else {
            sp_repr_css_set_property (css, "fill", "none");
        }

        sp_repr_css_set_property(css, "fill-rule", "nonzero");
        sp_repr_css_set_property(css, "stroke", "none");

        sp_desktop_apply_css_recursive(item, css, true);
        sp_repr_css_attr_unref (css);

        line_width.param_set_value(width);
        line_width.write_to_SVG();
    } else {
        printf("WARNING: It only makes sense to apply Taper stroke to paths (not groups).\n");
    }
}

// from LPEPowerStroke -- sets stroke color from existing fill color

void LPETaperStroke::doOnRemove(SPLPEItem const* lpeitem)
{
    if (SP_IS_SHAPE(lpeitem)) {
        SPLPEItem *item = const_cast<SPLPEItem*>(lpeitem);

        SPCSSAttr *css = sp_repr_css_attr_new ();
        if (lpeitem->style->fill.isPaintserver()) {
            SPPaintServer * server = lpeitem->style->getFillPaintServer();
            if (server) {
                Glib::ustring str;
                str += "url(#";
                str += server->getId();
                str += ")";
                sp_repr_css_set_property (css, "stroke", str.c_str());
            }
        } else if (lpeitem->style->fill.isColor()) {
            gchar c[64];
            sp_svg_write_color (c, sizeof(c), lpeitem->style->fill.value.color.toRGBA32(SP_SCALE24_TO_FLOAT(lpeitem->style->fill_opacity.value)));
            sp_repr_css_set_property (css, "stroke", c);
        } else {
            sp_repr_css_set_property (css, "stroke", "none");
        }

        Inkscape::CSSOStringStream os;
        os << fabs(line_width);
        sp_repr_css_set_property (css, "stroke-width", os.str().c_str());

        sp_repr_css_set_property(css, "fill", "none");

        sp_desktop_apply_css_recursive(item, css, true);
        sp_repr_css_attr_unref (css);
    }
}

using Geom::Piecewise;
using Geom::D2;
using Geom::SBasis;
// leave Geom::Path

static Geom::Path return_at_first_cusp(Geom::Path const & path_in, double /*smooth_tolerance*/ = 0.05)
{
    Geom::Path temp;

    for (unsigned i = 0; i < path_in.size(); i++) {
        temp.append(path_in[i]);
        if (Geom::get_nodetype(path_in[i], path_in[i + 1]) != Geom::NODE_SMOOTH ) {
            break;
        }
    }
    
    return temp;
}

Piecewise<D2<SBasis> > stretch_along(Piecewise<D2<SBasis> > pwd2_in, Geom::Path pattern, double width);

// actual effect

Geom::PathVector LPETaperStroke::doEffect_path(Geom::PathVector const& path_in)
{
    Geom::Path first_cusp = return_at_first_cusp(path_in[0]);
    Geom::Path last_cusp = return_at_first_cusp(path_in[0].reversed());

    bool zeroStart = false; // [distance from start taper knot -> start of path] == 0
    bool zeroEnd = false; // [distance from end taper knot -> end of path] == 0
    bool metInMiddle = false; // knots are touching
    
    // there is a pretty good chance that people will try to drag the knots
    // on top of each other, so block it

    unsigned size = path_in[0].size();
    if (size == first_cusp.size()) {
        // check to see if the knots were dragged over each other
        // if so, reset the end offset, but still allow the start offset.
        if ( attach_start >= (size - attach_end) ) {
            attach_end.param_set_value( size - attach_start );
            metInMiddle = true;
        }
    }
    
    if (attach_start == size - attach_end) {
        metInMiddle = true;
    }
    if (attach_end == size - attach_start) {
        metInMiddle = true;
    }

    // don't let it be integer (TODO this is stupid!)
    {
        if (double(unsigned(attach_start)) == attach_start) {
            attach_start.param_set_value(attach_start - 0.00001);
        }
        if (double(unsigned(attach_end)) == attach_end) {
            attach_end.param_set_value(attach_end -     0.00001);
        }
    }

    unsigned allowed_start = first_cusp.size();
    unsigned allowed_end = last_cusp.size();

    // don't let the knots be farther than they are allowed to be
    {
        if ((unsigned)attach_start >= allowed_start) {
            attach_start.param_set_value((double)allowed_start - 0.00001);
        }
        if ((unsigned)attach_end >= allowed_end) {
            attach_end.param_set_value((double)allowed_end - 0.00001);
        }
    }
    
    // don't let it be zero (this is stupid too!)
    if (attach_start < 0.0000001 || withinRange(double(attach_start), 0.00000001, 0.000001)) {
        attach_start.param_set_value( 0.0000001 );
        zeroStart = true;
    }
    if (attach_end < 0.0000001 || withinRange(double(attach_end), 0.00000001, 0.000001)) {
        attach_end.param_set_value( 0.0000001 );
        zeroEnd = true;
    }

    // Path::operator () means get point at time t
    start_attach_point = first_cusp(attach_start);
    end_attach_point = last_cusp(attach_end);
    Geom::PathVector pathv_out;

    // the following function just splits it up into three pieces.
    pathv_out = doEffect_simplePath(path_in);

    // now for the actual tapering. the stretch_along method (stolen from PaP) is used to accomplish this

    Geom::PathVector real_pathv;
    Geom::Path real_path;
    Geom::PathVector pat_vec;
    Piecewise<D2<SBasis> > pwd2;
    Geom::Path throwaway_path;

    if (!zeroStart) {
        // Construct the pattern
        std::stringstream pat_str;
        pat_str.imbue(std::locale::classic());
        pat_str << "M 1,0 C " << 1 - (double)smoothing << ",0 0,0.5 0,0.5 0,0.5 " << 1 - (double)smoothing << ",1 1,1";

        pat_vec = sp_svg_read_pathv(pat_str.str().c_str());
        pwd2.concat(stretch_along(pathv_out[0].toPwSb(), pat_vec[0], fabs(line_width)));
        throwaway_path = Geom::path_from_piecewise(pwd2, LPE_CONVERSION_TOLERANCE)[0];

        real_path.append(throwaway_path);
    }
    
    // if this condition happens to evaluate false, i.e. there was no space for a path to be drawn, it is simply skipped.
    // although this seems obvious, it can probably lead to bugs.
    if (!metInMiddle) {
        // append the outside outline of the path (goes with the direction of the path)
        throwaway_path = half_outline(pathv_out[1], fabs(line_width)/2., miter_limit, static_cast<LineJoinType>(join_type.get_value()));
        if (!zeroStart && real_path.size() >= 1 && throwaway_path.size() >= 1) {
            if (!Geom::are_near(real_path.finalPoint(), throwaway_path.initialPoint())) {
                real_path.appendNew<Geom::LineSegment>(throwaway_path.initialPoint());
            } else {
                real_path.setFinal(throwaway_path.initialPoint());
            }
        }
        real_path.append(throwaway_path);
    }

    if (!zeroEnd) {
        // append the ending taper
        std::stringstream pat_str_1;
        pat_str_1.imbue(std::locale::classic());
        pat_str_1 << "M 0,1 C " << (double)smoothing << ",1 1,0.5 1,0.5 1,0.5 " << double(smoothing) << ",0 0,0";
        pat_vec = sp_svg_read_pathv(pat_str_1.str().c_str());

        pwd2 = Piecewise<D2<SBasis> >();
        pwd2.concat(stretch_along(pathv_out[2].toPwSb(), pat_vec[0], fabs(line_width)));

        throwaway_path = Geom::path_from_piecewise(pwd2, LPE_CONVERSION_TOLERANCE)[0];
        if (!Geom::are_near(real_path.finalPoint(), throwaway_path.initialPoint()) && real_path.size() >= 1) {
            real_path.appendNew<Geom::LineSegment>(throwaway_path.initialPoint());
        } else {
            real_path.setFinal(throwaway_path.initialPoint());
        }
        real_path.append(throwaway_path);
    }
    
    if (!metInMiddle) {
        // append the inside outline of the path (against direction)
        throwaway_path = half_outline(pathv_out[1].reversed(), fabs(line_width)/2., miter_limit, static_cast<LineJoinType>(join_type.get_value()));
        
        if (!Geom::are_near(real_path.finalPoint(), throwaway_path.initialPoint()) && real_path.size() >= 1) {
            real_path.appendNew<Geom::LineSegment>(throwaway_path.initialPoint());
        } else {
            real_path.setFinal(throwaway_path.initialPoint());
        }
        real_path.append(throwaway_path);
    }
    
    if (!Geom::are_near(real_path.finalPoint(), real_path.initialPoint())) {
        real_path.appendNew<Geom::LineSegment>(real_path.initialPoint());
    } else {
        real_path.setFinal(real_path.initialPoint());
    }
    real_path.close();
    
    real_pathv.push_back(real_path);

    return real_pathv;
}

/**
 * @return Always returns a PathVector with three elements.
 *
 *  The positions of the effect knots are accessed to determine
 *  where exactly the input path should be split.
 */
Geom::PathVector LPETaperStroke::doEffect_simplePath(Geom::PathVector const & path_in)
{
    Geom::Coord endTime = path_in[0].size() - attach_end;

    Geom::Path p1 = path_in[0].portion(0., attach_start);
    Geom::Path p2 = path_in[0].portion(attach_start, endTime);
    Geom::Path p3 = path_in[0].portion(endTime, path_in[0].size());
    
    Geom::PathVector out;
    out.push_back(p1);
    out.push_back(p2);
    out.push_back(p3);
    
    return out;
}


/**
 * Most of the below function is verbatim from Pattern Along Path. However, it needed a little
 * tweaking to get it to work right in this case. Also, large portions of the effect have been
 * stripped out as I deemed them unnecessary for the relative simplicity of this effect.
 */
Piecewise<D2<SBasis> > stretch_along(Piecewise<D2<SBasis> > pwd2_in, Geom::Path pattern, double prop_scale)
{
    using namespace Geom;

    // Don't allow empty path parameter:
    if ( pattern.empty() ) {
        return pwd2_in;
    }

    /* Much credit should go to jfb and mgsloan of lib2geom development for the code below! */
    Piecewise<D2<SBasis> > output;
    std::vector<Piecewise<D2<SBasis> > > pre_output;

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(pattern.toPwSb());
    Piecewise<SBasis> x0 = Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y0 = Piecewise<SBasis>(patternd2[1]);
    OptInterval pattBndsX = bounds_exact(x0);
    OptInterval pattBndsY = bounds_exact(y0);
    if (pattBndsX && pattBndsY) {
        x0 -= pattBndsX->min();
        y0 -= pattBndsY->middle();

        double noffset = 0;
        double toffset = 0;
        // Prevent more than 90% overlap...

        y0+=noffset;

        std::vector<Piecewise<D2<SBasis> > > paths_in;
        paths_in = split_at_discontinuities(pwd2_in);

        for (unsigned idx = 0; idx < paths_in.size(); idx++) {
            Piecewise<D2<SBasis> > path_i = paths_in[idx];
            Piecewise<SBasis> x = x0;
            Piecewise<SBasis> y = y0;
            Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(path_i,2,.1);
            uskeleton = remove_short_cuts(uskeleton,.01);
            Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
            n = force_continuity(remove_short_cuts(n,.1));

            int nbCopies = 0;
            double scaling = (uskeleton.domain().extent() - toffset)/pattBndsX->extent();
            nbCopies = 1;

            double pattWidth = pattBndsX->extent() * scaling;

            if (scaling != 1.0) {
                x*=scaling;
            }
            if ( false ) {
                y*=(scaling*prop_scale);
            } else {
                if (prop_scale != 1.0) y *= prop_scale;
            }
            x += toffset;

            double offs = 0;
            for (int i=0; i<nbCopies; i++) {
                if (false) {
                    Piecewise<D2<SBasis> > output_piece = compose(uskeleton,x+offs)+y*compose(n,x+offs);
                    std::vector<Piecewise<D2<SBasis> > > splited_output_piece = split_at_discontinuities(output_piece);
                    pre_output.insert(pre_output.end(), splited_output_piece.begin(), splited_output_piece.end() );
                } else {
                    output.concat(compose(uskeleton,x+offs)+y*compose(n,x+offs));
                }
                offs+=pattWidth;
            }
        }
        return output;
    } else {
        return pwd2_in;
    }
}

void LPETaperStroke::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    KnotHolderEntity *e = new TpS::KnotHolderEntityAttachBegin(this);
    e->create(desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN, _("Start point of the taper"), SP_KNOT_SHAPE_CIRCLE);
    knotholder->add(e);

    KnotHolderEntity *f = new TpS::KnotHolderEntityAttachEnd(this);
    f->create(desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN, _("End point of the taper"), SP_KNOT_SHAPE_CIRCLE);
    knotholder->add(f);
}

namespace TpS {

void KnotHolderEntityAttachBegin::knot_set(Geom::Point const &p, Geom::Point const&/*origin*/, guint state)
{
    using namespace Geom;

    LPETaperStroke* lpe = dynamic_cast<LPETaperStroke *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    if (!SP_IS_SHAPE(lpe->sp_lpe_item)) {
        printf("WARNING: LPEItem is not a path!\n");
        return;
    }
    
    SPCurve* curve;
    if (!(curve = SP_SHAPE(lpe->sp_lpe_item)->getCurve())) {
        // oops
        return;
    }
    // in case you are wondering, the above are simply sanity checks. we never want to actually
    // use that object.
    
    Geom::PathVector pathv = lpe->pathvector_before_effect;
    
    Piecewise<D2<SBasis> > pwd2;
    Geom::Path p_in = return_at_first_cusp(pathv[0]);
    pwd2.concat(p_in.toPwSb());

    double t0 = nearest_time(s, pwd2);
    lpe->attach_start.param_set_value(t0);

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, true);
}
void KnotHolderEntityAttachEnd::knot_set(Geom::Point const &p, Geom::Point const& /*origin*/, guint state)
{
    using namespace Geom;

    LPETaperStroke* lpe = dynamic_cast<LPETaperStroke *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    if (!SP_IS_SHAPE(lpe->sp_lpe_item) ) {
        printf("WARNING: LPEItem is not a path!\n");
        return;
    }
    
    SPCurve* curve;
    if ( !(curve = SP_SHAPE(lpe->sp_lpe_item)->getCurve()) ) {
        // oops
        return;
    }
    Geom::PathVector pathv = lpe->pathvector_before_effect;
    Geom::Path p_in = return_at_first_cusp(pathv[0].reversed());
    Piecewise<D2<SBasis> > pwd2 = p_in.toPwSb();
    
    double t0 = nearest_time(s, pwd2);
    lpe->attach_end.param_set_value(t0);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point KnotHolderEntityAttachBegin::knot_get() const
{
    LPETaperStroke const * lpe = dynamic_cast<LPETaperStroke const*> (_effect);
    return lpe->start_attach_point;
}

Geom::Point KnotHolderEntityAttachEnd::knot_get() const
{
    LPETaperStroke const * lpe = dynamic_cast<LPETaperStroke const*> (_effect);
    return lpe->end_attach_point;
}

} // namespace TpS
} // namespace LivePathEffect
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

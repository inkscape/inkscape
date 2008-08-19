#define INKSCAPE_LPE_RULER_CPP

/** \file
 * LPE <ruler> implementation, see lpe-ruler.cpp.
 */

/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-ruler.h"
#include <2geom/piecewise.h>
#include <2geom/sbasis-geometric.h>
#include "inkscape.h"
#include "desktop.h"

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<MarkDirType> MarkDirData[] = {
    {MARKDIR_LEFT   , N_("Left"),  "left"},
    {MARKDIR_RIGHT  , N_("Right"), "right"},
    {MARKDIR_BOTH   , N_("Both"),  "both"},
};
static const Util::EnumDataConverter<MarkDirType> MarkDirTypeConverter(MarkDirData, sizeof(MarkDirData)/sizeof(*MarkDirData));

static const Util::EnumData<BorderMarkType> BorderMarkData[] = {
    {BORDERMARK_NONE    , N_("None"),  "none"},
    {BORDERMARK_START   , N_("Start"), "start"},
    {BORDERMARK_END     , N_("End"),   "end"},
    {BORDERMARK_BOTH    , N_("Both"),  "both"},
};
static const Util::EnumDataConverter<BorderMarkType> BorderMarkTypeConverter(BorderMarkData, sizeof(BorderMarkData)/sizeof(*BorderMarkData));

LPERuler::LPERuler(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    mark_distance(_("Mark distance"), _("Distance between successive ruler marks"), "mark_distance", &wr, this, 20.0),
    mark_length(_("Major length"), _("Length of major ruler marks"), "mark_length", &wr, this, 14.0),
    minor_mark_length(_("Minor length"), _("Length of minor ruler marks"), "minor_mark_length", &wr, this, 7.0),
    major_mark_steps(_("Major steps"), _("Draw a major mark every ... steps"), "major_mark_steps", &wr, this, 5),
    shift(_("Shift marks by"), _("Shift marks by this many steps"), "shift", &wr, this, 0),
    mark_dir(_("Mark direction"), _("Direction of marks (when viewing along the path from start to end)"), "mark_dir", MarkDirTypeConverter, &wr, this, MARKDIR_LEFT),
    offset(_("Offset"), _("Offset of first mark"), "offset", &wr, this, 0.0),
    border_marks(_("Border marks"), _("Choose whether to draw marks at the beginning and end of the path"), "border_marks", BorderMarkTypeConverter, &wr, this, BORDERMARK_BOTH)
{
    registerParameter(dynamic_cast<Parameter *>(&mark_distance));
    registerParameter(dynamic_cast<Parameter *>(&mark_length));
    registerParameter(dynamic_cast<Parameter *>(&minor_mark_length));
    registerParameter(dynamic_cast<Parameter *>(&major_mark_steps));
    registerParameter(dynamic_cast<Parameter *>(&shift));
    registerParameter(dynamic_cast<Parameter *>(&offset));
    registerParameter(dynamic_cast<Parameter *>(&mark_dir));
    registerParameter(dynamic_cast<Parameter *>(&border_marks));

    major_mark_steps.param_make_integer();
    major_mark_steps.param_set_range(1, 1000);
    shift.param_make_integer();

    mark_length.param_set_increments(1.0, 10.0);
    minor_mark_length.param_set_increments(1.0, 10.0);
    offset.param_set_increments(1.0, 10.0);
}

LPERuler::~LPERuler()
{

}

Geom::Point LPERuler::n_major;
Geom::Point LPERuler::n_minor;

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPERuler::ruler_mark(Geom::Point const &A, Geom::Point const &n, MarkType const &marktype)
{
    using namespace Geom;

    n_major = mark_length * n;
    n_minor = minor_mark_length * n;

    Point C, D;
    switch (marktype) {
        case MARK_MAJOR:
            C = A;
            D = A + n_major;
            if (mark_dir == MARKDIR_BOTH)
                C -= n_major;
            break;
        case MARK_MINOR:
            C = A;
            D = A + n_minor;
            if (mark_dir == MARKDIR_BOTH)
                C -= n_minor;
            break;
        default:
            // do nothing
            break;
    }

    Piecewise<D2<SBasis> > seg(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));
    return seg;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPERuler::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > pwd2_arclength = arc_length_parametrization(pwd2_in);
    Point A(pwd2_arclength.firstValue());
    Point B(pwd2_arclength.lastValue());
    double path_length = Geom::length(pwd2_arclength);

    Piecewise<D2<SBasis> > n = -rot90(unitVector(derivative(pwd2_arclength)));
    Piecewise<D2<SBasis> >output(pwd2_arclength);

    if (mark_dir == MARKDIR_RIGHT) {
        n *= -1.0;
    }

    int j = 0;
    const int mminterval = static_cast<int>(major_mark_steps);
    const int j_shift = static_cast<int>(shift) % mminterval;

    /* draw the ruler */
    if ((border_marks == BORDERMARK_START || border_marks == BORDERMARK_BOTH) && (offset != 0.0 || j_shift != 0))
        output.concat (ruler_mark(A, n.firstValue(), MARK_MAJOR));
    for (double t = offset; t < path_length; t += mark_distance, ++j) {
        if ((j % mminterval) == j_shift) {
            output.concat (ruler_mark(pwd2_arclength(t), n(t), MARK_MAJOR));
        } else {
            output.concat (ruler_mark(pwd2_arclength(t), n(t), MARK_MINOR));
        }
    }
    if (border_marks == BORDERMARK_END || border_marks == BORDERMARK_BOTH)
        output.concat (ruler_mark(B, n.lastValue(), MARK_MAJOR));

    return output;
}

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

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
    {BORDERMARK_NONE    , NC_("Border mark", "None"),  "none"},
    {BORDERMARK_START   , N_("Start"), "start"},
    {BORDERMARK_END     , N_("End"),   "end"},
    {BORDERMARK_BOTH    , N_("Both"),  "both"},
};
static const Util::EnumDataConverter<BorderMarkType> BorderMarkTypeConverter(BorderMarkData, sizeof(BorderMarkData)/sizeof(*BorderMarkData));

LPERuler::LPERuler(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    mark_distance(_("_Mark distance:"), _("Distance between successive ruler marks"), "mark_distance", &wr, this, 20.0),
    unit(_("Unit:"), _("Unit"), "unit", &wr, this),
    mark_length(_("Ma_jor length:"), _("Length of major ruler marks"), "mark_length", &wr, this, 14.0),
    minor_mark_length(_("Mino_r length:"), _("Length of minor ruler marks"), "minor_mark_length", &wr, this, 7.0),
    major_mark_steps(_("Major steps_:"), _("Draw a major mark every ... steps"), "major_mark_steps", &wr, this, 5),
    shift(_("Shift marks _by:"), _("Shift marks by this many steps"), "shift", &wr, this, 0),
    mark_dir(_("Mark direction:"), _("Direction of marks (when viewing along the path from start to end)"), "mark_dir", MarkDirTypeConverter, &wr, this, MARKDIR_LEFT),
    offset(_("_Offset:"), _("Offset of first mark"), "offset", &wr, this, 0.0),
    border_marks(_("Border marks:"), _("Choose whether to draw marks at the beginning and end of the path"), "border_marks", BorderMarkTypeConverter, &wr, this, BORDERMARK_BOTH)
{
    registerParameter(dynamic_cast<Parameter *>(&unit));
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

    double real_mark_length = mark_length;
    real_mark_length = Inkscape::Util::Quantity::convert(real_mark_length, unit.get_abbreviation(), "px");
    double real_minor_mark_length = minor_mark_length;
    real_minor_mark_length = Inkscape::Util::Quantity::convert(real_minor_mark_length, unit.get_abbreviation(), "px");

    n_major = real_mark_length * n;
    n_minor = real_minor_mark_length * n;
    if (mark_dir == MARKDIR_BOTH) {
        n_major = n_major * 0.5;
        n_minor = n_minor * 0.5;
    }

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

    const int mminterval = static_cast<int>(major_mark_steps);
    const int i_shift = static_cast<int>(shift) % mminterval;
    int sign = (mark_dir == MARKDIR_RIGHT ? 1 : -1 );

    Piecewise<D2<SBasis> >output(pwd2_in);
    Piecewise<D2<SBasis> >speed = derivative(pwd2_in);
    Piecewise<SBasis> arclength = arcLengthSb(pwd2_in);
    double totlength = arclength.lastValue();
    
    //find at which times to draw a mark:
    std::vector<double> s_cuts;

    double real_mark_distance = mark_distance;
    real_mark_distance = Inkscape::Util::Quantity::convert(real_mark_distance, unit.get_abbreviation(), "px");

    double real_offset = offset;
    real_offset = Inkscape::Util::Quantity::convert(real_offset, unit.get_abbreviation(), "px");
    for (double s = real_offset; s<totlength; s+=real_mark_distance){
        s_cuts.push_back(s);
    }
    std::vector<std::vector<double> > roots = multi_roots(arclength, s_cuts);
    std::vector<double> t_cuts;
    for (unsigned v=0; v<roots.size();v++){
        //FIXME: 2geom multi_roots solver seem to sometimes "repeat" solutions.
        //Here, we are supposed to have one and only one solution for each s.
        if(roots[v].size()>0) 
            t_cuts.push_back(roots[v][0]);
    }
    //draw the marks
    for (size_t i = 0; i < t_cuts.size(); i++) {
        Point A = pwd2_in(t_cuts[i]);
        Point n = rot90(unit_vector(speed(t_cuts[i])))*sign;
        if (static_cast<int>(i % mminterval) == i_shift) {
            output.concat (ruler_mark(A, n, MARK_MAJOR));
        } else {
            output.concat (ruler_mark(A, n, MARK_MINOR));
        }
    }
    //eventually draw a mark at start
    if ((border_marks == BORDERMARK_START || border_marks == BORDERMARK_BOTH) && (offset != 0.0 || i_shift != 0)){
        Point A = pwd2_in.firstValue();
        Point n = rot90(unit_vector(speed.firstValue()))*sign;
        output.concat (ruler_mark(A, n, MARK_MAJOR));
    }
    //eventually draw a mark at end
    if (border_marks == BORDERMARK_END || border_marks == BORDERMARK_BOTH){
        Point A = pwd2_in.lastValue();
        Point n = rot90(unit_vector(speed.lastValue()))*sign;
        //speed.lastValue() is somtimes wrong when the path is closed: a tiny line seg might added at the end to fix rounding errors...
        //TODO: Find a better fix!! (How do we know if the path was closed?)
        if ( A == pwd2_in.firstValue() &&
             speed.segs.size() > 1 &&
             speed.segs.back()[X].size() <= 1 &&
             speed.segs.back()[Y].size() <= 1 &&
             speed.segs.back()[X].tailError(0) <= 1e-10 &&
             speed.segs.back()[Y].tailError(0) <= 1e-10 
            ){
            n = rot90(unit_vector(speed.segs[speed.segs.size()-2].at1()))*sign;
        }
        output.concat (ruler_mark(A, n, MARK_MAJOR));
    }

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

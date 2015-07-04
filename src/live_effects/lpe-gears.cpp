/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright 2006 Michael G. Sloan <mgsloan@gmail.com>
 * Copyright 2006 Aaron Spike <aaron@ekips.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-gears.h"

#include <vector>

#include <glibmm/i18n.h>

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/path.h>

using std::vector;
using namespace Geom;

class Gear {
public:
    // pitch circles touch on two properly meshed gears
    // all measurements are taken from the pitch circle
    double pitch_diameter() {return (_number_of_teeth * _module) / M_PI;}
    double pitch_radius() {return pitch_diameter() / 2.0;}
    void pitch_radius(double R) {_module = (2 * M_PI * R) / _number_of_teeth;}

    // base circle serves as the basis for the involute toothe profile
    double base_diameter() {return pitch_diameter() * cos(_pressure_angle);}
    double base_radius() {return base_diameter() / 2.0;}

    // diametrical pitch
    double diametrical_pitch() {return _number_of_teeth / pitch_diameter();}

    // height of the tooth above the pitch circle
    double addendum() {return 1.0 / diametrical_pitch();}
    // depth of the tooth below the pitch circle
    double dedendum() {return addendum() + _clearance;}

    // root circle specifies the bottom of the fillet between teeth
    double root_radius() {return pitch_radius() - dedendum();}
    double root_diameter() {return root_radius() * 2.0;}

    // outer circle is the outside diameter of the gear
    double outer_radius() {return pitch_radius() + addendum();}
    double outer_diameter() {return outer_radius() * 2.0;}

    // angle covered by the tooth on the pitch circle
    double tooth_thickness_angle() {return M_PI / _number_of_teeth;}

    Geom::Point centre() {return _centre;}
    void centre(Geom::Point c) {_centre = c;}

    double angle() {return _angle;}
    void angle(double a) {_angle = a;}

    int number_of_teeth() {return _number_of_teeth;}

    Geom::Path path();
    Gear spawn(Geom::Point p);

    Gear(int n, double m, double phi) {
        _number_of_teeth = n;
        _module = m;
        _pressure_angle = phi;
        _clearance = 0.0;
        _angle = 0.0;
        _centre = Geom::Point(0.0,0.0);
    }
private:
    int _number_of_teeth;
    double _pressure_angle;
    double _module;
    double _clearance;
    double _angle;
    Geom::Point _centre;
    D2<SBasis> _involute(double start, double stop) {
        D2<SBasis> B;
        D2<SBasis> I;
        Linear bo = Linear(start,stop);

        B[0] = cos(bo,2);
        B[1] = sin(bo,2);

        I = B - Linear(0,1) * derivative(B);
        I = I*base_radius() + _centre;
        return I;
    }
    D2<SBasis> _arc(double start, double stop, double R) {
        D2<SBasis> B;
        Linear bo = Linear(start,stop);

        B[0] = cos(bo,2);
        B[1] = sin(bo,2);

        B = B*R + _centre;
        return B;
    }
    // angle of the base circle used to create the involute to a certain radius
    double involute_swath_angle(double R) {
        if (R <= base_radius()) return 0.0;
        return sqrt(R*R - base_radius()*base_radius())/base_radius();
    }

    // angle of the base circle between the origin of the involute and the intersection on another radius
    double involute_intersect_angle(double R) {
        if (R <= base_radius()) return 0.0;
        return (sqrt(R*R - base_radius()*base_radius())/base_radius()) - acos(base_radius()/R);
    }
};

static void
makeContinuous(D2<SBasis> &a, Point const b) {
    for(unsigned d=0;d<2;d++)
        a[d][0][0] = b[d];
}

Geom::Path Gear::path() {
    Geom::Path pb;

    // angle covered by a full tooth and fillet
    double tooth_rotation = 2.0 * tooth_thickness_angle();
    // angle covered by an involute
    double involute_advance = involute_intersect_angle(outer_radius()) - involute_intersect_angle(root_radius());
    // angle covered by the tooth tip
    double tip_advance = tooth_thickness_angle() - (2 * (involute_intersect_angle(outer_radius()) - involute_intersect_angle(pitch_radius())));
    // angle covered by the toothe root
    double root_advance = (tooth_rotation - tip_advance) - (2.0 * involute_advance);
    // begin drawing the involute at t if the root circle is larger than the base circle
    double involute_t = involute_swath_angle(root_radius())/involute_swath_angle(outer_radius());

    //rewind angle to start drawing from the leading edge of the tooth
    double first_tooth_angle = _angle - ((0.5 * tip_advance) + involute_advance);

    Geom::Point prev;
    for (int i=0; i < _number_of_teeth; i++)
    {
        double cursor = first_tooth_angle + (i * tooth_rotation);

        D2<SBasis> leading_I = compose(_involute(cursor, cursor + involute_swath_angle(outer_radius())), Linear(involute_t,1));
        if(i != 0) makeContinuous(leading_I, prev);
        pb.append(SBasisCurve(leading_I));
        cursor += involute_advance;
        prev = leading_I.at1();

        D2<SBasis> tip = _arc(cursor, cursor+tip_advance, outer_radius());
        makeContinuous(tip, prev);
        pb.append(SBasisCurve(tip));
        cursor += tip_advance;
        prev = tip.at1();

        cursor += involute_advance;
        D2<SBasis> trailing_I = compose(_involute(cursor, cursor - involute_swath_angle(outer_radius())), Linear(1,involute_t));
        makeContinuous(trailing_I, prev);
        pb.append(SBasisCurve(trailing_I));
        prev = trailing_I.at1();

        if (base_radius() > root_radius()) {
            Geom::Point leading_start = trailing_I.at1();
            Geom::Point leading_end = (root_radius() * unit_vector(leading_start - _centre)) + _centre;
            prev = leading_end;
            pb.appendNew<LineSegment>(leading_end);
        }

        D2<SBasis> root = _arc(cursor, cursor+root_advance, root_radius());
        makeContinuous(root, prev);
        pb.append(SBasisCurve(root));
        //cursor += root_advance;
        prev = root.at1();

        if (base_radius() > root_radius()) {
            Geom::Point trailing_start = root.at1();
            Geom::Point trailing_end = (base_radius() * unit_vector(trailing_start - _centre)) + _centre;
            pb.appendNew<LineSegment>(trailing_end);
            prev = trailing_end;
        }
    }

    return pb;
}

Gear Gear::spawn(Geom::Point p) {
    double radius = Geom::distance(this->centre(), p) - this->pitch_radius();
    int N  = (int) floor( (radius / this->pitch_radius()) * this->number_of_teeth() );

    Gear gear(N, _module, _pressure_angle);
    gear.centre(p);

    double a = atan2(p - this->centre());
    double new_angle = 0.0;
    if (gear.number_of_teeth() % 2 == 0)
        new_angle -= gear.tooth_thickness_angle();
    new_angle -= (_angle) * (pitch_radius() / gear.pitch_radius());
    new_angle += (a) * (pitch_radius() / gear.pitch_radius());
    gear.angle(new_angle + a);
    return gear;
}



// #################################################################



namespace Inkscape {
namespace LivePathEffect {


LPEGears::LPEGears(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    teeth(_("_Teeth:"), _("The number of teeth"), "teeth", &wr, this, 10),
    phi(_("_Phi:"), _("Tooth pressure angle (typically 20-25 deg).  The ratio of teeth not in contact."), "phi", &wr, this, 5)
{
    /* Tooth pressure angle: The angle between the tooth profile and a perpendicular to the pitch
     * circle, usually at the point where the pitch circle meets the tooth profile. Standard angles
     * are 20 and 25 degrees. The pressure angle affects the force that tends to separate mating
     * gears. A high pressure angle means that higher ratio of teeth not in contact. However, this
     * allows the teeth to have higher capacity and also allows fewer teeth without undercutting.
     */

    teeth.param_make_integer();
    teeth.param_set_range(3, 1e10);
    registerParameter( dynamic_cast<Parameter *>(&teeth) );
    registerParameter( dynamic_cast<Parameter *>(&phi) );
}

LPEGears::~LPEGears()
{

}

Geom::PathVector
LPEGears::doEffect_path (Geom::PathVector const &path_in)
{
    Geom::PathVector path_out;
    Geom::Path gearpath = path_in[0];

    Geom::Path::iterator it(gearpath.begin());
    if ( it == gearpath.end() ) return path_out;

    Gear * gear = new Gear(teeth, 200.0, phi * M_PI / 180);
    Geom::Point gear_centre = (*it).finalPoint();
    gear->centre(gear_centre);
    gear->angle(atan2((*it).initialPoint() - gear_centre));

    ++it;
	if ( it == gearpath.end() ) return path_out;
    gear->pitch_radius(Geom::distance(gear_centre, (*it).finalPoint()));

    path_out.push_back( gear->path());

    for (++it; it != gearpath.end() ; ++it) {
        // iterate through Geom::Curve in path_in
        Gear* gearnew = new Gear(gear->spawn( (*it).finalPoint() ));
        path_out.push_back( gearnew->path() );
        delete gear;
        gear = gearnew;
    }
    delete gear;

    return path_out;
}

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

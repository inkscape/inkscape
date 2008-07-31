#define INKSCAPE_LPE_RULER_CPP

/** \file
 * LPE <ruler> implementation, see lpe-ruler.cpp.
 */

/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-ruler.h"
#include <2geom/piecewise.h>
#include "inkscape.h"
#include "desktop.h"

namespace Inkscape {
namespace LivePathEffect {

LPERuler::LPERuler(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    mark_distance(_("Mark distance"), _("Distance between ruler marks"), "mark_distance", &wr, this, 20),
    mark_length(_("Mark length"), _("Length of ruler marks"), "mark_length", &wr, this, 10),
    scale(_("Scale factor"), _("Scale factor for ruler distance (only affects on-canvas display of ruler length)"), "scale", &wr, this, 1.0),
    info_text(this),
    unit(_("Unit"), _("Unit"), "unit", &wr, this)
{
    registerParameter(dynamic_cast<Parameter *>(&mark_distance));
    registerParameter(dynamic_cast<Parameter *>(&mark_length));
    registerParameter(dynamic_cast<Parameter *>(&scale));
    registerParameter(dynamic_cast<Parameter *>(&info_text));
    registerParameter(dynamic_cast<Parameter *>(&unit));

    mark_distance.param_make_integer();
    mark_length.param_make_integer();
}

LPERuler::~LPERuler()
{

}

enum MarkType {
    RULER_MARK_BORDER,
    RULER_MARK_MAJOR,
    RULER_MARK_MINOR
};

static Geom::Piecewise<Geom::D2<Geom::SBasis> >
ruler_mark(Geom::Point A, Geom::Point n, MarkType marktype)
{
    using namespace Geom;

    Point C, D;
    switch (marktype) {
        case RULER_MARK_BORDER:
            C = A - 1.5 * n;
            D = A + 1.5 * n;
            break;
        case RULER_MARK_MAJOR:
            C = A;
            D = A + 1.5 * n;
            break;
        case RULER_MARK_MINOR:
            C = A;
            D = A + n;
            break;
    }

    Piecewise<D2<SBasis> > seg(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));
    return seg;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPERuler::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Point A(pwd2_in.firstValue());
    Point B(pwd2_in.lastValue());

    Piecewise<D2<SBasis> >output(D2<SBasis>(Linear(A[X], B[X]), Linear(A[Y], B[Y])));

    Point dir(unit_vector(B - A));
    Point n(-rot90(dir) * mark_length);
    double length = L2(B - A);

    /* convert the measured length to the correct unit ... */
    double lengthval = length * scale;
    gboolean success = sp_convert_distance(&lengthval, &sp_unit_get_by_id(SP_UNIT_PX), unit);

    /* ... set it as the canvas text ... */
    gchar *dist = g_strdup_printf("%.2f %s", lengthval, success ? unit.get_abbreviation() : "px");
    info_text.param_setValue(dist);
    g_free(dist);

    /* ... and adjust the text's position on canvas */
    double angle = Geom::angle_between(dir, Geom::Point(1,0));
    info_text.setPos((A + B) / 2 + 2.0 * n);
    info_text.setAnchor(std::sin(angle), -std::cos(angle));

    /* draw the actual ruler */
    Point C, D;
    C = A - n;
    D = A + n;
    output.concat (ruler_mark(A, n, RULER_MARK_BORDER));
    int j = 0;
    for (double i = 0; i < length; i += mark_distance, ++j) {
        if ((j % 5) == 0) {
            output.concat (ruler_mark(A + dir * i, n, RULER_MARK_MAJOR));
        } else {
            output.concat (ruler_mark(A + dir * i, n, RULER_MARK_MINOR));
        }
    }
    output.concat (ruler_mark(B, n, RULER_MARK_BORDER));

    return output;
}

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

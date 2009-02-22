#define INKSCAPE_LPE_CIRCLE_3PTS_CPP
/** \file
 * LPE "Circle through 3 points" implementation
 */

/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-circle_3pts.h"

// You might need to include other 2geom files. You can add them here:
#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {

LPECircle3Pts::LPECircle3Pts(LivePathEffectObject *lpeobject) :
    Effect(lpeobject)
{
}

LPECircle3Pts::~LPECircle3Pts()
{
}

static void _circle(Geom::Point center, double radius, std::vector<Geom::Path> &path_out) {
    using namespace Geom;

    Geom::Path pb;

    D2<SBasis> B;
    Linear bo = Linear(0, 2 * M_PI);

    B[0] = cos(bo,4);
    B[1] = sin(bo,4);

    B = B * radius + center;

    pb.append(SBasisCurve(B));

    path_out.push_back(pb);
}

static void _circle3(Geom::Point const &A, Geom::Point const &B, Geom::Point const &C, std::vector<Geom::Path> &path_out) {
    using namespace Geom;

    Point D = (A + B)/2;
    Point E = (B + C)/2;

    Point v = (B - A).ccw();
    Point w = (C - B).ccw();
    double det = -v[0] * w[1] + v[1] * w[0];

    Point F = E - D;
    double lambda = 1/det * (-w[1] * F[0] + w[0] * F[1]);

    Point M = D + v * lambda;
    double radius = L2(M - A);

    _circle(M, radius, path_out);
}

std::vector<Geom::Path>
LPECircle3Pts::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    std::vector<Geom::Path> path_out = std::vector<Geom::Path>();

    // we assume that the path has >= 3 nodes
    Geom::Point A = path_in[0].initialPoint();
    Geom::Point B = path_in[0].pointAt(1);
    Geom::Point C = path_in[0].pointAt(2);

    _circle3(A, B, C, path_out);

    return path_out;
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

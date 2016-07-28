/** \file
 * LPE "Ellipse through 5 points" implementation
 */

/*
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-ellipse_5pts.h"

// You might need to include other 2geom files. You can add them here:
#include <glibmm/i18n.h>
#include <2geom/path.h>
#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/path-sink.h>
#include "inkscape.h"
#include "desktop.h"
#include "message-stack.h"

namespace Inkscape {
namespace LivePathEffect {

LPEEllipse5Pts::LPEEllipse5Pts(LivePathEffectObject *lpeobject) :
    Effect(lpeobject)
{
    //perceived_path = true;
}

LPEEllipse5Pts::~LPEEllipse5Pts()
{
}

static double _det3(double (*mat)[3])
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = i + 1; j < 3; j++)
        {
            for (int k = i + 1; k < 3; k++)
            {
                mat[j][k] = (mat[j][k] * mat[i][i] - mat[j][i] * mat[i][k]);
                if (i) mat[j][k] /= mat[i-1][i-1];
            }
        }
    }
    return mat[2][2];
}
static double _det5(double (*mat)[5])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 5; j++)
        {
            for (int k = i + 1; k < 5; k++)
            {
                mat[j][k] = (mat[j][k] * mat[i][i] - mat[j][i] * mat[i][k]);
                if (i) mat[j][k] /= mat[i-1][i-1];
            }
        }
    }
    return mat[4][4];
}

Geom::PathVector
LPEEllipse5Pts::doEffect_path (Geom::PathVector const & path_in)
{
    Geom::PathVector path_out = Geom::PathVector();

    if (path_in[0].size() < 4) {
        
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Five points required for constructing an ellipse"));
        return path_in;
    }
    // we assume that the path has >= 3 nodes
    Geom::Point A = path_in[0].initialPoint();
    Geom::Point B = path_in[0].pointAt(1);
    Geom::Point C = path_in[0].pointAt(2);
    Geom::Point D = path_in[0].pointAt(3);
    Geom::Point E = path_in[0].pointAt(4);

    using namespace Geom;

    double rowmajor_matrix[5][6] =
    {
        {A.x()*A.x(), A.x()*A.y(), A.y()*A.y(), A.x(), A.y(), 1},
        {B.x()*B.x(), B.x()*B.y(), B.y()*B.y(), B.x(), B.y(), 1},
        {C.x()*C.x(), C.x()*C.y(), C.y()*C.y(), C.x(), C.y(), 1},
        {D.x()*D.x(), D.x()*D.y(), D.y()*D.y(), D.x(), D.y(), 1},
        {E.x()*E.x(), E.x()*E.y(), E.y()*E.y(), E.x(), E.y(), 1}
    };
    
    double mat_a[5][5] =
    {
        {rowmajor_matrix[0][1], rowmajor_matrix[1][1], rowmajor_matrix[2][1], rowmajor_matrix[3][1], rowmajor_matrix[4][1]},
        {rowmajor_matrix[0][2], rowmajor_matrix[1][2], rowmajor_matrix[2][2], rowmajor_matrix[3][2], rowmajor_matrix[4][2]},
        {rowmajor_matrix[0][3], rowmajor_matrix[1][3], rowmajor_matrix[2][3], rowmajor_matrix[3][3], rowmajor_matrix[4][3]},
        {rowmajor_matrix[0][4], rowmajor_matrix[1][4], rowmajor_matrix[2][4], rowmajor_matrix[3][4], rowmajor_matrix[4][4]},
        {rowmajor_matrix[0][5], rowmajor_matrix[1][5], rowmajor_matrix[2][5], rowmajor_matrix[3][5], rowmajor_matrix[4][5]}
    };
    double mat_b[5][5] =
    {
        {rowmajor_matrix[0][0], rowmajor_matrix[1][0], rowmajor_matrix[2][0], rowmajor_matrix[3][0], rowmajor_matrix[4][0]},
        {rowmajor_matrix[0][2], rowmajor_matrix[1][2], rowmajor_matrix[2][2], rowmajor_matrix[3][2], rowmajor_matrix[4][2]},
        {rowmajor_matrix[0][3], rowmajor_matrix[1][3], rowmajor_matrix[2][3], rowmajor_matrix[3][3], rowmajor_matrix[4][3]},
        {rowmajor_matrix[0][4], rowmajor_matrix[1][4], rowmajor_matrix[2][4], rowmajor_matrix[3][4], rowmajor_matrix[4][4]},
        {rowmajor_matrix[0][5], rowmajor_matrix[1][5], rowmajor_matrix[2][5], rowmajor_matrix[3][5], rowmajor_matrix[4][5]}
    };
    double mat_c[5][5] =
    {
        {rowmajor_matrix[0][0], rowmajor_matrix[1][0], rowmajor_matrix[2][0], rowmajor_matrix[3][0], rowmajor_matrix[4][0]},
        {rowmajor_matrix[0][1], rowmajor_matrix[1][1], rowmajor_matrix[2][1], rowmajor_matrix[3][1], rowmajor_matrix[4][1]},
        {rowmajor_matrix[0][3], rowmajor_matrix[1][3], rowmajor_matrix[2][3], rowmajor_matrix[3][3], rowmajor_matrix[4][3]},
        {rowmajor_matrix[0][4], rowmajor_matrix[1][4], rowmajor_matrix[2][4], rowmajor_matrix[3][4], rowmajor_matrix[4][4]},
        {rowmajor_matrix[0][5], rowmajor_matrix[1][5], rowmajor_matrix[2][5], rowmajor_matrix[3][5], rowmajor_matrix[4][5]}
    };
    double mat_d[5][5] =
    {
        {rowmajor_matrix[0][0], rowmajor_matrix[1][0], rowmajor_matrix[2][0], rowmajor_matrix[3][0], rowmajor_matrix[4][0]},
        {rowmajor_matrix[0][1], rowmajor_matrix[1][1], rowmajor_matrix[2][1], rowmajor_matrix[3][1], rowmajor_matrix[4][1]},
        {rowmajor_matrix[0][2], rowmajor_matrix[1][2], rowmajor_matrix[2][2], rowmajor_matrix[3][2], rowmajor_matrix[4][2]},
        {rowmajor_matrix[0][4], rowmajor_matrix[1][4], rowmajor_matrix[2][4], rowmajor_matrix[3][4], rowmajor_matrix[4][4]},
        {rowmajor_matrix[0][5], rowmajor_matrix[1][5], rowmajor_matrix[2][5], rowmajor_matrix[3][5], rowmajor_matrix[4][5]}
    };
    double mat_e[5][5] =
    {
        {rowmajor_matrix[0][0], rowmajor_matrix[1][0], rowmajor_matrix[2][0], rowmajor_matrix[3][0], rowmajor_matrix[4][0]},
        {rowmajor_matrix[0][1], rowmajor_matrix[1][1], rowmajor_matrix[2][1], rowmajor_matrix[3][1], rowmajor_matrix[4][1]},
        {rowmajor_matrix[0][2], rowmajor_matrix[1][2], rowmajor_matrix[2][2], rowmajor_matrix[3][2], rowmajor_matrix[4][2]},
        {rowmajor_matrix[0][3], rowmajor_matrix[1][3], rowmajor_matrix[2][3], rowmajor_matrix[3][3], rowmajor_matrix[4][3]},
        {rowmajor_matrix[0][5], rowmajor_matrix[1][5], rowmajor_matrix[2][5], rowmajor_matrix[3][5], rowmajor_matrix[4][5]}
    };
    double mat_f[5][5] =
    {
        {rowmajor_matrix[0][0], rowmajor_matrix[1][0], rowmajor_matrix[2][0], rowmajor_matrix[3][0], rowmajor_matrix[4][0]},
        {rowmajor_matrix[0][1], rowmajor_matrix[1][1], rowmajor_matrix[2][1], rowmajor_matrix[3][1], rowmajor_matrix[4][1]},
        {rowmajor_matrix[0][2], rowmajor_matrix[1][2], rowmajor_matrix[2][2], rowmajor_matrix[3][2], rowmajor_matrix[4][2]},
        {rowmajor_matrix[0][3], rowmajor_matrix[1][3], rowmajor_matrix[2][3], rowmajor_matrix[3][3], rowmajor_matrix[4][3]},
        {rowmajor_matrix[0][4], rowmajor_matrix[1][4], rowmajor_matrix[2][4], rowmajor_matrix[3][4], rowmajor_matrix[4][4]}
    };
    
    double a1 = _det5(mat_a);
    double b1 = -_det5(mat_b);
    double c1 = _det5(mat_c);
    double d1 = -_det5(mat_d);
    double e1 = _det5(mat_e);
    double f1 = -_det5(mat_f);
    
    double mat_check[][3] = 
    {
        {a1, b1/2, d1/2},
        {b1/2, c1, e1/2},
        {d1/2, e1/2, f1}
    };
    
    if (_det3(mat_check) == 0 || a1*c1 - b1*b1/4 <= 0) {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No ellipse found for specified points"));
        return path_in;
    }
    
    Geom::Ellipse el(a1, b1, c1, d1, e1, f1);
    
    double s, e;
    double x0, y0, x1, y1, x2, y2, x3, y3;
    double len;

    // figure out if we have a slice, guarding against rounding errors

    Path p(Geom::Point(cos(0), sin(0)));

    double end = 2 * M_PI;
    for (s = 0; s < end; s += M_PI_2) {
        e = s + M_PI_2;
        if (e > end)
            e = end;
        len = 4*tan((e - s)/4)/3;
        x0 = cos(s);
        y0 = sin(s);
        x1 = x0 + len * cos(s + M_PI_2);
        y1 = y0 + len * sin(s + M_PI_2);
        x3 = cos(e);
        y3 = sin(e);
        x2 = x3 + len * cos(e - M_PI_2);
        y2 = y3 + len * sin(e - M_PI_2);
        p.appendNew<Geom::CubicBezier>(Geom::Point(x1,y1), Geom::Point(x2,y2), Geom::Point(x3,y3));
    }
    
    Geom::Affine aff = Geom::Scale(el.rays()) * Geom::Rotate(el.rotationAngle()) * Geom::Translate(el.center());
    
    path_out.push_back(p * aff);

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

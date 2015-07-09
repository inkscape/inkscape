/*
 *  PathConversion.cpp
 *  nlivarot
 *
 *  Created by fred on Mon Nov 03 2003.
 *
 */

#include <glib.h>
#include <2geom/transforms.h>
#include "Path.h"
#include "Shape.h"
#include "livarot/path-description.h"

/*
 * path description -> polyline
 * and Path -> Shape (the Fill() function at the bottom)
 * nathing fancy here: take each command and append an approximation of it to the polyline
 */

void Path::ConvertWithBackData(double treshhold)
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }

    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    }

    SetBackData(true);
    ResetPoints();
    if ( descr_cmd.empty() ) {
        return;
    }

    Geom::Point curX;
    int curP = 1;
    int lastMoveTo = -1;

    // The initial moveto.
    {
        int const firstTyp = descr_cmd[0]->getType();
        if ( firstTyp == descr_moveto ) {
            curX = dynamic_cast<PathDescrMoveTo *>(descr_cmd[0])->p;
        } else {
            curP = 0;
            curX[Geom::X] = curX[Geom::Y] = 0;
        }
        lastMoveTo = AddPoint(curX, 0, 0.0, true);
    }

    // And the rest, one by one.
    while ( curP < int(descr_cmd.size()) ) {

        int const nType = descr_cmd[curP]->getType();
        Geom::Point nextX;

        switch (nType) {
            case descr_forced: {
                AddForcedPoint(curX, curP, 1.0);
                curP++;
                break;
            }

            case descr_moveto: {
                PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo*>(descr_cmd[curP]);
                nextX = nData->p;
                lastMoveTo = AddPoint(nextX, curP, 0.0, true);
                // et on avance
                curP++;
                break;
            }

            case descr_close: {
                nextX = pts[lastMoveTo].p;
                int n = AddPoint(nextX, curP, 1.0, false);
                if (n > 0) pts[n].closed = true;
                curP++;
                break;
            }

            case descr_lineto: {
                PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[curP]);
                nextX = nData->p;
                AddPoint(nextX,curP,1.0,false);
                // et on avance
                curP++;
                break;
            }

            case descr_cubicto: {
                PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[curP]);
                nextX = nData->p;
                RecCubicTo(curX, nData->start, nextX, nData->end, treshhold, 8, 0.0, 1.0, curP);
                AddPoint(nextX, curP, 1.0, false);
                // et on avance
                curP++;
                break;
            }

            case descr_arcto: {
                PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[curP]);
                nextX = nData->p;
                DoArc(curX, nextX, nData->rx, nData->ry, nData->angle, nData->large, nData->clockwise, treshhold, curP);
                AddPoint(nextX, curP, 1.0, false);
                // et on avance
                curP++;
                break;
            }

            case descr_bezierto: {
                PathDescrBezierTo *nBData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[curP]);
                int nbInterm = nBData->nb;
                nextX = nBData->p;

                int ip = curP + 1;
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                if ( nbInterm >= 1 ) {
                    Geom::Point bx = curX;
                    Geom::Point dx = nData->p;
                    Geom::Point cx = 2 * bx - dx;

                    ip++;
                    nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                    for (int k = 0; k < nbInterm - 1; k++) {
                        bx = cx;
                        cx = dx;

                        dx = nData->p;
                        ip++;
                        nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                        Geom::Point stx;
                        stx = (bx + cx) / 2;
                        if ( k > 0 ) {
                            AddPoint(stx,curP - 1+k,1.0,false);
                        }

                        {
                            Geom::Point mx;
                            mx = (cx + dx) / 2;
                            RecBezierTo(cx, stx, mx, treshhold, 8, 0.0, 1.0, curP + k);
                        }
                    }
                    {
                        bx = cx;
                        cx = dx;

                        dx = nextX;
                        dx = 2 * dx - cx;

                        Geom::Point stx;
                        stx = (bx + cx) / 2;

                        if ( nbInterm > 1 ) {
                            AddPoint(stx, curP + nbInterm - 2, 1.0, false);
                        }

                        {
                            Geom::Point mx;
                            mx = (cx + dx) / 2;
                            RecBezierTo(cx, stx, mx, treshhold, 8, 0.0, 1.0, curP + nbInterm - 1);
                        }
                    }

                }


                AddPoint(nextX, curP - 1 + nbInterm, 1.0, false);

                // et on avance
                curP += 1 + nbInterm;
                break;
            }
        }
        curX = nextX;
    }
}


void Path::Convert(double treshhold)
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }

    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    }

    SetBackData(false);
    ResetPoints();
    if ( descr_cmd.empty() ) {
        return;
    }

    Geom::Point curX;
    int curP = 1;
    int lastMoveTo = 0;

    // le moveto
    {
        int const firstTyp = descr_cmd[0]->getType();
        if ( firstTyp == descr_moveto ) {
            curX = dynamic_cast<PathDescrMoveTo *>(descr_cmd[0])->p;
        } else {
            curP = 0;
            curX[0] = curX[1] = 0;
        }
        lastMoveTo = AddPoint(curX, true);
    }
    descr_cmd[0]->associated = lastMoveTo;

    // et le reste, 1 par 1
    while ( curP < int(descr_cmd.size()) ) {

        int const nType = descr_cmd[curP]->getType();
        Geom::Point nextX;

        switch (nType) {
            case descr_forced: {
                descr_cmd[curP]->associated = AddForcedPoint(curX);
                curP++;
                break;
            }

            case descr_moveto: {
                PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[curP]);
                nextX = nData->p;
                lastMoveTo = AddPoint(nextX, true);
                descr_cmd[curP]->associated = lastMoveTo;

                // et on avance
                curP++;
                break;
            }

            case descr_close: {
                nextX = pts[lastMoveTo].p;
                descr_cmd[curP]->associated = AddPoint(nextX, false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                if ( descr_cmd[curP]->associated > 0 ) {
                    pts[descr_cmd[curP]->associated].closed = true;
                }
                curP++;
                break;
            }

            case descr_lineto: {
                PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[curP]);
                nextX = nData->p;
                descr_cmd[curP]->associated = AddPoint(nextX, false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                // et on avance
                curP++;
                break;
            }

            case descr_cubicto: {
                PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[curP]);
                nextX = nData->p;
                RecCubicTo(curX, nData->start, nextX, nData->end, treshhold, 8);
                descr_cmd[curP]->associated = AddPoint(nextX,false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                // et on avance
                curP++;
                break;
            }

            case descr_arcto: {
                PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[curP]);
                nextX = nData->p;
                DoArc(curX, nextX, nData->rx, nData->ry, nData->angle, nData->large, nData->clockwise, treshhold);
                descr_cmd[curP]->associated = AddPoint(nextX, false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                // et on avance
                curP++;
                break;
            }

            case descr_bezierto: {
                PathDescrBezierTo *nBData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[curP]);
                int nbInterm = nBData->nb;
                nextX = nBData->p;
                int curBD = curP;

                curP++;
                int ip = curP;
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                if ( nbInterm == 1 ) {
                    Geom::Point const midX = nData->p;
                    RecBezierTo(midX, curX, nextX, treshhold, 8);
                } else if ( nbInterm > 1 ) {
                    Geom::Point bx = curX;
                    Geom::Point dx = nData->p;
                    Geom::Point cx = 2 * bx - dx;

                    ip++;
                    nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                    for (int k = 0; k < nbInterm - 1; k++) {
                        bx = cx;
                        cx = dx;

                        dx = nData->p;
                        ip++;
                        nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                        Geom::Point stx = (bx + cx) / 2;
                        if ( k > 0 ) {
                            descr_cmd[ip - 2]->associated = AddPoint(stx, false);
                            if ( descr_cmd[ip - 2]->associated < 0 ) {
                                if ( curP == 0 ) {
                                    descr_cmd[ip - 2]->associated = 0;
                                } else {
                                    descr_cmd[ip - 2]->associated = descr_cmd[ip - 3]->associated;
                                }
                            }
                        }

                        {
                            Geom::Point const mx = (cx + dx) / 2;
                            RecBezierTo(cx, stx, mx, treshhold, 8);
                        }
                    }

                    {
                        bx = cx;
                        cx = dx;

                        dx = nextX;
                        dx = 2 * dx - cx;

                        Geom::Point stx = (bx + cx) / 2;

                        descr_cmd[ip - 1]->associated = AddPoint(stx, false);
                        if ( descr_cmd[ip - 1]->associated < 0 ) {
                            if ( curP == 0 ) {
                                descr_cmd[ip - 1]->associated = 0;
                            } else {
                                descr_cmd[ip - 1]->associated = descr_cmd[ip - 2]->associated;
                            }
                        }

                        {
                            Geom::Point mx = (cx + dx) / 2;
                            RecBezierTo(cx, stx, mx, treshhold, 8);
                        }
                    }
                }

                descr_cmd[curBD]->associated = AddPoint(nextX, false);
                if ( descr_cmd[curBD]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curBD]->associated = 0;
                    } else {
                        descr_cmd[curBD]->associated = descr_cmd[curBD - 1]->associated;
                    }
                }

                // et on avance
                curP += nbInterm;
                break;
            }
        }

        curX = nextX;
    }
}

void Path::ConvertEvenLines(double treshhold)
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }

    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    }

    SetBackData(false);
    ResetPoints();
    if ( descr_cmd.empty() ) {
        return;
    }

    Geom::Point curX;
    int curP = 1;
    int lastMoveTo = 0;

    // le moveto
    {
        int const firstTyp = descr_cmd[0]->getType();
        if ( firstTyp == descr_moveto ) {
            curX = dynamic_cast<PathDescrMoveTo *>(descr_cmd[0])->p;
        } else {
            curP = 0;
            curX[0] = curX[1] = 0;
        }
        lastMoveTo = AddPoint(curX, true);
    }
    descr_cmd[0]->associated = lastMoveTo;

    // et le reste, 1 par 1
    while ( curP < int(descr_cmd.size()) ) {

        int const nType = descr_cmd[curP]->getType();
        Geom::Point nextX;

        switch (nType) {
            case descr_forced: {
                descr_cmd[curP]->associated = AddForcedPoint(curX);
                curP++;
                break;
            }

            case descr_moveto: {
                PathDescrMoveTo* nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[curP]);
                nextX = nData->p;
                lastMoveTo = AddPoint(nextX,true);
                descr_cmd[curP]->associated = lastMoveTo;

                // et on avance
                curP++;
                break;
            }

            case descr_close: {
                nextX = pts[lastMoveTo].p;
                {
                    Geom::Point nexcur;
                    nexcur = nextX - curX;
                    const double segL = Geom::L2(nexcur);
                    if ( (segL > treshhold) && (treshhold > 0) ) {
                        for (double i = treshhold; i < segL; i += treshhold) {
                            Geom::Point nX;
                            nX = (segL - i) * curX + i * nextX;
                            nX /= segL;
                            AddPoint(nX);
                        }
                    }
                }

                descr_cmd[curP]->associated = AddPoint(nextX,false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                if ( descr_cmd[curP]->associated > 0 ) {
                    pts[descr_cmd[curP]->associated].closed = true;
                }
                curP++;
                break;
            }

            case descr_lineto: {
                PathDescrLineTo* nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[curP]);
                nextX = nData->p;
                Geom::Point nexcur = nextX - curX;
                const double segL = L2(nexcur);
                if ( (segL > treshhold) && (treshhold > 0)) {
                    for (double i = treshhold; i < segL; i += treshhold) {
                        Geom::Point nX = ((segL - i) * curX + i * nextX) / segL;
                        AddPoint(nX);
                    }
                }

                descr_cmd[curP]->associated = AddPoint(nextX,false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                // et on avance
                curP++;
                break;
            }

            case descr_cubicto: {
                PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[curP]);
                nextX = nData->p;
                RecCubicTo(curX, nData->start, nextX, nData->end, treshhold, 8, 4 * treshhold);
                descr_cmd[curP]->associated = AddPoint(nextX, false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }
                // et on avance
                curP++;
                break;
            }

            case descr_arcto: {
                PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[curP]);
                nextX = nData->p;
                DoArc(curX, nextX, nData->rx, nData->ry, nData->angle, nData->large, nData->clockwise, treshhold);
                descr_cmd[curP]->associated =AddPoint(nextX, false);
                if ( descr_cmd[curP]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curP]->associated = 0;
                    } else {
                        descr_cmd[curP]->associated = descr_cmd[curP - 1]->associated;
                    }
                }

                // et on avance
                curP++;
                break;
            }

            case descr_bezierto: {
                PathDescrBezierTo *nBData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[curP]);
                int nbInterm = nBData->nb;
                nextX = nBData->p;
                int curBD = curP;

                curP++;
                int ip = curP;
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                if ( nbInterm == 1 ) {
                    Geom::Point const midX = nData->p;
                    RecBezierTo(midX, curX, nextX, treshhold, 8, 4 * treshhold);
                } else if ( nbInterm > 1 ) {
                    Geom::Point bx = curX;
                    Geom::Point dx = nData->p;
                    Geom::Point cx = 2 * bx - dx;

                    ip++;
                    nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                    for (int k = 0; k < nbInterm - 1; k++) {
                        bx = cx;
                        cx = dx;
                        dx = nData->p;

                        ip++;
                        nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[ip]);

                        Geom::Point stx = (bx+cx) / 2;
                        if ( k > 0 ) {
                            descr_cmd[ip - 2]->associated = AddPoint(stx, false);
                            if ( descr_cmd[ip - 2]->associated < 0 ) {
                                if ( curP == 0 ) {
                                    descr_cmd[ip- 2]->associated = 0;
                                } else {
                                    descr_cmd[ip - 2]->associated = descr_cmd[ip - 3]->associated;
                                }
                            }
                        }

                        {
                            Geom::Point const mx = (cx + dx) / 2;
                            RecBezierTo(cx, stx, mx, treshhold, 8, 4 * treshhold);
                        }
                    }

                    {
                        bx = cx;
                        cx = dx;

                        dx = nextX;
                        dx = 2 * dx - cx;

                        Geom::Point const stx = (bx + cx) / 2;

                        descr_cmd[ip - 1]->associated = AddPoint(stx, false);
                        if ( descr_cmd[ip - 1]->associated < 0 ) {
                            if ( curP == 0 ) {
                                descr_cmd[ip - 1]->associated = 0;
                            } else {
                                descr_cmd[ip - 1]->associated = descr_cmd[ip - 2]->associated;
                            }
                        }

                        {
                            Geom::Point const mx = (cx + dx) / 2;
                            RecBezierTo(cx, stx, mx, treshhold, 8, 4 * treshhold);
                        }
                    }
                }

                descr_cmd[curBD]->associated = AddPoint(nextX, false);
                if ( descr_cmd[curBD]->associated < 0 ) {
                    if ( curP == 0 ) {
                        descr_cmd[curBD]->associated = 0;
                    } else {
                        descr_cmd[curBD]->associated = descr_cmd[curBD - 1]->associated;
                    }
                }

                // et on avance
                curP += nbInterm;
                break;
            }
        }
        if ( Geom::LInfty(curX - nextX) > 0.00001 ) {
            curX = nextX;
        }
    }
}

const Geom::Point Path::PrevPoint(int i) const
{
    /* TODO: I suspect this should assert `(unsigned) i < descr_nb'.  We can probably change
       the argument to unsigned.  descr_nb should probably be changed to unsigned too. */
    g_assert( i >= 0 );
    switch ( descr_cmd[i]->getType() ) {
        case descr_moveto: {
            PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
            return nData->p;
        }
        case descr_lineto: {
            PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
            return nData->p;
        }
        case descr_arcto: {
            PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
            return nData->p;
        }
        case descr_cubicto: {
            PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
            return nData->p;
        }
        case descr_bezierto: {
            PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
            return nData->p;
        }
        case descr_interm_bezier:
        case descr_close:
        case descr_forced:
            return PrevPoint(i - 1);
        default:
            g_assert_not_reached();
            return Geom::Point(0, 0);
    }
}

// utilitaries: given a quadratic bezier curve (start point, control point, end point, ie that's a clamped curve),
// and an abcissis on it, get the point with that abcissis.
// warning: it's NOT a curvilign abcissis (or whatever you call that in english), so "t" is NOT the length of "start point"->"result point"
void Path::QuadraticPoint(double t, Geom::Point &oPt,
                          const Geom::Point &iS, const Geom::Point &iM, const Geom::Point &iE)
{
    Geom::Point const ax = iE - 2 * iM + iS;
    Geom::Point const bx = 2 * iM - 2 * iS;
    Geom::Point const cx = iS;

    oPt = t * t * ax + t * bx + cx;
}
// idem for cubic bezier patch
void Path::CubicTangent(double t, Geom::Point &oPt, const Geom::Point &iS, const Geom::Point &isD,
                        const Geom::Point &iE, const Geom::Point &ieD)
{
    Geom::Point const ax = ieD - 2 * iE + 2 * iS + isD;
    Geom::Point const bx = 3 * iE - ieD - 2 * isD - 3 * iS;
    Geom::Point const cx = isD;

    oPt = 3 * t * t * ax + 2 * t * bx + cx;
}

// extract interesting info of a SVG arc description
static void ArcAnglesAndCenter(Geom::Point const &iS, Geom::Point const &iE,
			       double rx, double ry, double angle,
			       bool large, bool wise,
			       double &sang, double &eang, Geom::Point &dr);

void Path::ArcAngles(const Geom::Point &iS, const Geom::Point &iE,
                     double rx, double ry, double angle, bool large, bool wise, double &sang, double &eang)
{
    Geom::Point dr;
    ArcAnglesAndCenter(iS, iE, rx, ry, angle, large, wise, sang, eang, dr);
}

/* N.B. If iS == iE then sang,eang,dr each become NaN.  Probably a bug. */
static void ArcAnglesAndCenter(Geom::Point const &iS, Geom::Point const &iE,
                               double rx, double ry, double angle,
                               bool large, bool wise,
                               double &sang, double &eang, Geom::Point &dr)
{
    Geom::Point se = iE - iS;
    Geom::Point ca(cos(angle), sin(angle));
    Geom::Point cse(dot(ca, se), cross(ca, se));
    cse[0] /= rx;
    cse[1] /= ry;
    double const lensq = dot(cse,cse);
    Geom::Point csd = ( ( lensq < 4
                        ? sqrt( 1/lensq - .25 )
                        : 0.0 )
                      * cse.ccw() );

    Geom::Point ra = -csd - 0.5 * cse;
    if ( ra[0] <= -1 ) {
        sang = M_PI;
    } else if ( ra[0] >= 1 ) {
        sang = 0;
    } else {
        sang = acos(ra[0]);
        if ( ra[1] < 0 ) {
            sang = 2 * M_PI - sang;
        }
    }

    ra = -csd + 0.5 * cse;
    if ( ra[0] <= -1 ) {
        eang = M_PI;
    } else if ( ra[0] >= 1 ) {
        eang = 0;
    } else {
        eang = acos(ra[0]);
        if ( ra[1] < 0 ) {
            eang = 2 * M_PI - eang;
        }
    }

    csd[0] *= rx;
    csd[1] *= ry;
    ca[1] = -ca[1]; // because it's the inverse rotation

    dr[0] = dot(ca, csd);
    dr[1] = cross(ca, csd);

    ca[1] = -ca[1];

    if ( wise ) {

        if (large) {
            dr = -dr;
            double swap = eang;
            eang = sang;
            sang = swap;
            eang += M_PI;
            sang += M_PI;
            if ( eang >= 2*M_PI ) {
                eang -= 2*M_PI;
            }
            if ( sang >= 2*M_PI ) {
                sang -= 2*M_PI;
            }
        }

    } else {
        if (!large) {
            dr = -dr;
            double swap = eang;
            eang = sang;
            sang = swap;
            eang += M_PI;
            sang += M_PI;
            if ( eang >= 2*M_PI ) {
                eang -= 2 * M_PI;
            }
            if ( sang >= 2*M_PI ) {
                sang -= 2 * M_PI;
            }
        }
    }

    dr += 0.5 * (iS + iE);
}



void Path::DoArc(Geom::Point const &iS, Geom::Point const &iE,
                 double const rx, double const ry, double const angle,
                 bool const large, bool const wise, double const /*tresh*/)
{
    /* TODO: Check that our behaviour is standards-conformant if iS and iE are (much) further
       apart than the diameter.  Also check that we do the right thing for negative radius.
       (Same for the other DoArc functions in this file.) */
    if ( rx <= 0.0001 || ry <= 0.0001 ) {
        return;
        // We always add a lineto afterwards, so this is fine.
        // [on ajoute toujours un lineto apres, donc c bon]
    }

    double sang;
    double eang;
    Geom::Point dr_temp;
    ArcAnglesAndCenter(iS, iE, rx, ry, angle*M_PI/180.0, large, wise, sang, eang, dr_temp);
    Geom::Point dr = dr_temp;
    /* TODO: This isn't as good numerically as treating iS and iE as primary.  E.g. consider
       the case of low curvature (i.e. very large radius). */

    Geom::Scale const ar(rx, ry);
    Geom::Rotate cb(sang);
    Geom::Rotate cbangle(angle*M_PI/180.0);
    if (wise) {

        double const incr = -0.1;
        if ( sang < eang ) {
            sang += 2*M_PI;
        }
        Geom::Rotate const omega(incr);
        for (double b = sang + incr ; b > eang ; b += incr) {
            cb = omega * cb;
            AddPoint( cb.vector() * ar * cbangle + dr );
        }

    } else {

        double const incr = 0.1;
        if ( sang > eang ) {
            sang -= 2*M_PI;
        }
        Geom::Rotate const omega(incr);
        for (double b = sang + incr ; b < eang ; b += incr) {
            cb = omega * cb;
            AddPoint( cb.vector() * ar * cbangle + dr);
        }
    }
}


void Path::RecCubicTo( Geom::Point const &iS, Geom::Point const &isD,
                       Geom::Point const &iE, Geom::Point const &ieD,
                       double tresh, int lev, double maxL)
{
    Geom::Point se = iE - iS;
    const double dC = Geom::L2(se);
    if ( dC < 0.01 ) {

        const double sC = dot(isD,isD);
        const double eC = dot(ieD,ieD);
        if ( sC < tresh && eC < tresh ) {
            return;
        }

    } else {
        const double sC = fabs(cross(se, isD)) / dC;
        const double eC = fabs(cross(se, ieD)) / dC;
        if ( sC < tresh && eC < tresh ) {
            // presque tt droit -> attention si on nous demande de bien subdiviser les petits segments
            if ( maxL > 0 && dC > maxL ) {
                if ( lev <= 0 ) {
                    return;
                }
                Geom::Point m = 0.5 * (iS + iE) + 0.125 * (isD - ieD);
                Geom::Point md = 0.75 * (iE - iS) - 0.125 * (isD + ieD);

                Geom::Point hisD = 0.5 * isD;
                Geom::Point hieD = 0.5 * ieD;

                RecCubicTo(iS, hisD, m, md, tresh, lev - 1, maxL);
                AddPoint(m);
                RecCubicTo(m, md, iE, hieD, tresh, lev - 1,maxL);
            }
            return;
        }
    }

    if ( lev <= 0 ) {
        return;
    }

    {
        Geom::Point m = 0.5 * (iS + iE) + 0.125 * (isD - ieD);
        Geom::Point md = 0.75 * (iE - iS) - 0.125 * (isD + ieD);

        Geom::Point hisD = 0.5 * isD;
        Geom::Point hieD = 0.5 * ieD;

        RecCubicTo(iS, hisD, m, md, tresh, lev - 1, maxL);
        AddPoint(m);
        RecCubicTo(m, md, iE, hieD, tresh, lev - 1,maxL);
    }
}



void Path::RecBezierTo(const Geom::Point &iP,
                       const Geom::Point &iS,
                       const Geom::Point &iE,
                       double tresh, int lev, double maxL)
{
    if ( lev <= 0 ) {
        return;
    }

    Geom::Point ps = iS - iP;
    Geom::Point pe = iE - iP;
    Geom::Point se = iE - iS;
    double s = fabs(cross(pe, ps));
    if ( s < tresh ) {
        const double l = L2(se);
        if ( maxL > 0 && l > maxL ) {
            const Geom::Point m = 0.25 * (iS + iE + 2 * iP);
            Geom::Point md = 0.5 * (iS + iP);
            RecBezierTo(md, iS, m, tresh, lev - 1, maxL);
            AddPoint(m);
            md = 0.5 * (iP + iE);
            RecBezierTo(md, m, iE, tresh, lev - 1, maxL);
        }
        return;
    }

    {
        const Geom::Point m = 0.25 * (iS + iE + 2 * iP);
        Geom::Point md = 0.5 * (iS + iP);
        RecBezierTo(md, iS, m, tresh, lev - 1, maxL);
        AddPoint(m);
        md = 0.5 * (iP + iE);
        RecBezierTo(md, m, iE, tresh, lev - 1, maxL);
    }
}


void Path::DoArc(Geom::Point const &iS, Geom::Point const &iE,
                 double const rx, double const ry, double const angle,
                 bool const large, bool const wise, double const /*tresh*/, int const piece)
{
    /* TODO: Check that our behaviour is standards-conformant if iS and iE are (much) further
       apart than the diameter.  Also check that we do the right thing for negative radius.
       (Same for the other DoArc functions in this file.) */
    if ( rx <= 0.0001 || ry <= 0.0001 ) {
        return;
        // We always add a lineto afterwards, so this is fine.
        // [on ajoute toujours un lineto apres, donc c bon]
    }

    double sang;
    double eang;
    Geom::Point dr_temp;
    ArcAnglesAndCenter(iS, iE, rx, ry, angle*M_PI/180.0, large, wise, sang, eang, dr_temp);
    Geom::Point dr = dr_temp;
    /* TODO: This isn't as good numerically as treating iS and iE as primary.  E.g. consider
       the case of low curvature (i.e. very large radius). */

    Geom::Scale const ar(rx, ry);
    Geom::Rotate cb(sang);
    Geom::Rotate cbangle(angle*M_PI/180.0);
    if (wise) {

        double const incr = -0.1;
        if ( sang < eang ) {
            sang += 2*M_PI;
        }
        Geom::Rotate const omega(incr);
        for (double b = sang + incr; b > eang; b += incr) {
            cb = omega * cb;
            AddPoint(cb.vector() * ar * cbangle + dr, piece, (sang - b) / (sang - eang));
        }

    } else {

        double const incr = 0.1;
        if ( sang > eang ) {
            sang -= 2 * M_PI;
        }
        Geom::Rotate const omega(incr);
        for (double b = sang + incr ; b < eang ; b += incr) {
            cb = omega * cb;
            AddPoint(cb.vector() * ar * cbangle + dr, piece, (b - sang) / (eang - sang));
        }
    }
}

void Path::RecCubicTo(Geom::Point const &iS, Geom::Point const &isD,
                      Geom::Point const &iE, Geom::Point const &ieD,
                      double tresh, int lev, double st, double et, int piece)
{
    const Geom::Point se = iE - iS;
    const double dC = Geom::L2(se);
    if ( dC < 0.01 ) {
        const double sC = dot(isD, isD);
        const double eC = dot(ieD, ieD);
        if ( sC < tresh && eC < tresh ) {
            return;
        }
    } else {
        const double sC = fabs(cross(se, isD)) / dC;
        const double eC = fabs(cross(se, ieD)) / dC;
        if ( sC < tresh && eC < tresh ) {
            return;
        }
    }

    if ( lev <= 0 ) {
        return;
    }

    Geom::Point m = 0.5 * (iS + iE) + 0.125 * (isD - ieD);
    Geom::Point md = 0.75 * (iE - iS) - 0.125 * (isD + ieD);
    double mt = (st + et) / 2;

    Geom::Point hisD = 0.5 * isD;
    Geom::Point hieD = 0.5 * ieD;

    RecCubicTo(iS, hisD, m, md, tresh, lev - 1, st, mt, piece);
    AddPoint(m, piece, mt);
    RecCubicTo(m, md, iE, hieD, tresh, lev - 1, mt, et, piece);

}



void Path::RecBezierTo(Geom::Point const &iP,
                       Geom::Point const &iS,
                       Geom::Point const &iE,
                       double tresh, int lev, double st, double et, int piece)
{
    if ( lev <= 0 ) {
        return;
    }

    Geom::Point ps = iS - iP;
    Geom::Point pe = iE - iP;
    const double s = fabs(cross(pe, ps));
    if ( s < tresh ) {
        return;
    }

    {
        const double mt = (st + et) / 2;
        const Geom::Point m = 0.25 * (iS + iE + 2 * iP);
        RecBezierTo(0.5 * (iS + iP), iS, m, tresh, lev - 1, st, mt, piece);
        AddPoint(m, piece, mt);
        RecBezierTo(0.5 * (iP + iE), m, iE, tresh, lev - 1, mt, et, piece);
    }
}



void Path::DoArc(Geom::Point const &iS, Geom::Point const &iE,
                 double const rx, double const ry, double const angle,
                 bool const large, bool const wise, double const /*tresh*/,
                 int const piece, offset_orig &/*orig*/)
{
    // Will never arrive here, as offsets are made of cubics.
    // [on n'arrivera jamais ici, puisque les offsets sont fait de cubiques]
    /* TODO: Check that our behaviour is standards-conformant if iS and iE are (much) further
       apart than the diameter.  Also check that we do the right thing for negative radius.
       (Same for the other DoArc functions in this file.) */
    if ( rx <= 0.0001 || ry <= 0.0001 ) {
        return;
        // We always add a lineto afterwards, so this is fine.
        // [on ajoute toujours un lineto apres, donc c bon]
    }

    double sang;
    double eang;
    Geom::Point dr_temp;
    ArcAnglesAndCenter(iS, iE, rx, ry, angle*M_PI/180.0, large, wise, sang, eang, dr_temp);
    Geom::Point dr = dr_temp;
    /* TODO: This isn't as good numerically as treating iS and iE as primary.  E.g. consider
       the case of low curvature (i.e. very large radius). */

    Geom::Scale const ar(rx, ry);
    Geom::Rotate cb(sang);
    Geom::Rotate cbangle(angle*M_PI/180.0);
    if (wise) {

        double const incr = -0.1;
        if ( sang < eang ) {
            sang += 2*M_PI;
        }
        Geom::Rotate const omega(incr);
        for (double b = sang + incr; b > eang ;b += incr) {
            cb = omega * cb;
            AddPoint(cb.vector() * ar * cbangle + dr, piece, (sang - b) / (sang - eang));
        }

    } else {
        double const incr = 0.1;
        if ( sang > eang ) {
            sang -= 2*M_PI;
        }
        Geom::Rotate const omega(incr);
        for (double b = sang + incr ; b < eang ; b += incr) {
            cb = omega * cb;
            AddPoint(cb.vector() * ar * cbangle + dr, piece, (b - sang) / (eang - sang));
        }
    }
}


void Path::RecCubicTo(Geom::Point const &iS, Geom::Point const &isD,
                      Geom::Point const &iE, Geom::Point const &ieD,
                      double tresh, int lev, double st, double et,
                      int piece, offset_orig &orig)
{
    const Geom::Point se = iE - iS;
    const double dC = Geom::L2(se);
    bool doneSub = false;
    if ( dC < 0.01 ) {
        const double sC = dot(isD, isD);
        const double eC = dot(ieD, ieD);
        if ( sC < tresh && eC < tresh ) {
            return;
        }
    } else {
        const double sC = fabs(cross(se, isD)) / dC;
        const double eC = fabs(cross(se, ieD)) / dC;
        if ( sC < tresh && eC < tresh ) {
            doneSub = true;
        }
    }

    if ( lev <= 0 ) {
        doneSub = true;
    }

    // test des inversions
    bool stInv = false;
    bool enInv = false;
    {
        Geom::Point os_pos;
        Geom::Point os_tgt;
        Geom::Point oe_pos;
        Geom::Point oe_tgt;

        orig.orig->PointAndTangentAt(orig.piece, orig.tSt * (1 - st) + orig.tEn * st, os_pos, os_tgt);
        orig.orig->PointAndTangentAt(orig.piece, orig.tSt * (1 - et) + orig.tEn * et, oe_pos, oe_tgt);


        Geom::Point n_tgt = isD;
        double si = dot(n_tgt, os_tgt);
        if ( si < 0 ) {
            stInv = true;
        }
        n_tgt = ieD;
        si = dot(n_tgt, oe_tgt);
        if ( si < 0 ) {
            enInv = true;
        }
        if ( stInv && enInv ) {

            AddPoint(os_pos, -1, 0.0);
            AddPoint(iE, piece, et);
            AddPoint(iS, piece, st);
            AddPoint(oe_pos, -1, 0.0);
            return;

        } else if ( ( stInv && !enInv ) || ( !stInv && enInv ) ) {
            return;
        }

    }

    if ( ( !stInv && !enInv && doneSub ) || lev <= 0 ) {
        return;
    }

    {
        const Geom::Point m = 0.5 * (iS+iE) + 0.125 * (isD - ieD);
        const Geom::Point md = 0.75 * (iE - iS) - 0.125 * (isD + ieD);
        const double mt = (st + et) / 2;
        const Geom::Point hisD = 0.5 * isD;
        const Geom::Point hieD = 0.5 * ieD;

        RecCubicTo(iS, hisD, m, md, tresh, lev - 1, st, mt, piece, orig);
        AddPoint(m, piece, mt);
        RecCubicTo(m, md, iE, hieD, tresh, lev - 1, mt, et, piece, orig);
    }
}



void Path::RecBezierTo(Geom::Point const &iP, Geom::Point const &iS,Geom::Point const &iE,
                       double tresh, int lev, double st, double et,
                       int piece, offset_orig& orig)
{
    bool doneSub = false;
    if ( lev <= 0 ) {
        return;
    }

    const Geom::Point ps = iS - iP;
    const Geom::Point pe = iE - iP;
    const double s = fabs(cross(pe, ps));
    if ( s < tresh ) {
        doneSub = true ;
    }

    // test des inversions
    bool stInv = false;
    bool enInv = false;
    {
        Geom::Point os_pos;
        Geom::Point os_tgt;
        Geom::Point oe_pos;
        Geom::Point oe_tgt;
        Geom::Point n_tgt;
        Geom::Point n_pos;

        double n_len;
        double n_rad;
        PathDescrIntermBezierTo mid(iP);
        PathDescrBezierTo fin(iE, 1);

        TangentOnBezAt(0.0, iS, mid, fin, false, n_pos, n_tgt, n_len, n_rad);
        orig.orig->PointAndTangentAt(orig.piece, orig.tSt * (1 - st) + orig.tEn * st, os_pos, os_tgt);
        double si = dot(n_tgt, os_tgt);
        if ( si < 0 ) {
            stInv = true;
        }

        TangentOnBezAt(1.0, iS, mid, fin, false, n_pos, n_tgt, n_len, n_rad);
        orig.orig->PointAndTangentAt(orig.piece, orig.tSt * (1 - et) + orig.tEn * et, oe_pos, oe_tgt);
        si = dot(n_tgt, oe_tgt);
        if ( si < 0 ) {
            enInv = true;
        }

        if ( stInv && enInv ) {
            AddPoint(os_pos, -1, 0.0);
            AddPoint(iE, piece, et);
            AddPoint(iS, piece, st);
            AddPoint(oe_pos, -1, 0.0);
            return;
        }
    }

    if ( !stInv && !enInv && doneSub ) {
        return;
    }

    {
        double mt = (st + et) / 2;
        Geom::Point m = 0.25 * (iS + iE + 2 * iP);
        Geom::Point md = 0.5 * (iS + iP);
        RecBezierTo(md, iS, m, tresh, lev - 1, st, mt, piece, orig);
        AddPoint(m, piece, mt);
        md = 0.5 * (iP + iE);
        RecBezierTo(md, m, iE, tresh, lev - 1, mt, et, piece, orig);
    }
}


/*
 * put a polyline in a Shape instance, for further fun
 * pathID is the ID you want this Path instance to be associated with, for when you're going to recompose the polyline
 * in a path description ( you need to have prepared the back data for that, of course)
 */

void Path::Fill(Shape* dest, int pathID, bool justAdd, bool closeIfNeeded, bool invert)
{
    if ( dest == NULL ) {
        return;
    }

    if ( justAdd == false ) {
        dest->Reset(pts.size(), pts.size());
    }

    if ( pts.size() <= 1 ) {
        return;
    }

    int first = dest->numberOfPoints();

    if ( back ) {
        dest->MakeBackData(true);
    }

    if ( invert ) {
        if ( back ) {
            {
                // invert && back && !weighted
                for (int i = 0; i < int(pts.size()); i++) {
                    dest->AddPoint(pts[i].p);
                }
                int lastM = 0;
                int curP = 1;
                int pathEnd = 0;
                bool closed = false;
                int lEdge = -1;

                while ( curP < int(pts.size()) ) {
                    int sbp = curP;
                    int lm = lastM;
                    int prp = pathEnd;

                    if ( pts[sbp].isMoveTo == polyline_moveto ) {

                        if ( closeIfNeeded ) {
                            if ( closed && lEdge >= 0 ) {
                                dest->DisconnectStart(lEdge);
                                dest->ConnectStart(first + lastM, lEdge);
                            } else {
                                lEdge = dest->AddEdge(first + lastM, first+pathEnd);
                                if ( lEdge >= 0 ) {
                                    dest->ebData[lEdge].pathID = pathID;
                                    dest->ebData[lEdge].pieceID = pts[lm].piece;
                                    dest->ebData[lEdge].tSt = 1.0;
                                    dest->ebData[lEdge].tEn = 0.0;
                                }
                            }
                        }

                        lastM = curP;
                        pathEnd = curP;
                        closed = false;
                        lEdge = -1;

                    } else {

                        if ( Geom::LInfty(pts[sbp].p - pts[prp].p) >= 0.00001 ) {
                            lEdge = dest->AddEdge(first + curP, first + pathEnd);
                            if ( lEdge >= 0 ) {
                                dest->ebData[lEdge].pathID = pathID;
                                dest->ebData[lEdge].pieceID = pts[sbp].piece;
                                if ( pts[sbp].piece == pts[prp].piece ) {
                                    dest->ebData[lEdge].tSt = pts[sbp].t;
                                    dest->ebData[lEdge].tEn = pts[prp].t;
                                } else {
                                    dest->ebData[lEdge].tSt = pts[sbp].t;
                                    dest->ebData[lEdge].tEn = 0.0;
                                }
                            }
                            pathEnd = curP;
                            if ( Geom::LInfty(pts[sbp].p - pts[lm].p) < 0.00001 ) {
                                closed = true;
                            } else {
                                closed = false;
                            }
                        }
                    }

                    curP++;
                }

                if ( closeIfNeeded ) {
                    if ( closed && lEdge >= 0 ) {
                        dest->DisconnectStart(lEdge);
                        dest->ConnectStart(first + lastM, lEdge);
                    } else {
                        int lm = lastM;
                        lEdge = dest->AddEdge(first + lastM, first + pathEnd);
                        if ( lEdge >= 0 ) {
                            dest->ebData[lEdge].pathID = pathID;
                            dest->ebData[lEdge].pieceID = pts[lm].piece;
                            dest->ebData[lEdge].tSt = 1.0;
                            dest->ebData[lEdge].tEn = 0.0;
                        }
                    }
                }
            }

        } else {

            {
                // invert && !back && !weighted
                for (int i = 0; i < int(pts.size()); i++) {
                    dest->AddPoint(pts[i].p);
                }
                int lastM = 0;
                int curP = 1;
                int pathEnd = 0;
                bool closed = false;
                int lEdge = -1;
                while ( curP < int(pts.size()) ) {
                    int sbp = curP;
                    int lm = lastM;
                    int prp = pathEnd;
                    if ( pts[sbp].isMoveTo == polyline_moveto ) {
                        if ( closeIfNeeded ) {
                            if ( closed && lEdge >= 0 ) {
                                dest->DisconnectStart(lEdge);
                                dest->ConnectStart(first + lastM, lEdge);
                            } else {
                                dest->AddEdge(first + lastM, first + pathEnd);
                            }
                        }
                        lastM = curP;
                        pathEnd = curP;
                        closed = false;
                        lEdge = -1;
                    } else {
                        if ( Geom::LInfty(pts[sbp].p - pts[prp].p) >= 0.00001 ) {
                            lEdge = dest->AddEdge(first+curP, first+pathEnd);
                            pathEnd = curP;
                            if ( Geom::LInfty(pts[sbp].p - pts[lm].p) < 0.00001 ) {
                                closed = true;
                            } else {
                                closed = false;
                            }
                        }
                    }
                    curP++;
                }

                if ( closeIfNeeded ) {
                    if ( closed && lEdge >= 0 ) {
                        dest->DisconnectStart(lEdge);
                        dest->ConnectStart(first + lastM, lEdge);
                    } else {
                        dest->AddEdge(first + lastM, first + pathEnd);
                    }
                }

            }
        }

    } else {

        if ( back ) {
            {
                // !invert && back && !weighted
                for (int i = 0; i < int(pts.size()); i++) {
                    dest->AddPoint(pts[i].p);
                }

                int lastM = 0;
                int curP = 1;
                int pathEnd = 0;
                bool closed = false;
                int lEdge = -1;
                while ( curP < int(pts.size()) ) {
                    int sbp = curP;
                    int lm = lastM;
                    int prp = pathEnd;
                    if ( pts[sbp].isMoveTo == polyline_moveto ) {
                        if ( closeIfNeeded ) {
                            if ( closed && lEdge >= 0 ) {
                                dest->DisconnectEnd(lEdge);
                                dest->ConnectEnd(first + lastM, lEdge);
                            } else {
                                lEdge = dest->AddEdge(first + pathEnd, first+lastM);
                                if ( lEdge >= 0 ) {
                                    dest->ebData[lEdge].pathID = pathID;
                                    dest->ebData[lEdge].pieceID = pts[lm].piece;
                                    dest->ebData[lEdge].tSt = 0.0;
                                    dest->ebData[lEdge].tEn = 1.0;
                                }
                            }
                        }
                        lastM = curP;
                        pathEnd = curP;
                        closed = false;
                        lEdge = -1;
                    } else {
                        if ( Geom::LInfty(pts[sbp].p - pts[prp].p) >= 0.00001 ) {
                            lEdge = dest->AddEdge(first + pathEnd, first + curP);
                            dest->ebData[lEdge].pathID = pathID;
                            dest->ebData[lEdge].pieceID = pts[sbp].piece;
                            if ( pts[sbp].piece == pts[prp].piece ) {
                                dest->ebData[lEdge].tSt = pts[prp].t;
                                dest->ebData[lEdge].tEn = pts[sbp].t;
                            } else {
                                dest->ebData[lEdge].tSt = 0.0;
                                dest->ebData[lEdge].tEn = pts[sbp].t;
                            }
                            pathEnd = curP;
                            if ( Geom::LInfty(pts[sbp].p - pts[lm].p) < 0.00001 ) {
                                closed = true;
                            } else {
                                closed = false;
                            }
                        }
                    }
                    curP++;
                }

                if ( closeIfNeeded ) {
                    if ( closed && lEdge >= 0 ) {
                        dest->DisconnectEnd(lEdge);
                        dest->ConnectEnd(first + lastM, lEdge);
                    } else {
                        int lm = lastM;
                        lEdge = dest->AddEdge(first + pathEnd, first + lastM);
                        if ( lEdge >= 0 ) {
                            dest->ebData[lEdge].pathID = pathID;
                            dest->ebData[lEdge].pieceID = pts[lm].piece;
                            dest->ebData[lEdge].tSt = 0.0;
                            dest->ebData[lEdge].tEn = 1.0;
                        }
                    }
                }
            }

        } else {
            {
                // !invert && !back && !weighted
                for (int i = 0;i < int(pts.size()); i++) {
                    dest->AddPoint(pts[i].p);
                }

                int lastM = 0;
                int curP = 1;
                int pathEnd = 0;
                bool closed = false;
                int lEdge = -1;
                while ( curP < int(pts.size()) ) {
                    int sbp = curP;
                    int lm = lastM;
                    int prp = pathEnd;
                    if ( pts[sbp].isMoveTo == polyline_moveto ) {
                        if ( closeIfNeeded ) {
                            if ( closed && lEdge >= 0 ) {
                                dest->DisconnectEnd(lEdge);
                                dest->ConnectEnd(first + lastM, lEdge);
                            } else {
                                dest->AddEdge(first + pathEnd, first + lastM);
                            }
                        }
                        lastM = curP;
                        pathEnd = curP;
                        closed = false;
                        lEdge = -1;
                    } else {
                        if ( Geom::LInfty(pts[sbp].p - pts[prp].p) >= 0.00001 ) {
                            lEdge = dest->AddEdge(first+pathEnd, first+curP);
                            pathEnd = curP;
                            if ( Geom::LInfty(pts[sbp].p - pts[lm].p) < 0.00001 ) {
                                closed = true;
                            } else {
                                closed = false;
                            }
                        }
                    }
                    curP++;
                }

                if ( closeIfNeeded ) {
                    if ( closed && lEdge >= 0 ) {
                        dest->DisconnectEnd(lEdge);
                        dest->ConnectEnd(first + lastM, lEdge);
                    } else {
                        dest->AddEdge(first + pathEnd, first + lastM);
                    }
                }

            }
        }
    }
}

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

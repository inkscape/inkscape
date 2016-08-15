/*
 *  PathOutline.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Nov 28 2003.
 *
 */

#include "livarot/Path.h"
#include "livarot/path-description.h"

/*
 * the "outliner"
 * takes a sequence of path commands and produces a set of commands that approximates the offset
 * result is stored in dest (that paremeter is handed to all the subfunctions)
 * not that the result is in general not mathematically correct; you can end up with unwanted holes in your
 * beautiful offset. a better way is to do path->polyline->polygon->offset of polygon->polyline(=contours of the polygon)->path
 * but computing offsets of the path is faster...
 */

// outline of a path.
// computed by making 2 offsets, one of the "left" side of the path, one of the right side, and then glueing the two
// the left side has to be reversed to make a contour
void Path::Outline(Path *dest, double width, JoinType join, ButtType butt, double miter)
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }
    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    }
    if ( descr_cmd.size() <= 1 ) {
        return;
    }
    if ( dest == NULL ) {
        return;
    }

    dest->Reset();
    dest->SetBackData(false);

    outline_callbacks calls;
    Geom::Point endButt;
    Geom::Point endPos;
    calls.cubicto = StdCubicTo;
    calls.bezierto = StdBezierTo;
    calls.arcto = StdArcTo;

    Path *rev = new Path;

    // we repeat the offset contour creation for each subpath
    int curP = 0;
    do {
        int lastM = curP;
        do {
            curP++;
            if (curP >= int(descr_cmd.size())) {
                break;
            }
            int typ = descr_cmd[curP]->getType();
            if (typ == descr_moveto) {
                break;
            }
        } while (curP < int(descr_cmd.size()));

        if (curP >= int(descr_cmd.size())) {
            curP = descr_cmd.size();
        }

        if (curP > lastM + 1) {
            // we have isolated a subpath, now we make a reversed version of it
            // we do so by taking the subpath in the reverse and constructing a path as appropriate
            // the construct is stored in "rev"
            int curD = curP - 1;
            Geom::Point curX;
            Geom::Point nextX;
            int firstTyp = descr_cmd[curD]->getType();
            bool const needClose = (firstTyp == descr_close);
            while (curD > lastM && descr_cmd[curD]->getType() == descr_close) {
                curD--;
            }

            int realP = curD + 1;
            if (curD > lastM) {
                curX = PrevPoint(curD);
                rev->Reset ();
                rev->MoveTo(curX);
                while (curD > lastM) {
                    int const typ = descr_cmd[curD]->getType();
                    if (typ == descr_moveto) {
                        //                                              rev->Close();
                        curD--;
                    } else if (typ == descr_forced) {
                        //                                              rev->Close();
                        curD--;
                    } else if (typ == descr_lineto) {
                        nextX = PrevPoint (curD - 1);
                        rev->LineTo (nextX);
                        curX = nextX;
                        curD--;
                    } else if (typ == descr_cubicto) {
                        PathDescrCubicTo* nData = dynamic_cast<PathDescrCubicTo*>(descr_cmd[curD]);
                        nextX = PrevPoint (curD - 1);
                        Geom::Point  isD=-nData->start;
                        Geom::Point  ieD=-nData->end;
                        rev->CubicTo (nextX, ieD,isD);
                        curX = nextX;
                        curD--;
                    } else if (typ == descr_arcto) {
                        PathDescrArcTo* nData = dynamic_cast<PathDescrArcTo*>(descr_cmd[curD]);
                        nextX = PrevPoint (curD - 1);
                        rev->ArcTo (nextX, nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
                        curX = nextX;
                        curD--;
                    } else if (typ == descr_bezierto) {
                        nextX = PrevPoint (curD - 1);
                        rev->LineTo (nextX);
                        curX = nextX;
                        curD--;
                    }  else if (typ == descr_interm_bezier) {
                        int nD = curD - 1;
                        while (nD > lastM && descr_cmd[nD]->getType() != descr_bezierto) nD--;
                        if ((descr_cmd[nD]->getType()) !=  descr_bezierto)  {
                            // pas trouve le debut!?
                            // Not find the start?!
                            nextX = PrevPoint (nD);
                            rev->LineTo (nextX);
                            curX = nextX;
                        } else {
                            nextX = PrevPoint (nD - 1);
                            rev->BezierTo (nextX);
                            for (int i = curD; i > nD; i--) {
                                PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(descr_cmd[i]);
                                rev->IntermBezierTo (nData->p);
                            }
                            rev->EndBezierTo ();
                            curX = nextX;
                        }
                        curD = nD - 1;
                    } else {
                        curD--;
                    }
                }

                // offset the paths and glue everything
                // actual offseting is done in SubContractOutline()
                if (needClose) {
                    rev->Close ();
                    rev->SubContractOutline (0, rev->descr_cmd.size(),
                                             dest, calls, 0.0025 * width * width, width,
                                             join, butt, miter, true, false, endPos, endButt);
                    SubContractOutline (lastM, realP + 1 - lastM,
                                        dest, calls, 0.0025 * width * width,
                                        width, join, butt, miter, true, false, endPos, endButt);
                } else {
                    rev->SubContractOutline (0, rev->descr_cmd.size(),
                                             dest, calls,  0.0025 * width * width, width,
                                             join, butt, miter, false, false, endPos, endButt);
                    Geom::Point endNor=endButt.ccw();
                    if (butt == butt_round) {
                        dest->ArcTo (endPos+width*endNor,  1.0001 * width, 1.0001 * width, 0.0, true, true);
                    }  else if (butt == butt_square) {
                        dest->LineTo (endPos-width*endNor+width*endButt);
                        dest->LineTo (endPos+width*endNor+width*endButt);
                        dest->LineTo (endPos+width*endNor);
                    }  else if (butt == butt_pointy) {
                        dest->LineTo (endPos+width*endButt);
                        dest->LineTo (endPos+width*endNor);
                    } else {
                        dest->LineTo (endPos+width*endNor);
                    }
                    SubContractOutline (lastM, realP - lastM,
                                        dest, calls, 0.0025 * width * width,  width, join, butt,
                                        miter, false, true, endPos, endButt);

                    endNor=endButt.ccw();
                    if (butt == butt_round) {
                        dest->ArcTo (endPos+width*endNor, 1.0001 * width, 1.0001 * width, 0.0, true, true);
                    } else if (butt == butt_square) {
                        dest->LineTo (endPos-width*endNor+width*endButt);
                        dest->LineTo (endPos+width*endNor+width*endButt);
                        dest->LineTo (endPos+width*endNor);
                    } else if (butt == butt_pointy) {
                        dest->LineTo (endPos+width*endButt);
                        dest->LineTo (endPos+width*endNor);
                    } else {
                        dest->LineTo (endPos+width*endNor);
                    }
                    dest->Close ();
                }
            } // if (curD > lastM)
        } // if (curP > lastM + 1)

    } while (curP < int(descr_cmd.size()));

    delete rev;
}

// versions for outlining closed path: they only make one side of the offset contour
void
Path::OutsideOutline (Path * dest, double width, JoinType join, ButtType butt,
                      double miter)
{
	if (descr_flags & descr_adding_bezier) {
		CancelBezier();
	}
	if (descr_flags & descr_doing_subpath) {
		CloseSubpath();
	}
	if (int(descr_cmd.size()) <= 1) return;
	if (dest == NULL) return;
	dest->Reset ();
	dest->SetBackData (false);

	outline_callbacks calls;
	Geom::Point endButt, endPos;
	calls.cubicto = StdCubicTo;
	calls.bezierto = StdBezierTo;
	calls.arcto = StdArcTo;
	SubContractOutline (0, descr_cmd.size(),
                            dest, calls, 0.0025 * width * width, width, join, butt,
			    miter, true, false, endPos, endButt);
}

void
Path::InsideOutline (Path * dest, double width, JoinType join, ButtType butt,
                     double miter)
{
	if ( descr_flags & descr_adding_bezier ) {
		CancelBezier();
	}
	if ( descr_flags & descr_doing_subpath ) {
		CloseSubpath();
	}
	if (int(descr_cmd.size()) <= 1) return;
	if (dest == NULL) return;
	dest->Reset ();
	dest->SetBackData (false);

	outline_callbacks calls;
	Geom::Point endButt, endPos;
	calls.cubicto = StdCubicTo;
	calls.bezierto = StdBezierTo;
	calls.arcto = StdArcTo;

	Path *rev = new Path;

	int curP = 0;
	do {
		int lastM = curP;
		do {
			curP++;
			if (curP >= int(descr_cmd.size())) break;
			int typ = descr_cmd[curP]->getType();
			if (typ == descr_moveto) break;
		} while (curP < int(descr_cmd.size()));
		if (curP >= int(descr_cmd.size()))  curP = descr_cmd.size();
		if (curP > lastM + 1) {
			// Otherwise there's only one point.  (tr: or "only a point")
			// [sinon il n'y a qu'un point]
			int curD = curP - 1;
			Geom::Point curX;
			Geom::Point nextX;
			while (curD > lastM && (descr_cmd[curD]->getType()) == descr_close) curD--;
			if (curD > lastM) {
				curX = PrevPoint (curD);
				rev->Reset ();
				rev->MoveTo (curX);
				while (curD > lastM) {
					int typ = descr_cmd[curD]->getType();
					if (typ == descr_moveto) {
						rev->Close ();
						curD--;
					} else if (typ == descr_forced) {
						curD--;
					} else if (typ == descr_lineto) {
						nextX = PrevPoint (curD - 1);
						rev->LineTo (nextX);
						curX = nextX;
						curD--;
					}  else if (typ == descr_cubicto) {
                                            PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo*>(descr_cmd[curD]);
						nextX = PrevPoint (curD - 1);
						Geom::Point  isD=-nData->start;
						Geom::Point  ieD=-nData->end;
						rev->CubicTo (nextX, ieD,isD);
						curX = nextX;
						curD--;
					} else if (typ == descr_arcto) {
                                            PathDescrArcTo* nData = dynamic_cast<PathDescrArcTo*>(descr_cmd[curD]);
						nextX = PrevPoint (curD - 1);
						rev->ArcTo (nextX, nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
						curX = nextX;
						curD--;
					} else if (typ == descr_bezierto) {
						nextX = PrevPoint (curD - 1);
						rev->LineTo (nextX);
						curX = nextX;
						curD--;
					} else if (typ == descr_interm_bezier) {
						int nD = curD - 1;
						while (nD > lastM && (descr_cmd[nD]->getType()) != descr_bezierto) nD--;
						if (descr_cmd[nD]->getType() != descr_bezierto) {
							// pas trouve le debut!?
							nextX = PrevPoint (nD);
							rev->LineTo (nextX);
							curX = nextX;
						} else {
							nextX = PrevPoint (nD - 1);
							rev->BezierTo (nextX);
							for (int i = curD; i > nD; i--) {
                                                            PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(descr_cmd[i]);
								rev->IntermBezierTo (nData->p);
							}
							rev->EndBezierTo ();
							curX = nextX;
						}
						curD = nD - 1;
					} else {
						curD--;
					}
				}
				rev->Close ();
				rev->SubContractOutline (0, rev->descr_cmd.size(),
                                                         dest, calls, 0.0025 * width * width,
							 width, join, butt, miter, true, false,
							 endPos, endButt);
			}
		}
	}  while (curP < int(descr_cmd.size()));

	delete rev;
}


// the offset
// take each command and offset it.
// the bezier spline is split in a sequence of bezier curves, and these are transformed in cubic bezier (which is
// not hard since they are quadratic bezier)
// joins are put where needed
void Path::SubContractOutline(int off, int num_pd,
                              Path *dest, outline_callbacks & calls,
                              double tolerance, double width, JoinType join,
                              ButtType /*butt*/, double miter, bool closeIfNeeded,
                              bool skipMoveto, Geom::Point &lastP, Geom::Point &lastT)
{
    outline_callback_data callsData;

    callsData.orig = this;
    callsData.dest = dest;
    int curP = 1;

    // le moveto
    Geom::Point curX;
    {
        int firstTyp = descr_cmd[off]->getType();
        if ( firstTyp != descr_moveto ) {
            curX[0] = curX[1] = 0;
            curP = 0;
        } else {
            PathDescrMoveTo* nData = dynamic_cast<PathDescrMoveTo*>(descr_cmd[off]);
            curX = nData->p;
        }
    }
    Geom::Point curT(0, 0);

    bool doFirst = true;
    Geom::Point firstP(0, 0);
    Geom::Point firstT(0, 0);

	// et le reste, 1 par 1
	while (curP < num_pd)
	{
            int curD = off + curP;
		int nType = descr_cmd[curD]->getType();
		Geom::Point nextX;
		Geom::Point stPos, enPos, stTgt, enTgt, stNor, enNor;
		double stRad, enRad, stTle, enTle;
		if (nType == descr_forced)  {
			curP++;
		} else if (nType == descr_moveto) {
			PathDescrMoveTo* nData = dynamic_cast<PathDescrMoveTo*>(descr_cmd[curD]);
			nextX = nData->p;
			// et on avance
			if (doFirst) {
			} else {
				if (closeIfNeeded) {
					if ( Geom::LInfty (curX- firstP) < 0.0001 ) {
						OutlineJoin (dest, firstP, curT, firstT, width, join,
									 miter, nType);
						dest->Close ();
					}  else {
                                            PathDescrLineTo temp(firstP);

						TangentOnSegAt (0.0, curX, temp, stPos, stTgt,
										stTle);
						TangentOnSegAt (1.0, curX, temp, enPos, enTgt,
										enTle);
						stNor=stTgt.cw();
						enNor=enTgt.cw();

						// jointure
						{
							Geom::Point pos;
							pos = curX;
							OutlineJoin (dest, pos, curT, stNor, width, join,
										 miter, nType);
						}
						dest->LineTo (enPos+width*enNor);

						// jointure
						{
							Geom::Point pos;
							pos = firstP;
							OutlineJoin (dest, enPos, enNor, firstT, width, join,
										 miter, nType);
							dest->Close ();
						}
					}
				}
			}
			firstP = nextX;
			curP++;
		}
		else if (nType == descr_close)
		{
			if (doFirst == false)
			{
				if (Geom::LInfty (curX - firstP) < 0.0001)
				{
					OutlineJoin (dest, firstP, curT, firstT, width, join,
								 miter, nType);
					dest->Close ();
				}
				else
				{
                                    PathDescrLineTo temp(firstP);
					nextX = firstP;

					TangentOnSegAt (0.0, curX, temp, stPos, stTgt, stTle);
					TangentOnSegAt (1.0, curX, temp, enPos, enTgt, enTle);
					stNor=stTgt.cw();
					enNor=enTgt.cw();

					// jointure
					{
						OutlineJoin (dest, stPos, curT, stNor, width, join,
									 miter, nType);
					}

					dest->LineTo (enPos+width*enNor);

					// jointure
					{
						OutlineJoin (dest, enPos, enNor, firstT, width, join,
									 miter, nType);
						dest->Close ();
					}
				}
			}
			doFirst = true;
			curP++;
		}
		else if (nType == descr_lineto)
		{
			PathDescrLineTo* nData = dynamic_cast<PathDescrLineTo*>(descr_cmd[curD]);
			nextX = nData->p;
			// et on avance
			TangentOnSegAt (0.0, curX, *nData, stPos, stTgt, stTle);
			TangentOnSegAt (1.0, curX, *nData, enPos, enTgt, enTle);
			// test de nullité du segment
			if (IsNulCurve (descr_cmd, curD, curX))
			{
                if (descr_cmd.size() == 2) {  // single point, see LP Bug 1006666
                    stTgt = dest->descr_cmd.size() ? Geom::Point(1, 0) : Geom::Point(-1, 0); // reverse direction
                    enTgt = stTgt;
                } else {
                    curP++;
                    continue;
                }
			}
			stNor=stTgt.cw();
			enNor=enTgt.cw();

			lastP = enPos;
			lastT = enTgt;

			if (doFirst)
			{
				doFirst = false;
				firstP = stPos;
				firstT = stNor;
				if (skipMoveto)
				{
					skipMoveto = false;
				}
				else
					dest->MoveTo (curX+width*stNor);
			}
			else
			{
				// jointure
				Geom::Point pos;
				pos = curX;
				OutlineJoin (dest, pos, curT, stNor, width, join, miter, nType);
			}

			int n_d = dest->LineTo (nextX+width*enNor);
			if (n_d >= 0)
			{
				dest->descr_cmd[n_d]->associated = curP;
				dest->descr_cmd[n_d]->tSt = 0.0;
				dest->descr_cmd[n_d]->tEn = 1.0;
			}
			curP++;
		}
		else if (nType == descr_cubicto)
		{
			PathDescrCubicTo* nData = dynamic_cast<PathDescrCubicTo*>(descr_cmd[curD]);
			nextX = nData->p;
			// test de nullite du segment
			if (IsNulCurve (descr_cmd, curD, curX))
			{
				curP++;
				continue;
			}
			// et on avance
			TangentOnCubAt (0.0, curX, *nData, false, stPos, stTgt,
							stTle, stRad);
			TangentOnCubAt (1.0, curX, *nData, true, enPos, enTgt,
							enTle, enRad);
			stNor=stTgt.cw();
			enNor=enTgt.cw();

			lastP = enPos;
			lastT = enTgt;

			if (doFirst)
			{
				doFirst = false;
				firstP = stPos;
				firstT = stNor;
				if (skipMoveto)
				{
					skipMoveto = false;
				}
				else
					dest->MoveTo (curX+width*stNor);
			}
			else
			{
				// jointure
				Geom::Point pos;
				pos = curX;
				OutlineJoin (dest, pos, curT, stNor, width, join, miter, nType);
			}

			callsData.piece = curP;
			callsData.tSt = 0.0;
			callsData.tEn = 1.0;
			callsData.x1 = curX[0];
			callsData.y1 = curX[1];
			callsData.x2 = nextX[0];
			callsData.y2 = nextX[1];
			callsData.d.c.dx1 = nData->start[0];
			callsData.d.c.dy1 = nData->start[1];
			callsData.d.c.dx2 = nData->end[0];
			callsData.d.c.dy2 = nData->end[1];
			(calls.cubicto) (&callsData, tolerance, width);

			curP++;
		}
		else if (nType == descr_arcto)
		{
			PathDescrArcTo* nData = dynamic_cast<PathDescrArcTo*>(descr_cmd[curD]);
			nextX = nData->p;
			// test de nullité du segment
			if (IsNulCurve (descr_cmd, curD, curX))
			{
				curP++;
				continue;
			}
			// et on avance
			TangentOnArcAt (0.0, curX, *nData, stPos, stTgt, stTle,
							stRad);
			TangentOnArcAt (1.0, curX, *nData, enPos, enTgt, enTle,
							enRad);
			stNor=stTgt.cw();
			enNor=enTgt.cw();

			lastP = enPos;
			lastT = enTgt;	// tjs definie

			if (doFirst)
			{
				doFirst = false;
				firstP = stPos;
				firstT = stNor;
				if (skipMoveto)
				{
					skipMoveto = false;
				}
				else
					dest->MoveTo (curX+width*stNor);
			}
			else
			{
				// jointure
				Geom::Point pos;
				pos = curX;
				OutlineJoin (dest, pos, curT, stNor, width, join, miter, nType);
			}

			callsData.piece = curP;
			callsData.tSt = 0.0;
			callsData.tEn = 1.0;
			callsData.x1 = curX[0];
			callsData.y1 = curX[1];
			callsData.x2 = nextX[0];
			callsData.y2 = nextX[1];
			callsData.d.a.rx = nData->rx;
			callsData.d.a.ry = nData->ry;
			callsData.d.a.angle = nData->angle;
			callsData.d.a.clock = nData->clockwise;
			callsData.d.a.large = nData->large;
			(calls.arcto) (&callsData, tolerance, width);

			curP++;
		}
		else if (nType == descr_bezierto)
		{
			PathDescrBezierTo* nBData = dynamic_cast<PathDescrBezierTo*>(descr_cmd[curD]);
			int nbInterm = nBData->nb;
			nextX = nBData->p;

			if (IsNulCurve (descr_cmd, curD, curX)) {
				curP += nbInterm + 1;
				continue;
			}

			curP++;

			curD = off + curP;
                        int ip = curD;
			PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(descr_cmd[ip]);

			if (nbInterm <= 0) {
				// et on avance
                            PathDescrLineTo temp(nextX);
				TangentOnSegAt (0.0, curX, temp, stPos, stTgt, stTle);
				TangentOnSegAt (1.0, curX, temp, enPos, enTgt, enTle);
				stNor=stTgt.cw();
				enNor=enTgt.cw();

				lastP = enPos;
				lastT = enTgt;

				if (doFirst) {
					doFirst = false;
					firstP = stPos;
					firstT = stNor;
					if (skipMoveto) {
						skipMoveto = false;
					} else dest->MoveTo (curX+width*stNor);
				} else {
					// jointure
					Geom::Point pos;
					pos = curX;
					if (stTle > 0) OutlineJoin (dest, pos, curT, stNor, width, join, miter, nType);
				}
				int n_d = dest->LineTo (nextX+width*enNor);
				if (n_d >= 0) {
					dest->descr_cmd[n_d]->associated = curP - 1;
					dest->descr_cmd[n_d]->tSt = 0.0;
					dest->descr_cmd[n_d]->tEn = 1.0;
				}
			} else if (nbInterm == 1) {
				Geom::Point  midX;
				midX = nData->p;
				// et on avance
				TangentOnBezAt (0.0, curX, *nData, *nBData, false, stPos, stTgt, stTle, stRad);
				TangentOnBezAt (1.0, curX, *nData, *nBData, true, enPos, enTgt, enTle, enRad);
				stNor=stTgt.cw();
				enNor=enTgt.cw();

				lastP = enPos;
				lastT = enTgt;

				if (doFirst) {
					doFirst = false;
					firstP = stPos;
					firstT = stNor;
					if (skipMoveto) {
						skipMoveto = false;
					} else dest->MoveTo (curX+width*stNor);
				}  else {
					// jointure
					Geom::Point pos;
					pos = curX;
					OutlineJoin (dest, pos, curT, stNor, width, join, miter, nType);
				}

				callsData.piece = curP;
				callsData.tSt = 0.0;
				callsData.tEn = 1.0;
				callsData.x1 = curX[0];
				callsData.y1 = curX[1];
				callsData.x2 = nextX[0];
				callsData.y2 = nextX[1];
				callsData.d.b.mx = midX[0];
				callsData.d.b.my = midX[1];
				(calls.bezierto) (&callsData, tolerance, width);

			} else if (nbInterm > 1) {
				Geom::Point  bx=curX;
				Geom::Point cx=curX;
				Geom::Point dx=nData->p;

				TangentOnBezAt (0.0, curX, *nData, *nBData, false, stPos, stTgt, stTle, stRad);
				stNor=stTgt.cw();

				ip++;
				nData = dynamic_cast<PathDescrIntermBezierTo*>(descr_cmd[ip]);
				// et on avance
				if (stTle > 0) {
					if (doFirst) {
						doFirst = false;
						firstP = stPos;
						firstT = stNor;
						if (skipMoveto) {
							skipMoveto = false;
						} else  dest->MoveTo (curX+width*stNor);
					} else {
						// jointure
						Geom::Point pos=curX;
						OutlineJoin (dest, pos, stTgt, stNor, width, join,  miter, nType);
						//                                              dest->LineTo(curX+width*stNor.x,curY+width*stNor.y);
					}
				}

				cx = 2 * bx - dx;

				for (int k = 0; k < nbInterm - 1; k++) {
					bx = cx;
					cx = dx;

					dx = nData->p;
                                        ip++;
					nData = dynamic_cast<PathDescrIntermBezierTo*>(descr_cmd[ip]);
					Geom::Point stx = (bx + cx) / 2;
					//                                      double  stw=(bw+cw)/2;

					PathDescrBezierTo tempb((cx + dx) / 2, 1);
					PathDescrIntermBezierTo tempi(cx);
					TangentOnBezAt (1.0, stx, tempi, tempb, true, enPos, enTgt, enTle, enRad);
					enNor=enTgt.cw();

					lastP = enPos;
					lastT = enTgt;

					callsData.piece = curP + k;
					callsData.tSt = 0.0;
					callsData.tEn = 1.0;
					callsData.x1 = stx[0];
					callsData.y1 = stx[1];
					callsData.x2 = (cx[0] + dx[0]) / 2;
					callsData.y2 = (cx[1] + dx[1]) / 2;
					callsData.d.b.mx = cx[0];
					callsData.d.b.my = cx[1];
					(calls.bezierto) (&callsData, tolerance, width);
				}
				{
					bx = cx;
					cx = dx;

					dx = nextX;
					dx = 2 * dx - cx;

					Geom::Point stx = (bx + cx) / 2;
					//                                      double  stw=(bw+cw)/2;

					PathDescrBezierTo tempb((cx + dx) / 2, 1);
					PathDescrIntermBezierTo tempi(cx);
					TangentOnBezAt (1.0, stx, tempi, tempb, true, enPos,
									enTgt, enTle, enRad);
					enNor=enTgt.cw();

					lastP = enPos;
					lastT = enTgt;

					callsData.piece = curP + nbInterm - 1;
					callsData.tSt = 0.0;
					callsData.tEn = 1.0;
					callsData.x1 = stx[0];
					callsData.y1 = stx[1];
					callsData.x2 = (cx[0] + dx[0]) / 2;
					callsData.y2 = (cx[1] + dx[1]) / 2;
					callsData.d.b.mx = cx[0];
					callsData.d.b.my = cx[1];
					(calls.bezierto) (&callsData, tolerance, width);

				}
			}

			// et on avance
			curP += nbInterm;
		}
		curX = nextX;
		curT = enNor;		// sera tjs bien definie
	}
	if (closeIfNeeded)
	{
		if (doFirst == false)
		{
		}
	}

}

/*
 *
 * utilitaires pour l'outline
 *
 */

// like the name says: check whether the path command is actually more than a dumb point.
bool
Path::IsNulCurve (std::vector<PathDescr*> const &cmd, int curD, Geom::Point const &curX)
{
	switch(cmd[curD]->getType()) {
    case descr_lineto:
    {
		PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo*>(cmd[curD]);
		if (Geom::LInfty(nData->p - curX) < 0.00001) {
			return true;
		}
		return false;
    }
	case descr_cubicto:
    {
		PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo*>(cmd[curD]);
		Geom::Point A = nData->start + nData->end + 2*(curX - nData->p);
		Geom::Point B = 3*(nData->p - curX) - 2*nData->start - nData->end;
		Geom::Point C = nData->start;
		if (Geom::LInfty(A) < 0.0001
			&& Geom::LInfty(B) < 0.0001
			&& Geom::LInfty (C) < 0.0001) {
			return true;
		}
		return false;
    }
    case descr_arcto:
    {
		PathDescrArcTo* nData = dynamic_cast<PathDescrArcTo*>(cmd[curD]);
		if ( Geom::LInfty(nData->p - curX) < 0.00001) {
			if ((nData->large == false)
				|| (fabs (nData->rx) < 0.00001
					|| fabs (nData->ry) < 0.00001)) {
				return true;
			}
		}
		return false;
    }
    case descr_bezierto:
    {
		PathDescrBezierTo* nBData = dynamic_cast<PathDescrBezierTo*>(cmd[curD]);
		if (nBData->nb <= 0)
		{
			if (Geom::LInfty(nBData->p - curX) < 0.00001) {
				return true;
			}
			return false;
		}
		else if (nBData->nb == 1)
		{
			if (Geom::LInfty(nBData->p - curX) < 0.00001) {
				int ip = curD + 1;
				PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(cmd[ip]);
				if (Geom::LInfty(nData->p - curX) < 0.00001) {
					return true;
				}
			}
			return false;
		} else if (Geom::LInfty(nBData->p - curX) < 0.00001) {
			for (int i = 1; i <= nBData->nb; i++) {
				int ip = curD + i;
				PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(cmd[ip]);
				if (Geom::LInfty(nData->p - curX) > 0.00001) {
					return false;
				}
			}
			return true;
		}
    }
    default:
		return true;
	}
}

// tangents and cuvarture computing, for the different path command types.
// the need for tangent is obvious: it gives the normal, along which we offset points
// curvature is used to do strength correction on the length of the tangents to the offset (see
// cubic offset)

/**
 *    \param at Distance along a tangent (0 <= at <= 1).
 *    \param iS Start point.
 *    \param fin LineTo description containing end point.
 *    \param pos Filled in with the position of `at' on the segment.
 *    \param tgt Filled in with the normalised tangent vector.
 *    \param len Filled in with the length of the segment.
 */

void Path::TangentOnSegAt(double at, Geom::Point const &iS, PathDescrLineTo const &fin,
                          Geom::Point &pos, Geom::Point &tgt, double &len)
{
    Geom::Point const iE = fin.p;
    Geom::Point const seg = iE - iS;
    double const l = L2(seg);
    if (l <= 0.000001) {
        pos = iS;
        tgt = Geom::Point(0, 0);
        len = 0;
    } else {
        tgt = seg / l;
        pos = (1 - at) * iS + at * iE; // in other words, pos = iS + at * seg
        len = l;
    }
}

// barf
void Path::TangentOnArcAt(double at, const Geom::Point &iS, PathDescrArcTo const &fin,
                          Geom::Point &pos, Geom::Point &tgt, double &len, double &rad)
{
	Geom::Point const iE  = fin.p;
	double const rx = fin.rx;
	double const ry = fin.ry;
	double const angle = fin.angle*M_PI/180.0;
	bool const large = fin.large;
	bool const wise = fin.clockwise;

	pos = iS;
	tgt[0] = tgt[1] = 0;
	if (rx <= 0.0001 || ry <= 0.0001)
		return;

	double const sex = iE[0] - iS[0], sey = iE[1] - iS[1];
	double const ca = cos (angle), sa = sin (angle);
	double csex =  ca * sex + sa * sey;
        double csey = -sa * sex + ca * sey;
	csex /= rx;
	csey /= ry;
	double l = csex * csex + csey * csey;
	if (l >= 4)
		return;
	double const d = sqrt(std::max(1 - l / 4, 0.0));
	double csdx = csey;
        double csdy = -csex;
	l = sqrt(l);
	csdx /= l;
	csdy /= l;
	csdx *= d;
	csdy *= d;

	double sang;
        double eang;
	double rax = -csdx - csex / 2;
        double ray = -csdy - csey / 2;
	if (rax < -1)
	{
		sang = M_PI;
	}
	else if (rax > 1)
	{
		sang = 0;
	}
	else
	{
		sang = acos (rax);
		if (ray < 0)
			sang = 2 * M_PI - sang;
	}
	rax = -csdx + csex / 2;
	ray = -csdy + csey / 2;
	if (rax < -1)
	{
		eang = M_PI;
	}
	else if (rax > 1)
	{
		eang = 0;
	}
	else
	{
		eang = acos (rax);
		if (ray < 0)
			eang = 2 * M_PI - eang;
	}

	csdx *= rx;
	csdy *= ry;
	double drx = ca * csdx - sa * csdy;
        double dry = sa * csdx + ca * csdy;

	if (wise)
	{
		if (large == true)
		{
			drx = -drx;
			dry = -dry;
			double swap = eang;
			eang = sang;
			sang = swap;
			eang += M_PI;
			sang += M_PI;
			if (eang >= 2 * M_PI)
				eang -= 2 * M_PI;
			if (sang >= 2 * M_PI)
				sang -= 2 * M_PI;
		}
	}
	else
	{
		if (large == false)
		{
			drx = -drx;
			dry = -dry;
			double swap = eang;
			eang = sang;
			sang = swap;
			eang += M_PI;
			sang += M_PI;
			if (eang >= 2 * M_PI)
				eang -= 2 * M_PI;
			if (sang >= 2 * M_PI)
				sang -= 2 * M_PI;
		}
	}
	drx += (iS[0] + iE[0]) / 2;
	dry += (iS[1] + iE[1]) / 2;

	if (wise) {
		if (sang < eang)
			sang += 2 * M_PI;
		double b = sang * (1 - at) + eang * at;
		double cb = cos (b), sb = sin (b);
		pos[0] = drx + ca * rx * cb - sa * ry * sb;
		pos[1] = dry + sa * rx * cb + ca * ry * sb;
		tgt[0] = ca * rx * sb + sa * ry * cb;
		tgt[1] = sa * rx * sb - ca * ry * cb;
		Geom::Point dtgt;
		dtgt[0] = -ca * rx * cb + sa * ry * sb;
		dtgt[1] = -sa * rx * cb - ca * ry * sb;
		len = L2(tgt);
		rad = -len * dot(tgt, tgt) / (tgt[0] * dtgt[1] - tgt[1] * dtgt[0]);
		tgt /= len;
	}
	else
	{
		if (sang > eang)
			sang -= 2 * M_PI;
		double b = sang * (1 - at) + eang * at;
		double cb = cos (b), sb = sin (b);
		pos[0] = drx + ca * rx * cb - sa * ry * sb;
		pos[1] = dry + sa * rx * cb + ca * ry * sb;
		tgt[0] = ca * rx * sb + sa * ry * cb;
		tgt[1] = sa * rx * sb - ca * ry * cb;
		Geom::Point dtgt;
		dtgt[0] = -ca * rx * cb + sa * ry * sb;
		dtgt[1] = -sa * rx * cb - ca * ry * sb;
		len = L2(tgt);
		rad = len * dot(tgt, tgt) / (tgt[0] * dtgt[1] - tgt[1] * dtgt[0]);
		tgt /= len;
	}
}
void
Path::TangentOnCubAt (double at, Geom::Point const &iS, PathDescrCubicTo const &fin, bool before,
                      Geom::Point &pos, Geom::Point &tgt, double &len, double &rad)
{
	const Geom::Point E = fin.p;
	const Geom::Point Sd = fin.start;
	const Geom::Point Ed = fin.end;

	pos = iS;
	tgt = Geom::Point(0,0);
	len = rad = 0;

	const Geom::Point A = Sd + Ed - 2*E + 2*iS;
	const Geom::Point B = 0.5*(Ed - Sd);
	const Geom::Point C = 0.25*(6*E - 6*iS - Sd - Ed);
	const Geom::Point D = 0.125*(4*iS + 4*E - Ed + Sd);
	const double atb = at - 0.5;
	pos = (atb * atb * atb)*A + (atb * atb)*B + atb*C + D;
	const Geom::Point der = (3 * atb * atb)*A  + (2 * atb)*B + C;
	const Geom::Point dder = (6 * atb)*A + 2*B;
	const Geom::Point ddder = 6 * A;

	double l = Geom::L2 (der);
  // lots of nasty cases. inversion points are sadly too common...
	if (l <= 0.0001) {
		len = 0;
		l = L2(dder);
		if (l <= 0.0001) {
			l = L2(ddder);
			if (l <= 0.0001) {
				// pas de segment....
				return;
			}
			rad = 100000000;
			tgt = ddder / l;
			if (before) {
				tgt = -tgt;
			}
			return;
		}
		rad = -l * (dot(dder,dder)) / (cross(dder, ddder));
		tgt = dder / l;
		if (before) {
			tgt = -tgt;
		}
		return;
	}
	len = l;

	rad = -l * (dot(der,der)) / (cross(der, dder));

	tgt = der / l;
}

void
Path::TangentOnBezAt (double at, Geom::Point const &iS,
                      PathDescrIntermBezierTo & mid,
                      PathDescrBezierTo & fin, bool before, Geom::Point & pos,
                      Geom::Point & tgt, double &len, double &rad)
{
	pos = iS;
	tgt = Geom::Point(0,0);
	len = rad = 0;

	const Geom::Point A = fin.p + iS - 2*mid.p;
	const Geom::Point B = 2*mid.p - 2 * iS;
	const Geom::Point C = iS;

	pos = at * at * A + at * B + C;
	const Geom::Point der = 2 * at * A + B;
	const Geom::Point dder = 2 * A;
	double l = Geom::L2(der);

	if (l <= 0.0001) {
		l = Geom::L2(dder);
		if (l <= 0.0001) {
			// pas de segment....
			// Not a segment.
			return;
		}
		rad = 100000000; // Why this number?
		tgt = dder / l;
		if (before) {
			tgt = -tgt;
		}
		return;
	}
	len = l;
	rad = -l * (dot(der,der)) / (cross(der, dder));

	tgt = der / l;
}

void
Path::OutlineJoin (Path * dest, Geom::Point pos, Geom::Point stNor, Geom::Point enNor, double width,
                   JoinType join, double miter, int nType)
{
    /* 
        Arbitrarily decide if we're on the inside or outside of a half turn.
        A turn of 180 degrees (line path leaves the node in the same direction as it arrived)
        is symmetric and has no real inside and outside. However when outlining we shall handle
        one path as inside and the reverse path as outside. Handling both as inside joins (as
        was done previously) will cut off round joins. Handling both as outside joins could
        ideally work because both should fall together, but it seems that this causes many
        extra nodes (due to rounding errors). Solution: for the 'half turn'-case toggle 
        inside/outside each time the same node is processed 2 consecutive times.
    */
    static bool TurnInside = true;
    static Geom::Point PrevPos(0, 0);
    TurnInside ^= PrevPos == pos;
    PrevPos = pos;

	const double angSi = cross (stNor, enNor);
	const double angCo = dot (stNor, enNor);

    if ((fabs(angSi) < .0000001) && angCo > 0) { // The join is straight -> nothing to do.
    } else {
        if ((angSi > 0 && width >= 0) 
            || (angSi < 0 && width < 0)) { // This is an inside join -> join is independent of chosen JoinType.
            if ((dest->descr_cmd[dest->descr_cmd.size() - 1]->getType() == descr_lineto) && (nType == descr_lineto)) {
                Geom::Point const biss = unit_vector(Geom::rot90( stNor - enNor ));
                double c2 = Geom::dot (biss, enNor);
                if (fabs(c2) > M_SQRT1_2) {    // apply only to obtuse angles
                    double l = width / c2;
                    PathDescrLineTo* nLine = dynamic_cast<PathDescrLineTo*>(dest->descr_cmd[dest->descr_cmd.size() - 1]);
                    nLine->p = pos + l*biss;  // relocate to bisector
                } else {
                    dest->LineTo (pos + width*enNor);
                }
            } else {
//                dest->LineTo (pos);	// redundant
                dest->LineTo (pos + width*enNor);
            }
        } else if (angSi == 0 && TurnInside) { // Half turn (180 degrees) ... inside (see above).
            dest->LineTo (pos + width*enNor);
        } else { // This is an outside join -> chosen JoinType should be applied.
            if (join == join_round) {
                // Use the ends of the cubic: approximate the arc at the
                // point where .., and support better the rounding of
                // coordinates of the end points.

                // utiliser des bouts de cubique: approximation de l'arc (au point ou on en est...), et supporte mieux
                // l'arrondi des coordonnees des extremites
                /* double   angle=acos(angCo);
                   if ( angCo >= 0 ) {
                   Geom::Point   stTgt,enTgt;
                   RotCCWTo(stNor,stTgt);
                   RotCCWTo(enNor,enTgt);
                   dest->CubicTo(pos.x+width*enNor.x,pos.y+width*enNor.y,
                   angle*width*stTgt.x,angle*width*stTgt.y,
                   angle*width*enTgt.x,angle*width*enTgt.y);
                   } else {
                   Geom::Point   biNor;
                   Geom::Point   stTgt,enTgt,biTgt;
                   biNor.x=stNor.x+enNor.x;
                   biNor.y=stNor.y+enNor.y;
                   double  biL=sqrt(biNor.x*biNor.x+biNor.y*biNor.y);
                   biNor.x/=biL;
                   biNor.y/=biL;
                   RotCCWTo(stNor,stTgt);
                   RotCCWTo(enNor,enTgt);
                   RotCCWTo(biNor,biTgt);
                   dest->CubicTo(pos.x+width*biNor.x,pos.y+width*biNor.y,
                   angle*width*stTgt.x,angle*width*stTgt.y,
                   angle*width*biTgt.x,angle*width*biTgt.y);
                   dest->CubicTo(pos.x+width*enNor.x,pos.y+width*enNor.y,
                   angle*width*biTgt.x,angle*width*biTgt.y,
                   angle*width*enTgt.x,angle*width*enTgt.y);
                   }*/
                if (width > 0) {
                    dest->ArcTo (pos + width*enNor,
                                 1.0001 * width, 1.0001 * width, 0.0, false, true);
                } else {
                    dest->ArcTo (pos + width*enNor,
                                 -1.0001 * width, -1.0001 * width, 0.0, false,
                                 false);
                }
            } else if (join == join_pointy) {
                Geom::Point const biss = unit_vector(Geom::rot90( stNor - enNor ));
                double c2 = Geom::dot (biss, enNor);
                double l = width / c2;
                if ( fabs(l) > miter) {
                    dest->LineTo (pos + width*enNor);
                } else {
                    if (dest->descr_cmd[dest->descr_cmd.size() - 1]->getType() == descr_lineto) {
                        PathDescrLineTo* nLine = dynamic_cast<PathDescrLineTo*>(dest->descr_cmd[dest->descr_cmd.size() - 1]);
                        nLine->p = pos+l*biss; // relocate to bisector
                    } else {
                        dest->LineTo (pos+l*biss);
                    }
                    if (nType != descr_lineto)
                        dest->LineTo (pos+width*enNor);
                }
            } else { // Bevel join
                dest->LineTo (pos + width*enNor);
            }
        }
	}
}

// les callbacks

// see http://www.home.unix-ag.org/simon/sketch/pathstroke.py to understand what's happening here

void
Path::RecStdCubicTo (outline_callback_data * data, double tol, double width,
                     int lev)
{
	Geom::Point stPos, miPos, enPos;
	Geom::Point stTgt, enTgt, miTgt, stNor, enNor, miNor;
	double stRad, miRad, enRad;
	double stTle, miTle, enTle;
	// un cubic
	{
            PathDescrCubicTo temp(Geom::Point(data->x2, data->y2),
                                    Geom::Point(data->d.c.dx1, data->d.c.dy1),
                                    Geom::Point(data->d.c.dx2, data->d.c.dy2));

		Geom::Point initial_point(data->x1, data->y1);
		TangentOnCubAt (0.0, initial_point, temp, false, stPos, stTgt, stTle,
						stRad);
		TangentOnCubAt (0.5, initial_point, temp, false, miPos, miTgt, miTle,
						miRad);
		TangentOnCubAt (1.0, initial_point, temp, true, enPos, enTgt, enTle,
						enRad);
		stNor=stTgt.cw();
		miNor=miTgt.cw();
		enNor=enTgt.cw();
	}

	double stGue = 1, miGue = 1, enGue = 1;
  // correction of the lengths of the tangent to the offset
  // if you don't see why i wrote that, draw a little figure and everything will be clear
	if (fabs (stRad) > 0.01)
		stGue += width / stRad;
	if (fabs (miRad) > 0.01)
		miGue += width / miRad;
	if (fabs (enRad) > 0.01)
		enGue += width / enRad;
	stGue *= stTle;
	miGue *= miTle;
	enGue *= enTle;


	if (lev <= 0) {
		int n_d = data->dest->CubicTo (enPos + width*enNor,
									   stGue*stTgt,
									   enGue*enTgt);
		if (n_d >= 0) {
			data->dest->descr_cmd[n_d]->associated = data->piece;
			data->dest->descr_cmd[n_d]->tSt = data->tSt;
			data->dest->descr_cmd[n_d]->tEn = data->tEn;
		}
		return;
	}

	Geom::Point chk;
	const Geom::Point req = miPos + width * miNor;
	{
            PathDescrCubicTo temp(enPos + width * enNor,
                                    stGue * stTgt,
                                    enGue * enTgt);
		double chTle, chRad;
		Geom::Point chTgt;
		TangentOnCubAt (0.5, stPos+width*stNor,
						temp, false, chk, chTgt, chTle, chRad);
	}
	const Geom::Point diff = req - chk;
	const double err = dot(diff,diff);
	if (err <= tol ) {  // tolerance is given as a quadratic value, no need to use tol*tol here
//    printf("%f <= %f %i\n",err,tol,lev);
		int n_d = data->dest->CubicTo (enPos + width*enNor,
									   stGue*stTgt,
									   enGue*enTgt);
		if (n_d >= 0) {
			data->dest->descr_cmd[n_d]->associated = data->piece;
			data->dest->descr_cmd[n_d]->tSt = data->tSt;
			data->dest->descr_cmd[n_d]->tEn = data->tEn;
		}
	} else {
		outline_callback_data desc = *data;

		desc.tSt = data->tSt;
		desc.tEn = (data->tSt + data->tEn) / 2;
		desc.x1 = data->x1;
		desc.y1 = data->y1;
		desc.x2 = miPos[0];
		desc.y2 = miPos[1];
		desc.d.c.dx1 = 0.5 * stTle * stTgt[0];
		desc.d.c.dy1 = 0.5 * stTle * stTgt[1];
		desc.d.c.dx2 = 0.5 * miTle * miTgt[0];
		desc.d.c.dy2 = 0.5 * miTle * miTgt[1];
		RecStdCubicTo (&desc, tol, width, lev - 1);

		desc.tSt = (data->tSt + data->tEn) / 2;
		desc.tEn = data->tEn;
		desc.x1 = miPos[0];
		desc.y1 = miPos[1];
		desc.x2 = data->x2;
		desc.y2 = data->y2;
		desc.d.c.dx1 = 0.5 * miTle * miTgt[0];
		desc.d.c.dy1 = 0.5 * miTle * miTgt[1];
		desc.d.c.dx2 = 0.5 * enTle * enTgt[0];
		desc.d.c.dy2 = 0.5 * enTle * enTgt[1];
		RecStdCubicTo (&desc, tol, width, lev - 1);
	}
}

void
Path::StdCubicTo (Path::outline_callback_data * data, double tol, double width)
{
//	fflush (stdout);
	RecStdCubicTo (data, tol, width, 8);
}

void
Path::StdBezierTo (Path::outline_callback_data * data, double tol, double width)
{
    PathDescrBezierTo tempb(Geom::Point(data->x2, data->y2), 1);
    PathDescrIntermBezierTo tempi(Geom::Point(data->d.b.mx, data->d.b.my));
	Geom::Point stPos, enPos, stTgt, enTgt;
	double stRad, enRad, stTle, enTle;
	Geom::Point  tmp(data->x1,data->y1);
	TangentOnBezAt (0.0, tmp, tempi, tempb, false, stPos, stTgt,
					stTle, stRad);
	TangentOnBezAt (1.0, tmp, tempi, tempb, true, enPos, enTgt,
					enTle, enRad);
	data->d.c.dx1 = stTle * stTgt[0];
	data->d.c.dy1 = stTle * stTgt[1];
	data->d.c.dx2 = enTle * enTgt[0];
	data->d.c.dy2 = enTle * enTgt[1];
	RecStdCubicTo (data, tol, width, 8);
}

void
Path::RecStdArcTo (outline_callback_data * data, double tol, double width,
                   int lev)
{
	Geom::Point stPos, miPos, enPos;
	Geom::Point stTgt, enTgt, miTgt, stNor, enNor, miNor;
	double stRad, miRad, enRad;
	double stTle, miTle, enTle;
	// un cubic
	{
            PathDescrArcTo temp(Geom::Point(data->x2, data->y2),
                                  data->d.a.rx, data->d.a.ry,
                                  data->d.a.angle, data->d.a.large, data->d.a.clock);

		Geom::Point tmp(data->x1,data->y1);
		TangentOnArcAt (data->d.a.stA, tmp, temp, stPos, stTgt,
						stTle, stRad);
		TangentOnArcAt ((data->d.a.stA + data->d.a.enA) / 2, tmp,
						temp, miPos, miTgt, miTle, miRad);
		TangentOnArcAt (data->d.a.enA, tmp, temp, enPos, enTgt,
						enTle, enRad);
		stNor=stTgt.cw();
		miNor=miTgt.cw();
		enNor=enTgt.cw();
	}

	double stGue = 1, miGue = 1, enGue = 1;
	if (fabs (stRad) > 0.01)
		stGue += width / stRad;
	if (fabs (miRad) > 0.01)
		miGue += width / miRad;
	if (fabs (enRad) > 0.01)
		enGue += width / enRad;
	stGue *= stTle;
	miGue *= miTle;
	enGue *= enTle;
	double sang, eang;
	{
		Geom::Point  tms(data->x1,data->y1),tme(data->x2,data->y2);
		ArcAngles (tms,tme, data->d.a.rx,
				   data->d.a.ry, data->d.a.angle*M_PI/180.0, data->d.a.large, !data->d.a.clock,
				   sang, eang);
	}
	double scal = eang - sang;
	if (scal < 0)
		scal += 2 * M_PI;
	if (scal > 2 * M_PI)
		scal -= 2 * M_PI;
	scal *= data->d.a.enA - data->d.a.stA;

	if (lev <= 0)
	{
		int n_d = data->dest->CubicTo (enPos + width*enNor,
									   stGue*scal*stTgt,
									   enGue*scal*enTgt);
		if (n_d >= 0) {
			data->dest->descr_cmd[n_d]->associated = data->piece;
			data->dest->descr_cmd[n_d]->tSt = data->d.a.stA;
			data->dest->descr_cmd[n_d]->tEn = data->d.a.enA;
		}
		return;
	}

	Geom::Point chk;
	const Geom::Point req = miPos + width*miNor;
	{
            PathDescrCubicTo temp(enPos + width * enNor, stGue * scal * stTgt, enGue * scal * enTgt);
		double chTle, chRad;
		Geom::Point chTgt;
		TangentOnCubAt (0.5, stPos+width*stNor,
						temp, false, chk, chTgt, chTle, chRad);
	}
	const Geom::Point diff = req - chk;
	const double err = (dot(diff,diff));
	if (err <= tol)  // tolerance is given as a quadratic value, no need to use tol*tol here
	{
		int n_d = data->dest->CubicTo (enPos + width*enNor,
									   stGue*scal*stTgt,
									   enGue*scal*enTgt);
		if (n_d >= 0) {
			data->dest->descr_cmd[n_d]->associated = data->piece;
			data->dest->descr_cmd[n_d]->tSt = data->d.a.stA;
			data->dest->descr_cmd[n_d]->tEn = data->d.a.enA;
		}
	} else {
		outline_callback_data desc = *data;

		desc.d.a.stA = data->d.a.stA;
		desc.d.a.enA = (data->d.a.stA + data->d.a.enA) / 2;
		RecStdArcTo (&desc, tol, width, lev - 1);

		desc.d.a.stA = (data->d.a.stA + data->d.a.enA) / 2;
		desc.d.a.enA = data->d.a.enA;
		RecStdArcTo (&desc, tol, width, lev - 1);
	}
}

void
Path::StdArcTo (Path::outline_callback_data * data, double tol, double width)
{
	data->d.a.stA = 0.0;
	data->d.a.enA = 1.0;
	RecStdArcTo (data, tol, width, 8);
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

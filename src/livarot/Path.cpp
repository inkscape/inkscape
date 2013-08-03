/*
 *  Path.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#include <glib.h>
#include "Path.h"
#include "livarot/path-description.h"

/*
 * manipulation of the path data: path description and polyline
 * grunt work...
 * at the end of this file, 2 utilitary functions to get the point and tangent to path associated with a (command no;abcissis)
 */


Path::Path()
{
	descr_flags = 0;
	pending_bezier_cmd = -1;
	pending_moveto_cmd = -1;
  
	back = false;
}

Path::~Path()
{
    for (std::vector<PathDescr*>::iterator i = descr_cmd.begin(); i != descr_cmd.end(); ++i) {
        delete *i;
    }
}

// debug function do dump the path contents on stdout
void Path::Affiche()
{
    std::cout << "path: " << descr_cmd.size() << " commands." << std::endl;
    for (std::vector<PathDescr*>::const_iterator i = descr_cmd.begin(); i != descr_cmd.end(); ++i) {
        (*i)->dump(std::cout);
        std::cout << std::endl;
    }

    std::cout << std::endl;
}

void Path::Reset()
{
    for (std::vector<PathDescr*>::iterator i = descr_cmd.begin(); i != descr_cmd.end(); ++i) {
        delete *i;
    }
    
    descr_cmd.clear();
    pending_bezier_cmd = -1;
    pending_moveto_cmd = -1;
    descr_flags = 0;
}

void Path::Copy(Path * who)
{
    ResetPoints();
    
    for (std::vector<PathDescr*>::iterator i = descr_cmd.begin(); i != descr_cmd.end(); ++i) {
        delete *i;
    }
        
    descr_cmd.clear();
        
    for (std::vector<PathDescr*>::const_iterator i = who->descr_cmd.begin();
         i != who->descr_cmd.end();
         ++i)
    {
        descr_cmd.push_back((*i)->clone());
    }
}

void Path::CloseSubpath()
{
    descr_flags &= ~(descr_doing_subpath);
    pending_moveto_cmd = -1;
}

int Path::ForcePoint()
{
    if (descr_flags & descr_adding_bezier) {
        EndBezierTo ();
    }
    
    if ( (descr_flags & descr_doing_subpath) == 0 ) {
        return -1;
    }
    
    if (descr_cmd.empty()) {
        return -1;
    }

    descr_cmd.push_back(new PathDescrForced);
    return descr_cmd.size() - 1;
}


void Path::InsertForcePoint(int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
	return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
	ForcePoint();
	return;
    }
    
    descr_cmd.insert(descr_cmd.begin() + at, new PathDescrForced);
}

int Path::Close()
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }
    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    } else {
        // Nothing to close.
        return -1;
    }

    descr_cmd.push_back(new PathDescrClose);
    
    descr_flags &= ~(descr_doing_subpath);
    pending_moveto_cmd = -1;
    
    return descr_cmd.size() - 1;
}

int Path::MoveTo(Geom::Point const &iPt)
{
    if ( descr_flags & descr_adding_bezier ) {
	EndBezierTo(iPt);
    }
    if ( descr_flags & descr_doing_subpath ) {
	CloseSubpath();
    }
    pending_moveto_cmd = descr_cmd.size();
    
    descr_cmd.push_back(new PathDescrMoveTo(iPt));

    descr_flags |= descr_doing_subpath;
    return descr_cmd.size() - 1;
}

void Path::InsertMoveTo(Geom::Point const &iPt, int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
        return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
        MoveTo(iPt);
        return;
    }

  descr_cmd.insert(descr_cmd.begin() + at, new PathDescrMoveTo(iPt));
}

int Path::LineTo(Geom::Point const &iPt)
{
    if (descr_flags & descr_adding_bezier) {
	EndBezierTo (iPt);
    }
    if (!( descr_flags & descr_doing_subpath )) {
	return MoveTo (iPt);
    }
    
    descr_cmd.push_back(new PathDescrLineTo(iPt));
    return descr_cmd.size() - 1;
}

void Path::InsertLineTo(Geom::Point const &iPt, int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
        return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
        LineTo(iPt);
        return;
    }
    
    descr_cmd.insert(descr_cmd.begin() + at, new PathDescrLineTo(iPt));
}

int Path::CubicTo(Geom::Point const &iPt, Geom::Point const &iStD, Geom::Point const &iEnD)
{
    if (descr_flags & descr_adding_bezier) {
	EndBezierTo(iPt);
    }
    if ( (descr_flags & descr_doing_subpath) == 0) {
	return MoveTo (iPt);
    }

    descr_cmd.push_back(new PathDescrCubicTo(iPt, iStD, iEnD));
    return descr_cmd.size() - 1;
}


void Path::InsertCubicTo(Geom::Point const &iPt, Geom::Point const &iStD, Geom::Point const &iEnD, int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
	return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
	CubicTo(iPt,iStD,iEnD);
	return;
    }
  
    descr_cmd.insert(descr_cmd.begin() + at, new PathDescrCubicTo(iPt, iStD, iEnD));
}

int Path::ArcTo(Geom::Point const &iPt, double iRx, double iRy, double angle,
		bool iLargeArc, bool iClockwise)
{
    if (descr_flags & descr_adding_bezier) {
	EndBezierTo(iPt);
    }
    if ( (descr_flags & descr_doing_subpath) == 0 ) {
	return MoveTo(iPt);
    }

    descr_cmd.push_back(new PathDescrArcTo(iPt, iRx, iRy, angle, iLargeArc, iClockwise));
    return descr_cmd.size() - 1;
}


void Path::InsertArcTo(Geom::Point const &iPt, double iRx, double iRy, double angle,
		       bool iLargeArc, bool iClockwise, int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
	return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
	ArcTo(iPt, iRx, iRy, angle, iLargeArc, iClockwise);
	return;
    }
  
    descr_cmd.insert(descr_cmd.begin() + at, new PathDescrArcTo(iPt, iRx, iRy,
                                                                angle, iLargeArc, iClockwise));
}

int Path::TempBezierTo()
{
    if (descr_flags & descr_adding_bezier) {
	CancelBezier();
    }
    if ( (descr_flags & descr_doing_subpath) == 0) {
	// No starting point -> bad.
	return -1;
    }
    pending_bezier_cmd = descr_cmd.size();
    
    descr_cmd.push_back(new PathDescrBezierTo(Geom::Point(0, 0), 0));
    descr_flags |= descr_adding_bezier;
    descr_flags |= descr_delayed_bezier;
    return descr_cmd.size() - 1;
}

void Path::CancelBezier()
{
    descr_flags &= ~(descr_adding_bezier);
    descr_flags &= ~(descr_delayed_bezier);
    if (pending_bezier_cmd < 0) {
	return;
    }

    /* FIXME: I think there's a memory leak here */
    descr_cmd.resize(pending_bezier_cmd);
    pending_bezier_cmd = -1;
}

int Path::EndBezierTo()
{
    if (descr_flags & descr_delayed_bezier) {
	CancelBezier ();
    } else {
	pending_bezier_cmd = -1;
	descr_flags &= ~(descr_adding_bezier);
	descr_flags &= ~(descr_delayed_bezier);
    }
    return -1;
}

int Path::EndBezierTo(Geom::Point const &iPt)
{
    if ( (descr_flags & descr_adding_bezier) == 0 ) {
	return LineTo(iPt);
    }
    if ( (descr_flags & descr_doing_subpath) == 0 ) {
	return MoveTo(iPt);
    }
    if ( (descr_flags & descr_delayed_bezier) == 0 ) {
	return EndBezierTo();
    }
    PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[pending_bezier_cmd]);
    nData->p = iPt;
    pending_bezier_cmd = -1;
    descr_flags &= ~(descr_adding_bezier);
    descr_flags &= ~(descr_delayed_bezier);
    return -1;
}


int Path::IntermBezierTo(Geom::Point const &iPt)
{
    if ( (descr_flags & descr_adding_bezier) == 0 ) {
	return LineTo (iPt);
    }
    
    if ( (descr_flags & descr_doing_subpath) == 0) {
	return MoveTo (iPt);
    }

    descr_cmd.push_back(new PathDescrIntermBezierTo(iPt));

    PathDescrBezierTo *nBData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[pending_bezier_cmd]);
    nBData->nb++;
    return descr_cmd.size() - 1;
}


void Path::InsertIntermBezierTo(Geom::Point const &iPt, int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
        return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
        IntermBezierTo(iPt);
        return;
    }
    
    descr_cmd.insert(descr_cmd.begin() + at, new PathDescrIntermBezierTo(iPt));
}


int Path::BezierTo(Geom::Point const &iPt)
{
    if ( descr_flags & descr_adding_bezier ) {
	EndBezierTo(iPt);
    }
    
    if ( (descr_flags & descr_doing_subpath) == 0 ) {
	return MoveTo (iPt);
    }
    
    pending_bezier_cmd = descr_cmd.size();
    
    descr_cmd.push_back(new PathDescrBezierTo(iPt, 0));
    descr_flags |= descr_adding_bezier;
    descr_flags &= ~(descr_delayed_bezier);
    return descr_cmd.size() - 1;
}


void Path::InsertBezierTo(Geom::Point const &iPt, int iNb, int at)
{
    if ( at < 0 || at > int(descr_cmd.size()) ) {
	return;
    }
    
    if ( at == int(descr_cmd.size()) ) {
	BezierTo(iPt);
	return;
    }
  
    descr_cmd.insert(descr_cmd.begin() + at, new PathDescrBezierTo(iPt, iNb));
}


/*
 * points de la polyligne
 */
void
Path::SetBackData (bool nVal)
{
	if (back == false) {
		if (nVal == true && back == false) {
			back = true;
			ResetPoints();
		} else if (nVal == false && back == true) {
			back = false;
			ResetPoints();
		}
	} else {
		if (nVal == true && back == false) {
			back = true;
			ResetPoints();
		} else if (nVal == false && back == true) {
			back = false;
			ResetPoints();
		}
	}
}


void Path::ResetPoints()
{
    pts.clear();
}


int Path::AddPoint(Geom::Point const &iPt, bool mvto)
{
    if (back) {
        return AddPoint (iPt, -1, 0.0, mvto);
    }
  
    if ( !mvto && pts.empty() == false && pts.back().p == iPt ) {
        return -1;
    }
    
    int const n = pts.size();
    pts.push_back(path_lineto(mvto ? polyline_moveto : polyline_lineto, iPt));
    return n;
}


int Path::ReplacePoint(Geom::Point const &iPt)
{
    if (pts.empty()) {
        return -1;
    }
  
    int const n = pts.size() - 1;
    pts[n] = path_lineto(polyline_lineto, iPt);
    return n;
}


int Path::AddPoint(Geom::Point const &iPt, int ip, double it, bool mvto)
{
    if (back == false) {
        return AddPoint (iPt, mvto);
    }
    
    if ( !mvto && pts.empty() == false && pts.back().p == iPt ) {
        return -1;
    }
    
    int const n = pts.size();
    pts.push_back(path_lineto(mvto ? polyline_moveto : polyline_lineto, iPt, ip, it));
    return n;
}

int Path::AddForcedPoint(Geom::Point const &iPt)
{
    if (back) {
        return AddForcedPoint (iPt, -1, 0.0);
    }
    
    if ( pts.empty() || pts.back().isMoveTo != polyline_lineto ) {
        return -1;
    }
    
    int const n = pts.size();
    pts.push_back(path_lineto(polyline_forced, pts[n - 1].p));
    return n;
}


int Path::AddForcedPoint(Geom::Point const &iPt, int /*ip*/, double /*it*/)
{
    /* FIXME: ip & it aren't used.  Is this deliberate? */
    if (!back) {
        return AddForcedPoint (iPt);
    }
    
    if ( pts.empty() || pts.back().isMoveTo != polyline_lineto ) {
        return -1;
    }
    
    int const n = pts.size();
    pts.push_back(path_lineto(polyline_forced, pts[n - 1].p, pts[n - 1].piece, pts[n - 1].t));
    return n;
}

void Path::PolylineBoundingBox(double &l, double &t, double &r, double &b)
{
  l = t = r = b = 0.0;
  if ( pts.empty() ) {
      return;
  }

  std::vector<path_lineto>::const_iterator i = pts.begin();
  l = r = i->p[Geom::X];
  t = b = i->p[Geom::Y];
  ++i;

  for (; i != pts.end(); ++i) {
      r = std::max(r, i->p[Geom::X]);
      l = std::min(l, i->p[Geom::X]);
      b = std::max(b, i->p[Geom::Y]);
      t = std::min(t, i->p[Geom::Y]);
  }
}


/**
 *    \param piece Index of a one of our commands.
 *    \param at Distance along the segment that corresponds to `piece' (0 <= at <= 1)
 *    \param pos Filled in with the point at `at' on `piece'.
 */

void Path::PointAt(int piece, double at, Geom::Point &pos)
{
    if (piece < 0 || piece >= int(descr_cmd.size())) {
	// this shouldn't happen: the piece we are asked for doesn't
	// exist in the path
	pos = Geom::Point(0,0);
	return;
    }
    
    PathDescr const *theD = descr_cmd[piece];
    int const typ = theD->getType();
    Geom::Point tgt;
    double len;
    double rad;
    
    if (typ == descr_moveto) {
	
	return PointAt (piece + 1, 0.0, pos);
	
    } else if (typ == descr_close || typ == descr_forced) {
	
	return PointAt (piece - 1, 1.0, pos);
	
    } else if (typ == descr_lineto) {
	
	PathDescrLineTo const *nData = dynamic_cast<PathDescrLineTo const *>(theD);
	TangentOnSegAt(at, PrevPoint (piece - 1), *nData, pos, tgt, len);
	
    } else if (typ == descr_arcto) {
	
	PathDescrArcTo const *nData = dynamic_cast<PathDescrArcTo const *>(theD);
	TangentOnArcAt(at,PrevPoint (piece - 1), *nData, pos, tgt, len, rad);
	
    } else if (typ == descr_cubicto) {
	
	PathDescrCubicTo const *nData = dynamic_cast<PathDescrCubicTo const *>(theD);
	TangentOnCubAt(at, PrevPoint (piece - 1), *nData, false, pos, tgt, len, rad);
	
    } else if (typ == descr_bezierto || typ == descr_interm_bezier) {
	
	int bez_st = piece;
	while (bez_st >= 0) {
	    int nt = descr_cmd[bez_st]->getType();
	    if (nt == descr_bezierto)
		break;
	    bez_st--;
	}
	if ( bez_st < 0 ) {
	    // Didn't find the beginning of the spline (bad).
	    // [pas trouvé le dubut de la spline (mauvais)]
	    return PointAt(piece - 1, 1.0, pos);
	}
	
	PathDescrBezierTo *stB = dynamic_cast<PathDescrBezierTo *>(descr_cmd[bez_st]);
	if ( piece > bez_st + stB->nb ) {
	    // The spline goes past the authorized number of commands (bad).
	    // [la spline sort du nombre de commandes autorisé (mauvais)]
	    return PointAt(piece - 1, 1.0, pos);
	}
	
	int k = piece - bez_st;
	Geom::Point const bStPt = PrevPoint(bez_st - 1);
	if (stB->nb == 1 || k <= 0) {
	    PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + 1]);
	    TangentOnBezAt(at, bStPt, *nData, *stB, false, pos, tgt, len, rad);
	} else {
	    // forcement plus grand que 1
	    if (k == 1) {
		PathDescrIntermBezierTo *nextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + 1]);
		PathDescrIntermBezierTo *nnextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + 2]);
		PathDescrBezierTo fin(0.5 * (nextI->p + nnextI->p), 1);
		TangentOnBezAt(at, bStPt, *nextI,  fin, false, pos, tgt, len, rad);
	    } else if (k == stB->nb) {
		PathDescrIntermBezierTo *nextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k]);
		PathDescrIntermBezierTo *prevI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k - 1]);
		Geom::Point stP = 0.5 * ( prevI->p + nextI->p );
		TangentOnBezAt(at, stP, *nextI, *stB, false, pos, tgt, len, rad);
	    } else {
		PathDescrIntermBezierTo *nextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k]);
		PathDescrIntermBezierTo *prevI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k - 1]);
		PathDescrIntermBezierTo *nnextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k + 1]);
		Geom::Point stP = 0.5 * ( prevI->p + nextI->p );
		PathDescrBezierTo fin(0.5 * (nextI->p + nnextI->p), 1);
		TangentOnBezAt(at, stP, *nextI, fin, false, pos, tgt, len, rad);
	    }
	}
    }
}


void Path::PointAndTangentAt(int piece, double at, Geom::Point &pos, Geom::Point &tgt)
{
    if (piece < 0 || piece >= int(descr_cmd.size())) {
	// this shouldn't happen: the piece we are asked for doesn't exist in the path
	pos = Geom::Point(0, 0);
	return;
    }
    
    PathDescr const *theD = descr_cmd[piece];
    int typ = theD->getType();
    double len;
    double rad;
    if (typ == descr_moveto) {
	
	return PointAndTangentAt(piece + 1, 0.0, pos, tgt);
	
    } else if (typ == descr_close ) {
	
	int cp = piece - 1;
	while ( cp >= 0 && (descr_cmd[cp]->getType()) != descr_moveto ) {
	    cp--;
	}
	if ( cp >= 0 ) {
	    PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[cp]);
	    PathDescrLineTo dst(nData->p);
	    TangentOnSegAt(at, PrevPoint (piece - 1), dst, pos, tgt, len);
	}
	
    } else if ( typ == descr_forced) {
	
	return PointAndTangentAt(piece - 1, 1.0, pos,tgt);
	
    } else if (typ == descr_lineto) {

	PathDescrLineTo const *nData = dynamic_cast<PathDescrLineTo const *>(theD);
	TangentOnSegAt(at, PrevPoint (piece - 1), *nData, pos, tgt, len);
	
    } else if (typ == descr_arcto) {
	
	PathDescrArcTo const *nData = dynamic_cast<PathDescrArcTo const *>(theD);
	TangentOnArcAt (at,PrevPoint (piece - 1), *nData, pos, tgt, len, rad);
	
    } else if (typ == descr_cubicto) {
	
	PathDescrCubicTo const *nData = dynamic_cast<PathDescrCubicTo const *>(theD);
	TangentOnCubAt (at, PrevPoint (piece - 1), *nData, false, pos, tgt, len, rad);
	
    } else if (typ == descr_bezierto || typ == descr_interm_bezier) {
	int bez_st = piece;
	while (bez_st >= 0) {
	    int nt = descr_cmd[bez_st]->getType();
	    if (nt == descr_bezierto) break;
	    bez_st--;
	}
	if ( bez_st < 0 ) {
	    return PointAndTangentAt(piece - 1, 1.0, pos, tgt);
	    // Didn't find the beginning of the spline (bad).
	    // [pas trouvé le dubut de la spline (mauvais)]
	}
	
	PathDescrBezierTo* stB = dynamic_cast<PathDescrBezierTo*>(descr_cmd[bez_st]);
	if ( piece > bez_st + stB->nb ) {
	    return PointAndTangentAt(piece - 1, 1.0, pos, tgt);
	    // The spline goes past the number of authorized commands (bad).
	    // [la spline sort du nombre de commandes autorisé (mauvais)]
	}
	
	int k = piece - bez_st;
	Geom::Point const bStPt(PrevPoint( bez_st - 1 ));
	if (stB->nb == 1 || k <= 0) {
	    PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + 1]);
	    TangentOnBezAt (at, bStPt, *nData, *stB, false, pos, tgt, len, rad);
	} else {
	    // forcement plus grand que 1
	    if (k == 1) {
		PathDescrIntermBezierTo *nextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + 1]);
		PathDescrIntermBezierTo *nnextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + 2]);
		PathDescrBezierTo fin(0.5 * (nextI->p + nnextI->p), 1);
		TangentOnBezAt(at, bStPt, *nextI, fin, false, pos, tgt, len, rad);
	    } else if (k == stB->nb) {
		PathDescrIntermBezierTo *prevI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k - 1]);
		PathDescrIntermBezierTo *nextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k]);
		Geom::Point stP = 0.5 * ( prevI->p + nextI->p );
		TangentOnBezAt(at, stP, *nextI, *stB, false, pos, tgt, len, rad);
	    } else {
		PathDescrIntermBezierTo *prevI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k - 1]);
		PathDescrIntermBezierTo *nextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k]);
		PathDescrIntermBezierTo *nnextI = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[bez_st + k + 1]);
		Geom::Point stP = 0.5 * ( prevI->p + nextI->p );
		PathDescrBezierTo fin(0.5 * (nnextI->p + nnextI->p), 1);
		TangentOnBezAt(at, stP, *nextI, fin, false, pos, tgt, len, rad);
	    }
	}
    }
}

void Path::Transform(const Geom::Affine &trans)
{
    for (std::vector<PathDescr*>::iterator i = descr_cmd.begin(); i != descr_cmd.end(); ++i) {
        (*i)->transform(trans);
    }
}

void Path::FastBBox(double &l,double &t,double &r,double &b)
{
    l = t = r = b = 0;
    bool empty = true;
    Geom::Point lastP(0, 0);
    
    for (int i = 0; i < int(descr_cmd.size()); i++) {
	int const typ = descr_cmd[i]->getType();
	switch ( typ ) {
	case descr_lineto:
	{
	    PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
	    if ( empty ) {
		l = r = nData->p[Geom::X];
		t = b = nData->p[Geom::Y];
		empty = false;
	    } else {
		if ( nData->p[Geom::X] < l ) {
		    l = nData->p[Geom::X];
		}
		if ( nData->p[Geom::X] > r ) {
		    r = nData->p[Geom::X];
		}
		if ( nData->p[Geom::Y] < t ) {
		    t = nData->p[Geom::Y];
		}
		if ( nData->p[Geom::Y] > b ) {
		    b = nData->p[Geom::Y];
		}
	    }
	    lastP = nData->p;
	}
        break;
	
	case descr_moveto:
	{
	    PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
	    if ( empty ) {
		l = r = nData->p[Geom::X];
		t = b = nData->p[Geom::Y];
		empty = false;
	    } else {
		if ( nData->p[Geom::X] < l ) {
		    l = nData->p[Geom::X];
		}
		if ( nData->p[Geom::X] > r ) {
		    r = nData->p[Geom::X];
		}
		if ( nData->p[Geom::Y] < t ) {
		    t = nData->p[Geom::Y];
		}
		if ( nData->p[Geom::Y] > b ) {
		    b = nData->p[Geom::Y];
		}
	    }
	    lastP = nData->p;
	}
        break;
	
	case descr_arcto:
	{
	    PathDescrArcTo *nData  =  dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
	    if ( empty ) {
		l = r = nData->p[Geom::X];
		t = b = nData->p[Geom::Y];
		empty = false;
	    } else {
		if ( nData->p[Geom::X] < l ) {
		    l = nData->p[Geom::X];
		}
		if ( nData->p[Geom::X] > r ) {
		    r = nData->p[Geom::X];
		}
		if ( nData->p[Geom::Y] < t ) {
		    t = nData->p[Geom::Y];
		}
		if ( nData->p[Geom::Y] > b ) {
		    b = nData->p[Geom::Y];
		}
	    }
	    lastP = nData->p;
	}
        break;
	
	case descr_cubicto:
	{
	    PathDescrCubicTo *nData  =  dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
	    if ( empty ) {
		l = r = nData->p[Geom::X];
		t = b = nData->p[Geom::Y];
		empty = false;
	    } else {
		if ( nData->p[Geom::X] < l ) {
		    l = nData->p[Geom::X];
		}
		if ( nData->p[Geom::X] > r ) {
		    r = nData->p[Geom::X];
		}
		if ( nData->p[Geom::Y] < t ) {
		    t = nData->p[Geom::Y];
		}
		if ( nData->p[Geom::Y] > b ) {
		    b = nData->p[Geom::Y];
		}
	    }
	    
/* bug 249665: "...the calculation of the bounding-box for cubic-paths
has some extra steps to make it work corretly in Win32 that unfortunately
are unnecessary in Linux, generating wrong results. This only shows in 
Type1 fonts because they use cubic-paths instead of the
bezier-paths used by True-Type fonts."
*/

#ifdef WIN32
	    Geom::Point np = nData->p - nData->end;
	    if ( np[Geom::X] < l ) {
		l = np[Geom::X];
	    }
	    if ( np[Geom::X] > r ) {
		r = np[Geom::X];
	    }
	    if ( np[Geom::Y] < t ) {
		t = np[Geom::Y];
	    }
	    if ( np[Geom::Y] > b ) {
		b = np[Geom::Y];
	    }
	    
	    np = lastP + nData->start;
	    if ( np[Geom::X] < l ) {
		l = np[Geom::X];
	    }
	    if ( np[Geom::X] > r ) {
		r = np[Geom::X];
	    }
	    if ( np[Geom::Y] < t ) {
		t = np[Geom::Y];
	    }
	    if ( np[Geom::Y] > b ) {
		b = np[Geom::Y];
	    }
#endif

	    lastP = nData->p;
	}
        break;
	
	case descr_bezierto:
	{
	    PathDescrBezierTo *nData  =  dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
	    if ( empty ) {
		l = r = nData->p[Geom::X];
		t = b = nData->p[Geom::Y];
		empty = false;
	    } else {
		if ( nData->p[Geom::X] < l ) {
		    l = nData->p[Geom::X];
		}
		if ( nData->p[Geom::X] > r ) {
		    r = nData->p[Geom::X];
		}
		if ( nData->p[Geom::Y] < t ) {
		    t = nData->p[Geom::Y];
		}
		if ( nData->p[Geom::Y] > b ) {
		    b = nData->p[Geom::Y];
		}
	    }
	    lastP = nData->p;
	}
        break;
	
	case descr_interm_bezier:
	{
	    PathDescrIntermBezierTo *nData  =  dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
	    if ( empty ) {
		l = r = nData->p[Geom::X];
		t = b = nData->p[Geom::Y];
		empty = false;
	    } else {
		if ( nData->p[Geom::X] < l ) {
		    l = nData->p[Geom::X];
		}
		if ( nData->p[Geom::X] > r ) {
		    r = nData->p[Geom::X];
		}
		if ( nData->p[Geom::Y] < t ) {
		    t = nData->p[Geom::Y];
		}
		if ( nData->p[Geom::Y] > b ) {
		    b = nData->p[Geom::Y];
		}
	    }
	}
        break;
	}
    }
}

char *Path::svg_dump_path() const
{
    Inkscape::SVGOStringStream os;

    for (int i = 0; i < int(descr_cmd.size()); i++) {
        Geom::Point const p = (i == 0) ? Geom::Point(0, 0) : PrevPoint(i - 1);
        descr_cmd[i]->dumpSVG(os, p);
    }
  
    return g_strdup (os.str().c_str());
}

// Find out if the segment that corresponds to 'piece' is a straight line
bool Path::IsLineSegment(int piece)
{
    if (piece < 0 || piece >= int(descr_cmd.size())) {
        return false;
    }
    
    PathDescr const *theD = descr_cmd[piece];
    int const typ = theD->getType();
    
    return (typ == descr_lineto);
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

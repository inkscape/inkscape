/*
 *  PathCutting.cpp
 *  nlivarot
 *
 *  Created by fred on someday in 2004.
 *  public domain
 *
 *  Additional Code by Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 *  Copyright (C) 2005 Richard Hughes
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>
#include <cstdio>
#include <typeinfo>
#include "Path.h"
#include "style.h"
#include "livarot/path-description.h"
#include <2geom/pathvector.h>
#include <2geom/point.h>
#include <2geom/affine.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/curves.h>
#include "helper/geom-curves.h"
#include "helper/geom.h"

#include "svg/svg.h"

void  Path::DashPolyline(float head,float tail,float body,int nbD,float *dashs,bool stPlain,float stOffset)
{
  if ( nbD <= 0 || body <= 0.0001 ) return; // pas de tirets, en fait

  std::vector<path_lineto> orig_pts = pts;
  pts.clear();

  int       lastMI=-1;
  int curP = 0;
  int lastMP = -1;

  for (int i = 0; i < int(orig_pts.size()); i++) {
    if ( orig_pts[curP].isMoveTo == polyline_moveto ) {
      if ( lastMI >= 0 && lastMI < i-1 ) { // au moins 2 points
        DashSubPath(i-lastMI,lastMP, orig_pts, head,tail,body,nbD,dashs,stPlain,stOffset);
      }
      lastMI=i;
      lastMP=curP;
    }
    curP++;
  }
  if ( lastMI >= 0 && lastMI < int(orig_pts.size()) - 1 ) {
    DashSubPath(orig_pts.size() - lastMI, lastMP, orig_pts, head, tail, body, nbD, dashs, stPlain, stOffset);
  }
}

void  Path::DashPolylineFromStyle(SPStyle *style, float scale, float min_len)
{
    if (!style->stroke_dasharray.values.empty()) {

        double dlen = 0.0;
        // Find total length
        for (unsigned i = 0; i < style->stroke_dasharray.values.size(); i++) {
            dlen += style->stroke_dasharray.values[i] * scale;
        }
        if (dlen >= min_len) {
            // Extract out dash pattern (relative positions)
            double dash_offset = style->stroke_dashoffset.value * scale;
            size_t n_dash = style->stroke_dasharray.values.size();
            double *dash = g_new(double, n_dash);
            for (unsigned i = 0; i < n_dash; i++) {
                dash[i] = style->stroke_dasharray.values[i] * scale;
            }

            // Convert relative positions to absolute postions
            int    nbD = n_dash;
            float  *dashs=(float*)malloc((nbD+1)*sizeof(float));
            while ( dash_offset >= dlen ) dash_offset-=dlen;
            dashs[0]=dash[0];
            for (int i=1; i<nbD; i++) {
                dashs[i]=dashs[i-1]+dash[i];
            }

            // modulo dlen
            this->DashPolyline(0.0, 0.0, dlen, nbD, dashs, true, dash_offset);

            free(dashs);
            g_free(dash);
        }
    }
}


void Path::DashSubPath(int spL, int spP, std::vector<path_lineto> const &orig_pts, float head,float tail,float body,int nbD,float *dashs,bool stPlain,float stOffset)
{
  if ( spL <= 0 || spP == -1 ) return;
  
  double      totLength=0;
  Geom::Point   lastP;
  lastP = orig_pts[spP].p;
  for (int i=1;i<spL;i++) {
    Geom::Point const n = orig_pts[spP + i].p;
    Geom::Point d=n-lastP;
    double    nl=Geom::L2(d);
    if ( nl > 0.0001 ) {
      totLength+=nl;
      lastP=n;
    }
  }
  
  if ( totLength <= head+tail ) return; // tout mange par la tete et la queue
  
  double    curLength=0;
  double    dashPos=0;
  int       dashInd=0;
  bool      dashPlain=false;
  double    lastT=0;
  int       lastPiece=-1;
  lastP = orig_pts[spP].p;
  for (int i=1;i<spL;i++) {
    Geom::Point   n;
    int         nPiece=-1;
    double      nT=0;
    if ( back ) {
      n = orig_pts[spP + i].p;
      nPiece = orig_pts[spP + i].piece;
      nT = orig_pts[spP + i].t;
    } else {
      n = orig_pts[spP + i].p;
    }
    Geom::Point d=n-lastP;
    double    nl=Geom::L2(d);
    if ( nl > 0.0001 ) {
      double   stLength=curLength;
      double   enLength=curLength+nl;
      // couper les bouts en trop
      if ( curLength <= head && curLength+nl > head ) {
        nl-=head-curLength;
        curLength=head;
        dashInd=0;
        dashPos=stOffset;
        bool nPlain=stPlain;
        while ( dashs[dashInd] < stOffset ) {
          dashInd++;
          nPlain=!(nPlain);
          if ( dashInd >= nbD ) {
            dashPos=0;
            dashInd=0;
            break;
          }
        }
        if ( nPlain == true && dashPlain == false ) {
          Geom::Point  p=(enLength-curLength)*lastP+(curLength-stLength)*n;
          p/=(enLength-stLength);
          if ( back ) {
            double pT=0;
            if ( nPiece == lastPiece ) {
              pT=(lastT*(enLength-curLength)+nT*(curLength-stLength))/(enLength-stLength);
            } else {
              pT=(nPiece*(curLength-stLength))/(enLength-stLength);
            }
            AddPoint(p,nPiece,pT,true);
          } else {
            AddPoint(p,true);
          }
        } else if ( nPlain == false && dashPlain == true ) {
        }
        dashPlain=nPlain;
      }
      // faire les tirets
      if ( curLength >= head /*&& curLength+nl <= totLength-tail*/ ) {
        while ( curLength <= totLength-tail && nl > 0 ) {
          if ( enLength <= totLength-tail ) nl=enLength-curLength; else nl=totLength-tail-curLength;
          double  leftInDash=body-dashPos;
          if ( dashInd < nbD ) {
            leftInDash=dashs[dashInd]-dashPos;
          }
          if ( leftInDash <= nl ) {
            bool nPlain=false;
            if ( dashInd < nbD ) {
              dashPos=dashs[dashInd];
              dashInd++;
              if ( dashPlain ) nPlain=false; else nPlain=true;
            } else {
              dashInd=0;
              dashPos=0;
              //nPlain=stPlain;
              nPlain=dashPlain;
            }
            if ( nPlain == true && dashPlain == false ) {
              Geom::Point  p=(enLength-curLength-leftInDash)*lastP+(curLength+leftInDash-stLength)*n;
              p/=(enLength-stLength);
              if ( back ) {
                double pT=0;
                if ( nPiece == lastPiece ) {
                  pT=(lastT*(enLength-curLength-leftInDash)+nT*(curLength+leftInDash-stLength))/(enLength-stLength);
                } else {
                  pT=(nPiece*(curLength+leftInDash-stLength))/(enLength-stLength);
                }
                AddPoint(p,nPiece,pT,true);
              } else {
                AddPoint(p,true);
              }
            } else if ( nPlain == false && dashPlain == true ) {
              Geom::Point  p=(enLength-curLength-leftInDash)*lastP+(curLength+leftInDash-stLength)*n;
              p/=(enLength-stLength);
              if ( back ) {
                double pT=0;
                if ( nPiece == lastPiece ) {
                  pT=(lastT*(enLength-curLength-leftInDash)+nT*(curLength+leftInDash-stLength))/(enLength-stLength);
                } else {
                  pT=(nPiece*(curLength+leftInDash-stLength))/(enLength-stLength);
                }
                AddPoint(p,nPiece,pT,false);
              } else {
                AddPoint(p,false);
              }
            }
            dashPlain=nPlain;
            
            curLength+=leftInDash;
            nl-=leftInDash;
          } else {
            dashPos+=nl;
            curLength+=nl;
            nl=0;
          }
        }
        if ( dashPlain ) {
          if ( back ) {
            AddPoint(n,nPiece,nT,false);
          } else {
            AddPoint(n,false);
          }
        }
        nl=enLength-curLength;
      }
      if ( curLength <= totLength-tail && curLength+nl > totLength-tail ) {
        nl=totLength-tail-curLength;
        dashInd=0;
        dashPos=0;
        bool nPlain=false;
        if ( nPlain == true && dashPlain == false ) {
        } else if ( nPlain == false && dashPlain == true ) {
          Geom::Point  p=(enLength-curLength)*lastP+(curLength-stLength)*n;
          p/=(enLength-stLength);
          if ( back ) {
            double pT=0;
            if ( nPiece == lastPiece ) {
              pT=(lastT*(enLength-curLength)+nT*(curLength-stLength))/(enLength-stLength);
            } else {
              pT=(nPiece*(curLength-stLength))/(enLength-stLength);
            }
            AddPoint(p,nPiece,pT,false);
          } else {
            AddPoint(p,false);
          }
        }
        dashPlain=nPlain;
      }
      // continuer
      curLength=enLength;
      lastP=n;
      lastPiece=nPiece;
      lastT=nT;
    }
  }
}

Geom::PathVector *
Path::MakePathVector()
{
    Geom::PathVector *pv = new Geom::PathVector();
    Geom::Path * currentpath = NULL;

    Geom::Point   lastP,bezSt,bezEn;
    int         bezNb=0;
    for (int i=0;i<int(descr_cmd.size());i++) {
        int const typ = descr_cmd[i]->getType();
        switch ( typ ) {
            case descr_close:
            {
                currentpath->close(true);
            }
            break;

            case descr_lineto:
            {
                PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
                currentpath->appendNew<Geom::LineSegment>(Geom::Point(nData->p[0], nData->p[1]));
                lastP = nData->p;
            }
            break;

            case descr_moveto:
            {
                PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
                pv->push_back(Geom::Path());
                currentpath = &pv->back();
                currentpath->start(Geom::Point(nData->p[0], nData->p[1]));
                lastP = nData->p;
            }
            break;

            case descr_arcto:
            {
                /* TODO: add testcase for this descr_arcto case */
                PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
                currentpath->appendNew<Geom::EllipticalArc>( nData->rx, nData->ry, nData->angle*M_PI/180.0, nData->large, !nData->clockwise, nData->p );
                lastP = nData->p;
            }
            break;

            case descr_cubicto:
            {
                PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
                gdouble x1=lastP[0]+0.333333*nData->start[0];
                gdouble y1=lastP[1]+0.333333*nData->start[1];
                gdouble x2=nData->p[0]-0.333333*nData->end[0];
                gdouble y2=nData->p[1]-0.333333*nData->end[1];
                gdouble x3=nData->p[0];
                gdouble y3=nData->p[1];
                currentpath->appendNew<Geom::CubicBezier>( Geom::Point(x1,y1) , Geom::Point(x2,y2) , Geom::Point(x3,y3) );
                lastP = nData->p;
            }
            break;

            case descr_bezierto:
            {
                PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
                if ( nData->nb <= 0 ) {
                    currentpath->appendNew<Geom::LineSegment>( Geom::Point(nData->p[0], nData->p[1]) );
                    bezNb=0;
                } else if ( nData->nb == 1 ){
                    PathDescrIntermBezierTo *iData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i+1]);
                    gdouble x1=0.333333*(lastP[0]+2*iData->p[0]);
                    gdouble y1=0.333333*(lastP[1]+2*iData->p[1]);
                    gdouble x2=0.333333*(nData->p[0]+2*iData->p[0]);
                    gdouble y2=0.333333*(nData->p[1]+2*iData->p[1]);
                    gdouble x3=nData->p[0];
                    gdouble y3=nData->p[1];
                    currentpath->appendNew<Geom::CubicBezier>( Geom::Point(x1,y1) , Geom::Point(x2,y2) , Geom::Point(x3,y3) );
                    bezNb=0;
                } else {
                    bezSt = 2*lastP-nData->p;
                    bezEn = nData->p;
                    bezNb = nData->nb;
                }
                lastP = nData->p;
            }
            break;

            case descr_interm_bezier:
            {
                if ( bezNb > 0 ) {
                    PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
                    Geom::Point p_m=nData->p,p_s=0.5*(bezSt+p_m),p_e;
                    if ( bezNb > 1 ) {
                        PathDescrIntermBezierTo *iData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i+1]);
                        p_e=0.5*(p_m+iData->p);
                    } else {
                        p_e=bezEn;
                    }

                    Geom::Point  cp1=0.333333*(p_s+2*p_m),cp2=0.333333*(2*p_m+p_e);
                    gdouble x1=cp1[0];
                    gdouble y1=cp1[1];
                    gdouble x2=cp2[0];
                    gdouble y2=cp2[1];
                    gdouble x3=p_e[0];
                    gdouble y3=p_e[1];
                    currentpath->appendNew<Geom::CubicBezier>( Geom::Point(x1,y1) , Geom::Point(x2,y2) , Geom::Point(x3,y3) );

                    bezNb--;
                }
            }
            break;
        }
    }

    return pv;
}

void  Path::AddCurve(Geom::Curve const &c)
{
    if( is_straight_curve(c) )
    {
        LineTo( c.finalPoint() );
    }
    /*
    else if(Geom::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::QuadraticBezier const  *>(c)) {
        ...
    }
    */
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const *>(&c)) {
        Geom::Point tmp = (*cubic_bezier)[3];
        Geom::Point tms = 3 * ((*cubic_bezier)[1] - (*cubic_bezier)[0]);
        Geom::Point tme = 3 * ((*cubic_bezier)[3] - (*cubic_bezier)[2]);
        CubicTo (tmp, tms, tme);
    }
    else if(Geom::EllipticalArc const *elliptical_arc = dynamic_cast<Geom::EllipticalArc const *>(&c)) {
        ArcTo( elliptical_arc->finalPoint(),
               elliptical_arc->ray(Geom::X), elliptical_arc->ray(Geom::Y),
               elliptical_arc->rotationAngle()*180.0/M_PI,  // convert from radians to degrees
               elliptical_arc->largeArc(), !elliptical_arc->sweep() );
    } else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            AddCurve(*iter);
        }
    }
}

/**  append is false by default: it means that the path should be resetted. If it is true, the path is not resetted and Geom::Path will be appended as a new path
 */
void  Path::LoadPath(Geom::Path const &path, Geom::Affine const &tr, bool doTransformation, bool append)
{
    if (!append) {
        SetBackData (false);
        Reset();
    }
    if (path.empty())
        return;

    // TODO: this can be optimized by not generating a new path here, but doing the transform in AddCurve
    //       directly on the curve parameters

    Geom::Path const pathtr = doTransformation ? path * tr : path;

    MoveTo( pathtr.initialPoint() );

    for(Geom::Path::const_iterator cit = pathtr.begin(); cit != pathtr.end(); ++cit) {
        AddCurve(*cit);
    }

    if (pathtr.closed()) {
        Close();
    }
}

void  Path::LoadPathVector(Geom::PathVector const &pv)
{
    LoadPathVector(pv, Geom::Affine(), false);
}

void  Path::LoadPathVector(Geom::PathVector const &pv, Geom::Affine const &tr, bool doTransformation)
{
    SetBackData (false);
    Reset();

    // FIXME: 2geom is currently unable to maintain SVGElliptical arcs through transformation, and
    // sometimes it crashes on a continuity error during conversions, therefore convert to beziers here.
    // (the fix is of course to fix 2geom and then remove this if-statement, and just execute the 'else'-clause)
    if (doTransformation) {
        Geom::PathVector pvbezier = pathv_to_linear_and_cubic_beziers(pv);
        for(Geom::PathVector::const_iterator it = pvbezier.begin(); it != pvbezier.end(); ++it) {
            LoadPath(*it, tr, doTransformation, true);
        }
    } else {
        for(Geom::PathVector::const_iterator it = pv.begin(); it != pv.end(); ++it) {
            LoadPath(*it, tr, doTransformation, true);
        }
    }
}

/**
 *    \return Length of the lines in the pts vector.
 */

double Path::Length()
{
    if ( pts.empty() ) {
        return 0;
    }

    Geom::Point lastP = pts[0].p;

    double len = 0;
    for (std::vector<path_lineto>::const_iterator i = pts.begin(); i != pts.end(); ++i) {

        if ( i->isMoveTo != polyline_moveto ) {
            len += Geom::L2(i->p - lastP);
        }

        lastP = i->p;
    }
    
    return len;
}


double Path::Surface()
{
    if ( pts.empty() ) {
        return 0;
    }
    
    Geom::Point lastM = pts[0].p;
    Geom::Point lastP = lastM;

    double surf = 0;
    for (std::vector<path_lineto>::const_iterator i = pts.begin(); i != pts.end(); ++i) {

        if ( i->isMoveTo == polyline_moveto ) {
            surf += Geom::cross(lastM, lastM - lastP);
            lastP = lastM = i->p;
        } else {
            surf += Geom::cross(i->p, i->p - lastP);
            lastP = i->p;
        }
        
    }
    
  return surf;
}


Path**      Path::SubPaths(int &outNb,bool killNoSurf)
{
  int      nbRes=0;
  Path**   res=NULL;
  Path*    curAdd=NULL;
  
  for (int i=0;i<int(descr_cmd.size());i++) {
    int const typ = descr_cmd[i]->getType();
    switch ( typ ) {
      case descr_moveto:
        if ( curAdd ) {
          if ( curAdd->descr_cmd.size() > 1 ) {
            curAdd->Convert(1.0);
            double addSurf=curAdd->Surface();
            if ( fabs(addSurf) > 0.0001 || killNoSurf == false ) {
              res=(Path**)g_realloc(res,(nbRes+1)*sizeof(Path*));
              res[nbRes++]=curAdd;
            } else { 
              delete curAdd;
            }
          } else {
            delete curAdd;
          }
          curAdd=NULL;
        }
        curAdd=new Path;
        curAdd->SetBackData(false);
        {
          PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
          curAdd->MoveTo(nData->p);
        }
          break;
      case descr_close:
      {
        curAdd->Close();
      }
        break;        
      case descr_lineto:
      {
        PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
        curAdd->LineTo(nData->p);
      }
        break;
      case descr_cubicto:
      {
        PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
        curAdd->CubicTo(nData->p,nData->start,nData->end);
      }
        break;
      case descr_arcto:
      {
        PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
        curAdd->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
      }
        break;
      case descr_bezierto:
      {
        PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
        curAdd->BezierTo(nData->p);
      }
        break;
      case descr_interm_bezier:
      {
        PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
        curAdd->IntermBezierTo(nData->p);
      }
        break;
      default:
        break;
    }
  }
  if ( curAdd ) {
    if ( curAdd->descr_cmd.size() > 1 ) {
      curAdd->Convert(1.0);
      double addSurf=curAdd->Surface();
      if ( fabs(addSurf) > 0.0001 || killNoSurf == false  ) {
        res=(Path**)g_realloc(res,(nbRes+1)*sizeof(Path*));
        res[nbRes++]=curAdd;
      } else {
        delete curAdd;
      }
    } else {
      delete curAdd;
    }
  }
  curAdd=NULL;
  
  outNb=nbRes;
  return res;
}
Path**      Path::SubPathsWithNesting(int &outNb,bool killNoSurf,int nbNest,int* nesting,int* conts)
{
  int      nbRes=0;
  Path**   res=NULL;
  Path*    curAdd=NULL;
  bool     increment=false;
  
  for (int i=0;i<int(descr_cmd.size());i++) {
    int const typ = descr_cmd[i]->getType();
    switch ( typ ) {
      case descr_moveto:
      {
        if ( curAdd && increment == false ) {
          if ( curAdd->descr_cmd.size() > 1 ) {
            // sauvegarder descr_cmd[0]->associated
            int savA=curAdd->descr_cmd[0]->associated;
            curAdd->Convert(1.0);
            curAdd->descr_cmd[0]->associated=savA; // associated n'est pas utilise apres
            double addSurf=curAdd->Surface();
            if ( fabs(addSurf) > 0.0001 || killNoSurf == false ) {
              res=(Path**)g_realloc(res,(nbRes+1)*sizeof(Path*));
              res[nbRes++]=curAdd;
            } else { 
              delete curAdd;
            }
          } else {
            delete curAdd;
          }
          curAdd=NULL;
        }
        Path*  hasParent=NULL;
        for (int j=0;j<nbNest;j++) {
          if ( conts[j] == i && nesting[j] >= 0 ) {
            int  parentMvt=conts[nesting[j]];
            for (int k=0;k<nbRes;k++) {
              if ( res[k] && res[k]->descr_cmd.empty() == false && res[k]->descr_cmd[0]->associated == parentMvt ) {
                hasParent=res[k];
                break;
              }
            }
          }
          if ( conts[j] > i  ) break;
        }
        if ( hasParent ) {
          curAdd=hasParent;
          increment=true;
        } else {
          curAdd=new Path;
          curAdd->SetBackData(false);
          increment=false;
        }
        PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
        int mNo=curAdd->MoveTo(nData->p);
        curAdd->descr_cmd[mNo]->associated=i;
        }
        break;
      case descr_close:
      {
        curAdd->Close();
      }
        break;        
      case descr_lineto:
      {
        PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
        curAdd->LineTo(nData->p);
      }
        break;
      case descr_cubicto:
      {
        PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
        curAdd->CubicTo(nData->p,nData->start,nData->end);
      }
        break;
      case descr_arcto:
      {
        PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
        curAdd->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
      }
        break;
      case descr_bezierto:
      {
        PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
        curAdd->BezierTo(nData->p);
      }
        break;
      case descr_interm_bezier:
      {
        PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
        curAdd->IntermBezierTo(nData->p);
      }
        break;
      default:
        break;
    }
  }
  if ( curAdd && increment == false ) {
    if ( curAdd->descr_cmd.size() > 1 ) {
      curAdd->Convert(1.0);
      double addSurf=curAdd->Surface();
      if ( fabs(addSurf) > 0.0001 || killNoSurf == false  ) {
        res=(Path**)g_realloc(res,(nbRes+1)*sizeof(Path*));
        res[nbRes++]=curAdd;
      } else {
        delete curAdd;
      }
    } else {
      delete curAdd;
    }
  }
  curAdd=NULL;
  
  outNb=nbRes;
  return res;
}


void Path::ConvertForcedToVoid()
{  
    for (int i=0; i < int(descr_cmd.size()); i++) {
        if ( descr_cmd[i]->getType() == descr_forced) {
            delete descr_cmd[i];
            descr_cmd.erase(descr_cmd.begin() + i);
        }
    }
}


void Path::ConvertForcedToMoveTo()
{  
    Geom::Point lastSeen(0, 0);
    Geom::Point lastMove(0, 0);
    
    {
        Geom::Point lastPos(0, 0);
        for (int i = int(descr_cmd.size()) - 1; i >= 0; i--) {
            int const typ = descr_cmd[i]->getType();
            switch ( typ ) {
            case descr_forced:
            {
                PathDescrForced *d = dynamic_cast<PathDescrForced *>(descr_cmd[i]);
                d->p = lastPos;
                break;
            }
            case descr_close:
            {
                PathDescrClose *d = dynamic_cast<PathDescrClose *>(descr_cmd[i]);
                d->p = lastPos;
                break;
            }
            case descr_moveto:
            {
                PathDescrMoveTo *d = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
                lastPos = d->p;
                break;
            }
            case descr_lineto:
            {
                PathDescrLineTo *d = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
                lastPos = d->p;
                break;
            }
            case descr_arcto:
            {
                PathDescrArcTo *d = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
                lastPos = d->p;
                break;
            }
            case descr_cubicto:
            {
                PathDescrCubicTo *d = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
                lastPos = d->p;
                break;
            }
            case descr_bezierto:
            {
                PathDescrBezierTo *d = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
                lastPos = d->p;
                break;
            }
            case descr_interm_bezier:
            {
                PathDescrIntermBezierTo *d = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
                lastPos = d->p;
                break;
            }
            default:
                break;
            }
        }
    }

    bool hasMoved = false;
    for (int i = 0; i < int(descr_cmd.size()); i++) {
        int const typ = descr_cmd[i]->getType();
        switch ( typ ) {
        case descr_forced:
            if ( i < int(descr_cmd.size()) - 1 && hasMoved ) { // sinon il termine le chemin

                delete descr_cmd[i];
                descr_cmd[i] = new PathDescrMoveTo(lastSeen);
                lastMove = lastSeen;
                hasMoved = true;
            }
            break;
            
        case descr_moveto:
        {
          PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
          lastMove = lastSeen = nData->p;
          hasMoved = true;
        }
        break;
      case descr_close:
      {
        lastSeen=lastMove;
      }
        break;        
      case descr_lineto:
      {
        PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
        lastSeen=nData->p;
      }
        break;
      case descr_cubicto:
      {
        PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
        lastSeen=nData->p;
     }
        break;
      case descr_arcto:
      {
        PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
        lastSeen=nData->p;
      }
        break;
      case descr_bezierto:
      {
        PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
        lastSeen=nData->p;
     }
        break;
      case descr_interm_bezier:
      {
        PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
        lastSeen=nData->p;
      }
        break;
      default:
        break;
    }
  }
}
static int       CmpPosition(const void * p1, const void * p2) {
  Path::cut_position *cp1=(Path::cut_position*)p1;
  Path::cut_position *cp2=(Path::cut_position*)p2;
  if ( cp1->piece < cp2->piece ) return -1;
  if ( cp1->piece > cp2->piece ) return 1;
  if ( cp1->t < cp2->t ) return -1;
  if ( cp1->t > cp2->t ) return 1;
  return 0;
}
static int       CmpCurv(const void * p1, const void * p2) {
  double *cp1=(double*)p1;
  double *cp2=(double*)p2;
  if ( *cp1 < *cp2 ) return -1;
  if ( *cp1 > *cp2 ) return 1;
  return 0;
}


Path::cut_position* Path::CurvilignToPosition(int nbCv, double *cvAbs, int &nbCut)
{
    if ( nbCv <= 0 || pts.empty() || back == false ) {
        return NULL;
    }
  
    qsort(cvAbs, nbCv, sizeof(double), CmpCurv);
  
    cut_position *res = NULL;
    nbCut = 0;
    int curCv = 0;
  
    double len = 0;
    double lastT = 0;
    int lastPiece = -1;

    Geom::Point lastM = pts[0].p;
    Geom::Point lastP = lastM;

    for (std::vector<path_lineto>::const_iterator i = pts.begin(); i != pts.end(); ++i) {

        if ( i->isMoveTo == polyline_moveto ) {

            lastP = lastM = i->p;
            lastT = i->t;
            lastPiece = i->piece;

        } else {
            
            double const add = Geom::L2(i->p - lastP);
            double curPos = len;
            double curAdd = add;
            
            while ( curAdd > 0.0001 && curCv < nbCv && curPos + curAdd >= cvAbs[curCv] ) {
                double const theta = (cvAbs[curCv] - len) / add;
                res = (cut_position*) g_realloc(res, (nbCut + 1) * sizeof(cut_position));
                res[nbCut].piece = i->piece;
                res[nbCut].t = theta * i->t + (1 - theta) * ( (lastPiece != i->piece) ? 0 : lastT);
                nbCut++;
                curAdd -= cvAbs[curCv] - curPos;
                curPos = cvAbs[curCv];
                curCv++;
            }
            
            len += add;
            lastPiece = i->piece;
            lastP = i->p;
            lastT = i->t;
        }
    }
    
    return res;
}

/* 
Moved from Layout-TNG-OutIter.cpp
TODO: clean up uses of the original function and remove

Original Comment:
"this function really belongs to Path. I'll probably move it there eventually,
hence the Path-esque coding style"

*/
template<typename T> inline static T square(T x) {return x*x;}
Path::cut_position Path::PointToCurvilignPosition(Geom::Point const &pos, unsigned seg) const
{
    // if the parameter "seg" == 0, then all segments will be considered
    // In however e.g. "seg" == 6 , then only the 6th segment will be considered 
 
    unsigned bestSeg = 0;
    double bestRangeSquared = DBL_MAX;
    double bestT = 0.0; // you need a sentinel, or make sure that you prime with correct values.

    for (unsigned i = 1 ; i < pts.size() ; i++) {
        if (pts[i].isMoveTo == polyline_moveto || (seg > 0 && i != seg)) continue;
        Geom::Point p1, p2, localPos;
        double thisRangeSquared;
        double t;

        if (pts[i - 1].p == pts[i].p) {
            thisRangeSquared = square(pts[i].p[Geom::X] - pos[Geom::X]) + square(pts[i].p[Geom::Y] - pos[Geom::Y]);
            t = 0.0;
        } else {
            // we rotate all our coordinates so we're always looking at a mostly vertical line.
            if (fabs(pts[i - 1].p[Geom::X] - pts[i].p[Geom::X]) < fabs(pts[i - 1].p[Geom::Y] - pts[i].p[Geom::Y])) {
                p1 = pts[i - 1].p;
                p2 = pts[i].p;
                localPos = pos;
            } else {
                p1 = pts[i - 1].p.cw();
                p2 = pts[i].p.cw();
                localPos = pos.cw();
            }
            double gradient = (p2[Geom::X] - p1[Geom::X]) / (p2[Geom::Y] - p1[Geom::Y]);
            double intersection = p1[Geom::X] - gradient * p1[Geom::Y];
            /*
              orthogonalGradient = -1.0 / gradient; // you are going to have numerical problems here.
              orthogonalIntersection = localPos[Geom::X] - orthogonalGradient * localPos[Geom::Y];
              nearestY = (orthogonalIntersection - intersection) / (gradient - orthogonalGradient);

              expand out nearestY fully :
              nearestY = (localPos[Geom::X] - (-1.0 / gradient) * localPos[Geom::Y] - intersection) / (gradient - (-1.0 / gradient));

              multiply top and bottom by gradient:
              nearestY = (localPos[Geom::X] * gradient - (-1.0) * localPos[Geom::Y] - intersection * gradient) / (gradient * gradient - (-1.0));

              and simplify to get:
            */
            double nearestY =  (localPos[Geom::X] * gradient + localPos[Geom::Y] - intersection * gradient)
                             / (gradient * gradient + 1.0);
            t = (nearestY - p1[Geom::Y]) / (p2[Geom::Y] - p1[Geom::Y]);
            if (t <= 0.0) {
                thisRangeSquared = square(p1[Geom::X] - localPos[Geom::X]) + square(p1[Geom::Y] - localPos[Geom::Y]);
                t = 0.0;
            } else if (t >= 1.0) {
                thisRangeSquared = square(p2[Geom::X] - localPos[Geom::X]) + square(p2[Geom::Y] - localPos[Geom::Y]);
                t = 1.0;
            } else {
                thisRangeSquared = square(nearestY * gradient + intersection - localPos[Geom::X]) + square(nearestY - localPos[Geom::Y]);
            }
        }

        if (thisRangeSquared < bestRangeSquared) {
            bestSeg = i;
            bestRangeSquared = thisRangeSquared;
            bestT = t;
        }
    }
    Path::cut_position result;
    if (bestSeg == 0) {
        result.piece = 0;
        result.t = 0.0;
    } else {
        result.piece = pts[bestSeg].piece;
        if (result.piece == pts[bestSeg - 1].piece) {
            result.t = pts[bestSeg - 1].t * (1.0 - bestT) + pts[bestSeg].t * bestT;
        } else {
            result.t = pts[bestSeg].t * bestT;
        }
    }
    return result;
}
/*
    this one also belongs to Path
    returns the length of the path up to the position indicated by t (0..1)

    TODO: clean up uses of the original function and remove

    should this take a cut_position as a parameter?
*/
double Path::PositionToLength(int piece, double t)
{
    double length = 0.0;
    for (unsigned i = 1 ; i < pts.size() ; i++) {
        if (pts[i].isMoveTo == polyline_moveto) continue;
        if (pts[i].piece == piece && t < pts[i].t) {
            length += Geom::L2((t - pts[i - 1].t) / (pts[i].t - pts[i - 1].t) * (pts[i].p - pts[i - 1].p));
            break;
        }
        length += Geom::L2(pts[i].p - pts[i - 1].p);
    }
    return length;
}

void Path::ConvertPositionsToForced(int nbPos, cut_position *poss)
{
    if ( nbPos <= 0 ) {
        return;
    }
    
    {
        Geom::Point lastPos(0, 0);
        for (int i = int(descr_cmd.size()) - 1; i >= 0; i--) {
            int const typ = descr_cmd[i]->getType();
            switch ( typ ) {
                
            case descr_forced:
            {
                PathDescrForced *d = dynamic_cast<PathDescrForced *>(descr_cmd[i]);
                d->p = lastPos;
                break;
            }
                
            case descr_close:
            {
                delete descr_cmd[i];
                descr_cmd[i] = new PathDescrLineTo(Geom::Point(0, 0));

                int fp = i - 1;
                while ( fp >= 0 && (descr_cmd[fp]->getType()) != descr_moveto ) {
                    fp--;
                }
                
                if ( fp >= 0 ) {
                    PathDescrMoveTo *oData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[fp]);
                    dynamic_cast<PathDescrLineTo*>(descr_cmd[i])->p = oData->p;
                }
            }
            break;
            
            case descr_bezierto:
            {
                PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
                Geom::Point theP = nData->p;
                if ( nData->nb == 0 ) {
                    lastPos = theP;
                }
            }
            break;
            
        case descr_moveto:
        {
            PathDescrMoveTo *d = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
            lastPos = d->p;
            break;
        }
        case descr_lineto:
        {
            PathDescrLineTo *d = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
            lastPos = d->p;
            break;
        }
        case descr_arcto:
        {
            PathDescrArcTo *d = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
            lastPos = d->p;
            break;
        }
        case descr_cubicto:
        {
            PathDescrCubicTo *d = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
            lastPos = d->p;
            break;
        }
        case descr_interm_bezier:
        {
            PathDescrIntermBezierTo *d = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
            lastPos = d->p;
            break;
        }
        default:
          break;
      }
    }
  }
  if (descr_cmd[0]->getType() == descr_moveto)
    descr_flags |= descr_doing_subpath;         // see LP Bug 166302

  qsort(poss, nbPos, sizeof(cut_position), CmpPosition);

  for (int curP=0;curP<nbPos;curP++) {
    int   cp=poss[curP].piece;
    if ( cp < 0 || cp >= int(descr_cmd.size()) ) break;
    float ct=poss[curP].t;
    if ( ct < 0 ) continue;
    if ( ct > 1 ) continue;
        
    int const typ = descr_cmd[cp]->getType();
    if ( typ == descr_moveto || typ == descr_forced || typ == descr_close ) {
      // ponctuel= rien a faire
    } else if ( typ == descr_lineto || typ == descr_arcto || typ == descr_cubicto ) {
      // facile: creation d'un morceau et d'un forced -> 2 commandes
      Geom::Point        theP;
      Geom::Point        theT;
      Geom::Point        startP;
      startP=PrevPoint(cp-1);
      if ( typ == descr_cubicto ) {
        double           len,rad;
        Geom::Point        stD,enD,endP;
        {
          PathDescrCubicTo *oData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[cp]);
          stD=oData->start;
          enD=oData->end;
          endP=oData->p;
          TangentOnCubAt (ct, startP, *oData,true, theP,theT,len,rad);
        }
        
        theT*=len;
        
        InsertCubicTo(endP,(1-ct)*theT,(1-ct)*enD,cp+1);
        InsertForcePoint(cp+1);
        {
          PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[cp]);
          nData->start=ct*stD;
          nData->end=ct*theT;
          nData->p=theP;
        }
        // decalages dans le tableau des positions de coupe
        for (int j=curP+1;j<nbPos;j++) {
          if ( poss[j].piece == cp ) {
            poss[j].piece+=2;
            poss[j].t=(poss[j].t-ct)/(1-ct);
          } else {
            poss[j].piece+=2;
          }
        }
      } else if ( typ == descr_lineto ) {
        Geom::Point        endP;
        {
          PathDescrLineTo *oData = dynamic_cast<PathDescrLineTo *>(descr_cmd[cp]);
          endP=oData->p;
        }

        theP=ct*endP+(1-ct)*startP;
        
        InsertLineTo(endP,cp+1);
        InsertForcePoint(cp+1);
        {
          PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[cp]);
          nData->p=theP;
        }
        // decalages dans le tableau des positions de coupe
       for (int j=curP+1;j<nbPos;j++) {
          if ( poss[j].piece == cp ) {
            poss[j].piece+=2;
            poss[j].t=(poss[j].t-ct)/(1-ct);
          } else {
            poss[j].piece+=2;
          }
        }
      } else if ( typ == descr_arcto ) {
        Geom::Point        endP;
        double           rx,ry,angle;
        bool             clockw,large;
        double   delta=0;
        {
          PathDescrArcTo *oData = dynamic_cast<PathDescrArcTo *>(descr_cmd[cp]);
          endP=oData->p;
          rx=oData->rx;
          ry=oData->ry;
          angle=oData->angle;
          clockw=oData->clockwise;
          large=oData->large;
        }
        {
          double      sang,eang;
          ArcAngles(startP,endP,rx,ry,angle*M_PI/180.0,large,clockw,sang,eang);
          
          if (clockw) {
            if ( sang < eang ) sang += 2*M_PI;
            delta=eang-sang;
          } else {
            if ( sang > eang ) sang -= 2*M_PI;
            delta=eang-sang;
          }
          if ( delta < 0 ) delta=-delta;
        }
        
        PointAt (cp,ct, theP);
        
        if ( delta*(1-ct) > M_PI ) {
          InsertArcTo(endP,rx,ry,angle,true,clockw,cp+1);
        } else {
          InsertArcTo(endP,rx,ry,angle,false,clockw,cp+1);
        }
        InsertForcePoint(cp+1);
        {
          PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[cp]);
          nData->p=theP;
          if ( delta*ct > M_PI ) {
            nData->clockwise=true;
          } else {
            nData->clockwise=false;
          }
        }
        // decalages dans le tableau des positions de coupe
        for (int j=curP+1;j<nbPos;j++) {
          if ( poss[j].piece == cp ) {
            poss[j].piece+=2;
            poss[j].t=(poss[j].t-ct)/(1-ct);
          } else {
            poss[j].piece+=2;
          }
        }
      }
    } else if ( typ == descr_bezierto || typ == descr_interm_bezier ) {
      // dur
      int theBDI=cp;
      while ( theBDI >= 0 && (descr_cmd[theBDI]->getType()) != descr_bezierto ) theBDI--;
      if ( (descr_cmd[theBDI]->getType()) == descr_bezierto ) {
        PathDescrBezierTo theBD=*(dynamic_cast<PathDescrBezierTo *>(descr_cmd[theBDI]));
        if ( cp >= theBDI && cp < theBDI+theBD.nb ) {
          if ( theBD.nb == 1 ) {
            Geom::Point        endP=theBD.p;
            Geom::Point        midP;
            Geom::Point        startP;
            startP=PrevPoint(theBDI-1);
            {
              PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[theBDI+1]);
              midP=nData->p;
            }
            Geom::Point       aP=ct*midP+(1-ct)*startP;
            Geom::Point       bP=ct*endP+(1-ct)*midP;
            Geom::Point       knotP=ct*bP+(1-ct)*aP;
                        
            InsertIntermBezierTo(bP,theBDI+2);
            InsertBezierTo(knotP,1,theBDI+2);
            InsertForcePoint(theBDI+2);
            {
              PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[theBDI+1]);
              nData->p=aP;
            }
            {
              PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[theBDI]);
              nData->p=knotP;
            }
            // decalages dans le tableau des positions de coupe
            for (int j=curP+1;j<nbPos;j++) {
              if ( poss[j].piece == cp ) {
                poss[j].piece+=3;
                poss[j].t=(poss[j].t-ct)/(1-ct);
              } else {
                poss[j].piece+=3;
              }
            }
            
          } else {
            // decouper puis repasser
            if ( cp > theBDI ) {
              Geom::Point   pcP,ncP;
              {
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[cp]);
                pcP=nData->p;
              }
              {
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[cp+1]);
                ncP=nData->p;
              }
              Geom::Point knotP=0.5*(pcP+ncP);
              
              InsertBezierTo(knotP,theBD.nb-(cp-theBDI),cp+1);
              {
                PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[theBDI]);
                nData->nb=cp-theBDI;
              }
              
              // decalages dans le tableau des positions de coupe
              for (int j=curP;j<nbPos;j++) {
                if ( poss[j].piece == cp ) {
                  poss[j].piece+=1;
                } else {
                  poss[j].piece+=1;
                }
              }
              curP--;
            } else {
              Geom::Point   pcP,ncP;
              {
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[cp+1]);
                pcP=nData->p;
              }
              {
                PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[cp+2]);
                ncP=nData->p;
              }
              Geom::Point knotP=0.5*(pcP+ncP);
              
              InsertBezierTo(knotP,theBD.nb-1,cp+2);
              {
                PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[theBDI]);
                nData->nb=1;
              }
              
              // decalages dans le tableau des positions de coupe
              for (int j=curP;j<nbPos;j++) {
                if ( poss[j].piece == cp ) {
//                  poss[j].piece+=1;
                } else {
                  poss[j].piece+=1;
                }
              }
              curP--;
            }
          }
        } else {
          // on laisse aussi tomber
        }
      } else {
        // on laisse tomber
      }
    }
  }
}

void        Path::ConvertPositionsToMoveTo(int nbPos,cut_position* poss)
{
  ConvertPositionsToForced(nbPos,poss);
//  ConvertForcedToMoveTo();
  // on fait une version customizee a la place

  Path*  res=new Path;
  
  Geom::Point    lastP(0,0);
  for (int i=0;i<int(descr_cmd.size());i++) {
    int const typ = descr_cmd[i]->getType();
    if ( typ == descr_moveto ) {
      Geom::Point  np;
      {
        PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[i]);
        np=nData->p;
      }
      Geom::Point  endP;
      bool       hasClose=false;
      int        hasForced=-1;
      bool       doesClose=false;
      int        j=i+1;
      for (;j<int(descr_cmd.size());j++) {
        int const ntyp = descr_cmd[j]->getType();
        if ( ntyp == descr_moveto ) {
          j--;
          break;
        } else if ( ntyp == descr_forced ) {
          if ( hasForced < 0 ) hasForced=j;
        } else if ( ntyp == descr_close ) {
          hasClose=true;
          break;
        } else if ( ntyp == descr_lineto ) {
          PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[j]);
          endP=nData->p;
        } else if ( ntyp == descr_arcto ) {
          PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[j]);
          endP=nData->p;
        } else if ( ntyp == descr_cubicto ) {
          PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[j]);
          endP=nData->p;
        } else if ( ntyp == descr_bezierto ) {
          PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[j]);
          endP=nData->p;
        } else {
        }
      }
      if ( Geom::LInfty(endP-np) < 0.00001 ) {
        doesClose=true;
      }
      if ( ( doesClose || hasClose ) && hasForced >= 0 ) {
 //       printf("nasty i=%i j=%i frc=%i\n",i,j,hasForced);
        // aghhh.
        Geom::Point   nMvtP=PrevPoint(hasForced);
        res->MoveTo(nMvtP);
        Geom::Point   nLastP=nMvtP;
        for (int k = hasForced + 1; k < j; k++) {
          int ntyp=descr_cmd[k]->getType();
          if ( ntyp == descr_moveto ) {
            // ne doit pas arriver
          } else if ( ntyp == descr_forced ) {
            res->MoveTo(nLastP);
          } else if ( ntyp == descr_close ) {
            // rien a faire ici; de plus il ne peut y en avoir qu'un
          } else if ( ntyp == descr_lineto ) {
            PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[k]);
            res->LineTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_arcto ) {
            PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[k]);
            res->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
            nLastP=nData->p;
          } else if ( ntyp == descr_cubicto ) {
            PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[k]);
            res->CubicTo(nData->p,nData->start,nData->end);
            nLastP=nData->p;
          } else if ( ntyp == descr_bezierto ) {
            PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[k]);
            res->BezierTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_interm_bezier ) {
            PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[k]);
            res->IntermBezierTo(nData->p);
          } else {
          }
        }
        if ( doesClose == false ) res->LineTo(np);
        nLastP=np;
        for (int k=i+1;k<hasForced;k++) {
          int ntyp=descr_cmd[k]->getType();
          if ( ntyp == descr_moveto ) {
            // ne doit pas arriver
          } else if ( ntyp == descr_forced ) {
            res->MoveTo(nLastP);
          } else if ( ntyp == descr_close ) {
            // rien a faire ici; de plus il ne peut y en avoir qu'un
          } else if ( ntyp == descr_lineto ) {
            PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[k]);
            res->LineTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_arcto ) {
            PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[k]);
            res->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
            nLastP=nData->p;
          } else if ( ntyp == descr_cubicto ) {
            PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[k]);
            res->CubicTo(nData->p,nData->start,nData->end);
            nLastP=nData->p;
          } else if ( ntyp == descr_bezierto ) {
            PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[k]);
            res->BezierTo(nData->p);
            nLastP=nData->p;
          } else if ( ntyp == descr_interm_bezier ) {
            PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[k]);
            res->IntermBezierTo(nData->p);
          } else {
          }
        }
        lastP=nMvtP;
        i=j;
      } else {
        // regular, just move on
        res->MoveTo(np);
        lastP=np;
      }
    } else if ( typ == descr_close ) {
      res->Close();
    } else if ( typ == descr_forced ) {
      res->MoveTo(lastP);
    } else if ( typ == descr_lineto ) {
      PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[i]);
      res->LineTo(nData->p);
      lastP=nData->p;
    } else if ( typ == descr_arcto ) {
      PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[i]);
      res->ArcTo(nData->p,nData->rx,nData->ry,nData->angle,nData->large,nData->clockwise);
      lastP=nData->p;
    } else if ( typ == descr_cubicto ) {
      PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(descr_cmd[i]);
      res->CubicTo(nData->p,nData->start,nData->end);
      lastP=nData->p;
    } else if ( typ == descr_bezierto ) {
      PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[i]);
      res->BezierTo(nData->p);
      lastP=nData->p;
    } else if ( typ == descr_interm_bezier ) {
      PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(descr_cmd[i]);
      res->IntermBezierTo(nData->p);
    } else {
    }
  }

  Copy(res);
  delete res;
  return;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

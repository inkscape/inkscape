/*
 *  ShapeMisc.cpp
 *  nlivarot
 *
 *  Created by fred on Sun Jul 20 2003.
 *
 */

#include "livarot/Shape.h"
#include "livarot/Path.h"
#include "livarot/path-description.h"
#include <glib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <2geom/point.h>
#include <2geom/affine.h>

/*
 * polygon offset and polyline to path reassembling (when using back data)
 */

// until i find something better
#define MiscNormalize(v) {\
  double _l=sqrt(dot(v,v)); \
    if ( _l < 0.0000001 ) { \
      v[0]=v[1]=0; \
    } else { \
      v/=_l; \
    }\
}

// extracting the contour of an uncrossed polygon: a mere depth first search
// more precisely that's extracting an eulerian path from a graph, but here we want to split
// the polygon into contours and avoid holes. so we take a "next counter-clockwise edge first" approach
// (make a checkboard and extract its contours to see the difference)
void
Shape::ConvertToForme (Path * dest)
{
  if (numberOfPoints() <= 1 || numberOfEdges() <= 1)
    return;
  
  // prepare
  dest->Reset ();
  
  MakePointData (true);
  MakeEdgeData (true);
  MakeSweepDestData (true);
  
  for (int i = 0; i < numberOfPoints(); i++)
  {
    pData[i].rx[0] = Round (getPoint(i).x[0]);
    pData[i].rx[1] = Round (getPoint(i).x[1]);
  }
  for (int i = 0; i < numberOfEdges(); i++)
  {
    eData[i].rdx = pData[getEdge(i).en].rx - pData[getEdge(i).st].rx;
  }
  
  // sort edge clockwise, with the closest after midnight being first in the doubly-linked list
  // that's vital to the algorithm...
  SortEdges ();
  
  // depth-first search implies: we make a stack of edges traversed.
  // precParc: previous in the stack
  // suivParc: next in the stack
  for (int i = 0; i < numberOfEdges(); i++)
  {
    swdData[i].misc = 0;
    swdData[i].precParc = swdData[i].suivParc = -1;
  }
  
  int searchInd = 0;
  
  int lastPtUsed = 0;
  do
  {
    // first get a starting point, and a starting edge
    // -> take the upper left point, and take its first edge
    // points traversed have swdData[].misc != 0, so it's easy
    int startBord = -1;
    {
      int fi = 0;
      for (fi = lastPtUsed; fi < numberOfPoints(); fi++)
      {
        if (getPoint(fi).incidentEdge[FIRST] >= 0 && swdData[getPoint(fi).incidentEdge[FIRST]].misc == 0)
          break;
      }
      lastPtUsed = fi + 1;
      if (fi < numberOfPoints())
      {
        int bestB = getPoint(fi).incidentEdge[FIRST];
        while (bestB >= 0 && getEdge(bestB).st != fi)
          bestB = NextAt (fi, bestB);
        if (bestB >= 0)
	      {
          startBord = bestB;
          dest->MoveTo (getPoint(getEdge(startBord).en).x);
	      }
      }
    }
    // and walk the graph, doing contours when needed
    if (startBord >= 0)
    {
      // parcours en profondeur pour mettre les leF et riF a leurs valeurs
      swdData[startBord].misc = (void *) 1;
      //                      printf("part de %d\n",startBord);
      int curBord = startBord;
      bool back = false;
      swdData[curBord].precParc = -1;
      swdData[curBord].suivParc = -1;
      do
	    {
	      int cPt = getEdge(curBord).en;
	      int nb = curBord;
        //                              printf("de curBord= %d au point %i  -> ",curBord,cPt);
        // get next edge
	      do
        {
          int nnb = CycleNextAt (cPt, nb);
          if (nnb == nb)
          {
            // cul-de-sac
            nb = -1;
            break;
          }
          nb = nnb;
          if (nb < 0 || nb == curBord)
            break;
        }
	      while (swdData[nb].misc != 0 || getEdge(nb).st != cPt);
        
	      if (nb < 0 || nb == curBord)
        {
          // no next edge: end of this contour, we get back
          if (back == false)
            dest->Close ();
          back = true;
          // retour en arriere
          curBord = swdData[curBord].precParc;
          //                                      printf("retour vers %d\n",curBord);
          if (curBord < 0)
            break;
        }
	      else
        {
          // new edge, maybe for a new contour
          if (back)
          {
            // we were backtracking, so if we have a new edge, that means we're creating a new contour
            dest->MoveTo (getPoint(cPt).x);
            back = false;
          }
          swdData[nb].misc = (void *) 1;
          swdData[nb].ind = searchInd++;
          swdData[nb].precParc = curBord;
          swdData[curBord].suivParc = nb;
          curBord = nb;
          //                                      printf("suite %d\n",curBord);
          {
            // add that edge
            dest->LineTo (getPoint(getEdge(nb).en).x);
          }
        }
	    }
      while (1 /*swdData[curBord].precParc >= 0 */ );
      // fin du cas non-oriente
    }
  }
  while (lastPtUsed < numberOfPoints());
  
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepDestData (false);
}

// same as before, but each time we have a contour, try to reassemble the segments on it to make chunks of
// the original(s) path(s)
// originals are in the orig array, whose size is nbP
void
Shape::ConvertToForme (Path * dest, int nbP, Path * *orig, bool splitWhenForced)
{
  if (numberOfPoints() <= 1 || numberOfEdges() <= 1)
    return;
//  if (Eulerian (true) == false)
//    return;
  
  if (_has_back_data == false)
  {
    ConvertToForme (dest);
    return;
  }
  
  dest->Reset ();
  
  MakePointData (true);
  MakeEdgeData (true);
  MakeSweepDestData (true);
  
  for (int i = 0; i < numberOfPoints(); i++)
  {
    pData[i].rx[0] = Round (getPoint(i).x[0]);
    pData[i].rx[1] = Round (getPoint(i).x[1]);
  }
  for (int i = 0; i < numberOfEdges(); i++)
  {
    eData[i].rdx = pData[getEdge(i).en].rx - pData[getEdge(i).st].rx;
  }
  
  SortEdges ();
  
  for (int i = 0; i < numberOfEdges(); i++)
  {
    swdData[i].misc = 0;
    swdData[i].precParc = swdData[i].suivParc = -1;
  }
  
  int searchInd = 0;
  
  int lastPtUsed = 0;
  do
  {
    int startBord = -1;
    {
      int fi = 0;
      for (fi = lastPtUsed; fi < numberOfPoints(); fi++)
      {
        if (getPoint(fi).incidentEdge[FIRST] >= 0 && swdData[getPoint(fi).incidentEdge[FIRST]].misc == 0)
          break;
      }
      lastPtUsed = fi + 1;
      if (fi < numberOfPoints())
      {
        int bestB = getPoint(fi).incidentEdge[FIRST];
        while (bestB >= 0 && getEdge(bestB).st != fi)
          bestB = NextAt (fi, bestB);
        if (bestB >= 0)
	      {
          startBord = bestB;
	      }
      }
    }
    if (startBord >= 0)
    {
      // parcours en profondeur pour mettre les leF et riF a leurs valeurs
      swdData[startBord].misc = (void *) 1;
      //printf("part de %d\n",startBord);
      int curBord = startBord;
      bool back = false;
      swdData[curBord].precParc = -1;
      swdData[curBord].suivParc = -1;
      int curStartPt=getEdge(curBord).st;
      do
	    {
	      int cPt = getEdge(curBord).en;
	      int nb = curBord;
        //printf("de curBord= %d au point %i  -> ",curBord,cPt);
	      do
        {
          int nnb = CycleNextAt (cPt, nb);
          if (nnb == nb)
          {
            // cul-de-sac
            nb = -1;
            break;
          }
          nb = nnb;
          if (nb < 0 || nb == curBord)
            break;
        }
	      while (swdData[nb].misc != 0 || getEdge(nb).st != cPt);
        
	      if (nb < 0 || nb == curBord)
        {
          if (back == false)
          {
            if (curBord == startBord || curBord < 0)
            {
              // probleme -> on vire le moveto
              //                                                      dest->descr_nb--;
            }
            else
            {
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
            }
            //                                              dest->Close();
          }
          back = true;
          // retour en arriere
          curBord = swdData[curBord].precParc;
          //printf("retour vers %d\n",curBord);
          if (curBord < 0)
            break;
        }
	      else
        {
          if (back)
          {
            back = false;
            startBord = nb;
            curStartPt=getEdge(nb).st;
          } else {
            if ( getEdge(curBord).en == curStartPt ) {
              //printf("contour %i ",curStartPt);
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
              startBord=nb;
            }
          }
          swdData[nb].misc = (void *) 1;
          swdData[nb].ind = searchInd++;
          swdData[nb].precParc = curBord;
          swdData[curBord].suivParc = nb;
          curBord = nb;
          //printf("suite %d\n",curBord);
        }
	    }
      while (1 /*swdData[curBord].precParc >= 0 */ );
      // fin du cas non-oriente
    }
  }
  while (lastPtUsed < numberOfPoints());
  
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepDestData (false);
}
void 
Shape::ConvertToFormeNested (Path * dest, int nbP, Path * *orig, int /*wildPath*/,int &nbNest,int *&nesting,int *&contStart,bool splitWhenForced)
{
  nesting=NULL;
  contStart=NULL;
  nbNest=0;

  if (numberOfPoints() <= 1 || numberOfEdges() <= 1)
    return;
  //  if (Eulerian (true) == false)
  //    return;
  
  if (_has_back_data == false)
  {
    ConvertToForme (dest);
    return;
  }
  
  dest->Reset ();
  
//  MakePointData (true);
  MakeEdgeData (true);
  MakeSweepDestData (true);
  
  for (int i = 0; i < numberOfPoints(); i++)
  {
    pData[i].rx[0] = Round (getPoint(i).x[0]);
    pData[i].rx[1] = Round (getPoint(i).x[1]);
  }
  for (int i = 0; i < numberOfEdges(); i++)
  {
    eData[i].rdx = pData[getEdge(i).en].rx - pData[getEdge(i).st].rx;
  }
  
  SortEdges ();
  
  for (int i = 0; i < numberOfEdges(); i++)
  {
    swdData[i].misc = 0;
    swdData[i].precParc = swdData[i].suivParc = -1;
  }
  
  int searchInd = 0;
  
  int lastPtUsed = 0;
  int parentContour=-1;
  do
  {
    int childEdge = -1;
    bool foundChild = false;
    int startBord = -1;
    {
      int fi = 0;
      for (fi = lastPtUsed; fi < numberOfPoints(); fi++)
      {
        if (getPoint(fi).incidentEdge[FIRST] >= 0 && swdData[getPoint(fi).incidentEdge[FIRST]].misc == 0)
          break;
      }
      {
        int askTo = pData[fi].askForWindingB;
        if (askTo < 0 || askTo >= numberOfEdges() ) {
          parentContour=-1;
        } else {
          if (getEdge(askTo).prevS >= 0) {
              parentContour = GPOINTER_TO_INT(swdData[askTo].misc);
              parentContour-=1; // pour compenser le decalage
          }
          childEdge = getPoint(fi).incidentEdge[FIRST];
        }
      }
      lastPtUsed = fi + 1;
      if (fi < numberOfPoints())
      {
        int bestB = getPoint(fi).incidentEdge[FIRST];
        while (bestB >= 0 && getEdge(bestB).st != fi)
          bestB = NextAt (fi, bestB);
        if (bestB >= 0)
	      {
          startBord = bestB;
	      }
      }
    }
    if (startBord >= 0)
    {
      // parcours en profondeur pour mettre les leF et riF a leurs valeurs
      swdData[startBord].misc = (void *)(intptr_t)(1 + nbNest);
      if (startBord == childEdge) {
          foundChild = true;
      }
      //printf("part de %d\n",startBord);
      int curBord = startBord;
      bool back = false;
      swdData[curBord].precParc = -1;
      swdData[curBord].suivParc = -1;
      int curStartPt=getEdge(curBord).st;
      do
	    {
	      int cPt = getEdge(curBord).en;
	      int nb = curBord;
        //printf("de curBord= %d au point %i  -> ",curBord,cPt);
	      do
        {
          int nnb = CycleNextAt (cPt, nb);
          if (nnb == nb)
          {
            // cul-de-sac
            nb = -1;
            break;
          }
          nb = nnb;
          if (nb < 0 || nb == curBord)
            break;
        }
	      while (swdData[nb].misc != 0 || getEdge(nb).st != cPt);
        
	      if (nb < 0 || nb == curBord)
        {
          if (back == false)
          {
            if (curBord == startBord || curBord < 0)
            {
              // probleme -> on vire le moveto
              //                                                      dest->descr_nb--;
            }
            else
            {
//              bool escapePath=false;
//              int tb=curBord;
//              while ( tb >= 0 && tb < numberOfEdges() ) {
//                if ( ebData[tb].pathID == wildPath ) {
//                  escapePath=true;
//                  break;
//                }
//                tb=swdData[tb].precParc;
//              }
              nesting=(int*)g_realloc(nesting,(nbNest+1)*sizeof(int));
              contStart=(int*)g_realloc(contStart,(nbNest+1)*sizeof(int));
              contStart[nbNest]=dest->descr_cmd.size();
              if (foundChild) {
                nesting[nbNest++]=parentContour;
                foundChild = false;
              } else {
                nesting[nbNest++]=-1; // contient des bouts de coupure -> a part
              }
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
            }
            //                                              dest->Close();
          }
          back = true;
          // retour en arriere
          curBord = swdData[curBord].precParc;
          //printf("retour vers %d\n",curBord);
          if (curBord < 0)
            break;
        }
	      else
        {
          if (back)
          {
            back = false;
            startBord = nb;
            curStartPt=getEdge(nb).st;
          } else {
            if ( getEdge(curBord).en == curStartPt ) {
              //printf("contour %i ",curStartPt);
              
//              bool escapePath=false;
//              int tb=curBord;
//              while ( tb >= 0 && tb < numberOfEdges() ) {
//                if ( ebData[tb].pathID == wildPath ) {
//                  escapePath=true;
//                  break;
//                }
//                tb=swdData[tb].precParc;
//              }
              nesting=(int*)g_realloc(nesting,(nbNest+1)*sizeof(int));
              contStart=(int*)g_realloc(contStart,(nbNest+1)*sizeof(int));
              contStart[nbNest]=dest->descr_cmd.size();
              if (foundChild) {
                nesting[nbNest++]=parentContour;
                foundChild = false;
              } else {
                nesting[nbNest++]=-1; // contient des bouts de coupure -> a part
              }
              swdData[curBord].suivParc = -1;
              AddContour (dest, nbP, orig, startBord, curBord,splitWhenForced);
              startBord=nb;
            }
          }
          swdData[nb].misc = (void *)(intptr_t)(1 + nbNest);
          swdData[nb].ind = searchInd++;
          swdData[nb].precParc = curBord;
          swdData[curBord].suivParc = nb;
          curBord = nb;
          if (nb == childEdge) {
              foundChild = true;
          }
          //printf("suite %d\n",curBord);
        }
	    }
      while (1 /*swdData[curBord].precParc >= 0 */ );
      // fin du cas non-oriente
    }
  }
  while (lastPtUsed < numberOfPoints());
  
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepDestData (false);
}


int
Shape::MakeTweak (int mode, Shape *a, double power, JoinType join, double miter, bool do_profile, Geom::Point c, Geom::Point vector, double radius, Geom::Affine *i2doc)
{
  Reset (0, 0);
  MakeBackData(a->_has_back_data);

	bool done_something = false;

  if (power == 0)
  {
    _pts = a->_pts;
    if (numberOfPoints() > maxPt)
    {
      maxPt = numberOfPoints();
      if (_has_points_data) {
        pData.resize(maxPt);
        _point_data_initialised = false;
        _bbox_up_to_date = false;
        }
    }
    
    _aretes = a->_aretes;
    if (numberOfEdges() > maxAr)
    {
      maxAr = numberOfEdges();
      if (_has_edges_data)
	      eData.resize(maxAr);
      if (_has_sweep_src_data)
        swsData.resize(maxAr);
      if (_has_sweep_dest_data)
        swdData.resize(maxAr);
      if (_has_raster_data)
        swrData.resize(maxAr);
      if (_has_back_data)
        ebData.resize(maxAr);
    }
    return 0;
  }
  if (a->numberOfPoints() <= 1 || a->numberOfEdges() <= 1 || a->type != shape_polygon)
    return shape_input_err;
  
  a->SortEdges ();
  
  a->MakeSweepDestData (true);
  a->MakeSweepSrcData (true);
  
  for (int i = 0; i < a->numberOfEdges(); i++)
  {
    int stB = -1, enB = -1;
    if (power <= 0 || mode == tweak_mode_push || mode == tweak_mode_repel || mode == tweak_mode_roughen)  {
      stB = a->CyclePrevAt (a->getEdge(i).st, i);
      enB = a->CycleNextAt (a->getEdge(i).en, i);
    } else {
      stB = a->CycleNextAt (a->getEdge(i).st, i);
      enB = a->CyclePrevAt (a->getEdge(i).en, i);
    }
    
    Geom::Point stD = a->getEdge(stB).dx;
    Geom::Point seD = a->getEdge(i).dx;
    Geom::Point enD = a->getEdge(enB).dx;

    double stL = sqrt (dot(stD,stD));
    double seL = sqrt (dot(seD,seD));
    //double enL = sqrt (dot(enD,enD));
    MiscNormalize (stD);
    MiscNormalize (enD);
    MiscNormalize (seD);
    
    Geom::Point ptP;
    int stNo, enNo;
    ptP = a->getPoint(a->getEdge(i).st).x;

  	Geom::Point to_center = ptP * (*i2doc) - c;
  	Geom::Point to_center_normalized = (1/Geom::L2(to_center)) * to_center;

		double this_power;
		if (do_profile && i2doc) {
			double alpha = 1;
			double x;
  		if (mode == tweak_mode_repel) {
				x = (Geom::L2(to_center)/radius);
			} else {
				x = (Geom::L2(ptP * (*i2doc) - c)/radius);
			}
			if (x > 1) {
				this_power = 0;
			} else if (x <= 0) {
    		if (mode == tweak_mode_repel) {
					this_power = 0;
				} else {
					this_power = power;
				}
			} else {
				this_power = power * (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5);
			}
		} else {
  		if (mode == tweak_mode_repel) {
				this_power = 0;
			} else {
				this_power = power;
			}
		}

		if (this_power != 0)
			done_something = true;

		double scaler = 1 / (*i2doc).descrim();

		Geom::Point this_vec(0,0);
    if (mode == tweak_mode_push) {
			Geom::Affine tovec (*i2doc);
			tovec[4] = tovec[5] = 0;
			tovec = tovec.inverse();
			this_vec = this_power * (vector * tovec) ;
		} else if (mode == tweak_mode_repel) {
			this_vec = this_power * scaler * to_center_normalized;
		} else if (mode == tweak_mode_roughen) {
  		double angle = g_random_double_range(0, 2*M_PI);
	  	this_vec = g_random_double_range(0, 1) * this_power * scaler * Geom::Point(sin(angle), cos(angle));
		}

    int   usePathID=-1;
    int   usePieceID=0;
    double useT=0.0;
    if ( a->_has_back_data ) {
      if ( a->ebData[i].pathID >= 0 && a->ebData[stB].pathID == a->ebData[i].pathID && a->ebData[stB].pieceID == a->ebData[i].pieceID
           && a->ebData[stB].tEn == a->ebData[i].tSt ) {
        usePathID=a->ebData[i].pathID;
        usePieceID=a->ebData[i].pieceID;
        useT=a->ebData[i].tSt;
      } else {
        usePathID=a->ebData[i].pathID;
        usePieceID=0;
        useT=0;
      }
    }

		if (mode == tweak_mode_push || mode == tweak_mode_repel || mode == tweak_mode_roughen) {
			Path::DoLeftJoin (this, 0, join, ptP+this_vec, stD+this_vec, seD+this_vec, miter, stL, seL,
												stNo, enNo,usePathID,usePieceID,useT);
			a->swsData[i].stPt = enNo;
			a->swsData[stB].enPt = stNo;
		} else {
			if (power > 0) {
				Path::DoRightJoin (this, this_power * scaler, join, ptP, stD, seD, miter, stL, seL,
													 stNo, enNo,usePathID,usePieceID,useT);
				a->swsData[i].stPt = enNo;
				a->swsData[stB].enPt = stNo;
			} else {
				Path::DoLeftJoin (this, -this_power * scaler, join, ptP, stD, seD, miter, stL, seL,
													stNo, enNo,usePathID,usePieceID,useT);
				a->swsData[i].stPt = enNo;
				a->swsData[stB].enPt = stNo;
			}
		}
  }

  if (power < 0 || mode == tweak_mode_push || mode == tweak_mode_repel || mode == tweak_mode_roughen)
  {
    for (int i = 0; i < numberOfEdges(); i++)
      Inverse (i);
  }

  if ( _has_back_data ) {
    for (int i = 0; i < a->numberOfEdges(); i++)
    {
      int nEd=AddEdge (a->swsData[i].stPt, a->swsData[i].enPt);
      ebData[nEd]=a->ebData[i];
    }
  } else {
    for (int i = 0; i < a->numberOfEdges(); i++)
    {
      AddEdge (a->swsData[i].stPt, a->swsData[i].enPt);
    }
  }

  a->MakeSweepSrcData (false);
  a->MakeSweepDestData (false);
  
  return (done_something? 0 : shape_nothing_to_do);
}


// offsets
// take each edge, offset it, and make joins with previous at edge start and next at edge end (previous and
// next being with respect to the clockwise order)
// you gotta be very careful with the join, as anything but the right one will fuck everything up
// see PathStroke.cpp for the "right" joins
int
Shape::MakeOffset (Shape * a, double dec, JoinType join, double miter, bool do_profile, double cx, double cy, double radius, Geom::Affine *i2doc)
{
  Reset (0, 0);
  MakeBackData(a->_has_back_data);

	bool done_something = false;
  
  if (dec == 0)
  {
    _pts = a->_pts;
    if (numberOfPoints() > maxPt)
    {
      maxPt = numberOfPoints();
      if (_has_points_data) {
        pData.resize(maxPt);
        _point_data_initialised = false;
        _bbox_up_to_date = false;
        }
    }
    
    _aretes = a->_aretes;
    if (numberOfEdges() > maxAr)
    {
      maxAr = numberOfEdges();
      if (_has_edges_data)
	eData.resize(maxAr);
      if (_has_sweep_src_data)
        swsData.resize(maxAr);
      if (_has_sweep_dest_data)
        swdData.resize(maxAr);
      if (_has_raster_data)
        swrData.resize(maxAr);
      if (_has_back_data)
        ebData.resize(maxAr);
    }
    return 0;
  }
  if (a->numberOfPoints() <= 1 || a->numberOfEdges() <= 1 || a->type != shape_polygon)
    return shape_input_err;
  
  a->SortEdges ();
  
  a->MakeSweepDestData (true);
  a->MakeSweepSrcData (true);
  
  for (int i = 0; i < a->numberOfEdges(); i++)
  {
    //              int    stP=a->swsData[i].stPt/*,enP=a->swsData[i].enPt*/;
    int stB = -1, enB = -1;
    if (dec > 0)
    {
      stB = a->CycleNextAt (a->getEdge(i).st, i);
      enB = a->CyclePrevAt (a->getEdge(i).en, i);
    }
    else
    {
      stB = a->CyclePrevAt (a->getEdge(i).st, i);
      enB = a->CycleNextAt (a->getEdge(i).en, i);
    }
    
    Geom::Point stD = a->getEdge(stB).dx;
    Geom::Point seD = a->getEdge(i).dx;
    Geom::Point enD = a->getEdge(enB).dx;

    double stL = sqrt (dot(stD,stD));
    double seL = sqrt (dot(seD,seD));
    //double enL = sqrt (dot(enD,enD));
    MiscNormalize (stD);
    MiscNormalize (enD);
    MiscNormalize (seD);
    
    Geom::Point ptP;
    int stNo, enNo;
    ptP = a->getPoint(a->getEdge(i).st).x;

		double this_dec;
		if (do_profile && i2doc) {
			double alpha = 1;
			double x = (Geom::L2(ptP * (*i2doc) - Geom::Point(cx,cy))/radius);
			if (x > 1) {
				this_dec = 0;
			} else if (x <= 0) {
				this_dec = dec;
			} else {
				this_dec = dec * (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5);
			}
		} else {
			this_dec = dec;
		}

		if (this_dec != 0)
			done_something = true;

    int   usePathID=-1;
    int   usePieceID=0;
    double useT=0.0;
    if ( a->_has_back_data ) {
      if ( a->ebData[i].pathID >= 0 && a->ebData[stB].pathID == a->ebData[i].pathID && a->ebData[stB].pieceID == a->ebData[i].pieceID
           && a->ebData[stB].tEn == a->ebData[i].tSt ) {
        usePathID=a->ebData[i].pathID;
        usePieceID=a->ebData[i].pieceID;
        useT=a->ebData[i].tSt;
      } else {
        usePathID=a->ebData[i].pathID;
        usePieceID=0;
        useT=0;
      }
    }
    if (dec > 0)
    {
      Path::DoRightJoin (this, this_dec, join, ptP, stD, seD, miter, stL, seL,
                         stNo, enNo,usePathID,usePieceID,useT);
      a->swsData[i].stPt = enNo;
      a->swsData[stB].enPt = stNo;
    }
    else
    {
      Path::DoLeftJoin (this, -this_dec, join, ptP, stD, seD, miter, stL, seL,
                        stNo, enNo,usePathID,usePieceID,useT);
      a->swsData[i].stPt = enNo;
      a->swsData[stB].enPt = stNo;
    }
  }

  if (dec < 0)
  {
    for (int i = 0; i < numberOfEdges(); i++)
      Inverse (i);
  }

  if ( _has_back_data ) {
    for (int i = 0; i < a->numberOfEdges(); i++)
    {
      int nEd=AddEdge (a->swsData[i].stPt, a->swsData[i].enPt);
      ebData[nEd]=a->ebData[i];
    }
  } else {
    for (int i = 0; i < a->numberOfEdges(); i++)
    {
      AddEdge (a->swsData[i].stPt, a->swsData[i].enPt);
    }
  }

  a->MakeSweepSrcData (false);
  a->MakeSweepDestData (false);
  
  return (done_something? 0 : shape_nothing_to_do);
}



// we found a contour, now reassemble the edges on it, instead of dumping them in the Path "dest" as a
// polyline. since it was a DFS, the precParc and suivParc make a nice doubly-linked list of the edges in
// the contour. the first and last edges of the contour are startBord and curBord
void
Shape::AddContour (Path * dest, int nbP, Path * *orig, int startBord, int curBord, bool splitWhenForced)
{
  int bord = startBord;
  
  {
    dest->MoveTo (getPoint(getEdge(bord).st).x);
  }
  
  while (bord >= 0)
  {
    int nPiece = ebData[bord].pieceID;
    int nPath = ebData[bord].pathID;
    
    if (nPath < 0 || nPath >= nbP || orig[nPath] == NULL)
    {
      // segment batard
      dest->LineTo (getPoint(getEdge(bord).en).x);
      bord = swdData[bord].suivParc;
    }
    else
    {
      Path *from = orig[nPath];
      if (nPiece < 0 || nPiece >= int(from->descr_cmd.size()))
	    {
	      // segment batard
	      dest->LineTo (getPoint(getEdge(bord).en).x);
	      bord = swdData[bord].suivParc;
	    }
      else
	    {
	      int nType = from->descr_cmd[nPiece]->getType();
	      if (nType == descr_close || nType == descr_moveto
            || nType == descr_forced)
        {
          // devrait pas arriver
          dest->LineTo (getPoint(getEdge(bord).en).x);
          bord = swdData[bord].suivParc;
        }
	      else if (nType == descr_lineto)
        {
          bord = ReFormeLineTo (bord, curBord, dest, from);
        }
	      else if (nType == descr_arcto)
        {
          bord = ReFormeArcTo (bord, curBord, dest, from);
        }
	      else if (nType == descr_cubicto)
        {
          bord = ReFormeCubicTo (bord, curBord, dest, from);
        }
	      else if (nType == descr_bezierto)
        {
          PathDescrBezierTo* nBData =
	    dynamic_cast<PathDescrBezierTo *>(from->descr_cmd[nPiece]);
	  
          if (nBData->nb == 0)
          {
            bord = ReFormeLineTo (bord, curBord, dest, from);
          }
          else
          {
            bord = ReFormeBezierTo (bord, curBord, dest, from);
          }
        }
	      else if (nType == descr_interm_bezier)
        {
          bord = ReFormeBezierTo (bord, curBord, dest, from);
        }
	      else
        {
          // devrait pas arriver non plus
          dest->LineTo (getPoint(getEdge(bord).en).x);
          bord = swdData[bord].suivParc;
        }
	      if (bord >= 0 && getPoint(getEdge(bord).st).totalDegree() > 2 ) {
          dest->ForcePoint ();
        } else if ( bord >= 0 && getPoint(getEdge(bord).st).oldDegree > 2 && getPoint(getEdge(bord).st).totalDegree() == 2)  {
          if ( splitWhenForced ) {
            // pour les coupures
            dest->ForcePoint ();
         } else {
            if ( _has_back_data ) {
              int   prevEdge=getPoint(getEdge(bord).st).incidentEdge[FIRST];
              int   nextEdge=getPoint(getEdge(bord).st).incidentEdge[LAST];
              if ( getEdge(prevEdge).en != getEdge(bord).st ) {
                int  swai=prevEdge;prevEdge=nextEdge;nextEdge=swai;
              }
              if ( ebData[prevEdge].pieceID == ebData[nextEdge].pieceID  && ebData[prevEdge].pathID == ebData[nextEdge].pathID ) {
                if ( fabs(ebData[prevEdge].tEn-ebData[nextEdge].tSt) < 0.05 ) {
                } else {
                  dest->ForcePoint ();
                }
              } else {
                dest->ForcePoint ();
              }
            } else {
              dest->ForcePoint ();
            }    
          }
        }
      }
    }
  }
  dest->Close ();
}

int
Shape::ReFormeLineTo (int bord, int /*curBord*/, Path * dest, Path * /*orig*/)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double /*ts=ebData[bord].tSt, */ te = ebData[bord].tEn;
  Geom::Point nx = getPoint(getEdge(bord).en).x;
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (getPoint(getEdge(bord).st).totalDegree() > 2
        || getPoint(getEdge(bord).st).oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pieceID == nPiece && ebData[bord].pathID == nPath)
    {
      if (fabs (te - ebData[bord].tSt) > 0.0001)
        break;
      nx = getPoint(getEdge(bord).en).x;
      te = ebData[bord].tEn;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }
  {
    dest->LineTo (nx);
  }
  return bord;
}

int
Shape::ReFormeArcTo (int bord, int /*curBord*/, Path * dest, Path * from)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double ts = ebData[bord].tSt, te = ebData[bord].tEn;
  //      double  px=pts[getEdge(bord).st].x,py=pts[getEdge(bord).st].y;
  Geom::Point nx = getPoint(getEdge(bord).en).x;
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (getPoint(getEdge(bord).st).totalDegree() > 2
        || getPoint(getEdge(bord).st).oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pieceID == nPiece && ebData[bord].pathID == nPath)
    {
      if (fabs (te - ebData[bord].tSt) > 0.0001)
	    {
	      break;
	    }
      nx = getPoint(getEdge(bord).en).x;
      te = ebData[bord].tEn;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }
  double sang, eang;
  PathDescrArcTo* nData = dynamic_cast<PathDescrArcTo *>(from->descr_cmd[nPiece]);
  bool nLarge = nData->large;
  bool nClockwise = nData->clockwise;
  Path::ArcAngles (from->PrevPoint (nPiece - 1), nData->p,nData->rx,nData->ry,nData->angle*M_PI/180.0, nLarge, nClockwise,  sang, eang);
  if (nClockwise)
  {
    if (sang < eang)
      sang += 2 * M_PI;
  }
  else
  {
    if (sang > eang)
      sang -= 2 * M_PI;
  }
  double delta = eang - sang;
  double ndelta = delta * (te - ts);
  if (ts > te)
    nClockwise = !nClockwise;
  if (ndelta < 0)
    ndelta = -ndelta;
  if (ndelta > M_PI)
    nLarge = true;
  else
    nLarge = false;
  /*	if ( delta < 0 ) delta=-delta;
	if ( ndelta < 0 ) ndelta=-ndelta;
	if ( ( delta < M_PI && ndelta < M_PI ) || ( delta >= M_PI && ndelta >= M_PI ) ) {
		if ( ts < te ) {
		} else {
			nClockwise=!(nClockwise);
		}
	} else {
    //		nLarge=!(nLarge);
		nLarge=false; // c'est un sous-segment -> l'arc ne peut que etre plus petit
		if ( ts < te ) {
		} else {
			nClockwise=!(nClockwise);
		}
	}*/
  {
    PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(from->descr_cmd[nPiece]);
    dest->ArcTo (nx, nData->rx,nData->ry,nData->angle, nLarge, nClockwise);
  }
  return bord;
}

int
Shape::ReFormeCubicTo (int bord, int /*curBord*/, Path * dest, Path * from)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double ts = ebData[bord].tSt, te = ebData[bord].tEn;
  Geom::Point nx = getPoint(getEdge(bord).en).x;
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (getPoint(getEdge(bord).st).totalDegree() > 2
        || getPoint(getEdge(bord).st).oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pieceID == nPiece && ebData[bord].pathID == nPath)
    {
      if (fabs (te - ebData[bord].tSt) > 0.0001)
	    {
	      break;
	    }
      nx = getPoint(getEdge(bord).en).x;
      te = ebData[bord].tEn;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }
  Geom::Point prevx = from->PrevPoint (nPiece - 1);
  
  Geom::Point sDx, eDx;
  {
    PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(from->descr_cmd[nPiece]);
    Path::CubicTangent (ts, sDx, prevx,nData->start,nData->p,nData->end);
    Path::CubicTangent (te, eDx, prevx,nData->start,nData->p,nData->end);
  }
  sDx *= (te - ts);
  eDx *= (te - ts);
  {
    dest->CubicTo (nx,sDx,eDx);
  }
  return bord;
}

int
Shape::ReFormeBezierTo (int bord, int /*curBord*/, Path * dest, Path * from)
{
  int nPiece = ebData[bord].pieceID;
  int nPath = ebData[bord].pathID;
  double ts = ebData[bord].tSt, te = ebData[bord].tEn;
  int ps = nPiece, pe = nPiece;
  Geom::Point px = getPoint(getEdge(bord).st).x;
  Geom::Point nx = getPoint(getEdge(bord).en).x;
  int inBezier = -1, nbInterm = -1;
  int typ;
  typ = from->descr_cmd[nPiece]->getType();
  PathDescrBezierTo *nBData = NULL;
  if (typ == descr_bezierto)
  {
    nBData = dynamic_cast<PathDescrBezierTo *>(from->descr_cmd[nPiece]);
    inBezier = nPiece;
    nbInterm = nBData->nb;
  }
  else
  {
    int n = nPiece - 1;
    while (n > 0)
    {
      typ = from->descr_cmd[n]->getType();
      if (typ == descr_bezierto)
      {
        inBezier = n;
        nBData = dynamic_cast<PathDescrBezierTo *>(from->descr_cmd[n]);
        nbInterm = nBData->nb;
        break;
      }
      n--;
    }
    if (inBezier < 0)
    {
      bord = swdData[bord].suivParc;
      dest->LineTo (nx);
      return bord;
    }
  }
  bord = swdData[bord].suivParc;
  while (bord >= 0)
  {
    if (getPoint(getEdge(bord).st).totalDegree() > 2
        || getPoint(getEdge(bord).st).oldDegree > 2)
    {
      break;
    }
    if (ebData[bord].pathID == nPath)
    {
      if (ebData[bord].pieceID < inBezier
          || ebData[bord].pieceID >= inBezier + nbInterm)
        break;
      if (ebData[bord].pieceID == pe
          && fabs (te - ebData[bord].tSt) > 0.0001)
        break;
      if (ebData[bord].pieceID != pe
          && (ebData[bord].tSt > 0.0001 && ebData[bord].tSt < 0.9999))
        break;
      if (ebData[bord].pieceID != pe && (te > 0.0001 && te < 0.9999))
        break;
      nx = getPoint(getEdge(bord).en).x;
      te = ebData[bord].tEn;
      pe = ebData[bord].pieceID;
    }
    else
    {
      break;
    }
    bord = swdData[bord].suivParc;
  }

  g_return_val_if_fail(nBData != NULL, 0);
  
  if (pe == ps)
  {
    ReFormeBezierChunk (px, nx, dest, inBezier, nbInterm, from, ps,
                        ts, te);
  }
  else if (ps < pe)
  {
    if (ts < 0.0001)
    {
      if (te > 0.9999)
      {
        dest->BezierTo (nx);
        for (int i = ps; i <= pe; i++)
        {
          PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i+1]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        Geom::Point tx;
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe+1]);
          tx = (pnData->p + psData->p) / 2;
        }
        dest->BezierTo (tx);
        for (int i = ps; i < pe; i++)
        {
          PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i+1]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 0.0, te);
      }
    }
    else
    {
      if (te > 0.9999)
      {
        Geom::Point tx;
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps+1]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps+2]);
          tx = (psData->p +  pnData->p) / 2;
        }
        ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 1.0);
        dest->BezierTo (nx);
        for (int i = ps + 1; i <= pe; i++)
        {
          PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i+1]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        Geom::Point tx;
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps+1]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps+2]);
          tx = (pnData->p + psData->p) / 2;
        }
        ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 1.0);
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe+1]);
          tx = (pnData->p + psData->p) / 2;
        }
         dest->BezierTo (tx);
        for (int i = ps + 1; i <= pe; i++)
        {
          PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i+1]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 0.0, te);
      }
    }
  }
  else
  {
    if (ts > 0.9999)
    {
      if (te < 0.0001)
      {
        dest->BezierTo (nx);
        for (int i = ps; i >= pe; i--)
        {
          PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i+1]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        Geom::Point tx;
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe+1]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe+2]);
          tx = (pnData->p + psData->p) / 2;
        }
        dest->BezierTo (tx);
        for (int i = ps; i > pe; i--)
        {
          PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i+1]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 1.0, te);
      }
    }
    else
    {
      if (te < 0.0001)
      {
        Geom::Point tx;
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps+1]);
          tx = (pnData->p + psData->p) / 2;
        }
         ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 0.0);
        dest->BezierTo (nx);
        for (int i = ps + 1; i >= pe; i--)
        {
          PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
      }
      else
      {
        Geom::Point tx;
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[ps+1]);
          tx = (pnData->p + psData->p) / 2;
        }
        ReFormeBezierChunk (px, tx, dest, inBezier, nbInterm,
                            from, ps, ts, 0.0);
        {
          PathDescrIntermBezierTo* psData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe+1]);
          PathDescrIntermBezierTo* pnData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[pe+2]);
          tx = (pnData->p + psData->p) / 2;
        }
        dest->BezierTo (tx);
        for (int i = ps + 1; i > pe; i--)
        {
          PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[i]);
          dest->IntermBezierTo (nData->p);
        }
        dest->EndBezierTo ();
        ReFormeBezierChunk (tx, nx, dest, inBezier, nbInterm,
                            from, pe, 1.0, te);
      }
    }
  }
  return bord;
}

void
Shape::ReFormeBezierChunk (Geom::Point px, Geom::Point nx,
                           Path * dest, int inBezier, int nbInterm,
                           Path * from, int p, double ts, double te)
{
  PathDescrBezierTo* nBData = dynamic_cast<PathDescrBezierTo*>(from->descr_cmd[inBezier]);
  Geom::Point bstx = from->PrevPoint (inBezier - 1);
  Geom::Point benx = nBData->p;
  
  Geom::Point mx;
  if (p == inBezier)
  {
    // premier bout
    if (nbInterm <= 1)
    {
      // seul bout de la spline
      PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[inBezier+1]);
      mx = nData->p;
    }
    else
    {
      // premier bout d'une spline qui en contient plusieurs
      PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[inBezier+1]);
      mx = nData->p;
      nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[inBezier+2]);
      benx = (nData->p + mx) / 2;
    }
  }
  else if (p == inBezier + nbInterm - 1)
  {
    // dernier bout
    // si nbInterm == 1, le cas a deja ete traite
    // donc dernier bout d'une spline qui en contient plusieurs
    PathDescrIntermBezierTo* nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[inBezier+nbInterm]);
    mx = nData->p;
    nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[inBezier+nbInterm-1]);
    bstx = (nData->p + mx) / 2;
  }
  else
  {
    // la spline contient forcément plusieurs bouts, et ce n'est ni le premier ni le dernier
    PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[p+1]);
    mx = nData->p;
    nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[p]);
    bstx = (nData->p + mx) / 2;
    nData = dynamic_cast<PathDescrIntermBezierTo*>(from->descr_cmd[p+2]);
    benx = (nData->p + mx) / 2;
  }
  Geom::Point cx;
  {
    Path::QuadraticPoint ((ts + te) / 2, cx, bstx, mx, benx);
  }
  cx = 2 * cx - (px + nx) / 2;
  {
    dest->BezierTo (nx);
    dest->IntermBezierTo (cx);
    dest->EndBezierTo ();
  }
}

#undef MiscNormalize

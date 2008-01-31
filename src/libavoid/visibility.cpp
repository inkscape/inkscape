/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include <algorithm>
#include <cfloat>

#include "libavoid/shape.h"
#include "libavoid/debug.h"
#include "libavoid/visibility.h"
#include "libavoid/vertices.h"
#include "libavoid/graph.h"
#include "libavoid/geometry.h"
#include "libavoid/router.h"

#include <math.h>

#ifdef LINEDEBUG
  #include "SDL_gfxPrimitives.h"
#endif

namespace Avoid {


void shapeVis(ShapeRef *shape)
{
    Router *router = shape->router();

    if ( !(router->InvisibilityGrph) )
    {
        // Clear shape from graph.
        shape->removeFromGraph();
    }

    VertInf *shapeBegin = shape->firstVert();
    VertInf *shapeEnd = shape->lastVert()->lstNext;

    VertInf *pointsBegin = NULL;
    if (router->IncludeEndpoints)
    {
        pointsBegin = router->vertices.connsBegin();
    }
    else
    {
        pointsBegin = router->vertices.shapesBegin();
    }

    for (VertInf *curr = shapeBegin; curr != shapeEnd; curr = curr->lstNext)
    {
        bool knownNew = true;

        db_printf("-- CONSIDERING --\n");
        curr->id.db_print();

        db_printf("\tFirst Half:\n");
        for (VertInf *j = pointsBegin ; j != curr; j = j->lstNext)
        {
            EdgeInf::checkEdgeVisibility(curr, j, knownNew);
        }

        db_printf("\tSecond Half:\n");
        VertInf *pointsEnd = router->vertices.end();
        for (VertInf *k = shapeEnd; k != pointsEnd; k = k->lstNext)
        {
            EdgeInf::checkEdgeVisibility(curr, k, knownNew);
        }
    }
}


void shapeVisSweep(ShapeRef *shape)
{
    Router *router = shape->router();

    if ( !(router->InvisibilityGrph) )
    {
        // Clear shape from graph.
        shape->removeFromGraph();
    }

    VertInf *startIter = shape->firstVert();
    VertInf *endIter = shape->lastVert()->lstNext;

    for (VertInf *i = startIter; i != endIter; i = i->lstNext)
    {
        vertexSweep(i);
    }
}


void vertexVisibility(VertInf *point, VertInf *partner, bool knownNew,
        const bool gen_contains)
{
    Router *router = point->_router;
    const VertID& pID = point->id;

    // Make sure we're only doing ptVis for endpoints.
    assert(!(pID.isShape));

    if ( !(router->InvisibilityGrph) )
    {
        point->removeFromGraph();
    }

    if (gen_contains && !(pID.isShape))
    {
        router->generateContains(point);
    }

    if (router->UseLeesAlgorithm)
    {
        vertexSweep(point);
    }
    else
    {
        VertInf *shapesEnd = router->vertices.end();
        for (VertInf *k = router->vertices.shapesBegin(); k != shapesEnd;
                k = k->lstNext)
        {
            EdgeInf::checkEdgeVisibility(point, k, knownNew);
        }
        if (router->IncludeEndpoints && partner)
        {
            EdgeInf::checkEdgeVisibility(point, partner, knownNew);
        }
    }
}


//============================================================================
//  SWEEP CODE
//

static VertInf *centerInf;
static Point centerPoint;
static VertID centerID;
static double centerAngle;


class PointPair
{
    public:
        PointPair(VertInf *inf)
            : vInf(inf)
        {
            double x = vInf->point.x - centerPoint.x;
            double y = vInf->point.y - centerPoint.y;

            angle = pos_to_angle(x, y);
        }
        bool operator==(const PointPair& rhs) const
        {
            if (vInf->id == rhs.vInf->id)
            {
                return true;
            }
            return false;
        }
        static double pos_to_angle(double x, double y)
        {
            double ang = atan(y / x);
            ang = (ang * 180) / M_PI;
            if (x < 0)
            {
                ang += 180;
            }
            else if (y < 0)
            {
                ang += 360;
            }
            return ang;
        }

        VertInf    *vInf;
        double     angle;
};


typedef std::list<PointPair > VertList;


class EdgePair
{
    public:
        EdgePair(VertInf *v1, VertInf *v2, double d, double a)
            : vInf1(v1), vInf2(v2), initdist(d), initangle(a)
        {
            currdist  = initdist;
            currangle = initangle;
        }
        bool operator<(const EdgePair& rhs) const
        {
            if (initdist == rhs.initdist)
            {
                // TODO: This is a bit of a hack, should be
                //       set by the call to the constructor.
                return dist(centerPoint, vInf2->point) <
                        dist(centerPoint, rhs.vInf2->point);
            }
            return (initdist < rhs.initdist);
        }
        bool operator==(const EdgePair& rhs) const
        {
            if (((vInf1->id == rhs.vInf1->id) &&
                        (vInf2->id == rhs.vInf2->id)) ||
                ((vInf1->id == rhs.vInf2->id) &&
                        (vInf2->id == rhs.vInf1->id)))
            {
                return true;
            }
            return false;
        }
        bool operator!=(const EdgePair& rhs) const
        {
            if (((vInf1->id == rhs.vInf1->id) &&
                        (vInf2->id == rhs.vInf2->id)) ||
                ((vInf1->id == rhs.vInf2->id) &&
                        (vInf2->id == rhs.vInf1->id)))
            {
                return false;
            }
            return true;
        }
        void SetObsAng(double a)
        {
            obsAngle = fmod(initangle - (a - 180), 360);

            //db_printf("SetObsAng: %.2f  (from init %.2f, a %.2f)\n",
            //      obsAngle, initangle, a);
        }

        VertInf *vInf1;
        VertInf *vInf2;
        double  initdist;
        double  initangle;
        double  currdist;
        double  currangle;
        double  obsAngle;
};

typedef std::set<EdgePair> EdgeSet;


static bool ppCompare(PointPair& pp1, PointPair& pp2)
{
    if (pp1.angle == pp2.angle)
    {
        // If the points are colinear, then order them in increasing
        // distance from the point we are sweeping around.
        return dist(centerPoint, pp1.vInf->point) <
                dist(centerPoint, pp2.vInf->point);
    }
    return pp1.angle < pp2.angle;
}


#define AHEAD    1
#define BEHIND  -1

class isBoundingShape
{
    public:
        // constructor remembers the value provided
        isBoundingShape(ShapeSet& set)
        : ss(set)
        { }
        // the following is an overloading of the function call operator
        bool operator () (const PointPair& pp)
        {
            if (pp.vInf->id.isShape &&
                    (ss.find(pp.vInf->id.objID) != ss.end()))
            {
                return true;
            }
            return false;
        }
    private:
        ShapeSet& ss;
};


static bool sweepVisible(EdgeSet& T, VertInf *currInf, VertInf *lastInf,
        bool lastVisible, double lastAngle, int *blocker)
{

    if (!lastInf || (lastAngle != centerAngle))
    {
        // Nothing before it on the current ray
        EdgeSet::iterator closestIt = T.begin();
        if (closestIt != T.end())
        {

            Point &e1 = (*closestIt).vInf1->point;
            Point &e2 = (*closestIt).vInf2->point;

            if (segmentIntersect(centerInf->point, currInf->point, e1, e2))
            {
                *blocker = (*closestIt).vInf1->id.objID;
                return false;
            }
        }
    }
    else
    {
        // There was another point before this on the ray (lastInf)
        if (!lastVisible)
        {
            *blocker = -1;
            return false;
        }
        else
        {
            // Check if there is an edge in T that blocks the ray
            // between lastInf and currInf.
            EdgeSet::iterator tfin = T.end();
            for (EdgeSet::iterator l = T.begin(); l != tfin; ++l)
            {
                Point &e1 = (*l).vInf1->point;
                Point &e2 = (*l).vInf2->point;

                if (segmentIntersect(lastInf->point, currInf->point, e1, e2))
                {
                    *blocker = (*l).vInf1->id.objID;
                    return false;
                }
            }
        }
    }
    return true;
}


void vertexSweep(VertInf *vert)
{
    Router *router = vert->_router;
    VertID& pID = vert->id;
    Point& pPoint = vert->point;

    centerInf = vert;
    centerID = pID;
    centerPoint = pPoint;
    Point centerPt = pPoint;
    centerAngle = -1;

    // List of shape (and maybe endpt) vertices, except p
    // Sort list, around
    VertList v;

    // Initialise the vertex list
    VertInf *beginVert = router->vertices.connsBegin();
    VertInf *endVert = router->vertices.end();
    for (VertInf *inf = beginVert; inf != endVert; inf = inf->lstNext)
    {
        if (inf->id == centerID)
        {
            // Don't include the center point
            continue;
        }

        if (inf->id.isShape)
        {
            // Add shape vertex
            v.push_back(inf);
        }
        else
        {
            if (router->IncludeEndpoints)
            {
                if (centerID.isShape)
                {
                    // Add endpoint vertex
                    v.push_back(inf);
                }
                else
                {
                    // Center is an endpoint, so only include the other
                    // endpoint from the matching connector.
                    VertID partnerID = VertID(centerID.objID, false,
                            (centerID.vn == 1) ? 2 : 1);
                    if (inf->id == partnerID)
                    {
                        v.push_back(inf);
                    }
                }
            }
        }
    }
    // TODO: This should be done with a sorted data type and insertion sort.
    v.sort(ppCompare);

    EdgeSet e;
    ShapeSet& ss = router->contains[centerID];

    // And edge to T that intersect the initial ray.
    VertInf *last = router->vertices.end();
    for (VertInf *k = router->vertices.shapesBegin(); k != last; )
    {
        VertID kID = k->id;
        if (!(centerID.isShape) && (ss.find(kID.objID) != ss.end()))
        {
            unsigned int shapeID = kID.objID;
            db_printf("Center is inside shape %u so ignore shape edges.\n",
                    shapeID);
            // One of the endpoints is inside this shape so ignore it.
            while ((k != last) && (k->id.objID == shapeID))
            {
                // And skip the other vertices from this shape.
                k = k->lstNext;
            }
            continue;
        }

        VertInf *kPrev = k->shPrev;
        if ((centerInf == k) || (centerInf == kPrev))
        {
            k = k->lstNext;
            continue;
        }

        Point xaxis(DBL_MAX, centerInf->point.y);

        if (segmentIntersect(centerInf->point, xaxis, kPrev->point, k->point))
        {
            double distance;
            if (vecDir(centerInf->point, xaxis, kPrev->point) == BEHIND)
            {
                distance = dist(centerInf->point, kPrev->point);
            }
            else
            {
                distance = dist(centerInf->point, k->point);
            }

            EdgePair intPair = EdgePair(k, kPrev, distance, 0.0);
            e.insert(intPair).first;
        }
        k = k->lstNext;
    }

    // Start the actual sweep.
    db_printf("SWEEP: "); centerID.db_print(); db_printf("\n");

    VertInf *lastInf     = NULL;
    double   lastAngle   = 0;
    bool     lastVisible = false;
    int      lastBlocker = 0;

    isBoundingShape isBounding(router->contains[centerID]);
    VertList::iterator vfst = v.begin();
    VertList::iterator vfin = v.end();
    for (VertList::iterator t = vfst; t != vfin; ++t)
    {
        VertInf *currInf = (*t).vInf;
        VertID& currID = currInf->id;
        Point&  currPt = currInf->point;
        centerAngle = (*t).angle;

#ifdef LINEDEBUG
        Sint16 ppx = (int) centerPt.x;
        Sint16 ppy = (int) centerPt.y;

        Sint16 cx = (int) currPt.x;
        Sint16 cy = (int) currPt.y;
#endif

        double currDist = dist(centerPt, currPt);
        db_printf("Dist: %.1f.\n", currDist);

        EdgeInf *edge = EdgeInf::existingEdge(centerInf, currInf);
        if (edge == NULL)
        {
            edge = new EdgeInf(centerInf, currInf);
        }
        // Ignore vertices from bounding shapes, if sweeping round an endpoint.
        if (!(centerID.isShape) && isBounding(*t))
        {
            if (router->InvisibilityGrph)
            {
                // if p and t can't see each other, add blank edge
                db_printf("\tSkipping visibility edge... \n\t\t");
                edge->addBlocker(currInf->id.objID);
                edge->db_print();
            }
            continue;
        }


        bool cone1 = true, cone2 = true;
        if (centerID.isShape)
        {
            cone1 = inValidRegion(router->IgnoreRegions,
                    centerInf->shPrev->point, centerPoint,
                    centerInf->shNext->point, currInf->point);
        }
        if (currInf->id.isShape)
        {
            cone2 = inValidRegion(router->IgnoreRegions,
                    currInf->shPrev->point, currInf->point,
                    currInf->shNext->point, centerPoint);
        }

        if (!cone1 || !cone2)
        {
            lastInf = NULL;
            if (router->InvisibilityGrph)
            {
                db_printf("\tSetting invisibility edge... \n\t\t");
                edge->addBlocker(0);
                edge->db_print();
            }
        }
        else
        {
            int blocker = 0;
            // Check visibility.
            bool currVisible = sweepVisible(e, currInf,
                    lastInf, lastVisible, lastAngle, &blocker);
            if (blocker == -1)
            {
                blocker = lastBlocker;
            }
            if (currVisible)
            {
#ifdef LINEDEBUG
                lineRGBA(avoid_screen, ppx, ppy, cx, cy, 255, 0, 0, 32);
#endif
                db_printf("\tSetting visibility edge... \n\t\t");
                edge->setDist(currDist);
                edge->db_print();
            }
            else if (router->InvisibilityGrph)
            {
                db_printf("\tSetting invisibility edge... \n\t\t");
                edge->addBlocker(blocker);
                edge->db_print();
            }

            lastVisible = currVisible;
            lastInf     = currInf;
            lastAngle   = centerAngle;
            lastBlocker = blocker;
        }

        if (currID.isShape)
        {
            // This is a shape edge
            Point& prevPt = currInf->shPrev->point;
            Point& nextPt = currInf->shNext->point;

            int prevDir = vecDir(centerPt, currPt, prevPt);
            EdgePair prevPair = EdgePair(currInf, currInf->shPrev,
                    currDist, centerAngle);

            EdgeSet::iterator ePtr;
            if (prevDir == BEHIND)
            {
                // XXX: Strangely e.find does not return the correct results.
                // ePtr = e.find(prevPair);
                ePtr = std::find(e.begin(), e.end(), prevPair);
                if (ePtr != e.end())
                {
                    e.erase(ePtr);
                }
            }
            else if ((prevDir == AHEAD) && (currInf->shPrev != centerInf))
            {
                double x = prevPt.x - currPt.x;
                double y = prevPt.y - currPt.y;
                double angle = PointPair::pos_to_angle(x, y);
                prevPair.SetObsAng(angle);

                ePtr = e.insert(prevPair).first;
            }


            int nextDir = vecDir(centerPt, currPt, nextPt);
            EdgePair nextPair = EdgePair(currInf, currInf->shNext,
                    currDist, centerAngle);

            if (nextDir == BEHIND)
            {
                // XXX: Strangely e.find does not return the correct results.
                // ePtr = e.find(nextPair);
                ePtr = std::find(e.begin(), e.end(), nextPair);
                if (ePtr != e.end())
                {
                    e.erase(ePtr);
                }
            }
            else if ((nextDir == AHEAD) && (currInf->shNext != centerInf))
            {
                double x = nextPt.x - currPt.x;
                double y = nextPt.y - currPt.y;
                double angle = PointPair::pos_to_angle(x, y);
                nextPair.SetObsAng(angle);

                ePtr = e.insert(nextPair).first;
            }
        }

#ifdef LINEDEBUG
        SDL_Flip(avoid_screen);
#endif
    }
}


}


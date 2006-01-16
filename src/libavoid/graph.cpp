/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2005  Michael Wybrow <mjwybrow@users.sourceforge.net>
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

#include "libavoid/debug.h"
#include "libavoid/graph.h"
#include "libavoid/connector.h"
#include "libavoid/polyutil.h"
#include "libavoid/timer.h"

#include <math.h>

namespace Avoid {


static int st_checked_edges = 0;

EdgeList visGraph;
EdgeList invisGraph;


EdgeInf::EdgeInf(VertInf *v1, VertInf *v2)
    : lstPrev(NULL)
    , lstNext(NULL)
    , _added(false)
    , _visible(false)
    , _v1(v1)
    , _v2(v2)
    , _dist(-1)
{
    _blockers.clear();
    _conns.clear();
}


EdgeInf::~EdgeInf()
{
    if (_added)
    {
        makeInactive();
    }
}


void EdgeInf::makeActive(void)
{
    assert(_added == false);

    if (_visible)
    {
        visGraph.addEdge(this);
        _pos1 = _v1->visList.insert(_v1->visList.begin(), this);
        _v1->visListSize++;
        _pos2 = _v2->visList.insert(_v2->visList.begin(), this);
        _v2->visListSize++;
    }
    else // if (invisible)
    {
        invisGraph.addEdge(this);
        _pos1 = _v1->invisList.insert(_v1->invisList.begin(), this);
        _v1->invisListSize++;
        _pos2 = _v2->invisList.insert(_v2->invisList.begin(), this);
        _v2->invisListSize++;
    }
    _added = true;
}


void EdgeInf::makeInactive(void)
{
    assert(_added == true);

    if (_visible)
    {
        visGraph.removeEdge(this);
        _v1->visList.erase(_pos1);
        _v1->visListSize--;
        _v2->visList.erase(_pos2);
        _v2->visListSize--;
    }
    else // if (invisible)
    {
        invisGraph.removeEdge(this);
        _v1->invisList.erase(_pos1);
        _v1->invisListSize--;
        _v2->invisList.erase(_pos2);
        _v2->invisListSize--;
    }
    _blockers.clear();
    _conns.clear();
    _added = false;
}


double EdgeInf::getDist(void)
{
    return _dist;
}


void EdgeInf::setDist(double dist)
{
    //assert(dist != 0);

    if (_added && !_visible)
    {
        makeInactive();
    }
    if (!_added)
    {
        _visible = true;
        makeActive();
    }
    _dist = dist;
    _blockers.clear();
}


void EdgeInf::alertConns(void)
{
    for (FlagList::iterator i = _conns.begin(); i != _conns.end(); ++i)
    {
        *(*i) = true;
    }
    _conns.clear();
}


void EdgeInf::addConn(bool *flag)
{
    _conns.push_back(flag);
}


void EdgeInf::addCycleBlocker(void)
{
    // Needs to be in invisibility graph.
    addBlocker(-1);
}


void EdgeInf::addBlocker(int b)
{
    assert(InvisibilityGrph);

    if (_added && _visible)
    {
        makeInactive();
    }
    if (!_added)
    {
        _visible = false;
        makeActive();
    }
    _dist = 0;
    _blockers.clear();
    _blockers.push_back(b);
}


bool EdgeInf::hasBlocker(int b)
{
    assert(InvisibilityGrph);

    ShapeList::iterator finish = _blockers.end();
    for (ShapeList::iterator it = _blockers.begin(); it != finish; ++it)
    {
        if ((*it) == -1)
        {
            alertConns();
            return true;
        }
        else if ((*it) == b)
        {
            return true;
        }
    }
    return false;
}


pair<VertID, VertID> EdgeInf::ids(void)
{
    return std::make_pair(_v1->id, _v2->id);
}


pair<Point, Point> EdgeInf::points(void)
{
    return std::make_pair(_v1->point, _v2->point);
}


void EdgeInf::db_print(void)
{
    db_printf("Edge(");
    _v1->id.db_print();
    db_printf(",");
    _v2->id.db_print();
    db_printf(")\n");
}


void EdgeInf::checkVis(void)
{
    if (_added && !_visible)
    {
        db_printf("\tChecking visibility for existing invisibility edge..."
                "\n\t\t");
        db_print();
    }
    else if (_added && _visible)
    {
        db_printf("\tChecking visibility for existing visibility edge..."
                "\n\t\t");
        db_print();
    }

    int blocker = 0;
    bool cone1 = true;
    bool cone2 = true;

    VertInf *i = _v1;
    VertInf *j = _v2;
    const VertID& iID = i->id;
    const VertID& jID = j->id;
    const Point& iPoint = i->point;
    const Point& jPoint = j->point;

    st_checked_edges++;

    if (iID.isShape)
    {
        cone1 = inValidRegion(i->shPrev->point, iPoint, i->shNext->point,
                jPoint);
    }
    else
    {
        ShapeSet& ss = contains[iID];

        if ((jID.isShape) && (ss.find(jID.objID) != ss.end()))
        {
            db_printf("1: Edge of bounding shape\n");
            // Don't even check this edge, it should be zero,
            // since a point in a shape can't see it's corners
            cone1 = false;
        }
    }

    if (cone1)
    {
        // If outside the first cone, don't even bother checking.
        if (jID.isShape)
        {
            cone2 = inValidRegion(j->shPrev->point, jPoint, j->shNext->point,
                    iPoint);
        }
        else
        {
            ShapeSet& ss = contains[jID];

            if ((iID.isShape) && (ss.find(iID.objID) != ss.end()))
            {
                db_printf("2: Edge of bounding shape\n");
                // Don't even check this edge, it should be zero,
                // since a point in a shape can't see it's corners
                cone2 = false;
            }
        }
    }

    if (cone1 && cone2 && ((blocker = firstBlocker()) == 0))
    {

        // if i and j see each other, add edge
        db_printf("\tSetting visibility edge... \n\t\t");
        db_print();

        double d = dist(iPoint, jPoint);

        setDist(d);

    }
    else if (InvisibilityGrph)
    {
#if 0
        db_printf("%d, %d, %d\n", cone1, cone2, blocker);
        db_printf("\t(%d, %d)--(%d, %d)\n", (int) iInfo.point.x,
                (int) iInfo.point.y, (int) jInfo.point.x,
                (int) jInfo.point.y);
#endif

        // if i and j can't see each other, add blank edge
        db_printf("\tSetting invisibility edge... \n\t\t");
        db_print();
        addBlocker(blocker);
    }
}


int EdgeInf::firstBlocker(void)
{
    ShapeSet ss = ShapeSet();

    Point& pti = _v1->point;
    Point& ptj = _v2->point;
    VertID& iID = _v1->id;
    VertID& jID = _v2->id;

    if (!(iID.isShape))
    {
        ss.insert(contains[iID].begin(), contains[iID].end());
    }
    if (!(jID.isShape))
    {
        ss.insert(contains[jID].begin(), contains[jID].end());
    }

    VertInf *last = vertices.end();
    for (VertInf *k = vertices.shapesBegin(); k != last; )
    {
        VertID kID = k->id;
        if ((ss.find(kID.objID) != ss.end()))
        {
            uint shapeID = kID.objID;
            db_printf("Endpoint is inside shape %u so ignore shape edges.\n",
                    kID.objID);
            // One of the endpoints is inside this shape so ignore it.
            while ((k != last) && (k->id.objID == shapeID))
            {
                // And skip the other vertices from this shape.
                k = k->lstNext;
            }
            continue;
        }
        Point& kPoint = k->point;
        Point& kPrevPoint = k->shPrev->point;

        if (segmentIntersect(pti, ptj, kPrevPoint, kPoint))
        {
            ss.clear();
            return kID.objID;
        }
        k = k->lstNext;
    }
    ss.clear();
    return 0;
}


bool EdgeInf::isBetween(VertInf *i, VertInf *j)
{
    if ( ((i == _v1) && (j == _v2)) ||
         ((i == _v2) && (j == _v1)) )
    {
        return true;
    }
    return false;
}


VertInf *EdgeInf::otherVert(VertInf *vert)
{
    assert((vert == _v1) || (vert == _v2));

    if (vert == _v1)
    {
        return _v2;
    }
    return _v1;
}


EdgeInf *EdgeInf::checkEdgeVisibility(VertInf *i, VertInf *j, bool knownNew)
{
    EdgeInf *edge = NULL;

    if (knownNew)
    {
        assert(existingEdge(i, j) == NULL);
        edge = new EdgeInf(i, j);
    }
    else
    {
        edge = existingEdge(i, j);
        if (edge == NULL)
        {
            edge = new EdgeInf(i, j);
        }
    }
    edge->checkVis();
    if (!(edge->_added) && !InvisibilityGrph)
    {
        delete edge;
        edge = NULL;
    }

    return edge;
}


EdgeInf *EdgeInf::existingEdge(VertInf *i, VertInf *j)
{
    VertInf *selected = NULL;

    if (i->visListSize <= j->visListSize)
    {
        selected = i;
    }
    else
    {
        selected = j;
    }

    EdgeInfList& visList = selected->visList;
    EdgeInfList::iterator finish = visList.end();
    for (EdgeInfList::iterator edge = visList.begin(); edge != finish;
            ++edge)
    {
        if ((*edge)->isBetween(i, j))
        {
            return (*edge);
        }
    }

    if (i->invisListSize <= j->invisListSize)
    {
        selected = i;
    }
    else
    {
        selected = j;
    }

    EdgeInfList& invisList = selected->invisList;
    finish = invisList.end();
    for (EdgeInfList::iterator edge = invisList.begin(); edge != finish;
            ++edge)
    {
        if ((*edge)->isBetween(i, j))
        {
            return (*edge);
        }
    }

    return NULL;
}


//===========================================================================


EdgeList::EdgeList()
    : _firstEdge(NULL)
    , _lastEdge(NULL)
    , _count(0)
{
}


void EdgeList::addEdge(EdgeInf *edge)
{
    if (_firstEdge == NULL)
    {
        assert(_lastEdge == NULL);

        _lastEdge = edge;
        _firstEdge = edge;

        edge->lstPrev = NULL;
        edge->lstNext = NULL;
    }
    else
    {
        assert(_lastEdge != NULL);

        _lastEdge->lstNext = edge;
        edge->lstPrev = _lastEdge;

        _lastEdge = edge;

        edge->lstNext = NULL;
    }
    _count++;
}


void EdgeList::removeEdge(EdgeInf *edge)
{
    if (edge->lstPrev)
    {
        edge->lstPrev->lstNext = edge->lstNext;
    }
    if (edge->lstNext)
    {
        edge->lstNext->lstPrev = edge->lstPrev;
    }
    if (edge == _lastEdge)
    {
        _lastEdge = edge->lstPrev;
        if (edge == _firstEdge)
        {
            _firstEdge = NULL;
        }
    }
    else if (edge == _firstEdge)
    {
        _firstEdge = edge->lstNext;
    }


    edge->lstPrev = NULL;
    edge->lstNext = NULL;

    _count--;
}


EdgeInf *EdgeList::begin(void)
{
    return _firstEdge;
}


EdgeInf *EdgeList::end(void)
{
    return NULL;
}


// General visibility graph utility functions


void newBlockingShape(Polygn *poly, int pid)
{
    // o  Check all visibility edges to see if this one shape
    //    blocks them.
    EdgeInf *finish = visGraph.end();
    for (EdgeInf *iter = visGraph.begin(); iter != finish ; )
    {
        EdgeInf *tmp = iter;
        iter = iter->lstNext;

        if (tmp->getDist() != 0)
        {
            pair<VertID, VertID> ids(tmp->ids());
            VertID eID1 = ids.first;
            VertID eID2 = ids.second;
            pair<Point, Point> points(tmp->points());
            Point e1 = points.first;
            Point e2 = points.second;
            bool blocked = false;

            bool ep_in_poly1 = !(eID1.isShape) ? inPoly(*poly, e1) : false;
            bool ep_in_poly2 = !(eID2.isShape) ? inPoly(*poly, e2) : false;
            if (ep_in_poly1 || ep_in_poly2)
            {
                // Don't check edges that have a connector endpoint
                // and are inside the shape being added.
                continue;
            }

            for (int pt_i = 0; pt_i < poly->pn; pt_i++)
            {
                int pt_n = (pt_i == (poly->pn - 1)) ? 0 : pt_i + 1;
                if (segmentIntersect(e1, e2, poly->ps[pt_i], poly->ps[pt_n]))
                {
                    blocked = true;
                    break;
                }
            }
            if (blocked)
            {
                db_printf("\tRemoving newly blocked edge (by shape %3d)"
                        "... \n\t\t", pid);
                tmp->alertConns();
                tmp->db_print();
                if (InvisibilityGrph)
                {
                    tmp->addBlocker(pid);
                }
                else
                {
                    delete tmp;
                }
            }
        }
    }
}


void checkAllBlockedEdges(int pid)
{
    assert(InvisibilityGrph);

    for (EdgeInf *iter = invisGraph.begin(); iter != invisGraph.end() ; )
    {
        EdgeInf *tmp = iter;
        iter = iter->lstNext;

        if (tmp->hasBlocker(pid))
        {
            tmp->checkVis();
        }
    }
}


void checkAllMissingEdges(void)
{
    assert(!InvisibilityGrph);

    VertInf *first = NULL;

    if (IncludeEndpoints)
    {
        first = vertices.connsBegin();
    }
    else
    {
        first = vertices.shapesBegin();
    }

    VertInf *pend = vertices.end();
    for (VertInf *i = first; i != pend; i = i->lstNext)
    {
        VertID iID = i->id;

        // Check remaining, earlier vertices
        for (VertInf *j = first ; j != i; j = j->lstNext)
        {
            VertID jID = j->id;
            if (!(iID.isShape) && (iID.objID != jID.objID))
            {
                // Don't keep visibility between edges of different conns
                continue;
            }

            // See if the edge is already there?
            bool found = (EdgeInf::existingEdge(i, j) != NULL);

            if (!found)
            {
                // Didn't already exist, check.
                bool knownNew = true;
                EdgeInf::checkEdgeVisibility(i, j, knownNew);
            }
        }
    }
}


void generateContains(VertInf *pt)
{
    contains[pt->id].clear();

    ShapeRefList::iterator finish = shapeRefs.end();
    for (ShapeRefList::iterator i = shapeRefs.begin(); i != finish; ++i)
    {
        Polygn poly = copyPoly(*i);
        if (inPoly(poly, pt->point))
        {
            contains[pt->id].insert((*i)->id());
        }
        freePoly(poly);
    }
}


void adjustContainsWithAdd(const Polygn& poly, const int p_shape)
{
    for (VertInf *k = vertices.connsBegin(); k != vertices.shapesBegin();
            k = k->lstNext)
    {
        if (inPoly(poly, k->point))
        {
            contains[k->id].insert(p_shape);
        }
    }
}


void adjustContainsWithDel(const int p_shape)
{
    for (VertInf *k = vertices.connsBegin(); k != vertices.shapesBegin();
            k = k->lstNext)
    {
        contains[k->id].erase(p_shape);
    }
}


// Maybe this one should be in with the connector stuff, but it may later
// need to operate on a particular section of the visibility graph so it
// may have to stay here.
//
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))

#ifdef SELECTIVE_DEBUG
static double AngleAFromThreeSides(const double a, const double b,
        const double c)
{
    // returns angle A, the angle opposite from side a, in radians
    return acos((pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c));
}
#endif

void markConnectors(ShapeRef *shape)
{
    assert(SelectiveReroute);

    ConnRefList::iterator end = connRefs.end();
    for (ConnRefList::iterator it = connRefs.begin(); it != end; ++it)
    {
        ConnRef *conn = (*it);

        if (conn->_route.pn == 0)
        {
            // Ignore uninitialised connectors.
            continue;
        }

        Point start = conn->_route.ps[0];
        Point end = conn->_route.ps[conn->_route.pn - 1];

        double conndist = conn->_route_dist;

        double estdist;
        double e1, e2;

        VertInf *beginV = shape->firstVert();
        VertInf *endV = shape->lastVert()->lstNext;
        for (VertInf *i = beginV; i != endV; i = i->lstNext)
        {
            const Point& p1 = i->point;
            const Point& p2 = i->shNext->point;

            double offy;
            double a;
            double b;
            double c;
            double d;

            double min;
            double max;

            if (p1.y == p2.y)
            {
                // Standard case
                offy = p1.y;
                a = start.x;
                b = start.y - offy;
                c = end.x;
                d = end.y - offy;

                min = MIN(p1.x, p2.x);
                max = MAX(p1.x, p2.x);
            }
            else if (p1.x == p2.x)
            {
                // Other Standard case
                offy = p1.x;
                a = start.y;
                b = start.x - offy;
                c = end.y;
                d = end.x - offy;

                min = MIN(p1.y, p2.y);
                max = MAX(p1.y, p2.y);
            }
            else
            {
                // Need to do rotation
                Point n_p2 = { p2.x - p1.x, p2.y - p1.y };
                Point n_start = { start.x - p1.x, start.y - p1.y };
                Point n_end = { end.x - p1.x, end.y - p1.y };
                //printf("n_p2:    (%.1f, %.1f)\n", n_p2.x, n_p2.y);
                //printf("n_start: (%.1f, %.1f)\n", n_start.x, n_start.y);
                //printf("n_end:   (%.1f, %.1f)\n", n_end.x, n_end.y);

                double theta = 0 - atan2(n_p2.y, n_p2.x);
                //printf("theta = %.2f\n", theta * (180 / PI));

                Point r_p1 = {0, 0};
                Point r_p2 = n_p2;
                start = n_start;
                end = n_end;

                double cosv = cos(theta);
                double sinv = sin(theta);

                r_p2.x = cosv * n_p2.x - sinv * n_p2.y;
                r_p2.y = cosv * n_p2.y + sinv * n_p2.x;
                start.x = cosv * n_start.x - sinv * n_start.y;
                start.y = cosv * n_start.y + sinv * n_start.x;
                end.x = cosv * n_end.x - sinv * n_end.y;
                end.y = cosv * n_end.y + sinv * n_end.x;
                //printf("r_p2:    (%.1f, %.1f)\n", r_p2.x, r_p2.y);
                //printf("r_start: (%.1f, %.1f)\n", start.x, start.y);
                //printf("r_end:   (%.1f, %.1f)\n", end.x, end.y);

                if (((int) r_p2.y) != 0)
                {
                    printf("r_p2.y: %f != 0\n", r_p2.y);
                    abort();
                }
                // This might be slightly off.
                r_p2.y = 0;

                offy = r_p1.y;
                a = start.x;
                b = start.y - offy;
                c = end.x;
                d = end.y - offy;

                min = MIN(r_p1.x, r_p2.x);
                max = MAX(r_p1.x, r_p2.x);

            }

            double x;
            if ((b + d) == 0)
            {
                db_printf("WARNING: (b + d) == 0\n");
                d = d * -1;
            }

            if ((b == 0) && (d == 0))
            {
                db_printf("WARNING: b == d == 0\n");
                if (((a < min) && (c < min)) ||
                        ((a > max) && (c > max)))
                {
                    // It's going to get adjusted.
                    x = a;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                x = ((b*c) + (a*d)) / (b + d);
            }

            //printf("%.1f, %.1f, %.1f, %.1f\n", a, b, c, d);
            //printf("x = %.1f\n", x);

            // XXX: Use MAX and MIN
            x = (x < min) ? min : x;
            x = (x > max) ? max : x;

            //printf("x = %.1f\n", x);

            Point xp;
            if (p1.x == p2.x)
            {
                xp.x = offy;
                xp.y = x;
            }
            else
            {
                xp.x = x;
                xp.y = offy;
            }
            //printf("(%.1f, %.1f)\n", xp.x, xp.y);

            e1 = dist(start, xp);
            e2 = dist(xp, end);
            estdist = e1 + e2;


            //printf("is %.1f < %.1f\n", estdist, conndist);
            if (estdist < conndist)
            {
#ifdef SELECTIVE_DEBUG
                //double angle = AngleAFromThreeSides(dist(start, end),
                //        e1, e2);
                printf("[%3d] - Possible better path found (%.1f < %.1f)\n",
                        conn->_id, estdist, conndist);
#endif
                conn->_needs_reroute_flag = true;
                break;
            }

        }
    }
}


void printInfo(void)
{
    FILE *fp = stdout;
    fprintf(fp, "\nVisibility Graph info:\n");
    fprintf(fp, "----------------------\n");

    uint currshape = 0;
    int st_shapes = 0;
    int st_vertices = 0;
    int st_endpoints = 0;
    int st_valid_shape_visedges = 0;
    int st_valid_endpt_visedges = 0;
    int st_invalid_visedges = 0;
    VertInf *finish = vertices.end();
    for (VertInf *t = vertices.connsBegin(); t != finish; t = t->lstNext)
    {
        VertID pID = t->id;

        if ((pID.isShape) && (pID.objID != currshape))
        {
            currshape = pID.objID;
            st_shapes++;
        }
        if (pID.isShape)
        {
            st_vertices++;
        }
        else
        {
            // The shape 0 ones are temporary and not considered.
            st_endpoints++;
        }
    }
    for (EdgeInf *t = visGraph.begin(); t != visGraph.end();
            t = t->lstNext)
    {
        std::pair<VertID, VertID> idpair = t->ids();

        if (!(idpair.first.isShape) || !(idpair.second.isShape))
        {
            st_valid_endpt_visedges++;
        }
        else
        {
            st_valid_shape_visedges++;
        }
    }
    for (EdgeInf *t = invisGraph.begin(); t != invisGraph.end();
            t = t->lstNext)
    {
        st_invalid_visedges++;
    }
    fprintf(fp, "Number of shapes: %d\n", st_shapes);
    fprintf(fp, "Number of vertices: %d (%d real, %d endpoints)\n",
            st_vertices + st_endpoints, st_vertices, st_endpoints);
    fprintf(fp, "Number of vis_edges: %d (%d valid [%d normal, %d endpt], "
            "%d invalid)\n", st_valid_shape_visedges + st_invalid_visedges +
            st_valid_endpt_visedges, st_valid_shape_visedges +
            st_valid_endpt_visedges, st_valid_shape_visedges,
            st_valid_endpt_visedges, st_invalid_visedges);
    fprintf(fp, "----------------------\n");
    fprintf(fp, "checkVisEdge tally: %d\n", st_checked_edges);
    fprintf(fp, "----------------------\n");

    fprintf(fp, "ADDS:  "); timers.Print(tmAdd);
    fprintf(fp, "DELS:  "); timers.Print(tmDel);
    fprintf(fp, "MOVS:  "); timers.Print(tmMov);
    fprintf(fp, "***S:  "); timers.Print(tmSev);
    fprintf(fp, "PTHS:  "); timers.Print(tmPth);
    fprintf(fp, "\n");
}


}



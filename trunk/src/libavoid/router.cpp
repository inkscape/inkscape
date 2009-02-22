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

#include <cstdlib>
#include "libavoid/shape.h"
#include "libavoid/router.h"
#include "libavoid/visibility.h"
#include "libavoid/connector.h"
#include "libavoid/polyutil.h"
#include "libavoid/debug.h"
#include "libavoid/region.h"
#include "math.h"

//#define ORTHOGONAL_ROUTING

namespace Avoid {


static const unsigned int infoAdd = 1;
static const unsigned int infoDel = 2;
static const unsigned int infoMov = 3;


class MoveInfo {
    public:
        MoveInfo(ShapeRef *s, Polygn *p, bool fM)
            : shape(s)
            , newPoly(copyPoly(*p))
            , firstMove(fM)
        { }
        ~MoveInfo()
        {
            freePoly(newPoly);
        }
        ShapeRef *shape;
        Polygn newPoly;
        bool firstMove;
};



Router::Router()
    : PartialTime(false)
    , SimpleRouting(false)
    , segmt_penalty(0)
    , angle_penalty(0)
    , crossing_penalty(200)
    // Algorithm options:
    , UseAStarSearch(true)
    , IgnoreRegions(true)
    , SelectiveReroute(true)
    , IncludeEndpoints(true)
    , UseLeesAlgorithm(false)
    , InvisibilityGrph(true)
    , ConsolidateMoves(true)
    , PartialFeedback(false)
    // Instrumentation:
    , st_checked_edges(0)
#ifdef LINEDEBUG
    , avoid_screen(NULL)
#endif
{ }




void Router::addShape(ShapeRef *shape)
{
    unsigned int pid = shape->id();
    Polygn poly = shape->poly();

    adjustContainsWithAdd(poly, pid);
    
    // o  Check all visibility edges to see if this one shape
    //    blocks them.
    newBlockingShape(&poly, pid);

#ifdef ORTHOGONAL_ROUTING
    Region::addShape(shape);
#endif

    // o  Calculate visibility for the new vertices.
    if (UseLeesAlgorithm)
    {
        shapeVisSweep(shape);
    }
    else
    {
        shapeVis(shape);
    }
    callbackAllInvalidConnectors();
}


void Router::delShape(ShapeRef *shape)
{
    unsigned int pid = shape->id();

    // Delete items that are queued in the movList.
    for (MoveInfoList::iterator it = moveList.begin(); it != moveList.end(); )
    {
        if ((*it)->shape->id() == pid)
        {
            MoveInfoList::iterator doomed = it;
            ++it;
            moveList.erase(doomed);
        }
        else
        {
            ++it;
        }
    }

    // o  Remove entries related to this shape's vertices
    shape->removeFromGraph();
    
    if (SelectiveReroute)
    {
        markConnectors(shape);
    }

    adjustContainsWithDel(pid);
    
#ifdef ORTHOGONAL_ROUTING
    Region::removeShape(shape);
#endif

    delete shape;
    
    // o  Check all edges that were blocked by this shape.
    if (InvisibilityGrph)
    {
        checkAllBlockedEdges(pid);
    }
    else
    {
        // check all edges not in graph
        checkAllMissingEdges();
    }
    callbackAllInvalidConnectors();
}


void Router::moveShape(ShapeRef *shape, Polygn *newPoly, const bool first_move)
{
    // Sanely cope with the case where the user requests moving the same
    // shape multiple times before rerouting connectors.
    bool alreadyThere = false;
    unsigned int id = shape->id();
    MoveInfoList::iterator finish = moveList.end();
    for (MoveInfoList::iterator it = moveList.begin(); it != finish; ++it)
    {
        if ((*it)->shape->id() == id)
        {
            if (!SimpleRouting)
            {
                fprintf(stderr,
                        "warning: multiple moves requested for shape %d.\n",
                        (int) id);
            }
            // Just update the MoveInfo with the second polygon, but
            // leave the firstMove setting alone.
            (*it)->newPoly = copyPoly(*newPoly);
            alreadyThere = true;
        }
    }

    if (!alreadyThere)
    {
        MoveInfo *moveInfo = new MoveInfo(shape, newPoly, first_move);
        moveList.push_back(moveInfo);
    }

    if (!ConsolidateMoves)
    {
        processMoves();
    }
}


void Router::processMoves(void)
{
    // If SimpleRouting, then don't update yet.
    if (moveList.empty() || SimpleRouting)
    {
        return;
    }

    MoveInfoList::iterator curr;
    MoveInfoList::iterator finish = moveList.end();
    for (curr = moveList.begin(); curr != finish; ++curr)
    {
        MoveInfo *moveInf = *curr;
        ShapeRef *shape = moveInf->shape;
        Polygn *newPoly = &(moveInf->newPoly);
        bool first_move = moveInf->firstMove;

        unsigned int pid = shape->id();
        bool notPartialTime = !(PartialFeedback && PartialTime);

        // o  Remove entries related to this shape's vertices
        shape->removeFromGraph();
        
        if (SelectiveReroute && (notPartialTime || first_move))
        {
            markConnectors(shape);
        }

        adjustContainsWithDel(pid);
        
#ifdef ORTHOGONAL_ROUTING
        Region::removeShape(shape);
#endif

        shape->setNewPoly(*newPoly);

        adjustContainsWithAdd(*newPoly, pid);

        // Ignore this shape for visibility.
        // XXX: We don't really need to do this if we're not using Partial
        //      Feedback.  Without this the blocked edges still route
        //      around the shape until it leaves the connector.
        shape->makeInactive();
        
    }
    
    if (InvisibilityGrph)
    {
        for (curr = moveList.begin(); curr != finish; ++curr)
        {
            MoveInfo *moveInf = *curr;
            ShapeRef *shape = moveInf->shape;
            unsigned int pid = shape->id();
            
            // o  Check all edges that were blocked by this shape.
            checkAllBlockedEdges(pid);
        }
    }
    else
    {
        // check all edges not in graph
        checkAllMissingEdges();
    }

    while ( ! moveList.empty() )
    {
        MoveInfo *moveInf = moveList.front();
        ShapeRef *shape = moveInf->shape;
        Polygn *newPoly = &(moveInf->newPoly);

        unsigned int pid = shape->id();
        bool notPartialTime = !(PartialFeedback && PartialTime);

        // Restore this shape for visibility.
        shape->makeActive();

#ifdef ORTHOGONAL_ROUTING
        Region::addShape(shape);
#endif

        // o  Check all visibility edges to see if this one shape
        //    blocks them.
        if (notPartialTime)
        {
            newBlockingShape(newPoly, pid);
        }

        // o  Calculate visibility for the new vertices.
        if (UseLeesAlgorithm)
        {
            shapeVisSweep(shape);
        }
        else
        {
            shapeVis(shape);
        }
        
        moveList.pop_front();
        delete moveInf;
    }
    callbackAllInvalidConnectors();
}


//----------------------------------------------------------------------------

// XXX: attachedShapes and attachedConns both need to be rewritten
//      for constant time lookup of attached objects once this info
//      is stored better within libavoid.  Also they shouldn't need to
//      be friends of ConnRef.

    // Returns a list of connector Ids of all the connectors of type
    // 'type' attached to the shape with the ID 'shapeId'.
void Router::attachedConns(IntList &conns, const unsigned int shapeId,
        const unsigned int type)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) {

        if ((type & runningTo) && ((*i)->_dstId == shapeId)) {
            conns.push_back((*i)->_id);
        }
        else if ((type & runningFrom) && ((*i)->_srcId == shapeId)) {
            conns.push_back((*i)->_id);
        }
    }
}


    // Returns a list of shape Ids of all the shapes attached to the
    // shape with the ID 'shapeId' with connection type 'type'.
void Router::attachedShapes(IntList &shapes, const unsigned int shapeId,
        const unsigned int type)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) {
        if ((type & runningTo) && ((*i)->_dstId == shapeId)) {
            if ((*i)->_srcId != 0)
            {
                // Only if there is a shape attached to the other end.
                shapes.push_back((*i)->_srcId);
            }
        }
        else if ((type & runningFrom) && ((*i)->_srcId == shapeId)) {
            if ((*i)->_dstId != 0)
            {
                // Only if there is a shape attached to the other end.
                shapes.push_back((*i)->_dstId);
            }
        }
    }
}


    // It's intended this function is called after shape movement has 
    // happened to alert connectors that they need to be rerouted.
void Router::callbackAllInvalidConnectors(void)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) {
        (*i)->handleInvalid();
    }
}


void Router::newBlockingShape(Polygn *poly, int pid)
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
            std::pair<VertID, VertID> ids(tmp->ids());
            VertID eID1 = ids.first;
            VertID eID2 = ids.second;
            std::pair<Point, Point> points(tmp->points());
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


void Router::checkAllBlockedEdges(int pid)
{
    assert(InvisibilityGrph);

    for (EdgeInf *iter = invisGraph.begin(); iter != invisGraph.end() ; )
    {
        EdgeInf *tmp = iter;
        iter = iter->lstNext;

        if (tmp->_blocker == -1)
        {
            tmp->alertConns();
            tmp->checkVis();
        }
        else if (tmp->_blocker == pid)
        {
            tmp->checkVis();
        }
    }
}


void Router::checkAllMissingEdges(void)
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


void Router::generateContains(VertInf *pt)
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


void Router::adjustContainsWithAdd(const Polygn& poly, const int p_shape)
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


void Router::adjustContainsWithDel(const int p_shape)
{
    for (VertInf *k = vertices.connsBegin(); k != vertices.shapesBegin();
            k = k->lstNext)
    {
        contains[k->id].erase(p_shape);
    }
}


#ifdef SELECTIVE_DEBUG
static double AngleAFromThreeSides(const double a, const double b,
        const double c)
{
    // returns angle A, the angle opposite from side a, in radians
    return acos((pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c));
}
#endif

void Router::markConnectors(ShapeRef *shape)
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
        else if (conn->_needs_reroute_flag)
        {
            // Already marked, so skip.
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

                min = std::min(p1.x, p2.x);
                max = std::max(p1.x, p2.x);
            }
            else if (p1.x == p2.x)
            {
                // Other Standard case
                offy = p1.x;
                a = start.y;
                b = start.x - offy;
                c = end.y;
                d = end.x - offy;

                min = std::min(p1.y, p2.y);
                max = std::max(p1.y, p2.y);
            }
            else
            {
                // Need to do rotation
                Point n_p2(p2.x - p1.x, p2.y - p1.y);
                Point n_start(start.x - p1.x, start.y - p1.y);
                Point n_end(end.x - p1.x, end.y - p1.y);
                //printf("n_p2:    (%.1f, %.1f)\n", n_p2.x, n_p2.y);
                //printf("n_start: (%.1f, %.1f)\n", n_start.x, n_start.y);
                //printf("n_end:   (%.1f, %.1f)\n", n_end.x, n_end.y);

                double theta = 0 - atan2(n_p2.y, n_p2.x);
                //printf("theta = %.2f\n", theta * (180 / PI));

                Point r_p1(0, 0);
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
                    std::abort();
                }
                // This might be slightly off.
                r_p2.y = 0;

                offy = r_p1.y;
                a = start.x;
                b = start.y - offy;
                c = end.x;
                d = end.y - offy;

                min = std::min(r_p1.x, r_p2.x);
                max = std::max(r_p1.x, r_p2.x);

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

            x = std::max(min, x);
            x = std::min(max, x);

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


void Router::printInfo(void)
{
    FILE *fp = stdout;
    fprintf(fp, "\nVisibility Graph info:\n");
    fprintf(fp, "----------------------\n");

    unsigned int currshape = 0;
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


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


#ifndef AVOID_ROUTER_H
#define AVOID_ROUTER_H

//#define LINEDEBUG

#include "libavoid/shape.h"
#include "libavoid/graph.h"
#include "libavoid/timer.h"
#include <list>
#include <utility>
#ifdef LINEDEBUG       
    #include <SDL.h>
#endif


namespace Avoid {

class ConnRef;
typedef std::list<ConnRef *> ConnRefList;
typedef std::list<unsigned int> IntList;
class MoveInfo;
typedef std::list<MoveInfo *> MoveInfoList;


static const unsigned int runningTo = 1;
static const unsigned int runningFrom = 2;
static const unsigned int runningToAndFrom = runningTo | runningFrom;


class Router {
    public:
        Router();

        ShapeRefList shapeRefs;
        ConnRefList connRefs;
        EdgeList visGraph;
        EdgeList invisGraph;
        ContainsMap contains;
        VertInfList vertices;
        
        bool PartialTime;
        bool SimpleRouting;
        double segmt_penalty;
        double angle_penalty;
        double crossing_penalty;


        bool UseAStarSearch;
        bool IgnoreRegions;
        bool SelectiveReroute;
        bool IncludeEndpoints;
        bool UseLeesAlgorithm;
        bool InvisibilityGrph;
        bool ConsolidateMoves;
        bool PartialFeedback;

        // Instrumentation:
        Timer timers;
        int st_checked_edges;
#ifdef LINEDEBUG
        SDL_Surface *avoid_screen;
#endif

        void addShape(ShapeRef *shape);
        void delShape(ShapeRef *shape);
        void moveShape(ShapeRef *shape, Polygn *newPoly,
                const bool first_move = false);
        void processMoves(void);
        
        void attachedConns(IntList &conns, const unsigned int shapeId,
                const unsigned int type);
        void attachedShapes(IntList &shapes, const unsigned int shapeId,
                const unsigned int type);
        
        void markConnectors(ShapeRef *shape);
        void generateContains(VertInf *pt);
        void printInfo(void);
    private:
        void newBlockingShape(Polygn *poly, int pid);
        void checkAllBlockedEdges(int pid);
        void checkAllMissingEdges(void);
        void adjustContainsWithAdd(const Polygn& poly, const int p_shape);
        void adjustContainsWithDel(const int p_shape);
        void callbackAllInvalidConnectors(void);

        MoveInfoList moveList;
};

}



#endif

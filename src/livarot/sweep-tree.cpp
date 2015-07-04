#include "livarot/sweep-event-queue.h"
#include "livarot/sweep-tree-list.h"
#include "livarot/sweep-tree.h"
#include "livarot/sweep-event.h"
#include "livarot/Shape.h"


/*
 * the AVL tree holding the edges intersecting the sweepline
 * that structure is very sensitive to anything
 * you have edges stored in nodes, the nodes are sorted in increasing x-order of intersection
 * with the sweepline, you have the 2 potential intersections of the edge in the node with its
 * neighbours, plus the fact that it's stored in an array that's realloc'd
 */

SweepTree::SweepTree()
{
    src = NULL;
    bord = -1;
    startPoint = -1;
    evt[LEFT] = evt[RIGHT] = NULL;
    sens = true;
    //invDirLength=1;
}

SweepTree::~SweepTree()
{
    MakeDelete();
}

void
SweepTree::MakeNew(Shape *iSrc, int iBord, int iWeight, int iStartPoint)
{
    AVLTree::MakeNew();
    ConvertTo(iSrc, iBord, iWeight, iStartPoint);
}

void
SweepTree::ConvertTo(Shape *iSrc, int iBord, int iWeight, int iStartPoint)
{
    src = iSrc;
    bord = iBord;
    evt[LEFT] = evt[RIGHT] = NULL;
    startPoint = iStartPoint;
    if (src->getEdge(bord).st < src->getEdge(bord).en) {
        if (iWeight >= 0)
            sens = true;
        else
            sens = false;
    } else {
        if (iWeight >= 0)
            sens = false;
        else
            sens = true;
    }
    //invDirLength=src->eData[bord].isqlength;
    //invDirLength=1/sqrt(src->getEdge(bord).dx*src->getEdge(bord).dx+src->getEdge(bord).dy*src->getEdge(bord).dy);
}


void SweepTree::MakeDelete()
{
    for (int i = 0; i < 2; i++) {
        if (evt[i]) {
            evt[i]->sweep[1 - i] = NULL;
        }
        evt[i] = NULL;
    }

    AVLTree::MakeDelete();
}


// find the position at which node "newOne" should be inserted in the subtree rooted here
// we want to order with respect to the order of intersections with the sweepline, currently 
// lying at y=px[1].
// px is the upper endpoint of newOne
int
SweepTree::Find(Geom::Point const &px, SweepTree *newOne, SweepTree *&insertL,
                SweepTree *&insertR, bool sweepSens)
{
    // get the edge associated with this node: one point+one direction
    // since we're dealing with line, the direction (bNorm) is taken downwards
    Geom::Point bOrig, bNorm;
    bOrig = src->pData[src->getEdge(bord).st].rx;
    bNorm = src->eData[bord].rdx;
    if (src->getEdge(bord).st > src->getEdge(bord).en) {
        bNorm = -bNorm;
    }
    // rotate to get the normal to the edge
    bNorm=bNorm.ccw();

    Geom::Point diff;
    diff = px - bOrig;

    // compute (px-orig)^dir to know on which side of this edge the point px lies
    double y = 0;
    //if ( startPoint == newOne->startPoint ) {
    //   y=0;
    //} else {
    y = dot(bNorm, diff);
    //}
    //y*=invDirLength;
    if (fabs(y) < 0.000001) {
        // that damn point px lies on me, so i need to consider to direction of the edge in
        // newOne to know if it goes toward my left side or my right side
        // sweepSens is needed (actually only used by the Scan() functions) because if the sweepline goes upward,
        // signs change
        // prendre en compte les directions
        Geom::Point nNorm;
        nNorm = newOne->src->eData[newOne->bord].rdx;
        if (newOne->src->getEdge(newOne->bord).st >
            newOne->src->getEdge(newOne->bord).en)
	{
            nNorm = -nNorm;
	}
        nNorm=nNorm.ccw();

        if (sweepSens) {
            y = cross(bNorm, nNorm);
        } else {
            y = cross(nNorm, bNorm);
        }
        if (y == 0) {
            y = dot(bNorm, nNorm);
            if (y == 0) {
                insertL = this;
                insertR = static_cast<SweepTree *>(elem[RIGHT]);
                return found_exact;
            }
        }
    }
    if (y < 0) {
        if (child[LEFT]) {
            return (static_cast<SweepTree *>(child[LEFT]))->Find(px, newOne,
                                                               insertL, insertR,
                                                               sweepSens);
	} else {
            insertR = this;
            insertL = static_cast<SweepTree *>(elem[LEFT]);
            if (insertL) {
                return found_between;
            } else {
                return found_on_left;
	    }
	}
    } else {
        if (child[RIGHT]) {
            return (static_cast<SweepTree *>(child[RIGHT]))->Find(px, newOne,
                                                                insertL, insertR,
                                                                sweepSens);
	} else {
            insertL = this;
            insertR = static_cast<SweepTree *>(elem[RIGHT]);
            if (insertR) {
                return found_between;
	    } else {
                return found_on_right;
	    }
	}
    }
    return not_found;
}

// only find a point's position
int
SweepTree::Find(Geom::Point const &px, SweepTree * &insertL,
		 SweepTree * &insertR)
{
  Geom::Point bOrig, bNorm;
  bOrig = src->pData[src->getEdge(bord).st].rx;
  bNorm = src->eData[bord].rdx;
  if (src->getEdge(bord).st > src->getEdge(bord).en)
    {
      bNorm = -bNorm;
    }
 bNorm=bNorm.ccw();

  Geom::Point diff;
  diff = px - bOrig;

  double y = 0;
  y = dot(bNorm, diff);
  if (y == 0)
    {
      insertL = this;
      insertR = static_cast<SweepTree *>(elem[RIGHT]);
      return found_exact;
    }
  if (y < 0)
    {
      if (child[LEFT])
	{
	  return (static_cast<SweepTree *>(child[LEFT]))->Find(px, insertL,
							    insertR);
	}
      else
	{
	  insertR = this;
	  insertL = static_cast<SweepTree *>(elem[LEFT]);
	  if (insertL)
	    {
	      return found_between;
	    }
	  else
	    {
	      return found_on_left;
	    }
	}
    }
  else
    {
      if (child[RIGHT])
	{
	  return (static_cast<SweepTree *>(child[RIGHT]))->Find(px, insertL,
							    insertR);
	}
      else
	{
	  insertL = this;
	  insertR = static_cast<SweepTree *>(elem[RIGHT]);
	  if (insertR)
	    {
	      return found_between;
	    }
	  else
	    {
	      return found_on_right;
	    }
	}
    }
  return not_found;
}

void
SweepTree::RemoveEvents(SweepEventQueue & queue)
{
    RemoveEvent(queue, LEFT);
    RemoveEvent(queue, RIGHT);
}

void SweepTree::RemoveEvent(SweepEventQueue &queue, Side s)
{
    if (evt[s]) {
        queue.remove(evt[s]);
        evt[s] = NULL;
    }
}

int
SweepTree::Remove(SweepTreeList &list, SweepEventQueue &queue,
                  bool rebalance)
{
  RemoveEvents(queue);
  AVLTree *tempR = static_cast<AVLTree *>(list.racine);
  int err = AVLTree::Remove(tempR, rebalance);
  list.racine = static_cast<SweepTree *>(tempR);
  MakeDelete();
  if (list.nbTree <= 1)
    {
      list.nbTree = 0;
      list.racine = NULL;
    }
  else
    {
      if (list.racine == list.trees + (list.nbTree - 1))
	list.racine = this;
      list.trees[--list.nbTree].Relocate(this);
    }
  return err;
}

int
SweepTree::Insert(SweepTreeList &list, SweepEventQueue &queue,
                  Shape *iDst, int iAtPoint, bool rebalance, bool sweepSens)
{
  if (list.racine == NULL)
    {
      list.racine = this;
      return avl_no_err;
    }
  SweepTree *insertL = NULL;
  SweepTree *insertR = NULL;
  int insertion =
    list.racine->Find(iDst->getPoint(iAtPoint).x, this,
		       insertL, insertR, sweepSens);
  
    if (insertion == found_exact) {
	if (insertR) {
	    insertR->RemoveEvent(queue, LEFT);
	}
	if (insertL) {
	    insertL->RemoveEvent(queue, RIGHT);
	}

    } else if (insertion == found_between) {
      insertR->RemoveEvent(queue, LEFT);
      insertL->RemoveEvent(queue, RIGHT);
    }

  AVLTree *tempR = static_cast<AVLTree *>(list.racine);
  int err =
    AVLTree::Insert(tempR, insertion, static_cast<AVLTree *>(insertL),
		     static_cast<AVLTree *>(insertR), rebalance);
  list.racine = static_cast<SweepTree *>(tempR);
  return err;
}

// insertAt() is a speedup on the regular sweepline: if the polygon contains a point of high degree, you
// get a set of edge that are to be added in the same position. thus you insert one edge with a regular insert(),
// and then insert all the other in a doubly-linked list fashion. this avoids the Find() call, but is O(d^2) worst-case
// where d is the number of edge to add in this fashion. hopefully d remains small

int
SweepTree::InsertAt(SweepTreeList &list, SweepEventQueue &queue,
                    Shape */*iDst*/, SweepTree *insNode, int fromPt,
                    bool rebalance, bool sweepSens)
{
  if (list.racine == NULL)
    {
      list.racine = this;
      return avl_no_err;
    }

  Geom::Point fromP;
  fromP = src->pData[fromPt].rx;
  Geom::Point nNorm;
  nNorm = src->getEdge(bord).dx;
  if (src->getEdge(bord).st > src->getEdge(bord).en)
    {
      nNorm = -nNorm;
    }
  if (sweepSens == false)
    {
      nNorm = -nNorm;
    }

  Geom::Point bNorm;
  bNorm = insNode->src->getEdge(insNode->bord).dx;
  if (insNode->src->getEdge(insNode->bord).st >
      insNode->src->getEdge(insNode->bord).en)
    {
      bNorm = -bNorm;
    }

  SweepTree *insertL = NULL;
  SweepTree *insertR = NULL;
  double ang = cross(bNorm, nNorm);
  if (ang == 0)
    {
      insertL = insNode;
      insertR = static_cast<SweepTree *>(insNode->elem[RIGHT]);
    }
  else if (ang > 0)
    {
      insertL = insNode;
      insertR = static_cast<SweepTree *>(insNode->elem[RIGHT]);

      while (insertL)
	{
	  if (insertL->src == src)
	    {
	      if (insertL->src->getEdge(insertL->bord).st != fromPt
		  && insertL->src->getEdge(insertL->bord).en != fromPt)
		{
		  break;
		}
	    }
	  else
	    {
	      int ils = insertL->src->getEdge(insertL->bord).st;
	      int ile = insertL->src->getEdge(insertL->bord).en;
	      if ((insertL->src->pData[ils].rx[0] != fromP[0]
		   || insertL->src->pData[ils].rx[1] != fromP[1])
		  && (insertL->src->pData[ile].rx[0] != fromP[0]
		      || insertL->src->pData[ile].rx[1] != fromP[1]))
		{
		  break;
		}
	    }
	  bNorm = insertL->src->getEdge(insertL->bord).dx;
	  if (insertL->src->getEdge(insertL->bord).st >
	      insertL->src->getEdge(insertL->bord).en)
	    {
	      bNorm = -bNorm;
	    }
	  ang = cross(bNorm, nNorm);
	  if (ang <= 0)
	    {
	      break;
	    }
	  insertR = insertL;
	  insertL = static_cast<SweepTree *>(insertR->elem[LEFT]);
	}
    }
  else if (ang < 0)
    {
      insertL = insNode;
      insertR = static_cast<SweepTree *>(insNode->elem[RIGHT]);

      while (insertR)
	{
	  if (insertR->src == src)
	    {
	      if (insertR->src->getEdge(insertR->bord).st != fromPt
		  && insertR->src->getEdge(insertR->bord).en != fromPt)
		{
		  break;
		}
	    }
	  else
	    {
	      int ils = insertR->src->getEdge(insertR->bord).st;
	      int ile = insertR->src->getEdge(insertR->bord).en;
	      if ((insertR->src->pData[ils].rx[0] != fromP[0]
		   || insertR->src->pData[ils].rx[1] != fromP[1])
		  && (insertR->src->pData[ile].rx[0] != fromP[0]
		      || insertR->src->pData[ile].rx[1] != fromP[1]))
		{
		  break;
		}
	    }
	  bNorm = insertR->src->getEdge(insertR->bord).dx;
	  if (insertR->src->getEdge(insertR->bord).st >
	      insertR->src->getEdge(insertR->bord).en)
	    {
	      bNorm = -bNorm;
	    }
	  ang = cross(bNorm, nNorm);
	  if (ang > 0)
	    {
	      break;
	    }
	  insertL = insertR;
	  insertR = static_cast<SweepTree *>(insertL->elem[RIGHT]);
	}
    }

  int insertion = found_between;

  if (insertL == NULL) {
    insertion = found_on_left;
  }
  if (insertR == NULL) {
    insertion = found_on_right;
  }
  
  if (insertion == found_exact) {
      /* FIXME: surely this can never be called? */
      if (insertR) {
	  insertR->RemoveEvent(queue, LEFT);
      }
      if (insertL) {
	  insertL->RemoveEvent(queue, RIGHT);
      }
  } else if (insertion == found_between) {
      insertR->RemoveEvent(queue, LEFT);
      insertL->RemoveEvent(queue, RIGHT);
  }

  AVLTree *tempR = static_cast<AVLTree *>(list.racine);
  int err =
    AVLTree::Insert(tempR, insertion, static_cast<AVLTree *>(insertL),
		     static_cast<AVLTree *>(insertR), rebalance);
  list.racine = static_cast<SweepTree *>(tempR);
  return err;
}

void
SweepTree::Relocate(SweepTree * to)
{
  if (this == to)
    return;
  AVLTree::Relocate(to);
  to->src = src;
  to->bord = bord;
  to->sens = sens;
  to->evt[LEFT] = evt[LEFT];
  to->evt[RIGHT] = evt[RIGHT];
  to->startPoint = startPoint;
  if (unsigned(bord) < src->swsData.size())
    src->swsData[bord].misc = to;
  if (unsigned(bord) < src->swrData.size())
    src->swrData[bord].misc = to;
  if (evt[LEFT])
    evt[LEFT]->sweep[RIGHT] = to;
  if (evt[RIGHT])
    evt[RIGHT]->sweep[LEFT] = to;
}

// TODO check if ignoring these parameters is bad
void
SweepTree::SwapWithRight(SweepTreeList &/*list*/, SweepEventQueue &/*queue*/)
{
    SweepTree *tL = this;
    SweepTree *tR = static_cast<SweepTree *>(elem[RIGHT]);

    tL->src->swsData[tL->bord].misc = tR;
    tR->src->swsData[tR->bord].misc = tL;

    {
        Shape *swap = tL->src;
        tL->src = tR->src;
        tR->src = swap;
    }
    {
        int swap = tL->bord;
        tL->bord = tR->bord;
        tR->bord = swap;
    }
    {
        int swap = tL->startPoint;
        tL->startPoint = tR->startPoint;
        tR->startPoint = swap;
    }
    //{double swap=tL->invDirLength;tL->invDirLength=tR->invDirLength;tR->invDirLength=swap;}
    {
        bool swap = tL->sens;
        tL->sens = tR->sens;
        tR->sens = swap;
    }
}

void
SweepTree::Avance(Shape */*dstPts*/, int /*curPoint*/, Shape */*a*/, Shape */*b*/)
{
    return;
/*	if ( curPoint != startPoint ) {
		int nb=-1;
		if ( sens ) {
//			nb=dstPts->AddEdge(startPoint,curPoint);
		} else {
//			nb=dstPts->AddEdge(curPoint,startPoint);
		}
		if ( nb >= 0 ) {
			dstPts->swsData[nb].misc=(void*)((src==b)?1:0);
			int   wp=waitingPoint;
			dstPts->eData[nb].firstLinkedPoint=waitingPoint;
			waitingPoint=-1;
			while ( wp >= 0 ) {
				dstPts->pData[wp].edgeOnLeft=nb;
				wp=dstPts->pData[wp].nextLinkedPoint;
			}
		}
		startPoint=curPoint;
	}*/
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

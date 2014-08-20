/*
 *  ShapeRaster.cpp
 *  nlivarot
 *
 *  Created by fred on Sat Jul 19 2003.
 *
 */

#include "Shape.h"

#include "livarot/float-line.h"
#include "AlphaLigne.h"
#include "BitLigne.h"

#include "livarot/sweep-event-queue.h"
#include "livarot/sweep-tree-list.h"
#include "livarot/sweep-tree.h"

/*
 * polygon rasterization: the sweepline algorithm in all its glory
 * nothing unusual in this implementation, so nothing special to say
 * the *Quick*() functions are not useful. forget about them
 */

void Shape::BeginRaster(float &pos, int &curPt)
{
    if ( numberOfPoints() <= 1 || numberOfEdges() <= 1 ) {
        curPt = 0;
        pos = 0;
        return;
    }
    
    MakeRasterData(true);
    MakePointData(true);
    MakeEdgeData(true);

    if (sTree == NULL) {
        sTree = new SweepTreeList(numberOfEdges());
    }
    if (sEvts == NULL) {
        sEvts = new SweepEventQueue(numberOfEdges());
    }

    SortPoints();

    curPt = 0;
    pos = getPoint(0).x[1] - 1.0;

    for (int i = 0; i < numberOfPoints(); i++) {
        pData[i].pending = 0;
        pData[i].edgeOnLeft = -1;
        pData[i].nextLinkedPoint = -1;
        pData[i].rx[0] = /*Round(*/getPoint(i).x[0]/*)*/;
        pData[i].rx[1] = /*Round(*/getPoint(i).x[1]/*)*/;
    }

    for (int i = 0;i < numberOfEdges(); i++) {
        swrData[i].misc = NULL;
        eData[i].rdx=pData[getEdge(i).en].rx - pData[getEdge(i).st].rx;
    }
}


void Shape::EndRaster()
{
    delete sTree;
    sTree = NULL;
    delete sEvts;
    sEvts = NULL;
    
    MakePointData(false);
    MakeEdgeData(false);
    MakeRasterData(false);
}


void Shape::BeginQuickRaster(float &pos, int &curPt)
{
    if ( numberOfPoints() <= 1 || numberOfEdges() <= 1 ) {
        curPt = 0;
        pos = 0;
        return;
    }
    
    MakeRasterData(true);
    MakeQuickRasterData(true);
    nbQRas = 0;
    firstQRas = lastQRas = -1;
    MakePointData(true);
    MakeEdgeData(true);

    curPt = 0;
    pos = getPoint(0).x[1] - 1.0;

    initialisePointData();
    
    for (int i=0;i<numberOfEdges();i++) {
        swrData[i].misc = NULL;
        qrsData[i].ind = -1;
        eData[i].rdx = pData[getEdge(i).en].rx - pData[getEdge(i).st].rx;
    }
    
    SortPoints();
//	SortPointsRounded();
}


void Shape::EndQuickRaster()
{
    MakePointData(false);
    MakeEdgeData(false);
    MakeRasterData(false);
    MakeQuickRasterData(false);
}


// 2 versions of the Scan() series to move the scanline to a given position withou actually computing coverages
void Shape::Scan(float &pos, int &curP, float to, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }

    if ( pos == to ) {
        return;
    }

    enum Direction {
        DOWNWARDS,
        UPWARDS
    };

    Direction const d = (pos < to) ? DOWNWARDS : UPWARDS;

    // points of the polygon are sorted top-down, so we take them in order, starting with the one at index curP,
    // until we reach the wanted position to.
    // don't forget to update curP and pos when we're done
    int curPt = curP;
    while ( ( d == DOWNWARDS && curPt < numberOfPoints() && getPoint(curPt).x[1]     <= to) ||
            ( d == UPWARDS   && curPt > 0                && getPoint(curPt - 1).x[1] >= to) )
    {
        int nPt = (d == DOWNWARDS) ? curPt++ : --curPt;
      
        // treat a new point: remove and add edges incident to it
        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);

				if ( d == DOWNWARDS ) {
					if ( nbDn <= 0 ) {
            upNo = -1;
					}
					if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
					}
				} else {
					if ( nbUp <= 0 ) {
            dnNo = -1;
					}
					if ( dnNo >= 0 && swrData[dnNo].misc == NULL ) {
            dnNo = -1;
					}
				}
        
        if ( ( d == DOWNWARDS && nbUp > 0 ) || ( d == UPWARDS && nbDn > 0 ) ) {
            // first remove edges coming from above or below, as appropriate
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {

                Shape::dg_arete const &e = getEdge(cb);
                if ( (d == DOWNWARDS && nPt == std::max(e.st, e.en)) ||
                     (d == UPWARDS   && nPt == std::min(e.st, e.en)) )
                {
                    if ( ( d == DOWNWARDS && cb != upNo ) || ( d == UPWARDS && cb != dnNo ) ) {
                        // we salvage the edge upNo to plug the edges we'll be addingat its place
                        // but the other edge don't have this chance
                        SweepTree *node = swrData[cb].misc;
                        if ( node ) {
                            swrData[cb].misc = NULL;
                            node->Remove(*sTree, *sEvts, true);
                        }
                    }
                }
                cb = NextAt(nPt, cb);
            }
 				}
      
        // if there is one edge going down and one edge coming from above, we don't Insert() the new edge,
        // but replace the upNo edge by the new one (faster)
        SweepTree* insertionNode = NULL;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
							int    rmNo=(d == DOWNWARDS) ? upNo:dnNo;
							int    neNo=(d == DOWNWARDS) ? dnNo:upNo;
							  SweepTree* node = swrData[rmNo].misc;
                swrData[rmNo].misc = NULL;

                int const P = (d == DOWNWARDS) ? nPt : Other(nPt, neNo);
                node->ConvertTo(this, neNo, 1, P);
                
                swrData[neNo].misc = node;
                insertionNode = node;
                CreateEdge(neNo, to, step);
            } else {
							// always DOWNWARDS
                SweepTree* node = sTree->add(this, dnNo, 1, nPt, this);
                swrData[dnNo].misc = node;
                node->Insert(*sTree, *sEvts, this, nPt, true);
                //if (d == UPWARDS) {
                //    node->startPoint = Other(nPt, dnNo);
                //}
                insertionNode = node;
                CreateEdge(dnNo,to,step);
            }
        } else {
					if ( upNo >= 0 ) {
						// always UPWARDS
						SweepTree* node = sTree->add(this, upNo, 1, nPt, this);
						swrData[upNo].misc = node;
						node->Insert(*sTree, *sEvts, this, nPt, true);
						//if (d == UPWARDS) {
							node->startPoint = Other(nPt, upNo);
						//}
						insertionNode = node;
						CreateEdge(upNo,to,step);
					}
				}
      
        // add the remaining edges
        if ( ( d == DOWNWARDS && nbDn > 1 ) || ( d == UPWARDS && nbUp > 1 ) ) {
            // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo && cb != upNo ) {
                        SweepTree *node = sTree->add(this, cb, 1, nPt, this);
                        swrData[cb].misc = node;
                        node->InsertAt(*sTree, *sEvts, this, insertionNode, nPt, true);
                        if (d == UPWARDS) {
                            node->startPoint = Other(nPt, cb);
                        }
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
    }
        
    curP = curPt;
    if ( curPt > 0 ) {
        pos = getPoint(curPt - 1).x[1];
    } else {
        pos = to;
    }
    
    // the final touch: edges intersecting the sweepline must be update so that their intersection with
    // said sweepline is correct.
    pos = to;
    if ( sTree->racine ) {
        SweepTree* curS = static_cast<SweepTree*>(sTree->racine->Leftmost());
        while ( curS ) {
            int cb = curS->bord;
            AvanceEdge(cb, to, true, step);
            curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
        }
    }
}

    

void Shape::QuickScan(float &pos,int &curP, float to, bool /*doSort*/, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }
    
    if ( pos == to ) {
        return;
    }

    enum Direction {
        DOWNWARDS,
        UPWARDS
    };

    Direction const d = (pos < to) ? DOWNWARDS : UPWARDS;
    
    int curPt = curP;
    while ( (d == DOWNWARDS && curPt < numberOfPoints() && getPoint(curPt    ).x[1] <= to) ||
            (d == UPWARDS   && curPt > 0                && getPoint(curPt - 1).x[1] >= to) )
    {
        int nPt = (d == DOWNWARDS) ? curPt++ : --curPt;

        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
            
        if ( nbDn <= 0 ) {
            upNo = -1;
        }
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
        }

        if ( nbUp > 0 ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( (d == DOWNWARDS && nPt == std::max(e.st, e.en)) ||
                     (d == UPWARDS && nPt == std::min(e.st, e.en)) )
                {
                    if ( cb != upNo ) {
                        QuickRasterSubEdge(cb);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }

        // traitement du "upNo devient dnNo"
        int ins_guess = -1;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                ins_guess = QuickRasterChgEdge(upNo, dnNo, getPoint(nPt).x[0]);
            } else {
                ins_guess = QuickRasterAddEdge(dnNo, getPoint(nPt).x[0], ins_guess);
            }
            CreateEdge(dnNo, to, step);
        }

        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( (d == DOWNWARDS && nPt == std::min(e.st, e.en)) ||
                     (d == UPWARDS && nPt == std::max(e.st, e.en)) )
                {
                    if ( cb != dnNo ) {
                        ins_guess = QuickRasterAddEdge(cb, getPoint(nPt).x[0], ins_guess);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
    
        
        curP = curPt;
        if ( curPt > 0 ) {
            pos = getPoint(curPt-1).x[1];
        } else {
            pos = to;
        }
    }

    pos = to;
        
    for (int i=0; i < nbQRas; i++) {
        int cb = qrsData[i].bord;
        AvanceEdge(cb, to, true, step);
        qrsData[i].x=swrData[cb].curX;
    }
    
    QuickRasterSort();
}



int Shape::QuickRasterChgEdge(int oBord, int nBord, double x)
{
    if ( oBord == nBord ) {
        return -1;
    }
    
    int no = qrsData[oBord].ind;
    if ( no >= 0 ) {
        qrsData[no].bord = nBord;
        qrsData[no].x = x;
        qrsData[oBord].ind = -1;
        qrsData[nBord].ind = no;
    }
    
    return no;
}



int Shape::QuickRasterAddEdge(int bord, double x, int guess)
{
    int no = nbQRas++;
    qrsData[no].bord = bord;
    qrsData[no].x = x;
    qrsData[bord].ind = no;
    qrsData[no].prev = -1;
    qrsData[no].next = -1;
    
    if ( no < 0 || no >= nbQRas ) {
        return -1;
    }
  
    if ( firstQRas < 0 ) {
        firstQRas = lastQRas = no;
        qrsData[no].prev = -1;
        qrsData[no].next = -1;
        return no;
    }

    if ( guess < 0 || guess >= nbQRas ) {

        int c = firstQRas;
        while ( c >= 0 && c < nbQRas && CmpQRs(qrsData[c],qrsData[no]) < 0 ) {
            c = qrsData[c].next;
        }
        
        if ( c < 0 || c >= nbQRas ) {
            qrsData[no].prev = lastQRas;
            qrsData[lastQRas].next = no;
            lastQRas = no;
        } else {
            qrsData[no].prev = qrsData[c].prev;
            if ( qrsData[no].prev >= 0 ) {
                qrsData[qrsData[no].prev].next=no;
            } else {
                firstQRas = no;
            }
            
            qrsData[no].next = c;
            qrsData[c].prev = no;
        }
        
    } else {
        int c = guess;
        int stTst = CmpQRs(qrsData[c],qrsData[no]);
        if ( stTst == 0 ) {

            qrsData[no].prev = qrsData[c].prev;
            if ( qrsData[no].prev >= 0 ) {
                qrsData[qrsData[no].prev].next = no;
            } else {
                firstQRas = no;
            }
            
            qrsData[no].next = c;
            qrsData[c].prev = no;
            
        } else if ( stTst > 0 ) {

            while ( c >= 0 && c < nbQRas && CmpQRs(qrsData[c],qrsData[no]) > 0 ) {
                c = qrsData[c].prev;
            }
            
            if ( c < 0 || c >= nbQRas ) {
                qrsData[no].next = firstQRas;
                qrsData[firstQRas].prev = no; // firstQRas != -1
                firstQRas = no;
            } else {
                qrsData[no].next = qrsData[c].next;
                if ( qrsData[no].next >= 0 ) {
                    qrsData[qrsData[no].next].prev = no;
                } else {
                    lastQRas = no;
                }
                qrsData[no].prev = c;
                qrsData[c].next = no;
            }
            
        } else {

            while ( c >= 0 && c < nbQRas && CmpQRs(qrsData[c],qrsData[no]) < 0 ) {
                c = qrsData[c].next;
            }
            
            if ( c < 0 || c >= nbQRas ) {
                qrsData[no].prev = lastQRas;
                qrsData[lastQRas].next = no;
                lastQRas = no;
            } else {
                qrsData[no].prev = qrsData[c].prev;
                if ( qrsData[no].prev >= 0 ) {
                    qrsData[qrsData[no].prev].next = no;
                } else {
                    firstQRas = no;
                }
                
                qrsData[no].next = c;
                qrsData[c].prev = no;
            }
        }
    }
  
    return no;
}



void Shape::QuickRasterSubEdge(int bord)
{
    int no = qrsData[bord].ind;
    if ( no < 0 || no >= nbQRas ) {
        return; // euuhHHH
    }
    
    if ( qrsData[no].prev >= 0 ) {
        qrsData[qrsData[no].prev].next=qrsData[no].next;
    }
    
    if ( qrsData[no].next >= 0 ) {
        qrsData[qrsData[no].next].prev = qrsData[no].prev;
    }
    
    if ( no == firstQRas ) {
        firstQRas = qrsData[no].next;
    }
    
    if ( no == lastQRas ) {
        lastQRas = qrsData[no].prev;
    }
    
    qrsData[no].prev = qrsData[no].next = -1;
  
    int savInd = qrsData[no].ind;
    qrsData[no] = qrsData[--nbQRas];
    qrsData[no].ind = savInd;
    qrsData[qrsData[no].bord].ind = no;
    qrsData[bord].ind = -1;
  
    if ( nbQRas > 0 ) {
        if ( firstQRas == nbQRas ) {
            firstQRas = no;
        }
        if ( lastQRas == nbQRas ) {
            lastQRas = no;
        }
        if ( qrsData[no].prev >= 0 ) {
            qrsData[qrsData[no].prev].next = no;
        }
        if ( qrsData[no].next >= 0 ) {
            qrsData[qrsData[no].next].prev = no;
        }
    }  
}



void Shape::QuickRasterSwapEdge(int a, int b)
{
    if ( a == b ) {
        return;
    }
    
    int na = qrsData[a].ind;
    int nb = qrsData[b].ind;
    if ( na < 0 || na >= nbQRas || nb < 0 || nb >= nbQRas ) {
        return; // errrm
    }
  
    qrsData[na].bord = b;
    qrsData[nb].bord = a;
    qrsData[a].ind = nb;
    qrsData[b].ind = na;
    
    double swd = qrsData[na].x;
    qrsData[na].x = qrsData[nb].x;
    qrsData[nb].x = swd;
}


void Shape::QuickRasterSort()
{
    if ( nbQRas <= 1 ) {
        return;
    }
    
    int cb = qrsData[firstQRas].bord;
    
    while ( cb >= 0 ) {
        int bI = qrsData[cb].ind;
        int nI = qrsData[bI].next;
        
        if ( nI < 0 ) {
            break;
        }
    
        int ncb = qrsData[nI].bord;
        if ( CmpQRs(qrsData[nI], qrsData[bI]) < 0 ) {
            QuickRasterSwapEdge(cb, ncb);
            int pI = qrsData[bI].prev; // ca reste bI, puisqu'on a juste echange les contenus
            if ( pI < 0 ) {
                cb = ncb; // en fait inutile; mais bon...
            } else {
                int pcb = qrsData[pI].bord;
                cb = pcb;
            }
        } else {
            cb = ncb;
        }
    }
}


// direct scan to a given position. goes through the edge list to keep only the ones intersecting the target sweepline
// good for initial setup of scanline algo, bad for incremental changes
void Shape::DirectScan(float &pos, int &curP, float to, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }
    
    if ( pos == to ) {
        return;
    }
    
    if ( pos < to ) {
        // we're moving downwards
        // points of the polygon are sorted top-down, so we take them in order, starting with the one at index curP,
        // until we reach the wanted position to.
        // don't forget to update curP and pos when we're done
        int curPt = curP;
        while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
            curPt++;
        }
        
        for (int i=0;i<numberOfEdges();i++) {
            if ( swrData[i].misc ) {
                SweepTree* node = swrData[i].misc;
                swrData[i].misc = NULL;
                node->Remove(*sTree, *sEvts, true);
            }
        }

        for (int i=0; i < numberOfEdges(); i++) {
            Shape::dg_arete const &e = getEdge(i);
            if ( ( e.st < curPt && e.en >= curPt ) || ( e.en < curPt && e.st >= curPt )) {
                // crosses sweepline
                int nPt = (e.st < curPt) ? e.st : e.en;
                SweepTree* node = sTree->add(this, i, 1, nPt, this);
                swrData[i].misc = node;
                node->Insert(*sTree, *sEvts, this, nPt, true);
                CreateEdge(i, to, step);
            }
        }
        
        curP = curPt;
        if ( curPt > 0 ) {
            pos = getPoint(curPt - 1).x[1];
        } else {
            pos = to;
        }
        
    } else {
        
        // same thing, but going up. so the sweepSens is inverted for the Find() function
        int curPt=curP;
        while ( curPt > 0 && getPoint(curPt-1).x[1] >= to ) {
            curPt--;
        }

        for (int i = 0; i < numberOfEdges(); i++) {
            if ( swrData[i].misc ) {
                SweepTree* node = swrData[i].misc;
                swrData[i].misc = NULL;
                node->Remove(*sTree, *sEvts, true);
            }
        }
        
        for (int i=0;i<numberOfEdges();i++) {
            Shape::dg_arete const &e = getEdge(i);
            if ( ( e.st > curPt - 1 && e.en <= curPt - 1 ) || ( e.en > curPt - 1 && e.st <= curPt - 1 )) {
                // crosses sweepline
                int nPt = (e.st > curPt) ? e.st : e.en;
                SweepTree* node = sTree->add(this, i, 1, nPt, this);
                swrData[i].misc = node;
                node->Insert(*sTree, *sEvts, this, nPt, false);
                node->startPoint = Other(nPt, i);
                CreateEdge(i, to, step);
            }
        }
		
        curP = curPt;
        if ( curPt > 0 ) {
            pos = getPoint(curPt - 1).x[1];
        } else {
            pos = to;
        }
    }
        
    // the final touch: edges intersecting the sweepline must be update so that their intersection with
    // said sweepline is correct.
    pos = to;
    if ( sTree->racine ) {
        SweepTree* curS=static_cast<SweepTree*>(sTree->racine->Leftmost());
        while ( curS ) {
            int cb = curS->bord;
            AvanceEdge(cb, to, true, step);
            curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
        }
    }
}


    
void Shape::DirectQuickScan(float &pos, int &curP, float to, bool /*doSort*/, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }

    if ( pos == to ) {
        return;
    }
    
    if ( pos < to ) {
        // we're moving downwards
        // points of the polygon are sorted top-down, so we take them in order, starting with the one at index curP,
        // until we reach the wanted position to.
        // don't forget to update curP and pos when we're done
        int curPt=curP;
        while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
            curPt++;
        }
        
        for (int i = 0; i < numberOfEdges(); i++) {
            if ( qrsData[i].ind < 0 ) {
                QuickRasterSubEdge(i);
            }
        }
        
        for (int i = 0; i < numberOfEdges(); i++) {
            Shape::dg_arete const &e = getEdge(i);
            if ( ( e.st < curPt && e.en >= curPt ) || ( e.en < curPt && e.st >= curPt )) {
                // crosses sweepline
                int nPt = (e.st < e.en) ? e.st : e.en;
                QuickRasterAddEdge(i, getPoint(nPt).x[0], -1);
                CreateEdge(i, to, step);
            }
        }
    
        curP = curPt;
        if ( curPt > 0 ) {
            pos=getPoint(curPt-1).x[1];
        } else {
            pos = to;
        }
        
    } else {

        // same thing, but going up. so the sweepSens is inverted for the Find() function
        int curPt=curP;
        while ( curPt > 0 && getPoint(curPt-1).x[1] >= to ) {
            curPt--;
        }
    
        for (int i = 0; i < numberOfEdges(); i++) {
            if ( qrsData[i].ind < 0 ) {
                QuickRasterSubEdge(i);
            }
        }
        
        for (int i=0;i<numberOfEdges();i++) {
            Shape::dg_arete const &e = getEdge(i);
            if ( ( e.st < curPt-1 && e.en >= curPt-1 ) || ( e.en < curPt-1 && e.st >= curPt-1 )) {
                // crosses sweepline
                int nPt = (e.st > e.en) ? e.st : e.en;
                QuickRasterAddEdge(i, getPoint(nPt).x[0], -1);
                CreateEdge(i, to, step);
            }
        }
		
        curP = curPt;
        if ( curPt > 0 ) {
            pos = getPoint(curPt-1).x[1];
        } else {
            pos = to;
        }
        
    }
    
    pos = to;
    for (int i = 0; i < nbQRas; i++) {
        int cb = qrsData[i].bord;
        AvanceEdge(cb, to, true, step);
        qrsData[i].x = swrData[cb].curX;
    }
    
    QuickRasterSort();
}


// scan and compute coverage, FloatLigne version coverage of the line is bult in 2 parts: first a
// set of rectangles of height the height of the line (here: "step") one rectangle for each portion
// of the sweepline that is in the polygon at the beginning of the scan.  then a set ot trapezoids
// are added or removed to these rectangles, one trapezoid for each edge destroyed or edge crossing
// the entire line. think of it as a refinement of the coverage by rectangles

void Shape::Scan(float &pos, int &curP, float to, FloatLigne *line, bool exact, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }
    
    if ( pos >= to ) {
        return;
    }
    
    // first step: the rectangles since we read the sweepline left to right, we know the
    // boundaries of the rectangles are appended in a list, hence the AppendBord(). we salvage
    // the guess value for the trapezoids the edges will induce
        
    if ( sTree->racine ) {
        SweepTree *curS = static_cast<SweepTree*>(sTree->racine->Leftmost());
        while ( curS ) {
            
            int lastGuess = -1;
            int cb = curS->bord;
            
            if ( swrData[cb].sens == false && curS->elem[LEFT] ) {
                
                int lb = (static_cast<SweepTree*>(curS->elem[LEFT]))->bord;
                
                lastGuess = line->AppendBord(swrData[lb].curX,
                                             to - swrData[lb].curY,
                                             swrData[cb].curX,
                                             to - swrData[cb].curY,0.0);
                
                swrData[lb].guess = lastGuess - 1;
                swrData[cb].guess = lastGuess;
            } else {
                int lb = curS->bord;
                swrData[lb].guess = -1;
            }
            
            curS=static_cast <SweepTree*> (curS->elem[RIGHT]);
        }
    }
    
    int curPt = curP;
    while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
        
        int nPt = curPt++;

        // same thing as the usual Scan(), just with a hardcoded "indegree+outdegree=2" case, since
        // it's the most common one
        
        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        if ( getPoint(nPt).totalDegree() == 2 ) {
            _countUpDownTotalDegree2(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        } else {
            _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        }
        
        if ( nbDn <= 0 ) {
            upNo = -1;
        }
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
        }

        if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::max(e.st, e.en) ) {
                    if ( cb != upNo ) {
                        SweepTree* node = swrData[cb].misc;
                        if ( node ) {
                            _updateIntersection(cb, nPt);
                            // create trapezoid for the chunk of edge intersecting with the line
                            DestroyEdge(cb, to, line);
                            node->Remove(*sTree, *sEvts, true);
                        }
                    }
                }
                
                cb = NextAt(nPt,cb);
            }
        }

        // traitement du "upNo devient dnNo"
        SweepTree *insertionNode = NULL;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                SweepTree* node = swrData[upNo].misc;
                _updateIntersection(upNo, nPt);
                DestroyEdge(upNo, to, line);
                
                node->ConvertTo(this, dnNo, 1, nPt);
                
                swrData[dnNo].misc = node;
                insertionNode = node;
                CreateEdge(dnNo, to, step);
                swrData[dnNo].guess = swrData[upNo].guess;
            } else {
                SweepTree *node = sTree->add(this, dnNo, 1, nPt, this);
                swrData[dnNo].misc = node;
                node->Insert(*sTree, *sEvts, this, nPt, true);
                insertionNode = node;
                CreateEdge(dnNo, to, step);
            }
        }
        
        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo ) {
                        SweepTree *node = sTree->add(this, cb, 1, nPt, this);
                        swrData[cb].misc = node;
                        node->InsertAt(*sTree, *sEvts, this, insertionNode, nPt, true);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
    }
    
    curP = curPt;
    if ( curPt > 0 ) {
        pos = getPoint(curPt - 1).x[1];
    } else {
        pos = to;
    } 
    
    // update intersections with the sweepline, and add trapezoids for edges crossing the line
    pos = to;
    if ( sTree->racine ) {
        SweepTree* curS = static_cast<SweepTree*>(sTree->racine->Leftmost());
        while ( curS ) {
            int cb = curS->bord;
            AvanceEdge(cb, to, line, exact, step);
            curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
        }
    }
}




void Shape::Scan(float &pos, int &curP, float to, FillRule directed, BitLigne *line, bool exact, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }
    
    if ( pos >= to ) {
        return;
    }
    
    if ( sTree->racine ) {
        int curW = 0;
        float lastX = 0;
        SweepTree* curS = static_cast<SweepTree*>(sTree->racine->Leftmost());

        if ( directed == fill_oddEven ) {

            while ( curS ) {
                int cb = curS->bord;
                curW++;
                curW &= 0x00000001;
                if ( curW == 0 ) {
                    line->AddBord(lastX,swrData[cb].curX,true);
                } else {
                    lastX = swrData[cb].curX;
                }
                curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
            }
            
        } else if ( directed == fill_positive ) {

            // doesn't behave correctly; no way i know to do this without a ConvertToShape()
            while ( curS ) {
                int cb = curS->bord;
              int oW = curW;
              if ( swrData[cb].sens ) {
                  curW++;
              } else {
                  curW--;
              }

              if ( curW <= 0 && oW > 0) {
                  line->AddBord(lastX, swrData[cb].curX, true);
              } else if ( curW > 0 && oW <= 0 ) {
                  lastX = swrData[cb].curX;
              }
              
              curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
            }
          
        } else if ( directed == fill_nonZero ) {

            while ( curS ) {
                int cb = curS->bord;
                int oW = curW;
                if ( swrData[cb].sens ) {
                    curW++;
                } else {
                    curW--;
                }
                
                if ( curW == 0 && oW != 0) {
                    line->AddBord(lastX,swrData[cb].curX,true);
                } else if ( curW != 0 && oW == 0 ) {
                    lastX=swrData[cb].curX;
                }
                curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
            }
        }
        
    }
  
    int curPt = curP;
    while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
        int nPt = curPt++;
        
        int cb;
        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        
        if ( getPoint(nPt).totalDegree() == 2 ) {
            _countUpDownTotalDegree2(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        } else {
            _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        }
        
        if ( nbDn <= 0 ) {
            upNo = -1;
        }
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
        }
        
        if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::max(e.st, e.en) ) {
                    if ( cb != upNo ) {
                        SweepTree* node=swrData[cb].misc;
                        if ( node ) {
                            _updateIntersection(cb, nPt);
                            DestroyEdge(cb, line);
                            node->Remove(*sTree,*sEvts,true);
                        }
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
        
        // traitement du "upNo devient dnNo"
        SweepTree* insertionNode = NULL;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                SweepTree* node = swrData[upNo].misc;
                _updateIntersection(upNo, nPt);
                DestroyEdge(upNo, line);
                
                node->ConvertTo(this, dnNo, 1, nPt);
                
                swrData[dnNo].misc = node;
                insertionNode = node;
                CreateEdge(dnNo, to, step);
                
            } else {
              
                SweepTree* node = sTree->add(this,dnNo,1,nPt,this);
                swrData[dnNo].misc = node;
                node->Insert(*sTree, *sEvts, this, nPt, true);
                insertionNode = node;
                CreateEdge(dnNo, to, step);
            }
        }
        
        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo ) {
                        SweepTree* node = sTree->add(this, cb, 1, nPt, this);
                        swrData[cb].misc = node;
                        node->InsertAt(*sTree, *sEvts, this, insertionNode, nPt, true);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt, cb);
            }
        }
    }
  
    curP = curPt;
    if ( curPt > 0 ) {
        pos = getPoint(curPt - 1).x[1];
    } else {
        pos = to;
    }
    
    pos = to;
    if ( sTree->racine ) {
        SweepTree* curS = static_cast<SweepTree*>(sTree->racine->Leftmost());
        while ( curS ) {
            int cb = curS->bord;
            AvanceEdge(cb, to, line, exact, step);
            curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
        }
    }
}


void Shape::Scan(float &pos, int &curP, float to, AlphaLigne *line, bool exact, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }

    if ( pos >= to ) {
        return;
    }

    int curPt = curP;
    while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
        int nPt = curPt++;

        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        if ( getPoint(nPt).totalDegree() == 2 ) {
            _countUpDownTotalDegree2(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        } else {
            _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        }

        if ( nbDn <= 0 ) {
            upNo=-1;
        }
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo=-1;
        }

        if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::max(e.st, e.en) ) {
                    if ( cb != upNo ) {
                        SweepTree* node = swrData[cb].misc;
                        if ( node ) {
                            _updateIntersection(cb, nPt);
                            DestroyEdge(cb, line);
                            node->Remove(*sTree, *sEvts, true);
                        }
                    }
                }
                
                cb = NextAt(nPt,cb);
            }
        }

        // traitement du "upNo devient dnNo"
        SweepTree* insertionNode = NULL;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                SweepTree* node = swrData[upNo].misc;
                _updateIntersection(upNo, nPt);
                DestroyEdge(upNo, line);

                node->ConvertTo(this, dnNo, 1, nPt);

                swrData[dnNo].misc = node;
                insertionNode = node;
                CreateEdge(dnNo, to, step);
                swrData[dnNo].guess = swrData[upNo].guess;
            } else {
                SweepTree* node = sTree->add(this, dnNo, 1, nPt, this);
                swrData[dnNo].misc = node;
                node->Insert(*sTree, *sEvts, this, nPt, true);
                insertionNode = node;
                CreateEdge(dnNo, to, step);
            }
        }

        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo ) {
                        SweepTree* node = sTree->add(this, cb, 1, nPt, this);
                        swrData[cb].misc = node;
                        node->InsertAt(*sTree, *sEvts, this, insertionNode, nPt, true);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
    }

    curP = curPt;
    if ( curPt > 0 ) {
        pos = getPoint(curPt - 1).x[1];
    } else {
        pos = to;
    }
    
    pos = to;
    if ( sTree->racine ) {
        SweepTree* curS = static_cast<SweepTree*>(sTree->racine->Leftmost());
        while ( curS ) {
            int cb = curS->bord;
            AvanceEdge(cb, to, line, exact, step);
            curS = static_cast<SweepTree*>(curS->elem[RIGHT]);
        }
    }
}



void Shape::QuickScan(float &pos, int &curP, float to, FloatLigne* line, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }
    
    if ( pos >= to ) {
        return;
    }
    
    if ( nbQRas > 1 ) {
        int curW = 0;
        // float lastX = 0;
        // float lastY = 0;
        int lastGuess = -1;
        int lastB = -1;
        
        for (int i = firstQRas; i >= 0 && i < nbQRas; i = qrsData[i].next) {
            int cb = qrsData[i].bord;
            int oW = curW;
            if ( swrData[cb].sens ) {
                curW++;
            } else {
                curW--;
            }

            if ( curW % 2 == 0 && oW % 2 != 0) {

                lastGuess = line->AppendBord(swrData[lastB].curX,
                                             to - swrData[lastB].curY,
                                             swrData[cb].curX,
                                             to - swrData[cb].curY,
                                             0.0);
                
                swrData[cb].guess = lastGuess;
                if ( lastB >= 0 ) {
                    swrData[lastB].guess = lastGuess - 1;
                }
                
            } else if ( curW%2 != 0 && oW%2 == 0 ) {

                // lastX = swrData[cb].curX;
                // lastY = swrData[cb].curY;
                lastB = cb;
                swrData[cb].guess = -1;
                
            } else {
                swrData[cb].guess = -1;
            }
        }
    }

    int curPt = curP;
    while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
        int nPt = curPt++;

        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        if ( getPoint(nPt).totalDegree() == 2 ) {
            _countUpDownTotalDegree2(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        } else {
            _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        }

        if ( nbDn <= 0 ) {
            upNo = -1;
        }
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
        }

        if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::max(e.st, e.en) ) {
                    if ( cb != upNo ) {
                        QuickRasterSubEdge(cb);
                        _updateIntersection(cb, nPt);
                        DestroyEdge(cb, to, line);
                    }
                }
                cb = NextAt(nPt, cb);
            }
        }

        // traitement du "upNo devient dnNo"
        int ins_guess=-1;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                ins_guess = QuickRasterChgEdge(upNo ,dnNo, getPoint(nPt).x[0]);
                _updateIntersection(upNo, nPt);
                DestroyEdge(upNo, to, line);

                CreateEdge(dnNo, to, step);
                swrData[dnNo].guess = swrData[upNo].guess;
            } else {
                ins_guess = QuickRasterAddEdge(dnNo, getPoint(nPt).x[0], ins_guess);
                CreateEdge(dnNo, to, step);
            }
        }
        
        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo ) {
                        ins_guess = QuickRasterAddEdge(cb, getPoint(nPt).x[0], ins_guess);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt, cb);
            }
        }
    }
    
    curP = curPt;
    if ( curPt > 0 ) {
        pos = getPoint(curPt-1).x[1];
    } else {
        pos=to;
    }
    
    pos = to;
    for (int i=0; i < nbQRas; i++) {
        int cb = qrsData[i].bord;
        AvanceEdge(cb, to, line, true, step);
        qrsData[i].x = swrData[cb].curX;
    }
    
    QuickRasterSort();
}




void Shape::QuickScan(float &pos, int &curP, float to, FillRule directed, BitLigne* line, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }

    if ( pos >= to ) {
        return;
    }
    
    if ( nbQRas > 1 ) {
        int curW = 0;
        float lastX = 0;

        if ( directed == fill_oddEven ) {

            for (int i = firstQRas; i >= 0 && i < nbQRas; i = qrsData[i].next) {
                int cb = qrsData[i].bord;
                curW++;
                curW &= 1;
                if ( curW == 0 ) {
                    line->AddBord(lastX, swrData[cb].curX, true);
                } else {
                    lastX = swrData[cb].curX;
                }
            }

        } else if ( directed == fill_positive ) {
            // doesn't behave correctly; no way i know to do this without a ConvertToShape()
            for (int i = firstQRas; i >= 0 && i < nbQRas; i = qrsData[i].next) {
                int cb = qrsData[i].bord;
                int oW = curW;
                if ( swrData[cb].sens ) {
                    curW++;
                } else {
                    curW--;
                }

                if ( curW <= 0 && oW > 0) {
                    line->AddBord(lastX, swrData[cb].curX, true);
                } else if ( curW > 0 && oW <= 0 ) {
                    lastX = swrData[cb].curX;
                }
            }

        } else if ( directed == fill_nonZero ) {
            for (int i = firstQRas; i >= 0 && i < nbQRas; i = qrsData[i].next) {
                int cb = qrsData[i].bord;
                int oW = curW;
                if ( swrData[cb].sens ) {
                    curW++;
                } else {
                    curW--;
                }

                if ( curW == 0 && oW != 0) {
                    line->AddBord(lastX, swrData[cb].curX, true);
                } else if ( curW != 0 && oW == 0 ) {
                    lastX = swrData[cb].curX;
                }
            }
        }
    }

    int curPt = curP;
    while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
        int nPt = curPt++;
        
        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        if ( getPoint(nPt).totalDegree() == 2 ) {
            _countUpDownTotalDegree2(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        } else {
            _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        }

        if ( nbDn <= 0 ) {
            upNo = -1;
        }
        
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
        }

        if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::max(e.st, e.en) ) {
                    if ( cb != upNo ) {
                        QuickRasterSubEdge(cb);
                        _updateIntersection(cb, nPt);
                        DestroyEdge(cb, line);
                    }
                }
                cb = NextAt(nPt, cb);
            }
        }
        
        // traitement du "upNo devient dnNo"
        int ins_guess = -1;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                ins_guess = QuickRasterChgEdge(upNo, dnNo, getPoint(nPt).x[0]);
                _updateIntersection(upNo, nPt);
                DestroyEdge(upNo, line);
                
                CreateEdge(dnNo, to, step);
            } else {
                ins_guess = QuickRasterAddEdge(dnNo, getPoint(nPt).x[0], ins_guess);
                CreateEdge(dnNo, to, step);
            }
        }

        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo ) {
                        ins_guess = QuickRasterAddEdge(cb, getPoint(nPt).x[0], ins_guess);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
    }
    
    curP = curPt;
    if ( curPt > 0 ) {
        pos=getPoint(curPt - 1).x[1];
    } else {
        pos = to;
    }
    
    pos = to;
    for (int i = 0; i < nbQRas; i++) {
        int cb = qrsData[i].bord;
        AvanceEdge(cb, to, line, true, step);
        qrsData[i].x = swrData[cb].curX;
    }
    
    QuickRasterSort();
}



void Shape::QuickScan(float &pos, int &curP, float to, AlphaLigne* line, float step)
{
    if ( numberOfEdges() <= 1 ) {
        return;
    }
    if ( pos >= to ) {
        return;
    }
    
    int curPt = curP;
    while ( curPt < numberOfPoints() && getPoint(curPt).x[1] <= to ) {
        int nPt = curPt++;

        int nbUp;
        int nbDn;
        int upNo;
        int dnNo;
        if ( getPoint(nPt).totalDegree() == 2 ) {
            _countUpDownTotalDegree2(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        } else {
            _countUpDown(nPt, &nbUp, &nbDn, &upNo, &dnNo);
        }
        
        if ( nbDn <= 0 ) {
            upNo = -1;
        }
        if ( upNo >= 0 && swrData[upNo].misc == NULL ) {
            upNo = -1;
        }

        if ( nbUp > 1 || ( nbUp == 1 && upNo < 0 ) ) {
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::max(e.st, e.en) ) {
                    if ( cb != upNo ) {
                        QuickRasterSubEdge(cb);
                        _updateIntersection(cb, nPt);
                        DestroyEdge(cb, line);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }

        // traitement du "upNo devient dnNo"
        int ins_guess = -1;
        if ( dnNo >= 0 ) {
            if ( upNo >= 0 ) {
                ins_guess = QuickRasterChgEdge(upNo, dnNo, getPoint(nPt).x[0]);
                _updateIntersection(upNo, nPt);
                DestroyEdge(upNo, line);

                CreateEdge(dnNo, to, step);
                swrData[dnNo].guess = swrData[upNo].guess;
            } else {
                ins_guess = QuickRasterAddEdge(dnNo, getPoint(nPt).x[0], ins_guess);
                CreateEdge(dnNo, to, step);
            }
        }

        if ( nbDn > 1 ) { // si nbDn == 1 , alors dnNo a deja ete traite
            int cb = getPoint(nPt).incidentEdge[FIRST];
            while ( cb >= 0 && cb < numberOfEdges() ) {
                Shape::dg_arete const &e = getEdge(cb);
                if ( nPt == std::min(e.st, e.en) ) {
                    if ( cb != dnNo ) {
                        ins_guess = QuickRasterAddEdge(cb,getPoint(nPt).x[0], ins_guess);
                        CreateEdge(cb, to, step);
                    }
                }
                cb = NextAt(nPt,cb);
            }
        }
    }

    curP = curPt;
    if ( curPt > 0 ) {
        pos = getPoint(curPt-1).x[1];
    } else {
        pos = to;
    }
    
    pos = to;
    for (int i = 0; i < nbQRas; i++) {
        int cb = qrsData[i].bord;
        AvanceEdge(cb, to, line, true, step);
        qrsData[i].x = swrData[cb].curX;
    }
    
    QuickRasterSort();
}


/*
 * operations de bases pour la rasterization
 *
 */
void Shape::CreateEdge(int no, float to, float step)
{
    int cPt;
    Geom::Point dir;
    if ( getEdge(no).st < getEdge(no).en ) {
        cPt = getEdge(no).st;
        swrData[no].sens = true;
        dir = getEdge(no).dx;
    } else {
        cPt = getEdge(no).en;
        swrData[no].sens = false;
        dir = -getEdge(no).dx;
    }

    swrData[no].lastX = swrData[no].curX = getPoint(cPt).x[0];
    swrData[no].lastY = swrData[no].curY = getPoint(cPt).x[1];
    
    if ( fabs(dir[1]) < 0.000001 ) {
        swrData[no].dxdy = 0;
    } else {
        swrData[no].dxdy = dir[0]/dir[1];
    }
    
    if ( fabs(dir[0]) < 0.000001 ) {
        swrData[no].dydx = 0;
    } else {
        swrData[no].dydx = dir[1]/dir[0];
    }
    
    swrData[no].calcX = swrData[no].curX + (to - step - swrData[no].curY) * swrData[no].dxdy;
    swrData[no].guess = -1;
}


void Shape::AvanceEdge(int no, float to, bool exact, float step)
{
    if ( exact ) {
        Geom::Point dir;
        Geom::Point stp;
        if ( swrData[no].sens ) {
            stp = getPoint(getEdge(no).st).x;
            dir = getEdge(no).dx;
        } else {
            stp = getPoint(getEdge(no).en).x;
            dir = -getEdge(no).dx;
        }
        
        if ( fabs(dir[1]) < 0.000001 ) {
            swrData[no].calcX = stp[0] + dir[0];
        } else {
            swrData[no].calcX = stp[0] + ((to - stp[1]) * dir[0]) / dir[1];
        }
    } else {
        swrData[no].calcX += step * swrData[no].dxdy;
    }
    
    swrData[no].lastX = swrData[no].curX;
    swrData[no].lastY = swrData[no].curY;
    swrData[no].curX = swrData[no].calcX;
    swrData[no].curY = to;
}

/*
 * specialisation par type de structure utilise
 */

void Shape::DestroyEdge(int no, float to, FloatLigne* line)
{
    if ( swrData[no].sens ) {

        if ( swrData[no].curX < swrData[no].lastX ) {

            swrData[no].guess = line->AddBordR(swrData[no].curX,
                                               to - swrData[no].curY,
                                               swrData[no].lastX,
                                               to - swrData[no].lastY,
                                               -swrData[no].dydx,
                                               swrData[no].guess);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            swrData[no].guess = line->AddBord(swrData[no].lastX,
                                              -(to - swrData[no].lastY),
                                              swrData[no].curX,
                                              -(to - swrData[no].curY),
                                              swrData[no].dydx,
                                              swrData[no].guess);
        }
        
    } else {
        
        if ( swrData[no].curX < swrData[no].lastX ) {

            swrData[no].guess = line->AddBordR(swrData[no].curX,
                                               -(to - swrData[no].curY),
                                               swrData[no].lastX,
                                               -(to - swrData[no].lastY),
                                               swrData[no].dydx,
                                               swrData[no].guess);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            swrData[no].guess = line->AddBord(swrData[no].lastX,
                                              to - swrData[no].lastY,
                                              swrData[no].curX,
                                              to - swrData[no].curY,
                                              -swrData[no].dydx,
                                              swrData[no].guess);
        }
    }
}



void Shape::AvanceEdge(int no, float to, FloatLigne *line, bool exact, float step)
{
    AvanceEdge(no,to,exact,step);

    if ( swrData[no].sens ) {
        
        if ( swrData[no].curX < swrData[no].lastX ) {
            
            swrData[no].guess = line->AddBordR(swrData[no].curX,
                                               to - swrData[no].curY,
                                               swrData[no].lastX,
                                               to - swrData[no].lastY,
                                               -swrData[no].dydx,
                                               swrData[no].guess);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            swrData[no].guess = line->AddBord(swrData[no].lastX,
                                              -(to - swrData[no].lastY),
                                              swrData[no].curX,
                                              -(to - swrData[no].curY),
                                              swrData[no].dydx,
                                              swrData[no].guess);
        }
        
    } else {

        if ( swrData[no].curX < swrData[no].lastX ) {

            swrData[no].guess = line->AddBordR(swrData[no].curX,
                                               -(to - swrData[no].curY),
                                               swrData[no].lastX,
                                               -(to - swrData[no].lastY),
                                               swrData[no].dydx,
                                               swrData[no].guess);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            swrData[no].guess = line->AddBord(swrData[no].lastX,
                                              to - swrData[no].lastY,
                                              swrData[no].curX,
                                              to - swrData[no].curY,
                                              -swrData[no].dydx,
                                              swrData[no].guess);
        }
    }
}


void Shape::DestroyEdge(int no, BitLigne *line)
{
    if ( swrData[no].sens ) {
        
        if ( swrData[no].curX < swrData[no].lastX ) {
            
            line->AddBord(swrData[no].curX, swrData[no].lastX, false);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX,swrData[no].curX,false);
        }
        
    } else {

	if ( swrData[no].curX < swrData[no].lastX ) {
            
            line->AddBord(swrData[no].curX, swrData[no].lastX, false);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX, swrData[no].curX, false);
            
        }
    }
}


void Shape::AvanceEdge(int no, float to, BitLigne *line, bool exact, float step)
{
    AvanceEdge(no, to, exact, step);

    if ( swrData[no].sens ) {
        
        if ( swrData[no].curX < swrData[no].lastX ) {
            
            line->AddBord(swrData[no].curX, swrData[no].lastX, false);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX, swrData[no].curX, false);
        }

    } else {
        
        if ( swrData[no].curX < swrData[no].lastX ) {
            
            line->AddBord(swrData[no].curX, swrData[no].lastX, false);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX, swrData[no].curX, false);
        }
    }
}


void Shape::DestroyEdge(int no, AlphaLigne* line)
{
    if ( swrData[no].sens ) {
        
        if ( swrData[no].curX <= swrData[no].lastX ) {

            line->AddBord(swrData[no].curX,
                          0,
                          swrData[no].lastX,
                          swrData[no].curY - swrData[no].lastY,
                          -swrData[no].dydx);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX,
                          0,
                          swrData[no].curX,
                          swrData[no].curY - swrData[no].lastY,
                          swrData[no].dydx);
        }
        
    } else {
        
        if ( swrData[no].curX <= swrData[no].lastX ) {

            line->AddBord(swrData[no].curX,
                          0,
                          swrData[no].lastX,
                          swrData[no].lastY - swrData[no].curY,
                          swrData[no].dydx);

        } else if ( swrData[no].curX > swrData[no].lastX ) {

            line->AddBord(swrData[no].lastX,
                          0,
                          swrData[no].curX,
                          swrData[no].lastY - swrData[no].curY,
                          -swrData[no].dydx);
        }
    }
}


void Shape::AvanceEdge(int no, float to, AlphaLigne *line, bool exact, float step)
{
    AvanceEdge(no,to,exact,step);

    if ( swrData[no].sens ) {
        
        if ( swrData[no].curX <= swrData[no].lastX ) {
            
            line->AddBord(swrData[no].curX,
                          0,
                          swrData[no].lastX,
                          swrData[no].curY - swrData[no].lastY,
                          -swrData[no].dydx);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX,
                          0,
                          swrData[no].curX,
                          swrData[no].curY - swrData[no].lastY,
                          swrData[no].dydx);
        }
        
    } else {
        
        if ( swrData[no].curX <= swrData[no].lastX ) {
            
            line->AddBord(swrData[no].curX,
                          0,
                          swrData[no].lastX,
                          swrData[no].lastY - swrData[no].curY,
                          swrData[no].dydx);
            
        } else if ( swrData[no].curX > swrData[no].lastX ) {
            
            line->AddBord(swrData[no].lastX,
                          0,
                          swrData[no].curX,
                          swrData[no].lastY - swrData[no].curY,
                          -swrData[no].dydx);
        }
    }
}

/**
 *    \param P point index.
 *    \param numberUp Filled in with the number of edges coming into P from above.
 *    \param numberDown Filled in with the number of edges coming exiting P to go below.
 *    \param upEdge One of the numberUp edges, or -1.
 *    \param downEdge One of the numberDown edges, or -1.
 */

void Shape::_countUpDown(int P, int *numberUp, int *numberDown, int *upEdge, int *downEdge) const
{
    *numberUp = 0;
    *numberDown = 0;
    *upEdge = -1;
    *downEdge = -1;
    
    int i = getPoint(P).incidentEdge[FIRST];
    
    while ( i >= 0 && i < numberOfEdges() ) {
        Shape::dg_arete const &e = getEdge(i);
        if ( P == std::max(e.st, e.en) ) {
            *upEdge = i;
            (*numberUp)++;
        }
        if ( P == std::min(e.st, e.en) ) {
            *downEdge = i;
            (*numberDown)++;
        }
        i = NextAt(P, i);
    }
    
}



/**
 *     Version of Shape::_countUpDown optimised for the case when getPoint(P).totalDegree() == 2.
 */

void Shape::_countUpDownTotalDegree2(int P,
                                     int *numberUp, int *numberDown, int *upEdge, int *downEdge) const
{
    *numberUp = 0;
    *numberDown = 0;
    *upEdge = -1;
    *downEdge = -1;
    
    for (int i = 0; i < 2; i++) {
        int const j = getPoint(P).incidentEdge[i];
        Shape::dg_arete const &e = getEdge(j);
        if ( P == std::max(e.st, e.en) ) {
            *upEdge = j;
            (*numberUp)++;
        }
        if ( P == std::min(e.st, e.en) ) {
            *downEdge = j;
            (*numberDown)++;
        }
    }
}


void Shape::_updateIntersection(int e, int p)
{
    swrData[e].lastX = swrData[e].curX;
    swrData[e].lastY = swrData[e].curY;
    swrData[e].curX = getPoint(p).x[0];
    swrData[e].curY = getPoint(p).x[1];
    swrData[e].misc = NULL;
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

/**
 *  \file livarot/float-line.cpp
 *
 * Implementation of coverage with floating-point values.
 *
 *  \author Fred
 *
 *  public domain
 *
 */

#ifdef faster_flatten
# include <cmath>  // std::abs(float)
#endif
#include <stdio.h>
#include "livarot/float-line.h"
#include "livarot/int-line.h"
#include <cstdio>

FloatLigne::FloatLigne()
{
    s_first = s_last = -1;
}


FloatLigne::~FloatLigne()
{
}

/// Reset the line to  empty (boundaries and runs).
void FloatLigne::Reset()
{
    bords.clear();
    runs.clear();
    s_first = s_last = -1;
}

/**
 * Add a coverage portion.
 *
 * \param guess Position from where we should try to insert the first 
 * boundary, or -1 if we don't have a clue.
 */
int FloatLigne::AddBord(float spos, float sval, float epos, float eval, int guess)
{
//  if ( showCopy ) printf("b= %f %f -> %f %f \n",spos,sval,epos,eval);
    if ( spos >= epos ) {
        return -1;
    }

    float pente = (eval - sval) / (epos - spos);
    
#ifdef faster_flatten
    if ( std::abs(epos - spos) < 0.001 || std::abs(pente) > 1000 ) {
        return -1;
        epos = spos;
        pente = 0;
    }
#endif
  
    if ( guess >= int(bords.size()) ) {
        guess = -1;
    }
    
    // add the left boundary
    float_ligne_bord b;
    int n = bords.size();
    b.pos = spos;
    b.val = sval;
    b.start = true;
    b.other = n + 1;
    b.pente = pente;
    b.s_prev = b.s_next = -1;
    bords.push_back(b);

    // insert it in the doubly-linked list
    InsertBord(n, spos, guess);
    
    // add the right boundary
    n = bords.size();
    b.pos = epos;
    b.val = eval;
    b.start = false;
    b.other = n-1;
    b.pente = pente;
    b.s_prev = b.s_next = -1;
    bords.push_back(b);
    
    // insert it in the doubly-linked list, knowing that boundary at index n-1 is not too far before me
    InsertBord(n, epos, n - 1);
  	
    return n;
}

/**
 * Add a coverage portion.
 *
 * \param guess Position from where we should try to insert the first 
 * boundary, or -1 if we don't have a clue.
 */
int FloatLigne::AddBord(float spos, float sval, float epos, float eval, float pente, int guess)
{
//  if ( showCopy ) printf("b= %f %f -> %f %f \n",spos,sval,epos,eval);
    if ( spos >= epos ) {
        return -1;
    }

#ifdef faster_flatten
    if ( std::abs(epos - spos) < 0.001 || std::abs(pente) > 1000 ) {
        return -1;
        epos = spos;
        pente = 0;
    }
#endif
    
    if ( guess >= int(bords.size()) ) {
        guess=-1;
    }

    float_ligne_bord b;
    int n = bords.size();
    b.pos = spos;
    b.val = sval;
    b.start = true;
    b.other = n + 1;
    b.pente = pente;
    b.s_prev = b.s_next = -1;
    bords.push_back(b);

    n = bords.size();
    b.pos = epos;
    b.val = eval;
    b.start = false;
    b.other = n - 1;
    b.pente = pente;
    b.s_prev = b.s_next = -1;
    bords.push_back(b);

    InsertBord(n - 1, spos, guess);
    InsertBord(n, epos, n - 1);
/*	if ( bords[n-1].s_next < 0 ) {
		bords[n].s_next=-1;
		s_last=n;

		bords[n].s_prev=n-1;
		bords[n-1].s_next=n;
	} else if ( bords[bords[n-1].s_next].pos >= epos ) {
		bords[n].s_next=bords[n-1].s_next;
		bords[bords[n].s_next].s_prev=n;
		
		bords[n].s_prev=n-1;
		bords[n-1].s_next=n;
	} else {
		int c=bords[bords[n-1].s_next].s_next;
		while ( c >= 0 && bords[c].pos < epos ) c=bords[c].s_next;
		if ( c < 0 ) {
			bords[n].s_prev=s_last;
			bords[s_last].s_next=n;
			s_last=n;
		} else {
			bords[n].s_prev=bords[c].s_prev;
			bords[bords[n].s_prev].s_next=n;

			bords[n].s_next=c;
			bords[c].s_prev=n;
		}

	}*/
    return n;
}

/**
 * Add a coverage portion.
 *
 * \param guess Position from where we should try to insert the last
 * boundary, or -1 if we don't have a clue.
 */
int FloatLigne::AddBordR(float spos, float sval, float epos, float eval, float pente, int guess)
{
//  if ( showCopy ) printf("br= %f %f -> %f %f \n",spos,sval,epos,eval);
//	return AddBord(spos,sval,epos,eval,pente,guess);
    if ( spos >= epos ){
        return -1;
    }
    
#ifdef faster_flatten
    if ( std::abs(epos - spos) < 0.001 || std::abs(pente) > 1000 ) {
        return -1;
        epos = spos;
        pente = 0;
    }
#endif

    if ( guess >= int(bords.size()) ) {
        guess=-1;
    }

    float_ligne_bord b;
    int n = bords.size();
    b.pos = spos;
    b.val = sval;
    b.start = true;
    b.other = n + 1;
    b.pente = pente;
    b.s_prev = b.s_next = -1;
    bords.push_back(b);
    
    n = bords.size();
    b.pos = epos;
    b.val = eval;
    b.start = false;
    b.other = n - 1;
    b.pente = pente;
    b.s_prev = b.s_next = -1;
    bords.push_back(b);
    
    InsertBord(n, epos, guess);
    InsertBord(n - 1, spos, n);
    
/*	if ( bords[n].s_prev < 0 ) {
		bords[n-1].s_prev=-1;
		s_first=n-1;

		bords[n-1].s_next=n;
		bords[n].s_prev=n-1;
	} else if ( bords[bords[n].s_prev].pos <= spos ) {
		bords[n-1].s_prev=bords[n].s_prev;
		bords[bords[n-1].s_prev].s_next=n-1;

		bords[n-1].s_next=n;
		bords[n].s_prev=n-1;
	} else {
		int c=bords[bords[n].s_prev].s_prev;
		while ( c >= 0 && bords[c].pos > spos ) c=bords[c].s_prev;
		if ( c < 0 ) {
			bords[n-1].s_next=s_first;
			bords[s_first].s_prev=n-1;
			s_first=n-1;
		} else {
			bords[n-1].s_next=bords[c].s_next;
			bords[bords[n-1].s_next].s_prev=n-1;

			bords[n-1].s_prev=c;
			bords[c].s_next=n-1;
		}
		
                }*/
    return n - 1;
}

/**
 * Add a coverage portion by appending boundaries at the end of the list.
 *
 * This works because we know they are on the right.
 */
int FloatLigne::AppendBord(float spos, float sval, float epos, float eval, float pente)
{
//  if ( showCopy ) printf("b= %f %f -> %f %f \n",spos,sval,epos,eval);
//	return AddBord(spos,sval,epos,eval,pente,s_last);
    if ( spos >= epos ) {
        return -1;
    }
    
#ifdef faster_flatten
    if ( std::abs(epos - spos) < 0.001 || std::abs(pente) > 1000 ) {
        return -1;
        epos = spos;
        pente = 0;
    }
#endif
    
    int n = bords.size();
    float_ligne_bord b;
    b.pos = spos;
    b.val = sval;
    b.start = true;
    b.other = n + 1;
    b.pente = pente;
    b.s_prev = s_last;
    b.s_next = n + 1;
    bords.push_back(b);
 
    if ( s_last >=  0 ) {
        bords[s_last].s_next = n;
    }
    
    if ( s_first < 0 ) {
        s_first = n;
    }

    n = bords.size();
    b.pos = epos;
    b.val = eval;
    b.start = false;
    b.other = n - 1;
    b.pente = pente;
    b.s_prev = n - 1;
    b.s_next = -1;
    bords.push_back(b);

    s_last = n;

    return n;
}



// insertion in a boubly-linked list. nothing interesting here
void FloatLigne::InsertBord(int no, float /*p*/, int guess)
{
// TODO check if ignoring p is bad
    if ( no < 0 || no >= int(bords.size()) ) {
        return;
    }
    
    if ( s_first < 0 ) {
        s_first = s_last = no;
        bords[no].s_prev = -1;
        bords[no].s_next = -1;
        return;
    }
    
    if ( guess < 0 || guess >= int(bords.size()) ) {
        int c = s_first;
        while ( c >= 0 && c < int(bords.size()) && CmpBord(bords[c], bords[no]) < 0 ) {
            c = bords[c].s_next;
        }
        
        if ( c < 0 || c >= int(bords.size()) ) {
            bords[no].s_prev = s_last;
            bords[s_last].s_next = no;
            s_last = no;
        } else {
            bords[no].s_prev = bords[c].s_prev;
            if ( bords[no].s_prev >= 0 ) {
                bords[bords[no].s_prev].s_next = no;
            } else {
                s_first = no;
            }
            bords[no].s_next = c;
            bords[c].s_prev = no;
        }
    } else {
        int c = guess;
        int stTst = CmpBord(bords[c], bords[no]);

        if ( stTst == 0 ) {

            bords[no].s_prev = bords[c].s_prev;
            if ( bords[no].s_prev >= 0 ) {
                bords[bords[no].s_prev].s_next = no;
            } else {
                s_first = no;
            }
            bords[no].s_next = c;
            bords[c].s_prev = no;
            
        } else if ( stTst > 0 ) {
            
            while ( c >= 0 && c < int(bords.size()) && CmpBord(bords[c], bords[no]) > 0 ) {
                c = bords[c].s_prev;
            }
            
            if ( c < 0 || c >= int(bords.size()) ) {
                bords[no].s_next = s_first;
                bords[s_first].s_prev =no; // s_first != -1
                s_first = no; 
            } else {
                bords[no].s_next = bords[c].s_next;
                if ( bords[no].s_next >= 0 ) {
                    bords[bords[no].s_next].s_prev = no;
                } else {
                    s_last = no;
                }
                bords[no].s_prev = c;
                bords[c].s_next = no;
            }
            
        } else {

            while ( c >= 0 && c < int(bords.size()) && CmpBord(bords[c],bords[no]) < 0 ) {
                c = bords[c].s_next;
            }
            
            if ( c < 0 || c >= int(bords.size()) ) {
                bords[no].s_prev = s_last;
                bords[s_last].s_next = no;
                s_last = no;
            } else {
                bords[no].s_prev = bords[c].s_prev;
                if ( bords[no].s_prev >= 0 ) {
                    bords[bords[no].s_prev].s_next = no;
                } else {
                    s_first = no;
                }
                bords[no].s_next = c;
                bords[c].s_prev = no;
            }
        }
    }
}

/**
 * Computes the sum of the coverages of the runs currently being scanned, 
 * of which there are "pending".
 */
float FloatLigne::RemainingValAt(float at, int pending)
{
    float sum = 0;
/*	int     no=firstAc;
	while ( no >= 0 && no < bords.size() ) {
		int   nn=bords[no].other;
		sum+=bords[nn].val+(at-bords[nn].pos)*bords[nn].pente;
//				sum+=((at-bords[nn].pos)*bords[no].val+(bords[no].pos-at)*bords[nn].val)/(bords[no].pos-bords[nn].pos);
//		sum+=ValAt(at,bords[nn].pos,bords[no].pos,bords[nn].val,bords[no].val);
		no=bords[no].next;
	}*/
  // for each portion being scanned, compute coverage at position "at" and sum.
  // we could simply compute the sum of portion coverages as a "f(x)=ux+y" and evaluate it at "x=at",
  // but there are numerical problems with this approach, and it produces ugly lines of incorrectly 
  // computed alpha values, so i reverted to this "safe but slow" version
    
    for (int i=0; i < pending; i++) {
        int const nn = bords[i].pend_ind;
        sum += bords[nn].val + (at - bords[nn].pos) * bords[nn].pente;
    }
    
    return sum;
}


/**
 * Extract a set of non-overlapping runs from the boundaries.
 *
 * We scan the boundaries left to right, maintaining a set of coverage 
 * portions currently being scanned. For each such portion, the function 
 * will add the index of its first boundary in an array; but instead of 
 * allocating another array, it uses a field in float_ligne_bord: pend_ind.
 * The outcome is that an array of float_ligne_run is produced.
 */
void FloatLigne::Flatten()
{
    if ( int(bords.size()) <= 1 ) {
        Reset();
        return;
    }
    
    runs.clear();

//	qsort(bords,bords.size(),sizeof(float_ligne_bord),FloatLigne::CmpBord);
//	SortBords(0,bords.size()-1);
  
    float totPente = 0;
    float totStart = 0;
    float totX = bords[0].pos;
    
    bool startExists = false;
    float lastStart = 0;
    float lastVal = 0;
    int pending = 0;
    
//	for (int i=0;i<bords.size();) {
    // read the list from left to right, adding a run for each boundary crossed, minus runs with alpha=0
    for (int i=/*0*/s_first; i>=0 && i < int(bords.size()) ;) {
        
        float cur = bords[i].pos;  // position of the current boundary (there may be several boundaries at this position)
        float leftV = 0;  // deltas in coverage value at this position
        float rightV = 0;
        float leftP = 0; // deltas in coverage increase per unit length at this position
        float rightP = 0;
        
        // more precisely, leftV is the sum of decreases of coverage value,
        // while rightV is the sum of increases, so that leftV+rightV is the delta.
        // idem for leftP and rightP
    
        // start by scanning all boundaries that end a portion at this position
        while ( i >= 0 && i < int(bords.size()) && bords[i].pos == cur && bords[i].start == false ) {
            leftV += bords[i].val;
            leftP += bords[i].pente;
            
#ifndef faster_flatten
            // we need to remove the boundary that started this coverage portion for the pending list
            if ( bords[i].other >= 0 && bords[i].other < int(bords.size()) ) {
                // so we use the pend_inv "array"
                int const k = bords[bords[i].other].pend_inv;
                if ( k >= 0 && k < pending ) {
                    // and update the pend_ind array and its inverse pend_inv
                    bords[k].pend_ind = bords[pending - 1].pend_ind;
                    bords[bords[k].pend_ind].pend_inv = k;
                }
            }
#endif
            
            // one less portion pending
            pending--;
            // and we move to the next boundary in the doubly linked list
            i=bords[i].s_next;
            //i++;
        }
        
        // then scan all boundaries that start a portion at this position
        while ( i >= 0 && i < int(bords.size()) && bords[i].pos == cur && bords[i].start == true ) {
            rightV += bords[i].val;
            rightP += bords[i].pente;
#ifndef faster_flatten
            bords[pending].pend_ind=i;
            bords[i].pend_inv=pending;
#endif
            pending++;
            i = bords[i].s_next;
            //i++;
        }

        // coverage value at end of the run will be "start coverage"+"delta per unit length"*"length"
        totStart = totStart + totPente * (cur - totX);
    
        if ( startExists ) {
            // add that run
            AddRun(lastStart, cur, lastVal, totStart, totPente);
        }
        // update "delta coverage per unit length"
        totPente += rightP - leftP;
        // not really needed here
        totStart += rightV - leftV;
        // update position
        totX = cur;
        if ( pending > 0 ) {
            startExists = true;
            
#ifndef faster_flatten
            // to avoid accumulation of numerical errors, we compute an accurate coverage for this position "cur"
            totStart = RemainingValAt(cur, pending);
#endif
            lastVal = totStart;
            lastStart = cur;
        } else {
            startExists = false;
            totStart = 0;
            totPente = 0;
        }
    }
}


/// Debug dump of the instance.
void FloatLigne::Affiche()
{
    printf("%lu : \n", (long unsigned int) bords.size());
    for (int i = 0; i < int(bords.size()); i++) {
        printf("(%f %f %f %i) ",bords[i].pos,bords[i].val,bords[i].pente,(bords[i].start?1:0)); // localization ok
    }
    
    printf("\n");
    printf("%lu : \n", (long unsigned int) runs.size());
    
    for (int i = 0; i < int(runs.size()); i++) {
        printf("(%f %f -> %f %f / %f)",
               runs[i].st, runs[i].vst, runs[i].en, runs[i].ven, runs[i].pente); // localization ok
    }
    
    printf("\n");
}


int FloatLigne::AddRun(float st, float en, float vst, float ven)
{
    return AddRun(st, en, vst, ven, (ven - vst) / (en - st));
}


int FloatLigne::AddRun(float st, float en, float vst, float ven, float pente)
{
    if ( st >= en ) {
        return -1;
    }

    int const n = runs.size();
    float_ligne_run r;
    r.st = st;
    r.en = en;
    r.vst = vst;
    r.ven = ven;
    r.pente = pente;
    runs.push_back(r);
    
    return n;
}

void FloatLigne::Copy(FloatLigne *a)
{
    if ( a->runs.empty() ) {
        Reset();
        return;
    }
    
    bords.clear();
    runs = a->runs;
}

void FloatLigne::Copy(IntLigne *a)
{
    if ( a->nbRun ) {
        Reset();
        return;
    }
    
    bords.clear();
    runs.resize(a->nbRun);
    
    for (int i = 0; i < int(runs.size()); i++) {
        runs[i].st = a->runs[i].st;
        runs[i].en = a->runs[i].en;
        runs[i].vst = a->runs[i].vst;
        runs[i].ven = a->runs[i].ven;
    }
}

/// Cuts the parts having less than tresh coverage.
void FloatLigne::Min(FloatLigne *a, float tresh, bool addIt)
{
    Reset();
    if ( a->runs.empty() ) {
        return;
    }

    bool startExists = false;
    float lastStart=0;
    float lastEnd = 0;
    
    for (int i = 0; i < int(a->runs.size()); i++) {
        float_ligne_run runA = a->runs[i];
        if ( runA.vst <= tresh ) {
            if ( runA.ven <= tresh ) {
                if ( startExists ) {
                    if ( lastEnd >= runA.st - 0.00001 ) {
                        lastEnd = runA.en;
                    } else {
                        if ( addIt ) {
                            AddRun(lastStart, lastEnd, tresh, tresh);
                        }
                        lastStart = runA.st;
                        lastEnd = runA.en;
                    }
                } else {
                    lastStart = runA.st;
                    lastEnd = runA.en;
                }
                startExists = true;
            } else {
                float cutPos = (runA.st * (tresh - runA.ven) + runA.en * (runA.vst - tresh)) / (runA.vst - runA.ven);
                if ( startExists ) {
                    if ( lastEnd >= runA.st - 0.00001 ) {
                        if ( addIt ) {
                            AddRun(lastStart, cutPos, tresh, tresh);
                        }
                        AddRun(cutPos,runA.en, tresh, runA.ven);
                    } else {
                        if ( addIt ) {
                            AddRun(lastStart, lastEnd, tresh, tresh);
                        }
                        if ( addIt ) {
                            AddRun(runA.st, cutPos, tresh, tresh);
                        }
                        AddRun(cutPos, runA.en, tresh, runA.ven);
                    }
                } else {
                    if ( addIt ) {
                        AddRun(runA.st, cutPos, tresh, tresh);
                    }
                    AddRun(cutPos, runA.en, tresh, runA.ven);
                }
                startExists = false;
            }
            
        } else {
            
            if ( runA.ven <= tresh ) {
                float cutPos = (runA.st * (runA.ven - tresh) + runA.en * (tresh - runA.vst)) / (runA.ven - runA.vst);
                if ( startExists ) {
                    if ( addIt ) {
                        AddRun(lastStart, lastEnd, tresh, tresh);
                    }
                }
                AddRun(runA.st, cutPos, runA.vst, tresh);
                startExists = true;
                lastStart = cutPos;
                lastEnd = runA.en;
            } else {
                if ( startExists ) {
                    if ( addIt ) {
                        AddRun(lastStart, lastEnd, tresh, tresh);
                    }
                }
                startExists = false;
                AddRun(runA.st, runA.en, runA.vst, runA.ven);
            }
        }
    }
    
    if ( startExists ) {
        if ( addIt ) {
            AddRun(lastStart, lastEnd, tresh, tresh);
        }
    }
}

/** 
 * Cuts the coverage a in 2 parts.
 *
 * over will receive the parts where coverage > tresh, while the present
 * FloatLigne will receive the parts where coverage <= tresh.
 */
void FloatLigne::Split(FloatLigne *a, float tresh, FloatLigne *over)
{
    Reset();
    if ( a->runs.empty() ) {
        return;
    }

    for (int i = 0; i < int(a->runs.size()); i++) {
        float_ligne_run runA = a->runs[i];
        if ( runA.vst >= tresh ) {
            if ( runA.ven >= tresh ) {
                if ( over ) {
                    over->AddRun(runA.st, runA.en, runA.vst, runA.ven);
                }
            } else {
                float cutPos = (runA.st * (tresh - runA.ven) + runA.en * (runA.vst - tresh)) / (runA.vst - runA.ven);
                if ( over ) {
                    over->AddRun(runA.st, cutPos, runA.vst, tresh);
                }
                AddRun(cutPos, runA.en, tresh, runA.ven);
            }
        } else {
            if ( runA.ven >= tresh ) {
                float cutPos = (runA.st * (runA.ven - tresh) + runA.en * (tresh-runA.vst)) / (runA.ven - runA.vst);
                AddRun(runA.st, cutPos, runA.vst, tresh);
                if ( over ) {
                    over->AddRun(cutPos, runA.en, tresh, runA.ven);
                }
            } else {
                AddRun(runA.st, runA.en, runA.vst, runA.ven);
            }
        }
    }
}

/** 
 * Clips the coverage runs to tresh.
 *
 * If addIt == false, it only leaves the parts that are not entirely under 
 * tresh. If addIt == true, it's the coverage clamped to tresh.
 */
void FloatLigne::Max(FloatLigne *a, float tresh, bool addIt)
{
    Reset();
    if ( a->runs.empty() <= 0 ) {
        return;
    }

    bool startExists = false;
    float lastStart = 0;
    float lastEnd = 0;
    for (int i = 0; i < int(a->runs.size()); i++) {
        float_ligne_run runA = a->runs[i];
        if ( runA.vst >= tresh ) {
            if ( runA.ven >= tresh ) {
                if ( startExists ) {
                    if ( lastEnd >= runA.st-0.00001 ) {
                        lastEnd = runA.en;
                    } else {
                        if ( addIt ) {
                            AddRun(lastStart,lastEnd,tresh,tresh);
                        }
                        lastStart = runA.st;
                        lastEnd = runA.en;
                    }
                } else {
                    lastStart = runA.st;
                    lastEnd = runA.en;
                }
                startExists = true;
            } else {
                float cutPos = (runA.st * (tresh - runA.ven) + runA.en * (runA.vst - tresh)) / (runA.vst - runA.ven);
                if ( startExists ) {
                    if ( lastEnd >= runA.st-0.00001 ) {
                        if ( addIt ) {
                            AddRun(lastStart, cutPos, tresh, tresh);
                        }
                        AddRun(cutPos, runA.en, tresh, runA.ven);
                    } else {
                        if ( addIt ) {
                            AddRun(lastStart, lastEnd, tresh, tresh);
                        }
                        if ( addIt ) {
                            AddRun(runA.st, cutPos, tresh, tresh);
                        }
                        AddRun(cutPos, runA.en, tresh, runA.ven);
                    }
                } else {
                    if ( addIt ) {
                        AddRun(runA.st, cutPos, tresh, tresh);
                    }
                    AddRun(cutPos, runA.en, tresh, runA.ven);
                }
                startExists = false;
            }
            
        } else {
            
            if ( runA.ven >= tresh ) {
                float cutPos = (runA.st * (runA.ven - tresh) + runA.en * (tresh - runA.vst)) / (runA.ven - runA.vst);
                if ( startExists ) {
                    if ( addIt ) {
                        AddRun(lastStart,lastEnd,tresh,tresh);
                    }
                }
                AddRun(runA.st, cutPos, runA.vst, tresh);
                startExists = true;
                lastStart = cutPos;
                lastEnd = runA.en;
            } else {
                if ( startExists ) {
                    if ( addIt ) {
                        AddRun(lastStart,lastEnd,tresh,tresh);
                    }
                }
                startExists = false;
                AddRun(runA.st, runA.en, runA.vst, runA.ven);
            }
        }
    }
    
    if ( startExists ) {
        if ( addIt ) {
            AddRun(lastStart, lastEnd, tresh, tresh);
        }
    }
}

/// Extract the parts where coverage > tresh.
void FloatLigne::Over(FloatLigne *a, float tresh)
{
    Reset();
    if ( a->runs.empty() ) {
        return;
    }

    bool startExists = false;
    float lastStart = 0;
    float lastEnd = 0;
    
    for (int i = 0; i < int(a->runs.size()); i++) {
        float_ligne_run runA = a->runs[i];
        if ( runA.vst >= tresh ) {
            if ( runA.ven >= tresh ) {
                if ( startExists ) {
                    if ( lastEnd >= runA.st - 0.00001 ) {
                        lastEnd = runA.en;
                    } else {
                        AddRun(lastStart, lastEnd, tresh, tresh);
                        lastStart = runA.st;
                        lastEnd = runA.en;
                    }
                } else {
                    lastStart = runA.st;
                    lastEnd = runA.en;
                }
                startExists = true;
                
            } else {
                
                float cutPos = (runA.st * (tresh - runA.ven) + runA.en * (runA.vst - tresh)) / (runA.vst - runA.ven);
                if ( startExists ) {
                    if ( lastEnd >= runA.st - 0.00001 ) {
                        AddRun(lastStart, cutPos, tresh, tresh);
                    } else {
                        AddRun(lastStart, lastEnd, tresh, tresh);
                        AddRun(runA.st, cutPos, tresh, tresh);
                    }
                } else {
                    AddRun(runA.st, cutPos, tresh, tresh);
                }
                startExists = false;
            }
            
        } else {
            if ( runA.ven >= tresh ) {
                float cutPos = (runA.st * (runA.ven - tresh) + runA.en * (tresh - runA.vst)) / (runA.ven - runA.vst);
                if ( startExists ) {
                    AddRun(lastStart, lastEnd, tresh, tresh);
                }
                startExists = true;
                lastStart = cutPos;
                lastEnd = runA.en;
            } else {
                if ( startExists ) {
                    AddRun(lastStart, lastEnd, tresh, tresh);
                }
                startExists = false;
            }
        }
    }
    
    if ( startExists ) {
        AddRun(lastStart, lastEnd, tresh, tresh);
    }
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

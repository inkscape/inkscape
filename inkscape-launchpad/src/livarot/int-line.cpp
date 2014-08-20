/**
 *  \file livarot/int-line.cpp
 *
 * Implementation of coverage with integer boundaries.
 *
 *  \author Fred
 *
 *  public domain
 *
 */

#include <glib.h>
#include <cmath>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "livarot/int-line.h"
#include "livarot/float-line.h"
#include "livarot/BitLigne.h"

IntLigne::IntLigne()
{
    nbBord = maxBord = 0;
    bords = NULL;

    nbRun = maxRun = 0;
    runs = NULL;

    firstAc = lastAc = -1;
}


IntLigne::~IntLigne()
{
    if ( maxBord > 0 ) {
        g_free(bords);
        nbBord = maxBord = 0;
        bords = NULL;
    }
    if ( maxRun > 0 ) {
        g_free(runs);
        nbRun = maxRun = 0;
        runs = NULL;
    }
}

void IntLigne::Reset()
{
    nbBord = 0;
    nbRun = 0;
    firstAc = lastAc = -1;
}

int IntLigne::AddBord(int spos, float sval, int epos, float eval)
{
    if ( nbBord + 1 >= maxBord ) {
        maxBord = 2 * nbBord + 2;
        bords = (int_ligne_bord *) g_realloc(bords, maxBord * sizeof(int_ligne_bord));
    
    }
    
    int n = nbBord++;
    bords[n].pos = spos;
    bords[n].val = sval;
    bords[n].start = true;
    bords[n].other = n+1;
    bords[n].prev = bords[n].next = -1;
    
    n = nbBord++;
    bords[n].pos = epos;
    bords[n].val = eval;
    bords[n].start = false;
    bords[n].other = n-1;
    bords[n].prev = bords[n].next = -1;
    
    return n - 1;
}


float IntLigne::RemainingValAt(int at)
{
    int no = firstAc;
    float sum = 0;
    while ( no >= 0 ) {
        int nn = bords[no].other;
        sum += ValAt(at, bords[nn].pos, bords[no].pos, bords[nn].val, bords[no].val);
        no = bords[no].next;
    }
    return sum;
}

void IntLigne::Flatten()
{
    if ( nbBord <= 1 ) {
        Reset();
        return;
    }
    
    nbRun = 0;
    firstAc = lastAc = -1;
	
    for (int i = 0; i < nbBord; i++) {
        bords[i].prev = i;
    }
    
    qsort(bords, nbBord, sizeof(int_ligne_bord), IntLigne::CmpBord);
    for (int i = 0; i < nbBord; i++) {
        bords[bords[i].prev].next = i;
    }
    
    for (int i = 0; i < nbBord; i++) {
        bords[i].other = bords[bords[i].other].next;
    }
    
    int lastStart = 0;
    float lastVal = 0;
    bool startExists = false;
    
    for (int i = 0; i < nbBord; ) {
        int cur = bords[i].pos;
        float leftV = 0;
        float rightV = 0;
        float midV = 0;
        while ( i < nbBord && bords[i].pos == cur && bords[i].start == false ) {
            Dequeue(i);
            leftV += bords[i].val;
            i++;
        }
        midV = RemainingValAt(cur);
        while ( i < nbBord && bords[i].pos == cur && bords[i].start == true ) {
            rightV += bords[i].val;
            Enqueue(bords[i].other);
            i++;
        }
        
        if ( startExists ) {
            AddRun(lastStart, cur, lastVal, leftV + midV);
        }
        if ( firstAc >= 0 ) {
            startExists = true;
            lastVal = midV + rightV;
            lastStart = cur;
        } else {
            startExists = false;
        }
    }
}


void IntLigne::Affiche()
{
    printf("%i : \n", nbRun);
    for (int i = 0; i < nbRun;i++) {
        printf("(%i %f -> %i %f) ", runs[i].st, runs[i].vst, runs[i].en, runs[i].ven); // localization ok
    }
    printf("\n");
}

int IntLigne::AddRun(int st, int en, float vst, float ven)
{
    if ( st >= en ) {
        return -1;
    }

    if ( nbRun >= maxRun ) {
        maxRun = 2 * nbRun + 1;
        runs = (int_ligne_run *) g_realloc(runs, maxRun * sizeof(int_ligne_run));
    }
    
    int n = nbRun++;
    runs[n].st = st;
    runs[n].en = en;
    runs[n].vst = vst;
    runs[n].ven = ven;
    return n;
}

void IntLigne::Booleen(IntLigne *a, IntLigne *b, BooleanOp mod)
{
    Reset();
    if ( a->nbRun <= 0 && b->nbRun <= 0 ) {
        return;
    }

    if ( a->nbRun <= 0 ) {
        if ( mod == bool_op_union || mod == bool_op_symdiff ) {
            Copy(b);
        }
        return;
    }
    
    if ( b->nbRun <= 0 ) {
        if ( mod == bool_op_union || mod == bool_op_diff || mod == bool_op_symdiff ) {
            Copy(a);
        }
        return;
    }

    int curA = 0;
    int curB = 0;
    int curPos = (a->runs[0].st < b->runs[0].st) ? a->runs[0].st : b->runs[0].st;
    int nextPos = curPos;
    float valA = 0;
    float valB = 0;
    if ( curPos == a->runs[0].st ) {
        valA = a->runs[0].vst;
    }
    if ( curPos == b->runs[0].st ) {
        valB = b->runs[0].vst;
    }
	
    while ( curA < a->nbRun && curB < b->nbRun ) {
        int_ligne_run runA = a->runs[curA];
        int_ligne_run runB = b->runs[curB];
        const bool inA = ( curPos >= runA.st && curPos < runA.en );
        const bool inB = ( curPos >= runB.st && curPos < runB.en );

        bool startA = false;
        bool startB = false;
        bool endA = false;
        bool endB = false;
        
        if ( curPos < runA.st ) {
            if ( curPos < runB.st ) {
                startA = runA.st <= runB.st;
                startB = runA.st >= runB.st;
                nextPos = startA ? runA.st : runB.st;
            } else if ( curPos >= runB.st ) {
                startA = runA.st <= runB.en;
                endB = runA.st >= runB.en;
                nextPos = startA ? runA.st : runB.en;
            }
        } else if ( curPos == runA.st ) {
            if ( curPos < runB.st ) {
                endA = runA.en <= runB.st;
                startB = runA.en >= runB.st;
                nextPos = startB ? runB.en : runA.st;
            } else if ( curPos == runB.st ) {
                endA = runA.en <= runB.en;
                endB = runA.en >= runB.en;
                nextPos = endA? runA.en : runB.en;
            } else {
                endA = runA.en <= runB.en;
                endB = runA.en >= runB.en;
                nextPos = endA ? runA.en : runB.en;
            }
        } else {
            if ( curPos < runB.st ) {
                endA = runA.en <= runB.st;
                startB = runA.en >= runB.st;
                nextPos = startB ? runB.st : runA.en;
            } else if ( curPos == runB.st ) {
                endA = runA.en <= runB.en;
                endB = runA.en >= runB.en;
                nextPos = endA ? runA.en : runB.en;
            } else {
                endA = runA.en <= runB.en;
                endB = runA.en >= runB.en;
                nextPos = endA ? runA.en : runB.en;
            }
        }
        
        float oValA = valA;
        float oValB = valB;
        valA = inA ? ValAt(nextPos, runA.st, runA.en, runA.vst, runA.ven) : 0;
        valB = inB ? ValAt(nextPos, runB.st, runB.en, runB.vst, runB.ven) : 0;
		
        if ( mod == bool_op_union ) {
            
            if ( inA || inB ) {
                AddRun(curPos, nextPos, oValA + oValB, valA + valB);
            }
            
        } else if ( mod == bool_op_inters ) {
            
            if ( inA && inB ) {
                AddRun(curPos, nextPos, oValA * oValB, valA * valB);
            }
                        
        } else if ( mod == bool_op_diff ) {
            
            if ( inA ) {
                AddRun(curPos, nextPos, oValA - oValB, valA - valB);
            }
            
        } else if ( mod == bool_op_symdiff ) {
            if ( inA && !(inB) ) {
                AddRun(curPos, nextPos, oValA - oValB, valA - valB);
            }
            if ( !(inA) && inB ) {
                AddRun(curPos, nextPos, oValB - oValA, valB - valA);
            }
        }

        curPos = nextPos;
        if ( startA ) {
            // inA=true; these are never used
            valA = runA.vst;
        }
        if ( startB ) {
            //inB=true;
            valB = runB.vst;
        }
        if ( endA ) {
            //inA=false;
            valA = 0;
            curA++;
            if ( curA < a->nbRun && a->runs[curA].st == curPos ) {
                valA = a->runs[curA].vst;
            }
        }
        if ( endB ) {
            //inB=false;
            valB = 0;
            curB++;
            if ( curB < b->nbRun && b->runs[curB].st == curPos ) {
                valB = b->runs[curB].vst;
            }
        }
    }
    
    while ( curA < a->nbRun ) {
        int_ligne_run runA = a->runs[curA];
        const bool inA = ( curPos >= runA.st && curPos < runA.en );
        const bool inB = false;

        bool startA = false;
        bool endA = false;
        if ( curPos < runA.st ) {
            nextPos = runA.st;
            startA = true;
        } else if ( curPos >= runA.st ) {
            nextPos = runA.en;
            endA = true;
        }

        float oValA = valA;
        float oValB = valB;
        valA = inA ? ValAt(nextPos,runA.st, runA.en, runA.vst, runA.ven) : 0;
        valB = 0;

        if ( mod == bool_op_union ) {
            if ( inA || inB ) {
                AddRun(curPos, nextPos, oValA + oValB, valA + valB);
            }
        } else if ( mod == bool_op_inters ) {
            if ( inA && inB ) {
                AddRun(curPos, nextPos, oValA * oValB, valA * valB);
            }
        } else if ( mod == bool_op_diff ) {
            if ( inA ) {
                AddRun(curPos, nextPos, oValA - oValB, valA - valB);
            }
        } else if ( mod == bool_op_symdiff ) {
            if ( inA && !(inB) ) {
                AddRun(curPos, nextPos, oValA - oValB, valA - valB);
            }
            if ( !(inA) && inB ) {
                AddRun(curPos,nextPos,oValB-oValA,valB-valA);
            }
        }

        curPos = nextPos;
        if ( startA ) {
            //inA=true;
            valA = runA.vst;
        }
        if ( endA ) {
            //inA=false;
            valA = 0;
            curA++;
            if ( curA < a->nbRun && a->runs[curA].st == curPos ) {
                valA = a->runs[curA].vst;
            }
        }
    }
    
    while ( curB < b->nbRun ) {
        int_ligne_run runB = b->runs[curB];
        const bool inB = ( curPos >= runB.st && curPos < runB.en );
        const bool inA = false;

        bool startB = false;
        bool endB = false;
        if ( curPos < runB.st ) {
            nextPos = runB.st;
            startB = true;
        } else if ( curPos >= runB.st ) {
            nextPos = runB.en;
            endB = true;
        }

        float oValA = valA;
        float oValB = valB;
        valB = inB ? ValAt(nextPos, runB.st, runB.en, runB.vst, runB.ven) : 0;
        valA = 0;

        if ( mod == bool_op_union ) {
            if ( inA || inB ) {
                AddRun(curPos, nextPos, oValA + oValB,valA + valB);
            }
        } else if ( mod == bool_op_inters ) {
            if ( inA && inB ) {
                AddRun(curPos, nextPos, oValA * oValB, valA * valB);
            }
        } else if ( mod == bool_op_diff ) {
            if ( inA ) {
                AddRun(curPos, nextPos, oValA - oValB, valA - valB);
            }
        } else if ( mod == bool_op_symdiff ) {
            if ( inA && !(inB) ) {
                AddRun(curPos, nextPos, oValA - oValB,valA - valB);
            }
            if ( !(inA) && inB ) {
                AddRun(curPos, nextPos, oValB - oValA, valB - valA);
            }
        }

        curPos = nextPos;
        if ( startB ) {
            //inB=true;
            valB = runB.vst;
        }
        if ( endB ) {
            //inB=false;
            valB = 0;
            curB++;
            if ( curB < b->nbRun && b->runs[curB].st == curPos ) {
                valB = b->runs[curB].vst;
            }
        }
    }
}

/**
 * Transform a line of bits into pixel coverage values.
 *
 * This is where you go from supersampled data to alpha values.
 * \see IntLigne::Copy(int nbSub,BitLigne* *a).
 */
void IntLigne::Copy(BitLigne* a)
{
    if ( a->curMax <= a->curMin ) {
        Reset();
        return;
    }
    
    if ( a->curMin < a->st ) {
        a->curMin = a->st;
    }
    
    if ( a->curMax < a->st ) {
        Reset();
        return;
    }
    
    if ( a->curMin > a->en ) {
        Reset();
        return;
    }
    
    if ( a->curMax > a->en ) {
        a->curMax=a->en;
    }
    
    nbBord = 0;
    nbRun = 0;

    int lastVal = 0;
    int lastStart = 0;
    bool startExists = false;

    int masks[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

    uint32_t c_full = a->fullB[(a->curMin-a->st) >> 3];
    uint32_t c_part = a->partB[(a->curMin-a->st) >> 3];
    c_full <<= 4 * ((a->curMin - a->st) & 0x00000007);
    c_part <<= 4 * ((a->curMin - a->st) & 0x00000007);
    for (int i = a->curMin; i <= a->curMax; i++) {
        int nbBit = masks[c_full >> 28] + masks[c_part >> 28];

        if ( nbBit > 0 ) {
            if ( startExists ) {
                if ( lastVal == nbBit ) {
                    // on continue le run
                } else {
                    AddRun(lastStart, i, ((float) lastVal) / 4, ((float) lastVal) / 4);
                    lastStart = i;
                    lastVal = nbBit;
                }
            } else {
                lastStart = i;
                lastVal = nbBit;
                startExists = true;
            }
        } else {
            if ( startExists ) {
                AddRun(lastStart, i, ((float) lastVal) / 4, ((float) lastVal) / 4);
            }
            startExists = false;
        }
        int chg = (i + 1 - a->st) & 0x00000007;
        if ( chg == 0 ) {
            c_full = a->fullB[(i + 1 - a->st) >> 3];
            c_part = a->partB[(i + 1 - a->st) >> 3];
        } else {
            c_full <<= 4;
            c_part <<= 4;
        }
    }
    if ( startExists ) {
        AddRun(lastStart, a->curMax + 1, ((float) lastVal) / 4, ((float) lastVal) / 4);
    }
}

/**
 * Transform a line of bits into pixel coverage values.
 *
 * Alpha values are computed from supersampled data, so we have to scan the 
 * BitLigne left to right, summing the bits in each pixel. The alpha value 
 * is then "number of bits"/(nbSub*nbSub)". Full bits and partial bits are 
 * treated as equals because the method produces ugly results otherwise.
 *
 * \param nbSub Number of BitLigne in the array "a".
 */
void IntLigne::Copy(int nbSub, BitLigne **as)
{
    if ( nbSub <= 0 ) {
        Reset();
        return;
    }

    if ( nbSub == 1 ) {
        Copy(as[0]);
        return;
    }
    
    // compute the min-max of the pixels to be rasterized from the min-max of the  inpur bitlignes
    int curMin = as[0]->curMin;
    int curMax = as[0]->curMax;
    for (int i = 1; i < nbSub; i++) {
        if ( as[i]->curMin < curMin ) {
            curMin = as[i]->curMin;
        }
        if ( as[i]->curMax > curMax ) {
            curMax = as[i]->curMax;
        }
    }
    
    if ( curMin < as[0]->st ) {
        curMin = as[0]->st;
    }

    if ( curMax > as[0]->en ) {
        curMax = as[0]->en;
    }
    
    if ( curMax <= curMin ) {
        Reset();
        return;
    }

    nbBord = 0;
    nbRun = 0;
    
    int lastVal = 0;
    int lastStart = 0;
    bool startExists = false;
    float spA;
    int masks[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

    int  theSt = as[0]->st;
    if ( nbSub == 4 ) {
        // special case for 4*4 supersampling, to avoid a few loops
        uint32_t c_full[4];
        c_full[0] = as[0]->fullB[(curMin - theSt) >> 3] | as[0]->partB[(curMin - theSt) >> 3];
        c_full[0] <<= 4 * ((curMin - theSt) & 7);
        c_full[1] = as[1]->fullB[(curMin - theSt) >> 3] | as[1]->partB[(curMin - theSt) >> 3];
        c_full[1] <<= 4 * ((curMin - theSt) & 7);
        c_full[2] = as[2]->fullB[(curMin - theSt) >> 3] | as[2]->partB[(curMin - theSt) >> 3];
        c_full[2] <<= 4* ((curMin - theSt) & 7);
        c_full[3] = as[3]->fullB[(curMin - theSt) >> 3] | as[3]->partB[(curMin - theSt) >> 3];
        c_full[3] <<= 4* ((curMin - theSt) & 7);
        
        spA = 1.0 / (4 * 4);
        for (int i = curMin; i <= curMax; i++) {
            int nbBit = 0;

            if ( c_full[0] == 0 && c_full[1] == 0 && c_full[2] == 0 && c_full[3] == 0 ) {

                if ( startExists ) {
                    AddRun(lastStart, i, ((float) lastVal) * spA, ((float) lastVal) * spA);
                }
                startExists = false;
                i = theSt + (((i - theSt) & (~7) ) + 7);
                
            } else if ( c_full[0] == 0xFFFFFFFF && c_full[1] == 0xFFFFFFFF &&
                        c_full[2] == 0xFFFFFFFF && c_full[3] == 0xFFFFFFFF ) {
                
                if ( startExists ) {
                    if ( lastVal == 4*4) {
                    } else {
                        AddRun(lastStart, i, ((float) lastVal) * spA, ((float) lastVal) * spA);
                        lastStart = i;
                    }
                } else {
                    lastStart = i;
                }
                lastVal = 4*4;
                startExists = true;
                i = theSt + (((i - theSt) & (~7) ) + 7);
                
            } else {
                nbBit += masks[c_full[0] >> 28];
                nbBit += masks[c_full[1] >> 28];
                nbBit += masks[c_full[2] >> 28];
                nbBit += masks[c_full[3] >> 28];
                
                if ( nbBit > 0 ) {
                    if ( startExists ) {
                        if ( lastVal == nbBit ) {
                            // on continue le run
                        } else {
                            AddRun(lastStart, i, ((float) lastVal) * spA, ((float) lastVal) * spA);
                            lastStart = i;
                            lastVal = nbBit;
                        }
                    } else {
                        lastStart = i;
                        lastVal = nbBit;
                        startExists = true;
                    }
                } else {
                    if ( startExists ) {
                        AddRun(lastStart, i, ((float) lastVal) * spA, ((float) lastVal) * spA);
                    }
                    startExists = false;
                }
            }
            int chg = (i + 1 - theSt) & 7;
            if ( chg == 0 ) {
                if ( i < curMax ) {
                    c_full[0] = as[0]->fullB[(i + 1 - theSt) >> 3] | as[0]->partB[(i + 1 - theSt) >> 3];
                    c_full[1] = as[1]->fullB[(i + 1 - theSt) >> 3] | as[1]->partB[(i + 1 - theSt) >> 3];
                    c_full[2] = as[2]->fullB[(i + 1 - theSt) >> 3] | as[2]->partB[(i + 1 - theSt) >> 3];
                    c_full[3] = as[3]->fullB[(i + 1 - theSt) >> 3] | as[3]->partB[(i + 1 - theSt) >> 3];
                } else {
                    // end of line. byebye
                }
            } else {
                c_full[0] <<= 4;
                c_full[1] <<= 4;
                c_full[2] <<= 4;
                c_full[3] <<= 4;
            }
        }
        
    } else {

        uint32_t c_full[16]; // we take nbSub < 16, since 16*16 supersampling makes a 1/256 precision in alpha values
        // and that's the max of what 32bit argb can represent
        // in fact, we'll treat it as 4*nbSub supersampling, so that's a half truth and a full lazyness from me
        //	uint32_t  c_part[16];
        // start by putting the bits of the nbSub BitLignes in as[] in their respective c_full

        for (int i = 0; i < nbSub; i++) {
            // fullB and partB treated equally
            c_full[i] = as[i]->fullB[(curMin - theSt) >> 3] | as[i]->partB[(curMin - theSt) >> 3]; 
            c_full[i] <<= 4 * ((curMin - theSt) & 7);
            /*		c_part[i]=as[i]->partB[(curMin-theSt)>>3];
			c_part[i]<<=4*((curMin-theSt)&7);*/
        }

        spA = 1.0 / (4 * nbSub); // contribution to the alpha value of a single bit of the supersampled data
        for (int i = curMin; i <= curMax;i++) {
            int  nbBit = 0;
            //			int nbPartBit=0;
            // a little acceleration: if the lines only contain full or empty bits, we can flush
            // what's remaining in the c_full at best we flush an entire c_full, ie 32 bits, or 32/4=8 pixels
            bool allEmpty = true;
            bool allFull = true;
            for (int j = 0; j < nbSub; j++) {
                if ( c_full[j] != 0 /*|| c_part[j] != 0*/ ) {
                    allEmpty=false;
                    break;
                }
            }
            
            if ( allEmpty ) {
                // the remaining bits in c_full[] are empty: flush
                if ( startExists ) {
                    AddRun(lastStart, i, ((float) lastVal) * spA, ((float) lastVal) * spA);
                }
                startExists = false;
                i = theSt + (((i - theSt) & (~7) ) + 7);
            } else {
                for (int j = 0; j < nbSub; j++) {
                    if ( c_full[j] != 0xFFFFFFFF ) {
                        allFull=false;
                        break;
                    }
                }
                
                if ( allFull ) {
                    // the remaining bits in c_full[] are empty: flush
                    if ( startExists ) {
                        if ( lastVal == 4 * nbSub) {
                        } else {
                            AddRun(lastStart, i, ((float) lastVal) * spA,((float) lastVal) * spA);
                            lastStart = i;
                        }
                    } else {
                        lastStart = i;
                    }
                    lastVal = 4 * nbSub;
                    startExists = true;
                    i = theSt + (((i - theSt) & (~7) ) + 7);
                } else {
                    // alpha values will be between 0 and 1, so we have more work to do
                    // compute how many bit this pixel holds
                    for (int j = 0; j < nbSub; j++) {
                        nbBit += masks[c_full[j] >> 28];
//						nbPartBit+=masks[c_part[j]>>28];
                    }
                    // and add a single-pixel run if needed, or extend the current run if the alpha value hasn't changed
                    if ( nbBit > 0 ) {
                        if ( startExists ) {
                            if ( lastVal == nbBit ) {
                                // alpha value hasn't changed: we continue
                            } else {
                                // alpha value did change: put the run that was being done,...
                                AddRun(lastStart, i, ((float) lastVal) * spA, ((float) lastVal) * spA);
                                // ... and start a new one
                                lastStart = i;
                                lastVal = nbBit;
                            }
                        } else {
                            // alpha value was 0, so we "create" a new run with alpha nbBit
                            lastStart = i;
                            lastVal = nbBit;
                            startExists = true;
                        }
                    } else {
                        if ( startExists ) {
                            AddRun(lastStart, i, ((float) lastVal) * spA,((float) lastVal) * spA);
                        }
                        startExists = false;
                    }
                }
            }
            // move to the right: shift bits in the c_full[], and if we shifted everything, load the next c_full[]
            int chg = (i + 1 - theSt) & 7;
            if ( chg == 0 ) {
                if ( i < curMax ) {
                    for (int j = 0; j < nbSub; j++) {
                        c_full[j] = as[j]->fullB[(i + 1 - theSt) >> 3] | as[j]->partB[(i + 1 - theSt) >> 3];
                        //			c_part[j]=as[j]->partB[(i+1-theSt)>>3];
                    }
                } else {
                    // end of line. byebye
                }        
            } else {
                for (int j = 0; j < nbSub; j++) {
                    c_full[j]<<=4;
                    //			c_part[j]<<=4;
                }
            }
        }
    }
    
    if ( startExists ) {
        AddRun(lastStart, curMax + 1, ((float) lastVal) * spA,((float) lastVal) * spA);
    }
}

/// Copy another IntLigne
void IntLigne::Copy(IntLigne *a)
{
    if ( a->nbRun <= 0 ) {
        Reset();
        return;
    }
    
    nbBord = 0;
    nbRun = a->nbRun;
    if ( nbRun > maxRun ) {
        maxRun = nbRun;
        runs = (int_ligne_run*) g_realloc(runs, maxRun * sizeof(int_ligne_run));
    }
    memcpy(runs, a->runs, nbRun * sizeof(int_ligne_run));
}


/** 
 * Copy a FloatLigne's runs.
 *
 * Compute non-overlapping runs with integer boundaries from a set of runs 
 * with floating-point boundaries. This involves replacing floating-point 
 * boundaries that are not integer by single-pixel runs, so this function 
 * contains plenty of rounding and float->integer conversion (read: 
 * time-consuming).
 *
 * \todo
 * Optimization Questions: Why is this called so often compared with the 
 * other Copy() routines? How does AddRun() look for optimization potential?
 */
void IntLigne::Copy(FloatLigne* a)
{
    if ( a->runs.empty() ) {
	Reset();
	return;
    }
    
    /*  if ( showCopy ) {
	printf("\nfloatligne:\n");
	a->Affiche();
	}*/
    
    nbBord = 0;
    nbRun = 0;
    firstAc = lastAc = -1;
    bool pixExists = false;
    int curPos = (int) floor(a->runs[0].st) - 1;
    float lastSurf = 0;
    float tolerance = 0.00001;
	
    // we take each run of the FloatLigne in sequence and make single-pixel runs of its boundaries as needed
    // since the float_ligne_runs are non-overlapping, when a single-pixel run intersects with another runs, 
    // it must intersect with the single-pixel run created for the end of that run. so instead of creating a new
    // int_ligne_run, we just add the coverage to that run.
    for (int i = 0; i < int(a->runs.size()); i++) {
	float_ligne_run runA = a->runs[i];
	float curStF = floor(runA.st);
	float curEnF = floor(runA.en);
	int   curSt  = (int) curStF;
	int   curEn  = (int) curEnF;

	// stEx: start boundary is not integer -> create single-pixel run for it
	// enEx: end boundary is not integer -> create single-pixel run for it
	// miEx: the runs minus the eventual single-pixel runs is not empty
	bool  stEx  = true;
	bool  miEx  = true;
	bool  enEx  = true;
	int   miSt  = curSt;
	float miStF = curStF;
	float msv;
	float mev;
	if ( runA.en - curEnF < tolerance ) {
	    enEx = false;
	}

	// msv and mev are the start and end value of the middle section of the run, that is the run minus the
	// single-pixel runs creaed for its boundaries
	if ( runA.st-curStF < tolerance /*miSt == runA.st*/ ) {
	    stEx = false;
	    msv = runA.vst;
	} else {
	    miSt  += 1;
	    miStF += 1.0;
	    if ( enEx == false && miSt == curEn ) {
		msv = runA.ven;
	    } else {
		//			msv=a->ValAt(miSt,runA.st,runA.en,runA.vst,runA.ven);
		msv = runA.vst + (miStF-runA.st) * runA.pente;
	    }
	}

	if ( miSt >= curEn ) {
	    miEx = false;
	}
	if ( stEx == false && miEx == false /*curEn == runA.st*/ ) {
	    mev = runA.vst;
	} else if ( enEx == false /*curEn == runA.en*/ ) {
	    mev = runA.ven;
	} else {
	    //			mev=a->ValAt(curEn,runA.st,runA.en,runA.vst,runA.ven);
	    mev = runA.vst + (curEnF-runA.st) * runA.pente;
	}
	
	// check the different cases
	if ( stEx && enEx ) {
	    // stEx && enEx
	    if ( curEn > curSt ) {
		if ( pixExists ) {
		    if ( curPos < curSt ) {
			AddRun(curPos,curPos+1,lastSurf,lastSurf);
			lastSurf=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
			AddRun(curSt,curSt+1,lastSurf,lastSurf);
		    } else {
			lastSurf+=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
			AddRun(curSt,curSt+1,lastSurf,lastSurf);
		    }
		    pixExists=false;
		} else {
		    lastSurf=0.5*(msv+a->runs[i].vst)*(miStF-a->runs[i].st);
		    AddRun(curSt,curSt+1,lastSurf,lastSurf);						
		}
	    } else if ( pixExists ) {
		if ( curPos < curSt ) {
		    AddRun(curPos,curPos+1,lastSurf,lastSurf);
		    lastSurf=0.5*(a->runs[i].ven+a->runs[i].vst)*(a->runs[i].en-a->runs[i].st);
		    curPos=curSt;
		} else {
		    lastSurf += 0.5 * (a->runs[i].ven+a->runs[i].vst)*(a->runs[i].en-a->runs[i].st);
		}
	    } else {
		lastSurf=0.5*(a->runs[i].ven+a->runs[i].vst)*(a->runs[i].en-a->runs[i].st);
		curPos=curSt;
		pixExists=true;
	    }
	} else if ( pixExists ) {
	    if ( curPos < curSt ) {
		AddRun(curPos,curPos+1,lastSurf,lastSurf);
		lastSurf = 0.5 * (msv+a->runs[i].vst) * (miStF-a->runs[i].st);
		AddRun(curSt,curSt+1,lastSurf,lastSurf);
	    } else {
		lastSurf += 0.5 * (msv+a->runs[i].vst) * (miStF-a->runs[i].st);
		AddRun(curSt,curSt+1,lastSurf,lastSurf);
	    }
	    pixExists=false;
	} else {
	    lastSurf = 0.5 * (msv+a->runs[i].vst) * (miStF-a->runs[i].st);
	    AddRun(curSt,curSt+1,lastSurf,lastSurf);
	}
	if ( miEx ) {
	    if ( pixExists && curPos < miSt ) {
		AddRun(curPos,curPos+1,lastSurf,lastSurf);
	    }
	    pixExists=false;
	    AddRun(miSt,curEn,msv,mev);
	}
	if ( enEx ) {
	    if ( curEn > curSt ) {
		lastSurf=0.5*(mev+a->runs[i].ven)*(a->runs[i].en-curEnF);
		pixExists=true;
		curPos=curEn;
	    } else if ( ! stEx ) {
		if ( pixExists ) {
		    AddRun(curPos,curPos+1,lastSurf,lastSurf);
		}
		lastSurf=0.5*(mev+a->runs[i].ven)*(a->runs[i].en-curEnF);
		pixExists=true;
		curPos=curEn;					
	    }
	}
    }
    if ( pixExists ) {
	AddRun(curPos,curPos+1,lastSurf,lastSurf);
    }
    /*  if ( showCopy ) {
	printf("-> intligne:\n");
	Affiche();
	}*/
}


void IntLigne::Enqueue(int no)
{
    if ( firstAc < 0 ) {
        firstAc = lastAc = no;
        bords[no].prev = bords[no].next = -1;
    } else {
        bords[no].next = -1;
        bords[no].prev = lastAc;
        bords[lastAc].next = no;
        lastAc = no;
    }
}


void IntLigne::Dequeue(int no)
{
    if ( no == firstAc ) {
        if ( no == lastAc ) {
            firstAc = lastAc = -1;
        } else {
            firstAc = bords[no].next;
        }
    } else if ( no == lastAc ) {
        lastAc = bords[no].prev;
    } else {
    }
    if ( bords[no].prev >= 0 ) {
        bords[bords[no].prev].next = bords[no].next;
    }
    if ( bords[no].next >= 0 ) {
        bords[bords[no].next].prev = bords[no].prev;
    }
    
    bords[no].prev = bords[no].next = -1;
}

/**
 * Rasterization.
 *
 * The parameters have the same meaning as in the AlphaLigne class.
 */
void IntLigne::Raster(raster_info &dest, void *color, RasterInRunFunc worker)
{
    if ( nbRun <= 0 ) {
        return;
    }
    
    int min = runs[0].st;
    int max = runs[nbRun-1].en;
    if ( dest.endPix <= min || dest.startPix >= max ) {
        return;
    }

    int curRun = -1;
    for (curRun = 0; curRun < nbRun; curRun++) {
        if ( runs[curRun].en > dest.startPix ) {
            break;
        }
    }
  
    if ( curRun >= nbRun ) {
        return;
    }
  
    if ( runs[curRun].st < dest.startPix ) {
        int nst = runs[curRun].st;
        int nen = runs[curRun].en;
        float vst = runs[curRun].vst;
        float ven = runs[curRun].ven;
        float nvst = (vst * (nen - dest.startPix) + ven * (dest.startPix - nst)) / ((float) (nen - nst));
        if ( runs[curRun].en <= dest.endPix ) {
            (worker)(dest, color, dest.startPix, nvst, runs[curRun].en, runs[curRun].ven);
        } else {
            float nven = (vst * (nen - dest.endPix) + ven * (dest.endPix - nst)) / ((float)(nen - nst));
            (worker)(dest, color, dest.startPix, nvst, dest.endPix, nven);
            return;
        }
        curRun++;
    }

    for (; (curRun < nbRun && runs[curRun].en <= dest.endPix); curRun++) {
        (worker)(dest, color, runs[curRun].st, runs[curRun].vst, runs[curRun].en, runs[curRun].ven);
//Buffer::RasterRun(*dest,color,runs[curRun].st,runs[curRun].vst,runs[curRun].en,runs[curRun].ven);
    }
    
    if ( curRun >= nbRun ) {
        return;
    }
    
    if ( runs[curRun].st < dest.endPix && runs[curRun].en > dest.endPix ) {
        int const nst = runs[curRun].st;
        int const nen = runs[curRun].en;
        float const vst = runs[curRun].vst;
        float const ven = runs[curRun].ven;
        float const nven = (vst * (nen - dest.endPix) + ven * (dest.endPix - nst)) / ((float)(nen - nst));
        
        (worker)(dest,color,runs[curRun].st,runs[curRun].vst,dest.endPix,nven);
        curRun++;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

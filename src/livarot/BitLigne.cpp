/*
 *  BitLigne.cpp
 *  nlivarot
 *
 *  Created by fred on Wed Jul 23 2003.
 *  public domain
 *
 */

#include "BitLigne.h"

#include <math.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <cmath>
#include <cstdio>
#include <glib.h>

BitLigne::BitLigne(int ist,int ien,float iScale)
{
  scale=iScale;
  invScale=1/iScale;
	st=ist;
	en=ien;
	if ( en <= st ) en=st+1;
	stBit=(int)floor(((float)st)*invScale); // round to pixel boundaries in the canvas
	enBit=(int)ceil(((float)en)*invScale);
	int  nbBit=enBit-stBit;
	if ( nbBit&31 ) {
		nbInt=nbBit/32+1;
	} else {
		nbInt=nbBit/32;
	}
  nbInt+=1;
	fullB=(uint32_t*)g_malloc(nbInt*sizeof(uint32_t));
	partB=(uint32_t*)g_malloc(nbInt*sizeof(uint32_t));

	curMin=en;
	curMax=st;
}
BitLigne::~BitLigne(void)
{
	g_free(fullB);
	g_free(partB);
}

void             BitLigne::Reset(void)
{
	curMin=en;
	curMax=st+1;
	memset(fullB,0,nbInt*sizeof(uint32_t));
	memset(partB,0,nbInt*sizeof(uint32_t));
}
int              BitLigne::AddBord(float spos,float epos,bool full)
{
	if ( spos >= epos ) return 0;
	
  // separation of full and not entirely full bits is a bit useless
  // the goal is to obtain a set of bits that are "on the edges" of the polygon, so that their coverage
  // will be 1/2 on the average. in practice it's useless for anything but the even-odd fill rule
	int   ffBit,lfBit; // first and last bit of the portion of the line that is entirely covered
	ffBit=(int)(ceil(invScale*spos));
	lfBit=(int)(floor(invScale*epos));
	int   fpBit,lpBit; // first and last bit of the portion of the line that is not entirely but partially covered
	fpBit=(int)(floor(invScale*spos));
	lpBit=(int)(ceil(invScale*epos));
  
  // update curMin and curMax to reflect the start and end pixel that need to be updated on the canvas
	if ( floor(spos) < curMin ) curMin=(int)floor(spos);
	if ( ceil(epos) > curMax ) curMax=(int)ceil(epos);

  // clamp to the line
	if ( ffBit < stBit ) ffBit=stBit;
	if ( ffBit > enBit ) ffBit=enBit;
	if ( lfBit < stBit ) lfBit=stBit;
	if ( lfBit > enBit ) lfBit=enBit;
	if ( fpBit < stBit ) fpBit=stBit;
	if ( fpBit > enBit ) fpBit=enBit;
	if ( lpBit < stBit ) lpBit=stBit;
	if ( lpBit > enBit ) lpBit=enBit;
  
  // offset to get actual bit position in the array
	ffBit-=stBit;
	lfBit-=stBit;
	fpBit-=stBit;
	lpBit-=stBit;

  // get the end and start indices of the elements of fullB and partB that will receives coverage
	int   ffPos=ffBit>>5;
	int   lfPos=lfBit>>5;
	int   fpPos=fpBit>>5;
	int   lpPos=lpBit>>5;
  // get bit numbers in the last and first changed elements of the fullB and partB arrays
	int   ffRem=ffBit&31;
	int   lfRem=lfBit&31;
	int   fpRem=fpBit&31;
	int   lpRem=lpBit&31;
  // add the coverage
  // note that the "full" bits are always a subset of the "not empty" bits, ie of the partial bits
  // the function is a bit lame: since there is at most one bit that is partial but not full, or no full bit,
  // it does 2 times the optimal amount of work when the coverage is full. but i'm too lazy to change that...
	if ( fpPos == lpPos ) { // only one element of the arrays is modified
    // compute the vector of changed bits in the element
		uint32_t  add=0xFFFFFFFF;
		if ( lpRem < 32 ) {add>>=32-lpRem;add<<=32-lpRem; }
    if ( lpRem <= 0 ) add=0;
		if ( fpRem > 0) {add<<=fpRem;add>>=fpRem;}
    // and put it in the line
    fullB[fpPos]&=~(add); // partial is exclusive from full, so partial bits are removed from fullB
    partB[fpPos]|=add;    // and added to partB
    if ( full ) { // if the coverage is full, add the vector of full bits
      if ( ffBit <= lfBit ) {
        add=0xFFFFFFFF;
        if ( lfRem < 32 ) {add>>=32-lfRem;add<<=32-lfRem;}
        if ( lfRem <= 0 ) add=0;
        if ( ffRem > 0 ) {add<<=ffRem;add>>=ffRem;}
        fullB[ffPos]|=add;
        partB[ffPos]&=~(add);
      }
    }
	} else {
    // first and last elements are differents, so add what appropriate to each
		uint32_t  add=0xFFFFFFFF;
		if ( fpRem > 0 ) {add<<=fpRem;add>>=fpRem;}
    fullB[fpPos]&=~(add);
    partB[fpPos]|=add;

		add=0xFFFFFFFF;
		if ( lpRem < 32 ) {add>>=32-lpRem;add<<=32-lpRem;}
    if ( lpRem <= 0 ) add=0;
    fullB[lpPos]&=~(add);
    partB[lpPos]|=add;

    // and fill what's in between with partial bits
    if ( lpPos > fpPos+1 ) memset(fullB+(fpPos+1),0x00,(lpPos-fpPos-1)*sizeof(uint32_t));
    if ( lpPos > fpPos+1 ) memset(partB+(fpPos+1),0xFF,(lpPos-fpPos-1)*sizeof(uint32_t));

		if ( full ) { // is the coverage is full, do your magic
      if ( ffBit <= lfBit ) {
        if ( ffPos == lfPos ) {
          add=0xFFFFFFFF;
          if ( lfRem < 32 ) {add>>=32-lfRem;add<<=32-lfRem;}
          if ( lfRem <= 0 ) add=0;
          if ( ffRem > 0 ) {add<<=ffRem;add>>=ffRem;}
          fullB[ffPos]|=add;
          partB[ffPos]&=~(add);
        } else {
          add=0xFFFFFFFF;
          if ( ffRem > 0 ) {add<<=ffRem;add>>=ffRem;}
          fullB[ffPos]|=add;
          partB[ffPos]&=~add;
          
          add=0xFFFFFFFF;
          if ( lfRem < 32 ) {add>>=32-lfRem;add<<=32-lfRem;}
          if ( lfRem <= 0 ) add=0;
          fullB[lfPos]|=add;
          partB[lfPos]&=~add;
          
          if ( lfPos > ffPos+1 ) memset(fullB+(ffPos+1),0xFF,(lfPos-ffPos-1)*sizeof(uint32_t));
          if ( lfPos > ffPos+1 ) memset(partB+(ffPos+1),0x00,(lfPos-ffPos-1)*sizeof(uint32_t));
        }
      }
    }
	}
	return 0;
}


void             BitLigne::Affiche(void)
{
	for (int i=0;i<nbInt;i++) printf(" %.8x",fullB[i]);
	printf("\n");
	for (int i=0;i<nbInt;i++) printf(" %.8x",partB[i]);
	printf("\n\n");
}


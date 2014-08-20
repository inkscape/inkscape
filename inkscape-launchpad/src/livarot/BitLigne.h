/*
 *  BitLigne.h
 *  nlivarot
 *
 *  Created by fred on Wed Jul 23 2003.
 *  public domain
 *
 */

#ifndef my_bit_ligne
#define my_bit_ligne

#include "LivarotDefs.h"

/*
 * a line of bits used for rasterizations of polygons
 * the Scan() and QuickScan() functions fill the line with bits; after that you can use the Copy() function
 * of the IntLigne class to have a set of pixel coverage runs
 */

class BitLigne {
public:
  // start and end pixels of the line
	int           st,en;
  // start and end bits of the line
	int           stBit,enBit;
  // size of the fullB and partB arrays
	int           nbInt;
  // arrays of uint32_t used to store the bits
  // bits of fullB mean "this pixel/bit is entirely covered"
  // bits of partB mean "this pixel/bit is not entirely covered" (a better use would be: "this pixel is at least partially covered)
  // so it's in fact a triage mask
	uint32_t*     fullB;
	uint32_t*     partB;

  // when adding bits, these 2 values are updated to reflect which portion of the line has received coverage
	int           curMin,curMax;
  // invScale is: canvas -> bit in the line
  // scale is: bit -> canvas, ie the size (width) of a bit
  float         scale,invScale;

	BitLigne(int ist,int ien,float iScale=0.25);  // default scale is 1/4 for 4x4 supersampling
    virtual ~BitLigne(void);

  // reset the line to full empty
	void             Reset(void);
	
  // put coverage from spos to epos (in canvas coordinates)
  // full==true means that the bits from (fractional) position spos to epos are entirely covered
  // full==false means the bits are not entirely covered, ie this is an edge
  // see the Scan() and AvanceEdge() functions to see the difference
	int              AddBord(float spos,float epos,bool full);

  // debug dump
	void             Affiche(void);

};

#endif



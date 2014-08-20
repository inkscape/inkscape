/*
 *  AlphaLigne.h
 *  nlivarot
 *
 *  Created by fred on Fri Jul 25 2003.
 *  public domain
 *
 */

#ifndef my_alpha_ligne
#define my_alpha_ligne

#include "LivarotDefs.h"

/*
 * pixel coverage of a line, libart style: each pixel coverage is obtained from the coverage of the previous one by
 * adding a delta given by a step. the goal is to have only a limited number of positions where the delta != 0, so that
 * you only have to store a limited number of steps.
 */

// a step
typedef struct alpha_step {
	int           x;     // position
	float         delta; // increase or decrease in pixel coverage with respect to the coverage of the previous pixel
} alpha_step;


class AlphaLigne {
public:
  // bounds of the line
  // necessary since the visible portion of the canvas is bounded, and you need to compute
  // the value of the pixel "just before the visible portion of the line"
	int          min,max;
	int          length;

  // before is the step containing the delta relative to a pixel infinitely far on the left of the line
  // thus the initial pixel coverage is before.delta
	alpha_step   before,after;
  // array of steps
	int          nbStep,maxStep;
	alpha_step*  steps;
	
  // bounds of the portion of the line that has received some coverage
	int          curMin,curMax;

  // iMin and iMax are the bounds of the visible portion of the line
	AlphaLigne(int iMin,int iMax);
    virtual ~AlphaLigne(void);

  // empties the line
	void             Reset(void);
  
  // add some coverage.
  // pente is (eval-sval)/(epos-spos), because you can compute it once per edge, and thus spare the
  // CPU some potentially costly divisions
	int              AddBord(float spos,float sval,float epos,float eval,float iPente);
  // version where you don't have the pente parameter
	int              AddBord(float spos,float sval,float epos,float eval);

  // sorts the steps in increasing order. needed before you raster the line
	void             Flatten(void);
	
  // debug dump of the steps
	void						 Affiche(void);

  // private
	void             AddRun(int st,float pente);

  // raster the line in the buffer given in "dest", with the rasterization primitive worker
  // worker() is given the color parameter each time it is called. the type of the function is 
  // defined in LivarotDefs.h
	void             Raster(raster_info &dest,void* color,RasterInRunFunc worker);
	
  // also private. that's the comparison function given to qsort()
	static int       CmpStep(const void * p1, const void * p2) {
		alpha_step* d1=(alpha_step*)p1;
		alpha_step* d2=(alpha_step*)p2;
		return  d1->x - d2->x ;
	};
};


#endif


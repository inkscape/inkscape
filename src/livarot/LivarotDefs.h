/*
 *  LivarotDefs.h
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#ifndef my_defs
#define my_defs

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>

// error codes (mostly obsolete)
enum
{
  avl_no_err = 0,		// 0 is the error code for "everything OK"
  avl_bal_err = 1,
  avl_rm_err = 2,
  avl_ins_err = 3,
  shape_euler_err = 4,		// computations result in a non-eulerian graph, thus the function cannot do a proper polygon
  // despite the rounding sheme, this still happen with uber-complex graphs
  // note that coordinates are stored in double => double precision for the computation is not even
  // enough to get exact results (need quadruple precision, i think).
  shape_input_err = 5,		// the function was given an incorrect input (not a polygon, or not eulerian)
  shape_nothing_to_do = 6		// the function had nothing to do (zero offset, etc)
};

// return codes for the find function in the AVL tree (private)
enum
{
  not_found = 0,
  found_exact = 1,
  found_on_left = 2,
  found_on_right = 3,
  found_between = 4
};

// boolean operation
enum bool_op
{
  bool_op_union,		// A OR B
  bool_op_inters,		// A AND B
  bool_op_diff,			// A \ B
  bool_op_symdiff,  // A XOR B
  bool_op_cut,      // coupure (pleines)
  bool_op_slice     // coupure (contour)
};
typedef enum bool_op BooleanOp;

// types of cap for stroking polylines
enum butt_typ
{
  butt_straight,		// straight line
  butt_square,			// half square
  butt_round,			// half circle
  butt_pointy			// a little pointy hat
};
// types of joins for stroking paths
enum join_typ
{
  join_straight,		// a straight line
  join_round,			// arc of circle (in fact, one or two quadratic bezier curve chunks)
  join_pointy			// a miter join (uses the miter parameter)
};
typedef enum butt_typ ButtType;
typedef enum join_typ JoinType;

enum fill_typ
{
  fill_oddEven   = 0,
  fill_nonZero   = 1,
  fill_positive  = 2,
  fill_justDont = 3
};
typedef enum fill_typ FillRule;

// stupid version of dashes: in dash x is plain, dash x+1 must be empty, so the gap field is extremely redundant
typedef struct one_dash
{
  bool gap;
  double length;
}
one_dash;

// color definition structures for the rasterizations primitives (not present here)
typedef struct std_color
{
  uint32_t uCol;
  uint16_t iColA, iColR, iColG, iColB;
  double fColA, fColR, fColG, fColB;
  uint32_t iColATab[256];
}
std_color;

typedef struct grad_stop
{
  double at;
  double ca, cr, cg, cb;
  double iSize;
}
grad_stop;

// linear gradient for filling polygons
typedef struct lin_grad
{
  int type;			// 0= gradient appears once
  // 1= repeats itself start-end/start-end/start-end...
  // 2= repeats itself start-end/end-start/start-end...
  double u, v, w;		// u*x+v*y+w = position in the gradient (clipped to [0;1])
//      double       caa,car,cag,cab; // color at gradient position 0
//      double       cba,cbr,cbg,cbb; // color at gradient position 1
  int nbStop;
  grad_stop stops[2];
}
lin_grad;

// radial gradient (color is funciton of r^2, need to be corrected with a sqrt() to be r)
typedef struct rad_grad
{
  int type;			// 0= gradient appears once
  // 1= repeats itself start-end/start-end/start-end...
  // 2= repeats itself start-end/end-start/start-end...
  double mh, mv;			// center
  double rxx, rxy, ryx, ryy;	// 1/radius
  int nbStop;
  grad_stop stops[2];
}
rad_grad;

// functions types for an arbitrary filling shader
typedef void (*InitColorFunc) (int ph, int pv, void *);	// init for position ph,pv; the last parameter is a pointer
						     // on the gen_color structure
typedef void (*NextPixelColorFunc) (void *);	// go to next pixel and update the color
typedef void (*NextLigneColorFunc) (void *);	// go to next line (the h-coordinate must be the ph passed in
					    // the InitColorFunc)
typedef void (*GotoPixelColorFunc) (int ph, void *);	// move to h-coordinate ph
typedef void (*GotoLigneColorFunc) (int pv, void *);	// move to v-coordinate pv (the h-coordinate must be the ph passed
						  // in the InitColorFunc)

// an arbitrary shader
typedef struct gen_color
{
  double colA, colR, colG, colB;
  InitColorFunc iFunc;
  NextPixelColorFunc npFunc;
  NextLigneColorFunc nlFunc;
  GotoPixelColorFunc gpFunc;
  GotoLigneColorFunc glFunc;
}
gen_color;

// info for a run of pixel to fill
typedef struct raster_info {
		int       startPix,endPix;  // start and end pixel from the polygon POV
		int       sth,stv;          // coordinates for the first pixel in the run, in (possibly another) POV
		uint32_t* buffer;           // pointer to the first pixel in the run
} raster_info;
typedef void (*RasterInRunFunc) (raster_info &dest,void *data,int nst,float vst,int nen,float ven);	// init for position ph,pv; the last parameter is a pointer


enum Side {
    LEFT = 0,
    RIGHT = 1
};

enum FirstOrLast {
    FIRST = 0,
    LAST = 1
};

#endif

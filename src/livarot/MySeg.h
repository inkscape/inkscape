/*
 *  MySeg.h
 *  nlivarot
 *
 *  Created by fred on Wed Nov 12 2003.
 *
 */

#ifndef my_math_seg
#define my_math_seg

#include "MyMath.h"

// codes for the intersections computations
//  pt = point
//  seg = segment
//  dmd = half line
//  dr = infinite line
enum
{
  inters_seg_seg,		// intersection between 2 segments
  inters_seg_dmd,		// intersection between one segment (parameter no 1) and one half-line (parameter no 2)
  inters_seg_dr,		// ....
  inters_dmd_dmd,
  inters_dmd_dr,
  inters_dr_dr,

  inters_seg_pt,		// "intersection" between segment and point=  "does the segment contain the point?"
  inters_dmd_pt,
  inters_dr_pt,

  inters_orseg_pt		// don't use
};

// return codes for the intersection computations; build as a concatenation of
// _a = first parameter
// _b = second parameter
// _st = start of the segment/half-line (lines don't have starts)
// _en = end of thz segment (half-lines and lines don't have ends)
// _mi = inside of the segment/half-line/line
// _colinear = this flag is set if the intersection of the 2 parameter is more than a point
// the first 2 bits of the return code contain the position of the intersection on the first parameter (_st, _mi or _en)
// the next 2 bits of the return code contain the position of the intersection on the second parameter (_st, _mi or _en)
// the 5th bit is set if the parameters are colinear
enum
{
  inters_a_st = 1,
  inters_a_mi = 2,
  inters_a_en = 3,
  inters_b_st = 4,
  inters_b_mi = 8,
  inters_b_en = 12,
  inters_colinear = 16
};


//
// a class to describe a segment: defined by its startpoint p and its direction d
// if the object is considered as a segment: p+xd, where x ranges from 0 to 1
// if the object is considered as an half-line, the length of the direction vector doesn't matter:
// p+xd, where x ranges from 0 to +infinity
// if the object is considered as a line: p+xd, where x ranges from -infinity to +infinity
//
class L_SEG
{
public:
  vec2d p, d;

  // constructors
  L_SEG (vec2d & st, vec2d & dir):p (st), d (dir)
  {
  };				// by default, you give one startpoint and one direction
  L_SEG (void)
  {
  };
  ~L_SEG (void)
  {
  };

  // assignations
  void Set (L_SEG * s)
  {
    p = s->p;
    d = s->d;
  };
  void Set (L_SEG & s)
  {
    p = s.p;
    d = s.d;
  };
  // 2 specific assignations:
  // assignation where you give the startpoint and the direction (like in the constructor):
  void SetSD (vec2d & st, vec2d & dir)
  {
    p = st;
    d = dir;
  };
  // assignation where you give the startpoint and the endpoint:
  void SetSE (vec2d & st, vec2d & en)
  {
    p = st;
    d.x = en.x - st.x;
    d.y = en.y - st.y;
  };

  // reverses the segment
  void Rev (void)
  {
    p.x += d.x;
    p.y += d.y;
    d.x = -d.x;
    d.y = -d.y;
  };
  // transitibve version: the reversed segment is stored in s
  void Rev (L_SEG & s)
  {
    s.p.x = p.x + d.x;
    s.p.y = p.y + d.y;
    s.d.x = -d.x;
    s.d.y = -d.y;
  };

  // distance of the point iv to the segment/half-line/line
  // the mode parameter specifies how the caller instance should be handled:
  // inters_seg_pt : segment
  // inters_dmd_pt : half-line
  // inters_dr_pt : line
  void Distance (vec2d & iv, double &d, int mode = inters_dr_pt);
  // distance between 2 segments
  // mode parameter specifies how the segments have to be treated (just like above)
  void Distance (L_SEG & is, double &d, int mode = inters_seg_seg);

  // tests if the segment contains the point pos
  // mode is as in the Distance function
  int Contains (vec2d & pos, int mode);

  // intersection between 2 lines/half-lines/segments
  // mode specifies how the L_SEG instances have to be considered; codes at the beginning of this file
  // the "at" parameter stores the intersection point, if it exists and is unique
  static int Intersect (L_SEG & iu, L_SEG & iv, int mode);
  static int Intersect (L_SEG & iu, L_SEG & iv, vec2d & at, int mode);
  // specific version, when you can garantuee the colinearity case won't occur
  static int IntersectGeneral (L_SEG & iu, L_SEG & iv, vec2d & at, int mode);
};


#endif

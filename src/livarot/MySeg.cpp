/*
 *  MySeg.cpp
 *  nlivarot
 *
 *  Created by fred on Wed Nov 12 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "MySeg.h"
#include <math.h>

void
L_SEG::Distance (L_SEG & is, double &di, int mode)
{
  if (mode == inters_seg_seg)
    {
      double ms, me, os, oe;

      vec2d en = is.p;
      L_VEC_Add (en, is.d, en);

      Distance (is.p, os, inters_seg_pt);
      Distance (en, oe, inters_seg_pt);

      en = p;
      L_VEC_Add (en, d, en);

      is.Distance (p, ms, inters_orseg_pt);
      is.Distance (en, me, inters_orseg_pt);
      if (L_VAL_Zero (ms) < 0 || L_VAL_Zero (me) < 0)
	{
	  di = 1000000;
	  return;
	}
      if (L_VAL_Cmp (oe, os) < 0)
	os = oe;
      if (L_VAL_Cmp (me, ms) < 0)
	ms = me;
      if (L_VAL_Cmp (ms, os) < 0)
	os = ms;
      di = os;
    }
  else if (mode == inters_seg_dmd)
    {
    }
  else if (mode == inters_seg_dr)
    {
    }
}

void
L_SEG::Distance (vec2d & iv, double &di, int mode)
{
  if (L_VAL_Zero (d.x) == 0 && L_VAL_Zero (d.y) == 0)
    {
      L_VEC_Distance (p, iv, di);
      return;
    }
  double dd, sqd;
  vec2d nd = d;
  L_VEC_RotCW (nd);
  L_VEC_Cross (nd, nd, dd);
  sqd = sqrt (dd);
  vec2d diff = iv;
  L_VEC_Sub (diff, p, diff);
  if (mode == inters_dr_pt)
    {
      double cp;
      L_VEC_Cross (diff, nd, cp);
      di = cp / sqd;
      if (di < 0)
	di = -di;
    }
  else if (mode == inters_dmd_pt)
    {
      double cp;
      L_VEC_Cross (diff, d, cp);
      if (cp < 0)
	{
	  L_VEC_Distance (p, iv, di);
	  return;
	}
      L_VEC_Cross (diff, nd, cp);
      di = cp / sqd;
      if (di < 0)
	di = -di;
    }
  else if (mode == inters_seg_pt)
    {
      double cp;
      L_VEC_Cross (diff, d, cp);
      if (cp < 0)
	{
	  L_VEC_Distance (p, iv, di);
	  return;
	}
      if (cp > dd)
	{
	  vec2d se = p;
	  L_VEC_Add (se, d, se);
	  L_VEC_Distance (se, iv, di);
	  return;
	}
      L_VEC_Cross (diff, nd, cp);
      di = cp / sqd;
      if (di < 0)
	di = -di;
    }
  else if (mode == inters_orseg_pt)
    {
      double cp;
      L_VEC_Cross (diff, d, cp);
      if (cp < 0)
	{
	  L_VEC_Distance (p, iv, di);
	  L_VEC_Dot (diff, d, cp);
	  if (L_VAL_Zero (cp) < 0)
	    di = -di;
	  return;
	}
      if (cp > dd)
	{
	  vec2d se = p;
	  L_VEC_Add (se, d, se);
	  L_VEC_Distance (se, iv, di);

	  L_VEC_Dot (diff, d, cp);
	  if (cp < 0)
	    di = -di;
	  return;
	}
      L_VEC_Cross (diff, nd, cp);
      di = cp / sqd;
//              if ( diL_VAL_Zero() < 0 ) di.Neg();
    }
}

int
L_SEG::Intersect (L_SEG & iu, L_SEG & iv, int mode)
{
  double iudd, ivdd;
  L_VEC_Cross (iu.d, iu.d, iudd);
  L_VEC_Cross (iv.d, iv.d, ivdd);
  if (L_VAL_Zero (iudd) <= 0)
    return 0;			// cas illicite
  if (L_VAL_Zero (ivdd) <= 0)
    return 0;			// cas illicite

  vec2d usvs, uevs, usve, ueve;
  L_VEC_Sub (iv.p, iu.p, usvs);
  L_VEC_Sub (usvs, iu.d, uevs);
  L_VEC_Add (usvs, iv.d, usve);
  L_VEC_Sub (usve, iu.d, ueve);
  double usvsl, uevsl, usvel, uevel;
  L_VEC_Cross (usvs, usvs, usvsl);
  L_VEC_Cross (uevs, uevs, uevsl);
  L_VEC_Cross (usve, usve, usvel);
  L_VEC_Cross (ueve, ueve, uevel);

  double dd;
  L_VEC_Cross (iu.d, iv.d, dd);

  if (L_VAL_Zero (usvsl) <= 0)
    {
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_st + inters_b_st;
	}
      else if (mode == inters_dmd_dmd || mode == inters_seg_dmd
	       || mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_colinear + inters_a_st + inters_b_st;
	    }
	  else
	    {
	      return inters_a_st + inters_b_st;
	    }
	}
      return 0;
    }
  if (L_VAL_Zero (uevsl) <= 0)
    {
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_en + inters_b_st;
	}
      else if (mode == inters_dmd_dmd)
	{
	  return inters_colinear + inters_a_en + inters_b_st;
	}
      else if (mode == inters_seg_dmd || mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_a_en + inters_b_st;
	    }
	  else
	    {
	      return inters_colinear + inters_a_en + inters_b_st;
	    }
	}
      return 0;
    }
  if (L_VAL_Zero (usvel) <= 0)
    {
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_st + inters_b_en;
	}
      else if (mode == inters_dmd_dmd || mode == inters_seg_dmd)
	{
	  return inters_colinear + inters_a_st + inters_b_en;
	}
      else if (mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_a_st + inters_b_en;
	    }
	  else
	    {
	      return inters_colinear + inters_a_st + inters_b_en;
	    }
	}
      return 0;
    }
  if (L_VAL_Zero (uevel) <= 0)
    {
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_en + inters_b_en;
	}
      else if (mode == inters_dmd_dmd || mode == inters_seg_dmd)
	{
	  return inters_colinear + inters_a_en + inters_b_en;
	}
      else if (mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_colinear + inters_a_en + inters_b_en;
	    }
	  else
	    {
	      return inters_a_en + inters_b_en;
	    }
	}
      return 0;
    }

  // plus d'extremites en commun a partir de ce point

  mat2d m;
  L_MAT_SetC (m, iu.d, iv.d);
  double det;
  L_MAT_Det (m, det);

  if (L_VAL_Zero (det) == 0)
    {				// ces couillons de vecteurs sont colineaires
      vec2d iudp;
      iudp.x = iu.d.y;
      iudp.y = -iu.d.x;
      double dist;
      L_VEC_Cross (iudp, usvs, dist);
      if (L_VAL_Zero (dist) == 0)
	{
	  if (mode == inters_dr_dr || mode == inters_dmd_dr
	      || mode == inters_seg_dr)
	    {
	      return inters_colinear;
	    }
	  else if (mode == inters_dmd_dmd)
	    {
	      if (L_VAL_Zero (dd) > 0)
		return inters_colinear;
	      double ts;
	      L_VEC_Cross (iu.d, usvs, ts);
	      if (L_VAL_Zero (ts) > 0)
		return inters_colinear;
	      return 0;
	    }
	  else if (mode == inters_seg_dmd)
	    {
	      if (L_VAL_Zero (dd) > 0)
		{
		  double ts;
		  L_VEC_Cross (iv.d, uevs, ts);
		  if (L_VAL_Zero (ts) < 0)
		    return inters_colinear;
		  return 0;
		}
	      else
		{
		  double ts;
		  L_VEC_Cross (iv.d, usvs, ts);
		  if (L_VAL_Zero (ts) < 0)
		    return inters_colinear;
		  return 0;
		}
	    }
	  else if (mode == inters_seg_seg)
	    {
	      double ts, te;
	      L_VEC_Cross (iu.d, usvs, ts);
	      L_VEC_Cross (iu.d, uevs, te);
	      if (L_VAL_Zero (ts) > 0 && L_VAL_Zero (te) < 0)
		return inters_colinear;
	      L_VEC_Cross (iu.d, usve, ts);
	      L_VEC_Cross (iu.d, ueve, te);
	      if (L_VAL_Zero (ts) > 0 && L_VAL_Zero (te) < 0)
		return inters_colinear;
	      L_VEC_Cross (iv.d, usvs, ts);
	      L_VEC_Cross (iv.d, usve, te);
	      if (L_VAL_Zero (ts) < 0 && L_VAL_Zero (te) > 0)
		return inters_colinear;
	      L_VEC_Cross (iv.d, uevs, ts);
	      L_VEC_Cross (iv.d, ueve, te);
	      if (L_VAL_Zero (ts) < 0 && L_VAL_Zero (te) > 0)
		return inters_colinear;
	      return 0;
	    }
	}
      else
	{
	  return 0;		// paralleles
	}
    }

  // plus de colinearite ni d'extremites en commun
  L_MAT_Inv (m);
  vec2d res;
  L_MAT_MulV (m, usvs, res);

  if (mode == inters_dr_dr)
    {
      return inters_a_mi + inters_b_mi;
    }
  else if (mode == inters_dmd_dr)
    {
      int i = L_VAL_Zero (res.x);
      if (i == 0)
	return inters_a_st + inters_b_mi;
      if (i > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_dr)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      if (i == 0)
	return inters_a_st + inters_b_mi;
      if (j == 0)
	return inters_a_en + inters_b_mi;
      if (i > 0 && j < 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_dmd_dmd)
    {
      int i = L_VAL_Zero (res.x);
      int j = -(L_VAL_Zero (res.y));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && j > 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && i > 0)
	return inters_a_mi + inters_b_st;
      if (i > 0 && j > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_dmd)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      int k = -(L_VAL_Zero (res.y));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && k > 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && k > 0)
	return inters_a_en + inters_b_mi;
      if (i > 0 && j < 0 && k == 0)
	return inters_a_mi + inters_b_st;
      if (i > 0 && j < 0 && k > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_seg)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      int k = -(L_VAL_Zero (res.y));
      int l = -(L_VAL_Cmp (res.y, 1));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && k > 0 && l < 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && k > 0 && l < 0)
	return inters_a_en + inters_b_mi;
      if (k == 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_st;
      if (l == 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_en;
      if (k > 0 && l < 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }

  return 0;
}

int
L_SEG::Intersect (L_SEG & iu, L_SEG & iv, vec2d & at, int mode)
{
  double iudd, ivdd;
  L_VEC_Cross (iu.d, iu.d, iudd);
  L_VEC_Cross (iv.d, iv.d, ivdd);
  if (L_VAL_Zero (iudd) <= 0)
    return 0;			// cas illicite
  if (L_VAL_Zero (ivdd) <= 0)
    return 0;			// cas illicite

  vec2d usvs, uevs, usve, ueve;
  L_VEC_Sub (iv.p, iu.p, usvs);
  L_VEC_Sub (usvs, iu.d, uevs);
  L_VEC_Add (usvs, iv.d, usve);
  L_VEC_Sub (usve, iu.d, ueve);
  double usvsl, uevsl, usvel, uevel;
  L_VEC_Cross (usvs, usvs, usvsl);
  L_VEC_Cross (uevs, uevs, uevsl);
  L_VEC_Cross (usve, usve, usvel);
  L_VEC_Cross (ueve, ueve, uevel);

  double dd;
  L_VEC_Cross (iu.d, iv.d, dd);

  if (L_VAL_Zero (usvsl) <= 0)
    {
      at = iu.p;
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_st + inters_b_st;
	}
      else if (mode == inters_dmd_dmd || mode == inters_seg_dmd
	       || mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_colinear + inters_a_st + inters_b_st;
	    }
	  else
	    {
	      return inters_a_st + inters_b_st;
	    }
	}
      return 0;
    }
  if (L_VAL_Zero (uevsl) <= 0)
    {
      at = iv.p;
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_en + inters_b_st;
	}
      else if (mode == inters_dmd_dmd)
	{
	  return inters_colinear + inters_a_en + inters_b_st;
	}
      else if (mode == inters_seg_dmd || mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_a_en + inters_b_st;
	    }
	  else
	    {
	      return inters_colinear + inters_a_en + inters_b_st;
	    }
	}
      return 0;
    }
  if (L_VAL_Zero (usvel) <= 0)
    {
      at = iu.p;
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_st + inters_b_en;
	}
      else if (mode == inters_dmd_dmd || mode == inters_seg_dmd)
	{
	  return inters_colinear + inters_a_st + inters_b_en;
	}
      else if (mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_a_st + inters_b_en;
	    }
	  else
	    {
	      return inters_colinear + inters_a_st + inters_b_en;
	    }
	}
      return 0;
    }
  if (L_VAL_Zero (uevel) <= 0)
    {
      at = iu.p;
      L_VEC_Add (at, iu.d, at);
      if (mode == inters_dr_dr || mode == inters_dmd_dr
	  || mode == inters_seg_dr)
	{
	  return inters_colinear + inters_a_en + inters_b_en;
	}
      else if (mode == inters_dmd_dmd || mode == inters_seg_dmd)
	{
	  return inters_colinear + inters_a_en + inters_b_en;
	}
      else if (mode == inters_seg_seg)
	{
	  if (L_VAL_Zero (dd) > 0)
	    {
	      return inters_colinear + inters_a_en + inters_b_en;
	    }
	  else
	    {
	      return inters_a_en + inters_b_en;
	    }
	}
      return 0;
    }

  // plus d'extremites en commun a partir de ce point

  mat2d m;
  L_MAT_SetC (m, iu.d, iv.d);
  double det;
  L_MAT_Det (m, det);

  if (L_VAL_Zero (det) == 0)
    {				// ces couillons de vecteurs sont colineaires
      vec2d iudp;
      iudp.x = iu.d.y;
      iudp.y = -iu.d.x;
      double dist;
      L_VEC_Cross (iudp, usvs, dist);
      if (L_VAL_Zero (dist) == 0)
	{
	  if (mode == inters_dr_dr || mode == inters_dmd_dr
	      || mode == inters_seg_dr)
	    {
	      return inters_colinear;
	    }
	  else if (mode == inters_dmd_dmd)
	    {
	      if (L_VAL_Zero (dd) > 0)
		return inters_colinear;
	      double ts;
	      L_VEC_Cross (iu.d, usvs, ts);
	      if (L_VAL_Zero (ts) > 0)
		return inters_colinear;
	      return 0;
	    }
	  else if (mode == inters_seg_dmd)
	    {
	      if (L_VAL_Zero (dd) > 0)
		{
		  double ts;
		  L_VEC_Cross (iv.d, uevs, ts);
		  if (L_VAL_Zero (ts) < 0)
		    return inters_colinear;
		  return 0;
		}
	      else
		{
		  double ts;
		  L_VEC_Cross (iv.d, usvs, ts);
		  if (L_VAL_Zero (ts) < 0)
		    return inters_colinear;
		  return 0;
		}
	    }
	  else if (mode == inters_seg_seg)
	    {
	      double ts, te;
	      L_VEC_Cross (iu.d, usvs, ts);
	      L_VEC_Cross (iu.d, uevs, te);
	      if (L_VAL_Zero (ts) > 0 && L_VAL_Zero (te) < 0)
		return inters_colinear;
	      L_VEC_Cross (iu.d, usve, ts);
	      L_VEC_Cross (iu.d, ueve, te);
	      if (L_VAL_Zero (ts) > 0 && L_VAL_Zero (te) < 0)
		return inters_colinear;
	      L_VEC_Cross (iv.d, usvs, ts);
	      L_VEC_Cross (iv.d, usve, te);
	      if (L_VAL_Zero (ts) < 0 && L_VAL_Zero (te) > 0)
		return inters_colinear;
	      L_VEC_Cross (iv.d, uevs, ts);
	      L_VEC_Cross (iv.d, ueve, te);
	      if (L_VAL_Zero (ts) < 0 && L_VAL_Zero (te) > 0)
		return inters_colinear;
	      return 0;
	    }
	}
      else
	{
	  return 0;		// paralleles
	}
    }

  // plus de colinearite ni d'extremites en commun
  L_MAT_Inv (m);
  vec2d res;
  L_MAT_MulV (m, usvs, res);

  // l'intersection
  L_VEC_MulC (iu.d, res.x, at);
  L_VEC_Add (at, iu.p, at);

  if (mode == inters_dr_dr)
    {
      return inters_a_mi + inters_b_mi;
    }
  else if (mode == inters_dmd_dr)
    {
      int i = L_VAL_Zero (res.x);
      if (i == 0)
	return inters_a_st + inters_b_mi;
      if (i > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_dr)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      if (i == 0)
	return inters_a_st + inters_b_mi;
      if (j == 0)
	return inters_a_en + inters_b_mi;
      if (i > 0 && j < 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_dmd_dmd)
    {
      int i = L_VAL_Zero (res.x);
      int j = -(L_VAL_Zero (res.y));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && j > 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && i > 0)
	return inters_a_mi + inters_b_st;
      if (i > 0 && j > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_dmd)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      int k = -(L_VAL_Zero (res.y));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && k > 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && k > 0)
	return inters_a_en + inters_b_mi;
      if (i > 0 && j < 0 && k == 0)
	return inters_a_mi + inters_b_st;
      if (i > 0 && j < 0 && k > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_seg)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      int k = -(L_VAL_Zero (res.y));	// la coordonnée sur iv est inversee
      int l = -(L_VAL_Cmp (res.y, -1));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && k > 0 && l < 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && k > 0 && l < 0)
	return inters_a_en + inters_b_mi;
      if (k == 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_st;
      if (l == 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_en;
      if (k > 0 && l < 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }

  return 0;
}

int
L_SEG::IntersectGeneral (L_SEG & iu, L_SEG & iv, vec2d & at, int mode)
{

  vec2d usvs;
  L_VEC_Sub (iv.p, iu.p, usvs);

  double dd;
  L_VEC_Cross (iu.d, iv.d, dd);


  mat2d m;
  L_MAT_SetC (m, iu.d, iv.d);
  double det;
  L_MAT_Det (m, det);

  if (L_VAL_Zero (det))
    {				// ces couillons de vecteurs sont colineaires
      return 0;			// paralleles
    }

  // plus de colinearite ni d'extremites en commun
  L_MAT_Inv (m);
  vec2d res;
  L_MAT_MulV (m, usvs, res);

  // l'intersection
  L_VEC_MulC (iu.d, res.x, at);
  L_VEC_Add (at, iu.p, at);

  if (mode == inters_dr_dr)
    {
      return inters_a_mi + inters_b_mi;
    }
  else if (mode == inters_dmd_dr)
    {
      int i = L_VAL_Zero (res.x);
      if (i == 0)
	return inters_a_st + inters_b_mi;
      if (i > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_dr)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      if (i == 0)
	return inters_a_st + inters_b_mi;
      if (j == 0)
	return inters_a_en + inters_b_mi;
      if (i > 0 && j < 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_dmd_dmd)
    {
      int i = L_VAL_Zero (res.x);
      int j = -(L_VAL_Zero (res.y));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && j > 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && i > 0)
	return inters_a_mi + inters_b_st;
      if (i > 0 && j > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_dmd)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      int k = -(L_VAL_Zero (res.y));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && k > 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && k > 0)
	return inters_a_en + inters_b_mi;
      if (i > 0 && j < 0 && k == 0)
	return inters_a_mi + inters_b_st;
      if (i > 0 && j < 0 && k > 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }
  else if (mode == inters_seg_seg)
    {
      int i = L_VAL_Zero (res.x);
      int j = L_VAL_Cmp (res.x, 1);
      int k = -(L_VAL_Zero (res.y));	// la coordonnée sur iv est inversee
      int l = -(L_VAL_Cmp (res.y, -1));
      // nota : i=0 et j=0 a ete elimine au debut
      if (i == 0 && k > 0 && l < 0)
	return inters_a_st + inters_b_mi;
      if (j == 0 && k > 0 && l < 0)
	return inters_a_en + inters_b_mi;
      if (k == 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_st;
      if (l == 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_en;
      if (k > 0 && l < 0 && i > 0 && j < 0)
	return inters_a_mi + inters_b_mi;
      return 0;
    }

  return 0;
}

int
L_SEG::Contains (vec2d & pos, int mode)
{
  vec2d sp, ep;
  L_VEC_Sub (pos, p, sp);
  L_VEC_Sub (sp, d, ep);
  double spl, epl;
  L_VEC_Cross (sp, sp, spl);
  L_VEC_Cross (ep, ep, epl);
  if (L_VAL_Zero (spl) == 0)
    {
      if (mode == inters_dr_pt)
	return inters_a_mi;
      return inters_a_st;
    }
  if (L_VAL_Zero (epl) == 0)
    {
      if (mode == inters_dr_pt || mode == inters_dmd_pt)
	return inters_a_mi;
      return inters_a_en;
    }

  vec2d perp = d;
  L_VEC_RotCW (perp);

  double dd, ps;
  L_VEC_Cross (d, d, dd);
  L_VEC_Cross (perp, sp, ps);
  if (L_VAL_Zero (ps) == 0)
    {				// sur la droite
      if (mode == inters_dr_pt)
	return inters_a_mi;
      L_VEC_Cross (d, sp, ps);
      // ps != 0 car le cas est traité avant
      if (mode == inters_dmd_pt)
	{
	  if (L_VAL_Zero (ps) > 0)
	    {
	      return inters_a_mi;
	    }
	}
      else
	{
	  if (L_VAL_Zero (ps) > 0)
	    {
	      if (L_VAL_Cmp (ps, dd) < 0)
		{
		  return inters_a_mi;
		}
	    }
	}
    }

  return 0;
}

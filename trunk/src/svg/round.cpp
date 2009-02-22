/////////////////////////////////////////////////////////////////////////
//                               ftos.cc
//
// Copyright (c) 1996-2003 Bryce W. Harrington  [bryce at osdl dot org]
//
//-----------------------------------------------------------------------
// License:  This code may be used by anyone for any purpose
//           so long as the copyright notices and this license
//           statement remains attached.
//-----------------------------------------------------------------------
// This routine rounds a double using the "rounding rule", as expressed
// in _Advanced Engineering Mathematics_ by Erwin Kreyszig, 6th ed., 
// John Wiley & Sons, Inc., 1988, page 945.
//
// Discard the (k+1)th and all subsequent decimals.
//  (a) If the number thus discarded is less than half a unit in the
//      kth place, leave the kth decimal unchanged ("rounding down")
//  (b) If it is greater than half a unit in the kth place, add one
//      to the kth decimal ("rounding up")
//  (c) If it is exactly half a unit, round off to the nearest *even* 
//      decimal.
//  Example:  Rounding off 3.45 and 3.55 by one decimal gives 3.4 and
//      3.6, respectively.
//  Rule (c) is to ensure that in discarding exactly half a decimal,
//      rounding up and rounding down happens about equally often,
//      on the average.
///////////////////////////////////////////////////////////////////////
#include <math.h>

double rround(double x)
{
	double xlow = floor(x);
	if (x - xlow != 0.5000)
		return floor(x + 0.5000);
	else if ( floor(x/2.0) == xlow/2.0)
		return xlow;
	else
		return xlow++;		
}

// This version allows rounding to a specific digit
double rround(double x, int k)
{
	if (k==0) return rround(x);
	else return rround(x*pow(10,k)) / pow(10,k);
}


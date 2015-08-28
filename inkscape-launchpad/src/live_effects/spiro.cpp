/*
Copyright (C) 2007-2012 Authors

Authors: Raph Levien
         Johan Engelen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/
/* C implementation of third-order polynomial spirals. */

#include "spiro.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "display/curve.h"
#include <2geom/math-utils.h>

#define SPIRO_SHOW_INFINITE_COORDINATE_CALLS

namespace Spiro {

void spiro_run(const spiro_cp *src, int src_len, SPCurve &curve)
{
    spiro_seg *s = Spiro::run_spiro(src, src_len);
    Spiro::ConverterSPCurve bc(curve);
    Spiro::spiro_to_otherpath(s, src_len, bc);
    free(s);
}

void spiro_run(const spiro_cp *src, int src_len, Geom::Path &path)
{
    spiro_seg *s = Spiro::run_spiro(src, src_len);
    Spiro::ConverterPath bc(path);
    Spiro::spiro_to_otherpath(s, src_len, bc);
    free(s);
}


/************************************
 * Spiro math
 */

struct spiro_seg_s {
    double x;
    double y;
    char ty;
    double bend_th;
    double ks[4];
    double seg_ch;
    double seg_th;
    double l;
};

typedef struct {
    double a[11]; /* band-diagonal matrix */
    double al[5]; /* lower part of band-diagonal decomposition */
} bandmat;

#ifndef M_PI
#define M_PI            3.14159265358979323846  /* pi */
#endif

int n = 4;

#ifndef ORDER
#define ORDER 12
#endif

/* Integrate polynomial spiral curve over range -.5 .. .5. */
static void
integrate_spiro(const double ks[4], double xy[2])
{
#if 0
    int n = 1024;
#endif
    double th1 = ks[0];
    double th2 = .5 * ks[1];
    double th3 = (1./6) * ks[2];
    double th4 = (1./24) * ks[3];
    double x, y;
    double ds = 1. / n;
    double ds2 = ds * ds;
    double ds3 = ds2 * ds;
    double k0 = ks[0] * ds;
    double k1 = ks[1] * ds;
    double k2 = ks[2] * ds;
    double k3 = ks[3] * ds;
    int i;
    double s = .5 * ds - .5;

    x = 0;
    y = 0;

    for (i = 0; i < n; i++) {

#if ORDER > 2
	double u, v;
	double km0, km1, km2, km3;

	if (n == 1) {
	    km0 = k0;
	    km1 = k1 * ds;
	    km2 = k2 * ds2;
	} else {
	    km0 = (((1./6) * k3 * s + .5 * k2) * s + k1) * s + k0;
	    km1 = ((.5 * k3 * s + k2) * s + k1) * ds;
	    km2 = (k3 * s + k2) * ds2;
	}
	km3 = k3 * ds3;
#endif

	{

#if ORDER == 4
	double km0_2 = km0 * km0;
	u = 24 - km0_2;
	v = km1;
#endif

#if ORDER == 6
	double km0_2 = km0 * km0;
	double km0_4 = km0_2 * km0_2;
	u = 24 - km0_2 + (km0_4 - 4 * km0 * km2 - 3 * km1 * km1) * (1./80);
	v = km1 + (km3 - 6 * km0_2 * km1) * (1./80);
#endif

#if ORDER == 8
	double t1_1 = km0;
	double t1_2 = .5 * km1;
	double t1_3 = (1./6) * km2;
	double t1_4 = (1./24) * km3;
	double t2_2 = t1_1 * t1_1;
	double t2_3 = 2 * (t1_1 * t1_2);
	double t2_4 = 2 * (t1_1 * t1_3) + t1_2 * t1_2;
	double t2_5 = 2 * (t1_1 * t1_4 + t1_2 * t1_3);
	double t2_6 = 2 * (t1_2 * t1_4) + t1_3 * t1_3;
	double t3_4 = t2_2 * t1_2 + t2_3 * t1_1;
	double t3_6 = t2_2 * t1_4 + t2_3 * t1_3 + t2_4 * t1_2 + t2_5 * t1_1;
	double t4_4 = t2_2 * t2_2;
	double t4_5 = 2 * (t2_2 * t2_3);
	double t4_6 = 2 * (t2_2 * t2_4) + t2_3 * t2_3;
	double t5_6 = t4_4 * t1_2 + t4_5 * t1_1;
	double t6_6 = t4_4 * t2_2;
	u = 1;
	v = 0;
	v += (1./12) * t1_2 + (1./80) * t1_4;
	u -= (1./24) * t2_2 + (1./160) * t2_4 + (1./896) * t2_6;
	v -= (1./480) * t3_4 + (1./2688) * t3_6;
	u += (1./1920) * t4_4 + (1./10752) * t4_6;
	v += (1./53760) * t5_6;
	u -= (1./322560) * t6_6;
#endif

#if ORDER == 10
	double t1_1 = km0;
	double t1_2 = .5 * km1;
	double t1_3 = (1./6) * km2;
	double t1_4 = (1./24) * km3;
	double t2_2 = t1_1 * t1_1;
	double t2_3 = 2 * (t1_1 * t1_2);
	double t2_4 = 2 * (t1_1 * t1_3) + t1_2 * t1_2;
	double t2_5 = 2 * (t1_1 * t1_4 + t1_2 * t1_3);
	double t2_6 = 2 * (t1_2 * t1_4) + t1_3 * t1_3;
	double t2_7 = 2 * (t1_3 * t1_4);
	double t2_8 = t1_4 * t1_4;
	double t3_4 = t2_2 * t1_2 + t2_3 * t1_1;
	double t3_6 = t2_2 * t1_4 + t2_3 * t1_3 + t2_4 * t1_2 + t2_5 * t1_1;
	double t3_8 = t2_4 * t1_4 + t2_5 * t1_3 + t2_6 * t1_2 + t2_7 * t1_1;
	double t4_4 = t2_2 * t2_2;
	double t4_5 = 2 * (t2_2 * t2_3);
	double t4_6 = 2 * (t2_2 * t2_4) + t2_3 * t2_3;
	double t4_7 = 2 * (t2_2 * t2_5 + t2_3 * t2_4);
	double t4_8 = 2 * (t2_2 * t2_6 + t2_3 * t2_5) + t2_4 * t2_4;
	double t5_6 = t4_4 * t1_2 + t4_5 * t1_1;
	double t5_8 = t4_4 * t1_4 + t4_5 * t1_3 + t4_6 * t1_2 + t4_7 * t1_1;
	double t6_6 = t4_4 * t2_2;
	double t6_7 = t4_4 * t2_3 + t4_5 * t2_2;
	double t6_8 = t4_4 * t2_4 + t4_5 * t2_3 + t4_6 * t2_2;
	double t7_8 = t6_6 * t1_2 + t6_7 * t1_1;
	double t8_8 = t6_6 * t2_2;
	u = 1;
	v = 0;
	v += (1./12) * t1_2 + (1./80) * t1_4;
	u -= (1./24) * t2_2 + (1./160) * t2_4 + (1./896) * t2_6 + (1./4608) * t2_8;
	v -= (1./480) * t3_4 + (1./2688) * t3_6 + (1./13824) * t3_8;
	u += (1./1920) * t4_4 + (1./10752) * t4_6 + (1./55296) * t4_8;
	v += (1./53760) * t5_6 + (1./276480) * t5_8;
	u -= (1./322560) * t6_6 + (1./1.65888e+06) * t6_8;
	v -= (1./1.16122e+07) * t7_8;
	u += (1./9.28973e+07) * t8_8;
#endif

#if ORDER == 12
	double t1_1 = km0;
	double t1_2 = .5 * km1;
	double t1_3 = (1./6) * km2;
	double t1_4 = (1./24) * km3;
	double t2_2 = t1_1 * t1_1;
	double t2_3 = 2 * (t1_1 * t1_2);
	double t2_4 = 2 * (t1_1 * t1_3) + t1_2 * t1_2;
	double t2_5 = 2 * (t1_1 * t1_4 + t1_2 * t1_3);
	double t2_6 = 2 * (t1_2 * t1_4) + t1_3 * t1_3;
	double t2_7 = 2 * (t1_3 * t1_4);
	double t2_8 = t1_4 * t1_4;
	double t3_4 = t2_2 * t1_2 + t2_3 * t1_1;
	double t3_6 = t2_2 * t1_4 + t2_3 * t1_3 + t2_4 * t1_2 + t2_5 * t1_1;
	double t3_8 = t2_4 * t1_4 + t2_5 * t1_3 + t2_6 * t1_2 + t2_7 * t1_1;
	double t3_10 = t2_6 * t1_4 + t2_7 * t1_3 + t2_8 * t1_2;
	double t4_4 = t2_2 * t2_2;
	double t4_5 = 2 * (t2_2 * t2_3);
	double t4_6 = 2 * (t2_2 * t2_4) + t2_3 * t2_3;
	double t4_7 = 2 * (t2_2 * t2_5 + t2_3 * t2_4);
	double t4_8 = 2 * (t2_2 * t2_6 + t2_3 * t2_5) + t2_4 * t2_4;
	double t4_9 = 2 * (t2_2 * t2_7 + t2_3 * t2_6 + t2_4 * t2_5);
	double t4_10 = 2 * (t2_2 * t2_8 + t2_3 * t2_7 + t2_4 * t2_6) + t2_5 * t2_5;
	double t5_6 = t4_4 * t1_2 + t4_5 * t1_1;
	double t5_8 = t4_4 * t1_4 + t4_5 * t1_3 + t4_6 * t1_2 + t4_7 * t1_1;
	double t5_10 = t4_6 * t1_4 + t4_7 * t1_3 + t4_8 * t1_2 + t4_9 * t1_1;
	double t6_6 = t4_4 * t2_2;
	double t6_7 = t4_4 * t2_3 + t4_5 * t2_2;
	double t6_8 = t4_4 * t2_4 + t4_5 * t2_3 + t4_6 * t2_2;
	double t6_9 = t4_4 * t2_5 + t4_5 * t2_4 + t4_6 * t2_3 + t4_7 * t2_2;
	double t6_10 = t4_4 * t2_6 + t4_5 * t2_5 + t4_6 * t2_4 + t4_7 * t2_3 + t4_8 * t2_2;
	double t7_8 = t6_6 * t1_2 + t6_7 * t1_1;
	double t7_10 = t6_6 * t1_4 + t6_7 * t1_3 + t6_8 * t1_2 + t6_9 * t1_1;
	double t8_8 = t6_6 * t2_2;
	double t8_9 = t6_6 * t2_3 + t6_7 * t2_2;
	double t8_10 = t6_6 * t2_4 + t6_7 * t2_3 + t6_8 * t2_2;
	double t9_10 = t8_8 * t1_2 + t8_9 * t1_1;
	double t10_10 = t8_8 * t2_2;
	u = 1;
	v = 0;
	v += (1./12) * t1_2 + (1./80) * t1_4;
	u -= (1./24) * t2_2 + (1./160) * t2_4 + (1./896) * t2_6 + (1./4608) * t2_8;
	v -= (1./480) * t3_4 + (1./2688) * t3_6 + (1./13824) * t3_8 + (1./67584) * t3_10;
	u += (1./1920) * t4_4 + (1./10752) * t4_6 + (1./55296) * t4_8 + (1./270336) * t4_10;
	v += (1./53760) * t5_6 + (1./276480) * t5_8 + (1./1.35168e+06) * t5_10;
	u -= (1./322560) * t6_6 + (1./1.65888e+06) * t6_8 + (1./8.11008e+06) * t6_10;
	v -= (1./1.16122e+07) * t7_8 + (1./5.67706e+07) * t7_10;
	u += (1./9.28973e+07) * t8_8 + (1./4.54164e+08) * t8_10;
	v += (1./4.08748e+09) * t9_10;
	u -= (1./4.08748e+10) * t10_10;
#endif

#if ORDER == 14
	double t1_1 = km0;
	double t1_2 = .5 * km1;
	double t1_3 = (1./6) * km2;
	double t1_4 = (1./24) * km3;
	double t2_2 = t1_1 * t1_1;
	double t2_3 = 2 * (t1_1 * t1_2);
	double t2_4 = 2 * (t1_1 * t1_3) + t1_2 * t1_2;
	double t2_5 = 2 * (t1_1 * t1_4 + t1_2 * t1_3);
	double t2_6 = 2 * (t1_2 * t1_4) + t1_3 * t1_3;
	double t2_7 = 2 * (t1_3 * t1_4);
	double t2_8 = t1_4 * t1_4;
	double t3_4 = t2_2 * t1_2 + t2_3 * t1_1;
	double t3_6 = t2_2 * t1_4 + t2_3 * t1_3 + t2_4 * t1_2 + t2_5 * t1_1;
	double t3_8 = t2_4 * t1_4 + t2_5 * t1_3 + t2_6 * t1_2 + t2_7 * t1_1;
	double t3_10 = t2_6 * t1_4 + t2_7 * t1_3 + t2_8 * t1_2;
	double t3_12 = t2_8 * t1_4;
	double t4_4 = t2_2 * t2_2;
	double t4_5 = 2 * (t2_2 * t2_3);
	double t4_6 = 2 * (t2_2 * t2_4) + t2_3 * t2_3;
	double t4_7 = 2 * (t2_2 * t2_5 + t2_3 * t2_4);
	double t4_8 = 2 * (t2_2 * t2_6 + t2_3 * t2_5) + t2_4 * t2_4;
	double t4_9 = 2 * (t2_2 * t2_7 + t2_3 * t2_6 + t2_4 * t2_5);
	double t4_10 = 2 * (t2_2 * t2_8 + t2_3 * t2_7 + t2_4 * t2_6) + t2_5 * t2_5;
	double t4_11 = 2 * (t2_3 * t2_8 + t2_4 * t2_7 + t2_5 * t2_6);
	double t4_12 = 2 * (t2_4 * t2_8 + t2_5 * t2_7) + t2_6 * t2_6;
	double t5_6 = t4_4 * t1_2 + t4_5 * t1_1;
	double t5_8 = t4_4 * t1_4 + t4_5 * t1_3 + t4_6 * t1_2 + t4_7 * t1_1;
	double t5_10 = t4_6 * t1_4 + t4_7 * t1_3 + t4_8 * t1_2 + t4_9 * t1_1;
	double t5_12 = t4_8 * t1_4 + t4_9 * t1_3 + t4_10 * t1_2 + t4_11 * t1_1;
	double t6_6 = t4_4 * t2_2;
	double t6_7 = t4_4 * t2_3 + t4_5 * t2_2;
	double t6_8 = t4_4 * t2_4 + t4_5 * t2_3 + t4_6 * t2_2;
	double t6_9 = t4_4 * t2_5 + t4_5 * t2_4 + t4_6 * t2_3 + t4_7 * t2_2;
	double t6_10 = t4_4 * t2_6 + t4_5 * t2_5 + t4_6 * t2_4 + t4_7 * t2_3 + t4_8 * t2_2;
	double t6_11 = t4_4 * t2_7 + t4_5 * t2_6 + t4_6 * t2_5 + t4_7 * t2_4 + t4_8 * t2_3 + t4_9 * t2_2;
	double t6_12 = t4_4 * t2_8 + t4_5 * t2_7 + t4_6 * t2_6 + t4_7 * t2_5 + t4_8 * t2_4 + t4_9 * t2_3 + t4_10 * t2_2;
	double t7_8 = t6_6 * t1_2 + t6_7 * t1_1;
	double t7_10 = t6_6 * t1_4 + t6_7 * t1_3 + t6_8 * t1_2 + t6_9 * t1_1;
	double t7_12 = t6_8 * t1_4 + t6_9 * t1_3 + t6_10 * t1_2 + t6_11 * t1_1;
	double t8_8 = t6_6 * t2_2;
	double t8_9 = t6_6 * t2_3 + t6_7 * t2_2;
	double t8_10 = t6_6 * t2_4 + t6_7 * t2_3 + t6_8 * t2_2;
	double t8_11 = t6_6 * t2_5 + t6_7 * t2_4 + t6_8 * t2_3 + t6_9 * t2_2;
	double t8_12 = t6_6 * t2_6 + t6_7 * t2_5 + t6_8 * t2_4 + t6_9 * t2_3 + t6_10 * t2_2;
	double t9_10 = t8_8 * t1_2 + t8_9 * t1_1;
	double t9_12 = t8_8 * t1_4 + t8_9 * t1_3 + t8_10 * t1_2 + t8_11 * t1_1;
	double t10_10 = t8_8 * t2_2;
	double t10_11 = t8_8 * t2_3 + t8_9 * t2_2;
	double t10_12 = t8_8 * t2_4 + t8_9 * t2_3 + t8_10 * t2_2;
	double t11_12 = t10_10 * t1_2 + t10_11 * t1_1;
	double t12_12 = t10_10 * t2_2;
	u = 1;
	v = 0;
	v += (1./12) * t1_2 + (1./80) * t1_4;
	u -= (1./24) * t2_2 + (1./160) * t2_4 + (1./896) * t2_6 + (1./4608) * t2_8;
	v -= (1./480) * t3_4 + (1./2688) * t3_6 + (1./13824) * t3_8 + (1./67584) * t3_10 + (1./319488) * t3_12;
	u += (1./1920) * t4_4 + (1./10752) * t4_6 + (1./55296) * t4_8 + (1./270336) * t4_10 + (1./1.27795e+06) * t4_12;
	v += (1./53760) * t5_6 + (1./276480) * t5_8 + (1./1.35168e+06) * t5_10 + (1./6.38976e+06) * t5_12;
	u -= (1./322560) * t6_6 + (1./1.65888e+06) * t6_8 + (1./8.11008e+06) * t6_10 + (1./3.83386e+07) * t6_12;
	v -= (1./1.16122e+07) * t7_8 + (1./5.67706e+07) * t7_10 + (1./2.6837e+08) * t7_12;
	u += (1./9.28973e+07) * t8_8 + (1./4.54164e+08) * t8_10 + (1./2.14696e+09) * t8_12;
	v += (1./4.08748e+09) * t9_10 + (1./1.93226e+10) * t9_12;
	u -= (1./4.08748e+10) * t10_10 + (1./1.93226e+11) * t10_12;
	v -= (1./2.12549e+12) * t11_12;
	u += (1./2.55059e+13) * t12_12;
#endif

#if ORDER == 16
	double t1_1 = km0;
	double t1_2 = .5 * km1;
	double t1_3 = (1./6) * km2;
	double t1_4 = (1./24) * km3;
	double t2_2 = t1_1 * t1_1;
	double t2_3 = 2 * (t1_1 * t1_2);
	double t2_4 = 2 * (t1_1 * t1_3) + t1_2 * t1_2;
	double t2_5 = 2 * (t1_1 * t1_4 + t1_2 * t1_3);
	double t2_6 = 2 * (t1_2 * t1_4) + t1_3 * t1_3;
	double t2_7 = 2 * (t1_3 * t1_4);
	double t2_8 = t1_4 * t1_4;
	double t3_4 = t2_2 * t1_2 + t2_3 * t1_1;
	double t3_6 = t2_2 * t1_4 + t2_3 * t1_3 + t2_4 * t1_2 + t2_5 * t1_1;
	double t3_8 = t2_4 * t1_4 + t2_5 * t1_3 + t2_6 * t1_2 + t2_7 * t1_1;
	double t3_10 = t2_6 * t1_4 + t2_7 * t1_3 + t2_8 * t1_2;
	double t3_12 = t2_8 * t1_4;
	double t4_4 = t2_2 * t2_2;
	double t4_5 = 2 * (t2_2 * t2_3);
	double t4_6 = 2 * (t2_2 * t2_4) + t2_3 * t2_3;
	double t4_7 = 2 * (t2_2 * t2_5 + t2_3 * t2_4);
	double t4_8 = 2 * (t2_2 * t2_6 + t2_3 * t2_5) + t2_4 * t2_4;
	double t4_9 = 2 * (t2_2 * t2_7 + t2_3 * t2_6 + t2_4 * t2_5);
	double t4_10 = 2 * (t2_2 * t2_8 + t2_3 * t2_7 + t2_4 * t2_6) + t2_5 * t2_5;
	double t4_11 = 2 * (t2_3 * t2_8 + t2_4 * t2_7 + t2_5 * t2_6);
	double t4_12 = 2 * (t2_4 * t2_8 + t2_5 * t2_7) + t2_6 * t2_6;
	double t4_13 = 2 * (t2_5 * t2_8 + t2_6 * t2_7);
	double t4_14 = 2 * (t2_6 * t2_8) + t2_7 * t2_7;
	double t5_6 = t4_4 * t1_2 + t4_5 * t1_1;
	double t5_8 = t4_4 * t1_4 + t4_5 * t1_3 + t4_6 * t1_2 + t4_7 * t1_1;
	double t5_10 = t4_6 * t1_4 + t4_7 * t1_3 + t4_8 * t1_2 + t4_9 * t1_1;
	double t5_12 = t4_8 * t1_4 + t4_9 * t1_3 + t4_10 * t1_2 + t4_11 * t1_1;
	double t5_14 = t4_10 * t1_4 + t4_11 * t1_3 + t4_12 * t1_2 + t4_13 * t1_1;
	double t6_6 = t4_4 * t2_2;
	double t6_7 = t4_4 * t2_3 + t4_5 * t2_2;
	double t6_8 = t4_4 * t2_4 + t4_5 * t2_3 + t4_6 * t2_2;
	double t6_9 = t4_4 * t2_5 + t4_5 * t2_4 + t4_6 * t2_3 + t4_7 * t2_2;
	double t6_10 = t4_4 * t2_6 + t4_5 * t2_5 + t4_6 * t2_4 + t4_7 * t2_3 + t4_8 * t2_2;
	double t6_11 = t4_4 * t2_7 + t4_5 * t2_6 + t4_6 * t2_5 + t4_7 * t2_4 + t4_8 * t2_3 + t4_9 * t2_2;
	double t6_12 = t4_4 * t2_8 + t4_5 * t2_7 + t4_6 * t2_6 + t4_7 * t2_5 + t4_8 * t2_4 + t4_9 * t2_3 + t4_10 * t2_2;
	double t6_13 = t4_5 * t2_8 + t4_6 * t2_7 + t4_7 * t2_6 + t4_8 * t2_5 + t4_9 * t2_4 + t4_10 * t2_3 + t4_11 * t2_2;
	double t6_14 = t4_6 * t2_8 + t4_7 * t2_7 + t4_8 * t2_6 + t4_9 * t2_5 + t4_10 * t2_4 + t4_11 * t2_3 + t4_12 * t2_2;
	double t7_8 = t6_6 * t1_2 + t6_7 * t1_1;
	double t7_10 = t6_6 * t1_4 + t6_7 * t1_3 + t6_8 * t1_2 + t6_9 * t1_1;
	double t7_12 = t6_8 * t1_4 + t6_9 * t1_3 + t6_10 * t1_2 + t6_11 * t1_1;
	double t7_14 = t6_10 * t1_4 + t6_11 * t1_3 + t6_12 * t1_2 + t6_13 * t1_1;
	double t8_8 = t6_6 * t2_2;
	double t8_9 = t6_6 * t2_3 + t6_7 * t2_2;
	double t8_10 = t6_6 * t2_4 + t6_7 * t2_3 + t6_8 * t2_2;
	double t8_11 = t6_6 * t2_5 + t6_7 * t2_4 + t6_8 * t2_3 + t6_9 * t2_2;
	double t8_12 = t6_6 * t2_6 + t6_7 * t2_5 + t6_8 * t2_4 + t6_9 * t2_3 + t6_10 * t2_2;
	double t8_13 = t6_6 * t2_7 + t6_7 * t2_6 + t6_8 * t2_5 + t6_9 * t2_4 + t6_10 * t2_3 + t6_11 * t2_2;
	double t8_14 = t6_6 * t2_8 + t6_7 * t2_7 + t6_8 * t2_6 + t6_9 * t2_5 + t6_10 * t2_4 + t6_11 * t2_3 + t6_12 * t2_2;
	double t9_10 = t8_8 * t1_2 + t8_9 * t1_1;
	double t9_12 = t8_8 * t1_4 + t8_9 * t1_3 + t8_10 * t1_2 + t8_11 * t1_1;
	double t9_14 = t8_10 * t1_4 + t8_11 * t1_3 + t8_12 * t1_2 + t8_13 * t1_1;
	double t10_10 = t8_8 * t2_2;
	double t10_11 = t8_8 * t2_3 + t8_9 * t2_2;
	double t10_12 = t8_8 * t2_4 + t8_9 * t2_3 + t8_10 * t2_2;
	double t10_13 = t8_8 * t2_5 + t8_9 * t2_4 + t8_10 * t2_3 + t8_11 * t2_2;
	double t10_14 = t8_8 * t2_6 + t8_9 * t2_5 + t8_10 * t2_4 + t8_11 * t2_3 + t8_12 * t2_2;
	double t11_12 = t10_10 * t1_2 + t10_11 * t1_1;
	double t11_14 = t10_10 * t1_4 + t10_11 * t1_3 + t10_12 * t1_2 + t10_13 * t1_1;
	double t12_12 = t10_10 * t2_2;
	double t12_13 = t10_10 * t2_3 + t10_11 * t2_2;
	double t12_14 = t10_10 * t2_4 + t10_11 * t2_3 + t10_12 * t2_2;
	double t13_14 = t12_12 * t1_2 + t12_13 * t1_1;
	double t14_14 = t12_12 * t2_2;
	u = 1;
	u -= 1./24 * t2_2 + 1./160 * t2_4 + 1./896 * t2_6 + 1./4608 * t2_8;
	u += 1./1920 * t4_4 + 1./10752 * t4_6 + 1./55296 * t4_8 + 1./270336 * t4_10 + 1./1277952 * t4_12 + 1./5898240 * t4_14;
	u -= 1./322560 * t6_6 + 1./1658880 * t6_8 + 1./8110080 * t6_10 + 1./38338560 * t6_12 + 1./176947200 * t6_14;
	u += 1./92897280 * t8_8 + 1./454164480 * t8_10 + 4.6577500191e-10 * t8_12 + 1.0091791708e-10 * t8_14;
	u -= 2.4464949595e-11 * t10_10 + 5.1752777990e-12 * t10_12 + 1.1213101898e-12 * t10_14;
	u += 3.9206649992e-14 * t12_12 + 8.4947741650e-15 * t12_14;
	u -= 4.6674583324e-17 * t14_14;
	v = 0;
	v += 1./12 * t1_2 + 1./80 * t1_4;
	v -= 1./480 * t3_4 + 1./2688 * t3_6 + 1./13824 * t3_8 + 1./67584 * t3_10 + 1./319488 * t3_12;
	v += 1./53760 * t5_6 + 1./276480 * t5_8 + 1./1351680 * t5_10 + 1./6389760 * t5_12 + 1./29491200 * t5_14;
	v -= 1./11612160 * t7_8 + 1./56770560 * t7_10 + 1./268369920 * t7_12 + 8.0734333664e-10 * t7_14;
	v += 2.4464949595e-10 * t9_10 + 5.1752777990e-11 * t9_12 + 1.1213101898e-11 * t9_14;
	v -= 4.7047979991e-13 * t11_12 + 1.0193728998e-13 * t11_14;
	v += 6.5344416654e-16 * t13_14;
#endif

	}

	if (n == 1) {
#if ORDER == 2
	    x = 1;
	    y = 0;
#else
	    x = u;
	    y = v;
#endif
	} else {
	    double th = (((th4 * s + th3) * s + th2) * s + th1) * s;
	    double cth = cos(th);
	    double sth = sin(th);

#if ORDER == 2
	    x += cth;
	    y += sth;
#else
	    x += cth * u - sth * v;
	    y += cth * v + sth * u;
#endif
	    s += ds;
	}
    }

#if ORDER == 4 || ORDER == 6
    xy[0] = x * (1./24 * ds);
    xy[1] = y * (1./24 * ds);
#else
    xy[0] = x * ds;
    xy[1] = y * ds;
#endif
}

static double
compute_ends(const double ks[4], double ends[2][4], double seg_ch)
{
    double xy[2];
    double ch, th;
    double l, l2, l3;
    double th_even, th_odd;
    double k0_even, k0_odd;
    double k1_even, k1_odd;
    double k2_even, k2_odd;

    integrate_spiro(ks, xy);
    ch = hypot(xy[0], xy[1]);
    th = atan2(xy[1], xy[0]);
    l = ch / seg_ch;

    th_even = .5 * ks[0] + (1./48) * ks[2];
    th_odd = .125 * ks[1] + (1./384) * ks[3] - th;
    ends[0][0] = th_even - th_odd;
    ends[1][0] = th_even + th_odd;
    k0_even = l * (ks[0] + .125 * ks[2]);
    k0_odd = l * (.5 * ks[1] + (1./48) * ks[3]);
    ends[0][1] = k0_even - k0_odd;
    ends[1][1] = k0_even + k0_odd;
    l2 = l * l;
    k1_even = l2 * (ks[1] + .125 * ks[3]);
    k1_odd = l2 * .5 * ks[2];
    ends[0][2] = k1_even - k1_odd;
    ends[1][2] = k1_even + k1_odd;
    l3 = l2 * l;
    k2_even = l3 * ks[2];
    k2_odd = l3 * .5 * ks[3];
    ends[0][3] = k2_even - k2_odd;
    ends[1][3] = k2_even + k2_odd;

    return l;
}

static void
compute_pderivs(const spiro_seg *s, double ends[2][4], double derivs[4][2][4],
		int jinc)
{
    double recip_d = 2e6;
    double delta = 1./ recip_d;
    double try_ks[4];
    double try_ends[2][4];
    int i, j, k;

    compute_ends(s->ks, ends, s->seg_ch);
    for (i = 0; i < jinc; i++) {
	for (j = 0; j < 4; j++)
	    try_ks[j] = s->ks[j];
	try_ks[i] += delta;
	compute_ends(try_ks, try_ends, s->seg_ch);
	for (k = 0; k < 2; k++)
	    for (j = 0; j < 4; j++)
		derivs[j][k][i] = recip_d * (try_ends[k][j] - ends[k][j]);
    }
}

static double
mod_2pi(double th)
{
    double u = th / (2 * M_PI);
    return 2 * M_PI * (u - floor(u + 0.5));
}

static spiro_seg *
setup_path(const spiro_cp *src, int n)
{
    int n_seg = src[0].ty == '{' ? n - 1 : n;
    spiro_seg *r = (spiro_seg *)malloc((n_seg + 1) * sizeof(spiro_seg));
    int i;
    int ilast;

    for (i = 0; i < n_seg; i++) {
	r[i].x = src[i].x;
	r[i].y = src[i].y;
	r[i].ty = src[i].ty;
	r[i].ks[0] = 0.;
	r[i].ks[1] = 0.;
	r[i].ks[2] = 0.;
	r[i].ks[3] = 0.;
    }
    r[n_seg].x = src[n_seg % n].x;
    r[n_seg].y = src[n_seg % n].y;
    r[n_seg].ty = src[n_seg % n].ty;

    for (i = 0; i < n_seg; i++) {
	double dx = r[i + 1].x - r[i].x;
	double dy = r[i + 1].y - r[i].y;
	r[i].seg_ch = hypot(dx, dy);
	r[i].seg_th = atan2(dy, dx);
    }

    ilast = n_seg - 1;
    for (i = 0; i < n_seg; i++) {
	if (r[i].ty == '{' || r[i].ty == '}' || r[i].ty == 'v')
	    r[i].bend_th = 0.;
	else
	    r[i].bend_th = mod_2pi(r[i].seg_th - r[ilast].seg_th);
	ilast = i;
    }
    return r;
}

static void
bandec11(bandmat *m, int *perm, int n)
{
    int i, j, k;
    int l;

    /* pack top triangle to the left. */
    for (i = 0; i < 5; i++) {
	for (j = 0; j < i + 6; j++)
	    m[i].a[j] = m[i].a[j + 5 - i];
	for (; j < 11; j++)
	    m[i].a[j] = 0.;
    }
    l = 5;
    for (k = 0; k < n; k++) {
	int pivot = k;
	double pivot_val = m[k].a[0];
	double pivot_scale;

	l = l < n ? l + 1 : n;

	for (j = k + 1; j < l; j++)
	    if (fabs(m[j].a[0]) > fabs(pivot_val)) {
		pivot_val = m[j].a[0];
		pivot = j;
	    }

	perm[k] = pivot;
	if (pivot != k) {
	    for (j = 0; j < 11; j++) {
		double tmp = m[k].a[j];
		m[k].a[j] = m[pivot].a[j];
		m[pivot].a[j] = tmp;
	    }
	}

	if (fabs(pivot_val) < 1e-12) pivot_val = 1e-12;
	pivot_scale = 1. / pivot_val;
	for (i = k + 1; i < l; i++) {
	    double x = m[i].a[0] * pivot_scale;
	    m[k].al[i - k - 1] = x;
	    for (j = 1; j < 11; j++)
		m[i].a[j - 1] = m[i].a[j] - x * m[k].a[j];
	    m[i].a[10] = 0.;
	}
    }
}

static void
banbks11(const bandmat *m, const int *perm, double *v, int n)
{
    int i, k, l;

    /* forward substitution */
    l = 5;
    for (k = 0; k < n; k++) {
	i = perm[k];
	if (i != k) {
	    double tmp = v[k];
	    v[k] = v[i];
	    v[i] = tmp;
	}
	if (l < n) l++;
	for (i = k + 1; i < l; i++)
	    v[i] -= m[k].al[i - k - 1] * v[k];
    }

    /* back substitution */
    l = 1;
    for (i = n - 1; i >= 0; i--) {
	double x = v[i];
	for (k = 1; k < l; k++)
	    x -= m[i].a[k] * v[k + i];
	v[i] = x / m[i].a[0];
	if (l < 11) l++;
    }
}

static int compute_jinc(char ty0, char ty1)
{
    if (ty0 == 'o' || ty1 == 'o' ||
	ty0 == ']' || ty1 == '[')
	return 4;
    else if (ty0 == 'c' && ty1 == 'c')
	return 2;
    else if (((ty0 == '{' || ty0 == 'v' || ty0 == '[') && ty1 == 'c') ||
	     (ty0 == 'c' && (ty1 == '}' || ty1 == 'v' || ty1 == ']')))
	return 1;
    else
	return 0;
}

static int count_vec(const spiro_seg *s, int nseg)
{
    int i;
    int n = 0;

    for (i = 0; i < nseg; i++)
	n += compute_jinc(s[i].ty, s[i + 1].ty);
    return n;
}

static void
add_mat_line(bandmat *m, double *v,
	     double derivs[4], double x, double y, int j, int jj, int jinc,
	     int nmat)
{
    if (jj >= 0) {
	int joff =  (j + 5 - jj + nmat) % nmat;
	if (nmat < 6) {
	    joff = j + 5 - jj;
	} else if (nmat == 6) {
	    joff = 2 + (j + 3 - jj + nmat) % nmat;
	}
#ifdef VERBOSE
	printf("add_mat_line j=%d jj=%d jinc=%d nmat=%d joff=%d\n", j, jj, jinc, nmat, joff);
#endif
	v[jj] += x;
	for (int k = 0; k < jinc; k++)
	    m[jj].a[joff + k] += y * derivs[k];
    }
}

static double
spiro_iter(spiro_seg *s, bandmat *m, int *perm, double *v, const int n)
{
    int cyclic = s[0].ty != '{' && s[0].ty != 'v';
    int nmat = count_vec(s, n);
    int n_invert;

    for (int i = 0; i < nmat; i++) {
        v[i] = 0.;
        for (int j = 0; j < 11; j++) {
            m[i].a[j] = 0.;
        }
        for (int j = 0; j < 5; j++) {
            m[i].al[j] = 0.;
        }
    }

    int j = 0;
    int jj;
    if (s[0].ty == 'o') {
        jj = nmat - 2;
    } else if (s[0].ty == 'c') {
        jj = nmat - 1;
    } else {
        jj = 0;
    }
    for (int i = 0; i < n; i++) {
        char ty0 = s[i].ty;
        char ty1 = s[i + 1].ty;
        int jinc = compute_jinc(ty0, ty1);
        double th = s[i].bend_th;
        double ends[2][4];
        double derivs[4][2][4];
        int jthl = -1, jk0l = -1, jk1l = -1, jk2l = -1;
        int jthr = -1, jk0r = -1, jk1r = -1, jk2r = -1;

        compute_pderivs(&s[i], ends, derivs, jinc);

        /* constraints crossing left */
        if (ty0 == 'o' || ty0 == 'c' || ty0 == '[' || ty0 == ']') {
            jthl = jj++;
            jj %= nmat;
            jk0l = jj++;
        }
        if (ty0 == 'o') {
            jj %= nmat;
            jk1l = jj++;
            jk2l = jj++;
        }

        /* constraints on left */
        if ((ty0 == '[' || ty0 == 'v' || ty0 == '{' || ty0 == 'c') &&
            jinc == 4) {
            if (ty0 != 'c')
            jk1l = jj++;
            jk2l = jj++;
        }

        /* constraints on right */
        if ((ty1 == ']' || ty1 == 'v' || ty1 == '}' || ty1 == 'c') &&
            jinc == 4) {
            if (ty1 != 'c')
            jk1r = jj++;
            jk2r = jj++;
        }

        /* constraints crossing right */
        if (ty1 == 'o' || ty1 == 'c' || ty1 == '[' || ty1 == ']') {
            jthr = jj;
            jk0r = (jj + 1) % nmat;
        }
        if (ty1 == 'o') {
            jk1r = (jj + 2) % nmat;
            jk2r = (jj + 3) % nmat;
        }

        add_mat_line(m, v, derivs[0][0], th - ends[0][0], 1, j, jthl, jinc, nmat);
        add_mat_line(m, v, derivs[1][0], ends[0][1], -1, j, jk0l, jinc, nmat);
        add_mat_line(m, v, derivs[2][0], ends[0][2], -1, j, jk1l, jinc, nmat);
        add_mat_line(m, v, derivs[3][0], ends[0][3], -1, j, jk2l, jinc, nmat);
        add_mat_line(m, v, derivs[0][1], -ends[1][0], 1, j, jthr, jinc, nmat);
        add_mat_line(m, v, derivs[1][1], -ends[1][1], 1, j, jk0r, jinc, nmat);
        add_mat_line(m, v, derivs[2][1], -ends[1][2], 1, j, jk1r, jinc, nmat);
        add_mat_line(m, v, derivs[3][1], -ends[1][3], 1, j, jk2r, jinc, nmat);
        if (jthl >= 0)
            v[jthl] = mod_2pi(v[jthl]);
        if (jthr >= 0)
            v[jthr] = mod_2pi(v[jthr]);
        j += jinc;
    }
    if (cyclic) {
        memcpy(m + nmat, m, sizeof(bandmat) * nmat);
        memcpy(m + 2 * nmat, m, sizeof(bandmat) * nmat);
        memcpy(v + nmat, v, sizeof(double) * nmat);
        memcpy(v + 2 * nmat, v, sizeof(double) * nmat);
        n_invert = 3 * nmat;
        j = nmat;
    } else {
        n_invert = nmat;
        j = 0;
    }
#ifdef VERBOSE
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < 11; k++) {
            printf(" %2.4f", m[i].a[k]);
        }
        printf(": %2.4f\n", v[i]);
    }
    printf("---\n");
#endif
    bandec11(m, perm, n_invert);
    banbks11(m, perm, v, n_invert);
    
    double norm = 0.;
    for (int i = 0; i < n; i++) {
        char ty0 = s[i].ty;
        char ty1 = s[i + 1].ty;
        int jinc = compute_jinc(ty0, ty1);
        int k;

        for (k = 0; k < jinc; k++) {
            double dk = v[j++];

#ifdef VERBOSE
            printf("s[%d].ks[%d] += %f\n", i, k, dk);
#endif
            s[i].ks[k] += dk;
            norm += dk * dk;
        }
        s[i].ks[0] = 2.0*mod_2pi(s[i].ks[0]/2.0);
    }
    return norm;
}

static int
solve_spiro(spiro_seg *s, const int nseg)
{
    int nmat = count_vec(s, nseg);
    int n_alloc = nmat;

    if (nmat == 0) {
        return 0;
    }
    if (s[0].ty != '{' && s[0].ty != 'v') {
        n_alloc *= 3;
    }
    if (n_alloc < 5) {
        n_alloc = 5;
    }

    bandmat *m = (bandmat *)malloc(sizeof(bandmat) * n_alloc);
    double *v = (double *)malloc(sizeof(double) * n_alloc);
    int *perm = (int *)malloc(sizeof(int) * n_alloc);

    for (unsigned i = 0; i < 10; i++) {
        double norm = spiro_iter(s, m, perm, v, nseg);
#ifdef VERBOSE
        printf("%% norm = %g\n", norm);
#endif
        if (norm < 1e-12) break;
    }

    free(m);
    free(v);
    free(perm);
    return 0;
}

static void
spiro_seg_to_otherpath(const double ks[4],
		   double x0, double y0, double x1, double y1,
		   ConverterBase &bc, int depth, bool close_last)
{
    double bend = fabs(ks[0]) + fabs(.5 * ks[1]) + fabs(.125 * ks[2]) +
	fabs((1./48) * ks[3]);

    if (!(bend > 1e-8)) {
        bc.lineto(x1, y1, close_last);
    } else {
	double seg_ch = hypot(x1 - x0, y1 - y0);
	double seg_th = atan2(y1 - y0, x1 - x0);
	double xy[2];
	double ch, th;
	double scale, rot;

	integrate_spiro(ks, xy);
	ch = hypot(xy[0], xy[1]);
	th = atan2(xy[1], xy[0]);
	scale = seg_ch / ch;
	rot = seg_th - th;
	if (depth > 5 || bend < 1.) {
        double ul, vl;
        double ur, vr;
        double th_even, th_odd;
	    th_even = (1./384) * ks[3] + (1./8) * ks[1] + rot;
	    th_odd = (1./48) * ks[2] + .5 * ks[0];
	    ul = (scale * (1./3)) * cos(th_even - th_odd);
	    vl = (scale * (1./3)) * sin(th_even - th_odd);
	    ur = (scale * (1./3)) * cos(th_even + th_odd);
	    vr = (scale * (1./3)) * sin(th_even + th_odd);
        bc.curveto(x0 + ul, y0 + vl, x1 - ur, y1 - vr, x1, y1, close_last);
	} else {
	    /* subdivide */
	    double ksub[4];
	    double thsub;
	    double xysub[2];
	    double xmid, ymid;
	    double cth, sth;

	    ksub[0] = .5 * ks[0] - .125 * ks[1] + (1./64) * ks[2] - (1./768) * ks[3];
	    ksub[1] = .25 * ks[1] - (1./16) * ks[2] + (1./128) * ks[3];
	    ksub[2] = .125 * ks[2] - (1./32) * ks[3];
	    ksub[3] = (1./16) * ks[3];
	    thsub = rot - .25 * ks[0] + (1./32) * ks[1] - (1./384) * ks[2] + (1./6144) * ks[3];
	    cth = .5 * scale * cos(thsub);
	    sth = .5 * scale * sin(thsub);
	    integrate_spiro(ksub, xysub);
	    xmid = x0 + cth * xysub[0] - sth * xysub[1];
	    ymid = y0 + cth * xysub[1] + sth * xysub[0];
	    spiro_seg_to_otherpath(ksub, x0, y0, xmid, ymid, bc, depth + 1, false);
	    ksub[0] += .25 * ks[1] + (1./384) * ks[3];
	    ksub[1] += .125 * ks[2];
	    ksub[2] += (1./16) * ks[3];
	    spiro_seg_to_otherpath(ksub, xmid, ymid, x1, y1, bc, depth + 1, close_last);
	}
    }
}

spiro_seg *
run_spiro(const spiro_cp *src, int n)
{
    int nseg = src[0].ty == '{' ? n - 1 : n;
    spiro_seg *s = setup_path(src, n);
    if (nseg > 1)
	solve_spiro(s, nseg);
    return s;
}

void
free_spiro(spiro_seg *s)
{
    free(s);
}

void
spiro_to_otherpath(const spiro_seg *s, int n, ConverterBase &bc)
{
    int i;
    int nsegs = s[n - 1].ty == '}' ? n - 1 : n;

    for (i = 0; i < nsegs; i++) {
	double x0 = s[i].x;
	double y0 = s[i].y;
	double x1 = s[i + 1].x;
	double y1 = s[i + 1].y;

        if (i == 0) {
            bc.moveto(x0, y0);
        }
        // on the last segment, set the 'close_last' flag if path is closed
        spiro_seg_to_otherpath(s[i].ks, x0, y0, x1, y1, bc, 0, (nsegs == n) && (i == n - 1));
    }
}

double
get_knot_th(const spiro_seg *s, int i)
{
    double ends[2][4];

    if (i == 0) {
	compute_ends(s[i].ks, ends, s[i].seg_ch);
	return s[i].seg_th - ends[0][0];
    } else {
	compute_ends(s[i - 1].ks, ends, s[i - 1].seg_ch);
	return s[i - 1].seg_th + ends[1][0];
    }
}


} // namespace Spiro

/************************************
 * Unit_test code
 */


#ifdef UNIT_TEST
#include <stdio.h>
#include <sys/time.h> /* for gettimeofday */

using namespace Spiro;

static double
get_time (void)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday (&tv, &tz);

  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

int
test_integ(void) {
    double ks[] = {1, 2, 3, 4};
    double xy[2];
    double xynom[2];
    int i, j;
    int nsubdiv;

    n = ORDER < 6 ? 4096 : 1024;
    integrate_spiro(ks, xynom);
    nsubdiv = ORDER < 12 ? 8 : 7;
    for (i = 0; i < nsubdiv; i++) {
	double st, en;
	double err;
	int n_iter = (1 << (20 - i));

	n = 1 << i;
	st = get_time();
	for (j = 0; j < n_iter; j++)
	    integrate_spiro(ks, xy);
	en = get_time();
	err = hypot(xy[0] - xynom[0], xy[1] - xynom[1]);
	printf("%d %d %g %g\n", ORDER, n, (en - st) / n_iter, err);
#if 0
    double ch, th;
	ch = hypot(xy[0], xy[1]);
	th = atan2(xy[1], xy[0]);
	printf("n = %d: integ(%g %g %g %g) = %g %g, ch = %g, th = %g\n", n,
	       ks[0], ks[1], ks[2], ks[3], xy[0], xy[1], ch, th);
	printf("%d: %g %g\n", n, xy[0] - xynom[0], xy[1] - xynom[1]);
#endif
    }
    return 0;
}

void
print_seg(const double ks[4], double x0, double y0, double x1, double y1)
{
    double bend = fabs(ks[0]) + fabs(.5 * ks[1]) + fabs(.125 * ks[2]) +
	fabs((1./48) * ks[3]);

    if (bend < 1e-8) {
	printf("%g %g lineto\n", x1, y1);
    } else {
	double seg_ch = hypot(x1 - x0, y1 - y0);
	double seg_th = atan2(y1 - y0, x1 - x0);
	double xy[2];
	double ch, th;
	double scale, rot;

	integrate_spiro(ks, xy);
	ch = hypot(xy[0], xy[1]);
	th = atan2(xy[1], xy[0]);
	scale = seg_ch / ch;
	rot = seg_th - th;
	if (bend < 1.) {
        double th_even, th_odd;
        double ul, vl;
        double ur, vr;
	    th_even = (1./384) * ks[3] + (1./8) * ks[1] + rot;
	    th_odd = (1./48) * ks[2] + .5 * ks[0];
	    ul = (scale * (1./3)) * cos(th_even - th_odd);
	    vl = (scale * (1./3)) * sin(th_even - th_odd);
	    ur = (scale * (1./3)) * cos(th_even + th_odd);
	    vr = (scale * (1./3)) * sin(th_even + th_odd);
	    printf("%g %g %g %g %g %g curveto\n",
		   x0 + ul, y0 + vl, x1 - ur, y1 - vr, x1, y1);
	    
	} else {
	    /* subdivide */
	    double ksub[4];
	    double thsub;
	    double xysub[2];
	    double xmid, ymid;
	    double cth, sth;

	    ksub[0] = .5 * ks[0] - .125 * ks[1] + (1./64) * ks[2] - (1./768) * ks[3];
	    ksub[1] = .25 * ks[1] - (1./16) * ks[2] + (1./128) * ks[3];
	    ksub[2] = .125 * ks[2] - (1./32) * ks[3];
	    ksub[3] = (1./16) * ks[3];
	    thsub = rot - .25 * ks[0] + (1./32) * ks[1] - (1./384) * ks[2] + (1./6144) * ks[3];
	    cth = .5 * scale * cos(thsub);
	    sth = .5 * scale * sin(thsub);
	    integrate_spiro(ksub, xysub);
	    xmid = x0 + cth * xysub[0] - sth * xysub[1];
	    ymid = y0 + cth * xysub[1] + sth * xysub[0];
	    print_seg(ksub, x0, y0, xmid, ymid);
	    ksub[0] += .25 * ks[1] + (1./384) * ks[3];
	    ksub[1] += .125 * ks[2];
	    ksub[2] += (1./16) * ks[3];
	    print_seg(ksub, xmid, ymid, x1, y1);
	}
    }
}

void
print_segs(const spiro_seg *segs, int nsegs)
{
    int i;

    for (i = 0; i < nsegs; i++) {
	double x0 = segs[i].x;
	double y0 = segs[i].y;
	double x1 = segs[i + 1].x;
	double y1 = segs[i + 1].y;

	if (i == 0)
	    printf("%g %g moveto\n", x0, y0);
	printf("%% ks = [ %g %g %g %g ]\n",
	       segs[i].ks[0], segs[i].ks[1], segs[i].ks[2], segs[i].ks[3]);
	print_seg(segs[i].ks, x0, y0, x1, y1);
    }
    printf("stroke\n");
}

int
test_curve(void)
{
    spiro_cp path[] = {
	{334, 117, 'v'},
	{305, 176, 'v'},
	{212, 142, 'c'},
	{159, 171, 'c'},
	{224, 237, 'c'},
	{347, 335, 'c'},
	{202, 467, 'c'},
	{81, 429, 'v'},
	{114, 368, 'v'},
	{201, 402, 'c'},
	{276, 369, 'c'},
	{218, 308, 'c'},
	{91, 211, 'c'},
	{124, 111, 'c'},
	{229, 82, 'c'}
    };
    spiro_seg *segs;
    int i;

    n = 1;
    for (i = 0; i < 1000; i++) {
	segs = setup_path(path, 15);
	solve_spiro(segs, 15);
    }
    printf("100 800 translate 1 -1 scale 1 setlinewidth\n");
    print_segs(segs, 15);
    printf("showpage\n");
    return 0;
}

int main(int argc, char **argv)
{
    return test_curve();
}
#endif

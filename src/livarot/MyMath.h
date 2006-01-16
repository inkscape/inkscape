/*
 *  MyMath.h
 *  nlivarot
 *
 *  Created by fred on Wed Jun 18 2003.
 *
 */

#ifndef my_math
#define my_math

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
//#include <iostream.h>


typedef struct vec2
{
    double x, y;
} vec2;


typedef struct mat2
{
    double xx, xy, yx, yy;
} mat2;


typedef struct vec2d
{
    double x, y;
} vec2d;


typedef struct mat2d
{
    double xx, xy, yx, yy;
} mat2d;


#define RotCCW(a) {\
    double t = (a).x;\
    (a).x = (a).y;\
    (a).y = -t;\
}

#define RotCCWTo(a,d) {\
    (d).x =  (a).y;\
    (d).y = -(a).x;\
}

#define RotCW(a) {\
    double t = (a).x;\
    (a).x = -(a).y;\
    (a).y = t;\
}

#define RotCWTo(a,d) {\
    (d).x = -(a).y;\
    (d).y =  (a).x;\
}

#define Normalize(a) { \
    double _le = (a).x*(a).x+(a).y*(a).y; \
    if ( _le > 0.0001 ) { \
    	_le = 1.0 / sqrt(_le); \
    	(a).x *= _le; \
    	(a).y *= _le; \
    } \
}

#define L_VEC_Set(a,u,v) { \
    a.x = u; \
    a.y = v; \
}


#define L_VEC_Length(a,l) { \
    l = sqrt(a.x*a.x+a.y*a.y); \
}

#define L_VEC_Add(a,b,r) { \
    r.x = a.x+b.x; \
    r.y = a.y+b.y; \
}

#define L_VEC_Sub(a,b,r) { \
    r.x = a.x-b.x; \
    r.y = a.y-b.y; \
}

#define L_VEC_Mul(a,b,r) { \
    r.x = a.x*b.x; \
    r.y = a.y*b.y; \
}

#define L_VEC_Div(a,b,r) { \
    r.x = a.x/b.x; \
    r.y = a.y/b.y; \
}

#define L_VEC_AddMul(a,b,c,r) { \
    r.x = a.x+b.x*c.x; \
    r.y = a.y+b.y*c.y; \
}

#define L_VEC_SubMul(a,b,c,r) { \
    r.x = a.x-b.x*c.x; \
    r.y = a.y-b.y*c.y; \
}


#define L_VEC_MulC(a,b,r) { \
    r.x = a.x*(b); \
    r.y = a.y*(b); \
}

#define L_VEC_DivC(a,b,r) { \
    r.x = a.x/(b); \
    r.y = a.y/(b); \
}

#define L_VEC_AddMulC(a,b,c,r) { \
    r.x = a.x+b.x*c; \
    r.y = a.y+b.y*c; \
}

#define L_VEC_SubMulC(a,b,c,r) { \
    r.x = a.x-b.x*c; \
    r.y = a.y-b.y*c; \
}

#define L_VEC_Cmp(a,b) ((fabs(a.y-b.y)<0.0000001)? \
    ((fabs(a.x-b.x)<0.0000001)?0:((a.x > b.x)?1:-1)): \
    ((a.y > b.y)?1:-1))

#define L_VAL_Cmp(a,b) ((fabs(a-b)<0.0000001)?0:((a>b)?1:-1))

#define L_VEC_Normalize(d) { \
    double l=sqrt(d.x*d.x+d.y*d.y); \
    if ( l < 0.00000001 ) { \
        d.x=d.y=0; \
    } else { \
        d.x/=l; \
        d.y/=l; \
    } \
}

#define L_VEC_Distance(a,b,d) { \
    double dx = a.x-b.x; \
    double dy = a.y-b.y; \
    d = sqrt(dx*dx + dy*dy); \
}

#define L_VEC_Neg(d) { \
    d.x=d.x; d.y=-d.y; \
}

#define L_VEC_RotCW(d) { \
    double t=d.x; d.x=d.y; d.y=-t; \
} \

#define L_VEC_RotCCW(d) { \
    double t=d.x; d.x=-d.y; d.y=t; \
}

#define L_VAL_Zero(a) ((fabs(a)<0.00000001)?0:((a>0)?1:-1))

#define L_VEC_Cross(a,b,r) { \
    r = a.x*b.x+a.y*b.y; \
}

#define L_VEC_Dot(a,b,r) { \
    r = a.x*b.y-a.y*b.x; \
}


#define	L_MAT(m,a,b) { \
    c[0][0].Set(ica.x); c[0][1].Set(icb.x); c[1][0].Set(ica.y); c[1][1].Set(icb.y); \
}

#define	L_MAT_Set(m,a00,a10,a01,a11) {m.xx = a00; m.xy = a01; m.yx = a10; m.yy = a11;}

#define L_MAT_SetC(m,a,b) {m.xx = a.x; m.xy = b.x; m.yx = a.y; m.yy = b.y;}

#define L_MAT_SetL(m,a,b) {m.xx = a.x; m.xy = a.y;m.yx  = b.x; m.yy = b.y;}

#define L_MAT_Init(m) {m.xx=m.xy=m.yx=m.yy=0;}

#define L_MAT_Col(m,no,r) { \
    if ( no == 0 ) { \
        r.x = m.xx; \
        r.y = m.yx; \
    } \
    if ( no == 0 ) { \
        r.x = m.xy; \
        r.y = m.yy; \
    } \
}

#define L_MAT_Row(m,no,r) { \
    if ( no == 0 ) { \
        r.x = m.xx; \
        r.y = m.xy; \
    } \
    if ( no == 0 ) { \
        r.x = m.yx; \
        r.y = m.yy; \
    } \
}

#define L_MAT_Det(m,d) {d=m.xx*m.yy-m.xy*m.yx;}

#define L_MAT_Neg(m) {m.xx=-m.xx; m.xy=-m.xy; m.yx=-m.yx; m.yy=-m.yy;}

#define L_MAT_Trs(m) {double t=m.xy; m.xy=m.yx; m.yx=t;}

#define L_MAT_Inv(m) { \
    double d; \
    L_MAT_Det(m,d); \
    m.yx =- m.yx; \
    m.xy =- m.xy; \
    double t=m.xx;m.xx=m.yy;m.yy=t; \
    double inv_d = 1.0/d; \
    m.xx *= inv_d; \
    m.xy *= inv_d; \
    m.yx *= inv_d; \
    m.yy *= inv_d; \
}

#define L_MAT_Cof(m) { \
    m.yx =- m.yx; \
    m.xy =- m.xy; \
    double t=m.xx; m.xx=m.yy; m.yy=t; \
}

#define L_MAT_Add(u,v,m) { \
    m.xx=u.xx+v.xx; m.xy=u.xy+v.xy; m.yx=u.yx+v.yx; m.yy=u.yy+v.yy; \
}

#define L_MAT_Sub(u,v,m) { \
    m.xx=u.xx-v.xx; m.xy=u.xy-v.xy; m.yx=u.yx-v.yx; m.yy=u.yy-v.yy; \
}

#define L_MAT_Mul(u,v,m) { \
    mat2d r; \
    r.xx = u.xx*v.xx+u.xy*v.yx; \
    r.yx = u.yx*v.xx+u.yy*y.yx; \
    r.xy = u.xx*v.xy+u.xy*v.yy; \
    r.yy = u.yx*v.xy+u.yy*v.yy; \
    m=r; \
}

#define L_MAT_MulC(u,v,m) { \
    m.xx=u.xx*v; m.xy=u.xy*v; m.yx=u.yx*v; m.yy=u.yy*v; \
}

#define L_MAT_DivC(u,v,m) { \
    double iv = 1.0/v; \
    m.xx = u.xx*iv; m.xy=u.xy*iv; m.yx=u.yx*iv; m.yy=u.yy*iv; \
}

#define L_MAT_MulV(m,v,r) { \
    vec2d t; \
    t.x = m.xx*v.x+m.xy*v.y; \
    t.y = m.yx*v.x+m.yy*v.y; \
    r=t; \
}

#define L_MAT_TMulV(m,v,r) { \
    vec2d t; \
    t.x = m.xx*v.x+m.yx*v.y; \
    t.y = m.xy*v.x+m.yy*v.y; \
    r=t; \
}



#endif

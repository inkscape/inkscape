/*
 * Colors.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cmath>
#include <cstdio>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#include "color.h"
#include "svg/svg-icc-color.h"
#include "svg/svg-color.h"

#include "svg/css-ostringstream.h"

#define return_if_fail(x) if (!(x)) { printf("assertion failed: " #x); return; }
#define return_val_if_fail(x, val) if (!(x)) { printf("assertion failed: " #x); return val; }

using Inkscape::CSSOStringStream;
using std::vector;

static bool profileMatches( SVGICCColor const* first, SVGICCColor const* second );

#define PROFILE_EPSILON 0.00000001

SPColor::SPColor() :
    icc(0)
{
    v.c[0] = 0;
    v.c[1] = 0;
    v.c[2] = 0;
}

SPColor::SPColor( SPColor const& other ) :
    icc(0)
{
    *this = other;
}

SPColor::SPColor( float r, float g, float b ) :
    icc(0)
{
    set( r, g, b );
}

SPColor::SPColor( guint32 value ) :
    icc(0)
{
    set( value );
}

SPColor::~SPColor()
{
    delete icc;
    icc = 0;
}


SPColor& SPColor::operator= (SPColor const& other)
{
    if (this == &other){
        return *this;
    }
    
    SVGICCColor* tmp_icc = other.icc ? new SVGICCColor(*other.icc) : 0;

    v.c[0] = other.v.c[0];
    v.c[1] = other.v.c[1];
    v.c[2] = other.v.c[2];
    if ( icc ) {
        delete icc;
    }
    icc = tmp_icc;

    return *this;
}


/**
 * Returns true if colors match.
 */
bool SPColor::operator == (SPColor const& other) const
{
    bool match =
        (v.c[0] == other.v.c[0]) &&
        (v.c[1] == other.v.c[1]) &&
        (v.c[2] == other.v.c[2]);

    match &= profileMatches( icc, other.icc );

    return match;
}

/**
 * Returns true if no RGB value differs epsilon or more in both colors,
 * false otherwise.
 */
bool SPColor::isClose( SPColor const& other, float epsilon ) const
{
    bool match = (fabs((v.c[0]) - (other.v.c[0])) < epsilon)
        && (fabs((v.c[1]) - (other.v.c[1])) < epsilon)
        && (fabs((v.c[2]) - (other.v.c[2])) < epsilon);

    match &= profileMatches( icc, other.icc );

    return match;
}

static bool profileMatches( SVGICCColor const* first, SVGICCColor const* second ) {
    bool match = false;
    if ( !first && !second ) {
        match = true;
    } else {
        match = first && second
            && (first->colorProfile == second->colorProfile)
            && (first->colors.size() == second->colors.size());
        if ( match ) {
            for ( unsigned i = 0; i < first->colors.size(); i++ ) {
                match &= (fabs(first->colors[i] - second->colors[i]) < PROFILE_EPSILON);
            }
        }
    }
    return match;
}

/**
 * Sets RGB values and colorspace in color.
 * \pre 0 <={r,g,b}<=1
 */
void SPColor::set( float r, float g, float b )
{
    return_if_fail(r >= 0.0);
    return_if_fail(r <= 1.0);
    return_if_fail(g >= 0.0);
    return_if_fail(g <= 1.0);
    return_if_fail(b >= 0.0);
    return_if_fail(b <= 1.0);

    // TODO clear icc if set?
    v.c[0] = r;
    v.c[1] = g;
    v.c[2] = b;
}

/**
 * Converts 32bit value to RGB floats and sets color.
 */
void SPColor::set( guint32 value )
{
    // TODO clear icc if set?
    v.c[0] = (value >> 24) / 255.0F;
    v.c[1] = ((value >> 16) & 0xff) / 255.0F;
    v.c[2] = ((value >> 8) & 0xff) / 255.0F;
}

/**
 * Convert SPColor with integer alpha value to 32bit RGBA value.
 * \pre alpha < 256
 */
guint32 SPColor::toRGBA32( int alpha ) const
{
    return_val_if_fail (alpha <= 0xff, 0x0);

    guint32 rgba = SP_RGBA32_U_COMPOSE( SP_COLOR_F_TO_U(v.c[0]),
                                        SP_COLOR_F_TO_U(v.c[1]),
                                        SP_COLOR_F_TO_U(v.c[2]),
                                        alpha );
    return rgba;
}

/**
 * Convert SPColor with float alpha value to 32bit RGBA value.
 * \pre color != NULL && 0 <= alpha <= 1
 */
guint32 SPColor::toRGBA32( double alpha ) const
{
    return_val_if_fail(alpha >= 0.0, 0x0);
    return_val_if_fail(alpha <= 1.0, 0x0);

    return toRGBA32( static_cast<int>(SP_COLOR_F_TO_U(alpha)) );
}

std::string SPColor::toString() const
{
    CSSOStringStream css;
    char tmp[64] = {0};

    sp_svg_write_color(tmp, sizeof(tmp), toRGBA32(0x0ff));
    css << tmp;

    if ( icc ) {
        if ( !css.str().empty() ) {
            css << " ";
        }
        css << "icc-color(" << icc->colorProfile;
        for (vector<double>::const_iterator i(icc->colors.begin()),
                 iEnd(icc->colors.end());
             i != iEnd; ++i) {
            css << ", " << *i;
        }
        css << ')';
    }

    return css.str();
}


/**
 * Fill rgb float array with values from SPColor.
 * \pre color != NULL && rgb != NULL && rgb[0-2] is meaningful
 */
void
sp_color_get_rgb_floatv(SPColor const *color, float *rgb)
{
    return_if_fail (color != NULL);
    return_if_fail (rgb != NULL);

    rgb[0] = color->v.c[0];
    rgb[1] = color->v.c[1];
    rgb[2] = color->v.c[2];
}

/**
 * Fill cmyk float array with values from SPColor.
 * \pre color != NULL && cmyk != NULL && cmyk[0-3] is meaningful
 */
void
sp_color_get_cmyk_floatv(SPColor const *color, float *cmyk)
{
    return_if_fail (color != NULL);
    return_if_fail (cmyk != NULL);

    sp_color_rgb_to_cmyk_floatv( cmyk,
                                 color->v.c[0],
                                 color->v.c[1],
                                 color->v.c[2] );
}

/* Plain mode helpers */

/**
 * Fill hsv float array from r,g,b float values.
 */
void
sp_color_rgb_to_hsv_floatv (float *hsv, float r, float g, float b)
{
    float max, min, delta;

    max = MAX (MAX (r, g), b);
    min = MIN (MIN (r, g), b);
    delta = max - min;

    hsv[2] = max;

    if (max > 0) {
        hsv[1] = delta / max;
    } else {
        hsv[1] = 0.0;
    }

    if (hsv[1] != 0.0) {
        if (r == max) {
            hsv[0] = (g - b) / delta;
        } else if (g == max) {
            hsv[0] = 2.0 + (b - r) / delta;
        } else {
            hsv[0] = 4.0 + (r - g) / delta;
        }

        hsv[0] = hsv[0] / 6.0;

        if (hsv[0] < 0) hsv[0] += 1.0;
    }
    else
        hsv[0] = 0.0;
}

/**
 * Fill rgb float array from h,s,v float values.
 */
void
sp_color_hsv_to_rgb_floatv (float *rgb, float h, float s, float v)
{
    double f, w, q, t, d;

    d = h * 5.99999999;
    f = d - floor (d);
    w = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));

    if (d < 1.0) {
        *rgb++ = v;
        *rgb++ = t;
        *rgb++ = w;
    } else if (d < 2.0) {
        *rgb++ = q;
        *rgb++ = v;
        *rgb++ = w;
    } else if (d < 3.0) {
        *rgb++ = w;
        *rgb++ = v;
        *rgb++ = t;
    } else if (d < 4.0) {
        *rgb++ = w;
        *rgb++ = q;
        *rgb++ = v;
    } else if (d < 5.0) {
        *rgb++ = t;
        *rgb++ = w;
        *rgb++ = v;
    } else {
        *rgb++ = v;
        *rgb++ = w;
        *rgb++ = q;
    }
}

/**
 * Fill hsl float array from r,g,b float values.
 */
void
sp_color_rgb_to_hsl_floatv (float *hsl, float r, float g, float b)
{
    float max = MAX (MAX (r, g), b);
    float min = MIN (MIN (r, g), b);
    float delta = max - min;

    hsl[2] = (max + min)/2.0;

    if (delta == 0) {
        hsl[0] = 0;
        hsl[1] = 0;
    } else {
        if (hsl[2] <= 0.5)
            hsl[1] = delta / (max + min);
        else
            hsl[1] = delta / (2 - max - min);

        if (r == max) hsl[0] = (g - b) / delta;
        else if (g == max) hsl[0] = 2.0 + (b - r) / delta;
        else if (b == max) hsl[0] = 4.0 + (r - g) / delta;

        hsl[0] = hsl[0] / 6.0;

        if (hsl[0] < 0) hsl[0] += 1;
        if (hsl[0] > 1) hsl[0] -= 1;
    }
}

static float
hue_2_rgb (float v1, float v2, float h)
{
    if (h < 0) h += 6.0;
    if (h > 6) h -= 6.0;

    if (h < 1) return v1 + (v2 - v1) * h;
    if (h < 3) return v2;
    if (h < 4) return v1 + (v2 - v1) * (4 - h);
    return v1;
}

/**
 * Fill rgb float array from h,s,l float values.
 */
void
sp_color_hsl_to_rgb_floatv (float *rgb, float h, float s, float l)
{
    if (s == 0) {
        rgb[0] = l;
        rgb[1] = l;
        rgb[2] = l;
    } else {
        float v2;
        if (l < 0.5) {
            v2 = l * (1 + s);
        } else {
            v2 = l + s - l*s;
        }
        float v1 = 2*l - v2;

        rgb[0] = hue_2_rgb (v1, v2, h*6 + 2.0);
        rgb[1] = hue_2_rgb (v1, v2, h*6);
        rgb[2] = hue_2_rgb (v1, v2, h*6 - 2.0);
    }
}

/**
 * Fill cmyk float array from r,g,b float values.
 */
void
sp_color_rgb_to_cmyk_floatv (float *cmyk, float r, float g, float b)
{
    float c, m, y, k, kd;

    c = 1.0 - r;
    m = 1.0 - g;
    y = 1.0 - b;
    k = MIN (MIN (c, m), y);

    c = c - k;
    m = m - k;
    y = y - k;

    kd = 1.0 - k;

    if (kd > 1e-9) {
        c = c / kd;
        m = m / kd;
        y = y / kd;
    }

    cmyk[0] = c;
    cmyk[1] = m;
    cmyk[2] = y;
    cmyk[3] = k;
}

/**
 * Fill rgb float array from c,m,y,k float values.
 */
void
sp_color_cmyk_to_rgb_floatv (float *rgb, float c, float m, float y, float k)
{
    float kd;

    kd = 1.0 - k;

    c = c * kd;
    m = m * kd;
    y = y * kd;

    c = c + k;
    m = m + k;
    y = y + k;

    rgb[0] = 1.0 - c;
    rgb[1] = 1.0 - m;
    rgb[2] = 1.0 - y;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

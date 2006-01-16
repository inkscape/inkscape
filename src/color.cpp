#define __SP_COLOR_C__

/** \file
 * Colors and colorspaces.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include "color.h"

/// A color space is just a name.
struct SPColorSpace {
    gchar const *name;
};

static SPColorSpace const RGB = {"RGB"};
static SPColorSpace const CMYK = {"CMYK"};

/**
 * Returns one of three values depending on if color is valid and if it 
 * has a valid color space. Likely redundant as soon as this is C++.
 */
SPColorSpaceClass
sp_color_get_colorspace_class(SPColor const *color)
{
    g_return_val_if_fail (color != NULL, SP_COLORSPACE_CLASS_INVALID);

    if (color->colorspace == &RGB) return SP_COLORSPACE_CLASS_PROCESS;
    if (color->colorspace == &CMYK) return SP_COLORSPACE_CLASS_PROCESS;

    return SP_COLORSPACE_CLASS_UNKNOWN;
}

/**
 * Returns the SPColorSpaceType corresponding to color's color space.
 * \todo color->colorspace should simply be a named enum.
 */
SPColorSpaceType
sp_color_get_colorspace_type(SPColor const *color)
{
    g_return_val_if_fail (color != NULL, SP_COLORSPACE_TYPE_INVALID);

    if (color->colorspace == &RGB) return SP_COLORSPACE_TYPE_RGB;
    if (color->colorspace == &CMYK) return SP_COLORSPACE_TYPE_CMYK;

    return SP_COLORSPACE_TYPE_UNKNOWN;
}

/**
 * Shallow copy of color struct with NULL check.
 */
void
sp_color_copy(SPColor *dst, SPColor const *src)
{
    g_return_if_fail (dst != NULL);
    g_return_if_fail (src != NULL);

    *dst = *src;
}

/**
 * Returns TRUE if c0 or c1 is NULL, or if colors/opacities differ.
 */
gboolean
sp_color_is_equal (SPColor const *c0, SPColor const *c1)
{
    g_return_val_if_fail (c0 != NULL, TRUE);
    g_return_val_if_fail (c1 != NULL, TRUE);

    if (c0->colorspace != c1->colorspace) return FALSE;
    if (c0->v.c[0] != c1->v.c[0]) return FALSE;
    if (c0->v.c[1] != c1->v.c[1]) return FALSE;
    if (c0->v.c[2] != c1->v.c[2]) return FALSE;
    if ((c0->colorspace == &CMYK) && (c0->v.c[3] != c1->v.c[3])) return FALSE;

    return TRUE;
}

/**
 * Returns TRUE if no RGB value differs epsilon or more in both colors, 
 * with CMYK aditionally comparing opacity, or if c0 or c1 is NULL. 
 * \note Do we want the latter?
 */
gboolean
sp_color_is_close (SPColor const *c0, SPColor const *c1, float epsilon)
{
    g_return_val_if_fail (c0 != NULL, TRUE);
    g_return_val_if_fail (c1 != NULL, TRUE);

    if (c0->colorspace != c1->colorspace) return FALSE;
    if (fabs ((c0->v.c[0]) - (c1->v.c[0])) >= epsilon) return FALSE;
    if (fabs ((c0->v.c[1]) - (c1->v.c[1])) >= epsilon) return FALSE;
    if (fabs ((c0->v.c[2]) - (c1->v.c[2])) >= epsilon) return FALSE;
    if ((c0->colorspace == &CMYK) && (fabs ((c0->v.c[3]) - (c1->v.c[3])) >= epsilon)) return FALSE;

    return TRUE;
}

/**
 * Sets RGB values and colorspace in color.
 * \pre color != NULL && 0 <={r,g,b}<=1
 */
void
sp_color_set_rgb_float(SPColor *color, float r, float g, float b)
{
    g_return_if_fail(color != NULL);
    g_return_if_fail(r >= 0.0);
    g_return_if_fail(r <= 1.0);
    g_return_if_fail(g >= 0.0);
    g_return_if_fail(g <= 1.0);
    g_return_if_fail(b >= 0.0);
    g_return_if_fail(b <= 1.0);

    color->colorspace = &RGB;
    color->v.c[0] = r;
    color->v.c[1] = g;
    color->v.c[2] = b;
    color->v.c[3] = 0;
}

/**
 * Converts 32bit value to RGB floats and sets color.
 * \pre color != NULL 
 */
void
sp_color_set_rgb_rgba32(SPColor *color, guint32 value)
{
    g_return_if_fail (color != NULL);

    color->colorspace = &RGB;
    color->v.c[0] = (value >> 24) / 255.0F;
    color->v.c[1] = ((value >> 16) & 0xff) / 255.0F;
    color->v.c[2] = ((value >> 8) & 0xff) / 255.0F;
    color->v.c[3] = 0;
}

/**
 * Sets CMYK values and colorspace in color.
 * \pre color != NULL && 0 <={c,m,y,k}<=1
 */
void
sp_color_set_cmyk_float(SPColor *color, float c, float m, float y, float k)
{
    g_return_if_fail(color != NULL);
    g_return_if_fail(c >= 0.0);
    g_return_if_fail(c <= 1.0);
    g_return_if_fail(m >= 0.0);
    g_return_if_fail(m <= 1.0);
    g_return_if_fail(y >= 0.0);
    g_return_if_fail(y <= 1.0);
    g_return_if_fail(k >= 0.0);
    g_return_if_fail(k <= 1.0);

    color->colorspace = &CMYK;
    color->v.c[0] = c;
    color->v.c[1] = m;
    color->v.c[2] = y;
    color->v.c[3] = k;
}

/**
 * Convert SPColor with integer alpha value to 32bit RGBA value.
 * \pre color != NULL && alpha < 256
 */
guint32
sp_color_get_rgba32_ualpha(SPColor const *color, guint32 alpha)
{
    guint32 rgba;

    g_return_val_if_fail (color != NULL, 0x0);
    g_return_val_if_fail (alpha <= 0xff, 0x0);

    if (color->colorspace == &RGB) {
        rgba = SP_RGBA32_U_COMPOSE(SP_COLOR_F_TO_U(color->v.c[0]),
                                   SP_COLOR_F_TO_U(color->v.c[1]),
                                   SP_COLOR_F_TO_U(color->v.c[2]),
                                   alpha);
    } else {
        float rgb[3];
        sp_color_get_rgb_floatv (color, rgb);
        rgba = SP_RGBA32_U_COMPOSE(SP_COLOR_F_TO_U(rgb[0]),
                                   SP_COLOR_F_TO_U(rgb[1]),
                                   SP_COLOR_F_TO_U(rgb[2]),
                                   alpha);
    }

    return rgba;
}

/**
 * Convert SPColor with float alpha value to 32bit RGBA value.
 * \pre color != NULL && 0 <= alpha <= 1
 */
guint32
sp_color_get_rgba32_falpha(SPColor const *color, float alpha)
{
    g_return_val_if_fail(color != NULL, 0x0);
    g_return_val_if_fail(alpha >= 0.0, 0x0);
    g_return_val_if_fail(alpha <= 1.0, 0x0);

    return sp_color_get_rgba32_ualpha(color, SP_COLOR_F_TO_U(alpha));
}

/**
 * Fill rgb float array with values from SPColor.
 * \pre color != NULL && rgb != NULL && rgb[0-2] is meaningful
 */
void
sp_color_get_rgb_floatv(SPColor const *color, float *rgb)
{
    g_return_if_fail (color != NULL);
    g_return_if_fail (rgb != NULL);

    if (color->colorspace == &RGB) {
        rgb[0] = color->v.c[0];
        rgb[1] = color->v.c[1];
        rgb[2] = color->v.c[2];
    } else if (color->colorspace == &CMYK) {
        sp_color_cmyk_to_rgb_floatv(rgb,
                                    color->v.c[0],
                                    color->v.c[1],
                                    color->v.c[2],
                                    color->v.c[3]);
    }
}

/**
 * Fill cmyk float array with values from SPColor.
 * \pre color != NULL && cmyk != NULL && cmyk[0-3] is meaningful
 */
void
sp_color_get_cmyk_floatv(SPColor const *color, float *cmyk)
{
    g_return_if_fail (color != NULL);
    g_return_if_fail (cmyk != NULL);

    if (color->colorspace == &CMYK) {
        cmyk[0] = color->v.c[0];
        cmyk[1] = color->v.c[1];
        cmyk[2] = color->v.c[2];
        cmyk[3] = color->v.c[3];
    } else if (color->colorspace == &RGB) {
        sp_color_rgb_to_cmyk_floatv(cmyk,
                                    color->v.c[0],
                                    color->v.c[1],
                                    color->v.c[2]);
    }
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
}

/**
 * Fill rgb float array from h,s,v float values.
 */
void
sp_color_hsv_to_rgb_floatv (float *rgb, float h, float s, float v)
{
    gdouble f, w, q, t, d;

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

float
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

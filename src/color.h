#ifndef __SP_COLOR_H__
#define __SP_COLOR_H__

/** \file
 * Colors and colorspaces
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

#include <gdk/gdktypes.h>

struct SPColorSpace;

/* Useful composition macros */

#define SP_RGBA32_R_U(v) (((v) >> 24) & 0xff)
#define SP_RGBA32_G_U(v) (((v) >> 16) & 0xff)
#define SP_RGBA32_B_U(v) (((v) >> 8) & 0xff)
#define SP_RGBA32_A_U(v) ((v) & 0xff)
#define SP_COLOR_U_TO_F(v) ((v) / 255.0)
#define SP_COLOR_F_TO_U(v) ((unsigned int) ((v) * 255.9999))
#define SP_RGBA32_R_F(v) SP_COLOR_U_TO_F (SP_RGBA32_R_U (v))
#define SP_RGBA32_G_F(v) SP_COLOR_U_TO_F (SP_RGBA32_G_U (v))
#define SP_RGBA32_B_F(v) SP_COLOR_U_TO_F (SP_RGBA32_B_U (v))
#define SP_RGBA32_A_F(v) SP_COLOR_U_TO_F (SP_RGBA32_A_U (v))
#define SP_RGBA32_U_COMPOSE(r,g,b,a) ((((r) & 0xff) << 24) | (((g) & 0xff) << 16) | (((b) & 0xff) << 8) | ((a) & 0xff))
#define SP_RGBA32_F_COMPOSE(r,g,b,a) SP_RGBA32_U_COMPOSE (SP_COLOR_F_TO_U (r), SP_COLOR_F_TO_U (g), SP_COLOR_F_TO_U (b), SP_COLOR_F_TO_U (a))

typedef enum {
	SP_COLORSPACE_CLASS_INVALID,
	SP_COLORSPACE_CLASS_NONE,
	SP_COLORSPACE_CLASS_UNKNOWN,
	SP_COLORSPACE_CLASS_PROCESS,
	SP_COLORSPACE_CLASS_SPOT
} SPColorSpaceClass;

typedef enum {
	SP_COLORSPACE_TYPE_INVALID,
	SP_COLORSPACE_TYPE_NONE,
	SP_COLORSPACE_TYPE_UNKNOWN,
	SP_COLORSPACE_TYPE_RGB,
	SP_COLORSPACE_TYPE_CMYK
} SPColorSpaceType;

/**
 * An RGBA color in an SPColorSpace
 */
struct SPColor {
	const SPColorSpace *colorspace;
	union {
		float c[4];
	} v;
};

SPColorSpaceClass sp_color_get_colorspace_class (const SPColor *color);
SPColorSpaceType sp_color_get_colorspace_type (const SPColor *color);

void sp_color_copy (SPColor *dst, const SPColor *src);

gboolean sp_color_is_equal (const SPColor *c0, const SPColor *c1);
gboolean sp_color_is_close (const SPColor *c0, const SPColor *c1, float epsilon);

void sp_color_set_rgb_float (SPColor *color, float r, float g, float b);
void sp_color_set_rgb_rgba32 (SPColor *color, guint32 value);

void sp_color_set_cmyk_float (SPColor *color, float c, float m, float y, float k);

guint32 sp_color_get_rgba32_ualpha (const SPColor *color, guint32 alpha);
guint32 sp_color_get_rgba32_falpha (const SPColor *color, float alpha);

void sp_color_get_rgb_floatv (const SPColor *color, float *rgb);
void sp_color_get_cmyk_floatv (const SPColor *color, float *cmyk);

/* Plain mode helpers */

void sp_color_rgb_to_hsv_floatv (float *hsv, float r, float g, float b);
void sp_color_hsv_to_rgb_floatv (float *rgb, float h, float s, float v);

void sp_color_rgb_to_hsl_floatv (float *hsl, float r, float g, float b);
void sp_color_hsl_to_rgb_floatv (float *rgb, float h, float s, float l);

void sp_color_rgb_to_cmyk_floatv (float *cmyk, float r, float g, float b);
void sp_color_cmyk_to_rgb_floatv (float *rgb, float c, float m, float y, float k);



#endif


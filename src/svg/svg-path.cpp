#define __SP_SVG_PARSE_C__
/*
   svg-path.c: Parse SVG path element data into bezier path.

   Copyright (C) 2000 Eazel, Inc.
   Copyright (C) 2000 Lauris Kaplinski
   Copyright (C) 2001 Ximian, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors:
     Raph Levien <raph@artofcode.com>
     Lauris Kaplinski <lauris@ximian.com>
*/

#include <cstring>
#include <string>
#include <cassert>
#include <glib/gmem.h>
#include <glib/gmessages.h>
#include <glib/gstrfuncs.h>
#include <glib.h> // g_assert()

#include "libnr/n-art-bpath.h"
#include "gnome-canvas-bpath-util.h"
#include "svg/svg.h"
#include "svg/path-string.h"

#include <2geom/pathvector.h>
#include <2geom/path.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/svg-path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/exception.h>

/* This module parses an SVG path element into an RsvgBpathDef.

   At present, there is no support for <marker> or any other contextual
   information from the SVG file. The API will need to change rather
   significantly to support these.

   Reference: SVG working draft 3 March 2000, section 8.
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  /*  M_PI  */

/* We are lazy ;-) (Lauris) */
#define rsvg_bpath_def_new gnome_canvas_bpath_def_new
#define rsvg_bpath_def_moveto gnome_canvas_bpath_def_moveto
#define rsvg_bpath_def_lineto gnome_canvas_bpath_def_lineto
#define rsvg_bpath_def_curveto gnome_canvas_bpath_def_curveto
#define rsvg_bpath_def_closepath gnome_canvas_bpath_def_closepath

struct RSVGParsePathCtx {
    GnomeCanvasBpathDef *bpath;
    double cpx, cpy;  /* current point */
    double rpx, rpy;  /* reflection point (for 's' and 't' commands) */
    double spx, spy;  /* beginning of current subpath point */
    char cmd;         /* current command (lowercase) */
    int param;        /* parameter number */
    bool rel;         /* true if relative coords */
    double params[7]; /* parameters that have been parsed */
};

static void rsvg_path_arc_segment(RSVGParsePathCtx *ctx,
              double xc, double yc,
              double th0, double th1,
              double rx, double ry, double x_axis_rotation)
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double th_half;

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0)); 
    /* inverse transform compared with rsvg_path_arc */
    a00 = cos_th * rx;
    a01 = -sin_th * ry;
    a10 = sin_th * rx;
    a11 = cos_th * ry;

    th_half = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * sin(th_half * 0.5) * sin(th_half * 0.5) / sin(th_half);
    x1 = xc + cos (th0) - t * sin (th0);
    y1 = yc + sin (th0) + t * cos (th0);
    x3 = xc + cos (th1);
    y3 = yc + sin (th1);
    x2 = x3 + t * sin (th1);
    y2 = y3 - t * cos (th1);
    rsvg_bpath_def_curveto(ctx->bpath,
                           a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
                           a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                           a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

/**
 * rsvg_path_arc: Add an RSVG arc to the path context.
 * @ctx: Path context.
 * @rx: Radius in x direction (before rotation).
 * @ry: Radius in y direction (before rotation).
 * @x_axis_rotation: Rotation angle for axes.
 * @large_arc_flag: 0 for arc length <= 180, 1 for arc >= 180.
 * @sweep: 0 for "negative angle", 1 for "positive angle".
 * @x: New x coordinate.
 * @y: New y coordinate.
 *
 **/
static void rsvg_path_arc (RSVGParsePathCtx *ctx,
                           double rx, double ry, double x_axis_rotation,
                           int large_arc_flag, int sweep_flag,
                           double x, double y)
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    double px, py, pl;
    int i, n_segs;

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0));

    /*                                                                            
                                                                                  Correction of out-of-range radii as described in Appendix F.6.6:           

                                                                                  1. Ensure radii are non-zero (Done?).                                      
                                                                                  2. Ensure that radii are positive.                                         
                                                                                  3. Ensure that radii are large enough.                                     
    */                                                                            

    if(rx < 0.0) rx = -rx;                                                        
    if(ry < 0.0) ry = -ry;                                                        

    px = cos_th * (ctx->cpx - x) * 0.5 + sin_th * (ctx->cpy - y) * 0.5;           
    py = cos_th * (ctx->cpy - y) * 0.5 - sin_th * (ctx->cpx - x) * 0.5;           
    pl = (px * px) / (rx * rx) + (py * py) / (ry * ry);                           

    if(pl > 1.0)                                                                  
    {                                                                             
        pl  = sqrt(pl);                                                           
        rx *= pl;                                                                 
        ry *= pl;                                                                 
    }                                                                             

    /* Proceed with computations as described in Appendix F.6.5 */                

    a00 = cos_th / rx;
    a01 = sin_th / rx;
    a10 = -sin_th / ry;
    a11 = cos_th / ry;
    x0 = a00 * ctx->cpx + a01 * ctx->cpy;
    y0 = a10 * ctx->cpx + a11 * ctx->cpy;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
       (x1, y1) is new point in transformed coordinate space.

       The arc fits a unit-radius circle in this space.
    */
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    sfactor_sq = 1.0 / d - 0.25;
    if (sfactor_sq < 0) sfactor_sq = 0;
    sfactor = sqrt (sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */

    th0 = atan2 (y0 - yc, x0 - xc);
    th1 = atan2 (y1 - yc, x1 - xc);

    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag)
        th_arc += 2 * M_PI;
    else if (th_arc > 0 && !sweep_flag)
        th_arc -= 2 * M_PI;

    n_segs = (int) ceil (fabs (th_arc / (M_PI * 0.5 + 0.001)));

    for (i = 0; i < n_segs; i++) {
        rsvg_path_arc_segment(ctx, xc, yc,
                              th0 + i * th_arc / n_segs,
                              th0 + (i + 1) * th_arc / n_segs,
                              rx, ry, x_axis_rotation);
    }

    ctx->cpx = x;
    ctx->cpy = y;
}

static void rsvg_parse_path_do_cmd(RSVGParsePathCtx *ctx)
{
    double x1, y1, x2, y2, x3, y3;

    switch (ctx->cmd) {
    case 'm':
        /* moveto */
        if (ctx->param == 2)
        {
#ifdef VERBOSE
            g_print ("'m' moveto %g,%g\n",
                     ctx->params[0], ctx->params[1]);
#endif
            rsvg_bpath_def_moveto (ctx->bpath,
                                   ctx->params[0], ctx->params[1]);
            ctx->cpx = ctx->rpx = ctx->spx = ctx->params[0];
            ctx->cpy = ctx->rpy = ctx->spy = ctx->params[1];
            ctx->param = 0;
            ctx->cmd = 'l';
            /* Ref: http://www.w3.org/TR/SVG11/paths.html#PathDataMovetoCommands: "If a moveto is
             * followed by multiple pairs of coordinates, the subsequent pairs are treated as
             * implicit lineto commands." */
        }
        break;

    case 'l':
        /* lineto */
        if (ctx->param == 2)
        {
#ifdef VERBOSE
            g_print ("'l' lineto %g,%g\n",
                     ctx->params[0], ctx->params[1]);
#endif
            rsvg_bpath_def_lineto (ctx->bpath,
                                   ctx->params[0], ctx->params[1]);
            ctx->cpx = ctx->rpx = ctx->params[0];
            ctx->cpy = ctx->rpy = ctx->params[1];
            ctx->param = 0;
        }
        break;

    case 'c':
        /* curveto */
        if (ctx->param == 6)
        {
            x1 = ctx->params[0];
            y1 = ctx->params[1];
            x2 = ctx->params[2];
            y2 = ctx->params[3];
            x3 = ctx->params[4];
            y3 = ctx->params[5];
#ifdef VERBOSE
            g_print ("'c' curveto %g,%g %g,%g, %g,%g\n",
                     x1, y1, x2, y2, x3, y3);
#endif
            rsvg_bpath_def_curveto (ctx->bpath,
                                    x1, y1, x2, y2, x3, y3);
            ctx->rpx = x2;
            ctx->rpy = y2;
            ctx->cpx = x3;
            ctx->cpy = y3;
            ctx->param = 0;
        }
        break;

    case 's':
        /* smooth curveto */
        if (ctx->param == 4)
        {
            x1 = 2 * ctx->cpx - ctx->rpx;
            y1 = 2 * ctx->cpy - ctx->rpy;
            x2 = ctx->params[0];
            y2 = ctx->params[1];
            x3 = ctx->params[2];
            y3 = ctx->params[3];
#ifdef VERBOSE
            g_print ("'s' curveto %g,%g %g,%g, %g,%g\n",
                     x1, y1, x2, y2, x3, y3);
#endif
            rsvg_bpath_def_curveto (ctx->bpath,
                                    x1, y1, x2, y2, x3, y3);
            ctx->rpx = x2;
            ctx->rpy = y2;
            ctx->cpx = x3;
            ctx->cpy = y3;
            ctx->param = 0;
        }
        break;

    case 'h':
        /* horizontal lineto */
        if (ctx->param == 1) {
#ifdef VERBOSE
            g_print ("'h' lineto %g,%g\n",
                     ctx->params[0], ctx->cpy);
#endif
            rsvg_bpath_def_lineto (ctx->bpath,
                                   ctx->params[0], ctx->cpy);
            ctx->cpx = ctx->rpx = ctx->params[0];
            ctx->param = 0;
        }
        break;

    case 'v':
        /* vertical lineto */
        if (ctx->param == 1) {
#ifdef VERBOSE
            g_print ("'v' lineto %g,%g\n",
                     ctx->cpx, ctx->params[0]);
#endif
            rsvg_bpath_def_lineto (ctx->bpath,
                                   ctx->cpx, ctx->params[0]);
            ctx->cpy = ctx->rpy = ctx->params[0];
            ctx->param = 0;
        }
        break;

    case 'q':
        /* quadratic bezier curveto */

        /* non-normative reference:
           http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/beziers.html
        */
        if (ctx->param == 4)
        {
            /* raise quadratic bezier to cubic */
            x1 = (ctx->cpx + 2 * ctx->params[0]) * (1.0 / 3.0);
            y1 = (ctx->cpy + 2 * ctx->params[1]) * (1.0 / 3.0);
            x3 = ctx->params[2];
            y3 = ctx->params[3];
            x2 = (x3 + 2 * ctx->params[0]) * (1.0 / 3.0);
            y2 = (y3 + 2 * ctx->params[1]) * (1.0 / 3.0);
#ifdef VERBOSE
            g_print("'q' curveto %g,%g %g,%g, %g,%g\n",
                    x1, y1, x2, y2, x3, y3);
#endif
            rsvg_bpath_def_curveto(ctx->bpath,
                                   x1, y1, x2, y2, x3, y3);
            ctx->rpx = ctx->params[0];
            ctx->rpy = ctx->params[1];
            ctx->cpx = x3;
            ctx->cpy = y3;
            ctx->param = 0;
        }
        break;

    case 't':
        /* Truetype quadratic bezier curveto */
        if (ctx->param == 2) {
            double xc, yc; /* quadratic control point */

            xc = 2 * ctx->cpx - ctx->rpx;
            yc = 2 * ctx->cpy - ctx->rpy;
            /* generate a quadratic bezier with control point = xc, yc */
            x1 = (ctx->cpx + 2 * xc) * (1.0 / 3.0);
            y1 = (ctx->cpy + 2 * yc) * (1.0 / 3.0);
            x3 = ctx->params[0];
            y3 = ctx->params[1];
            x2 = (x3 + 2 * xc) * (1.0 / 3.0);
            y2 = (y3 + 2 * yc) * (1.0 / 3.0);
#ifdef VERBOSE
            g_print ("'t' curveto %g,%g %g,%g, %g,%g\n",
                     x1, y1, x2, y2, x3, y3);
#endif
            rsvg_bpath_def_curveto (ctx->bpath,
                                    x1, y1, x2, y2, x3, y3);
            ctx->rpx = xc;
            ctx->rpy = yc;
            ctx->cpx = x3;
            ctx->cpy = y3;
            ctx->param = 0;
        }
        break;

    case 'a':
        if (ctx->param == 7)
        {
            rsvg_path_arc(ctx,
                          ctx->params[0], ctx->params[1], ctx->params[2],
                          (int) ctx->params[3], (int) ctx->params[4],
                          ctx->params[5], ctx->params[6]);
            ctx->param = 0;
        }
        break;

    default:
        g_assert_not_reached();
    }
}

static void rsvg_parse_path_do_closepath(RSVGParsePathCtx *const ctx, const char next_cmd)
{
    g_assert(ctx->param == 0);

    rsvg_bpath_def_closepath (ctx->bpath);
    ctx->cpx = ctx->rpx = ctx->spx;
    ctx->cpy = ctx->rpy = ctx->spy;

    if (next_cmd != 0 && next_cmd != 'm') {
        // This makes sure we do the right moveto if the closepath is followed by anything other than a moveto.
        /* Ref: http://www.w3.org/TR/SVG11/paths.html#PathDataClosePathCommand: "If a
         * "closepath" is followed immediately by a "moveto", then the "moveto" identifies
         * the start point of the next subpath. If a "closepath" is followed immediately by
         * any other command, then the next subpath starts at the same initial point as the
         * current subpath." */

        ctx->cmd = 'm';
        ctx->params[0] = ctx->cpx;
        ctx->params[1] = ctx->cpy;
        ctx->param = 2;
        rsvg_parse_path_do_cmd(ctx);

        /* Any token after a closepath must be a command, not a parameter.  We enforce this
         * by clearing cmd rather than leaving as 'm'. */
        ctx->cmd = '\0';
    }
}

static char const* rsvg_parse_unsigned_int(guint64 *val, char const *begin, bool zeroVal = true) {
    if (zeroVal) *val = 0;
    while('0' <= *begin && *begin <= '9') {
        *val *= 10;
        *val += *begin - '0';
        begin++;
    }
    return begin;
}

static char const* rsvg_parse_sign(bool *neg, char const *begin) {
    *neg = false;
    if (*begin == '+') {
        begin++;
    } else if (*begin == '-') {
        *neg = true;
        begin++;
    }
    return begin;
}

static char const* rsvg_parse_int(gint64 *val, char const *begin) {
    bool neg;
    char const *begin_of_int = rsvg_parse_sign(&neg, begin);
    char const *end_of_int = rsvg_parse_unsigned_int((guint64*)val, begin_of_int);
    if (neg) *val = -(*val);
    return end_of_int==begin_of_int ? begin : end_of_int;
}

static char const* rsvg_parse_unsigned_float(double *val, char const *begin) {
    // A number is either one or more digits, optionally followed by a period and zero or more digits (and an exponent),
    //                 or zero or more digits, followed by a period and one or more digits (and an exponent)
    // See http://www.w3.org/TR/SVG/paths.html#PathDataBNF
    guint64 intval;
    int exp=0;
    char const *begin_of_num = begin;
    char const *end_of_num = rsvg_parse_unsigned_int(&intval, begin_of_num);
    if (*end_of_num == '.') {
        char const *begin_of_frac = end_of_num+1;
        char const *end_of_frac = rsvg_parse_unsigned_int(&intval, begin_of_frac, false);
        if (end_of_num != begin_of_num || end_of_frac != begin_of_frac) {
            end_of_num = end_of_frac;
            exp = -(int)(end_of_frac-begin_of_frac);
        }
    }
    if (end_of_num != begin_of_num && (*end_of_num == 'e' || *end_of_num == 'E')) {
        gint64 exponent;
        char const *begin_of_exp = end_of_num+1;
        char const *end_of_exp = rsvg_parse_int(&exponent, begin_of_exp);
        if (end_of_exp != begin_of_exp) {
            end_of_num = end_of_exp;
            exp += (int)exponent;
        }
    }

    *val = ( exp < 0
             ? intval / pow(10, -exp)
             : intval * pow(10, exp) );
    return end_of_num;
}

static char const* rsvg_parse_float(double *val, char const *begin) {
    bool neg;
    char const *begin_of_num = rsvg_parse_sign(&neg, begin);
    char const *end_of_num = rsvg_parse_unsigned_float(val, begin_of_num);
    if (neg) *val = -(*val);
    return end_of_num == begin_of_num ? begin : end_of_num;
}

static void rsvg_parse_path_data(RSVGParsePathCtx *ctx, char const *const begin) {
    /* fixme: Do better error processing: e.g. at least stop parsing as soon as we find an error.
     * At some point we'll need to do all of
     * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing.
     */

    /* Comma is always allowed after a number token (so long as it's followed by another number:
     * see require_number), and never allowed anywhere else.  Only one comma is allowed between
     * neighbouring number tokens. */
    bool comma_allowed = false;

    /* After a command other than closepath, and after a comma, we require a number. */
    bool require_number = false;

    for (char const *cur = begin;; ++cur) {
        int const c = *cur;
        if (c <= ' ') {
            switch (c) {
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    /* wsp */
                    break;

                case '\0':
                    if (require_number || ctx->param) {
                        goto error;
                    }
                    goto done;

                default:
                    goto error;
            }
        } else if (c == ',') {
            if (!comma_allowed) {
                goto error;
            }
            comma_allowed = false;
            require_number = true;
        } else if (c <= '9') {
            if (!ctx->cmd || ctx->cmd == 'z') {
                goto error;
            }

            double val;
            char const *const end = rsvg_parse_float(&val, cur);
            if (cur == end) {
                goto error;
            }

            /* Special requirements for elliptical-arc arguments. */
            if (ctx->cmd == 'a') {
                if (ctx->param < 2) {
                    if (c <= '-') {
                        /* Error: sign not allowed for first two params. */
                        goto error;
                    }
                } else if (ctx->param <= 4 && ctx->param >= 3) {
                    if (end - cur != 1 || c < '0' || c > '1') {
                        /* Error: flag must be either literally "0" or literally "1". */
                        goto error;
                    }
                }
            }

            if (ctx->rel) {
                /* Handle relative coordinates. */
                switch (ctx->cmd) {
                case 'l':
                case 'm':
                case 'c':
                case 's':
                case 'q':
                case 't':
                    if ( ctx->param & 1 ) {
                        val += ctx->cpy; /* odd param, y */
                    } else {
                        val += ctx->cpx; /* even param, x */
                    }
                    break;
                case 'a':
                    /* rule: sixth and seventh are x and y, rest are not
                       relative */
                    if (ctx->param == 5)
                        val += ctx->cpx;
                    else if (ctx->param == 6)
                        val += ctx->cpy;
                    break;
                case 'h':
                    /* rule: x-relative */
                    val += ctx->cpx;
                    break;
                case 'v':
                    /* rule: y-relative */
                    val += ctx->cpy;
                    break;
                }
            }
            ctx->params[ctx->param++] = val;
            rsvg_parse_path_do_cmd(ctx);
            comma_allowed = true;
            require_number = false;
            cur = end - 1;
        } else {
            /* Command. */
            if (require_number || ctx->param) {
                goto error;
            }
            char next_cmd;
            if (c <= 'Z') {
                next_cmd = c + ('a' - 'A');
                ctx->rel = false;
            } else {
                next_cmd = c;
                ctx->rel = true;
            }

            comma_allowed = false;
            require_number = true;
            switch (next_cmd) {
                case 'z':
                    require_number = false;
                case 'm':
                case 'l':
                case 'h':
                case 'v':
                case 'c':
                case 's':
                case 'q':
                case 't':
                case 'a':
                    /* valid command */
                    break;

                default:
                    goto error;
            }

            if (ctx->cmd == 'z') {
                /* Closepath is the only command that allows no arguments. */
                rsvg_parse_path_do_closepath(ctx, next_cmd);
            }
            ctx->cmd = next_cmd;
        }
    }

done:
    if (ctx->cmd == 'z') {
        rsvg_parse_path_do_closepath(ctx, 0);
    }
    return;

error:
    /* todo: set an error indicator. */
    goto done;
}


NArtBpath *sp_svg_read_path(gchar const *str)
{
    RSVGParsePathCtx ctx;
    NArtBpath *bpath;

    ctx.bpath = gnome_canvas_bpath_def_new ();
    ctx.cpx = 0.0;
    ctx.cpy = 0.0;
    ctx.cmd = 0;
    ctx.param = 0;

    rsvg_parse_path_data (&ctx, str);

    gnome_canvas_bpath_def_art_finish (ctx.bpath);

    bpath = g_new (NArtBpath, ctx.bpath->n_bpath);
    memcpy (bpath, ctx.bpath->bpath, ctx.bpath->n_bpath * sizeof (NArtBpath));
    g_assert ((bpath + ctx.bpath->n_bpath - 1)->code == NR_END);
    gnome_canvas_bpath_def_unref (ctx.bpath);

    return bpath;
}

/*
 * Parses the path in str. When an error is found in the pathstring, this method
 * returns a truncated path up to where the error was found in the pathstring.
 * Returns an empty PathVector when str==NULL
 */
Geom::PathVector sp_svg_read_pathv(char const * str)
{
    Geom::PathVector pathv;
    if (!str)
        return pathv;  // return empty pathvector when str == NULL


    typedef std::back_insert_iterator<Geom::PathVector> Inserter;
    Inserter iter(pathv);
    Geom::SVGPathGenerator<Inserter> generator(iter);

    try {
        Geom::parse_svg_path(str, generator);
    }
    catch (Geom::SVGPathParseError e) {
        generator.finish();
        g_warning("Malformed SVG path, truncated path up to where error was found.\n Input path=\"%s\"\n Parsed path=\"%s\"", str, sp_svg_write_path(pathv));
    }

    return pathv;
}

gchar *sp_svg_write_path(NArtBpath const *bpath)
{
    bool closed=false;
    
    g_return_val_if_fail (bpath != NULL, NULL);

    Inkscape::SVG::PathString str;

    for (int i = 0; bpath[i].code != NR_END; i++){
        switch (bpath [i].code){
        case NR_LINETO:
            if (!closed || bpath[i+1].code == NR_LINETO || bpath[i+1].code == NR_CURVETO) {
                str.lineTo(bpath[i].x3, bpath[i].y3);
            }
            break;

        case NR_CURVETO:
            str.curveTo(bpath[i].x1, bpath[i].y1,
                        bpath[i].x2, bpath[i].y2,
                        bpath[i].x3, bpath[i].y3);
            break;

        case NR_MOVETO_OPEN:
        case NR_MOVETO:
            if (closed) {
                str.closePath();
            }
            closed = ( bpath[i].code == NR_MOVETO );
            str.moveTo(bpath[i].x3, bpath[i].y3);
            break;

        default:
            g_assert_not_reached ();
        }
    }
    if (closed) {
        str.closePath();
    }

    return g_strdup(str.c_str());
}

static void sp_svg_write_curve(Inkscape::SVG::PathString & str, Geom::Curve const * c) {
    if(Geom::LineSegment const *line_segment = dynamic_cast<Geom::LineSegment const  *>(c)) {
        // don't serialize stitch segments
        if (!dynamic_cast<Geom::Path::StitchSegment const *>(c)) {
            str.lineTo( (*line_segment)[1][0], (*line_segment)[1][1] );
        }
    }
    else if(Geom::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::QuadraticBezier const  *>(c)) {
        str.quadTo( (*quadratic_bezier)[1][0], (*quadratic_bezier)[1][1],
                    (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1] );
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(c)) {
        str.curveTo( (*cubic_bezier)[1][0], (*cubic_bezier)[1][1],
                     (*cubic_bezier)[2][0], (*cubic_bezier)[2][1],
                     (*cubic_bezier)[3][0], (*cubic_bezier)[3][1] );
    }
    else if(Geom::SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<Geom::SVGEllipticalArc const *>(c)) {
        str.arcTo( svg_elliptical_arc->ray(0), svg_elliptical_arc->ray(1),
                   svg_elliptical_arc->rotation_angle(),
                   svg_elliptical_arc->large_arc_flag(), svg_elliptical_arc->sweep_flag(),
                   svg_elliptical_arc->finalPoint() );
    }
    else if(Geom::HLineSegment const *hline_segment = dynamic_cast<Geom::HLineSegment const  *>(c)) {
        str.horizontalLineTo( hline_segment->finalPoint()[0] );
    }
    else if(Geom::VLineSegment const *vline_segment = dynamic_cast<Geom::VLineSegment const  *>(c)) {
        str.verticalLineTo( vline_segment->finalPoint()[1] );
    } else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c->toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            sp_svg_write_curve(str, &(*iter));
        }
    }
}

gchar * sp_svg_write_path(Geom::PathVector const &p) {
    Inkscape::SVG::PathString str;

    for(Geom::PathVector::const_iterator pit = p.begin(); pit != p.end(); pit++) {
        str.moveTo( pit->initialPoint()[0], pit->initialPoint()[1] );

        for(Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); cit++) {
            sp_svg_write_curve(str, &(*cit));
        }

        if (pit->closed()) {
            str.closePath();
        }
    }

    return g_strdup(str.c_str());
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

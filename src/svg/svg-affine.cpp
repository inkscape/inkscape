#define __SP_SVG_AFFINE_C__

/*
 * SVG data parser
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Raph Levien <raph@acm.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 1999 Raph Levien
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <glib/gstrfuncs.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <2geom/transforms.h>
#include <2geom/angle.h>
#include <libnr/nr-convert2geom.h>
#include "svg.h"
#include "preferences.h"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

bool
sp_svg_transform_read(gchar const *str, Geom::Matrix *transform)
{
    NR::Matrix mat;
    if (sp_svg_transform_read(str, &mat)) {
        *transform = mat;
        return true;
    } else {
        return false;
    }
}

bool
sp_svg_transform_read(gchar const *str, NR::Matrix *transform)
{
    int idx;
    char keyword[32];
    double args[6];
    int n_args;
    size_t key_len;

    if (str == NULL) return false;

    NR::Matrix a(NR::identity());

    idx = 0;
    while (str[idx]) {
        /* skip initial whitespace */
        while (g_ascii_isspace (str[idx])) idx++;

        /* parse keyword */
        for (key_len = 0; key_len < sizeof (keyword); key_len++) {
            char c;

            c = str[idx];
            if (g_ascii_isalpha (c) || c == '-') {
                keyword[key_len] = str[idx++];
            } else {
                break;
            }
        }
        if (key_len >= sizeof (keyword)) return false;
        keyword[key_len] = '\0';

        /* skip whitespace */
        while (g_ascii_isspace (str[idx])) idx++;

        if (str[idx] != '(') return false;
        idx++;

        for (n_args = 0; ; n_args++) {
            char c;
            char *end_ptr;

            /* skip whitespace */
            while (g_ascii_isspace (str[idx])) idx++;
            c = str[idx];
            if (g_ascii_isdigit (c) || c == '+' || c == '-' || c == '.') {
                if (n_args == sizeof (args) / sizeof (args[0])) return false; /* Too many args */
                args[n_args] = g_ascii_strtod (str + idx, &end_ptr);
                
                //printf("took %d chars from '%s' to make %f\n",
                //		end_ptr-(str+idx),
                //		str+idx,
                //		args[n_args]);

                idx = end_ptr - (char *) str;

                while (g_ascii_isspace (str[idx])) idx++;

                /* skip optional comma */
                if (str[idx] == ',') idx++;
            } else if (c == ')') {
                break;
            } else {
                return false;
            }
        }
        idx++;

        /* ok, have parsed keyword and args, now modify the transform */
        if (!strcmp (keyword, "matrix")) {
            if (n_args != 6) return false;
            a = (*NR_MATRIX_D_FROM_DOUBLE(args)) * a;
        } else if (!strcmp (keyword, "translate")) {
            if (n_args == 1) {
                args[1] = 0;
            } else if (n_args != 2) {
                return false;
            }
            a = Geom::Translate(args[0], args[1]) * a;
        } else if (!strcmp (keyword, "scale")) {
            if (n_args == 1) {
                args[1] = args[0];
            } else if (n_args != 2) {
                return false;
            }
            a = Geom::Scale(args[0], args[1]) * a;
        } else if (!strcmp (keyword, "rotate")) {
            if (n_args != 1 && n_args != 3) {
                return false;
            }
            Geom::Rotate const rot(Geom::deg_to_rad(args[0]));
            if (n_args == 3) {
                a = ( Geom::Translate(-args[1], -args[2])
                      * rot
                      * Geom::Translate(args[1], args[2])
                      * Geom::Matrix(a) );
            } else {
                a = rot * a;
            }
        } else if (!strcmp (keyword, "skewX")) {
            if (n_args != 1) return false;
            a = ( NR::Matrix(1, 0,
                     tan(args[0] * M_PI / 180.0), 1,
                     0, 0)
                  * a );
        } else if (!strcmp (keyword, "skewY")) {
            if (n_args != 1) return false;
            a = ( NR::Matrix(1, tan(args[0] * M_PI / 180.0),
                     0, 1,
                     0, 0)
                  * a );
        } else {
            return false; /* unknown keyword */
        }
        /* Skip trailing whitespace */
             while (g_ascii_isspace (str[idx])) idx++;
    }

    *transform = a;
    return true;
}

#define EQ(a,b) (fabs ((a) - (b)) < 1e-9)

gchar *
sp_svg_transform_write(Geom::Matrix const &transform)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    double e = 0.000001 * transform.descrim();
    int prec = prefs->getInt("/options/svgoutput/numericprecision", 8);
    int min_exp = prefs->getInt("/options/svgoutput/minimumexponent", -8);

    /* fixme: We could use t1 * t1 + t2 * t2 here instead */
    if ( Geom::are_near(transform[1], 0.0, e) && Geom::are_near (transform[2], 0.0, e)) {
        if (Geom::are_near (transform[4], 0.0, e) && Geom::are_near (transform[5], 0.0, e)) {
            if (Geom::are_near (transform[0], 1.0, e) && Geom::are_near (transform[3], 1.0, e)) {
                /* We are more or less identity */
                return NULL;
            } else {
                /* We are more or less scale */
                gchar c[256];
                unsigned p = 0;
                strcpy (c + p, "scale(");
                p += 6;
                p += sp_svg_number_write_de (c + p, transform[0], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[3], prec, min_exp);
                c[p++] = ')';
                c[p] = '\000';
                g_assert( p <= sizeof(c) );
                return g_strdup(c);
            }
        } else {
            if (Geom::are_near (transform[0], 1.0, e) && Geom::are_near (transform[3], 1.0, e)) {
                /* We are more or less translate */
                gchar c[256];
                unsigned p = 0;
                strcpy (c + p, "translate(");
                p += 10;
                p += sp_svg_number_write_de (c + p, transform[4], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[5], prec, min_exp);
                c[p++] = ')';
                c[p] = '\000';
                g_assert( p <= sizeof(c) );
                return g_strdup(c);
            } else {
                gchar c[256];
                unsigned p = 0;
                strcpy (c + p, "matrix(");
                p += 7;
                p += sp_svg_number_write_de (c + p, transform[0], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[1], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[2], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[3], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[4], prec, min_exp);
                c[p++] = ',';
                p += sp_svg_number_write_de (c + p, transform[5], prec, min_exp);
                c[p++] = ')';
                c[p] = '\000';
                g_assert( p <= sizeof(c) );
                return g_strdup(c);
            }
        }
    } else {
        gchar c[256];
        unsigned p = 0;
        strcpy (c + p, "matrix(");
        p += 7;
        p += sp_svg_number_write_de (c + p, transform[0], prec, min_exp);
        c[p++] = ',';
        p += sp_svg_number_write_de (c + p, transform[1], prec, min_exp);
        c[p++] = ',';
        p += sp_svg_number_write_de (c + p, transform[2], prec, min_exp);
        c[p++] = ',';
        p += sp_svg_number_write_de (c + p, transform[3], prec, min_exp);
        c[p++] = ',';
        p += sp_svg_number_write_de (c + p, transform[4], prec, min_exp);
        c[p++] = ',';
        p += sp_svg_number_write_de (c + p, transform[5], prec, min_exp);
        c[p++] = ')';
        c[p] = '\000';
        g_assert( p <= sizeof(c) );
        return g_strdup(c);
    }
}


gchar *
sp_svg_transform_write(Geom::Matrix const *transform)
{
    return sp_svg_transform_write(*transform);
}

gchar *
sp_svg_transform_write(NR::Matrix const &transform)
{
    return sp_svg_transform_write((Geom::Matrix)transform);
}

gchar *
sp_svg_transform_write(NR::Matrix const *transform)
{
    return sp_svg_transform_write(*transform);
}


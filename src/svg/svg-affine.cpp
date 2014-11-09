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
#include <glib.h>
#include <2geom/transforms.h>
#include <2geom/angle.h>
#include "svg.h"
#include "preferences.h"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

bool
sp_svg_transform_read(gchar const *str, Geom::Affine *transform)
{
    int idx;
    char keyword[32];
    double args[6];
    int n_args;
    size_t key_len;

    if (str == NULL) return false;

    Geom::Affine a(Geom::identity());

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
            a = (*((Geom::Affine *) &(args)[0])) * a;
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
                      * Geom::Affine(a) );
            } else {
                a = rot * a;
            }
        } else if (!strcmp (keyword, "skewX")) {
            if (n_args != 1) return false;
            a = ( Geom::Affine(1, 0,
                     tan(args[0] * M_PI / 180.0), 1,
                     0, 0)
                  * a );
        } else if (!strcmp (keyword, "skewY")) {
            if (n_args != 1) return false;
            a = ( Geom::Affine(1, tan(args[0] * M_PI / 180.0),
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
sp_svg_transform_write(Geom::Affine const &transform)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    double e = 0.000001 * transform.descrim();
    int prec = prefs->getInt("/options/svgoutput/numericprecision", 8);
    int min_exp = prefs->getInt("/options/svgoutput/minimumexponent", -8);

    // Special case: when all fields of the affine are zero,
    // the optimized transformation is scale(0)
    if (transform[0] == 0 && transform[1] == 0 && transform[2] == 0 &&
        transform[3] == 0 && transform[4] == 0 && transform[5] == 0)
    {
        return g_strdup("scale(0)");
    }

    // FIXME legacy C code!
    // the function sp_svg_number_write_de is stopping me from using a proper C++ string

    gchar c[256]; // string buffer
    unsigned p = 0; // position in the buffer

    if (transform.isIdentity()) {
        // We are more or less identity, so no transform attribute needed:
        return NULL;
    } else if (transform.isScale()) {
        // We are more or less a uniform scale
        strcpy (c + p, "scale(");
        p += 6;
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[0], prec, min_exp );
        if (Geom::are_near(transform[3], 0.0, e)) {
            c[p++] = ')';
            c[p] = '\000';
        } else {
            c[p++] = ',';
            p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[3], prec, min_exp );
            c[p++] = ')';
            c[p] = '\000';
        }
    } else if (transform.isTranslation()) {
        // We are more or less a pure translation
        strcpy (c + p, "translate(");
        p += 10;
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[4], prec, min_exp );
        if (Geom::are_near(transform[5], 0.0, e)) {
            c[p++] = ')';
            c[p] = '\000';
        } else {
            c[p++] = ',';
            p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[5], prec, min_exp );
            c[p++] = ')';
            c[p] = '\000';
        }
    } else if (transform.isRotation()) {
        // We are more or less a pure rotation
        strcpy(c + p, "rotate(");
        p += 7;

        double angle = std::atan2(transform[1], transform[0]) * (180 / M_PI);
        p += sp_svg_number_write_de(c + p, sizeof(c) - p, angle, prec, min_exp);

        c[p++] = ')';
        c[p] = '\000';
    /* } else if (transform.withoutTranslation().isRotation()) {
        // FIXME someone please figure out if this can actually be done
        // The rotation angle is correct, the points are not
        // Refer to the matrix in svg-affine-test.h

        // We are a rotation about a special axis
        strcpy(c + p, "rotate(");
        p += 7;

        Geom::Affine const sans_translate = transform.withoutTranslation();
        double angle = std::atan2(sans_translate[1], sans_translate[0]) * (180 / M_PI);
        p += sp_svg_number_write_de(c + p, sizeof(c) - p, angle, prec, min_exp);
        c[p++] = ',';

        Geom::Point pt = transform.translation();
        p += sp_svg_number_write_de(c + p, sizeof(c) - p, pt[Geom::X], prec, min_exp);

        c[p++] = ',';

        p += sp_svg_number_write_de(c + p, sizeof(c) - p, pt[Geom::Y], prec, min_exp);

        c[p++] = ')';
        c[p] = '\000';*/
    } else if (transform.isHShear()) {
        // We are more or less a pure skewX
        strcpy(c + p, "skewX(");
        p += 6;

        double angle = atan(transform[2]) * (180 / M_PI);
        p += sp_svg_number_write_de(c + p, sizeof(c) - p, angle, prec, min_exp);

        c[p++] = ')';
        c[p] = '\000';
    } else if (transform.isVShear()) {
        // We are more or less a pure skewY
        strcpy(c + p, "skewY(");
        p += 6;

        double angle = atan(transform[1]) * (180 / M_PI);
        p += sp_svg_number_write_de(c + p, sizeof(c) - p, angle, prec, min_exp);

        c[p++] = ')';
        c[p] = '\000';
    } else {
        strcpy (c + p, "matrix(");
        p += 7;
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[0], prec, min_exp );
        c[p++] = ',';
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[1], prec, min_exp );
        c[p++] = ',';
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[2], prec, min_exp );
        c[p++] = ',';
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[3], prec, min_exp );
        c[p++] = ',';
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[4], prec, min_exp );
        c[p++] = ',';
        p += sp_svg_number_write_de( c + p, sizeof(c) - p, transform[5], prec, min_exp );
        c[p++] = ')';
        c[p] = '\000';
    }

    assert(p <= sizeof(c));
    return g_strdup(c);
}


gchar *
sp_svg_transform_write(Geom::Affine const *transform)
{
    return sp_svg_transform_write(*transform);
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

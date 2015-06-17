/**
 *  \file src/svg/svg-angle.cpp
 *  \brief SVG angle type
 */
/*
 * Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <math.h>
#include <glib.h>

#include "svg.h"
#include "stringstream.h"
#include "svg/svg-angle.h"
#include "util/units.h"


static bool sp_svg_angle_read_lff(gchar const *str, SVGAngle::Unit &unit, float &val, float &computed);

SVGAngle::SVGAngle()
    : _set(false)
    , unit(NONE)
    , value(0)
    , computed(0)
{
}

/* Angle */

bool SVGAngle::read(gchar const *str)
{
    if (!str) {
        return false;
    }

    SVGAngle::Unit u;
    float v;
    float c;
    if (!sp_svg_angle_read_lff(str, u, v, c)) {
        return false;
    }

    _set = true;
    unit = u;
    value = v;
    computed = c;

    return true;
}

void SVGAngle::unset(SVGAngle::Unit u, float v, float c) {
    _set = false;
    unit = u;
    value = v;
    computed = c;
}

void SVGAngle::readOrUnset(gchar const *str, Unit u, float v, float c) {
    if (!read(str)) {
        unset(u, v, c);
    }
}

static bool sp_svg_angle_read_lff(gchar const *str, SVGAngle::Unit &unit, float &val, float &computed)
{
    if (!str) {
        return false;
    }

    gchar const *e;
    float const v = g_ascii_strtod(str, (char **) &e);
    if (e == str) {
        return false;
    }

    if (!e[0]) {
        /* Unitless (defaults to degrees)*/
        unit = SVGAngle::NONE;
        val = v;
        computed = v;
        return true;
    } else if (!g_ascii_isalnum(e[0])) {
        if (g_ascii_isspace(e[0]) && e[1] && g_ascii_isalpha(e[1])) {
            return false; // spaces between value and unit are not allowed
        } else {
            /* Unitless (defaults to degrees)*/
            unit = SVGAngle::NONE;
            val = v;
            computed = v;
            return true;
        }
    } else {
        if (strncmp(e, "deg", 3) == 0) {
            unit = SVGAngle::DEG;
            val = v;
            computed = v;
        } else if (strncmp(e, "grad", 4) == 0) {
            unit = SVGAngle::GRAD;
            val = v;
            computed = Inkscape::Util::Quantity::convert(v, "grad", "°");
        } else if (strncmp(e, "rad", 3) == 0) {
            unit = SVGAngle::RAD;
            val = v;
            computed = Inkscape::Util::Quantity::convert(v, "rad", "°");
        } else if (strncmp(e, "turn", 4) == 0) {
            unit = SVGAngle::TURN;
            val = v;
            computed = Inkscape::Util::Quantity::convert(v, "turn", "°");
        } else {
            return false;
        }
        return true;
    }

    /* Invalid */
    return false;
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

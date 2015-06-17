#ifndef SEEN_SP_SVG_ANGLE_H
#define SEEN_SP_SVG_ANGLE_H

/**
 *  \file src/svg/svg-angle.h
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

#include <glib.h>

class SVGAngle
{
public:
    SVGAngle();

    enum Unit {
        NONE,
        DEG,
        GRAD,
        RAD,
        TURN,
        LAST_UNIT = TURN
    };

    // The object's value is valid / exists in SVG.
    bool _set;

    // The unit of value.
    Unit unit;

    // The value of this SVGAngle as found in the SVG.
    float value;

    // The value in degrees.
    float computed;

    float operator=(float v) {
        _set = true;
        unit = NONE;
        value = computed = v;
        return v;
    }

    bool read(gchar const *str);
    void unset(Unit u = NONE, float v = 0, float c = 0);
    void readOrUnset(gchar const *str, Unit u = NONE, float v = 0, float c = 0);
};

#endif // SEEN_SP_SVG_ANGLE_H

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

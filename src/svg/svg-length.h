/**
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SP_SVG_LENGTH_H
#define SEEN_SP_SVG_LENGTH_H

/**
 *  SVG length type
 */
class SVGLength {
public:
    SVGLength();

    enum Unit {
        NONE,
        PX,
        PT,
        PC,
        MM,
        CM,
        INCH,
        EM,
        EX,
        PERCENT,
        LAST_UNIT = PERCENT
    };

    // The object's value is valid / exists in SVG.
    bool _set;

    // The unit of value.
    Unit unit;

    // The value of this SVGLength as found in the SVG.
    float value;

    // The value in pixels (value * pixels/unit).
    float computed;

    float operator=(float v) {
        _set = true;
        unit = NONE;
        value = computed = v;
        return v;
    }

    bool read(char const *str);
    void readOrUnset(char const *str, Unit u = NONE, float v = 0, float c = 0);
    bool readAbsolute(char const *str);
    std::string write() const;
    // To set 'v' use '='
    void set(Unit u, float v); // Sets computed value based on u and v.
    void set(Unit u, float v, float c); // Sets all three values.
    void unset(Unit u = NONE, float v = 0, float c = 0);
    void scale(double scale); // Scales length (value, computed), leaving unit alone.
    void update(double em, double ex, double scale); // Updates computed value
};

#endif // SEEN_SP_SVG_LENGTH_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

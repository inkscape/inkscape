/*
    Authors:
      bulia byak <buliabyak@gmail.com>

    Copyright (C) 2004 Authors

    Released under GNU GPL, read the file 'COPYING' for more information
*/
#ifndef SEEN_COLOR_RGBA_H
#define SEEN_COLOR_RGBA_H

#include <cassert>
#include "decimal-round.h"
typedef unsigned int guint32;

/**
 * A class to contain a floating point RGBA color as one unit.
 */
class ColorRGBA {
public:

    /**
     * A constructor to create the color from four floating point values.
     * Load the values into the array of floats in this object.
     *
     * @param  c0  Red
     * @param  c1  Green
     * @param  c2  Blue
     * @param  c3  Alpha
     */
    ColorRGBA(float c0, float c1, float c2, float c3)
    {
        _c[0] = c0; _c[1] = c1;
        _c[2] = c2; _c[3] = c3;
    }

    /**
     * Create a quick ColorRGBA with all zeros.
     */
    ColorRGBA(void)
    {
        for (int i = 0; i < 4; i++)
            _c[i] = 0.0;
    }

    /**
     * A constructor to create the color from an unsigned int, as found everywhere when dealing with colors.
     *
     * Separate the values and load them into the array of floats in this object.
     * TODO : maybe get rid of the NR_RGBA32_x C-style functions and replace
     *  the calls with the bitshifting they do
     *
     * @param  intcolor   rgba32 "unsigned int representation (0xRRGGBBAA)
     */
    ColorRGBA(guint32 intcolor)
    {
         _c[0] = ((intcolor & 0xff000000) >> 24) / 255.0;
         _c[1] = ((intcolor & 0x00ff0000) >> 16) / 255.0;
         _c[2] = ((intcolor & 0x0000ff00) >>  8) / 255.0;
         _c[3] = ((intcolor & 0x000000ff) >>  0) / 255.0;

    }

    /**
     * Create a ColorRGBA using an array of floats.
     *
     * Go through each entry in the array and put it into \c _c.
     *
     * @param  in_array  The values to be placed into the object
     */
    ColorRGBA(float in_array[4])
    {
        for (int i = 0; i < 4; i++)
            _c[i] = in_array[i];
    }

    /**
     * Overwrite the values in this object with another \c ColorRGBA.
     * Copy all the values from \c m into \c this.
     *
     * @param  m  Values to use for the array.
     * @return This ColorRGBA object.
     */
    ColorRGBA &operator=(ColorRGBA const &m) {
        for (unsigned i = 0 ; i < 4 ; ++i) {
            _c[i] = m._c[i];
        }
        return *this;
    }

    /**
     * Grab a particular value from the ColorRGBA object.
     * First checks to make sure that the value is within the array,
     * and then return the value if it is.
     *
     * @param  i  Which value to grab.
     * @return The requested value.
     */
    float operator[](unsigned int const i) const {
        assert( unsigned(i) < 4 );
        return _c[i];
    }

    /**
     * Check to ensure that two \c ColorRGBA's are equal.
     * Check each value to see if they are equal.  If they all are,
     * return true.
     *
     * @param  other  The guy to check against.
     * @return Whether or not they are equal.
     */
    bool operator== (const ColorRGBA &other) const {
        for (int i = 0; i < 4; i++) {
            if (_c[i] != other[i])
                return false;
        }
        return true;
    }

    bool operator!=(ColorRGBA const &o) const {
        return !(*this == o);
    }

    /**
     * Average two \c ColorRGBAs to create another one.
     * This function goes through all the points in the two objects and
     * merges them together based on the weighting.  The current objects
     * value are multiplied by 1.0 - weight and the second object by weight.
     * This means that they should always be balanced by the parameter.
     *
     * @param  second  The second RGBA, with this being the first
     * @param  weight  How much of each should be used.  Zero is all
     * this while one is all the second.  Default is
     * half and half.
     */
    ColorRGBA average (const ColorRGBA &second, const float weight = 0.5) const {
        float returnval[4];

        for (int i = 0; i < 4; i++) {
            returnval[i] = _c[i] * (1.0 - weight) + second[i] * weight; 
        }

        return ColorRGBA(returnval[0], returnval[1], returnval[2], returnval[3]);
    }

    /**
     * Give the rgba32 "unsigned int" representation of the color.
     * round each components*255 and combine them (RRGGBBAA).
     * WARNING : this reduces color precision (from float to 0->255 int per component)
     * but it should be expected since we request this kind of output
     */
    unsigned int getIntValue() const {

         return   (int(Inkscape::decimal_round(_c[0]*255, 0)) << 24) | 
                        (int(Inkscape::decimal_round(_c[1]*255, 0))  << 16) | 
                        (int(Inkscape::decimal_round(_c[2]*255, 0))  << 8) | 
                        (int(Inkscape::decimal_round(_c[3]*255, 0)));
    }

private:
    /** Array of values that are stored. */
    float _c[4];
};


#endif // SEEN_COLOR_RGBA_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

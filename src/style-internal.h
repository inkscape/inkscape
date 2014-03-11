#ifndef SEEN_SP_STYLE_INTERNAL_H
#define SEEN_SP_STYLE_INTERNAL_H

/** \file
 * SPStyle internal: classes that are internal to SPStyle
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "color.h"
#include "sp-marker-loc.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "sp-paint-server-reference.h"
#include "uri.h"

#include <vector>

/// Float type internal to SPStyle.
struct SPIFloat {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned data : 30;
    float value;
};

/*
 * One might think that the best value for SP_SCALE24_MAX would be ((1<<24)-1), which allows the
 * greatest possible precision for fitting [0, 1] fractions into 24 bits.
 *
 * However, in practice, that gives a problem with 0.5, which falls half way between two fractions
 * of ((1<<24)-1).  What's worse is that casting double(1<<23) / ((1<<24)-1) to float on x86
 * produces wrong rounding behaviour, resulting in a fraction of ((1<<23)+2.0f) / (1<<24) rather
 * than ((1<<23)+1.0f) / (1<<24) as one would expect, let alone ((1<<23)+0.0f) / (1<<24) as one
 * would ideally like for this example.
 *
 * The value (1<<23) is thus best if one considers float conversions alone.
 *
 * The value 0xff0000 can exactly represent all 8-bit alpha channel values,
 * and can exactly represent all multiples of 0.1.  I haven't yet tested whether
 * rounding bugs still get in the way of conversions to & from float, but my instinct is that
 * it's fairly safe because 0xff fits three times inside float's significand.
 *
 * We should probably use the value 0xffff00 once we support 16 bits per channel and/or LittleCMS,
 * though that might need to be accompanied by greater use of double instead of float for
 * colours and opacities, to be safe from rounding bugs.
 */
static const unsigned SP_SCALE24_MAX = 0xff0000;
#define SP_SCALE24_TO_FLOAT(v) ((double) (v) / SP_SCALE24_MAX)
#define SP_SCALE24_FROM_FLOAT(v) unsigned(((v) * SP_SCALE24_MAX) + .5)

/** Returns a scale24 for the product of two scale24 values. */
#define SP_SCALE24_MUL(_v1, _v2) unsigned((double)(_v1) * (_v2) / SP_SCALE24_MAX + .5)

/// 24 bit data type internal to SPStyle.
struct SPIScale24 {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned value : 24;
};

/// Int type internal to SPStyle.
struct SPIInt {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned data : 30;
    int value;
};

/// Short type internal to SPStyle.
struct SPIShort {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned data : 14;
    int value : 16;
};

/// Enum type internal to SPStyle.
struct SPIEnum {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned value : 8;
    unsigned computed : 8;
};

/// String type internal to SPStyle.
struct SPIString {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned data : 30;
    gchar *value;
};

enum SPCSSUnit {
    SP_CSS_UNIT_NONE,
    SP_CSS_UNIT_PX,
    SP_CSS_UNIT_PT,
    SP_CSS_UNIT_PC,
    SP_CSS_UNIT_MM,
    SP_CSS_UNIT_CM,
    SP_CSS_UNIT_IN,
    SP_CSS_UNIT_EM,
    SP_CSS_UNIT_EX,
    SP_CSS_UNIT_PERCENT
};

/// Length type internal to SPStyle.
struct SPILength {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned unit : 4;
    float value;
    float computed;
};

#define SP_STYLE_FILL_SERVER(s) ((const_cast<SPStyle *> (s))->getFillPaintServer())
#define SP_STYLE_STROKE_SERVER(s) ((const_cast<SPStyle *> (s))->getStrokePaintServer())

/// Paint type internal to SPStyle.
struct SPIPaint {
    unsigned int set : 1; //c++ bitfields are used here as opposed to bools to reduce memory consumption, see http://tinyurl.com/cswh6mq
    unsigned int inherit : 1;
    unsigned int currentcolor : 1;
    unsigned int colorSet : 1;
    unsigned int noneSet : 1;
    struct {
         SPPaintServerReference *href;
         SPColor color;
    } value;

    SPIPaint();

    bool isSet() const { return true; /* set || colorSet*/}
    bool isSameType( SPIPaint const & other ) const {return (isPaintserver() == other.isPaintserver()) && (colorSet == other.colorSet) && (currentcolor == other.currentcolor);}

    bool isNoneSet() const {return noneSet;}

    bool isNone() const {return !currentcolor && !colorSet && !isPaintserver();} // TODO refine
    bool isColor() const {return colorSet && !isPaintserver();}
    bool isPaintserver() const {return (value.href) ? value.href->getObject():0;}

    void clear();

    void setColor( float r, float g, float b ) {value.color.set( r, g, b ); colorSet = true;}
    void setColor( guint32 val ) {value.color.set( val ); colorSet = true;}
    void setColor( SPColor const& color ) {value.color = color; colorSet = true;}

    void read( gchar const *str, SPStyle &tyle, SPDocument *document = 0);
};

class SPIDashArray {
 public:
    unsigned set : 1;
    unsigned inherit : 1;
    std::vector<double> values;
};

// SVG 2
enum SPPaintOrderLayer {
    SP_CSS_PAINT_ORDER_NORMAL,
    SP_CSS_PAINT_ORDER_FILL,
    SP_CSS_PAINT_ORDER_STROKE,
    SP_CSS_PAINT_ORDER_MARKER
};

const size_t PAINT_ORDER_LAYERS = 3;
struct SPIPaintOrder {
    unsigned set : 1;
    unsigned inherit : 1;
    SPPaintOrderLayer layer[PAINT_ORDER_LAYERS];
    bool layer_set[PAINT_ORDER_LAYERS];
    gchar *value;  // Raw string
};

/// Filter type internal to SPStyle
struct SPIFilter {
    unsigned set : 1;
    unsigned inherit : 1;
    SPFilterReference *href;
};

enum {
    SP_FONT_SIZE_LITERAL,
    SP_FONT_SIZE_LENGTH,
    SP_FONT_SIZE_PERCENTAGE
};

enum {
    SP_BASELINE_SHIFT_LITERAL,
    SP_BASELINE_SHIFT_LENGTH,
    SP_BASELINE_SHIFT_PERCENTAGE
};


#define SP_STYLE_FLAG_IFSET (1 << 0)
#define SP_STYLE_FLAG_IFDIFF (1 << 1)
#define SP_STYLE_FLAG_ALWAYS (1 << 2)

/// Fontsize type internal to SPStyle (also used by libnrtype/Layout-TNG-Input.cpp).
struct SPIFontSize {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned type : 2;
    unsigned unit : 4;
    unsigned literal: 4;
    float value;
    float computed;
};

/// Baseline shift type internal to SPStyle.
struct SPIBaselineShift {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned type : 2;
    unsigned unit : 4;
    unsigned literal: 2;
    float value; // Can be negative
    float computed;
};

// CSS 2.  Changes in CSS 3, where description is for TextDecorationLine, NOT TextDecoration
/// Text decoration type internal to SPStyle.
struct SPITextDecorationLine {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned underline : 1;
    unsigned overline : 1;
    unsigned line_through : 1;
    unsigned blink : 1;    // "Conforming user agents are not required to support this value." yay!
};

// CSS3 2.2
/// Text decoration style type internal to SPStyle.
struct SPITextDecorationStyle {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned solid : 1;
    unsigned isdouble : 1;  // cannot use "double" as it is a reserved keyword
    unsigned dotted : 1;
    unsigned dashed : 1;
    unsigned wavy : 1;
};

/// Extended length type internal to SPStyle.
struct SPILengthOrNormal {
    unsigned set : 1;
    unsigned inherit : 1;
    unsigned normal : 1;
    unsigned unit : 4;
    float value;
    float computed;
};

// These are used to implement text_decoration. The values are not saved to or read from SVG file
struct SPITextDecorationData {
    float   phase_length;          // length along text line,used for phase for dot/dash/wavy
    bool    tspan_line_start;      // is first  span on a line
    bool    tspan_line_end;        // is last span on a line
    float   tspan_width;           // from libnrtype, when it calculates spans
    float   ascender;              // the rest from tspan's font
    float   descender;
    float   line_gap;
    float   underline_thickness;
    float   underline_position; 
    float   line_through_thickness;
    float   line_through_position;
};

struct SPTextStyle;

/// An SPTextStyle has a refcount, a font family, and a font name.
struct SPTextStyle {
    int refcount;

    /* CSS font properties */
    SPIString font_family;

    /* Full font name, as font_factory::ConstructFontSpecification would give */
    SPIString font_specification;

    /** \todo fixme: The 'font' property is ugly, and not working (lauris) */
    SPIString font;
};

#endif // SEEN_SP_STYLE_INTERNAL_H


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

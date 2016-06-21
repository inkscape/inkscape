#ifndef SEEN_SP_STYLE_INTERNAL_H
#define SEEN_SP_STYLE_INTERNAL_H

/** \file
 * SPStyle internal: classes that are internal to SPStyle
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2014 Tavmjong Bah
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "style-enums.h"

#include "color.h"
#include "svg/svg-icc-color.h"
#include "sp-marker-loc.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "sp-paint-server-reference.h"
#include "uri.h"
#include "xml/repr.h"

#include <vector>

struct SPStyleEnum;

static const unsigned SP_STYLE_FLAG_ALWAYS (1 << 2);
static const unsigned SP_STYLE_FLAG_IFSET  (1 << 0);
static const unsigned SP_STYLE_FLAG_IFDIFF (1 << 1);

enum SPStyleSrc {
    SP_STYLE_SRC_UNSET,
    SP_STYLE_SRC_STYLE_PROP,
    SP_STYLE_SRC_STYLE_SHEET,
    SP_STYLE_SRC_ATTRIBUTE
};

/* General comments:
 *
 * This code is derived from the original C style code in style.cpp.
 *
 * Overview:
 *   Style can be obtained (in order of precidence) [CHECK]
 *     1. "style" property in an element (style="fill:red").
 *     2. Style sheet, internal or external (<style> rect {fill:red;}</style>). 
 *     3. Attributes in an element (fill="red").
 *     4. Parent's style.
 *   A later property overrides an earlier property. This is implemented by
 *   reading in the properties backwards. If a property is already set, it
 *   prevents an earlier property from being read.
 *
 *   In order for cascading to work, each element in the tree must be read in from top to bottom
 *   (parent before child). At each step, if a style property is not explicitly set, the property
 *   value is taken from the parent. Some properties have "computed" values that depend on:
 *      the parent's value (e.g. "font-size:larger"),
 *      another property value ("stroke-width":1em"), or
 *      an external value ("stroke-width:5%").
 *
 * To summarize:
 *
 *   An explicitly set value (including 'inherit') has a 'true' "set" flag.
 *   The "value" is either explicitly set or inherited.
 *   The "computed" value (if present) is calculated from "value" and some other input. 
 *
 * Functions:
 *   write():    Write a property and its value to a string.
 *     Flags:
 *       ALWAYS: Always write out property.
 *       IFSET:  Write a property if 'set' flag is true, otherwise return empty string.
 *       IFDIFF: Write a property if computed values are different, otherwise return empty string,
 *               This is only used for text!!
 *
 *   read():     Set a property value from a string.
 *   clear():    Set a property to its default value and set the 'set' flag to false.
 *   cascade():  Cascade the parent's property values to the child if the child's property
 *               is unset (and it allows inheriting) or the value is 'inherit'.
 *               Calculate computed values that depend on parent.
 *               This requires that the parent already be updated.
 *   merge():    Merge the property values of a child and a parent that is being deleted,
 *               attempting to preserve the style of the child.
 *   operator=:  Assignment operator required due to use of templates (in original C code).
 *   operator==: True if computed values are equal.  TO DO: DEFINE EXACTLY WHAT THIS MEANS
 *   operator!=: Inverse of operator==.
 *
 *
 * Outside dependencies:
 *
 *   The C structures that these classes are evolved from were designed to be embedded in to the
 *   style structure (i.e they are "internal" and thus have an "I" in the SPI prefix). However,
 *   they should be reasonably stand-alone and can provide some functionality outside of the style
 *   stucture (i.e. reading and writing style strings). Some properties do need access to other
 *   properties from the same object (e.g. SPILength sometimes needs to know font size) to
 *   calculate 'computed' values. Inheritence, of course, requires access to the parent object's
 *   style class.
 *
 *   The only real outside dependancy is SPObject... which is needed in the cases of SPIPaint and
 *   SPIFilter for setting up the "href". (Currently, SPDocument is needed but this dependency
 *   should be removed as an "href" only needs the SPDocument for attaching an external document to
 *   the XML tree [see uri-references.cpp]. If SPDocument is really needed, it can be obtained from
 *   SPObject.)
 *
 */

/// Virtual base class for all SPStyle interal classes
class SPIBase
{

public:
    SPIBase( Glib::ustring const &name, bool inherits = true )
        : name(name),
          inherits(inherits),
          set(false),
          inherit(false),
          style_src(SP_STYLE_SRC_UNSET),
          style(NULL)
    {}

    virtual ~SPIBase()
    {}

    virtual void read( gchar const *str ) = 0;
    virtual void readIfUnset( gchar const *str ) {
        if ( !set ) {
            read( str );
        }
    }

    virtual void readAttribute( Inkscape::XML::Node *repr ) {
        readIfUnset( repr->attribute( name.c_str() ) );
    }

    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const = 0;
    virtual void clear() {
        set = false, inherit = false;
    }

    virtual void cascade( const SPIBase* const parent ) = 0;
    virtual void merge(   const SPIBase* const parent ) = 0;

    virtual void setStylePointer( SPStyle *style_in  ) {
        style = style_in;
    }

    // Explicit assignment operator required due to templates.
    SPIBase& operator=(const SPIBase& rhs) {
        name        = rhs.name;
        inherits    = rhs.inherits;
        set         = rhs.set;
        inherit     = rhs.inherit;
        style_src   = rhs.style_src;
        style       = rhs.style;
        return *this;
    }

    // Check apples being compared to apples
    virtual bool operator==(const SPIBase& rhs) {
        return (name == rhs.name);
    }

    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    Glib::ustring name;       // Make const
    unsigned inherits : 1;    // Property inherits by default from parent.
    unsigned set : 1;         // Property has been explicitly set (vs. inherited).
    unsigned inherit : 1;     // Property value set to 'inherit'.
    SPStyleSrc style_src : 2; // Source (attribute, style attribute, style-sheet). NOT USED YET FIX ME

  // To do: make private after g_asserts removed
public:
    SPStyle* style;       // Used by SPIPaint, SPIFilter... to find values of other properties
};

/// Float type internal to SPStyle. (Only 'stroke-miterlimit')
class SPIFloat : public SPIBase
{

public:
    SPIFloat()
        : SPIBase( "anonymous_float" ),
          value(0.0)
    {}

    SPIFloat( Glib::ustring const &name, float value_default  = 0.0 )
        : SPIBase( name ),
          value(value_default),
          value_default(value_default)
    {}

    virtual ~SPIFloat() {}
    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        value = value_default;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIFloat& operator=(const SPIFloat& rhs) {
        SPIBase::operator=(rhs);
        value         = rhs.value;
        value_default = rhs.value_default;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    float value;

private:
    float value_default;
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
// Used only for opacity, fill-opacity, stroke-opacity.
// Opacity does not inherit but stroke-opacity and fill-opacity do. 
class SPIScale24 : public SPIBase
{

public:
    SPIScale24()
        : SPIBase( "anonymous_scale24" ),
          value(0)
    {}

    SPIScale24( Glib::ustring const &name, unsigned value = 0, bool inherits = true )
        : SPIBase( name, inherits ),
          value(value),
          value_default(value)
    {}

    virtual ~SPIScale24()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        value = value_default;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIScale24& operator=(const SPIScale24& rhs) {
        SPIBase::operator=(rhs);
        value         = rhs.value;
        value_default = rhs.value_default;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }


  // To do: make private
public:
    unsigned value : 24;

private:
    unsigned value_default : 24;
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
// Needs access to 'font-size' and 'font-family' for computed values.
// Used for 'stroke-width' 'stroke-dash-offset' ('none' not handled), text-indent
class SPILength : public SPIBase
{

public:
    SPILength()
        : SPIBase( "anonymous_length" ),
          unit(SP_CSS_UNIT_NONE),
          value(0),
          computed(0)
    {}

    SPILength( Glib::ustring const &name, float value = 0 )
        : SPIBase( name ),
          unit(SP_CSS_UNIT_NONE),
          value(value),
          computed(value),
          value_default(value)
    {}

    virtual ~SPILength()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        unit = SP_CSS_UNIT_NONE, value = value_default;
        computed = value_default;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPILength& operator=(const SPILength& rhs) {
        SPIBase::operator=(rhs);
        unit          = rhs.unit;
        value         = rhs.value;
        computed      = rhs.computed;
        value_default = rhs.value_default;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    unsigned unit : 4;
    float value;
    float computed;

private:
    float value_default;
};


/// Extended length type internal to SPStyle.
// Used for: line-height, letter-spacing, word-spacing
class SPILengthOrNormal : public SPILength
{

public:
    SPILengthOrNormal()
        : SPILength( "anonymous_length" ),
          normal(true)
    {}

    SPILengthOrNormal( Glib::ustring const &name, float value = 0 )
        : SPILength( name, value ),
          normal(true)
    {}

    virtual ~SPILengthOrNormal()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPILength::clear();
        normal = true;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPILengthOrNormal& operator=(const SPILengthOrNormal& rhs) {
        SPILength::operator=(rhs);
        normal = rhs.normal;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    bool normal : 1;
};


/// Enum type internal to SPStyle.
// Used for many properties. 'font-stretch' and 'font-weight' must be special cased.
class SPIEnum : public SPIBase
{

public:
    SPIEnum() :
        SPIBase( "anonymous_enum" ),
        enums( NULL ),
        value(0),
        computed(0)
    {}

    SPIEnum( Glib::ustring const &name, SPStyleEnum const *enums, unsigned value = 0, bool inherits = true ) :
        SPIBase( name, inherits ),
        enums( enums ),
        value(value),
        computed(value),
        value_default(value),
        computed_default(value)
    {}

    // Following is needed for font-weight
    SPIEnum( Glib::ustring const &name, SPStyleEnum const *enums, SPCSSFontWeight value, SPCSSFontWeight computed ) :
        SPIBase( name ),
        enums( enums ),
        value(value),
        computed(computed),
        value_default(value),
        computed_default(computed)
    {}

    virtual ~SPIEnum()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        value = value_default, computed = computed_default;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIEnum& operator=(const SPIEnum& rhs) {
        SPIBase::operator=(rhs);
        value            = rhs.value;
        computed         = rhs.computed;
        value_default    = rhs.value_default;
        computed_default = rhs.computed_default;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    SPStyleEnum const *enums;

    unsigned value : 16;  // 9 bits required for 'font-variant-east-asian'
    unsigned computed: 16;

private:
    unsigned value_default : 16;
    unsigned computed_default: 16; // for font-weight
};


/// SPIEnum w/ bits, allows values with multiple key words.
class SPIEnumBits : public SPIEnum
{

public:
    SPIEnumBits() :
        SPIEnum( "anonymous_enumbits", NULL )
    {}

    SPIEnumBits( Glib::ustring const &name, SPStyleEnum const *enums, unsigned value = 0, bool inherits = true ) :
        SPIEnum( name, enums, value, inherits )
    {}

    virtual ~SPIEnumBits()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;

};


/// SPIEnum w/ extra bits. The 'font-variants-ligatures' property is a complete mess that needs
/// special handling. For OpenType fonts the values 'common-ligatures', 'contextual',
/// 'no-discretionary-ligatures', and 'no-historical-ligatures' are not useful but we still must be
/// able to parse them.
class SPILigatures : public SPIEnum
{

public:
    SPILigatures() :
        SPIEnum( "anonymous_enumligatures", NULL )
    {}

    SPILigatures( Glib::ustring const &name, SPStyleEnum const *enums) :
        SPIEnum( name, enums, SP_CSS_FONT_VARIANT_LIGATURES_NORMAL )
    {}

    virtual ~SPILigatures()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
};


/// SPIEnum w/ extra bits. The 'font-variants-numeric' property is a complete mess that needs
/// special handling. Multiple key words can be specified, some exclusive of others.
class SPINumeric : public SPIEnum
{

public:
    SPINumeric() :
        SPIEnum( "anonymous_enumnumeric", NULL )
    {}

    SPINumeric( Glib::ustring const &name, SPStyleEnum const *enums) :
        SPIEnum( name, enums, SP_CSS_FONT_VARIANT_NUMERIC_NORMAL )
    {}

    virtual ~SPINumeric()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
};


/// String type internal to SPStyle.
// Used for 'marker', ..., 'font', 'font-family', 'inkscape-font-specification'
class SPIString : public SPIBase
{

public:
    SPIString()
        : SPIBase( "anonymous_string" ),
          value(NULL)
    {}

    // TODO probably want to avoid gchar* and c-style strings.
    SPIString( Glib::ustring const &name, gchar const* value_default_in = NULL )
        : SPIBase( name ),
          value(NULL),
          value_default(value_default_in ? g_strdup(value_default_in) : NULL)
    {}

    virtual ~SPIString() {
        g_free(value);
        g_free(value_default);
    }

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear(); // TODO check about value and value_default
    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIString& operator=(const SPIString& rhs) {
        SPIBase::operator=(rhs);
        g_free(value);
        g_free(value_default);
        value            = rhs.value ? g_strdup(rhs.value) : NULL;
        value_default    = rhs.value_default ? g_strdup(rhs.value_default) : NULL;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private, convert value to Glib::ustring
public:
    gchar *value;
    gchar *value_default;
};

/// Color type interal to SPStyle, FIXME Add string value to store SVG named color.
class SPIColor : public SPIBase
{

public:
    SPIColor()
        : SPIBase( "anonymous_color" ),
          currentcolor(false) {
        value.color.set(0);
    }

    SPIColor( Glib::ustring const &name )
        : SPIBase( name ),
          currentcolor(false) {
        value.color.set(0);
    }

    virtual ~SPIColor()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        value.color.set(0);
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIColor& operator=(const SPIColor& rhs) {
        SPIBase::operator=(rhs);
        currentcolor = rhs.currentcolor;
        value.color  = rhs.value.color;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

    void setColor( float r, float g, float b ) {
        value.color.set( r, g, b );
    }

    void setColor( guint32 val ) {
        value.color.set( val );
    }

    void setColor( SPColor const& color ) {
        value.color = color;
    }

public:
    bool currentcolor : 1;
    // FIXME: remove structure and derive SPIPaint from this class.
    struct {
         SPColor color;
    } value;
};



#define SP_STYLE_FILL_SERVER(s) ((const_cast<SPStyle *> (s))->getFillPaintServer())
#define SP_STYLE_STROKE_SERVER(s) ((const_cast<SPStyle *> (s))->getStrokePaintServer())

// SVG 2
enum SPPaintOrigin {
    SP_CSS_PAINT_ORIGIN_NORMAL,
    SP_CSS_PAINT_ORIGIN_CURRENT_COLOR,
    SP_CSS_PAINT_ORIGIN_CONTEXT_FILL,
    SP_CSS_PAINT_ORIGIN_CONTEXT_STROKE
};


/// Paint type internal to SPStyle.
class SPIPaint : public SPIBase
{

public:
    SPIPaint()
        : SPIBase( "anonymous_paint" ),
          paintOrigin( SP_CSS_PAINT_ORIGIN_NORMAL ),
          colorSet(false),
          noneSet(false) {
        value.href = NULL;
        clear();
    }

    SPIPaint( Glib::ustring const &name )
        : SPIBase( name ),
          colorSet(false),
          noneSet(false) {
        value.href = NULL;
        clear();  // Sets defaults
    }

    virtual ~SPIPaint();  // Clear and delete href.
    virtual void read( gchar const *str );
    virtual void read( gchar const *str, SPStyle &style, SPDocument *document = 0);
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear();
    virtual void reset( bool init ); // Used internally when reading or cascading
    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIPaint& operator=(const SPIPaint& rhs) {
        SPIBase::operator=(rhs);
        paintOrigin     = rhs.paintOrigin;
        colorSet        = rhs.colorSet;
        noneSet         = rhs.noneSet;
        value.color     = rhs.value.color;
        value.href      = rhs.value.href;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

    bool isSameType( SPIPaint const & other ) const {
        return (isPaintserver() == other.isPaintserver()) && (colorSet == other.colorSet) && (paintOrigin == other.paintOrigin);
    }

    bool isNoneSet() const {
        return noneSet;
    }

    bool isNone() const {
        return (paintOrigin == SP_CSS_PAINT_ORIGIN_NORMAL) && !colorSet && !isPaintserver();
    } // TODO refine

    bool isColor() const {
        return colorSet && !isPaintserver();
    }

    bool isPaintserver() const {
        return (value.href) ? value.href->getObject() : 0;
    }

    void setColor( float r, float g, float b ) {
        value.color.set( r, g, b ); colorSet = true;
    }

    void setColor( guint32 val ) {
        value.color.set( val ); colorSet = true;
    }

    void setColor( SPColor const& color ) {
        value.color = color; colorSet = true;
    }

    void setNone() {noneSet = true; colorSet=false;}

  // To do: make private
public:
    SPPaintOrigin paintOrigin : 2;
    bool colorSet : 1;
    bool noneSet : 1;
    struct {
         SPPaintServerReference *href;
         SPColor color;
    } value;
};


// SVG 2
enum SPPaintOrderLayer {
    SP_CSS_PAINT_ORDER_NORMAL,
    SP_CSS_PAINT_ORDER_FILL,
    SP_CSS_PAINT_ORDER_STROKE,
    SP_CSS_PAINT_ORDER_MARKER
};

// Normal maybe should be moved out as is done in other classes.
// This could be replaced by a generic enum class where multiple keywords are allowed and
// where order matters (in contrast to 'text-decoration-line' where order does not matter).

// Each layer represents a layer of paint which can be a fill, a stroke, or markers.
const size_t PAINT_ORDER_LAYERS = 3;

/// Paint order type internal to SPStyle
class SPIPaintOrder : public SPIBase
{

public:
    SPIPaintOrder()
        : SPIBase( "paint-order" ),
          value(NULL) {
        this->clear();
    }

    virtual ~SPIPaintOrder() {
        g_free( value );
    }

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        for( unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i ) {
            layer[i]     = SP_CSS_PAINT_ORDER_NORMAL;
            layer_set[i] = false;
        }
        g_free(value);
        value = NULL;
    }
    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIPaintOrder& operator=(const SPIPaintOrder& rhs) {
        SPIBase::operator=(rhs);
        for( unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i ) {
            layer[i]     = rhs.layer[i];
            layer_set[i] = rhs.layer_set[i];
        }
        g_free(value);
        value            = g_strdup(rhs.value);
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }


  // To do: make private
public:
    SPPaintOrderLayer layer[PAINT_ORDER_LAYERS];
    bool layer_set[PAINT_ORDER_LAYERS];
    gchar *value;  // Raw string
};


/// Filter type internal to SPStyle
class SPIDashArray : public SPIBase
{

public:
    SPIDashArray()
        : SPIBase( "stroke-dasharray" )
    {}  // Only one instance of SPIDashArray

    virtual ~SPIDashArray()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        values.clear();
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIDashArray& operator=(const SPIDashArray& rhs) {
        SPIBase::operator=(rhs);
        values = rhs.values;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }


  // To do: make private, change double to SVGLength
public:
    std::vector<double> values;
};

/// Filter type internal to SPStyle
class SPIFilter : public SPIBase
{

public:
    SPIFilter()
        : SPIBase( "filter", false ),
          href(NULL)
    {}

    virtual ~SPIFilter();
    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear();
    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIFilter& operator=(const SPIFilter& rhs) {
        SPIBase::operator=(rhs);
        href = rhs.href;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    SPFilterReference *href;
};



enum {
    SP_FONT_SIZE_LITERAL,
    SP_FONT_SIZE_LENGTH,
    SP_FONT_SIZE_PERCENTAGE
};

/// Fontsize type internal to SPStyle (also used by libnrtype/Layout-TNG-Input.cpp).
class SPIFontSize : public SPIBase
{

public:
    SPIFontSize()
        : SPIBase( "font-size" ) {
        this->clear();
    }

    virtual ~SPIFontSize()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        type = SP_FONT_SIZE_LITERAL, unit = SP_CSS_UNIT_NONE,
            literal = SP_CSS_FONT_SIZE_MEDIUM, value = 12.0, computed = 12.0;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIFontSize& operator=(const SPIFontSize& rhs) {
        SPIBase::operator=(rhs);
        type      = rhs.type;
        unit      = rhs.unit;
        literal   = rhs.literal;
        value     = rhs.value;
        computed  = rhs.computed;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

public:
    static float const font_size_default;

  // To do: make private
public:
    unsigned type : 2;
    unsigned unit : 4;
    unsigned literal : 4;
    float value;
    float computed;

private:
    double relative_fraction() const;
    static float const font_size_table[];
};


/// Font type internal to SPStyle ('font' shorthand)
class SPIFont : public SPIBase
{

public:
    SPIFont()
        : SPIBase( "font" )
    {}

    virtual ~SPIFont()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
    }

    virtual void cascade( const SPIBase* const /*parent*/ )
    {} // Done in dependent properties

    virtual void merge(   const SPIBase* const /*parent*/ )
    {}

    SPIFont& operator=(const SPIFont& rhs) {
        SPIBase::operator=(rhs);
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }
};


enum {
    SP_BASELINE_SHIFT_LITERAL,
    SP_BASELINE_SHIFT_LENGTH,
    SP_BASELINE_SHIFT_PERCENTAGE
};

/// Baseline shift type internal to SPStyle. (This is actually just like SPIFontSize)
class SPIBaselineShift : public SPIBase
{

public:
    SPIBaselineShift()
        : SPIBase( "baseline-shift", false ) {
        this->clear();
    }

    virtual ~SPIBaselineShift()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        type=SP_BASELINE_SHIFT_LITERAL, unit=SP_CSS_UNIT_NONE,
            literal = SP_CSS_BASELINE_SHIFT_BASELINE, value = 0.0, computed = 0.0;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPIBaselineShift& operator=(const SPIBaselineShift& rhs) {
        SPIBase::operator=(rhs);
        type      = rhs.type;
        unit      = rhs.unit;
        literal   = rhs.literal;
        value     = rhs.value;
        computed  = rhs.computed;
        return *this;
    }

    // This is not used but we have it for completeness, it has not been tested.
    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

    bool isZero() const;

  // To do: make private
public:
    unsigned type : 2;
    unsigned unit : 4;
    unsigned literal: 2;
    float value; // Can be negative
    float computed;
};

// CSS 2.  Changes in CSS 3, where description is for TextDecorationLine, NOT TextDecoration
// See http://www.w3.org/TR/css-text-decor-3/

// CSS3 2.2
/// Text decoration line type internal to SPStyle.  THIS SHOULD BE A GENERIC CLASS
class SPITextDecorationLine : public SPIBase
{

public:
    SPITextDecorationLine()
        : SPIBase( "text-decoration-line" ) {
        this->clear();
    }

    virtual ~SPITextDecorationLine()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        underline = false, overline = false, line_through = false, blink = false;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPITextDecorationLine& operator=(const SPITextDecorationLine& rhs) {
        SPIBase::operator=(rhs);
        underline     = rhs.underline;
        overline      = rhs.overline;
        line_through  = rhs.line_through;
        blink         = rhs.blink;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    bool underline : 1;
    bool overline : 1;
    bool line_through : 1;
    bool blink : 1;    // "Conforming user agents are not required to support this value." yay!
};

// CSS3 2.2
/// Text decoration style type internal to SPStyle.  THIS SHOULD JUST BE SPIEnum!
class SPITextDecorationStyle : public SPIBase
{

public:
    SPITextDecorationStyle()
        : SPIBase( "text-decoration-style" ) {
        this->clear();
    }

    virtual ~SPITextDecorationStyle()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        solid = true, isdouble = false, dotted = false, dashed = false, wavy = false;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPITextDecorationStyle& operator=(const SPITextDecorationStyle& rhs) {
        SPIBase::operator=(rhs);
        solid     = rhs.solid;
        isdouble  = rhs.isdouble;
        dotted    = rhs.dotted;
        dashed    = rhs.dashed;
        wavy      = rhs.wavy;
        return *this;
    }

    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

  // To do: make private
public:
    bool solid : 1;
    bool isdouble : 1;  // cannot use "double" as it is a reserved keyword
    bool dotted : 1;
    bool dashed : 1;
    bool wavy : 1;
};



// This class reads in both CSS2 and CSS3 'text-decoration' property. It passes the line, style,
// and color parts to the appropriate CSS3 long-hand classes for reading and storing values.  When
// writing out data, we write all four properties, with 'text-decoration' being written out with
// the CSS2 format. This allows CSS1/CSS2 renderers to at least render lines, even if they are not
// the right style. (See http://www.w3.org/TR/css-text-decor-3/#text-decoration-property )

/// Text decoration type internal to SPStyle.
class SPITextDecoration : public SPIBase
{

public:
    SPITextDecoration()
        : SPIBase( "text-decoration" ),
          style_td( NULL )
    {}

    virtual ~SPITextDecoration()
    {}

    virtual void read( gchar const *str );
    virtual const Glib::ustring write( guint const flags = SP_STYLE_FLAG_IFSET,
                                       SPIBase const *const base = NULL ) const;
    virtual void clear() {
        SPIBase::clear();
        style_td = NULL;
    }

    virtual void cascade( const SPIBase* const parent );
    virtual void merge(   const SPIBase* const parent );

    SPITextDecoration& operator=(const SPITextDecoration& rhs) {
        SPIBase::operator=(rhs);
        return *this;
    }

    // Use CSS2 value
    virtual bool operator==(const SPIBase& rhs);
    virtual bool operator!=(const SPIBase& rhs) {
        return !(*this == rhs);
    }

public:
    SPStyle* style_td;   // Style to be used for drawing CSS2 text decorations 
};


// These are used to implement text_decoration. The values are not saved to or read from SVG file
struct SPITextDecorationData {
    float   phase_length;          // length along text line,used for phase for dot/dash/wavy
    bool    tspan_line_start;      // is first  span on a line
    bool    tspan_line_end;        // is last span on a line
    float   tspan_width;           // from libnrtype, when it calculates spans
    float   ascender;              // the rest from tspan's font
    float   descender;
    float   underline_thickness;
    float   underline_position; 
    float   line_through_thickness;
    float   line_through_position;
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

/**
 * @file
 * SVG stylesheets implementation.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2005 Monash University
 * Copyright (C) 2012 Kris De Gussem
 * Copyright (C) 2014 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <algorithm>

#include "libcroco/cr-sel-eng.h"
#include "xml/croco-node-iface.h"

#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/svg-icc-color.h"

#include "display/canvas-bpath.h"
#include "attributes.h"
#include "document.h"
#include "extract-uri.h"
#include "uri-references.h"
#include "uri.h"
#include "sp-paint-server.h"
#include "streq.h"
#include "strneq.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "xml/repr.h"
#include "xml/simple-document.h"
#include "util/units.h"
#include "macros.h"
#include "preferences.h"

#include "sp-filter-reference.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include <2geom/math-utils.h>

#include <glibmm/regex.h>

using Inkscape::CSSOStringStream;
using std::vector;

#define BMAX 8192

#define SP_CSS_FONT_SIZE_DEFAULT 12.0;

struct SPStyleEnum;

int SPStyle::_count = 0;

/*#########################
## FORWARD DECLARATIONS
#########################*/
void sp_style_filter_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style);
void sp_style_fill_paint_server_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style);
void sp_style_stroke_paint_server_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style);

static void sp_style_object_release(SPObject *object, SPStyle *style);
static CRSelEng *sp_repr_sel_eng();


//SPPropMap SPStyle::_propmap;

// C++11 allows one constructor to call another... might be useful. The original C code
// had separate calls to create SPStyle, one with only SPDocument and the other with only
// SPObject as parameters.
SPStyle::SPStyle(SPDocument *document_in, SPObject *object_in) :

    // Unimplemented SVG 1.1: alignment-baseline, clip, clip-path, color-profile, cursor,
    // dominant-baseline, flood-color, flood-opacity, font-size-adjust,
    // glyph-orientation-horizontal, glyph-orientation-vertical, kerning, lighting-color,
    // pointer-events, stop-color, stop-opacity, unicode-bidi

    // For enums:   property( name, enumeration, default value , inherits = true );
    // For scale24: property( name, default value = 0, inherits = true );

    // 'font', 'font-size', and 'font-family' must come first as other properties depend on them
    // for calculated values (through 'em' and 'ex'). ('ex' is currently not read.)
    // The following properties can depend on 'em' and 'ex':
    //   baseline-shift, kerning, letter-spacing, stroke-dash-offset, stroke-width, word-spacing,
    //   Non-SVG 1.1: text-indent, line-spacing

    // Hidden in SPIFontStyle:  (to be refactored)
    //   font-family
    //   font-specification

    // Font related properties and 'font' shorthand
    font_style(       "font-style",      enum_font_style,      SP_CSS_FONT_STYLE_NORMAL   ),
    font_variant(     "font-variant",    enum_font_variant,    SP_CSS_FONT_VARIANT_NORMAL ),
    font_weight(      "font-weight",     enum_font_weight,     SP_CSS_FONT_WEIGHT_NORMAL, SP_CSS_FONT_WEIGHT_400  ),
    font_stretch(     "font-stretch",    enum_font_stretch,    SP_CSS_FONT_STRETCH_NORMAL ), 
    font_size(),
    line_height(      "line-height",                     1.0 ),  // SPILengthOrNormal
    font_family(      "font-family",     "sans-serif"        ),  // SPIString w/default
    font(),                                                      // SPIFont
    font_specification( "-inkscape-font-specification"       ),  // SPIString

    // Text related properties
    text_indent(      "text-indent",                     0.0 ),  // SPILength
    text_align(       "text-align",      enum_text_align,      SP_CSS_TEXT_ALIGN_START    ),

    letter_spacing(   "letter-spacing",                  0.0 ),  // SPILengthOrNormal
    word_spacing(     "word-spacing",                    0.0 ),  // SPILengthOrNormal
    text_transform(   "text-transform",  enum_text_transform,  SP_CSS_TEXT_TRANSFORM_NONE ),

    direction(        "direction",       enum_direction,       SP_CSS_DIRECTION_LTR       ),
    block_progression("block-progression", enum_block_progression, SP_CSS_BLOCK_PROGRESSION_TB),
    writing_mode(     "writing-mode",    enum_writing_mode,    SP_CSS_WRITING_MODE_LR_TB  ),
    baseline_shift(),
    text_anchor(      "text-anchor",     enum_text_anchor,     SP_CSS_TEXT_ANCHOR_START   ),
    white_space(      "white-space",     enum_white_space,     SP_CSS_WHITE_SPACE_NORMAL  ),

    text_decoration(),
    text_decoration_line(),
    text_decoration_style(),
    text_decoration_color( "text-decoration-color" ),            // SPIColor

    // General visual properties
    clip_rule(        "clip-rule",       enum_clip_rule,       SP_WIND_RULE_NONZERO       ),
    display(          "display",         enum_display,         SP_CSS_DISPLAY_INLINE,   false ),
    overflow(         "overflow",        enum_overflow,        SP_CSS_OVERFLOW_VISIBLE, false ),
    visibility(       "visibility",      enum_visibility,      SP_CSS_VISIBILITY_VISIBLE  ),
    opacity(          "opacity",                               SP_SCALE24_MAX,          false ),

    isolation(        "isolation",       enum_isolation,       SP_CSS_ISOLATION_AUTO      ),
    mix_blend_mode(   "mix-blend-mode",  enum_blend_mode,      SP_CSS_BLEND_NORMAL        ),

    paint_order(), // SPIPaintOrder

    // Color properties
    color(            "color"                                ),  // SPIColor
    color_interpolation(        "color-interpolation",         enum_color_interpolation, SP_CSS_COLOR_INTERPOLATION_SRGB),
    color_interpolation_filters("color-interpolation-filters", enum_color_interpolation, SP_CSS_COLOR_INTERPOLATION_LINEARRGB),

    // Solid color properties
    solid_color(      "solid-color"                          ), // SPIColor
    solid_opacity(    "solid-opacity",                         SP_SCALE24_MAX             ),

    // Fill properties
    fill(             "fill"                                 ),  // SPIPaint
    fill_opacity(     "fill-opacity",                          SP_SCALE24_MAX             ),
    fill_rule(        "fill-rule",       enum_fill_rule,       SP_WIND_RULE_NONZERO       ),

    // Stroke properites
    stroke(           "stroke"                               ),  // SPIPaint
    stroke_width(     "stroke-width",      1.0               ),  // SPILength
    stroke_linecap(   "stroke-linecap",  enum_stroke_linecap,  SP_STROKE_LINECAP_BUTT     ),
    stroke_linejoin(  "stroke-linejoin", enum_stroke_linejoin, SP_STROKE_LINEJOIN_MITER   ),
    stroke_miterlimit("stroke-miterlimit",   4               ),  // SPIFloat (only use of float!)
    stroke_dasharray(),                                          // SPIDashArray
    stroke_dashoffset("stroke-dashoffset", 0.0               ),  // SPILength for now

    stroke_opacity(   "stroke-opacity",    SP_SCALE24_MAX    ),

    marker(           "marker"                               ),  // SPIString
    marker_start(     "marker-start"                         ),  // SPIString
    marker_mid(       "marker-mid"                           ),  // SPIString
    marker_end(       "marker-end"                           ),  // SPIString

    // Filter properties
    filter(),
    filter_blend_mode("filter-blend-mode", enum_blend_mode,    SP_CSS_BLEND_NORMAL),
    filter_gaussianBlur_deviation( "filter-gaussianBlur-deviation", 0.0 ), // SPILength
    enable_background("enable-background", enum_enable_background, SP_CSS_BACKGROUND_ACCUMULATE, false),

    // Rendering hint properties
    color_rendering(  "color-rendering", enum_color_rendering, SP_CSS_COLOR_RENDERING_AUTO),
    image_rendering(  "image-rendering", enum_image_rendering, SP_CSS_IMAGE_RENDERING_AUTO),
    shape_rendering(  "shape-rendering", enum_shape_rendering, SP_CSS_SHAPE_RENDERING_AUTO),
    text_rendering(    "text-rendering", enum_text_rendering,  SP_CSS_TEXT_RENDERING_AUTO )
{
    // std::cout << "SPStyle::SPStyle( SPDocument ): Entrance: (" << _count << ")" << std::endl;
    // std::cout << "                      Document: " << (document_in?"present":"null") << std::endl;
    // std::cout << "                        Object: "
    //           << (object_in?(object_in->getId()?object_in->getId():"id null"):"object null") << std::endl;

    // static bool first = true;
    // if( first ) {
    //     std::cout << "Size of SPStyle: " << sizeof(SPStyle) << std::endl;
    //     std::cout << "        SPIBase: " << sizeof(SPIBase) << std::endl;
    //     std::cout << "       SPIFloat: " << sizeof(SPIFloat) << std::endl;
    //     std::cout << "     SPIScale24: " << sizeof(SPIScale24) << std::endl;
    //     std::cout << "      SPILength: " << sizeof(SPILength) << std::endl;
    //     std::cout << "  SPILengthOrNormal: " << sizeof(SPILengthOrNormal) << std::endl;
    //     std::cout << "       SPIColor: " << sizeof(SPIColor) << std::endl;
    //     std::cout << "       SPIPaint: " << sizeof(SPIPaint) << std::endl;
    //     std::cout << "  SPITextDecorationLine" << sizeof(SPITextDecorationLine) << std::endl;
    //     std::cout << "   Glib::ustring:" << sizeof(Glib::ustring) << std::endl;
    //     std::cout << "        SPColor: " << sizeof(SPColor) << std::endl;
    //     first = false;
    // }

    ++_count; // Poor man's memory leak detector

    _refcount = 1;

    cloned = false;

    object = object_in;
    if( object ) {
        g_assert( SP_IS_OBJECT(object) );
        document = object->document;
        release_connection =
            object->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_style_object_release), this));

        cloned = object->cloned;

    } else {
        document = document_in;
    }

    // 'font' shorthand requires access to included properties.
    font.setStylePointer(              this );

    // Properties that depend on 'font-size' for calculating lengths.
    baseline_shift.setStylePointer(    this );
    text_indent.setStylePointer(       this );
    line_height.setStylePointer(       this );
    letter_spacing.setStylePointer(    this );
    word_spacing.setStylePointer(      this );
    stroke_width.setStylePointer(      this );
    stroke_dashoffset.setStylePointer( this );

    // Properties that depend on 'color'
    text_decoration_color.setStylePointer( this );
    fill.setStylePointer(                  this );
    stroke.setStylePointer(                this );
    // color.setStylePointer( this ); // Doen't need reference to self

    // 'text_decoration' shorthand requires access to included properties.
    text_decoration.setStylePointer( this );

    // SPIPaint, SPIFilter needs access to 'this' (SPStyle)
    // for setting up signals...  'fill', 'stroke' already done
    filter.setStylePointer( this );

    // Used to iterate over markers
    marker_ptrs[SP_MARKER_LOC]       = &marker;
    marker_ptrs[SP_MARKER_LOC_START] = &marker_start;
    marker_ptrs[SP_MARKER_LOC_MID]   = &marker_mid;
    marker_ptrs[SP_MARKER_LOC_END]   = &marker_end;


    // This might be too resource hungary... but for now it possible to loop over properties

    // 'color' must be before 'fill', 'stroke', 'text-decoration-color', ...
    _properties.push_back( &color );

    // 'font-size'/'font' must be before properties that need to know em, ex size (SPILength,
    // SPILenghtOrNormal)
    _properties.push_back( &font_style );
    _properties.push_back( &font_variant );
    _properties.push_back( &font_weight );
    _properties.push_back( &font_stretch );
    _properties.push_back( &font_size ); 
    _properties.push_back( &line_height );
    _properties.push_back( &font_family );
    _properties.push_back( &font );
    _properties.push_back( &font_specification );

    _properties.push_back( &text_indent );
    _properties.push_back( &text_align );

    _properties.push_back( &text_decoration );
    _properties.push_back( &text_decoration_line );
    _properties.push_back( &text_decoration_style );
    _properties.push_back( &text_decoration_color );

    _properties.push_back( &letter_spacing );
    _properties.push_back( &word_spacing );
    _properties.push_back( &text_transform );

    _properties.push_back( &direction );
    _properties.push_back( &block_progression );
    _properties.push_back( &writing_mode );
    _properties.push_back( &baseline_shift );
    _properties.push_back( &text_anchor );
    _properties.push_back( &white_space );

    _properties.push_back( &clip_rule );
    _properties.push_back( &display );
    _properties.push_back( &overflow );
    _properties.push_back( &visibility );
    _properties.push_back( &opacity );

    _properties.push_back( &isolation );
    _properties.push_back( &mix_blend_mode );

    _properties.push_back( &color_interpolation );
    _properties.push_back( &color_interpolation_filters );

    _properties.push_back( &solid_color );
    _properties.push_back( &solid_opacity );

    _properties.push_back( &fill );
    _properties.push_back( &fill_opacity );
    _properties.push_back( &fill_rule );

    _properties.push_back( &stroke );
    _properties.push_back( &stroke_width );
    _properties.push_back( &stroke_linecap );
    _properties.push_back( &stroke_linejoin );
    _properties.push_back( &stroke_miterlimit );
    _properties.push_back( &stroke_dasharray );
    _properties.push_back( &stroke_dashoffset );
    _properties.push_back( &stroke_opacity );

    _properties.push_back( &marker );
    _properties.push_back( &marker_start );
    _properties.push_back( &marker_mid );
    _properties.push_back( &marker_end );

    _properties.push_back( &paint_order );

    _properties.push_back( &filter );
    _properties.push_back( &filter_blend_mode );
    _properties.push_back( &filter_gaussianBlur_deviation );

    _properties.push_back( &color_rendering );
    _properties.push_back( &image_rendering );
    _properties.push_back( &shape_rendering );
    _properties.push_back( &text_rendering );

    _properties.push_back( &enable_background );

    // MAP -------------------------------------------

    // if( _propmap.size() == 0 ) {

    //     // 'color' must be before 'fill', 'stroke', 'text-decoration-color', ...
    //     _propmap.insert( std::make_pair( color.name,                 reinterpret_cast<SPIBasePtr>(&SPStyle::color                 ) ) );

    //     // 'font-size' must be before properties that need to know em, ex size (SPILength, SPILenghtOrNormal)
    //     _propmap.insert( std::make_pair( font_style.name,            reinterpret_cast<SPIBasePtr>(&SPStyle::font_style            ) ) );
    //     _propmap.insert( std::make_pair( font_variant.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::font_variant          ) ) );
    //     _propmap.insert( std::make_pair( font_weight.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::font_weight           ) ) );
    //     _propmap.insert( std::make_pair( font_stretch.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::font_stretch          ) ) );
    //     _propmap.insert( std::make_pair( font_size.name,             reinterpret_cast<SPIBasePtr>(&SPStyle::font_size             ) ) ); 
    //     _propmap.insert( std::make_pair( line_height.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::line_height           ) ) );
    //     _propmap.insert( std::make_pair( font_family.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::font_family           ) ) ); 
    //     _propmap.insert( std::make_pair( font.name,                  reinterpret_cast<SPIBasePtr>(&SPStyle::font                  ) ) ); 
    //     _propmap.insert( std::make_pair( font_specification.name,    reinterpret_cast<SPIBasePtr>(&SPStyle::font_specification    ) ) ); 

    //     _propmap.insert( std::make_pair( text_indent.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::text_indent           ) ) );
    //     _propmap.insert( std::make_pair( text_align.name,            reinterpret_cast<SPIBasePtr>(&SPStyle::text_align            ) ) );

    //     _propmap.insert( std::make_pair( text_decoration.name,       reinterpret_cast<SPIBasePtr>(&SPStyle::text_decoration       ) ) );
    //     _propmap.insert( std::make_pair( text_decoration_line.name,  reinterpret_cast<SPIBasePtr>(&SPStyle::text_decoration_line  ) ) );
    //     _propmap.insert( std::make_pair( text_decoration_style.name, reinterpret_cast<SPIBasePtr>(&SPStyle::text_decoration_style ) ) );
    //     _propmap.insert( std::make_pair( text_decoration_color.name, reinterpret_cast<SPIBasePtr>(&SPStyle::text_decoration_color ) ) );

    //     _propmap.insert( std::make_pair( letter_spacing.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::letter_spacing        ) ) );
    //     _propmap.insert( std::make_pair( word_spacing.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::word_spacing          ) ) );
    //     _propmap.insert( std::make_pair( text_transform.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::text_transform        ) ) );

    //     _propmap.insert( std::make_pair( direction.name,             reinterpret_cast<SPIBasePtr>(&SPStyle::direction             ) ) );
    //     _propmap.insert( std::make_pair( block_progression.name,     reinterpret_cast<SPIBasePtr>(&SPStyle::block_progression     ) ) );
    //     _propmap.insert( std::make_pair( writing_mode.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::writing_mode          ) ) );
    //     _propmap.insert( std::make_pair( baseline_shift.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::baseline_shift        ) ) );
    //     _propmap.insert( std::make_pair( text_anchor.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::text_anchor           ) ) );
    //     _propmap.insert( std::make_pair( white_space.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::white_space           ) ) );

    //     _propmap.insert( std::make_pair( clip_rule.name,             reinterpret_cast<SPIBasePtr>(&SPStyle::clip_rule             ) ) );
    //     _propmap.insert( std::make_pair( display.name,               reinterpret_cast<SPIBasePtr>(&SPStyle::display               ) ) );
    //     _propmap.insert( std::make_pair( overflow.name,              reinterpret_cast<SPIBasePtr>(&SPStyle::overflow              ) ) );
    //     _propmap.insert( std::make_pair( visibility.name,            reinterpret_cast<SPIBasePtr>(&SPStyle::visibility            ) ) );
    //     _propmap.insert( std::make_pair( opacity.name,               reinterpret_cast<SPIBasePtr>(&SPStyle::opacity               ) ) );

    //     _propmap.insert( std::make_pair( isolation.name,             reinterpret_cast<SPIBasePtr>(&SPStyle::isolation             ) ) );
    //     _propmap.insert( std::make_pair( mix_blend_mode.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::mix_blend_mode        ) ) );

    //     _propmap.insert( std::make_pair( color_interpolation.name,   reinterpret_cast<SPIBasePtr>(&SPStyle::color_interpolation   ) ) );
    //     _propmap.insert( std::make_pair( color_interpolation_filters.name, reinterpret_cast<SPIBasePtr>(&SPStyle::color_interpolation_filters ) ) );

    //     _propmap.insert( std::make_pair( solid_color.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::solid_color           ) ) );
    //     _propmap.insert( std::make_pair( solid_opacity.name,         reinterpret_cast<SPIBasePtr>(&SPStyle::solid_opacity         ) ) );

    //     _propmap.insert( std::make_pair( fill.name,                  reinterpret_cast<SPIBasePtr>(&SPStyle::fill                  ) ) );
    //     _propmap.insert( std::make_pair( fill_opacity.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::fill_opacity          ) ) );
    //     _propmap.insert( std::make_pair( fill_rule.name,             reinterpret_cast<SPIBasePtr>(&SPStyle::fill_rule             ) ) );


    //     _propmap.insert( std::make_pair( stroke.name,                reinterpret_cast<SPIBasePtr>(&SPStyle::stroke                ) ) );
    //     _propmap.insert( std::make_pair( stroke_width.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_width          ) ) );
    //     _propmap.insert( std::make_pair( stroke_linecap.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_linecap        ) ) );
    //     _propmap.insert( std::make_pair( stroke_linejoin.name,       reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_linejoin       ) ) );
    //     _propmap.insert( std::make_pair( stroke_miterlimit.name,     reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_miterlimit     ) ) );
    //     _propmap.insert( std::make_pair( stroke_dasharray.name,      reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_dasharray      ) ) );
    //     _propmap.insert( std::make_pair( stroke_dashoffset.name,     reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_dashoffset     ) ) );
    //     _propmap.insert( std::make_pair( stroke_opacity.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::stroke_opacity        ) ) );

    //     _propmap.insert( std::make_pair( marker.name,                reinterpret_cast<SPIBasePtr>(&SPStyle::marker                ) ) );
    //     _propmap.insert( std::make_pair( marker_start.name,          reinterpret_cast<SPIBasePtr>(&SPStyle::marker_start          ) ) );
    //     _propmap.insert( std::make_pair( marker_mid.name,            reinterpret_cast<SPIBasePtr>(&SPStyle::marker_mid            ) ) );
    //     _propmap.insert( std::make_pair( marker_end.name,            reinterpret_cast<SPIBasePtr>(&SPStyle::marker_end            ) ) );

    //     _propmap.insert( std::make_pair( paint_order.name,           reinterpret_cast<SPIBasePtr>(&SPStyle::paint_order           ) ) );

    //     _propmap.insert( std::make_pair( filter.name,                reinterpret_cast<SPIBasePtr>(&SPStyle::filter                ) ) );
    //     _propmap.insert( std::make_pair( filter_blend_mode.name,     reinterpret_cast<SPIBasePtr>(&SPStyle::filter_blend_mode     ) ) );
    //     _propmap.insert( std::make_pair( filter_gaussianBlur_deviation.name, reinterpret_cast<SPIBasePtr>(&SPStyle::filter_gaussianBlur_deviation ) ) );

    //     _propmap.insert( std::make_pair( color_rendering.name,       reinterpret_cast<SPIBasePtr>(&SPStyle::color_rendering       ) ) );
    //     _propmap.insert( std::make_pair( image_rendering.name,       reinterpret_cast<SPIBasePtr>(&SPStyle::image_rendering       ) ) );
    //     _propmap.insert( std::make_pair( shape_rendering.name,       reinterpret_cast<SPIBasePtr>(&SPStyle::shape_rendering       ) ) );
    //     _propmap.insert( std::make_pair( text_rendering.name,        reinterpret_cast<SPIBasePtr>(&SPStyle::text_rendering        ) ) );

    //     _propmap.insert( std::make_pair( enable_background.name,     reinterpret_cast<SPIBasePtr>(&SPStyle::enable_background     ) ) );

    // }
}

SPStyle::~SPStyle() {

    // std::cout << "SPStyle::~SPStyle" << std::endl;
    --_count; // Poor man's memory leak detector.

    // Remove connections
    release_connection.disconnect();
    fill_ps_changed_connection.disconnect();
    stroke_ps_changed_connection.disconnect();

    // The following shoud be moved into SPIPaint and SPIFilter
    if (fill.value.href) {
        fill_ps_modified_connection.disconnect();
    }

    if (stroke.value.href) {
        stroke_ps_modified_connection.disconnect();
    }

    if (filter.href) {
        filter_modified_connection.disconnect();
    }

    _properties.clear();

    // Conjecture: all this SPStyle ref counting is not needed. SPObject creates an instance of
    // SPStyle when it is constructed and deletes it when it is destructed. The refcount is
    // incremented and decremented only in the files: display/drawing-item.cpp,
    // display/nr-filter-primitive.cpp, and libnrtype/Layout-TNG-Input.cpp.
    if( _refcount > 1 ) {
        std::cerr << "SPStyle::~SPStyle: ref count greater than 1! " << _refcount << std::endl;
    }
    // std::cout << "SPStyle::~SPStyle(): Exit\n" << std::endl;
}

// Used in SPStyle::clear()
void clear_property( SPIBase* p ) {
    p->clear();
}


// Matches void sp_style_clear();
void
SPStyle::clear() {

    for_each( _properties.begin(), _properties.end(), clear_property );
    // for(SPPropMap::iterator i = _propmap.begin(); i != _propmap.end(); ++i ) {
    //     (this->*(i->second)).clear();
    // }

    // Release connection to object, created in constructor.
    release_connection.disconnect();

    // href->detach() called in fill->clear()...
    fill_ps_modified_connection.disconnect();
    if (fill.value.href) {
        delete fill.value.href;
        fill.value.href = NULL;
    }
    stroke_ps_modified_connection.disconnect();
    if (stroke.value.href) {
        delete stroke.value.href;
        stroke.value.href = NULL;
    }
    filter_modified_connection.disconnect();
    if (filter.href) {
        delete filter.href;
        filter.href = NULL;
    }

    if (document) {
        filter.href = new SPFilterReference(document);
        filter.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_filter_ref_changed), this));

        fill.value.href = new SPPaintServerReference(document);
        fill_ps_changed_connection = fill.value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_fill_paint_server_ref_changed), this));

        stroke.value.href = new SPPaintServerReference(document);
        stroke_ps_changed_connection = stroke.value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_stroke_paint_server_ref_changed), this));
    }

    cloned = false;

}

// Matches void sp_style_read(SPStyle *style, SPObject *object, Inkscape::XML::Node *repr)
void
SPStyle::read( SPObject *object, Inkscape::XML::Node *repr ) {

    // std::cout << "SPstyle::read( SPObject, Inkscape::XML::Node ): Entrance: "
    //           << (object?(object->getId()?object->getId():"id null"):"object null") << " "
    //           << (repr?(repr->name()?repr->name():"no name"):"repr null")
    //           << std::endl;
    g_assert(repr != NULL);
    g_assert(!object || (object->getRepr() == repr));

    // // Uncomment to verify that we don't need to call clear.
    // std::cout << " Creating temp style for testing" << std::endl;
    // SPStyle *temp = new SPStyle();
    // if( !(*temp == *this ) ) std::cout << "SPStyle::read: Need to clear" << std::endl;
    // delete temp;

    clear(); // FIXME, If this isn't here, gradient editing stops working. Why?

    if (object && object->cloned) {
        cloned = true;
    }

    /* 1. Style attribute */
    // std::cout << " MERGING STYLE ATTRIBUTE" << std::endl;
    gchar const *val = repr->attribute("style");
    if( val != NULL && *val ) {
        _mergeString( val );
    }

    /* 2 Style sheet */
    // std::cout << " MERGING OBJECT STYLESHEET" << std::endl;
    if (object) {
        _mergeObjectStylesheet( object );
    } else {
        // std::cerr << "SPStyle::read: No object! Can not read style sheet" << std::endl;
    }

    /* 3 Presentation attributes */
    // std::cout << " MERGING PRESENTATION ATTRIBUTES" << std::endl;
    for(std::vector<SPIBase*>::size_type i = 0; i != _properties.size(); ++i) {
        _properties[i]->readAttribute( repr );
    }
    // for(SPPropMap::iterator i = _propmap.begin(); i != _propmap.end(); ++i ) {
    //     (this->*(i->second)).readAttribute( repr );
    // }

    /* 4 Cascade from parent */
    // std::cout << " CASCADING FROM PARENT" << std::endl;
    if( object ) {
        if( object->parent ) {
            cascade( object->parent->style );
        }
    } else {
        // When does this happen?
        // std::cout << "SPStyle::read(): reading via repr->parent()" << std::endl;
        if( repr->parent() ) {
            SPStyle *parent = new SPStyle();
            parent->read( NULL, repr->parent() );
            cascade( parent );
            delete parent;
        }
    }
}

/**
 * Read style properties from object's repr.
 *
 * 1. Reset existing object style
 * 2. Load current effective object style
 * 3. Load i attributes from immediate parent (which has to be up-to-date)
 */
void
SPStyle::readFromObject( SPObject *object ) {

    // std::cout << "SPStyle::readFromObject: "<< (object->getId()?object->getId():"null")<< std::endl;

    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));

    Inkscape::XML::Node *repr = object->getRepr();
    g_return_if_fail(repr != NULL);

    read( object, repr );
}

/**
 * Read style properties from preferences.
 * @param path Preferences directory from which the style should be read
 */
void
SPStyle::readFromPrefs(Glib::ustring const &path) {

    g_return_if_fail(!path.empty());

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // not optimal: we reconstruct the node based on the prefs, then pass it to
    // sp_style_read for actual processing.
    Inkscape::XML::SimpleDocument *tempdoc = new Inkscape::XML::SimpleDocument;
    Inkscape::XML::Node *tempnode = tempdoc->createElement("prefs");

    std::vector<Inkscape::Preferences::Entry> attrs = prefs->getAllEntries(path);
    for (std::vector<Inkscape::Preferences::Entry>::iterator i = attrs.begin(); i != attrs.end(); ++i) {
        tempnode->setAttribute(i->getEntryName().data(), i->getString().data());
    }

    read( NULL, tempnode );

    Inkscape::GC::release(tempnode);
    Inkscape::GC::release(tempdoc);
    delete tempdoc;
}

// Matches sp_style_merge_property(SPStyle *style, gint id, gchar const *val)
void
SPStyle::readIfUnset( gint id, gchar const *val ) {

    // std::cout << "SPStyle::readIfUnset: Entrance: " << (val?val:"null") << std::endl;
    // To Do: If it is not too slow, use std::map instead of std::vector inorder to remove switch()
    // (looking up SP_PROP_xxxx already uses a hash).
    g_return_if_fail(val != NULL);

    switch (id) {
        case SP_PROP_INKSCAPE_FONT_SPEC:
            font_specification.readIfUnset( val );
            break;
        case SP_PROP_FONT_FAMILY:
            font_family.readIfUnset( val );
            break;
        case SP_PROP_FONT_SIZE:
            font_size.readIfUnset( val );
            break;
        case SP_PROP_FONT_SIZE_ADJUST:
            if (strcmp(val, "none") != 0) {
                g_warning("Unimplemented style property id SP_PROP_FONT_SIZE_ADJUST: value: %s", val);
            }
            break;
        case SP_PROP_FONT_STYLE:
            font_style.readIfUnset( val );
            break;
        case SP_PROP_FONT_VARIANT:
            font_variant.readIfUnset( val );
            break;
        case SP_PROP_FONT_WEIGHT:
            font_weight.readIfUnset( val );
            break;
        case SP_PROP_FONT_STRETCH:
            font_stretch.readIfUnset( val );
            break;
        case SP_PROP_FONT:
            font.readIfUnset( val );
            break;

            /* Text */
        case SP_PROP_TEXT_INDENT:
            text_indent.readIfUnset( val );
            break;
        case SP_PROP_TEXT_ALIGN:
            text_align.readIfUnset( val );
            break;
        case SP_PROP_TEXT_DECORATION:
            text_decoration.readIfUnset( val );
            break;
        case SP_PROP_TEXT_DECORATION_LINE:
            text_decoration_line.readIfUnset( val );
            break;
        case SP_PROP_TEXT_DECORATION_STYLE:
            text_decoration_style.readIfUnset( val );
            break;
        case SP_PROP_TEXT_DECORATION_COLOR:
            text_decoration_color.readIfUnset( val );
            break;
        case SP_PROP_LINE_HEIGHT:
            line_height.readIfUnset( val );
            break;
        case SP_PROP_LETTER_SPACING:
            letter_spacing.readIfUnset( val );
            break;
        case SP_PROP_WORD_SPACING:
            word_spacing.readIfUnset( val );
            break;
        case SP_PROP_TEXT_TRANSFORM:
            text_transform.readIfUnset( val );
            break;
            /* Text (css3) */
        case SP_PROP_DIRECTION:
            direction.readIfUnset( val );
            break;
        case SP_PROP_BLOCK_PROGRESSION:
            block_progression.readIfUnset( val );
            break;
        case SP_PROP_WRITING_MODE:
            writing_mode.readIfUnset( val );
            break;
        case SP_PROP_TEXT_ANCHOR:
            text_anchor.readIfUnset( val );
            break;
        case SP_PROP_WHITE_SPACE:
            white_space.readIfUnset( val );
            break;
        case SP_PROP_BASELINE_SHIFT:
            baseline_shift.readIfUnset( val );
            break;
        case SP_PROP_TEXT_RENDERING:
            text_rendering.readIfUnset( val );
            break;
        case SP_PROP_ALIGNMENT_BASELINE:
            g_warning("Unimplemented style property SP_PROP_ALIGNMENT_BASELINE: value: %s", val);
            break;
        case SP_PROP_DOMINANT_BASELINE:
            g_warning("Unimplemented style property SP_PROP_DOMINANT_BASELINE: value: %s", val);
            break;
        case SP_PROP_GLYPH_ORIENTATION_HORIZONTAL:
            g_warning("Unimplemented style property SP_PROP_ORIENTATION_HORIZONTAL: value: %s", val);
            break;
        case SP_PROP_GLYPH_ORIENTATION_VERTICAL:
            g_warning("Unimplemented style property SP_PROP_ORIENTATION_VERTICAL: value: %s", val);
            break;
        case SP_PROP_KERNING:
            g_warning("Unimplemented style property SP_PROP_KERNING: value: %s", val);
            break;
            /* Misc */
        case SP_PROP_CLIP:
            g_warning("Unimplemented style property SP_PROP_CLIP: value: %s", val);
            break;
        case SP_PROP_COLOR:
            color.readIfUnset( val );
            break;
        case SP_PROP_CURSOR:
            g_warning("Unimplemented style property SP_PROP_CURSOR: value: %s", val);
            break;
        case SP_PROP_DISPLAY:
            display.readIfUnset( val );
            break;
        case SP_PROP_OVERFLOW:
            overflow.readIfUnset( val );
            break;
        case SP_PROP_VISIBILITY:
            visibility.readIfUnset( val );
            break;
        case SP_PROP_ISOLATION:
            isolation.readIfUnset( val );
            break;
        case SP_PROP_MIX_BLEND_MODE:
            mix_blend_mode.readIfUnset( val );
            break;

            /* SVG */
            /* Clip/Mask */
        case SP_PROP_CLIP_PATH:
            /** \todo
             * This is a workaround. Inkscape only supports 'clip-path' as SVG attribute, not as
             * style property. By having both CSS and SVG attribute set, editing of clip-path
             * will fail, since CSS always overwrites SVG attributes.
             * Fixes Bug #324849
             */
            g_warning("attribute 'clip-path' given as CSS");

            //XML Tree being directly used here.
            this->object->getRepr()->setAttribute("clip-path", val);
            break;
        case SP_PROP_CLIP_RULE:
            clip_rule.readIfUnset( val );
            break;
        case SP_PROP_MASK:
            /** \todo
             * See comment for SP_PROP_CLIP_PATH
             */
            g_warning("attribute 'mask' given as CSS");
            
            //XML Tree being directly used here.
            this->object->getRepr()->setAttribute("mask", val);
            break;
        case SP_PROP_OPACITY:
            opacity.readIfUnset( val );
            break;
        case SP_PROP_ENABLE_BACKGROUND:
            enable_background.readIfUnset( val );
            break;
            /* Filter */
        case SP_PROP_FILTER:
            if( !filter.inherit ) filter.readIfUnset( val );
            break;
        case SP_PROP_FLOOD_COLOR:
            g_warning("Unimplemented style property SP_PROP_FLOOD_COLOR: value: %s", val);
            break;
        case SP_PROP_FLOOD_OPACITY:
            g_warning("Unimplemented style property SP_PROP_FLOOD_OPACITY: value: %s", val);
            break;
        case SP_PROP_LIGHTING_COLOR:
            g_warning("Unimplemented style property SP_PROP_LIGHTING_COLOR: value: %s", val);
            break;
            /* Gradient */
        case SP_PROP_STOP_COLOR:
            g_warning("Unimplemented style property SP_PROP_STOP_COLOR: value: %s", val);
            break;
        case SP_PROP_STOP_OPACITY:
            g_warning("Unimplemented style property SP_PROP_STOP_OPACITY: value: %s", val);
            break;
            /* Interactivity */
        case SP_PROP_POINTER_EVENTS:
            g_warning("Unimplemented style property SP_PROP_POINTER_EVENTS: value: %s", val);
            break;
            /* Paint */
        case SP_PROP_COLOR_INTERPOLATION:
            // We read it but issue warning
            color_interpolation.readIfUnset( val );
            if( color_interpolation.value != SP_CSS_COLOR_INTERPOLATION_SRGB ) {
                g_warning("Inkscape currently only supports color-interpolation = sRGB");
            }
            break;
        case SP_PROP_COLOR_INTERPOLATION_FILTERS:
            color_interpolation_filters.readIfUnset( val );
            break;
        case SP_PROP_COLOR_PROFILE:
            g_warning("Unimplemented style property SP_PROP_COLOR_PROFILE: value: %s", val);
            break;
        case SP_PROP_COLOR_RENDERING:
            color_rendering.readIfUnset( val );
            break;
        case SP_PROP_SOLID_COLOR:
            solid_color.readIfUnset( val );
            break;
        case SP_PROP_SOLID_OPACITY:
            solid_opacity.readIfUnset( val );
            break;
        case SP_PROP_FILL:
            fill.readIfUnset( val );
            break;
        case SP_PROP_FILL_OPACITY:
            fill_opacity.readIfUnset( val );
            break;
        case SP_PROP_FILL_RULE:
            fill_rule.readIfUnset( val );
            break;
        case SP_PROP_IMAGE_RENDERING:
            image_rendering.readIfUnset( val );
            break;
        case SP_PROP_MARKER:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */ 
            marker.readIfUnset( val );
            break;
        case SP_PROP_MARKER_START:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            marker_start.readIfUnset( val );
            break;
        case SP_PROP_MARKER_MID:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            marker_mid.readIfUnset( val );
            break;
        case SP_PROP_MARKER_END:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            marker_end.readIfUnset( val );
            break;
        case SP_PROP_SHAPE_RENDERING:
            shape_rendering.readIfUnset( val );
            break;
        case SP_PROP_STROKE:
            stroke.readIfUnset( val );
            break;
        case SP_PROP_STROKE_WIDTH:
            stroke_width.readIfUnset( val );
            break;
        case SP_PROP_STROKE_DASHARRAY:
            stroke_dasharray.readIfUnset( val );
            break;
        case SP_PROP_STROKE_DASHOFFSET:
            stroke_dashoffset.readIfUnset( val );
            break;
        case SP_PROP_STROKE_LINECAP:
            stroke_linecap.readIfUnset( val );
            break;
        case SP_PROP_STROKE_LINEJOIN:
            stroke_linejoin.readIfUnset( val );
            break;
        case SP_PROP_STROKE_MITERLIMIT:
            stroke_miterlimit.readIfUnset( val );
            break;
        case SP_PROP_STROKE_OPACITY:
            stroke_opacity.readIfUnset( val );
            break;
        case SP_PROP_PAINT_ORDER:
            paint_order.readIfUnset( val );
            break;
        default:
            g_warning("SPIStyle::readIfUnset(): Invalid style property id: %d value: %s", id, val);
            break;
    }
}

/**
 * Outputs the style to a CSS string.
 *
 * Use with SP_STYLE_FLAG_ALWAYS for copying an object's complete cascaded style to
 * style_clipboard.
 *
 * Use with SP_STYLE_FLAG_IFDIFF and a pointer to the parent class when you need a CSS string for
 * an object in the document tree.
 *
 * \pre flags in {IFSET, ALWAYS, IFDIFF}.
 * \pre base.
 * \post ret != NULL.
 */
Glib::ustring
SPStyle::write( guint const flags, SPStyle const *const base ) const {

    // std::cout << "SPStyle::write" << std::endl;

    Glib::ustring style_string;
    for(std::vector<SPIBase*>::size_type i = 0; i != _properties.size(); ++i) {
        if( base != NULL ) {
            style_string += _properties[i]->write( flags, base->_properties[i] );
        } else {
            style_string += _properties[i]->write( flags, NULL );
        }
    }
    // for(SPPropMap::iterator i = _propmap.begin(); i != _propmap.end(); ++i ) {
    //     if( base != NULL ) {
    //         style_string += (this->*(i->second)).write( flags, &(base->*(i->second)) );
    //     } else {
    //         style_string += (this->*(i->second)).write( flags, NULL );
    //     }
    // }

    // Remove trailing ';'
    if( style_string.size() > 0 ) {
        style_string.erase( style_string.size() - 1 );
    }
    return style_string;
}

// Corresponds to sp_style_merge_from_parent()
/**
 * Sets computed values in \a style, which may involve inheriting from (or in some other way
 * calculating from) corresponding computed values of \a parent.
 *
 * References: http://www.w3.org/TR/SVG11/propidx.html shows what properties inherit by default.
 * http://www.w3.org/TR/SVG11/styling.html#Inheritance gives general rules as to what it means to
 * inherit a value.  http://www.w3.org/TR/REC-CSS2/cascade.html#computed-value is more precise
 * about what the computed value is (not obvious for lengths).
 *
 * \pre \a parent's computed values are already up-to-date.
 */
void
SPStyle::cascade( SPStyle const *const parent ) {
    // std::cout << "SPStyle::cascade: " << (object->getId()?object->getId():"null") << std::endl;
    for(std::vector<SPIBase*>::size_type i = 0; i != _properties.size(); ++i) {
        _properties[i]->cascade( parent->_properties[i] );
    }
    // for(SPPropMap::iterator i = _propmap.begin(); i != _propmap.end(); ++i ) {
    //     (this->*(i->second)).cascade( &(parent->*(i->second)) );
    // }
}

// Corresponds to sp_style_merge_from_dying_parent()
/**
 * Combine \a style and \a parent style specifications into a single style specification that
 * preserves (as much as possible) the effect of the existing \a style being a child of \a parent.
 *
 * Called when the parent repr is to be removed (e.g. the parent is a \<use\> element that is being
 * unlinked), in which case we copy/adapt property values that are explicitly set in \a parent,
 * trying to retain the same visual appearance once the parent is removed.  Interesting cases are
 * when there is unusual interaction with the parent's value (opacity, display) or when the value
 * can be specified as relative to the parent computed value (font-size, font-weight etc.).
 *
 * Doesn't update computed values of \a style.  For correctness, you should subsequently call
 * sp_style_merge_from_parent against the new parent (presumably \a parent's parent) even if \a
 * style was previously up-to-date wrt \a parent.
 *
 * \pre \a parent's computed values are already up-to-date.
 *   (\a style's computed values needn't be up-to-date.)
 */
void
SPStyle::merge( SPStyle const *const parent ) {
    // std::cout << "SPStyle::merge" << std::endl;
    for(std::vector<SPIBase*>::size_type i = 0; i != _properties.size(); ++i) {
        _properties[i]->merge( parent->_properties[i] );
    }
    // for(SPPropMap::iterator i = _propmap.begin(); i != _propmap.end(); ++i ) {
    //     (this->*(i->second)).cascade( &(parent->*(i->second)) );
    // }
}

/**
 * Parses a style="..." string and merges it with an existing SPStyle.
 */
void
SPStyle::mergeString( gchar const *const p ) {
    _mergeString( p );
}

// Mostly for unit testing
bool
SPStyle::operator==(const SPStyle& rhs) {

    // Uncomment for testing
    // for(std::vector<SPIBase*>::size_type i = 0; i != _properties.size(); ++i) {
    //     if( *_properties[i] != *rhs._properties[i])
    //     std::cout << _properties[i]->name << ": "
    //               << _properties[i]->write(SP_STYLE_FLAG_ALWAYS,NULL) << " " 
    //               << rhs._properties[i]->write(SP_STYLE_FLAG_ALWAYS,NULL)
    //               << (*_properties[i]  == *rhs._properties[i]) << std::endl;
    // }

    for(std::vector<SPIBase*>::size_type i = 0; i != _properties.size(); ++i) {
        if( *_properties[i] != *rhs._properties[i]) return false;
    }
    return true;
}

void
SPStyle::_mergeString( gchar const *const p ) {

    // std::cout << "SPStyle::_mergeString: " << (p?p:"null") << std::endl;
    CRDeclaration *const decl_list
        = cr_declaration_parse_list_from_buf(reinterpret_cast<guchar const *>(p), CR_UTF_8);
    if (decl_list) {
        _mergeDeclList( decl_list );
        cr_declaration_destroy(decl_list);
    }
}

void
SPStyle::_mergeDeclList( CRDeclaration const *const decl_list ) {

    // std::cout << "SPStyle::_mergeDeclList" << std::endl;

    // In reverse order, as later declarations to take precedence over earlier ones.
    // (Properties are only set if not previously set. See:
    // Ref: http://www.w3.org/TR/REC-CSS2/cascade.html#cascading-order point 4.)
    if (decl_list->next) {
        _mergeDeclList( decl_list->next );
    }
    _mergeDecl( decl_list );
}

void
SPStyle::_mergeDecl(  CRDeclaration const *const decl ) {

    // std::cout << "SPStyle::_mergeDecl" << std::endl;

    unsigned const prop_idx = sp_attribute_lookup(decl->property->stryng->str);
    if (prop_idx != SP_ATTR_INVALID) {
        /** \todo
         * effic: Test whether the property is already set before trying to
         * convert to string. Alternatively, set from CRTerm directly rather
         * than converting to string.
         */
        guchar *const str_value_unsigned = cr_term_to_string(decl->value);
        gchar *const str_value = reinterpret_cast<gchar *>(str_value_unsigned);
        readIfUnset( prop_idx, str_value );
        g_free(str_value);
    }
}

void
SPStyle::_mergeProps( CRPropList *const props ) {

    // std::cout << "SPStyle::_mergeProps" << std::endl;

    // In reverse order, as later declarations to take precedence over earlier ones.
    if (props) {
        _mergeProps( cr_prop_list_get_next( props ) );
        CRDeclaration *decl = NULL;
        cr_prop_list_get_decl(props, &decl);
        _mergeDecl( decl );
    }
}

void
SPStyle::_mergeObjectStylesheet( SPObject const *const object ) {

    // std::cout << "SPStyle::_mergeObjectStylesheet: " << (object->getId()?object->getId():"null") << std::endl;

    static CRSelEng *sel_eng = NULL;
    if (!sel_eng) {
        sel_eng = sp_repr_sel_eng();
    }

    CRPropList *props = NULL;

    //XML Tree being directly used here while it shouldn't be.
    CRStatus status = cr_sel_eng_get_matched_properties_from_cascade(sel_eng,
                                                                     object->document->style_cascade,
                                                                     object->getRepr(),
                                                                     &props);
    g_return_if_fail(status == CR_OK);
    /// \todo Check what errors can occur, and handle them properly.
    if (props) {
        _mergeProps(props);
        cr_prop_list_destroy(props);
    }
}

// Internal
/**
 * Release callback.
 */
static void
sp_style_object_release(SPObject *object, SPStyle *style)
{
    (void)object; // TODO
    style->object = NULL;
}

// Internal
/**
 * Emit style modified signal on style's object if the filter changed.
 */
static void
sp_style_filter_ref_modified(SPObject *obj, guint flags, SPStyle *style)
{
    (void)flags; // TODO
    SPFilter *filter=static_cast<SPFilter *>(obj);
    if (style->getFilter() == filter)
    {
        if (style->object) {
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    }
}

// Internal
/**
 * Gets called when the filter is (re)attached to the style
 */
void
sp_style_filter_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style)
{
    if (old_ref) {
        style->filter_modified_connection.disconnect();
    }
    if ( SP_IS_FILTER(ref))
    {
        style->filter_modified_connection =
           ref->connectModified(sigc::bind(sigc::ptr_fun(&sp_style_filter_ref_modified), style));
    }

    sp_style_filter_ref_modified(ref, 0, style);
}

/**
 * Emit style modified signal on style's object if server is style's fill
 * or stroke paint server.
 */
static void
sp_style_paint_server_ref_modified(SPObject *obj, guint flags, SPStyle *style)
{
    (void)flags; // TODO
    SPPaintServer *server = static_cast<SPPaintServer *>(obj);

    if ((style->fill.isPaintserver())
        && style->getFillPaintServer() == server)
    {
        if (style->object) {
            /** \todo
             * fixme: I do not know, whether it is optimal - we are
             * forcing reread of everything (Lauris)
             */
            /** \todo
             * fixme: We have to use object_modified flag, because parent
             * flag is only available downstreams.
             */
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } else if ((style->stroke.isPaintserver())
        && style->getStrokePaintServer() == server)
    {
        if (style->object) {
            /// \todo fixme:
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } else if (server) {
        g_assert_not_reached();
    }
}

/**
 * Gets called when the paintserver is (re)attached to the style
 */
void
sp_style_fill_paint_server_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style)
{
    if (old_ref) {
        style->fill_ps_modified_connection.disconnect();
    }
    if (SP_IS_PAINT_SERVER(ref)) {
        style->fill_ps_modified_connection =
           ref->connectModified(sigc::bind(sigc::ptr_fun(&sp_style_paint_server_ref_modified), style));
    }

    style->signal_fill_ps_changed.emit(old_ref, ref);
    sp_style_paint_server_ref_modified(ref, 0, style);
}

/**
 * Gets called when the paintserver is (re)attached to the style
 */
void
sp_style_stroke_paint_server_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style)
{
    if (old_ref) {
        style->stroke_ps_modified_connection.disconnect();
    }
    if (SP_IS_PAINT_SERVER(ref)) {
        style->stroke_ps_modified_connection =
          ref->connectModified(sigc::bind(sigc::ptr_fun(&sp_style_paint_server_ref_modified), style));
    }

    style->signal_stroke_ps_changed.emit(old_ref, ref);
    sp_style_paint_server_ref_modified(ref, 0, style);
}

// Called in display/drawing-item.cpp, display/nr-filter-primitive.cpp, libnrtype/Layout-TNG-Input.cpp
/**
 * Increase refcount of style.
 */
SPStyle *
sp_style_ref(SPStyle *style)
{
    g_return_val_if_fail(style != NULL, NULL);

    style->style_ref(); // Increase ref count

    return style;
}

// Called in display/drawing-item.cpp, display/nr-filter-primitive.cpp, libnrtype/Layout-TNG-Input.cpp
/**
 * Decrease refcount of style with possible destruction.
 */
SPStyle *
sp_style_unref(SPStyle *style)
{
    g_return_val_if_fail(style != NULL, NULL);
    if (style->style_unref() < 1) {
        delete style;
        return NULL;
    }
    return style;
}

static CRSelEng *
sp_repr_sel_eng()
{
    CRSelEng *const ret = cr_sel_eng_new();
    cr_sel_eng_set_node_iface(ret, &Inkscape::XML::croco_node_iface);

    /** \todo
     * Check whether we need to register any pseudo-class handlers.
     * libcroco has its own default handlers for first-child and lang.
     *
     * We probably want handlers for link and arguably visited (though
     * inkscape can't visit links at the time of writing).  hover etc.
     * more useful in inkview than the editor inkscape.
     *
     * http://www.w3.org/TR/SVG11/styling.html#StylingWithCSS says that
     * the following should be honoured, at least by inkview:
     * :hover, :active, :focus, :visited, :link.
     */

    g_assert(ret);
    return ret;
}

/** Indexed by SP_CSS_FONT_SIZE_blah.   These seem a bit small */
static float const font_size_table[] = {6.0, 8.0, 10.0, 12.0, 14.0, 18.0, 24.0};

// The following functions should be incorporated into SPIPaint. FIXME
// Called in: style.cpp, style-internal.cpp
void
sp_style_set_ipaint_to_uri(SPStyle *style, SPIPaint *paint, const Inkscape::URI *uri, SPDocument *document)
{
    // std::cout << "sp_style_set_ipaint_to_uri: Entrance: " << uri << "  " << (void*)document << std::endl;
    // it may be that this style's SPIPaint has not yet created its URIReference;
    // now that we have a document, we can create it here
    if (!paint->value.href && document) {
        paint->value.href = new SPPaintServerReference(document);
        if (paint == &style->fill) {
            style->fill_ps_changed_connection = paint->value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_fill_paint_server_ref_changed), style));
        } else {
            style->stroke_ps_changed_connection = paint->value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_stroke_paint_server_ref_changed), style));
        }
    }

    if (paint->value.href){
        if (paint->value.href->getObject()){
            paint->value.href->detach();
        }

        try {
            paint->value.href->attach(*uri);
        } catch (Inkscape::BadURIException &e) {
            g_warning("%s", e.what());
            paint->value.href->detach();
        }
    }
}

// Called in: style.cpp, style-internal.cpp
void
sp_style_set_ipaint_to_uri_string (SPStyle *style, SPIPaint *paint, const gchar *uri)
{
    try {
        const Inkscape::URI IURI(uri);
        sp_style_set_ipaint_to_uri(style, paint, &IURI, style->document);
    } catch (...) {
        g_warning("URI failed to parse: %s", uri);
    }
}

// Called in: desktop-style.cpp
void
sp_style_set_to_uri_string (SPStyle *style, bool isfill, const gchar *uri)
{
    sp_style_set_ipaint_to_uri_string (style, isfill? &style->fill : &style->stroke, uri);
}

// Called in: widgets/font-selector.cpp, widgets/text-toolbar.cpp, ui/dialog/text-edit.cpp
gchar const *
sp_style_get_css_unit_string(int unit)
{
    // specify px by default, see inkscape bug 1221626, mozilla bug 234789

    switch (unit) {

        case SP_CSS_UNIT_NONE: return "px";
        case SP_CSS_UNIT_PX: return "px";
        case SP_CSS_UNIT_PT: return "pt";
        case SP_CSS_UNIT_PC: return "pc";
        case SP_CSS_UNIT_MM: return "mm";
        case SP_CSS_UNIT_CM: return "cm";
        case SP_CSS_UNIT_IN: return "in";
        case SP_CSS_UNIT_EM: return "em";
        case SP_CSS_UNIT_EX: return "ex";
        case SP_CSS_UNIT_PERCENT: return "%";
        default: return "px";
    }
    return "px";
}

// Called in: style-internal.cpp, widgets/text-toolbar.cpp, ui/dialog/text-edit.cpp
/*
 * Convert a size in pixels into another CSS unit size
 */
double
sp_style_css_size_px_to_units(double size, int unit)
{
    double unit_size = size;
    switch (unit) {

        case SP_CSS_UNIT_NONE: unit_size = size; break;
        case SP_CSS_UNIT_PX: unit_size = size; break;
        case SP_CSS_UNIT_PT: unit_size = Inkscape::Util::Quantity::convert(size, "px", "pt");  break;
        case SP_CSS_UNIT_PC: unit_size = Inkscape::Util::Quantity::convert(size, "px", "pc");  break;
        case SP_CSS_UNIT_MM: unit_size = Inkscape::Util::Quantity::convert(size, "px", "mm");  break;
        case SP_CSS_UNIT_CM: unit_size = Inkscape::Util::Quantity::convert(size, "px", "cm");  break;
        case SP_CSS_UNIT_IN: unit_size = Inkscape::Util::Quantity::convert(size, "px", "in");  break;
        case SP_CSS_UNIT_EM: unit_size = size / SP_CSS_FONT_SIZE_DEFAULT; break;
        case SP_CSS_UNIT_EX: unit_size = size * 2.0 / SP_CSS_FONT_SIZE_DEFAULT ; break;
        case SP_CSS_UNIT_PERCENT: unit_size = size * 100.0 / SP_CSS_FONT_SIZE_DEFAULT; break;

        default:
            g_warning("sp_style_get_css_font_size_units conversion to %d not implemented.", unit);
            break;
    }

    return unit_size;
}

// Called in: widgets/text-toolbar.cpp, ui/dialog/text-edit.cpp
/*
 * Convert a size in a CSS unit size to pixels
 */
double
sp_style_css_size_units_to_px(double size, int unit)
{
    if (unit == SP_CSS_UNIT_PX) {
        return size;
    }
    //g_message("sp_style_css_size_units_to_px %f %d = %f px", size, unit, out);
    return size * (size / sp_style_css_size_px_to_units(size, unit));;
}


// FIXME: Everything below this line belongs in a different file - css-chemistry?

void
sp_style_set_property_url (SPObject *item, gchar const *property, SPObject *linked, bool recursive)
{
    Inkscape::XML::Node *repr = item->getRepr();

    if (repr == NULL) return;

    SPCSSAttr *css = sp_repr_css_attr_new();
    if (linked) {
        gchar *val = g_strdup_printf("url(#%s)", linked->getId());
        sp_repr_css_set_property(css, property, val);
        g_free(val);
    } else {
        sp_repr_css_unset_property(css, "filter");
    }

    if (recursive) {
        sp_repr_css_change_recursive(repr, css, "style");
    } else {
        sp_repr_css_change(repr, css, "style");
    }
    sp_repr_css_attr_unref(css);
}

// Called in sp-object.cpp
/**
 * Clear all style property attributes in object.
 */
void
sp_style_unset_property_attrs(SPObject *o)
{
    if (!o) {
        return;
    }

    SPStyle *style = o->style;
    if (!style) {
        return;
    }

    Inkscape::XML::Node *repr = o->getRepr();
    if (!repr) {
        return;
    }

    if (style->opacity.set) {
        repr->setAttribute("opacity", NULL);
    }
    if (style->color.set) {
        repr->setAttribute("color", NULL);
    }
    if (style->color_interpolation.set) {
        repr->setAttribute("color-interpolation", NULL);
    }
    if (style->color_interpolation_filters.set) {
        repr->setAttribute("color-interpolation-filters", NULL);
    }
    if (style->solid_color.set) {
        repr->setAttribute("solid-color", NULL);
    }
    if (style->solid_opacity.set) {
        repr->setAttribute("solid-opacity", NULL);
    }
    if (style->fill.set) {
        repr->setAttribute("fill", NULL);
    }
    if (style->fill_opacity.set) {
        repr->setAttribute("fill-opacity", NULL);
    }
    if (style->fill_rule.set) {
        repr->setAttribute("fill-rule", NULL);
    }
    if (style->stroke.set) {
        repr->setAttribute("stroke", NULL);
    }
    if (style->stroke_width.set) {
        repr->setAttribute("stroke-width", NULL);
    }
    if (style->stroke_linecap.set) {
        repr->setAttribute("stroke-linecap", NULL);
    }
    if (style->stroke_linejoin.set) {
        repr->setAttribute("stroke-linejoin", NULL);
    }
    if (style->marker.set) {
        repr->setAttribute("marker", NULL);
    }
    if (style->marker_start.set) {
        repr->setAttribute("marker-start", NULL);
    }
    if (style->marker_mid.set) {
        repr->setAttribute("marker-mid", NULL);
    }
    if (style->marker_end.set) {
        repr->setAttribute("marker-end", NULL);
    }
    if (style->stroke_opacity.set) {
        repr->setAttribute("stroke-opacity", NULL);
    }
    if (style->stroke_dasharray.set) {
        repr->setAttribute("stroke-dasharray", NULL);
    }
    if (style->stroke_dashoffset.set) {
        repr->setAttribute("stroke-dashoffset", NULL);
    }
    if (style->paint_order.set) {
        repr->setAttribute("paint-order", NULL);
    }
    if (style->font_specification.set) {
        repr->setAttribute("-inkscape-font-specification", NULL);
    }
    if (style->font_family.set) {
        repr->setAttribute("font-family", NULL);
    }
    if (style->text_anchor.set) {
        repr->setAttribute("text-anchor", NULL);
    }
    if (style->white_space.set) {
        repr->setAttribute("white_space", NULL);
    }
    if (style->writing_mode.set) {
        repr->setAttribute("writing_mode", NULL);
    }
    if (style->filter.set) {
        repr->setAttribute("filter", NULL);
    }
    if (style->enable_background.set) {
        repr->setAttribute("enable-background", NULL);
    }
    if (style->clip_rule.set) {
        repr->setAttribute("clip-rule", NULL);
    }
    if (style->color_rendering.set) {
        repr->setAttribute("color-rendering", NULL);
    }
    if (style->image_rendering.set) {
        repr->setAttribute("image-rendering", NULL);
    }
    if (style->shape_rendering.set) {
        repr->setAttribute("shape-rendering", NULL);
    }
    if (style->text_rendering.set) {
        repr->setAttribute("text-rendering", NULL);
    }
}

/**
 * \pre style != NULL.
 * \pre flags in {IFSET, ALWAYS}.
 */
SPCSSAttr *
sp_css_attr_from_style(SPStyle const *const style, guint const flags)
{
    g_return_val_if_fail(style != NULL, NULL);
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);
    Glib::ustring style_str = style->write(flags);
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_attr_add_from_string(css, style_str.c_str());
    return css;
}

// Called in: selection-chemistry.cpp, widgets/stroke-marker-selector.cpp, widgets/stroke-style.cpp,
// ui/tools/freehand-base.cpp
/**
 * \pre object != NULL
 * \pre flags in {IFSET, ALWAYS}.
 */
SPCSSAttr *sp_css_attr_from_object(SPObject *object, guint const flags)
{
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);
    SPCSSAttr * result = 0;    
    if (object->style) {
        result = sp_css_attr_from_style(object->style, flags);
    }
    return result;
}

// Called in: selection-chemistry.cpp, ui/dialog/inkscape-preferences.cpp
/**
 * Unset any text-related properties
 */
SPCSSAttr *
sp_css_attr_unset_text(SPCSSAttr *css)
{
    sp_repr_css_set_property(css, "font", NULL);
    sp_repr_css_set_property(css, "-inkscape-font-specification", NULL);
    sp_repr_css_set_property(css, "font-size", NULL);
    sp_repr_css_set_property(css, "font-size-adjust", NULL); // not implemented yet
    sp_repr_css_set_property(css, "font-style", NULL);
    sp_repr_css_set_property(css, "font-variant", NULL);
    sp_repr_css_set_property(css, "font-weight", NULL);
    sp_repr_css_set_property(css, "font-stretch", NULL);
    sp_repr_css_set_property(css, "font-family", NULL);
    sp_repr_css_set_property(css, "text-indent", NULL);
    sp_repr_css_set_property(css, "text-align", NULL);
    sp_repr_css_set_property(css, "line-height", NULL);
    sp_repr_css_set_property(css, "letter-spacing", NULL);
    sp_repr_css_set_property(css, "word-spacing", NULL);
    sp_repr_css_set_property(css, "text-transform", NULL);
    sp_repr_css_set_property(css, "direction", NULL);
    sp_repr_css_set_property(css, "block-progression", NULL);
    sp_repr_css_set_property(css, "writing-mode", NULL);
    sp_repr_css_set_property(css, "text-anchor", NULL);
    sp_repr_css_set_property(css, "white-space", NULL);
    sp_repr_css_set_property(css, "kerning", NULL); // not implemented yet
    sp_repr_css_set_property(css, "dominant-baseline", NULL); // not implemented yet
    sp_repr_css_set_property(css, "alignment-baseline", NULL); // not implemented yet
    sp_repr_css_set_property(css, "baseline-shift", NULL);

    sp_repr_css_set_property(css, "text-decoration", NULL);
    sp_repr_css_set_property(css, "text-decoration-line", NULL);
    sp_repr_css_set_property(css, "text-decoration-color", NULL);
    sp_repr_css_set_property(css, "text-decoration-style", NULL);

    return css;
}

// ui/dialog/inkscape-preferences.cpp
/**
 * Unset properties that should not be set for default tool style.
 * This list needs to be reviewed.
 */
SPCSSAttr *
sp_css_attr_unset_blacklist(SPCSSAttr *css)
{
    sp_repr_css_set_property(css, "color",               NULL);
    sp_repr_css_set_property(css, "clip-rule",           NULL);
    sp_repr_css_set_property(css, "display",             NULL);
    sp_repr_css_set_property(css, "overflow",            NULL);
    sp_repr_css_set_property(css, "visibility",          NULL);
    sp_repr_css_set_property(css, "isolation",           NULL);
    sp_repr_css_set_property(css, "mix-blend-mode",      NULL);
    sp_repr_css_set_property(css, "color-interpolation", NULL);
    sp_repr_css_set_property(css, "color-interpolation-filters", NULL);
    sp_repr_css_set_property(css, "solid-color",         NULL);
    sp_repr_css_set_property(css, "solid-opacity",       NULL);
    sp_repr_css_set_property(css, "fill-rule",           NULL);
    sp_repr_css_set_property(css, "filter-blend-mode",   NULL);
    sp_repr_css_set_property(css, "filter-gaussianBlur-deviation", NULL);
    sp_repr_css_set_property(css, "color-rendering",     NULL);
    sp_repr_css_set_property(css, "image-rendering",     NULL);
    sp_repr_css_set_property(css, "shape-rendering",     NULL);
    sp_repr_css_set_property(css, "text-rendering",      NULL);
    sp_repr_css_set_property(css, "enable-background",   NULL);

    return css;
}

// Called in style.cpp
static bool
is_url(char const *p)
{
    if (p == NULL)
        return false;
/** \todo
 * FIXME: I'm not sure if this applies to SVG as well, but CSS2 says any URIs
 * in property values must start with 'url('.
 */
    return (g_ascii_strncasecmp(p, "url(", 4) == 0);
}

// Called in: ui/dialog/inkscape-preferences.cpp, ui/tools/tweek-tool.cpp
/**
 * Unset any properties that contain URI values.
 *
 * Used for storing style that will be reused across documents when carrying
 * the referenced defs is impractical.
 */
SPCSSAttr *
sp_css_attr_unset_uris(SPCSSAttr *css)
{
// All properties that may hold <uri> or <paint> according to SVG 1.1
    if (is_url(sp_repr_css_property(css, "clip-path", NULL))) sp_repr_css_set_property(css, "clip-path", NULL);
    if (is_url(sp_repr_css_property(css, "color-profile", NULL))) sp_repr_css_set_property(css, "color-profile", NULL);
    if (is_url(sp_repr_css_property(css, "cursor", NULL))) sp_repr_css_set_property(css, "cursor", NULL);
    if (is_url(sp_repr_css_property(css, "filter", NULL))) sp_repr_css_set_property(css, "filter", NULL);
    if (is_url(sp_repr_css_property(css, "marker", NULL))) sp_repr_css_set_property(css, "marker", NULL);
    if (is_url(sp_repr_css_property(css, "marker-start", NULL))) sp_repr_css_set_property(css, "marker-start", NULL);
    if (is_url(sp_repr_css_property(css, "marker-mid", NULL))) sp_repr_css_set_property(css, "marker-mid", NULL);
    if (is_url(sp_repr_css_property(css, "marker-end", NULL))) sp_repr_css_set_property(css, "marker-end", NULL);
    if (is_url(sp_repr_css_property(css, "mask", NULL))) sp_repr_css_set_property(css, "mask", NULL);
    if (is_url(sp_repr_css_property(css, "fill", NULL))) sp_repr_css_set_property(css, "fill", NULL);
    if (is_url(sp_repr_css_property(css, "stroke", NULL))) sp_repr_css_set_property(css, "stroke", NULL);

    return css;
}

// Called in style.cpp
/**
 * Scale a single-value property.
 */
static void
sp_css_attr_scale_property_single(SPCSSAttr *css, gchar const *property,
                                  double ex, bool only_with_units = false)
{
    gchar const *w = sp_repr_css_property(css, property, NULL);
    if (w) {
        gchar *units = NULL;
        double wd = g_ascii_strtod(w, &units) * ex;
        if (w == units) {// nothing converted, non-numeric value
            return;
        }
        if (only_with_units && (units == NULL || *units == '\0' || *units == '%')) {
            // only_with_units, but no units found, so do nothing.
            return;
        }
        Inkscape::CSSOStringStream os;
        os << wd << units; // reattach units
        sp_repr_css_set_property(css, property, os.str().c_str());
    }
}

// Called in style.cpp for stroke-dasharray
/**
 * Scale a list-of-values property.
 */
static void
sp_css_attr_scale_property_list(SPCSSAttr *css, gchar const *property, double ex)
{
    gchar const *string = sp_repr_css_property(css, property, NULL);
    if (string) {
        Inkscape::CSSOStringStream os;
        gchar **a = g_strsplit(string, ",", 10000);
        bool first = true;
        for (gchar **i = a; i != NULL; i++) {
            gchar *w = *i;
            if (w == NULL)
                break;
            gchar *units = NULL;
            double wd = g_ascii_strtod(w, &units) * ex;
            if (w == units) {// nothing converted, non-numeric value ("none" or "inherit"); do nothing
                g_strfreev(a);
                return;
            }
            if (!first) {
                os << ",";
            }
            os << wd << units; // reattach units
            first = false;
        }
        sp_repr_css_set_property(css, property, os.str().c_str());
        g_strfreev(a);
    }
}

// Called in: text-editing.cpp, 
/**
 * Scale any properties that may hold <length> by ex.
 */
SPCSSAttr *
sp_css_attr_scale(SPCSSAttr *css, double ex)
{
    sp_css_attr_scale_property_single(css, "baseline-shift", ex);
    sp_css_attr_scale_property_single(css, "stroke-width", ex);
    sp_css_attr_scale_property_list  (css, "stroke-dasharray", ex);
    sp_css_attr_scale_property_single(css, "stroke-dashoffset", ex);
    sp_css_attr_scale_property_single(css, "font-size", ex);
    sp_css_attr_scale_property_single(css, "kerning", ex);
    sp_css_attr_scale_property_single(css, "letter-spacing", ex);
    sp_css_attr_scale_property_single(css, "word-spacing", ex);
    sp_css_attr_scale_property_single(css, "line-height", ex, true);

    return css;
}


/**
 * Quote and/or escape string for writing to CSS, changing strings in place.
 * See: http://www.w3.org/TR/CSS21/syndata.html#value-def-identifier
 */
void
css_quote(Glib::ustring &val)
{
    Glib::ustring out;
    bool quote = false;

    // Can't wait for C++11!
    for( Glib::ustring::iterator it = val.begin(); it != val.end(); ++it) {
        if(g_ascii_isalnum(*it) || *it=='-' || *it=='_' || *it > 0xA0) {
            out += *it;
        } else if (*it == '\'') {
            // Single quotes require escaping and quotes.
            out += '\\';
            out += *it;
            quote = true;
        } else {
            // Quote everything else including spaces.
            // (CSS Fonts Level 3 recommends quoting with spaces.)
            out += *it;
            quote = true;
        }
        if( it == val.begin() && !g_ascii_isalpha(*it) ) {
            // A non-ASCII/non-alpha initial value on any indentifier needs quotes.
            // (Actually it's a bit more complicated but as it never hurts to quote...)
            quote = true;
        }
    }
    if( quote ) {
        out.insert( out.begin(), '\'' );
        out += '\'';
    }
    val = out;
}


/**
 * Quote font names in font-family lists, changing string in place.
 * We use unquoted names internally but some need to be quoted in CSS.
 */
void
css_font_family_quote(Glib::ustring &val)
{
    std::vector<Glib::ustring> tokens = Glib::Regex::split_simple("\\s*,\\s*", val );

    val.erase();
    for( unsigned i=0; i < tokens.size(); ++i ) {
        css_quote( tokens[i] );
        val += tokens[i] + ", ";
    }
    if( val.size() > 1 )
        val.erase( val.size() - 2 ); // Remove trailing ", "
}


// Called in style-internal.cpp, xml/repr-css.cpp
/**
 * Remove paired single and double quotes from a string, changing string in place.
 */
void
css_unquote(Glib::ustring &val)
{
  if( val.size() > 1 &&
      ( (val[0] == '"'  && val[val.size()-1] == '"'  ) ||
	(val[0] == '\'' && val[val.size()-1] == '\'' ) ) ) {

    val.erase( 0, 1 );
    val.erase( val.size()-1 );
  }
}

// Called in style-internal.cpp, text-toolbar.cpp
/**
 * Remove paired single and double quotes from font names in font-family lists,
 * changing string in place.
 * We use unquoted family names internally but CSS sometimes uses quoted names.
 */
void
css_font_family_unquote(Glib::ustring &val)
{
    std::vector<Glib::ustring> tokens = Glib::Regex::split_simple("\\s*,\\s*", val );

    val.erase();
    for( unsigned i=0; i < tokens.size(); ++i ) {
        css_unquote( tokens[i] );
        val += tokens[i] + ", ";
    }
    if( val.size() > 1 )
        val.erase( val.size() - 2 ); // Remove trailing ", "
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

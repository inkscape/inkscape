/*
 * Inkscape::Text::Layout - text layout engine input functions
 *
 * Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 * Copyright (C) 2005 Richard Hughes
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PANGO_ENABLE_ENGINE
#define PANGO_ENABLE_ENGINE
#endif

#include <gtk/gtk.h>
#include "Layout-TNG.h"
#include "style.h"
#include "svg/svg-length.h"
#include "sp-object.h"
#include "sp-string.h"
#include "FontFactory.h"


namespace Inkscape {
namespace Text {

void Layout::_clearInputObjects()
{
    for(std::vector<InputStreamItem*>::iterator it = _input_stream.begin() ; it != _input_stream.end() ; ++it) {
        delete *it;
    }

    _input_stream.clear();
    _input_wrap_shapes.clear();
}

// this function does nothing more than store all its parameters for future reference
void Layout::appendText(Glib::ustring const &text, SPStyle *style, void *source_cookie, OptionalTextTagAttrs const *optional_attributes, unsigned optional_attributes_offset, Glib::ustring::const_iterator text_begin, Glib::ustring::const_iterator text_end)
{
    if (style == NULL) return;

    InputStreamTextSource *new_source = new InputStreamTextSource;

    new_source->source_cookie = source_cookie;
    new_source->text = &text;
    new_source->text_begin = text_begin;
    new_source->text_end = text_end;
    new_source->style = style;
    sp_style_ref(style);

    new_source->text_length = 0;
    for ( ; text_begin != text_end && text_begin != text.end() ; ++text_begin)
        new_source->text_length++;        // save this because calculating the length of a UTF-8 string is expensive

    if (optional_attributes) {
        // we need to fill in x and y even if the text is empty so that empty paragraphs can be positioned correctly
        _copyInputVector(optional_attributes->x, optional_attributes_offset, &new_source->x, std::max(1, new_source->text_length));
        _copyInputVector(optional_attributes->y, optional_attributes_offset, &new_source->y, std::max(1, new_source->text_length));
        _copyInputVector(optional_attributes->dx, optional_attributes_offset, &new_source->dx, new_source->text_length);
        _copyInputVector(optional_attributes->dy, optional_attributes_offset, &new_source->dy, new_source->text_length);
        _copyInputVector(optional_attributes->rotate, optional_attributes_offset, &new_source->rotate, new_source->text_length);
        if (!optional_attributes->rotate.empty() && optional_attributes_offset >= optional_attributes->rotate.size()) {
            SVGLength last_rotate;
            last_rotate = 0.f;
            for (std::vector<SVGLength>::const_iterator it = optional_attributes->rotate.begin() ; it != optional_attributes->rotate.end() ; ++it)
                if (it->_set)
                    last_rotate = *it;
            new_source->rotate.resize(1, last_rotate);
        }
        new_source->textLength._set = optional_attributes->textLength._set;
        new_source->textLength.value = optional_attributes->textLength.value;
        new_source->textLength.computed = optional_attributes->textLength.computed;
        new_source->textLength.unit = optional_attributes->textLength.unit;
        new_source->lengthAdjust = optional_attributes->lengthAdjust;
    }
    
    _input_stream.push_back(new_source);
}

void Layout::_copyInputVector(std::vector<SVGLength> const &input_vector, unsigned input_offset, std::vector<SVGLength> *output_vector, size_t max_length)
{
    output_vector->clear();
    if (input_offset >= input_vector.size()) return;
    output_vector->reserve(std::min(max_length, input_vector.size() - input_offset));
    while (input_offset < input_vector.size() && max_length != 0) {
        if (!input_vector[input_offset]._set)
            break;
        output_vector->push_back(input_vector[input_offset]);
        input_offset++;
        max_length--;
    }
}

// just save what we've been given, really
void Layout::appendControlCode(TextControlCode code, void *source_cookie, double width, double ascent, double descent)
{
    InputStreamControlCode *new_code = new InputStreamControlCode;

    new_code->source_cookie = source_cookie;
    new_code->code = code;
    new_code->width = width;
    new_code->ascent = ascent;
    new_code->descent = descent;
    
    _input_stream.push_back(new_code);
}

// more saving of the parameters
void Layout::appendWrapShape(Shape const *shape, DisplayAlign display_align)
{
    _input_wrap_shapes.push_back(InputWrapShape());
    _input_wrap_shapes.back().shape = shape;
    _input_wrap_shapes.back().display_align = display_align;
}

int Layout::_enum_converter(int input, EnumConversionItem const *conversion_table, unsigned conversion_table_size)
{
    for (unsigned i = 0 ; i < conversion_table_size ; i++)
        if (conversion_table[i].input == input)
            return conversion_table[i].output;
    return conversion_table[0].output;
}

Layout::Direction Layout::InputStreamTextSource::styleGetBlockProgression() const
{
  switch( style->writing_mode.computed ) {

  case SP_CSS_WRITING_MODE_LR_TB:
  case SP_CSS_WRITING_MODE_RL_TB:
    return TOP_TO_BOTTOM;
      
  case SP_CSS_WRITING_MODE_TB_RL:
    return RIGHT_TO_LEFT;

  case SP_CSS_WRITING_MODE_TB_LR:
    return LEFT_TO_RIGHT;

  default:
    std::cerr << "Layout::InputTextStream::styleGetBlockProgression: invalid writing mode." << std::endl;
  }
  return TOP_TO_BOTTOM;
}

SPCSSTextOrientation Layout::InputStreamTextSource::styleGetTextOrientation() const
{
  return ((SPCSSTextOrientation)style->text_orientation.computed);
}

SPCSSBaseline Layout::InputStreamTextSource::styleGetDominantBaseline() const
{
  return ((SPCSSBaseline)style->dominant_baseline.computed);
}

static Layout::Alignment text_anchor_to_alignment(unsigned anchor, Layout::Direction para_direction)
{
    switch (anchor) {
        default:
        case SP_CSS_TEXT_ANCHOR_START:  return para_direction == Layout::LEFT_TO_RIGHT ? Layout::LEFT : Layout::RIGHT;
        case SP_CSS_TEXT_ANCHOR_MIDDLE: return Layout::CENTER;
        case SP_CSS_TEXT_ANCHOR_END:    return para_direction == Layout::LEFT_TO_RIGHT ? Layout::RIGHT : Layout::LEFT;
    }
}

Layout::Alignment Layout::InputStreamTextSource::styleGetAlignment(Layout::Direction para_direction, bool try_text_align) const
{
    if (!try_text_align)
        return text_anchor_to_alignment(style->text_anchor.computed, para_direction);

    // there's no way to tell the difference between text-anchor set higher up the cascade to the default and
    // text-anchor never set anywhere in the cascade, so in order to detect which of text-anchor or text-align
    // to use we'll have to run up the style tree ourselves.
    SPStyle const *this_style = style;

    for ( ; ; ) {
        // If both text-align and text-anchor are set at the same level, text-align takes
        // precedence because it is the most expressive.
        if (this_style->text_align.set) {
            switch (style->text_align.computed) {
                default:
                case SP_CSS_TEXT_ALIGN_START:   return para_direction == LEFT_TO_RIGHT ? LEFT : RIGHT;
                case SP_CSS_TEXT_ALIGN_END:     return para_direction == LEFT_TO_RIGHT ? RIGHT : LEFT;
                case SP_CSS_TEXT_ALIGN_LEFT:    return LEFT;
                case SP_CSS_TEXT_ALIGN_RIGHT:   return RIGHT;
                case SP_CSS_TEXT_ALIGN_CENTER:  return CENTER;
                case SP_CSS_TEXT_ALIGN_JUSTIFY: return FULL;
            }
        }
        if (this_style->text_anchor.set)
            return text_anchor_to_alignment(this_style->text_anchor.computed, para_direction);
        if (this_style->object == NULL || this_style->object->parent == NULL) break;
        this_style = this_style->object->parent->style;
        if (this_style == NULL) break;
    }
    return para_direction == LEFT_TO_RIGHT ? LEFT : RIGHT;
}

static const Layout::EnumConversionItem enum_convert_spstyle_style_to_pango_style[] = {
    {SP_CSS_FONT_STYLE_NORMAL,  PANGO_STYLE_NORMAL},
    {SP_CSS_FONT_STYLE_ITALIC,  PANGO_STYLE_ITALIC},
    {SP_CSS_FONT_STYLE_OBLIQUE, PANGO_STYLE_OBLIQUE}};

static const Layout::EnumConversionItem enum_convert_spstyle_weight_to_pango_weight[] = {
  // NB: The Pango web page calls 500 "the normal font" but both CSS2 and the Pango
  // enumeration define 400 as normal.
    {SP_CSS_FONT_WEIGHT_NORMAL, PANGO_WEIGHT_NORMAL},
    {SP_CSS_FONT_WEIGHT_BOLD,PANGO_WEIGHT_BOLD},
    {SP_CSS_FONT_WEIGHT_100, PANGO_WEIGHT_THIN},
    {SP_CSS_FONT_WEIGHT_200, PANGO_WEIGHT_ULTRALIGHT},
    {SP_CSS_FONT_WEIGHT_300, PANGO_WEIGHT_LIGHT},
    {SP_CSS_FONT_WEIGHT_400, PANGO_WEIGHT_NORMAL},
    {SP_CSS_FONT_WEIGHT_500, PANGO_WEIGHT_MEDIUM},
    {SP_CSS_FONT_WEIGHT_600, PANGO_WEIGHT_SEMIBOLD},
    {SP_CSS_FONT_WEIGHT_700, PANGO_WEIGHT_BOLD},
    {SP_CSS_FONT_WEIGHT_800, PANGO_WEIGHT_ULTRABOLD},
    {SP_CSS_FONT_WEIGHT_900, PANGO_WEIGHT_HEAVY}};

static const Layout::EnumConversionItem enum_convert_spstyle_stretch_to_pango_stretch[] = {
    {SP_CSS_FONT_STRETCH_NORMAL,          PANGO_STRETCH_NORMAL},
    {SP_CSS_FONT_STRETCH_ULTRA_CONDENSED, PANGO_STRETCH_ULTRA_CONDENSED},
    {SP_CSS_FONT_STRETCH_EXTRA_CONDENSED, PANGO_STRETCH_EXTRA_CONDENSED},
    {SP_CSS_FONT_STRETCH_CONDENSED,       PANGO_STRETCH_CONDENSED},
    {SP_CSS_FONT_STRETCH_SEMI_CONDENSED,  PANGO_STRETCH_SEMI_CONDENSED},
    {SP_CSS_FONT_STRETCH_SEMI_EXPANDED,   PANGO_STRETCH_SEMI_EXPANDED},
    {SP_CSS_FONT_STRETCH_EXPANDED,        PANGO_STRETCH_EXPANDED},
    {SP_CSS_FONT_STRETCH_EXTRA_EXPANDED,  PANGO_STRETCH_EXTRA_EXPANDED},
    {SP_CSS_FONT_STRETCH_ULTRA_EXPANDED,  PANGO_STRETCH_ULTRA_EXPANDED}};

static const Layout::EnumConversionItem enum_convert_spstyle_variant_to_pango_variant[] = {
    {SP_CSS_FONT_VARIANT_NORMAL,     PANGO_VARIANT_NORMAL},
    {SP_CSS_FONT_VARIANT_SMALL_CAPS, PANGO_VARIANT_SMALL_CAPS}};

font_instance *Layout::InputStreamTextSource::styleGetFontInstance() const
{
    PangoFontDescription *descr = styleGetFontDescription();
    if (descr == NULL) return NULL;
    font_instance *res = (font_factory::Default())->Face(descr);
    pango_font_description_free(descr);
    return res;
}

PangoFontDescription *Layout::InputStreamTextSource::styleGetFontDescription() const
{
    PangoFontDescription *descr = pango_font_description_new();
    // Pango can't cope with spaces before or after the commas - let's remove them.
    // this code is not exactly unicode-safe, but it's similar to what's done in
    // pango, so it's not the limiting factor
    Glib::ustring family;
    if (style->font_family.value == NULL) {
        family = "sans-serif";
    } else {
        gchar **families = g_strsplit(style->font_family.value, ",", -1);
        if (families) {
            for (gchar **f = families ; *f ; ++f) {
                g_strstrip(*f);
                if (!family.empty()) family += ',';
                family += *f;
            }
        }
        g_strfreev(families);
    }

    pango_font_description_set_family(descr,family.c_str());
    pango_font_description_set_weight(descr,(PangoWeight)_enum_converter(style->font_weight.computed,  enum_convert_spstyle_weight_to_pango_weight,   sizeof(enum_convert_spstyle_weight_to_pango_weight)/sizeof(enum_convert_spstyle_weight_to_pango_weight[0])));
    pango_font_description_set_style(descr,(PangoStyle)_enum_converter(style->font_style.computed,   enum_convert_spstyle_style_to_pango_style,     sizeof(enum_convert_spstyle_style_to_pango_style)/sizeof(enum_convert_spstyle_style_to_pango_style[0])));
    pango_font_description_set_variant(descr,(PangoVariant)_enum_converter(style->font_variant.computed, enum_convert_spstyle_variant_to_pango_variant, sizeof(enum_convert_spstyle_variant_to_pango_variant)/sizeof(enum_convert_spstyle_variant_to_pango_variant[0])));
#ifdef USE_PANGO_WIN32
    // damn Pango fudges the size, so we need to unfudge. See source of pango_win32_font_map_init()
    pango_font_description_set_size(descr, (int) ((font_factory::Default())->fontSize*PANGO_SCALE*72/GetDeviceCaps(pango_win32_get_dc(),LOGPIXELSY))); // mandatory huge size (hinting workaround)
    // we don't set stretch on Win32, because pango-win32 has no concept of it
    // (Windows doesn't really provide any useful field it could use).
    // If we did set stretch, then any text with a font-stretch attribute would
    // end up falling back to Arial.
#else
    pango_font_description_set_size(descr, (int) ((font_factory::Default())->fontSize*PANGO_SCALE)); // mandatory huge size (hinting workaround)
    pango_font_description_set_stretch(descr,(PangoStretch)_enum_converter(style->font_stretch.computed, enum_convert_spstyle_stretch_to_pango_stretch, sizeof(enum_convert_spstyle_stretch_to_pango_stretch)/sizeof(enum_convert_spstyle_stretch_to_pango_stretch[0])));
#endif
    return descr;
}

Layout::InputStreamTextSource::~InputStreamTextSource()
{
  sp_style_unref(style);
}

}//namespace Text
}//namespace Inkscape

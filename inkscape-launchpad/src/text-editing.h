#ifndef SEEN_SP_TEXT_EDITING_H
#define SEEN_SP_TEXT_EDITING_H

/*
 * Text editing functions common for for text and flowtext
 *
 * Authors:
 *   bulia byak
 *   Richard Hughes
 *
 * Copyright (C) 2004-5 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <utility>   // std::pair
#include "libnrtype/Layout-TNG.h"
#include "text-tag-attributes.h"

class SPCSSAttr;
class SPDesktop;
class SPItem;
class SPObject;
class SPStyle;

typedef std::pair<Inkscape::Text::Layout::iterator, Inkscape::Text::Layout::iterator> iterator_pair; 


Inkscape::Text::Layout const * te_get_layout (SPItem const *item);

void te_update_layout_now_recursive(SPItem *item);

/** Returns true if there are no visible characters on the canvas. */
bool sp_te_output_is_empty(SPItem const *item);

/** Returns true if the user has typed nothing in the text box. */
bool sp_te_input_is_empty(SPObject const *item);

/** Recursively gets the length of all the SPStrings at or below the given
\a item. Also adds 1 for each line break encountered. */
unsigned sp_text_get_length(SPObject const *item);

/** Recursively gets the length of all the SPStrings at or below the given
\a item, before and not including \a upto. Also adds 1 for each line break encountered. */
unsigned sp_text_get_length_upto(SPObject const *item, SPObject const *upto);

std::vector<Geom::Point> sp_te_create_selection_quads(SPItem const *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, Geom::Affine const &transform);

Inkscape::Text::Layout::iterator sp_te_get_position_by_coords (SPItem const *item, Geom::Point const &i_p);
void sp_te_get_cursor_coords (SPItem const *item, Inkscape::Text::Layout::iterator const &position, Geom::Point &p0, Geom::Point &p1);
double sp_te_get_average_linespacing (SPItem *text);

SPStyle const * sp_te_style_at_position(SPItem const *text, Inkscape::Text::Layout::iterator const &position);
SPObject const * sp_te_object_at_position(SPItem const *text, Inkscape::Text::Layout::iterator const &position);

Inkscape::Text::Layout::iterator sp_te_insert(SPItem *item, Inkscape::Text::Layout::iterator const &position, char const *utf8);
Inkscape::Text::Layout::iterator sp_te_replace(SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, char const *utf8);
Inkscape::Text::Layout::iterator sp_te_insert_line (SPItem *text, Inkscape::Text::Layout::iterator const &position);
bool sp_te_delete (SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, iterator_pair &iter_pair);

char *sp_te_get_string_multiline(SPItem const *text);
Glib::ustring sp_te_get_string_multiline(SPItem const *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end);
void sp_te_set_repr_text_multiline(SPItem *text, char const *str);

TextTagAttributes*
text_tag_attributes_at_position(SPItem *item, Inkscape::Text::Layout::iterator const &position, unsigned *char_index);

void sp_te_adjust_kerning_screen(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, Geom::Point by);
void sp_te_adjust_dx (SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double delta);
void sp_te_adjust_dy (SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double delta);

void sp_te_adjust_rotation_screen(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double pixels);
void sp_te_adjust_rotation(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double degrees);
void sp_te_set_rotation(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double degrees);

void sp_te_adjust_tspan_letterspacing_screen(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double by);
void sp_te_adjust_linespacing_screen(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPDesktop *desktop, double by);
void sp_te_apply_style(SPItem *text, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, SPCSSAttr const *css);

bool is_part_of_text_subtree (SPObject *obj);
bool is_top_level_text_object (SPObject *obj);
bool has_visible_text (SPObject *obj);

#endif // SEEN_SP_TEXT_EDITING_H

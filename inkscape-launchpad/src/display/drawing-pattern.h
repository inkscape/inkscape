/**
 * @file
 * Canvas belonging to SVG pattern.
 *//*
 * Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *
 * Copyright (C) 2014 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DISPLAY_DRAWING_PATTERN_H
#define SEEN_INKSCAPE_DISPLAY_DRAWING_PATTERN_H

#include "display/drawing-group.h"

typedef struct _cairo_pattern cairo_pattern_t;

namespace Inkscape {

/**
 * @brief Drawing tree node used for rendering paints.
 *
 * DrawingPattern is used for rendering patterns and hatches.
 *
 * It renders it's children to a cairo_pattern_t structure that can be
 * applied as source for fill or stroke operations.
 */
class DrawingPattern
    : public DrawingGroup
{
public:
    DrawingPattern(Drawing &drawing, bool debug = false);
    ~DrawingPattern();

    /**
     * Set the transformation from pattern to user coordinate systems.
     * @see SPPattern description for explanation of coordinate systems.
     */
    void setPatternToUserTransform(Geom::Affine const &new_trans);
    /**
     * Set the tile rect position and dimentions in content coordinate system
     */
    void setTileRect(Geom::Rect const &tile_rect);
    /**
     * Turn on overflow rendering.
     *
     * Overflow is implemented as repeated rendering of pattern contents. In every step
     * a translation transform is applied.
     */
    void setOverflow(Geom::Affine initial_transform, int steps, Geom::Affine step_transform);
    /**
     * Render the pattern.
     *
     * Returns caito_pattern_t structure that can be set as source surface.
     */
    cairo_pattern_t *renderPattern(float opacity);
protected:
    virtual unsigned _updateItem(Geom::IntRect const &area, UpdateContext const &ctx,
                                     unsigned flags, unsigned reset);

    Geom::Affine *_pattern_to_user;
    Geom::Affine _overflow_initial_transform;
    Geom::Affine _overflow_step_transform;
    int _overflow_steps;
    Geom::OptRect _tile_rect;
    bool _debug;
    Geom::IntPoint _pattern_resolution;
};

bool is_drawing_group(DrawingItem *item);

} // end namespace Inkscape

#endif // !SEEN_INKSCAPE_DISPLAY_DRAWING_PATTERN_H

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

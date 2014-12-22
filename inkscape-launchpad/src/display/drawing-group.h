/**
 * @file
 * Group belonging to an SVG drawing element.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DISPLAY_DRAWING_GROUP_H
#define SEEN_INKSCAPE_DISPLAY_DRAWING_GROUP_H

#include "display/drawing-item.h"

namespace Inkscape {

class DrawingGroup
    : public DrawingItem
{
public:
    DrawingGroup(Drawing &drawing);
    ~DrawingGroup();

    bool pickChildren() { return _pick_children; }
    void setPickChildren(bool p);

    void setChildTransform(Geom::Affine const &new_trans);

protected:
    virtual unsigned _updateItem(Geom::IntRect const &area, UpdateContext const &ctx,
                                 unsigned flags, unsigned reset);
    virtual unsigned _renderItem(DrawingContext &dc, Geom::IntRect const &area, unsigned flags,
                                 DrawingItem *stop_at);
    virtual void _clipItem(DrawingContext &dc, Geom::IntRect const &area);
    virtual DrawingItem *_pickItem(Geom::Point const &p, double delta, unsigned flags);
    virtual bool _canClip();

    Geom::Affine *_child_transform;
};

bool is_drawing_group(DrawingItem *item);

} // end namespace Inkscape

#endif // !SEEN_INKSCAPE_DISPLAY_DRAWING_ITEM_H

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

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

#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "display/drawing-group.h"
#include "style.h"

#include "display/cairo-utils.h"

namespace Inkscape {

DrawingGroup::DrawingGroup(Drawing &drawing)
    : DrawingItem(drawing)
    , _child_transform(NULL)
{}

DrawingGroup::~DrawingGroup()
{
    delete _child_transform; // delete NULL; is safe
}

/**
 * Set whether the group returns children from pick calls.
 * Previously this feature was called "transparent groups".
 */
void
DrawingGroup::setPickChildren(bool p)
{
    _pick_children = p;
}

/**
 * Set additional transform for the group.
 * This is applied after the normal transform and mainly useful for
 * markers, clipping paths, etc.
 */
void
DrawingGroup::setChildTransform(Geom::Affine const &new_trans)
{
    Geom::Affine current;
    if (_child_transform) {
        current = *_child_transform;
    }

    if (!Geom::are_near(current, new_trans, 1e-18)) {
        // mark the area where the object was for redraw.
        _markForRendering();
        if (new_trans.isIdentity()) {
            delete _child_transform; // delete NULL; is safe
            _child_transform = NULL;
        } else {
            _child_transform = new Geom::Affine(new_trans);
        }
        _markForUpdate(STATE_ALL, true);
    }
}

unsigned
DrawingGroup::_updateItem(Geom::IntRect const &area, UpdateContext const &ctx, unsigned flags, unsigned reset)
{
    unsigned beststate = STATE_ALL;
    bool outline = _drawing.outline();

    UpdateContext child_ctx(ctx);
    if (_child_transform) {
        child_ctx.ctm = *_child_transform * ctx.ctm;
    }
    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        i->update(area, child_ctx, flags, reset);
    }
    if (beststate & STATE_BBOX) {
        _bbox = Geom::OptIntRect();
        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            if (i->visible()) {
                _bbox.unionWith(outline ? i->geometricBounds() : i->visualBounds());
            }
        }
    }
    return beststate;
}

unsigned
DrawingGroup::_renderItem(DrawingContext &dc, Geom::IntRect const &area, unsigned flags, DrawingItem *stop_at)
{
    if (stop_at == NULL) {
        // normal rendering
        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            i->render(dc, area, flags, stop_at);
        }
    } else {
        // background rendering
        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            if (&*i == stop_at) return RENDER_OK; // do not render the stop_at item at all
            if (i->isAncestorOf(stop_at)) {
                // render its ancestors without masks, opacity or filters
                i->render(dc, area, flags | RENDER_FILTER_BACKGROUND, stop_at);
                // stop further rendering
                return RENDER_OK;
            } else {
                i->render(dc, area, flags, stop_at);
            }
        }
    }
    return RENDER_OK;
}

void
DrawingGroup::_clipItem(DrawingContext &dc, Geom::IntRect const &area)
{
    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        i->clip(dc, area);
    }
}

DrawingItem *
DrawingGroup::_pickItem(Geom::Point const &p, double delta, unsigned flags)
{
    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        DrawingItem *picked = i->pick(p, delta, flags);
        if (picked) {
            return _pick_children ? picked : this;
        }
    }
    return NULL;
}

bool
DrawingGroup::_canClip()
{
    return true;
}

bool is_drawing_group(DrawingItem *item)
{
    return dynamic_cast<DrawingGroup *>(item) != NULL;
}

} // end namespace Inkscape

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

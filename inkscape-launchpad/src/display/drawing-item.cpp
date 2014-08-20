/**
 * @file
 * Canvas item belonging to an SVG drawing element.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <climits>
#include "display/cairo-utils.h"
#include "display/cairo-templates.h"
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "display/drawing-group.h"
#include "display/drawing-surface.h"
#include "nr-filter.h"
#include "preferences.h"
#include "style.h"

namespace Inkscape {

void set_cairo_blend_operator( DrawingContext &dc, unsigned blend_mode ) {

    // All of the blend modes are implemented in Cairo as of 1.10.
    // For a detailed description, see:
    // http://cairographics.org/operators/
    switch (blend_mode) {
    case SP_CSS_BLEND_MULTIPLY:
        dc.setOperator(CAIRO_OPERATOR_MULTIPLY);
        break;
    case SP_CSS_BLEND_SCREEN:
        dc.setOperator(CAIRO_OPERATOR_SCREEN);
        break;
    case SP_CSS_BLEND_DARKEN:
        dc.setOperator(CAIRO_OPERATOR_DARKEN);
        break;
    case SP_CSS_BLEND_LIGHTEN:
        dc.setOperator(CAIRO_OPERATOR_LIGHTEN);
        break;
    case SP_CSS_BLEND_OVERLAY:   
        dc.setOperator(CAIRO_OPERATOR_OVERLAY);
        break;
    case SP_CSS_BLEND_COLORDODGE:
        dc.setOperator(CAIRO_OPERATOR_COLOR_DODGE);
        break;
    case SP_CSS_BLEND_COLORBURN:
        dc.setOperator(CAIRO_OPERATOR_COLOR_BURN);
        break;
    case SP_CSS_BLEND_HARDLIGHT:
        dc.setOperator(CAIRO_OPERATOR_HARD_LIGHT);
        break;
    case SP_CSS_BLEND_SOFTLIGHT:
        dc.setOperator(CAIRO_OPERATOR_SOFT_LIGHT);
        break;
    case SP_CSS_BLEND_DIFFERENCE:
        dc.setOperator(CAIRO_OPERATOR_DIFFERENCE);
        break;
    case SP_CSS_BLEND_EXCLUSION:
        dc.setOperator(CAIRO_OPERATOR_EXCLUSION);
        break;
    case SP_CSS_BLEND_HUE:       
        dc.setOperator(CAIRO_OPERATOR_HSL_HUE);
        break;
    case SP_CSS_BLEND_SATURATION:
        dc.setOperator(CAIRO_OPERATOR_HSL_SATURATION);
        break;
    case SP_CSS_BLEND_COLOR:
        dc.setOperator(CAIRO_OPERATOR_HSL_COLOR);
        break;
    case SP_CSS_BLEND_LUMINOSITY:
        dc.setOperator(CAIRO_OPERATOR_HSL_LUMINOSITY);
        break;
    case SP_CSS_BLEND_NORMAL:
    default:
        dc.setOperator(CAIRO_OPERATOR_OVER);
        break;
    }
}

/**
 * @class DrawingItem
 * SVG drawing item for display.
 *
 * This was previously known as NRArenaItem. It represents the renderable
 * portion of the SVG document. Typically this is created by the SP tree,
 * in particular the show() virtual function.
 *
 * @section ObjectLifetime Object lifetime
 * Deleting a DrawingItem will cause all of its children to be deleted as well.
 * This can lead to nasty surprises if you hold references to things
 * which are children of what is being deleted. Therefore, in the SP tree,
 * you always need to delete the item views of children before deleting
 * the view of the parent. Do not call delete on things returned from show()
 * - this will cause dangling pointers inside the SPItem and lead to a crash.
 * Use the corresponing hide() method.
 *
 * Outside of the SP tree, you should not use any references after the root node
 * has been deleted.
 */

DrawingItem::DrawingItem(Drawing &drawing)
    : _drawing(drawing)
    , _parent(NULL)
    , _key(0)
    , _opacity(1.0)
    , _transform(NULL)
    , _clip(NULL)
    , _mask(NULL)
    , _filter(NULL)
    , _user_data(NULL)
    , _cache(NULL)
    , _state(0)
    , _child_type(CHILD_ORPHAN)
    , _background_new(0)
    , _background_accumulate(0)
    , _visible(true)
    , _sensitive(true)
    , _cached(0)
    , _cached_persistent(0)
    , _has_cache_iterator(0)
    , _propagate(0)
//    , _renders_opacity(0)
    , _pick_children(0)
    , _antialias(1)
    , _isolation(SP_CSS_ISOLATION_AUTO)
    , _mix_blend_mode(SP_CSS_BLEND_NORMAL)
{}

DrawingItem::~DrawingItem()
{
    _drawing.signal_item_deleted.emit(this);
    //if (!_children.empty()) {
    //    g_warning("Removing item with children");
    //}

    // remove from the set of cached items and delete cache
    setCached(false, true);
    if (_has_cache_iterator) {
        _drawing._candidate_items.erase(_cache_iterator);
    }
    // remove this item from parent's children list
    // due to the effect of clearChildren(), this only happens for the top-level deleted item
    if (_parent) {
        _markForRendering();
    }

    switch (_child_type) {
    case CHILD_NORMAL: {
        ChildrenList::iterator ithis = _parent->_children.iterator_to(*this);
        _parent->_children.erase(ithis);
        } break;
    case CHILD_CLIP:
        // we cannot call setClip(NULL) or setMask(NULL),
        // because that would be an endless loop
        _parent->_clip = NULL;
        break;
    case CHILD_MASK:
        _parent->_mask = NULL;
        break;
    case CHILD_ROOT:
        _drawing._root = NULL;
        break;
    default: ;
    }

    if (_parent) {
        _parent->_markForUpdate(STATE_ALL, false);
    }
    clearChildren();
    delete _transform;
    delete _clip;
    delete _mask;
    delete _filter;
}

DrawingItem *
DrawingItem::parent() const
{
    // initially I wanted to return NULL if we are a clip or mask child,
    // but the previous behavior was just to return the parent regardless of child type
    return _parent;
}

/// Returns true if item is among the descendants. Will return false if item == this.
bool
DrawingItem::isAncestorOf(DrawingItem *item) const
{
    for (DrawingItem *i = item->_parent; i; i = i->_parent) {
        if (i == this) return true;
    }
    return false;
}

void
DrawingItem::appendChild(DrawingItem *item)
{
    item->_parent = this;
    assert(item->_child_type == CHILD_ORPHAN);
    item->_child_type = CHILD_NORMAL;
    _children.push_back(*item);

    // This ensures that _markForUpdate() called on the child will recurse to this item
    item->_state = STATE_ALL;
    // Because _markForUpdate recurses through ancestors, we can simply call it
    // on the just-added child. This has the additional benefit that we do not
    // rely on the appended child being in the default non-updated state.
    // We set propagate to true, because the child might have descendants of its own.
    item->_markForUpdate(STATE_ALL, true);
}

void
DrawingItem::prependChild(DrawingItem *item)
{
    item->_parent = this;
    assert(item->_child_type == CHILD_ORPHAN);
    item->_child_type = CHILD_NORMAL;
    _children.push_front(*item);
    // See appendChild for explanation
    item->_state = STATE_ALL;
    item->_markForUpdate(STATE_ALL, true);
}

/// Delete all regular children of this item (not mask or clip).
void
DrawingItem::clearChildren()
{
    if (_children.empty()) return;

    _markForRendering();
    // prevent children from referencing the parent during deletion
    // this way, children won't try to remove themselves from a list
    // from which they have already been removed by clear_and_dispose
    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        i->_parent = NULL;
        i->_child_type = CHILD_ORPHAN;
    }
    _children.clear_and_dispose(DeleteDisposer());
    _markForUpdate(STATE_ALL, false);
}

/// Set the incremental transform for this item
void
DrawingItem::setTransform(Geom::Affine const &new_trans)
{
    Geom::Affine current;
    if (_transform) {
        current = *_transform;
    }
    
    if (!Geom::are_near(current, new_trans, 1e-18)) {
        // mark the area where the object was for redraw.
        _markForRendering();
        if (new_trans.isIdentity()) {
            delete _transform; // delete NULL; is safe
            _transform = NULL;
        } else {
            _transform = new Geom::Affine(new_trans);
        }
        _markForUpdate(STATE_ALL, true);
    }
}

void
DrawingItem::setOpacity(float opacity)
{
    if (_opacity != opacity) {
        _opacity = opacity;
        _markForRendering();
    }
}

void
DrawingItem::setAntialiasing(bool a)
{
    if (_antialias != a) {
        _antialias = a;
        _markForRendering();
    }
}

void
DrawingItem::setIsolation(unsigned isolation)
{
    _isolation = isolation;
    //if( isolation != 0 ) std::cout << "isolation: " << isolation << std::endl;
    _markForRendering();
}

void
DrawingItem::setBlendMode(unsigned mix_blend_mode)
{
    _mix_blend_mode = mix_blend_mode;
    //if( mix_blend_mode != 0 ) std::cout << "setBlendMode: " << mix_blend_mode << std::endl;
    _markForRendering();
}

void
DrawingItem::setVisible(bool v)
{
    if (_visible != v) {
        _visible = v;
        _markForRendering();
    }
}

/// This is currently unused
void
DrawingItem::setSensitive(bool s)
{
    _sensitive = s;
}

/**
 * Enable / disable storing the rendering in memory.
 * Calling setCached(false, true) will also remove the persistent status
 */
void
DrawingItem::setCached(bool cached, bool persistent)
{
    static const char *cache_env = getenv("_INKSCAPE_DISABLE_CACHE");
    if (cache_env) return;

    if (_cached_persistent && !persistent)
        return;

    _cached = cached;
    _cached_persistent = persistent ? cached : false;
    if (cached) {
        _drawing._cached_items.insert(this);
    } else {
        _drawing._cached_items.erase(this);
        delete _cache;
        _cache = NULL;
    }
}

void
DrawingItem::setClip(DrawingItem *item)
{
    _markForRendering();
    delete _clip;
    _clip = item;
    if (item) {
        item->_parent = this;
        assert(item->_child_type == CHILD_ORPHAN);
        item->_child_type = CHILD_CLIP;
    }
    _markForUpdate(STATE_ALL, true);
}

void
DrawingItem::setMask(DrawingItem *item)
{
    _markForRendering();
    delete _mask;
    _mask = item;
        if (item) {
        item->_parent = this;
        assert(item->_child_type == CHILD_ORPHAN);
        item->_child_type = CHILD_MASK;
    }
    _markForUpdate(STATE_ALL, true);
}

/// Move this item to the given place in the Z order of siblings.
/// Does nothing if the item has no parent.
void
DrawingItem::setZOrder(unsigned z)
{
    if (!_parent) return;

    ChildrenList::iterator it = _parent->_children.iterator_to(*this);
    _parent->_children.erase(it);

    ChildrenList::iterator i = _parent->_children.begin();
    std::advance(i, std::min(z, unsigned(_parent->_children.size())));
    _parent->_children.insert(i, *this);
    _markForRendering();
}

void
DrawingItem::setItemBounds(Geom::OptRect const &bounds)
{
    _item_bbox = bounds;
}

/**
 * Update derived data before operations.
 * The purpose of this call is to recompute internal data which depends
 * on the attributes of the object, but is not directly settable by the user.
 * Precomputing this data speeds up later rendering, because some items
 * can be omitted.
 *
 * Currently this method handles updating the visual and geometric bounding boxes
 * in pixels, storing the total transformation from item space to the screen
 * and cache invalidation.
 *
 * @param area Area to which the update should be restricted. Only takes effect
 *             if the bounding box is known.
 * @param ctx A structure to store cascading state.
 * @param flags Which internal data should be recomputed. This can be any combination
 *              of StateFlags.
 * @param reset State fields that should be reset before processing them. This is
 *              a means to force a recomputation of internal data even if the item
 *              considers it up to date. Mainly for internal use, such as
 *              propagating bounding box recomputation to children when the item's
 *              transform changes.
 */
void
DrawingItem::update(Geom::IntRect const &area, UpdateContext const &ctx, unsigned flags, unsigned reset)
{
    bool render_filters = _drawing.renderFilters();
    bool outline = _drawing.outline();

    // Set reset flags according to propagation status
    reset |= _propagate_state;
    _propagate_state = 0;

    _state &= ~reset; // reset state of this item

    if ((~_state & flags) == 0) return;  // nothing to do

    // TODO this might be wrong
    if (_state & STATE_BBOX) {
        // we have up-to-date bbox
        if (!area.intersects(outline ? _bbox : _drawbox)) return;
    }

    // compute which elements need an update
    unsigned to_update = _state ^ flags;

    // this needs to be called before we recurse into children
    if (to_update & STATE_BACKGROUND) {
        _background_accumulate = _background_new;
        if (_child_type == CHILD_NORMAL && _parent->_background_accumulate)
            _background_accumulate = true;
    }

    UpdateContext child_ctx(ctx);
    if (_transform) {
        child_ctx.ctm = *_transform * ctx.ctm;
    }
    /* Remember the transformation matrix */
    Geom::Affine ctm_change = _ctm.inverse() * child_ctx.ctm;
    _ctm = child_ctx.ctm;

    // update _bbox and call this function for children
    _state = _updateItem(area, child_ctx, flags, reset);

    if (to_update & STATE_BBOX) {
        // compute drawbox
        if (_filter && render_filters) {
            Geom::OptRect enlarged = _filter->filter_effect_area(_item_bbox);
            if (enlarged) {
                *enlarged *= ctm();
                _drawbox = enlarged->roundOutwards();
            } else {
                _drawbox = Geom::OptIntRect();
            }
        } else {
            _drawbox = _bbox;
        }

        // Clipping
        if (_clip) {
            _clip->update(area, child_ctx, flags, reset);
            if (outline) {
                _bbox.unionWith(_clip->_bbox);
            } else {
                _drawbox.intersectWith(_clip->_bbox);
            }
        }
        // Masking
        if (_mask) {
            _mask->update(area, child_ctx, flags, reset);
            if (outline) {
                _bbox.unionWith(_mask->_bbox);
            } else {
                // for masking, we need full drawbox of mask
                _drawbox.intersectWith(_mask->_drawbox);
            }
        }
    }

    if (to_update & STATE_CACHE) {
        // Update cache score for this item
        if (_has_cache_iterator) {
            // remove old score information
            _drawing._candidate_items.erase(_cache_iterator);
            _has_cache_iterator = false;
        }
        double score = _cacheScore();
        if (score >= _drawing._cache_score_threshold) {
            CacheRecord cr;
            cr.score = score;
            // if _cacheRect() is empty, a negative score will be returned from _cacheScore(),
            // so this will not execute (cache score threshold must be positive)
            cr.cache_size = _cacheRect()->area() * 4;
            cr.item = this;
            _drawing._candidate_items.push_front(cr);
            _cache_iterator = _drawing._candidate_items.begin();
            _has_cache_iterator = true;
        }

        /* Update cache if enabled.
         * General note: here we only tell the cache how it has to transform
         * during the render phase. The transformation is deferred because
         * after the update the item can have its caching turned off,
         * e.g. because its filter was removed. This way we avoid tempoerarily
         * using more memory than the cache budget */
        if (_cache) {
            Geom::OptIntRect cl = _cacheRect();
            if (_visible && cl) { // never create cache for invisible items
                // this takes care of invalidation on transform
                _cache->scheduleTransform(*cl, ctm_change);
            } else {
                // Destroy cache for this item - outside of canvas or invisible.
                // The opposite transition (invisible -> visible or object
                // entering the canvas) is handled during the render phase
                delete _cache;
                _cache = NULL;
            }
        }
    }

    if (to_update & STATE_RENDER) {
        // now that we know drawbox, dirty the corresponding rect on canvas
        // unless filtered, groups do not need to render by themselves, only their members
        if (!is_drawing_group(this) || (_filter && render_filters)) {
            _markForRendering();
        }
    }
}

struct MaskLuminanceToAlpha {
    guint32 operator()(guint32 in) {
        guint r = 0, g = 0, b = 0;
        Display::ExtractRGB32(in, r, g, b);
        // the operation of unpremul -> luminance-to-alpha -> multiply by alpha
        // is equivalent to luminance-to-alpha on premultiplied color values
        // original computation in double: r*0.2125 + g*0.7154 + b*0.0721
        guint32 ao = r*109 + g*366 + b*37; // coeffs add up to 512
        return ((ao + 256) << 15) & 0xff000000; // equivalent to ((ao + 256) / 512) << 24
    }
};

/**
 * Rasterize items.
 * This method submits the drawing opeartions required to draw this item
 * to the supplied DrawingContext, restricting drawing the specified area.
 *
 * This method does some common tasks and calls the item-specific rendering
 * function, _renderItem(), to render e.g. paths or bitmaps.
 *
 * @param flags Rendering options. This deals mainly with cache control.
 */
unsigned
DrawingItem::render(DrawingContext &dc, Geom::IntRect const &area, unsigned flags, DrawingItem *stop_at)
{
    bool outline = _drawing.outline();
    bool render_filters = _drawing.renderFilters();

    // stop_at is handled in DrawingGroup, but this check is required to handle the case
    // where a filtered item with background-accessing filter has enable-background: new
    if (this == stop_at) return RENDER_STOP;

    // If we are invisible, return immediately
    if (!_visible) return RENDER_OK;
    if (_ctm.isSingular(1e-18)) return RENDER_OK;

    // TODO convert outline rendering to a separate virtual function
    if (outline) {
        _renderOutline(dc, area, flags);
        return RENDER_OK;
    }

    // carea is the area to paint
    Geom::OptIntRect carea = Geom::intersect(area, _drawbox);
    if (!carea) return RENDER_OK;

    if (_antialias) {
        cairo_set_antialias(dc.raw(), CAIRO_ANTIALIAS_DEFAULT);
    } else {
        cairo_set_antialias(dc.raw(), CAIRO_ANTIALIAS_NONE);
    }

    // render from cache if possible
    if (_cached) {
        if (_cache) {
            _cache->prepare();
            set_cairo_blend_operator( dc, _mix_blend_mode );

            _cache->paintFromCache(dc, carea);
            if (!carea) return RENDER_OK;
        } else {
            // There is no cache. This could be because caching of this item
            // was just turned on after the last update phase, or because
            // we were previously outside of the canvas.
            Geom::OptIntRect cl = _drawing.cacheLimit();
            cl.intersectWith(_drawbox);
            if (cl) {
                _cache = new DrawingCache(*cl);
            }
        }
    } else {
        // if our caching was turned off after the last update, it was already
        // deleted in setCached()
    }

    // determine whether this shape needs intermediate rendering.
    bool needs_intermediate_rendering = false;
    bool &nir = needs_intermediate_rendering;
    bool needs_opacity = (_opacity < 0.995);

    // this item needs an intermediate rendering if:
    nir |= (_clip != NULL); // 1. it has a clipping path
    nir |= (_mask != NULL); // 2. it has a mask
    nir |= (_filter != NULL && render_filters); // 3. it has a filter
    nir |= needs_opacity; // 4. it is non-opaque
    nir |= (_cache != NULL); // 5. it is cached
    nir |= (_mix_blend_mode != SP_CSS_BLEND_NORMAL); // 6. Blend mode not normal
    nir |= (_isolation == SP_CSS_ISOLATION_ISOLATE); // 7. Explicit isolatiom

    /* How the rendering is done.
     *
     * Clipping, masking and opacity are done by rendering them to a surface
     * and then compositing the object's rendering onto it with the IN operator.
     * The object itself is rendered to a group.
     *
     * Opacity is done by rendering the clipping path with an alpha
     * value corresponding to the opacity. If there is no clipping path,
     * the entire intermediate surface is painted with alpha corresponding
     * to the opacity value.
     */

    // Short-circuit the simple case.
    // We also use this path for filter background rendering, because masking, clipping,
    // filters and opacity do not apply when rendering the ancestors of the filtered
    // element
    if ((flags & RENDER_FILTER_BACKGROUND) || !needs_intermediate_rendering) {
        return _renderItem(dc, *carea, flags & ~RENDER_FILTER_BACKGROUND, stop_at);
    }

    // iarea is the bounding box for intermediate rendering
    // Note 1: Pixels inside iarea but outside carea are invalid
    //         (incomplete filter dependence region).
    // Note 2: We only need to render carea of clip and mask, but
    //         iarea of the object.
    Geom::OptIntRect iarea = carea;
    // expand carea to contain the dependent area of filters.
    if (_filter && render_filters) {
        _filter->area_enlarge(*iarea, this);
        iarea.intersectWith(_drawbox);
    }

    DrawingSurface intermediate(*iarea);
    DrawingContext ict(intermediate);
    unsigned render_result = RENDER_OK;

    // 1. Render clipping path with alpha = opacity.
    ict.setSource(0,0,0,_opacity);
    // Since clip can be combined with opacity, the result could be incorrect
    // for overlapping clip children. To fix this we use the SOURCE operator
    // instead of the default OVER.
    ict.setOperator(CAIRO_OPERATOR_SOURCE);
    ict.paint();
    if (_clip) {
        ict.pushGroup();
        _clip->clip(ict, *carea); // fixme: carea or area?
        ict.popGroupToSource();
        ict.setOperator(CAIRO_OPERATOR_IN);
        ict.paint();
    }
    ict.setOperator(CAIRO_OPERATOR_OVER); // reset back to default

    // 2. Render the mask if present and compose it with the clipping path + opacity.
    if (_mask) {
        ict.pushGroup();
        _mask->render(ict, *carea, flags);

        cairo_surface_t *mask_s = ict.rawTarget();
        // Convert mask's luminance to alpha
        ink_cairo_surface_filter(mask_s, mask_s, MaskLuminanceToAlpha());
        ict.popGroupToSource();
        ict.setOperator(CAIRO_OPERATOR_IN);
        ict.paint();
        ict.setOperator(CAIRO_OPERATOR_OVER);
    }

    // 3. Render object itself
    ict.pushGroup();
    render_result = _renderItem(ict, *iarea, flags, stop_at);

    // 4. Apply filter.
    if (_filter && render_filters) {
        bool rendered = false;
        if (_filter->uses_background() && _background_accumulate) {
            DrawingItem *bg_root = this;
            for (; bg_root; bg_root = bg_root->_parent) {
                if (bg_root->_background_new) break;
            }
            if (bg_root) {
                DrawingSurface bg(*iarea);
                DrawingContext bgdc(bg);
                bg_root->render(bgdc, *iarea, flags | RENDER_FILTER_BACKGROUND, this);
                _filter->render(this, ict, &bgdc);
                rendered = true;
            }
        }
        if (!rendered) {
            _filter->render(this, ict, NULL);
        }
        // Note that because the object was rendered to a group,
        // the internals of the filter need to use cairo_get_group_target()
        // instead of cairo_get_target().
    }

    // 5. Render object inside the composited mask + clip
    ict.popGroupToSource();
    ict.setOperator(CAIRO_OPERATOR_IN);
    ict.paint();

    // 6. Paint the completed rendering onto the base context (or into cache)
    if (_cached && _cache) {
        DrawingContext cachect(*_cache);
        cachect.rectangle(*carea);
        cachect.setOperator(CAIRO_OPERATOR_SOURCE);
        cachect.setSource(&intermediate);
        cachect.fill();
        _cache->markClean(*carea);
    }
    dc.rectangle(*carea);
    dc.setSource(&intermediate);
    set_cairo_blend_operator( dc, _mix_blend_mode );
    dc.fill();
    dc.setSource(0,0,0,0);
    // the call above is to clear a ref on the intermediate surface held by dc

    return render_result;
}

void
DrawingItem::_renderOutline(DrawingContext &dc, Geom::IntRect const &area, unsigned flags)
{
    // intersect with bbox rather than drawbox, as we want to render things outside
    // of the clipping path as well
    Geom::OptIntRect carea = Geom::intersect(area, _bbox);
    if (!carea) return;

    // just render everything: item, clip, mask
    // First, render the object itself
    _renderItem(dc, *carea, flags, NULL);

    // render clip and mask, if any
    guint32 saved_rgba = _drawing.outlinecolor; // save current outline color
    // render clippath as an object, using a different color
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (_clip) {
        _drawing.outlinecolor = prefs->getInt("/options/wireframecolors/clips", 0x00ff00ff); // green clips
        _clip->render(dc, *carea, flags);
    }
    // render mask as an object, using a different color
    if (_mask) {
        _drawing.outlinecolor = prefs->getInt("/options/wireframecolors/masks", 0x0000ffff); // blue masks
        _mask->render(dc, *carea, flags);
    }
    _drawing.outlinecolor = saved_rgba; // restore outline color
}

/**
 * Rasterize the clipping path.
 * This method submits drawing operations required to draw a basic filled shape
 * of the item to the supplied drawing context. Rendering is limited to the
 * given area. The rendering of the clipped object is composited into
 * the result of this call using the IN operator. See the implementation
 * of render() for details.
 */
void
DrawingItem::clip(Inkscape::DrawingContext &dc, Geom::IntRect const &area)
{
    // don't bother if the object does not implement clipping (e.g. DrawingImage)
    if (!_canClip()) return;
    if (!_visible) return;
    if (!area.intersects(_bbox)) return;

    dc.setSource(0,0,0,1);
    dc.pushGroup();
    // rasterize the clipping path
    _clipItem(dc, area);
    if (_clip) {
        // The item used as the clipping path itself has a clipping path.
        // Render this item's clipping path onto a temporary surface, then composite it
        // with the item using the IN operator
        dc.pushGroup();
        _clip->clip(dc, area);
        dc.popGroupToSource();
        dc.setOperator(CAIRO_OPERATOR_IN);
        dc.paint();
    }
    dc.popGroupToSource();
    dc.setOperator(CAIRO_OPERATOR_OVER);
    dc.paint();
    dc.setSource(0,0,0,0);
}

/**
 * Get the item under the specified point.
 * Searches the tree for the first item in the Z-order which is closer than
 * @a delta to the given point. The pick should be visual - for example
 * an object with a thick stroke should pick on the entire area of the stroke.
 * @param p Search point
 * @param delta Maximum allowed distance from the point
 * @param sticky Whether the pick should ignore visibility and sensitivity.
 *               When false, only visible and sensitive objects are considered.
 *               When true, invisible and insensitive objects can also be picked.
 */
DrawingItem *
DrawingItem::pick(Geom::Point const &p, double delta, unsigned flags)
{
    // Sometimes there's no BBOX in state, reason unknown (bug 992817)
    // I made this not an assert to remove the warning
    if (!(_state & STATE_BBOX) || !(_state & STATE_PICK)) {
        g_warning("Invalid state when picking: STATE_BBOX = %d, STATE_PICK = %d",
                  _state & STATE_BBOX, _state & STATE_PICK);
        return NULL;
    }
    // ignore invisible and insensitive items unless sticky
    if (!(flags & PICK_STICKY) && !(_visible && _sensitive))
        return NULL;

    bool outline = _drawing.outline();

    if (!_drawing.outline()) {
        // pick inside clipping path; if NULL, it means the object is clipped away there
        if (_clip) {
            DrawingItem *cpick = _clip->pick(p, delta, flags | PICK_AS_CLIP);
            if (!cpick) return NULL;
        }
        // same for mask
        if (_mask) {
            DrawingItem *mpick = _mask->pick(p, delta, flags);
            if (!mpick) return NULL;
        }
    }

    Geom::OptIntRect box = (outline || (flags & PICK_AS_CLIP)) ? _bbox : _drawbox;
    if (!box) {
        return NULL;
    }

    Geom::Rect expanded = *box;
    expanded.expandBy(delta);

    if (expanded.contains(p)) {
        return _pickItem(p, delta, flags);
    }
    return NULL;
}

/**
 * Marks the current visual bounding box of the item for redrawing.
 * This is called whenever the object changes its visible appearance.
 * For some cases (such as setting opacity) this is enough, but for others
 * _markForUpdate() also needs to be called.
 */
void
DrawingItem::_markForRendering()
{
    // TODO: this function does too much work when a large subtree
    // is invalidated - fix
    
    bool outline = _drawing.outline();
    Geom::OptIntRect dirty = outline ? _bbox : _drawbox;
    if (!dirty) return;

    // dirty the caches of all parents
    DrawingItem *bkg_root = NULL;

    for (DrawingItem *i = this; i; i = i->_parent) {
        if (i != this && i->_filter) {
            i->_filter->area_enlarge(*dirty, i);
        }
        if (i->_cache) {
            i->_cache->markDirty(*dirty);
        }
        if (i->_background_accumulate) {
            bkg_root = i;
        }
    }
    
    if (bkg_root) {
        bkg_root->_invalidateFilterBackground(*dirty);
    }
    _drawing.signal_request_render.emit(*dirty);
}

void
DrawingItem::_invalidateFilterBackground(Geom::IntRect const &area)
{
    if (!_drawbox.intersects(area)) return;

    if (_cache && _filter && _filter->uses_background()) {
        _cache->markDirty(area);
    }

    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        i->_invalidateFilterBackground(area);
    }
}

/**
 * Marks the item as needing a recomputation of internal data.
 *
 * This mechanism avoids traversing the entire rendering tree (which could be vast)
 * on every trivial state changed in any item. Only items marked as needing
 * an update (having some bits in their _state unset) will be traversed
 * during the update call.
 *
 * The _propagate variable is another optimization. We use it to specify that
 * all children should also have the corresponding flags unset before checking
 * whether they need to be traversed. This way there is one less traversal
 * of the tree. Without this we would need to unset state bits in all children.
 * With _propagate we do this during the update call, when we have to recurse
 * into children anyway.
 */
void
DrawingItem::_markForUpdate(unsigned flags, bool propagate)
{
    if (propagate) {
        _propagate_state |= flags;
    }

    if (_state & flags) {
        unsigned oldstate = _state;
        _state &= ~flags;
        if (oldstate != _state && _parent) {
            // If we actually reset anything in state, recurse on the parent.
            _parent->_markForUpdate(flags, false);
        } else {
            // If nothing changed, it means our ancestors are already invalidated
            // up to the root. Do not bother recursing, because it won't change anything.
            // Also do this if we are the root item, because we have no more ancestors
            // to invalidate.
            _drawing.signal_request_update.emit(this);
        }
    }
}

/**
 * Process information related to the new style.
 *
 * This function is something of a hack to avoid creating an extra class in the hierarchy
 * which would differ from DrawingItem only by having a _style member.
 * This is mainly to the benefit of DrawingGlyphs, which use the style of their parent.
 * This should probably be refactored some day, possibly by creating the relevant class
 * or creating a more complex data model in DrawingText and removing DrawingGlyphs,
 * which would cause every item to have a style.
 */
void
DrawingItem::_setStyleCommon(SPStyle *&_style, SPStyle *style)
{
    if (style) sp_style_ref(style);
    if (_style) sp_style_unref(_style);
    _style = style;

    if (style->filter.set && style->getFilter()) {
        if (!_filter) {
            int primitives = sp_filter_primitive_count(SP_FILTER(style->getFilter()));
            _filter = new Inkscape::Filters::Filter(primitives);
        }
        sp_filter_build_renderer(SP_FILTER(style->getFilter()), _filter);
    } else {
        // no filter set for this group
        delete _filter;
        _filter = NULL;
    }

    if (style && style->enable_background.set) {
        if (style->enable_background.value == SP_CSS_BACKGROUND_NEW && !_background_new) {
            _background_new = true;
            _markForUpdate(STATE_BACKGROUND, true);
        } else if (style->enable_background.value == SP_CSS_BACKGROUND_ACCUMULATE && _background_new) {
            _background_new = false;
            _markForUpdate(STATE_BACKGROUND, true);
        }
    }

    _markForUpdate(STATE_ALL, false);
}

/**
 * Compute the caching score.
 *
 * Higher scores mean the item is more aggresively prioritized for automatic
 * caching by Inkscape::Drawing.
 */
double
DrawingItem::_cacheScore()
{
    Geom::OptIntRect cache_rect = _cacheRect();
    if (!cache_rect) return -1.0;

    // a crude first approximation:
    // the basic score is the number of pixels in the drawbox
    double score = cache_rect->area();
    // this is multiplied by the filter complexity and its expansion
    if (_filter &&_drawing.renderFilters()) {
        score *= _filter->complexity(_ctm);
        Geom::IntRect ref_area = Geom::IntRect::from_xywh(0, 0, 16, 16);
        Geom::IntRect test_area = ref_area;
        Geom::IntRect limit_area(0, INT_MIN, 16, INT_MAX);
        _filter->area_enlarge(test_area, this);
        // area_enlarge never shrinks the rect, so the result of intersection below
        // must be non-empty
        score *= double((test_area & limit_area)->area()) / ref_area.area();
    }
    // if the object is clipped, add 1/2 of its bbox pixels
    if (_clip && _clip->_bbox) {
        score += _clip->_bbox->area() * 0.5;
    }
    // if masked, add mask score
    if (_mask) {
        score += _mask->_cacheScore();
    }
    //g_message("caching score: %f", score);
    return score;
}

Geom::OptIntRect
DrawingItem::_cacheRect()
{
    Geom::OptIntRect r = _drawbox & _drawing.cacheLimit();
    return r;
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

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

#ifndef SEEN_INKSCAPE_DISPLAY_DRAWING_ITEM_H
#define SEEN_INKSCAPE_DISPLAY_DRAWING_ITEM_H

#include <2geom/rect.h>
#include <2geom/affine.h>
#include <boost/operators.hpp>
#include <boost/utility.hpp>
#include <boost/intrusive/list.hpp>
#include <exception>
#include <list>

namespace Glib {
class ustring;
}

class SPStyle;

namespace Inkscape {

class Drawing;
class DrawingCache;
class DrawingContext;
class DrawingItem;
class DrawingPattern;

namespace Filters {

class Filter;

} // namespace Filters



struct UpdateContext {
    Geom::Affine ctm;
};

struct CacheRecord
    : boost::totally_ordered<CacheRecord>
{
    bool operator<(CacheRecord const &other) const { return score < other.score; }
    bool operator==(CacheRecord const &other) const { return score == other.score; }
    operator DrawingItem *() const { return item; }
    double score;
    size_t cache_size;
    DrawingItem *item;
};
typedef std::list<CacheRecord> CacheList;

class InvalidItemException : public std::exception {
    virtual const char *what() const throw() {
        return "Invalid item in drawing";
    }
};

class DrawingItem
    : boost::noncopyable
{
public:
    enum RenderFlags {
        RENDER_DEFAULT = 0,
        RENDER_CACHE_ONLY = 1,
        RENDER_BYPASS_CACHE = 2,
        RENDER_FILTER_BACKGROUND = 4
    };
    enum StateFlags {
        STATE_NONE = 0,
        STATE_BBOX = (1<<0),    // bounding boxes are up-to-date
        STATE_CACHE = (1<<1),   // cache extents and clean area are up-to-date
        STATE_PICK = (1<<2),    // can process pick requests
        STATE_RENDER = (1<<3),  // can be rendered
        STATE_BACKGROUND = (1<<4), // filter background data is up to date
        STATE_ALL = (1<<5)-1
    };
    enum PickFlags {
        PICK_NORMAL = 0, // normal pick
        PICK_STICKY = (1<<0), // sticky pick - ignore visibility and sensitivity
        PICK_AS_CLIP = (1<<2) // pick with no stroke and opaque fill regardless of item style
    };

    DrawingItem(Drawing &drawing);
    virtual ~DrawingItem();

    Geom::OptIntRect geometricBounds() const { return _bbox; }
    Geom::OptIntRect visualBounds() const { return _drawbox; }
    Geom::OptRect itemBounds() const { return _item_bbox; }
    Geom::Affine ctm() const { return _ctm; }
    Geom::Affine transform() const { return _transform ? *_transform : Geom::identity(); }
    Drawing &drawing() const { return _drawing; }
    DrawingItem *parent() const;
    bool isAncestorOf(DrawingItem *item) const;

    void appendChild(DrawingItem *item);
    void prependChild(DrawingItem *item);
    void clearChildren();

    bool visible() const { return _visible; }
    void setVisible(bool v);
    bool sensitive() const { return _sensitive; }
    void setSensitive(bool v);
    bool cached() const { return _cached; }
    void setCached(bool c, bool persistent = false);

    virtual void setStyle(SPStyle *style, SPStyle *context_style = NULL);
    virtual void setChildrenStyle(SPStyle *context_style);
    void setOpacity(float opacity);
    void setAntialiasing(bool a);
    void setIsolation(unsigned isolation); // CSS Compositing and Blending
    void setBlendMode(unsigned blend_mode);
    void setTransform(Geom::Affine const &trans);
    void setClip(DrawingItem *item);
    void setMask(DrawingItem *item);
    void setFillPattern(DrawingPattern *pattern);
    void setStrokePattern(DrawingPattern *pattern);
    void setZOrder(unsigned z);
    void setItemBounds(Geom::OptRect const &bounds);
    void setFilterBounds(Geom::OptRect const &bounds);

    void setKey(unsigned key) { _key = key; }
    unsigned key() const { return _key; }
    void setData(void *data) { _user_data = data; }
    void *data() const { return _user_data; }

    void update(Geom::IntRect const &area = Geom::IntRect::infinite(), UpdateContext const &ctx = UpdateContext(), unsigned flags = STATE_ALL, unsigned reset = 0);
    unsigned render(DrawingContext &dc, Geom::IntRect const &area, unsigned flags = 0, DrawingItem *stop_at = NULL);
    void clip(DrawingContext &dc, Geom::IntRect const &area);
    DrawingItem *pick(Geom::Point const &p, double delta, unsigned flags = 0);

    virtual Glib::ustring name(); // For debugging
    void recursivePrintTree(unsigned level = 0);  // For debugging

protected:
    enum ChildType {
        CHILD_ORPHAN = 0, // no parent - implies _parent == NULL
        CHILD_NORMAL = 1, // contained in _children of parent
        CHILD_CLIP = 2, // referenced by _clip member of parent
        CHILD_MASK = 3, // referenced by _mask member of parent
        CHILD_ROOT = 4, // root item of _drawing
        CHILD_FILL_PATTERN = 5, // referenced by fill pattern of parent
        CHILD_STROKE_PATTERN = 6 // referenced by stroke pattern of parent
    };
    enum RenderResult {
        RENDER_OK = 0,
        RENDER_STOP = 1
    };
    void _renderOutline(DrawingContext &dc, Geom::IntRect const &area, unsigned flags);
    void _markForUpdate(unsigned state, bool propagate);
    void _markForRendering();
    void _invalidateFilterBackground(Geom::IntRect const &area);
    double _cacheScore();
    Geom::OptIntRect _cacheRect();
    virtual unsigned _updateItem(Geom::IntRect const &/*area*/, UpdateContext const &/*ctx*/,
                                 unsigned /*flags*/, unsigned /*reset*/) { return 0; }
    virtual unsigned _renderItem(DrawingContext &/*dc*/, Geom::IntRect const &/*area*/, unsigned /*flags*/,
                                 DrawingItem * /*stop_at*/) { return RENDER_OK; }
    virtual void _clipItem(DrawingContext &/*dc*/, Geom::IntRect const &/*area*/) {}
    virtual DrawingItem *_pickItem(Geom::Point const &/*p*/, double /*delta*/, unsigned /*flags*/) { return NULL; }
    virtual bool _canClip() { return false; }

    // member variables start here

    Drawing &_drawing;
    DrawingItem *_parent;

    typedef boost::intrusive::list_member_hook<> ListHook;
    ListHook _child_hook;

    typedef boost::intrusive::list<
        DrawingItem,
        boost::intrusive::member_hook<DrawingItem, ListHook, &DrawingItem::_child_hook>
        > ChildrenList;
    ChildrenList _children;

    unsigned _key; ///< Some SPItems can have more than one DrawingItem;
                   ///  this value is a hack used to distinguish between them
    SPStyle *_style; // Not used by DrawingGlyphs
    SPStyle *_context_style; // Used for 'context-fill', 'context-stroke'
    
    float _opacity;

    Geom::Affine *_transform; ///< Incremental transform from parent to this item's coords
    Geom::Affine _ctm; ///< Total transform from item coords to display coords
    Geom::OptIntRect _bbox; ///< Bounding box in display (pixel) coords including stroke
    Geom::OptIntRect _drawbox; ///< Full visual bounding box - enlarged by filters, shrunk by clips and masks
    Geom::OptRect _item_bbox; ///< Geometric bounding box in item's user space.
                              ///  This is used to compute the filter effect region and render in
                              ///  objectBoundingBox units.

    DrawingItem *_clip;
    DrawingItem *_mask;
    DrawingPattern *_fill_pattern;
    DrawingPattern *_stroke_pattern;
    Inkscape::Filters::Filter *_filter;
    void *_user_data; ///< Used to associate DrawingItems with SPItems that created them
    DrawingCache *_cache;

    CacheList::iterator _cache_iterator;

    unsigned _state : 8;
    unsigned _propagate_state : 8;
    unsigned _child_type : 3; // see ChildType enum
    unsigned _background_new : 1; ///< Whether enable-background: new is set for this element
    unsigned _background_accumulate : 1; ///< Whether this element accumulates background 
                                         ///  (has any ancestor with enable-background: new)
    unsigned _visible : 1;
    unsigned _sensitive : 1; ///< Whether this item responds to events
    unsigned _cached : 1; ///< Whether the rendering is stored for reuse
    unsigned _cached_persistent : 1; ///< If set, will always be cached regardless of score
    unsigned _has_cache_iterator : 1; ///< If set, _cache_iterator is valid
    unsigned _propagate : 1; ///< Whether to call update for all children on next update
    //unsigned _renders_opacity : 1; ///< Whether object needs temporary surface for opacity
    unsigned _pick_children : 1; ///< For groups: if true, children are returned from pick(),
                                 ///  otherwise the group is returned
    unsigned _antialias : 1; ///< Whether to use antialiasing

    unsigned _isolation : 1;
    unsigned _mix_blend_mode : 4;

    friend class Drawing;
};

struct DeleteDisposer {
    void operator()(DrawingItem *item) { delete item; }
};

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

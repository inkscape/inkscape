#ifndef SEEN_NR_FILTER_SLOT_H
#define SEEN_NR_FILTER_SLOT_H

/*
 * A container class for filter slots. Allows for simple getting and
 * setting images in filter slots without having to bother with
 * table indexes and such.
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include "display/nr-filter-types.h"
#include "display/nr-filter-units.h"

extern "C" {
typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
}

namespace Inkscape {
class DrawingContext;
class DrawingItem;

namespace Filters {

class FilterSlot {
public:
    /** Creates a new FilterSlot object. */
    FilterSlot(DrawingItem *item, DrawingContext *bgdc,
        DrawingContext &graphic, FilterUnits const &u);
    /** Destroys the FilterSlot object and all its contents */
    virtual ~FilterSlot();

    /** Returns the pixblock in specified slot.
     * Parameter 'slot' may be either an positive integer or one of
     * pre-defined filter slot types: NR_FILTER_SLOT_NOT_SET,
     * NR_FILTER_SOURCEGRAPHIC, NR_FILTER_SOURCEALPHA,
     * NR_FILTER_BACKGROUNDIMAGE, NR_FILTER_BACKGROUNDALPHA,
     * NR_FILTER_FILLPAINT, NR_FILTER_SOURCEPAINT.
     */
    cairo_surface_t *getcairo(int slot);

    /** Sets or re-sets the pixblock associated with given slot.
     * If there was a pixblock already assigned with this slot,
     * that pixblock is destroyed.
     */
    void set(int slot, cairo_surface_t *s);

    cairo_surface_t *get_result(int slot_nr);

    void set_primitive_area(int slot, Geom::Rect &area);
    Geom::Rect get_primitive_area(int slot);
    
    /** Returns the number of slots in use. */
    int get_slot_count();

    /** Sets the unit system to be used for the internal images. */
    //void set_units(FilterUnits const &units);

    /** Sets the filtering quality. Affects used interpolation methods */
    void set_quality(FilterQuality const q);

    /** Sets the gaussian filtering quality. Affects used interpolation methods */
    void set_blurquality(int const q);

    /** Gets the gaussian filtering quality. Affects used interpolation methods */
    int get_blurquality(void);

    FilterUnits const &get_units() const { return _units; }
    Geom::Rect get_slot_area() const;

private:
    typedef std::map<int, cairo_surface_t *> SlotMap;
    SlotMap _slots;

    // We need to keep track of the primitive area as this is needed in feTile
    typedef std::map<int, Geom::Rect> PrimitiveAreaMap;
    PrimitiveAreaMap _primitiveAreas;

    DrawingItem *_item;

    //Geom::Rect _source_bbox; ///< bounding box of source graphic surface
    //Geom::Rect _intermediate_bbox; ///< bounding box of intermediate surfaces

    int _slot_w, _slot_h;
    double _slot_x, _slot_y;
    cairo_surface_t *_source_graphic;
    cairo_t *_background_ct;
    Geom::IntRect _source_graphic_area;
    Geom::IntRect _background_area; ///< needed to extract background
    FilterUnits const &_units;
    int _last_out;
    FilterQuality filterquality;
    int blurquality;

    cairo_surface_t *_get_transformed_source_graphic();
    cairo_surface_t *_get_transformed_background();
    cairo_surface_t *_get_fill_paint();
    cairo_surface_t *_get_stroke_paint();

    void _set_internal(int slot, cairo_surface_t *s);
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif // __NR_FILTER_SLOT_H__
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

#ifndef __SELTRANS_H__
#define __SELTRANS_H__

/*
 * Helper object for transforming selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/sigc++.h>
#include <libnr/nr-point.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include "knot.h"
#include "forward.h"
#include "selcue.h"
#include "message-context.h"
#include <vector>

struct SPKnot;
class SPDesktop;
class SPCanvasItem;
class SPSelTransHandle;

namespace Inkscape
{

NR::scale calcScaleFactors(NR::Point const &initial_point, NR::Point const &new_point, NR::Point const &origin, bool const skew = false);

namespace XML
{
  class Node;
}

class SelTrans
{
public:
    SelTrans(SPDesktop *desktop);
    ~SelTrans();

    Inkscape::MessageContext &messageContext() {
        return _message_context;
    }

    void increaseState();
    void resetState();
    void setCenter(NR::Point const &p);
    void grab(NR::Point const &p, gdouble x, gdouble y, bool show_handles);
    void transform(NR::Matrix const &rel_affine, NR::Point const &norm);
    void ungrab();
    void stamp();
    void moveTo(NR::Point const &xy, guint state);
    void stretch(SPSelTransHandle const &handle, NR::Point &pt, guint state);
    void scale(NR::Point &pt, guint state);
    void skew(SPSelTransHandle const &handle, NR::Point &pt, guint state);
    void rotate(NR::Point &pt, guint state);
    gboolean scaleRequest(NR::Point &pt, guint state);
    gboolean stretchRequest(SPSelTransHandle const &handle, NR::Point &pt, guint state);
    gboolean skewRequest(SPSelTransHandle const &handle, NR::Point &pt, guint state);
    gboolean rotateRequest(NR::Point &pt, guint state);
    gboolean centerRequest(NR::Point &pt, guint state);

    gboolean handleRequest(SPKnot *knot, NR::Point *position, guint state, SPSelTransHandle const &handle);
    void handleGrab(SPKnot *knot, guint state, SPSelTransHandle const &handle);
    void handleClick(SPKnot *knot, guint state, SPSelTransHandle const &handle);
    void handleNewEvent(SPKnot *knot, NR::Point *position, guint state, SPSelTransHandle const &handle);

    enum Show
    {
        SHOW_CONTENT,
        SHOW_OUTLINE
    };

    void setShow(Show s) {
        _show = s;
    }
    bool isEmpty() {
        return _empty;
    }
    bool isGrabbed() {
        return _grabbed;    
    }
	bool centerIsVisible() {
		return ( _chandle && SP_KNOT_IS_VISIBLE (_chandle) );
	}

private:
    void _updateHandles();
    void _updateVolatileState();
    void _selChanged(Inkscape::Selection *selection);
    void _selModified(Inkscape::Selection *selection, guint flags);
    void _showHandles(SPKnot *knot[], SPSelTransHandle const handle[], gint num,
                      gchar const *even_tip, gchar const *odd_tip);
    NR::Point _getGeomHandlePos(NR::Point const &visual_handle_pos);
    NR::Point _calcAbsAffineDefault(NR::scale const default_scale);
    NR::Point _calcAbsAffineGeom(NR::scale const geom_scale);

    enum State {
        STATE_SCALE, //scale or stretch
        STATE_ROTATE //rotate or skew
    };
    
    SPDesktop *_desktop;

    std::vector<std::pair<SPItem *, NR::Matrix> > _items;
    std::vector<std::pair<SPItem *, NR::Point> > _items_centers;
    
    std::vector<NR::Point> _snap_points;
    std::vector<NR::Point> _bbox_points;
    
    Inkscape::SelCue _selcue;

    Inkscape::Selection *_selection;
    State _state;
    Show _show;

    bool _grabbed;
    bool _show_handles;
    bool _empty;
    bool _changed;

    SPItem::BBoxType _snap_bbox_type;
    
    NR::Maybe<NR::Rect> _bbox;
    NR::Maybe<NR::Rect> _approximate_bbox;
    NR::Maybe<NR::Rect> _geometric_bbox;
    gdouble _strokewidth;
    
    NR::Matrix _current_relative_affine;
    NR::Matrix _absolute_affine;
    NR::Matrix _relative_affine;
    /* According to Merriam - Webster's online dictionary
     * Affine: a transformation (as a translation, a rotation, or a uniform stretching) that carries straight
     * lines into straight lines and parallel lines into parallel lines but may alter distance between points
     * and angles between lines <affine geometry>
     */
    
    NR::Point _opposite; ///< opposite point to where a scale is taking place    
    NR::Point _opposite_for_specpoints;
    NR::Point _opposite_for_bboxpoints;
    NR::Point _origin_for_specpoints;
    NR::Point _origin_for_bboxpoints;

    gdouble _handle_x;
    gdouble _handle_y;

    NR::Maybe<NR::Point> _center;
    bool _center_is_set; ///< we've already set _center, no need to reread it from items

    SPKnot *_shandle[8];
    SPKnot *_rhandle[8];
    SPKnot *_chandle;
    SPCanvasItem *_norm;
    SPCanvasItem *_grip;
    SPCanvasItem *_l[4];
    guint _sel_changed_id;
    guint _sel_modified_id;
    GSList *_stamp_cache;

    NR::Point _origin; ///< position of origin for transforms
    NR::Point _point; ///< original position of the knot being used for the current transform
    NR::Point _point_geom; ///< original position of the knot being used for the current transform
    Inkscape::MessageContext _message_context;
    sigc::connection _sel_changed_connection;
    sigc::connection _sel_modified_connection;
};

}

#endif


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

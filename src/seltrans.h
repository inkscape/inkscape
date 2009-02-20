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
#include <2geom/point.h>
#include <2geom/matrix.h>
#include <2geom/rect.h>
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

Geom::Scale calcScaleFactors(Geom::Point const &initial_point, Geom::Point const &new_point, Geom::Point const &origin, bool const skew = false);

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
    void setCenter(Geom::Point const &p);
    void grab(Geom::Point const &p, gdouble x, gdouble y, bool show_handles);
    void transform(Geom::Matrix const &rel_affine, Geom::Point const &norm);
    void ungrab();
    void stamp();
    void moveTo(Geom::Point const &xy, guint state);
    void stretch(SPSelTransHandle const &handle, Geom::Point &pt, guint state);
    void scale(Geom::Point &pt, guint state);
    void skew(SPSelTransHandle const &handle, Geom::Point &pt, guint state);
    void rotate(Geom::Point &pt, guint state);
    gboolean scaleRequest(Geom::Point &pt, guint state);
    gboolean stretchRequest(SPSelTransHandle const &handle, Geom::Point &pt, guint state);
    gboolean skewRequest(SPSelTransHandle const &handle, Geom::Point &pt, guint state);
    gboolean rotateRequest(Geom::Point &pt, guint state);
    gboolean centerRequest(Geom::Point &pt, guint state);

    gboolean handleRequest(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const &handle);
    void handleGrab(SPKnot *knot, guint state, SPSelTransHandle const &handle);
    void handleClick(SPKnot *knot, guint state, SPSelTransHandle const &handle);
    void handleNewEvent(SPKnot *knot, Geom::Point *position, guint state, SPSelTransHandle const &handle);

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
    Geom::Point _getGeomHandlePos(Geom::Point const &visual_handle_pos);
    Geom::Point _calcAbsAffineDefault(Geom::Scale const default_scale);
    Geom::Point _calcAbsAffineGeom(Geom::Scale const geom_scale);
    void _keepClosestPointOnly(std::vector<std::pair<Geom::Point, int> > &points, const Geom::Point &reference);
    void _display_snapsource();

    enum State {
        STATE_SCALE, //scale or stretch
        STATE_ROTATE //rotate or skew
    };

    SPDesktop *_desktop;

    std::vector<SPItem *> _items;
    std::vector<SPItem const *> _items_const;
    std::vector<Geom::Matrix> _items_affines;
    std::vector<Geom::Point> _items_centers;

    std::vector<std::pair<Geom::Point, int> > _snap_points;
    std::vector<std::pair<Geom::Point, int> > _bbox_points;

    Inkscape::SelCue _selcue;

    Inkscape::Selection *_selection;
    State _state;
    Show _show;

    bool _grabbed;
    bool _show_handles;
    bool _empty;
    bool _changed;

    SPItem::BBoxType _snap_bbox_type;

    Geom::OptRect _bbox;
    Geom::OptRect _approximate_bbox;
    Geom::OptRect _geometric_bbox;
    gdouble _strokewidth;

    Geom::Matrix _current_relative_affine;
    Geom::Matrix _absolute_affine;
    Geom::Matrix _relative_affine;
    /* According to Merriam - Webster's online dictionary
     * Affine: a transformation (as a translation, a rotation, or a uniform stretching) that carries straight
     * lines into straight lines and parallel lines into parallel lines but may alter distance between points
     * and angles between lines <affine geometry>
     */

    Geom::Point _opposite; ///< opposite point to where a scale is taking place
    Geom::Point _opposite_for_specpoints;
    Geom::Point _opposite_for_bboxpoints;
    Geom::Point _origin_for_specpoints;
    Geom::Point _origin_for_bboxpoints;

    gdouble _handle_x;
    gdouble _handle_y;

    boost::optional<Geom::Point> _center;
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

    Geom::Point _origin; ///< position of origin for transforms
    Geom::Point _point; ///< original position of the knot being used for the current transform
    Geom::Point _point_geom; ///< original position of the knot being used for the current transform
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

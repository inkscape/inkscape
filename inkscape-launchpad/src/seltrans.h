#ifndef SEEN_SELTRANS_H
#define SEEN_SELTRANS_H

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

#include <2geom/point.h>
#include <2geom/affine.h>
#include <2geom/rect.h>
#include <cstddef>
#include <sigc++/sigc++.h>
#include <vector>

#include "knot.h"
#include "message-context.h"
#include "seltrans-handles.h"
#include "selcue.h"
#include "sp-item.h"


class  SPKnot;
class  SPDesktop;
struct SPCanvasItem;
struct SPCtrlLine;
struct SPSelTransHandle;

namespace Inkscape {

Geom::Scale calcScaleFactors(Geom::Point const &initial_point, Geom::Point const &new_point, Geom::Point const &origin, bool const skew = false);

namespace XML {
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
    void grab(Geom::Point const &p, double x, double y, bool show_handles, bool translating);
    void transform(Geom::Affine const &rel_affine, Geom::Point const &norm);
    void ungrab();
    void stamp();
    void moveTo(Geom::Point const &xy, unsigned int state);
    void stretch(SPSelTransHandle const &handle, Geom::Point &pt, unsigned int state);
    void scale(Geom::Point &pt, unsigned int state);
    void skew(SPSelTransHandle const &handle, Geom::Point &pt, unsigned int state);
    void rotate(Geom::Point &pt, unsigned int state);
    int request(SPSelTransHandle const &handle, Geom::Point &pt, unsigned int state);
    int scaleRequest(Geom::Point &pt, unsigned int state);
    int stretchRequest(SPSelTransHandle const &handle, Geom::Point &pt, unsigned int state);
    int skewRequest(SPSelTransHandle const &handle, Geom::Point &pt, unsigned int state);
    int rotateRequest(Geom::Point &pt, unsigned int state);
    int centerRequest(Geom::Point &pt, unsigned int state);

    int handleRequest(SPKnot *knot, Geom::Point *position, unsigned int state, SPSelTransHandle const &handle);
    void handleGrab(SPKnot *knot, unsigned int state, SPSelTransHandle const &handle);
    void handleClick(SPKnot *knot, unsigned int state, SPSelTransHandle const &handle);
    void handleNewEvent(SPKnot *knot, Geom::Point *position, unsigned int state, SPSelTransHandle const &handle);

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
        return ( SP_KNOT_IS_VISIBLE (knots[0]) );
    }

    void getNextClosestPoint(bool reverse);

private:
    class BoundingBoxPrefsObserver: public Preferences::Observer
    {
    public:
        BoundingBoxPrefsObserver(SelTrans &sel_trans);

        void notify(Preferences::Entry const &val);

    private:
        SelTrans &_sel_trans;
    };

    friend class Inkscape::SelTrans::BoundingBoxPrefsObserver;

    void _updateHandles();
    void _updateVolatileState();
    void _selChanged(Inkscape::Selection *selection);
    void _selModified(Inkscape::Selection *selection, unsigned int flags);
    void _boundingBoxPrefsChanged(int prefs_bbox);
    void _makeHandles();
    void _showHandles(SPSelTransType type);
    Geom::Point _getGeomHandlePos(Geom::Point const &visual_handle_pos);
    Geom::Point _calcAbsAffineDefault(Geom::Scale const default_scale);
    Geom::Point _calcAbsAffineGeom(Geom::Scale const geom_scale);
    void _keepClosestPointOnly(Geom::Point const &p);

    enum State {
        STATE_SCALE, //scale or stretch
        STATE_ROTATE //rotate or skew
    };

    SPDesktop *_desktop;

    std::vector<SPItem *> _items;
    std::vector<SPItem const *> _items_const;
    std::vector<Geom::Affine> _items_affines;
    std::vector<Geom::Point> _items_centers;

    std::vector<Inkscape::SnapCandidatePoint> _snap_points;
    std::vector<Inkscape::SnapCandidatePoint> _bbox_points;
    std::vector<Inkscape::SnapCandidatePoint> _all_snap_sources_sorted;
    std::vector<Inkscape::SnapCandidatePoint>::iterator _all_snap_sources_iter;
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
    Geom::OptRect _visual_bbox;
    Geom::OptRect _geometric_bbox;
    double _strokewidth;

    Geom::Affine _current_relative_affine;
    Geom::Affine _absolute_affine;
    Geom::Affine _relative_affine;
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

    double _handle_x;
    double _handle_y;

    boost::optional<Geom::Point> _center;
    bool _center_is_set; ///< we've already set _center, no need to reread it from items
    int  _center_handle;

    SPKnot *knots[NUMHANDS];
    SPCanvasItem *_norm;
    SPCanvasItem *_grip;
    SPCtrlLine *_l[4];
    unsigned int _sel_changed_id;
    unsigned int _sel_modified_id;
    std::vector<SPItem*> _stamp_cache;

    Geom::Point _origin; ///< position of origin for transforms
    Geom::Point _point; ///< original position of the knot being used for the current transform
    Geom::Point _point_geom; ///< original position of the knot being used for the current transform
    Inkscape::MessageContext _message_context;
    sigc::connection _sel_changed_connection;
    sigc::connection _sel_modified_connection;
    BoundingBoxPrefsObserver _bounding_box_prefs_observer;
};

}

#endif // SEEN_SELTRANS_H


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

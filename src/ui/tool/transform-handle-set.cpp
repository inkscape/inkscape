/** @file
 * Affine transform handles component
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <algorithm>
#include "control-point.h"
#include <glib/gi18n.h>
#include <2geom/transforms.h>
#include "desktop.h"

#include "display/sodipodi-ctrlrect.h"
#include "preferences.h"
#include "snap.h"
#include "snap-candidate.h"
#include "sp-namedview.h"
#include "ui/tool/commit-events.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/selectable-control-point.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/transform-handle-set.h"
#include "ui/tools/node-tool.h"
#include "ui/tool/node.h"
#include "seltrans.h"

// FIXME BRAIN DAMAGE WARNING: this is a global variable in select-context.cpp
// It should be moved to a header
extern GdkPixbuf *handles[];
GType sp_select_context_get_type();

namespace Inkscape {
namespace UI {

namespace {

SPAnchorType corner_to_anchor(unsigned c) {
    switch (c % 4) {
    case 0: return SP_ANCHOR_NE;
    case 1: return SP_ANCHOR_NW;
    case 2: return SP_ANCHOR_SW;
    default: return SP_ANCHOR_SE;
    }
}

SPAnchorType side_to_anchor(unsigned s) {
    switch (s % 4) {
    case 0: return SP_ANCHOR_N;
    case 1: return SP_ANCHOR_W;
    case 2: return SP_ANCHOR_S;
    default: return SP_ANCHOR_E;
    }
}

// TODO move those two functions into a common place
double snap_angle(double a) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
    double unit_angle = M_PI / snaps;
    return CLAMP(unit_angle * round(a / unit_angle), -M_PI, M_PI);
}

double snap_increment_degrees() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getIntLimited("/options/rotationsnapsperpi/value", 12, 1, 1000);
    return 180.0 / snaps;
}

} // anonymous namespace

ControlPoint::ColorSet TransformHandle::thandle_cset = {
    {0x000000ff, 0x000000ff},
    {0x00ff6600, 0x000000ff},
    {0x00ff6600, 0x000000ff},
    //
    {0x000000ff, 0x000000ff},
    {0x00ff6600, 0x000000ff},
    {0x00ff6600, 0x000000ff}
};

TransformHandle::TransformHandle(TransformHandleSet &th, SPAnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pb) :
    ControlPoint(th._desktop, Geom::Point(), anchor,
                 pb,
                 thandle_cset, th._transform_handle_group),
    _th(th)
{
    setVisible(false);
}

void TransformHandle::getNextClosestPoint(bool reverse)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/snapclosestonly/value", false)) {
        if (!_all_snap_sources_sorted.empty()) {
            if (reverse) { // Shift-tab will find a closer point
                if (_all_snap_sources_iter == _all_snap_sources_sorted.begin()) {
                    _all_snap_sources_iter = _all_snap_sources_sorted.end();
                }
                --_all_snap_sources_iter;
            } else { // Tab will find a point further away
                ++_all_snap_sources_iter;
                if (_all_snap_sources_iter == _all_snap_sources_sorted.end()) {
                    _all_snap_sources_iter = _all_snap_sources_sorted.begin();
                }
            }

            _snap_points.clear();
            _snap_points.push_back(*_all_snap_sources_iter);

        }
    }
}

bool TransformHandle::grabbed(GdkEventMotion *)
{
    _origin = position();
    _last_transform.setIdentity();
    startTransform();

    _th._setActiveHandle(this);
    _setLurking(true);
    _setState(_state);

    // Collect the snap-candidates, one for each selected node. These will be stored in the _snap_points vector.
    Inkscape::UI::Tools::NodeTool *nt = INK_NODE_TOOL(_th._desktop->event_context);
    //ControlPointSelection *selection = nt->_selected_nodes.get();
    ControlPointSelection* selection = nt->_selected_nodes;

    selection->setOriginalPoints();
    selection->getOriginalPoints(_snap_points);
    selection->getUnselectedPoints(_unselected_points);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/snapclosestonly/value", false)) {
        // Find the closest snap source candidate
        _all_snap_sources_sorted = _snap_points;

        // Calculate and store the distance to the reference point for each snap candidate point
        for(std::vector<Inkscape::SnapCandidatePoint>::iterator i = _all_snap_sources_sorted.begin(); i != _all_snap_sources_sorted.end(); ++i) {
            (*i).setDistance(Geom::L2((*i).getPoint() - _origin));
        }

        // Sort them ascending, using the distance calculated above as the single criteria
        std::sort(_all_snap_sources_sorted.begin(), _all_snap_sources_sorted.end());

        // Now get the closest snap source
        _snap_points.clear();
        if (!_all_snap_sources_sorted.empty()) {
            _all_snap_sources_iter = _all_snap_sources_sorted.begin();
            _snap_points.push_back(_all_snap_sources_sorted.front());
        }
    }

    return false;
}

void TransformHandle::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    Geom::Affine t = computeTransform(new_pos, event);
    // protect against degeneracies
    if (t.isSingular()) return;
    Geom::Affine incr = _last_transform.inverse() * t;
    if (incr.isSingular()) return;
    _th.signal_transform.emit(incr);
    _last_transform = t;
}

void TransformHandle::ungrabbed(GdkEventButton *)
{
    _snap_points.clear();
    _th._clearActiveHandle();
    _setLurking(false);
    _setState(_state);
    endTransform();
    _th.signal_commit.emit(getCommitEvent());
}


class ScaleHandle : public TransformHandle {
public:
    ScaleHandle(TransformHandleSet &th, SPAnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pb)
        : TransformHandle(th, anchor, pb)
    {}
protected:
    virtual Glib::ustring _getTip(unsigned state) const {
        if (state_held_control(state)) {
            if (state_held_shift(state)) {
                return C_("Transform handle tip",
                    "<b>Shift+Ctrl</b>: scale uniformly about the rotation center");
            }
            return C_("Transform handle tip", "<b>Ctrl:</b> scale uniformly");
        }
        if (state_held_shift(state)) {
            if (state_held_alt(state)) {
                return C_("Transform handle tip",
                    "<b>Shift+Alt</b>: scale using an integer ratio about the rotation center");
            }
            return C_("Transform handle tip", "<b>Shift</b>: scale from the rotation center");
        }
        if (state_held_alt(state)) {
            return C_("Transform handle tip", "<b>Alt</b>: scale using an integer ratio");
        }
        return C_("Transform handle tip", "<b>Scale handle</b>: drag to scale the selection");
    }

    virtual Glib::ustring _getDragTip(GdkEventMotion */*event*/) const {
        return format_tip(C_("Transform handle tip",
            "Scale by %.2f%% x %.2f%%"), _last_scale_x * 100, _last_scale_y * 100);
    }

    virtual bool _hasDragTips() const { return true; }

    static double _last_scale_x, _last_scale_y;
};
double ScaleHandle::_last_scale_x = 1.0;
double ScaleHandle::_last_scale_y = 1.0;

/**
 * Corner scaling handle for node transforms.
 */
class ScaleCornerHandle : public ScaleHandle {
public:

    ScaleCornerHandle(TransformHandleSet &th, unsigned corner) :
        ScaleHandle(th, corner_to_anchor(corner), _corner_to_pixbuf(corner)),
        _corner(corner)
    {}

protected:
    virtual void startTransform() {
        _sc_center = _th.rotationCenter();
        _sc_opposite = _th.bounds().corner(_corner + 2);
        _last_scale_x = _last_scale_y = 1.0;
    }

    virtual Geom::Affine computeTransform(Geom::Point const &new_pos, GdkEventMotion *event) {
        Geom::Point scc = held_shift(*event) ? _sc_center : _sc_opposite;
        Geom::Point vold = _origin - scc, vnew = new_pos - scc;

        // avoid exploding the selection
        if (Geom::are_near(vold[Geom::X], 0) || Geom::are_near(vold[Geom::Y], 0))
            return Geom::identity();

        double scale[2] = { vnew[Geom::X] / vold[Geom::X], vnew[Geom::Y] / vold[Geom::Y] };

        if (held_alt(*event)) {
            for (unsigned i = 0; i < 2; ++i) {
                if (fabs(scale[i]) >= 1.0) {
                    scale[i] = round(scale[i]);
                } else {
                    scale[i] = 1.0 / round(1.0 / MIN(scale[i],10));
                }
            }
        } else {
            SnapManager &m = _th._desktop->namedview->snap_manager;
            m.setupIgnoreSelection(_th._desktop, true, &_unselected_points);

            Inkscape::SnappedPoint sp;
            if (held_control(*event)) {
                scale[0] = scale[1] = std::min(scale[0], scale[1]);
                sp = m.constrainedSnapScale(_snap_points, _origin, Geom::Scale(scale[0], scale[1]), scc);
            } else {
                sp = m.freeSnapScale(_snap_points, _origin, Geom::Scale(scale[0], scale[1]), scc);
            }
            m.unSetup();

            if (sp.getSnapped()) {
                Geom::Point result = sp.getTransformation();
                scale[0] = result[0];
                scale[1] = result[1];
            }
        }

        _last_scale_x = scale[0];
        _last_scale_y = scale[1];
        Geom::Affine t = Geom::Translate(-scc)
            * Geom::Scale(scale[0], scale[1])
            * Geom::Translate(scc);
        return t;
    }

    virtual CommitEvent getCommitEvent() {
        return _last_transform.isUniformScale()
            ? COMMIT_MOUSE_SCALE_UNIFORM
            : COMMIT_MOUSE_SCALE;
    }

private:

    static Glib::RefPtr<Gdk::Pixbuf> _corner_to_pixbuf(unsigned c) {
        //sp_select_context_get_type();
        switch (c % 2) {
            case 0:
                return Glib::wrap(handles[1], true);
                break;
            default:
                return Glib::wrap(handles[0], true);
                break;
        }
    }

    Geom::Point _sc_center;
    Geom::Point _sc_opposite;
    unsigned _corner;
};

/**
 * Side scaling handle for node transforms.
 */
class ScaleSideHandle : public ScaleHandle {
public:
    ScaleSideHandle(TransformHandleSet &th, unsigned side)
        : ScaleHandle(th, side_to_anchor(side), _side_to_pixbuf(side))
        , _side(side)
    {}
protected:
    virtual void startTransform() {
        _sc_center = _th.rotationCenter();
        Geom::Rect b = _th.bounds();
        _sc_opposite = Geom::middle_point(b.corner(_side + 2), b.corner(_side + 3));
        _last_scale_x = _last_scale_y = 1.0;
    }
    virtual Geom::Affine computeTransform(Geom::Point const &new_pos, GdkEventMotion *event) {
        Geom::Point scc = held_shift(*event) ? _sc_center : _sc_opposite;
        Geom::Point vs;
        Geom::Dim2 d1 = static_cast<Geom::Dim2>((_side + 1) % 2);
        Geom::Dim2 d2 = static_cast<Geom::Dim2>(_side % 2);

        // avoid exploding the selection
        if (Geom::are_near(scc[d1], _origin[d1]))
            return Geom::identity();

        vs[d1] = (new_pos - scc)[d1] / (_origin - scc)[d1];
        if (held_alt(*event)) {
            if (fabs(vs[d1]) >= 1.0) {
                vs[d1] = round(vs[d1]);
            } else {
                vs[d1] = 1.0 / round(1.0 / MIN(vs[d1],10));
            }
            vs[d2] = 1.0;
        } else {
            SnapManager &m = _th._desktop->namedview->snap_manager;
            m.setupIgnoreSelection(_th._desktop, true, &_unselected_points);

            bool uniform = held_control(*event);
            Inkscape::SnappedPoint sp = m.constrainedSnapStretch(_snap_points, _origin, vs[d1], scc, d1, uniform);
            m.unSetup();

            if (sp.getSnapped()) {
                Geom::Point result = sp.getTransformation();
                vs[d1] = result[d1];
                vs[d2] = result[d2];
            } else {
                // on ctrl, apply uniform scaling instead of stretching
                // Preserve aspect ratio, but never flip in the dimension not being edited (by using fabs())
                vs[d2] = uniform ? fabs(vs[d1]) : 1.0;
            }
        }

        _last_scale_x = vs[Geom::X];
        _last_scale_y = vs[Geom::Y];
        Geom::Affine t = Geom::Translate(-scc)
            * Geom::Scale(vs)
            * Geom::Translate(scc);
        return t;
    }
    virtual CommitEvent getCommitEvent() {
        return _last_transform.isUniformScale()
            ? COMMIT_MOUSE_SCALE_UNIFORM
            : COMMIT_MOUSE_SCALE;
    }
private:
    static Glib::RefPtr<Gdk::Pixbuf> _side_to_pixbuf(unsigned c) {
        //sp_select_context_get_type();
        switch (c % 2) {
        case 0: return Glib::wrap(handles[3], true);
        default: return Glib::wrap(handles[2], true);
        }
    }
    Geom::Point _sc_center;
    Geom::Point _sc_opposite;
    unsigned _side;
};

/**
 * Rotation handle for node transforms.
 */
class RotateHandle : public TransformHandle {
public:
    RotateHandle(TransformHandleSet &th, unsigned corner)
        : TransformHandle(th, corner_to_anchor(corner), _corner_to_pixbuf(corner))
        , _corner(corner)
    {}
protected:

    virtual void startTransform() {
        _rot_center = _th.rotationCenter();
        _rot_opposite = _th.bounds().corner(_corner + 2);
        _last_angle = 0;
    }

    virtual Geom::Affine computeTransform(Geom::Point const &new_pos, GdkEventMotion *event)
    {
        Geom::Point rotc = held_shift(*event) ? _rot_opposite : _rot_center;
        double angle = Geom::angle_between(_origin - rotc, new_pos - rotc);
        if (held_control(*event)) {
            angle = snap_angle(angle);
        } else {
            SnapManager &m = _th._desktop->namedview->snap_manager;
            m.setupIgnoreSelection(_th._desktop, true, &_unselected_points);
            Inkscape::SnappedPoint sp = m.constrainedSnapRotate(_snap_points, _origin, angle, rotc);
            m.unSetup();

            if (sp.getSnapped()) {
                angle = sp.getTransformation()[0];
            }
        }

        _last_angle = angle;
        Geom::Affine t = Geom::Translate(-rotc)
            * Geom::Rotate(angle)
            * Geom::Translate(rotc);
        return t;
    }

    virtual CommitEvent getCommitEvent() { return COMMIT_MOUSE_ROTATE; }

    virtual Glib::ustring _getTip(unsigned state) const {
        if (state_held_shift(state)) {
            if (state_held_control(state)) {
                return format_tip(C_("Transform handle tip",
                    "<b>Shift+Ctrl</b>: rotate around the opposite corner and snap "
                    "angle to %f° increments"), snap_increment_degrees());
            }
            return C_("Transform handle tip", "<b>Shift</b>: rotate around the opposite corner");
        }
        if (state_held_control(state)) {
            return format_tip(C_("Transform handle tip",
                "<b>Ctrl</b>: snap angle to %f° increments"), snap_increment_degrees());
        }
        return C_("Transform handle tip", "<b>Rotation handle</b>: drag to rotate "
            "the selection around the rotation center");
    }

    virtual Glib::ustring _getDragTip(GdkEventMotion */*event*/) const {
        return format_tip(C_("Transform handle tip", "Rotate by %.2f°"),
            _last_angle * 180.0 / M_PI);
    }

    virtual bool _hasDragTips() const { return true; }

private:
    static Glib::RefPtr<Gdk::Pixbuf> _corner_to_pixbuf(unsigned c) {
        //sp_select_context_get_type();
        switch (c % 4) {
        case 0: return Glib::wrap(handles[7], true);
        case 1: return Glib::wrap(handles[6], true);
        case 2: return Glib::wrap(handles[5], true);
        default: return Glib::wrap(handles[4], true);
        }
    }
    Geom::Point _rot_center;
    Geom::Point _rot_opposite;
    unsigned _corner;
    static double _last_angle;
};
double RotateHandle::_last_angle = 0;

class SkewHandle : public TransformHandle {
public:
    SkewHandle(TransformHandleSet &th, unsigned side)
        : TransformHandle(th, side_to_anchor(side), _side_to_pixbuf(side))
        , _side(side)
    {}

protected:

    virtual void startTransform() {
        _skew_center = _th.rotationCenter();
        Geom::Rect b = _th.bounds();
        _skew_opposite = Geom::middle_point(b.corner(_side + 2), b.corner(_side + 3));
        _last_angle = 0;
        _last_horizontal = _side % 2;
    }

    virtual Geom::Affine computeTransform(Geom::Point const &new_pos, GdkEventMotion *event)
    {
        Geom::Point scc = held_shift(*event) ? _skew_center : _skew_opposite;
        Geom::Dim2 d1 = static_cast<Geom::Dim2>((_side + 1) % 2);
        Geom::Dim2 d2 = static_cast<Geom::Dim2>(_side % 2);

        Geom::Point const initial_delta = _origin - scc;

        if (fabs(initial_delta[d1]) < 1e-15) {
            return Geom::Affine();
        }

        // Calculate the scale factors, which can be either visual or geometric
        // depending on which type of bbox is currently being used (see preferences -> selector tool)
        Geom::Scale scale = calcScaleFactors(_origin, new_pos, scc, false);
        Geom::Scale skew = calcScaleFactors(_origin, new_pos, scc, true);
        scale[d2] = 1;
        skew[d2] = 1;

        // Skew handles allow scaling up to integer multiples of the original size
        // in the second direction; prevent explosions

        if (fabs(scale[d1]) < 1) {
            // Prevent shrinking of the selected object, while allowing mirroring
            scale[d1] = copysign(1.0, scale[d1]);
        } else {
            // Allow expanding of the selected object by integer multiples
            scale[d1] = floor(scale[d1] + 0.5);
        }

        double angle = atan(skew[d1] / scale[d1]);

        if (held_control(*event)) {
            angle = snap_angle(angle);
            skew[d1] = tan(angle) * scale[d1];
        } else {
            SnapManager &m = _th._desktop->namedview->snap_manager;
            m.setupIgnoreSelection(_th._desktop, true, &_unselected_points);

            Geom::Point cvec; cvec[d2] = 1.0;
            Inkscape::Snapper::SnapConstraint const constraint(cvec);
            Inkscape::SnappedPoint sp = m.constrainedSnapSkew(_snap_points, _origin, constraint, Geom::Point(skew[d1], scale[d1]), scc, d2);
            m.unSetup();

            if (sp.getSnapped()) {
                skew[d1] =  sp.getTransformation()[0];
            }
        }

        _last_angle = angle;

        // Update the handle position
        Geom::Point new_new_pos;
        new_new_pos[d2] = initial_delta[d1] * skew[d1] + _origin[d2];
        new_new_pos[d1] = initial_delta[d1] * scale[d1] + scc[d1];

        // Calculate the relative affine
        Geom::Affine relative_affine = Geom::identity();
        relative_affine[2*d1 + d1] = (new_new_pos[d1] - scc[d1]) / initial_delta[d1];
        relative_affine[2*d1 + (d2)] = (new_new_pos[d2] - _origin[d2]) / initial_delta[d1];
        relative_affine[2*(d2) + (d1)] = 0;
        relative_affine[2*(d2) + (d2)] = 1;

        for (int i = 0; i < 2; i++) {
            if (fabs(relative_affine[3*i]) < 1e-15) {
                relative_affine[3*i] = 1e-15;
            }
        }

        Geom::Affine t = Geom::Translate(-scc)
            * relative_affine
            * Geom::Translate(scc);

        return t;
    }

    virtual CommitEvent getCommitEvent() {
        return (_side % 2)
            ? COMMIT_MOUSE_SKEW_Y
            : COMMIT_MOUSE_SKEW_X;
    }

    virtual Glib::ustring _getTip(unsigned state) const {
        if (state_held_shift(state)) {
            if (state_held_control(state)) {
                return format_tip(C_("Transform handle tip",
                    "<b>Shift+Ctrl</b>: skew about the rotation center with snapping "
                    "to %f° increments"), snap_increment_degrees());
            }
            return C_("Transform handle tip", "<b>Shift</b>: skew about the rotation center");
        }
        if (state_held_control(state)) {
            return format_tip(C_("Transform handle tip",
                "<b>Ctrl</b>: snap skew angle to %f° increments"), snap_increment_degrees());
        }
        return C_("Transform handle tip",
            "<b>Skew handle</b>: drag to skew (shear) selection about "
            "the opposite handle");
    }

    virtual Glib::ustring _getDragTip(GdkEventMotion */*event*/) const {
        if (_last_horizontal) {
            return format_tip(C_("Transform handle tip", "Skew horizontally by %.2f°"),
                _last_angle * 360.0);
        } else {
            return format_tip(C_("Transform handle tip", "Skew vertically by %.2f°"),
                _last_angle * 360.0);
        }
    }

    virtual bool _hasDragTips() const { return true; }

private:

    static Glib::RefPtr<Gdk::Pixbuf> _side_to_pixbuf(unsigned s) {
        //sp_select_context_get_type();
        switch (s % 4) {
        case 0: return Glib::wrap(handles[10], true);
        case 1: return Glib::wrap(handles[9], true);
        case 2: return Glib::wrap(handles[8], true);
        default: return Glib::wrap(handles[11], true);
        }
    }
    Geom::Point _skew_center;
    Geom::Point _skew_opposite;
    unsigned _side;
    static bool _last_horizontal;
    static double _last_angle;
};
bool SkewHandle::_last_horizontal = false;
double SkewHandle::_last_angle = 0;

class RotationCenter : public ControlPoint {

public:
    RotationCenter(TransformHandleSet &th) :
        ControlPoint(th._desktop, Geom::Point(), SP_ANCHOR_CENTER,
                     _get_pixbuf(),
                     _center_cset, th._transform_handle_group),
        _th(th)
    {
        setVisible(false);
    }

protected:
    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *event) {
        SnapManager &sm = _th._desktop->namedview->snap_manager;
        sm.setup(_th._desktop);
        bool snap = !held_shift(*event) && sm.someSnapperMightSnap();
        if (held_control(*event)) {
            // constrain to axes
            Geom::Point origin = _last_drag_origin();
            std::vector<Inkscape::Snapper::SnapConstraint> constraints;
            constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, Geom::Point(1, 0)));
            constraints.push_back(Inkscape::Snapper::SnapConstraint(origin, Geom::Point(0, 1)));
            new_pos = sm.multipleConstrainedSnaps(Inkscape::SnapCandidatePoint(new_pos,
                SNAPSOURCE_ROTATION_CENTER), constraints, held_shift(*event)).getPoint();
        } else if (snap) {
            sm.freeSnapReturnByRef(new_pos, SNAPSOURCE_ROTATION_CENTER);
        }
        sm.unSetup();
    }
    virtual Glib::ustring _getTip(unsigned /*state*/) const {
        return C_("Transform handle tip",
            "<b>Rotation center</b>: drag to change the origin of transforms");
    }

private:

    static Glib::RefPtr<Gdk::Pixbuf> _get_pixbuf() {
        //sp_select_context_get_type();
        return Glib::wrap(handles[12], true);
    }

    static ColorSet _center_cset;
    TransformHandleSet &_th;
};

ControlPoint::ColorSet RotationCenter::_center_cset = {
    {0x00000000, 0x000000ff},
    {0x00000000, 0xff0000b0},
    {0x00000000, 0xff0000b0},
    //
    {0x00000000, 0x000000ff},
    {0x00000000, 0xff0000b0},
    {0x00000000, 0xff0000b0}    
};


TransformHandleSet::TransformHandleSet(SPDesktop *d, SPCanvasGroup *th_group)
    : Manipulator(d)
    , _active(0)
    , _transform_handle_group(th_group)
    , _mode(MODE_SCALE)
    , _in_transform(false)
    , _visible(true)
{
    _trans_outline = static_cast<CtrlRect*>(sp_canvas_item_new(_desktop->getControls(),
        SP_TYPE_CTRLRECT, NULL));
    sp_canvas_item_hide(_trans_outline);
    _trans_outline->setDashed(true);

    for (unsigned i = 0; i < 4; ++i) {
        _scale_corners[i] = new ScaleCornerHandle(*this, i);
        _scale_sides[i] = new ScaleSideHandle(*this, i);
        _rot_corners[i] = new RotateHandle(*this, i);
        _skew_sides[i] = new SkewHandle(*this, i);
    }
    _center = new RotationCenter(*this);
    // when transforming, update rotation center position
    signal_transform.connect(sigc::mem_fun(*_center, &RotationCenter::transform));
}

TransformHandleSet::~TransformHandleSet()
{
    for (unsigned i = 0; i < 17; ++i) {
        delete _handles[i];
    }
}

void TransformHandleSet::setMode(Mode m)
{
    _mode = m;
    _updateVisibility(_visible);
}

Geom::Rect TransformHandleSet::bounds() const
{
    return Geom::Rect(*_scale_corners[0], *_scale_corners[2]);
}

ControlPoint const &TransformHandleSet::rotationCenter() const
{
    return *_center;
}

ControlPoint &TransformHandleSet::rotationCenter()
{
    return *_center;
}

void TransformHandleSet::setVisible(bool v)
{
    if (_visible != v) {
        _visible = v;
        _updateVisibility(_visible);
    }
}

void TransformHandleSet::setBounds(Geom::Rect const &r, bool preserve_center)
{
    if (_in_transform) {
        _trans_outline->setRectangle(r);
    } else {
        for (unsigned i = 0; i < 4; ++i) {
            _scale_corners[i]->move(r.corner(i));
            _scale_sides[i]->move(Geom::middle_point(r.corner(i), r.corner(i+1)));
            _rot_corners[i]->move(r.corner(i));
            _skew_sides[i]->move(Geom::middle_point(r.corner(i), r.corner(i+1)));
        }
        if (!preserve_center) _center->move(r.midpoint());
        if (_visible) _updateVisibility(true);
    }
}

bool TransformHandleSet::event(Inkscape::UI::Tools::ToolBase *, GdkEvent*)
{
    return false;
}

void TransformHandleSet::_emitTransform(Geom::Affine const &t)
{
    signal_transform.emit(t);
    _center->transform(t);
}

void TransformHandleSet::_setActiveHandle(ControlPoint *th)
{
    _active = th;
    if (_in_transform)
        throw std::logic_error("Transform initiated when another transform in progress");
    _in_transform = true;
    // hide all handles except the active one
    _updateVisibility(false);
    sp_canvas_item_show(_trans_outline);
}

void TransformHandleSet::_clearActiveHandle()
{
    // This can only be called from handles, so they had to be visible before _setActiveHandle
    sp_canvas_item_hide(_trans_outline);
    _active = 0;
    _in_transform = false;
    _updateVisibility(_visible);
}

void TransformHandleSet::_updateVisibility(bool v)
{
    if (v) {
        Geom::Rect b = bounds();
        Geom::Point handle_size(
            gdk_pixbuf_get_width(handles[0]) / _desktop->current_zoom(),
            gdk_pixbuf_get_height(handles[0]) / _desktop->current_zoom());
        Geom::Point bp = b.dimensions();

        // do not scale when the bounding rectangle has zero width or height
        bool show_scale = (_mode == MODE_SCALE) && !Geom::are_near(b.minExtent(), 0);
        // do not rotate if the bounding rectangle is degenerate
        bool show_rotate = (_mode == MODE_ROTATE_SKEW) && !Geom::are_near(b.maxExtent(), 0);
        bool show_scale_side[2], show_skew[2];

        // show sides if:
        // a) there is enough space between corner handles, or
        // b) corner handles are not shown, but side handles make sense
        // this affects horizontal and vertical scale handles; skew handles never
        // make sense if rotate handles are not shown
        for (unsigned i = 0; i < 2; ++i) {
            Geom::Dim2 d = static_cast<Geom::Dim2>(i);
            Geom::Dim2 otherd = static_cast<Geom::Dim2>((i+1)%2);
            show_scale_side[i] = (_mode == MODE_SCALE);
            show_scale_side[i] &= (show_scale ? bp[d] >= handle_size[d]
                : !Geom::are_near(bp[otherd], 0));
            show_skew[i] = (show_rotate && bp[d] >= handle_size[d]
                && !Geom::are_near(bp[otherd], 0));
        }
        for (unsigned i = 0; i < 4; ++i) {
            _scale_corners[i]->setVisible(show_scale);
            _rot_corners[i]->setVisible(show_rotate);
            _scale_sides[i]->setVisible(show_scale_side[i%2]);
            _skew_sides[i]->setVisible(show_skew[i%2]);
        }
        // show rotation center if there is enough space (?)
        _center->setVisible(show_rotate /*&& bp[Geom::X] > handle_size[Geom::X]
            && bp[Geom::Y] > handle_size[Geom::Y]*/);
    } else {
        for (unsigned i = 0; i < 17; ++i) {
            if (_handles[i] != _active)
                _handles[i]->setVisible(false);
        }
    }
    
}

} // namespace UI
} // namespace Inkscape

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

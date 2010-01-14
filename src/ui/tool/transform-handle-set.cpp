/** @file
 * Affine transform handles component
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <algorithm>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <2geom/transforms.h>
#include "desktop.h"
#include "desktop-handles.h"
#include "display/sodipodi-ctrlrect.h"
#include "preferences.h"
#include "ui/tool/commit-events.h"
#include "ui/tool/control-point.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/transform-handle-set.h"

// FIXME BRAIN DAMAGE WARNING: this is a global variable in select-context.cpp
// It should be moved to a header
extern GdkPixbuf *handles[];
GType sp_select_context_get_type();

namespace Inkscape {
namespace UI {

namespace {
Gtk::AnchorType corner_to_anchor(unsigned c) {
    switch (c % 4) {
    case 0: return Gtk::ANCHOR_NE;
    case 1: return Gtk::ANCHOR_NW;
    case 2: return Gtk::ANCHOR_SW;
    default: return Gtk::ANCHOR_SE;
    }
}
Gtk::AnchorType side_to_anchor(unsigned s) {
    switch (s % 4) {
    case 0: return Gtk::ANCHOR_N;
    case 1: return Gtk::ANCHOR_W;
    case 2: return Gtk::ANCHOR_S;
    default: return Gtk::ANCHOR_E;
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

ControlPoint::ColorSet thandle_cset = {
    {0x000000ff, 0x000000ff},
    {0x00ff6600, 0x000000ff},
    {0x00ff6600, 0x000000ff}
};

ControlPoint::ColorSet center_cset = {
    {0x00000000, 0x000000ff},
    {0x00000000, 0xff0000b0},
    {0x00000000, 0xff0000b0}    
};
} // anonymous namespace

/** Base class for node transform handles to simplify implementation */
class TransformHandle : public ControlPoint {
public:
    TransformHandle(TransformHandleSet &th, Gtk::AnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pb)
        : ControlPoint(th._desktop, Geom::Point(), anchor, pb, &thandle_cset,
            th._transform_handle_group)
        , _th(th)
    {
        setVisible(false);
        signal_grabbed.connect(
            sigc::bind_return(
                sigc::hide(
                    sigc::mem_fun(*this, &TransformHandle::_grabbedHandler)),
                false));
        signal_dragged.connect(
            sigc::hide<0>(
                sigc::mem_fun(*this, &TransformHandle::_draggedHandler)));
        signal_ungrabbed.connect(
            sigc::hide(
                sigc::mem_fun(*this, &TransformHandle::_ungrabbedHandler)));
    }
protected:
    virtual void startTransform() {}
    virtual void endTransform() {}
    virtual Geom::Matrix computeTransform(Geom::Point const &pos, GdkEventMotion *event) = 0;
    virtual CommitEvent getCommitEvent() = 0;

    Geom::Matrix _last_transform;
    Geom::Point _origin;
    TransformHandleSet &_th;
private:
    void _grabbedHandler() {
        _origin = position();
        _last_transform.setIdentity();
        startTransform();

        _th._setActiveHandle(this);
        _cset = &invisible_cset;
        _setState(_state);
    }
    void _draggedHandler(Geom::Point &new_pos, GdkEventMotion *event)
    {
        Geom::Matrix t = computeTransform(new_pos, event);
        // protect against degeneracies
        if (t.isSingular()) return;
        Geom::Matrix incr = _last_transform.inverse() * t;
        if (incr.isSingular()) return;
        _th.signal_transform.emit(incr);
        _last_transform = t;
    }
    void _ungrabbedHandler() {
        _th._clearActiveHandle();
        _cset = &thandle_cset;
        _setState(_state);
        endTransform();
        _th.signal_commit.emit(getCommitEvent());
    }
};

class ScaleHandle : public TransformHandle {
public:
    ScaleHandle(TransformHandleSet &th, Gtk::AnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pb)
        : TransformHandle(th, anchor, pb)
    {}
protected:
    virtual Glib::ustring _getTip(unsigned state) {
        if (state_held_control(state)) {
            if (state_held_shift(state)) {
                return C_("Transform handle tip",
                    "<b>Shift+Ctrl:</b> scale uniformly about the rotation center");
            }
            return C_("Transform handle tip", "<b>Ctrl:</b> scale uniformly");
        }
        if (state_held_shift(state)) {
            if (state_held_alt(state)) {
                return C_("Transform handle tip",
                    "<b>Shift+Alt:</b> scale using an integer ratio about the rotation center");
            }
            return C_("Transform handle tip", "<b>Shift:</b> scale from the rotation center");
        }
        if (state_held_alt(state)) {
            return C_("Transform handle tip", "<b>Alt:</b> scale using an integer ratio");
        }
        return C_("Transform handle tip", "<b>Scale handle:</b> drag to scale the selection");
    }
    virtual Glib::ustring _getDragTip(GdkEventMotion *event) {
        return format_tip(C_("Transform handle tip",
            "Scale by %.2f%% x %.2f%%"), _last_scale_x * 100, _last_scale_y * 100);
    }
    virtual bool _hasDragTips() { return true; }

    static double _last_scale_x, _last_scale_y;
};
double ScaleHandle::_last_scale_x = 1.0;
double ScaleHandle::_last_scale_y = 1.0;

/// Corner scaling handle for node transforms
class ScaleCornerHandle : public ScaleHandle {
public:
    ScaleCornerHandle(TransformHandleSet &th, unsigned corner)
        : ScaleHandle(th, corner_to_anchor(corner), _corner_to_pixbuf(corner))
        , _corner(corner)
    {}
protected:
    virtual void startTransform() {
        _sc_center = _th.rotationCenter();
        _sc_opposite = _th.bounds().corner(_corner + 2);
        _last_scale_x = _last_scale_y = 1.0;
    }
    virtual Geom::Matrix computeTransform(Geom::Point const &new_pos, GdkEventMotion *event) {
        Geom::Point scc = held_shift(*event) ? _sc_center : _sc_opposite;
        Geom::Point vold = _origin - scc, vnew = new_pos - scc;
        // avoid exploding the selection
        if (Geom::are_near(vold[Geom::X], 0) || Geom::are_near(vold[Geom::Y], 0))
            return Geom::identity();

        double scale[2] = { vnew[Geom::X] / vold[Geom::X], vnew[Geom::Y] / vold[Geom::Y] };
        if (held_alt(*event)) {
            for (unsigned i = 0; i < 2; ++i) {
                if (scale[i] >= 1.0) scale[i] = round(scale[i]);
                else scale[i] = 1.0 / round(1.0 / scale[i]);
            }
        } else if (held_control(*event)) {
            scale[0] = scale[1] = std::min(scale[0], scale[1]);
        }
        _last_scale_x = scale[0];
        _last_scale_y = scale[1];
        Geom::Matrix t = Geom::Translate(-scc)
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
        sp_select_context_get_type();
        switch (c % 2) {
        case 0: return Glib::wrap(handles[1], true);
        default: return Glib::wrap(handles[0], true);
        }
    }
    Geom::Point _sc_center;
    Geom::Point _sc_opposite;
    unsigned _corner;
};

/// Side scaling handle for node transforms
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
    virtual Geom::Matrix computeTransform(Geom::Point const &new_pos, GdkEventMotion *event) {
        Geom::Point scc = held_shift(*event) ? _sc_center : _sc_opposite;
        Geom::Point vs;
        Geom::Dim2 d1 = static_cast<Geom::Dim2>((_side + 1) % 2);
        Geom::Dim2 d2 = static_cast<Geom::Dim2>(_side % 2);

        // avoid exploding the selection
        if (Geom::are_near(scc[d1], _origin[d1]))
            return Geom::identity();

        vs[d1] = (new_pos - scc)[d1] / (_origin - scc)[d1];
        if (held_alt(*event)) {
            if (vs[d1] >= 1.0) vs[d1] = round(vs[d1]);
            else vs[d1] = 1.0 / round(1.0 / vs[d1]);
        }
        vs[d2] = held_control(*event) ? vs[d1] : 1.0;

        _last_scale_x = vs[Geom::X];
        _last_scale_y = vs[Geom::Y];
        Geom::Matrix t = Geom::Translate(-scc)
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
        sp_select_context_get_type();
        switch (c % 2) {
        case 0: return Glib::wrap(handles[3], true);
        default: return Glib::wrap(handles[2], true);
        }
    }
    Geom::Point _sc_center;
    Geom::Point _sc_opposite;
    unsigned _side;
};

/// Rotation handle for node transforms
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
    virtual Geom::Matrix computeTransform(Geom::Point const &new_pos, GdkEventMotion *event)
    {
        Geom::Point rotc = held_shift(*event) ? _rot_opposite : _rot_center;
        double angle = Geom::angle_between(_origin - rotc, new_pos - rotc);
        if (held_control(*event)) {
            angle = snap_angle(angle);
        }
        _last_angle = angle;
        Geom::Matrix t = Geom::Translate(-rotc)
            * Geom::Rotate(angle)
            * Geom::Translate(rotc);
        return t;
    }
    virtual CommitEvent getCommitEvent() { return COMMIT_MOUSE_ROTATE; }
    virtual Glib::ustring _getTip(unsigned state) {
        if (state_held_shift(state)) {
            if (state_held_control(state)) {
                return format_tip(C_("Transform handle tip",
                    "<b>Shift+Ctrl:</b> rotate around the opposite corner and snap "
                    "angle to %f° increments"), snap_increment_degrees());
            }
            return C_("Transform handle tip", "<b>Shift:</b> rotate around the opposite corner");
        }
        if (state_held_control(state)) {
            return format_tip(C_("Transform handle tip",
                "<b>Ctrl:</b> snap angle to %f° increments"), snap_increment_degrees());
        }
        return C_("Transform handle tip", "<b>Rotation handle:</b> drag to rotate "
            "the selection around the rotation center");
    }
    virtual Glib::ustring _getDragTip(GdkEventMotion *event) {
        return format_tip(C_("Transform handle tip", "Rotate by %.2f°"),
            _last_angle * 360.0);
    }
    virtual bool _hasDragTips() { return true; }
private:
    static Glib::RefPtr<Gdk::Pixbuf> _corner_to_pixbuf(unsigned c) {
        sp_select_context_get_type();
        switch (c % 4) {
        case 0: return Glib::wrap(handles[10], true);
        case 1: return Glib::wrap(handles[8], true);
        case 2: return Glib::wrap(handles[6], true);
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
    virtual Geom::Matrix computeTransform(Geom::Point const &new_pos, GdkEventMotion *event)
    {
        Geom::Point scc = held_shift(*event) ? _skew_center : _skew_opposite;
        // d1 and d2 are reversed with respect to ScaleSideHandle
        Geom::Dim2 d1 = static_cast<Geom::Dim2>(_side % 2);
        Geom::Dim2 d2 = static_cast<Geom::Dim2>((_side + 1) % 2);
        Geom::Point proj, scale(1.0, 1.0);

        // Skew handles allow scaling up to integer multiples of the original size
        // in the second direction; prevent explosions
        // TODO should the scaling part be only active with Alt?
        if (!Geom::are_near(_origin[d2], scc[d2])) {
            scale[d2] = (new_pos - scc)[d2] / (_origin - scc)[d2];
        }

        if (scale[d2] < 1.0) {
            scale[d2] = copysign(1.0, scale[d2]);
        } else {
            scale[d2] = floor(scale[d2]);
        }

        // Calculate skew angle. The angle is calculated with regards to the point obtained
        // by projecting the handle position on the relevant side of the bounding box.
        // This avoids degeneracies when moving the skew angle over the rotation center
        proj[d1] = new_pos[d1];
        proj[d2] = scc[d2] + (_origin[d2] - scc[d2]) * scale[d2];
        double angle = 0;
        if (!Geom::are_near(proj[d2], scc[d2]))
            angle = Geom::angle_between(_origin - scc, proj - scc);
        if (held_control(*event)) angle = snap_angle(angle);

        // skew matrix has the from [[1, k],[0, 1]] for horizontal skew
        // and [[1,0],[k,1]] for vertical skew.
        Geom::Matrix skew = Geom::identity();
        // correct the sign of the tangent
        skew[d2 + 1] = (d1 == Geom::X ? -1.0 : 1.0) * tan(angle);

        _last_angle = angle;
        Geom::Matrix t = Geom::Translate(-scc)
            * Geom::Scale(scale) * skew
            * Geom::Translate(scc);
        return t;
    }
    virtual CommitEvent getCommitEvent() {
        return _side % 2
            ? COMMIT_MOUSE_SKEW_Y
            : COMMIT_MOUSE_SKEW_X;
    }
    virtual Glib::ustring _getTip(unsigned state) {
        if (state_held_shift(state)) {
            if (state_held_control(state)) {
                return format_tip(C_("Transform handle tip",
                    "<b>Shift+Ctrl:</b> skew about the rotation center with snapping "
                    "to %f° increments"), snap_increment_degrees());
            }
            return C_("Transform handle tip", "<b>Shift:</b> skew about the rotation center");
        }
        if (state_held_control(state)) {
            return format_tip(C_("Transform handle tip",
                "<b>Ctrl:</b> snap skew angle to %f° increments"), snap_increment_degrees());
        }
        return C_("Transform handle tip",
            "<b>Skew handle:</b> drag to skew (shear) selection about "
            "the opposite handle");
    }
    virtual Glib::ustring _getDragTip(GdkEventMotion *event) {
        if (_last_horizontal) {
            return format_tip(C_("Transform handle tip", "Skew horizontally by %.2f°"),
                _last_angle * 360.0);
        } else {
            return format_tip(C_("Transform handle tip", "Skew vertically by %.2f°"),
                _last_angle * 360.0);
        }
    }
    virtual bool _hasDragTips() { return true; }
private:
    static Glib::RefPtr<Gdk::Pixbuf> _side_to_pixbuf(unsigned s) {
        sp_select_context_get_type();
        switch (s % 4) {
        case 0: return Glib::wrap(handles[9], true);
        case 1: return Glib::wrap(handles[7], true);
        case 2: return Glib::wrap(handles[5], true);
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
    RotationCenter(TransformHandleSet &th)
        : ControlPoint(th._desktop, Geom::Point(), Gtk::ANCHOR_CENTER, _get_pixbuf(),
            &center_cset, th._transform_handle_group)
        , _th(th)
    {
        setVisible(false);
    }
protected:
    virtual Glib::ustring _getTip(unsigned state) {
        return C_("Transform handle tip",
            "<b>Rotation center:</b> drag to change the origin of transforms");
    }
private:
    static Glib::RefPtr<Gdk::Pixbuf> _get_pixbuf() {
        sp_select_context_get_type();
        return Glib::wrap(handles[12], true);
    }
    TransformHandleSet &_th;
};

TransformHandleSet::TransformHandleSet(SPDesktop *d, SPCanvasGroup *th_group)
    : Manipulator(d)
    , _active(0)
    , _transform_handle_group(th_group)
    , _mode(MODE_SCALE)
    , _in_transform(false)
    , _visible(true)
{
    _trans_outline = static_cast<CtrlRect*>(sp_canvas_item_new(sp_desktop_controls(_desktop),
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

/** Sets the mode of transform handles (scale or rotate). */
void TransformHandleSet::setMode(Mode m)
{
    _mode = m;
    _updateVisibility(_visible);
}

Geom::Rect TransformHandleSet::bounds()
{
    return Geom::Rect(*_scale_corners[0], *_scale_corners[2]);
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

bool TransformHandleSet::event(GdkEvent*)
{
    return false;
}

void TransformHandleSet::_emitTransform(Geom::Matrix const &t)
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

/** Update the visibility of transformation handles according to settings and the dimensions
 * of the bounding box. It hides the handles that would have no effect or lead to
 * discontinuities. Additionally, side handles for which there is no space are not shown. */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

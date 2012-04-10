/** @file
 * Affine transform handles component
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_TRANSFORM_HANDLE_SET_H
#define SEEN_UI_TOOL_TRANSFORM_HANDLE_SET_H

#include <memory>
#include <gdk/gdk.h>
#include <2geom/forward.h>
#include "ui/tool/commit-events.h"
#include "ui/tool/manipulator.h"
#include "enums.h"

class SPDesktop;
class CtrlRect;
namespace Inkscape {
namespace UI {

//class TransformHandle;
class RotateHandle;
class SkewHandle;
class ScaleCornerHandle;
class ScaleSideHandle;
class RotationCenter;

class TransformHandleSet : public Manipulator {
public:
    enum Mode {
        MODE_SCALE,
        MODE_ROTATE_SKEW
    };

    TransformHandleSet(SPDesktop *d, SPCanvasGroup *th_group);
    virtual ~TransformHandleSet();
    virtual bool event(SPEventContext *, GdkEvent *);

    bool visible() { return _visible; }
    Mode mode() { return _mode; }
    Geom::Rect bounds();
    void setVisible(bool v);
    void setMode(Mode);
    void setBounds(Geom::Rect const &, bool preserve_center = false);

    bool transforming() { return _in_transform; }
    ControlPoint &rotationCenter();

    sigc::signal<void, Geom::Affine const &> signal_transform;
    sigc::signal<void, CommitEvent> signal_commit;
private:
    void _emitTransform(Geom::Affine const &);
    void _setActiveHandle(ControlPoint *h);
    void _clearActiveHandle();
    void _updateVisibility(bool v);
    union {
        ControlPoint *_handles[17];
        struct {
            ScaleCornerHandle *_scale_corners[4];
            ScaleSideHandle *_scale_sides[4];
            RotateHandle *_rot_corners[4];
            SkewHandle *_skew_sides[4];
            RotationCenter *_center;
        };
    };
    ControlPoint *_active;
    SPCanvasGroup *_transform_handle_group;
    CtrlRect *_trans_outline;
    Mode _mode;
    bool _in_transform;
    bool _visible;
    bool _rot_center_visible;
    friend class TransformHandle;
    friend class RotationCenter;
};

/** Base class for node transform handles to simplify implementation */
class TransformHandle : public ControlPoint {
public:
    TransformHandle(TransformHandleSet &th, SPAnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pb);
    void getNextClosestPoint(bool reverse);

protected:
    virtual void startTransform() {}
    virtual void endTransform() {}
    virtual Geom::Affine computeTransform(Geom::Point const &pos, GdkEventMotion *event) = 0;
    virtual CommitEvent getCommitEvent() = 0;

    Geom::Affine _last_transform;
    Geom::Point _origin;
    TransformHandleSet &_th;
    std::vector<Inkscape::SnapCandidatePoint> _snap_points;
    std::vector<Inkscape::SnapCandidatePoint> _unselected_points;
    std::vector<Inkscape::SnapCandidatePoint> _all_snap_sources_sorted;
    std::vector<Inkscape::SnapCandidatePoint>::iterator _all_snap_sources_iter;

private:
    virtual bool grabbed(GdkEventMotion *);
    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *event);
    virtual void ungrabbed(GdkEventButton *);
};

} // namespace UI
} // namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

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
#include "display/display-forward.h"
#include "ui/tool/commit-events.h"
#include "ui/tool/manipulator.h"

class SPDesktop;
class CtrlRect; // this is not present in display-forward.h!
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
    virtual bool event(GdkEvent *);

    bool visible() { return _visible; }
    Mode mode() { return _mode; }
    Geom::Rect bounds();
    void setVisible(bool v);
    void setMode(Mode);
    void setBounds(Geom::Rect const &, bool preserve_center = false);

    bool transforming() { return _in_transform; }
    ControlPoint &rotationCenter();

    sigc::signal<void, Geom::Matrix const &> signal_transform;
    sigc::signal<void, CommitEvent> signal_commit;
private:
    void _emitTransform(Geom::Matrix const &);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

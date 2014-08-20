/** @file
 * Fine-grained modifier tracker for event handling.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_MODIFIER_TRACKER_H
#define SEEN_UI_TOOL_MODIFIER_TRACKER_H

#include <gdk/gdk.h>

namespace Inkscape {
namespace UI {

class ModifierTracker {
public:
    ModifierTracker();
    bool event(GdkEvent *);

    bool leftShift() const { return _left_shift; }
    bool rightShift() const { return _right_shift; }
    bool leftControl() const { return _left_ctrl; }
    bool rightControl() const { return _right_ctrl; }
    bool leftAlt() const { return _left_alt; }
    bool rightAlt() const { return _right_alt; }

private:
    bool _left_shift;
    bool _right_shift;
    bool _left_ctrl;
    bool _right_ctrl;
    bool _left_alt;
    bool _right_alt;
};

} // namespace UI
} // namespace Inkscape

#endif // SEEN_UI_TOOL_MODIFIER_TRACKER_H

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

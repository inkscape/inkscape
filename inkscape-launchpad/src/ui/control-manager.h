/*
 * Inkscape::ControlManager - Coordinates creation and styling of nodes, handles, etc.
 *
 * Author:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_INKSCAPE_CONTROL_MANAGER_H
#define SEEN_INKSCAPE_CONTROL_MANAGER_H

#include <memory>
#include <sigc++/sigc++.h>

#include "ui/control-types.h"

struct SPCanvasGroup;
struct SPCanvasItem;
struct SPCtrlLine;
struct SPCtrlCurve;

namespace Geom
{

class Point;

} // namespace Geom

namespace Inkscape {

enum CtrlLineType {
    CTLINE_PRIMARY,
    CTLINE_SECONDARY,
    CTLINE_TERTIARY,
};


class ControlManagerImpl;

class ControlManager
{
public:

    static ControlManager &getManager();

    ~ControlManager();

    sigc::connection connectCtrlSizeChanged(const sigc::slot<void> &slot);

    SPCanvasItem *createControl(SPCanvasGroup *parent, ControlType type);

    SPCtrlLine *createControlLine(SPCanvasGroup *parent, CtrlLineType type = CTLINE_PRIMARY);

    SPCtrlLine *createControlLine(SPCanvasGroup *parent, Geom::Point const &p1, Geom::Point const &p2, CtrlLineType type = CTLINE_PRIMARY);

    SPCtrlCurve *createControlCurve(SPCanvasGroup *parent, Geom::Point const &p0, Geom::Point const &p1, Geom::Point const &p2, Geom::Point const &p3, CtrlLineType type = CTLINE_PRIMARY);

    void track(SPCanvasItem *item);

    void updateItem(SPCanvasItem *item);

    bool setControlType(SPCanvasItem *item, ControlType type);

    bool setControlResize(SPCanvasItem *item, int ctrlResize);

    bool isActive(SPCanvasItem *item) const;
    void setActive(SPCanvasItem *item, bool active);

    bool isPrelight(SPCanvasItem *item) const;
    void setPrelight(SPCanvasItem *item, bool prelight);

    bool isSelected(SPCanvasItem *item) const;
    void setSelected(SPCanvasItem *item, bool selected);

private:
    ControlManager();
#if __cplusplus <= 199711L
    std::auto_ptr<ControlManagerImpl> _impl;
#else
    std::unique_ptr<ControlManagerImpl> _impl;
#endif
    friend class ControlManagerImpl;
};

} // namespace Inkscape

#endif // SEEN_INKSCAPE_CONTROL_MANAGER_H
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

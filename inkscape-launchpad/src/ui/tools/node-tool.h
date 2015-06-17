/** @file
 * @brief New node tool with support for multiple path editing
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_NODE_TOOL_H
#define SEEN_UI_TOOL_NODE_TOOL_H

#include <boost/ptr_container/ptr_map.hpp>
#include <glib.h>
#include "ui/tools/tool-base.h"

// we need it to call it from Live Effect
#include "selection.h"

namespace Inkscape {
    namespace Display {
        class TemporaryItem;
    }

    namespace UI {
        class MultiPathManipulator;
        class ControlPointSelection;
        class Selector;
        class ControlPoint;

        struct PathSharedData;
    }
}

struct SPCanvasGroup;

#define INK_NODE_TOOL(obj) (dynamic_cast<Inkscape::UI::Tools::NodeTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define INK_IS_NODE_TOOL(obj) (dynamic_cast<const Inkscape::UI::Tools::NodeTool*>((const Inkscape::UI::Tools::ToolBase*)obj))

namespace Inkscape {
namespace UI {
namespace Tools {

class NodeTool : public ToolBase {
public:
    NodeTool();
    virtual ~NodeTool();

    Inkscape::UI::ControlPointSelection* _selected_nodes;
    Inkscape::UI::MultiPathManipulator* _multipath;

    bool edit_clipping_paths;
    bool edit_masks;

    static const std::string prefsPath;

    virtual void setup();
    virtual void update_helperpath();
    virtual void set(const Inkscape::Preferences::Entry& val);
    virtual bool root_handler(GdkEvent* event);

    virtual const std::string& getPrefsPath();

private:
    sigc::connection _selection_changed_connection;
    sigc::connection _mouseover_changed_connection;
    sigc::connection _sizeUpdatedConn;

    SPItem *flashed_item;
    Inkscape::Display::TemporaryItem *helperpath_tmpitem;
    Inkscape::Display::TemporaryItem *flash_tempitem;
    Inkscape::UI::Selector* _selector;
    Inkscape::UI::PathSharedData* _path_data;
    SPCanvasGroup *_transform_handle_group;
    SPItem *_last_over;
    boost::ptr_map<SPItem*, ShapeEditor> _shape_editors;

    bool cursor_drag;
    bool show_handles;
    bool show_outline;
    bool live_outline;
    bool live_objects;
    bool show_path_direction;
    bool show_transform_handles;
    bool single_node_transform_handles;

    std::vector<SPItem*> _current_selection;
    std::vector<SPItem*> _previous_selection;

    void selection_changed(Inkscape::Selection *sel);

    void select_area(Geom::Rect const &sel, GdkEventButton *event);
    void select_point(Geom::Point const &sel, GdkEventButton *event);
    void mouseover_changed(Inkscape::UI::ControlPoint *p);
    void update_tip(GdkEvent *event);
    void handleControlUiStyleChange();
};

}
}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

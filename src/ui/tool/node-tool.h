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

#include <memory>
#include <glib.h>
#include <sigc++/sigc++.h>
#include "event-context.h"
#include "forward.h"
#include "display/display-forward.h"
#include "ui/tool/node-types.h"

#define INK_TYPE_NODE_TOOL               (ink_node_tool_get_type ())
#define INK_NODE_TOOL(obj)            (GTK_CHECK_CAST ((obj), INK_TYPE_NODE_TOOL, InkNodeTool))
#define INK_NODE_TOOL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), INK_TYPE_NODE_TOOL, InkNodeToolClass))
#define INK_IS_NODE_TOOL(obj)         (GTK_CHECK_TYPE ((obj), INK_TYPE_NODE_TOOL))
#define INK_IS_NODE_TOOL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), INK_TYPE_NODE_TOOL))

class InkNodeTool;
class InkNodeToolClass;

namespace Inkscape {
namespace UI {
class MultiPathManipulator;
class ControlPointSelection;
class Selector;
struct PathSharedData;
}
}

typedef std::auto_ptr<Inkscape::UI::MultiPathManipulator> MultiPathPtr;
typedef std::auto_ptr<Inkscape::UI::ControlPointSelection> CSelPtr;
typedef std::auto_ptr<Inkscape::UI::Selector> SelectorPtr;
typedef std::auto_ptr<Inkscape::UI::PathSharedData> PathSharedDataPtr;

struct InkNodeTool : public SPEventContext
{
    sigc::connection _selection_changed_connection;
    sigc::connection _mouseover_changed_connection;
    sigc::connection _selection_modified_connection;
    Inkscape::MessageContext *_node_message_context;
    SPItem *flashed_item;
    Inkscape::Display::TemporaryItem *flash_tempitem;
    CSelPtr _selected_nodes;
    MultiPathPtr _multipath;
    SelectorPtr _selector;
    PathSharedDataPtr _path_data;
    SPCanvasGroup *_transform_handle_group;
    SPItem *_last_over;

    unsigned cursor_drag : 1;
    unsigned show_handles : 1;
    unsigned show_outline : 1;
    unsigned live_outline : 1;
    unsigned live_objects : 1;
    unsigned show_path_direction : 1;
    unsigned show_transform_handles : 1;
    unsigned single_node_transform_handles : 1;
    unsigned edit_clipping_paths : 1;
    unsigned edit_masks : 1;
};

struct InkNodeToolClass {
	SPEventContextClass parent_class;
};

GType ink_node_tool_get_type (void);

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

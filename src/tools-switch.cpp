/*
 * Utility functions for switching tools (= contexts)
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Josh Andler <scislac@users.sf.net>
 *
 * Copyright (C) 2003-2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include "inkscape-private.h"
#include "desktop.h"
#include "desktop-handles.h"
#include <glibmm/i18n.h>

#include <xml/repr.h>

#include "select-context.h"
#include "node-context.h"
#include "tweak-context.h"
#include "sp-path.h"
#include "rect-context.h"
#include "sp-rect.h"
#include "box3d-context.h"
#include "box3d.h"
#include "arc-context.h"
#include "sp-ellipse.h"
#include "star-context.h"
#include "sp-star.h"
#include "spiral-context.h"
#include "sp-spiral.h"
#include "dyna-draw-context.h"
#include "eraser-context.h"
#include "pen-context.h"
#include "pencil-context.h"
#include "text-context.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "gradient-context.h"
#include "zoom-context.h"
#include "dropper-context.h"
#include "connector-context.h"
#include "flood-context.h"
#include "sp-offset.h"
#include "message-context.h"

#include "tools-switch.h"

static char const *const tool_names[] = {
    NULL,
    "tools.select",
    "tools.nodes",
    "tools.tweak",
    "tools.shapes.rect",
    "tools.shapes.3dbox",
    "tools.shapes.arc",
    "tools.shapes.star",
    "tools.shapes.spiral",
    "tools.freehand.pencil",
    "tools.freehand.pen",
    "tools.calligraphic",
    "tools.text",
    "tools.gradient",
    "tools.zoom",
    "tools.dropper",
    "tools.connector",
    "tools.paintbucket",
    "tools.eraser",
    NULL
};

static char const *const tool_ids[] = {
    NULL,
    "select",
    "nodes",
    "tweak",
    "rect",
    "3dbox",
    "arc",
    "star",
    "spiral",
    "pencil",
    "pen",
    "calligraphic",
    "text",
    "gradient",
    "zoom",
    "dropper",
    "connector",
    "paintbucket",
    "eraser",
    NULL
};

static int
tools_id2num(char const *id)
{
    int i = 1;
    while (tool_ids[i]) {
        if (strcmp(tool_ids[i], id) == 0)
            return i;
        else i++;
    }
    g_assert( 0 == TOOLS_INVALID );
    return 0; //nothing found
}

int
tools_isactive(SPDesktop *dt, unsigned num)
{
    g_assert( num < G_N_ELEMENTS(tool_ids) );
    if (SP_IS_EVENT_CONTEXT(dt->event_context))
        return (!strcmp(dt->event_context->prefs_repr->attribute("id"), tool_ids[num]));
    else return FALSE;
}

int
tools_active(SPDesktop *dt)
{
    return (tools_id2num(dt->event_context->prefs_repr->attribute("id")));
}

void
tools_switch(SPDesktop *dt, int num)
{
    if (dt) {
        dt->_tool_changed.emit(num);
    }

    switch (num) {
        case TOOLS_SELECT:
            dt->set_event_context(SP_TYPE_SELECT_CONTEXT, tool_names[num]);
            /* fixme: This is really ugly hack. We should bind and unbind class methods */
            dt->activate_guides(true);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            break;
        case TOOLS_NODES:
            dt->set_event_context(SP_TYPE_NODE_CONTEXT, tool_names[num]);
            dt->activate_guides(true);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("To edit a path, <b>click</b>, <b>Shift+click</b>, or <b>drag around</b> nodes to select them, then <b>drag</b> nodes and handles. <b>Click</b> on an object to select."));
            break;
        case TOOLS_TWEAK:
            dt->set_event_context(SP_TYPE_TWEAK_CONTEXT, tool_names[num]);
            dt->activate_guides(true);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("To tweak a path by pushing, select it and drag over it."));
            break;
        case TOOLS_SHAPES_RECT:
            dt->set_event_context(SP_TYPE_RECT_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to create a rectangle. <b>Drag controls</b> to round corners and resize. <b>Click</b> to select."));
            break;
        case TOOLS_SHAPES_3DBOX:
            dt->set_event_context(SP_TYPE_BOX3D_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to create a 3D box. <b>Drag controls</b> to resize in perspective. <b>Click</b> to select (with <b>Ctrl+Alt</b> for single faces)."));
            break;
        case TOOLS_SHAPES_ARC:
            dt->set_event_context(SP_TYPE_ARC_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to create an ellipse. <b>Drag controls</b> to make an arc or segment. <b>Click</b> to select."));
            break;
        case TOOLS_SHAPES_STAR:
            dt->set_event_context(SP_TYPE_STAR_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to create a star. <b>Drag controls</b> to edit the star shape. <b>Click</b> to select."));
            break;
        case TOOLS_SHAPES_SPIRAL:
            dt->set_event_context(SP_TYPE_SPIRAL_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to create a spiral. <b>Drag controls</b> to edit the spiral shape. <b>Click</b> to select."));
            break;
        case TOOLS_FREEHAND_PENCIL:
            dt->set_event_context(SP_TYPE_PENCIL_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to create a freehand line. Start drawing with <b>Shift</b> to append to selected path. <b>Ctrl+click</b> to create single dots."));
            break;
        case TOOLS_FREEHAND_PEN:
            dt->set_event_context(SP_TYPE_PEN_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> or <b>click and drag</b> to start a path; with <b>Shift</b> to append to selected path. <b>Ctrl+click</b> to create single dots."));
            break;
        case TOOLS_CALLIGRAPHIC:
            dt->set_event_context(SP_TYPE_DYNA_DRAW_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to draw a calligraphic stroke; with <b>Ctrl</b> to track a guide, with <b>Alt</b> to thin/thicken. <b>Arrow keys</b> adjust width (left/right) and angle (up/down)."));
            break;
        case TOOLS_TEXT:
            dt->set_event_context(SP_TYPE_TEXT_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> to select or create text, <b>drag</b> to create flowed text; then type."));
            break;
        case TOOLS_GRADIENT:
            dt->set_event_context(SP_TYPE_GRADIENT_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> or <b>double click</b> to create a gradient on selected objects, <b>drag handles</b> to adjust gradients."));
            break;
        case TOOLS_ZOOM:
            dt->set_event_context(SP_TYPE_ZOOM_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> or <b>drag around an area</b> to zoom in, <b>Shift+click</b> to zoom out."));
            break;
        case TOOLS_DROPPER:
            dt->set_event_context(SP_TYPE_DROPPER_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> to set fill, <b>Shift+click</b> to set stroke; <b>drag</b> to average color in area; with <b>Alt</b> to pick inverse color; <b>Ctrl+C</b> to copy the color under mouse to clipboard"));
            break;
        case TOOLS_CONNECTOR:
            dt->set_event_context(SP_TYPE_CONNECTOR_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click and drag</b> between shapes to create a connector."));
            break;
        case TOOLS_PAINTBUCKET:
            dt->set_event_context(SP_TYPE_FLOOD_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> to paint a bounded area, <b>Shift+click</b> to union the new fill with the current selection, <b>Ctrl+click</b> to change the clicked object's fill and stroke to the current setting."));
            break;
        case TOOLS_ERASER:
            dt->set_event_context(SP_TYPE_ERASER_CONTEXT, tool_names[num]);
            dt->activate_guides(false);
            inkscape_eventcontext_set(sp_desktop_event_context(dt));
            dt->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag</b> to erase."));
            break;
    }
}

void
tools_switch_current(int num)
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (dt) tools_switch(dt, num);
}

void tools_switch_by_item(SPDesktop *dt, SPItem *item)
{
    if (SP_IS_RECT(item)) {
        tools_switch(dt, TOOLS_SHAPES_RECT);
    } else if (SP_IS_BOX3D(item)) {
        tools_switch(dt, TOOLS_SHAPES_3DBOX);
    } else if (SP_IS_GENERICELLIPSE(item)) {
        tools_switch(dt, TOOLS_SHAPES_ARC);
    } else if (SP_IS_STAR(item)) {
        tools_switch(dt, TOOLS_SHAPES_STAR);
    } else if (SP_IS_SPIRAL(item)) {
        tools_switch(dt, TOOLS_SHAPES_SPIRAL);
    } else if (SP_IS_PATH(item)) {
        if (cc_item_is_connector(item)) {
            tools_switch(dt, TOOLS_CONNECTOR);
        }
        else {
            tools_switch(dt, TOOLS_NODES);
        }
    } else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))  {
        tools_switch(dt, TOOLS_TEXT);
    } else if (SP_IS_OFFSET(item))  {
        tools_switch(dt, TOOLS_NODES);
    }
}

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

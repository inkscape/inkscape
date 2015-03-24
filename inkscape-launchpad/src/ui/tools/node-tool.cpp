/**
 * @file
 * New node tool - implementation.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/tool/curve-drag-point.h"
#include <glib/gi18n.h>
#include "desktop.h"

#include "display/sp-canvas-group.h"
#include "display/canvas-bpath.h"
#include "display/curve.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "message-context.h"
#include "selection.h"
#include "ui/shape-editor.h" // temporary!
#include "live_effects/effect.h"
#include "display/curve.h"
#include "sp-clippath.h"
#include "sp-item-group.h"
#include "sp-mask.h"
#include "sp-object-group.h"
#include "sp-path.h"
#include "sp-text.h"
#include "ui/control-manager.h"
#include "ui/tools/node-tool.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/manipulator.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tool/path-manipulator.h"
#include "ui/tool/selector.h"
#include "ui/tool/shape-record.h"

#include "pixmaps/cursor-node.xpm"
#include "pixmaps/cursor-node-d.xpm"
#include "selection-chemistry.h"

#include <gdk/gdkkeysyms.h>

/** @struct NodeTool
 *
 * Node tool event context.
 *
 * @par Architectural overview of the tool
 * @par
 * Here's a breakdown of what each object does.
 * - Handle: shows a handle and keeps the node type constraint (smooth / symmetric) by updating
 *   the other handle's position when dragged. Its move() method cannot violate the constraints.
 * - Node: keeps node type constraints for auto nodes and smooth nodes at ends of linear segments.
 *   Its move() method cannot violate constraints. Handles linear grow and dispatches spatial grow
 *   to MultiPathManipulator. Keeps a reference to its NodeList.
 * - NodeList: exposes an iterator-based interface to nodes. It is possible to obtain an iterator
 *   to a node from the node. Keeps a reference to its SubpathList.
 * - SubpathList: list of NodeLists that represents an editable pathvector. Keeps a reference
 *   to its PathManipulator.
 * - PathManipulator: performs most of the single-path actions like reverse subpaths,
 *   delete segment, shift selection, etc. Keeps a reference to MultiPathManipulator.
 * - MultiPathManipulator: performs additional operations for actions that are not per-path,
 *   for example node joins and segment joins. Tracks the control transforms for PMs that edit
 *   clipping paths and masks. It is more or less equivalent to ShapeEditor and in the future
 *   it might handle all shapes. Handles XML commit of actions that affect all paths or
 *   the node selection and removes PathManipulators that have no nodes left after e.g. node
 *   deletes.
 * - ControlPointSelection: keeps track of node selection and a set of nodes that can potentially
 *   be selected. There can be more than one selection. Performs actions that require no
 *   knowledge about the path, only about the nodes, like dragging and transforms. It is not
 *   specific to nodes and can accommodate any control point derived from SelectableControlPoint.
 *   Transforms nodes in response to transform handle events.
 * - TransformHandleSet: displays nodeset transform handles and emits transform events. The aim
 *   is to eventually use a common class for object and control point transforms.
 * - SelectableControlPoint: base for any type of selectable point. It can belong to only one
 *   selection.
 *
 * @par Functionality that resides in weird places
 * @par
 *
 * This list is probably incomplete.
 * - Curve dragging: CurveDragPoint, controlled by PathManipulator
 * - Single handle shortcuts: MultiPathManipulator::event(), ModifierTracker
 * - Linear and spatial grow: Node, spatial grow routed to ControlPointSelection
 * - Committing handle actions performed with the mouse: PathManipulator
 * - Sculpting: ControlPointSelection
 *
 * @par Plans for the future
 * @par
 * - MultiPathManipulator should become a generic shape editor that manages all active manipulator,
 *   more or less like the old ShapeEditor.
 * - Knotholder should be rewritten into one manipulator class per shape, using the control point
 *   classes. Interesting features like dragging rectangle sides could be added along the way.
 * - Better handling of clip and mask editing, particularly in response to undo.
 * - High level refactoring of the event context hierarchy. All aspects of tools, like toolbox
 *   controls, icons, event handling should be collected in one class, though each aspect
 *   of a tool might be in an separate class for better modularity. The long term goal is to allow
 *   tools to be defined in extensions or shared library plugins.
 */

using Inkscape::ControlManager;

namespace Inkscape {
namespace UI {
namespace Tools {

const std::string& NodeTool::getPrefsPath() {
	return NodeTool::prefsPath;
}

const std::string NodeTool::prefsPath = "/tools/nodes";

SPCanvasGroup *create_control_group(SPDesktop *d);

NodeTool::NodeTool()
    : ToolBase(cursor_node_xpm, 1, 1)
    , _selected_nodes(NULL)
    , _multipath(NULL)
    , edit_clipping_paths(false)
    , edit_masks(false)
    , flashed_item(NULL)
    , flash_tempitem(NULL)
    , _selector(NULL)
    , _path_data(NULL)
    , _transform_handle_group(NULL)
    , _last_over(NULL)
    , cursor_drag(false)
    , show_handles(false)
    , show_outline(false)
    , live_outline(false)
    , live_objects(false)
    , show_path_direction(false)
    , show_transform_handles(false)
    , single_node_transform_handles(false)
{
}

SPCanvasGroup *create_control_group(SPDesktop *d)
{
    return reinterpret_cast<SPCanvasGroup*>(sp_canvas_item_new(
        d->getControls(), SP_TYPE_CANVAS_GROUP, NULL));
}

void destroy_group(SPCanvasGroup *g)
{
    sp_canvas_item_destroy(SP_CANVAS_ITEM(g));
}

NodeTool::~NodeTool() {
    this->enableGrDrag(false);

    if (this->flash_tempitem) {
        this->desktop->remove_temporary_canvasitem(this->flash_tempitem);
    }
    if (this->helperpath_tmpitem) {
        this->desktop->remove_temporary_canvasitem(this->helperpath_tmpitem);
    }

    if (this->helperpath_tmpitem) {
        this->desktop->remove_temporary_canvasitem(this->helperpath_tmpitem);
    }

    this->_selection_changed_connection.disconnect();
    //this->_selection_modified_connection.disconnect();
    this->_mouseover_changed_connection.disconnect();
    this->_sizeUpdatedConn.disconnect();

    delete this->_multipath;
    delete this->_selected_nodes;
    delete this->_selector;

    Inkscape::UI::PathSharedData &data = *this->_path_data;
    destroy_group(data.node_data.node_group);
    destroy_group(data.node_data.handle_group);
    destroy_group(data.node_data.handle_line_group);
    destroy_group(data.outline_group);
    destroy_group(data.dragpoint_group);
    destroy_group(this->_transform_handle_group);
}

void NodeTool::setup() {
    ToolBase::setup();

    this->_path_data = new Inkscape::UI::PathSharedData();

    Inkscape::UI::PathSharedData &data = *this->_path_data;
    data.node_data.desktop = this->desktop;

    // selector has to be created here, so that its hidden control point is on the bottom
    this->_selector = new Inkscape::UI::Selector(this->desktop);

    // Prepare canvas groups for controls. This guarantees correct z-order, so that
    // for example a dragpoint won't obscure a node
    data.outline_group = create_control_group(this->desktop);
    data.node_data.handle_line_group = create_control_group(this->desktop);
    data.dragpoint_group = create_control_group(this->desktop);
    this->_transform_handle_group = create_control_group(this->desktop);
    data.node_data.node_group = create_control_group(this->desktop);
    data.node_data.handle_group = create_control_group(this->desktop);

    Inkscape::Selection *selection = this->desktop->getSelection();

    this->_selection_changed_connection.disconnect();
    this->_selection_changed_connection =
        selection->connectChanged(sigc::mem_fun(this, &NodeTool::selection_changed));

    this->_mouseover_changed_connection.disconnect();
    this->_mouseover_changed_connection = 
        Inkscape::UI::ControlPoint::signal_mouseover_change.connect(sigc::mem_fun(this, &NodeTool::mouseover_changed));

    this->_sizeUpdatedConn = ControlManager::getManager().connectCtrlSizeChanged(
    		sigc::mem_fun(this, &NodeTool::handleControlUiStyleChange)
    );
    
    this->_selected_nodes = new Inkscape::UI::ControlPointSelection(this->desktop, this->_transform_handle_group);

    data.node_data.selection = this->_selected_nodes;

    this->_multipath = new Inkscape::UI::MultiPathManipulator(data, this->_selection_changed_connection);

    this->_selector->signal_point.connect(sigc::mem_fun(this, &NodeTool::select_point));
    this->_selector->signal_area.connect(sigc::mem_fun(this, &NodeTool::select_area));

    this->_multipath->signal_coords_changed.connect(
        sigc::bind(
            sigc::mem_fun(*this->desktop, &SPDesktop::emitToolSubselectionChanged),
            (void*)NULL
        )
    );

    this->_selected_nodes->signal_selection_changed.connect(
		// Hide both signal parameters and bind the function parameter to 0
		// sigc::signal<void, SelectableControlPoint *, bool>
		// <=>
		// void update_tip(GdkEvent *event)
		sigc::hide(sigc::hide(sigc::bind(
				sigc::mem_fun(this, &NodeTool::update_tip),
				(GdkEvent*)NULL
		)))
    );

    this->helperpath_tmpitem = NULL;
    this->cursor_drag = false;
    this->show_transform_handles = true;
    this->single_node_transform_handles = false;
    this->flash_tempitem = NULL;
    this->flashed_item = NULL;
    this->_last_over = NULL;
    this->helperpath_tmpitem = NULL;

    // read prefs before adding items to selection to prevent momentarily showing the outline
    sp_event_context_read(this, "show_handles");
    sp_event_context_read(this, "show_outline");
    sp_event_context_read(this, "live_outline");
    sp_event_context_read(this, "live_objects");
    sp_event_context_read(this, "show_path_direction");
    sp_event_context_read(this, "show_transform_handles");
    sp_event_context_read(this, "single_node_transform_handles");
    sp_event_context_read(this, "edit_clipping_paths");
    sp_event_context_read(this, "edit_masks");

    this->selection_changed(selection);
    this->update_tip(NULL);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool("/tools/nodes/selcue")) {
        this->enableSelectionCue();
    }

    if (prefs->getBool("/tools/nodes/gradientdrag")) {
        this->enableGrDrag();
    }

    this->desktop->emitToolSubselectionChanged(NULL); // sets the coord entry fields to inactive
    this->update_helperpath();
}

// show helper paths of the applied LPE, if any
void  NodeTool::update_helperpath () {
    Inkscape::Selection *selection = this->desktop->getSelection();

    if (this->helperpath_tmpitem) {
        this->desktop->remove_temporary_canvasitem(this->helperpath_tmpitem);
        this->helperpath_tmpitem = NULL;
    }

    if (SP_IS_LPE_ITEM(selection->singleItem())) {
        Inkscape::LivePathEffect::Effect *lpe = SP_LPE_ITEM(selection->singleItem())->getCurrentLPE();
        if (lpe && lpe->isVisible()/* && lpe->showOrigPath()*/) {
            Inkscape::UI::ControlPointSelection::Set &selectionNodes = _selected_nodes->allPoints();
            std::vector<Geom::Point> selectedNodesPositions;
            for (Inkscape::UI::ControlPointSelection::Set::iterator i = selectionNodes.begin(); i != selectionNodes.end(); ++i) {
                if ((*i)->selected()) {
                    Inkscape::UI::Node *n = dynamic_cast<Inkscape::UI::Node *>(*i);
                    selectedNodesPositions.push_back(n->position());
                }
            }
            lpe->setSelectedNodePoints(selectedNodesPositions);
            lpe->setCurrentZoom(this->desktop->current_zoom());
            SPCurve *c = new SPCurve();
            SPCurve *cc = new SPCurve();
            std::vector<Geom::PathVector> cs = lpe->getCanvasIndicators(SP_LPE_ITEM(selection->singleItem()));
            for (std::vector<Geom::PathVector>::iterator p = cs.begin(); p != cs.end(); ++p) {
                cc->set_pathvector(*p);
                c->append(cc, false);
                cc->reset();
            }
            if (!c->is_empty()) {
                SPCanvasItem *helperpath = sp_canvas_bpath_new(this->desktop->getTempGroup(), c);
                sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(helperpath), 0x0000ff9A, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
                sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(helperpath), 0, SP_WIND_RULE_NONZERO);
                sp_canvas_item_affine_absolute(helperpath, selection->singleItem()->i2dt_affine());
                this->helperpath_tmpitem = this->desktop->add_temporary_canvasitem(helperpath, 0);
            }
            c->unref();
            cc->unref();
        }
    }
}

void NodeTool::set(const Inkscape::Preferences::Entry& value) {
    Glib::ustring entry_name = value.getEntryName();

    if (entry_name == "show_handles") {
        this->show_handles = value.getBool(true);
        this->_multipath->showHandles(this->show_handles);
    } else if (entry_name == "show_outline") {
        this->show_outline = value.getBool();
        this->_multipath->showOutline(this->show_outline);
    } else if (entry_name == "live_outline") {
        this->live_outline = value.getBool();
        this->_multipath->setLiveOutline(this->live_outline);
    } else if (entry_name == "live_objects") {
        this->live_objects = value.getBool();
        this->_multipath->setLiveObjects(this->live_objects);
    } else if (entry_name == "show_path_direction") {
        this->show_path_direction = value.getBool();
        this->_multipath->showPathDirection(this->show_path_direction);
    } else if (entry_name == "show_transform_handles") {
        this->show_transform_handles = value.getBool(true);
        this->_selected_nodes->showTransformHandles(
            this->show_transform_handles, this->single_node_transform_handles);
    } else if (entry_name == "single_node_transform_handles") {
        this->single_node_transform_handles = value.getBool();
        this->_selected_nodes->showTransformHandles(
            this->show_transform_handles, this->single_node_transform_handles);
    } else if (entry_name == "edit_clipping_paths") {
        this->edit_clipping_paths = value.getBool();
        this->selection_changed(this->desktop->selection);
    } else if (entry_name == "edit_masks") {
        this->edit_masks = value.getBool();
        this->selection_changed(this->desktop->selection);
    } else {
    	ToolBase::set(value);
    }
}

/** Recursively collect ShapeRecords */
void gather_items(NodeTool *nt, SPItem *base, SPObject *obj, Inkscape::UI::ShapeRole role,
    std::set<Inkscape::UI::ShapeRecord> &s)
{
    using namespace Inkscape::UI;

    if (!obj) {
    	return;
    }

    //XML Tree being used directly here while it shouldn't be.
    if (SP_IS_PATH(obj) && obj->getRepr()->attribute("inkscape:original-d") != NULL) {
        ShapeRecord r;
        r.item = static_cast<SPItem*>(obj);
        r.edit_transform = Geom::identity(); // TODO wrong?
        r.role = role;
        s.insert(r);
    } else if (role != SHAPE_ROLE_NORMAL && (SP_IS_GROUP(obj) || SP_IS_OBJECTGROUP(obj))) {
        for (SPObject *c = obj->children; c; c = c->next) {
            gather_items(nt, base, c, role, s);
        }
    } else if (SP_IS_ITEM(obj)) {
        SPItem *item = static_cast<SPItem*>(obj);
        ShapeRecord r;
        r.item = item;
        // TODO add support for objectBoundingBox
        r.edit_transform = base ? base->i2doc_affine() : Geom::identity();
        r.role = role;

        if (s.insert(r).second) {
            // this item was encountered the first time
            if (nt->edit_clipping_paths && item->clip_ref) {
                gather_items(nt, item, item->clip_ref->getObject(), SHAPE_ROLE_CLIPPING_PATH, s);
            }

            if (nt->edit_masks && item->mask_ref) {
                gather_items(nt, item, item->mask_ref->getObject(), SHAPE_ROLE_MASK, s);
            }
        }
    }
}

void NodeTool::selection_changed(Inkscape::Selection *sel) {
    using namespace Inkscape::UI;

    std::set<ShapeRecord> shapes;

    GSList const *ilist = sel->itemList();

    for (GSList *i = const_cast<GSList*>(ilist); i; i = i->next) {
        SPObject *obj = static_cast<SPObject*>(i->data);

        if (SP_IS_ITEM(obj)) {
            gather_items(this, NULL, static_cast<SPItem*>(obj), SHAPE_ROLE_NORMAL, shapes);
        }
    }

    // use multiple ShapeEditors for now, to allow editing many shapes at once
    // needs to be rethought
    for (boost::ptr_map<SPItem*, ShapeEditor>::iterator i = this->_shape_editors.begin();
         i != this->_shape_editors.end(); )
    {
        ShapeRecord s;
        s.item = i->first;

        if (shapes.find(s) == shapes.end()) {
            this->_shape_editors.erase(i++);
        } else {
            ++i;
        }
    }

    for (std::set<ShapeRecord>::iterator i = shapes.begin(); i != shapes.end(); ++i) {
        ShapeRecord const &r = *i;

        if ((SP_IS_SHAPE(r.item) || SP_IS_TEXT(r.item) || SP_IS_GROUP(r.item) || SP_IS_OBJECTGROUP(r.item)) &&
            this->_shape_editors.find(r.item) == this->_shape_editors.end())
        {
            ShapeEditor *si = new ShapeEditor(this->desktop);
            si->set_item(r.item);
            this->_shape_editors.insert(const_cast<SPItem*&>(r.item), si);
        }
    }

    this->_multipath->setItems(shapes);
    this->update_tip(NULL);
    this->desktop->updateNow();
}

bool NodeTool::root_handler(GdkEvent* event) {
    /* things to handle here:
     * 1. selection of items
     * 2. passing events to manipulators
     * 3. some keybindings
     */
    using namespace Inkscape::UI; // pull in event helpers
    
    Inkscape::Selection *selection = desktop->selection;
    static Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (this->_multipath->event(this, event)) {
    	return true;
    }

    if (this->_selector->event(this, event)) {
    	return true;
    }

    if (this->_selected_nodes->event(this, event)) {
    	return true;
    }

    switch (event->type)
    {
    case GDK_MOTION_NOTIFY: {
	this->update_helperpath();
        combine_motion_events(desktop->canvas, event->motion, 0);
        this->update_helperpath();
        SPItem *over_item = sp_event_context_find_item (desktop, event_point(event->button),
                FALSE, TRUE);

        if (over_item != this->_last_over) {
            this->_last_over = over_item;
            //ink_node_tool_update_tip(nt, event);
            this->update_tip(event);
        }
        // create pathflash outline
        if (prefs->getBool("/tools/nodes/pathflash_enabled")) {
            if (over_item == this->flashed_item) {
            	break;
            }

            if (!prefs->getBool("/tools/nodes/pathflash_selected") && selection->includes(over_item)) {
            	break;
            }

            if (this->flash_tempitem) {
                desktop->remove_temporary_canvasitem(this->flash_tempitem);
                this->flash_tempitem = NULL;
                this->flashed_item = NULL;
            }

            if (!SP_IS_SHAPE(over_item)) {
            	break; // for now, handle only shapes
            }

            this->flashed_item = over_item;
            SPCurve *c = SP_SHAPE(over_item)->getCurveBeforeLPE();

            if (!c) {
            	break; // break out when curve doesn't exist
            }

            c->transform(over_item->i2dt_affine());
            SPCanvasItem *flash = sp_canvas_bpath_new(desktop->getTempGroup(), c);

            sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(flash),
                //prefs->getInt("/tools/nodes/highlight_color", 0xff0000ff), 1.0,
                over_item->highlight_color(), 1.0,
                SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

            sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(flash), 0, SP_WIND_RULE_NONZERO);

            this->flash_tempitem = desktop->add_temporary_canvasitem(flash,
                prefs->getInt("/tools/nodes/pathflash_timeout", 500));

            c->unref();
        }
        } break; // do not return true, because we need to pass this event to the parent context
        // otherwise some features cease to work

    case GDK_KEY_PRESS:
        switch (get_group0_keyval(&event->key))
        {
        case GDK_KEY_Escape: // deselect everything
            if (this->_selected_nodes->empty()) {
                Inkscape::SelectionHelper::selectNone(desktop);
            } else {
                this->_selected_nodes->clear();
            }
            //ink_node_tool_update_tip(nt, event);
            this->update_tip(event);
            return TRUE;

        case GDK_KEY_a:
        case GDK_KEY_A:
            if (held_control(event->key) && held_alt(event->key)) {
                this->_selected_nodes->selectAll();
                // Ctrl+A is handled in selection-chemistry.cpp via verb
                //ink_node_tool_update_tip(nt, event);
                this->update_tip(event);
                return TRUE;
            }
            break;

        case GDK_KEY_h:
        case GDK_KEY_H:
            if (held_only_control(event->key)) {
                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                prefs->setBool("/tools/nodes/show_handles", !this->show_handles);
                return TRUE;
            }
            break;

        default:
            break;
        }
        //ink_node_tool_update_tip(nt, event);
        this->update_tip(event);
        break;

    case GDK_KEY_RELEASE:
        //ink_node_tool_update_tip(nt, event);
    	this->update_tip(event);
        break;

    default:
    	break;
    }
    
    return ToolBase::root_handler(event);
}

void NodeTool::update_tip(GdkEvent *event) {
    using namespace Inkscape::UI;

    if (event && (event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE)) {
        unsigned new_state = state_after_event(event);

        if (new_state == event->key.state) {
        	return;
        }

        if (state_held_shift(new_state)) {
            if (this->_last_over) {
                this->message_context->set(Inkscape::NORMAL_MESSAGE,
                    C_("Node tool tip", "<b>Shift</b>: drag to add nodes to the selection, "
                    "click to toggle object selection"));
            } else {
                this->message_context->set(Inkscape::NORMAL_MESSAGE,
                    C_("Node tool tip", "<b>Shift</b>: drag to add nodes to the selection"));
            }

            return;
        }
    }

    unsigned sz = this->_selected_nodes->size();
    unsigned total = this->_selected_nodes->allPoints().size();

    if (sz != 0) {
        char *nodestring = g_strdup_printf(
            ngettext("<b>%u of %u</b> node selected.", "<b>%u of %u</b> nodes selected.", total),
            sz, total);

        if (this->_last_over) {
            // TRANSLATORS: The %s below is where the "%u of %u nodes selected" sentence gets put
            char *dyntip = g_strdup_printf(C_("Node tool tip",
                "%s Drag to select nodes, click to edit only this object (more: Shift)"),
                nodestring);
            this->message_context->set(Inkscape::NORMAL_MESSAGE, dyntip);
            g_free(dyntip);
        } else {
            char *dyntip = g_strdup_printf(C_("Node tool tip",
                "%s Drag to select nodes, click clear the selection"),
                nodestring);
            this->message_context->set(Inkscape::NORMAL_MESSAGE, dyntip);
            g_free(dyntip);
        }
        g_free(nodestring);
    } else if (!this->_multipath->empty()) {
        if (this->_last_over) {
            this->message_context->set(Inkscape::NORMAL_MESSAGE, C_("Node tool tip",
                "Drag to select nodes, click to edit only this object"));
        } else {
            this->message_context->set(Inkscape::NORMAL_MESSAGE, C_("Node tool tip",
                "Drag to select nodes, click to clear the selection"));
        }
    } else {
        if (this->_last_over) {
            this->message_context->set(Inkscape::NORMAL_MESSAGE, C_("Node tool tip",
                "Drag to select objects to edit, click to edit this object (more: Shift)"));
        } else {
            this->message_context->set(Inkscape::NORMAL_MESSAGE, C_("Node tool tip",
                "Drag to select objects to edit"));
        }
    }
}

void NodeTool::select_area(Geom::Rect const &sel, GdkEventButton *event) {
    using namespace Inkscape::UI;

    if (this->_multipath->empty()) {
        // if multipath is empty, select rubberbanded items rather than nodes
        Inkscape::Selection *selection = this->desktop->selection;
        GSList *items = this->desktop->getDocument()->getItemsInBox(this->desktop->dkey, sel);
        selection->setList(items);
        g_slist_free(items);
    } else {
        if (!held_shift(*event)) {
        	this->_selected_nodes->clear();
        }

        this->_selected_nodes->selectArea(sel);
    }
}

void NodeTool::select_point(Geom::Point const &/*sel*/, GdkEventButton *event) {
    using namespace Inkscape::UI; // pull in event helpers

    if (!event) {
    	return;
    }

    if (event->button != 1) {
    	return;
    }

    Inkscape::Selection *selection = this->desktop->selection;

    SPItem *item_clicked = sp_event_context_find_item (this->desktop, event_point(*event),
                    (event->state & GDK_MOD1_MASK) && !(event->state & GDK_CONTROL_MASK), TRUE);

    if (item_clicked == NULL) { // nothing under cursor
        // if no Shift, deselect
        // if there are nodes selected, the first click should deselect the nodes
        // and the second should deselect the items
        if (!state_held_shift(event->state)) {
            if (this->_selected_nodes->empty()) {
                selection->clear();
            } else {
                this->_selected_nodes->clear();
            }
        }
    } else {
        if (held_shift(*event)) {
            selection->toggle(item_clicked);
        } else {
            selection->set(item_clicked);
        }

        this->desktop->updateNow();
    }
}

void NodeTool::mouseover_changed(Inkscape::UI::ControlPoint *p) {
    using Inkscape::UI::CurveDragPoint;

    CurveDragPoint *cdp = dynamic_cast<CurveDragPoint*>(p);

    if (cdp && !this->cursor_drag) {
        this->cursor_shape = cursor_node_d_xpm;
        this->hot_x = 1;
        this->hot_y = 1;
        this->sp_event_context_update_cursor();
        this->cursor_drag = true;
    } else if (!cdp && this->cursor_drag) {
        this->cursor_shape = cursor_node_xpm;
        this->hot_x = 1;
        this->hot_y = 1;
        this->sp_event_context_update_cursor();
        this->cursor_drag = false;
    }
}

void NodeTool::handleControlUiStyleChange() {
    this->_multipath->updateHandles();
}

}
}
}

//} // anonymous namespace

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

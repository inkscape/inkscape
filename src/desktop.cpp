/*
 * Editable view implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Jon A. Cruz
 * Copyright (C) 2006-2008 Johan Engelen
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ui/dialog/dialog-manager.h"
#include <glibmm/i18n.h>
#include <sigc++/functors/mem_fun.h>

#include <2geom/transforms.h>
#include <2geom/rect.h>

#include "ui/tools/box3d-tool.h"
#include "color.h"
#include "desktop-events.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "device-manager.h"
#include "display/canvas-arena.h"
#include "display/canvas-grid.h"
#include "display/canvas-temporary-item-list.h"
#include "display/drawing-group.h"
#include "display/gnome-canvas-acetate.h"
#include "display/drawing.h"
#include "display/snap-indicator.h"
#include "display/sodipodi-ctrlrect.h"
#include "display/sp-canvas-group.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-util.h"
#include "document.h"
#include "document-undo.h"
#include "event-log.h"
#include "helper/action-context.h"
#include "interface.h"
#include "inkscape-private.h"
#include "layer-fns.h"
#include "layer-manager.h"
#include "layer-model.h"
#include "macros.h"
#include "message-context.h"
#include "message-stack.h"
#include "preferences.h"
#include "resource-manager.h"
#include "ui/tools/select-tool.h"
#include "selection.h"
#include "sp-item-group.h"
#include "sp-item-group.h"
#include "sp-namedview.h"
#include "sp-root.h"
#include "sp-defs.h"
#include "tool-factory.h"
#include "widgets/desktop-widget.h"
#include "xml/repr.h"
#include "helper/action.h" //sp_action_perform

// TODO those includes are only for node tool quick zoom. Remove them after fixing it.
#include "ui/tools/node-tool.h"
#include "ui/tool/control-point-selection.h"

namespace Inkscape { namespace XML { class Node; }}

// Callback declarations
static void _onSelectionChanged (Inkscape::Selection *selection, SPDesktop *desktop);
static gint _arena_handler (SPCanvasArena *arena, Inkscape::DrawingItem *ai, GdkEvent *event, SPDesktop *desktop);
static void _layer_activated(SPObject *layer, SPDesktop *desktop);
static void _layer_deactivated(SPObject *layer, SPDesktop *desktop);
static void _layer_hierarchy_changed(SPObject *top, SPObject *bottom, SPDesktop *desktop);
static void _reconstruction_start(SPDesktop * desktop);
static void _reconstruction_finish(SPDesktop * desktop);
static void _namedview_modified (SPObject *obj, guint flags, SPDesktop *desktop);

SPDesktop::SPDesktop() :
    _dlg_mgr( 0 ),
    namedview( 0 ),
    canvas( 0 ),
    layers( 0 ),
    selection( 0 ),
    event_context( 0 ),
    layer_manager( 0 ),
    event_log( 0 ),
    temporary_item_list( 0 ),
    snapindicator( 0 ),
    acetate( 0 ),
    main( 0 ),
    gridgroup( 0 ),
    guides( 0 ),
    drawing( 0 ),
    sketch( 0 ),
    controls( 0 ),
    tempgroup ( 0 ),
    table( 0 ),
    page( 0 ),
    page_border( 0 ),
    current( 0 ),
    _focusMode(false),
    dkey( 0 ),
    number( 0 ),
    window_state(0),
    interaction_disabled_counter( 0 ),
    waiting_cursor( false ),
    showing_dialogs ( false ),
    guides_active( false ),
    gr_item( 0 ),
    gr_point_type( POINT_LG_BEGIN ),
    gr_point_i( 0 ),
    gr_fill_or_stroke( Inkscape::FOR_FILL ),
    _reconstruction_old_layer_id(), // an id attribute is not allowed to be the empty string
    _display_mode(Inkscape::RENDERMODE_NORMAL),
    _display_color_mode(Inkscape::COLORMODE_NORMAL),
    _widget( 0 ),
    _inkscape( 0 ),
    _guides_message_context( 0 ),
    _active( false ),
    _w2d(),
    _d2w(),
    _doc2dt( Geom::Scale(1, -1) ),
    // This doesn't work I don't know why.
    _image_render_observer(this, "/options/rendering/imageinoutlinemode"),
    grids_visible( false )
{
    _d2w.setIdentity();
    _w2d.setIdentity();
    
    layers = new Inkscape::LayerModel();
    layers->_layer_activated_signal.connect(sigc::bind(sigc::ptr_fun(_layer_activated), this));
    layers->_layer_deactivated_signal.connect(sigc::bind(sigc::ptr_fun(_layer_deactivated), this));
    layers->_layer_changed_signal.connect(sigc::bind(sigc::ptr_fun(_layer_hierarchy_changed), this));
    selection = Inkscape::GC::release( new Inkscape::Selection(layers, this) );
}

void
SPDesktop::init (SPNamedView *nv, SPCanvas *aCanvas, Inkscape::UI::View::EditWidgetInterface *widget)
{
    _widget = widget;

    // Temporary workaround for link order issues:
    Inkscape::DeviceManager::getManager().getDevices();
    Inkscape::ResourceManager::getManager();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    _guides_message_context = new Inkscape::MessageContext(const_cast<Inkscape::MessageStack*>(messageStack()));

    current = prefs->getStyle("/desktop/style");

    namedview = nv;
    canvas = aCanvas;

    SPDocument *document = namedview->document;
    /* XXX:
     * ensureUpToDate() sends a 'modified' signal to the root element.
     * This is reportedly required to prevent flickering after the document
     * loads. However, many SPObjects write to their repr in response
     * to this signal. This is apparently done to support live path effects,
     * which rewrite their result paths after each modification of the base object.
     * This causes the generation of an incomplete undo transaction,
     * which causes problems down the line, including crashes in the
     * Undo History dialog.
     *
     * For now, this is handled by disabling undo tracking during this call.
     * A proper fix would involve modifying the way ensureUpToDate() works,
     * so that the LPE results are not rewritten.
     */
    Inkscape::DocumentUndo::setUndoSensitive(document, false);
    document->ensureUpToDate();
    Inkscape::DocumentUndo::setUndoSensitive(document, true);

    /* Setup Dialog Manager */
    _dlg_mgr = &Inkscape::UI::Dialog::DialogManager::getInstance();

    dkey = SPItem::display_key_new(1);

    /* Connect display key to layer model */
    layers->setDisplayKey(dkey);

    /* Connect document */
    setDocument (document);

    number = namedview->getViewCount();


    /* Setup Canvas */
    g_object_set_data (G_OBJECT (canvas), "SPDesktop", this);

    SPCanvasGroup *root = canvas->getRoot();

    /* Setup adminstrative layers */
    acetate = sp_canvas_item_new (root, GNOME_TYPE_CANVAS_ACETATE, NULL);
    g_signal_connect (G_OBJECT (acetate), "event", G_CALLBACK (sp_desktop_root_handler), this);
    main = (SPCanvasGroup *) sp_canvas_item_new (root, SP_TYPE_CANVAS_GROUP, NULL);
    g_signal_connect (G_OBJECT (main), "event", G_CALLBACK (sp_desktop_root_handler), this);

    table = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);
    SP_CTRLRECT(table)->setRectangle(Geom::Rect(Geom::Point(-80000, -80000), Geom::Point(80000, 80000)));
    SP_CTRLRECT(table)->setColor(0x00000000, true, 0x00000000);
    sp_canvas_item_move_to_z (table, 0);

    page = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);
    ((CtrlRect *) page)->setColor(0x00000000, FALSE, 0x00000000);
    page_border = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);

    drawing = sp_canvas_item_new (main, SP_TYPE_CANVAS_ARENA, NULL);
    g_signal_connect (G_OBJECT (drawing), "arena_event", G_CALLBACK (_arena_handler), this);

    SP_CANVAS_ARENA (drawing)->drawing.delta = prefs->getDouble("/options/cursortolerance/value", 1.0); // default is 1 px

    if (prefs->getBool("/options/startmode/outline")) {
        // Start in outline mode
        setDisplayModeOutline();
    } else {
        // Start in normal mode, default
        setDisplayModeNormal();
    }

    // The order in which these canvas items are added determines the z-order. It's therefore
    // important to add the tempgroup (which will contain the snapindicator) before adding the
    // controls. Only this way one will be able to quickly (before the snap indicator has
    // disappeared) reselect a node after snapping it. If the z-order is wrong however, this
    // will not work (the snap indicator is on top of the node handler; is the snapindicator
    // being selected? or does it intercept some of the events that should have gone to the
    // node handler? see bug https://bugs.launchpad.net/inkscape/+bug/414142)
    gridgroup = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    guides = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    sketch = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    tempgroup = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    controls = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);

    // Set the select tool as the active tool.
    set_event_context2("/tools/select");

    // display rect and zoom are now handled in sp_desktop_widget_realize()

    Geom::Rect const d(Geom::Point(0.0, 0.0),
                       Geom::Point(document->getWidth().value("px"), document->getHeight().value("px")));

    SP_CTRLRECT(page)->setRectangle(d);
    SP_CTRLRECT(page_border)->setRectangle(d);

    /* the following sets the page shadow on the canvas
       It was originally set to 5, which is really cheesy!
       It now is an attribute in the document's namedview. If a value of
       0 is used, then the constructor for a shadow is not initialized.
    */

    if ( namedview->pageshadow != 0 && namedview->showpageshadow ) {
        SP_CTRLRECT(page_border)->setShadow(namedview->pageshadow, 0x3f3f3fff);
    }


    /* Connect event for page resize */
    _doc2dt[5] = document->getHeight().value("px");
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (drawing), _doc2dt);

    _modified_connection = namedview->connectModified(sigc::bind<2>(sigc::ptr_fun(&_namedview_modified), this));

    Inkscape::DrawingItem *ai = document->getRoot()->invoke_show(
            SP_CANVAS_ARENA (drawing)->drawing,
            dkey,
            SP_ITEM_SHOW_DISPLAY);
    if (ai) {
        SP_CANVAS_ARENA (drawing)->drawing.root()->prependChild(ai);
    }

    namedview->show(this);
    /* Ugly hack */
    activate_guides (true);
    /* Ugly hack */
    _namedview_modified (namedview, SP_OBJECT_MODIFIED_FLAG, this);

/* Set up notification of rebuilding the document, this allows
       for saving object related settings in the document. */
    _reconstruction_start_connection =
        document->connectReconstructionStart(sigc::bind(sigc::ptr_fun(_reconstruction_start), this));
    _reconstruction_finish_connection =
        document->connectReconstructionFinish(sigc::bind(sigc::ptr_fun(_reconstruction_finish), this));
    _reconstruction_old_layer_id.clear();

    // ?
    // sp_active_desktop_set (desktop);
    _inkscape = INKSCAPE;

    _activate_connection = _activate_signal.connect(
        sigc::bind(
            sigc::ptr_fun(_onActivate),
            this
        )
    );
     _deactivate_connection = _deactivate_signal.connect(
        sigc::bind(
            sigc::ptr_fun(_onDeactivate),
            this
        )
    );

    _sel_modified_connection = selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&_onSelectionModified),
            this
        )
    );
    _sel_changed_connection = selection->connectChanged(
        sigc::bind(
            sigc::ptr_fun(&_onSelectionChanged),
            this
        )
    );


    /* setup LayerManager */
    //   (Setting up after the connections are all in place, as it may use some of them)
    layer_manager = new Inkscape::LayerManager( this );

    showGrids(namedview->grids_visible, false);

    temporary_item_list = new Inkscape::Display::TemporaryItemList( this );
    snapindicator = new Inkscape::Display::SnapIndicator ( this );
}


void SPDesktop::destroy()
{
    _destroy_signal.emit(this);

    if (snapindicator) {
        delete snapindicator;
        snapindicator = NULL;
    }
    if (temporary_item_list) {
        delete temporary_item_list;
        temporary_item_list = NULL;
    }

    if (selection) {
        delete selection;
        selection = NULL;
    }

    namedview->hide(this);

    _activate_connection.disconnect();
    _deactivate_connection.disconnect();
    _sel_modified_connection.disconnect();
    _sel_changed_connection.disconnect();
    _modified_connection.disconnect();
    _commit_connection.disconnect();
    _reconstruction_start_connection.disconnect();
    _reconstruction_finish_connection.disconnect();

    g_signal_handlers_disconnect_by_func(G_OBJECT (acetate), (gpointer) G_CALLBACK(sp_desktop_root_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT (main), (gpointer) G_CALLBACK(sp_desktop_root_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT (drawing), (gpointer) G_CALLBACK(_arena_handler), this);

    if (event_context) {
        event_context->finish();
    	delete event_context;
    	event_context = NULL;
    }

    delete layers;

    if (layer_manager) {
        delete layer_manager;
        layer_manager = NULL;
    }

    if (_inkscape) {
        _inkscape = NULL;
    }

    if (drawing) {
        doc()->getRoot()->invoke_hide(dkey);
        g_object_unref(drawing);
        drawing = NULL;
    }

    delete _guides_message_context;
    _guides_message_context = NULL;
}

SPDesktop::~SPDesktop()
{
}


Inkscape::UI::Tools::ToolBase* SPDesktop::getEventContext() const {
	return event_context;
}

Inkscape::Selection* SPDesktop::getSelection() const {
	return selection;
}

SPDocument* SPDesktop::getDocument() const {
	return doc();
}

SPCanvas* SPDesktop::getCanvas() const {
	return SP_CANVAS_ITEM(main)->canvas;
}

SPCanvasItem* SPDesktop::getAcetate() const {
	return acetate;
}

SPCanvasGroup* SPDesktop::getMain() const {
	return main;
}

SPCanvasGroup* SPDesktop::getGridGroup() const {
	return gridgroup;
}

SPCanvasGroup* SPDesktop::getGuides() const {
	return guides;
}

SPCanvasItem* SPDesktop::getDrawing() const {
	return drawing;
}

SPCanvasGroup* SPDesktop::getSketch() const {
	return sketch;
}

SPCanvasGroup* SPDesktop::getControls() const {
	return controls;
}

SPCanvasGroup* SPDesktop::getTempGroup() const {
	return tempgroup;
}

Inkscape::MessageStack* SPDesktop::getMessageStack() const {
	return messageStack();
}

SPNamedView* SPDesktop::getNamedView() const {
	return namedview;
}



//--------------------------------------------------------------------
/* Public methods */


/* These methods help for temporarily showing things on-canvas.
 * The *only* valid use of the TemporaryItem* that you get from add_temporary_canvasitem
 * is when you want to prematurely remove the item from the canvas, by calling
 * desktop->remove_temporary_canvasitem(tempitem).
 */
/** Note that lifetime is measured in milliseconds
 * One should *not* keep a reference to the SPCanvasItem, the temporary item code will
 * delete the object for you and the reference will become invalid without you knowing it.
 * It is perfectly safe to ignore the returned pointer: the object is deleted by itself, so don't delete it elsewhere!
 * The *only* valid use of the returned TemporaryItem* is as argument for SPDesktop::remove_temporary_canvasitem,
 * because the object might be deleted already without you knowing it.
 * move_to_bottom = true by default so the item does not interfere with handling of other items on the canvas like nodes.
 */
Inkscape::Display::TemporaryItem *
SPDesktop::add_temporary_canvasitem (SPCanvasItem *item, guint lifetime, bool move_to_bottom)
{
    if (move_to_bottom) {
        sp_canvas_item_move_to_z(item, 0);
    }

    return temporary_item_list->add_item(item, lifetime);
}

/** It is perfectly safe to call this function while the object has already been deleted due to a timeout.
*/
void
SPDesktop::remove_temporary_canvasitem (Inkscape::Display::TemporaryItem * tempitem)
{
    // check for non-null temporary_item_list, because during destruction of desktop, some destructor might try to access this list!
    if (tempitem && temporary_item_list) {
        temporary_item_list->delete_item(tempitem);
    }
}

void SPDesktop::redrawDesktop() {
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (main), _d2w); // redraw
}

void SPDesktop::_setDisplayMode(Inkscape::RenderMode mode) {
    SP_CANVAS_ARENA (drawing)->drawing.setRenderMode(mode);
    canvas->rendermode = mode;
    _display_mode = mode;
    redrawDesktop();
    _widget->setTitle( sp_desktop_document(this)->getName() );
}
void SPDesktop::_setDisplayColorMode(Inkscape::ColorMode mode) {
    // reload grayscale matrix from prefs
    if (mode == Inkscape::COLORMODE_GRAYSCALE) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gdouble r = prefs->getDoubleLimited("/options/rendering/grayscale/red-factor",0.21,0.,1.);
        gdouble g = prefs->getDoubleLimited("/options/rendering/grayscale/green-factor",0.72,0.,1.);
        gdouble b = prefs->getDoubleLimited("/options/rendering/grayscale/blue-factor",0.072,0.,1.);
        gdouble grayscale_value_matrix[20] = { r, g, b, 0, 0,
                                               r, g, b, 0, 0,
                                               r, g, b, 0, 0,
                                               0, 0, 0, 1, 0 };
        g_message("%g",grayscale_value_matrix[0]);
        SP_CANVAS_ARENA (drawing)->drawing.setGrayscaleMatrix(grayscale_value_matrix);
    }

    SP_CANVAS_ARENA (drawing)->drawing.setColorMode(mode);
    canvas->colorrendermode = mode;
    _display_color_mode = mode;
    redrawDesktop();
    _widget->setTitle( sp_desktop_document(this)->getName() );
}

void SPDesktop::displayModeToggle() {
    switch (_display_mode) {
    case Inkscape::RENDERMODE_NORMAL:
        _setDisplayMode(Inkscape::RENDERMODE_NO_FILTERS);
        break;
    case Inkscape::RENDERMODE_NO_FILTERS:
        _setDisplayMode(Inkscape::RENDERMODE_OUTLINE);
        break;
    case Inkscape::RENDERMODE_OUTLINE:
        _setDisplayMode(Inkscape::RENDERMODE_NORMAL);
        break;
    default:
        _setDisplayMode(Inkscape::RENDERMODE_NORMAL);
    }
}
void SPDesktop::displayColorModeToggle() {
    switch (_display_color_mode) {
    case Inkscape::COLORMODE_NORMAL:
        _setDisplayColorMode(Inkscape::COLORMODE_GRAYSCALE);
        break;
    case Inkscape::COLORMODE_GRAYSCALE:
        _setDisplayColorMode(Inkscape::COLORMODE_NORMAL);
        break;
//    case Inkscape::COLORMODE_PRINT_COLORS_PREVIEW:
    default:
        _setDisplayColorMode(Inkscape::COLORMODE_NORMAL);
    }
}

// Pass-through LayerModel functions
SPObject *SPDesktop::currentRoot() const
{
    return layers->currentRoot();
}

SPObject *SPDesktop::currentLayer() const
{
    return layers->currentLayer();
}

void SPDesktop::setCurrentLayer(SPObject *object)
{
    layers->setCurrentLayer(object);
}

void SPDesktop::toggleLayerSolo(SPObject *object)
{
    layers->toggleLayerSolo(object);
}

void SPDesktop::toggleHideAllLayers(bool hide)
{
    layers->toggleHideAllLayers(hide);
}

void SPDesktop::toggleLockAllLayers(bool lock)
{
    layers->toggleLockAllLayers(lock);
}

void SPDesktop::toggleLockOtherLayers(SPObject *object)
{
    layers->toggleLockOtherLayers(object);
}

bool SPDesktop::isLayer(SPObject *object) const
{
    return layers->isLayer(object);
}

/**
 * True if desktop viewport intersects \a item's bbox.
 */
bool SPDesktop::isWithinViewport (SPItem *item) const
{
    Geom::Rect const viewport = get_display_area();
    Geom::OptRect const bbox = item->desktopVisualBounds();
    if (bbox) {
        return viewport.intersects(*bbox);
    } else {
        return false;
    }
}

///
bool SPDesktop::itemIsHidden(SPItem const *item) const {
    return item->isHidden(this->dkey);
}

/**
 * Set activate property of desktop; emit signal if changed.
 */
void
SPDesktop::set_active (bool new_active)
{
    if (new_active != _active) {
        _active = new_active;
        if (new_active) {
            _activate_signal.emit();
        } else {
            _deactivate_signal.emit();
        }
    }
}

/**
 * Set activate status of current desktop's named view.
 */
void
SPDesktop::activate_guides(bool activate)
{
    guides_active = activate;
    namedview->activateGuides(this, activate);
}

/**
 * Make desktop switch documents.
 */
void
SPDesktop::change_document (SPDocument *theDocument)
{
    g_return_if_fail (theDocument != NULL);

    /* unselect everything before switching documents */
    selection->clear();

    setDocument (theDocument);

    /* update the rulers, connect the desktop widget's signal to the new namedview etc.
       (this can probably be done in a better way) */
    Gtk::Window *parent = this->getToplevel();
    g_assert(parent != NULL);
    SPDesktopWidget *dtw = (SPDesktopWidget *) parent->get_data("desktopwidget");
    if (dtw) {
        dtw->desktop = this;
        dtw->updateNamedview();
    }

    _namedview_modified (namedview, SP_OBJECT_MODIFIED_FLAG, this);
    _document_replaced_signal.emit (this, theDocument);
}

/**
 * Replaces the currently active tool with a new one.
 */
void SPDesktop::set_event_context2(const std::string& toolName)
{
    Inkscape::UI::Tools::ToolBase* old_tool = event_context;

    if (old_tool) {
        if (toolName.compare(old_tool->pref_observer->observed_path) != 0) {
            //g_message("Old tool: %s", old_tool->pref_observer->observed_path.c_str());
            //g_message("New tool: %s", toolName.c_str());
            old_tool->finish();
            delete old_tool;
        } else {
            _event_context_changed_signal.emit(this, event_context);
            return;
        }
    }
    
    Inkscape::UI::Tools::ToolBase* new_tool = ToolFactory::instance().createObject(toolName);
    new_tool->desktop = this;
    new_tool->message_context = new Inkscape::MessageContext(this->messageStack());
    event_context = new_tool;
    new_tool->setup();

    // Make sure no delayed snapping events are carried over after switching tools
    // (this is only an additional safety measure against sloppy coding, because each
    // tool should take care of this by itself)
    sp_event_context_discard_delayed_snap_event(event_context);

    _event_context_changed_signal.emit(this, event_context);
}

/**
 * Sets the coordinate status to a given point
 */
void
SPDesktop::set_coordinate_status (Geom::Point p) {
    _widget->setCoordinateStatus(p);
}

Inkscape::UI::Widget::Dock* SPDesktop::getDock() {
	return _widget->getDock();
}

/**
 * \see SPDocument::getItemFromListAtPointBottom()
 */
SPItem *SPDesktop::getItemFromListAtPointBottom(const GSList *list, Geom::Point const &p) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return SPDocument::getItemFromListAtPointBottom(dkey, doc()->getRoot(), list, p);
}

/**
 * \see SPDocument::getItemAtPoint()
 */
SPItem *SPDesktop::getItemAtPoint(Geom::Point const &p, bool into_groups, SPItem *upto) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return doc()->getItemAtPoint( dkey, p, into_groups, upto);
}

/**
 * \see SPDocument::getGroupAtPoint()
 */
SPItem *SPDesktop::getGroupAtPoint(Geom::Point const &p) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return doc()->getGroupAtPoint(dkey, p);
}

/**
 * Returns the mouse point in document coordinates; if mouse is
 * outside the canvas, returns the center of canvas viewpoint.
 */
Geom::Point SPDesktop::point() const
{
    Geom::Point p = _widget->getPointer();
    Geom::Point pw = sp_canvas_window_to_world (canvas, p);
    p = w2d(pw);

    Geom::Rect const r = canvas->getViewbox();

    Geom::Point r0 = w2d(r.min());
    Geom::Point r1 = w2d(r.max());

    if (p[Geom::X] >= r0[Geom::X] &&
        p[Geom::X] <= r1[Geom::X] &&
        p[Geom::Y] >= r1[Geom::Y] &&
        p[Geom::Y] <= r0[Geom::Y])
    {
        return p;
    } else {
        return (r0 + r1) / 2;
    }
}

/**
 * Put current zoom data in history list.
 */
void
SPDesktop::push_current_zoom (std::list<Geom::Rect> &history)
{
    Geom::Rect area = get_display_area();

    if (history.empty() || history.front() != area) {
        history.push_front(area);
    }
}

/**
 * Set viewbox (x0, x1, y0 and y1 are in document pixels. Border is in screen pixels).
 */
void
SPDesktop::set_display_area (double x0, double y0, double x1, double y1, double border, bool log)
{
    g_assert(_widget);
    bool zoomChanged = false;

    // save the zoom
    if (log) {
        push_current_zoom(zooms_past);
        // if we do a logged zoom, our zoom-forward list is invalidated, so delete it
        zooms_future.clear();
    }

    double const cx = 0.5 * (x0 + x1);
    double const cy = 0.5 * (y0 + y1);

    // FIXME: This 2geom idiom doesn't allow us to declare dbox const
    Geom::Rect viewbox = canvas->getViewbox();
    viewbox.expandBy(-border);

    double scale = _d2w.descrim();
    double newscale;
    if (((x1 - x0) * viewbox.dimensions()[Geom::Y]) > ((y1 - y0) * viewbox.dimensions()[Geom::X])) {
        newscale = viewbox.dimensions()[Geom::X] / (x1 - x0);
    } else {
        newscale = viewbox.dimensions()[Geom::Y] / (y1 - y0);
    }

    newscale = CLAMP(newscale, SP_DESKTOP_ZOOM_MIN, SP_DESKTOP_ZOOM_MAX); // unit: 'screen pixels' per 'document pixels'

    int clear = FALSE;
    if (!Geom::are_near(newscale, scale, Geom::EPSILON * scale)) {
        // zoom changed - set new zoom factors
        _d2w = Geom::Scale(newscale, -newscale);
        _w2d = Geom::Scale(1/newscale, 1/-newscale);
        redrawDesktop();
        clear = TRUE;
        zoomChanged = true;
    }

    /* Calculate top left corner (in document pixels) */
    x0 = cx - 0.5 * viewbox.dimensions()[Geom::X] / newscale;
    y1 = cy + 0.5 * viewbox.dimensions()[Geom::Y] / newscale;

    // Scroll
    canvas->scrollTo(x0 * newscale - border, y1 * -newscale - border, clear);

    /*  update perspective lines if we are in the 3D box tool (so that infinite ones are shown correctly) */
    //sp_box3d_context_update_lines(event_context);
    if (SP_IS_BOX3D_CONTEXT(event_context)) {
    	SP_BOX3D_CONTEXT(event_context)->_vpdrag->updateLines();
    }

    _widget->updateRulers();
    _widget->updateScrollbars(_d2w.descrim());
    _widget->updateZoom();

    if ( zoomChanged ) {
        signal_zoom_changed.emit(_d2w.descrim());
    }
}

void SPDesktop::set_display_area(Geom::Rect const &a, Geom::Coord b, bool log)
{
    set_display_area(a.min()[Geom::X], a.min()[Geom::Y], a.max()[Geom::X], a.max()[Geom::Y], b, log);
}

/**
 * Return viewbox dimensions.
 */
Geom::Rect SPDesktop::get_display_area() const
{
    Geom::Rect const viewbox = canvas->getViewbox();

    double const scale = _d2w[0];

    /// @fixme hardcoded desktop transform
    return Geom::Rect(Geom::Point(viewbox.min()[Geom::X] / scale, viewbox.max()[Geom::Y] / -scale),
                      Geom::Point(viewbox.max()[Geom::X] / scale, viewbox.min()[Geom::Y] / -scale));
}

/**
 * Revert back to previous zoom if possible.
 */
void
SPDesktop::prev_zoom()
{
    if (zooms_past.empty()) {
        messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No previous zoom."));
        return;
    }

    // push current zoom into forward zooms list
    push_current_zoom (zooms_future);

    // restore previous zoom
    Geom::Rect past = zooms_past.front();
    set_display_area (past.left(), past.top(), past.right(), past.bottom(), 0, false);

    // remove the just-added zoom from the past zooms list
    zooms_past.pop_front();
}

/**
 * Set zoom to next in list.
 */
void SPDesktop::next_zoom()
{
    if (zooms_future.empty()) {
        this->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No next zoom."));
        return;
    }

    // push current zoom into past zooms list
    push_current_zoom (zooms_past);

    // restore next zoom
    Geom::Rect future = zooms_future.front();
    set_display_area (future.left(), future.top(), future.right(), future.bottom(), 0, false);

    // remove the just-used zoom from the zooms_future list
    zooms_future.pop_front();
}

/**
 * Performs a quick zoom into what the user is working on.
 *
 * @param  enable  Whether we're going in or out of quick zoom.
 */
void SPDesktop::zoom_quick(bool enable)
{
    if (enable == _quick_zoom_enabled) {
        return;
    }

    if (enable == true) {
        _quick_zoom_stored_area = get_display_area();
        bool zoomed = false;

        // TODO This needs to migrate into the node tool, but currently the design
        // of this method is sufficiently wrong to prevent this.
        if (!zoomed && INK_IS_NODE_TOOL(event_context)) {
            Inkscape::UI::Tools::NodeTool *nt = static_cast<Inkscape::UI::Tools::NodeTool*>(event_context);
            if (!nt->_selected_nodes->empty()) {
                Geom::Rect nodes = *nt->_selected_nodes->bounds();
                double area = nodes.area();
                // do not zoom if a single cusp node is selected aand the bounds
                // have zero area.
                if (!Geom::are_near(area, 0) && area * 2.0 < _quick_zoom_stored_area.area()) {
                    set_display_area(nodes, true);
                    zoomed = true;
                }
            }
        }

        if (!zoomed) {
            Geom::OptRect const d = selection->visualBounds();
            if (d && d->area() * 2.0 < _quick_zoom_stored_area.area()) {
                set_display_area(*d, true);
                zoomed = true;
            }
        }

        if (!zoomed) {
            zoom_relative(_quick_zoom_stored_area.midpoint()[Geom::X], _quick_zoom_stored_area.midpoint()[Geom::Y], 2.0);
            zoomed = true;
        }
    } else {
        set_display_area(_quick_zoom_stored_area, false);
    }

    _quick_zoom_enabled = enable;
    return;
}

/**
 * Zoom to point with absolute zoom factor.
 */
void
SPDesktop::zoom_absolute_keep_point (double cx, double cy, double px, double py, double zoom)
{
    zoom = CLAMP (zoom, SP_DESKTOP_ZOOM_MIN, SP_DESKTOP_ZOOM_MAX);

    // maximum or minimum zoom reached, but there's no exact equality because of rounding errors;
    // this check prevents "sliding" when trying to zoom in at maximum zoom;
    /// \todo someone please fix calculations properly and remove this hack
    if (fabs(_d2w.descrim() - zoom) < 0.0001*zoom && (fabs(SP_DESKTOP_ZOOM_MAX - zoom) < 0.01 || fabs(SP_DESKTOP_ZOOM_MIN - zoom) < 0.000001))
        return;

    Geom::Rect const viewbox = canvas->getViewbox();

    double const width2 = viewbox.dimensions()[Geom::X] / zoom;
    double const height2 = viewbox.dimensions()[Geom::Y] / zoom;

    set_display_area(cx - px * width2,
                     cy - py * height2,
                     cx + (1 - px) * width2,
                     cy + (1 - py) * height2,
                     0.0);
}

/**
  * Apply the desktop's current style or the tool style to the object.
  */
void SPDesktop::applyCurrentOrToolStyle(SPObject *obj, Glib::ustring const &tool_path, bool with_text)
{
    SPCSSAttr *css_current = sp_desktop_get_style(this, with_text);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool(tool_path + "/usecurrent") && css_current) {
        obj->setCSS(css_current,"style");
    } else {
        SPCSSAttr *css = prefs->getInheritedStyle(tool_path + "/style");
        obj->setCSS(css,"style");
        sp_repr_css_attr_unref(css);
    }
    if (css_current) {
        sp_repr_css_attr_unref(css_current);
    }
}

/**
 * Zoom to center with absolute zoom factor.
 */
void
SPDesktop::zoom_absolute (double cx, double cy, double zoom)
{
    zoom_absolute_keep_point (cx, cy, 0.5, 0.5, zoom);
}

/**
 * Zoom to point with relative zoom factor.
 */
void
SPDesktop::zoom_relative_keep_point (double cx, double cy, double zoom)
{
    Geom::Rect const area = get_display_area();

    if (cx < area.min()[Geom::X]) {
        cx = area.min()[Geom::X];
    }
    if (cx > area.max()[Geom::X]) {
        cx = area.max()[Geom::X];
    }
    if (cy < area.min()[Geom::Y]) {
        cy = area.min()[Geom::Y];
    }
    if (cy > area.max()[Geom::Y]) {
        cy = area.max()[Geom::Y];
    }

    gdouble const scale = _d2w.descrim() * zoom;
    double const px = (cx - area.min()[Geom::X]) / area.dimensions()[Geom::X];
    double const py = (cy - area.min()[Geom::Y]) / area.dimensions()[Geom::Y];

    zoom_absolute_keep_point(cx, cy, px, py, scale);
}

/**
 * Zoom to center with relative zoom factor.
 */
void
SPDesktop::zoom_relative (double cx, double cy, double zoom)
{
    gdouble scale = _d2w.descrim() * zoom;
    zoom_absolute (cx, cy, scale);
}

/**
 * Set display area to origin and current document dimensions.
 */
void
SPDesktop::zoom_page()
{
    Geom::Rect d(Geom::Point(0, 0),
                 Geom::Point(doc()->getWidth().value("px"), doc()->getHeight().value("px")));

    if (d.minExtent() < 1.0) {
        return;
    }

    set_display_area(d, 10);
}

/**
 * Set display area to current document width.
 */
void
SPDesktop::zoom_page_width()
{
    Geom::Rect const a = get_display_area();

    if (doc()->getWidth().value("px") < 1.0) {
        return;
    }

    Geom::Rect d(Geom::Point(0, a.midpoint()[Geom::Y]),
                 Geom::Point(doc()->getWidth().value("px"), a.midpoint()[Geom::Y]));

    set_display_area(d, 10);
}

/**
 * Zoom to selection.
 */
void
SPDesktop::zoom_selection()
{
    Geom::OptRect const d = selection->visualBounds();

    if ( !d || d->minExtent() < 0.1 ) {
        return;
    }

    set_display_area(*d, 10);
}

/**
 * Tell widget to let zoom widget grab keyboard focus.
 */
void
SPDesktop::zoom_grab_focus()
{
    _widget->letZoomGrabFocus();
}

/**
 * Zoom to whole drawing.
 */
void
SPDesktop::zoom_drawing()
{
    g_return_if_fail (doc() != NULL);
    SPItem *docitem = doc()->getRoot();
    g_return_if_fail (docitem != NULL);

    docitem->bbox_valid = FALSE;
    Geom::OptRect d = docitem->desktopVisualBounds();

    /* Note that the second condition here indicates that
    ** there are no items in the drawing.
    */
    if ( !d || d->minExtent() < 0.1 ) {
        return;
    }

    set_display_area(*d, 10);
}

/**
 * Scroll canvas by specific coordinate amount in svg coordinates.
 */
void
SPDesktop::scroll_world_in_svg_coords (double dx, double dy, bool is_scrolling)
{
    double scale = _d2w.descrim();
    scroll_world(dx*scale, dy*scale, is_scrolling);
}

/**
 * Scroll canvas by specific coordinate amount.
 */
void
SPDesktop::scroll_world (double dx, double dy, bool is_scrolling)
{
    g_assert(_widget);

    Geom::Rect const viewbox = canvas->getViewbox();

    canvas->scrollTo(viewbox.min()[Geom::X] - dx, viewbox.min()[Geom::Y] - dy, FALSE, is_scrolling);

    /*  update perspective lines if we are in the 3D box tool (so that infinite ones are shown correctly) */
    //sp_box3d_context_update_lines(event_context);
    if (SP_IS_BOX3D_CONTEXT(event_context)) {
		SP_BOX3D_CONTEXT(event_context)->_vpdrag->updateLines();
	}

    _widget->updateRulers();
    _widget->updateScrollbars(_d2w.descrim());
}

bool
SPDesktop::scroll_to_point (Geom::Point const &p, gdouble autoscrollspeed)
{
    using Geom::X;
    using Geom::Y;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble autoscrolldistance = (gdouble) prefs->getIntLimited("/options/autoscrolldistance/value", 0, -1000, 10000);

    // autoscrolldistance is in screen pixels, but the display area is in document units
    autoscrolldistance /= _d2w.descrim();
    // FIXME: This 2geom idiom doesn't allow us to declare dbox const
    Geom::Rect dbox = get_display_area();
    dbox.expandBy(-autoscrolldistance);

    if (!(p[X] > dbox.min()[X] && p[X] < dbox.max()[X]) ||
        !(p[Y] > dbox.min()[Y] && p[Y] < dbox.max()[Y])   ) {

        Geom::Point const s_w( p * (Geom::Affine)_d2w );

        gdouble x_to;
        if (p[X] < dbox.min()[X])
            x_to = dbox.min()[X];
        else if (p[X] > dbox.max()[X])
            x_to = dbox.max()[X];
        else
            x_to = p[X];

        gdouble y_to;
        if (p[Y] < dbox.min()[Y])
            y_to = dbox.min()[Y];
        else if (p[Y] > dbox.max()[Y])
            y_to = dbox.max()[Y];
        else
            y_to = p[Y];

        Geom::Point const d_dt(x_to, y_to);
        Geom::Point const d_w( d_dt * _d2w );
        Geom::Point const moved_w( d_w - s_w );

        if (autoscrollspeed == 0)
            autoscrollspeed = prefs->getDoubleLimited("/options/autoscrollspeed/value", 1, 0, 10);

        if (autoscrollspeed != 0)
            scroll_world (autoscrollspeed * moved_w);

        return true;
    }
    return false;
}

bool
SPDesktop::is_iconified()
{
    return 0!=(window_state & GDK_WINDOW_STATE_ICONIFIED);
}

void
SPDesktop::iconify()
{
    _widget->setIconified();
}

bool
SPDesktop::is_maximized()
{
    return 0!=(window_state & GDK_WINDOW_STATE_MAXIMIZED);
}

void
SPDesktop::maximize()
{
    _widget->setMaximized();
}

bool
SPDesktop::is_fullscreen()
{
    return 0!=(window_state & GDK_WINDOW_STATE_FULLSCREEN);
}

void
SPDesktop::fullscreen()
{
    _widget->setFullscreen();
}

/**
 * Checks to see if the user is working in focused mode.
 *
 * @return  the value of \c _focusMode.
 */
bool SPDesktop::is_focusMode()
{
    return _focusMode;
}

/**
 * Changes whether the user is in focus mode or not.
 *
 * @param  mode  Which mode the view should be in.
 */
void SPDesktop::focusMode(bool mode)
{
    if (mode == _focusMode) { return; }

    _focusMode = mode;

    layoutWidget();
    //sp_desktop_widget_layout(SPDesktopWidget);

    return;
}

void
SPDesktop::getWindowGeometry (gint &x, gint &y, gint &w, gint &h)
{
    _widget->getGeometry (x, y, w, h);
}

void
SPDesktop::setWindowPosition (Geom::Point p)
{
    _widget->setPosition (p);
}

void
SPDesktop::setWindowSize (gint w, gint h)
{
    _widget->setSize (w, h);
}

void
SPDesktop::setWindowTransient (void *p, int transient_policy)
{
    _widget->setTransient (p, transient_policy);
}

Gtk::Window*
SPDesktop::getToplevel( )
{
    return _widget->getWindow();
}

void
SPDesktop::presentWindow()
{
    _widget->present();
}

bool SPDesktop::showInfoDialog( Glib::ustring const & message )
{
    return _widget->showInfoDialog( message );
}

bool
SPDesktop::warnDialog (Glib::ustring const &text)
{
    return _widget->warnDialog (text);
}

void
SPDesktop::toggleRulers()
{
    _widget->toggleRulers();
}

void
SPDesktop::toggleScrollbars()
{
    _widget->toggleScrollbars();
}


void SPDesktop::toggleToolbar(gchar const *toolbar_name)
{
    Glib::ustring pref_path = getLayoutPrefPath(this) + toolbar_name + "/state";
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean visible = prefs->getBool(pref_path, true);
    prefs->setBool(pref_path, !visible);

    layoutWidget();
}

void
SPDesktop::layoutWidget()
{
    _widget->layout();
}

void
SPDesktop::destroyWidget()
{
    _widget->destroy();
}

bool
SPDesktop::shutdown()
{
    return _widget->shutdown();
}

bool SPDesktop::onDeleteUI (GdkEventAny*)
{
    if(shutdown())
        return true;

    destroyWidget();
    return false;
}

/**
 *  onWindowStateEvent
 *
 *  Called when the window changes its maximize/fullscreen/iconify/pinned state.
 *  Since GTK doesn't have a way to query this state information directly, we
 *  record it for the desktop here, and also possibly trigger a layout.
 */
bool
SPDesktop::onWindowStateEvent (GdkEventWindowState* event)
{
    // Record the desktop window's state
    window_state = event->new_window_state;

    // Layout may differ depending on full-screen mode or not
    GdkWindowState changed = event->changed_mask;
    if (changed & (GDK_WINDOW_STATE_FULLSCREEN|GDK_WINDOW_STATE_MAXIMIZED)) {
        layoutWidget();
    }

    return false;
}

void
SPDesktop::setToolboxFocusTo (gchar const *label)
{
    _widget->setToolboxFocusTo (label);
}

void
SPDesktop::setToolboxAdjustmentValue (gchar const* id, double val)
{
    _widget->setToolboxAdjustmentValue (id, val);
}

void
SPDesktop::setToolboxSelectOneValue (gchar const* id, gint val)
{
    _widget->setToolboxSelectOneValue (id, val);
}

bool
SPDesktop::isToolboxButtonActive (gchar const *id)
{
    return _widget->isToolboxButtonActive (id);
}

void
SPDesktop::emitToolSubselectionChanged(gpointer data)
{
    _tool_subselection_changed.emit(data);
    inkscape_subselection_changed (this);
}

void SPDesktop::updateNow()
{
    canvas->updateNow();
}

void
SPDesktop::enableInteraction()
{
  _widget->enableInteraction();
}

void SPDesktop::disableInteraction()
{
  _widget->disableInteraction();
}

void SPDesktop::setWaitingCursor()
{
    GdkCursor *waiting = gdk_cursor_new(GDK_WATCH);
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(sp_desktop_canvas(this))), waiting);
#if GTK_CHECK_VERSION(3,0,0)
    g_object_unref(waiting);
#else
    gdk_cursor_unref(waiting);
#endif
    // GDK needs the flush for the cursor change to take effect
    gdk_flush();
    waiting_cursor = true;
}

void SPDesktop::clearWaitingCursor() {
  if (waiting_cursor) {
      this->event_context->sp_event_context_update_cursor();
  }
}

void SPDesktop::toggleColorProfAdjust()
{
    _widget->toggleColorProfAdjust();
}

bool SPDesktop::colorProfAdjustEnabled()
{
    return _widget->colorProfAdjustEnabled();
}

void SPDesktop::toggleGrids()
{
    if (namedview->grids) {
        if(gridgroup) {
            showGrids(!grids_visible);
        }
    } else {
        //there is no grid present at the moment. add a rectangular grid and make it visible
        namedview->writeNewGrid(sp_desktop_document(this), Inkscape::GRID_RECTANGULAR);
        showGrids(true);
    }
}

void SPDesktop::showGrids(bool show, bool dirty_document)
{
    grids_visible = show;
    sp_namedview_show_grids(namedview, grids_visible, dirty_document);
    if (show) {
        sp_canvas_item_show(SP_CANVAS_ITEM(gridgroup));
    } else {
        sp_canvas_item_hide(SP_CANVAS_ITEM(gridgroup));
    }
}

void SPDesktop::toggleSnapGlobal()
{
    bool v = namedview->getSnapGlobal();
    namedview->setSnapGlobal(!v);
}

//----------------------------------------------------------------------
// Callback implementations. The virtual ones are connected by the view.

void
SPDesktop::onResized (double /*x*/, double /*y*/)
{
   // Nothing called here
}

/**
 * Redraw callback; queues Gtk redraw; connected by View.
 */
void
SPDesktop::onRedrawRequested ()
{
    if (main) {
        _widget->requestCanvasUpdate();
    }
}

void
SPDesktop::updateCanvasNow()
{
  _widget->requestCanvasUpdateAndWait();
}

/**
 * Associate document with desktop.
 */
void
SPDesktop::setDocument (SPDocument *doc)
{
    if (!doc) return;

    if (this->doc()) {
        namedview->hide(this);
        this->doc()->getRoot()->invoke_hide(dkey);
    }

    layers->setDocument(doc);

	// remove old EventLog if it exists (see also: bug #1071082)
	if (event_log) {
		doc->removeUndoObserver(*event_log);
		delete event_log;
		event_log = 0;
	}

    /* setup EventLog */
    event_log = new Inkscape::EventLog(doc);
    doc->addUndoObserver(*event_log);

    _commit_connection.disconnect();
    _commit_connection = doc->connectCommit(sigc::mem_fun(*this, &SPDesktop::updateNow));

    /// \todo fixme: This condition exists to make sure the code
    /// inside is NOT called on initialization, only on replacement. But there
    /// are surely more safe methods to accomplish this.
    // TODO since the comment had reversed logic, check the intent of this block of code:
    if (drawing) {
        Inkscape::DrawingItem *ai = 0;

        namedview = sp_document_namedview (doc, NULL);
        _modified_connection = namedview->connectModified(sigc::bind<2>(sigc::ptr_fun(&_namedview_modified), this));
        number = namedview->getViewCount();

        ai = doc->getRoot()->invoke_show(
                SP_CANVAS_ARENA (drawing)->drawing,
                dkey,
                SP_ITEM_SHOW_DISPLAY);
        if (ai) {
            SP_CANVAS_ARENA (drawing)->drawing.root()->prependChild(ai);
        }
        namedview->show(this);
        /* Ugly hack */
        activate_guides (true);
        /* Ugly hack */
        _namedview_modified (namedview, SP_OBJECT_MODIFIED_FLAG, this);
    }

    _document_replaced_signal.emit (this, doc);

    View::setDocument (doc);
}

void
SPDesktop::onStatusMessage
(Inkscape::MessageType type, gchar const *message)
{
    if (_widget) {
        _widget->setMessage(type, message);
    }
}

void
SPDesktop::onDocumentURISet (gchar const* uri)
{
    _widget->setTitle(uri);
}

/**
 * Resized callback.
 */
void
SPDesktop::onDocumentResized (gdouble width, gdouble height)
{
    _doc2dt[5] = height;
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (drawing), _doc2dt);
    Geom::Rect const a(Geom::Point(0, 0), Geom::Point(width, height));
    SP_CTRLRECT(page)->setRectangle(a);
    SP_CTRLRECT(page_border)->setRectangle(a);
}


void
SPDesktop::_onActivate (SPDesktop* dt)
{
    if (!dt->_widget) return;
    dt->_widget->activateDesktop();
}

void
SPDesktop::_onDeactivate (SPDesktop* dt)
{
    if (!dt->_widget) return;
    dt->_widget->deactivateDesktop();
}

void
SPDesktop::_onSelectionModified
(Inkscape::Selection */*selection*/, guint /*flags*/, SPDesktop *dt)
{
    if (!dt->_widget) return;
    dt->_widget->updateScrollbars (dt->_d2w.descrim());
}

static void
_onSelectionChanged
(Inkscape::Selection *selection, SPDesktop *desktop)
{
    /** \todo
     * only change the layer for single selections, or what?
     * This seems reasonable -- for multiple selections there can be many
     * different layers involved.
     */
    SPItem *item=selection->singleItem();
    if (item) {
        SPObject *layer=desktop->layers->layerForObject(item);
        if ( layer && layer != desktop->currentLayer() ) {
            desktop->layers->setCurrentLayer(layer);
        }
    }
}

/**
 * Calls event handler of current event context.
 * \param arena Unused
 * \todo fixme
 */
static gint
_arena_handler (SPCanvasArena */*arena*/, Inkscape::DrawingItem *ai, GdkEvent *event, SPDesktop *desktop)
{
    if (ai) {
        SPItem *spi = (SPItem*) ai->data();
        return sp_event_context_item_handler (desktop->event_context, spi, event);
    } else {
        return sp_event_context_root_handler (desktop->event_context, event);
    }
}

static void
_layer_activated(SPObject *layer, SPDesktop *desktop) {
    g_return_if_fail(SP_IS_GROUP(layer));
    SP_GROUP(layer)->setLayerDisplayMode(desktop->dkey, SPGroup::LAYER);
}

/// Callback
static void
_layer_deactivated(SPObject *layer, SPDesktop *desktop) {
    g_return_if_fail(SP_IS_GROUP(layer));
    SP_GROUP(layer)->setLayerDisplayMode(desktop->dkey, SPGroup::GROUP);
}

/// Callback
static void
_layer_hierarchy_changed(SPObject */*top*/, SPObject *bottom,
                                         SPDesktop *desktop)
{
    desktop->_layer_changed_signal.emit (bottom);
}

/// Called when document is starting to be rebuilt.
static void _reconstruction_start(SPDesktop * desktop)
{
    desktop->_reconstruction_old_layer_id = desktop->currentLayer()->getId() ? desktop->currentLayer()->getId() : "";
    desktop->layers->reset();

    /*
    GSList const * selection_objs = desktop->selection->list();
    for (; selection_objs != NULL; selection_objs = selection_objs->next) {

    }
    */
    desktop->selection->clear();
}

/// Called when document rebuild is finished.
static void _reconstruction_finish(SPDesktop * desktop)
{
    g_debug("Desktop, finishing reconstruction\n");
    if ( !desktop->_reconstruction_old_layer_id.empty() ) {
        SPObject * newLayer = desktop->namedview->document->getObjectById(desktop->_reconstruction_old_layer_id);
        if (newLayer != NULL) {
            desktop->layers->setCurrentLayer(newLayer);
        }

        desktop->_reconstruction_old_layer_id.clear();
    }
    g_debug("Desktop, finishing reconstruction end\n");
}

/**
 * Namedview_modified callback.
 */
static void _namedview_modified (SPObject *obj, guint flags, SPDesktop *desktop)
{
    SPNamedView *nv=SP_NAMEDVIEW(obj);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {

        /* Show/hide page background */
        if (nv->pagecolor | (0xff != 0xffffffff)) {
            sp_canvas_item_show (desktop->table);
            ((CtrlRect *) desktop->table)->setColor(0x00000000, true, nv->pagecolor | 0xff);
            sp_canvas_item_move_to_z (desktop->table, 0);
        } else {
            sp_canvas_item_hide (desktop->table);
        }

        /* Show/hide page border */
        if (nv->showborder) {
            // show
            sp_canvas_item_show (desktop->page_border);
            // set color and shadow
            ((CtrlRect *) desktop->page_border)->setColor(nv->bordercolor, false, 0x00000000);
            if (nv->pageshadow) {
                ((CtrlRect *) desktop->page_border)->setShadow(nv->pageshadow, nv->bordercolor);
            }
            // place in the z-order stack
            if (nv->borderlayer == SP_BORDER_LAYER_BOTTOM) {
                 sp_canvas_item_move_to_z (desktop->page_border, 2);
            } else {
                int order = sp_canvas_item_order (desktop->page_border);
                int morder = sp_canvas_item_order (desktop->drawing);
                if (morder > order) sp_canvas_item_raise (desktop->page_border,
                    morder - order);
            }
        } else {
                sp_canvas_item_hide (desktop->page_border);
                if (nv->pageshadow) {
                    ((CtrlRect *) desktop->page)->setShadow(0, 0x00000000);
                }
        }

        /* Show/hide page shadow */
        if (nv->showpageshadow && nv->pageshadow) {
            ((CtrlRect *) desktop->page_border)->setShadow(nv->pageshadow, nv->bordercolor);
        } else {
            ((CtrlRect *) desktop->page_border)->setShadow(0, 0x00000000);
        }

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (SP_RGBA32_R_U(nv->pagecolor) +
            SP_RGBA32_G_U(nv->pagecolor) +
            SP_RGBA32_B_U(nv->pagecolor) >= 384) {
            // the background color is light, use black outline
            SP_CANVAS_ARENA (desktop->drawing)->drawing.outlinecolor = prefs->getInt("/options/wireframecolors/onlight", 0xff);
        } else { // use white outline
            SP_CANVAS_ARENA (desktop->drawing)->drawing.outlinecolor = prefs->getInt("/options/wireframecolors/ondark", 0xffffffff);
        }
    }
}

Geom::Affine SPDesktop::w2d() const
{
    return _w2d;
}

Geom::Point SPDesktop::w2d(Geom::Point const &p) const
{
    return p * _w2d;
}

Geom::Point SPDesktop::d2w(Geom::Point const &p) const
{
    return p * _d2w;
}

Geom::Affine SPDesktop::doc2dt() const
{
    return _doc2dt;
}

Geom::Affine SPDesktop::dt2doc() const
{
    return _doc2dt.inverse();
}

Geom::Point SPDesktop::doc2dt(Geom::Point const &p) const
{
    return p * _doc2dt;
}

Geom::Point SPDesktop::dt2doc(Geom::Point const &p) const
{
    return p * dt2doc();
}

void
SPDesktop::show_dialogs()
{

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs == NULL) {
        return;
    }

    int active = prefs->getInt("/options/savedialogposition/value", 1);
    if (active == 0) {
        // User has turned off this feature in preferences
        return;
    }

    if (showing_dialogs) {
        return;
    }

    showing_dialogs = TRUE;


    /*
     * Get each dialogs previous state from preferences and reopen on startup if needed, without grabbing focus (canvas retains focus).
     * Map dialog manager's dialog IDs to dialog last visible state preference. FIXME: store this correspondence in dialogs themselves!
     */
    std::map<Glib::ustring, Glib::ustring> mapVerbPreference;
    mapVerbPreference.insert(std::make_pair ("LayersPanel", "/dialogs/layers") );
    mapVerbPreference.insert(std::make_pair ("FillAndStroke", "/dialogs/fillstroke") );
    mapVerbPreference.insert(std::make_pair ("ExtensionEditor", "/dialogs/extensioneditor") );
    mapVerbPreference.insert(std::make_pair ("AlignAndDistribute", "/dialogs/align") );
    mapVerbPreference.insert(std::make_pair ("DocumentMetadata", "/dialogs/documentmetadata") );
    mapVerbPreference.insert(std::make_pair ("DocumentProperties", "/dialogs/documentoptions") );
    mapVerbPreference.insert(std::make_pair ("FilterEffectsDialog", "/dialogs/filtereffects") );
    mapVerbPreference.insert(std::make_pair ("Find", "/dialogs/find") );
    mapVerbPreference.insert(std::make_pair ("Glyphs", "/dialogs/glyphs") );
    mapVerbPreference.insert(std::make_pair ("Messages", "/dialogs/messages") );
    mapVerbPreference.insert(std::make_pair ("Memory", "/dialogs/memory") );
    mapVerbPreference.insert(std::make_pair ("LivePathEffect", "/dialogs/livepatheffect") );
    mapVerbPreference.insert(std::make_pair ("UndoHistory", "/dialogs/undo-history") );
    mapVerbPreference.insert(std::make_pair ("Transformation", "/dialogs/transformation") );
    mapVerbPreference.insert(std::make_pair ("Swatches", "/dialogs/swatches") );
    mapVerbPreference.insert(std::make_pair ("IconPreviewPanel", "/dialogs/iconpreview") );
    mapVerbPreference.insert(std::make_pair ("SvgFontsDialog", "/dialogs/svgfonts") );
    mapVerbPreference.insert(std::make_pair ("InputDevices", "/dialogs/inputdevices") );
    mapVerbPreference.insert(std::make_pair ("InkscapePreferences", "/dialogs/preferences") );
    mapVerbPreference.insert(std::make_pair ("TileDialog", "/dialogs/gridtiler") );
    mapVerbPreference.insert(std::make_pair ("Trace", "/dialogs/trace") );
    mapVerbPreference.insert(std::make_pair ("PixelArt", "/dialogs/pixelart") );
    mapVerbPreference.insert(std::make_pair ("TextFont", "/dialogs/textandfont") );
    mapVerbPreference.insert(std::make_pair ("Export", "/dialogs/export") );
    mapVerbPreference.insert(std::make_pair ("XmlTree", "/dialogs/xml") );
    mapVerbPreference.insert(std::make_pair ("CloneTiler", "/dialogs/clonetiler") );
    mapVerbPreference.insert(std::make_pair ("ObjectProperties", "/dialogs/object") );
    mapVerbPreference.insert(std::make_pair ("SpellCheck", "/dialogs/spellcheck") );
    mapVerbPreference.insert(std::make_pair ("Symbols", "/dialogs/symbols") );

    for (std::map<Glib::ustring, Glib::ustring>::const_iterator iter = mapVerbPreference.begin(); iter != mapVerbPreference.end(); ++iter) {
        Glib::ustring pref = iter->second;
        int visible = prefs->getInt(pref + "/visible", 0);
        if (visible) {
            _dlg_mgr->showDialog(iter->first.c_str(), false); // without grabbing focus, we need focus to remain on the canvas
        }
    }
}
/*
 * Pop event context from desktop's context stack. Never used.
 */
// void
// SPDesktop::pop_event_context (unsigned int key)
// {
//    ToolBase *ec = NULL;
//
//    if (event_context && event_context->key == key) {
//        g_return_if_fail (event_context);
//        g_return_if_fail (event_context->next);
//        ec = event_context;
//        sp_event_context_deactivate (ec);
//        event_context = ec->next;
//        sp_event_context_activate (event_context);
//        _event_context_changed_signal.emit (this, ec);
//    }
//
//    ToolBase *ref = event_context;
//    while (ref && ref->next && ref->next->key != key)
//        ref = ref->next;
//
//    if (ref && ref->next) {
//        ec = ref->next;
//        ref->next = ec->next;
//    }
//
//    if (ec) {
//        sp_event_context_finish (ec);
//        g_object_unref (G_OBJECT (ec));
//    }
// }

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


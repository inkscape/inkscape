#define __SP_DESKTOP_C__

/** \file
 * Editable view implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
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

/** \class SPDesktop
 * SPDesktop is a subclass of View, implementing an editable document
 * canvas.  It is extensively used by many UI controls that need certain
 * visual representations of their own.
 *
 * SPDesktop provides a certain set of SPCanvasItems, serving as GUI
 * layers of different control objects. The one containing the whole
 * document is the drawing layer. In addition to it, there are grid,
 * guide, sketch and control layers. The sketch layer is used for
 * temporary drawing objects, before the real objects in document are
 * created. The control layer contains editing knots, rubberband and
 * similar non-document UI objects.
 *
 * Each SPDesktop is associated with a SPNamedView node of the document
 * tree.  Currently, all desktops are created from a single main named
 * view, but in the future there may be support for different ones.
 * SPNamedView serves as an in-document container for desktop-related
 * data, like grid and guideline placement, snapping options and so on.
 *
 * Associated with each SPDesktop are the two most important editing
 * related objects - SPSelection and SPEventContext.
 *
 * Sodipodi keeps track of the active desktop and invokes notification
 * signals whenever it changes. UI elements can use these to update their
 * display to the selection of the currently active editing window.
 * (Lauris Kaplinski)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "macros.h"
#include "inkscape-private.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "document.h"
#include "message-stack.h"
#include "selection.h"
#include "select-context.h"
#include "sp-namedview.h"
#include "color.h"
#include "sp-item-group.h"
#include "prefs-utils.h"
#include "object-hierarchy.h"
#include "helper/units.h"
#include "display/canvas-arena.h"
#include "display/nr-arena.h"
#include "display/gnome-canvas-acetate.h"
#include "display/sodipodi-ctrlrect.h"
#include "display/sp-canvas-util.h"
#include "display/canvas-temporary-item-list.h"
#include "display/snap-indicator.h"
#include "libnr/nr-matrix-div.h"
#include "libnr/nr-rect-ops.h"
#include "ui/dialog/dialog-manager.h"
#include "xml/repr.h"
#include "message-context.h"
#include "device-manager.h"
#include "layer-manager.h"
#include "event-log.h"
#include "display/canvas-grid.h"
#include "widgets/desktop-widget.h"
#include "box3d-context.h"

#include "display/sp-canvas.h"

namespace Inkscape { namespace XML { class Node; }}

// Callback declarations
static void _onSelectionChanged (Inkscape::Selection *selection, SPDesktop *desktop);
static gint _arena_handler (SPCanvasArena *arena, NRArenaItem *ai, GdkEvent *event, SPDesktop *desktop);
static void _layer_activated(SPObject *layer, SPDesktop *desktop);
static void _layer_deactivated(SPObject *layer, SPDesktop *desktop);
static void _layer_hierarchy_changed(SPObject *top, SPObject *bottom, SPDesktop *desktop);
static void _reconstruction_start(SPDesktop * desktop);
static void _reconstruction_finish(SPDesktop * desktop);
static void _namedview_modified (SPObject *obj, guint flags, SPDesktop *desktop);
static void _update_snap_distances (SPDesktop *desktop);

/**
 * Return new desktop object.
 * \pre namedview != NULL.
 * \pre canvas != NULL.
 */
SPDesktop::SPDesktop() :
    _dlg_mgr( 0 ),
    namedview( 0 ),
    canvas( 0 ),
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
    zooms_past( 0 ),
    zooms_future( 0 ),
    dkey( 0 ),
    number( 0 ),
    window_state(0),
    interaction_disabled_counter( 0 ),
    waiting_cursor( false ),
    guides_active( false ),
    gr_item( 0 ),
    gr_point_type( 0 ),
    gr_point_i( 0 ),
    gr_fill_or_stroke( true ),
    displayMode(Inkscape::RENDERMODE_NORMAL),
    _layer_hierarchy( 0 ),
    _reconstruction_old_layer_id( 0 ),
    _widget( 0 ),
    _inkscape( 0 ),
    _guides_message_context( 0 ),
    _active( false ),
    _w2d(),
    _d2w(),
    _doc2dt( NR::Matrix(NR::scale(1, -1)) ),
    grids_visible( false )
{
    _d2w.set_identity();
    _w2d.set_identity();

    selection = Inkscape::GC::release( new Inkscape::Selection(this) );
}

void
SPDesktop::init (SPNamedView *nv, SPCanvas *aCanvas)
{
    // Temporary workaround for link order issues:
    Inkscape::DeviceManager::getManager().getDevices();

    _guides_message_context = new Inkscape::MessageContext(const_cast<Inkscape::MessageStack*>(messageStack()));

    current = sp_repr_css_attr_inherited (inkscape_get_repr (INKSCAPE, "desktop"), "style");

    namedview = nv;
    canvas = aCanvas;

    SPDocument *document = SP_OBJECT_DOCUMENT (namedview);
    /* Kill flicker */
    sp_document_ensure_up_to_date (document);

    /* Setup Dialog Manager */
    _dlg_mgr = &Inkscape::UI::Dialog::DialogManager::getInstance();

    dkey = sp_item_display_key_new (1);

    /* Connect document */
    setDocument (document);

    number = namedview->getViewCount();


    /* Setup Canvas */
    g_object_set_data (G_OBJECT (canvas), "SPDesktop", this);

    SPCanvasGroup *root = sp_canvas_root (canvas);

    /* Setup adminstrative layers */
    acetate = sp_canvas_item_new (root, GNOME_TYPE_CANVAS_ACETATE, NULL);
    g_signal_connect (G_OBJECT (acetate), "event", G_CALLBACK (sp_desktop_root_handler), this);
    main = (SPCanvasGroup *) sp_canvas_item_new (root, SP_TYPE_CANVAS_GROUP, NULL);
    g_signal_connect (G_OBJECT (main), "event", G_CALLBACK (sp_desktop_root_handler), this);

    table = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);
    SP_CTRLRECT(table)->setRectangle(NR::Rect(NR::Point(-80000, -80000), NR::Point(80000, 80000)));
    SP_CTRLRECT(table)->setColor(0x00000000, true, 0x00000000);
    sp_canvas_item_move_to_z (table, 0);

    page = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);
    ((CtrlRect *) page)->setColor(0x00000000, FALSE, 0x00000000);
    page_border = sp_canvas_item_new (main, SP_TYPE_CTRLRECT, NULL);

    drawing = sp_canvas_item_new (main, SP_TYPE_CANVAS_ARENA, NULL);
    g_signal_connect (G_OBJECT (drawing), "arena_event", G_CALLBACK (_arena_handler), this);

    SP_CANVAS_ARENA (drawing)->arena->delta = prefs_get_double_attribute ("options.cursortolerance", "value", 1.0); // default is 1 px

    if (prefs_get_int_attribute("options.startmode", "outline", 0)) {
        // Start in outline mode
        setDisplayModeOutline();
    } else {
        // Start in normal mode, default
        setDisplayModeNormal();
    }

    gridgroup = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    guides = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    sketch = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    controls = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);
    tempgroup = (SPCanvasGroup *) sp_canvas_item_new (main, SP_TYPE_CANVAS_GROUP, NULL);

    /* Push select tool to the bottom of stack */
    /** \todo
     * FIXME: this is the only call to this.  Everything else seems to just
     * call "set" instead of "push".  Can we assume that there is only one
     * context ever?
     */
    push_event_context (SP_TYPE_SELECT_CONTEXT, "tools.select", SP_EVENT_CONTEXT_STATIC);

    // display rect and zoom are now handled in sp_desktop_widget_realize()

    NR::Rect const d(NR::Point(0.0, 0.0),
                     NR::Point(sp_document_width(document), sp_document_height(document)));

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
    _doc2dt[5] = sp_document_height (document);
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (drawing), _doc2dt);

    _modified_connection = namedview->connectModified(sigc::bind<2>(sigc::ptr_fun(&_namedview_modified), this));

    NRArenaItem *ai = sp_item_invoke_show (SP_ITEM (sp_document_root (document)),
            SP_CANVAS_ARENA (drawing)->arena,
            dkey,
            SP_ITEM_SHOW_DISPLAY);
    if (ai) {
        nr_arena_item_add_child (SP_CANVAS_ARENA (drawing)->root, ai, NULL);
        nr_arena_item_unref (ai);
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
    _reconstruction_old_layer_id = NULL;

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
    if (snapindicator) {
        delete snapindicator;
        snapindicator = NULL;
    }
    if (temporary_item_list) {
        delete temporary_item_list;
        temporary_item_list = NULL;
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

    while (event_context) {
        SPEventContext *ec = event_context;
        event_context = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    if (_layer_hierarchy) {
        delete _layer_hierarchy;
//        _layer_hierarchy = NULL; //this should be here, but commented to find other bug somewhere else.
    }

    if (layer_manager) {
        delete layer_manager;
        layer_manager = NULL;
    }

    if (_inkscape) {
        _inkscape = NULL;
    }

    if (drawing) {
        sp_item_invoke_hide (SP_ITEM (sp_document_root (doc())), dkey);
        drawing = NULL;
    }

    delete _guides_message_context;
    _guides_message_context = NULL;

    g_list_free (zooms_past);
    g_list_free (zooms_future);
}

SPDesktop::~SPDesktop() {}

//--------------------------------------------------------------------
/* Public methods */


/** Note that lifetime is measured in milliseconds
* it is perfectly safe to ignore the returned pointer: the object is deleted by itself, so don't delete it elsewhere!
* The return value should only be used as argument for SPDesktop::remove_temporary_canvasitem, because the object might be deleted already.
*/
Inkscape::Display::TemporaryItem *
SPDesktop::add_temporary_canvasitem (SPCanvasItem *item, guint lifetime)
{
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

void SPDesktop::setDisplayModeNormal()
{
    SP_CANVAS_ARENA (drawing)->arena->rendermode = Inkscape::RENDERMODE_NORMAL;
    canvas->rendermode = Inkscape::RENDERMODE_NORMAL; // canvas needs that for choosing the best buffer size
    displayMode = Inkscape::RENDERMODE_NORMAL;
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (main), _d2w); // redraw
    _widget->setTitle(SP_DOCUMENT_NAME(sp_desktop_document(this)));
}

void SPDesktop::setDisplayModeOutline()
{
    SP_CANVAS_ARENA (drawing)->arena->rendermode = Inkscape::RENDERMODE_OUTLINE;
    canvas->rendermode = Inkscape::RENDERMODE_OUTLINE; // canvas needs that for choosing the best buffer size
    displayMode = Inkscape::RENDERMODE_OUTLINE;
    sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (main), _d2w); // redraw
    _widget->setTitle(SP_DOCUMENT_NAME(sp_desktop_document(this)));
}

void SPDesktop::displayModeToggle()
{
    if (displayMode == Inkscape::RENDERMODE_OUTLINE)
        setDisplayModeNormal();
    else
        setDisplayModeOutline();
}

/**
 * Returns current root (=bottom) layer.
 */
SPObject *SPDesktop::currentRoot() const
{
    return _layer_hierarchy ? _layer_hierarchy->top() : NULL;
}

/**
 * Returns current top layer.
 */
SPObject *SPDesktop::currentLayer() const
{
    return _layer_hierarchy ? _layer_hierarchy->bottom() : NULL;
}

/**
 * Sets the current layer of the desktop.
 *
 * Make \a object the top layer.
 */
void SPDesktop::setCurrentLayer(SPObject *object) {
    g_return_if_fail(SP_IS_GROUP(object));
    g_return_if_fail( currentRoot() == object || (currentRoot() && currentRoot()->isAncestorOf(object)) );
    // printf("Set Layer to ID: %s\n", SP_OBJECT_ID(object));
    _layer_hierarchy->setBottom(object);
}

/**
 * Return layer that contains \a object.
 */
SPObject *SPDesktop::layerForObject(SPObject *object) {
    g_return_val_if_fail(object != NULL, NULL);

    SPObject *root=currentRoot();
    object = SP_OBJECT_PARENT(object);
    while ( object && object != root && !isLayer(object) ) {
        object = SP_OBJECT_PARENT(object);
    }
    return object;
}

/**
 * True if object is a layer.
 */
bool SPDesktop::isLayer(SPObject *object) const {
    return ( SP_IS_GROUP(object)
             && ( SP_GROUP(object)->effectiveLayerMode(this->dkey)
                  == SPGroup::LAYER ) );
}

/**
 * True if desktop viewport fully contains \a item's bbox.
 */
bool SPDesktop::isWithinViewport (SPItem *item) const
{
    NR::Rect const viewport = get_display_area();
    NR::Maybe<NR::Rect> const bbox = sp_item_bbox_desktop(item);
    if (bbox) {
        return viewport.contains(*bbox);
    } else {
        return true;
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
    if (dtw) dtw->desktop = this;
    sp_desktop_widget_update_namedview(dtw);

    _namedview_modified (namedview, SP_OBJECT_MODIFIED_FLAG, this);
    _document_replaced_signal.emit (this, theDocument);
}

/**
 * Make desktop switch event contexts.
 */
void
SPDesktop::set_event_context (GtkType type, const gchar *config)
{
    SPEventContext *ec;
    while (event_context) {
        ec = event_context;
        sp_event_context_deactivate (ec);
        event_context = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    Inkscape::XML::Node *repr = (config) ? inkscape_get_repr (_inkscape, config) : NULL;
    ec = sp_event_context_new (type, this, repr, SP_EVENT_CONTEXT_STATIC);
    ec->next = event_context;
    event_context = ec;
    sp_event_context_activate (ec);
    _event_context_changed_signal.emit (this, ec);
}

/**
 * Push event context onto desktop's context stack.
 */
void
SPDesktop::push_event_context (GtkType type, const gchar *config, unsigned int key)
{
    SPEventContext *ref, *ec;
    Inkscape::XML::Node *repr;

    if (event_context && event_context->key == key) return;
    ref = event_context;
    while (ref && ref->next && ref->next->key != key) ref = ref->next;
    if (ref && ref->next) {
        ec = ref->next;
        ref->next = ec->next;
        sp_event_context_finish (ec);
        g_object_unref (G_OBJECT (ec));
    }

    if (event_context) sp_event_context_deactivate (event_context);
    repr = (config) ? inkscape_get_repr (INKSCAPE, config) : NULL;
    ec = sp_event_context_new (type, this, repr, key);
    ec->next = event_context;
    event_context = ec;
    sp_event_context_activate (ec);
    _event_context_changed_signal.emit (this, ec);
}

/**
 * Sets the coordinate status to a given point
 */
void
SPDesktop::set_coordinate_status (NR::Point p) {
    _widget->setCoordinateStatus(p);
}

/**
 * \see sp_document_item_from_list_at_point_bottom()
 */
SPItem *
SPDesktop::item_from_list_at_point_bottom (const GSList *list, NR::Point const p) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return sp_document_item_from_list_at_point_bottom (dkey, SP_GROUP (doc()->root), list, p);
}

/**
 * \see sp_document_item_at_point()
 */
SPItem *
SPDesktop::item_at_point (NR::Point const p, bool into_groups, SPItem *upto) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return sp_document_item_at_point (doc(), dkey, p, into_groups, upto);
}

/**
 * \see sp_document_group_at_point()
 */
SPItem *
SPDesktop::group_at_point (NR::Point const p) const
{
    g_return_val_if_fail (doc() != NULL, NULL);
    return sp_document_group_at_point (doc(), dkey, p);
}

/**
 * \brief  Returns the mouse point in document coordinates; if mouse is
 * outside the canvas, returns the center of canvas viewpoint
 */
NR::Point
SPDesktop::point() const
{
    NR::Point p = _widget->getPointer();
    NR::Point pw = sp_canvas_window_to_world (canvas, p);
    p = w2d(pw);

    NR::Rect const r = canvas->getViewbox();

    NR::Point r0 = w2d(r.min());
    NR::Point r1 = w2d(r.max());

    if (p[NR::X] >= r0[NR::X] &&
        p[NR::X] <= r1[NR::X] &&
        p[NR::Y] >= r1[NR::Y] &&
        p[NR::Y] <= r0[NR::Y])
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
SPDesktop::push_current_zoom (GList **history)
{
    NR::Rect const area = get_display_area();

    NRRect *old_zoom = g_new(NRRect, 1);
    old_zoom->x0 = area.min()[NR::X];
    old_zoom->x1 = area.max()[NR::X];
    old_zoom->y0 = area.min()[NR::Y];
    old_zoom->y1 = area.max()[NR::Y];
    if ( *history == NULL
         || !( ( ((NRRect *) ((*history)->data))->x0 == old_zoom->x0 ) &&
               ( ((NRRect *) ((*history)->data))->x1 == old_zoom->x1 ) &&
               ( ((NRRect *) ((*history)->data))->y0 == old_zoom->y0 ) &&
               ( ((NRRect *) ((*history)->data))->y1 == old_zoom->y1 ) ) )
    {
        *history = g_list_prepend (*history, old_zoom);
    }
}

/**
 * Set viewbox (x0, x1, y0 and y1 are in document pixels. Border is in screen pixels).
 */
void
SPDesktop::set_display_area (double x0, double y0, double x1, double y1, double border, bool log)
{
    g_assert(_widget);

    // save the zoom
    if (log) {
        push_current_zoom(&zooms_past);
        // if we do a logged zoom, our zoom-forward list is invalidated, so delete it
        g_list_free (zooms_future);
        zooms_future = NULL;
    }

    double const cx = 0.5 * (x0 + x1);
    double const cy = 0.5 * (y0 + y1);

    NR::Rect const viewbox = NR::expand(canvas->getViewbox(), border);

    double scale = expansion(_d2w);
    double newscale;
    if (((x1 - x0) * viewbox.dimensions()[NR::Y]) > ((y1 - y0) * viewbox.dimensions()[NR::X])) {
        newscale = viewbox.dimensions()[NR::X] / (x1 - x0);
    } else {
        newscale = viewbox.dimensions()[NR::Y] / (y1 - y0);
    }

    newscale = CLAMP(newscale, SP_DESKTOP_ZOOM_MIN, SP_DESKTOP_ZOOM_MAX); // unit: 'screen pixels' per 'document pixels'

    int clear = FALSE;
    if (!NR_DF_TEST_CLOSE (newscale, scale, 1e-4 * scale)) {
        /* Set zoom factors */
        _d2w = NR::Matrix(NR::scale(newscale, -newscale));
        _w2d = NR::Matrix(NR::scale(1/newscale, 1/-newscale));
        sp_canvas_item_affine_absolute(SP_CANVAS_ITEM(main), _d2w);
        clear = TRUE;
    }

    /* Calculate top left corner (in document pixels) */
    x0 = cx - 0.5 * viewbox.dimensions()[NR::X] / newscale;
    y1 = cy + 0.5 * viewbox.dimensions()[NR::Y] / newscale;

    /* Scroll */
    sp_canvas_scroll_to (canvas, x0 * newscale - border, y1 * -newscale - border, clear);

    /*  update perspective lines if we are in the 3D box tool (so that infinite ones are shown correctly) */
    sp_box3d_context_update_lines(event_context);

    _widget->updateRulers();
    _widget->updateScrollbars(expansion(_d2w));
    _widget->updateZoom();
}

void SPDesktop::set_display_area(NR::Rect const &a, NR::Coord b, bool log)
{
    set_display_area(a.min()[NR::X], a.min()[NR::Y], a.max()[NR::X], a.max()[NR::Y], b, log);
}

/**
 * Return viewbox dimensions.
 */
NR::Rect SPDesktop::get_display_area() const
{
    NR::Rect const viewbox = canvas->getViewbox();

    double const scale = _d2w[0];

    return NR::Rect(NR::Point(viewbox.min()[NR::X] / scale, viewbox.max()[NR::Y] / -scale),
                    NR::Point(viewbox.max()[NR::X] / scale, viewbox.min()[NR::Y] / -scale));
}

/**
 * Revert back to previous zoom if possible.
 */
void
SPDesktop::prev_zoom()
{
    if (zooms_past == NULL) {
        messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No previous zoom."));
        return;
    }

    // push current zoom into forward zooms list
    push_current_zoom (&zooms_future);

    // restore previous zoom
    set_display_area (((NRRect *) zooms_past->data)->x0,
            ((NRRect *) zooms_past->data)->y0,
            ((NRRect *) zooms_past->data)->x1,
            ((NRRect *) zooms_past->data)->y1,
            0, false);

    // remove the just-added zoom from the past zooms list
    zooms_past = g_list_remove (zooms_past, ((NRRect *) zooms_past->data));
}

/**
 * Set zoom to next in list.
 */
void
SPDesktop::next_zoom()
{
    if (zooms_future == NULL) {
        this->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No next zoom."));
        return;
    }

    // push current zoom into past zooms list
    push_current_zoom (&zooms_past);

    // restore next zoom
    set_display_area (((NRRect *) zooms_future->data)->x0,
            ((NRRect *) zooms_future->data)->y0,
            ((NRRect *) zooms_future->data)->x1,
            ((NRRect *) zooms_future->data)->y1,
            0, false);

    // remove the just-used zoom from the zooms_future list
    zooms_future = g_list_remove (zooms_future, ((NRRect *) zooms_future->data));
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
    if (fabs(expansion(_d2w) - zoom) < 0.0001*zoom && (fabs(SP_DESKTOP_ZOOM_MAX - zoom) < 0.01 || fabs(SP_DESKTOP_ZOOM_MIN - zoom) < 0.000001))
        return;

    NR::Rect const viewbox = canvas->getViewbox();

    double const width2 = viewbox.dimensions()[NR::X] / zoom;
    double const height2 = viewbox.dimensions()[NR::Y] / zoom;

    set_display_area(cx - px * width2,
                     cy - py * height2,
                     cx + (1 - px) * width2,
                     cy + (1 - py) * height2,
                     0.0);
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
    NR::Rect const area = get_display_area();

    if (cx < area.min()[NR::X]) {
        cx = area.min()[NR::X];
    }
    if (cx > area.max()[NR::X]) {
        cx = area.max()[NR::X];
    }
    if (cy < area.min()[NR::Y]) {
        cy = area.min()[NR::Y];
    }
    if (cy > area.max()[NR::Y]) {
        cy = area.max()[NR::Y];
    }

    gdouble const scale = expansion(_d2w) * zoom;
    double const px = (cx - area.min()[NR::X]) / area.dimensions()[NR::X];
    double const py = (cy - area.min()[NR::Y]) / area.dimensions()[NR::Y];

    zoom_absolute_keep_point(cx, cy, px, py, scale);
}

/**
 * Zoom to center with relative zoom factor.
 */
void
SPDesktop::zoom_relative (double cx, double cy, double zoom)
{
    gdouble scale = expansion(_d2w) * zoom;
    zoom_absolute (cx, cy, scale);
}

/**
 * Set display area to origin and current document dimensions.
 */
void
SPDesktop::zoom_page()
{
    NR::Rect d(NR::Point(0, 0),
               NR::Point(sp_document_width(doc()), sp_document_height(doc())));

    if (d.isEmpty(1.0)) {
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
    NR::Rect const a = get_display_area();

    if (sp_document_width(doc()) < 1.0) {
        return;
    }

    NR::Rect d(NR::Point(0, a.midpoint()[NR::Y]),
               NR::Point(sp_document_width(doc()), a.midpoint()[NR::Y]));

    set_display_area(d, 10);
}

/**
 * Zoom to selection.
 */
void
SPDesktop::zoom_selection()
{
    NR::Maybe<NR::Rect> const d = selection->bounds();

    if ( !d || d->isEmpty(0.1) ) {
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
    SPItem *docitem = SP_ITEM (sp_document_root (doc()));
    g_return_if_fail (docitem != NULL);

    NR::Maybe<NR::Rect> d = sp_item_bbox_desktop(docitem);

    /* Note that the second condition here indicates that
    ** there are no items in the drawing.
    */
    if ( !d || d->isEmpty(1.0) ) {
        return;
    }

    set_display_area(*d, 10);
}

/**
 * Scroll canvas by specific coordinate amount.
 */
void
SPDesktop::scroll_world (double dx, double dy, bool is_scrolling)
{
    g_assert(_widget);

    NR::Rect const viewbox = canvas->getViewbox();

    sp_canvas_scroll_to(canvas, viewbox.min()[NR::X] - dx, viewbox.min()[NR::Y] - dy, FALSE, is_scrolling);

    /*  update perspective lines if we are in the 3D box tool (so that infinite ones are shown correctly) */
    sp_box3d_context_update_lines(event_context);

    _widget->updateRulers();
    _widget->updateScrollbars(expansion(_d2w));
}

bool
SPDesktop::scroll_to_point (NR::Point const *p, gdouble autoscrollspeed)
{
    gdouble autoscrolldistance = (gdouble) prefs_get_int_attribute_limited ("options.autoscrolldistance", "value", 0, -1000, 10000);

    // autoscrolldistance is in screen pixels, but the display area is in document units
    autoscrolldistance /= expansion(_d2w);
    NR::Rect const dbox = NR::expand(get_display_area(), -autoscrolldistance);

    if (!((*p)[NR::X] > dbox.min()[NR::X] && (*p)[NR::X] < dbox.max()[NR::X]) ||
        !((*p)[NR::Y] > dbox.min()[NR::Y] && (*p)[NR::Y] < dbox.max()[NR::Y])   ) {

        NR::Point const s_w( (*p) * _d2w );

        gdouble x_to;
        if ((*p)[NR::X] < dbox.min()[NR::X])
            x_to = dbox.min()[NR::X];
        else if ((*p)[NR::X] > dbox.max()[NR::X])
            x_to = dbox.max()[NR::X];
        else
            x_to = (*p)[NR::X];

        gdouble y_to;
        if ((*p)[NR::Y] < dbox.min()[NR::Y])
            y_to = dbox.min()[NR::Y];
        else if ((*p)[NR::Y] > dbox.max()[NR::Y])
            y_to = dbox.max()[NR::Y];
        else
            y_to = (*p)[NR::Y];

        NR::Point const d_dt(x_to, y_to);
        NR::Point const d_w( d_dt * _d2w );
        NR::Point const moved_w( d_w - s_w );

        if (autoscrollspeed == 0)
            autoscrollspeed = prefs_get_double_attribute_limited ("options.autoscrollspeed", "value", 1, 0, 10);

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

void
SPDesktop::getWindowGeometry (gint &x, gint &y, gint &w, gint &h)
{
    _widget->getGeometry (x, y, w, h);
}

void
SPDesktop::setWindowPosition (NR::Point p)
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

bool
SPDesktop::warnDialog (gchar *text)
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

void
SPDesktop::updateNow()
{
  sp_canvas_update_now(canvas);
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
    gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(this))->window, waiting);
    gdk_cursor_unref(waiting);
    waiting_cursor = true;

    // Stupidly broken GDK cannot just set the new cursor right now - it needs some main loop iterations for that
    // Since setting waiting_cursor is usually immediately followed by some Real Work, we must run the iterations here
    // CAUTION: iterations may redraw, and redraw may be interrupted, so you cannot assume that anything is the same
    // after the call to setWaitingCursor as it was before
    while( Gtk::Main::events_pending() )
       Gtk::Main::iteration();
}

void SPDesktop::clearWaitingCursor()
{
  if (waiting_cursor)
      sp_event_context_update_cursor(sp_desktop_event_context(this));
}

void SPDesktop::toggleColorProfAdjust()
{
    _widget->toggleColorProfAdjust();
}

void SPDesktop::toggleGrids()
{
    if (namedview->grids) {
        if(gridgroup) {
            showGrids(!grids_visible);
        }
    } else {
        //there is no grid present at the moment. add a rectangular grid and make it visible
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(namedview);
        Inkscape::CanvasGrid::writeNewGridToRepr(repr, sp_desktop_document(this), Inkscape::GRID_RECTANGULAR);
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

void SPDesktop::toggleSnapping()
{
    bool v = namedview->snap_manager.getSnapEnabledGlobally();
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(namedview);
    sp_repr_set_boolean(repr, "inkscape:snap-global", !v);
}

//----------------------------------------------------------------------
// Callback implementations. The virtual ones are connected by the view.

void
SPDesktop::onPositionSet (double x, double y)
{
    _widget->viewSetPosition (NR::Point(x,y));
}

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
    if (this->doc() && doc) {
        namedview->hide(this);
        sp_item_invoke_hide (SP_ITEM (sp_document_root (this->doc())), dkey);
    }

    if (_layer_hierarchy) {
        _layer_hierarchy->clear();
        delete _layer_hierarchy;
    }
    _layer_hierarchy = new Inkscape::ObjectHierarchy(NULL);
    _layer_hierarchy->connectAdded(sigc::bind(sigc::ptr_fun(_layer_activated), this));
    _layer_hierarchy->connectRemoved(sigc::bind(sigc::ptr_fun(_layer_deactivated), this));
    _layer_hierarchy->connectChanged(sigc::bind(sigc::ptr_fun(_layer_hierarchy_changed), this));
    _layer_hierarchy->setTop(SP_DOCUMENT_ROOT(doc));

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
        NRArenaItem *ai = 0;

        namedview = sp_document_namedview (doc, NULL);
        _modified_connection = namedview->connectModified(sigc::bind<2>(sigc::ptr_fun(&_namedview_modified), this));
        number = namedview->getViewCount();

        ai = sp_item_invoke_show (SP_ITEM (sp_document_root (doc)),
                SP_CANVAS_ARENA (drawing)->arena,
                dkey,
                SP_ITEM_SHOW_DISPLAY);
        if (ai) {
            nr_arena_item_add_child (SP_CANVAS_ARENA (drawing)->root, ai, NULL);
            nr_arena_item_unref (ai);
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
    NR::Rect const a(NR::Point(0, 0), NR::Point(width, height));
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
    dt->_widget->updateScrollbars (expansion(dt->_d2w));
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
        SPObject *layer=desktop->layerForObject(item);
        if ( layer && layer != desktop->currentLayer() ) {
            desktop->setCurrentLayer(layer);
        }
    }
}

/**
 * Calls event handler of current event context.
 * \param arena Unused
 * \todo fixme
 */
static gint
_arena_handler (SPCanvasArena */*arena*/, NRArenaItem *ai, GdkEvent *event, SPDesktop *desktop)
{
    if (ai) {
        SPItem *spi = (SPItem*)NR_ARENA_ITEM_GET_DATA (ai);
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
static void
_reconstruction_start (SPDesktop * desktop)
{
    // printf("Desktop, starting reconstruction\n");
    desktop->_reconstruction_old_layer_id = g_strdup(SP_OBJECT_ID(desktop->currentLayer()));
    desktop->_layer_hierarchy->setBottom(desktop->currentRoot());

    /*
    GSList const * selection_objs = desktop->selection->list();
    for (; selection_objs != NULL; selection_objs = selection_objs->next) {

    }
    */
    desktop->selection->clear();

    // printf("Desktop, starting reconstruction end\n");
}

/// Called when document rebuild is finished.
static void
_reconstruction_finish (SPDesktop * desktop)
{
    // printf("Desktop, finishing reconstruction\n");
    if (desktop->_reconstruction_old_layer_id == NULL)
        return;

    SPObject * newLayer = SP_OBJECT_DOCUMENT(desktop->namedview)->getObjectById(desktop->_reconstruction_old_layer_id);
    if (newLayer != NULL)
        desktop->setCurrentLayer(newLayer);

    g_free(desktop->_reconstruction_old_layer_id);
    desktop->_reconstruction_old_layer_id = NULL;
    // printf("Desktop, finishing reconstruction end\n");
    return;
}

/**
 * Namedview_modified callback.
 */
static void
_namedview_modified (SPObject *obj, guint flags, SPDesktop *desktop)
{
    SPNamedView *nv=SP_NAMEDVIEW(obj);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {

        /* Recalculate snap distances */
        /* FIXME: why is the desktop getting involved in setting up something
        ** that is entirely to do with the namedview?
        */
        _update_snap_distances (desktop);

        /* Show/hide page background */
        if (nv->pagecolor & 0xff) {
            sp_canvas_item_show (desktop->table);
            ((CtrlRect *) desktop->table)->setColor(0x00000000, true, nv->pagecolor);
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

        if (SP_RGBA32_A_U(nv->pagecolor) < 128 ||
            (SP_RGBA32_R_U(nv->pagecolor) +
             SP_RGBA32_G_U(nv->pagecolor) +
             SP_RGBA32_B_U(nv->pagecolor)) >= 384) {
            // the background color is light or transparent, use black outline
            SP_CANVAS_ARENA (desktop->drawing)->arena->outlinecolor = prefs_get_int_attribute("options.wireframecolors", "onlight", 0xff);
        } else { // use white outline
            SP_CANVAS_ARENA (desktop->drawing)->arena->outlinecolor = prefs_get_int_attribute("options.wireframecolors", "ondark", 0xffffffff);
        }
    }
}

/**
 * Callback to reset snapper's distances.
 */
static void
_update_snap_distances (SPDesktop *desktop)
{
    SPUnit const &px = sp_unit_get_by_id(SP_UNIT_PX);

    SPNamedView &nv = *desktop->namedview;

    //tell all grid snappers
    for ( GSList const *l = nv.grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
        grid->snapper->setSnapperTolerance(sp_convert_distance_full(nv.gridtolerance,
                                                                      *nv.gridtoleranceunit,
                                                                      px));
    }

    nv.snap_manager.guide.setSnapperTolerance(sp_convert_distance_full(nv.guidetolerance,
                                                                       *nv.guidetoleranceunit,
                                                                       px));
    nv.snap_manager.object.setSnapperTolerance(sp_convert_distance_full(nv.objecttolerance,
                                                                        *nv.objecttoleranceunit,
                                                                        px));
}


NR::Matrix SPDesktop::w2d() const
{
    return _w2d;
}

NR::Point SPDesktop::w2d(NR::Point const &p) const
{
    return p * _w2d;
}

NR::Point SPDesktop::d2w(NR::Point const &p) const
{
    return p * _d2w;
}

NR::Matrix SPDesktop::doc2dt() const
{
    return _doc2dt;
}

NR::Point SPDesktop::doc2dt(NR::Point const &p) const
{
    return p * _doc2dt;
}

NR::Point SPDesktop::dt2doc(NR::Point const &p) const
{
    return p / _doc2dt;
}


/**
 * Pop event context from desktop's context stack. Never used.
 */
// void
// SPDesktop::pop_event_context (unsigned int key)
// {
//    SPEventContext *ec = NULL;
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
//    SPEventContext *ref = event_context;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

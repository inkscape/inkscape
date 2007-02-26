#define __SP_FLOOD_CONTEXT_C__

/*
 * Flood fill drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   John Bintz <jcoswell@coswellproductions.org>
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gdk/gdkkeysyms.h>
#include <queue>

#include "macros.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-rect.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "desktop-handles.h"
#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-rect.xpm"
#include "flood-context.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "prefs-utils.h"
#include "context-fns.h"

#include "display/nr-arena-item.h"
#include "display/nr-arena.h"
#include "display/nr-arena-image.h"
#include "display/canvas-arena.h"
#include "helper/png-write.h"
#include "libnr/nr-pixops.h"
#include "libnr/nr-matrix-rotate-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-rotate-fns.h"
#include "libnr/nr-scale-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-matrix-ops.h"
#include "libnr/nr-translate-scale-ops.h"
#include "libnr/nr-matrix-ops.h"
#include "sp-item.h"
#include "sp-root.h"
#include "sp-defs.h"
#include "splivarot.h"
#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "libnr/n-art-bpath.h"
#include "svg/svg.h"

#include "trace/trace.h"
#include "trace/potrace/inkscape-potrace.h"

static void sp_flood_context_class_init(SPFloodContextClass *klass);
static void sp_flood_context_init(SPFloodContext *flood_context);
static void sp_flood_context_dispose(GObject *object);

static void sp_flood_context_setup(SPEventContext *ec);

static gint sp_flood_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_flood_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_flood_finish(SPFloodContext *rc);

static SPEventContextClass *parent_class;


struct SPEBP {
    int width, height, sheight;
    guchar r, g, b, a;
    NRArenaItem *root; // the root arena item to show; it is assumed that all unneeded items are hidden
    guchar *px;
    unsigned (*status)(float, void *);
    void *data;
};

GtkType sp_flood_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPFloodContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_flood_context_class_init,
            NULL, NULL,
            sizeof(SPFloodContext),
            4,
            (GInstanceInitFunc) sp_flood_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPFloodContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_flood_context_class_init(SPFloodContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_flood_context_dispose;

    event_context_class->setup = sp_flood_context_setup;
    event_context_class->root_handler  = sp_flood_context_root_handler;
    event_context_class->item_handler  = sp_flood_context_item_handler;
}

static void sp_flood_context_init(SPFloodContext *flood_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(flood_context);

    event_context->cursor_shape = cursor_rect_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    event_context->shape_repr = NULL;
    event_context->shape_knot_holder = NULL;

    flood_context->item = NULL;

    new (&flood_context->sel_changed_connection) sigc::connection();
}

static void sp_flood_context_dispose(GObject *object)
{
    SPFloodContext *rc = SP_FLOOD_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection.~connection();

    /* fixme: This is necessary because we do not grab */
    if (rc->item) {
        sp_flood_finish(rc);
    }

    if (ec->shape_repr) { // remove old listener
        sp_repr_remove_listener_by_data(ec->shape_repr, ec);
        Inkscape::GC::release(ec->shape_repr);
        ec->shape_repr = 0;
    }

    if (rc->_message_context) {
        delete rc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static Inkscape::XML::NodeEventVector ec_shape_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    ec_shape_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void sp_flood_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPFloodContext *rc = SP_FLOOD_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    if (ec->shape_repr) { // remove old listener
        sp_repr_remove_listener_by_data(ec->shape_repr, ec);
        Inkscape::GC::release(ec->shape_repr);
        ec->shape_repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        Inkscape::XML::Node *shape_repr = SP_OBJECT_REPR(item);
        if (shape_repr) {
            ec->shape_repr = shape_repr;
            Inkscape::GC::anchor(shape_repr);
            sp_repr_add_listener(shape_repr, &ec_shape_repr_events, ec);
        }
    }
}

static void sp_flood_context_setup(SPEventContext *ec)
{
    SPFloodContext *rc = SP_FLOOD_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        Inkscape::XML::Node *shape_repr = SP_OBJECT_REPR(item);
        if (shape_repr) {
            ec->shape_repr = shape_repr;
            Inkscape::GC::anchor(shape_repr);
            sp_repr_add_listener(shape_repr, &ec_shape_repr_events, ec);
        }
    }

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection = sp_desktop_selection(ec->desktop)->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_flood_context_selection_changed), (gpointer)rc)
    );

    rc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

/**
Hide all items which are not listed in list, recursively, skipping groups and defs
*/
static void
hide_other_items_recursively(SPObject *o, GSList *list, unsigned dkey)
{
    if (SP_IS_ITEM(o)
        && !SP_IS_DEFS(o)
        && !SP_IS_ROOT(o)
        && !SP_IS_GROUP(o)
        && !g_slist_find(list, o))
    {
        sp_item_invoke_hide(SP_ITEM(o), dkey);
    }

     // recurse
    if (!g_slist_find(list, o)) {
        for (SPObject *child = sp_object_first_child(o) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            hide_other_items_recursively(child, list, dkey);
        }
    }
}

inline unsigned char * get_pixel(guchar *px, int x, int y, int width) {
  return px + (x + y * width) * 4;
}

static bool compare_pixels(unsigned char *a, unsigned char *b, int fuzziness) {
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) { 
      return false;
    }
  }
  return true;
}

static void try_add_to_queue(std::queue<NR::Point> *fill_queue, guchar *px, unsigned char *orig, int x, int y, int width) {
  unsigned char *t = get_pixel(px, x, y, width);
  if (compare_pixels(t, orig, 0)) {
    fill_queue->push(NR::Point(x, y));
  }
}

static void do_trace(GdkPixbuf *px, SPDesktop *desktop, NR::Matrix transform) {
    SPDocument *document = sp_desktop_document(desktop);
    
    Inkscape::Trace::Potrace::PotraceTracingEngine pte;
        
    pte.setTraceType(Inkscape::Trace::Potrace::TRACE_BRIGHTNESS);
    pte.setInvert(false);

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Glib::wrap(px, true);
    
    std::vector<Inkscape::Trace::TracingEngineResult> results = pte.trace(pixbuf);
    
    Inkscape::XML::Node *layer_repr = SP_GROUP(desktop->currentLayer())->repr;
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());

    long totalNodeCount = 0L;

    for (unsigned int i=0 ; i<results.size() ; i++) {
        Inkscape::Trace::TracingEngineResult result = results[i];
        totalNodeCount += result.getNodeCount();

        Inkscape::XML::Node *pathRepr = xml_doc->createElement("svg:path");
        /* Set style */
        sp_desktop_apply_style_tool (desktop, pathRepr, "tools.paintbucket", false);

        NArtBpath *bpath = sp_svg_read_path(result.getPathData().c_str());
        Path *path = bpath_to_Path(bpath);
        g_free(bpath);
        
        Shape *path_shape = new Shape();
        
        path->ConvertWithBackData(0.03);
        path->Fill(path_shape, 0);
        delete path;
        
        Shape *expanded_path_shape = new Shape();
        
        expanded_path_shape->ConvertToShape(path_shape, fill_nonZero);
        path_shape->MakeOffset(expanded_path_shape, 1.5, join_round, 4);
        expanded_path_shape->ConvertToShape(path_shape, fill_positive);

        Path *expanded_path = new Path();
        
        expanded_path->Reset();
        expanded_path_shape->ConvertToForme(expanded_path);
        expanded_path->ConvertEvenLines(1.0);
        expanded_path->Simplify(1.0);
        
        delete path_shape;
        delete expanded_path_shape;
        
        gchar *str = expanded_path->svg_dump_path();
        delete expanded_path;
        pathRepr->setAttribute("d", str);
        g_free(str);
        
        layer_repr->addChild(pathRepr, NULL);

        SPObject *reprobj = document->getObjectByRepr(pathRepr);
        if (reprobj) {
            sp_item_write_transform(SP_ITEM(reprobj), pathRepr, transform, NULL);
            Inkscape::Selection *selection = sp_desktop_selection(desktop);
            selection->set(reprobj);
            pathRepr->setPosition(-1);
        }
        
        Inkscape::GC::release(pathRepr);
    }
}

static void sp_flood_do_flood_fill(SPEventContext *event_context, GdkEvent *event) {
    SPDesktop *desktop = event_context->desktop;
    SPDocument *document = sp_desktop_document(desktop);

    /* Create new arena */
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);

    sp_document_ensure_up_to_date (document);
    
    SPItem *document_root = SP_ITEM(SP_DOCUMENT_ROOT(document));
    NR::Rect bbox = document_root->invokeBbox(NR::identity());

    if (bbox.isEmpty()) { return; }

    int width = (int)ceil(bbox.extent(NR::X));
    int height = (int)ceil(bbox.extent(NR::Y));

    NR::Point origin(bbox.min()[NR::X], bbox.min()[NR::Y]);
    
    NR::scale scale(width / (bbox.extent(NR::X)), height / (bbox.extent(NR::Y)));
    NR::Matrix affine = scale * NR::translate(-origin * scale);
    
    /* Create ArenaItems and set transform */
    NRArenaItem *root = sp_item_invoke_show(SP_ITEM(sp_document_root(document)), arena, dkey, SP_ITEM_SHOW_DISPLAY);
    nr_arena_item_set_transform(NR_ARENA_ITEM(root), affine);

    NRGC gc(NULL);
    nr_matrix_set_identity(&gc.transform);
    
    NRRectL final_bbox;
    final_bbox.x0 = 0;
    final_bbox.y0 = 0;//row;
    final_bbox.x1 = width;
    final_bbox.y1 = height;//row + num_rows;
    
    nr_arena_item_invoke_update(root, &final_bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

    guchar *px = g_new(guchar, 4 * width * height);
    memset(px, 0x00, 4 * width * height);

    NRPixBlock B;
    nr_pixblock_setup_extern( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                              final_bbox.x0, final_bbox.y0, final_bbox.x1, final_bbox.y1,
                              px, 4 * width, FALSE, FALSE );
    
    nr_arena_item_invoke_render( root, &final_bbox, &B, NR_ARENA_ITEM_RENDER_NO_CACHE );
    nr_pixblock_release(&B);
    
    // Hide items
    sp_item_invoke_hide(SP_ITEM(sp_document_root(document)), dkey);
    
    nr_arena_item_unref(root);
    nr_object_unref((NRObject *) arena);
    
    double zoom_scale = desktop->current_zoom();

    NR::Point pw = NR::Point(event->button.x / zoom_scale, sp_document_height(document) + (event->button.y / zoom_scale)) * affine;
    
    pw[NR::X] = (int)MIN(width - 1, MAX(0, pw[NR::X]));
    pw[NR::Y] = (int)MIN(height - 1, MAX(0, pw[NR::Y]));

    guchar *trace_px = g_new(guchar, 4 * width * height);
    memset(trace_px, 0x00, 4 * width * height);
    
    std::queue<NR::Point> fill_queue;
    fill_queue.push(pw);
    
    bool aborted = false;
    int y_limit = height - 1;
    
    unsigned char orig_color[4];
    unsigned char *orig_px = get_pixel(px, (int)pw[NR::X], (int)pw[NR::Y], width);
    for (int i = 0; i < 4; i++) { orig_color[i] = orig_px[i]; }

    while (!fill_queue.empty() && !aborted) {
      NR::Point cp = fill_queue.front();
      fill_queue.pop();
      unsigned char *s = get_pixel(px, (int)cp[NR::X], (int)cp[NR::Y], width);
      
      // same color at this point
      if (compare_pixels(s, orig_color, 0)) {
        int left = (int)cp[NR::X];
        int right = (int)cp[NR::X] + 1;
        int x = (int)cp[NR::X];
        int y = (int)cp[NR::Y];
        
        if (y > 0) { 
          try_add_to_queue(&fill_queue, px, orig_color, x, y - 1, width);
        } else {
          aborted = true; break;
        }
        if (y < y_limit) { 
          try_add_to_queue(&fill_queue, px, orig_color, x, y + 1, width);
        } else {
          aborted = true; break;
        }
        
        unsigned char *t, *trace_t;
        bool ok = false;
        
        do {
          ok = false;
          // go left
          if (left >= 0) {
            t = get_pixel(px, left, y, width);
            if (compare_pixels(t, orig_color, 0)) {
              for (int i = 0; i < 4; i++) { t[i] = 255 - t[i]; }
              trace_t = get_pixel(trace_px, left, y, width);
              trace_t[3] = 255;
              if (y > 0) { try_add_to_queue(&fill_queue, px, orig_color, left, y - 1, width); }
              if (y < y_limit) { try_add_to_queue(&fill_queue, px, orig_color, left, y + 1, width); }
              left--; ok = true;
            }
          } else {
            aborted = true; break;
          }
        } while (ok);
      
        do {
          ok = false;
          // go left
          if (right < width) {
            t = get_pixel(px, right, y, width);
            if (compare_pixels(t, orig_color, 0)) {
              for (int i = 0; i < 4; i++) { t[i] = 255 - t[i]; }
              trace_t = get_pixel(trace_px, right, y, width);
              trace_t[3] = 255; 
              if (y > 0) { try_add_to_queue(&fill_queue, px, orig_color, right, y - 1, width); }
              if (y < y_limit) { try_add_to_queue(&fill_queue, px, orig_color, right, y + 1, width); }
              right++; ok = true;
            }
          } else {
            aborted = true; break;
          }
        } while (ok);
      }
    }
    
    g_free(px);
    
    if (aborted) {
      g_free(trace_px);
      return;
    }
    
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(trace_px,
                                      GDK_COLORSPACE_RGB,
                                      TRUE,
                                      8, width, height, width * 4,
                                      (GdkPixbufDestroyNotify)g_free,
                                      NULL);

    NR::Matrix inverted_affine = NR::Matrix(affine).inverse();
    
    do_trace(pixbuf, desktop, inverted_affine);

    g_free(trace_px);
    
    sp_document_done(document, SP_VERB_CONTEXT_PAINTBUCKET, _("Fill bounded area"));
}

static gint sp_flood_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_flood_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 ) {
            sp_flood_do_flood_fill(event_context, event);

            ret = TRUE;
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;
        case GDK_Escape:
            sp_desktop_selection(desktop)->clear();
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}


static void sp_flood_finish(SPFloodContext *rc)
{
    rc->_message_context->clear();

    if ( rc->item != NULL ) {
        SPDesktop * desktop;

        desktop = SP_EVENT_CONTEXT_DESKTOP(rc);

        SP_OBJECT(rc->item)->updateRepr();

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(rc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_PAINTBUCKET,
                         _("Fill bounded area"));

        rc->item = NULL;
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

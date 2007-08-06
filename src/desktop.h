#ifndef __SP_DESKTOP_H__
#define __SP_DESKTOP_H__

/** \file
 * SPDesktop: an editable view.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gdk/gdkevents.h>
#include <gtk/gtktypeutils.h>
#include <sigc++/sigc++.h>

#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-rect.h"
#include "ui/view/view.h"
#include "ui/view/edit-widget-interface.h"
#include "perspective3d.h"

class NRRect;
class SPCSSAttr;
struct _GtkWidget;
typedef struct _GtkWidget GtkWidget;
struct SPCanvas;
struct SPCanvasItem;
struct SPCanvasGroup;
struct SPDesktopWidget;
struct SPEventContext;
struct SPItem;
struct SPNamedView;
struct SPObject;
struct SPStyle;
struct SPViewWidget;

namespace Gtk
{
  class Window;
}

typedef int sp_verb_t;



namespace Inkscape { 
  class Application;
  class MessageContext;
  class Selection; 
  class ObjectHierarchy;
  class LayerManager;
  class EventLog;
  namespace UI { 
      namespace Dialog { 
          class DialogManager; 
      }
  }
  namespace Whiteboard {
      class SessionManager;
  }
}

/**
 * Editable view.
 *
 * @see \ref desktop-handles.h for desktop macros.
 */
struct SPDesktop : public Inkscape::UI::View::View
{
    Inkscape::UI::Dialog::DialogManager *_dlg_mgr;
    SPNamedView               *namedview;
    SPCanvas                  *canvas;
    /// current selection; will never generally be NULL
    Inkscape::Selection       *selection;
    SPEventContext            *event_context;
    Inkscape::LayerManager    *layer_manager;
    Inkscape::EventLog        *event_log;

    SPCanvasItem  *acetate;
    SPCanvasGroup *main;
    SPCanvasGroup *gridgroup;
    SPCanvasGroup *guides;
    SPCanvasItem  *drawing;
    SPCanvasGroup *sketch;
    SPCanvasGroup *controls;
    SPCanvasItem  *table;       ///< outside-of-page background
    SPCanvasItem  *page;        ///< page background
    SPCanvasItem  *page_border; ///< page border
    SPCSSAttr     *current;     ///< current style

    GList *zooms_past;
    GList *zooms_future;
    unsigned int dkey;
    unsigned int number;
    guint window_state;
    unsigned int interaction_disabled_counter;
    bool waiting_cursor;

    // All 3D perspectives that are present in the current desktop
    GSList *perspectives;

    /// \todo fixme: This has to be implemented in different way */
    guint guides_active : 1;

    // storage for selected dragger used by GrDrag as it's 
    // created and deleted by tools
    SPItem *gr_item;
    guint  gr_point_type;
    guint  gr_point_i;
    bool   gr_fill_or_stroke;   
    

    Inkscape::ObjectHierarchy *_layer_hierarchy;
    gchar * _reconstruction_old_layer_id;

    sigc::signal<void, sp_verb_t>      _tool_changed;
    sigc::signal<void, SPObject *>     _layer_changed_signal;
    sigc::signal<bool, const SPCSSAttr *>::accumulated<StopOnTrue> _set_style_signal;
    sigc::signal<int, SPStyle *, int>::accumulated<StopOnNonZero> _query_style_signal;
    sigc::connection connectDocumentReplaced (const sigc::slot<void,SPDesktop*,SPDocument*> & slot)
    {
        return _document_replaced_signal.connect (slot);
    }
    
    sigc::connection connectEventContextChanged (const sigc::slot<void,SPDesktop*,SPEventContext*> & slot) 
    {
        return _event_context_changed_signal.connect (slot);
    }
    sigc::connection connectSetStyle (const sigc::slot<bool, const SPCSSAttr *> & slot) 
    {
	return _set_style_signal.connect (slot);
    }
    sigc::connection connectQueryStyle (const sigc::slot<int, SPStyle *, int> & slot) 
    {
	return _query_style_signal.connect (slot);
    }
     // subselection is some sort of selection which is specific to the tool, such as a handle in gradient tool, or a text selection
    sigc::connection connectToolSubselectionChanged(const sigc::slot<void, gpointer> & slot) {
	return _tool_subselection_changed.connect(slot);
    }
    void emitToolSubselectionChanged(gpointer data); 
    sigc::connection connectCurrentLayerChanged(const sigc::slot<void, SPObject *> & slot) {
	return _layer_changed_signal.connect(slot);
    }
    
	// Whiteboard changes

#ifdef WITH_INKBOARD
	Inkscape::Whiteboard::SessionManager* whiteboard_session_manager() {
	return _whiteboard_session_manager;
	}

	Inkscape::Whiteboard::SessionManager* _whiteboard_session_manager;
#endif
   
    SPDesktop();
    void init (SPNamedView* nv, SPCanvas* canvas);
    ~SPDesktop();
    void destroy();

    Inkscape::MessageContext *guidesMessageContext() const {
	return _guides_message_context;
    }

    void setDisplayModeNormal();
    void setDisplayModeOutline();
    void displayModeToggle();
    int displayMode;
    int getMode() const { return displayMode; }

    void set_active (bool new_active);
    SPObject *currentRoot() const;
    SPObject *currentLayer() const;
    void setCurrentLayer(SPObject *object);
    SPObject *layerForObject(SPObject *object);
    bool isLayer(SPObject *object) const;
    bool isWithinViewport(SPItem *item) const;
    bool itemIsHidden(SPItem const *item) const;
    
    void activate_guides (bool activate);
    void change_document (SPDocument *document);

    void set_event_context (GtkType type, const gchar *config);
    void push_event_context (GtkType type, const gchar *config, unsigned int key);

    void set_coordinate_status (NR::Point p);
    SPItem *item_from_list_at_point_bottom (const GSList *list, NR::Point const p) const;
    SPItem *item_at_point (NR::Point const p, bool into_groups, SPItem *upto = NULL) const;
    SPItem *group_at_point (NR::Point const p) const;
    NR::Point point() const;

    // Methods to handle 3D perspective
    void add_perspective (Box3D::Perspective3D * const persp);
    void remove_perspective (Box3D::Perspective3D * const persp);

    NR::Rect get_display_area() const;
    void set_display_area (double x0, double y0, double x1, double y1, double border, bool log = true);
    void set_display_area(NR::Rect const &a, NR::Coord border, bool log = true);
    void zoom_absolute (double cx, double cy, double zoom);
    void zoom_relative (double cx, double cy, double zoom);
    void zoom_absolute_keep_point (double cx, double cy, double px, double py, double zoom);
    void zoom_relative_keep_point (double cx, double cy, double zoom);
    void zoom_relative_keep_point (NR::Point const &c, double const zoom)
    {
            using NR::X;
            using NR::Y;
            zoom_relative_keep_point (c[X], c[Y], zoom);
    }

    void zoom_page();
    void zoom_page_width();
    void zoom_drawing();
    void zoom_selection();
    void zoom_grab_focus();
    double current_zoom() const  { return _d2w.expansion(); }
    void prev_zoom();
    void next_zoom();

    bool scroll_to_point (NR::Point const *s_dt, gdouble autoscrollspeed = 0);
    void scroll_world (double dx, double dy, bool is_scrolling = false);
    void scroll_world (NR::Point const scroll, bool is_scrolling = false)
    {
        using NR::X;
        using NR::Y;
        scroll_world(scroll[X], scroll[Y], is_scrolling);
    }

    void getWindowGeometry (gint &x, gint &y, gint &w, gint &h);
    void setWindowPosition (NR::Point p);
    void setWindowSize (gint w, gint h);
    void setWindowTransient (void* p, int transient_policy=1);
    Gtk::Window* getToplevel();
    void presentWindow();
    bool warnDialog (gchar *text);
    void toggleRulers();
    void toggleScrollbars();
    void layoutWidget();
    void destroyWidget();
    void setToolboxFocusTo (gchar const* label);
    void setToolboxAdjustmentValue (gchar const* id, double val);
    bool isToolboxButtonActive (gchar const *id);
    void updateNow();
    void updateCanvasNow();
    
    void enableInteraction();
    void disableInteraction();

    void setWaitingCursor();
    void clearWaitingCursor();
    
    void toggleGrid();
    bool gridsEnabled() { return grids_visible; }
    
    bool is_iconified();
    bool is_maximized();
    bool is_fullscreen();

    void iconify();
    void maximize();
    void fullscreen();

    void registerEditWidget (Inkscape::UI::View::EditWidgetInterface *widget)
    { _widget = widget; }

    NR::Matrix w2d() const; //transformation from window to desktop coordinates (used for zooming)
    NR::Point w2d(NR::Point const &p) const;
    NR::Point d2w(NR::Point const &p) const;
    NR::Matrix doc2dt() const;
    NR::Point doc2dt(NR::Point const &p) const;
    NR::Point dt2doc(NR::Point const &p) const;

    virtual void setDocument (SPDocument* doc);
    virtual bool shutdown();
    virtual void mouseover() {}
    virtual void mouseout() {}

	virtual bool onDeleteUI (GdkEventAny*);
	virtual bool onWindowStateEvent (GdkEventWindowState* event);

private:
    Inkscape::UI::View::EditWidgetInterface       *_widget;
    Inkscape::Application     *_inkscape;
    Inkscape::MessageContext  *_guides_message_context;
    bool _active;
    NR::Matrix _w2d;
    NR::Matrix _d2w;
    NR::Matrix _doc2dt;
    
    bool grids_visible;
    
    void push_current_zoom (GList**);

    sigc::signal<void,SPDesktop*,SPDocument*>     _document_replaced_signal;
    sigc::signal<void>                 _activate_signal;
    sigc::signal<void>                 _deactivate_signal;
    sigc::signal<void,SPDesktop*,SPEventContext*> _event_context_changed_signal;
    sigc::signal<void, gpointer>       _tool_subselection_changed;
  
    sigc::connection _activate_connection;
    sigc::connection _deactivate_connection;
    sigc::connection _sel_modified_connection;
    sigc::connection _sel_changed_connection;
    sigc::connection _reconstruction_start_connection;
    sigc::connection _reconstruction_finish_connection;
    sigc::connection _commit_connection;
    sigc::connection _modified_connection;
    
    virtual void onPositionSet (double, double);
    virtual void onResized (double, double);
    virtual void onRedrawRequested();
    virtual void onStatusMessage (Inkscape::MessageType type, gchar const *message);
    virtual void onDocumentURISet (gchar const* uri);
    virtual void onDocumentResized (double, double);

    static void _onActivate (SPDesktop* dt);
    static void _onDeactivate (SPDesktop* dt);
    static void _onSelectionModified (Inkscape::Selection *selection, guint flags, SPDesktop *dt);
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

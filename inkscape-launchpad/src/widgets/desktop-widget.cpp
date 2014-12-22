/** \file
 * Desktop widget implementation
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Johan Engelen
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

#include <2geom/rect.h>

#include "ui/dialog/dialog-manager.h"
#include "ui/tools/box3d-tool.h"
#include "cms-system.h"
#include "conn-avoid-ref.h"
#include "desktop.h"
#include "desktop-events.h"

#include "desktop-widget.h"
#include "display/sp-canvas.h"
#include "display/canvas-arena.h"
#include "document.h"
#include "ege-color-prof-tracker.h"
#include "widgets/ege-select-one-action.h"
#include <extension/db.h>
#include "file.h"
#include "helper/action.h"
#include "helper/action-context.h"
#include "util/units.h"
#include "ui/widget/unit-tracker.h"
#include "inkscape.h"
#include "ui/interface.h"
#include "macros.h"
#include "preferences.h"
#include "sp-image.h"
#include "sp-item.h"
#include "sp-namedview.h"
#include "ui/dialog/swatches.h"
#include "ui/icon-names.h"
#include "ui/widget/dock.h"
#include "ui/widget/layer-selector.h"
#include "ui/widget/selected-style.h"
#include "ui/uxmanager.h"
#include "util/ege-appear-time-tracker.h"
#include "sp-root.h"

// We're in the "widgets" directory, so no need to explicitly prefix these:
#include "button.h"
#include "ruler.h"
#include "spinbutton-events.h"
#include "spw-utilities.h"
#include "toolbox.h"
#include "widget-sizes.h"

#include "verbs.h"
#include <gtkmm/paned.h>
#include <gtkmm/messagedialog.h>

#include <gtk/gtk.h>

#if defined (SOLARIS) && (SOLARIS == 8)
#include "round.h"
using Inkscape::round;
#endif

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::UI::UXManager;
using Inkscape::UI::ToolboxFactory;
using ege::AppearTimeTracker;
using Inkscape::Util::unit_table;

enum {
    ACTIVATE,
    DEACTIVATE,
    MODIFIED,
    EVENT_CONTEXT_CHANGED,
    LAST_SIGNAL
};


//---------------------------------------------------------------------
/* SPDesktopWidget */

static void sp_desktop_widget_class_init (SPDesktopWidgetClass *klass);
static void sp_desktop_widget_dispose(GObject *object);

static void sp_desktop_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void sp_desktop_widget_realize (GtkWidget *widget);

static gint sp_desktop_widget_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw);

static void sp_dtw_color_profile_event(EgeColorProfTracker *widget, SPDesktopWidget *dtw);
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
static void cms_adjust_toggled( GtkWidget *button, gpointer data );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
static void cms_adjust_set_sensitive( SPDesktopWidget *dtw, bool enabled );
static void sp_desktop_widget_adjustment_value_changed (GtkAdjustment *adj, SPDesktopWidget *dtw);

static gdouble sp_dtw_zoom_value_to_display (gdouble value);
static gdouble sp_dtw_zoom_display_to_value (gdouble value);
static gint sp_dtw_zoom_input (GtkSpinButton *spin, gdouble *new_val, gpointer data);
static bool sp_dtw_zoom_output (GtkSpinButton *spin, gpointer data);
static void sp_dtw_zoom_value_changed (GtkSpinButton *spin, gpointer data);
static void sp_dtw_zoom_populate_popup (GtkEntry *entry, GtkMenu *menu, gpointer data);
static void sp_dtw_zoom_menu_handler (SPDesktop *dt, gdouble factor);
static void sp_dtw_zoom_50 (GtkMenuItem *item, gpointer data);
static void sp_dtw_zoom_100 (GtkMenuItem *item, gpointer data);
static void sp_dtw_zoom_200 (GtkMenuItem *item, gpointer data);
static void sp_dtw_zoom_page (GtkMenuItem *item, gpointer data);
static void sp_dtw_zoom_drawing (GtkMenuItem *item, gpointer data);
static void sp_dtw_zoom_selection (GtkMenuItem *item, gpointer data);
static void sp_dtw_sticky_zoom_toggled (GtkMenuItem *item, gpointer data);

SPViewWidgetClass *dtw_parent_class;

class CMSPrefWatcher {
public:
    CMSPrefWatcher() :
        _dpw(*this),
        _spw(*this),
        _tracker(ege_color_prof_tracker_new(0))
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        g_signal_connect( G_OBJECT(_tracker), "modified", G_CALLBACK(hook), this );
        prefs->addObserver(_dpw);
        prefs->addObserver(_spw);
    }
    virtual ~CMSPrefWatcher() {}

    //virtual void notify(PrefValue &);
    void add( SPDesktopWidget* dtw ) {
        _widget_list.push_back(dtw);
    }
    void remove( SPDesktopWidget* dtw ) {
        _widget_list.remove(dtw);
    }

private:
    static void hook(EgeColorProfTracker *tracker, gint a, gint b, CMSPrefWatcher *watcher);

    class DisplayProfileWatcher : public Inkscape::Preferences::Observer {
    public:
        DisplayProfileWatcher(CMSPrefWatcher &pw) : Observer("/options/displayprofile"), _pw(pw) {}
        virtual void notify(Inkscape::Preferences::Entry const &/*val*/) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            _pw._setCmsSensitive(!prefs->getString("/options/displayprofile/uri").empty());
            _pw._refreshAll();
        }
    private:
        CMSPrefWatcher &_pw;
    };

    DisplayProfileWatcher _dpw;

    class SoftProofWatcher : public Inkscape::Preferences::Observer {
    public:
        SoftProofWatcher(CMSPrefWatcher &pw) : Observer("/options/softproof"), _pw(pw) {}
        virtual void notify(Inkscape::Preferences::Entry const &) {
            _pw._refreshAll();
        }
    private:
        CMSPrefWatcher &_pw;
    };

    SoftProofWatcher _spw;

    void _refreshAll();
    void _setCmsSensitive(bool value);

    std::list<SPDesktopWidget*> _widget_list;
    EgeColorProfTracker *_tracker;

    friend class DisplayProfileWatcher;
    friend class SoftproofWatcher;
};

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void CMSPrefWatcher::hook(EgeColorProfTracker * /*tracker*/, gint screen, gint monitor, CMSPrefWatcher * /*watcher*/)
{
    unsigned char* buf = 0;
    guint len = 0;

    ege_color_prof_tracker_get_profile_for( screen, monitor, reinterpret_cast<gpointer*>(&buf), &len );
    Glib::ustring id = Inkscape::CMSSystem::setDisplayPer( buf, len, screen, monitor );
}
#else
void CMSPrefWatcher::hook(EgeColorProfTracker * /*tracker*/, gint /*screen*/, gint /*monitor*/, CMSPrefWatcher * /*watcher*/)
{
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

/// @todo Use conditional compilation in saner places. The whole PrefWatcher
/// object is unnecessary if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2) is not defined.
void CMSPrefWatcher::_refreshAll()
{
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    for ( std::list<SPDesktopWidget*>::iterator it = _widget_list.begin(); it != _widget_list.end(); ++it ) {
        (*it)->requestCanvasUpdate();
    }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
}

void CMSPrefWatcher::_setCmsSensitive(bool enabled)
{
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    for ( std::list<SPDesktopWidget*>::iterator it = _widget_list.begin(); it != _widget_list.end(); ++it ) {
        SPDesktopWidget *dtw = *it;
        if ( gtk_widget_get_sensitive( dtw->cms_adjust ) != enabled ) {
            cms_adjust_set_sensitive( dtw, enabled );
        }
    }
#else
    (void) enabled;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
}

static CMSPrefWatcher* watcher = NULL;

void
SPDesktopWidget::setMessage (Inkscape::MessageType type, const gchar *message)
{
    GtkLabel *sb=GTK_LABEL(this->select_status);
    gtk_label_set_markup (sb, message ? message : "");

    // make sure the important messages are displayed immediately!
    if (type == Inkscape::IMMEDIATE_MESSAGE && gtk_widget_is_drawable (GTK_WIDGET(sb))) {
        gtk_widget_queue_draw(GTK_WIDGET(sb));
        gdk_window_process_updates(gtk_widget_get_window(GTK_WIDGET(sb)), TRUE);
    }

    gtk_widget_set_tooltip_text (this->select_status_eventbox, gtk_label_get_text (sb));
}

Geom::Point
SPDesktopWidget::window_get_pointer()
{
    gint x,y;
    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(canvas));

#if GTK_CHECK_VERSION(3,0,0)
    GdkDisplay *display = gdk_window_get_display(window);
    GdkDeviceManager *dm = gdk_display_get_device_manager(display);
    GdkDevice *device = gdk_device_manager_get_client_pointer(dm);
    gdk_window_get_device_position(window, device, &x, &y, NULL);
#else
    gdk_window_get_pointer(window, &x, &y, NULL);
#endif

    return Geom::Point(x,y);
}

static GTimer *overallTimer = 0;

/**
 * Registers SPDesktopWidget class and returns its type number.
 */
GType SPDesktopWidget::getType(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPDesktopWidgetClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_desktop_widget_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPDesktopWidget),
            0, // n_preallocs
            (GInstanceInitFunc)SPDesktopWidget::init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_VIEW_WIDGET, "SPDesktopWidget", &info, static_cast<GTypeFlags>(0));
        // Begin a timer to watch for the first desktop to appear on-screen
        overallTimer = g_timer_new();
    }
    return type;
}

/**
 * SPDesktopWidget vtable initialization
 */
static void
sp_desktop_widget_class_init (SPDesktopWidgetClass *klass)
{
    dtw_parent_class = SP_VIEW_WIDGET_CLASS(g_type_class_peek_parent(klass));

    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = sp_desktop_widget_dispose;

    widget_class->size_allocate = sp_desktop_widget_size_allocate;
    widget_class->realize = sp_desktop_widget_realize;
}

/**
 * Callback for changes in size of the canvas table (i.e. the container for
 * the canvas, the rulers etc).
 *
 * This adjusts the range of the rulers when the dock container is adjusted
 * (fixes lp:950552)
 */
static void canvas_tbl_size_allocate(GtkWidget    * /*widget*/,
                                     GdkRectangle * /*allocation*/,
                                     gpointer      data)
{
    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET(data); 
    sp_desktop_widget_update_rulers(dtw);
}

/**
 * Callback for SPDesktopWidget object initialization.
 */
void SPDesktopWidget::init( SPDesktopWidget *dtw )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    new (&dtw->modified_connection) sigc::connection();

    dtw->window = 0;
    dtw->desktop = NULL;
    dtw->_interaction_disabled_counter = 0;

    /* Main table */
#if GTK_CHECK_VERSION(3,0,0)
    dtw->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    dtw->vbox = gtk_vbox_new (FALSE, 0);
#endif
    gtk_container_add( GTK_CONTAINER(dtw), GTK_WIDGET(dtw->vbox) );

#if GTK_CHECK_VERSION(3,0,0)
    dtw->statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    dtw->statusbar = gtk_hbox_new (FALSE, 0);
#endif
    //gtk_widget_set_usize (dtw->statusbar, -1, BOTTOM_BAR_HEIGHT);
    gtk_box_pack_end (GTK_BOX (dtw->vbox), dtw->statusbar, FALSE, TRUE, 0);

    {
        using Inkscape::UI::Dialogs::SwatchesPanel;

        dtw->panels = new SwatchesPanel("/embedded/swatches" /*false*/);
        dtw->panels->setOrientation(SP_ANCHOR_SOUTH);

#if GTK_CHECK_VERSION(3,0,0)
        dtw->panels->set_vexpand(false);
#endif

        gtk_box_pack_end( GTK_BOX( dtw->vbox ), GTK_WIDGET(dtw->panels->gobj()), FALSE, TRUE, 0 );
    }

#if GTK_CHECK_VERSION(3,0,0)
    dtw->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    dtw->hbox = gtk_hbox_new(FALSE, 0);
#endif

    gtk_box_pack_end( GTK_BOX (dtw->vbox), dtw->hbox, TRUE, TRUE, 0 );
    gtk_widget_show(dtw->hbox);

    dtw->aux_toolbox = ToolboxFactory::createAuxToolbox();
    gtk_box_pack_end (GTK_BOX (dtw->vbox), dtw->aux_toolbox, FALSE, TRUE, 0);

    dtw->snap_toolbox = ToolboxFactory::createSnapToolbox();
    ToolboxFactory::setOrientation( dtw->snap_toolbox, GTK_ORIENTATION_VERTICAL );
    gtk_box_pack_end( GTK_BOX(dtw->hbox), dtw->snap_toolbox, FALSE, TRUE, 0 );

    dtw->commands_toolbox = ToolboxFactory::createCommandsToolbox();
    gtk_box_pack_end (GTK_BOX (dtw->vbox), dtw->commands_toolbox, FALSE, TRUE, 0);

    dtw->tool_toolbox = ToolboxFactory::createToolToolbox();
    ToolboxFactory::setOrientation( dtw->tool_toolbox, GTK_ORIENTATION_VERTICAL );
    gtk_box_pack_start( GTK_BOX(dtw->hbox), dtw->tool_toolbox, FALSE, TRUE, 0 );

    /* Horizontal ruler */
    GtkWidget *eventbox = gtk_event_box_new ();
    dtw->hruler = sp_ruler_new(GTK_ORIENTATION_HORIZONTAL);
    dtw->hruler_box = eventbox;
    Inkscape::Util::Unit const *pt = unit_table.getUnit("pt");
    sp_ruler_set_unit(SP_RULER(dtw->hruler), pt);
    gtk_widget_set_tooltip_text (dtw->hruler_box, gettext(pt->name_plural.c_str()));
    gtk_container_add (GTK_CONTAINER (eventbox), dtw->hruler);
    g_signal_connect (G_OBJECT (eventbox), "button_press_event", G_CALLBACK (sp_dt_hruler_event), dtw);
    g_signal_connect (G_OBJECT (eventbox), "button_release_event", G_CALLBACK (sp_dt_hruler_event), dtw);
    g_signal_connect (G_OBJECT (eventbox), "motion_notify_event", G_CALLBACK (sp_dt_hruler_event), dtw);

#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *tbl = gtk_grid_new();
    dtw->canvas_tbl = gtk_grid_new();
    
    gtk_grid_attach(GTK_GRID(dtw->canvas_tbl), eventbox, 1, 0, 1, 1);
#else
    GtkWidget *tbl = gtk_table_new(2, 3, FALSE);
    dtw->canvas_tbl = gtk_table_new(3, 3, FALSE);
   
    gtk_table_attach(GTK_TABLE(dtw->canvas_tbl),
                     eventbox,
                     1, 2,     0, 1, 
		     GTK_FILL, GTK_FILL, 
		     0,        0);
#endif

    gtk_box_pack_start( GTK_BOX(dtw->hbox), tbl, TRUE, TRUE, 1 );

    /* Vertical ruler */
    eventbox = gtk_event_box_new ();
    dtw->vruler = sp_ruler_new(GTK_ORIENTATION_VERTICAL);
    dtw->vruler_box = eventbox;
    sp_ruler_set_unit (SP_RULER (dtw->vruler), pt);
    gtk_widget_set_tooltip_text (dtw->vruler_box, gettext(pt->name_plural.c_str()));
    gtk_container_add (GTK_CONTAINER (eventbox), GTK_WIDGET (dtw->vruler));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(dtw->canvas_tbl), eventbox, 0, 1, 1, 1);
#else
    gtk_table_attach(GTK_TABLE (dtw->canvas_tbl),
                     eventbox,
		     0, 1,     1, 2,
                     GTK_FILL, GTK_FILL,
                     0,        0);
#endif

    g_signal_connect (G_OBJECT (eventbox), "button_press_event", G_CALLBACK (sp_dt_vruler_event), dtw);
    g_signal_connect (G_OBJECT (eventbox), "button_release_event", G_CALLBACK (sp_dt_vruler_event), dtw);
    g_signal_connect (G_OBJECT (eventbox), "motion_notify_event", G_CALLBACK (sp_dt_vruler_event), dtw);

    // Horizontal scrollbar
    dtw->hadj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, -4000.0, 4000.0, 10.0, 100.0, 4.0));

#if GTK_CHECK_VERSION(3,0,0)
    dtw->hscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (dtw->hadj));
    gtk_grid_attach(GTK_GRID(dtw->canvas_tbl), dtw->hscrollbar, 1, 2, 1, 1);
    dtw->vscrollbar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    dtw->hscrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (dtw->hadj));
    gtk_table_attach(GTK_TABLE (dtw->canvas_tbl), dtw->hscrollbar, 1, 2, 2, 3,
            GTK_FILL, GTK_SHRINK,
            0, 0);
    dtw->vscrollbar_box = gtk_vbox_new (FALSE, 0);
#endif

    // Sticky zoom button
    dtw->sticky_zoom = sp_button_new_from_data ( Inkscape::ICON_SIZE_DECORATION,
                                                 SP_BUTTON_TYPE_TOGGLE,
                                                 NULL,
                                                 INKSCAPE_ICON("zoom-original"),
                                                 _("Zoom drawing if window size changes"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dtw->sticky_zoom), prefs->getBool("/options/stickyzoom/value"));
    gtk_box_pack_start (GTK_BOX (dtw->vscrollbar_box), dtw->sticky_zoom, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (dtw->sticky_zoom), "toggled", G_CALLBACK (sp_dtw_sticky_zoom_toggled), dtw);

    // Vertical scrollbar
    dtw->vadj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, -4000.0, 4000.0, 10.0, 100.0, 4.0));

#if GTK_CHECK_VERSION(3,0,0)
    dtw->vscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT(dtw->vadj));
#else
    dtw->vscrollbar = gtk_vscrollbar_new (GTK_ADJUSTMENT (dtw->vadj));
#endif

    gtk_box_pack_start (GTK_BOX (dtw->vscrollbar_box), dtw->vscrollbar, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(dtw->canvas_tbl), dtw->vscrollbar_box, 2, 0, 1, 2);
#else
    gtk_table_attach(GTK_TABLE(dtw->canvas_tbl), dtw->vscrollbar_box, 2, 3, 0, 2,
            GTK_SHRINK, GTK_FILL,
            0, 0);
#endif

    gchar const* tip = "";
    Inkscape::Verb* verb = Inkscape::Verb::get( SP_VERB_VIEW_CMS_TOGGLE );
    if ( verb ) {
        SPAction *act = verb->get_action( Inkscape::ActionContext( dtw->viewwidget.view ) );
        if ( act && act->tip ) {
            tip = act->tip;
        }
    }
    dtw->cms_adjust = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                               SP_BUTTON_TYPE_TOGGLE,
                                               NULL,
                                               INKSCAPE_ICON("color-management"),
                                               tip );
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    {
        Glib::ustring current = prefs->getString("/options/displayprofile/uri");
        bool enabled = current.length() > 0;
        cms_adjust_set_sensitive( dtw, enabled );
        if ( enabled ) {
            bool active = prefs->getBool("/options/displayprofile/enable");
            if ( active ) {
                sp_button_toggle_set_down( SP_BUTTON(dtw->cms_adjust), TRUE );
            }
        }
    }
    g_signal_connect_after( G_OBJECT(dtw->cms_adjust), "clicked", G_CALLBACK(cms_adjust_toggled), dtw );
#else
    cms_adjust_set_sensitive(dtw, FALSE);
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach( GTK_GRID(dtw->canvas_tbl), dtw->cms_adjust, 2, 2, 1, 1);
#else
    gtk_table_attach( GTK_TABLE(dtw->canvas_tbl), dtw->cms_adjust, 2, 3, 2, 3,
            (GtkAttachOptions)(GTK_SHRINK),
            (GtkAttachOptions)(GTK_SHRINK),
            0, 0);
#endif

    {
        if (!watcher) {
            watcher = new CMSPrefWatcher();
        }
        watcher->add(dtw);
    }

    /* Canvas */
    dtw->canvas = SP_CANVAS(SPCanvas::createAA());
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    dtw->canvas->enable_cms_display_adj = prefs->getBool("/options/displayprofile/enable");
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    gtk_widget_set_can_focus (GTK_WIDGET (dtw->canvas), TRUE);

    sp_ruler_add_track_widget (SP_RULER(dtw->hruler), GTK_WIDGET(dtw->canvas));
    sp_ruler_add_track_widget (SP_RULER(dtw->vruler), GTK_WIDGET(dtw->canvas));

#if GTK_CHECK_VERSION(3,0,0)
    GdkRGBA white = {1,1,1,1};
    gtk_widget_override_background_color(GTK_WIDGET(dtw->canvas),
                                         GTK_STATE_FLAG_NORMAL,
					 &white);
#else
    GtkStyle *style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(dtw->canvas)));
    style->bg[GTK_STATE_NORMAL] = style->white;
    gtk_widget_set_style (GTK_WIDGET (dtw->canvas), style);
#endif

    g_signal_connect (G_OBJECT (dtw->canvas), "event", G_CALLBACK (sp_desktop_widget_event), dtw);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_hexpand(GTK_WIDGET(dtw->canvas), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(dtw->canvas), TRUE);
    gtk_grid_attach(GTK_GRID(dtw->canvas_tbl), GTK_WIDGET(dtw->canvas), 1, 1, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (dtw->canvas_tbl), GTK_WIDGET(dtw->canvas), 1, 2, 1, 2, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0);
#endif

    /* Dock */
    bool create_dock =
        prefs->getIntLimited("/options/dialogtype/value", Inkscape::UI::Dialog::FLOATING, 0, 1) ==
        Inkscape::UI::Dialog::DOCK;

    if (create_dock) {
        dtw->dock = new Inkscape::UI::Widget::Dock();

#if WITH_GTKMM_3_0
        Gtk::Paned *paned = new Gtk::Paned();
#else
        Gtk::HPaned *paned = new Gtk::HPaned();
#endif

        paned->pack1(*Glib::wrap(dtw->canvas_tbl));
        paned->pack2(dtw->dock->getWidget(), Gtk::FILL);

        /* Prevent the paned from catching F6 and F8 by unsetting the default callbacks */
        if (GtkPanedClass *paned_class = GTK_PANED_CLASS (G_OBJECT_GET_CLASS (paned->gobj()))) {
            paned_class->cycle_child_focus = NULL;
            paned_class->cycle_handle_focus = NULL;
        }

#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_hexpand(GTK_WIDGET(paned->gobj()), TRUE);
        gtk_widget_set_vexpand(GTK_WIDGET(paned->gobj()), TRUE);
        gtk_grid_attach(GTK_GRID(tbl), GTK_WIDGET (paned->gobj()), 1, 1, 1, 1);
#else
        gtk_table_attach (GTK_TABLE (tbl), GTK_WIDGET (paned->gobj()), 1, 2, 1, 2, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
#endif

    } else {
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_hexpand(GTK_WIDGET(dtw->canvas_tbl), TRUE);
        gtk_widget_set_vexpand(GTK_WIDGET(dtw->canvas_tbl), TRUE);
        gtk_grid_attach(GTK_GRID(tbl), GTK_WIDGET (dtw->canvas_tbl), 1, 1, 1, 1);
#else
        gtk_table_attach (GTK_TABLE (tbl), GTK_WIDGET (dtw->canvas_tbl), 1, 2, 1, 2, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
#endif
    }

    dtw->selected_style = new Inkscape::UI::Widget::SelectedStyle(true);
    GtkHBox *ss_ = dtw->selected_style->gobj();
    gtk_box_pack_start (GTK_BOX (dtw->statusbar), GTK_WIDGET(ss_), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(dtw->statusbar), 
#if GTK_CHECK_VERSION(3,0,0)
		    gtk_separator_new(GTK_ORIENTATION_VERTICAL), 
#else
		    gtk_vseparator_new(), 
#endif
		    FALSE, FALSE, 0);

    // connect scrollbar signals
    g_signal_connect (G_OBJECT (dtw->hadj), "value-changed", G_CALLBACK (sp_desktop_widget_adjustment_value_changed), dtw);
    g_signal_connect (G_OBJECT (dtw->vadj), "value-changed", G_CALLBACK (sp_desktop_widget_adjustment_value_changed), dtw);

    GtkWidget *statusbar_tail=gtk_statusbar_new();
    gtk_box_pack_end (GTK_BOX (dtw->statusbar), statusbar_tail, FALSE, FALSE, 0);

    // zoom status spinbutton
    dtw->zoom_status = gtk_spin_button_new_with_range (log(SP_DESKTOP_ZOOM_MIN)/log(2), log(SP_DESKTOP_ZOOM_MAX)/log(2), 0.1);
    gtk_widget_set_tooltip_text (dtw->zoom_status, _("Zoom"));
    gtk_widget_set_size_request (dtw->zoom_status, STATUS_ZOOM_WIDTH, -1);
    gtk_entry_set_width_chars (GTK_ENTRY (dtw->zoom_status), 6);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dtw->zoom_status), FALSE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (dtw->zoom_status), GTK_UPDATE_ALWAYS);
    g_signal_connect (G_OBJECT (dtw->zoom_status), "input", G_CALLBACK (sp_dtw_zoom_input), dtw);
    g_signal_connect (G_OBJECT (dtw->zoom_status), "output", G_CALLBACK (sp_dtw_zoom_output), dtw);
    g_object_set_data (G_OBJECT (dtw->zoom_status), "dtw", dtw->canvas);
    g_signal_connect (G_OBJECT (dtw->zoom_status), "focus-in-event", G_CALLBACK (spinbutton_focus_in), dtw->zoom_status);
    g_signal_connect (G_OBJECT (dtw->zoom_status), "key-press-event", G_CALLBACK (spinbutton_keypress), dtw->zoom_status);
    dtw->zoom_update = g_signal_connect (G_OBJECT (dtw->zoom_status), "value_changed", G_CALLBACK (sp_dtw_zoom_value_changed), dtw);
    dtw->zoom_update = g_signal_connect (G_OBJECT (dtw->zoom_status), "populate_popup", G_CALLBACK (sp_dtw_zoom_populate_popup), dtw);

    // cursor coordinates
#if GTK_CHECK_VERSION(3,0,0)
    dtw->coord_status = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(dtw->coord_status), 0);
    gtk_grid_set_column_spacing(GTK_GRID(dtw->coord_status), 2);
    GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_grid_attach(GTK_GRID(dtw->coord_status), 
		    GTK_WIDGET(sep),
		    0, 0, 1, 2);
#else
    dtw->coord_status = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(dtw->coord_status), 0);
    gtk_table_set_col_spacings(GTK_TABLE(dtw->coord_status), 2);
    gtk_table_attach(GTK_TABLE(dtw->coord_status), 
		    gtk_vseparator_new(), 
		    0, 1, 0, 2,
                    GTK_FILL, GTK_FILL, 0, 0);
#endif

    eventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (eventbox), dtw->coord_status);
    gtk_widget_set_tooltip_text (eventbox, _("Cursor coordinates"));
    GtkWidget *label_x = gtk_label_new(_("X:"));
    gtk_misc_set_alignment (GTK_MISC(label_x), 0.0, 0.5);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(dtw->coord_status), 
            label_x, 1, 0, 1, 1);
#else
    gtk_table_attach(GTK_TABLE(dtw->coord_status),  label_x, 1,2, 0,1, GTK_FILL, GTK_FILL, 0, 0);
#endif

    GtkWidget *label_y = gtk_label_new(_("Y:"));
    gtk_misc_set_alignment (GTK_MISC(label_y), 0.0, 0.5);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(dtw->coord_status), label_y, 1, 1, 1, 1);
#else
    gtk_table_attach(GTK_TABLE(dtw->coord_status),  label_y, 1,2, 1,2, GTK_FILL, GTK_FILL, 0, 0);
#endif

    dtw->coord_status_x = gtk_label_new(NULL);
    gtk_label_set_markup( GTK_LABEL(dtw->coord_status_x), "<tt>   0.00 </tt>" );
    gtk_misc_set_alignment (GTK_MISC(dtw->coord_status_x), 1.0, 0.5);
    dtw->coord_status_y = gtk_label_new(NULL);
    gtk_label_set_markup( GTK_LABEL(dtw->coord_status_y), "<tt>   0.00 </tt>" );
    gtk_misc_set_alignment (GTK_MISC(dtw->coord_status_y), 1.0, 0.5);
    GtkWidget* label_z = gtk_label_new(_("Z:"));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_grid_attach(GTK_GRID(dtw->coord_status), dtw->coord_status_x, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(dtw->coord_status), dtw->coord_status_y, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(dtw->coord_status), label_z, 3, 0, 1, 2);
    gtk_grid_attach(GTK_GRID(dtw->coord_status), dtw->zoom_status, 4, 0, 1, 2);
#else
    gtk_table_attach(GTK_TABLE(dtw->coord_status), dtw->coord_status_x, 2,3, 0,1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(dtw->coord_status), dtw->coord_status_y, 2,3, 1,2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(dtw->coord_status),  label_z, 3,4, 0,2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(dtw->coord_status),  dtw->zoom_status, 4,5, 0,2, GTK_FILL, GTK_FILL, 0, 0);
#endif

    sp_set_font_size_smaller (dtw->coord_status);
    gtk_box_pack_end (GTK_BOX (statusbar_tail), eventbox, FALSE, FALSE, 1);

    dtw->layer_selector = new Inkscape::Widgets::LayerSelector(NULL);
    // FIXME: need to unreference on container destruction to avoid leak
    dtw->layer_selector->reference();
    //dtw->layer_selector->set_size_request(-1, SP_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(dtw->statusbar), GTK_WIDGET(dtw->layer_selector->gobj()), FALSE, FALSE, 1);

    dtw->_tracker = ege_color_prof_tracker_new(GTK_WIDGET(dtw->layer_selector->gobj()));
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    bool fromDisplay = prefs->getBool( "/options/displayprofile/from_display");
    if ( fromDisplay ) {
        Glib::ustring id = Inkscape::CMSSystem::getDisplayId( 0, 0 );

        bool enabled = false;
        dtw->canvas->cms_key = id;
        enabled = !dtw->canvas->cms_key.empty();
        cms_adjust_set_sensitive( dtw, enabled );
    }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    g_signal_connect( G_OBJECT(dtw->_tracker), "changed", G_CALLBACK(sp_dtw_color_profile_event), dtw );

    dtw->select_status_eventbox = gtk_event_box_new ();
    dtw->select_status = gtk_label_new (NULL);
    gtk_label_set_ellipsize (GTK_LABEL(dtw->select_status), PANGO_ELLIPSIZE_END);
    gtk_misc_set_alignment (GTK_MISC (dtw->select_status), 0.0, 0.5);
    gtk_widget_set_size_request (dtw->select_status, 1, -1);
    // display the initial welcome message in the statusbar
    gtk_label_set_markup (GTK_LABEL (dtw->select_status), _("<b>Welcome to Inkscape!</b> Use shape or freehand tools to create objects; use selector (arrow) to move or transform them."));
    // space label 2 pixels from left edge
    gtk_container_add (GTK_CONTAINER (dtw->select_status_eventbox), dtw->select_status);
#if GTK_CHECK_VERSION(3,0,0)
    gtk_box_pack_start(GTK_BOX(dtw->statusbar), 
                       gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), 
                       FALSE, FALSE, 2);
#else
    gtk_box_pack_start (GTK_BOX (dtw->statusbar), gtk_hbox_new(FALSE, 0), FALSE, FALSE, 2);
#endif
    gtk_box_pack_start (GTK_BOX (dtw->statusbar), dtw->select_status_eventbox, TRUE, TRUE, 0);

    gtk_widget_show_all (dtw->vbox);

    gtk_widget_grab_focus (GTK_WIDGET(dtw->canvas));

    // If this is the first desktop created, report the time it takes to show up
    if ( overallTimer ) {
        if ( prefs->getBool("/dialogs/debug/trackAppear", false) ) {
            // Time tracker takes ownership of the timer.
            AppearTimeTracker *tracker = new AppearTimeTracker(overallTimer, GTK_WIDGET(dtw), "first SPDesktopWidget");
            tracker->setAutodelete(true);
        } else {
            g_timer_destroy(overallTimer);
        }
        overallTimer = 0;
    }
    
    // Ensure that ruler ranges are updated correctly whenever the canvas table
    // is resized
    g_signal_connect (G_OBJECT (dtw->canvas_tbl),
                      "size-allocate",
                      G_CALLBACK (canvas_tbl_size_allocate),
                      dtw);
}

/**
 * Called before SPDesktopWidget destruction.
 */
static void sp_desktop_widget_dispose(GObject *object)
{
    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET (object);

    if (dtw == NULL) {
        return;
    }
    
    UXManager::getInstance()->delTrack(dtw);

    if (dtw->desktop) {
        if ( watcher ) {
            watcher->remove(dtw);
        }
        g_signal_handlers_disconnect_by_func(G_OBJECT (dtw->zoom_status), (gpointer) G_CALLBACK(sp_dtw_zoom_input), dtw);
        g_signal_handlers_disconnect_by_func(G_OBJECT (dtw->zoom_status), (gpointer) G_CALLBACK(sp_dtw_zoom_output), dtw);
        g_signal_handlers_disconnect_matched (G_OBJECT (dtw->zoom_status), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, dtw->zoom_status);
        g_signal_handlers_disconnect_by_func (G_OBJECT (dtw->zoom_status), (gpointer) G_CALLBACK (sp_dtw_zoom_value_changed), dtw);
        g_signal_handlers_disconnect_by_func (G_OBJECT (dtw->zoom_status), (gpointer) G_CALLBACK (sp_dtw_zoom_populate_popup), dtw);
        g_signal_handlers_disconnect_by_func (G_OBJECT (dtw->canvas), (gpointer) G_CALLBACK (sp_desktop_widget_event), dtw);
        g_signal_handlers_disconnect_by_func (G_OBJECT (dtw->canvas_tbl), (gpointer) G_CALLBACK (canvas_tbl_size_allocate), dtw);


        dtw->layer_selector->setDesktop(NULL);
        dtw->layer_selector->unreference();
        INKSCAPE.remove_desktop (dtw->desktop); // clears selection too
        dtw->modified_connection.disconnect();
        dtw->desktop->destroy();
        Inkscape::GC::release (dtw->desktop);
        dtw->desktop = NULL;
    }

    dtw->modified_connection.~connection();

    if (G_OBJECT_CLASS (dtw_parent_class)->dispose) {
        (* G_OBJECT_CLASS (dtw_parent_class)->dispose) (object);
    }
}


/**
 * Set the title in the desktop-window (if desktop has an own window).
 *
 * The title has form file name: desktop number - Inkscape.
 * The desktop number is only shown if it's 2 or higher,
 */
void
SPDesktopWidget::updateTitle(gchar const* uri)
{
    Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));

    if (window) {
        gchar const *fname = uri;
        GString *name = g_string_new ("");

        gchar const *grayscalename = N_("grayscale");
        gchar const *grayscalenamecomma = N_(", grayscale");
        gchar const *printcolorsname = N_("print colors preview");
        gchar const *printcolorsnamecomma = N_(", print colors preview");
        gchar const *outlinename = N_("outline");
        gchar const *nofiltersname = N_("no filters");
        gchar const *colormodename = NULL;
        gchar const *colormodenamecomma = NULL;
        gchar const *rendermodename = NULL;
        gchar const *modifiedname = "";
        SPDocument *doc = this->desktop->doc();
        if (doc->isModifiedSinceSave()) {
            modifiedname = "*";
        }

        if (this->desktop->getColorMode() == Inkscape::COLORMODE_GRAYSCALE) {
                colormodename = grayscalename;
                colormodenamecomma = grayscalenamecomma;
        } else if (this->desktop->getColorMode() == Inkscape::COLORMODE_PRINT_COLORS_PREVIEW) {
                colormodename = printcolorsname;
                colormodenamecomma = printcolorsnamecomma;
        }
        if (this->desktop->getMode() == Inkscape::RENDERMODE_OUTLINE) {
                rendermodename = outlinename;
        } else if (this->desktop->getMode() == Inkscape::RENDERMODE_NO_FILTERS) {
                rendermodename = nofiltersname;
        }
        

        if (this->desktop->number > 1) {
            if (rendermodename) {
                if (colormodenamecomma) {
                    g_string_printf (name, _("%s%s: %d (%s%s) - Inkscape"), modifiedname, fname, this->desktop->number, _(rendermodename), _(colormodenamecomma));
                } else {
                    g_string_printf (name, _("%s%s: %d (%s) - Inkscape"), modifiedname, fname, this->desktop->number, _(rendermodename));
                }
            } else {
                 if (colormodename) {
                    g_string_printf (name, _("%s%s: %d (%s) - Inkscape"), modifiedname, fname, this->desktop->number, _(colormodename));
                } else {
                    g_string_printf (name, _("%s%s: %d - Inkscape"), modifiedname, fname, this->desktop->number);
                }
            }
        } else {
            if (rendermodename) {
                if (colormodenamecomma) {
                    g_string_printf (name, _("%s%s (%s%s) - Inkscape"), modifiedname, fname, _(rendermodename), _(colormodenamecomma));
                } else {
                    g_string_printf (name, _("%s%s (%s) - Inkscape"), modifiedname, fname, _(rendermodename));
                }
            } else {
                 if (colormodename) {
                    g_string_printf (name, _("%s%s (%s) - Inkscape"), modifiedname, fname, _(colormodename));
                } else {
                    g_string_printf (name, _("%s%s - Inkscape"), modifiedname, fname);
                }
            }
        }
        window->set_title (name->str);
        g_string_free (name, TRUE);
    }
}

Inkscape::UI::Widget::Dock*
SPDesktopWidget::getDock()
{
    return dock;
}

/**
 * Callback to allocate space for desktop widget.
 */
static void
sp_desktop_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET (widget);
    GtkAllocation widg_allocation;
    gtk_widget_get_allocation(widget, &widg_allocation);

    if ((allocation->x == widg_allocation.x) &&
        (allocation->y == widg_allocation.y) &&
        (allocation->width == widg_allocation.width) &&
        (allocation->height == widg_allocation.height)) {
        if (GTK_WIDGET_CLASS (dtw_parent_class)->size_allocate)
            GTK_WIDGET_CLASS (dtw_parent_class)->size_allocate (widget, allocation);
        return;
    }

    if (gtk_widget_get_realized (widget)) {
        Geom::Rect const area = dtw->desktop->get_display_area();
        double zoom = dtw->desktop->current_zoom();

        if (GTK_WIDGET_CLASS(dtw_parent_class)->size_allocate) {
            GTK_WIDGET_CLASS(dtw_parent_class)->size_allocate (widget, allocation);
        }

        if (SP_BUTTON_IS_DOWN(dtw->sticky_zoom)) {
            /* Find new visible area */
            Geom::Rect newarea = dtw->desktop->get_display_area();
            /* Calculate adjusted zoom */
            double oldshortside = MIN(   area.width(),    area.height());
            double newshortside = MIN(newarea.width(), newarea.height());
            zoom *= newshortside / oldshortside;
        }
        dtw->desktop->zoom_absolute(area.midpoint()[Geom::X], area.midpoint()[Geom::Y], zoom);

        // TODO - Should call show_dialogs() from sp_namedview_window_from_document only.
        // But delaying the call to here solves dock sizing issues on OS X, (see #171579)
        dtw->desktop->show_dialogs();

    } else {
        if (GTK_WIDGET_CLASS (dtw_parent_class)->size_allocate) {
            GTK_WIDGET_CLASS (dtw_parent_class)->size_allocate (widget, allocation);
        }
//            this->size_allocate (widget, allocation);
    }
}

/**
 * Callback to realize desktop widget.
 */
static void
sp_desktop_widget_realize (GtkWidget *widget)
{

    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET (widget);

    if (GTK_WIDGET_CLASS (dtw_parent_class)->realize)
        (* GTK_WIDGET_CLASS (dtw_parent_class)->realize) (widget);

    Geom::Rect d = Geom::Rect::from_xywh(Geom::Point(0,0), (dtw->desktop->doc())->getDimensions());

    if (d.width() < 1.0 || d.height() < 1.0) return;

    dtw->desktop->set_display_area (d.left(), d.top(), d.right(), d.bottom(), 10);

    dtw->updateNamedview();
}

/* This is just to provide access to common functionality from sp_desktop_widget_realize() above
   as well as from SPDesktop::change_document() */
void SPDesktopWidget::updateNamedview()
{
    // Listen on namedview modification
    // originally (prior to the sigc++ conversion) the signal was simply
    // connected twice rather than disconnecting the first connection
    modified_connection.disconnect();

    modified_connection = desktop->namedview->connectModified(sigc::mem_fun(*this, &SPDesktopWidget::namedviewModified));
    namedviewModified(desktop->namedview, SP_OBJECT_MODIFIED_FLAG);

    updateTitle( desktop->doc()->getName() );
}

/**
 * Callback to handle desktop widget event.
 */
static gint
sp_desktop_widget_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
    if (event->type == GDK_BUTTON_PRESS) {
        // defocus any spinbuttons
        gtk_widget_grab_focus (GTK_WIDGET(dtw->canvas));
    }

    if ((event->type == GDK_BUTTON_PRESS) && (event->button.button == 3)) {
        if (event->button.state & GDK_SHIFT_MASK) {
            sp_canvas_arena_set_sticky (SP_CANVAS_ARENA (dtw->desktop->drawing), TRUE);
        } else {
            sp_canvas_arena_set_sticky (SP_CANVAS_ARENA (dtw->desktop->drawing), FALSE);
        }
    }

    if (GTK_WIDGET_CLASS (dtw_parent_class)->event) {
        return (* GTK_WIDGET_CLASS (dtw_parent_class)->event) (widget, event);
    } else {
        // The key press/release events need to be passed to desktop handler explicitly,
        // because otherwise the event contexts only receive key events when the mouse cursor
        // is over the canvas. This redirection is only done for key events and only if there's no
        // current item on the canvas, because item events and all mouse events are caught
        // and passed on by the canvas acetate (I think). --bb
        if ((event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE)
                && !dtw->canvas->current_item) {
            return sp_desktop_root_handler (NULL, event, dtw->desktop);
        }
    }

    return FALSE;
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void sp_dtw_color_profile_event(EgeColorProfTracker */*tracker*/, SPDesktopWidget *dtw)
{
    // Handle profile changes
    GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(dtw));
    GdkWindow *window = gtk_widget_get_window(gtk_widget_get_toplevel(GTK_WIDGET(dtw)));
    gint screenNum = gdk_screen_get_number(screen);
    gint monitor = gdk_screen_get_monitor_at_window(screen, window);
    Glib::ustring id = Inkscape::CMSSystem::getDisplayId( screenNum, monitor );
    bool enabled = false;
    dtw->canvas->cms_key = id;
    dtw->requestCanvasUpdate();
    enabled = !dtw->canvas->cms_key.empty();
    cms_adjust_set_sensitive( dtw, enabled );
}
#else
void sp_dtw_color_profile_event(EgeColorProfTracker */*tracker*/, SPDesktopWidget * /*dtw*/)
{
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void cms_adjust_toggled( GtkWidget */*button*/, gpointer data )
{
    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET(data);

    bool down = SP_BUTTON_IS_DOWN(dtw->cms_adjust);
    if ( down != dtw->canvas->enable_cms_display_adj ) {
        dtw->canvas->enable_cms_display_adj = down;
        dtw->requestCanvasUpdate();
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/options/displayprofile/enable", down);
        if (down) {
            dtw->setMessage (Inkscape::NORMAL_MESSAGE, _("Color-managed display is <b>enabled</b> in this window"));
        } else {
            dtw->setMessage (Inkscape::NORMAL_MESSAGE, _("Color-managed display is <b>disabled</b> in this window"));
        }
    }
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

void cms_adjust_set_sensitive( SPDesktopWidget *dtw, bool enabled )
{
    Inkscape::Verb* verb = Inkscape::Verb::get( SP_VERB_VIEW_CMS_TOGGLE );
    if ( verb ) {
        SPAction *act = verb->get_action( Inkscape::ActionContext( dtw->viewwidget.view ) );
        if ( act ) {
            sp_action_set_sensitive( act, enabled );
        }
    }
    gtk_widget_set_sensitive( dtw->cms_adjust, enabled );
}

void
sp_dtw_desktop_activate (SPDesktopWidget */*dtw*/)
{
    /* update active desktop indicator */
}

void
sp_dtw_desktop_deactivate (SPDesktopWidget */*dtw*/)
{
    /* update inactive desktop indicator */
}

/**
 *  Shuts down the desktop object for the view being closed.  It checks
 *  to see if the document has been edited, and if so prompts the user
 *  to save, discard, or cancel.  Returns TRUE if the shutdown operation
 *  is cancelled or if the save is cancelled or fails, FALSE otherwise.
 */
bool
SPDesktopWidget::shutdown()
{
    g_assert(desktop != NULL);

    if (INKSCAPE.sole_desktop_for_document(*desktop)) {
        SPDocument *doc = desktop->doc();
        if (doc->isModifiedSinceSave()) {
            GtkWidget *dialog;

            /** \todo
             * FIXME !!! obviously this will have problems if the document
             * name contains markup characters
             */
            dialog = gtk_message_dialog_new_with_markup(
                GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(this))),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_NONE,
                _("<span weight=\"bold\" size=\"larger\">Save changes to document \"%s\" before closing?</span>\n\n"
                  "If you close without saving, your changes will be discarded."),
                doc->getName());
            // fix for bug lp:168809
	    GtkWidget *ma = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
	    GList *ma_labels = gtk_container_get_children(GTK_CONTAINER(ma));
	    GtkWidget *label = GTK_WIDGET(g_list_first(ma_labels)->data);
	    gtk_widget_set_can_focus(label, FALSE);

            GtkWidget *close_button;
            close_button = gtk_button_new_with_mnemonic(_("Close _without saving"));
            gtk_widget_show(close_button);
            gtk_dialog_add_action_widget(GTK_DIALOG(dialog), close_button, GTK_RESPONSE_NO);

            gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
            gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Save"),   GTK_RESPONSE_YES);
            gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);

            gint response;
            response = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);

            switch (response) {
            case GTK_RESPONSE_YES:
            {
                Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));

                doc->doRef();
                sp_namedview_document_from_window(desktop);
                if (sp_file_save_document(*window, doc)) {
                    doc->doUnref();
                } else { // save dialog cancelled or save failed
                    doc->doUnref();
                    return TRUE;
                }

                break;
            }
            case GTK_RESPONSE_NO:
                break;
            default: // cancel pressed, or dialog was closed
                return TRUE;
                break;
            }
        }
        /* Code to check data loss */
        bool allow_data_loss = FALSE;
        while (doc->getReprRoot()->attribute("inkscape:dataloss") != NULL && allow_data_loss == FALSE) {
            GtkWidget *dialog;

            /** \todo
             * FIXME !!! obviously this will have problems if the document
             * name contains markup characters
             */
            dialog = gtk_message_dialog_new_with_markup(
                GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(this))),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_NONE,
                _("<span weight=\"bold\" size=\"larger\">The file \"%s\" was saved with a format that may cause data loss!</span>\n\n"
                  "Do you want to save this file as Inkscape SVG?"),
                doc->getName() ? doc->getName() : "Unnamed");
            // fix for bug lp:168809
	    GtkWidget *ma = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
	    GList *ma_labels = gtk_container_get_children(GTK_CONTAINER(ma));
	    GtkWidget *label = GTK_WIDGET(g_list_first(ma_labels)->data);
	    gtk_widget_set_can_focus(label, FALSE);

            GtkWidget *close_button;
            close_button = gtk_button_new_with_mnemonic(_("Close _without saving"));
            gtk_widget_show(close_button);
            GtkWidget *save_button = gtk_button_new_with_mnemonic(_("_Save as Inkscape SVG"));
	    gtk_widget_set_can_default(save_button, TRUE);
            gtk_widget_show(save_button);
            gtk_dialog_add_action_widget(GTK_DIALOG(dialog), close_button, GTK_RESPONSE_NO);

            gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
            gtk_dialog_add_action_widget(GTK_DIALOG(dialog), save_button, GTK_RESPONSE_YES);
            gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);

            gint response;
            response = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);

            switch (response) {
            case GTK_RESPONSE_YES:
            {
                doc->doRef();

                Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));

                if (sp_file_save_dialog(*window, doc, Inkscape::Extension::FILE_SAVE_METHOD_INKSCAPE_SVG)) {
                    doc->doUnref();
                } else { // save dialog cancelled or save failed
                    doc->doUnref();
                    return TRUE;
                }

                break;
            }
            case GTK_RESPONSE_NO:
                allow_data_loss = TRUE;
                break;
            default: // cancel pressed, or dialog was closed
                return TRUE;
                break;
            }
        }
    }

    /* Save window geometry to prefs for use as a default.
     * Use depends on setting of "options.savewindowgeometry".
     * But we save the info here regardless of the setting.
     */
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool maxed = desktop->is_maximized();
        bool full = desktop->is_fullscreen();
        prefs->setBool("/desktop/geometry/fullscreen", full);
        prefs->setBool("/desktop/geometry/maximized", maxed);
        gint w, h, x, y;
        desktop->getWindowGeometry(x, y, w, h);
        // Don't save geom for maximized windows.  It
        // just tells you the current maximized size, which is not
        // as useful as whatever value it had previously.
        if (!maxed && !full) {
            prefs->setInt("/desktop/geometry/width", w);
            prefs->setInt("/desktop/geometry/height", h);
            prefs->setInt("/desktop/geometry/x", x);
            prefs->setInt("/desktop/geometry/y", y);
        }
    }

    return FALSE;
}

/**
 * \pre this->desktop->main != 0
 */
void
SPDesktopWidget::requestCanvasUpdate() {
    // ^^ also this->desktop != 0
    g_return_if_fail(this->desktop != NULL);
    g_return_if_fail(this->desktop->main != NULL);
    gtk_widget_queue_draw (GTK_WIDGET (SP_CANVAS_ITEM (this->desktop->main)->canvas));
}

void
SPDesktopWidget::requestCanvasUpdateAndWait() {
    requestCanvasUpdate();

    while (gtk_events_pending())
      gtk_main_iteration_do(FALSE);

}

void
SPDesktopWidget::enableInteraction()
{
  g_return_if_fail(_interaction_disabled_counter > 0);

  _interaction_disabled_counter--;

  if (_interaction_disabled_counter == 0) {
    gtk_widget_set_sensitive(GTK_WIDGET(this), TRUE);
  }
}

void
SPDesktopWidget::disableInteraction()
{
  if (_interaction_disabled_counter == 0) {
    gtk_widget_set_sensitive(GTK_WIDGET(this), FALSE);
  }

  _interaction_disabled_counter++;
}

void
SPDesktopWidget::setCoordinateStatus(Geom::Point p)
{
    gchar *cstr;
    cstr = g_strdup_printf("<tt>%7.2f </tt>", dt2r * p[Geom::X]);
    gtk_label_set_markup( GTK_LABEL(this->coord_status_x), cstr );
    g_free(cstr);

    cstr = g_strdup_printf("<tt>%7.2f </tt>", dt2r * p[Geom::Y]);
    gtk_label_set_markup( GTK_LABEL(this->coord_status_y), cstr );
    g_free(cstr);
}

void
SPDesktopWidget::letZoomGrabFocus()
{
    if (zoom_status)
        gtk_widget_grab_focus (zoom_status);
}

void
SPDesktopWidget::getWindowGeometry (gint &x, gint &y, gint &w, gint &h)
{
    gboolean vis = gtk_widget_get_visible (GTK_WIDGET(this));
    (void)vis; // TODO figure out why it is here but not used.

    Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));

    if (window)
    {
        window->get_size (w, h);
        window->get_position (x, y);
    }
}

void
SPDesktopWidget::setWindowPosition (Geom::Point p)
{
    Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));

    if (window)
    {
        window->move (gint(round(p[Geom::X])), gint(round(p[Geom::Y])));
    }
}

void
SPDesktopWidget::setWindowSize (gint w, gint h)
{
    Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));

    if (window)
    {
        window->set_default_size (w, h);
        window->resize (w, h);
    }
}

/**
 * \note transientizing does not work on windows; when you minimize a document
 * and then open it back, only its transient emerges and you cannot access
 * the document window. The document window must be restored by rightclicking
 * the taskbar button and pressing "Restore"
 */
void
SPDesktopWidget::setWindowTransient (void *p, int transient_policy)
{
    Gtk::Window *window = static_cast<Gtk::Window*>(g_object_get_data(G_OBJECT(this), "window"));
    if (window)
    {
        GtkWindow *w = GTK_WINDOW(window->gobj());
        gtk_window_set_transient_for (GTK_WINDOW(p), w);

        /*
         * This enables "aggressive" transientization,
         * i.e. dialogs always emerging on top when you switch documents. Note
         * however that this breaks "click to raise" policy of a window
         * manager because the switched-to document will be raised at once
         * (so that its transients also could raise)
         */
        if (transient_policy == 2)
            // without this, a transient window not always emerges on top
            gtk_window_present (w);
    }
}

void
SPDesktopWidget::presentWindow()
{
    GtkWindow *w =GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(this)));
    if (w)
        gtk_window_present (w);
}

bool SPDesktopWidget::showInfoDialog( Glib::ustring const &message )
{
    bool result = false;
    GtkWindow *window = GTK_WINDOW( gtk_widget_get_toplevel( GTK_WIDGET(this) ) );
    if (window)
    {
        GtkWidget *dialog = gtk_message_dialog_new(
                window,
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "%s", message.c_str());
        gtk_window_set_title( GTK_WINDOW(dialog), _("Note:")); // probably want to take this as a parameter.
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    return result;
}

bool SPDesktopWidget::warnDialog (Glib::ustring const &text)
{
    Gtk::MessageDialog dialog (*window, text, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK_CANCEL);
    gint response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
        return true;
    else
        return false;
}

void
sp_desktop_widget_iconify(SPDesktopWidget *dtw)
{
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(dtw->canvas)));
    if (GTK_IS_WINDOW(topw)) {
        if (dtw->desktop->is_iconified()) {
            gtk_window_deiconify(topw);
        } else {
            gtk_window_iconify(topw);
        }
    }
}

void
sp_desktop_widget_maximize(SPDesktopWidget *dtw)
{
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(dtw->canvas)));
    if (GTK_IS_WINDOW(topw)) {
        if (dtw->desktop->is_maximized()) {
            gtk_window_unmaximize(topw);
        } else {
            // Save geometry to prefs before maximizing so that
            // something useful is stored there, because GTK doesn't maintain
            // a separate non-maximized size.
            if (!dtw->desktop->is_iconified() && !dtw->desktop->is_fullscreen())
            {
                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                gint w, h, x, y;
                dtw->getWindowGeometry(x, y, w, h);
                prefs->setInt("/desktop/geometry/width", w);
                prefs->setInt("/desktop/geometry/height", h);
                prefs->setInt("/desktop/geometry/x", x);
                prefs->setInt("/desktop/geometry/y", y);
            }
            gtk_window_maximize(topw);
        }
    }
}

void
sp_desktop_widget_fullscreen(SPDesktopWidget *dtw)
{
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
    GtkWindow *topw = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(dtw->canvas)));
    if (GTK_IS_WINDOW(topw)) {
        if (dtw->desktop->is_fullscreen()) {
            gtk_window_unfullscreen(topw);
            // widget layout is triggered by the resulting window_state_event
        } else {
            // Save geometry to prefs before maximizing so that
            // something useful is stored there, because GTK doesn't maintain
            // a separate non-maximized size.
            if (!dtw->desktop->is_iconified() && !dtw->desktop->is_maximized())
            {
                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                gint w, h, x, y;
                dtw->getWindowGeometry(x, y, w, h);
                prefs->setInt("/desktop/geometry/width", w);
                prefs->setInt("/desktop/geometry/height", h);
                prefs->setInt("/desktop/geometry/x", x);
                prefs->setInt("/desktop/geometry/y", y);
            }
            gtk_window_fullscreen(topw);
            // widget layout is triggered by the resulting window_state_event
        }
    }
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
}

/**
 * Hide whatever the user does not want to see in the window
 */
void SPDesktopWidget::layoutWidgets()
{
    SPDesktopWidget *dtw = this;
    Glib::ustring pref_root;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (dtw->desktop->is_focusMode()) {
        pref_root = "/focus/";
    } else if (dtw->desktop->is_fullscreen()) {
        pref_root = "/fullscreen/";
    } else {
        pref_root = "/window/";
    }

    if (!prefs->getBool(pref_root + "menu/state", true)) {
        gtk_widget_hide (dtw->menubar);
    } else {
        gtk_widget_show_all (dtw->menubar);
    }

    if (!prefs->getBool(pref_root + "commands/state", true)) {
        gtk_widget_hide (dtw->commands_toolbox);
    } else {
        gtk_widget_show_all (dtw->commands_toolbox);
    }

    if (!prefs->getBool(pref_root + "snaptoolbox/state", true)) {
        gtk_widget_hide (dtw->snap_toolbox);
    } else {
        gtk_widget_show_all (dtw->snap_toolbox);
    }

    if (!prefs->getBool(pref_root + "toppanel/state", true)) {
        gtk_widget_hide (dtw->aux_toolbox);
    } else {
        // we cannot just show_all because that will show all tools' panels;
        // this is a function from toolbox.cpp that shows only the current tool's panel
        ToolboxFactory::showAuxToolbox(dtw->aux_toolbox);
    }

    if (!prefs->getBool(pref_root + "toolbox/state", true)) {
        gtk_widget_hide (dtw->tool_toolbox);
    } else {
        gtk_widget_show_all (dtw->tool_toolbox);
    }

    if (!prefs->getBool(pref_root + "statusbar/state", true)) {
        gtk_widget_hide (dtw->statusbar);
    } else {
        gtk_widget_show_all (dtw->statusbar);
    }

    if (!prefs->getBool(pref_root + "panels/state", true)) {
        gtk_widget_hide ( GTK_WIDGET(dtw->panels->gobj()) );
    } else {
        gtk_widget_show_all( GTK_WIDGET(dtw->panels->gobj()) );
    }

    if (!prefs->getBool(pref_root + "scrollbars/state", true)) {
        gtk_widget_hide (dtw->hscrollbar);
        gtk_widget_hide (dtw->vscrollbar_box);
        gtk_widget_hide ( dtw->cms_adjust );
    } else {
        gtk_widget_show_all (dtw->hscrollbar);
        gtk_widget_show_all (dtw->vscrollbar_box);
        gtk_widget_show_all( dtw->cms_adjust );
    }

    if (!prefs->getBool(pref_root + "rulers/state", true)) {
        gtk_widget_hide (dtw->hruler);
        gtk_widget_hide (dtw->vruler);
    } else {
        gtk_widget_show_all (dtw->hruler);
        gtk_widget_show_all (dtw->vruler);
    }
}

void
SPDesktopWidget::setToolboxFocusTo (const gchar* label)
{
    gpointer hb = sp_search_by_data_recursive(aux_toolbox, (gpointer) label);
    if (hb && GTK_IS_WIDGET(hb))
    {
        gtk_widget_grab_focus(GTK_WIDGET(hb));
    }
}

void
SPDesktopWidget::setToolboxAdjustmentValue (gchar const *id, double value)
{
    GtkAdjustment *a = NULL;
    gpointer hb = sp_search_by_data_recursive (aux_toolbox, (gpointer) id);
    if (hb && GTK_IS_WIDGET(hb)) {
        if (GTK_IS_SPIN_BUTTON(hb))
            a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(hb));
        else if (GTK_IS_RANGE(hb))
            a = gtk_range_get_adjustment (GTK_RANGE(hb));
    }

    if (a)
        gtk_adjustment_set_value (a, value);
    else
        g_warning ("Could not find GtkAdjustment for %s\n", id);
}

void
SPDesktopWidget::setToolboxSelectOneValue (gchar const *id, int value)
{
    gpointer hb = sp_search_by_data_recursive(aux_toolbox, (gpointer) id);
    if (hb) {
        ege_select_one_action_set_active(EGE_SELECT_ONE_ACTION(hb), value);
    }
}


bool
SPDesktopWidget::isToolboxButtonActive (const gchar* id)
{
    bool isActive = false;
    gpointer thing = sp_search_by_data_recursive(aux_toolbox, (gpointer) id);
    if ( !thing ) {
        //g_message( "Unable to locate item for {%s}", id );
    } else if ( GTK_IS_TOGGLE_BUTTON(thing) ) {
        GtkToggleButton *b = GTK_TOGGLE_BUTTON(thing);
        isActive = gtk_toggle_button_get_active( b ) != 0;
    } else if ( GTK_IS_TOGGLE_ACTION(thing) ) {
        GtkToggleAction* act = GTK_TOGGLE_ACTION(thing);
        isActive = gtk_toggle_action_get_active( act ) != 0;
    } else {
        //g_message( "Item for {%s} is of an unsupported type", id );
    }

    return isActive;
}

void SPDesktopWidget::setToolboxPosition(Glib::ustring const& id, GtkPositionType pos)
{
    // Note - later on these won't be individual member variables.
    GtkWidget* toolbox = 0;
    if (id == "ToolToolbar") {
        toolbox = tool_toolbox;
    } else if (id == "AuxToolbar") {
        toolbox = aux_toolbox;
    } else if (id == "CommandsToolbar") {
        toolbox = commands_toolbox;
    } else if (id == "SnapToolbar") {
        toolbox = snap_toolbox;
    }


    if (toolbox) {
        switch(pos) {
            case GTK_POS_TOP:
            case GTK_POS_BOTTOM:
                if ( gtk_widget_is_ancestor(toolbox, hbox) ) {
                    gtk_widget_reparent( toolbox, vbox );
                    gtk_box_set_child_packing(GTK_BOX(vbox), toolbox, FALSE, TRUE, 0, GTK_PACK_START);
                }
                ToolboxFactory::setOrientation(toolbox, GTK_ORIENTATION_HORIZONTAL);
                break;
            case GTK_POS_LEFT:
            case GTK_POS_RIGHT:
                if ( !gtk_widget_is_ancestor(toolbox, hbox) ) {
                    gtk_widget_reparent( toolbox, hbox );
                    gtk_box_set_child_packing(GTK_BOX(hbox), toolbox, FALSE, TRUE, 0, GTK_PACK_START);
                    if (pos == GTK_POS_LEFT) {
                        gtk_box_reorder_child( GTK_BOX(hbox), toolbox, 0 );
                    }
                }
                ToolboxFactory::setOrientation(toolbox, GTK_ORIENTATION_VERTICAL);
                break;
        }
    }
}


SPViewWidget *sp_desktop_widget_new( SPNamedView *namedview )
{
    SPDesktopWidget* dtw = SPDesktopWidget::createInstance(namedview);

    return SP_VIEW_WIDGET(dtw);
}

SPDesktopWidget* SPDesktopWidget::createInstance(SPNamedView *namedview)
{
    SPDesktopWidget *dtw = static_cast<SPDesktopWidget*>(g_object_new(SP_TYPE_DESKTOP_WIDGET, NULL));

    dtw->dt2r = 1. / namedview->display_units->factor;

    dtw->ruler_origin = Geom::Point(0,0); //namedview->gridorigin;   Why was the grid origin used here?

    dtw->desktop = new SPDesktop();
    dtw->stub = new SPDesktopWidget::WidgetStub (dtw);
    dtw->desktop->init (namedview, dtw->canvas, dtw->stub);
    INKSCAPE.add_desktop (dtw->desktop);

    // Add the shape geometry to libavoid for autorouting connectors.
    // This needs desktop set for its spacing preferences.
    init_avoided_shape_geometry(dtw->desktop);

    dtw->selected_style->setDesktop(dtw->desktop);

    /* Once desktop is set, we can update rulers */
    sp_desktop_widget_update_rulers (dtw);

    sp_view_widget_set_view (SP_VIEW_WIDGET (dtw), dtw->desktop);

    /* Listen on namedview modification */
    dtw->modified_connection = namedview->connectModified(sigc::mem_fun(*dtw, &SPDesktopWidget::namedviewModified));

    dtw->layer_selector->setDesktop(dtw->desktop);

    dtw->menubar = sp_ui_main_menubar (dtw->desktop);
    gtk_widget_show_all (dtw->menubar);
    gtk_box_pack_start (GTK_BOX (dtw->vbox), dtw->menubar, FALSE, FALSE, 0);

    dtw->layoutWidgets();

    std::vector<GtkWidget *> toolboxes;
    toolboxes.push_back(dtw->tool_toolbox);
    toolboxes.push_back(dtw->aux_toolbox);
    toolboxes.push_back(dtw->commands_toolbox);
    toolboxes.push_back(dtw->snap_toolbox);

    dtw->panels->setDesktop( dtw->desktop );

    UXManager::getInstance()->addTrack(dtw);
    UXManager::getInstance()->connectToDesktop( toolboxes, dtw->desktop );

    return dtw;
}


void
sp_desktop_widget_update_rulers (SPDesktopWidget *dtw)
{
    Geom::Rect viewbox = dtw->desktop->get_display_area();

    double lower_x = dtw->dt2r * (viewbox.left()  - dtw->ruler_origin[Geom::X]);
    double upper_x = dtw->dt2r * (viewbox.right() - dtw->ruler_origin[Geom::X]);
    sp_ruler_set_range(SP_RULER(dtw->hruler),
	      	       lower_x,
		       upper_x,
		       (upper_x - lower_x));

    double lower_y = dtw->dt2r * (viewbox.bottom() - dtw->ruler_origin[Geom::Y]);
    double upper_y = dtw->dt2r * (viewbox.top()    - dtw->ruler_origin[Geom::Y]);
    sp_ruler_set_range(SP_RULER(dtw->vruler),
                       lower_y,
		       upper_y,
		       (upper_y - lower_y));
}


void SPDesktopWidget::namedviewModified(SPObject *obj, guint flags)
{
    SPNamedView *nv=SP_NAMEDVIEW(obj);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        this->dt2r = 1. / nv->display_units->factor;
        this->ruler_origin = Geom::Point(0,0); //nv->gridorigin;   Why was the grid origin used here?

        sp_ruler_set_unit(SP_RULER (this->vruler), nv->getDisplayUnit());
        sp_ruler_set_unit(SP_RULER (this->hruler), nv->getDisplayUnit());

        /* This loops through all the grandchildren of aux toolbox,
         * and for each that it finds, it performs an sp_search_by_data_recursive(),
         * looking for widgets that hold some "tracker" data (this is used by
         * all toolboxes to refer to the unit selector). The default document units
         * is then selected within these unit selectors.
         *
         * Of course it would be nice to be able to refer to the toolbox and the
         * unit selector directly by name, but I don't yet see a way to do that.
         *
         * This should solve: https://bugs.launchpad.net/inkscape/+bug/362995
         */
        if (GTK_IS_CONTAINER(aux_toolbox)) {
            GList *ch = gtk_container_get_children (GTK_CONTAINER(aux_toolbox));
            for (GList *i = ch; i != NULL; i = i->next) {
                if (GTK_IS_CONTAINER(i->data)) {
                    GList *grch = gtk_container_get_children (GTK_CONTAINER(i->data));
                    for (GList *j = grch; j != NULL; j = j->next) {
                        if (!GTK_IS_WIDGET(j->data)) // wasn't a widget
                            continue;

                        gpointer t = sp_search_by_data_recursive(GTK_WIDGET(j->data), (gpointer) "tracker");
                        if (t == NULL) // didn't find any tracker data
                            continue;

                        UnitTracker *tracker = reinterpret_cast<UnitTracker*>( t );
                        if (tracker == NULL) // it's null when inkscape is first opened
                            continue;

                        tracker->setActiveUnit( nv->display_units );
                    } // grandchildren
                } // if child is a container
            } // children
        } // if aux_toolbox is a container

        gtk_widget_set_tooltip_text(this->hruler_box, gettext(nv->display_units->name_plural.c_str()));
        gtk_widget_set_tooltip_text(this->vruler_box, gettext(nv->display_units->name_plural.c_str()));

        sp_desktop_widget_update_rulers(this);
        ToolboxFactory::updateSnapToolbox(this->desktop, 0, this->snap_toolbox);
    }
}

static void
sp_desktop_widget_adjustment_value_changed (GtkAdjustment */*adj*/, SPDesktopWidget *dtw)
{
    if (dtw->update)
        return;

    dtw->update = 1;

    dtw->canvas->scrollTo(gtk_adjustment_get_value(dtw->hadj), 
                          gtk_adjustment_get_value(dtw->vadj), FALSE);
    sp_desktop_widget_update_rulers (dtw);

    /*  update perspective lines if we are in the 3D box tool (so that infinite ones are shown correctly) */
    //sp_box3d_context_update_lines(dtw->desktop->event_context);
    if (SP_IS_BOX3D_CONTEXT(dtw->desktop->event_context)) {
		SP_BOX3D_CONTEXT(dtw->desktop->event_context)->_vpdrag->updateLines();
	}

    dtw->update = 0;
}

/* we make the desktop window with focus active, signal is connected in interface.c */
bool SPDesktopWidget::onFocusInEvent(GdkEventFocus*)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/bitmapautoreload/value", true)) {
        GSList const *imageList = (desktop->doc())->getResourceList("image");
        for (GSList const *p = imageList; p; p = p->next) {
            SPImage* image = SP_IMAGE(p->data);
            sp_image_refresh_if_outdated( image );
        }
    }

    INKSCAPE.activate_desktop (desktop);

    return false;
}

static gdouble
sp_dtw_zoom_value_to_display (gdouble value)
{
    return floor (10 * (pow (2, value) * 100.0 + 0.05)) / 10;
}

static gdouble
sp_dtw_zoom_display_to_value (gdouble value)
{
    return  log (value / 100.0) / log (2);
}

static gint
sp_dtw_zoom_input (GtkSpinButton *spin, gdouble *new_val, gpointer /*data*/)
{
    gdouble new_scrolled = gtk_spin_button_get_value (spin);
    const gchar *b = gtk_entry_get_text (GTK_ENTRY (spin));
    gdouble new_typed = atof (b);

    if (sp_dtw_zoom_value_to_display (new_scrolled) == new_typed) { // the new value is set by scrolling
        *new_val = new_scrolled;
    } else { // the new value is typed in
        *new_val = sp_dtw_zoom_display_to_value (new_typed);
    }

    return TRUE;
}

static bool
sp_dtw_zoom_output (GtkSpinButton *spin, gpointer /*data*/)
{
    gchar b[64];
    double val = sp_dtw_zoom_value_to_display (gtk_spin_button_get_value (spin));
    if (val < 10) {
        g_snprintf (b, 64, "%4.1f%%", val);
    } else {
        g_snprintf (b, 64, "%4.0f%%", val);
    }
    gtk_entry_set_text (GTK_ENTRY (spin), b);
    return TRUE;
}

static void
sp_dtw_zoom_value_changed (GtkSpinButton *spin, gpointer data)
{
    double const zoom_factor = pow (2, gtk_spin_button_get_value (spin));

    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET (data);
    SPDesktop *desktop = dtw->desktop;

    Geom::Rect const d = desktop->get_display_area();
    g_signal_handler_block (spin, dtw->zoom_update);
    desktop->zoom_absolute (d.midpoint()[Geom::X], d.midpoint()[Geom::Y], zoom_factor);
    g_signal_handler_unblock (spin, dtw->zoom_update);

    spinbutton_defocus (GTK_WIDGET(spin));
}


static void
sp_dtw_zoom_10 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 0.1);
}

static void
sp_dtw_zoom_25 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 0.25);
}

static void
sp_dtw_zoom_50 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 0.5);
}

static void
sp_dtw_zoom_100 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 1.0);
}

static void
sp_dtw_zoom_200 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 2.0);
}

static void
sp_dtw_zoom_500 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 5.0);
}

static void
sp_dtw_zoom_1000 (GtkMenuItem */*item*/, gpointer data)
{
    sp_dtw_zoom_menu_handler (static_cast<SPDesktop*>(data), 10.0);
}

static void
sp_dtw_zoom_page (GtkMenuItem */*item*/, gpointer data)
{
    static_cast<SPDesktop*>(data)->zoom_page();
}

static void
sp_dtw_zoom_drawing (GtkMenuItem */*item*/, gpointer data)
{
    static_cast<SPDesktop*>(data)->zoom_drawing();
}

static void
sp_dtw_zoom_selection (GtkMenuItem */*item*/, gpointer data)
{
    static_cast<SPDesktop*>(data)->zoom_selection();
}

static void
sp_dtw_zoom_populate_popup (GtkEntry */*entry*/, GtkMenu *menu, gpointer data)
{
    GList *children, *iter;
    GtkWidget *item;
    SPDesktop *dt = SP_DESKTOP_WIDGET (data)->desktop;

    children = gtk_container_get_children (GTK_CONTAINER (menu));
    for ( iter = children ; iter ; iter = g_list_next (iter)) {
        gtk_container_remove (GTK_CONTAINER (menu), GTK_WIDGET (iter->data));
    }
    g_list_free (children);

    item = gtk_menu_item_new_with_label ("1000%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_1000), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("500%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_500), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("200%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_200), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("100%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_100), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("50%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_50), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("25%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_25), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("10%");
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_10), dt);
    gtk_widget_show (item);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_menu_item_new_with_label (_("Page"));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_page), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label (_("Drawing"));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_drawing), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label (_("Selection"));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (sp_dtw_zoom_selection), dt);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
}

static void
sp_dtw_zoom_menu_handler (SPDesktop *dt, gdouble factor)
{
    Geom::Rect const d = dt->get_display_area();
    dt->zoom_absolute(d.midpoint()[Geom::X], d.midpoint()[Geom::Y], factor);
}


static void
sp_dtw_sticky_zoom_toggled (GtkMenuItem *, gpointer data)
{
    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET(data);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/options/stickyzoom/value", SP_BUTTON_IS_DOWN(dtw->sticky_zoom));
}


void
sp_desktop_widget_update_zoom (SPDesktopWidget *dtw)
{
    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(dtw->zoom_status));

    g_signal_handlers_block_by_func (G_OBJECT (dtw->zoom_status), (gpointer)G_CALLBACK (sp_dtw_zoom_value_changed), dtw);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dtw->zoom_status), log(dtw->desktop->current_zoom()) / log(2));
    gtk_widget_queue_draw(GTK_WIDGET(dtw->zoom_status));
    if (window)
        gdk_window_process_updates(window, TRUE);
    g_signal_handlers_unblock_by_func (G_OBJECT (dtw->zoom_status), (gpointer)G_CALLBACK (sp_dtw_zoom_value_changed), dtw);
}

void
sp_desktop_widget_toggle_rulers (SPDesktopWidget *dtw)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (gtk_widget_get_visible (dtw->hruler)) {
        gtk_widget_hide (dtw->hruler);
        gtk_widget_hide (dtw->vruler);
        prefs->setBool(dtw->desktop->is_fullscreen() ? "/fullscreen/rulers/state" : "/window/rulers/state", false);
    } else {
        gtk_widget_show_all (dtw->hruler);
        gtk_widget_show_all (dtw->vruler);
        prefs->setBool(dtw->desktop->is_fullscreen() ? "/fullscreen/rulers/state" : "/window/rulers/state", true);
    }
}

void
sp_desktop_widget_toggle_scrollbars (SPDesktopWidget *dtw)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (gtk_widget_get_visible (dtw->hscrollbar)) {
        gtk_widget_hide (dtw->hscrollbar);
        gtk_widget_hide (dtw->vscrollbar_box);
        gtk_widget_hide ( dtw->cms_adjust );
        prefs->setBool(dtw->desktop->is_fullscreen() ? "/fullscreen/scrollbars/state" : "/window/scrollbars/state", false);
    } else {
        gtk_widget_show_all (dtw->hscrollbar);
        gtk_widget_show_all (dtw->vscrollbar_box);
        gtk_widget_show_all( dtw->cms_adjust );
        prefs->setBool(dtw->desktop->is_fullscreen() ? "/fullscreen/scrollbars/state" : "/window/scrollbars/state", true);
    }
}

bool sp_desktop_widget_color_prof_adj_enabled( SPDesktopWidget *dtw )
{
    return gtk_widget_get_sensitive( dtw->cms_adjust ) &&
              SP_BUTTON_IS_DOWN(dtw->cms_adjust) ;
}

void sp_desktop_widget_toggle_color_prof_adj( SPDesktopWidget *dtw )
{

    if ( gtk_widget_get_sensitive( dtw->cms_adjust ) ) {
        if ( SP_BUTTON_IS_DOWN(dtw->cms_adjust) ) {
            sp_button_toggle_set_down( SP_BUTTON(dtw->cms_adjust), FALSE );
        } else {
            sp_button_toggle_set_down( SP_BUTTON(dtw->cms_adjust), TRUE );
        }
    }
}

/* Unused
void
sp_spw_toggle_menubar (SPDesktopWidget *dtw, bool is_fullscreen)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (gtk_widget_get_visible (dtw->menubar)) {
        gtk_widget_hide (dtw->menubar);
        prefs->setBool(is_fullscreen ? "/fullscreen/menu/state" : "/window/menu/state", false);
    } else {
        gtk_widget_show_all (dtw->menubar);
        prefs->setBool(is_fullscreen ? "/fullscreen/menu/state" : "/window/menu/state", true);
    }
}
*/

static void
set_adjustment (GtkAdjustment *adj, double l, double u, double ps, double si, double pi)
{
    if ((l != gtk_adjustment_get_lower(adj)) ||
        (u != gtk_adjustment_get_upper(adj)) ||
        (ps != gtk_adjustment_get_page_size(adj)) ||
        (si != gtk_adjustment_get_step_increment(adj)) ||
        (pi != gtk_adjustment_get_page_increment(adj))) {
	    gtk_adjustment_set_lower(adj, l);
	    gtk_adjustment_set_upper(adj, u);
	    gtk_adjustment_set_page_size(adj, ps);
	    gtk_adjustment_set_step_increment(adj, si);
	    gtk_adjustment_set_page_increment(adj, pi);
	    gtk_adjustment_changed (adj);
    }
}

void
sp_desktop_widget_update_scrollbars (SPDesktopWidget *dtw, double scale)
{
    if (!dtw) return;
    if (dtw->update) return;
    dtw->update = 1;

    /* The desktop region we always show unconditionally */
    SPDocument *doc = dtw->desktop->doc();
    Geom::Rect darea ( Geom::Point(-doc->getWidth().value("px"), -doc->getHeight().value("px")),
                     Geom::Point(2 * doc->getWidth().value("px"), 2 * doc->getHeight().value("px"))  );

    Geom::OptRect deskarea;
    if (Inkscape::Preferences::get()->getInt("/tools/bounding_box") == 0) {
        deskarea = darea | doc->getRoot()->desktopVisualBounds();
    } else {
        deskarea = darea | doc->getRoot()->desktopGeometricBounds();
    }

    /* Canvas region we always show unconditionally */
    Geom::Rect carea( Geom::Point(deskarea->min()[Geom::X] * scale - 64, deskarea->max()[Geom::Y] * -scale - 64),
                    Geom::Point(deskarea->max()[Geom::X] * scale + 64, deskarea->min()[Geom::Y] * -scale + 64)  );

    Geom::Rect viewbox = dtw->canvas->getViewbox();

    /* Viewbox is always included into scrollable region */
    carea = Geom::unify(carea, viewbox);

    set_adjustment(dtw->hadj, carea.min()[Geom::X], carea.max()[Geom::X],
                   viewbox.dimensions()[Geom::X],
                   0.1 * viewbox.dimensions()[Geom::X],
                   viewbox.dimensions()[Geom::X]);
    gtk_adjustment_set_value(dtw->hadj, viewbox.min()[Geom::X]);

    set_adjustment(dtw->vadj, carea.min()[Geom::Y], carea.max()[Geom::Y],
                   viewbox.dimensions()[Geom::Y],
                   0.1 * viewbox.dimensions()[Geom::Y],
                   viewbox.dimensions()[Geom::Y]);
    gtk_adjustment_set_value(dtw->vadj, viewbox.min()[Geom::Y]);

    dtw->update = 0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

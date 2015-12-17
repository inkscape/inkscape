#ifndef SEEN_SP_DESKTOP_WIDGET_H
#define SEEN_SP_DESKTOP_WIDGET_H

/** \file
 * SPDesktopWidget: handling Gtk events on a desktop.
 *
 * Authors:
 *      Jon A. Cruz <jon@joncruz.org> (c) 2010
 *      John Bintz <jcoswell@coswellproductions.org> (c) 2006
 *      Ralf Stephan <ralf@ark.in-berlin.de> (c) 2005, distrib. under GPL2
 *      Abhishek Sharma
 *      ? -2004
 */

#include <gtkmm/window.h>
#include "message.h"
#include "ui/view/view-widget.h"
#include "ui/view/edit-widget-interface.h"

#include <stddef.h>
#include <sigc++/connection.h>
#include <2geom/point.h>

// forward declaration
typedef struct _EgeColorProfTracker EgeColorProfTracker;
struct SPCanvas;
class SPDesktop;
struct SPDesktopWidget;
class SPObject;


#define SP_TYPE_DESKTOP_WIDGET SPDesktopWidget::getType()
#define SP_DESKTOP_WIDGET(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_DESKTOP_WIDGET, SPDesktopWidget))
#define SP_DESKTOP_WIDGET_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_DESKTOP_WIDGET, SPDesktopWidgetClass))
#define SP_IS_DESKTOP_WIDGET(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_DESKTOP_WIDGET))
#define SP_IS_DESKTOP_WIDGET_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_DESKTOP_WIDGET))

void sp_desktop_widget_show_decorations(SPDesktopWidget *dtw, gboolean show);
void sp_desktop_widget_iconify(SPDesktopWidget *dtw);
void sp_desktop_widget_maximize(SPDesktopWidget *dtw);
void sp_desktop_widget_fullscreen(SPDesktopWidget *dtw);
void sp_desktop_widget_update_zoom(SPDesktopWidget *dtw);
void sp_desktop_widget_update_rulers (SPDesktopWidget *dtw);
void sp_desktop_widget_update_hruler (SPDesktopWidget *dtw);
void sp_desktop_widget_update_vruler (SPDesktopWidget *dtw);

/* Show/hide rulers & scrollbars */
void sp_desktop_widget_toggle_rulers (SPDesktopWidget *dtw);
void sp_desktop_widget_toggle_scrollbars (SPDesktopWidget *dtw);
void sp_desktop_widget_update_scrollbars (SPDesktopWidget *dtw, double scale);
void sp_desktop_widget_toggle_color_prof_adj( SPDesktopWidget *dtw );
void sp_desktop_widget_toggle_guides_lock( SPDesktopWidget *dtw );
bool sp_desktop_widget_color_prof_adj_enabled( SPDesktopWidget *dtw );

void sp_dtw_desktop_activate (SPDesktopWidget *dtw);
void sp_dtw_desktop_deactivate (SPDesktopWidget *dtw);

namespace Inkscape { namespace Widgets { class LayerSelector; } }

namespace Inkscape { namespace UI { namespace Widget { class SelectedStyle; } } }

namespace Inkscape { namespace UI { namespace Dialogs { class SwatchesPanel; } } }

/// A GtkEventBox on an SPDesktop.
struct SPDesktopWidget {
    SPViewWidget viewwidget;

    unsigned int update : 1;

    sigc::connection modified_connection;

    SPDesktop *desktop;

    Gtk::Window *window;

    // The root vbox of the window layout.
    GtkWidget *vbox;

    GtkWidget *hbox;

    GtkWidget *menubar, *statusbar;

    Inkscape::UI::Dialogs::SwatchesPanel *panels;

    GtkWidget *hscrollbar, *vscrollbar, *vscrollbar_box;

    /* Rulers */
    GtkWidget *hruler, *vruler;
    GtkWidget *hruler_box, *vruler_box; // eventboxes for setting tooltips

    GtkWidget *guides_lock;
    GtkWidget *sticky_zoom;
    GtkWidget *cms_adjust;
    GtkWidget *coord_status;
    GtkWidget *coord_status_x;
    GtkWidget *coord_status_y;
    GtkWidget *select_status;
    GtkWidget *select_status_eventbox;
    GtkWidget *zoom_status;
    gulong zoom_update;

    Inkscape::UI::Widget::Dock *dock;

    Inkscape::UI::Widget::SelectedStyle *selected_style;

    gint coord_status_id, select_status_id;

    unsigned int _interaction_disabled_counter;

    SPCanvas  *canvas;

    /** A table for displaying the canvas, rulers etc */
    GtkWidget *canvas_tbl;

    Geom::Point ruler_origin;
    double dt2r;

    GtkAdjustment *hadj, *vadj;

    Inkscape::Widgets::LayerSelector *layer_selector;

    EgeColorProfTracker* _tracker;

    struct WidgetStub : public Inkscape::UI::View::EditWidgetInterface {
        SPDesktopWidget *_dtw;
        WidgetStub (SPDesktopWidget* dtw) : _dtw(dtw) {}

        virtual void setTitle (gchar const *uri)
            { _dtw->updateTitle (uri); }
        virtual Gtk::Window* getWindow()
            { return _dtw->window; }

        virtual void layout() {
            _dtw->layoutWidgets();
        }

        virtual void present()
            { _dtw->presentWindow(); }
        virtual void getGeometry (gint &x, gint &y, gint &w, gint &h)
            { _dtw->getWindowGeometry (x, y, w, h); }
        virtual void setSize (gint w, gint h)
            { _dtw->setWindowSize (w, h); }
        virtual void setPosition (Geom::Point p)
            { _dtw->setWindowPosition (p); }
        virtual void setTransient (void* p, int transient_policy)
            { _dtw->setWindowTransient (p, transient_policy); }
        virtual Geom::Point getPointer()
            { return _dtw->window_get_pointer(); }
        virtual void setIconified()
            { sp_desktop_widget_iconify (_dtw); }
        virtual void setMaximized()
            { sp_desktop_widget_maximize (_dtw); }
        virtual void setFullscreen()
            { sp_desktop_widget_fullscreen (_dtw); }
        virtual bool shutdown()
            { return _dtw->shutdown(); }
        virtual void destroy()
            {
                if(_dtw->window != NULL)
                    delete _dtw->window;
                _dtw->window = NULL;
            }

        virtual void requestCanvasUpdate()
            { _dtw->requestCanvasUpdate(); }
        virtual void requestCanvasUpdateAndWait()
            { _dtw->requestCanvasUpdateAndWait(); }
        virtual void enableInteraction()
            { _dtw->enableInteraction(); }
        virtual void disableInteraction()
            { _dtw->disableInteraction(); }
        virtual void activateDesktop()
            { sp_dtw_desktop_activate (_dtw); }
        virtual void deactivateDesktop()
            { sp_dtw_desktop_deactivate (_dtw); }
        virtual void updateRulers()
            { sp_desktop_widget_update_rulers (_dtw); }
        virtual void updateScrollbars (double scale)
            { sp_desktop_widget_update_scrollbars (_dtw, scale); }
        virtual void toggleRulers()
            { sp_desktop_widget_toggle_rulers (_dtw); }
        virtual void toggleScrollbars()
            { sp_desktop_widget_toggle_scrollbars (_dtw); }
        virtual void toggleColorProfAdjust()
            { sp_desktop_widget_toggle_color_prof_adj(_dtw); }
        virtual void toggleGuidesLock()
            { sp_desktop_widget_toggle_guides_lock(_dtw); }
        virtual bool colorProfAdjustEnabled()
            { return sp_desktop_widget_color_prof_adj_enabled(_dtw); }
        virtual void updateZoom()
            { sp_desktop_widget_update_zoom (_dtw); }
        virtual void letZoomGrabFocus()
            { _dtw->letZoomGrabFocus(); }
        virtual void setToolboxFocusTo (const gchar * id)
            { _dtw->setToolboxFocusTo (id); }
        virtual void setToolboxAdjustmentValue (const gchar *id, double val)
            { _dtw->setToolboxAdjustmentValue (id, val); }
        virtual void setToolboxSelectOneValue (gchar const *id, int val)
            { _dtw->setToolboxSelectOneValue (id, val); }
        virtual bool isToolboxButtonActive (gchar const* id)
            { return _dtw->isToolboxButtonActive (id); }
        virtual void setCoordinateStatus (Geom::Point p)
            { _dtw->setCoordinateStatus (p); }
        virtual void setMessage (Inkscape::MessageType type, gchar const* msg)
            { _dtw->setMessage (type, msg); }

        virtual bool showInfoDialog( Glib::ustring const &message ) {
            return _dtw->showInfoDialog( message );
        }

        virtual bool warnDialog (Glib::ustring const &text)
            { return _dtw->warnDialog (text); }

        virtual Inkscape::UI::Widget::Dock* getDock ()
            { return _dtw->getDock(); }
    };

    WidgetStub *stub;

    void setMessage(Inkscape::MessageType type, gchar const *message);
    Geom::Point window_get_pointer();
    bool shutdown();
    void viewSetPosition (Geom::Point p);
    void letZoomGrabFocus();
    void getWindowGeometry (gint &x, gint &y, gint &w, gint &h);
    void setWindowPosition (Geom::Point p);
    void setWindowSize (gint w, gint h);
    void setWindowTransient (void *p, int transient_policy);
    void presentWindow();
    bool showInfoDialog( Glib::ustring const &message );
    bool warnDialog (Glib::ustring const &text);
    void setToolboxFocusTo (gchar const *);
    void setToolboxAdjustmentValue (gchar const * id, double value);
    void setToolboxSelectOneValue (gchar const * id, gint value);
    bool isToolboxButtonActive (gchar const *id);
    void setToolboxPosition(Glib::ustring const& id, GtkPositionType pos);
    void setCoordinateStatus(Geom::Point p);
    void requestCanvasUpdate();
    void requestCanvasUpdateAndWait();
    void enableInteraction();
    void disableInteraction();
    void updateTitle(gchar const *uri);
    bool onFocusInEvent(GdkEventFocus*);

    Inkscape::UI::Widget::Dock* getDock();

    static GType getType();
    static SPDesktopWidget* createInstance(SPNamedView *namedview);

    void updateNamedview();

private:
    GtkWidget *tool_toolbox;
    GtkWidget *aux_toolbox;
    GtkWidget *commands_toolbox;
    GtkWidget *snap_toolbox;

    static void init(SPDesktopWidget *widget);
    void layoutWidgets();

    void namedviewModified(SPObject *obj, guint flags);

};

/// The SPDesktopWidget vtable
struct SPDesktopWidgetClass {
    SPViewWidgetClass parent_class;
};

#endif /* !SEEN_SP_DESKTOP_WIDGET_H */

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

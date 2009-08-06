/**
 * \brief This class implements the functionality of the window layout, menus,
 *        and signals.
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   John Bintz <jcoswell@coswellproductions.org>
 *
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_VIEW_EDIT_WIDGET_H
#define INKSCAPE_UI_VIEW_EDIT_WIDGET_H

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/togglebutton.h>

#include "ui/dialog/dialog-manager.h"
#include "ui/view/edit-widget-interface.h"
#include "ui/widget/dock.h"
#include "ui/widget/layer-selector.h"
#include "ui/widget/ruler.h"
#include "ui/widget/selected-style.h"
#include "ui/widget/svg-canvas.h"
#include "ui/widget/toolbox.h"
#include "ui/widget/zoom-status.h"

struct SPDesktop;
struct SPDocument;
struct SPNamedView;

namespace Inkscape {
namespace UI {
namespace View {

class EditWidget : public Gtk::Window,
                   public EditWidgetInterface {
public:
    EditWidget (SPDocument*);
    ~EditWidget();

    // Initialization
    void initActions();
    void initUIManager();
    void initLayout();
    void initEdit (SPDocument*);
    void destroyEdit();

    // Actions
    void onActionFileNew();
    void onActionFileOpen();
    void onActionFilePrint();
    void onActionFileQuit();
    void onToolbarItem();
    void onSelectTool();
    void onNodeTool();

    // Menus
    void onMenuItem();

    void onDialogAbout();
    void onDialogAlignAndDistribute();
    void onDialogInkscapePreferences();
    void onDialogDialog();
    void onDialogDocumentProperties();
    void onDialogExport();
    void onDialogExtensionEditor();
    void onDialogFillAndStroke();
    void onDialogFind();
    void onDialogLayerEditor();
    void onDialogMessages();
    void onDialogObjectProperties();
    void onDialogTextProperties();
    void onDialogTransform();
    void onDialogTransformation();
    void onDialogTrace();
    void onDialogXmlEditor();

    // Whiteboard (Inkboard)
#ifdef WITH_INKBOARD
    void onDialogWhiteboardConnect();
    void onDialogWhiteboardShareWithUser();
    void onDialogWhiteboardShareWithChat();
    void onDialogOpenSessionFile();
    void onDumpXMLTracker();
#endif

    void onUriChanged();

    // from EditWidgetInterface
    virtual Gtk::Window* getWindow();
    virtual void setTitle (gchar const*);
    virtual void layout();
    virtual void present();
    virtual void getGeometry (gint &x, gint &y, gint &w, gint &h);
    virtual void setSize (gint w, gint h);
    virtual void setPosition (Geom::Point p);
    virtual void setTransient (void*, int);
    virtual Geom::Point getPointer();
    virtual void setIconified();
    virtual void setMaximized();
    virtual void setFullscreen();
    virtual bool shutdown();
    virtual void destroy();
    virtual void requestCanvasUpdate();
    virtual void requestCanvasUpdateAndWait();
    virtual void enableInteraction();
    virtual void disableInteraction();
    virtual void activateDesktop();
    virtual void deactivateDesktop();
    virtual void viewSetPosition (Geom::Point p);
    virtual void updateRulers();
    virtual void updateScrollbars (double scale);
    virtual void toggleRulers();
    virtual void toggleScrollbars();
    virtual void toggleColorProfAdjust();
    virtual void updateZoom();
    virtual void letZoomGrabFocus();
    virtual void setToolboxFocusTo (const gchar *);
    virtual void setToolboxAdjustmentValue (const gchar *, double);
    virtual void setToolboxSelectOneValue (const gchar *, gint);
    virtual bool isToolboxButtonActive (gchar const*);
    virtual void setCoordinateStatus (Geom::Point p);
    virtual void setMessage (Inkscape::MessageType type, gchar const* msg);
    virtual bool warnDialog (gchar*);

    virtual Inkscape::UI::Widget::Dock* getDock ();

protected:
    void _namedview_modified(SPObject *namedview, guint);

    Gtk::Tooltips        _tooltips;

    // Child widgets:
    Gtk::Table           _main_window_table;
    Gtk::VBox            _toolbars_vbox;
    Gtk::HBox            _sub_window_hbox;
    Gtk::Table           _viewport_table;

    UI::Widget::Toolbox  *_tool_ctrl;
    Gtk::Toolbar         *_select_ctrl;
    Gtk::Toolbar         *_uri_ctrl;
    Gtk::Label           _uri_label;
    Gtk::Entry           _uri_entry;
    Gtk::Toolbar         *_node_ctrl;

    UI::Widget::HRuler   _top_ruler;
    UI::Widget::VRuler   _left_ruler;
    Gtk::HScrollbar      _bottom_scrollbar;
    Gtk::VScrollbar      _right_scrollbar;
    Gtk::ToggleButton    _sticky_zoom;
    UI::Widget::SVGCanvas _svg_canvas;
    Gtk::HBox            _statusbar;
    UI::Widget::Dock _dock;
    UI::Widget::SelectedStyle _selected_style_status;
    UI::Widget::ZoomStatus _zoom_status;
    Inkscape::Widgets::LayerSelector _layer_selector;
    Gtk::EventBox        _coord_eventbox;
    Gtk::Table           _coord_status;
    Gtk::Label           _coord_status_x, _coord_status_y;
    Gtk::Label           _select_status;

    SPDesktop*           _desktop;
    SPNamedView*         _namedview;
    double               _dt2r;

    Glib::RefPtr<Gtk::ActionGroup>  _act_grp;
    Glib::RefPtr<Gtk::UIManager>    _ui_mgr;
    UI::Dialog::DialogManager       _dlg_mgr;

    void initMenuActions();
    void initToolbarActions();
    void initAccelMap();
    void initMenuBar();
    void initCommandsBar();
    void initToolControlsBar();
    void initUriBar();
    void initToolsBar();
    void initBottomScrollbar();
    void initRightScrollbar();
    void initLeftRuler();
    void initTopRuler();
    void initStickyZoom();
    void initStatusbar();

    virtual bool on_key_press_event (GdkEventKey*);
    virtual bool on_delete_event (GdkEventAny*);
    virtual bool on_focus_in_event (GdkEventFocus*);

private:
    bool onEntryFocusIn (GdkEventFocus*);
    bool onEntryFocusOut (GdkEventFocus*);
    void onWindowSizeAllocate (Gtk::Allocation&);
    void onWindowRealize();
    void onAdjValueChanged();

    bool _update_s_f, _update_a_f;
    unsigned int _interaction_disabled_counter;

    sigc::connection _namedview_modified_connection;
};
} // namespace View
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_VIEW_EDIT_WIDGET_H

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

/**
 * @file
 * Main UI stuff.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2012 Kris De Gussem
 * Copyright (C) 2010 authors
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ui/dialog/dialog-manager.h"
#include <gtkmm/icontheme.h>
#include "file.h"
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>

#include "inkscape.h"
#include "extension/db.h"
#include "extension/effect.h"
#include "extension/input.h"
#include "widgets/icon.h"
#include "preferences.h"
#include "path-prefix.h"
#include "shortcuts.h"
#include "document.h"

#include "ui/interface.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "svg-view-widget.h"
#include "widgets/desktop-widget.h"
#include "sp-item-group.h"
#include "sp-text.h"
#include "sp-gradient.h"
#include "sp-flowtext.h"
#include "sp-namedview.h"
#include "sp-root.h"
#include "ui/view/view.h"
#include "helper/action.h"
#include "helper/action-context.h"
#include "helper/gnome-utils.h"
#include "helper/window.h"
#include "io/sys.h"
#include "ui/dialog-events.h"
#include "message-context.h"
#include "ui/uxmanager.h"
#include "ui/clipboard.h"

#include "display/sp-canvas.h"
#include "color.h"
#include "svg/svg-color.h"
#include "desktop-style.h"
#include "style.h"
#include "ui/tools/tool-base.h"
#include "gradient-drag.h"
#include "widgets/ege-paint-def.h"
#include "document-undo.h"
#include "sp-anchor.h"
#include "sp-clippath.h"
#include "sp-image.h"
#include "sp-item.h"
#include "sp-mask.h"
#include "message-stack.h"
#include "ui/dialog/layer-properties.h"

#include <gdk/gdkkeysyms.h>

#include <glibmm/miscutils.h>

using Inkscape::DocumentUndo;

/* Drag and Drop */
typedef enum {
    URI_LIST,
    SVG_XML_DATA,
    SVG_DATA,
    PNG_DATA,
    JPEG_DATA,
    IMAGE_DATA,
    APP_X_INKY_COLOR,
    APP_X_COLOR,
    APP_OSWB_COLOR,
    APP_X_INK_PASTE
} ui_drop_target_info;

static GtkTargetEntry ui_drop_target_entries [] = {
    {(gchar *)"text/uri-list",                0, URI_LIST        },
    {(gchar *)"image/svg+xml",                0, SVG_XML_DATA    },
    {(gchar *)"image/svg",                    0, SVG_DATA        },
    {(gchar *)"image/png",                    0, PNG_DATA        },
    {(gchar *)"image/jpeg",                   0, JPEG_DATA       },
#if ENABLE_MAGIC_COLORS
    {(gchar *)"application/x-inkscape-color", 0, APP_X_INKY_COLOR},
#endif // ENABLE_MAGIC_COLORS
    {(gchar *)"application/x-oswb-color",     0, APP_OSWB_COLOR  },
    {(gchar *)"application/x-color",          0, APP_X_COLOR     },
    {(gchar *)"application/x-inkscape-paste", 0, APP_X_INK_PASTE }
};

static GtkTargetEntry *completeDropTargets = 0;
static int completeDropTargetsCount = 0;
static bool temporarily_block_actions = false;

#define ENTRIES_SIZE(n) sizeof(n)/sizeof(n[0])
static guint nui_drop_target_entries = ENTRIES_SIZE(ui_drop_target_entries);
static void sp_ui_import_files(gchar *buffer);
static void sp_ui_import_one_file(char const *filename);
static void sp_ui_import_one_file_with_check(gpointer filename, gpointer unused);
static void sp_ui_drag_data_received(GtkWidget *widget,
                                     GdkDragContext *drag_context,
                                     gint x, gint y,
                                     GtkSelectionData *data,
                                     guint info,
                                     guint event_time,
                                     gpointer user_data);
static void sp_ui_drag_motion( GtkWidget *widget,
                               GdkDragContext *drag_context,
                               gint x, gint y,
                               GtkSelectionData *data,
                               guint info,
                               guint event_time,
                               gpointer user_data );
static void sp_ui_drag_leave( GtkWidget *widget,
                              GdkDragContext *drag_context,
                              guint event_time,
                              gpointer user_data );
static void sp_ui_menu_item_set_name(GtkWidget *data,
                                     Glib::ustring const &name);
static void sp_recent_open(GtkRecentChooser *, gpointer);

static void injectRenamedIcons();

static const int MIN_ONSCREEN_DISTANCE = 50;

void
sp_create_window(SPViewWidget *vw, bool editable)
{
    g_return_if_fail(vw != NULL);
    g_return_if_fail(SP_IS_VIEW_WIDGET(vw));

    Gtk::Window *win = Inkscape::UI::window_new("", TRUE);

    gtk_container_add(GTK_CONTAINER(win->gobj()), GTK_WIDGET(vw));
    gtk_widget_show(GTK_WIDGET(vw));

    if (editable) {
        g_object_set_data(G_OBJECT(vw), "window", win);

        SPDesktopWidget *desktop_widget = reinterpret_cast<SPDesktopWidget*>(vw);
        SPDesktop* desktop = desktop_widget->desktop;

        desktop_widget->window = win;

        win->set_data("desktop", desktop);
        win->set_data("desktopwidget", desktop_widget);

        win->signal_delete_event().connect(sigc::mem_fun(*(SPDesktop*)vw->view, &SPDesktop::onDeleteUI));
        win->signal_window_state_event().connect(sigc::mem_fun(*desktop, &SPDesktop::onWindowStateEvent));
        win->signal_focus_in_event().connect(sigc::mem_fun(*desktop_widget, &SPDesktopWidget::onFocusInEvent));

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint prefs_geometry =
            (2==prefs->getInt("/options/savewindowgeometry/value", 0));
        if (prefs_geometry) {
            gint pw = prefs->getInt("/desktop/geometry/width", -1);
            gint ph = prefs->getInt("/desktop/geometry/height", -1);
            gint px = prefs->getInt("/desktop/geometry/x", -1);
            gint py = prefs->getInt("/desktop/geometry/y", -1);
            gint full = prefs->getBool("/desktop/geometry/fullscreen");
            gint maxed = prefs->getBool("/desktop/geometry/maximized");
            if (pw>0 && ph>0) {
                gint w = MIN(gdk_screen_width(), pw);
                gint h = MIN(gdk_screen_height(), ph);
                gint x = MIN(gdk_screen_width() - MIN_ONSCREEN_DISTANCE, px);
                gint y = MIN(gdk_screen_height() - MIN_ONSCREEN_DISTANCE, py);
                if (w>0 && h>0) {
                    x = MIN(gdk_screen_width() - w, x);
                    y = MIN(gdk_screen_height() - h, y);
                    desktop->setWindowSize(w, h);
                }

                // Only restore xy for the first window so subsequent windows don't overlap exactly
                // with first.  (Maybe rule should be only restore xy if it's different from xy of
                // other desktops?)

                // Empirically it seems that active_desktop==this desktop only the first time a
                // desktop is created.
                SPDesktop *active_desktop = SP_ACTIVE_DESKTOP;
                if (active_desktop == desktop || active_desktop==NULL) {
                    desktop->setWindowPosition(Geom::Point(x, y));
                }
            }
            if (maxed) {
                win->maximize();
            }
            if (full) {
                win->fullscreen();
            }
        }

    } 

    if ( completeDropTargets == 0 || completeDropTargetsCount == 0 )
    {
        std::vector<gchar*> types;

        GSList *list = gdk_pixbuf_get_formats();
        while ( list ) {
            int i = 0;
            GdkPixbufFormat *one = (GdkPixbufFormat*)list->data;
            gchar** typesXX = gdk_pixbuf_format_get_mime_types(one);
            for ( i = 0; typesXX[i]; i++ ) {
                types.push_back(g_strdup(typesXX[i]));
            }
            g_strfreev(typesXX);

            list = g_slist_next(list);
        }
        completeDropTargetsCount = nui_drop_target_entries + types.size();
        completeDropTargets = new GtkTargetEntry[completeDropTargetsCount];
        for ( int i = 0; i < (int)nui_drop_target_entries; i++ ) {
            completeDropTargets[i] = ui_drop_target_entries[i];
        }
        int pos = nui_drop_target_entries;

        for (std::vector<gchar*>::iterator it = types.begin() ; it != types.end() ; ++it) {
            completeDropTargets[pos].target = *it;
            completeDropTargets[pos].flags = 0;
            completeDropTargets[pos].info = IMAGE_DATA;
            pos++;
        }
    }

    gtk_drag_dest_set((GtkWidget*)win->gobj(),
                      GTK_DEST_DEFAULT_ALL,
                      completeDropTargets,
                      completeDropTargetsCount,
                      GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));


    g_signal_connect(G_OBJECT(win->gobj()),
                     "drag_data_received",
                     G_CALLBACK(sp_ui_drag_data_received),
                     NULL);
    g_signal_connect(G_OBJECT(win->gobj()),
                     "drag_motion",
                     G_CALLBACK(sp_ui_drag_motion),
                     NULL);
    g_signal_connect(G_OBJECT(win->gobj()),
                     "drag_leave",
                     G_CALLBACK(sp_ui_drag_leave),
                     NULL);
    win->show();

    // needed because the first ACTIVATE_DESKTOP was sent when there was no window yet
    if ( SP_IS_DESKTOP_WIDGET(vw) ) {
        INKSCAPE.reactivate_desktop(SP_DESKTOP_WIDGET(vw)->desktop);
    }
}

void
sp_ui_new_view()
{
    SPDocument *document;
    SPViewWidget *dtw;

    document = SP_ACTIVE_DOCUMENT;
    if (!document) return;

    dtw = sp_desktop_widget_new(sp_document_namedview(document, NULL));
    g_return_if_fail(dtw != NULL);

    sp_create_window(dtw, TRUE);
    sp_namedview_window_from_document(static_cast<SPDesktop*>(dtw->view));
    sp_namedview_update_layers_from_document(static_cast<SPDesktop*>(dtw->view));
}

void sp_ui_new_view_preview()
{
    SPDocument *document = SP_ACTIVE_DOCUMENT;
    if ( document ) {
        SPViewWidget *dtw = reinterpret_cast<SPViewWidget *>(sp_svg_view_widget_new(document));
        g_return_if_fail(dtw != NULL);
        SP_SVG_VIEW_WIDGET(dtw)->setResize(true, 400.0, 400.0);

        sp_create_window(dtw, FALSE);
    }
}

void
sp_ui_close_view(GtkWidget */*widget*/)
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;

    if (dt == NULL) {
        return;
    }

    if (dt->shutdown()) {
        return; // Shutdown operation has been canceled, so do nothing
    }

    // If closing the last document, open a new document so Inkscape doesn't quit.
    std::list<SPDesktop *> desktops;
    INKSCAPE.get_all_desktops(desktops);
    if (desktops.size() == 1) {
        Glib::ustring templateUri = sp_file_default_template_uri();
        SPDocument *doc = SPDocument::createNewDoc( templateUri.c_str() , TRUE, true );
        // Set viewBox if it doesn't exist
        if (!doc->getRoot()->viewBox_set) {
            doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDisplayUnit()), doc->getHeight().value(doc->getDisplayUnit())));
        }
        dt->change_document(doc);
        sp_namedview_window_from_document(dt);
        sp_namedview_update_layers_from_document(dt);
        return;
    }

    // Shutdown can proceed; use the stored reference to the desktop here instead of the current SP_ACTIVE_DESKTOP,
    // because the user might have changed the focus in the meantime (see bug #381357 on Launchpad)
    dt->destroyWidget();
}


unsigned int
sp_ui_close_all(void)
{
    /* Iterate through all the windows, destroying each in the order they
       become active */
    while (SP_ACTIVE_DESKTOP) {
        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        if (dt->shutdown()) {
            /* The user canceled the operation, so end doing the close */
            return FALSE;
        }
        // Shutdown can proceed; use the stored reference to the desktop here instead of the current SP_ACTIVE_DESKTOP,
        // because the user might have changed the focus in the meantime (see bug #381357 on Launchpad)
        dt->destroyWidget();
    }

    return TRUE;
}

/*
 * Some day when the right-click menus are ready to start working
 * smarter with the verbs, we'll need to change this NULL being
 * sent to sp_action_perform to something useful, or set some kind
 * of global "right-clicked position" variable for actions to
 * investigate when they're called.
 */
static void
sp_ui_menu_activate(void */*object*/, SPAction *action)
{
    if (!temporarily_block_actions) {
        sp_action_perform(action, NULL);
    }
}

static void
sp_ui_menu_select_action(void */*object*/, SPAction *action)
{
    sp_action_get_view(action)->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, action->tip);
}

static void
sp_ui_menu_deselect_action(void */*object*/, SPAction *action)
{
    sp_action_get_view(action)->tipsMessageContext()->clear();
}

static void
sp_ui_menu_select(gpointer object, gpointer tip)
{
    Inkscape::UI::View::View *view = static_cast<Inkscape::UI::View::View*> (g_object_get_data(G_OBJECT(object), "view"));
    view->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, (gchar *)tip);
}

static void
sp_ui_menu_deselect(gpointer object)
{
    Inkscape::UI::View::View *view = static_cast<Inkscape::UI::View::View*>  (g_object_get_data(G_OBJECT(object), "view"));
    view->tipsMessageContext()->clear();
}

/**
 * Creates and attaches a scaled icon to the given menu item.
 */
static void
sp_ui_menuitem_add_icon( GtkWidget *item, gchar *icon_name )
{
    static bool iconsInjected = false;
    if ( !iconsInjected ) {
        iconsInjected = true;
        injectRenamedIcons();
    }
    GtkWidget *icon;

    icon = sp_icon_new( Inkscape::ICON_SIZE_MENU, icon_name );
    gtk_widget_show(icon);
    gtk_image_menu_item_set_image((GtkImageMenuItem *) item, icon);
} // end of sp_ui_menu_add_icon

void
sp_ui_dialog_title_string(Inkscape::Verb *verb, gchar *c)
{
    SPAction     *action;
    unsigned int shortcut;
    gchar        *s;
    gchar        *atitle;

    action = verb->get_action(Inkscape::ActionContext());
    if (!action)
        return;

    atitle = sp_action_get_title(action);

    s = g_stpcpy(c, atitle);

    g_free(atitle);

    shortcut = sp_shortcut_get_primary(verb);
    if (shortcut!=GDK_KEY_VoidSymbol) {
        gchar* key = sp_shortcut_get_label(shortcut);
        s = g_stpcpy(s, " (");
        s = g_stpcpy(s, key);
        g_stpcpy(s, ")");
        g_free(key);
    }
}


/**
 * Appends a custom menu UI from a verb.
 * 
 * @see ContextMenu::AppendItemFromVerb for a c++ified alternative. Consider dropping sp_ui_menu_append_item_from_verb when c++ifying interface.cpp.
 */
static GtkWidget *sp_ui_menu_append_item_from_verb(GtkMenu *menu, Inkscape::Verb *verb, Inkscape::UI::View::View *view, bool radio = false, GSList *group = NULL)
{
    SPAction *action;
    GtkWidget *item;

    if (verb->get_code() == SP_VERB_NONE) {

        item = gtk_separator_menu_item_new();

    } else {

        action = verb->get_action(Inkscape::ActionContext(view));
        if (!action) return NULL;

        if (radio) {
            item = gtk_radio_menu_item_new_with_mnemonic(group, action->name);
        } else {
            item = gtk_image_menu_item_new_with_mnemonic(action->name);
        }

        gtk_label_set_markup_with_mnemonic( GTK_LABEL(gtk_bin_get_child(GTK_BIN (item))), action->name);

        GtkAccelGroup *accel_group =  sp_shortcut_get_accel_group();
        gtk_menu_set_accel_group(menu, accel_group);

        sp_shortcut_add_accelerator(item, sp_shortcut_get_primary(verb));

        action->signal_set_sensitive.connect(
            sigc::bind<0>(
                sigc::ptr_fun(&gtk_widget_set_sensitive),
                item));
        action->signal_set_name.connect(
            sigc::bind<0>(
                sigc::ptr_fun(&sp_ui_menu_item_set_name),
                item));

        if (!action->sensitive) {
            gtk_widget_set_sensitive(item, FALSE);
        }

        if (action->image) {
            sp_ui_menuitem_add_icon(item, action->image);
        }
        gtk_widget_set_events(item, GDK_KEY_PRESS_MASK);
        g_object_set_data(G_OBJECT(item), "view", (gpointer) view);
        g_signal_connect( G_OBJECT(item), "activate", G_CALLBACK(sp_ui_menu_activate), action );
        g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select_action), action );
        g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect_action), action );
    }

    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    return item;

} // end of sp_ui_menu_append_item_from_verb


Glib::ustring getLayoutPrefPath( Inkscape::UI::View::View *view )
{
    Glib::ustring prefPath;

    if (reinterpret_cast<SPDesktop*>(view)->is_focusMode()) {
        prefPath = "/focus/";
    } else if (reinterpret_cast<SPDesktop*>(view)->is_fullscreen()) {
        prefPath = "/fullscreen/";
    } else {
        prefPath = "/window/";
    }

    return prefPath;
}

static void
checkitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data)
{
    gchar const *pref = (gchar const *) user_data;
    Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(menuitem), "view");
    SPAction *action = (SPAction *) g_object_get_data(G_OBJECT(menuitem), "action");

    if (action) {

        sp_ui_menu_activate(menuitem, action);

    } else if (pref) {
        // All check menu items should have actions now, but just in case
        Glib::ustring pref_path = getLayoutPrefPath( view );
        pref_path += pref;
        pref_path += "/state";

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gboolean checked = gtk_check_menu_item_get_active(menuitem);
        prefs->setBool(pref_path, checked);

        reinterpret_cast<SPDesktop*>(view)->layoutWidget();
    }
}

static bool getViewStateFromPref(Inkscape::UI::View::View *view, gchar const *pref)
{
    Glib::ustring pref_path = getLayoutPrefPath( view );
    pref_path += pref;
    pref_path += "/state";

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    return prefs->getBool(pref_path, true);
}

#if GTK_CHECK_VERSION(3,0,0)
static gboolean checkitem_update(GtkWidget *widget, cairo_t * /*cr*/, gpointer user_data)
#else
static gboolean checkitem_update(GtkWidget *widget, GdkEventExpose * /*event*/, gpointer user_data)
#endif
{
    GtkCheckMenuItem *menuitem=GTK_CHECK_MENU_ITEM(widget);

    gchar const *pref = (gchar const *) user_data;
    Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(menuitem), "view");
    SPAction *action = (SPAction *) g_object_get_data(G_OBJECT(menuitem), "action");
    SPDesktop *dt = static_cast<SPDesktop*>(view);

    bool ison = false;
    if (action) {

        if (!strcmp(action->id, "ToggleGrid")) {
            ison = dt->gridsEnabled();
        }
        else if (!strcmp(action->id, "EditGuidesToggleLock")) {
            ison = dt->namedview->lockguides;
        }
        else if (!strcmp(action->id, "ToggleGuides")) {
            ison = dt->namedview->getGuides();
        }
        else if (!strcmp(action->id, "ViewCmsToggle")) {
            ison = dt->colorProfAdjustEnabled();
        }
        else  {
            ison = getViewStateFromPref(view, pref);
        }
    } else if (pref) {
        // The Show/Hide menu items without actions
        ison = getViewStateFromPref(view, pref);
    }

    g_signal_handlers_block_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)checkitem_toggled, user_data);
    gtk_check_menu_item_set_active(menuitem, ison);
    g_signal_handlers_unblock_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)checkitem_toggled, user_data);

    return FALSE;
}


static void taskToggled(GtkCheckMenuItem *menuitem, gpointer userData)
{
    if ( gtk_check_menu_item_get_active(menuitem) ) {
        gint taskNum = GPOINTER_TO_INT(userData);
        taskNum = (taskNum < 0) ? 0 : (taskNum > 2) ? 2 : taskNum;

        Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(menuitem), "view");

        // note: this will change once more options are in the task set support:
        Inkscape::UI::UXManager::getInstance()->setTask( dynamic_cast<SPDesktop*>(view), taskNum );
    }
}


/**
 * Callback function to update the status of the radio buttons in the View -> Display mode menu (Normal, No Filters, Outline) and Color display mode.
 */
#if GTK_CHECK_VERSION(3,0,0)
static gboolean update_view_menu(GtkWidget *widget, cairo_t * /*cr*/, gpointer user_data)
#else
static gboolean update_view_menu(GtkWidget *widget, GdkEventExpose * /*event*/, gpointer user_data)
#endif
{
    SPAction *action = (SPAction *) user_data;
    g_assert(action->id != NULL);

    Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(widget), "view");
    SPDesktop *dt = static_cast<SPDesktop*>(view);
    Inkscape::RenderMode mode = dt->getMode();
    Inkscape::ColorMode colormode = dt->getColorMode();

    bool new_state = false;
    if (!strcmp(action->id, "ViewModeNormal")) {
        new_state = mode == Inkscape::RENDERMODE_NORMAL;
    } else if (!strcmp(action->id, "ViewModeNoFilters")) {
        new_state = mode == Inkscape::RENDERMODE_NO_FILTERS;
    } else if (!strcmp(action->id, "ViewModeOutline")) {
        new_state = mode == Inkscape::RENDERMODE_OUTLINE;
    } else if (!strcmp(action->id, "ViewColorModeNormal")) {
        new_state = colormode == Inkscape::COLORMODE_NORMAL;
    } else if (!strcmp(action->id, "ViewColorModeGrayscale")) {
        new_state = colormode == Inkscape::COLORMODE_GRAYSCALE;
    } else if (!strcmp(action->id, "ViewColorModePrintColorsPreview")) {
        new_state = colormode == Inkscape::COLORMODE_PRINT_COLORS_PREVIEW;
    } else {
        g_warning("update_view_menu does not handle this verb");
    }

    if (new_state) { //only one of the radio buttons has to be activated; the others will automatically be deactivated
        if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
            // When the GtkMenuItem version of the "activate" signal has been emitted by a GtkRadioMenuItem, there is a second
            // emission as the most recently active item is toggled to inactive. This is dealt with before the original signal is handled.
            // This emission however should not invoke any actions, hence we block it here:
            temporarily_block_actions = true;
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
            temporarily_block_actions = false;
        }
    }

    return FALSE;
}

static void
sp_ui_menu_append_check_item_from_verb(GtkMenu *menu, Inkscape::UI::View::View *view, gchar const *label, gchar const *tip, gchar const *pref,
                                       void (*callback_toggle)(GtkCheckMenuItem *, gpointer user_data),
#if GTK_CHECK_VERSION(3,0,0)
                                       gboolean (*callback_update)(GtkWidget *widget, cairo_t *cr, gpointer user_data),
#else
                                       gboolean (*callback_update)(GtkWidget *widget, GdkEventExpose *event, gpointer user_data),
#endif
                                       Inkscape::Verb *verb)
{
    unsigned int shortcut = (verb) ? sp_shortcut_get_primary(verb) : 0;
    SPAction *action = (verb) ? verb->get_action(Inkscape::ActionContext(view)) : 0;
    GtkWidget *item = gtk_check_menu_item_new_with_mnemonic(action ? action->name : label);

#if 0
    if (!action->sensitive) {
        gtk_widget_set_sensitive(item, FALSE);
    }
#endif

    sp_shortcut_add_accelerator(item, shortcut);

    gtk_widget_show(item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    g_object_set_data(G_OBJECT(item), "view", (gpointer) view);
    g_object_set_data(G_OBJECT(item), "action", (gpointer) action);

    g_signal_connect( G_OBJECT(item), "toggled", (GCallback) callback_toggle, (void *) pref);

#if GTK_CHECK_VERSION(3,0,0)
    g_signal_connect( G_OBJECT(item), "draw", (GCallback) callback_update, (void *) pref);
#else
    g_signal_connect( G_OBJECT(item), "expose_event", (GCallback) callback_update, (void *) pref);
#endif

    (*callback_update)(item, NULL, (void *)pref);

    g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select), (gpointer) (action ? action->tip : tip));
    g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect), NULL);

}

static void
sp_recent_open(GtkRecentChooser *recent_menu, gpointer /*user_data*/)
{
    // dealing with the bizarre filename convention in Inkscape for now
    gchar *uri = gtk_recent_chooser_get_current_uri(GTK_RECENT_CHOOSER(recent_menu));
    gchar *local_fn = g_filename_from_uri(uri, NULL, NULL);
    gchar *utf8_fn = g_filename_to_utf8(local_fn, -1, NULL, NULL, NULL);
    sp_file_open(utf8_fn, NULL);
    g_free(utf8_fn);
    g_free(local_fn);
    g_free(uri);
}

static void
sp_ui_checkboxes_menus(GtkMenu *m, Inkscape::UI::View::View *view)
{
    //sp_ui_menu_append_check_item_from_verb(m, view, _("_Menu"), _("Show or hide the menu bar"), "menu",
    //                                       checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "commands",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_COMMANDS_TOOLBAR));
    sp_ui_menu_append_check_item_from_verb(m, view,NULL, NULL, "snaptoolbox",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_SNAP_TOOLBAR));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "toppanel",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_TOOL_TOOLBAR));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "toolbox",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_TOOLBOX));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "rulers",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_RULERS));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "scrollbars",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_SCROLLBARS));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "panels",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_PALETTE));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "statusbar",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_STATUSBAR));
}


static void addTaskMenuItems(GtkMenu *menu, Inkscape::UI::View::View *view)
{
    gchar const* data[] = {
        C_("Interface setup", "Default"), _("Default interface setup"),
        C_("Interface setup", "Custom"), _("Setup for custom task"),
        C_("Interface setup", "Wide"), _("Setup for widescreen work"),
        0, 0
    };

    GSList *group = 0;
    int count = 0;
    gint active = Inkscape::UI::UXManager::getInstance()->getDefaultTask( dynamic_cast<SPDesktop*>(view) );
    for (gchar const **strs = data; strs[0]; strs += 2, count++)
    {
        GtkWidget *item = gtk_radio_menu_item_new_with_label( group, strs[0] );
        group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(item) );
        if ( count == active )
        {
            gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(item), TRUE );
        }

        g_object_set_data( G_OBJECT(item), "view", view );
        g_signal_connect( G_OBJECT(item), "toggled", reinterpret_cast<GCallback>(taskToggled), GINT_TO_POINTER(count) );
        g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select), const_cast<gchar*>(strs[1]) );
        g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect), 0 );

        gtk_widget_show( item );
        gtk_menu_shell_append( GTK_MENU_SHELL(menu), item );
    }
}


/**
 * Observer that updates the recent list's max document count.
 */
class MaxRecentObserver : public Inkscape::Preferences::Observer {
public:
    MaxRecentObserver(GtkWidget *recent_menu) :
        Observer("/options/maxrecentdocuments/value"),
        _rm(recent_menu)
    {}
    virtual void notify(Inkscape::Preferences::Entry const &e) {
        gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(_rm), e.getInt());
        // hack: the recent menu doesn't repopulate after changing the limit, so we force it
        g_signal_emit_by_name((gpointer) gtk_recent_manager_get_default(), "changed");
    }
private:
    GtkWidget *_rm;
};

/**
 * This function turns XML into a menu.
 *
 *  This function is realitively simple as it just goes through the XML
 *  and parses the individual elements.  In the case of a submenu, it
 *  just calls itself recursively.  Because it is only reasonable to have
 *  a couple of submenus, it is unlikely this will go more than two or
 *  three times.
 *
 *  In the case of an unrecognized verb, a menu item is made to identify
 *  the verb that is missing, and display that.  The menu item is also made
 *  insensitive.
 *
 * @param  menus  This is the XML that defines the menu
 * @param  menu   Menu to be added to
 * @param  view   The View that this menu is being built for
 */
static void sp_ui_build_dyn_menus(Inkscape::XML::Node *menus, GtkWidget *menu, Inkscape::UI::View::View *view)
{
    if (menus == NULL) return;
    if (menu == NULL)  return;
    GSList *group = NULL;

    for (Inkscape::XML::Node *menu_pntr = menus;
         menu_pntr != NULL;
         menu_pntr = menu_pntr->next()) {
        if (!strcmp(menu_pntr->name(), "submenu")) {
            GtkWidget *mitem = gtk_menu_item_new_with_mnemonic(_(menu_pntr->attribute("name")));
            GtkWidget *submenu = gtk_menu_new();
            sp_ui_build_dyn_menus(menu_pntr->firstChild(), submenu, view);
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), GTK_WIDGET(submenu));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "verb")) {
            gchar const *verb_name = menu_pntr->attribute("verb-id");
            Inkscape::Verb *verb = Inkscape::Verb::getbyid(verb_name);

            if (verb != NULL) {
                if (menu_pntr->attribute("radio") != NULL) {
                    GtkWidget *item = sp_ui_menu_append_item_from_verb (GTK_MENU(menu), verb, view, true, group);
                    group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(item));
                    if (menu_pntr->attribute("default") != NULL) {
                        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
                    }
                    if (verb->get_code() != SP_VERB_NONE) {
                        SPAction *action = verb->get_action(Inkscape::ActionContext(view));
#if GTK_CHECK_VERSION(3,0,0)
                        g_signal_connect( G_OBJECT(item), "draw", (GCallback) update_view_menu, (void *) action);
#else
                        g_signal_connect( G_OBJECT(item), "expose_event", (GCallback) update_view_menu, (void *) action);
#endif
                    }
                } else if (menu_pntr->attribute("check") != NULL) {
                    if (verb->get_code() != SP_VERB_NONE) {
                        SPAction *action = verb->get_action(Inkscape::ActionContext(view));
                        sp_ui_menu_append_check_item_from_verb(GTK_MENU(menu), view, action->name, action->tip, NULL,
                            checkitem_toggled, checkitem_update, verb);
                    }
                } else {
                    sp_ui_menu_append_item_from_verb(GTK_MENU(menu), verb, view);
                    group = NULL;
                }
            } else {
                gchar string[120];
                g_snprintf(string, 120, _("Verb \"%s\" Unknown"), verb_name);
                string[119] = '\0'; /* may not be terminated */
                GtkWidget *item = gtk_menu_item_new_with_label(string);
                gtk_widget_set_sensitive(item, false);
                gtk_widget_show(item);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
            }
            continue;
        }
        if (!strcmp(menu_pntr->name(), "separator")) {
            GtkWidget *item = gtk_separator_menu_item_new();
            gtk_widget_show(item);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
            continue;
        }
        
        if (!strcmp(menu_pntr->name(), "recent-file-list")) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();

            // create recent files menu
            int max_recent = prefs->getInt("/options/maxrecentdocuments/value");
            GtkWidget *recent_menu = gtk_recent_chooser_menu_new_for_manager(gtk_recent_manager_get_default());
            gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(recent_menu), max_recent);
            // sort most recently used documents first to preserve previous behavior
            gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(recent_menu), GTK_RECENT_SORT_MRU);
            g_signal_connect(G_OBJECT(recent_menu), "item-activated", G_CALLBACK(sp_recent_open), (gpointer) NULL);

            // add filter to only open files added by Inkscape
            GtkRecentFilter *inkscape_only_filter = gtk_recent_filter_new();
            gtk_recent_filter_add_application(inkscape_only_filter, g_get_prgname());
            gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(recent_menu), inkscape_only_filter);

            gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER(recent_menu), TRUE);
            gtk_recent_chooser_set_show_not_found (GTK_RECENT_CHOOSER(recent_menu), FALSE);

            GtkWidget *recent_item = gtk_menu_item_new_with_mnemonic(_("Open _Recent"));
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(recent_item), recent_menu);

            gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(recent_item));
            // this will just sit and update the list's item count
            static MaxRecentObserver *mro = new MaxRecentObserver(recent_menu);
            prefs->addObserver(*mro);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "objects-checkboxes")) {
            sp_ui_checkboxes_menus(GTK_MENU(menu), view);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "task-checkboxes")) {
            addTaskMenuItems(GTK_MENU(menu), view);
            continue;
        }
    }
}

GtkWidget *sp_ui_main_menubar(Inkscape::UI::View::View *view)
{
    GtkWidget *mbar = gtk_menu_bar_new();
    sp_ui_build_dyn_menus(INKSCAPE.get_menus(), mbar, view);
    return mbar;
}


/* Drag and Drop */
void
sp_ui_drag_data_received(GtkWidget *widget,
                         GdkDragContext *drag_context,
                         gint x, gint y,
                         GtkSelectionData *data,
                         guint info,
                         guint /*event_time*/,
                         gpointer /*user_data*/)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    switch (info) {
#if ENABLE_MAGIC_COLORS
        case APP_X_INKY_COLOR:
        {
            int destX = 0;
            int destY = 0;
            gtk_widget_translate_coordinates( widget, &(desktop->canvas->widget), x, y, &destX, &destY );
            Geom::Point where( sp_canvas_window_to_world( desktop->canvas, Geom::Point( destX, destY ) ) );

            SPItem *item = desktop->getItemAtPoint( where, true );
            if ( item )
            {
                bool fillnotstroke = (drag_context->action != GDK_ACTION_MOVE);

                if ( data->length >= 8 ) {
                    cmsHPROFILE srgbProf = cmsCreate_sRGBProfile();

                    gchar c[64] = {0};
                    // Careful about endian issues.
                    guint16* dataVals = (guint16*)data->data;
                    sp_svg_write_color( c, sizeof(c),
                                        SP_RGBA32_U_COMPOSE(
                                            0x0ff & (dataVals[0] >> 8),
                                            0x0ff & (dataVals[1] >> 8),
                                            0x0ff & (dataVals[2] >> 8),
                                            0xff // can't have transparency in the color itself
                                            //0x0ff & (data->data[3] >> 8),
                                            ));
                    SPCSSAttr *css = sp_repr_css_attr_new();
                    bool updatePerformed = false;

                    if ( data->length > 14 ) {
                        int flags = dataVals[4];

                        // piggie-backed palette entry info
                        int index = dataVals[5];
                        Glib::ustring palName;
                        for ( int i = 0; i < dataVals[6]; i++ ) {
                            palName += (gunichar)dataVals[7+i];
                        }

                        // Now hook in a magic tag of some sort.
                        if ( !palName.empty() && (flags & 1) ) {
                            gchar* str = g_strdup_printf("%d|", index);
                            palName.insert( 0, str );
                            g_free(str);
                            str = 0;

                            item->setAttribute( 
                                fillnotstroke ? "inkscape:x-fill-tag":"inkscape:x-stroke-tag",
                                palName.c_str(),
                                false );
                            item->updateRepr();

                            sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", c );
                            updatePerformed = true;
                        }
                    }

                    if ( !updatePerformed ) {
                        sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", c );
                    }

                    sp_desktop_apply_css_recursive( item, css, true );
                    item->updateRepr();

                    SPDocumentUndo::done( doc , SP_VERB_NONE,
                                      _("Drop color"));

                    if ( srgbProf ) {
                        cmsCloseProfile( srgbProf );
                    }
                }
            }
        }
        break;
#endif // ENABLE_MAGIC_COLORS

        case APP_X_COLOR:
        {
            int destX = 0;
            int destY = 0;
            gtk_widget_translate_coordinates( widget, &(desktop->canvas->widget), x, y, &destX, &destY );
            Geom::Point where( sp_canvas_window_to_world( desktop->canvas, Geom::Point( destX, destY ) ) );
            Geom::Point const button_dt(desktop->w2d(where));
            Geom::Point const button_doc(desktop->dt2doc(button_dt));

            if ( gtk_selection_data_get_length (data) == 8 ) {
                gchar colorspec[64] = {0};
                // Careful about endian issues.
                guint16* dataVals = (guint16*)gtk_selection_data_get_data (data);
                sp_svg_write_color( colorspec, sizeof(colorspec),
                                    SP_RGBA32_U_COMPOSE(
                                        0x0ff & (dataVals[0] >> 8),
                                        0x0ff & (dataVals[1] >> 8),
                                        0x0ff & (dataVals[2] >> 8),
                                        0xff // can't have transparency in the color itself
                                        //0x0ff & (data->data[3] >> 8),
                                        ));

                SPItem *item = desktop->getItemAtPoint( where, true );

                bool consumed = false;
                if (desktop->event_context && desktop->event_context->get_drag()) {
                    consumed = desktop->event_context->get_drag()->dropColor(item, colorspec, button_dt);
                    if (consumed) {
                        DocumentUndo::done( doc , SP_VERB_NONE, _("Drop color on gradient") );
                        desktop->event_context->get_drag()->updateDraggers();
                    }
                }

                //if (!consumed && tools_active(desktop, TOOLS_TEXT)) {
                //    consumed = sp_text_context_drop_color(c, button_doc);
                //    if (consumed) {
                //        SPDocumentUndo::done( doc , SP_VERB_NONE, _("Drop color on gradient stop"));
                //    }
                //}

                if (!consumed && item) {
                    bool fillnotstroke = (gdk_drag_context_get_actions (drag_context) != GDK_ACTION_MOVE);
                    if (fillnotstroke &&
                        (SP_IS_SHAPE(item) || SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))) {
                        Path *livarot_path = Path_for_item(item, true, true);
                        livarot_path->ConvertWithBackData(0.04);

                        boost::optional<Path::cut_position> position = get_nearest_position_on_Path(livarot_path, button_doc);
                        if (position) {
                            Geom::Point nearest = get_point_on_Path(livarot_path, position->piece, position->t);
                            Geom::Point delta = nearest - button_doc;
                            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                            delta = desktop->d2w(delta);
                            double stroke_tolerance =
                                ( !item->style->stroke.isNone() ?
                                  desktop->current_zoom() *
                                  item->style->stroke_width.computed *
                                  item->i2dt_affine().descrim() * 0.5
                                  : 0.0)
                                + prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

                            if (Geom::L2 (delta) < stroke_tolerance) {
                                fillnotstroke = false;
                            }
                        }
                        delete livarot_path;
                    }

                    SPCSSAttr *css = sp_repr_css_attr_new();
                    sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", colorspec );

                    sp_desktop_apply_css_recursive( item, css, true );
                    item->updateRepr();

                    DocumentUndo::done( doc , SP_VERB_NONE,
                                        _("Drop color") );
                }
            }
        }
        break;

        case APP_OSWB_COLOR:
        {
            bool worked = false;
            Glib::ustring colorspec;
            if ( gtk_selection_data_get_format (data) == 8 ) {
                ege::PaintDef color;
                worked = color.fromMIMEData("application/x-oswb-color",
                                            reinterpret_cast<char const *>(gtk_selection_data_get_data (data)),
                                            gtk_selection_data_get_length (data),
                                            gtk_selection_data_get_format (data));
                if ( worked ) {
                    if ( color.getType() == ege::PaintDef::CLEAR ) {
                        colorspec = ""; // TODO check if this is sufficient
                    } else if ( color.getType() == ege::PaintDef::NONE ) {
                        colorspec = "none";
                    } else {
                        unsigned int r = color.getR();
                        unsigned int g = color.getG();
                        unsigned int b = color.getB();

                        SPGradient* matches = 0;
                        const GSList *gradients = doc->getResourceList("gradient");
                        for (const GSList *item = gradients; item; item = item->next) {
                            SPGradient* grad = SP_GRADIENT(item->data);
                            if ( color.descr == grad->getId() ) {
                                if ( grad->hasStops() ) {
                                    matches = grad;
                                    break;
                                }
                            }
                        }
                        if (matches) {
                            colorspec = "url(#";
                            colorspec += matches->getId();
                            colorspec += ")";
                        } else {
                            gchar* tmp = g_strdup_printf("#%02x%02x%02x", r, g, b);
                            colorspec = tmp;
                            g_free(tmp);
                        }
                    }
                }
            }
            if ( worked ) {
                int destX = 0;
                int destY = 0;
                gtk_widget_translate_coordinates( widget, &(desktop->canvas->widget), x, y, &destX, &destY );
                Geom::Point where( sp_canvas_window_to_world( desktop->canvas, Geom::Point( destX, destY ) ) );
                Geom::Point const button_dt(desktop->w2d(where));
                Geom::Point const button_doc(desktop->dt2doc(button_dt));

                SPItem *item = desktop->getItemAtPoint( where, true );

                bool consumed = false;
                if (desktop->event_context && desktop->event_context->get_drag()) {
                    consumed = desktop->event_context->get_drag()->dropColor(item, colorspec.c_str(), button_dt);
                    if (consumed) {
                        DocumentUndo::done( doc , SP_VERB_NONE, _("Drop color on gradient") );
                        desktop->event_context->get_drag()->updateDraggers();
                    }
                }

                if (!consumed && item) {
                    bool fillnotstroke = (gdk_drag_context_get_actions (drag_context) != GDK_ACTION_MOVE);
                    if (fillnotstroke &&
                        (SP_IS_SHAPE(item) || SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))) {
                        Path *livarot_path = Path_for_item(item, true, true);
                        livarot_path->ConvertWithBackData(0.04);

                        boost::optional<Path::cut_position> position = get_nearest_position_on_Path(livarot_path, button_doc);
                        if (position) {
                            Geom::Point nearest = get_point_on_Path(livarot_path, position->piece, position->t);
                            Geom::Point delta = nearest - button_doc;
                            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                            delta = desktop->d2w(delta);
                            double stroke_tolerance =
                                ( !item->style->stroke.isNone() ?
                                  desktop->current_zoom() *
                                  item->style->stroke_width.computed *
                                  item->i2dt_affine().descrim() * 0.5
                                  : 0.0)
                                + prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

                            if (Geom::L2 (delta) < stroke_tolerance) {
                                fillnotstroke = false;
                            }
                        }
                        delete livarot_path;
                    }

                    SPCSSAttr *css = sp_repr_css_attr_new();
                    sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", colorspec.c_str() );

                    sp_desktop_apply_css_recursive( item, css, true );
                    item->updateRepr();

                    DocumentUndo::done( doc , SP_VERB_NONE,
                                        _("Drop color") );
                }
            }
        }
        break;

        case SVG_DATA:
        case SVG_XML_DATA: {
            gchar *svgdata = (gchar *)gtk_selection_data_get_data (data);

            Inkscape::XML::Document *rnewdoc = sp_repr_read_mem(svgdata, gtk_selection_data_get_length (data), SP_SVG_NS_URI);

            if (rnewdoc == NULL) {
                sp_ui_error_dialog(_("Could not parse SVG data"));
                return;
            }

            Inkscape::XML::Node *repr = rnewdoc->root();
            gchar const *style = repr->attribute("style");

            Inkscape::XML::Node *newgroup = rnewdoc->createElement("svg:g");
            newgroup->setAttribute("style", style);

            Inkscape::XML::Document * xml_doc =  doc->getReprDoc();
            for (Inkscape::XML::Node *child = repr->firstChild(); child != NULL; child = child->next()) {
                Inkscape::XML::Node *newchild = child->duplicate(xml_doc);
                newgroup->appendChild(newchild);
            }

            Inkscape::GC::release(rnewdoc);

            // Add it to the current layer

            // Greg's edits to add intelligent positioning of svg drops
            SPObject *new_obj = NULL;
            new_obj = desktop->currentLayer()->appendChildRepr(newgroup);

            Inkscape::Selection *selection = desktop->getSelection();
            selection->set(SP_ITEM(new_obj));

            // move to mouse pointer
            {
                desktop->getDocument()->ensureUpToDate();
                Geom::OptRect sel_bbox = selection->visualBounds();
                if (sel_bbox) {
                    Geom::Point m( desktop->point() - sel_bbox->midpoint() );
                    sp_selection_move_relative(selection, m, false);
                }
            }

            Inkscape::GC::release(newgroup);
            DocumentUndo::done( doc, SP_VERB_NONE,
                             _("Drop SVG") );
            break;
        }

        case URI_LIST: {
            gchar *uri = (gchar *)gtk_selection_data_get_data (data);
            sp_ui_import_files(uri);
            break;
        }

        case APP_X_INK_PASTE: {
            Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
            cm->paste(desktop);
            DocumentUndo::done( doc, SP_VERB_NONE, _("Drop Symbol") );
            break;
        }

        case PNG_DATA:
        case JPEG_DATA:
        case IMAGE_DATA: {
            const char *mime = (info == JPEG_DATA ? "image/jpeg" : "image/png");

            Inkscape::Extension::DB::InputList o;
            Inkscape::Extension::db.get_input_list(o);
            Inkscape::Extension::DB::InputList::const_iterator i = o.begin();
            while (i != o.end() && strcmp( (*i)->get_mimetype(), mime ) != 0) {
                ++i;
            }
            Inkscape::Extension::Extension *ext = *i;
            bool save = (strcmp(ext->get_param_optiongroup("link"), "embed") == 0);
            ext->set_param_optiongroup("link", "embed");
            ext->set_gui(false);

            gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-dnd-import", NULL );
            g_file_set_contents(filename, 
                reinterpret_cast<gchar const *>(gtk_selection_data_get_data (data)), 
                gtk_selection_data_get_length (data), 
                NULL);
            file_import(doc, filename, ext);
            g_free(filename);

            ext->set_param_optiongroup("link", save ? "embed" : "link");
            ext->set_gui(true);
            DocumentUndo::done( doc , SP_VERB_NONE,
                                _("Drop bitmap image") );
            break;
        }
    }
}

#include "ui/tools/gradient-tool.h"

void sp_ui_drag_motion( GtkWidget */*widget*/,
                        GdkDragContext */*drag_context*/,
                        gint /*x*/, gint /*y*/,
                        GtkSelectionData */*data*/,
                        guint /*info*/,
                        guint /*event_time*/,
                        gpointer /*user_data*/)
{
//     SPDocument *doc = SP_ACTIVE_DOCUMENT;
//     SPDesktop *desktop = SP_ACTIVE_DESKTOP;


//     g_message("drag-n-drop motion (%4d, %4d)  at %d", x, y, event_time);
}

static void sp_ui_drag_leave( GtkWidget */*widget*/,
                              GdkDragContext */*drag_context*/,
                              guint /*event_time*/,
                              gpointer /*user_data*/ )
{
//     g_message("drag-n-drop leave                at %d", event_time);
}

static void
sp_ui_import_files(gchar *buffer)
{
    GList *list = gnome_uri_list_extract_filenames(buffer);
    if (!list)
        return;
    g_list_foreach(list, sp_ui_import_one_file_with_check, NULL);
    g_list_foreach(list, (GFunc) g_free, NULL);
    g_list_free(list);
}

static void
sp_ui_import_one_file_with_check(gpointer filename, gpointer /*unused*/)
{
    if (filename) {
        if (strlen((char const *)filename) > 2)
            sp_ui_import_one_file((char const *)filename);
    }
}

static void
sp_ui_import_one_file(char const *filename)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!doc) return;

    if (filename == NULL) return;

    // Pass off to common implementation
    // TODO might need to get the proper type of Inkscape::Extension::Extension
    file_import( doc, filename, NULL );
}

void
sp_ui_error_dialog(gchar const *message)
{
    GtkWidget *dlg;
    gchar *safeMsg = Inkscape::IO::sanitizeString(message);

    dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE, "%s", safeMsg);
    sp_transientize(dlg);
    gtk_window_set_resizable(GTK_WINDOW(dlg), FALSE);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    g_free(safeMsg);
}

bool
sp_ui_overwrite_file(gchar const *filename)
{
    bool return_value = FALSE;

    if (Inkscape::IO::file_test(filename, G_FILE_TEST_EXISTS)) {
        Gtk::Window *window = SP_ACTIVE_DESKTOP->getToplevel();
        gchar* baseName = g_path_get_basename( filename );
        gchar* dirName = g_path_get_dirname( filename );
        GtkWidget* dialog = gtk_message_dialog_new_with_markup( window->gobj(),
                                                                (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                                                GTK_MESSAGE_QUESTION,
                                                                GTK_BUTTONS_NONE,
                                                                _( "<span weight=\"bold\" size=\"larger\">A file named \"%s\" already exists. Do you want to replace it?</span>\n\n"
                                                                   "The file already exists in \"%s\". Replacing it will overwrite its contents." ),
                                                                baseName,
                                                                dirName
            );
        gtk_dialog_add_buttons( GTK_DIALOG(dialog),
                                _("_Cancel"), GTK_RESPONSE_NO,
                                _("Replace"), GTK_RESPONSE_YES,
                                NULL );
        gtk_dialog_set_default_response( GTK_DIALOG(dialog), GTK_RESPONSE_YES );

        if ( gtk_dialog_run( GTK_DIALOG(dialog) ) == GTK_RESPONSE_YES ) {
            return_value = TRUE;
        } else {
            return_value = FALSE;
        }
        gtk_widget_destroy(dialog);
        g_free( baseName );
        g_free( dirName );
    } else {
        return_value = TRUE;
    }

    return return_value;
}

static void
sp_ui_menu_item_set_name(GtkWidget *data, Glib::ustring const &name)
{
    if (data || GTK_IS_BIN (data)) {
        void *child = gtk_bin_get_child (GTK_BIN (data));
        //child is either
        //- a GtkBox, whose first child is a label displaying name if the menu
        //item has an accel key
        //- a GtkLabel if the menu has no accel key
        if (child != NULL){
            if (GTK_IS_LABEL(child)) {
                gtk_label_set_markup_with_mnemonic(GTK_LABEL (child), name.c_str());
            } else if (GTK_IS_BOX(child)) {
                gtk_label_set_markup_with_mnemonic(
                GTK_LABEL (gtk_container_get_children(GTK_CONTAINER (child))->data),
                name.c_str());
            }//else sp_ui_menu_append_item_from_verb has been modified and can set
            //a menu item in yet another way...
        }
    }
}

void injectRenamedIcons()
{
    Glib::RefPtr<Gtk::IconTheme> iconTheme = Gtk::IconTheme::get_default();

    std::vector< std::pair<Glib::ustring, Glib::ustring> > renamed;
    renamed.push_back(std::make_pair("gtk-file", "document-x-generic"));
    renamed.push_back(std::make_pair("gtk-directory", "folder"));

    for ( std::vector< std::pair<Glib::ustring, Glib::ustring> >::iterator it = renamed.begin(); it < renamed.end(); ++it ) {
        bool hasIcon = iconTheme->has_icon(it->first);
        bool hasSecondIcon = iconTheme->has_icon(it->second);

        if ( !hasIcon && hasSecondIcon ) {
            Glib::ArrayHandle<int> sizes = iconTheme->get_icon_sizes(it->second);
            for ( Glib::ArrayHandle<int>::iterator it2 = sizes.begin(); it2 < sizes.end(); ++it2 ) {
                Glib::RefPtr<Gdk::Pixbuf> pb = iconTheme->load_icon( it->second, *it2 );
                if ( pb ) {
                    // install a private copy of the pixbuf to avoid pinning a theme
                    Glib::RefPtr<Gdk::Pixbuf> pbCopy = pb->copy();
                    Gtk::IconTheme::add_builtin_icon( it->first, *it2, pbCopy );
                }
            }
        }
    }
}


ContextMenu::ContextMenu(SPDesktop *desktop, SPItem *item) :
    _item(item),
    MIGroup(),
    MIParent(_("Go to parent"))
{
// g_message("ContextMenu");
    _object = static_cast<SPObject *>(item);
    _desktop = desktop;

    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_UNDO));
    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_REDO));
    AddSeparator();
    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_CUT));
    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_COPY));
    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_PASTE));
    AddSeparator();
    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_DUPLICATE));
    AppendItemFromVerb(Inkscape::Verb::get(SP_VERB_EDIT_DELETE));
    
    positionOfLastDialog = 10; // 9 in front + 1 for the separator in the next if; used to position the dialog menu entries below each other

    /* Item menu */
    if (item!=NULL) {
        AddSeparator();
        MakeObjectMenu();
    }
    
    /* layer menu */
    SPGroup *group=NULL;
    if (item) {
        if (SP_IS_GROUP(item)) {
            group = SP_GROUP(item);
        } else if ( item != _desktop->currentRoot() && SP_IS_GROUP(item->parent) ) {
            group = SP_GROUP(item->parent);
        }
    }

    if (( group && group != _desktop->currentLayer() ) ||
        ( _desktop->currentLayer() != _desktop->currentRoot() && _desktop->currentLayer()->parent != _desktop->currentRoot() ) ) {
        AddSeparator();
    }

    if ( group && group != _desktop->currentLayer() ) {
        /* TRANSLATORS: #%1 is the id of the group e.g. <g id="#g7">, not a number. */
        MIGroup.set_label (Glib::ustring::compose(_("Enter group #%1"), group->getId()));
        MIGroup.set_data("group", group);
        MIGroup.signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ContextMenu::EnterGroup),&MIGroup));
        MIGroup.show();
        append(MIGroup);
    }

    if ( _desktop->currentLayer() != _desktop->currentRoot() ) {
        if ( _desktop->currentLayer()->parent != _desktop->currentRoot() ) {
            MIParent.signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::LeaveGroup));
            MIParent.show();
            append(MIParent);
        }
    }
}

ContextMenu::~ContextMenu(void)
{
}

Gtk::SeparatorMenuItem* ContextMenu::AddSeparator(void)
{
    Gtk::SeparatorMenuItem* sep = Gtk::manage(new Gtk::SeparatorMenuItem());
    sep->show();
    append(*sep);
    return sep;
}

void ContextMenu::EnterGroup(Gtk::MenuItem* mi)
{
    _desktop->setCurrentLayer(reinterpret_cast<SPObject *>(mi->get_data("group")));
    _desktop->selection->clear();
}

void ContextMenu::LeaveGroup(void)
{
    _desktop->setCurrentLayer(_desktop->currentLayer()->parent);
}

void ContextMenu::AppendItemFromVerb(Inkscape::Verb *verb)//, SPDesktop *view)//, bool radio, GSList *group)
{
    SPAction *action;
    SPDesktop *view = _desktop;
    
    if (verb->get_code() == SP_VERB_NONE) {
        Gtk::MenuItem *item = AddSeparator();
        item->show();
        append(*item);
    } else {
        action = verb->get_action(Inkscape::ActionContext(view));
        if (!action) {
            return;
        }
        
        Gtk::ImageMenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(action->name, true));

        sp_shortcut_add_accelerator(GTK_WIDGET(item->gobj()), sp_shortcut_get_primary(verb));

        action->signal_set_sensitive.connect(sigc::mem_fun(*this, &ContextMenu::set_sensitive));
        action->signal_set_name.connect(sigc::mem_fun(*item, &ContextMenu::set_name));

        if (!action->sensitive) {
            item->set_sensitive(FALSE);
        }

        if (action->image) {
            sp_ui_menuitem_add_icon((GtkWidget*)item->gobj(), action->image);
        }
        item->set_events(Gdk::KEY_PRESS_MASK);
        item->signal_activate().connect(sigc::bind(sigc::ptr_fun(sp_ui_menu_activate),item,action));
        item->signal_select().connect(sigc::bind(sigc::ptr_fun(sp_ui_menu_select_action),item,action));
        item->signal_deselect().connect(sigc::bind(sigc::ptr_fun(sp_ui_menu_deselect_action),item,action));
        item->show();
        append(*item);
    }
}

void ContextMenu::MakeObjectMenu(void)
{
//    GObjectClass *klass = G_OBJECT_GET_CLASS(_object); //to deduce the object's type from its class
//
//    if (G_TYPE_CHECK_CLASS_TYPE(klass, SP_TYPE_ITEM))
//    {
//        MakeItemMenu ();
//    }
//    if (G_TYPE_CHECK_CLASS_TYPE(klass, SP_TYPE_GROUP))
//    {
//        MakeGroupMenu();
//    }
//    if (G_TYPE_CHECK_CLASS_TYPE(klass, SP_TYPE_ANCHOR))
//    {
//        MakeAnchorMenu();
//    }
//    if (G_TYPE_CHECK_CLASS_TYPE(klass, SP_TYPE_IMAGE))
//    {
//        MakeImageMenu();
//    }
//    if (G_TYPE_CHECK_CLASS_TYPE(klass, SP_TYPE_SHAPE))
//    {
//        MakeShapeMenu();
//    }
//    if (G_TYPE_CHECK_CLASS_TYPE(klass, SP_TYPE_TEXT))
//    {
//        MakeTextMenu();
//    }

	if (SP_IS_ITEM(_object)) {
		MakeItemMenu();
	}

	if (SP_IS_GROUP(_object)) {
		MakeGroupMenu();
	}

	if (SP_IS_ANCHOR(_object)) {
		MakeAnchorMenu();
	}

	if (SP_IS_IMAGE(_object)) {
		MakeImageMenu();
	}

	if (SP_IS_SHAPE(_object)) {
		MakeShapeMenu();
	}

	if (SP_IS_TEXT(_object)) {
		MakeTextMenu();
	}
}

void ContextMenu::MakeItemMenu (void)
{
    Gtk::MenuItem* mi;

    /* Item dialog */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Object Properties..."),1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ItemProperties));
    mi->show();
    append(*mi);//insert(*mi,positionOfLastDialog++);

    AddSeparator();
    
    /* Select item */
    if (Inkscape::Verb::getbyid( "org.inkscape.followlink" )) {
        mi = Gtk::manage(new Gtk::MenuItem(_("_Select This"), 1));
        if (_desktop->selection->includes(_item)) {
            mi->set_sensitive(FALSE);
        } else {
            mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ItemSelectThis));
        }
        mi->show();
        append(*mi);
    }


    mi = Gtk::manage(new Gtk::MenuItem(_("Select Same")));
    mi->show();
    Gtk::Menu *select_same_submenu = Gtk::manage(new Gtk::Menu());
    if (_desktop->selection->isEmpty()) {
        mi->set_sensitive(FALSE);
    }
    mi->set_submenu(*select_same_submenu);
    append(*mi);

    /* Select same fill and stroke */
    mi = Gtk::manage(new Gtk::MenuItem(_("Fill and Stroke"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SelectSameFillStroke));
    mi->set_sensitive(!SP_IS_ANCHOR(_item));
    mi->show();
    select_same_submenu->append(*mi);

    /* Select same fill color */
    mi = Gtk::manage(new Gtk::MenuItem(_("Fill Color"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SelectSameFillColor));
    mi->set_sensitive(!SP_IS_ANCHOR(_item));
    mi->show();
    select_same_submenu->append(*mi);

    /* Select same stroke color */
    mi = Gtk::manage(new Gtk::MenuItem(_("Stroke Color"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SelectSameStrokeColor));
    mi->set_sensitive(!SP_IS_ANCHOR(_item));
    mi->show();
    select_same_submenu->append(*mi);

    /* Select same stroke style */
    mi = Gtk::manage(new Gtk::MenuItem(_("Stroke Style"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SelectSameStrokeStyle));
    mi->set_sensitive(!SP_IS_ANCHOR(_item));
    mi->show();
    select_same_submenu->append(*mi);

    /* Select same stroke style */
    mi = Gtk::manage(new Gtk::MenuItem(_("Object type"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SelectSameObjectType));
    mi->set_sensitive(!SP_IS_ANCHOR(_item));
    mi->show();
    select_same_submenu->append(*mi);

    /* Move to layer */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Move to layer ..."), 1));
    if (_desktop->selection->isEmpty()) {
        mi->set_sensitive(FALSE);
    } else {
        mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ItemMoveTo));
    }
    mi->show();
    append(*mi);

    /* Create link */
    mi = Gtk::manage(new Gtk::MenuItem(_("Create _Link"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ItemCreateLink));
    mi->set_sensitive(!SP_IS_ANCHOR(_item));
    mi->show();
    append(*mi);
    
    bool ClipRefOK=false;
    bool MaskRefOK=false;
    if (_item){
        if (_item->clip_ref){
            if (_item->clip_ref->getObject()){
                ClipRefOK=true;
            }
        }
    }    
    if (_item){
        if (_item->mask_ref){
            if (_item->mask_ref->getObject()){
                MaskRefOK=true;
            }
        }
    }    
    /* Set mask */
    mi = Gtk::manage(new Gtk::MenuItem(_("Set Mask"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SetMask));
    if (ClipRefOK || MaskRefOK) {
        mi->set_sensitive(FALSE);
    } else {
        mi->set_sensitive(TRUE);
    }
    mi->show();
    append(*mi);
    
    /* Release mask */
    mi = Gtk::manage(new Gtk::MenuItem(_("Release Mask"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ReleaseMask));
    if (MaskRefOK) {
        mi->set_sensitive(TRUE);
    } else {
        mi->set_sensitive(FALSE);
    }
    mi->show();
    append(*mi);

    /*SSet Clip Group */
    mi = Gtk::manage(new Gtk::MenuItem(_("Create Clip G_roup"),1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::CreateGroupClip));
    mi->set_sensitive(TRUE);
    mi->show();
    append(*mi);
    
    /* Set Clip */
    mi = Gtk::manage(new Gtk::MenuItem(_("Set Cl_ip"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SetClip));
    if (ClipRefOK || MaskRefOK) {
        mi->set_sensitive(FALSE);
    } else {
        mi->set_sensitive(TRUE);
    }
    mi->show();
    append(*mi);
    
    /* Release Clip */
    mi = Gtk::manage(new Gtk::MenuItem(_("Release C_lip"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ReleaseClip));
    if (ClipRefOK) {
        mi->set_sensitive(TRUE);
    } else {
        mi->set_sensitive(FALSE);
    }
    mi->show();
    append(*mi);

    /* Group */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Group"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ActivateGroup));
    if (_desktop->selection->isEmpty() || _desktop->selection->single()) {
        mi->set_sensitive(FALSE);
    } else {
        mi->set_sensitive(TRUE);
    }
    mi->show();
    append(*mi);
}

void ContextMenu::SelectSameFillStroke(void)
{
    sp_select_same_fill_stroke_style(_desktop, true, true, true);
}

void ContextMenu::SelectSameFillColor(void)
{
    sp_select_same_fill_stroke_style(_desktop, true, false, false);
}

void ContextMenu::SelectSameStrokeColor(void)
{
    sp_select_same_fill_stroke_style(_desktop, false, true, false);
}

void ContextMenu::SelectSameStrokeStyle(void)
{
    sp_select_same_fill_stroke_style(_desktop, false, false, true);
}

void ContextMenu::SelectSameObjectType(void)
{
    sp_select_same_object_type(_desktop);
}

void ContextMenu::ItemProperties(void)
{
    _desktop->selection->set(_item);
    _desktop->_dlg_mgr->showDialog("ObjectProperties");
}

void ContextMenu::ItemSelectThis(void)
{
    _desktop->selection->set(_item);
}

void ContextMenu::ItemMoveTo(void)
{
    Inkscape::UI::Dialogs::LayerPropertiesDialog::showMove(_desktop, _desktop->currentLayer());
}



void ContextMenu::ItemCreateLink(void)
{
    Inkscape::XML::Document *xml_doc = _desktop->doc()->getReprDoc();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:a");
    _item->parent->getRepr()->addChild(repr, _item->getRepr());
    SPObject *object = _item->document->getObjectByRepr(repr);
    g_return_if_fail(SP_IS_ANCHOR(object));

    const char *id = _item->getRepr()->attribute("id");
    Inkscape::XML::Node *child = _item->getRepr()->duplicate(xml_doc);
    _item->deleteObject(false);
    repr->addChild(child, NULL);
    child->setAttribute("id", id);

    Inkscape::GC::release(repr);
    Inkscape::GC::release(child);

    DocumentUndo::done(object->document, SP_VERB_NONE, _("Create link"));

    _desktop->selection->set(SP_ITEM(object));
    _desktop->_dlg_mgr->showDialog("ObjectAttributes");
}

void ContextMenu::SetMask(void)
{
    sp_selection_set_mask(_desktop, false, false);
}

void ContextMenu::ReleaseMask(void)
{
    sp_selection_unset_mask(_desktop, false);
}

void ContextMenu::CreateGroupClip(void)
{
    sp_selection_set_clipgroup(_desktop);
}

void ContextMenu::SetClip(void)
{
    sp_selection_set_mask(_desktop, true, false);
}


void ContextMenu::ReleaseClip(void)
{
    sp_selection_unset_mask(_desktop, true);
}

void ContextMenu::MakeGroupMenu(void)
{
    /* Ungroup */
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::MenuItem(_("_Ungroup"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ActivateUngroup));
    mi->show();
    append(*mi);
}

void ContextMenu::ActivateGroup(void)
{
    sp_selection_group(_desktop->selection, _desktop);
}

void ContextMenu::ActivateUngroup(void)
{
	std::vector<SPItem*> children;

    sp_item_group_ungroup(static_cast<SPGroup*>(_item), children);
    _desktop->selection->setList(children);
}

void ContextMenu::MakeAnchorMenu(void)
{
    Gtk::MenuItem* mi;
    
    /* Link dialog */
    mi = Gtk::manage(new Gtk::MenuItem(_("Link _Properties..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::AnchorLinkProperties));
    mi->show();
    insert(*mi,positionOfLastDialog++);
    
    /* Select item */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Follow Link"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::AnchorLinkFollow));
    mi->show();
    append(*mi);
    
    /* Reset transformations */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Remove Link"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::AnchorLinkRemove));
    mi->show();
    append(*mi);
}

void ContextMenu::AnchorLinkProperties(void)
{
    _desktop->_dlg_mgr->showDialog("ObjectAttributes");
}

void ContextMenu::AnchorLinkFollow(void)
{

    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }
    // Opening the selected links with a python extension
    Inkscape::Verb *verb = Inkscape::Verb::getbyid( "org.inkscape.followlink" );
    if (verb) {
        SPAction *action = verb->get_action(Inkscape::ActionContext(_desktop));
        if (action) {
            sp_action_perform(action, NULL);
        }
    }
}

void ContextMenu::AnchorLinkRemove(void)
{
	std::vector<SPItem*> children;
    sp_item_group_ungroup(static_cast<SPAnchor*>(_item), children, false);
    DocumentUndo::done(_desktop->doc(), SP_VERB_NONE, _("Remove link"));
}

void ContextMenu::MakeImageMenu (void)
{
    Gtk::MenuItem* mi;
    Inkscape::XML::Node *ir = _object->getRepr();
    const gchar *href = ir->attribute("xlink:href");

    /* Image properties */
    mi = Gtk::manage(new Gtk::MenuItem(_("Image _Properties..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ImageProperties));
    mi->show();
    insert(*mi,positionOfLastDialog++);

    /* Edit externally */
    mi = Gtk::manage(new Gtk::MenuItem(_("Edit Externally..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ImageEdit));
    mi->show();
    insert(*mi,positionOfLastDialog++);
    if ( (!href) || ((strncmp(href, "data:", 5) == 0)) ) {
        mi->set_sensitive( FALSE );
    }

    /* Trace Bitmap */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Trace Bitmap..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ImageTraceBitmap));
    mi->show();
    insert(*mi,positionOfLastDialog++);
    if (_desktop->selection->isEmpty()) {
        mi->set_sensitive(FALSE);
    }

    /* Trace Pixel Art */
    mi = Gtk::manage(new Gtk::MenuItem(_("Trace Pixel Art"), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ImageTracePixelArt));
    mi->show();
    insert(*mi,positionOfLastDialog++);
    if (_desktop->selection->isEmpty()) {
        mi->set_sensitive(FALSE);
    }

    /* Embed image */
    if (Inkscape::Verb::getbyid( "org.ekips.filter.embedselectedimages" )) {
        mi = Gtk::manage(new Gtk::MenuItem(C_("Context menu", "Embed Image")));
        mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ImageEmbed));
        mi->show();
        insert(*mi,positionOfLastDialog++);
        if ( (!href) || ((strncmp(href, "data:", 5) == 0)) ) {
            mi->set_sensitive( FALSE );
        }
    }

    /* Extract image */
    if (Inkscape::Verb::getbyid( "org.ekips.filter.extractimage" )) {
        mi = Gtk::manage(new Gtk::MenuItem(C_("Context menu", "Extract Image...")));
        mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::ImageExtract));
        mi->show();
        insert(*mi,positionOfLastDialog++);
        if ( (!href) || ((strncmp(href, "data:", 5) != 0)) ) {
            mi->set_sensitive( FALSE );
        }
    }
}

void ContextMenu::ImageProperties(void)
{
    _desktop->_dlg_mgr->showDialog("ObjectAttributes");
}

Glib::ustring ContextMenu::getImageEditorName() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring value;
    Glib::ustring choices = prefs->getString("/options/bitmapeditor/value");
    if (!choices.empty()) {
        value = choices;
    }
    else {
        value = "gimp";
    }
    return value;
}

void ContextMenu::ImageEdit(void)
{
    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }

    GError* errThing = 0;
    Glib::ustring cmdline = getImageEditorName();
    Glib::ustring name;
    Glib::ustring fullname;

#ifdef WIN32
    // g_spawn_command_line_sync parsing is done according to Unix shell rules,
    // not Windows command interpreter rules. Thus we need to enclose the
    // executable path with single quotes.
    int index = cmdline.find(".exe");
    if ( index < 0 ) index = cmdline.find(".bat");
    if ( index < 0 ) index = cmdline.find(".com");
    if ( index >= 0 ) {
        Glib::ustring editorBin = cmdline.substr(0, index + 4).c_str();
        Glib::ustring args = cmdline.substr(index + 4, cmdline.length()).c_str();
        editorBin.insert(0, "'");
        editorBin.append("'");
        cmdline = editorBin;
        cmdline.append(args);
    } else {
        // Enclose the whole command line if no executable path can be extracted.
        cmdline.insert(0, "'");
        cmdline.append("'");
    }
#endif

    std::vector<SPItem*> itemlist=_desktop->selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        Inkscape::XML::Node *ir = (*i)->getRepr();
        const gchar *href = ir->attribute("xlink:href");
        
        if (strncmp (href,"file:",5) == 0) {
        // URI to filename conversion
          name = g_filename_from_uri(href, NULL, NULL);
        } else {
          name.append(href);
        }

        if (Glib::path_is_absolute(name)) {
            fullname = name;
        } else if (SP_ACTIVE_DOCUMENT->getBase()) {
            fullname = Glib::build_filename(SP_ACTIVE_DOCUMENT->getBase(), name);
        } else {
            fullname = Glib::build_filename(Glib::get_current_dir(), name);
        }

        cmdline.append(" '");
        cmdline.append(fullname.c_str());
        cmdline.append("'");
    }

    //g_warning("##Command line: %s\n", cmdline.c_str());

    g_spawn_command_line_async(cmdline.c_str(), &errThing); 

    if ( errThing ) {
        g_warning("Problem launching editor (%d). %s", errThing->code, errThing->message);
        (_desktop->messageStack())->flash(Inkscape::ERROR_MESSAGE, errThing->message);
        g_error_free(errThing);
        errThing = 0;
    }
}

void ContextMenu::ImageTraceBitmap(void)
{
    INKSCAPE.dialogs_unhide();
    _desktop->_dlg_mgr->showDialog("Trace");
}

void ContextMenu::ImageTracePixelArt(void)
{
    INKSCAPE.dialogs_unhide();
    _desktop->_dlg_mgr->showDialog("PixelArt");
}

void ContextMenu::ImageEmbed(void)
{
    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }

    Inkscape::Verb *verb = Inkscape::Verb::getbyid( "org.ekips.filter.embedselectedimages" );
    if (verb) {
        SPAction *action = verb->get_action(Inkscape::ActionContext(_desktop));
        if (action) {
            sp_action_perform(action, NULL);
        }
    }
}

void ContextMenu::ImageExtract(void)
{
    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }

    Inkscape::Verb *verb = Inkscape::Verb::getbyid( "org.ekips.filter.extractimage" );
    if (verb) {
        SPAction *action = verb->get_action(Inkscape::ActionContext(_desktop));
        if (action) {
            sp_action_perform(action, NULL);
        }
    }
}

void ContextMenu::MakeShapeMenu (void)
{
    Gtk::MenuItem* mi;
    
    /* Item dialog */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Fill and Stroke..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::FillSettings));
    mi->show();
    insert(*mi,positionOfLastDialog++);
}

void ContextMenu::FillSettings(void)
{
    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }

    _desktop->_dlg_mgr->showDialog("FillAndStroke");
}

void ContextMenu::MakeTextMenu (void)
{
    Gtk::MenuItem* mi;

    /* Fill and Stroke dialog */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Fill and Stroke..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::FillSettings));
    mi->show();
    insert(*mi,positionOfLastDialog++);
    
    /* Edit Text dialog */
    mi = Gtk::manage(new Gtk::MenuItem(_("_Text and Font..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::TextSettings));
    mi->show();
    insert(*mi,positionOfLastDialog++);

    /* Spellcheck dialog */
    mi = Gtk::manage(new Gtk::MenuItem(_("Check Spellin_g..."), 1));
    mi->signal_activate().connect(sigc::mem_fun(*this, &ContextMenu::SpellcheckSettings));
    mi->show();
    insert(*mi,positionOfLastDialog++);
}

void ContextMenu::TextSettings (void)
{
    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }

    _desktop->_dlg_mgr->showDialog("TextFont");
}

void ContextMenu::SpellcheckSettings (void)
{
    if (_desktop->selection->isEmpty()) {
        _desktop->selection->set(_item);
    }

    _desktop->_dlg_mgr->showDialog("SpellCheck");
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

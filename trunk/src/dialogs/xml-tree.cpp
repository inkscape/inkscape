/** @file
 * @brief XML editor
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   David Turner
 *
 * Copyright (C) 1999-2006 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include "helper/window.h"
#include "macros.h"
#include "../inkscape.h"
#include "../document.h"
#include "../desktop-handles.h"
#include "desktop.h"
#include "../selection.h"
#include "../sp-string.h"
#include "../sp-tspan.h"
#include "../sp-root.h"
#include "../event-context.h"
#include "in-dt-coordsys.h"


#include "../widgets/sp-xmlview-tree.h"
#include "../widgets/sp-xmlview-content.h"
#include "../widgets/sp-xmlview-attr-list.h"

#include "../inkscape-stock.h"
#include "widgets/icon.h"

#include "dialog-events.h"
#include "../preferences.h"
#include "../verbs.h"
#include "../interface.h"

#include "shortcuts.h"
#include <gdk/gdkkeysyms.h>

#include "message-stack.h"
#include "message-context.h"

#define MIN_ONSCREEN_DISTANCE 50

struct EditableDest {
    GtkEditable *editable;
    gchar *text;
};

static GtkWidget *dlg = NULL;
static sigc::connection sel_changed_connection;
static sigc::connection document_uri_set_connection;
static sigc::connection document_replaced_connection;
static win_data wd;
// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static Glib::ustring const prefs_path = "/dialogs/xml/";
static GtkWidget *status = NULL;
static Inkscape::MessageStack *_message_stack = NULL;
static Inkscape::MessageContext *_message_context = NULL;
static sigc::connection _message_changed_connection;

static GtkTooltips *tooltips = NULL;
static GtkEditable *attr_name = NULL;
static GtkTextView *attr_value = NULL;
static SPXMLViewTree *tree = NULL;
static SPXMLViewAttrList *attributes = NULL;
static SPXMLViewContent *content = NULL;

static gint blocked = 0;
static SPDesktop *current_desktop = NULL;
static SPDocument *current_document = NULL;
static gint selected_attr = 0;
static Inkscape::XML::Node *selected_repr = NULL;

static void sp_xmltree_desktop_activate( Inkscape::Application *inkscape,  SPDesktop *desktop, GtkWidget *dialog );
static void sp_xmltree_desktop_deactivate( Inkscape::Application *inkscape,  SPDesktop *desktop, GtkWidget *dialog );

static void set_tree_desktop(SPDesktop *desktop);
static void set_tree_document(SPDocument *document);
static void set_tree_repr(Inkscape::XML::Node *repr);

static void set_tree_select(Inkscape::XML::Node *repr);
static void propagate_tree_select(Inkscape::XML::Node *repr);

static Inkscape::XML::Node *get_dt_select();
static void set_dt_select(Inkscape::XML::Node *repr);

static void on_tree_select_row(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_unselect_row(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void after_tree_move(GtkCTree *tree, GtkCTreeNode *node, GtkCTreeNode *new_parent, GtkCTreeNode *new_sibling, gpointer data);
static void on_destroy(GtkObject *object, gpointer data);
static gboolean on_delete(GtkObject *object, GdkEvent *event, gpointer data);

static void on_tree_select_row_enable_if_element(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_enable_if_mutable(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_show_if_element(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_show_if_text(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_enable_if_indentable(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_enable_if_not_first_child(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_enable_if_not_last_child(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_select_row_enable_if_has_grandparent(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);

static void on_tree_unselect_row_clear_text(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_unselect_row_disable(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);
static void on_tree_unselect_row_hide(GtkCTree *tree, GtkCTreeNode *node, gint column, gpointer data);

static void on_attr_select_row(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_unselect_row(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_row_changed( GtkCList *list, gint row, gpointer data );

static void on_attr_select_row_enable(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_unselect_row_disable(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);

static void on_attr_select_row_set_name_content(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_select_row_set_value_content(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);
static void on_attr_unselect_row_clear_text(GtkCList *list, gint row, gint column, GdkEventButton *event, gpointer data);

static void on_editable_changed_enable_if_valid_xml_name(GtkEditable *editable, gpointer data);

static void on_desktop_selection_changed(Inkscape::Selection *selection);
static void on_document_replaced(SPDesktop *dt, SPDocument *document);
static void on_document_uri_set(gchar const *uri, SPDocument *document);

static void on_clicked_get_editable_text(GtkWidget *widget, gpointer data);

static void _set_status_message(Inkscape::MessageType type, const gchar *message, GtkWidget *dialog);

static void cmd_new_element_node(GtkObject *object, gpointer data);
static void cmd_new_text_node(GtkObject *object, gpointer data);
static void cmd_duplicate_node(GtkObject *object, gpointer data);
static void cmd_delete_node(GtkObject *object, gpointer data);

static void cmd_raise_node(GtkObject *object, gpointer data);
static void cmd_lower_node(GtkObject *object, gpointer data);
static void cmd_indent_node(GtkObject *object, gpointer data);
static void cmd_unindent_node(GtkObject *object, gpointer data);

static void cmd_delete_attr(GtkObject *object, gpointer data);
static void cmd_set_attr(GtkObject *object, gpointer data);

static gboolean sp_xml_tree_key_press(GtkWidget *widget, GdkEventKey *event);


/*
 * \brief Sets the XML status bar when the tree is selected.
 */
void tree_reset_context()
{
    _message_context->set(Inkscape::NORMAL_MESSAGE,
                          _("<b>Click</b> to select nodes, <b>drag</b> to rearrange."));
}


/*
 * \brief Sets the XML status bar, depending on which attr is selected.
 */
void attr_reset_context(gint attr)
{
    if (attr == 0) {
        _message_context->set(Inkscape::NORMAL_MESSAGE,
                              _("<b>Click</b> attribute to edit."));
    }
    else {
        const gchar *name = g_quark_to_string(attr);
        gchar *message = g_strdup_printf(_("Attribute <b>%s</b> selected. Press <b>Ctrl+Enter</b> when done editing to commit changes."), name);
        _message_context->set(Inkscape::NORMAL_MESSAGE, message);
        g_free(message);
    }
}


void sp_xml_tree_dialog()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (!desktop) {
        return;
    }

    if (dlg == NULL)
    { // very long block

        GtkWidget *box, *sw, *paned, *toolbar, *button;
        GtkWidget *text_container, *attr_container, *attr_subpaned_container, *box2;
        GtkWidget *set_attr;

        tooltips = gtk_tooltips_new();
        gtk_tooltips_enable(tooltips);

        dlg = sp_window_new("", TRUE);
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", -1000);
            y = prefs->getInt(prefs_path + "y", -1000);
        }
        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

//        if (x<0) x=0;
//        if (y<0) y=0;

        if (w && h) {
            gtk_window_resize((GtkWindow *) dlg, w, h);
        }
        if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE))) {
            gtk_window_move((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }

        sp_transientize(dlg);
        wd.win = dlg;
        wd.stop = 0;
        g_signal_connect  ( G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK(sp_transientize_callback), &wd );

        gtk_signal_connect( GTK_OBJECT(dlg), "event", GTK_SIGNAL_FUNC(sp_dialog_event_handler), dlg );

        gtk_signal_connect( GTK_OBJECT(dlg), "destroy", G_CALLBACK(on_destroy), dlg);
        gtk_signal_connect( GTK_OBJECT(dlg), "delete_event", G_CALLBACK(on_delete), dlg);
        g_signal_connect  ( G_OBJECT(INKSCAPE), "shut_down", G_CALLBACK(on_delete), dlg);

        g_signal_connect  ( G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(sp_dialog_hide), dlg);
        g_signal_connect  ( G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(sp_dialog_unhide), dlg);


        gtk_container_set_border_width(GTK_CONTAINER(dlg), 0);
        gtk_window_set_default_size(GTK_WINDOW(dlg), 640, 384);

        GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(dlg), vbox);

        GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

        status = gtk_label_new(NULL);
        gtk_misc_set_alignment(GTK_MISC(status), 0.0, 0.5);
        gtk_widget_set_size_request(status, 1, -1);
        gtk_label_set_markup(GTK_LABEL(status), "");
        gtk_box_pack_start(GTK_BOX(hbox), gtk_hbox_new(FALSE, 0), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(hbox), status, TRUE, TRUE, 0);

        paned = gtk_hpaned_new();
        gtk_paned_set_position(GTK_PANED(paned), 256);
        gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);

        _message_stack = new Inkscape::MessageStack();
        _message_context = new Inkscape::MessageContext(_message_stack);
        _message_changed_connection = _message_stack->connectChanged(
            sigc::bind(sigc::ptr_fun(_set_status_message), dlg)
        );

        /* tree view */

        box = gtk_vbox_new(FALSE, 0);
        gtk_paned_pack1(GTK_PANED(paned), box, FALSE, FALSE);

        tree = SP_XMLVIEW_TREE(sp_xmlview_tree_new(NULL, NULL, NULL));
        gtk_tooltips_set_tip( tooltips, GTK_WIDGET(tree),
                               _("Drag to reorder nodes"), NULL );

        g_signal_connect( G_OBJECT(tree), "tree_select_row",
                           G_CALLBACK(on_tree_select_row), NULL );

        g_signal_connect( G_OBJECT(tree), "tree_unselect_row",
                           G_CALLBACK(on_tree_unselect_row), NULL );

        g_signal_connect_after( G_OBJECT(tree), "tree_move",
                                 G_CALLBACK(after_tree_move), NULL);

        /* TODO: replace gtk_signal_connect_while_alive() with something
         * else...
         */
        toolbar = gtk_toolbar_new();
        gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
        gtk_container_set_border_width(GTK_CONTAINER(toolbar), 0);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                NULL,
                _("New element node"),
                NULL,
                sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                                    INKSCAPE_STOCK_ADD_XML_ELEMENT_NODE ),
                G_CALLBACK(cmd_new_element_node),
                NULL);

        gtk_signal_connect_while_alive( GTK_OBJECT(tree),
                        "tree_select_row",
                        G_CALLBACK(on_tree_select_row_enable_if_element),
                        button,
                        GTK_OBJECT(button));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree),
                        "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button,
                        GTK_OBJECT(button));

        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                NULL, _("New text node"), NULL,
                sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                             INKSCAPE_STOCK_ADD_XML_TEXT_NODE ),
                G_CALLBACK(cmd_new_text_node),
                NULL);

        gtk_signal_connect_while_alive(GTK_OBJECT(tree),
                        "tree_select_row",
                        G_CALLBACK(on_tree_select_row_enable_if_element),
                        button,
                        GTK_OBJECT(button));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree),
                        "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button,
                        GTK_OBJECT(button));

        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                NULL, _("Duplicate node"), NULL,
                sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                             INKSCAPE_STOCK_DUPLICATE_XML_NODE ),
                G_CALLBACK(cmd_duplicate_node),
                NULL);

        gtk_signal_connect_while_alive(GTK_OBJECT(tree),
                        "tree_select_row",
                        G_CALLBACK(on_tree_select_row_enable_if_mutable),
                        button,
                        GTK_OBJECT(button));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button, GTK_OBJECT(button));

        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                NULL, _("Delete node"), NULL,
                sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                             INKSCAPE_STOCK_DELETE_XML_NODE ),
                                           G_CALLBACK(cmd_delete_node), NULL );

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                        G_CALLBACK(on_tree_select_row_enable_if_mutable),
                        button, GTK_OBJECT(button));
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button, GTK_OBJECT(button));
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

        button = gtk_toolbar_append_item( GTK_TOOLBAR(toolbar), "<",
                        _("Unindent node"), NULL,
                        gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_IN),
                        G_CALLBACK(cmd_unindent_node), NULL);

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                    G_CALLBACK(on_tree_select_row_enable_if_has_grandparent),
                    button, GTK_OBJECT(button));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button, GTK_OBJECT(button));

        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), ">",
                        _("Indent node"), NULL,
                        gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_IN),
                        G_CALLBACK(cmd_indent_node), NULL);
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                        G_CALLBACK(on_tree_select_row_enable_if_indentable),
                        button, GTK_OBJECT(button));
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                       (GCallback) on_tree_unselect_row_disable,
                        button, GTK_OBJECT(button));
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), "^",
                        _("Raise node"), NULL,
                        gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_IN),
                        G_CALLBACK(cmd_raise_node), NULL);
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                    G_CALLBACK(on_tree_select_row_enable_if_not_first_child),
                    button, GTK_OBJECT(button));
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button, GTK_OBJECT(button));
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), "v",
                        _("Lower node"), NULL,
                        gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_IN),
                        G_CALLBACK(cmd_lower_node), NULL);
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                        G_CALLBACK(on_tree_select_row_enable_if_not_last_child),
                        button, GTK_OBJECT(button));
        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                        G_CALLBACK(on_tree_unselect_row_disable),
                        button, GTK_OBJECT(button));
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, TRUE, 0);

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(sw),
                                         GTK_POLICY_AUTOMATIC,
                                         GTK_POLICY_AUTOMATIC );
        gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);

        gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(tree));

        /* node view */

        box = gtk_vbox_new(FALSE, 0);
        gtk_paned_pack2(GTK_PANED(paned), box, TRUE, TRUE);

        /* attributes */

        attr_container = gtk_vbox_new(FALSE, 0);
        gtk_box_pack_start( GTK_BOX(box), GTK_WIDGET(attr_container),
                             TRUE, TRUE, 0 );

        attributes = SP_XMLVIEW_ATTR_LIST(sp_xmlview_attr_list_new(NULL));
        g_signal_connect( G_OBJECT(attributes), "select_row",
                           G_CALLBACK(on_attr_select_row), NULL);
        g_signal_connect( G_OBJECT(attributes), "unselect_row",
                           G_CALLBACK(on_attr_unselect_row), NULL);
        g_signal_connect( G_OBJECT(attributes), "row-value-changed",
                           G_CALLBACK(on_attr_row_changed), NULL);

        toolbar = gtk_toolbar_new();
        gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
        gtk_container_set_border_width(GTK_CONTAINER(toolbar), 0);

        button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                NULL, _("Delete attribute"), NULL,
                sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                             INKSCAPE_STOCK_DELETE_XML_ATTRIBUTE ),
               (GCallback) cmd_delete_attr, NULL);

        gtk_signal_connect_while_alive(GTK_OBJECT(attributes), "select_row",
                       (GCallback) on_attr_select_row_enable, button,
                        GTK_OBJECT(button));

        gtk_signal_connect_while_alive(GTK_OBJECT(attributes), "unselect_row",
                       (GCallback) on_attr_unselect_row_disable, button,
                        GTK_OBJECT(button));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                       (GCallback) on_tree_unselect_row_disable, button,
                        GTK_OBJECT(button));

        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        gtk_box_pack_start( GTK_BOX(attr_container),
                             GTK_WIDGET(toolbar), FALSE, TRUE, 0 );

        attr_subpaned_container = gtk_vpaned_new();
        gtk_box_pack_start( GTK_BOX(attr_container),
                             GTK_WIDGET(attr_subpaned_container),
                             TRUE, TRUE, 0 );
        gtk_widget_show(attr_subpaned_container);

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_paned_pack1( GTK_PANED(attr_subpaned_container),
                          GTK_WIDGET(sw), TRUE, TRUE );
        gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(attributes));

        toolbar = gtk_vbox_new(FALSE, 4);
        gtk_container_set_border_width(GTK_CONTAINER(toolbar), 4);

        box2 = gtk_hbox_new(FALSE, 4);
        gtk_box_pack_start( GTK_BOX(toolbar), GTK_WIDGET(box2),
                             FALSE, TRUE, 0);

        attr_name = GTK_EDITABLE(gtk_entry_new());
        gtk_tooltips_set_tip( tooltips, GTK_WIDGET(attr_name),
                               // TRANSLATORS: "Attribute" is a noun here
                               _("Attribute name"), NULL );

        gtk_signal_connect( GTK_OBJECT(attributes), "select_row",
                            (GCallback) on_attr_select_row_set_name_content,
                             attr_name);

        gtk_signal_connect( GTK_OBJECT(attributes), "unselect_row",
                            (GCallback) on_attr_unselect_row_clear_text,
                             attr_name);

        gtk_signal_connect( GTK_OBJECT(tree), "tree_unselect_row",
                            (GCallback) on_tree_unselect_row_clear_text,
                             attr_name);

        gtk_box_pack_start( GTK_BOX(box2), GTK_WIDGET(attr_name),
                             TRUE, TRUE, 0);

        set_attr = gtk_button_new();
        gtk_tooltips_set_tip( tooltips, GTK_WIDGET(set_attr),
                               // TRANSLATORS: "Set" is a verb here
                               _("Set attribute"), NULL );
        // TRANSLATORS: "Set" is a verb here
        GtkWidget *set_label = gtk_label_new(_("Set"));
        gtk_container_add(GTK_CONTAINER(set_attr), set_label);

        gtk_signal_connect( GTK_OBJECT(set_attr), "clicked",
                            (GCallback) cmd_set_attr, NULL);
        gtk_signal_connect( GTK_OBJECT(attr_name), "changed",
                   (GCallback) on_editable_changed_enable_if_valid_xml_name,
                    set_attr );
        gtk_widget_set_sensitive(GTK_WIDGET(set_attr), FALSE);

        gtk_box_pack_start(GTK_BOX(box2), set_attr, FALSE, FALSE, 0);

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(sw),
                                         GTK_POLICY_AUTOMATIC,
                                         GTK_POLICY_AUTOMATIC );
        gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN );
        gtk_box_pack_start(GTK_BOX(toolbar), sw, TRUE, TRUE, 0);

        attr_value =(GtkTextView *) gtk_text_view_new();
        gtk_text_view_set_wrap_mode((GtkTextView *) attr_value, GTK_WRAP_CHAR);
        gtk_tooltips_set_tip( tooltips, GTK_WIDGET(attr_value),
                               // TRANSLATORS: "Attribute" is a noun here
                               _("Attribute value"), NULL );
        gtk_signal_connect( GTK_OBJECT(attributes), "select_row",
                            (GCallback) on_attr_select_row_set_value_content,
                             attr_value );
        gtk_signal_connect( GTK_OBJECT(attributes), "unselect_row",
                            (GCallback) on_attr_unselect_row_clear_text,
                             attr_value );
        gtk_signal_connect( GTK_OBJECT(tree), "tree_unselect_row",
                            (GCallback) on_tree_unselect_row_clear_text,
                             attr_value );
        gtk_text_view_set_editable(attr_value, TRUE);
        gtk_container_add( GTK_CONTAINER(sw),
                            GTK_WIDGET(attr_value) );

        gtk_paned_pack2( GTK_PANED(attr_subpaned_container),
                          GTK_WIDGET(toolbar), FALSE, TRUE );

        /* text */

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(sw),
                                         GTK_POLICY_AUTOMATIC,
                                         GTK_POLICY_AUTOMATIC );
        gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN );
        gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(sw), TRUE, TRUE, 0);

        content = SP_XMLVIEW_CONTENT(sp_xmlview_content_new(NULL));
        gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(content));

        text_container = sw;

        /* initial show/hide */

        gtk_widget_show_all(GTK_WIDGET(dlg));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                       (GCallback) on_tree_select_row_show_if_element,
                        attr_container, GTK_OBJECT(attr_container));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                       (GCallback) on_tree_unselect_row_hide,
                        attr_container, GTK_OBJECT(attr_container));

        gtk_widget_hide(attr_container);

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_select_row",
                       (GCallback) on_tree_select_row_show_if_text,
                        text_container, GTK_OBJECT(text_container));

        gtk_signal_connect_while_alive(GTK_OBJECT(tree), "tree_unselect_row",
                       (GCallback) on_tree_unselect_row_hide,
                        text_container, GTK_OBJECT(text_container));

        gtk_widget_hide(text_container);

        g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop",
                           G_CALLBACK(sp_xmltree_desktop_activate), dlg);

        g_signal_connect( G_OBJECT(INKSCAPE), "deactivate_desktop",
                           G_CALLBACK(sp_xmltree_desktop_deactivate), dlg);

        g_signal_connect((GObject *) dlg, "key_press_event", (GCallback) sp_xml_tree_key_press, NULL);

        tree_reset_context();
    } // end of if (dlg == NULL)

    gtk_window_present((GtkWindow *) dlg);

    g_assert(desktop != NULL);
    set_tree_desktop(desktop);

} // end of sp_xml_tree_dialog()

static gboolean sp_xml_tree_key_press(GtkWidget */*widget*/, GdkEventKey *event)
{

    unsigned int shortcut = get_group0_keyval(event) |
        ( event->state & GDK_SHIFT_MASK ?
          SP_SHORTCUT_SHIFT_MASK : 0 ) |
        ( event->state & GDK_CONTROL_MASK ?
          SP_SHORTCUT_CONTROL_MASK : 0 ) |
        ( event->state & GDK_MOD1_MASK ?
          SP_SHORTCUT_ALT_MASK : 0 );

    /* fixme: if you need to add more xml-tree-specific callbacks, you should probably upgrade
     * the sp_shortcut mechanism to take into account windows. */
    if (shortcut == (SP_SHORTCUT_CONTROL_MASK | GDK_Return)) {
        cmd_set_attr(NULL, NULL);
        return true;
    }
    return false;
}


static void sp_xmltree_desktop_activate(Inkscape::Application */*inkscape*/,
                                      SPDesktop *desktop,
                                      GtkWidget */*dialog*/ )
{
    set_tree_desktop(desktop);
}

static void sp_xmltree_desktop_deactivate(Inkscape::Application */*inkscape*/,
                                      SPDesktop */*desktop*/,
                                      GtkWidget */*dialog*/ )
{
    set_tree_desktop(NULL);
}


void set_tree_desktop(SPDesktop *desktop)
{
    if ( desktop == current_desktop ) {
        return;
    }

    if (current_desktop) {
        sel_changed_connection.disconnect();
        document_replaced_connection.disconnect();
    }
    current_desktop = desktop;
    if (desktop) {
        sel_changed_connection = sp_desktop_selection(desktop)->connectChanged(&on_desktop_selection_changed);
        document_replaced_connection = desktop->connectDocumentReplaced(&on_document_replaced);
        set_tree_document(sp_desktop_document(desktop));
    } else {
        set_tree_document(NULL);
    }

} // end of set_tree_desktop()



void set_tree_document(SPDocument *document)
{
    if (document == current_document) {
        return;
    }

    if (current_document) {
        document_uri_set_connection.disconnect();
    }
    current_document = document;
    if (current_document) {

        document_uri_set_connection = current_document->connectURISet(sigc::bind(sigc::ptr_fun(&on_document_uri_set), current_document));
        on_document_uri_set(SP_DOCUMENT_URI(current_document), current_document);
        set_tree_repr(sp_document_repr_root(current_document));

    } else {
        set_tree_repr(NULL);
    }
}



void set_tree_repr(Inkscape::XML::Node *repr)
{
    if (repr == selected_repr) {
        return;
    }

    gtk_clist_freeze(GTK_CLIST(tree));

    sp_xmlview_tree_set_repr(tree, repr);

    if (repr) {
        set_tree_select(get_dt_select());
    } else {
        set_tree_select(NULL);
    }

    gtk_clist_thaw(GTK_CLIST(tree));

    propagate_tree_select(selected_repr);

}



void set_tree_select(Inkscape::XML::Node *repr)
{
    if (selected_repr) {
        Inkscape::GC::release(selected_repr);
    }

    selected_repr = repr;
    if (repr) {
        GtkCTreeNode *node;

        Inkscape::GC::anchor(selected_repr);

        node = sp_xmlview_tree_get_repr_node(SP_XMLVIEW_TREE(tree), repr);
        if (node) {
            GtkCTreeNode *parent;

            gtk_ctree_select(GTK_CTREE(tree), node);

            parent = GTK_CTREE_ROW(node)->parent;
            while (parent) {
                gtk_ctree_expand(GTK_CTREE(tree), parent);
                parent = GTK_CTREE_ROW(parent)->parent;
            }

            gtk_ctree_node_moveto(GTK_CTREE(tree), node, 0, 0.66, 0.0);
        }
    } else {
        gtk_clist_unselect_all(GTK_CLIST(tree));
    }
    propagate_tree_select(repr);
}



void propagate_tree_select(Inkscape::XML::Node *repr)
{
    if (repr && repr->type() == Inkscape::XML::ELEMENT_NODE) {
        sp_xmlview_attr_list_set_repr(attributes, repr);
    } else {
        sp_xmlview_attr_list_set_repr(attributes, NULL);
    }

    if (repr && ( repr->type() == Inkscape::XML::TEXT_NODE || repr->type() == Inkscape::XML::COMMENT_NODE || repr->type() == Inkscape::XML::PI_NODE ) ) {
        sp_xmlview_content_set_repr(content, repr);
    } else {
        sp_xmlview_content_set_repr(content, NULL);
    }
}


Inkscape::XML::Node *get_dt_select()
{
    if (!current_desktop) {
        return NULL;
    }

    return sp_desktop_selection(current_desktop)->singleRepr();
}



void set_dt_select(Inkscape::XML::Node *repr)
{
    if (!current_desktop) {
        return;
    }

    Inkscape::Selection *selection = sp_desktop_selection(current_desktop);

    SPObject *object;
    if (repr) {
        while ( ( repr->type() != Inkscape::XML::ELEMENT_NODE )
                && sp_repr_parent(repr) )
        {
            repr = sp_repr_parent(repr);
        } // end of while loop

        object = sp_desktop_document(current_desktop)->getObjectByRepr(repr);
    } else {
        object = NULL;
    }

    blocked++;
    if ( object && in_dt_coordsys(*object)
         && !(SP_IS_STRING(object) ||
                SP_IS_ROOT(object)     ) )
    {
            /* We cannot set selection to root or string - they are not items and selection is not
             * equipped to deal with them */
            selection->set(SP_ITEM(object));
    }
    blocked--;

} // end of set_dt_select()


void on_tree_select_row(GtkCTree *tree,
                        GtkCTreeNode *node,
                        gint /*column*/,
                        gpointer /*data*/)
{
    if (blocked) {
        return;
    }

    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);
    g_assert(repr != NULL);

    if (selected_repr == repr) {
        return;
    }

    if (selected_repr) {
        Inkscape::GC::release(selected_repr);
        selected_repr = NULL;
    }
    selected_repr = repr;
    Inkscape::GC::anchor(selected_repr);

    propagate_tree_select(selected_repr);

    set_dt_select(selected_repr);

    tree_reset_context();
}

void on_tree_unselect_row(GtkCTree *tree,
                          GtkCTreeNode *node,
                          gint /*column*/,
                          gpointer /*data*/)
{
    if (blocked) {
        return;
    }

    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);
    propagate_tree_select(NULL);
    set_dt_select(NULL);

    if (selected_repr && (selected_repr == repr)) {
        Inkscape::GC::release(selected_repr);
        selected_repr = NULL;
        selected_attr = 0;
    }
}



void after_tree_move(GtkCTree */*tree*/,
                     GtkCTreeNode *node,
                     GtkCTreeNode *new_parent,
                     GtkCTreeNode *new_sibling,
                     gpointer /*data*/)
{
    if (GTK_CTREE_ROW(node)->parent  == new_parent &&
        GTK_CTREE_ROW(node)->sibling == new_sibling)
    {
        sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                         _("Drag XML subtree"));
    } else {
        sp_document_cancel(current_document);
    }
}


static void on_destroy(GtkObject */*object*/, gpointer /*data*/)
{
    set_tree_desktop(NULL);
    gtk_object_destroy(GTK_OBJECT(tooltips));
    tooltips = NULL;
    sp_signal_disconnect_by_data(INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;

    _message_changed_connection.disconnect();
    delete _message_context;
    _message_context = NULL;
    Inkscape::GC::release(_message_stack);
    _message_stack = NULL;
    _message_changed_connection.~connection();

    status = NULL;
}



static gboolean on_delete(GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/)
{
    gtk_window_get_position((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it
}


static void _set_status_message(Inkscape::MessageType /*type*/, const gchar *message, GtkWidget */*dialog*/)
{
    if (status) {
        gtk_label_set_markup(GTK_LABEL(status), message ? message : "");
    }
}


void on_tree_select_row_enable(GtkCTree */*tree*/,
                               GtkCTreeNode */*node*/,
                               gint /*column*/,
                               gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
}



void on_tree_select_row_enable_if_element(GtkCTree *tree,
                                          GtkCTreeNode *node,
                                          gint /*column*/,
                                          gpointer data )
{
    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);

    if (repr->type() == Inkscape::XML::ELEMENT_NODE) {
        gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
    }
}



void on_tree_select_row_show_if_element(GtkCTree *tree, GtkCTreeNode *node,
                                        gint /*column*/, gpointer data)
{
    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);

    if (repr->type() == Inkscape::XML::ELEMENT_NODE) {
        gtk_widget_show(GTK_WIDGET(data));
    } else {
        gtk_widget_hide(GTK_WIDGET(data));
    }
}



void on_tree_select_row_show_if_text(GtkCTree *tree, GtkCTreeNode *node,
                                     gint /*column*/, gpointer data)
{
    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);

    if ( repr->type() == Inkscape::XML::TEXT_NODE || repr->type() == Inkscape::XML::COMMENT_NODE || repr->type() == Inkscape::XML::PI_NODE ) {
        gtk_widget_show(GTK_WIDGET(data));
    } else {
        gtk_widget_hide(GTK_WIDGET(data));
    }
}


gboolean xml_tree_node_mutable(GtkCTreeNode *node)
{
    // top-level is immutable, obviously
    if (!GTK_CTREE_ROW(node)->parent) {
        return false;
    }

    // if not in base level (where namedview, defs, etc go), we're mutable
    if (GTK_CTREE_ROW(GTK_CTREE_ROW(node)->parent)->parent) {
        return true;
    }

    Inkscape::XML::Node *repr;
    repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);
    g_assert(repr);

    // don't let "defs" or "namedview" disappear
    if ( !strcmp(repr->name(),"svg:defs") ||
         !strcmp(repr->name(),"sodipodi:namedview") ) {
        return false;
    }

    // everyone else is okay, I guess.  :)
    return true;
}


void on_tree_select_row_enable_if_mutable(GtkCTree */*tree*/, GtkCTreeNode *node,
                                          gint /*column*/, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data), xml_tree_node_mutable(node));
}



void on_tree_unselect_row_disable(GtkCTree */*tree*/, GtkCTreeNode */*node*/,
                                  gint /*column*/, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
}



void on_tree_unselect_row_hide(GtkCTree */*tree*/, GtkCTreeNode */*node*/,
                               gint /*column*/, gpointer data)
{
    gtk_widget_hide(GTK_WIDGET(data));
}



void on_tree_unselect_row_clear_text(GtkCTree */*tree*/, GtkCTreeNode */*node*/,
                                     gint /*column*/, gpointer data)
{
    if (GTK_IS_EDITABLE(data)) {
        gtk_editable_delete_text(GTK_EDITABLE(data), 0, -1);
    } else if (GTK_IS_TEXT_VIEW(data)) {
        GtkTextBuffer *tb;
        tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
        gtk_text_buffer_set_text(tb, "", 0);
    }
}


void on_attr_select_row(GtkCList *list, gint row, gint /*column*/,
                        GdkEventButton */*event*/, gpointer /*data*/)
{
    selected_attr = sp_xmlview_attr_list_get_row_key(list, row);
    gtk_window_set_focus(GTK_WINDOW(dlg), GTK_WIDGET(attr_value));

    attr_reset_context(selected_attr);
}


void on_attr_unselect_row(GtkCList */*list*/, gint /*row*/, gint /*column*/,
                          GdkEventButton */*event*/, gpointer /*data*/)
{
    selected_attr = 0;
    attr_reset_context(selected_attr);
}


void on_attr_row_changed(GtkCList *list, gint row, gpointer /*data*/)
{
    gint attr = sp_xmlview_attr_list_get_row_key(list, row);

    if (attr == selected_attr) {
        /* if the attr changed, reselect the row in the list to sync
           the edit box */

        /*
        // get current attr values
        const gchar * name = g_quark_to_string (sp_xmlview_attr_list_get_row_key (list, row));
        const gchar * value = selected_repr->attribute(name);

        g_warning("value: '%s'",value);

        // get the edit box value
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds ( gtk_text_view_get_buffer (attr_value),
                                     &start, &end );
        gchar * text = gtk_text_buffer_get_text ( gtk_text_view_get_buffer (attr_value),
                                       &start, &end, TRUE );
        g_warning("text: '%s'",text);

        // compare to edit box
        if (strcmp(text,value)) {
            // issue warning if they're different
            _message_stack->flash(Inkscape::WARNING_MESSAGE,
                                  _("Attribute changed in GUI while editing values!"));
        }
        g_free (text);

        */
        gtk_clist_unselect_row( GTK_CLIST(list), row, 0 );
        gtk_clist_select_row( GTK_CLIST(list), row, 0 );
    }
}


void on_attr_select_row_set_name_content(GtkCList *list, gint row,
                                         gint /*column*/, GdkEventButton */*event*/,
                                         gpointer data)
{
    GtkEditable *editable = GTK_EDITABLE(data);
    const gchar *name = g_quark_to_string(sp_xmlview_attr_list_get_row_key(list, row));
    gtk_editable_delete_text(editable, 0, -1);
    gint pos = 0;
    gtk_editable_insert_text(editable, name, strlen(name), &pos);
}



void on_attr_select_row_set_value_content(GtkCList *list, gint row, gint /*column*/,
                                          GdkEventButton */*event*/,
                                          gpointer data)
{
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    const gchar *name = g_quark_to_string(sp_xmlview_attr_list_get_row_key(list, row));
    const gchar *value = selected_repr->attribute(name);
    if (!value) {
        value = "";
    }
    gtk_text_buffer_set_text(tb, value, strlen(value));
}


void on_tree_select_row_enable_if_indentable(GtkCTree *tree, GtkCTreeNode *node,
                                             gint /*column*/, gpointer data)
{
    gboolean indentable = FALSE;

    if (xml_tree_node_mutable(node)) {
        Inkscape::XML::Node *repr, *prev;
        repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);

        Inkscape::XML::Node *parent=repr->parent();
        if ( parent && repr != parent->firstChild() ) {
            g_assert(parent->firstChild());

            // skip to the child just before the current repr
            for ( prev = parent->firstChild() ;
                  prev && prev->next() != repr ;
                  prev = prev->next() ){};

            if (prev && prev->type() == Inkscape::XML::ELEMENT_NODE) {
                indentable = TRUE;
            }
        }
    }

    gtk_widget_set_sensitive(GTK_WIDGET(data), indentable);
}



void on_tree_select_row_enable_if_not_first_child(GtkCTree *tree,
                                                  GtkCTreeNode *node,
                                                  gint /*column*/,
                                                  gpointer data)
{
    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);

    Inkscape::XML::Node *parent=repr->parent();
    if ( parent && repr != parent->firstChild() ) {
        gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
    }
}



void on_tree_select_row_enable_if_not_last_child(GtkCTree *tree,
                                                 GtkCTreeNode *node,
                                                 gint /*column*/, gpointer data)
{
    Inkscape::XML::Node *repr = sp_xmlview_tree_node_get_repr(SP_XMLVIEW_TREE(tree), node);

    Inkscape::XML::Node *parent=repr->parent();
    if ( parent && parent->parent() && repr->next() ) {
        gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
    }
}



void on_tree_select_row_enable_if_has_grandparent(GtkCTree */*tree*/,
                                                  GtkCTreeNode *node,
                                                  gint /*column*/, gpointer data)
{
    GtkCTreeNode *parent = GTK_CTREE_ROW(node)->parent;

    if (parent) {
        GtkCTreeNode *grandparent = GTK_CTREE_ROW(parent)->parent;
        if (grandparent) {
            gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
        }
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
    }
}



void on_attr_select_row_enable(GtkCList */*list*/, gint /*row*/, gint /*column*/,
                               GdkEventButton */*event*/, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
}



void on_attr_unselect_row_disable(GtkCList */*list*/, gint /*row*/, gint /*column*/,
                                  GdkEventButton */*event*/, gpointer data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
}



void on_attr_unselect_row_clear_text(GtkCList */*list*/, gint /*row*/, gint /*column*/,
                                     GdkEventButton */*event*/, gpointer data)
{
    if (GTK_IS_EDITABLE(data)) {
        gtk_editable_delete_text(GTK_EDITABLE(data), 0, -1);
    } else if (GTK_IS_TEXT_VIEW(data)) {
        GtkTextBuffer *tb;
        tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
        gtk_text_buffer_set_text(tb, "", 0);
    }
}



void on_editable_changed_enable_if_valid_xml_name(GtkEditable *editable,
                                                  gpointer data)
{
    gchar *text = gtk_editable_get_chars(editable, 0, -1);

    /* TODO: need to do checking a little more rigorous than this */

    if (strlen(text)) {
        gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
    }
    g_free(text);
}



void on_desktop_selection_changed(Inkscape::Selection */*selection*/)
{
    if (!blocked++) {
        set_tree_select(get_dt_select());
    }
    blocked--;
}

static void on_document_replaced(SPDesktop *dt, SPDocument *doc)
{
    if (current_desktop)
        sel_changed_connection.disconnect();

    sel_changed_connection = sp_desktop_selection(dt)->connectChanged(&on_desktop_selection_changed);
    set_tree_document(doc);
}

void on_document_uri_set(gchar const */*uri*/, SPDocument *document)
{
    gchar title[500];
    sp_ui_dialog_title_string(Inkscape::Verb::get(SP_VERB_DIALOG_XML_EDITOR), title);
    gchar *t = g_strdup_printf("%s: %s", SP_DOCUMENT_NAME(document), title);
    gtk_window_set_title(GTK_WINDOW(dlg), t);
    g_free(t);
}



void on_clicked_get_editable_text(GtkWidget */*widget*/, gpointer data)
{
    EditableDest *dest = (EditableDest *) data;
    dest->text = gtk_editable_get_chars(dest->editable, 0, -1);
}

gboolean
quit_on_esc (GtkWidget *w, GdkEventKey *event, GObject */*tbl*/)
{
    switch (get_group0_keyval (event)) {
        case GDK_Escape: // defocus
            gtk_widget_destroy(w);
            return TRUE;
    }
    return FALSE;
}

void cmd_new_element_node(GtkObject */*object*/, gpointer /*data*/)
{
    EditableDest name;
    GtkWidget *window, *create, *cancel, *vbox, *entry, *bbox, *sep;

    g_assert(selected_repr != NULL);

    window = sp_window_new(NULL, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 4);
    gtk_window_set_title(GTK_WINDOW(window), _("New element node..."));
    gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, TRUE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(dlg));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_signal_connect(GTK_OBJECT(window), "destroy", gtk_main_quit, NULL);
    gtk_signal_connect(GTK_OBJECT(window), "key-press-event", G_CALLBACK(quit_on_esc), window);

    vbox = gtk_vbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);

    sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, TRUE, 0);

    bbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(bbox), 4);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);

    cancel = gtk_button_new_with_label(_("Cancel"));
    GTK_WIDGET_SET_FLAGS( GTK_WIDGET(cancel),
                           GTK_CAN_DEFAULT );
    gtk_signal_connect_object( GTK_OBJECT(cancel), "clicked",
                                G_CALLBACK(gtk_widget_destroy),
                                GTK_OBJECT(window) );
    gtk_container_add(GTK_CONTAINER(bbox), cancel);

    create = gtk_button_new_with_label(_("Create"));
    gtk_widget_set_sensitive(GTK_WIDGET(create), FALSE);
    gtk_signal_connect( GTK_OBJECT(entry), "changed",
                    G_CALLBACK(on_editable_changed_enable_if_valid_xml_name),
                    create );
    gtk_signal_connect( GTK_OBJECT(create), "clicked",
                         G_CALLBACK(on_clicked_get_editable_text), &name );
    gtk_signal_connect_object( GTK_OBJECT(create), "clicked",
                                G_CALLBACK(gtk_widget_destroy),
                                GTK_OBJECT(window) );
    GTK_WIDGET_SET_FLAGS( GTK_WIDGET(create),
                           GTK_CAN_DEFAULT | GTK_RECEIVES_DEFAULT );
    gtk_container_add(GTK_CONTAINER(bbox), create);

    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_window_set_default(GTK_WINDOW(window), GTK_WIDGET(create));
    gtk_window_set_focus(GTK_WINDOW(window), GTK_WIDGET(entry));

    name.editable = GTK_EDITABLE(entry);
    name.text = NULL;

    gtk_main();

    if (selected_repr != NULL && name.text) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(current_document);
        Inkscape::XML::Node *new_repr;
        new_repr = xml_doc->createElement(name.text);
        Inkscape::GC::release(new_repr);
        g_free(name.text);
        selected_repr->appendChild(new_repr);
        set_tree_select(new_repr);
        set_dt_select(new_repr);

        sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Create new element node"));
    }

} // end of cmd_new_element_node()



void cmd_new_text_node(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(current_document);
    Inkscape::XML::Node *text = xml_doc->createTextNode("");
    selected_repr->appendChild(text);

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Create new text node"));

    set_tree_select(text);
    set_dt_select(text);

    gtk_window_set_focus(GTK_WINDOW(dlg), GTK_WIDGET(content));

}

void cmd_duplicate_node(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);

    Inkscape::XML::Node *parent = sp_repr_parent(selected_repr);
    Inkscape::XML::Node *dup = selected_repr->duplicate(parent->document());
    parent->addChild(dup, selected_repr);

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Duplicate node"));

    GtkCTreeNode *node = sp_xmlview_tree_get_repr_node(SP_XMLVIEW_TREE(tree), dup);

    if (node) {
        gtk_ctree_select(GTK_CTREE(tree), node);
    }
}



void cmd_delete_node(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);
    sp_repr_unparent(selected_repr);

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Delete node"));
}



void cmd_delete_attr(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);
    g_assert(selected_attr != 0);
    selected_repr->setAttribute(g_quark_to_string(selected_attr), NULL);

    SPObject *updated=current_document->getObjectByRepr(selected_repr);
    if (updated) {
        // force immediate update of dependant attributes
        updated->updateRepr();
    }

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Delete attribute"));
}



void cmd_set_attr(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);

    gchar *name = gtk_editable_get_chars(attr_name, 0, -1);
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_bounds( gtk_text_view_get_buffer(attr_value),
                                 &start, &end );
    gchar *value = gtk_text_buffer_get_text( gtk_text_view_get_buffer(attr_value),
                                       &start, &end, TRUE );

    selected_repr->setAttribute(name, value, false);

    g_free(name);
    g_free(value);

    SPObject *updated = current_document->getObjectByRepr(selected_repr);
    if (updated) {
        // force immediate update of dependant attributes
        updated->updateRepr();
    }

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Change attribute"));

    /* TODO: actually, the row won't have been created yet.  why? */
    gint row = sp_xmlview_attr_list_find_row_from_key(GTK_CLIST(attributes),
                                                      g_quark_from_string(name));
    if (row != -1) {
        gtk_clist_select_row(GTK_CLIST(attributes), row, 0);
    }
}



void cmd_raise_node(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);

    Inkscape::XML::Node *parent = sp_repr_parent(selected_repr);
    g_return_if_fail(parent != NULL);
    g_return_if_fail(parent->firstChild() != selected_repr);

    Inkscape::XML::Node *ref = NULL;
    Inkscape::XML::Node *before = parent->firstChild();
    while (before && before->next() != selected_repr) {
        ref = before;
        before = before->next();
    }

    parent->changeOrder(selected_repr, ref);

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Raise node"));

    set_tree_select(selected_repr);
    set_dt_select(selected_repr);
}



void cmd_lower_node(GtkObject */*object*/, gpointer /*data*/)
{
    g_assert(selected_repr != NULL);
    g_return_if_fail(selected_repr->next() != NULL);
    Inkscape::XML::Node *parent = sp_repr_parent(selected_repr);

    parent->changeOrder(selected_repr, selected_repr->next());

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Lower node"));

    set_tree_select(selected_repr);
    set_dt_select(selected_repr);
}

void cmd_indent_node(GtkObject */*object*/, gpointer /*data*/)
{
    Inkscape::XML::Node *repr = selected_repr;
    g_assert(repr != NULL);
    Inkscape::XML::Node *parent = sp_repr_parent(repr);
    g_return_if_fail(parent != NULL);
    g_return_if_fail(parent->firstChild() != repr);

    Inkscape::XML::Node* prev = parent->firstChild();
    while (prev && prev->next() != repr) {
        prev = prev->next();
    }
    g_return_if_fail(prev != NULL);
    g_return_if_fail(prev->type() == Inkscape::XML::ELEMENT_NODE);

    Inkscape::XML::Node* ref = NULL;
    if (prev->firstChild()) {
        for( ref = prev->firstChild() ; ref->next() ; ref = ref->next() ){};
    }

    parent->removeChild(repr);
    prev->addChild(repr, ref);

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Indent node"));
    set_tree_select(repr);
    set_dt_select(repr);

} // end of cmd_indent_node()



void cmd_unindent_node(GtkObject */*object*/, gpointer /*data*/)
{
    Inkscape::XML::Node *repr = selected_repr;
    g_assert(repr != NULL);
    Inkscape::XML::Node *parent = sp_repr_parent(repr);
    g_return_if_fail(parent);
    Inkscape::XML::Node *grandparent = sp_repr_parent(parent);
    g_return_if_fail(grandparent);

    parent->removeChild(repr);
    grandparent->addChild(repr, parent);

    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
                     _("Unindent node"));
    set_tree_select(repr);
    set_dt_select(repr);

} // end of cmd_unindent_node()


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

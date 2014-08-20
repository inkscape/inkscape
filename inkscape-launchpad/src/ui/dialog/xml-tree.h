/** @file
 * @brief XML tree editing dialog for Inkscape
 */
/* Copyright Lauris Kaplinski, 2000
 *
 * Released under GNU General Public License.
 *
 * This is XML tree editor, which allows direct modifying of all elements
 *   of Inkscape document, including foreign ones.
 *
 */

#ifndef SEEN_DIALOGS_XML_TREE_H
#define SEEN_DIALOGS_XML_TREE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/widget/panel.h"
#include <gtkmm/entry.h>
#include <gtkmm/textview.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/paned.h>

#include "ui/dialog/desktop-tracker.h"
#include "message.h"

class SPDesktop;
class SPObject;
struct SPXMLViewAttrList;
struct SPXMLViewContent;
struct SPXMLViewTree;

namespace Inkscape {
class MessageStack;
class MessageContext;

namespace XML {
class Node;
}

namespace UI {
namespace Dialog {

/**
 * A dialog widget to view and edit the document xml
 *
 */

class XmlTree : public Widget::Panel {
public:
    XmlTree ();
    ~XmlTree ();

    static XmlTree &getInstance() { return *new XmlTree(); }

private:

    /**
     * Is invoked by the desktop tracker when the desktop changes.
     */
    void set_tree_desktop(SPDesktop *desktop);

    /**
     * Is invoked when the documnet changes
     */
    void set_tree_document(SPDocument *document);

    /**
     * Select a node in the xml tree
     */
    void set_tree_repr(Inkscape::XML::Node *repr);

    /**
     * Sets the XML status bar when the tree is selected.
     */
    void tree_reset_context();

    /**
     * Sets the XML status bar, depending on which attr is selected.
     */
    void attr_reset_context(gint attr);

    /**
     * Is the selected tree node editable
     */
    gboolean xml_tree_node_mutable(GtkTreeIter *node);

    /**
     * Callback to close the add dialog on Escape key
     */
    static gboolean quit_on_esc (GtkWidget *w, GdkEventKey *event, GObject */*tbl*/);

    /**
     * Select a node in the xml tree
     */
    void set_tree_select(Inkscape::XML::Node *repr);

    /**
     * Set the attribute list to match the selected node in the tree
     */
    void propagate_tree_select(Inkscape::XML::Node *repr);

    /**
      * Find the current desktop selection
      */
    Inkscape::XML::Node *get_dt_select();

    /**
      * Select the current desktop selection
      */
    void set_dt_select(Inkscape::XML::Node *repr);

    /**
      * Callback for a node in the tree being selected
      */
    static void on_tree_select_row(GtkTreeSelection *selection, gpointer data);

    /**
      * Callback when a node is moved in the tree
      */
    static void after_tree_move(SPXMLViewTree *attributes, gpointer value, gpointer data);

    /**
      * Callback for when attribute selection changes
      */
    static void on_attr_select_row(GtkTreeSelection *selection, gpointer data);

    /**
      * Callback for when attribute list values change
      */
    static void on_attr_row_changed(SPXMLViewAttrList *attributes, const gchar * name, gpointer data);

    /**
      * Enable widgets based on current selections
      */
    void on_tree_select_row_enable(GtkTreeIter *node);
    void on_tree_unselect_row_disable();
    void on_tree_unselect_row_hide();
    void on_attr_unselect_row_disable();
    void on_attr_unselect_row_clear_text();

    void onNameChanged();
    void onCreateNameChanged();

    /**
      * Callbacks for changes in desktop selection and current document
      */
    void on_desktop_selection_changed();
    void on_document_replaced(SPDesktop *dt, SPDocument *document);
    static void on_document_uri_set(gchar const *uri, SPDocument *document);

    static void _set_status_message(Inkscape::MessageType type, const gchar *message, GtkWidget *dialog);

    /**
      * Callbacks for toolbar buttons being pressed
      */
    void cmd_new_element_node();
    void cmd_new_text_node();
    void cmd_duplicate_node();
    void cmd_delete_node();
    void cmd_raise_node();
    void cmd_lower_node();
    void cmd_indent_node();
    void cmd_unindent_node();

    void cmd_delete_attr();
    void cmd_set_attr();
    virtual void present();

    bool sp_xml_tree_key_press(GdkEventKey *event);

    bool in_dt_coordsys(SPObject const &item);

    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);

    /**
     * Flag to ensure only one operation is perfomed at once
     */
    gint blocked;

    /**
     * Status bar
     */
    Inkscape::MessageStack *_message_stack;
    Inkscape::MessageContext *_message_context;

    /**
     * Signal handlers
     */
    sigc::connection _message_changed_connection;
    sigc::connection document_replaced_connection;
    sigc::connection document_uri_set_connection;
    sigc::connection sel_changed_connection;

    /**
     * Current document and desktop this dialog is attached to
     */
    SPDesktop *current_desktop;
    SPDocument *current_document;

    gint selected_attr;
    Inkscape::XML::Node *selected_repr;

    /* XmlTree Widgets */
    SPXMLViewTree *tree;
    SPXMLViewAttrList *attributes;
    SPXMLViewContent *content;

    Gtk::Entry attr_name;
    Gtk::TextView attr_value;

    Gtk::Button *create_button;
    Gtk::Entry *name_entry;

#if WITH_GTKMM_3_0
    Gtk::Paned paned;
#else
    Gtk::HPaned paned;
#endif

    Gtk::VBox left_box;
    Gtk::VBox right_box;
    Gtk::HBox status_box;
    Gtk::Label status;
    Gtk::Toolbar    tree_toolbar;
    Gtk::ToolButton xml_element_new_button;
    Gtk::ToolButton xml_text_new_button;
    Gtk::ToolButton xml_node_delete_button;
    Gtk::SeparatorToolItem separator;
    Gtk::ToolButton xml_node_duplicate_button;
    Gtk::SeparatorToolItem separator2;
    Gtk::ToolButton unindent_node_button;
    Gtk::ToolButton indent_node_button;
    Gtk::ToolButton raise_node_button;
    Gtk::ToolButton lower_node_button;

    Gtk::Toolbar    attr_toolbar;
    Gtk::ToolButton xml_attribute_delete_button;

    Gtk::VBox attr_vbox;
    Gtk::ScrolledWindow text_container;
    Gtk::HBox attr_hbox;
    Gtk::VBox attr_container;

#if WITH_GTKMM_3_0
    Gtk::Paned attr_subpaned_container;
#else
    Gtk::VPaned attr_subpaned_container;
#endif

    Gtk::Button set_attr;

    GtkWidget *new_window;

    DesktopTracker deskTrack;
    sigc::connection desktopChangeConn;
};

}
}
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

/** @file
 * @brief Text-edit
 */
/* Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 1999-2007 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_TEXT_EDIT_H
#define INKSCAPE_UI_DIALOG_TEXT_EDIT_H

#include <gtk/gtk.h>

#include <gtkmm/box.h>
#include <gtkmm/textview.h>
#include <gtkmm/notebook.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>

#include <glibmm/i18n.h>

#include "ui/widget/panel.h"
#include "dialogs/dialog-events.h"
#include "widgets/font-selector.h"
#include "ui/dialog/desktop-tracker.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

#define VB_MARGIN 4

class TextEdit : public UI::Widget::Panel {
public:
    TextEdit();
    virtual ~TextEdit();

    static TextEdit &getInstance() { return *new TextEdit(); }

protected:

    /**
     * Callbacks for button presses and change handlers
     */
    void onSetDefault ();
    void onApply ();
    void onSelectionChange ();
    void onSelectionModified (guint flags);
    void onReadSelection (gboolean style, gboolean content);
    void onToggle ();
    static void onLineSpacingChange (GtkComboBox* widget, gpointer data);
    static void onTextChange (GtkTextBuffer *text_buffer, TextEdit *self);
    static void onFontChange (SPFontSelector *fontsel, font_instance *font, TextEdit *self);

    /**
     * Functions to get the selected text off the main canvas
     */
    SPItem *getSelectedTextItem (void);
    unsigned getSelectedTextCount (void);

    /**
     * Helper function to create markup from a font definition and display in the preview label
     */
    void setPreviewText (font_instance *font, Glib::ustring phrase);

    void updateObjectText ( SPItem *text );
    SPCSSAttr *getTextStyle ();

    /**
     * Helper function to style radio buttons with icons, tooltips
     */
    void styleButton(Gtk::RadioButton *button, gchar const *tooltip, gchar const *iconname, Gtk::RadioButton *group_button  );

    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);

    /**
     * Is invoked by the desktop tracker when the desktop changes.
     */
    void setTargetDesktop(SPDesktop *desktop);



private:

    /**
     * All the dialogs widgets
     */
    Gtk::Notebook notebook;

    Gtk::VBox font_vbox;
    Gtk::Label font_label;
    Gtk::HBox fontsel_hbox;
    SPFontSelector *fsel;

    Gtk::Frame layout_frame;
    Gtk::HBox layout_hbox;
    Gtk::RadioButton align_left;
    Gtk::RadioButton align_center;
    Gtk::RadioButton align_right;
    Gtk::RadioButton align_justify;
    Gtk::VSeparator align_sep;
    Gtk::RadioButton text_vertical;
    Gtk::RadioButton text_horizontal;
    Gtk::VSeparator text_sep;
    GtkWidget *spacing_combo;

    Gtk::Label preview_label;

    Gtk::Label text_label;
    Gtk::VBox text_vbox;
    Gtk::ScrolledWindow scroller;
    GtkWidget *text_view; // TODO - Convert this to a Gtk::TextView, but GtkSpell doesn't seem to work with it
    GtkTextBuffer *text_buffer;

    Gtk::HBox button_row;
    Gtk::Button setasdefault_button;
    Gtk::Button close_button;
    Gtk::Button apply_button;

    SPDesktop *desktop;
    DesktopTracker deskTrack;
    sigc::connection desktopChangeConn;
    sigc::connection selectChangedConn;
    sigc::connection subselChangedConn;
    sigc::connection selectModifiedConn;

    bool blocked;
    const Glib::ustring samplephrase;
    TextEdit(TextEdit const &d);
    TextEdit operator=(TextEdit const &d);
};


} //namespace Dialog
} //namespace UI
} //namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_TEXT_EDIT_H

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

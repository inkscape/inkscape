/** @file
 * @brief Text-edit
 */
/* Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   John Smith
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *   Tavmjong Bah
 *
 * Copyright (C) 1999-2013 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_TEXT_EDIT_H
#define INKSCAPE_UI_DIALOG_TEXT_EDIT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include "ui/widget/panel.h"
#include "ui/widget/frame.h"
#include "ui/dialog/desktop-tracker.h"
#include "ui/widget/font-variants.h"

class SPItem;
struct SPFontSelector;
//class FontVariants;
class font_instance;
class SPCSSAttr;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define VB_MARGIN 4
/**
 * The TextEdit class defines the Text and font dialog.
 * 
 * The Text and font dialog allows you to set the font family, style and size
 * and shows a preview of the result. The dialogs layout settings include
 * horizontal and vertical alignment and inter line distance.
 */
class TextEdit : public UI::Widget::Panel {
public:
    TextEdit();
    virtual ~TextEdit();

    /**
     * Helper function which returns a new instance of the dialog.
     * getInstance is needed by the dialog manager (Inkscape::UI::Dialog::DialogManager).
     */
    static TextEdit &getInstance() { return *new TextEdit(); }

protected:

    /**
     * Callback for pressing the default button.
     */
    void onSetDefault ();

    /**
     * Callback for pressing the apply button.
     */
    void onApply ();
    void onSelectionChange ();
    void onSelectionModified (guint flags);
    
    /**
     * Called whenever something 'changes' on canvas.
     * 
     * onReadSelection gets the currently selected item from the canvas and sets all the controls in this dialog to the correct state.
     * 
     * @param dostyle Indicates whether the modification of the user includes a style change.
     * @param content Indicates whether the modification of the user includes a style change. Actually refers to the question if we do want to show the content? (Parameter currently not used)
     */
    void onReadSelection (gboolean style, gboolean content);
    void onToggle ();
    static void onLineSpacingChange (GtkComboBox* widget, gpointer data);
    
    /**
     * Callback invoked when the user modifies the text of the selected text object.
     *
     * onTextChange is responsible for initiating the commands after the user
     * modified the text in the selected object. The UI of the dialog is
     * updated. The subfunction setPreviewText updates the preview label.
     *
     * @param text_buffer pointer to GtkTextBuffer with the text of the selected text object
     * @param self pointer to the current instance of the dialog.
     */
    static void onTextChange (GtkTextBuffer *text_buffer, TextEdit *self);
    
    /**
     * Callback invoked when the user modifies the font through the dialog or the tools control bar.
     *
     * onFontChange updates the dialog UI. The subfunction setPreviewText updates the preview label.
     *
     * @param fontsel pointer to SPFontSelector (currently not used).
     * @param fontspec for the text to be previewed.
     * @param self pointer to the current instance of the dialog.
     */
    static void onFontChange (SPFontSelector *fontsel, gchar* fontspec, TextEdit *self);

    /**
     * Callback invoked when the user modifies the font variant through the dialog.
     *
     * onFontChange updates the dialog UI. The subfunction setPreviewText updates the preview label.
     *
     * @param fontsel pointer to FontVariant (currently not used).
     * @param fontspec for the text to be previewed.
     * @param self pointer to the current instance of the dialog.
     */
    static void onFontVariantChange (TextEdit *self);

    /**
     * Callback invoked when the user modifies the startOffset of text on a path.
     *
     * @param text_buffer pointer to the GtkTextBuffer with the text of the selected text object.
     * @param self pointer to the current instance of the dialog.
     */
    static void onStartOffsetChange(GtkTextBuffer *text_buffer, TextEdit *self);

    /**
     * Get the selected text off the main canvas.
     *
     * @return SPItem pointer to the selected text object
     */
    SPItem *getSelectedTextItem (void);

    /**
     * Count the number of text objects in the selection on the canvas.
     */
    unsigned getSelectedTextCount (void);

    /**
     * Helper function to create markup from a fontspec and display in the preview label.
     * 
     * @param fontspec for the text to be previewed
     * @param phrase text to be shown
     */
    void setPreviewText (Glib::ustring font_spec, Glib::ustring phrase);

    void updateObjectText ( SPItem *text );
    SPCSSAttr *fillTextStyle ();

    /**
     * Helper function to style radio buttons with icons, tooltips.
     * 
     * styleButton is used when creating the dialog.
     * 
     * @param button pointer to the button which is created
     * @param tooltip pointer to its tooltip string
     * @param iconname string identifying the icon to be shown
     * @param group_button group to which the radio button belongs to
     */
    void styleButton(Gtk::RadioButton *button, gchar const *tooltip, gchar const *iconname, Gtk::RadioButton *group_button  );

    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);

    /**
     * Is invoked by the desktop tracker when the desktop changes.
     *
     * @see DesktopTracker
     */
    void setTargetDesktop(SPDesktop *desktop);



private:

    /*
     * All the dialogs widgets
     */
    Gtk::Notebook notebook;

    Gtk::VBox font_vbox;
    Gtk::Label font_label;
    Gtk::HBox fontsel_hbox;
    SPFontSelector *fsel;

    Gtk::Alignment layout_frame;
    Gtk::HBox layout_hbox;
    Gtk::RadioButton align_left;
    Gtk::RadioButton align_center;
    Gtk::RadioButton align_right;
    Gtk::RadioButton align_justify;

#if WITH_GTKMM_3_0
    Gtk::Separator  align_sep;
#else
    Gtk::VSeparator align_sep;
#endif

    Gtk::RadioButton text_vertical;
    Gtk::RadioButton text_horizontal;

#if WITH_GTKMM_3_0
    Gtk::Separator  text_sep;
#else
    Gtk::VSeparator text_sep;
#endif

    GtkWidget *spacing_combo;

    GtkWidget *startOffset;

    Gtk::Label preview_label;

    Gtk::Label text_label;
    Gtk::VBox text_vbox;
    Gtk::ScrolledWindow scroller;
    GtkWidget *text_view; // TODO - Convert this to a Gtk::TextView, but GtkSpell doesn't seem to work with it
    GtkTextBuffer *text_buffer;

    Inkscape::UI::Widget::FontVariants vari_vbox;
    Gtk::Label vari_label;
    
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
    sigc::connection fontVariantChangedConn;

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

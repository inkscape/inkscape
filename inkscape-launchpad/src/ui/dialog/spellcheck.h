/** @file
 * @brief  Spellcheck dialog
 */
/* Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SPELLCHECK_H
#define SEEN_SPELLCHECK_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "ui/dialog/desktop-tracker.h"
#include "ui/widget/panel.h"
#include "sp-text.h"

#ifdef HAVE_ASPELL
#include <aspell.h>
#endif  /* HAVE_ASPELL */

class SPDesktop;

namespace Inkscape {
class Preferences;

namespace UI {
namespace Dialog {

/**
 *
 * A dialog widget to checking spelling of text elements in the document
 * Uses ASpell and one of the languages set in the users preference file
 *
 */
class SpellCheck : public Widget::Panel {
public:
    SpellCheck ();
    ~SpellCheck ();

    static SpellCheck &getInstance() { return *new SpellCheck(); }

private:

    /**
     * Remove the highlight rectangle form the canvas
     */
    void clearRects();

    /**
     * Release handlers to the selected item
     */
    void disconnect();

    /**
     * Returns a list of all the text items in the SPObject
     */
    GSList *allTextItems (SPObject *r, GSList *l, bool hidden, bool locked);

    /**
     * Is text inside the SPOject's tree
     */
    bool textIsValid (SPObject *root, SPItem *text);

    /**
     * Compare the visual bounds of 2 SPItems referred to by a and b
     */
    static gint compareTextBboxes (gconstpointer a, gconstpointer b);
    SPItem *getText (SPObject *root);
    void    nextText ();

    /**
     * Initialize the controls and aspell
     */
    bool    init (SPDesktop *desktop);

    /**
     * Cleanup after spellcheck is finished
     */
    void    finished ();

    /**
     * Find the next word to spell check
     */
    bool    nextWord();
    void    deleteLastRect ();
    void    doSpellcheck ();

    /**
     * Accept button clicked
     */
    void    onAccept ();

    /**
     * Ignore button clicked
     */
    void    onIgnore ();

    /**
     * Ignore once button clicked
     */
    void    onIgnoreOnce ();

    /**
     * Add button clicked
     */
    void    onAdd ();

    /**
     * Stop button clicked
     */
    void    onStop ();

    /**
     * Start button clicked
     */
    void    onStart ();

    /**
     * Selected object modified on canvas
     */
    void    onObjModified (SPObject* /* blah */, unsigned int /* bleh */);

    /**
     * Selected object removed from canvas
     */
    void    onObjReleased (SPObject* /* blah */);

    /**
     * Selection in suggestions text view changed
     */
    void onTreeSelectionChange();

    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);

    /**
     * Is invoked by the desktop tracker when the desktop changes.
     */
    void setTargetDesktop(SPDesktop *desktop);

    SPObject *_root;

#ifdef HAVE_ASPELL
    AspellSpeller *_speller;
    AspellSpeller *_speller2;
    AspellSpeller *_speller3;
#endif  /* HAVE_ASPELL */

    /**
     * list of canvasitems (currently just rects) that mark misspelled things on canvas
     */
    GSList *_rects;

    /**
     * list of text objects we have already checked in this session
     */
    GSList *_seen_objects;

    /**
     *  the object currently being checked
     */
    SPItem *_text;

    /**
     * current objects layout
     */
    Inkscape::Text::Layout const *_layout;

    /**
     *  iterators for the start and end of the current word
     */
    Inkscape::Text::Layout::iterator _begin_w;
    Inkscape::Text::Layout::iterator _end_w;

    /**
     *  the word we're checking
     */
    Glib::ustring _word;

    /**
     *  counters for the number of stops and dictionary adds
     */
    int _stops;
    int _adds;

    /**
     *  true if we are in the middle of a check
     */
    bool _working;

    /**
     *  connect to the object being checked in case it is modified or deleted by user
     */
    sigc::connection _modified_connection;
    sigc::connection _release_connection;

    /**
     *  true if the spell checker dialog has changed text, to suppress modified callback
     */
    bool _local_change;

    Inkscape::Preferences *_prefs;

    Glib::ustring _lang;
    Glib::ustring _lang2;
    Glib::ustring _lang3;

    /*
     *  Dialogs widgets
     */
    Gtk::Label          banner_label;
#if WITH_GTKMM_3_0
    Gtk::ButtonBox      banner_hbox;
#else
    Gtk::HButtonBox     banner_hbox;
#endif
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TreeView       tree_view;
    Glib::RefPtr<Gtk::ListStore> model;

    Gtk::HBox       suggestion_hbox;
    Gtk::VBox       changebutton_vbox;
    Gtk::Button     accept_button;
    Gtk::Button     ignoreonce_button;
    Gtk::Button     ignore_button;

    Gtk::Button     add_button;
    GtkWidget *     dictionary_combo;
    Gtk::HBox       dictionary_hbox;

#if WITH_GTKMM_3_0
    Gtk::Separator  action_sep;
#else
    Gtk::HSeparator action_sep;
#endif

    Gtk::Button     stop_button;
    Gtk::Button     start_button;

#if WITH_GTKMM_3_0
    Gtk::ButtonBox  actionbutton_hbox;
#else
    Gtk::HButtonBox actionbutton_hbox;
#endif

    SPDesktop *     desktop;
    DesktopTracker  deskTrack;
    sigc::connection desktopChangeConn;

    class TreeColumns : public Gtk::TreeModel::ColumnRecord
    {
      public:
        TreeColumns()
        {
            add(suggestions);
        }
        virtual ~TreeColumns() {}
        Gtk::TreeModelColumn<Glib::ustring> suggestions;
    };
    TreeColumns tree_columns;

};

}
}
}

#endif /* !SEEN_SPELLCHECK_H */

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

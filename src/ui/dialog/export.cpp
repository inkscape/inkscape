/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Peter Bostrom
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 1999-2007, 2012 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

// This has to be included prior to anything that includes setjmp.h, it croaks otherwise
#include <png.h>

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stock.h>
#include <gtkmm/stockid.h>
#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif
#include <gtkmm/togglebutton.h>
#include <gtkmm/widget.h>

#ifdef WITH_GNOME_VFS
# include <libgnomevfs/gnome-vfs-init.h>  // gnome_vfs_initialized
#endif

#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>

#include "ui/widget/unit-menu.h"
#include "util/units.h"
#include "helper/window.h"
#include "inkscape-private.h"
#include "document.h"
#include "document-undo.h"
#include "desktop-handles.h"
#include "sp-item.h"
#include "selection.h"
#include "file.h"
#include "macros.h"
#include "sp-namedview.h"
#include "selection-chemistry.h"

#include "ui/dialog-events.h"
#include "preferences.h"
#include "verbs.h"
#include "ui/interface.h"
#include "sp-root.h"

#include "extension/output.h"
#include "extension/db.h"

#include "io/sys.h"

#include "helper/png-write.h"

#if WITH_EXT_GDL
#include <gdl/gdl-dock-item.h>
#else
#include "libgdl/gdl-dock-item.h"
#endif


// required to set status message after export
#include "desktop.h"
#include "message-stack.h"

#ifdef WIN32
#include <windows.h>
#include <commdlg.h>
#include <gdk/gdkwin32.h>
#include <glibmm/fileutils.h>
#endif

#include <gtk/gtk.h>

#define SP_EXPORT_MIN_SIZE 1.0

#define DPI_BASE Inkscape::Util::Quantity::convert(1, "in", "px")

#define EXPORT_COORD_PRECISION 3

#include "../../desktop-handles.h"
#include "../../document.h"
#include "../../document-undo.h"
#include "verbs.h"
#include "export.h"

using Inkscape::Util::unit_table;

namespace {

class MessageCleaner
{
public:
    MessageCleaner(Inkscape::MessageId messageId, SPDesktop *desktop) :
        _desktop(desktop),
        _messageId(messageId)
    {
    }

    ~MessageCleaner()
    {
        if (_messageId && _desktop) {
            _desktop->messageStack()->cancel(_messageId);
        }
    }

private:
    MessageCleaner(MessageCleaner const &other);
    MessageCleaner &operator=(MessageCleaner const &other);

    SPDesktop *_desktop;
    Inkscape::MessageId _messageId;
};

} // namespace

namespace Inkscape {
namespace UI {
namespace Dialog {

/** A list of strings that is used both in the preferences, and in the
    data fields to describe the various values of \c selection_type. */
static const char * selection_names[SELECTION_NUMBER_OF] = {
    "page", "drawing", "selection", "custom"
};

/** The names on the buttons for the various selection types. */
static const char * selection_labels[SELECTION_NUMBER_OF] = {
    N_("_Page"), N_("_Drawing"), N_("_Selection"), N_("_Custom")
};

Export::Export (void) :
    UI::Widget::Panel ("", "/dialogs/export/", SP_VERB_DIALOG_EXPORT),
    current_key(SELECTION_PAGE),
    original_name(),
    doc_export_name(),
    filename_modified(false),
    was_empty(true),
    update(false),
    togglebox(true, 0),
    area_box(false, 3),
    singleexport_box(false, 0),
    size_box(false, 3),
    file_box(false, 3),
    unitbox(false, 0),
    unit_selector(),
    units_label(_("Units:")),
    filename_box(false, 5),
    browse_label(_("_Export As..."), 1),
    browse_image(Gtk::StockID(Gtk::Stock::INDEX), Gtk::ICON_SIZE_BUTTON),
    batch_box(false, 5),
    batch_export(_("B_atch export all selected objects"), _("Export each selected object into its own PNG file, using export hints if any (caution, overwrites without asking!)")),
    hide_box(false, 5),
    hide_export(_("Hide a_ll except selected"), _("In the exported image, hide all objects except those that are selected")),
    closeWhenDone(_("Close when complete"), _("Once the export completes, close this dialog")),
    button_box(false, 3),
    export_label(_("_Export"), 1),
    export_image(Gtk::StockID(Gtk::Stock::APPLY), Gtk::ICON_SIZE_BUTTON),
    _prog(),
    prog_dlg(NULL),
    interrupted(false),
    prefs(NULL),
    desktop(NULL),
    deskTrack(),
    selectChangedConn(),
    subselChangedConn(),
    selectModifiedConn()
{
    prefs = Inkscape::Preferences::get();

    singleexport_box.set_border_width(0);

    /* Export area frame */
    {
        Gtk::Label* lbl = new Gtk::Label(_("<b>Export area</b>"), Gtk::ALIGN_START);
        lbl->set_use_markup(true);
        area_box.pack_start(*lbl);

        /* Units box */
        /* gets added to the vbox later, but the unit selector is needed
           earlier than that */
        unit_selector.setUnitType(Inkscape::Util::UNIT_TYPE_LINEAR);

        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (desktop) {
            unit_selector.setUnit(sp_desktop_namedview(desktop)->display_units->abbr);
        }
        unitChangedConn = unit_selector.signal_changed().connect(sigc::mem_fun(*this, &Export::onUnitChanged));
        unitbox.pack_end(unit_selector, false, false, 0);
        unitbox.pack_end(units_label, false, false, 3);

        for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
            selectiontype_buttons[i] = new Gtk::RadioButton(_(selection_labels[i]), true);
            if (i > 0) {
                Gtk::RadioButton::Group group = selectiontype_buttons[0]->get_group();
                selectiontype_buttons[i]->set_group(group);
            }
            selectiontype_buttons[i]->set_mode(false);
            togglebox.pack_start(*selectiontype_buttons[i], false, true, 0);
            selectiontype_buttons[i]->signal_clicked().connect(sigc::mem_fun(*this, &Export::onAreaToggled));
        }

#if WITH_GTKMM_3_0
        Gtk::Grid* t = new Gtk::Grid();
        t->set_row_spacing(4);
        t->set_column_spacing(4);
#else
        Gtk::Table* t = new Gtk::Table(3, 4, false);
        t->set_row_spacings (4);
        t->set_col_spacings (4);
#endif

        x0_adj = createSpinbutton ( "x0", 0.0, -1000000.0, 1000000.0, 0.1, 1.0,
                                    t, 0, 0, _("_x0:"), "", EXPORT_COORD_PRECISION, 1,
                                    &Export::onAreaX0Change);

        x1_adj = createSpinbutton ( "x1", 0.0, -1000000.0, 1000000.0, 0.1, 1.0,
                                    t, 0, 1, _("x_1:"), "", EXPORT_COORD_PRECISION, 1,
                                    &Export::onAreaX1Change);

        width_adj = createSpinbutton ( "width", 0.0, 0.0, PNG_UINT_31_MAX, 0.1, 1.0,
                                       t, 0, 2, _("Wid_th:"), "", EXPORT_COORD_PRECISION, 1,
                                       &Export::onAreaWidthChange);

        y0_adj = createSpinbutton ( "y0", 0.0, -1000000.0, 1000000.0, 0.1, 1.0,
                                    t, 2, 0, _("_y0:"), "", EXPORT_COORD_PRECISION, 1,
                                    &Export::onAreaY0Change);

        y1_adj = createSpinbutton ( "y1", 0.0, -1000000.0, 1000000.0, 0.1, 1.0,
                                    t, 2, 1, _("y_1:"), "", EXPORT_COORD_PRECISION, 1,
                                    &Export::onAreaY1Change);

        height_adj = createSpinbutton ( "height", 0.0, 0.0, PNG_UINT_31_MAX, 0.1, 1.0,
                                        t, 2, 2, _("Hei_ght:"), "", EXPORT_COORD_PRECISION, 1,
                                        &Export::onAreaHeightChange);

        area_box.pack_start(togglebox, false, false, 3);
        area_box.pack_start(*t, false, false, 0);
        area_box.pack_start(unitbox, false, false, 0);

        area_box.set_border_width(3);
        singleexport_box.pack_start(area_box, false, false, 0);

    } // end of area box

    /* Bitmap size frame */
    {
        size_box.set_border_width(3);
        bm_label = new Gtk::Label(_("<b>Image size</b>"), Gtk::ALIGN_START);
        bm_label->set_use_markup(true);
        size_box.pack_start(*bm_label, false, false, 0);

#if WITH_GTKMM_3_0
        Gtk::Grid *t = new Gtk::Grid();
        t->set_row_spacing(4);
        t->set_column_spacing(4);
#else
        Gtk::Table *t = new Gtk::Table(2, 5, false);
        t->set_row_spacings (4);
        t->set_col_spacings (4);
#endif

        size_box.pack_start(*t);

        bmwidth_adj = createSpinbutton ( "bmwidth", 16.0, 1.0, 1000000.0, 1.0, 10.0,
                                         t, 0, 0,
                                         _("_Width:"), _("pixels at"), 0, 1,
                                         &Export::onBitmapWidthChange);

        xdpi_adj = createSpinbutton ( "xdpi",
                                      prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE),
                                      0.01, 100000.0, 0.1, 1.0, t, 3, 0,
                                      "", _("dp_i"), 2, 1,
                                      &Export::onExportXdpiChange);

        bmheight_adj = createSpinbutton ( "bmheight", 16.0, 1.0, 1000000.0, 1.0, 10.0,
                                          t, 0, 1,
                                          _("_Height:"), _("pixels at"), 0, 1,
                                          &Export::onBitmapHeightChange);

        /** TODO
         *  There's no way to set ydpi currently, so we use the defaultxdpi value here, too...
         */
        ydpi_adj = createSpinbutton ( "ydpi", prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE),
                                      0.01, 100000.0, 0.1, 1.0, t, 3, 1,
                                      "", _("dpi"), 2, 0, NULL );

        singleexport_box.pack_start(size_box);
    }

    /* File entry */
    {
        file_box.set_border_width(3);
        flabel = new Gtk::Label(_("<b>_Filename</b>"), Gtk::ALIGN_START, Gtk::ALIGN_CENTER, true);
        flabel->set_use_markup(true);
        file_box.pack_start(*flabel, false, false, 0);

        set_default_filename();

        filename_box.pack_start (filename_entry, true, true, 0);

        Gtk::HBox* browser_im_label = new Gtk::HBox(false, 3);
        browser_im_label->pack_start(browse_image);
        browser_im_label->pack_start(browse_label);
        browse_button.add(*browser_im_label);
        filename_box.pack_end (browse_button, false, false, 4);

        file_box.add(filename_box);

        original_name = filename_entry.get_text();

        // focus is in the filename initially:
        filename_entry.grab_focus();

        // mnemonic in frame label moves focus to filename:
        flabel->set_mnemonic_widget(filename_entry);

        singleexport_box.pack_start(file_box);
    }

    batch_export.set_sensitive(true);
    batch_box.pack_start(batch_export, false, false);

    hide_export.set_sensitive(true);
    hide_export.set_active (prefs->getBool("/dialogs/export/hideexceptselected/value", false));
    hide_box.pack_start(hide_export, false, false);


    /* Export Button row */
    button_box.set_border_width(3);
    Gtk::HBox* export_image_label = new Gtk::HBox(false, 3);
    export_image_label->pack_start(export_image);
    export_image_label->pack_start(export_label);

    export_button.add(*export_image_label);
    export_button.set_tooltip_text (_("Export the bitmap file with these settings"));

    button_box.pack_start(closeWhenDone, true, true, 0 );
    button_box.pack_end(export_button, false, false, 0);

    /* Main dialog */
    Gtk::Box *contents = _getContents();
    contents->set_spacing(0);
    contents->pack_start(singleexport_box);
    contents->pack_start(batch_box);
    contents->pack_start(hide_box);
    contents->pack_end(button_box, false, 0);
    contents->pack_end(_prog, Gtk::PACK_EXPAND_WIDGET);

    /* Signal handlers */
    filename_entry.signal_changed().connect( sigc::mem_fun(*this, &Export::onFilenameModified) );
    // pressing enter in the filename field is the same as clicking export:
    filename_entry.signal_activate().connect(sigc::mem_fun(*this, &Export::onExport) );
    browse_button.signal_clicked().connect(sigc::mem_fun(*this, &Export::onBrowse));
    batch_export.signal_clicked().connect(sigc::mem_fun(*this, &Export::onBatchClicked));
    export_button.signal_clicked().connect(sigc::mem_fun(*this, &Export::onExport));
    hide_export.signal_clicked().connect(sigc::mem_fun(*this, &Export::onHideExceptSelected));

    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &Export::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children();
    setExporting(false);

    findDefaultSelection();
    onAreaToggled();
}

Export::~Export (void)
{
    was_empty = TRUE;

    selectModifiedConn.disconnect();
    subselChangedConn.disconnect();
    selectChangedConn.disconnect();
    desktopChangeConn.disconnect();
    deskTrack.disconnect();
}

void Export::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void Export::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (this->desktop) {
            selectModifiedConn.disconnect();
            subselChangedConn.disconnect();
            selectChangedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {

            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &Export::onSelectionChanged)));
            subselChangedConn = desktop->connectToolSubselectionChanged(sigc::hide(sigc::mem_fun(*this, &Export::onSelectionChanged)));

            //// Must check flags, so can't call widget_setup() directly.
            selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &Export::onSelectionModified)));
        }
    }
}

/*
 * set the default filename to be that of the current path + document
 * with .png extension
 *
 * One thing to notice here is that this filename may get
 * overwritten, but it won't happen here.  The filename gets
 * written into the text field, but then the button to select
 * the area gets set.  In that code the filename can be changed
 * if there are some with presidence in the document.  So, while
 * this code sets the name first, it may not be the one users
 * really see.
 */
void Export::set_default_filename () {

    if ( SP_ACTIVE_DOCUMENT && SP_ACTIVE_DOCUMENT->getURI() )
    {
        SPDocument * doc = SP_ACTIVE_DOCUMENT;
        const gchar *uri = doc->getURI();
        const gchar *text_extension = get_file_save_extension (Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS).c_str();
        Inkscape::Extension::Output * oextension = NULL;

        if (text_extension != NULL) {
            oextension = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(text_extension));
        }

        if (oextension != NULL) {
            gchar * old_extension = oextension->get_extension();
            if (g_str_has_suffix(uri, old_extension)) {
                gchar * uri_copy;
                gchar * extension_point;
                gchar * final_name;

                uri_copy = g_strdup(uri);
                extension_point = g_strrstr(uri_copy, old_extension);
                extension_point[0] = '\0';

                final_name = g_strconcat(uri_copy, ".png", NULL);
                filename_entry.set_text(final_name);
                filename_entry.set_position(strlen(final_name));

                g_free(final_name);
                g_free(uri_copy);
            }
        } else {
            gchar *name = g_strconcat(uri, ".png", NULL);
            filename_entry.set_text(name);
            filename_entry.set_position(strlen(name));

            g_free(name);
        }

        doc_export_name = filename_entry.get_text();
    }

}

#if WITH_GTKMM_3_0
Glib::RefPtr<Gtk::Adjustment> Export::createSpinbutton( gchar const * /*key*/, float val, float min, float max,
        float step, float page,
        Gtk::Grid *t, int x, int y,
        const Glib::ustring& ll, const Glib::ustring& lr,
        int digits, unsigned int sensitive,
        void (Export::*cb)() )
#else
Gtk::Adjustment * Export::createSpinbutton( gchar const * /*key*/, float val, float min, float max,
        float step, float page,
        Gtk::Table *t, int x, int y,
        const Glib::ustring& ll, const Glib::ustring& lr,
        int digits, unsigned int sensitive,
        void (Export::*cb)() )
#endif
{
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> adj = Gtk::Adjustment::create(val, min, max, step, page, 0);
#else
    Gtk::Adjustment *adj = new Gtk::Adjustment  ( val, min, max, step, page, 0 );
#endif

    int pos = 0;
    Gtk::Label *l = NULL;

    if (!ll.empty()) {
        l = new Gtk::Label(ll,true);
        l->set_alignment (1.0, 0.5);

#if WITH_GTKMM_3_0
        l->set_hexpand();
        l->set_vexpand();
        t->attach(*l, x + pos, y, 1, 1);
#else
        t->attach (*l, x + pos, x + pos + 1, y, y + 1, Gtk::EXPAND, Gtk::EXPAND, 0, 0 );
#endif

        l->set_sensitive(sensitive);
        pos++;
    }

#if WITH_GTKMM_3_0
    Gtk::SpinButton *sb = new Gtk::SpinButton(adj, 1.0, digits);
    sb->set_hexpand();
    sb->set_vexpand();
    t->attach(*sb, x + pos, y, 1, 1);
#else
    Gtk::SpinButton *sb = new Gtk::SpinButton(*adj, 1.0, digits);
    t->attach (*sb, x + pos, x + pos + 1, y, y + 1, Gtk::EXPAND, Gtk::EXPAND, 0, 0 );
#endif

    sb->set_width_chars(7);
    sb->set_sensitive (sensitive);
    pos++;

    if (l) {
        l->set_mnemonic_widget(*sb);
    }

    if (!lr.empty()) {
        l = new Gtk::Label(lr,true);
        l->set_alignment (0.0, 0.5);

#if WITH_GTKMM_3_0
        l->set_hexpand();
        l->set_vexpand();
        t->attach(*l, x + pos, y, 1, 1);
#else
        t->attach (*l, x + pos, x + pos + 1, y, y + 1, Gtk::EXPAND, Gtk::EXPAND, 0, 0 );
#endif

        l->set_sensitive (sensitive);
        pos++;
        l->set_mnemonic_widget (*sb);
    }

    if (cb) {
        adj->signal_value_changed().connect( sigc::mem_fun(*this, cb) );
    }

    return adj;
} // end of createSpinbutton()


Glib::ustring Export::create_filepath_from_id (Glib::ustring id, const Glib::ustring &file_entry_text)
{
    if (id.empty())
    {   /* This should never happen */
        id = "bitmap";
    }

    Glib::ustring directory;

    if (!file_entry_text.empty()) {
        directory = Glib::path_get_dirname(file_entry_text);
    }

    if (directory.empty()) {
        /* Grab document directory */
        const gchar* docURI = SP_ACTIVE_DOCUMENT->getURI();
        if (docURI) {
            directory = Glib::path_get_dirname(docURI);
        }
    }

    if (directory.empty()) {
        directory = homedir_path(NULL);
    }

    Glib::ustring filename = Glib::build_filename(directory, id+".png");
    return filename;
}

void Export::onBatchClicked ()
{
    if (batch_export.get_active()) {
        singleexport_box.set_sensitive(false);
    } else {
        singleexport_box.set_sensitive(true);
    }
}

void Export::updateCheckbuttons ()
{
    gint num = g_slist_length((GSList *) sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList());
    if (num >= 2) {
        batch_export.set_sensitive(true);
        batch_export.set_label(g_strdup_printf (ngettext("B_atch export %d selected object","B_atch export %d selected objects",num), num));
    } else {
        batch_export.set_active (false);
        batch_export.set_sensitive(false);
    }

    //hide_export.set_sensitive (num > 0);
}

inline void Export::findDefaultSelection()
{
    selection_type key = SELECTION_NUMBER_OF;

    if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
        key = SELECTION_SELECTION;
    }

    /* Try using the preferences */
    if (key == SELECTION_NUMBER_OF) {

        int i = SELECTION_NUMBER_OF;

        Glib::ustring what = prefs->getString("/dialogs/export/exportarea/value");

        if (!what.empty()) {
            for (i = 0; i < SELECTION_NUMBER_OF; i++) {
                if (what == selection_names[i]) {
                    break;
                }
            }
        }

        key = (selection_type)i;
    }

    if (key == SELECTION_NUMBER_OF) {
        key = SELECTION_SELECTION;
    }

    current_key = key;
    selectiontype_buttons[current_key]->set_active(true);
    updateCheckbuttons ();
}


/**
 * If selection changed or a different document activated, we must
 * recalculate any chosen areas.
 */
void Export::onSelectionChanged()
{
    Inkscape::Selection *selection = sp_desktop_selection (SP_ACTIVE_DESKTOP);

    if ((current_key == SELECTION_DRAWING || current_key == SELECTION_PAGE) &&
            (sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false &&
            was_empty) {
        current_key = SELECTION_SELECTION;
        selectiontype_buttons[current_key]->set_active(true);
    }
    was_empty = (sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty();

    if ( selection &&
            SELECTION_CUSTOM != current_key) {
        onAreaToggled();
    }

    updateCheckbuttons ();
}

void Export::onSelectionModified ( guint /*flags*/ )
{
    Inkscape::Selection * Sel;
    switch (current_key) {
    case SELECTION_DRAWING:
        if ( SP_ACTIVE_DESKTOP ) {
            SPDocument *doc;
            doc = sp_desktop_document (SP_ACTIVE_DESKTOP);
            Geom::OptRect bbox = doc->getRoot()->desktopVisualBounds();
            if (bbox) {
                setArea ( bbox->left(),
                          bbox->top(),
                          bbox->right(),
                          bbox->bottom());
            }
        }
        break;
    case SELECTION_SELECTION:
        Sel = sp_desktop_selection(SP_ACTIVE_DESKTOP);
        if (Sel->isEmpty() == false) {
            Geom::OptRect bbox = Sel->visualBounds();
            if (bbox)
            {
                setArea ( bbox->left(),
                          bbox->top(),
                          bbox->right(),
                          bbox->bottom());
            }
        }
        break;
    default:
        /* Do nothing for page or for custom */
        break;
    }

    return;
}

/// Called when one of the selection buttons was toggled.
void Export::onAreaToggled ()
{
    if (update) {
        return;
    }

    /* Find which button is active */
    selection_type key = current_key;
    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        if (selectiontype_buttons[i]->get_active()) {
            key = (selection_type)i;
        }
    }

    if ( SP_ACTIVE_DESKTOP )
    {
        SPDocument *doc;
        Geom::OptRect bbox;
        bbox = Geom::Rect(Geom::Point(0.0, 0.0),Geom::Point(0.0, 0.0));
        doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

        /* Notice how the switch is used to 'fall through' here to get
           various backups.  If you modify this without noticing you'll
           probabaly screw something up. */
        switch (key) {
        case SELECTION_SELECTION:
            if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false)
            {
                bbox = sp_desktop_selection (SP_ACTIVE_DESKTOP)->visualBounds();
                /* Only if there is a selection that we can set
                   do we break, otherwise we fall through to the
                   drawing */
                // std::cout << "Using selection: SELECTION" << std::endl;
                key = SELECTION_SELECTION;
                break;
            }
        case SELECTION_DRAWING:
            /** \todo
             * This returns wrong values if the document has a viewBox.
             */
            bbox = doc->getRoot()->desktopVisualBounds();
            /* If the drawing is valid, then we'll use it and break
               otherwise we drop through to the page settings */
            if (bbox) {
                // std::cout << "Using selection: DRAWING" << std::endl;
                key = SELECTION_DRAWING;
                break;
            }
        case SELECTION_PAGE:
            bbox = Geom::Rect(Geom::Point(0.0, 0.0),
                              Geom::Point(doc->getWidth().value("px"), doc->getHeight().value("px")));

            // std::cout << "Using selection: PAGE" << std::endl;
            key = SELECTION_PAGE;
            break;
        case SELECTION_CUSTOM:
        default:
            break;
        } // switch

        current_key = key;

        // remember area setting
        prefs->setString("/dialogs/export/exportarea/value", selection_names[current_key]);

        if ( key != SELECTION_CUSTOM && bbox ) {
            setArea ( bbox->min()[Geom::X],
                      bbox->min()[Geom::Y],
                      bbox->max()[Geom::X],
                      bbox->max()[Geom::Y]);
        }

    } // end of if ( SP_ACTIVE_DESKTOP )

    if (SP_ACTIVE_DESKTOP && !filename_modified) {

        Glib::ustring filename;
        float xdpi = 0.0, ydpi = 0.0;

        switch (key) {
        case SELECTION_PAGE:
        case SELECTION_DRAWING: {
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            sp_document_get_export_hints (doc, filename, &xdpi, &ydpi);

            if (filename.empty()) {
                if (!doc_export_name.empty()) {
                    filename = doc_export_name;
                }
            }
            break;
        }
        case SELECTION_SELECTION:
            if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {

                sp_selection_get_export_hints (sp_desktop_selection(SP_ACTIVE_DESKTOP), filename, &xdpi, &ydpi);

                /* If we still don't have a filename -- let's build
                   one that's nice */
                if (filename.empty()) {
                    const gchar * id = "object";
                    const GSList * reprlst = sp_desktop_selection(SP_ACTIVE_DESKTOP)->reprList();
                    for(; reprlst != NULL; reprlst = reprlst->next) {
                        Inkscape::XML::Node * repr = (Inkscape::XML::Node *)reprlst->data;
                        if (repr->attribute("id")) {
                            id = repr->attribute("id");
                            break;
                        }
                    }

                    filename = create_filepath_from_id (id, filename_entry.get_text());
                }
            }
            break;
        case SELECTION_CUSTOM:
        default:
            break;
        }

        if (!filename.empty()) {
            original_name = filename;
            filename_entry.set_text(filename);
            filename_entry.set_position(filename.length());
        }

        if (xdpi != 0.0) {
            setValue(xdpi_adj, xdpi);
        }

        /* These can't be separate, and setting x sets y, so for
           now setting this is disabled.  Hopefully it won't be in
           the future */
        if (FALSE && ydpi != 0.0) {
            setValue(ydpi_adj, ydpi);
        }
    }

    return;
} // end of sp_export_area_toggled()

/// Called when dialog is deleted
bool Export::onProgressDelete (GdkEventAny * /*event*/)
{
    interrupted = true;
    return TRUE;
} // end of sp_export_progress_delete()


/// Called when progress is cancelled
void Export::onProgressCancel ()
{
    interrupted = true;
} // end of sp_export_progress_cancel()


/// Called for every progress iteration
unsigned int Export::onProgressCallback(float value, void *dlg)
{
    Gtk::Dialog *dlg2 = reinterpret_cast<Gtk::Dialog*>(dlg);
    if (dlg2->get_data("cancel")) {
        return FALSE;
    }

    gint current = GPOINTER_TO_INT(dlg2->get_data("current"));
    gint total = GPOINTER_TO_INT(dlg2->get_data("total"));
    if (total > 0) {
        double completed = current;
        completed /= static_cast<double>(total);

        value = completed + (value / static_cast<double>(total));
    }

    Gtk::ProgressBar *prg = reinterpret_cast<Gtk::ProgressBar *>(dlg2->get_data("progress"));
    prg->set_fraction(value);

    Export *self = reinterpret_cast<Export *>(dlg2->get_data("exportPanel"));
    if (self) {
        self->_prog.set_fraction(value);
    }

    int evtcount = 0;
    while ((evtcount < 16) && gdk_events_pending()) {
        gtk_main_iteration_do(FALSE);
        evtcount += 1;
    }

    gtk_main_iteration_do(FALSE);
    return TRUE;
} // end of sp_export_progress_callback()

void Export::setExporting(bool exporting, Glib::ustring const &text)
{
    if (exporting) {
        _prog.set_text(text);
        _prog.set_fraction(0.0);
        _prog.set_sensitive(true);

        export_button.set_sensitive(false);
    } else {
        _prog.set_text("");
        _prog.set_fraction(0.0);
        _prog.set_sensitive(false);

        export_button.set_sensitive(true);
    }
}

Gtk::Dialog * Export::create_progress_dialog (Glib::ustring progress_text) {
    Gtk::Dialog *dlg = new Gtk::Dialog(_("Export in progress"), TRUE);

    Gtk::ProgressBar *prg = new Gtk::ProgressBar ();
    prg->set_text(progress_text);
    dlg->set_data ("progress", prg);
#if GTK_CHECK_VERSION(3,0,0)
    Gtk::Box* CA = dlg->get_content_area();
#else
    Gtk::Box* CA = dlg->get_vbox();
#endif
    CA->pack_start(*prg, FALSE, FALSE, 4);

    Gtk::Button* btn = dlg->add_button (Gtk::Stock::CANCEL,Gtk::RESPONSE_CANCEL );

    btn->signal_clicked().connect( sigc::mem_fun(*this, &Export::onProgressCancel) );
    dlg->signal_delete_event().connect( sigc::mem_fun(*this, &Export::onProgressDelete) );

    dlg->show_all ();
    return dlg;
}

// FIXME: Some lib function should be available to do this ...
Glib::ustring Export::filename_add_extension (Glib::ustring filename, Glib::ustring extension)
{
    Glib::ustring::size_type dot;

    dot = filename.find_last_of(".");
    if ( !dot )
    {
        return filename = filename + "." + extension;
    }
    else
    {
        if (dot==filename.find_last_of(Glib::ustring::compose(".", extension)))
        {
            return filename;
        }
        else
        {
            return filename = filename + "." + extension;
        }
    }
}

Glib::ustring Export::absolutize_path_from_document_location (SPDocument *doc, const Glib::ustring &filename)
{
    Glib::ustring path;
    //Make relative paths go from the document location, if possible:
    if (!Glib::path_is_absolute(filename) && doc->getURI()) {
        Glib::ustring dirname = Glib::path_get_dirname(doc->getURI());
        if (!dirname.empty()) {
            path = Glib::build_filename(dirname, filename);
        }
    }
    if (path.empty()) {
        path = filename;
    }
    return path;
}

// Called when unit is changed
void Export::onUnitChanged()
{
    onAreaToggled();
}

void Export::onHideExceptSelected ()
{
    prefs->setBool("/dialogs/export/hideexceptselected/value", hide_export.get_active());
}

/// Called when export button is clicked
void Export::onExport ()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return;

    SPNamedView *nv = sp_desktop_namedview(desktop);
    SPDocument *doc = sp_desktop_document (desktop);

    bool exportSuccessful = false;

    bool hide = hide_export.get_active ();
    if (batch_export.get_active ()) {
        // Batch export of selected objects

        gint num = g_slist_length(const_cast<GSList *>(sp_desktop_selection(desktop)->itemList()));
        gint n = 0;

        if (num < 1) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No items selected."));
            return;
        }

        prog_dlg = create_progress_dialog(Glib::ustring::compose(_("Exporting %1 files"), num));
        prog_dlg->set_data("exportPanel", this);
        setExporting(true, Glib::ustring::compose(_("Exporting %1 files"), num));

        gint export_count = 0;

        for (GSList *i = const_cast<GSList *>(sp_desktop_selection(desktop)->itemList()); i && !interrupted; i = i->next) {
            SPItem *item = reinterpret_cast<SPItem *>(i->data);

            prog_dlg->set_data("current", GINT_TO_POINTER(n));
            prog_dlg->set_data("total", GINT_TO_POINTER(num));
            onProgressCallback(0.0, prog_dlg);

            // retrieve export filename hint
            const gchar *filename = item->getRepr()->attribute("inkscape:export-filename");
            Glib::ustring path;
            if (!filename) {
                Glib::ustring tmp;
                path = create_filepath_from_id(item->getId(), tmp);
            } else {
                path = absolutize_path_from_document_location(doc, filename);
            }

            // retrieve export dpi hints
            const gchar *dpi_hint = item->getRepr()->attribute("inkscape:export-xdpi"); // only xdpi, ydpi is always the same now
            gdouble dpi = 0.0;
            if (dpi_hint) {
                dpi = atof(dpi_hint);
            }
            if (dpi == 0.0) {
                dpi = getValue(xdpi_adj);
            }

            Geom::OptRect area = item->desktopVisualBounds();
            if (area) {
                gint width = (gint) (area->width() * dpi / DPI_BASE + 0.5);
                gint height = (gint) (area->height() * dpi / DPI_BASE + 0.5);

                if (width > 1 && height > 1) {
                    // Do export
                    gchar * safeFile = Inkscape::IO::sanitizeString(path.c_str());
                    MessageCleaner msgCleanup(desktop->messageStack()->pushF(Inkscape::IMMEDIATE_MESSAGE,
                                              _("Exporting file <b>%s</b>..."), safeFile), desktop);
                    MessageCleaner msgFlashCleanup(desktop->messageStack()->flashF(Inkscape::IMMEDIATE_MESSAGE,
                                                   _("Exporting file <b>%s</b>..."), safeFile), desktop);

                    if (!sp_export_png_file (doc, path.c_str(),
                                             *area, width, height, dpi, dpi,
                                             nv->pagecolor,
                                             onProgressCallback, (void*)prog_dlg,
                                             TRUE,  // overwrite without asking
                                             hide ? const_cast<GSList *>(sp_desktop_selection(desktop)->itemList()) : NULL
                                            )) {
                        gchar * error = g_strdup_printf(_("Could not export to filename %s.\n"), safeFile);

                        desktop->messageStack()->flashF(Inkscape::ERROR_MESSAGE,
                                                        _("Could not export to filename <b>%s</b>."), safeFile);

                        sp_ui_error_dialog(error);
                        g_free(error);
                    } else {
                        ++export_count; // one more item exported successfully
                    }
                    g_free(safeFile);
                }
            }

            n++;
        }

        desktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE,
                                        _("Successfully exported <b>%d</b> files from <b>%d</b> selected items."), export_count, num);

        setExporting(false);
        delete prog_dlg;
        prog_dlg = NULL;
        interrupted = false;
        exportSuccessful = (export_count > 0);
    } else {
        Glib::ustring filename = filename_entry.get_text();

        if (filename.empty()) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You have to enter a filename."));
            sp_ui_error_dialog(_("You have to enter a filename"));
            return;
        }

        float const x0 = getValuePx(x0_adj);
        float const y0 = getValuePx(y0_adj);
        float const x1 = getValuePx(x1_adj);
        float const y1 = getValuePx(y1_adj);
        float const xdpi = getValue(xdpi_adj);
        float const ydpi = getValue(ydpi_adj);
        unsigned long int const width = int(getValue(bmwidth_adj) + 0.5);
        unsigned long int const height = int(getValue(bmheight_adj) + 0.5);

        if (!((x1 > x0) && (y1 > y0) && (width > 0) && (height > 0))) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("The chosen area to be exported is invalid."));
            sp_ui_error_dialog(_("The chosen area to be exported is invalid"));
            return;
        }

        // make sure that .png is the extension of the file:
        Glib::ustring const filename_ext = filename_add_extension(filename, "png");
        filename_entry.set_text(filename_ext);
        filename_entry.set_position(filename_ext.length());
        Glib::ustring path = absolutize_path_from_document_location(doc, filename_ext);

        Glib::ustring dirname = Glib::path_get_dirname(path);
        if ( dirname.empty()
                || !Inkscape::IO::file_test(dirname.c_str(), (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) )
        {
            gchar *safeDir = Inkscape::IO::sanitizeString(dirname.c_str());
            gchar *error = g_strdup_printf(_("Directory %s does not exist or is not a directory.\n"),
                                           safeDir);

            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error);
            sp_ui_error_dialog(error);

            g_free(safeDir);
            g_free(error);
            return;
        }

        Glib::ustring fn = path_get_basename (path);

        /* TRANSLATORS: %1 will be the filename, %2 the width, and %3 the height of the image */
        prog_dlg = create_progress_dialog (Glib::ustring::compose(_("Exporting %1 (%2 x %3)"), fn, width, height));
        prog_dlg->set_data("exportPanel", this);
        setExporting(true, Glib::ustring::compose(_("Exporting %1 (%2 x %3)"), fn, width, height));

        prog_dlg->set_data("current", GINT_TO_POINTER(0));
        prog_dlg->set_data("total", GINT_TO_POINTER(0));

        /* Do export */
        ExportResult status = sp_export_png_file(sp_desktop_document(desktop), path.c_str(),
                              Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1)), width, height, xdpi, ydpi,
                              nv->pagecolor,
                              onProgressCallback, (void*)prog_dlg,
                              FALSE,
                              hide ? const_cast<GSList *>(sp_desktop_selection(desktop)->itemList()) : NULL
                                                );
        if (status == EXPORT_ERROR) {
            gchar * safeFile = Inkscape::IO::sanitizeString(path.c_str());
            gchar * error = g_strdup_printf(_("Could not export to filename %s.\n"), safeFile);

            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, error);
            sp_ui_error_dialog(error);

            g_free(safeFile);
            g_free(error);
        } else if (status == EXPORT_OK) {
            exportSuccessful = true;
            gchar *safeFile = Inkscape::IO::sanitizeString(path.c_str());

            desktop->messageStack()->flashF(Inkscape::INFORMATION_MESSAGE, _("Drawing exported to <b>%s</b>."), safeFile);

            g_free(safeFile);
        } else {
            desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Export aborted."));
        }

        /* Reset the filename so that it can be changed again by changing
           selections and all that */
        original_name = filename_ext;
        filename_modified = false;

        setExporting(false);
        delete prog_dlg;
        prog_dlg = NULL;
        interrupted = false;

        /* Setup the values in the document */
        switch (current_key) {
        case SELECTION_PAGE:
        case SELECTION_DRAWING: {
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            Inkscape::XML::Node * repr = doc->getReprRoot();
            bool modified = false;

            bool saved = DocumentUndo::getUndoSensitive(doc);
            DocumentUndo::setUndoSensitive(doc, false);

            gchar const *temp_string = repr->attribute("inkscape:export-filename");
            if (temp_string == NULL || (filename_ext != temp_string)) {
                repr->setAttribute("inkscape:export-filename", filename_ext.c_str());
                modified = true;
            }
            temp_string = repr->attribute("inkscape:export-xdpi");
            if (temp_string == NULL || xdpi != atof(temp_string)) {
                sp_repr_set_svg_double(repr, "inkscape:export-xdpi", xdpi);
                modified = true;
            }
            temp_string = repr->attribute("inkscape:export-ydpi");
            if (temp_string == NULL || ydpi != atof(temp_string)) {
                sp_repr_set_svg_double(repr, "inkscape:export-ydpi", ydpi);
                modified = true;
            }
            DocumentUndo::setUndoSensitive(doc, saved);

            if (modified) {
                doc->setModifiedSinceSave();
            }
            break;
        }
        case SELECTION_SELECTION: {
            const GSList * reprlst;
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            bool modified = false;

            bool saved = DocumentUndo::getUndoSensitive(doc);
            DocumentUndo::setUndoSensitive(doc, false);
            reprlst = sp_desktop_selection(desktop)->reprList();

            for(; reprlst != NULL; reprlst = reprlst->next) {
                Inkscape::XML::Node * repr = static_cast<Inkscape::XML::Node *>(reprlst->data);
                const gchar * temp_string;
                Glib::ustring dir = Glib::path_get_dirname(filename.c_str());
                const gchar* docURI=SP_ACTIVE_DOCUMENT->getURI();
                Glib::ustring docdir;
                if (docURI)
                {
                    docdir = Glib::path_get_dirname(docURI);
                }
                if (repr->attribute("id") == NULL ||
                        !(filename_ext.find_last_of(repr->attribute("id")) &&
                          ( !docURI ||
                            (dir == docdir)))) {
                    temp_string = repr->attribute("inkscape:export-filename");
                    if (temp_string == NULL || (filename_ext != temp_string)) {
                        repr->setAttribute("inkscape:export-filename", filename_ext.c_str());
                        modified = true;
                    }
                }
                temp_string = repr->attribute("inkscape:export-xdpi");
                if (temp_string == NULL || xdpi != atof(temp_string)) {
                    sp_repr_set_svg_double(repr, "inkscape:export-xdpi", xdpi);
                    modified = true;
                }
                temp_string = repr->attribute("inkscape:export-ydpi");
                if (temp_string == NULL || ydpi != atof(temp_string)) {
                    sp_repr_set_svg_double(repr, "inkscape:export-ydpi", ydpi);
                    modified = true;
                }
            }
            DocumentUndo::setUndoSensitive(doc, saved);

            if (modified) {
                doc->setModifiedSinceSave();
            }
            break;
        }
        default:
            break;
        }
    }

    if (exportSuccessful && closeWhenDone.get_active()) {
        for ( Gtk::Container *parent = get_parent(); parent; parent = parent->get_parent()) {
            if ( GDL_IS_DOCK_ITEM(parent->gobj()) ) {
                GdlDockItem *item = GDL_DOCK_ITEM(parent->gobj());
                if (item) {
                    gdl_dock_item_hide_item(item);
                }
                break;
            }
        }
    }
} // end of sp_export_export_clicked()

/// Called when Browse button is clicked
/// @todo refactor this code to use ui/dialogs/filedialog.cpp
void Export::onBrowse ()
{
    GtkWidget *fs;
    Glib::ustring filename;

    fs = gtk_file_chooser_dialog_new (_("Select a filename for exporting"),
                                      (GtkWindow*)desktop->getToplevel(),
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
                                      _("_Save"),   GTK_RESPONSE_ACCEPT,
                                      NULL );

#ifdef WITH_GNOME_VFS
    if (gnome_vfs_initialized()) {
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(fs), false);
    }
#endif

    sp_transientize (fs);

    gtk_window_set_modal(GTK_WINDOW (fs), true);

    filename = filename_entry.get_text();

    if (filename.empty()) {
        Glib::ustring tmp;
        filename = create_filepath_from_id(tmp, tmp);
    }

    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fs), filename.c_str());

#ifdef WIN32
    // code in this section is borrowed from ui/dialogs/filedialogimpl-win32.cpp
    OPENFILENAMEW opf;
    WCHAR filter_string[20];
    wcsncpy(filter_string, L"PNG#*.png##", 11);
    filter_string[3] = L'\0';
    filter_string[9] = L'\0';
    filter_string[10] = L'\0';
    WCHAR* title_string = (WCHAR*)g_utf8_to_utf16(_("Select a filename for exporting"), -1, NULL, NULL, NULL);
    WCHAR* extension_string = (WCHAR*)g_utf8_to_utf16("*.png", -1, NULL, NULL, NULL);
    // Copy the selected file name, converting from UTF-8 to UTF-16
    std::string dirname = Glib::path_get_dirname(filename.raw());
    if ( !Glib::file_test(dirname, Glib::FILE_TEST_EXISTS) ||
            Glib::file_test(filename, Glib::FILE_TEST_IS_DIR) ||
            dirname.empty() )
    {
        Glib::ustring tmp;
        filename = create_filepath_from_id(tmp, tmp);
    }
    WCHAR _filename[_MAX_PATH + 1];
    memset(_filename, 0, sizeof(_filename));
    gunichar2* utf16_path_string = g_utf8_to_utf16(filename.c_str(), -1, NULL, NULL, NULL);
    wcsncpy(_filename, reinterpret_cast<wchar_t*>(utf16_path_string), _MAX_PATH);
    g_free(utf16_path_string);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Glib::RefPtr<const Gdk::Window> parentWindow = desktop->getToplevel()->get_window();
    g_assert(parentWindow->gobj() != NULL);

#if WITH_GTKMM_3_0
    opf.hwndOwner = (HWND)gdk_win32_window_get_handle((GdkWindow*)parentWindow->gobj());
#else
    opf.hwndOwner = (HWND)gdk_win32_drawable_get_handle((GdkDrawable*)parentWindow->gobj());
#endif
    opf.lpstrFilter = filter_string;
    opf.lpstrCustomFilter = 0;
    opf.nMaxCustFilter = 0L;
    opf.nFilterIndex = 1L;
    opf.lpstrFile = _filename;
    opf.nMaxFile = _MAX_PATH;
    opf.lpstrFileTitle = NULL;
    opf.nMaxFileTitle=0;
    opf.lpstrInitialDir = 0;
    opf.lpstrTitle = title_string;
    opf.nFileOffset = 0;
    opf.nFileExtension = 2;
    opf.lpstrDefExt = extension_string;
    opf.lpfnHook = NULL;
    opf.lCustData = 0;
    opf.Flags = OFN_PATHMUSTEXIST;
    opf.lStructSize = sizeof(OPENFILENAMEW);
    if (GetSaveFileNameW(&opf) != 0)
    {
        // Copy the selected file name, converting from UTF-16 to UTF-8
        gchar *utf8string = g_utf16_to_utf8((const gunichar2*)opf.lpstrFile, _MAX_PATH, NULL, NULL, NULL);
        filename_entry.set_text(utf8string);
        filename_entry.set_position(strlen(utf8string));
        g_free(utf8string);

    }
    g_free(extension_string);
    g_free(title_string);

#else
    if (gtk_dialog_run (GTK_DIALOG (fs)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *file;

        file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fs));

        gchar * utf8file = g_filename_to_utf8( file, -1, NULL, NULL, NULL );
        filename_entry.set_text (utf8file);
        filename_entry.set_position(strlen(utf8file));

        g_free(utf8file);
        g_free(file);
    }
#endif

    gtk_widget_destroy (fs);

    return;
} // end of sp_export_browse_clicked()

// TODO: Move this to nr-rect-fns.h.
bool Export::bbox_equal(Geom::Rect const &one, Geom::Rect const &two)
{
    double const epsilon = pow(10.0, -EXPORT_COORD_PRECISION);
    return (
               (fabs(one.min()[Geom::X] - two.min()[Geom::X]) < epsilon) &&
               (fabs(one.min()[Geom::Y] - two.min()[Geom::Y]) < epsilon) &&
               (fabs(one.max()[Geom::X] - two.max()[Geom::X]) < epsilon) &&
               (fabs(one.max()[Geom::Y] - two.max()[Geom::Y]) < epsilon)
           );
}

/**
 *This function is used to detect the current selection setting
 * based on the values in the x0, y0, x1 and y0 fields.
 *
 * One of the most confusing parts of this function is why the array
 * is built at the beginning.  What needs to happen here is that we
 * should always check the current selection to see if it is the valid
 * one.  While this is a performance improvement it is also a usability
 * one during the cases where things like selections and drawings match
 * size.  This way buttons change less 'randomly' (atleast in the eyes
 * of the user).  To do this an array is built where the current selection
 * type is placed first, and then the others in an order from smallest
 * to largest (this can be configured by reshuffling \c test_order).
 *
 * All of the values in this function are rounded to two decimal places
 * because that is what is shown to the user.  While everything is kept
 * more accurate than that, the user can't control more acurrate than
 * that, so for this to work for them - it needs to check on that level
 * of accuracy.
 *
 * @todo finish writing this up.
 */
void Export::detectSize() {
    static const selection_type test_order[SELECTION_NUMBER_OF] = {SELECTION_SELECTION, SELECTION_DRAWING, SELECTION_PAGE, SELECTION_CUSTOM};
    selection_type this_test[SELECTION_NUMBER_OF + 1];
    selection_type key = SELECTION_NUMBER_OF;

    Geom::Point x(getValuePx(x0_adj),
                  getValuePx(y0_adj));
    Geom::Point y(getValuePx(x1_adj),
                  getValuePx(y1_adj));
    Geom::Rect current_bbox(x, y);

    this_test[0] = current_key;
    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        this_test[i + 1] = test_order[i];
    }

    for (int i = 0;
            i < SELECTION_NUMBER_OF + 1 &&
            key == SELECTION_NUMBER_OF &&
            SP_ACTIVE_DESKTOP != NULL;
            i++) {
        switch (this_test[i]) {
        case SELECTION_SELECTION:
            if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
                Geom::OptRect bbox = (sp_desktop_selection (SP_ACTIVE_DESKTOP))->bounds(SPItem::VISUAL_BBOX);

                if ( bbox && bbox_equal(*bbox,current_bbox)) {
                    key = SELECTION_SELECTION;
                }
            }
            break;
        case SELECTION_DRAWING: {
            SPDocument *doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

            Geom::OptRect bbox = doc->getRoot()->desktopVisualBounds();

            if ( bbox && bbox_equal(*bbox,current_bbox) ) {
                key = SELECTION_DRAWING;
            }
            break;
        }

        case SELECTION_PAGE: {
            SPDocument *doc;

            doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

            Geom::Point x(0.0, 0.0);
            Geom::Point y(doc->getWidth().value("px"),
                          doc->getHeight().value("px"));
            Geom::Rect bbox(x, y);

            if (bbox_equal(bbox,current_bbox)) {
                key = SELECTION_PAGE;
            }

            break;
        }
        default:
            break;
        }
    }
    // std::cout << std::endl;

    if (key == SELECTION_NUMBER_OF) {
        key = SELECTION_CUSTOM;
    }

    current_key = key;
    selectiontype_buttons[current_key]->set_active(true);

    return;
} /* sp_export_detect_size */

/// Called when area x0 value is changed
#if WITH_GTKMM_3_0
void Export::areaXChange(Glib::RefPtr<Gtk::Adjustment>& adj)
#else
void Export::areaXChange (Gtk::Adjustment *adj)
#endif
{
    float x0, x1, xdpi, width;

    if (update) {
        return;
    }

    update = true;

    x0 = getValuePx(x0_adj);
    x1 = getValuePx(x1_adj);
    xdpi = getValue(xdpi_adj);

    width = floor ((x1 - x0) * xdpi / DPI_BASE + 0.5);

    if (width < SP_EXPORT_MIN_SIZE) {
        width = SP_EXPORT_MIN_SIZE;

        if (adj == x1_adj) {
            x1 = x0 + width * DPI_BASE / xdpi;
            setValuePx(x1_adj, x1);
        } else {
            x0 = x1 - width * DPI_BASE / xdpi;
            setValuePx(x0_adj, x0);
        }
    }

    setValuePx(width_adj, x1 - x0);
    setValue(bmwidth_adj, width);

    detectSize();

    update = false;

    return;
} // end of sp_export_area_x_value_changed()

/// Called when area y0 value is changed.
#if WITH_GTKMM_3_0
void Export::areaYChange(Glib::RefPtr<Gtk::Adjustment>& adj)
#else
void Export::areaYChange (Gtk::Adjustment *adj)
#endif
{
    float y0, y1, ydpi, height;

    if (update) {
        return;
    }

    update = true;

    y0 = getValuePx(y0_adj);
    y1 = getValuePx(y1_adj);
    ydpi = getValue(ydpi_adj);

    height = floor ((y1 - y0) * ydpi / DPI_BASE + 0.5);

    if (height < SP_EXPORT_MIN_SIZE) {
        //const gchar *key;
        height = SP_EXPORT_MIN_SIZE;
        //key = (const gchar *)g_object_get_data(G_OBJECT (adj), "key");
        if (adj == y1_adj) {
            //if (!strcmp (key, "y0")) {
            y1 = y0 + height * DPI_BASE / ydpi;
            setValuePx(y1_adj, y1);
        } else {
            y0 = y1 - height * DPI_BASE / ydpi;
            setValuePx(y0_adj, y0);
        }
    }

    setValuePx(height_adj, y1 - y0);
    setValue(bmheight_adj, height);

    detectSize();

    update = false;

    return;
} // end of sp_export_area_y_value_changed()

/// Called when x1-x0 or area width is changed
void Export::onAreaWidthChange()
{
    if (update) {
        return;
    }

    update = true;

    float x0 = getValuePx(x0_adj);
    float xdpi = getValue(xdpi_adj);
    float width = getValuePx(width_adj);
    float bmwidth = floor(width * xdpi / DPI_BASE + 0.5);

    if (bmwidth < SP_EXPORT_MIN_SIZE) {

        bmwidth = SP_EXPORT_MIN_SIZE;
        width = bmwidth * DPI_BASE / xdpi;
        setValuePx(width_adj, width);
    }

    setValuePx(x1_adj, x0 + width);
    setValue(bmwidth_adj, bmwidth);

    update = false;

    return;
} // end of sp_export_area_width_value_changed()

/// Called when y1-y0 or area height is changed.
void Export::onAreaHeightChange()
{
    if (update) {
        return;
    }

    update = true;

    float y0 = getValuePx(y0_adj);
    //float y1 = sp_export_value_get_px(y1_adj);
    float ydpi = getValue(ydpi_adj);
    float height = getValuePx(height_adj);
    float bmheight = floor (height * ydpi / DPI_BASE + 0.5);

    if (bmheight < SP_EXPORT_MIN_SIZE) {
        bmheight = SP_EXPORT_MIN_SIZE;
        height = bmheight * DPI_BASE / ydpi;
        setValuePx(height_adj, height);
    }

    setValuePx(y1_adj, y0 + height);
    setValue(bmheight_adj, bmheight);

    update = false;

    return;
} // end of sp_export_area_height_value_changed()

/**
 * A function to set the ydpi.
 * @param base The export dialog.
 *
 * This function grabs all of the y values and then figures out the
 * new bitmap size based on the changing dpi value.  The dpi value is
 * gotten from the xdpi setting as these can not currently be independent.
 */
void Export::setImageY()
{
    float y0, y1, xdpi;

    y0 = getValuePx(y0_adj);
    y1 = getValuePx(y1_adj);
    xdpi = getValue(xdpi_adj);

    setValue(ydpi_adj, xdpi);
    setValue(bmheight_adj, (y1 - y0) * xdpi / DPI_BASE);

    return;
} // end of setImageY()

/**
 * A function to set the xdpi.
 *
 * This function grabs all of the x values and then figures out the
 * new bitmap size based on the changing dpi value.  The dpi value is
 * gotten from the xdpi setting as these can not currently be independent.
 *
 */
void Export::setImageX()
{
    float x0, x1, xdpi;

    x0 = getValuePx(x0_adj);
    x1 = getValuePx(x1_adj);
    xdpi = getValue(xdpi_adj);

    setValue(ydpi_adj, xdpi);
    setValue(bmwidth_adj, (x1 - x0) * xdpi / DPI_BASE);

    return;
} // end of setImageX()

/// Called when pixel width is changed
void Export::onBitmapWidthChange ()
{
    float x0, x1, bmwidth, xdpi;

    if (update) {
        return;
    }

    update = true;

    x0 = getValuePx(x0_adj);
    x1 = getValuePx(x1_adj);
    bmwidth = getValue(bmwidth_adj);

    if (bmwidth < SP_EXPORT_MIN_SIZE) {
        bmwidth = SP_EXPORT_MIN_SIZE;
        setValue(bmwidth_adj, bmwidth);
    }

    xdpi = bmwidth * DPI_BASE / (x1 - x0);
    setValue(xdpi_adj, xdpi);

    setImageY ();

    update = false;

    return;
} // end of sp_export_bitmap_width_value_changed()

/// Called when pixel height is changed
void Export::onBitmapHeightChange ()
{
    float y0, y1, bmheight, xdpi;

    if (update) {
        return;
    }

    update = true;

    y0 = getValuePx(y0_adj);
    y1 = getValuePx(y1_adj);
    bmheight = getValue(bmheight_adj);

    if (bmheight < SP_EXPORT_MIN_SIZE) {
        bmheight = SP_EXPORT_MIN_SIZE;
        setValue(bmheight_adj, bmheight);
    }

    xdpi = bmheight * DPI_BASE / (y1 - y0);
    setValue(xdpi_adj, xdpi);

    setImageX ();

    update = false;

    return;
} // end of sp_export_bitmap_width_value_changed()

/**
 * A function to adjust the bitmap width when the xdpi value changes.
 *
 * The first thing this function checks is to see if we are doing an
 * update.  If we are, this function just returns because there is another
 * instance of it that will handle everything for us.  If there is a
 * units change, we also assume that everyone is being updated appropriately
 * and there is nothing for us to do.
 *
 * If we're the highest level function, we set the update flag, and
 * continue on our way.
 *
 * All of the values are grabbed using the \c sp_export_value_get functions
 * (call to the _pt ones for x0 and x1 but just standard for xdpi).  The
 * xdpi value is saved in the preferences for the next time the dialog
 * is opened.  (does the selection dpi need to be set here?)
 *
 * A check is done to to ensure that we aren't outputing an invalid width,
 * this is set by SP_EXPORT_MIN_SIZE.  If that is the case the dpi is
 * changed to make it valid.
 *
 * After all of this the bitmap width is changed.
 *
 * We also change the ydpi.  This is a temporary hack as these can not
 * currently be independent.  This is likely to change in the future.
 *
 */
void Export::onExportXdpiChange()
{
    float x0, x1, xdpi, bmwidth;

    if (update) {
        return;
    }

    update = true;

    x0 = getValuePx(x0_adj);
    x1 = getValuePx(x1_adj);
    xdpi = getValue(xdpi_adj);

    // remember xdpi setting
    prefs->setDouble("/dialogs/export/defaultxdpi/value", xdpi);

    bmwidth = (x1 - x0) * xdpi / DPI_BASE;

    if (bmwidth < SP_EXPORT_MIN_SIZE) {
        bmwidth = SP_EXPORT_MIN_SIZE;
        if (x1 != x0)
            xdpi = bmwidth * DPI_BASE / (x1 - x0);
        else
            xdpi = DPI_BASE;
        setValue(xdpi_adj, xdpi);
    }

    setValue(bmwidth_adj, bmwidth);

    setImageY ();

    update = false;

    return;
} // end of sp_export_xdpi_value_changed()


/**
 * A function to change the area that is used for the exported.
 * bitmap.
 *
 * This function just calls \c sp_export_value_set_px for each of the
 * parameters that is passed in.  This allows for setting them all in
 * one convient area.
 *
 * Update is set to suspend all of the other test running while all the
 * values are being set up.  This allows for a performance increase, but
 * it also means that the wrong type won't be detected with only some of
 * the values set.  After all the values are set everyone is told that
 * there has been an update.
 *
 * @param  x0    Horizontal upper left hand corner of the picture in points.
 * @param  y0    Vertical upper left hand corner of the picture in points.
 * @param  x1    Horizontal lower right hand corner of the picture in points.
 * @param  y1    Vertical lower right hand corner of the picture in points.
 */
void Export::setArea( double x0, double y0, double x1, double y1 )
{
    update = true;
    setValuePx(x1_adj, x1);
    setValuePx(y1_adj, y1);
    setValuePx(x0_adj, x0);
    setValuePx(y0_adj, y0);
    update = false;

    areaXChange (x1_adj);
    areaYChange (y1_adj);

    return;
}

/**
 * Sets the value of an adjustment.
 *
 * @param  adj   The adjustment widget
 * @param  val   What value to set it to.
 */
#if WITH_GTKMM_3_0
void Export::setValue(Glib::RefPtr<Gtk::Adjustment>& adj, double val )
#else
void Export::setValue(  Gtk::Adjustment *adj, double val )
#endif
{
    if (adj) {
        adj->set_value(val);
    }
}

/**
 * A function to set a value using the units points.
 *
 * This function first gets the adjustment for the key that is passed
 * in.  It then figures out what units are currently being used in the
 * dialog.  After doing all of that, it then converts the incoming
 *value and sets the adjustment.
 *
 * @param  adj   The adjustment widget
 * @param  val   What the value should be in points.
 */
#if WITH_GTKMM_3_0
void Export::setValuePx(Glib::RefPtr<Gtk::Adjustment>& adj, double val)
#else
void Export::setValuePx( Gtk::Adjustment *adj, double val)
#endif
{
    Unit const *unit = unit_selector.getUnit();

    setValue(adj, Inkscape::Util::Quantity::convert(val, "px", unit));

    return;
}

/**
 * Get the value of an adjustment in the export dialog.
 *
 * This function gets the adjustment from the data field in the export
 * dialog.  It then grabs the value from the adjustment.
 *
 * @param  adj   The adjustment widget
 *
 * @return The value in the specified adjustment.
 */
#if WITH_GTKMM_3_0
float Export::getValue(Glib::RefPtr<Gtk::Adjustment>& adj)
#else
float Export::getValue(  Gtk::Adjustment *adj )
#endif
{
    if (!adj) {
        g_message("sp_export_value_get : adj is NULL");
        return 0.0;
    }
    return adj->get_value();
}

/**
 * Grabs a value in the export dialog and converts the unit
 * to points.
 *
 * This function, at its most basic, is a call to \c sp_export_value_get
 * to get the value of the adjustment.  It then finds the units that
 * are being used by looking at the "units" attribute of the export
 * dialog.  Using that it converts the returned value into points.
 *
 * @param  adj   The adjustment widget
 *
 * @return The value in the adjustment in points.
 */
#if WITH_GTKMM_3_0
float Export::getValuePx(Glib::RefPtr<Gtk::Adjustment>& adj)
#else
float Export::getValuePx(  Gtk::Adjustment *adj )
#endif
{
    float value = getValue( adj);
    Unit const *unit = unit_selector.getUnit();

    return Inkscape::Util::Quantity::convert(value, unit, "px");
} // end of sp_export_value_get_px()

/**
 * This function is called when the filename is changed by
 * anyone.  It resets the virgin bit.
 *
 * This function gets called when the text area is modified.  It is
 * looking for the case where the text area is modified from its
 * original value.  In that case it sets the "filename-modified" bit
 * to TRUE.  If the text dialog returns back to the original text, the
 * bit gets reset.  This should stop simple mistakes.
 */
void Export::onFilenameModified()
{
    if (original_name == filename_entry.get_text()) {
        filename_modified = false;
    } else {
        filename_modified = true;
    }

    return;
} // end sp_export_filename_modified

}
}
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

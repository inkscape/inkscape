/**
 * @file
 * Open Clip Art Library integration dialogs - implementation.
 */
/* Authors:
 *   Bruno Dilly
 *   Other dudes from The Inkscape Organization
 *   Andrew Higginson
 *
 * Copyright (C) 2007 Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ocaldialogs.h"

#include <stdio.h>  // rename()
#include <unistd.h> // close()
#include <errno.h>  // errno
#include <string.h> // strerror()

#include "path-prefix.h"
#include "filedialogimpl-gtkmm.h"
#include "ui/interface.h"
#include "inkgc/gc-core.h"
#include "ui/dialog-events.h"
#include "io/sys.h"
#include "preferences.h"

#include <gtkmm/notebook.h>
#include <gtkmm/spinner.h>
#include <gtkmm/stock.h>
#include <gdkmm/general.h>
#include <libxml/tree.h>

#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>
#include <glibmm/main.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>
#include "ui/icon-names.h"

namespace Inkscape
{
namespace UI
{
namespace Dialog
{
namespace OCAL
{

//########################################################################
//# F I L E    E X P O R T   T O   O C A L
//########################################################################

/**
 * Callback for fileNameEntry widget
 */
/*
void ExportDialog::fileNameEntryChangedCallback()
{
    if (!fileNameEntry)
        return;

    Glib::ustring fileName = fileNameEntry->get_text();
    if (!Glib::get_charset()) //If we are not utf8
        fileName = Glib::filename_to_utf8(fileName);

    myFilename = fileName;
    response(Gtk::RESPONSE_OK);
}
*/
/**
 * Constructor
 */
/*
ExportDialog::ExportDialog(Gtk::Window &parentWindow,
            FileDialogType fileTypes,
            const Glib::ustring &title) :
    FileDialogBase(title, parentWindow)
{
*/
     /*
     * Start Taking the vertical Box and putting a Label
     * and a Entry to take the filename
     * Later put the extension selection and checkbox (?)
     */
    /* Initalize to Autodetect */
/*
    extension = NULL;
*/
    /* No filename to start out with */
/*
    myFilename = "";
*/
    /* Set our dialog type (save, export, etc...)*/
/*
    dialogType = fileTypes;
    Gtk::VBox *vbox = get_vbox();

    Gtk::Label *fileLabel = new Gtk::Label(_("File"));

    fileNameEntry = new Gtk::Entry();
    fileNameEntry->set_text(myFilename);
    fileNameEntry->set_max_length(252); // I am giving the extension approach.
    fileBox.pack_start(*fileLabel);
    fileBox.pack_start(*fileNameEntry, Gtk::PACK_EXPAND_WIDGET, 3);
    vbox->pack_start(fileBox);

    //Let's do some customization
    fileNameEntry = NULL;
    Gtk::Container *cont = get_toplevel();
    std::vector<Gtk::Entry *> entries;
    findEntryWidgets(cont, entries);
    if (entries.size() >=1 )
        {
        //Catch when user hits [return] on the text field
        fileNameEntry = entries[0];
        fileNameEntry->signal_activate().connect(
             sigc::mem_fun(*this, &ExportDialog::fileNameEntryChangedCallback) );
        }

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_OK));

    show_all_children();
}
*/
/**
 * Destructor
 */
/*
ExportDialog::~ExportDialog()
{
}
*/
/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
/*
bool
ExportDialog::show()
{
    set_modal (TRUE);                      //Window
    sp_transientize(GTK_WIDGET(gobj()));  //Make transient
    gint b = run();                        //Dialog
    hide();

    if (b == Gtk::RESPONSE_OK)
    {
        return TRUE;
        }
    else
        {
        return FALSE;
        }
}
*/
/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
/*
Glib::ustring
ExportDialog::get_filename()
{
    myFilename = fileNameEntry->get_text();
    if (!Glib::get_charset()) //If we are not utf8
        myFilename = Glib::filename_to_utf8(myFilename);

    return myFilename;
}


void
ExportDialog::change_title(const Glib::ustring& title)
{
    this->set_title(title);
}
*/

//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Constructor
 */
/*
ExportPasswordDialog::ExportPasswordDialog(Gtk::Window &parentWindow,
                             const Glib::ustring &title) : FileDialogBase(title, parentWindow)
{
*/
    /*
     * Start Taking the vertical Box and putting 2 Labels
     * and 2 Entries to take the username and password
     */
    /* No username and password to start out with */
/*
    myUsername = "";
    myPassword = "";

    Gtk::VBox *vbox = get_vbox();

    Gtk::Label *userLabel = new Gtk::Label(_("Username:"));
    Gtk::Label *passLabel = new Gtk::Label(_("Password:"));

    usernameEntry = new Gtk::Entry();
    usernameEntry->set_text(myUsername);
    usernameEntry->set_max_length(255);

    passwordEntry = new Gtk::Entry();
    passwordEntry->set_text(myPassword);
    passwordEntry->set_max_length(255);
    passwordEntry->set_invisible_char('*');
    passwordEntry->set_visibility(false);
    passwordEntry->set_activates_default(true);

    userBox.pack_start(*userLabel);
    userBox.pack_start(*usernameEntry, Gtk::PACK_EXPAND_WIDGET, 3);
    vbox->pack_start(userBox);

    passBox.pack_start(*passLabel);
    passBox.pack_start(*passwordEntry, Gtk::PACK_EXPAND_WIDGET, 3);
    vbox->pack_start(passBox);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::OK,   Gtk::RESPONSE_OK));

    show_all_children();
}
*/

/**
 * Destructor
 */
/*
ExportPasswordDialog::~ExportPasswordDialog()
{
}
*/
/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
/*
bool
ExportPasswordDialog::show()
{
    set_modal (TRUE);                      //Window
    sp_transientize(GTK_WIDGET(gobj()));  //Make transient
    gint b = run();                        //Dialog
    hide();

    if (b == Gtk::RESPONSE_OK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
*/
/**
 * Get the username.   Valid after an [OK]
 */
/*
Glib::ustring
ExportPasswordDialog::getUsername()
{
    myUsername = usernameEntry->get_text();
    return myUsername;
}
*/
/**
 * Get the password.   Valid after an [OK]
 */
/*
Glib::ustring
ExportPasswordDialog::getPassword()
{
    myPassword = passwordEntry->get_text();
    return myPassword;
}

void
ExportPasswordDialog::change_title(const Glib::ustring& title)
{
    this->set_title(title);
}
*/

//#########################################################################
//### F I L E   I M P O R T   F R O M   O C A L
//#########################################################################

WrapLabel::WrapLabel() : Gtk::Label()
{
    signal_size_allocate().connect(sigc::mem_fun(*this, &WrapLabel::_on_size_allocate));
}

void WrapLabel::_on_size_allocate(Gtk::Allocation& allocation)
{
    set_size_request(allocation.get_width(), -1);
}


LoadingBox::LoadingBox() : Gtk::EventBox()
{
    set_visible_window(false);
    draw_spinner = false;
    spinner_step = 0;

#if WITH_GTKMM_3_0
    signal_draw().connect(sigc::mem_fun(*this, &LoadingBox::_on_draw), false);
#else
    signal_expose_event().connect(sigc::mem_fun(*this, &LoadingBox::_on_expose_event), false);
#endif
}

#if !WITH_GTKMM_3_0
bool LoadingBox::_on_expose_event(GdkEventExpose* /*event*/)
{
    Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();

    return _on_draw(cr);
}
#endif

bool LoadingBox::_on_draw(const Cairo::RefPtr<Cairo::Context> &
#if WITH_GTKMM_3_0
cr
#endif
)
{
    // Draw shadow
    int x = get_allocation().get_x();
    int y = get_allocation().get_y();
    int width = get_allocation().get_width();
    int height = get_allocation().get_height();

#if WITH_GTKMM_3_0
    get_style_context()->render_frame(cr, x, y, width, height);
#else
    get_style()->paint_shadow(get_window(), get_state(), Gtk::SHADOW_IN,
        Gdk::Rectangle(x, y, width, height),
        *this, Glib::ustring("viewport"), x, y, width, height);
#endif

    if (draw_spinner) {
        int spinner_size = 16;
        int spinner_x = x + (width - spinner_size) / 2;
        int spinner_y = y + (height - spinner_size) / 2;

#if WITH_GTKMM_3_0
        get_style_context()->render_activity(cr, spinner_x, spinner_y, spinner_size, spinner_size);
#else
        gtk_paint_spinner(gtk_widget_get_style(GTK_WIDGET(gobj())),
            gtk_widget_get_window(GTK_WIDGET(gobj())),
            gtk_widget_get_state(GTK_WIDGET(gobj())), NULL, GTK_WIDGET(gobj()),
            NULL, spinner_step, spinner_x, spinner_y, spinner_size, spinner_size);
#endif
    }

    return false;
}

void LoadingBox::start()
{
    // Timeout hasn't been stopped, so must be disconnected
    if ((draw_spinner != false) && timeout) {
        timeout.disconnect();
    }
    
    draw_spinner = true;
    timeout = Glib::signal_timeout().connect(sigc::mem_fun(*this, &LoadingBox::on_timeout), 80);
}

void LoadingBox::stop()
{
    draw_spinner = false;
}

bool LoadingBox::on_timeout() {
    if (draw_spinner) {

        if (spinner_step == 11) {
            spinner_step = 0;
        } else {
            spinner_step ++;
        }
        
        queue_draw();
        return true;
    }
    return false;
}

PreviewWidget::PreviewWidget() : Gtk::VBox(false, 12)
{
    box_loading = new LoadingBox();
    image = new Gtk::Image();

    label_title = new WrapLabel();
    label_description = new WrapLabel();
    label_time = new WrapLabel();
    
    pack_start(*box_loading, false, false);
    pack_start(*image, false, false);
    pack_start(*label_title, false, false);
    pack_start(*label_description, false, false);
    pack_start(*label_time, false, false);

    label_title->set_line_wrap(true);
    label_title->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
    label_title->set_justify(Gtk::JUSTIFY_CENTER);
    label_description->set_line_wrap(true);
    label_description->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
    label_description->set_justify(Gtk::JUSTIFY_CENTER);
    label_time->set_line_wrap(true);
    label_time->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
    label_time->set_justify(Gtk::JUSTIFY_CENTER);

    box_loading->set_no_show_all(true);
    image->set_no_show_all(true);
    label_title->set_size_request(90, -1);
    label_description->set_size_request(90, -1);
    label_time->set_size_request(90, -1);
    box_loading->set_size_request(90, 90);
    set_border_width(12);

#if WITH_GTKMM_3_0
    signal_draw().connect(sigc::mem_fun(*this, &PreviewWidget::_on_draw), false);
#else 
    signal_expose_event().connect(sigc::mem_fun(*this, &PreviewWidget::_on_expose_event), false);
#endif

    clear();
}

void PreviewWidget::set_metadata(Glib::ustring description, Glib::ustring creator, 
    Glib::ustring time)
{
    label_title->set_markup(g_markup_printf_escaped("<b>%s</b>", description.c_str()));
    label_description->set_markup(g_markup_printf_escaped("%s", creator.c_str()));
    label_time->set_markup(g_markup_printf_escaped("<small>%s</small>", time.c_str()));

    show_box_loading();
}

void PreviewWidget::show_box_loading()
{
    box_loading->show();
    box_loading->start();
}

void PreviewWidget::hide_box_loading()
{
    box_loading->hide();
    box_loading->stop();
}

void PreviewWidget::set_image(std::string path)
{
    image->set(path);
    hide_box_loading();
    image->show();
}

void PreviewWidget::clear()
{
    label_title->set_markup("");
    label_description->set_markup("");
    label_time->set_markup("");

    box_loading->hide();
    image->hide();
}

#if !WITH_GTKMM_3_0
bool PreviewWidget::_on_expose_event(GdkEventExpose* /*event*/)
{
    Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();

    return _on_draw(cr);
}
#endif

bool PreviewWidget::_on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Draw background
    int x = get_allocation().get_x();
    int y = get_allocation().get_y();
    int width = get_allocation().get_width();
    int height = get_allocation().get_height();

#if WITH_GTKMM_3_0
    Gdk::RGBA background_fill;
    get_style_context()->lookup_color("base_color", background_fill);
    cr->rectangle(x, y, width, height);
    Gdk::Cairo::set_source_rgba(cr, background_fill);
#else
    Gdk::Color background_fill = get_style()->get_base(get_state());
    cr->rectangle(x, y, width, height);
    Gdk::Cairo::set_source_color(cr, background_fill);
#endif
    
    cr->fill();

    return false;
}

StatusWidget::StatusWidget() : Gtk::HBox(false, 6)
{
    image = new Gtk::Image(Gtk::Stock::DIALOG_ERROR, Gtk::ICON_SIZE_MENU);
    spinner = new Gtk::Spinner();
    label = new Gtk::Label();

    image->set_no_show_all(true);
    spinner->set_no_show_all(true);
    label->set_no_show_all(true);

    pack_start(*image, false, false);
    pack_start(*spinner, false, false);
    pack_start(*label, false, false);
}

void StatusWidget::clear()
{
    spinner->hide();
    image->hide();
    label->hide();
}

void StatusWidget::set_info(Glib::ustring text)
{
    spinner->hide();
    image->show();
    label->show();
    image->set(Gtk::Stock::DIALOG_INFO,  Gtk::ICON_SIZE_MENU);
    label->set_text(text);
}

void StatusWidget::set_error(Glib::ustring text)
{
    spinner->hide();
    image->show();
    label->show();
    image->set(Gtk::Stock::DIALOG_ERROR,  Gtk::ICON_SIZE_MENU);
    label->set_text(text);
}

void StatusWidget::start_process(Glib::ustring text)
{
    image->hide();
    spinner->show();
    label->show();
    label->set_text(text);
    spinner->start();
    show_all();
}

void StatusWidget::end_process()
{
    spinner->stop();
    spinner->hide();
    label->hide();
    clear();
}

#if !GTK_CHECK_VERSION(3,6,0)
SearchEntry::SearchEntry() : Gtk::Entry()
{
    signal_changed().connect(sigc::mem_fun(*this, &SearchEntry::_on_changed));
    signal_icon_press().connect(sigc::mem_fun(*this, &SearchEntry::_on_icon_pressed));

    set_icon_from_icon_name(INKSCAPE_ICON("edit-find"), Gtk::ENTRY_ICON_PRIMARY);
    gtk_entry_set_icon_from_icon_name(gobj(), GTK_ENTRY_ICON_SECONDARY, NULL);
}

void SearchEntry::_on_icon_pressed(Gtk::EntryIconPosition icon_position, const GdkEventButton* /*event*/)
{
    if (icon_position == Gtk::ENTRY_ICON_SECONDARY) {
        grab_focus();
        delete_text(0, -1);
    } else if (icon_position == Gtk::ENTRY_ICON_PRIMARY) {
        select_region(0, -1);
        grab_focus();
    }
}

void SearchEntry::_on_changed()
{
    if (get_text().empty()) {
        gtk_entry_set_icon_from_icon_name(gobj(), GTK_ENTRY_ICON_SECONDARY, NULL);
    } else {
        set_icon_from_icon_name(INKSCAPE_ICON("edit-clear"), Gtk::ENTRY_ICON_SECONDARY);
    }
}
#endif


BaseBox::BaseBox() : Gtk::EventBox()
{
#if WITH_GTKMM_3_0
    signal_draw().connect(sigc::mem_fun(*this, &BaseBox::_on_draw), false);
#else
    signal_expose_event().connect(sigc::mem_fun(*this, &BaseBox::_on_expose_event), false);
#endif
    set_visible_window(false);
}

#if !WITH_GTKMM_3_0
bool BaseBox::_on_expose_event(GdkEventExpose* /*event*/)
{
    Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();

    return _on_draw(cr);
}
#endif

bool BaseBox::_on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Draw background and shadow
    int x = get_allocation().get_x();
    int y = get_allocation().get_y();
    int width = get_allocation().get_width();
    int height = get_allocation().get_height();

#if WITH_GTKMM_3_0
    Gdk::RGBA background_fill;
    get_style_context()->lookup_color("base_color", background_fill);
    cr->rectangle(x, y, width, height);
    Gdk::Cairo::set_source_rgba(cr, background_fill);
    cr->fill();
    get_style_context()->render_frame(cr, x, y, width, height);
#else
    Gdk::Color background_fill = get_style()->get_base(get_state());
    cr->rectangle(x, y, width, height);
    Gdk::Cairo::set_source_color(cr, background_fill);
    cr->fill();
    
    get_style()->paint_shadow(get_window(), get_state(), Gtk::SHADOW_IN,
        Gdk::Rectangle(x, y, width, height),
        *this, Glib::ustring("viewport"), x, y, width, height);
#endif

    return false;
}

LogoArea::LogoArea() : Gtk::EventBox()
{
    // Try to load the OCAL logo, but if the file is not found, degrade gracefully
    try {
        std::string logo_path = Glib::build_filename(INKSCAPE_PIXMAPDIR, "OCAL.png");
        logo_mask = Cairo::ImageSurface::create_from_png(logo_path);
        draw_logo = true;
    } catch(Cairo::logic_error) {
        logo_mask = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 1,1);
        draw_logo = false;
    }

#if WITH_GTKMM_3_0
    signal_draw().connect(sigc::mem_fun(*this, &LogoArea::_on_draw));
#else
    signal_expose_event().connect(sigc::mem_fun(*this, &LogoArea::_on_expose_event));
#endif
    set_visible_window(false);
}

#if !WITH_GTKMM_3_0
bool LogoArea::_on_expose_event(GdkEventExpose* /*event*/)
{
        Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();

	return _on_draw(cr);
}
#endif

bool LogoArea::_on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if (draw_logo) {
        int x = get_allocation().get_x();
        int y = get_allocation().get_y();
        int width = get_allocation().get_width();
        int height = get_allocation().get_height();
        int x_logo = x + (width - 220) / 2;
        int y_logo = y + (height - 76) / 2;
        
        // Draw logo, we mask [read fill] it with the mid colour from the
        // user's GTK theme
#if WITH_GTKMM_3_0
        // For GTK+ 3, use grey
        Gdk::RGBA logo_fill("grey");
        Gdk::Cairo::set_source_rgba(cr, logo_fill);
#else
        Gdk::Color logo_fill = get_style()->get_mid(get_state());
        Gdk::Cairo::set_source_color(cr, logo_fill);
#endif

        cr->mask(logo_mask, x_logo, y_logo);
    }
    
    return false;
}

SearchResultList::SearchResultList(guint columns_count) : ListViewText(columns_count)
{
    set_headers_visible(false);
    set_column_title(RESULTS_COLUMN_MARKUP, _("Clipart found"));

    Gtk::CellRenderer* cr_markup = get_column_cell_renderer(RESULTS_COLUMN_MARKUP);
    cr_markup->set_property("ellipsize", Pango::ELLIPSIZE_END);
    get_column(RESULTS_COLUMN_MARKUP)->clear_attributes(*cr_markup);
    get_column(RESULTS_COLUMN_MARKUP)->add_attribute(*cr_markup,
        "markup", RESULTS_COLUMN_MARKUP);

    // Hide all columns except for the MARKUP column
    for (int i = 0; i < RESULTS_COLUMN_LENGTH; i++) {
        if (i != RESULTS_COLUMN_MARKUP) {
            get_column(i)->set_visible(false);
        }
    }
}

void ImportDialog::on_list_results_selection_changed()
{
    std::vector<Gtk::TreeModel::Path> pathlist;
    pathlist = list_results->get_selection()->get_selected_rows();
    std::vector<int> posArray(1);
    
    // If nothing is selected, then return
    if (((int) pathlist.size()) < 1) {
        return;
    }
    int row = pathlist[0][0];
    
    Glib::ustring guid = list_results->get_text(row, RESULTS_COLUMN_GUID);
    
    bool item_selected = (!guid.empty());
    button_import->set_sensitive(item_selected);
}


void ImportDialog::on_button_import_clicked() {
    std::vector<Gtk::TreeModel::Path> pathlist;
    pathlist = list_results->get_selection()->get_selected_rows();
    std::vector<int> posArray(1);
    
    // If nothing is selected, then return
    if (((int) pathlist.size()) < 1) {
        return;
    }
    int row = pathlist[0][0];

    button_import->set_sensitive(false);
    button_close->hide();
    button_cancel->show();
    widget_status->start_process(_("Downloading image..."));
    download_resource(TYPE_IMAGE, row);
}

/*
 * Callback for cursor change
 */
void ImportDialog::on_list_results_cursor_changed()
{
    std::vector<Gtk::TreeModel::Path> pathlist;
    pathlist = list_results->get_selection()->get_selected_rows();
    std::vector<int> posArray(1);
    
    // If nothing is selected, then return
    if (((int) pathlist.size()) < 1) {
        return;
    }
    int row = pathlist[0][0];

    if (downloading_thumbnail) {
        cancellable_thumbnail->cancel();
        cancelled_thumbnail = true;
    }

    update_preview(row);
    downloading_thumbnail = true;
    download_resource(TYPE_THUMBNAIL, row);
}
void ImportDialog::update_preview(int row)
{
    Glib::ustring description = list_results->get_text(row, RESULTS_COLUMN_DESCRIPTION);
    Glib::ustring creator = list_results->get_text(row, RESULTS_COLUMN_CREATOR);
    Glib::ustring date = list_results->get_text(row, RESULTS_COLUMN_DATE);

    preview_files->clear();
    preview_files->set_metadata(description, creator, date);
}


std::string ImportDialog::get_temporary_dir(ResourceType type)
{
    std::string ocal_tmp_dir = Glib::build_filename(Glib::get_tmp_dir(),
        "openclipart");

    if (type == TYPE_THUMBNAIL) {
        return Glib::build_filename(ocal_tmp_dir, "thumbnails");
    } else {
        return Glib::build_filename(ocal_tmp_dir, "images");
    }
}

void ImportDialog::create_temporary_dirs()
{
    // Make sure the temporary directories exists, if not, create them
    std::string ocal_tmp_thumbnail_dir = get_temporary_dir(TYPE_THUMBNAIL);
    std::string ocal_tmp_image_dir = get_temporary_dir(TYPE_IMAGE);

    if (!Glib::file_test(ocal_tmp_thumbnail_dir, Glib::FILE_TEST_EXISTS)) {
        Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(ocal_tmp_thumbnail_dir);
        directory->make_directory_with_parents();
    }
    
    if (!Glib::file_test(ocal_tmp_image_dir, Glib::FILE_TEST_EXISTS)) {
        Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(ocal_tmp_image_dir);
        directory->make_directory_with_parents();
    }
}

void ImportDialog::download_resource(ResourceType type, int row)
{
    // Get Temporary Directory
    std::string ocal_tmp_dir = get_temporary_dir(type);

    // Make a unique filename for the clipart, in the form 'GUID.extension'
    Glib::ustring guid = list_results->get_text(row, RESULTS_COLUMN_GUID);
    Glib::ustring original_filename;

    if (type == TYPE_IMAGE) {
        original_filename = list_results->get_text(row, RESULTS_COLUMN_FILENAME);
    } else {
        original_filename = list_results->get_text(row, RESULTS_COLUMN_THUMBNAIL_FILENAME);
    }
    Glib::ustring extension = Inkscape::IO::get_file_extension(original_filename);

    Glib::ustring filename = Glib::ustring::compose("%1%2", guid, extension);
    std::string path = Glib::build_filename(ocal_tmp_dir, filename.c_str());
    Glib::RefPtr<Gio::File> file_local = Gio::File::create_for_path(path);

    // If the file has already been downloaded, use it
    if (Glib::file_test(path, Glib::FILE_TEST_EXISTS)) {
        if (type == TYPE_IMAGE) {
            on_image_downloaded(path, true);
        } else {
            on_thumbnail_downloaded(path, true);
        }
        return;
    }

    // Get Remote File URL and get the respective cancellable object
    Glib::ustring url;
    Glib::RefPtr<Gio::Cancellable> cancellable;

    if (type == TYPE_IMAGE) {
        url = list_results->get_text(row, RESULTS_COLUMN_URL);
        cancellable_image = Gio::Cancellable::create();
        cancellable = cancellable_image;
    } else {
        url = list_results->get_text(row, RESULTS_COLUMN_THUMBNAIL_URL);
        cancellable_thumbnail = Gio::Cancellable::create();
        cancellable = cancellable_thumbnail;
    }
    
    Glib::RefPtr<Gio::File> file_remote = Gio::File::create_for_uri(url);

    // Download it asynchronously
    file_remote->copy_async(file_local,
        sigc::bind<Glib::RefPtr<Gio::File>, Glib::ustring, ResourceType>(
            sigc::mem_fun(*this, &ImportDialog::on_resource_downloaded),
            file_remote, path, type), cancellable,
        Gio::FILE_COPY_OVERWRITE);
}

void ImportDialog::on_resource_downloaded(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::File> file_remote, Glib::ustring path, ResourceType resource)
{
    bool success;
    
    try {
        success = file_remote->copy_finish(result);
    } catch(Glib::Error) {
        success = false;
    }

    if (resource == TYPE_IMAGE) {
        on_image_downloaded(path, success);
    } else {
        on_thumbnail_downloaded(path, success);
    }
}

void ImportDialog::on_image_downloaded(Glib::ustring path, bool success)
{
    button_import->set_sensitive(true);
    button_close->show();
    button_cancel->hide();
    
    // If anything went wrong, show an error message if the user didn't do it
    if (!success && !cancelled_image) {
        widget_status->set_error(_("Could not download image"));
    }
    if (!success) {
        widget_status->clear();
        return;
    }
    
    try {
        widget_status->clear();
        m_signal_response.emit(path);
        widget_status->set_info(_("Clipart downloaded successfully"));
    } catch(Glib::Error) {
        // success = false; //has no effect, value not returned
    }
    
    cancelled_image = false;
}

void ImportDialog::on_thumbnail_downloaded(Glib::ustring path, bool success)
{
    downloading_thumbnail = false;
    
    // If anything went wrong, show an error message if the user didn't do it
    if (!success && !cancelled_thumbnail) {
        widget_status->set_error(_("Could not download thumbnail file"));
        return;
    }
    if (!success) {
        widget_status->clear();
        return;
    }

    try {
        widget_status->clear();
        preview_files->set_image(path);
    } catch(Glib::Error) {
        // success = false; //has no effect, value not returned
    }
    
    cancelled_thumbnail = false;
}

/*
 * Callback for row activated
 */
void ImportDialog::on_list_results_row_activated(const Gtk::TreeModel::Path& /*path*/,
                                                 Gtk::TreeViewColumn* /*column*/)
{
    on_list_results_cursor_changed();
    button_import->signal_clicked();
}

/**
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node
 */
void SearchResultList::populate_from_xml(xmlNode * a_node)
{
    guint row_num = 0;
    
    for (xmlNode *cur_node = a_node; cur_node; cur_node = cur_node->next) {

        // Get items information
        if (strcmp(reinterpret_cast<const char*>(cur_node->name), "rss")) // Avoid the root
            if (cur_node->type == XML_ELEMENT_NODE &&
                    (cur_node->parent->name && !strcmp(reinterpret_cast<const char*>(cur_node->parent->name), "item")))
            {
                if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "title"))
                {
                    row_num = append("");
                    xmlChar *xml_title = xmlNodeGetContent(cur_node);
                    char* title = reinterpret_cast<char*>(xml_title);
                    
                    set_text(row_num, RESULTS_COLUMN_TITLE, title);
                    xmlFree(title);
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "pubDate"))
                {
                    xmlChar *xml_date = xmlNodeGetContent(cur_node);
                    char* date = reinterpret_cast<char*>(xml_date);
                    
                    set_text(row_num, RESULTS_COLUMN_DATE, date);
                    xmlFree(xml_date);
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "creator"))
                {
                    xmlChar *xml_creator = xmlNodeGetContent(cur_node);
                    char* creator = reinterpret_cast<char*>(xml_creator);
                    
                    set_text(row_num, RESULTS_COLUMN_CREATOR, creator);
                    xmlFree(xml_creator);
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "description"))
                {
                    xmlChar *xml_description = xmlNodeGetContent(cur_node);
                    //char* final_description;
                    char* stripped_description = g_strstrip(reinterpret_cast<char*>(xml_description));

                    if (!strcmp(stripped_description, "")) {
                        stripped_description = _("No description");
                    }

                    //GRegex* regex = g_regex_new(g_regex_escape_string(stripped_description, -1));
                    //final_description = g_regex_replace_literal(regex, "\n", -1, 0, " ");

                    set_text(row_num, RESULTS_COLUMN_DESCRIPTION, stripped_description);
                    xmlFree(xml_description);
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "enclosure"))
                {
                    xmlChar *xml_url = xmlGetProp(cur_node, reinterpret_cast<xmlChar const*>("url"));
                    char* url = reinterpret_cast<char*>(xml_url);
                    char* filename = g_path_get_basename(url);

                    set_text(row_num, RESULTS_COLUMN_URL, url);
                    set_text(row_num, RESULTS_COLUMN_FILENAME, filename);
                    xmlFree(xml_url);
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "thumbnail"))
                {
                    xmlChar *xml_thumbnail_url = xmlGetProp(cur_node, reinterpret_cast<xmlChar const*>("url"));
                    char* thumbnail_url = reinterpret_cast<char*>(xml_thumbnail_url);
                    char* thumbnail_filename = g_path_get_basename(thumbnail_url);

                    set_text(row_num, RESULTS_COLUMN_THUMBNAIL_URL, thumbnail_url);
                    set_text(row_num, RESULTS_COLUMN_THUMBNAIL_FILENAME, thumbnail_filename);
                    xmlFree(xml_thumbnail_url);
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur_node->name), "guid"))
                {
                    xmlChar *xml_guid = xmlNodeGetContent(cur_node);
                    char* guid_url = reinterpret_cast<char*>(xml_guid);
                    char* guid = g_path_get_basename(guid_url);

                    set_text(row_num, RESULTS_COLUMN_GUID, guid);
                    xmlFree(xml_guid);
                }
            }
        populate_from_xml(cur_node->children);
    }
}

/**
 * Callback for user input into entry_search
 */
void ImportDialog::on_button_search_clicked()
{
    on_entry_search_activated();
}

void ImportDialog::on_button_close_clicked()
{
    hide();
}

void ImportDialog::on_button_cancel_clicked()
{
    cancellable_image->cancel();
    cancelled_image = true;
}

/**
 * Callback for user input into entry_search
 */
void ImportDialog::on_entry_search_activated()
{
    preview_files->clear();
    widget_status->start_process(_("Searching clipart..."));
    
    notebook_content->set_current_page(NOTEBOOK_PAGE_LOGO);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Glib::ustring search_keywords = entry_search->get_text();
    
    // Create the URI to the OCAL RSS feed
    Glib::ustring xml_uri = Glib::ustring::compose("http://%1/media/feed/rss/%2",
        prefs->getString("/options/ocalurl/str"), search_keywords);
    // If we are not UTF8
    if (!Glib::get_charset()) {
        xml_uri = Glib::filename_to_utf8(xml_uri);
    }

    // Open the RSS feed
    Glib::RefPtr<Gio::File> xml_file = Gio::File::create_for_uri(xml_uri);
#ifdef WIN32
    if (!xml_file->query_exists()) {
        widget_status->set_error(_("Could not connect to the Open Clip Art Library"));
        return;
    }
#endif
    xml_file->load_contents_async(
        sigc::bind<Glib::RefPtr<Gio::File> , Glib::ustring>(
            sigc::mem_fun(*this, &ImportDialog::on_xml_file_read),
            xml_file, xml_uri)
    );
}

void ImportDialog::on_xml_file_read(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::File> xml_file, Glib::ustring xml_uri)
{
    widget_status->end_process();
    
    char* data;
    gsize length;
    
    bool sucess = xml_file->load_contents_finish(result, data, length);
    if (!sucess) {
        widget_status->set_error(_("Could not connect to the Open Clip Art Library"));
        return;
    }

    // Create the resulting xml document tree
    // Initialize libxml and test mistakes between compiled and shared library used
    LIBXML_TEST_VERSION
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    int parse_options = XML_PARSE_RECOVER + XML_PARSE_NOWARNING + XML_PARSE_NOERROR;  // do not use XML_PARSE_NOENT ! see bug lp:1025185
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool allowNetAccess = prefs->getBool("/options/externalresources/xml/allow_net_access", false);
    if (!allowNetAccess) {
        parse_options |= XML_PARSE_NONET;
    }

    doc = xmlReadMemory(data, (int) length, xml_uri.c_str(), NULL, parse_options);
        
    if (doc == NULL) {
        // If nothing is returned, no results could be found
        if (length == 0) {
            notebook_content->set_current_page(NOTEBOOK_PAGE_NOT_FOUND);
            update_label_no_search_results();
        } else {
            widget_status->set_error(_("Could not parse search results"));
        }
        return;
    }

    // Get the root element node
    root_element = xmlDocGetRootElement(doc);

    // Clear and populate the list_results
    list_results->clear_items();
    list_results->populate_from_xml(root_element);

    // Populate the MARKUP column with the title & description of the clipart
    for (guint i = 0; i < list_results->size(); i++) {
        Glib::ustring title = list_results->get_text(i, RESULTS_COLUMN_TITLE);
        Glib::ustring description = list_results->get_text(i, RESULTS_COLUMN_DESCRIPTION);
        char* markup = g_markup_printf_escaped("<b>%s</b>\n<span size=\"small\">%s</span>",
            title.c_str(), description.c_str());
        list_results->set_text(i, RESULTS_COLUMN_MARKUP, markup);
    }
    notebook_content->set_current_page(NOTEBOOK_PAGE_RESULTS);

    // free the document
    xmlFreeDoc(doc);
}


void ImportDialog::update_label_no_search_results()
{
    Glib::ustring keywords = Glib::Markup::escape_text(entry_search->get_text());

    Glib::ustring msg_one = Glib::ustring::compose(
        _("No clipart named <b>%1</b> was found."),
        keywords);
    Glib::ustring msg_two = _("Please make sure all keywords are spelled correctly,"
                              " or try again with different keywords.");

#if WITH_GTKMM_3_0
    Glib::ustring markup = Glib::ustring::compose(
        "<span size=\"large\">%1</span>\n<span>%2</span>",
        msg_one, msg_two);
#else
    Gdk::Color grey = entry_search->get_style()->get_text_aa(entry_search->get_state());
    Glib::ustring markup = Glib::ustring::compose(
        "<span size=\"large\">%1</span>\n<span color=\"%2\">%3</span>",
        msg_one, grey.to_string(), msg_two);
#endif

    label_not_found->set_markup(markup);
}

/**
 * Constructor.  Not called directly.  Use the factory.
 */
ImportDialog::ImportDialog(Gtk::Window& parent_window, FileDialogType file_types,
                                                   const Glib::ustring &title) :
    FileDialogBase(title, parent_window)
{
    // Initalize to Autodetect
    extension = NULL;
    // No filename to start out with
    Glib::ustring search_keywords = "";

    dialogType = file_types;

    // Creation
    Gtk::VBox *vbox = new Gtk::VBox(false, 0);

#if WITH_GTKMM_3_0
    Gtk::ButtonBox *hbuttonbox_bottom = new Gtk::ButtonBox();
#else
    Gtk::HButtonBox *hbuttonbox_bottom = new Gtk::HButtonBox();
#endif

    Gtk::HBox *hbox_bottom = new Gtk::HBox(false, 12);
    BaseBox *basebox_logo = new BaseBox();
    BaseBox *basebox_no_search_results = new BaseBox();
    label_not_found = new Gtk::Label();
    label_description = new Gtk::Label();

#if GTK_CHECK_VERSION(3,6,0)
    entry_search = new Gtk::SearchEntry();
#else
    entry_search = new SearchEntry();
#endif

    button_search = new Gtk::Button(_("Search"));

#if WITH_GTKMM_3_0
    Gtk::ButtonBox* hbuttonbox_search = new Gtk::ButtonBox();
#else
    Gtk::HButtonBox* hbuttonbox_search = new Gtk::HButtonBox();
#endif

    Gtk::ScrolledWindow* scrolledwindow_preview = new Gtk::ScrolledWindow();
    preview_files = new PreviewWidget();
    /// Add the buttons in the bottom of the dialog
    button_cancel = new Gtk::Button(Gtk::Stock::CANCEL);
    button_close = new Gtk::Button(_("Close"));
    button_import = new Gtk::Button(_("Import"));
    list_results = new SearchResultList(RESULTS_COLUMN_LENGTH);
    drawingarea_logo = new LogoArea();
    notebook_content = new Gtk::Notebook();
    widget_status = new StatusWidget();

    downloading_thumbnail = false;
    cancelled_thumbnail = false;
    cancelled_image = false;
    
    // Packing
    add(*vbox);
    vbox->pack_start(hbox_tags, false, false);
    vbox->pack_start(hbox_files, true, true);
    vbox->pack_start(*hbox_bottom, false, false);
    basebox_logo->add(*drawingarea_logo);
    basebox_no_search_results->add(*label_not_found);
    hbox_bottom->pack_start(*widget_status, true, true);
    hbox_bottom->pack_start(*hbuttonbox_bottom, true, true);
    hbuttonbox_bottom->pack_start(*button_cancel, false, false);
    hbuttonbox_bottom->pack_start(*button_close, false, false);
    hbuttonbox_bottom->pack_start(*button_import, false, false);
    hbuttonbox_search->pack_start(*button_search, false, false);
    hbox_tags.pack_start(*entry_search, true, true);
    hbox_tags.pack_start(*hbuttonbox_search, false, false);
    hbox_files.pack_start(*notebook_content, true, true);
    scrolledwindow_preview->add(*preview_files);
    hbox_files.pack_start(*scrolledwindow_preview, true, true);

    notebook_content->insert_page(*basebox_logo, NOTEBOOK_PAGE_LOGO);
    notebook_content->insert_page(scrolledwindow_list, NOTEBOOK_PAGE_RESULTS);
    notebook_content->insert_page(*basebox_no_search_results, NOTEBOOK_PAGE_NOT_FOUND);

    // Properties
    set_border_width(12);
    set_default_size(480, 330);
    vbox->set_spacing(12);
    hbuttonbox_bottom->set_spacing(6);
    hbuttonbox_bottom->set_layout(Gtk::BUTTONBOX_END);
    button_import->set_sensitive(false);
    entry_search->set_max_length(255);
    hbox_tags.set_spacing(6);
    preview_files->clear();
    notebook_content->set_current_page(NOTEBOOK_PAGE_LOGO);
    /// Add the listview inside a ScrolledWindow
    scrolledwindow_list.add(*list_results);
    scrolledwindow_list.set_shadow_type(Gtk::SHADOW_IN);
    /// Only show the scrollbars when they are necessary
    scrolledwindow_list.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    preview_files->set_size_request(120, -1);
    hbox_files.set_spacing(12);
    label_not_found->set_line_wrap(true);
    label_not_found->set_line_wrap_mode(Pango::WRAP_WORD);
    label_not_found->set_justify(Gtk::JUSTIFY_CENTER);
    label_not_found->set_size_request(260, -1);
    scrolledwindow_preview->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    notebook_content->set_show_tabs(false);
    notebook_content->set_show_border(false);
    button_cancel->set_no_show_all(true);
    button_close->set_no_show_all(true);
    button_close->show();
    button_cancel->hide();
    
    // Signals
    entry_search->signal_activate().connect(
            sigc::mem_fun(*this, &ImportDialog::on_entry_search_activated));
    button_import->signal_clicked().connect(
            sigc::mem_fun(*this, &ImportDialog::on_button_import_clicked));
    button_close->signal_clicked().connect(
            sigc::mem_fun(*this, &ImportDialog::on_button_close_clicked));
    button_cancel->signal_clicked().connect(
            sigc::mem_fun(*this, &ImportDialog::on_button_cancel_clicked));
    button_search->signal_clicked().connect(
            sigc::mem_fun(*this, &ImportDialog::on_button_search_clicked));
    list_results->signal_cursor_changed().connect(
            sigc::mem_fun(*this, &ImportDialog::on_list_results_cursor_changed));
    list_results->signal_row_activated().connect(
            sigc::mem_fun(*this, &ImportDialog::on_list_results_row_activated));
    list_results->get_selection()->signal_changed().connect(
            sigc::mem_fun(*this, &ImportDialog::on_list_results_selection_changed));

    show_all_children();
    entry_search->grab_focus();

    // Create the temporary directories that will be needed later
    create_temporary_dirs();
}

/**
 * Destructor
 */
ImportDialog::~ImportDialog()
{
    // free the global variables that may have been allocated by the parser
    xmlCleanupParser();
}

/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *
ImportDialog::get_selection_type()
{
    return extension;
}

ImportDialog::type_signal_response ImportDialog::signal_response()
{
  return m_signal_response;
}


} //namespace OCAL
} //namespace Dialog
} //namespace UI
} //namespace Inkscape

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

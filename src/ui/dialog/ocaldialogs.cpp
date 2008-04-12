/**
 * Implementation of the ocal dialog interfaces defined in ocaldialog.h
 *
 * Authors:
 *   Bruno Dilly
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2007 Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>  // rename()
#include <unistd.h> // close()
#include <errno.h>  // errno
#include <string.h> // strerror()

#include "ocaldialogs.h"
#include "filedialogimpl-gtkmm.h"
#include "interface.h"
#include "gc-core.h"
#include <dialogs/dialog-events.h>
#include "io/sys.h"

namespace Inkscape
{
namespace UI
{
namespace Dialog
{

//########################################################################
//# F I L E    E X P O R T   T O   O C A L
//########################################################################

/**
 * Callback for fileNameEntry widget
 */
void FileExportToOCALDialog::fileNameEntryChangedCallback()
{
    if (!fileNameEntry)
        return;

    Glib::ustring fileName = fileNameEntry->get_text();
    if (!Glib::get_charset()) //If we are not utf8
        fileName = Glib::filename_to_utf8(fileName);

    myFilename = fileName;
    response(Gtk::RESPONSE_OK);
}

/**
 * Constructor
 */
FileExportToOCALDialog::FileExportToOCALDialog(Gtk::Window &parentWindow,
            FileDialogType fileTypes,
            const Glib::ustring &title) :
    FileDialogOCALBase(title, parentWindow)
{
    /*
     * Start Taking the vertical Box and putting a Label
     * and a Entry to take the filename
     * Later put the extension selection and checkbox (?)
     */
    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename = "";
    /* Set our dialog type (save, export, etc...)*/
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
             sigc::mem_fun(*this, &FileExportToOCALDialog::fileNameEntryChangedCallback) );
        }

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_OK));

    show_all_children();
}

/**
 * Destructor
 */
FileExportToOCALDialog::~FileExportToOCALDialog()
{
}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileExportToOCALDialog::show()
{
    set_modal (TRUE);                      //Window
    sp_transientize((GtkWidget *)gobj());  //Make transient
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

/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
Glib::ustring
FileExportToOCALDialog::getFilename()
{
    myFilename = fileNameEntry->get_text();
    if (!Glib::get_charset()) //If we are not utf8
        myFilename = Glib::filename_to_utf8(myFilename);

    return myFilename;
}


void
FileExportToOCALDialog::change_title(const Glib::ustring& title)
{
    this->set_title(title);
}


//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Constructor
 */
FileExportToOCALPasswordDialog::FileExportToOCALPasswordDialog(Gtk::Window &parentWindow,
                             const Glib::ustring &title) : FileDialogOCALBase(title, parentWindow)
{
    /*
     * Start Taking the vertical Box and putting 2 Labels
     * and 2 Entries to take the username and password
     */
    /* No username and password to start out with */
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


/**
 * Destructor
 */
FileExportToOCALPasswordDialog::~FileExportToOCALPasswordDialog()
{
}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileExportToOCALPasswordDialog::show()
{
    set_modal (TRUE);                      //Window
    sp_transientize((GtkWidget *)gobj());  //Make transient
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

/**
 * Get the username.   Valid after an [OK]
 */
Glib::ustring
FileExportToOCALPasswordDialog::getUsername()
{
    myUsername = usernameEntry->get_text();
    return myUsername;
}

/**
 * Get the password.   Valid after an [OK]
 */
Glib::ustring
FileExportToOCALPasswordDialog::getPassword()
{
    myPassword = passwordEntry->get_text();
    return myPassword;
}

void
FileExportToOCALPasswordDialog::change_title(const Glib::ustring& title)
{
    this->set_title(title);
}


//#########################################################################
//### F I L E   I M P O R T   F R O M   O C A L
//#########################################################################

/*
 * Calalback for cursor chage
 */
void FileListViewText::on_cursor_changed()
{
    std::vector<Gtk::TreeModel::Path> pathlist;
    pathlist = this->get_selection()->get_selected_rows();
    std::vector<int> posArray(1);
    posArray = pathlist[0].get_indices();

#ifdef WITH_GNOME_VFS
    gnome_vfs_init();
    GnomeVFSHandle    *from_handle = NULL;
    GnomeVFSHandle    *to_handle = NULL;
    GnomeVFSFileSize  bytes_read;
    GnomeVFSFileSize  bytes_written;
    GnomeVFSResult    result;
    guint8 buffer[8192];
    Glib::ustring fileUrl;

    // FIXME: this would be better as a per-user OCAL cache of files
    // instead of filling /tmp with downloads.
    //
    // create file path
    const std::string tmptemplate = "ocal-";
    std::string tmpname;
    int fd = Inkscape::IO::file_open_tmp(tmpname, tmptemplate);
    if (fd<0) {
        g_warning("Error creating temp file");
        return;
    }
    close(fd);
    // make sure we don't collide with other users on the same machine
    myFilename = tmpname;
    myFilename.append("-");
    myFilename.append(get_text(posArray[0], 2));
    // rename based on original image's name, retaining extension
    if (rename(tmpname.c_str(),myFilename.c_str())<0) {
        unlink(tmpname.c_str());
        g_warning("Error creating destination file '%s': %s", myFilename.c_str(), strerror(errno));
        goto failquit;
    }

    //get file url
    fileUrl = get_text(posArray[0], 1); //http url

    //Glib::ustring fileUrl = "dav://"; //dav url
    //fileUrl.append(prefs_get_string_attribute("options.ocalurl", "str"));
    //fileUrl.append("/dav.php/");
    //fileUrl.append(get_text(posArray[0], 3)); //author dir
    //fileUrl.append("/");
    //fileUrl.append(get_text(posArray[0], 2)); //filename

    if (!Glib::get_charset()) //If we are not utf8
        fileUrl = Glib::filename_to_utf8(fileUrl);

    {
        // open the temp file to receive
        result = gnome_vfs_open (&to_handle, myFilename.c_str(), GNOME_VFS_OPEN_WRITE);
        if (result == GNOME_VFS_ERROR_NOT_FOUND){
            result = gnome_vfs_create (&to_handle, myFilename.c_str(), GNOME_VFS_OPEN_WRITE, FALSE, GNOME_VFS_PERM_USER_ALL);
        }
        if (result != GNOME_VFS_OK) {
            g_warning("Error creating temp file '%s': %s", myFilename.c_str(), gnome_vfs_result_to_string(result));
            goto fail;
        }
        result = gnome_vfs_open (&from_handle, fileUrl.c_str(), GNOME_VFS_OPEN_READ);
        if (result != GNOME_VFS_OK) {
            g_warning("Could not find the file in Open Clip Art Library.");
            goto fail;
        }
        // copy the file
        while (1) {
            result = gnome_vfs_read (from_handle, buffer, 8192, &bytes_read);
            if ((result == GNOME_VFS_ERROR_EOF) &&(!bytes_read)){
                result = gnome_vfs_close (from_handle);
                result = gnome_vfs_close (to_handle);
                break;
            }
            if (result != GNOME_VFS_OK) {
                g_warning("%s", gnome_vfs_result_to_string(result));
                goto fail;
            }
            result = gnome_vfs_write (to_handle, buffer, bytes_read, &bytes_written);
            if (result != GNOME_VFS_OK) {
                g_warning("%s", gnome_vfs_result_to_string(result));
                goto fail;
            }
            if (bytes_read != bytes_written){
                g_warning("Bytes read not equal to bytes written");
                goto fail;
            }
        }
    }
    myPreview->showImage(myFilename);
    myLabel->set_text(get_text(posArray[0], 4));
#endif
    return;
fail:
    unlink(myFilename.c_str());
failquit:
    myFilename = "";
}


/*
 * Callback for row activated
 */
void FileListViewText::on_row_activated(const Gtk::TreeModel::Path& /*path*/, Gtk::TreeViewColumn* /*column*/)
{
    this->on_cursor_changed();
    myButton->activate();
}


/*
 * Returns the selected filename
 */
Glib::ustring FileListViewText::getFilename()
{
    return myFilename;
}


#ifdef WITH_GNOME_VFS
/**
 * Read callback for xmlReadIO(), used below
 */
static int vfs_read_callback (GnomeVFSHandle *handle, char* buf, int nb)
{
    GnomeVFSFileSize ndone;
    GnomeVFSResult    result;

    result = gnome_vfs_read (handle, buf, nb, &ndone);

    if (result == GNOME_VFS_OK) {
        return (int)ndone;
    } else {
        if (result != GNOME_VFS_ERROR_EOF) {
            sp_ui_error_dialog(_("Error while reading the Open Clip Art RSS feed"));
            g_warning("%s\n", gnome_vfs_result_to_string(result));
        }
        return -1;
    }
}
#endif


/**
 * Callback for user input into searchTagEntry
 */
void FileImportFromOCALDialog::searchTagEntryChangedCallback()
{
    if (!searchTagEntry)
        return;

    notFoundLabel->hide();
    descriptionLabel->set_text("");

    Glib::ustring searchTag = searchTagEntry->get_text();
    // create the ocal uri to get rss feed
    Glib::ustring uri = "http://";
    uri.append(prefs_get_string_attribute("options.ocalurl", "str"));
    uri.append("/media/feed/rss/");
    uri.append(searchTag);
    if (!Glib::get_charset()) //If we are not utf8
        uri = Glib::filename_to_utf8(uri);

#ifdef WITH_GNOME_VFS

    // open the rss feed
    gnome_vfs_init();
    GnomeVFSHandle    *from_handle = NULL;
    GnomeVFSResult    result;

    result = gnome_vfs_open (&from_handle, uri.c_str(), GNOME_VFS_OPEN_READ);
    if (result != GNOME_VFS_OK) {
        sp_ui_error_dialog(_("Failed to receive the Open Clip Art Library RSS feed. Verify if the server name is correct in Configuration->Import/Export (e.g.: openclipart.org)"));
        return;
    }

    // create the resulting xml document tree
    // this initialize the library and test mistakes between compiled and shared library used
    LIBXML_TEST_VERSION
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    doc = xmlReadIO ((xmlInputReadCallback) vfs_read_callback,
        (xmlInputCloseCallback) gnome_vfs_close, from_handle, uri.c_str(), NULL,
        XML_PARSE_RECOVER);
    if (doc == NULL) {
        sp_ui_error_dialog(_("Server supplied malformed Clip Art feed"));
        g_warning("Failed to parse %s\n", uri.c_str());
        return;
    }

    // get the root element node
    root_element = xmlDocGetRootElement(doc);

    // clear the fileslist
    filesList->clear_items();
    filesList->set_sensitive(false);

    // print all xml the element names
    print_xml_element_names(root_element);

    if (filesList->size() == 0)
    {
        notFoundLabel->show();
        filesList->set_sensitive(false);
    }
    else
        filesList->set_sensitive(true);

    // free the document
    xmlFreeDoc(doc);
    // free the global variables that may have been allocated by the parser
    xmlCleanupParser();
    return;
#endif
}


/**
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node
 */
void FileImportFromOCALDialog::print_xml_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;
    guint row_num = 0;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        // get itens information
        if (strcmp((const char*)cur_node->name, "rss")) //avoid the root
            if (cur_node->type == XML_ELEMENT_NODE && !strcmp((const char*)cur_node->parent->name, "item"))
            {
                if (!strcmp((const char*)cur_node->name, "title"))
                {
                    xmlChar *title = xmlNodeGetContent(cur_node);
                    row_num = filesList->append_text((const char*)title);
                    xmlFree(title);
                }
#ifdef WITH_GNOME_VFS
                else if (!strcmp((const char*)cur_node->name, "enclosure"))
                {
                    xmlChar *urlattribute = xmlGetProp(cur_node, (xmlChar*)"url");
                    filesList->set_text(row_num, 1, (const char*)urlattribute);
                    gchar *tmp_file;
                    tmp_file = gnome_vfs_uri_extract_short_path_name(gnome_vfs_uri_new((const char*)urlattribute));
                    filesList->set_text(row_num, 2, (const char*)tmp_file);
                    xmlFree(urlattribute);
                }
                else if (!strcmp((const char*)cur_node->name, "creator"))
                {
                    filesList->set_text(row_num, 3, (const char*)xmlNodeGetContent(cur_node));
                }
                else if (!strcmp((const char*)cur_node->name, "description"))
                {
                    filesList->set_text(row_num, 4, (const char*)xmlNodeGetContent(cur_node));
                }
#endif
            }
        print_xml_element_names(cur_node->children);
    }
}

/**
 * Constructor.  Not called directly.  Use the factory.
 */
FileImportFromOCALDialog::FileImportFromOCALDialog(Gtk::Window& parentWindow,
                                                   const Glib::ustring &/*dir*/,
                                                   FileDialogType fileTypes,
                                                   const Glib::ustring &title) :
    FileDialogOCALBase(title, parentWindow)
{
    // Initalize to Autodetect
    extension = NULL;
    // No filename to start out with
    Glib::ustring searchTag = "";

    dialogType = fileTypes;
    Gtk::VBox *vbox = get_vbox();
    Gtk::Label *tagLabel = new Gtk::Label(_("Search for:"));
    notFoundLabel = new Gtk::Label(_("No files matched your search"));
    descriptionLabel = new Gtk::Label();
    descriptionLabel->set_max_width_chars(260);
    descriptionLabel->set_size_request(500, -1);
    descriptionLabel->set_single_line_mode(false);
    descriptionLabel->set_line_wrap(true);
    messageBox.pack_start(*notFoundLabel);
    descriptionBox.pack_start(*descriptionLabel);
    searchTagEntry = new Gtk::Entry();
    searchTagEntry->set_text(searchTag);
    searchTagEntry->set_max_length(255);
    searchButton = new Gtk::Button(_("Search"));
    tagBox.pack_start(*tagLabel);
    tagBox.pack_start(*searchTagEntry, Gtk::PACK_EXPAND_WIDGET, 3);
    tagBox.pack_start(*searchButton);
    filesPreview = new SVGPreview();
    filesPreview->showNoPreview();
    // add the buttons in the bottom of the dialog
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    okButton = add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);
    // sets the okbutton to default
    set_default(*okButton);
    filesList = new FileListViewText(5, *filesPreview, *descriptionLabel, *okButton);
    filesList->set_sensitive(false);
    // add the listview inside a ScrolledWindow
    listScrolledWindow.add(*filesList);
    // only show the scrollbars when they are necessary:
    listScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    filesList->set_column_title(0, _("Files found"));
    listScrolledWindow.set_size_request(400, 180);
    filesList->get_column(1)->set_visible(false); // file url
    filesList->get_column(2)->set_visible(false); // tmp file path
    filesList->get_column(3)->set_visible(false); // author dir
    filesList->get_column(4)->set_visible(false); // file description
    filesBox.pack_start(listScrolledWindow);
    filesBox.pack_start(*filesPreview);
    vbox->pack_start(tagBox, false, false);
    vbox->pack_start(messageBox);
    vbox->pack_start(filesBox);
    vbox->pack_start(descriptionBox);

    //Let's do some customization
    searchTagEntry = NULL;
    Gtk::Container *cont = get_toplevel();
    std::vector<Gtk::Entry *> entries;
    findEntryWidgets(cont, entries);
    if (entries.size() >=1 )
    {
    //Catch when user hits [return] on the text field
        searchTagEntry = entries[0];
        searchTagEntry->signal_activate().connect(
              sigc::mem_fun(*this, &FileImportFromOCALDialog::searchTagEntryChangedCallback));
    }

    searchButton->signal_clicked().connect(
            sigc::mem_fun(*this, &FileImportFromOCALDialog::searchTagEntryChangedCallback));

    show_all_children();
    notFoundLabel->hide();
}

/**
 * Destructor
 */
FileImportFromOCALDialog::~FileImportFromOCALDialog()
{

}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileImportFromOCALDialog::show()
{
    set_modal (TRUE);                      //Window
    sp_transientize((GtkWidget *)gobj());  //Make transient
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


/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *
FileImportFromOCALDialog::getSelectionType()
{
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
Glib::ustring
FileImportFromOCALDialog::getFilename (void)
{
    return filesList->getFilename();
}


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

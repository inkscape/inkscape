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

#include "ocaldialogs.h"
#include "filedialogimpl-gtkmm.h"
#include "interface.h"
#include "gc-core.h"
#include <dialogs/dialog-events.h>

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
 * Public factory method.  Used in file.cpp
 */

 FileExportToOCALDialog *FileExportToOCALDialog::create(Gtk::Window& parentWindow, 
                                           FileDialogType fileTypes,
                                           const Glib::ustring &title,
                                           const Glib::ustring &default_key)
{
    FileExportToOCALDialog *dialog = new FileExportToOCALDialogImpl(parentWindow, fileTypes, title, default_key);
    return dialog;
}

//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Public factory method.  Used in file.cpp
 */

FileExportToOCALPasswordDialog *FileExportToOCALPasswordDialog::create(Gtk::Window& parentWindow,
                                                        const Glib::ustring &title)
{
    FileExportToOCALPasswordDialog *dialog = new FileExportToOCALPasswordDialogImpl(parentWindow, title);
    return dialog;
}


//#########################################################################
//### F I L E    I M P O R T  F R O M  O C A L
//#########################################################################

/**
 * Public factory.  Called by file.cpp.
 */
FileImportFromOCALDialog *FileImportFromOCALDialog::create(Gtk::Window &parentWindow,
		                       const Glib::ustring &path,
                                       FileDialogType fileTypes,
                                       const Glib::ustring &title)
{
    FileImportFromOCALDialog *dialog = new FileImportFromOCALDialogImplGtk(parentWindow, path, fileTypes, title);
    return dialog;
}


//########################################################################
//# F I L E    E X P O R T   T O   O C A L
//########################################################################



/**
 * Callback for fileNameEntry widget
 */
void FileExportToOCALDialogImpl::fileNameEntryChangedCallback()
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
 * Callback for fileNameEntry widget
 */
void FileExportToOCALDialogImpl::fileTypeChangedCallback()
{
    int sel = fileTypeComboBox.get_active_row_number();
    if (sel<0 || sel >= (int)fileTypes.size())
        return;
    FileType type = fileTypes[sel];

    extension = type.extension;
    updateNameAndExtension();
}



void FileExportToOCALDialogImpl::createFileTypeMenu()
{
    Inkscape::Extension::DB::OutputList extension_list;
    Inkscape::Extension::db.get_output_list(extension_list);
    knownExtensions.clear();

    for (Inkscape::Extension::DB::OutputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); current_item++)
    {
        Inkscape::Extension::Output * omod = *current_item;

        // FIXME: would be nice to grey them out instead of not listing them
        if (omod->deactivated()) continue;

        FileType type;
        type.name     = (_(omod->get_filetypename()));
        type.pattern  = "*";
        Glib::ustring extension = omod->get_extension();
        knownExtensions.insert( extension.casefold() );
        fileDialogExtensionToPattern (type.pattern, extension);
        type.extension= omod;
        fileTypeComboBox.append_text(type.name);
        fileTypes.push_back(type);
    }

    //#Let user choose
    FileType guessType;
    guessType.name = _("Guess from extension");
    guessType.pattern = "*";
    guessType.extension = NULL;
    fileTypeComboBox.append_text(guessType.name);
    fileTypes.push_back(guessType);


    fileTypeComboBox.set_active(0);
    fileTypeChangedCallback(); //call at least once to set the filter
}



/**
 * Constructor
 */
FileExportToOCALDialogImpl::FileExportToOCALDialogImpl(Gtk::Window &parentWindow,
            FileDialogType fileTypes,
            const Glib::ustring &title,
            const Glib::ustring &default_key) :
    FileDialogOCALBase(title)
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

    //###### Do we want the .xxx extension automatically added?
    fileTypeCheckbox.set_label(Glib::ustring(_("Append filename extension automatically")));
    fileTypeCheckbox.set_active( (bool)prefs_get_int_attribute("dialogs.export",
                                                               "append_extension", 1) );

    createFileTypeMenu();

    fileTypeComboBox.set_size_request(200,40);
    fileTypeComboBox.signal_changed().connect(
        sigc::mem_fun(*this, &FileExportToOCALDialogImpl::fileTypeChangedCallback) );

    checksBox.pack_start( fileTypeCheckbox );
    vbox->pack_start( checksBox );

    vbox->pack_end( fileTypeComboBox );

    //Let's do some customization
    fileNameEntry = NULL;
    Gtk::Container *cont = get_toplevel();
    std::vector<Gtk::Entry *> entries;
    findEntryWidgets(cont, entries);
    //g_message("Found %d entry widgets\n", entries.size());
    if (entries.size() >=1 )
        {
        //Catch when user hits [return] on the text field
        fileNameEntry = entries[0];
        fileNameEntry->signal_activate().connect(
             sigc::mem_fun(*this, &FileExportToOCALDialogImpl::fileNameEntryChangedCallback) );
        }

    //Let's do more customization
    std::vector<Gtk::Expander *> expanders;
    findExpanderWidgets(cont, expanders);
    //g_message("Found %d expander widgets\n", expanders.size());
    if (expanders.size() >=1 )
        {
        //Always show the file list
        Gtk::Expander *expander = expanders[0];
        expander->set_expanded(true);
        }


    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_OK));

    show_all_children();
}



/**
 * Destructor
 */
FileExportToOCALDialogImpl::~FileExportToOCALDialogImpl()
{
}



/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileExportToOCALDialogImpl::show()
{
    set_modal (TRUE);                      //Window
    sp_transientize((GtkWidget *)gobj());  //Make transient
    gint b = run();                        //Dialog
    hide();

    if (b == Gtk::RESPONSE_OK)
    {
        updateNameAndExtension();

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
FileExportToOCALDialogImpl::getSelectionType()
{
    return extension;
}

void FileExportToOCALDialogImpl::setSelectionType( Inkscape::Extension::Extension * key )
{
    // If no pointer to extension is passed in, look up based on filename extension.
    if ( !key ) {
        // Not quite UTF-8 here.
        gchar *filenameLower = g_ascii_strdown(myFilename.c_str(), -1);
        for ( int i = 0; !key && (i < (int)fileTypes.size()); i++ ) {
            Inkscape::Extension::Output *ext = dynamic_cast<Inkscape::Extension::Output*>(fileTypes[i].extension);
            if ( ext && ext->get_extension() ) {
                gchar *extensionLower = g_ascii_strdown( ext->get_extension(), -1 );
                if ( g_str_has_suffix(filenameLower, extensionLower) ) {
                    key = fileTypes[i].extension;
                }
                g_free(extensionLower);
            }
        }
        g_free(filenameLower);
    }

    // Ensure the proper entry in the combo box is selected.
    if ( key ) {
        extension = key;
        gchar const * extensionID = extension->get_id();
        if ( extensionID ) {
            for ( int i = 0; i < (int)fileTypes.size(); i++ ) {
                Inkscape::Extension::Extension *ext = fileTypes[i].extension;
                if ( ext ) {
                    gchar const * id = ext->get_id();
                    if ( id && ( strcmp(extensionID, id) == 0) ) {
                        int oldSel = fileTypeComboBox.get_active_row_number();
                        if ( i != oldSel ) {
                            fileTypeComboBox.set_active(i);
                        }
                        break;
                    }
                }
            }
        }
    }
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
Glib::ustring
FileExportToOCALDialogImpl::getFilename()
{
    myFilename = fileNameEntry->get_text();
    updateNameAndExtension();
    return myFilename;
}


void
FileExportToOCALDialogImpl::change_title(const Glib::ustring& title)
{
    this->set_title(title);
}

void FileExportToOCALDialogImpl::updateNameAndExtension()
{
    // Pick up any changes the user has typed in.
    Glib::ustring tmp = myFilename;   // get_filename();

    Inkscape::Extension::Output* newOut = extension ? dynamic_cast<Inkscape::Extension::Output*>(extension) : 0;
    if ( fileTypeCheckbox.get_active() && newOut ) {
        try {
            bool appendExtension = true;
            Glib::ustring utf8Name = Glib::filename_to_utf8( myFilename );
            Glib::ustring::size_type pos = utf8Name.rfind('.');
            if ( pos != Glib::ustring::npos ) {
                Glib::ustring trail = utf8Name.substr( pos );
                Glib::ustring foldedTrail = trail.casefold();
                if ( (trail == ".")
                     | (foldedTrail != Glib::ustring( newOut->get_extension() ).casefold()
                        && ( knownExtensions.find(foldedTrail) != knownExtensions.end() ) ) ) {
                    utf8Name = utf8Name.erase( pos );
                } else {
                    appendExtension = false;
                }
            }

            if (appendExtension) {
                utf8Name = utf8Name + newOut->get_extension();
                myFilename = Glib::filename_from_utf8( utf8Name );

            }
        } catch ( Glib::ConvertError& e ) {
            // ignore
        }
    }
}


//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Constructor
 */
FileExportToOCALPasswordDialogImpl::FileExportToOCALPasswordDialogImpl(Gtk::Window &parentWindow,
                             const Glib::ustring &title) : FileDialogOCALBase(title)
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
    //usernameEntry->set_alignment(1);

    passwordEntry = new Gtk::Entry();
    passwordEntry->set_text(myPassword);
    passwordEntry->set_max_length(255);
    passwordEntry->set_invisible_char('*');
    passwordEntry->set_visibility(false);
    //passwordEntry->set_alignment(1);
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
FileExportToOCALPasswordDialogImpl::~FileExportToOCALPasswordDialogImpl()
{
}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileExportToOCALPasswordDialogImpl::show()
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
FileExportToOCALPasswordDialogImpl::getUsername()
{
    myUsername = usernameEntry->get_text();
    return myUsername;
}

/**
 * Get the password.   Valid after an [OK]
 */
Glib::ustring
FileExportToOCALPasswordDialogImpl::getPassword()
{
    myPassword = passwordEntry->get_text();
    return myPassword;
}

void
FileExportToOCALPasswordDialogImpl::change_title(const Glib::ustring& title)
{
    this->set_title(title);
}


//#########################################################################
//### F I L E   I M P O R T   F R O M   O C A L
//#########################################################################


/*
 * Callback for row activated
 */
void FileListViewText::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
    // create file path
    myFilename = Glib::get_tmp_dir();
    myFilename.append(G_DIR_SEPARATOR_S);
    std::vector<int> posArray(1);
    posArray = path.get_indices();
    myFilename.append(get_text(posArray[0], 2));
    
#ifdef WITH_GNOME_VFS
    gnome_vfs_init();
    GnomeVFSHandle    *from_handle = NULL;
    GnomeVFSHandle    *to_handle = NULL;
    GnomeVFSFileSize  bytes_read;
    GnomeVFSFileSize  bytes_written;
    GnomeVFSResult    result;
    guint8 buffer[8192];

    //get file url
    Glib::ustring fileUrl = get_text(posArray[0], 1); //http url

    //Glib::ustring fileUrl = "dav://"; //dav url
    //fileUrl.append(prefs_get_string_attribute("options.ocalurl", "str"));
    //fileUrl.append("/dav.php/");
    //fileUrl.append(get_text(posArray[0], 3)); //author dir
    //fileUrl.append("/");
    //fileUrl.append(get_text(posArray[0], 2)); //filename

    if (!Glib::get_charset()) //If we are not utf8
        fileUrl = Glib::filename_to_utf8(fileUrl);

    // verifies if the file wasn't previously downloaded
    if(gnome_vfs_open(&to_handle, myFilename.c_str(), GNOME_VFS_OPEN_READ) == GNOME_VFS_ERROR_NOT_FOUND)
    {
        // open the temp file to receive
        result = gnome_vfs_open (&to_handle, myFilename.c_str(), GNOME_VFS_OPEN_WRITE);
        if (result == GNOME_VFS_ERROR_NOT_FOUND){
            result = gnome_vfs_create (&to_handle, myFilename.c_str(), GNOME_VFS_OPEN_WRITE, FALSE, GNOME_VFS_PERM_USER_ALL);
        }
        if (result != GNOME_VFS_OK) {
            g_warning("Error creating temp file: %s", gnome_vfs_result_to_string(result));
            return;
        }
        result = gnome_vfs_open (&from_handle, fileUrl.c_str(), GNOME_VFS_OPEN_READ);
        if (result != GNOME_VFS_OK) {
            g_warning("Could not find the file in Open Clip Art Library.");
            return;
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
                return;
            }
            result = gnome_vfs_write (to_handle, buffer, bytes_read, &bytes_written);
            if (result != GNOME_VFS_OK) {
                g_warning("%s", gnome_vfs_result_to_string(result));
                return;
            }
            if (bytes_read != bytes_written){
                g_warning("Bytes read not equal to bytes written");
                return;
            }
        }
    }
    else
    {
        gnome_vfs_close(to_handle);
    }
    myPreview->showImage(myFilename);
    myLabel->set_text(get_text(posArray[0], 4));
#endif
}


/*
 * Returns the selected filename
 */
Glib::ustring FileListViewText::getFilename()
{
    return myFilename;
}

/**
 * Callback for user input into searchTagEntry
 */
void FileImportFromOCALDialogImplGtk::searchTagEntryChangedCallback()
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

    // get the rss feed
    gnome_vfs_init();
    GnomeVFSHandle    *from_handle = NULL;
    GnomeVFSHandle    *to_handle = NULL;
    GnomeVFSFileSize  bytes_read;
    GnomeVFSFileSize  bytes_written;
    GnomeVFSResult    result;
    guint8 buffer[8192];

    // create the temp file name
    Glib::ustring fileName = Glib::get_tmp_dir ();
    fileName.append(G_DIR_SEPARATOR_S);
    fileName.append("ocalfeed.xml");

    // open the temp file to receive
    result = gnome_vfs_open (&to_handle, fileName.c_str(), GNOME_VFS_OPEN_WRITE);
    if (result == GNOME_VFS_ERROR_NOT_FOUND){
        result = gnome_vfs_create (&to_handle, fileName.c_str(), GNOME_VFS_OPEN_WRITE, FALSE, GNOME_VFS_PERM_USER_ALL);
    }
    if (result != GNOME_VFS_OK) {
        g_warning("Error creating temp file: %s", gnome_vfs_result_to_string(result));
        return;
    }

    // open the rss feed
    result = gnome_vfs_open (&from_handle, uri.c_str(), GNOME_VFS_OPEN_READ);
    if (result != GNOME_VFS_OK) {
        sp_ui_error_dialog(_("Failed to receive the Open Clip Art Library RSS feed. Verify if the server name is correct in Configuration->Misc (e.g.: openclipart.org)"));
        return;
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
            return;
        }
        result = gnome_vfs_write (to_handle, buffer, bytes_read, &bytes_written);
        if (result != GNOME_VFS_OK) {
            g_warning("%s", gnome_vfs_result_to_string(result));
            return;
        }

        if (bytes_read != bytes_written){
            g_warning("Bytes read not equal to bytes written");
            return;
        }

    }

    // create the resulting xml document tree
    // this initialize the library and test mistakes between compiled and shared library used
    LIBXML_TEST_VERSION 
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    doc = xmlReadFile(fileName.c_str(), NULL, 0);
    if (doc == NULL) {
        g_warning("Failed to parse %s\n", fileName.c_str());
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
void FileImportFromOCALDialogImplGtk::print_xml_element_names(xmlNode * a_node)
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
FileImportFromOCALDialogImplGtk::FileImportFromOCALDialogImplGtk(Gtk::Window& parentWindow, 
		                       const Glib::ustring &dir,
                                       FileDialogType fileTypes,
                                       const Glib::ustring &title) :
     FileDialogOCALBase(title)
{
    // Initalize to Autodetect
    extension = NULL;
    // No filename to start out with
    Glib::ustring searchTag = "";

    dialogType = fileTypes;
    Gtk::VBox *vbox = get_vbox();
    Gtk::Label *tagLabel = new Gtk::Label(_("Search Tag"));
    notFoundLabel = new Gtk::Label(_("No files matched your search"));
    descriptionLabel = new Gtk::Label();
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
    filesList = new FileListViewText(5, *filesPreview, *descriptionLabel);
    filesList->set_sensitive(false);
    // add the listview inside a ScrolledWindow
    listScrolledWindow.add(*filesList);
    // only show the scrollbars when they are necessary:
    listScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    filesList->set_column_title(0, _("Files Found"));
    listScrolledWindow.set_size_request(400, 180);
    filesList->get_column(1)->set_visible(false); // file url
    filesList->get_column(2)->set_visible(false); // tmp file path
    filesList->get_column(3)->set_visible(false); // author dir
    filesList->get_column(4)->set_visible(false); // file description
    filesBox.pack_start(listScrolledWindow);
    filesBox.pack_start(*filesPreview);
    vbox->pack_start(tagBox);
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
              sigc::mem_fun(*this, &FileImportFromOCALDialogImplGtk::searchTagEntryChangedCallback));
    }

    searchButton->signal_clicked().connect(
            sigc::mem_fun(*this, &FileImportFromOCALDialogImplGtk::searchTagEntryChangedCallback));

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK));

    show_all_children();
    notFoundLabel->hide();
}

/**
 * Destructor
 */
FileImportFromOCALDialogImplGtk::~FileImportFromOCALDialogImplGtk()
{

}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileImportFromOCALDialogImplGtk::show()
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
FileImportFromOCALDialogImplGtk::getSelectionType()
{
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
Glib::ustring
FileImportFromOCALDialogImplGtk::getFilename (void)
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

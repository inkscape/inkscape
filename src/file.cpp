#define __SP_FILE_C__

/*
 * File/Print operations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Chema Celorio <chema@celorio.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2004 David Turner
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/**
 * Note: This file needs to be cleaned up extensively.
 * What it probably needs is to have one .h file for
 * the API, and two or more .cpp files for the implementations.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gmem.h>
#include <libnr/nr-pixops.h>

#include "document-private.h"
#include "selection-chemistry.h"
#include "ui/view/view-widget.h"
#include "dir-util.h"
#include "helper/png-write.h"
#include "dialogs/export.h"
#include <glibmm/i18n.h>
#include "inkscape.h"
#include "desktop.h"
#include "selection.h"
#include "interface.h"
#include "style.h"
#include "print.h"
#include "file.h"
#include "message-stack.h"
#include "dialogs/filedialog.h"
#include "prefs-utils.h"
#include "path-prefix.h"

#include "sp-namedview.h"
#include "desktop-handles.h"

#include "extension/db.h"
#include "extension/input.h"
#include "extension/output.h"
/* #include "extension/menu.h"  */
#include "extension/system.h"

#include "io/sys.h"
#include "application/application.h"
#include "application/editor.h"
#include "inkscape.h"
#include "uri.h"

#ifdef WITH_INKBOARD
#include "jabber_whiteboard/session-manager.h"
#endif

/**
 * 'Current' paths.  Used to remember which directory
 * had the last file accessed.
 * Static globals are evil.  This will be gone soon
 * as C++ification continues
 */
static gchar *import_path = NULL;

//#define INK_DUMP_FILENAME_CONV 1
#undef INK_DUMP_FILENAME_CONV

//#define INK_DUMP_FOPEN 1
#undef INK_DUMP_FOPEN

void dump_str(gchar const *str, gchar const *prefix);
void dump_ustr(Glib::ustring const &ustr);


/*######################
## N E W
######################*/

/**
 * Create a blank document and add it to the desktop
 */
SPDesktop*
sp_file_new(gchar const *templ)
{
    SPDocument *doc = sp_document_new(templ, TRUE, true);
    g_return_val_if_fail(doc != NULL, NULL);

    SPDesktop *dt;
    if (Inkscape::NSApplication::Application::getNewGui())
    {
        dt = Inkscape::NSApplication::Editor::createDesktop (doc);
    } else {
        SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
        g_return_val_if_fail(dtw != NULL, NULL);
        sp_document_unref(doc);

        sp_create_window(dtw, TRUE);
        dt = static_cast<SPDesktop*>(dtw->view);
        sp_namedview_window_from_document(dt);
    }
    return dt;
}

SPDesktop*
sp_file_new_default()
{
    std::list<gchar *> sources;
    sources.push_back( profile_path("templates") ); // first try user's local dir
    sources.push_back( g_strdup(INKSCAPE_TEMPLATESDIR) ); // then the system templates dir

    while (!sources.empty()) {
        gchar *dirname = sources.front();
        if ( Inkscape::IO::file_test( dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) ) ) {

            // TRANSLATORS: default.svg is localizable - this is the name of the default document
            //  template. This way you can localize the default pagesize, translate the name of
            //  the default layer, etc. If you wish to localize this file, please create a
            //  localized share/templates/default.xx.svg file, where xx is your language code.
            char *default_template = g_build_filename(dirname, _("default.svg"), NULL);
            if (Inkscape::IO::file_test(default_template, G_FILE_TEST_IS_REGULAR)) {
                return sp_file_new(default_template);
            }
        }
        g_free(dirname);
        sources.pop_front();
    }

    return sp_file_new(NULL);
}


/*######################
## D E L E T E
######################*/

/**
 *  Perform document closures preceding an exit()
 */
void
sp_file_exit()
{
    sp_ui_close_all();
    // no need to call inkscape_exit here; last document being closed will take care of that
}


/*######################
## O P E N
######################*/

/**
 *  Open a file, add the document to the desktop
 *
 *  \param replace_empty if true, and the current desktop is empty, this document
 *  will replace the empty one.
 */
bool
sp_file_open(gchar const *uri, Inkscape::Extension::Extension *key, bool add_to_recent, bool replace_empty)
{
    SPDocument *doc;
    try {
        doc = Inkscape::Extension::open(key, uri);
    } catch (Inkscape::Extension::Input::no_extension_found &e) {
        doc = NULL;
    } catch (Inkscape::Extension::Input::open_failed &e) {
        doc = NULL;
    }

    if (doc) {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        SPDocument *existing = desktop ? sp_desktop_document(desktop) : NULL;

        if (existing && existing->virgin && replace_empty) {
            // If the current desktop is empty, open the document there
            sp_document_ensure_up_to_date (doc);
            desktop->change_document(doc);
            sp_document_resized_signal_emit (doc, sp_document_width(doc), sp_document_height(doc));
        } else {
            if (!Inkscape::NSApplication::Application::getNewGui()) {
                // create a whole new desktop and window
                SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
                sp_create_window(dtw, TRUE);
                desktop = static_cast<SPDesktop*>(dtw->view);
            } else {
                desktop = Inkscape::NSApplication::Editor::createDesktop (doc);
            }
        }

        doc->virgin = FALSE;
        // everyone who cares now has a reference, get rid of ours
        sp_document_unref(doc);
        // resize the window to match the document properties
        // (this may be redundant for new windows... if so, move to the "virgin"
        //  section above)
        sp_namedview_window_from_document(desktop);

        if (add_to_recent) {
            prefs_set_recent_file(SP_DOCUMENT_URI(doc), SP_DOCUMENT_NAME(doc));
        }

        return TRUE;
    } else {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri);
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), safeUri);
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    }
}

/**
 *  Handle prompting user for "do you want to revert"?  Revert on "OK"
 */
void
sp_file_revert_dialog()
{
    SPDesktop  *desktop = SP_ACTIVE_DESKTOP;
    g_assert(desktop != NULL);

    SPDocument *doc = sp_desktop_document(desktop);
    g_assert(doc != NULL);

    Inkscape::XML::Node     *repr = sp_document_repr_root(doc);
    g_assert(repr != NULL);

    gchar const *uri = doc->uri;
    if (!uri) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved yet.  Cannot revert."));
        return;
    }

    bool do_revert = true;
    if (repr->attribute("sodipodi:modified") != NULL) {
        gchar *text = g_strdup_printf(_("Changes will be lost!  Are you sure you want to reload document %s?"), uri);

        bool response = desktop->warnDialog (text);
        g_free(text);

        if (!response) {
            do_revert = false;
        }
    }

    bool reverted;
    if (do_revert) {
        // Allow overwriting of current document.
        doc->virgin = TRUE;
        reverted = sp_file_open(uri,NULL);
    } else {
        reverted = false;
    }

    if (reverted) {
        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Document reverted."));
    } else {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not reverted."));
    }
}

void dump_str(gchar const *str, gchar const *prefix)
{
    Glib::ustring tmp;
    tmp = prefix;
    tmp += " [";
    size_t const total = strlen(str);
    for (unsigned i = 0; i < total; i++) {
        gchar *const tmp2 = g_strdup_printf(" %02x", (0x0ff & str[i]));
        tmp += tmp2;
        g_free(tmp2);
    }

    tmp += "]";
    g_message(tmp.c_str());
}

void dump_ustr(Glib::ustring const &ustr)
{
    char const *cstr = ustr.c_str();
    char const *data = ustr.data();
    Glib::ustring::size_type const byteLen = ustr.bytes();
    Glib::ustring::size_type const dataLen = ustr.length();
    Glib::ustring::size_type const cstrLen = strlen(cstr);

    g_message("   size: %lu\n   length: %lu\n   bytes: %lu\n    clen: %lu",
              gulong(ustr.size()), gulong(dataLen), gulong(byteLen), gulong(cstrLen) );
    g_message( "  ASCII? %s", (ustr.is_ascii() ? "yes":"no") );
    g_message( "  UTF-8? %s", (ustr.validate() ? "yes":"no") );

    try {
        Glib::ustring tmp;
        for (Glib::ustring::size_type i = 0; i < ustr.bytes(); i++) {
            tmp = "    ";
            if (i < dataLen) {
                Glib::ustring::value_type val = ustr.at(i);
                gchar* tmp2 = g_strdup_printf( (((val & 0xff00) == 0) ? "  %02x" : "%04x"), val );
                tmp += tmp2;
                g_free( tmp2 );
            } else {
                tmp += "    ";
            }

            if (i < byteLen) {
                int val = (0x0ff & data[i]);
                gchar *tmp2 = g_strdup_printf("    %02x", val);
                tmp += tmp2;
                g_free( tmp2 );
                if ( val > 32 && val < 127 ) {
                    tmp2 = g_strdup_printf( "   '%c'", (gchar)val );
                    tmp += tmp2;
                    g_free( tmp2 );
                } else {
                    tmp += "    . ";
                }
            } else {
                tmp += "       ";
            }

            if ( i < cstrLen ) {
                int val = (0x0ff & cstr[i]);
                gchar* tmp2 = g_strdup_printf("    %02x", val);
                tmp += tmp2;
                g_free(tmp2);
                if ( val > 32 && val < 127 ) {
                    tmp2 = g_strdup_printf("   '%c'", (gchar) val);
                    tmp += tmp2;
                    g_free( tmp2 );
                } else {
                    tmp += "    . ";
                }
            } else {
                tmp += "            ";
            }

            g_message( tmp.c_str() );
        }
    } catch (...) {
        g_message("XXXXXXXXXXXXXXXXXX Exception" );
    }
    g_message("---------------");
}

static Inkscape::UI::Dialogs::FileOpenDialog *openDialogInstance = NULL;

/**
 *  Display an file Open selector.  Open a document if OK is pressed.
 *  Can select single or multiple files for opening.
 */
void
sp_file_open_dialog(gpointer object, gpointer data)
{
    gchar *open_path2 = NULL;

    gchar *open_path = g_strdup(prefs_get_string_attribute("dialogs.open", "path"));
    if (open_path != NULL && open_path[0] == '\0') {
        g_free(open_path);
        open_path = NULL;
    }
    if (open_path && !Inkscape::IO::file_test(open_path, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
        g_free(open_path);
        open_path = NULL;
    }
    if (open_path == NULL)
        open_path = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, NULL);

    if (!openDialogInstance) {
        openDialogInstance =
              Inkscape::UI::Dialogs::FileOpenDialog::create(
                 (char const *)open_path,
                 Inkscape::UI::Dialogs::SVG_TYPES,
                 (char const *)_("Select file to open"));
    }
    bool const success = openDialogInstance->show();
    gchar *fileName = ( success
                        ? g_strdup(openDialogInstance->getFilename())
                        : NULL );
    Inkscape::Extension::Extension *selection =
            openDialogInstance->getSelectionType();
    g_free(open_path);

    if (!success) return;

    // Code to check & open iff multiple files.
    Glib::SListHandle<Glib::ustring> flist=openDialogInstance->getFilenames();
    GSList *list=flist.data();

    if(g_slist_length(list)>1)
    {
        gchar *fileName=NULL;

        while(list!=NULL)
        {

#ifdef INK_DUMP_FILENAME_CONV
            g_message(" FileName: %s",(const char *)list->data);
#endif

            fileName=(gchar *)g_strdup((gchar *)list->data);

            if (fileName && !g_file_test(fileName,G_FILE_TEST_IS_DIR)) {
                gsize bytesRead = 0;
                gsize bytesWritten = 0;
                GError *error = NULL;
#ifdef INK_DUMP_FILENAME_CONV
                dump_str( fileName, "A file pre  is " );
#endif
                gchar *newFileName = g_filename_to_utf8(fileName,
                                                -1,
                                                        &bytesRead,
                                                        &bytesWritten,
                                                        &error);
                if ( newFileName != NULL ) {
                    g_free(fileName);
                    fileName = newFileName;
#ifdef INK_DUMP_FILENAME_CONV
                    dump_str( fileName, "A file post is " );
#endif
                } else {
                    // TODO: bulia, please look over
                    g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );
                }

#ifdef INK_DUMP_FILENAME_CONV
                g_message("Opening File %s\n",fileName);
#endif

                sp_file_open(fileName, selection);
                g_free(fileName);
            }
            else
            {
                g_message("Cannot Open Directory %s\n",fileName);
            }

            list=list->next;
        }

        return;
    }


    if (fileName) {
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error = NULL;
#ifdef INK_DUMP_FILENAME_CONV
        dump_str( fileName, "A file pre  is " );
#endif
        gchar *newFileName = g_filename_to_utf8(fileName,
                                                -1,
                                                &bytesRead,
                                                &bytesWritten,
                                                &error);
        if ( newFileName != NULL ) {
            g_free(fileName);
            fileName = newFileName;
#ifdef INK_DUMP_FILENAME_CONV
            dump_str( fileName, "A file post is " );
#endif
        } else {
            // TODO: bulia, please look over
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );
        }


        if ( !g_utf8_validate(fileName, -1, NULL) ) {
            // TODO: bulia, please look over
            g_warning( "INPUT FILENAME IS NOT UTF-8" );
        }


        open_path = g_dirname(fileName);
        open_path2 = g_strconcat(open_path, G_DIR_SEPARATOR_S, NULL);
        prefs_set_string_attribute("dialogs.open", "path", open_path2);
        g_free(open_path);
        g_free(open_path2);

        sp_file_open(fileName, selection);
        g_free(fileName);
    }

    return;
}


/*######################
## V A C U U M
######################*/

/**
 * Remove unreferenced defs from the defs section of the document.
 */


void
sp_file_vacuum()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    unsigned int diff = vacuum_document (doc);

    sp_document_done(doc);

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (diff > 0) {
        dt->messageStack()->flashF(Inkscape::NORMAL_MESSAGE,
                ngettext("Removed <b>%i</b> unused definition in &lt;defs&gt;.",
                         "Removed <b>%i</b> unused definitions in &lt;defs&gt;.",
                         diff),
                diff);
    } else {
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE,  _("No unused definitions in &lt;defs&gt;."));
    }
}



/*######################
## S A V E
######################*/

/**
 * This 'save' function called by the others below
 */
static bool
file_save(SPDocument *doc, gchar const *uri, Inkscape::Extension::Extension *key, bool saveas)
{
    if (!doc || !uri) //Safety check
        return FALSE;

    try {
        Inkscape::Extension::save(key, doc, uri,
                                  saveas && prefs_get_int_attribute("dialogs.save_as", "append_extension", 1),
                                  saveas, TRUE); // save officially, with inkscape: attributes set
    } catch (Inkscape::Extension::Output::no_extension_found &e) {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri);
        gchar *text = g_strdup_printf(_("No Inkscape extension found to save document (%s).  This may have been caused by an unknown filename extension."), safeUri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    } catch (Inkscape::Extension::Output::save_failed &e) {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri);
        gchar *text = g_strdup_printf(_("File %s could not be saved."), safeUri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    } catch (Inkscape::Extension::Output::no_overwrite &e) {
        return sp_file_save_dialog(doc);
    }

    SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Document saved."));
    return TRUE;
}

static Inkscape::UI::Dialogs::FileSaveDialog *saveDialogInstance = NULL;

/**
 *  Display a SaveAs dialog.  Save the document if OK pressed.
 */
gboolean
sp_file_save_dialog(SPDocument *doc)
{
    Inkscape::XML::Node *repr = sp_document_repr_root(doc);
    gchar const *default_extension = NULL;
    gchar *save_loc;
    Inkscape::Extension::Output *extension;
    gchar *save_path = NULL;

    default_extension = repr->attribute("inkscape:output_extension");
    if (default_extension == NULL) {
        default_extension = prefs_get_string_attribute("dialogs.save_as", "default");
    }
    //g_warning("%s: extension name: '%s'", __FUNCTION__, default_extension);

    if (doc->uri == NULL) {
        int i = 1;
        char const *filename_extension;
        char *temp_filename;

        extension = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(default_extension));
        //g_warning("%s: extension ptr: 0x%x", __FUNCTION__, (unsigned int)extension);
        if (extension == NULL) {
            filename_extension = ".svg";
        } else {
            filename_extension = extension->get_extension();
        }

        save_path = g_strdup(prefs_get_string_attribute("dialogs.save_as", "path"));
        if (save_path != NULL && save_path[0] == '\0') {
            g_free(save_path);
            save_path = NULL;
        }
        if (save_path && !Inkscape::IO::file_test(save_path, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
            g_free(save_path);
            save_path = NULL;
        }
        if (save_path == NULL)
            save_path = g_strdup(g_get_home_dir());
        temp_filename = g_strdup_printf(_("drawing%s"), filename_extension);
        save_loc = g_build_filename(save_path, temp_filename, NULL);
        g_free(temp_filename);

        while (Inkscape::IO::file_test(save_loc, G_FILE_TEST_EXISTS)) {
            g_free(save_loc);
            temp_filename = g_strdup_printf(_("drawing-%d%s"), i++, filename_extension);
            save_loc = g_build_filename(save_path, temp_filename, NULL);
            g_free(temp_filename);
        }
    } else {
        save_loc = g_path_get_dirname(doc->uri); /* \todo should use a getter */
    }

    { // convert save_loc from utf-8 to locale
      // is this needed any more, now that everything is handled in
      // Inkscape::IO?
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError* error = NULL;
#ifdef INK_DUMP_FILENAME_CONV
        dump_str( save_loc, "B file pre  is " );
#endif
        gchar* save_loc_local = g_filename_from_utf8( save_loc, -1, &bytesRead, &bytesWritten, &error);

        if ( save_loc_local != NULL ) {
            g_free(save_loc);
            save_loc = save_loc_local;
#ifdef INK_DUMP_FILENAME_CONV
            dump_str( save_loc, "B file post is " );
#endif
        } else {
            //g_warning( "Error converting save filename stored in the file to locale encoding.");
        }
    }

    if (!saveDialogInstance) {
        saveDialogInstance =
             Inkscape::UI::Dialogs::FileSaveDialog::create(
                 (char const *) save_loc,
                 Inkscape::UI::Dialogs::SVG_TYPES,
                 (char const *) _("Select file to save to"),
                 default_extension
            );
    } // FIXME: else (i.e. if reshowing an already shown dialog) save_loc is not used, it thus always displays the previously opened dir
    bool success = saveDialogInstance->show();
    char *fileName = ( success
                       ? g_strdup(saveDialogInstance->getFilename())
                       : NULL );
    Inkscape::Extension::Extension *selectionType =
        saveDialogInstance->getSelectionType();
    g_free(save_loc);
    g_free(save_path);
    if (!success) {
        return success;
    }

    if (fileName && *fileName) {
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error = NULL;
#ifdef INK_DUMP_FILENAME_CONV
        dump_str( fileName, "C file pre  is " );
#endif
        gchar *newFileName = g_filename_to_utf8(fileName,
                                                -1,
                                                &bytesRead,
                                                &bytesWritten,
                                                &error);
        if ( newFileName != NULL ) {
            g_free(fileName);
            fileName = newFileName;
#ifdef INK_DUMP_FILENAME_CONV
            dump_str( fileName, "C file post is " );
#endif
        } else {
            g_warning( "Error converting save filename to UTF-8." );
        }

        if (!g_utf8_validate(fileName, -1, NULL)) {
            // TODO: bulia, please look over
            g_warning( "The filename is not UTF-8." );
        }

        success = file_save(doc, fileName, selectionType, TRUE);

        if (success) {
            prefs_set_recent_file(SP_DOCUMENT_URI(doc), SP_DOCUMENT_NAME(doc));
        }

        save_path = g_dirname(fileName);
        prefs_set_string_attribute("dialogs.save_as", "path", save_path);
        g_free(save_path);

        g_free(fileName);
        return success;
    } else {
        return FALSE;
    }
}


/**
 * Save a document, displaying a SaveAs dialog if necessary.
 */
gboolean
sp_file_save_document(SPDocument *doc)
{
    gboolean success = TRUE;

    Inkscape::XML::Node *repr = sp_document_repr_root(doc);

    gchar const *fn = repr->attribute("sodipodi:modified");
    if (fn != NULL) {
        if (doc->uri == NULL
            || repr->attribute("inkscape:output_extension") == NULL)
        {
            return sp_file_save_dialog(doc);
        } else {
            fn = g_strdup(doc->uri);
            gchar const *ext = repr->attribute("inkscape:output_extension");
            success = file_save(doc, fn, Inkscape::Extension::db.get(ext), FALSE);
            g_free((void *) fn);
        }
    } else {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No changes need to be saved."));
        success = TRUE;
    }

    return success;
}


/**
 * Save a document.
 */
bool
sp_file_save(gpointer object, gpointer data)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;
    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_document(SP_ACTIVE_DOCUMENT);
}


/**
 *  Save a document, always displaying the SaveAs dialog.
 */
bool
sp_file_save_as(gpointer object, gpointer data)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;
    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_dialog(SP_ACTIVE_DOCUMENT);
}




/*######################
## I M P O R T
######################*/

/**
 *  Import a resource.  Called by sp_file_import()
 */
void
file_import(SPDocument *in_doc, gchar const *uri, Inkscape::Extension::Extension *key)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    //DEBUG_MESSAGE( fileImport, "file_import( in_doc:%p uri:[%s], key:%p", in_doc, uri, key );
    SPDocument *doc;
    try {
        doc = Inkscape::Extension::open(key, uri);
    } catch (Inkscape::Extension::Input::no_extension_found &e) {
        doc = NULL;
    } catch (Inkscape::Extension::Input::open_failed &e) {
        doc = NULL;
    }

    if (doc != NULL) {
        // the import extension has passed us a document, now we need to embed it into our document
        if ( 0 ) {
//            const gchar *docbase = (sp_repr_document_root( sp_repr_document( repr ))->attribute("sodipodi:docbase" );
            g_message(" settings  uri  [%s]", doc->uri );
            g_message("           base [%s]", doc->base );
            g_message("           name [%s]", doc->name );
            Inkscape::IO::fixupHrefs( doc, doc->base, TRUE );
            g_message("        mid-fixup");
            Inkscape::IO::fixupHrefs( doc, in_doc->base, TRUE );
        }

        // move imported defs to our document's defs
        SPObject *in_defs = SP_DOCUMENT_DEFS(in_doc);
        SPObject *defs = SP_DOCUMENT_DEFS(doc);
        Inkscape::XML::Node *last_def = SP_OBJECT_REPR(in_defs)->lastChild();
        for (SPObject *child = sp_object_first_child(defs);
             child != NULL; child = SP_OBJECT_NEXT(child))
        {
            // FIXME: in case of id conflict, newly added thing will be re-ided and thus likely break a reference to it from imported stuff
            SP_OBJECT_REPR(in_defs)->addChild(SP_OBJECT_REPR(child)->duplicate(), last_def);
        }

        guint items_count = 0;
        for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc));
             child != NULL; child = SP_OBJECT_NEXT(child)) {
            if (SP_IS_ITEM(child))
                items_count ++;
        }
        SPCSSAttr *style = sp_css_attr_from_object (SP_DOCUMENT_ROOT (doc));

        SPObject *new_obj = NULL;

        if ((style && style->firstChild()) || items_count > 1) {
            // create group
            Inkscape::XML::Node *newgroup = sp_repr_new("svg:g");
            sp_repr_css_set (newgroup, style, "style");

            for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
                if (SP_IS_ITEM(child)) {
                    Inkscape::XML::Node *newchild = SP_OBJECT_REPR(child)->duplicate();

                    // convert layers to groups; FIXME: add "preserve layers" mode where each layer
                    // from impot is copied to the same-named layer in host
                    newchild->setAttribute("inkscape:groupmode", NULL);

                    newgroup->appendChild(newchild);
                }
            }

            if (desktop) {
                // Add it to the current layer
                new_obj = desktop->currentLayer()->appendChildRepr(newgroup);
            } else {
                // There's no desktop (command line run?)
                // FIXME: For such cases we need a document:: method to return the current layer
                new_obj = SP_DOCUMENT_ROOT(in_doc)->appendChildRepr(newgroup);
            }

            Inkscape::GC::release(newgroup);
        } else {
            // just add one item
            for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
                if (SP_IS_ITEM(child)) {
                    Inkscape::XML::Node *newitem = SP_OBJECT_REPR(child)->duplicate();
                    newitem->setAttribute("inkscape:groupmode", NULL);

                    if (desktop) {
                        // Add it to the current layer
                        new_obj = desktop->currentLayer()->appendChildRepr(newitem);
                    } else {
                        // There's no desktop (command line run?)
                        // FIXME: For such cases we need a document:: method to return the current layer
                        new_obj = SP_DOCUMENT_ROOT(in_doc)->appendChildRepr(newitem);
                    }

                }
            }
        }

        if (style) sp_repr_css_attr_unref (style);

        // select and move the imported item
        if (new_obj && SP_IS_ITEM(new_obj)) {
            Inkscape::Selection *selection = sp_desktop_selection(desktop);
            selection->set(SP_ITEM(new_obj));

            // To move the imported object, we must temporarily set the "transform pattern with
            // object" option.
            {
                int const saved_pref = prefs_get_int_attribute("options.transform", "pattern", 1);
                prefs_set_int_attribute("options.transform", "pattern", 1);
                sp_document_ensure_up_to_date(sp_desktop_document(desktop));
                NR::Point m( desktop->point() - selection->bounds().midpoint() );
                sp_selection_move_relative(selection, m);
                prefs_set_int_attribute("options.transform", "pattern", saved_pref);
            }
        }

        sp_document_unref(doc);
        sp_document_done(in_doc);

    } else {
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), uri);
        sp_ui_error_dialog(text);
        g_free(text);
    }

    return;
}


static Inkscape::UI::Dialogs::FileOpenDialog *importDialogInstance = NULL;

/**
 *  Display an Open dialog, import a resource if OK pressed.
 */
void
sp_file_import(GtkWidget *widget)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!doc)
        return;

    if (!importDialogInstance) {
        importDialogInstance =
             Inkscape::UI::Dialogs::FileOpenDialog::create(
                 (char const *)import_path,
                 Inkscape::UI::Dialogs::IMPORT_TYPES,
                 (char const *)_("Select file to import"));
    }
    bool success = importDialogInstance->show();
    char *fileName = ( success
                       ? g_strdup(importDialogInstance->getFilename())
                       : NULL );
    Inkscape::Extension::Extension *selection =
        importDialogInstance->getSelectionType();

    if (!success) return;
    if (fileName) {
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error = NULL;
#ifdef INK_DUMP_FILENAME_CONV
        dump_str( fileName, "D file pre  is " );
#endif
        gchar *newFileName = g_filename_to_utf8( fileName,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
        if ( newFileName != NULL ) {
            g_free(fileName);
            fileName = newFileName;
#ifdef INK_DUMP_FILENAME_CONV
            dump_str( fileName, "D file post is " );
#endif
        } else {
            // TODO: bulia, please look over
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );
        }


        if (!g_utf8_validate(fileName, -1, NULL)) {
            // TODO: bulia, please look over
            g_warning( "INPUT FILENAME IS NOT UTF-8" );
        }

        g_free(import_path);
        import_path = g_dirname(fileName);
        if (import_path) import_path = g_strconcat(import_path, G_DIR_SEPARATOR_S, NULL);

        file_import(doc, fileName, selection);
        g_free(fileName);
    }

    return;
}



/*######################
## E X P O R T
######################*/

/**
 *
 */
void
sp_file_export_dialog(void *widget)
{
    sp_export_dialog();
}

#include <display/nr-arena-item.h>
#include <display/nr-arena.h>

struct SPEBP {
    int width, height, sheight;
    guchar r, g, b, a;
    NRArenaItem *root; // the root arena item to show; it is assumed that all unneeded items are hidden
    guchar *px;
    unsigned (*status)(float, void *);
    void *data;
};


/**
 *
 */
static int
sp_export_get_rows(guchar const **rows, int row, int num_rows, void *data)
{
    struct SPEBP *ebp = (struct SPEBP *) data;

    if (ebp->status) {
        if (!ebp->status((float) row / ebp->height, ebp->data)) return 0;
    }

    num_rows = MIN(num_rows, ebp->sheight);
    num_rows = MIN(num_rows, ebp->height - row);

    /* Set area of interest */
    NRRectL bbox;
    bbox.x0 = 0;
    bbox.y0 = row;
    bbox.x1 = ebp->width;
    bbox.y1 = row + num_rows;
    /* Update to renderable state */
    NRGC gc(NULL);
    nr_matrix_set_identity(&gc.transform);

    nr_arena_item_invoke_update(ebp->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

    NRPixBlock pb;
    nr_pixblock_setup_extern(&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                             bbox.x0, bbox.y0, bbox.x1, bbox.y1,
                             ebp->px, 4 * ebp->width, FALSE, FALSE);

    for (int r = 0; r < num_rows; r++) {
        guchar *p = NR_PIXBLOCK_PX(&pb) + r * pb.rs;
        for (int c = 0; c < ebp->width; c++) {
            *p++ = ebp->r;
            *p++ = ebp->g;
            *p++ = ebp->b;
            *p++ = ebp->a;
        }
    }

    /* Render */
    nr_arena_item_invoke_render(ebp->root, &bbox, &pb, 0);

    for (int r = 0; r < num_rows; r++) {
        rows[r] = NR_PIXBLOCK_PX(&pb) + r * pb.rs;
    }

    nr_pixblock_release(&pb);

    return num_rows;
}

/**
Hide all items which are not listed in list, recursively, skipping groups and defs
*/
void
hide_other_items_recursively(SPObject *o, GSList *list, unsigned dkey)
{
    if (SP_IS_ITEM(o)
        && !SP_IS_DEFS(o)
        && !SP_IS_ROOT(o)
        && !SP_IS_GROUP(o)
        && !g_slist_find(list, o))
    {
        sp_item_invoke_hide(SP_ITEM(o), dkey);
    }

     // recurse
    if (!g_slist_find(list, o)) {
        for (SPObject *child = sp_object_first_child(o) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            hide_other_items_recursively(child, list, dkey);
        }
    }
}


/**
 *  Render the SVG drawing onto a PNG raster image, then save to
 *  a file.  Returns TRUE if succeeded in writing the file,
 *  FALSE otherwise.
 */
int
sp_export_png_file(SPDocument *doc, gchar const *filename,
                   double x0, double y0, double x1, double y1,
                   unsigned width, unsigned height, double xdpi, double ydpi,
                   unsigned long bgcolor,
                   unsigned (*status)(float, void *),
                   void *data, bool force_overwrite,
                   GSList *items_only)
{
    int write_status = TRUE;
    g_return_val_if_fail(doc != NULL, FALSE);
    g_return_val_if_fail(filename != NULL, FALSE);
    g_return_val_if_fail(width >= 1, FALSE);
    g_return_val_if_fail(height >= 1, FALSE);

    if (!force_overwrite && !sp_ui_overwrite_file(filename)) {
        return FALSE;
    }

    sp_document_ensure_up_to_date(doc);

    /* Go to document coordinates */
    gdouble t = y0;
    y0 = sp_document_height(doc) - y1;
    y1 = sp_document_height(doc) - t;

    /*
     * 1) a[0] * x0 + a[2] * y1 + a[4] = 0.0
     * 2) a[1] * x0 + a[3] * y1 + a[5] = 0.0
     * 3) a[0] * x1 + a[2] * y1 + a[4] = width
     * 4) a[1] * x0 + a[3] * y0 + a[5] = height
     * 5) a[1] = 0.0;
     * 6) a[2] = 0.0;
     *
     * (1,3) a[0] * x1 - a[0] * x0 = width
     * a[0] = width / (x1 - x0)
     * (2,4) a[3] * y0 - a[3] * y1 = height
     * a[3] = height / (y0 - y1)
     * (1) a[4] = -a[0] * x0
     * (2) a[5] = -a[3] * y1
     */

    NRMatrix affine;
    affine.c[0] = width / (x1 - x0);
    affine.c[1] = 0.0;
    affine.c[2] = 0.0;
    affine.c[3] = height / (y1 - y0);
    affine.c[4] = -affine.c[0] * x0;
    affine.c[5] = -affine.c[3] * y0;

    //SP_PRINT_MATRIX("SVG2PNG", &affine);

    struct SPEBP ebp;
    ebp.width  = width;
    ebp.height = height;
    ebp.r      = NR_RGBA32_R(bgcolor);
    ebp.g      = NR_RGBA32_G(bgcolor);
    ebp.b      = NR_RGBA32_B(bgcolor);
    ebp.a      = NR_RGBA32_A(bgcolor);

    /* Create new arena */
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);

    /* Create ArenaItems and set transform */
    ebp.root = sp_item_invoke_show(SP_ITEM(sp_document_root(doc)), arena, dkey, SP_ITEM_SHOW_DISPLAY);
    nr_arena_item_set_transform(NR_ARENA_ITEM(ebp.root), NR::Matrix(&affine));

    // We show all and then hide all items we don't want, instead of showing only requested items,
    // because that would not work if the shown item references something in defs
    if (items_only) {
        hide_other_items_recursively(sp_document_root(doc), items_only, dkey);
    }

    ebp.status = status;
    ebp.data   = data;

    if ((width < 256) || ((width * height) < 32768)) {
        ebp.px = nr_pixelstore_64K_new(FALSE, 0);
        ebp.sheight = 65536 / (4 * width);
        write_status = sp_png_write_rgba_striped(filename, width, height, xdpi, ydpi, sp_export_get_rows, &ebp);
        nr_pixelstore_64K_free(ebp.px);
    } else {
        ebp.px = g_new(guchar, 4 * 64 * width);
        ebp.sheight = 64;
        write_status = sp_png_write_rgba_striped(filename, width, height, xdpi, ydpi, sp_export_get_rows, &ebp);
        g_free(ebp.px);
    }

    // Hide items
    sp_item_invoke_hide(SP_ITEM(sp_document_root(doc)), dkey);

    /* Free Arena and ArenaItem */
    nr_arena_item_unref(ebp.root);
    nr_object_unref((NRObject *) arena);
    return write_status;
}


/*######################
## P R I N T
######################*/


/**
 *  Print the current document, if any.
 */
void
sp_file_print()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document(doc, FALSE);
}


/**
 *  Print the current document, if any.  Do not use
 *  the machine's print drivers.
 */
void
sp_file_print_direct()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document(doc, TRUE);
}


/**
 * Display what the drawing would look like, if
 * printed.
 */
void
sp_file_print_preview(gpointer object, gpointer data)
{

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_preview_document(doc);

}

void Inkscape::IO::fixupHrefs( SPDocument *doc, const gchar *base, gboolean spns )
{
    //g_message("Inkscape::IO::fixupHrefs( , [%s], )", base );

    if ( 0 ) {
        gchar const* things[] = {
            "data:foo,bar",
            "http://www.google.com/image.png",
            "ftp://ssd.com/doo",
            "/foo/dee/bar.svg",
            "foo.svg",
            "file:/foo/dee/bar.svg",
            "file:///foo/dee/bar.svg",
            "file:foo.svg",
            "/foo/bar\xe1\x84\x92.svg",
            "file:///foo/bar\xe1\x84\x92.svg",
            "file:///foo/bar%e1%84%92.svg",
            "/foo/bar%e1%84%92.svg",
            "bar\xe1\x84\x92.svg",
            "bar%e1%84%92.svg",
            NULL
        };
        g_message("+------");
        for ( int i = 0; things[i]; i++ )
        {
            try
            {
                URI uri(things[i]);
                gboolean isAbs = g_path_is_absolute( things[i] );
                gchar *str = uri.toString();
                g_message( "abs:%d  isRel:%d  scheme:[%s]  path:[%s][%s]   uri[%s] / [%s]", (int)isAbs,
                           (int)uri.isRelative(),
                           uri.getScheme(),
                           uri.getPath(),
                           uri.getOpaque(),
                           things[i],
                           str );
                g_free(str);
            }
            catch ( MalformedURIException err )
            {
                dump_str( things[i], "MalformedURIException" );
                xmlChar *redo = xmlURIEscape((xmlChar const *)things[i]);
                g_message("    gone from [%s] to [%s]", things[i], redo );
                if ( redo == NULL )
                {
                    URI again = URI::fromUtf8( things[i] );
                    gboolean isAbs = g_path_is_absolute( things[i] );
                    gchar *str = again.toString();
                    g_message( "abs:%d  isRel:%d  scheme:[%s]  path:[%s][%s]   uri[%s] / [%s]", (int)isAbs,
                               (int)again.isRelative(),
                               again.getScheme(),
                               again.getPath(),
                               again.getOpaque(),
                               things[i],
                               str );
                    g_free(str);
                    g_message("    ----");
                }
            }
        }
        g_message("+------");
    }

    GSList const *images = sp_document_get_resource_list(doc, "image");
    for (GSList const *l = images; l != NULL; l = l->next) {
        Inkscape::XML::Node *ir = SP_OBJECT_REPR(l->data);

        const gchar *href = ir->attribute("xlink:href");

        // First try to figure out an absolute path to the asset
        //g_message("image href [%s]", href );
        if (spns && !g_path_is_absolute(href)) {
            const gchar *absref = ir->attribute("sodipodi:absref");
            const gchar *base_href = g_build_filename(base, href, NULL);
            //g_message("      absr [%s]", absref );

            if ( absref && Inkscape::IO::file_test(absref, G_FILE_TEST_EXISTS) && !Inkscape::IO::file_test(base_href, G_FILE_TEST_EXISTS))
            {
                // only switch over if the absref is valid while href is not
                href = absref;
                //g_message("     copied absref to href");
            }
        }

        // Once we have an absolute path, convert it relative to the new location
        if (href && g_path_is_absolute(href)) {
            const gchar *relname = sp_relative_path_from_path(href, base);
            //g_message("     setting to [%s]", relname );
            ir->setAttribute("xlink:href", relname);
        }
// TODO next refinement is to make the first choice keeping the relative path as-is if
//      based on the new location it gives us a valid file.
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

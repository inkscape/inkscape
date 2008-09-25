/** \file
 * Code for handling extensions (i.e.\ scripts).
 */
/*
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define __INKSCAPE_EXTENSION_IMPLEMENTATION_SCRIPT_C__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>

#include <errno.h>
#include <gtkmm.h>

#include "ui/view/view.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "selection.h"
#include "sp-namedview.h"
#include "io/sys.h"
#include "preferences.h"
#include "../system.h"
#include "extension/effect.h"
#include "extension/output.h"
#include "extension/input.h"
#include "extension/db.h"
#include "script.h"
#include "dialogs/dialog-events.h"
#include "application/application.h"

#include "util/glib-list-iterators.h"



#ifdef WIN32
#include <windows.h>
#include <sys/stat.h>
#include "registrytool.h"
#endif



/** This is the command buffer that gets allocated from the stack */
#define BUFSIZE (255)



/* Namespaces */
namespace Inkscape {
namespace Extension {
namespace Implementation {

/** \brief  Make GTK+ events continue to come through a little bit
	
	This just keeps coming the events through so that we'll make the GUI
	update and look pretty.
*/
void
Script::pump_events (void) {
    while( Gtk::Main::events_pending() )
        Gtk::Main::iteration();
    return;
}


/** \brief  A table of what interpreters to call for a given language

    This table is used to keep track of all the programs to execute a
    given script.  It also tracks the preference to use to overwrite
    the given interpreter to a custom one per user.
*/
Script::interpreter_t const Script::interpreterTab[] = {
        {"perl",   "perl-interpreter",   "perl"   },
#ifdef WIN32
        {"python", "python-interpreter", "pythonw" },
#else
        {"python", "python-interpreter", "python" },
#endif
        {"ruby",   "ruby-interpreter",   "ruby"   },
        {"shell",  "shell-interpreter",  "sh"     },
        { NULL,    NULL,                  NULL    }
};



/** \brief Look up an interpreter name, and translate to something that
           is executable
    \param interpNameArg  The name of the interpreter that we're looking
	                      for, should be an entry in interpreterTab
*/
Glib::ustring
Script::resolveInterpreterExecutable(const Glib::ustring &interpNameArg)
{

    Glib::ustring interpName = interpNameArg;

    interpreter_t const *interp;
    bool foundInterp = false;
    for (interp =  interpreterTab ; interp->identity ; interp++ ){
        if (interpName == interp->identity) {
            foundInterp = true;
            break;
        }
    }

    // Do we have a supported interpreter type?
    if (!foundInterp)
        return "";
    interpName = interp->defaultval;

    // 1.  Check preferences
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring prefInterp = prefs->getString("extensions", interp->prefstring);

    if (!prefInterp.empty()) {
        interpName = prefInterp;
        return interpName;
    }

#ifdef WIN32

    // 2.  Windows.  Try looking relative to inkscape.exe
    RegistryTool rt;
    Glib::ustring fullPath;
    Glib::ustring path;
    Glib::ustring exeName;
    if (rt.getExeInfo(fullPath, path, exeName)) {
        Glib::ustring interpPath = path;
        interpPath.append("\\");
        interpPath.append(interpNameArg);
        interpPath.append("\\");
        interpPath.append(interpName);
        interpPath.append(".exe");
        struct stat finfo;
        if (stat(interpPath .c_str(), &finfo) ==0) {
            g_message("Found local interpreter, '%s',  Size: %d",
                      interpPath .c_str(),
                      (int)finfo.st_size);
            return interpPath;
        }
    }

    // 3. Try searching the path
    char szExePath[MAX_PATH];
    char szCurrentDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurrentDir), szCurrentDir);
    unsigned int ret = (unsigned int)FindExecutable(
                  interpName.c_str(), szCurrentDir, szExePath);
    if (ret > 32) {
        interpName = szExePath;
        return interpName;
    }

#endif // win32


    return interpName;
}

/** \brief     This function creates a script object and sets up the
               variables.
    \return    A script object

   This function just sets the command to NULL.  It should get built
   officially in the load function.  This allows for less allocation
   of memory in the unloaded state.
*/
Script::Script() :
    Implementation()
{
}

/**
 *   brief     Destructor
 */
Script::~Script()
{
}



/**
    \return    A string with the complete string with the relative directory expanded
    \brief     This function takes in a Repr that contains a reldir entry
               and returns that data with the relative directory expanded.
               Mostly it is here so that relative directories all get used
               the same way.
    \param     reprin   The Inkscape::XML::Node with the reldir in it.

    Basically this function looks at an attribute of the Repr, and makes
    a decision based on that.  Currently, it is only working with the
    'extensions' relative directory, but there will be more of them.
    One thing to notice is that this function always returns an allocated
    string.  This means that the caller of this function can always
    free what they are given (and should do it too!).
*/
Glib::ustring
Script::solve_reldir(Inkscape::XML::Node *reprin) {

    gchar const *s = reprin->attribute("reldir");

    if (!s) {
        Glib::ustring str = sp_repr_children(reprin)->content();
        return str;
    }

    Glib::ustring reldir = s;

    if (reldir == "extensions") {

        for (unsigned int i=0;
            i < Inkscape::Extension::Extension::search_path.size();
            i++) {

            gchar * fname = g_build_filename(
               Inkscape::Extension::Extension::search_path[i],
               sp_repr_children(reprin)->content(),
               NULL);
            Glib::ustring filename = fname;
            g_free(fname);

            if ( Inkscape::IO::file_test(filename.c_str(), G_FILE_TEST_EXISTS) )
                return filename;

        }
    } else {
        Glib::ustring str = sp_repr_children(reprin)->content();
        return str;
    }

    return "";
}



/**
    \return   Whether the command given exists, including in the path
    \brief    This function is used to find out if something exists for
              the check command.  It can look in the path if required.
    \param    command   The command or file that should be looked for

    The first thing that this function does is check to see if the
    incoming file name has a directory delimiter in it.  This would
    mean that it wants to control the directories, and should be
    used directly.

    If not, the path is used.  Each entry in the path is stepped through,
    attached to the string, and then tested.  If the file is found
    then a TRUE is returned.  If we get all the way through the path
    then a FALSE is returned, the command could not be found.
*/
bool
Script::check_existance(const Glib::ustring &command)
{

    // Check the simple case first
    if (command.size() == 0) {
        return false;
    }

    //Don't search when it contains a slash. */
    if (command.find(G_DIR_SEPARATOR) != command.npos) {
        if (Inkscape::IO::file_test(command.c_str(), G_FILE_TEST_EXISTS))
            return true;
        else
            return false;
    }


    Glib::ustring path;
    gchar *s = (gchar *) g_getenv("PATH");
    if (s)
        path = s;
    else
       /* There is no `PATH' in the environment.
           The default search path is the current directory */
        path = G_SEARCHPATH_SEPARATOR_S;

    std::string::size_type pos  = 0;
    std::string::size_type pos2 = 0;
    while ( pos < path.size() ) {

        Glib::ustring localPath;

        pos2 = path.find(G_SEARCHPATH_SEPARATOR, pos);
        if (pos2 == path.npos) {
            localPath = path.substr(pos);
            pos = path.size();
        } else {
            localPath = path.substr(pos, pos2-pos);
            pos = pos2+1;
        }

        //printf("### %s\n", localPath.c_str());
        Glib::ustring candidatePath =
                      Glib::build_filename(localPath, command);

        if (Inkscape::IO::file_test(candidatePath .c_str(),
                      G_FILE_TEST_EXISTS)) {
            return true;
        }

    }

    return false;
}





/**
    \return   none
    \brief    This function 'loads' an extention, basically it determines
              the full command for the extention and stores that.
    \param    module  The extention to be loaded.

    The most difficult part about this function is finding the actual
    command through all of the Reprs.  Basically it is hidden down a
    couple of layers, and so the code has to move down too.  When
    the command is actually found, it has its relative directory
    solved.

    At that point all of the loops are exited, and there is an
    if statement to make sure they didn't exit because of not finding
    the command.  If that's the case, the extention doesn't get loaded
    and should error out at a higher level.
*/

bool
Script::load(Inkscape::Extension::Extension *module)
{
    if (module->loaded())
        return true;

    helper_extension = "";

    /* This should probably check to find the executable... */
    Inkscape::XML::Node *child_repr = sp_repr_children(module->get_repr());
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "script")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "command")) {
                    const gchar *interpretstr = child_repr->attribute("interpreter");
                    if (interpretstr != NULL) {
                        Glib::ustring interpString =
                            resolveInterpreterExecutable(interpretstr);
                        //g_message("Found: %s and %s",interpString.c_str(),interpretstr);
                        command.insert(command.end(), interpretstr);
                    }
                    Glib::ustring tmp = "\"";
                    tmp += solve_reldir(child_repr);
                    tmp += "\"";

                    command.insert(command.end(), tmp);
                }
                if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "helper_extension")) {
                    helper_extension = sp_repr_children(child_repr)->content();
                }
                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    //g_return_val_if_fail(command.length() > 0, false);

    return true;
}


/**
    \return   None.
    \brief    Unload this puppy!
    \param    module  Extension to be unloaded.

    This function just sets the module to unloaded.  It free's the
    command if it has been allocated.
*/
void
Script::unload(Inkscape::Extension::Extension */*module*/)
{
    command.clear();
    helper_extension = "";
}




/**
    \return   Whether the check passed or not
    \brief    Check every dependency that was given to make sure we should keep this extension
    \param    module  The Extension in question

*/
bool
Script::check(Inkscape::Extension::Extension *module)
{
	int script_count = 0;
    Inkscape::XML::Node *child_repr = sp_repr_children(module->get_repr());
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "script")) {
			script_count++;
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "check")) {
                    Glib::ustring command_text = solve_reldir(child_repr);
                    if (command_text.size() > 0) {
                        /* I've got the command */
                        bool existance = check_existance(command_text);
                        if (!existance)
                            return false;
                    }
                }

                if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "helper_extension")) {
                    gchar const *helper = sp_repr_children(child_repr)->content();
                    if (Inkscape::Extension::db.get(helper) == NULL) {
                        return false;
                    }
                }

                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

	if (script_count == 0) {
		return false;
	}

    return true;
}

class ScriptDocCache : public ImplementationDocumentCache {
    friend class Script;
protected:
    std::string _filename;
    int _tempfd;
public:
    ScriptDocCache (Inkscape::UI::View::View * view);
    ~ScriptDocCache ( );
};

ScriptDocCache::ScriptDocCache (Inkscape::UI::View::View * view) :
    ImplementationDocumentCache(view),
    _filename(""),
    _tempfd(0)
{
    try {
        _tempfd = Inkscape::IO::file_open_tmp(_filename, "ink_ext_XXXXXX.svg");
    } catch (...) {
        /// \todo Popup dialog here
        return;
    }

    SPDesktop *desktop = (SPDesktop *) view;
    sp_namedview_document_from_window(desktop);

    Inkscape::Extension::save(
              Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE),
              view->doc(), _filename.c_str(), false, false, false);

    return;
}

ScriptDocCache::~ScriptDocCache ( )
{
    close(_tempfd);
    unlink(_filename.c_str());
}

ImplementationDocumentCache *
Script::newDocCache( Inkscape::Extension::Extension * /*ext*/, Inkscape::UI::View::View * view ) {
    return new ScriptDocCache(view);
}


/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module who's preferences need getting
    \param    filename  Hey, the file you're getting might be important

    This function should really do something, right now it doesn't.
*/
Gtk::Widget *
Script::prefs_input(Inkscape::Extension::Input *module,
                    const gchar */*filename*/)
{
    return module->autogui(NULL, NULL);
}



/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module whose preferences need getting

    This function should really do something, right now it doesn't.
*/
Gtk::Widget *
Script::prefs_output(Inkscape::Extension::Output *module)
{
    return module->autogui(NULL, NULL);
}

/**
    \return  A new document that has been opened
    \brief   This function uses a filename that is put in, and calls
             the extension's command to create an SVG file which is
             returned.
    \param   module   Extension to use.
    \param   filename File to open.

    First things first, this function needs a temporary file name.  To
    create on of those the function g_file_open_tmp is used with
    the header of ink_ext_.

    The extension is then executed using the 'execute' function
    with the filname coming in, and the temporary filename.  After
    That executing, the SVG should be in the temporary file.

    Finally, the temporary file is opened using the SVG input module and
    a document is returned.  That document has its filename set to
    the incoming filename (so that it's not the temporary filename).
    That document is then returned from this function.
*/
SPDocument *
Script::open(Inkscape::Extension::Input *module,
             const gchar *filenameArg)
{
    std::list<std::string> params;
    module->paramListString(params);

    std::string tempfilename_out;
    int tempfd_out = 0;
    try {
        tempfd_out = Inkscape::IO::file_open_tmp(tempfilename_out, "ink_ext_XXXXXX.svg");
    } catch (...) {
        /// \todo Popup dialog here
        return NULL;
    }

    std::string lfilename = Glib::filename_from_utf8(filenameArg);

    file_listener fileout;
    int data_read = execute(command, params, lfilename, fileout);
    fileout.toFile(tempfilename_out);

    SPDocument * mydoc = NULL;
    if (data_read > 10) {
        if (helper_extension.size()==0) {
            mydoc = Inkscape::Extension::open(
                  Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG),
                  tempfilename_out.c_str());
        } else {
            mydoc = Inkscape::Extension::open(
                  Inkscape::Extension::db.get(helper_extension.c_str()),
                  tempfilename_out.c_str());
        }
    } // data_read

    if (mydoc != NULL) {
        sp_document_set_uri(mydoc, filenameArg);
    }

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd_out);

    unlink(tempfilename_out.c_str());

    return mydoc;
} // open



/**
    \return   none
    \brief    This function uses an extention to save a document.  It first
              creates an SVG file of the document, and then runs it through
              the script.
    \param    module    Extention to be used
    \param    doc       Document to be saved
    \param    filename  The name to save the final file as

    Well, at some point people need to save - it is really what makes
    the entire application useful.  And, it is possible that someone
    would want to use an extetion for this, so we need a function to
    do that eh?

    First things first, the document is saved to a temporary file that
    is an SVG file.  To get the temporary filename g_file_open_tmp is used with
    ink_ext_ as a prefix.  Don't worry, this file gets deleted at the
    end of the function.

    After we have the SVG file, then extention_execute is called with
    the temporary file name and the final output filename.  This should
    put the output of the script into the final output file.  We then
    delete the temporary file.
*/
void
Script::save(Inkscape::Extension::Output *module,
             SPDocument *doc,
             const gchar *filenameArg)
{
    std::list<std::string> params;
    module->paramListString(params);

    std::string tempfilename_in;
    int tempfd_in = 0;
    try {
        tempfd_in = Inkscape::IO::file_open_tmp(tempfilename_in, "ink_ext_XXXXXX.svg");
    } catch (...) {
        /// \todo Popup dialog here
        return;
    }

    if (helper_extension.size() == 0) {
        Inkscape::Extension::save(
                   Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE),
                   doc, tempfilename_in.c_str(), false, false, false);
    } else {
        Inkscape::Extension::save(
                   Inkscape::Extension::db.get(helper_extension.c_str()),
                   doc, tempfilename_in.c_str(), false, false, false);
    }


    file_listener fileout;
    execute(command, params, tempfilename_in, fileout);

    std::string lfilename = Glib::filename_from_utf8(filenameArg);
    fileout.toFile(lfilename);

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd_in);
    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_in.c_str());

    return;
}



/**
    \return    none
    \brief     This function uses an extention as a effect on a document.
    \param     module   Extention to effect with.
    \param     doc      Document to run through the effect.

    This function is a little bit trickier than the previous two.  It
    needs two temporary files to get it's work done.  Both of these
    files have random names created for them using the g_file_open_temp function
    with the ink_ext_ prefix in the temporary directory.  Like the other
    functions, the temporary files are deleted at the end.

    To save/load the two temporary documents (both are SVG) the internal
    modules for SVG load and save are used.  They are both used through
    the module system function by passing their keys into the functions.

    The command itself is built a little bit differently than in other
    functions because the effect support selections.  So on the command
    line a list of all the ids that are selected is included.  Currently,
    this only works for a single selected object, but there will be more.
    The command string is filled with the data, and then after the execution
    it is freed.

    The execute function is used at the core of this function
    to execute the Script on the two SVG documents (actually only one
    exists at the time, the other is created by that script).  At that
    point both should be full, and the second one is loaded.
*/
void
Script::effect(Inkscape::Extension::Effect *module,
               Inkscape::UI::View::View *doc,
               ImplementationDocumentCache * docCache)
{
    if (docCache == NULL) {
        docCache = newDocCache(module, doc);
    }
    ScriptDocCache * dc = dynamic_cast<ScriptDocCache *>(docCache);
    if (dc == NULL) {
        printf("TOO BAD TO LIVE!!!");
        exit(1);
    }

    SPDesktop *desktop = (SPDesktop *)doc;
    sp_namedview_document_from_window(desktop);

    gchar * orig_output_extension = g_strdup(sp_document_repr_root(desktop->doc())->attribute("inkscape:output_extension"));

    std::list<std::string> params;
    module->paramListString(params);

    if (module->no_doc) {
        // this is a no-doc extension, e.g. a Help menu command;
        // just run the command without any files, ignoring errors

        Glib::ustring empty;
        file_listener outfile;
        execute(command, params, empty, outfile);

        return;
    }

    std::string tempfilename_out;
    int tempfd_out = 0;
    try {
        tempfd_out = Inkscape::IO::file_open_tmp(tempfilename_out, "ink_ext_XXXXXX.svg");
    } catch (...) {
        /// \todo Popup dialog here
        return;
    }

    if (desktop != NULL) {
        Inkscape::Util::GSListConstIterator<SPItem *> selected =
             sp_desktop_selection(desktop)->itemList();
        while ( selected != NULL ) {
            Glib::ustring selected_id;
            selected_id += "--id=";
            selected_id += SP_OBJECT_ID(*selected);
            params.insert(params.begin(), selected_id);
            ++selected;
        }
    }

    file_listener fileout;
    int data_read = execute(command, params, dc->_filename, fileout);
    fileout.toFile(tempfilename_out);

    pump_events();

    SPDocument * mydoc = NULL;
    if (data_read > 10) {
        mydoc = Inkscape::Extension::open(
              Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG),
              tempfilename_out.c_str());
    } // data_read

    pump_events();

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd_out);

    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_out.c_str());

    /* Do something with mydoc.... */
    if (mydoc) {
        doc->doc()->emitReconstructionStart();
        copy_doc(doc->doc()->rroot, mydoc->rroot);
        doc->doc()->emitReconstructionFinish();
        mydoc->release();
        sp_namedview_update_layers_from_document(desktop);

        sp_document_repr_root(desktop->doc())->setAttribute("inkscape:output_extension", orig_output_extension);
    }
    g_free(orig_output_extension);

    return;
}



/**
    \brief  A function to take all the svg elements from one document
            and put them in another.
    \param  oldroot  The root node of the document to be replaced
    \param  newroot  The root node of the document to replace it with

    This function first deletes all of the data in the old document.  It
    does this by creating a list of what needs to be deleted, and then
    goes through the list.  This two pass approach removes issues with
    the list being change while parsing through it.  Lots of nasty bugs.

    Then, it goes through the new document, duplicating all of the
    elements and putting them into the old document.  The copy
    is then complete.
*/
void
Script::copy_doc (Inkscape::XML::Node * oldroot, Inkscape::XML::Node * newroot)
{
    std::vector<Inkscape::XML::Node *> delete_list;
    Inkscape::XML::Node * oldroot_namedview = NULL;

    for (Inkscape::XML::Node * child = oldroot->firstChild();
            child != NULL;
            child = child->next()) {
        if (!strcmp("sodipodi:namedview", child->name())) {
            oldroot_namedview = child;
            for (Inkscape::XML::Node * oldroot_namedview_child = child->firstChild();
                    oldroot_namedview_child != NULL;
                    oldroot_namedview_child = oldroot_namedview_child->next()) {
                delete_list.push_back(oldroot_namedview_child);
            }
        } else {
            delete_list.push_back(child);
        }
    }
    for (unsigned int i = 0; i < delete_list.size(); i++)
        sp_repr_unparent(delete_list[i]);

    for (Inkscape::XML::Node * child = newroot->firstChild();
            child != NULL;
            child = child->next()) {
        if (!strcmp("sodipodi:namedview", child->name())) {
            if (oldroot_namedview != NULL) {
                for (Inkscape::XML::Node * newroot_namedview_child = child->firstChild();
                        newroot_namedview_child != NULL;
                        newroot_namedview_child = newroot_namedview_child->next()) {
                    oldroot_namedview->appendChild(newroot_namedview_child->duplicate(oldroot->document()));
                }
            }
        } else {
            oldroot->appendChild(child->duplicate(oldroot->document()));
        }
    }

    oldroot->setAttribute("width", newroot->attribute("width"));
    oldroot->setAttribute("height", newroot->attribute("height"));

    /** \todo  Restore correct layer */
    /** \todo  Restore correct selection */
}

/**  \brief  This function checks the stderr file, and if it has data,
             shows it in a warning dialog to the user
     \param  filename  Filename of the stderr file
*/
void
Script::checkStderr (const Glib::ustring &data,
                           Gtk::MessageType type,
                     const Glib::ustring &message)
{
    Gtk::MessageDialog warning(message, false, type, Gtk::BUTTONS_OK, true);
    warning.set_resizable(true);
    GtkWidget *dlg = GTK_WIDGET(warning.gobj());
    sp_transientize(dlg);

    Gtk::VBox * vbox = warning.get_vbox();

    /* Gtk::TextView * textview = new Gtk::TextView(Gtk::TextBuffer::create()); */
    Gtk::TextView * textview = new Gtk::TextView();
    textview->set_editable(false);
    textview->set_wrap_mode(Gtk::WRAP_WORD);
    textview->show();

    textview->get_buffer()->set_text(data.c_str());

    Gtk::ScrolledWindow * scrollwindow = new Gtk::ScrolledWindow();
    scrollwindow->add(*textview);
    scrollwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrollwindow->set_shadow_type(Gtk::SHADOW_IN);
    scrollwindow->show();

    vbox->pack_start(*scrollwindow, true, true, 5 /* fix these */);

    warning.run();

    return;
}

bool
Script::cancelProcessing (void) {
    _canceled = true;
    _main_loop->quit();
    Glib::spawn_close_pid(_pid);

    return true;
}


/** \brief    This is the core of the extension file as it actually does
              the execution of the extension.
    \param    in_command  The command to be executed
    \param    filein      Filename coming in
    \param    fileout     Filename of the out file
    \return   Number of bytes that were read into the output file.

    The first thing that this function does is build the command to be
    executed.  This consists of the first string (in_command) and then
    the filename for input (filein).  This file is put on the command
    line.

    The next thing is that this function does is open a pipe to the
    command and get the file handle in the ppipe variable.  It then
    opens the output file with the output file handle.  Both of these
    operations are checked extensively for errors.

    After both are opened, then the data is copied from the output
    of the pipe into the file out using fread and fwrite.  These two
    functions are used because of their primitive nature they make
    no assumptions about the data.  A buffer is used in the transfer,
    but the output of fread is stored so the exact number of bytes
    is handled gracefully.

    At the very end (after the data has been copied) both of the files
    are closed, and we return to what we were doing.
*/
int
Script::execute (const std::list<std::string> &in_command,
                 const std::list<std::string> &in_params,
                 const Glib::ustring &filein,
                 file_listener &fileout)
{
    g_return_val_if_fail(in_command.size() > 0, 0);
    // printf("Executing\n");

    std::vector <std::string> argv;

/*
    for (std::list<std::string>::const_iterator i = in_command.begin();
            i != in_command.end(); i++) {
        argv.push_back(*i);
    }
*/
    // according to http://www.gtk.org/api/2.6/glib/glib-Spawning-Processes.html spawn quotes parameter containing spaces
    // we tokenize so that spwan does not need to quote over all params
    for (std::list<std::string>::const_iterator i = in_command.begin();
            i != in_command.end(); i++) {
        std::string param_str = *i;
        do {
            //g_message("param: %s", param_str.c_str());
            size_t first_space = param_str.find_first_of(' ');
            size_t first_quote = param_str.find_first_of('"');
            //std::cout << "first space " << first_space << std::endl;
            //std::cout << "first quote " << first_quote << std::endl;

            if((first_quote != std::string::npos) && (first_quote == 0)) {
                size_t next_quote = param_str.find_first_of('"', first_quote + 1);
                //std::cout << "next quote " << next_quote << std::endl;

                if(next_quote != std::string::npos) {
                    //std::cout << "now split " << next_quote << std::endl;
                    //std::cout << "now split " << param_str.substr(1, next_quote - 1) << std::endl;
                    //std::cout << "now split " << param_str.substr(next_quote + 1) << std::endl;
                    std::string part_str = param_str.substr(1, next_quote - 1);
                    if(part_str.size() > 0)
                        argv.push_back(part_str);
                    param_str = param_str.substr(next_quote + 1);

                }
                else {
                    if(param_str.size() > 0)
                        argv.push_back(param_str);
                    param_str = "";
                }

            }
            else if(first_space != std::string::npos) {
                //std::cout << "now split " << first_space << std::endl;
                //std::cout << "now split " << param_str.substr(0, first_space) << std::endl;
                //std::cout << "now split " << param_str.substr(first_space + 1) << std::endl;
                std::string part_str = param_str.substr(0, first_space);
                if(part_str.size() > 0)
                    argv.push_back(part_str);
                param_str = param_str.substr(first_space + 1);
            }
            else {
                if(param_str.size() > 0)
                    argv.push_back(param_str);
                param_str = "";
            }
        } while(param_str.size() > 0);
    }

    for (std::list<std::string>::const_iterator i = in_params.begin();
            i != in_params.end(); i++) {
    	//g_message("Script parameter: %s",(*i)g.c_str());
        argv.push_back(*i);        
    }

    if (!(filein.empty())) {
		argv.push_back(filein);
    }

    int stdout_pipe, stderr_pipe;

    try {
        Inkscape::IO::spawn_async_with_pipes(Glib::get_current_dir(), // working directory
                                     argv,  // arg v
                                     Glib::SPAWN_SEARCH_PATH /*| Glib::SPAWN_DO_NOT_REAP_CHILD*/,
                                     sigc::slot<void>(),
                                     &_pid,          // Pid
                                     NULL,           // STDIN
                                     &stdout_pipe,   // STDOUT
                                     &stderr_pipe);  // STDERR
    } catch (Glib::SpawnError e) {
        printf("Can't Spawn!!! spawn returns: %d\n", e.code());
        return 0;
    }

    _main_loop = Glib::MainLoop::create(false);

    file_listener fileerr;
    fileout.init(stdout_pipe, _main_loop);
    fileerr.init(stderr_pipe, _main_loop);

    _canceled = false;
    _main_loop->run();

    // Ensure all the data is out of the pipe
    while (!fileout.isDead())
        fileout.read(Glib::IO_IN);
    while (!fileerr.isDead())
        fileerr.read(Glib::IO_IN);

    if (_canceled) {
        // std::cout << "Script Canceled" << std::endl;
        return 0;
    }

    Glib::ustring stderr_data = fileerr.string();
    if (stderr_data.length() != 0 &&
        Inkscape::NSApplication::Application::getUseGui()
       ) {
        checkStderr(stderr_data, Gtk::MESSAGE_INFO,
                                 _("Inkscape has received additional data from the script executed.  "
                                   "The script did not return an error, but this may indicate the results will not be as expected."));
    }

    Glib::ustring stdout_data = fileout.string();
    if (stdout_data.length() == 0) {
        return 0;
    }

    // std::cout << "Finishing Execution." << std::endl;
    return stdout_data.length();
}




}  // namespace Implementation
}  // namespace Extension
}  // namespace Inkscape

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

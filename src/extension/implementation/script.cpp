/** \file
 * Code for handling extensions (i.e.\ scripts).
 */
/*
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
TODO:
FIXME:
  After Inkscape makes a formal requirement for a GTK version above 2.11.4, please
  replace all the instances of ink_ext_XXXXXX in this file that represent
  svg files with ink_ext_XXXXXX.svg . Doing so will prevent errors in extensions
  that call inkscape to manipulate the file.
  
  "** (inkscape:5848): WARNING **: Format autodetect failed. The file is being opened as SVG."
  
  references:
  http://www.gtk.org/api/2.6/glib/glib-File-Utilities.html#g-mkstemp
  http://ftp.gnome.org/pub/gnome/sources/glib/2.11/glib-2.11.4.changes
  http://developer.gnome.org/doc/API/2.0/glib/glib-File-Utilities.html#g-mkstemp
  
  --Aaron Spike
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
#include "selection.h"
#include "sp-namedview.h"
#include "io/sys.h"
#include "prefs-utils.h"
#include "../system.h"
#include "extension/effect.h"
#include "extension/output.h"
#include "extension/db.h"
#include "script.h"
#include "dialogs/dialog-events.h"

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



//Interpreter lookup table
struct interpreter_t {
        gchar * identity;
        gchar * prefstring;
        gchar * defaultval;
};


static interpreter_t interpreterTab[] = {
        {"perl",   "perl-interpreter",   "perl"   },
        {"python", "python-interpreter", "python" },
        {"ruby",   "ruby-interpreter",   "ruby"   },
        {"shell",  "shell-interpreter",  "sh"     },
        { NULL,    NULL,                  NULL    }
};



/**
 * Look up an interpreter name, and translate to something that
 * is executable
 */
static Glib::ustring
resolveInterpreterExecutable(const Glib::ustring &interpNameArg)
{

    Glib::ustring interpName = interpNameArg;

    interpreter_t *interp;
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
    gchar *prefInterp = (gchar *)prefs_get_string_attribute(
                                "extensions", interp->prefstring);

    if (prefInterp) {
        interpName = prefInterp;
        return interpName;
    }

#ifdef _WIN32

    // 2.  Windows.  Try looking relative to inkscape.exe
    RegistryTool rt;
    Glib::ustring fullPath;
    Glib::ustring path;
    Glib::ustring exeName;
    if (rt.getExeInfo(fullPath, path, exeName)) {
        Glib::ustring interpPath = path;
        interpPath.append("\\");
        interpPath.append(interpName);
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






/**
    \return    A script object
    \brief     This function creates a script object and sets up the
               variables.

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
                      G_FILE_TEST_EXISTS))
            return true;

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
        return TRUE;

    helper_extension = "";

    /* This should probably check to find the executable... */
    Inkscape::XML::Node *child_repr = sp_repr_children(module->get_repr());
    Glib::ustring command_text;
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), "script")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), "command")) {
                    command_text = solve_reldir(child_repr);

                    const gchar *interpretstr = child_repr->attribute("interpreter");
                    if (interpretstr != NULL) {
                        Glib::ustring interpString =
                            resolveInterpreterExecutable(interpretstr);
                        interpString .append(" \"");
                        interpString .append(command_text);
                        interpString .append("\"");                        
                        command_text = interpString;
                    }
                }
                if (!strcmp(child_repr->name(), "helper_extension"))
                    helper_extension = sp_repr_children(child_repr)->content();
                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    g_return_val_if_fail(command_text.size() > 0, FALSE);

    command = command_text;
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
Script::unload(Inkscape::Extension::Extension *module)
{
    command          = "";
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
    Inkscape::XML::Node *child_repr = sp_repr_children(module->get_repr());
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), "script")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), "check")) {
                    Glib::ustring command_text = solve_reldir(child_repr);
                    if (command_text.size() > 0) {
                        /* I've got the command */
                        bool existance = check_existance(command_text);
                        if (!existance)
                            return FALSE;
                    }
                }

                if (!strcmp(child_repr->name(), "helper_extension")) {
                    gchar const *helper = sp_repr_children(child_repr)->content();
                    if (Inkscape::Extension::db.get(helper) == NULL) {
                        return FALSE;
                    }
                }

                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    return true;
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
                    const gchar *filename)
{
    /*return module->autogui(); */
    return NULL;
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
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module who's preferences need getting

    This function should really do something, right now it doesn't.
*/
Gtk::Widget *
Script::prefs_effect(Inkscape::Extension::Effect *module,
                     Inkscape::UI::View::View *view)
{

    SPDocument * current_document = view->doc();

    using Inkscape::Util::GSListConstIterator;
    GSListConstIterator<SPItem *> selected =
           sp_desktop_selection((SPDesktop *)view)->itemList();
    Inkscape::XML::Node * first_select = NULL;
    if (selected != NULL) 
           first_select = SP_OBJECT_REPR(*selected);

    return module->autogui(current_document, first_select);
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

    Glib::ustring filename = filenameArg;

    gchar *tmpname;

    // FIXME: process the GError instead of passing NULL
    gint tempfd = g_file_open_tmp("ink_ext_XXXXXX", &tmpname, NULL);
    if (tempfd == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return NULL;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    Glib::ustring tempfilename_out = tmpname;
    g_free(tmpname);

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    Glib::ustring local_filename =
            g_filename_from_utf8( filename.c_str(), -1,
                                  &bytesRead,  &bytesWritten, &error);

    int data_read = execute(command, local_filename, tempfilename_out);

    SPDocument *mydoc = NULL;
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
    }

    if (mydoc != NULL)
        sp_document_set_uri(mydoc, (const gchar *)filename.c_str());

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd);
    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_out.c_str());


    return mydoc;
}



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

    Glib::ustring filename = filenameArg;

    gchar *tmpname;
    // FIXME: process the GError instead of passing NULL
    gint tempfd = g_file_open_tmp("ink_ext_XXXXXX", &tmpname, NULL);
    if (tempfd == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    Glib::ustring tempfilename_in = tmpname;
    g_free(tmpname);

    if (helper_extension.size() == 0) {
        Inkscape::Extension::save(
                   Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE),
                   doc, tempfilename_in.c_str(), FALSE, FALSE, FALSE);
    } else {
        Inkscape::Extension::save(
                   Inkscape::Extension::db.get(helper_extension.c_str()),
                   doc, tempfilename_in.c_str(), FALSE, FALSE, FALSE);
    }

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    Glib::ustring local_filename =
            g_filename_from_utf8( filename.c_str(), -1,
                                 &bytesRead,  &bytesWritten, &error);

    Glib::ustring local_command = command;
    Glib::ustring paramString   = *module->paramString();
    local_command.append(paramString);

    execute(local_command, tempfilename_in, local_filename);


    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd);
    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_in.c_str());
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
Script::effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *doc)
{
    if (module->no_doc) { 
        // this is a no-doc extension, e.g. a Help menu command; 
        // just run the command without any files, ignoring errors
        Glib::ustring local_command(command);
        Glib::ustring paramString = *module->paramString();
        local_command.append(paramString);

        Glib::ustring empty;
        execute(local_command, empty, empty);

        return;
    }

    gchar *tmpname;
    // FIXME: process the GError instead of passing NULL
    gint tempfd_in = g_file_open_tmp("ink_ext_XXXXXX", &tmpname, NULL);
    if (tempfd_in == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    Glib::ustring tempfilename_in = tmpname;
    g_free(tmpname);


    // FIXME: process the GError instead of passing NULL
    gint tempfd_out = g_file_open_tmp("ink_ext_XXXXXX", &tmpname, NULL);
    if (tempfd_out == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    Glib::ustring tempfilename_out= tmpname;
    g_free(tmpname);

    SPDesktop *desktop = (SPDesktop *) doc;
    sp_namedview_document_from_window(desktop);

    Inkscape::Extension::save(
              Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE),
              doc->doc(), tempfilename_in.c_str(), FALSE, FALSE, FALSE);

    Glib::ustring local_command(command);

    /* fixme: Should be some sort of checking here.  Don't know how to do this with structs instead
     * of classes. */
    if (desktop != NULL) {
        Inkscape::Util::GSListConstIterator<SPItem *> selected =
             sp_desktop_selection(desktop)->itemList();
        while ( selected != NULL ) {
            local_command += " --id=";
            local_command += SP_OBJECT_ID(*selected);
            ++selected;
        }
    }

    Glib::ustring paramString = *module->paramString();
    local_command.append(paramString);


    // std::cout << local_command << std::endl;

    int data_read = execute(local_command, tempfilename_in, tempfilename_out);

    SPDocument * mydoc = NULL;
    if (data_read > 10)
        mydoc = Inkscape::Extension::open(
              Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG),
              tempfilename_out.c_str());

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd_in);
    close(tempfd_out);

    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_in.c_str());
    unlink(tempfilename_out.c_str());


    /* Do something with mydoc.... */
    if (mydoc) {
        doc->doc()->emitReconstructionStart();
        copy_doc(doc->doc()->rroot, mydoc->rroot);
        doc->doc()->emitReconstructionFinish();
        mydoc->release();
        sp_namedview_update_layers_from_document(desktop);
    }
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
    for (Inkscape::XML::Node * child = oldroot->firstChild();
            child != NULL;
            child = child->next()) {
        if (!strcmp("sodipodi:namedview", child->name()))
            continue;
        delete_list.push_back(child);
    }
    for (unsigned int i = 0; i < delete_list.size(); i++)
        sp_repr_unparent(delete_list[i]);

    for (Inkscape::XML::Node * child = newroot->firstChild();
            child != NULL;
            child = child->next()) {
        if (!strcmp("sodipodi:namedview", child->name()))
            continue;
        oldroot->appendChild(child->duplicate());
    }

    /** \todo  Restore correct layer */
    /** \todo  Restore correct selection */
}



/* Helper class used by Script::execute */
class pipe_t {
public:
    /* These functions set errno if they return false.
       I'm not sure whether that's a good idea or not, but it should be reasonably
       straightforward to change it if needed. */
    bool open(const Glib::ustring &command,
              const Glib::ustring &errorFile,
              int mode);
    bool close();

    /* These return the number of bytes read/written. */
    size_t read(void *buffer, size_t size);
    size_t write(void const *buffer, size_t size);

    enum {
        mode_read  = 1 << 0,
        mode_write = 1 << 1,
    };

private:
#ifdef WIN32
    /* This is used to translate win32 errors into errno errors.
       It only recognizes a few win32 errors for the moment though. */
    static int translate_error(DWORD err);

    HANDLE hpipe;
#else
    FILE *ppipe;
#endif
};




/**
    \brief    This is the core of the extension file as it actually does
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
Script::execute (const Glib::ustring &in_command,
                 const Glib::ustring &filein,
                 const Glib::ustring &fileout)
{
    g_return_val_if_fail(in_command.size() > 0, 0);
    // printf("Executing: %s\n", in_command);

    gchar *tmpname;
    gint errorFileNum;
    errorFileNum = g_file_open_tmp("ink_ext_stderr_XXXXXX", &tmpname, NULL);
    if (errorFileNum != 0) {
        close(errorFileNum);
    } else {
        g_free(tmpname);
    }

    Glib::ustring errorFile = tmpname;
    g_free(tmpname);

    Glib::ustring localCommand = in_command;

    if (!(filein.empty())) {
        localCommand .append(" \"");
        localCommand .append(filein);
        localCommand .append("\"");
    }

    // std::cout << "Command to run: " << command << std::endl;

    pipe_t pipe;
    bool open_success = pipe.open((char *)localCommand.c_str(),
                                  errorFile.c_str(),
                                  pipe_t::mode_read);

    /* Run script */
    if (!open_success) {
        /* Error - could not open pipe - check errno */
        if (errno == EINVAL) {
            perror("Extension::Script:  Invalid mode argument in popen\n");
        } else if (errno == ECHILD) {
            perror("Extension::Script:  Cannot obtain child extension status in popen\n");
        } else {
            perror("Extension::Script:  Unknown error for popen\n");
        }
        return 0;
    }

    if (fileout.empty()) { // no output file to create; just close everything and return 0
        if (errorFile.size()>0) {
            unlink(errorFile.c_str());
        }
        pipe.close();
        return 0;
    }

    /* Copy pipe output to fileout (temporary file) */
    Inkscape::IO::dump_fopen_call(fileout.c_str(), "J");
    FILE *pfile = Inkscape::IO::fopen_utf8name(fileout.c_str(), "w");

    if (pfile == NULL) {
        /* Error - could not open file */
        if (errno == EINVAL) {
            perror("Extension::Script:  The mode provided to fopen was invalid\n");
        } else {
            perror("Extension::Script:  Unknown error attempting to open temporary file\n");
        }
        return 0;
    }

    int amount_read = 0;
    char buf[BUFSIZE];
    int num_read;
    while ((num_read = pipe.read(buf, BUFSIZE)) != 0) {
        amount_read += num_read;
        fwrite(buf, 1, num_read, pfile);
    }

    /* Close file */
    if (fclose(pfile) == EOF) {
        if (errno == EBADF) {
            perror("Extension::Script:  The filedescriptor for the temporary file is invalid\n");
            return 0;
        } else {
            perror("Extension::Script:  Unknown error closing temporary file\n");
        }
    }

    /* Close pipe */
    if (!pipe.close()) {
        if (errno == EINVAL) {
            perror("Extension::Script:  Invalid mode set for pclose\n");
        } else if (errno == ECHILD) {
            perror("Extension::Script:  Could not obtain child status for pclose\n");
        } else {
            if (!errorFile.empty()) {
                checkStderr(errorFile, Gtk::MESSAGE_ERROR,
                    _("Inkscape has received an error from the script that it called.  "
                      "The text returned with the error is included below.  "
                      "Inkscape will continue working, but the action you requested has been cancelled."));
            } else {
                perror("Extension::Script:  Unknown error for pclose\n");
            }
        }
        /* Could be a lie, but if there is an error, we don't want
         * to count on what was read being good */
        amount_read = 0;
    } else {
        if (errorFile.size()>0) {
            checkStderr(errorFile, Gtk::MESSAGE_INFO,
                _("Inkscape has received additional data from the script executed.  "
                  "The script did not return an error, but this may indicate the results will not be as expected."));
        }
    }

    if (errorFile.size()>0) {
        unlink(errorFile.c_str());
    }

    return amount_read;
}




/**  \brief  This function checks the stderr file, and if it has data,
             shows it in a warning dialog to the user
     \param  filename  Filename of the stderr file
*/
void
Script::checkStderr (const Glib::ustring &filename,
                     Gtk::MessageType type,
                     const Glib::ustring &message)
{

    // magic win32 crlf->lf conversion means the file length is not the same as
    // the text length, but luckily gtk will accept crlf in textviews so we can
    // just use binary mode
    std::ifstream stderrf (filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!stderrf.is_open()) return;

    stderrf.seekg(0, std::ios::end);
    int length = stderrf.tellg();
    if (0 == length) return;
    stderrf.seekg(0, std::ios::beg);

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

    char * buffer = new char [length];
    stderrf.read(buffer, length);
    textview->get_buffer()->set_text(buffer, buffer + length);
    delete buffer;
    stderrf.close();

    Gtk::ScrolledWindow * scrollwindow = new Gtk::ScrolledWindow();
    scrollwindow->add(*textview);
    scrollwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrollwindow->set_shadow_type(Gtk::SHADOW_IN);
    scrollwindow->show();

    vbox->pack_start(*scrollwindow, true, true, 5 /* fix these */);

    warning.run();

    return;
}




#ifdef WIN32


bool pipe_t::open(const Glib::ustring &command,
                  const Glib::ustring &errorFile,
                  int mode_p) {
    HANDLE pipe_write;

    //###############  Create pipe
    SECURITY_ATTRIBUTES secattrs;
    ZeroMemory(&secattrs, sizeof(secattrs));
    secattrs.nLength = sizeof(secattrs);
    secattrs.lpSecurityDescriptor = 0;
    secattrs.bInheritHandle = TRUE;
    HANDLE t_pipe_read = 0;
    if ( !CreatePipe(&t_pipe_read, &pipe_write, &secattrs, 0) ) {
        errno = translate_error(GetLastError());
        return false;
    }
    // This duplicate handle makes the read pipe uninheritable
    BOOL ret = DuplicateHandle(GetCurrentProcess(),
                               t_pipe_read,
                               GetCurrentProcess(),
                               &hpipe, 0, FALSE,
                               DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS);
    if (!ret) {
        int en = translate_error(GetLastError());
        CloseHandle(t_pipe_read);
        CloseHandle(pipe_write);
        errno = en;
        return false;
    }

    //############### Open stderr file
    HANDLE hStdErrFile = CreateFile(errorFile.c_str(),
                      GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, CREATE_ALWAYS, 0, NULL);
    HANDLE hInheritableStdErr;
    DuplicateHandle(GetCurrentProcess(),
                    hStdErrFile,
                    GetCurrentProcess(),
                    &hInheritableStdErr,
                    0, 
                    TRUE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS);

    //############### Create process
    PROCESS_INFORMATION procinfo;
    STARTUPINFO startupinfo;
    ZeroMemory(&procinfo, sizeof(procinfo));
    ZeroMemory(&startupinfo, sizeof(startupinfo));
    startupinfo.cb = sizeof(startupinfo);
    //startupinfo.lpReserved = 0;
    //startupinfo.lpDesktop = 0;
    //startupinfo.lpTitle = 0;
    startupinfo.dwFlags = STARTF_USESTDHANDLES;
    startupinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupinfo.hStdOutput = pipe_write;
    startupinfo.hStdError = hInheritableStdErr;

    if ( !CreateProcess(NULL, (CHAR *)command.c_str(),
                        NULL, NULL, TRUE,
                        0, NULL, NULL,
                        &startupinfo, &procinfo) ) {
        errno = translate_error(GetLastError());
        return false;
    }
    CloseHandle(procinfo.hThread);
    CloseHandle(procinfo.hProcess);

    // Close our copy of the write handle
    CloseHandle(hInheritableStdErr);
    CloseHandle(pipe_write);

    return true;
}



bool pipe_t::close() {
    BOOL retval = CloseHandle(hpipe);
    if ( !retval )
        errno = translate_error(GetLastError());
    return retval != FALSE;
}

size_t pipe_t::read(void *buffer, size_t size) {
    DWORD bytes_read = 0;
    ReadFile(hpipe, buffer, size, &bytes_read, 0);
    return bytes_read;
}

size_t pipe_t::write(void const *buffer, size_t size) {
    DWORD bytes_written = 0;
    WriteFile(hpipe, buffer, size, &bytes_written, 0);
    return bytes_written;
}

int pipe_t::translate_error(DWORD err) {
    switch (err) {
        case ERROR_FILE_NOT_FOUND:
            return ENOENT;
        case ERROR_INVALID_HANDLE:
        case ERROR_INVALID_PARAMETER:
            return EINVAL;
        default:
            return 0;
    }
}


#else // not Win32


bool pipe_t::open(const Glib::ustring &command,
                  const Glib::ustring &errorFile,
                  int mode_p) {

    Glib::ustring popen_mode;

    if ( (mode_p & mode_read) != 0 )
        popen_mode.append("r");

    if ( (mode_p & mode_write) != 0 )
        popen_mode.append("w");

    // Get the commandline to be run
    Glib::ustring pipeStr = command;
    if (errorFile.size()>0) {
        pipeStr .append(" 2> ");
        pipeStr .append(errorFile);
    }

    ppipe = popen(pipeStr.c_str(), popen_mode.c_str());

    return ppipe != NULL;
}


bool pipe_t::close() {
    return fclose(ppipe) == 0;
}


size_t pipe_t::read(void *buffer, size_t size) {
    return fread(buffer, 1, size, ppipe);
}


size_t pipe_t::write(void const *buffer, size_t size) {
    return fwrite(buffer, 1, size, ppipe);
}




#endif // (Non-)Win32




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

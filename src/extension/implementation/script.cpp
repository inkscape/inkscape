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

#define __INKSCAPE_EXTENSION_IMPLEMENTATION_SCRIPT_C__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>

#include <errno.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

#include "ui/view/view.h"
#include "desktop-handles.h"
#include "selection.h"
#include "sp-namedview.h"
#include "io/sys.h"
#include "prefs-utils.h"
#include "../system.h"
#include "extension/effect.h"
#include "extension/db.h"
#include "script.h"

#include "util/glib-list-iterators.h"

#ifdef WIN32
#include <windows.h>
#endif

/** This is the command buffer that gets allocated from the stack */
#define BUFSIZE (255)

/* Namespaces */
namespace Inkscape {
namespace Extension {
namespace Implementation {

/* Real functions */
/**
    \return    A script object
    \brief     This function creates a script object and sets up the
               variables.

   This function just sets the command to NULL.  It should get built
   officially in the load function.  This allows for less allocation
   of memory in the unloaded state.
*/
Script::Script() :
    Implementation(),
    command(NULL),
    helper_extension(NULL)
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
gchar *
Script::solve_reldir(Inkscape::XML::Node *reprin) {
    gchar const *reldir = reprin->attribute("reldir");

    if (reldir == NULL) {
        return g_strdup(sp_repr_children(reprin)->content());
    }

    if (!strcmp(reldir, "extensions")) {
        for(unsigned int i=0; i<Inkscape::Extension::Extension::search_path.size(); i++) {
            gchar * filename = g_build_filename(Inkscape::Extension::Extension::search_path[i], sp_repr_children(reprin)->content(), NULL);
            if ( Inkscape::IO::file_test(filename, G_FILE_TEST_EXISTS) ) {
                return filename;
            }
            g_free(filename);
        }
    } else {
        return g_strdup(sp_repr_children(reprin)->content());
    }

    return NULL;
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
Script::check_existance(gchar const *command)
{
    if (*command == '\0') {
        /* We check the simple case first. */
        return FALSE;
    }

    if (g_utf8_strchr(command, -1, G_DIR_SEPARATOR) != NULL) {
        /* Don't search when it contains a slash. */
        if (Inkscape::IO::file_test(command, G_FILE_TEST_EXISTS))
            return TRUE;
        else
            return FALSE;
    }


    gchar *path = g_strdup(g_getenv("PATH"));
    if (path == NULL) {
        /* There is no `PATH' in the environment.
           The default search path is the current directory */
        path = g_strdup(G_SEARCHPATH_SEPARATOR_S);
    }
    gchar *orig_path = path;

    for (; path != NULL;) {
        gchar *const local_path = path;
        path = g_utf8_strchr(path, -1, G_SEARCHPATH_SEPARATOR);
        if (path == NULL) {
            break;
        }
        /* Not sure whether this is UTF8 happy, but it would seem
           like it considering that I'm searching (and finding)
           the ':' character */
        if (path != local_path && path != NULL) {
            path[0] = '\0';
            path++;
        } else {
            path = NULL;
        }

        gchar *final_name;
        if (local_path == '\0') {
            final_name = g_strdup(command);
        } else {
            final_name = g_build_filename(local_path, command, NULL);
        }

        if (Inkscape::IO::file_test(final_name, G_FILE_TEST_EXISTS)) {
            g_free(final_name);
            g_free(orig_path);
            return TRUE;
        }

        g_free(final_name);
    }

    return FALSE;
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
    if (module->loaded()) {
        return TRUE;
    }

    helper_extension = NULL;

    /* This should probably check to find the executable... */
    Inkscape::XML::Node *child_repr = sp_repr_children(module->get_repr());
    gchar *command_text = NULL;
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), "script")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), "command")) {
                    command_text = solve_reldir(child_repr);

                    const gchar * interpretstr = child_repr->attribute("interpreter");
                    if (interpretstr != NULL) {
                        struct interpreter_t {
                            gchar * identity;
                            gchar * prefstring;
                            gchar * defaultval;
                        };
                        const interpreter_t interpreterlst[] = {
                            {"perl", "perl-interpreter", "perl"},
                            {"python", "python-interpreter", "python"},
                            {"ruby", "ruby-interpreter", "ruby"},
                            {"shell", "shell-interpreter", "sh"}
                        }; /* Change count below if you change structure */
                        for (unsigned int i = 0; i < 4; i++) {
                            if (!strcmp(interpretstr, interpreterlst[i].identity)) {
                                const gchar * insertText = interpreterlst[i].defaultval;
                                if (prefs_get_string_attribute("extensions", interpreterlst[i].prefstring) != NULL)
                                    insertText = prefs_get_string_attribute("extensions", interpreterlst[i].prefstring);
#ifdef _WIN32
                                else {
                                    char szExePath[MAX_PATH];
                                    char szCurrentDir[MAX_PATH];
                                    GetCurrentDirectory(sizeof(szCurrentDir), szCurrentDir);
                                    if (reinterpret_cast<unsigned>(FindExecutable(command_text, szCurrentDir, szExePath)) > 32)
                                        insertText = szExePath;
                                }
#endif

                                gchar * temp = command_text;
                                command_text = g_strconcat(insertText, " ", temp, NULL);
                                g_free(temp);

                                break;
                            }
                        }
                    }
                }
                if (!strcmp(child_repr->name(), "helper_extension")) {
                    helper_extension = g_strdup(sp_repr_children(child_repr)->content());
                }
                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    g_return_val_if_fail(command_text != NULL, FALSE);

    if (command != NULL)
        g_free(command);
    command = command_text;

    return TRUE;
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
    if (command != NULL) {
        g_free(command);
        command = NULL;
    }
    if (helper_extension != NULL) {
        g_free(helper_extension);
        helper_extension = NULL;
    }

    return;
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
                    gchar *command_text = solve_reldir(child_repr);
                    if (command_text != NULL) {
                        /* I've got the command */
                        bool existance;

                        existance = check_existance(command_text);
                        g_free(command_text);
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

    return TRUE;
}

/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module who's preferences need getting
    \param    filename  Hey, the file you're getting might be important

    This function should really do something, right now it doesn't.
*/
Gtk::Widget *
Script::prefs_input(Inkscape::Extension::Input *module, gchar const *filename)
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
    /*return module->autogui();*/
    return NULL;
}

/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module who's preferences need getting

    This function should really do something, right now it doesn't.
*/
Gtk::Widget *
Script::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *view)
{
    return module->autogui();
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
Script::open(Inkscape::Extension::Input *module, gchar const *filename)
{
    int data_read = 0;
    gint tempfd;
    gchar *tempfilename_out;

    // FIXME: process the GError instead of passing NULL
    if ((tempfd = g_file_open_tmp("ink_ext_XXXXXX", &tempfilename_out, NULL)) == -1) {
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

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    gchar *local_filename = g_filename_from_utf8( filename,
                                                  -1,  &bytesRead,  &bytesWritten, &error);

    data_read = execute(command, local_filename, tempfilename_out);
    g_free(local_filename);

    SPDocument *mydoc = NULL;
    if (data_read > 10) {
        if (helper_extension == NULL) {
            mydoc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), tempfilename_out);
        } else {
            mydoc = Inkscape::Extension::open(Inkscape::Extension::db.get(helper_extension), tempfilename_out);
        }
    }

    if (mydoc != NULL)
        sp_document_set_uri(mydoc, (const gchar *)filename);

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd);
    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_out);
    g_free(tempfilename_out);

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
Script::save(Inkscape::Extension::Output *module, SPDocument *doc, gchar const *filename)
{
    gint tempfd;
    gchar *tempfilename_in;
    // FIXME: process the GError instead of passing NULL
    if ((tempfd = g_file_open_tmp("ink_ext_XXXXXX", &tempfilename_in, NULL)) == -1) {
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

    if (helper_extension == NULL) {
        Inkscape::Extension::save(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE), doc, tempfilename_in, FALSE, FALSE, FALSE);
    } else {
        Inkscape::Extension::save(Inkscape::Extension::db.get(helper_extension), doc, tempfilename_in, FALSE, FALSE, FALSE);
    }

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    gchar *local_filename = g_filename_from_utf8( filename,
                                                  -1,  &bytesRead,  &bytesWritten, &error);

    execute(command, tempfilename_in, local_filename);

    g_free(local_filename);

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd);
    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_in);
    g_free(tempfilename_in);
}

/**
    \return    none
    \brief     This function uses an extention as a effect on a document.
    \param     module   Extention to effect with.
    \param     doc      Document to run through the effect.

    This function is a little bit trickier than the previous two.  It
    needs two temporary files to get it's work done.  Both of these
    files have random names created for them using the g_file_open_temp function
    with the sp_ext_ prefix in the temporary directory.  Like the other
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
    int data_read = 0;
    SPDocument * mydoc = NULL;
    gint tempfd_in;
    gchar *tempfilename_in;

    // FIXME: process the GError instead of passing NULL
    if ((tempfd_in = g_file_open_tmp("ink_ext_XXXXXX", &tempfilename_in, NULL)) == -1) {
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

    gint tempfd_out;
    gchar *tempfilename_out;
    // FIXME: process the GError instead of passing NULL
    if ((tempfd_out = g_file_open_tmp("ink_ext_XXXXXX", &tempfilename_out, NULL)) == -1) {
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

    Inkscape::Extension::save(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE),
                              doc->doc(), tempfilename_in, FALSE, FALSE, FALSE);

    Glib::ustring local_command(command);

    /* fixme: Should be some sort of checking here.  Don't know how to do this with structs instead
     * of classes. */
    SPDesktop *desktop = (SPDesktop *) doc;
    if (desktop != NULL) {
        using Inkscape::Util::GSListConstIterator;
        GSListConstIterator<SPItem *> selected = sp_desktop_selection(desktop)->itemList();
        while ( selected != NULL ) {
            local_command += " --id=";
            local_command += SP_OBJECT_ID(*selected);
            ++selected;
        }
    }

    Glib::ustring * paramString = module->paramString();
    local_command += *paramString;
    delete paramString;

    // std::cout << local_command << std::endl;

    data_read = execute(local_command.c_str(), tempfilename_in, tempfilename_out);

    if (data_read > 10)
        mydoc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), tempfilename_out);

    // make sure we don't leak file descriptors from g_file_open_tmp
    close(tempfd_in);
    close(tempfd_out);
    // FIXME: convert to utf8 (from "filename encoding") and unlink_utf8name
    unlink(tempfilename_in);
    g_free(tempfilename_in);
    unlink(tempfilename_out);
    g_free(tempfilename_out);

    /* Do something with mydoc.... */
    if (mydoc != NULL) {
        doc->doc()->emitReconstructionStart();
        copy_doc(doc->doc()->rroot, mydoc->rroot);
        doc->doc()->emitReconstructionFinish();
        mydoc->release();
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
        if (!strcmp("svg:defs", child->name()))
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
        if (!strcmp("svg:defs", child->name()))
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
    bool open(char *command, char const *errorFile, int mode);
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
    \return   none
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
Script::execute (const gchar * in_command, const gchar * filein, const gchar * fileout)
{
    g_return_val_if_fail(in_command != NULL, 0);
    // printf("Executing: %s\n", in_command);

    gchar * errorFile;
    gint errorFileNum;
    errorFileNum = g_file_open_tmp("ink_ext_stderr_XXXXXX", &errorFile, NULL);
    if (errorFileNum != 0) {
        close(errorFileNum);
    } else {
        g_free(errorFile);
        errorFile = NULL;
    }

    char *command = g_strdup_printf("%s \"%s\"", in_command, filein);
    // std::cout << "Command to run: " << command << std::endl;

    pipe_t pipe;
    bool open_success = pipe.open(command, errorFile, pipe_t::mode_read);
    g_free(command);

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

    Inkscape::IO::dump_fopen_call(fileout, "J");
    FILE *pfile = Inkscape::IO::fopen_utf8name(fileout, "w");

    if (pfile == NULL) {
        /* Error - could not open file */
        if (errno == EINVAL) {
            perror("Extension::Script:  The mode provided to fopen was invalid\n");
        } else {
            perror("Extension::Script:  Unknown error attempting to open temporary file\n");
        }
        return 0;
    }

    /* Copy pipe output to a temporary file */
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
            if (errorFile != NULL) {
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
        if (errorFile != NULL) {
            checkStderr(errorFile, Gtk::MESSAGE_INFO,
                _("Inkscape has received additional data from the script executed.  "
                  "The script did not return an error, but this may indicate the results will not be as expected."));
        }
    }

    if (errorFile != NULL) {
        unlink(errorFile);
        g_free(errorFile);
    }

    return amount_read;
}

/**  \brief  This function checks the stderr file, and if it has data,
             shows it in a warning dialog to the user
     \param  filename  Filename of the stderr file
*/
void
Script::checkStderr (gchar * filename, Gtk::MessageType type, gchar * message)
{
    // magic win32 crlf->lf conversion means the file length is not the same as
    // the text length, but luckily gtk will accept crlf in textviews so we can
    // just use binary mode
    std::ifstream stderrf (filename, std::ios_base::in | std::ios_base::binary);
    if (!stderrf.is_open()) return;

    stderrf.seekg(0, std::ios::end);
    int length = stderrf.tellg();
    if (0 == length) return;
    stderrf.seekg(0, std::ios::beg);

    Gtk::MessageDialog warning(message, false, type, Gtk::BUTTONS_OK, true);
    warning.set_resizable(true);

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

bool pipe_t::open(char *command, char const *errorFile, int mode_p) {
    HANDLE pipe_write;

    // Create pipe
    {
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
        if ( !DuplicateHandle(GetCurrentProcess(), t_pipe_read, GetCurrentProcess(), &hpipe, 0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS) ) {
            int en = translate_error(GetLastError());
            CloseHandle(t_pipe_read);
            CloseHandle(pipe_write);
            errno = en;
            return false;
        }
    }
    // Open stderr file
    HANDLE hStdErrFile = CreateFile(errorFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
    HANDLE hInheritableStdErr;
    DuplicateHandle(GetCurrentProcess(), hStdErrFile, GetCurrentProcess(), &hInheritableStdErr, 0, TRUE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS);

    // Create process
    {
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

        if ( !CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &procinfo) ) {
            errno = translate_error(GetLastError());
            return false;
        }
        CloseHandle(procinfo.hThread);
        CloseHandle(procinfo.hProcess);
    }

    // Close our copy of the write handle
    CloseHandle(hInheritableStdErr);
    CloseHandle(pipe_write);

    return true;
}

bool pipe_t::close() {
    BOOL retval = CloseHandle(hpipe);
    if ( !retval ) {
        errno = translate_error(GetLastError());
    }
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

#else // Win32

bool pipe_t::open(char *command, char const *errorFile, int mode_p) {
    char popen_mode[4] = {0,0,0,0};
    char *popen_mode_cur = popen_mode;

    if ( (mode_p & mode_read) != 0 ) {
        *popen_mode_cur++ = 'r';
    }

    if ( (mode_p & mode_write) != 0 ) {
        *popen_mode_cur++ = 'w';
    }

    /* Get the commandline to be run */
    if (errorFile != NULL) {
        char * temp;
        temp = g_strdup_printf("%s 2> %s", command, errorFile);
        ppipe = popen(temp, popen_mode);
        g_free(temp);
    } else
        ppipe = popen(command, popen_mode);

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


}  /* Inkscape  */
}  /* module  */
}  /* Implementation  */


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

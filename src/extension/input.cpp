/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "prefdialog.h"
#include "implementation/implementation.h"
#include "timer.h"
#include "input.h"
#include "io/sys.h"

/* Inkscape::Extension::Input */

namespace Inkscape {
namespace Extension {

/**
    \return   None
    \brief    Builds a SPModuleInput object from a XML description
    \param    module  The module to be initialized
    \param    repr    The XML description in a Inkscape::XML::Node tree

    Okay, so you want to build a SPModuleInput object.

    This function first takes and does the build of the parent class,
    which is SPModule.  Then, it looks for the <input> section of the
    XML description.  Under there should be several fields which
    describe the input module to excruciating detail.  Those are parsed,
    copied, and put into the structure that is passed in as module.
    Overall, there are many levels of indentation, just to handle the
    levels of indentation in the XML file.
*/
Input::Input (Inkscape::XML::Node * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
    mimetype = NULL;
    extension = NULL;
    filetypename = NULL;
    filetypetooltip = NULL;
    output_extension = NULL;

    if (repr != NULL) {
        Inkscape::XML::Node * child_repr;

        child_repr = repr->firstChild();

        while (child_repr != NULL) {
            if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "input")) {
                child_repr = child_repr->firstChild();
                while (child_repr != NULL) {
                    char const * chname = child_repr->name();
					if (!strncmp(chname, INKSCAPE_EXTENSION_NS_NC, strlen(INKSCAPE_EXTENSION_NS_NC))) {
						chname += strlen(INKSCAPE_EXTENSION_NS);
					}
                    if (chname[0] == '_') /* Allow _ for translation of tags */
                        chname++;
                    if (!strcmp(chname, "extension")) {
                        g_free (extension);
                        extension = g_strdup(child_repr->firstChild()->content());
                    }
                    if (!strcmp(chname, "mimetype")) {
                        g_free (mimetype);
                        mimetype = g_strdup(child_repr->firstChild()->content());
                    }
                    if (!strcmp(chname, "filetypename")) {
                        g_free (filetypename);
                        filetypename = g_strdup(child_repr->firstChild()->content());
                    }
                    if (!strcmp(chname, "filetypetooltip")) {
                        g_free (filetypetooltip);
                        filetypetooltip = g_strdup(child_repr->firstChild()->content());
                    }
                    if (!strcmp(chname, "output_extension")) {
                        g_free (output_extension);
                        output_extension = g_strdup(child_repr->firstChild()->content());
                    }

                    child_repr = child_repr->next();
                }

                break;
            }

            child_repr = child_repr->next();
        }

    }

    return;
}

/**
    \return  None
    \brief   Destroys an Input extension
*/
Input::~Input (void)
{
    g_free(mimetype);
    g_free(extension);
    g_free(filetypename);
    g_free(filetypetooltip);
    g_free(output_extension);
    return;
}

/**
    \return  Whether this extension checks out
    \brief   Validate this extension

    This function checks to make sure that the input extension has
    a filename extension and a MIME type.  Then it calls the parent
    class' check function which also checks out the implmentation.
*/
bool
Input::check (void)
{
    if (extension == NULL)
        return FALSE;
    if (mimetype == NULL)
        return FALSE;

    return Extension::check();
}

/**
    \return  A new document
    \brief   This function creates a document from a file
    \param   uri  The filename to create the document from

    This function acts as the first step in creating a new document
    from a file.  The first thing that this does is make sure that the
    file actually exists.  If it doesn't, a NULL is returned.  If the
    file exits, then it is opened using the implmentation of this extension.
*/
SPDocument *
Input::open (const gchar *uri)
{
    if (!loaded()) {
        set_state(Extension::STATE_LOADED);
    }
    if (!loaded()) {
        return NULL;
    }
    timer->touch();

    SPDocument *const doc = imp->open(this, uri);

    if (imp->wasCancelled()) {
        throw Input::open_cancelled();
    }

    return doc;
}

/**
    \return  IETF mime-type for the extension
    \brief   Get the mime-type that describes this extension
*/
gchar *
Input::get_mimetype(void)
{
    return mimetype;
}

/**
    \return  Filename extension for the extension
    \brief   Get the filename extension for this extension
*/
gchar *
Input::get_extension(void)
{
    return extension;
}

/**
    \return  The name of the filetype supported
    \brief   Get the name of the filetype supported
*/
gchar *
Input::get_filetypename(void)
{
    if (filetypename != NULL)
        return filetypename;
    else
        return get_name();
}

/**
    \return  Tooltip giving more information on the filetype
    \brief   Get the tooltip for more information on the filetype
*/
gchar *
Input::get_filetypetooltip(void)
{
    return filetypetooltip;
}

/**
    \return  A dialog to get settings for this extension
    \brief   Create a dialog for preference for this extension

    Calls the implementation to get the preferences.
*/
bool
Input::prefs (const gchar *uri)
{
    if (!loaded()) {
        set_state(Extension::STATE_LOADED);
    }
    if (!loaded()) {
        return false;
    }

    Gtk::Widget * controls;
    controls = imp->prefs_input(this, uri);
    if (controls == NULL) {
        // std::cout << "No preferences for Input" << std::endl;
        return true;
    }

    PrefDialog * dialog = new PrefDialog(this->get_name(), this->get_help(), controls);
    int response = dialog->run();
    dialog->hide();

    delete dialog;

    return (response == Gtk::RESPONSE_OK);
}

} }  /* namespace Inkscape, Extension */

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

/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen 
 * Copyright (C) 2002-2004 Ted Gould
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <interface.h>

#include "extension.h"
#include "db.h"
#include "input.h"
#include "output.h"
#include "effect.h"
#include "patheffect.h"
#include "print.h"
#include "implementation/script.h"
#include "implementation/xslt.h"
/* #include "implementation/plugin.h" */

namespace Inkscape {
namespace Extension {

static void open_internal(Inkscape::Extension::Extension *in_plug, gpointer in_data);
static void save_internal(Inkscape::Extension::Extension *in_plug, gpointer in_data);
static Extension *build_from_reprdoc(Inkscape::XML::Document *doc, Implementation::Implementation *in_imp);

/**
 * \return   A new document created from the filename passed in
 * \brief    This is a generic function to use the open function of
 *           a module (including Autodetect)
 * \param    key       Identifier of which module to use
 * \param    filename  The file that should be opened
 *
 * First things first, are we looking at an autodetection?  Well if that's the case then the module
 * needs to be found, and that is done with a database lookup through the module DB.  The foreach
 * function is called, with the parameter being a gpointer array.  It contains both the filename
 * (to find its extension) and where to write the module when it is found.
 *
 * If there is no autodetection, then the module database is queried with the key given.
 *
 * If everything is cool at this point, the module is loaded, and there is possibility for
 * preferences.  If there is a function, then it is executed to get the dialog to be displayed.
 * After it is finished the function continues.
 *
 * Lastly, the open function is called in the module itself.
 */
SPDocument *
open(Extension *key, gchar const *filename)
{
    Input *imod = NULL;
    if (key == NULL) {
        gpointer parray[2];
        parray[0] = (gpointer)filename;
        parray[1] = (gpointer)&imod;
        db.foreach(open_internal, (gpointer)&parray);
    } else {
        imod = dynamic_cast<Input *>(key);
    }

    bool last_chance_svg = false;
    if (key == NULL && imod == NULL) {
        last_chance_svg = true;
        imod = dynamic_cast<Input *>(db.get(SP_MODULE_KEY_INPUT_SVG));
    }

    if (imod == NULL) {
        throw Input::no_extension_found();
    }

    imod->set_state(Extension::STATE_LOADED);

    if (!imod->loaded()) {
        throw Input::open_failed();
    }

    if (!imod->prefs(filename))
        return NULL;

    SPDocument *doc = imod->open(filename);
    if (!doc) {
        return NULL;
    }

    if (last_chance_svg) {
        /* We can't call sp_ui_error_dialog because we may be 
           running from the console, in which case calling sp_ui
           routines will cause a segfault.  See bug 1000350 - bryce */
        // sp_ui_error_dialog(_("Format autodetect failed. The file is being opened as SVG."));
        g_warning(_("Format autodetect failed. The file is being opened as SVG."));
    }

    /* This kinda overkill as most of these are already set, but I want
       to make sure for this release -- TJG */
    doc->setModifiedSinceSave(false);

    sp_document_set_uri(doc, filename);

    return doc;
}

/**
 * \return   none
 * \brief    This is the function that searches each module to see
 *           if it matches the filename for autodetection.
 * \param    in_plug  The module to be tested
 * \param    in_data  An array of pointers containing the filename, and
 *                    the place to put a successfully found module.
 *
 * Basically this function only looks at input modules as it is part of the open function.  If the
 * module is an input module, it then starts to take it apart, and the data that is passed in.
 * Because the data being passed in is in such a weird format, there are a few casts to make it
 * easier to use.  While it looks like a lot of local variables, they'll all get removed by the
 * compiler.
 *
 * First thing that is checked is if the filename is shorter than the extension itself.  There is
 * no way for a match in that case.  If it's long enough then there is a string compare of the end
 * of the filename (for the length of the extension), and the extension itself.  If this passes
 * then the pointer passed in is set to the current module.
 */
static void
open_internal(Extension *in_plug, gpointer in_data)
{
    if (!in_plug->deactivated() && dynamic_cast<Input *>(in_plug)) {
        gpointer *parray = (gpointer *)in_data;
        gchar const *filename = (gchar const *)parray[0];
        Input **pimod = (Input **)parray[1];

        // skip all the rest if we already found a function to open it
        // since they're ordered by preference now.
        if (!*pimod) {
            gchar const *ext = dynamic_cast<Input *>(in_plug)->get_extension();

            gchar *filenamelower = g_utf8_strdown(filename, -1);
            gchar *extensionlower = g_utf8_strdown(ext, -1);

            if (g_str_has_suffix(filenamelower, extensionlower)) {
                *pimod = dynamic_cast<Input *>(in_plug);
            }

            g_free(filenamelower);
            g_free(extensionlower);
        }
    }

    return;
}

/**
 * \return   None
 * \brief    This is a generic function to use the save function of
 *           a module (including Autodetect)
 * \param    key       Identifier of which module to use
 * \param    doc       The document to be saved
 * \param    filename  The file that the document should be saved to
 * \param    official  (optional) whether to set :output_module and :modified in the
 *                     document; is true for normal save, false for temporary saves
 *
 * First things first, are we looking at an autodetection?  Well if that's the case then the module
 * needs to be found, and that is done with a database lookup through the module DB.  The foreach
 * function is called, with the parameter being a gpointer array.  It contains both the filename
 * (to find its extension) and where to write the module when it is found.
 *
 * If there is no autodetection the module database is queried with the key given.
 *
 * If everything is cool at this point, the module is loaded, and there is possibility for
 * preferences.  If there is a function, then it is executed to get the dialog to be displayed.
 * After it is finished the function continues.
 *
 * Lastly, the save function is called in the module itself.
 */
void
save(Extension *key, SPDocument *doc, gchar const *filename, bool setextension, bool check_overwrite, bool official)
{
    Output *omod;
    if (key == NULL) {
        gpointer parray[2];
        parray[0] = (gpointer)filename;
        parray[1] = (gpointer)&omod;
        omod = NULL;
        db.foreach(save_internal, (gpointer)&parray);

        /* This is a nasty hack, but it is required to ensure that
           autodetect will always save with the Inkscape extensions
           if they are available. */
        if (omod != NULL && !strcmp(omod->get_id(), SP_MODULE_KEY_OUTPUT_SVG)) {
            omod = dynamic_cast<Output *>(db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE));
        }
        /* If autodetect fails, save as Inkscape SVG */
        if (omod == NULL) {
            omod = dynamic_cast<Output *>(db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE));
        }
    } else {
        omod = dynamic_cast<Output *>(key);
    }

    if (!dynamic_cast<Output *>(omod)) {
        g_warning("Unable to find output module to handle file: %s\n", filename);
        throw Output::no_extension_found();
        return;
    }

    omod->set_state(Extension::STATE_LOADED);
    if (!omod->loaded()) {
        throw Output::save_failed();
    }

    if (!omod->prefs()) {
        throw Output::save_cancelled();
    }

    gchar *fileName = NULL;
    if (setextension) {
        gchar *lowerfile = g_utf8_strdown(filename, -1);
        gchar *lowerext = g_utf8_strdown(omod->get_extension(), -1);

        if (!g_str_has_suffix(lowerfile, lowerext)) {
            fileName = g_strdup_printf("%s%s", filename, omod->get_extension());
        }

        g_free(lowerfile);
        g_free(lowerext);
    }

    if (fileName == NULL) {
        fileName = g_strdup(filename);
    }

    if (check_overwrite && !sp_ui_overwrite_file(fileName)) {
        g_free(fileName);
        throw Output::no_overwrite();
    }

    Inkscape::XML::Node *repr = sp_document_repr_root(doc);

    // remember attributes in case this is an unofficial save
    bool saved_modified = false;
    gchar *saved_output_extension = NULL;
    gchar *saved_dataloss = NULL;
    gchar *saved_uri = NULL;
    if (!official) {
        saved_modified = doc->isModifiedSinceSave();
        saved_output_extension = g_strdup(repr->attribute("inkscape:output_extension"));
        saved_dataloss = g_strdup(repr->attribute("inkscape:dataloss"));
        saved_uri = g_strdup(doc->uri);    
    }    

    // update attributes:
    bool saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive (doc, false); 
    {
        // save the filename for next use
        sp_document_set_uri(doc, fileName);
        // also save the extension for next use
        repr->setAttribute("inkscape:output_extension", omod->get_id());
        // set the "dataloss" attribute if the chosen extension is lossy
        repr->setAttribute("inkscape:dataloss", NULL);
        if ( omod->causes_dataloss() ) {
            repr->setAttribute("inkscape:dataloss", "true");
        }
    }
    sp_document_set_undo_sensitive (doc, saved);
    doc->setModifiedSinceSave(false);

    omod->save(doc, fileName);
    
    // if it is an unofficial save, set the modified attributes back to what they were    
    if ( !official) {
        saved = sp_document_get_undo_sensitive(doc);
        sp_document_set_undo_sensitive (doc, false);
        {
            repr->setAttribute("inkscape:output_extension", saved_output_extension);
            repr->setAttribute("inkscape:dataloss", saved_dataloss);
            sp_document_set_uri(doc, saved_uri);
        }
        sp_document_set_undo_sensitive (doc, saved);
        doc->setModifiedSinceSave(saved_modified);

        g_free(saved_output_extension);
        g_free(saved_dataloss);
        g_free(saved_uri);
    }
    
    g_free(fileName);
    return;
}

/**
 * \return   none
 * \brief    This is the function that searches each module to see
 *           if it matches the filename for autodetection.
 * \param    in_plug  The module to be tested
 * \param    in_data  An array of pointers containing the filename, and
 *                    the place to put a successfully found module.
 *
 * Basically this function only looks at output modules as it is part of the open function.  If the
 * module is an output module, it then starts to take it apart, and the data that is passed in.
 * Because the data being passed in is in such a weird format, there are a few casts to make it
 * easier to use.  While it looks like a lot of local variables, they'll all get removed by the
 * compiler.
 *
 * First thing that is checked is if the filename is shorter than the extension itself.  There is
 * no way for a match in that case.  If it's long enough then there is a string compare of the end
 * of the filename (for the length of the extension), and the extension itself.  If this passes
 * then the pointer passed in is set to the current module.
 */
static void
save_internal(Extension *in_plug, gpointer in_data)
{
    if (!in_plug->deactivated() && dynamic_cast<Output *>(in_plug)) {
        gpointer *parray = (gpointer *)in_data;
        gchar const *filename = (gchar const *)parray[0];
        Output **pomod = (Output **)parray[1];

        // skip all the rest if we already found someone to save it
        // since they're ordered by preference now.
        if (!*pomod) {
            gchar const *ext = dynamic_cast<Output *>(in_plug)->get_extension();

            gchar *filenamelower = g_utf8_strdown(filename, -1);
            gchar *extensionlower = g_utf8_strdown(ext, -1);

            if (g_str_has_suffix(filenamelower, extensionlower)) {
                *pomod = dynamic_cast<Output *>(in_plug);
            }

            g_free(filenamelower);
            g_free(extensionlower);
        }
    }

    return;
}

Print *
get_print(gchar const *key)
{
    return dynamic_cast<Print *>(db.get(key));
}

/**
 * \return   The built module
 * \brief    Creates a module from a Inkscape::XML::Document describing the module
 * \param    doc  The XML description of the module
 *
 * This function basically has two segments.  The first is that it goes through the Repr tree
 * provided, and determines what kind of of module this is, and what kind of implementation to use.
 * All of these are then stored in two enums that are defined in this function.  This makes it
 * easier to add additional types (which will happen in the future, I'm sure).
 *
 * Second, there is case statements for these enums.  The first one is the type of module.  This is
 * the one where the module is actually created.  After that, then the implementation is applied to
 * get the load and unload functions.  If there is no implementation then these are not set.  This
 * case could apply to modules that are built in (like the SVG load/save functions).
 */
static Extension *
build_from_reprdoc(Inkscape::XML::Document *doc, Implementation::Implementation *in_imp)
{
    enum {
        MODULE_EXTENSION,
        MODULE_XSLT,
        /* MODULE_PLUGIN, */
        MODULE_UNKNOWN_IMP
    } module_implementation_type = MODULE_UNKNOWN_IMP;
    enum {
        MODULE_INPUT,
        MODULE_OUTPUT,
        MODULE_FILTER,
        MODULE_PRINT,
        MODULE_PATH_EFFECT,
        MODULE_UNKNOWN_FUNC
    } module_functional_type = MODULE_UNKNOWN_FUNC;

    g_return_val_if_fail(doc != NULL, NULL);

    Inkscape::XML::Node *repr = doc->root();

    /* sp_repr_print(repr); */

    if (strcmp(repr->name(), INKSCAPE_EXTENSION_NS "inkscape-extension")) {
        g_warning("Extension definition started with <%s> instead of <" INKSCAPE_EXTENSION_NS "inkscape-extension>.  Extension will not be created. See http://wiki.inkscape.org/wiki/index.php/Extensions for reference.\n", repr->name());
        return NULL;
    }

    Inkscape::XML::Node *child_repr = sp_repr_children(repr);
    while (child_repr != NULL) {
        char const *element_name = child_repr->name();
        /* printf("Child: %s\n", child_repr->name()); */
        if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "input")) {
            module_functional_type = MODULE_INPUT;
        } else if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "output")) {
            module_functional_type = MODULE_OUTPUT;
        } else if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "effect")) {
            module_functional_type = MODULE_FILTER;
        } else if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "print")) {
            module_functional_type = MODULE_PRINT;
        } else if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "path-effect")) {
            module_functional_type = MODULE_PATH_EFFECT;
        } else if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "script")) {
            module_implementation_type = MODULE_EXTENSION;
        } else if (!strcmp(element_name, INKSCAPE_EXTENSION_NS "xslt")) {
            module_implementation_type = MODULE_XSLT;
#if 0
        } else if (!strcmp(element_name, "plugin")) {
            module_implementation_type = MODULE_PLUGIN;
#endif
        }

        //Inkscape::XML::Node *old_repr = child_repr;
        child_repr = sp_repr_next(child_repr);
        //Inkscape::GC::release(old_repr);
    }

    Implementation::Implementation *imp;
    if (in_imp == NULL) {
        switch (module_implementation_type) {
            case MODULE_EXTENSION: {
                Implementation::Script *script = new Implementation::Script();
                imp = static_cast<Implementation::Implementation *>(script);
                break;
            }
            case MODULE_XSLT: {
                Implementation::XSLT *xslt = new Implementation::XSLT();
                imp = static_cast<Implementation::Implementation *>(xslt);
                break;
            }
#if 0
            case MODULE_PLUGIN: {
                Implementation::Plugin *plugin = new Implementation::Plugin();
                imp = static_cast<Implementation::Implementation *>(plugin);
                break;
            }
#endif
            default: {
                imp = NULL;
                break;
            }
        }
    } else {
        imp = in_imp;
    }

    Extension *module = NULL;
    switch (module_functional_type) {
        case MODULE_INPUT: {
            module = new Input(repr, imp);
            break;
        }
        case MODULE_OUTPUT: {
            module = new Output(repr, imp);
            break;
        }
        case MODULE_FILTER: {
            module = new Effect(repr, imp);
            break;
        }
        case MODULE_PRINT: {
            module = new Print(repr, imp);
            break;
        }
        case MODULE_PATH_EFFECT: {
            module = new PathEffect(repr, imp);
            break;
        }
        default: {
            break;
        }
    }

    return module;
}

/**
 * \return   The module created
 * \brief    This function creates a module from a filename of an
 *           XML description.
 * \param    filename  The file holding the XML description of the module.
 *
 * This function calls build_from_reprdoc with using sp_repr_read_file to create the reprdoc.
 */
Extension *
build_from_file(gchar const *filename)
{
    Inkscape::XML::Document *doc = sp_repr_read_file(filename, INKSCAPE_EXTENSION_URI);
    Extension *ext = build_from_reprdoc(doc, NULL);
    if (ext != NULL)
        Inkscape::GC::release(doc);
    else
        g_warning("Unable to create extension from definition file %s.\n", filename);
    return ext;
}

/**
 * \return   The module created
 * \brief    This function creates a module from a buffer holding an
 *           XML description.
 * \param    buffer  The buffer holding the XML description of the module.
 *
 * This function calls build_from_reprdoc with using sp_repr_read_mem to create the reprdoc.  It
 * finds the length of the buffer using strlen.
 */
Extension *
build_from_mem(gchar const *buffer, Implementation::Implementation *in_imp)
{
    Inkscape::XML::Document *doc = sp_repr_read_mem(buffer, strlen(buffer), INKSCAPE_EXTENSION_URI);
    Extension *ext = build_from_reprdoc(doc, in_imp);
    Inkscape::GC::release(doc);
    return ext;
}


} } /* namespace Inkscape::Extension */

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

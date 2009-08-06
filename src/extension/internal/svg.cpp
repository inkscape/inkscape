/*
 * This is the code that moves all of the SVG loading and saving into
 * the module format.  Really Inkscape is built to handle these formats
 * internally, so this is just calling those internal functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "sp-object.h"
#include "svg.h"
#include "file.h"
#include "extension/system.h"
#include "extension/output.h"
#include <vector>

#ifdef WITH_GNOME_VFS
# include <libgnomevfs/gnome-vfs.h>
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

#include "clear-n_.h"

/**
    \return   None
    \brief    What would an SVG editor be without loading/saving SVG
              files.  This function sets that up.

    For each module there is a call to Inkscape::Extension::build_from_mem
    with a rather large XML file passed in.  This is a constant string
    that describes the module.  At the end of this call a module is
    returned that is basically filled out.  The one thing that it doesn't
    have is the key function for the operation.  And that is linked at
    the end of each call.
*/
void
Svg::init(void)
{
    Inkscape::Extension::Extension * ext;

    /* SVG in */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Input") "</name>\n"
            "<id>" SP_MODULE_KEY_INPUT_SVG "</id>\n"
            "<input>\n"
                "<extension>.svg</extension>\n"
                "<mimetype>image/svg+xml</mimetype>\n"
                "<filetypename>" N_("Scalable Vector Graphic (*.svg)") "</filetypename>\n"
                "<filetypetooltip>" N_("Inkscape native file format and W3C standard") "</filetypetooltip>\n"
                "<output_extension>" SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "</output_extension>\n"
            "</input>\n"
        "</inkscape-extension>", new Svg());

    /* SVG out Inkscape */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Output Inkscape") "</name>\n"
            "<id>" SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "</id>\n"
            "<output>\n"
                "<extension>.svg</extension>\n"
                "<mimetype>image/x-inkscape-svg</mimetype>\n"
                "<filetypename>" N_("Inkscape SVG (*.svg)") "</filetypename>\n"
                "<filetypetooltip>" N_("SVG format with Inkscape extensions") "</filetypetooltip>\n"
                "<dataloss>false</dataloss>\n"
            "</output>\n"
        "</inkscape-extension>", new Svg());

    /* SVG out */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Output") "</name>\n"
            "<id>" SP_MODULE_KEY_OUTPUT_SVG "</id>\n"
            "<output>\n"
                "<extension>.svg</extension>\n"
                "<mimetype>image/svg+xml</mimetype>\n"
                "<filetypename>" N_("Plain SVG (*.svg)") "</filetypename>\n"
                "<filetypetooltip>" N_("Scalable Vector Graphics format as defined by the W3C") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new Svg());

#ifdef WITH_GNOME_VFS
    gnome_vfs_init();
#endif


    return;
}


#ifdef WITH_GNOME_VFS
#define BUF_SIZE 8192

static gchar *
_load_uri (const gchar *uri)
{
    GnomeVFSHandle   *handle = NULL;
    GnomeVFSFileSize  bytes_read;

        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError* error = NULL;
        gchar* uri_local = g_filename_from_utf8( uri, -1, &bytesRead, &bytesWritten, &error);

        if ( uri_local == NULL ) {
            g_warning( "Error converting filename to locale encoding.");
        }

    GnomeVFSResult result = gnome_vfs_open (&handle, uri_local, GNOME_VFS_OPEN_READ);

    if (result != GNOME_VFS_OK) {
        g_warning("%s", gnome_vfs_result_to_string(result));
    }

    std::vector<gchar> doc;
    while (result == GNOME_VFS_OK) {
        gchar buffer[BUF_SIZE];
        result = gnome_vfs_read (handle, buffer, BUF_SIZE, &bytes_read);
        doc.insert(doc.end(), buffer, buffer+bytes_read);
    }

    return g_strndup(&doc[0], doc.size());
}
#endif


/**
    \return    A new document just for you!
    \brief     This function takes in a filename of a SVG document and
               turns it into a SPDocument.
    \param     mod   Module to use
    \param     uri   The path to the file (UTF-8)

    This function is really simple, it just calls sp_document_new...
*/
SPDocument *
Svg::open (Inkscape::Extension::Input */*mod*/, const gchar *uri)
{
#ifdef WITH_GNOME_VFS
    if (!gnome_vfs_initialized() || gnome_vfs_uri_is_local(gnome_vfs_uri_new(uri))) {
        // Use built-in loader instead of VFS for this
        return sp_document_new(uri, TRUE);
    }
    gchar * buffer = _load_uri(uri);
    if (buffer == NULL) {
        g_warning("Error:  Could not open file '%s' with VFS\n", uri);
        return NULL;
    }
    SPDocument * doc = sp_document_new_from_mem(buffer, strlen(buffer), 1);

    g_free(buffer);
    return doc;
#else
    return sp_document_new (uri, TRUE);
#endif
}

/**
    \return    None
    \brief     This is the function that does all of the SVG saves in
               Inkscape.  It detects whether it should do a Inkscape
               namespace save internally.
    \param     mod   Extension to use.
    \param     doc   Document to save.
    \param     uri   The filename to save the file to.

    This function first checks its parameters, and makes sure that
    we're getting good data.  It also checks the module ID of the
    incoming module to figure out whether this save should include
    the Inkscape namespace stuff or not.  The result of that comparison
    is stored in the spns variable.

    If there is not to be Inkscape name spaces a new document is created
    without.  (I think, I'm not sure on this code)

    All of the internally referenced imageins are also set to relative
    paths in the file.  And the file is saved.

    This really needs to be fleshed out more, but I don't quite understand
    all of this code.  I just stole it.
*/
void
Svg::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    g_return_if_fail(doc != NULL);
    g_return_if_fail(filename != NULL);

    gchar *save_path = g_path_get_dirname(filename);

    bool const spns = ( !mod->get_id()
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE)
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVGZ_INKSCAPE));

    Inkscape::XML::Document *rdoc = NULL;
    Inkscape::XML::Node *repr = NULL;
    if (spns) {
        repr = sp_document_repr_root (doc);
    } else {
        rdoc = sp_repr_document_new ("svg:svg");
        repr = rdoc->root();
        repr = sp_document_root (doc)->updateRepr(rdoc, repr, SP_OBJECT_WRITE_BUILD);
    }

    if (!sp_repr_save_rebased_file(repr->document(), filename, SP_SVG_NS_URI,
                                   doc->base, filename)) {
        throw Inkscape::Extension::Output::save_failed();
    }

    if (!spns) {
        Inkscape::GC::release(rdoc);
    }

    g_free(save_path);

    return;
}

} } }  /* namespace inkscape, module, implementation */

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

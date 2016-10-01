/*
 * This is the code that moves all of the SVG loading and saving into
 * the module format.  Really Inkscape is built to handle these formats
 * internally, so this is just calling those internal functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
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
#include "xml/attribute-record.h"
#include "xml/simple-document.h"
#include "sp-root.h"
#include "document.h"

#ifdef WITH_GNOME_VFS
# include <libgnomevfs/gnome-vfs.h>
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

#include "clear-n_.h"


using Inkscape::Util::List;
using Inkscape::XML::AttributeRecord;
using Inkscape::XML::Node;

/*
 * Removes all sodipodi and inkscape elements and attributes from an xml tree. 
 * used to make plain svg output.
 */
static void pruneExtendedNamespaces( Inkscape::XML::Node *repr )
{
    if (repr) {
        if ( repr->type() == Inkscape::XML::ELEMENT_NODE ) {
            std::vector<gchar const*> attrsRemoved;
            for ( List<AttributeRecord const> it = repr->attributeList(); it; ++it ) {
                const gchar* attrName = g_quark_to_string(it->key);
                if ((strncmp("inkscape:", attrName, 9) == 0) || (strncmp("sodipodi:", attrName, 9) == 0)) {
                    attrsRemoved.push_back(attrName);
                }
            }
            // Can't change the set we're interating over while we are iterating.
            for ( std::vector<gchar const*>::iterator it = attrsRemoved.begin(); it != attrsRemoved.end(); ++it ) {
                repr->setAttribute(*it, 0);
            }
        }

        std::vector<Inkscape::XML::Node *> nodesRemoved;
        for ( Node *child = repr->firstChild(); child; child = child->next() ) {
            if((strncmp("inkscape:", child->name(), 9) == 0) || strncmp("sodipodi:", child->name(), 9) == 0) {
                nodesRemoved.push_back(child);
            } else {
                pruneExtendedNamespaces(child);
            }
        }
        for ( std::vector<Inkscape::XML::Node *>::iterator it = nodesRemoved.begin(); it != nodesRemoved.end(); ++it ) {
            repr->removeChild(*it);
        }
    }
}

/*
 * Similar to the above sodipodi and inkscape prune, but used on all documents
 * to remove problematic elements (for example Adobe's i:pgf tag) only removes
 * known garbage tags.
 */
static void pruneProprietaryGarbage( Inkscape::XML::Node *repr )
{
    if (repr) {
        std::vector<Inkscape::XML::Node *> nodesRemoved;
        for ( Node *child = repr->firstChild(); child; child = child->next() ) { 
            if((strncmp("i:pgf", child->name(), 5) == 0)) {
                nodesRemoved.push_back(child);
                g_warning( "An Adobe proprietary tag was found which is known to cause issues. It was removed before saving.");
            } else {
                pruneProprietaryGarbage(child);
            }
        }
        for ( std::vector<Inkscape::XML::Node *>::iterator it = nodesRemoved.begin(); it != nodesRemoved.end(); ++it ) { 
            repr->removeChild(*it);
        }
    }
}

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
    /* SVG in */
    Inkscape::Extension::build_from_mem(
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
    Inkscape::Extension::build_from_mem(
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
    Inkscape::Extension::build_from_mem(
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
        return SPDocument::createNewDoc(uri, TRUE);
    }
    gchar * buffer = _load_uri(uri);
    if (buffer == NULL) {
        g_warning("Error:  Could not open file '%s' with VFS\n", uri);
        return NULL;
    }
    SPDocument * doc = SPDocument::createNewDocFromMem(buffer, strlen(buffer), 1);

    g_free(buffer);
    return doc;
#else
    return SPDocument::createNewDoc(uri, TRUE);
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
    is stored in the exportExtensions variable.

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
    Inkscape::XML::Document *rdoc = doc->rdoc;

    bool const exportExtensions = ( !mod->get_id()
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE)
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVGZ_INKSCAPE));

    // We prune the in-use document and deliberately loose data, because there
    // is no known use for this data at the present time.
    pruneProprietaryGarbage(rdoc->root());

    if (!exportExtensions) {
        // We make a duplicate document so we don't prune the in-use document
        // and loose data. Perhaps the user intends to save as inkscape-svg next.
        Inkscape::XML::Document *new_rdoc = new Inkscape::XML::SimpleDocument();

        // Comments and PI nodes are not included in this duplication
        // TODO: Move this code into xml/document.h and duplicate rdoc instead of root. 
        new_rdoc->setAttribute("version", "1.0");
        new_rdoc->setAttribute("standalone", "no");

        // Get a new xml repr for the svg root node
        Inkscape::XML::Node *root = rdoc->root()->duplicate(new_rdoc);

        // Add the duplicated svg node as the document's rdoc
        new_rdoc->appendChild(root);
        Inkscape::GC::release(root);

        pruneExtendedNamespaces(root);
        rdoc = new_rdoc;
    }

    if (!sp_repr_save_rebased_file(rdoc, filename, SP_SVG_NS_URI,
                                   doc->getBase(), filename)) {
        throw Inkscape::Extension::Output::save_failed();
    }

    if (!exportExtensions) {
        Inkscape::GC::release(rdoc);
    }

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

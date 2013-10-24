#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <boost/scoped_ptr.hpp>
#include <glib/gprintf.h>
#include <glibmm/i18n.h>
#include "document-private.h"
#include <dir-util.h>
#include "extension/input.h"
#include "extension/system.h"
#include "gdkpixbuf-input.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "sp-image.h"
#include "document-undo.h"
#include "util/units.h"
#include "image-resolution.h"
#include "display/cairo-utils.h"
#include <set>

namespace Inkscape {

namespace Extension {
namespace Internal {

SPDocument *
GdkpixbufInput::open(Inkscape::Extension::Input *mod, char const *uri)
{
    // determine whether the image should be embedded
    // TODO: this logic seems very wrong
    bool embed = false;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring attr = prefs->getString("/dialogs/import/link");
    if (strcmp(attr.c_str(), "embed") == 0) {
        embed = true;
    } else if (strcmp(attr.c_str(), "link") == 0) {
        embed = false;
    } else {
        embed = (strcmp(mod->get_param_optiongroup("link"), "embed") == 0);
        if (mod->get_param_bool("ask")) {
            prefs->setString("/dialogs/import/link", mod->get_param_optiongroup("link"));
            mod->set_param_bool("ask", false);
        }
    }

    SPDocument *doc = NULL;
    boost::scoped_ptr<Inkscape::Pixbuf> pb(Inkscape::Pixbuf::create_from_file(uri));

    // TODO: the pixbuf is created again from the base64-encoded attribute in SPImage.
    // Find a way to create the pixbuf only once.

    if (pb) {
        doc = SPDocument::createNewDoc(NULL, TRUE, TRUE);
        bool saved = DocumentUndo::getUndoSensitive(doc);
        DocumentUndo::setUndoSensitive(doc, false); // no need to undo in this temporary document

        double width = pb->width();
        double height = pb->height();
        double defaultxdpi = prefs->getDouble("/dialogs/import/defaultxdpi/value", Inkscape::Util::Quantity::convert(1, "in", "px"));
        bool forcexdpi = prefs->getBool("/dialogs/import/forcexdpi");
        ImageResolution *ir = 0;
        double xscale = 1;
        double yscale = 1;


        if (!ir && !forcexdpi) {
            ir = new ImageResolution(uri);
        }
        if (ir && ir->ok()) {
            xscale = 900.0 / floor(10.*ir->x() + .5);  // round-off to 0.1 dpi
            yscale = 900.0 / floor(10.*ir->y() + .5);
        } else {
            xscale = 90.0 / defaultxdpi;
            yscale = 90.0 / defaultxdpi;
        }

        width *= xscale;
        height *= yscale;

        delete ir; // deleting NULL is safe

        // Create image node
        Inkscape::XML::Document *xml_doc = doc->getReprDoc();
        Inkscape::XML::Node *image_node = xml_doc->createElement("svg:image");
        sp_repr_set_svg_double(image_node, "width", width);
        sp_repr_set_svg_double(image_node, "height", height);

        if (embed) {
            sp_embed_image(image_node, pb.get());
        } else {
            // convert filename to uri
            gchar* _uri = g_filename_to_uri(uri, NULL, NULL);
            if(_uri) {
                image_node->setAttribute("xlink:href", _uri);
                g_free(_uri);
            } else {
                image_node->setAttribute("xlink:href", uri);
            }
        }

        // Add it to the current layer
        doc->getRoot()->appendChildRepr(image_node);
        Inkscape::GC::release(image_node);
        fit_canvas_to_drawing(doc);
        
        // Set viewBox if it doesn't exist
        if (!doc->getRoot()->viewBox_set) {
            doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDefaultUnit()), doc->getHeight().value(doc->getDefaultUnit())));
        }
        
        // restore undo, as now this document may be shown to the user if a bitmap was opened
        DocumentUndo::setUndoSensitive(doc, saved);
    } else {
        printf("GdkPixbuf loader failed\n");
    }

    return doc;
}

#include "clear-n_.h"

void
GdkpixbufInput::init(void)
{
    GSList * formatlist, * formatlisthead;

    /* \todo I'm not sure if I need to free this list */
    for (formatlist = formatlisthead = gdk_pixbuf_get_formats();
         formatlist != NULL;
         formatlist = g_slist_next(formatlist)) {

        GdkPixbufFormat *pixformat = (GdkPixbufFormat *)formatlist->data;

        gchar *name =        gdk_pixbuf_format_get_name(pixformat);
        gchar *description = gdk_pixbuf_format_get_description(pixformat);
        gchar **extensions =  gdk_pixbuf_format_get_extensions(pixformat);
        gchar **mimetypes =   gdk_pixbuf_format_get_mime_types(pixformat);

        for (int i = 0; extensions[i] != NULL; i++) {
        for (int j = 0; mimetypes[j] != NULL; j++) {

            /* thanks but no thanks, we'll handle SVG extensions... */
            if (strcmp(extensions[i], "svg") == 0) {
                continue;
            }
            if (strcmp(extensions[i], "svgz") == 0) {
                continue;
            }
            if (strcmp(extensions[i], "svg.gz") == 0) {
                continue;
            }
            gchar *caption = g_strdup_printf(_("%s bitmap image import"), name);

            gchar *xmlString = g_strdup_printf(
                "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
                  "<name>%s</name>\n"
                  "<id>org.inkscape.input.gdkpixbuf.%s</id>\n"
                  "<param name='link' type='optiongroup' appearance='full' _gui-text='" N_("Link or embed image:") "' >\n"
                    "<_option value='embed' >" N_("Embed") "</_option>\n"
                    "<_option value='link' >" N_("Link") "</_option>\n"
                  "</param>\n"
                  "<_param name='help' type='description'>" N_("Embed results in stand-alone, larger SVG files. Link references a file outside this SVG document and all files must be moved together.") "</_param>\n"
                  "<param name=\"ask\" _gui-description='" N_("Hide the dialog next time and always apply the same action.") "' gui-text=\"" N_("Don't ask again") "\" type=\"boolean\" >false</param>\n"
                  "<input>\n"
                    "<extension>.%s</extension>\n"
                    "<mimetype>%s</mimetype>\n"
                    "<filetypename>%s (*.%s)</filetypename>\n"
                    "<filetypetooltip>%s</filetypetooltip>\n"
                  "</input>\n"
                "</inkscape-extension>",
                caption,
                extensions[i],
                extensions[i],
                mimetypes[j],
                name,
                extensions[i],
                description
                );

            Inkscape::Extension::build_from_mem(xmlString, new GdkpixbufInput());
            g_free(xmlString);
            g_free(caption);
        }}

        g_free(name);
        g_free(description);
        g_strfreev(mimetypes);
        g_strfreev(extensions);
    }

    g_slist_free(formatlisthead);
}

} } }  /* namespace Inkscape, Extension, Implementation */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

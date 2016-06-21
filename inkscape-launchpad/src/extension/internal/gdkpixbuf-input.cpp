#include <gdk-pixbuf/gdk-pixbuf.h>

#include <boost/scoped_ptr.hpp>
#include <glib/gprintf.h>
#include <glibmm/i18n.h>
#include "dir-util.h"
#include "display/cairo-utils.h"
#include "document-private.h"
#include "document-undo.h"
#include "extension/input.h"
#include "extension/system.h"
#include "image-resolution.h"
#include "gdkpixbuf-input.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "sp-image.h"
#include "util/units.h"
#include <set>

namespace Inkscape {

namespace Extension {
namespace Internal {

SPDocument *
GdkpixbufInput::open(Inkscape::Extension::Input *mod, char const *uri)
{
    // Determine whether the image should be embedded
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool ask = prefs->getBool("/dialogs/import/ask");
    Glib::ustring link  = prefs->getString("/dialogs/import/link");
    bool forcexdpi = prefs->getBool("/dialogs/import/forcexdpi");
    Glib::ustring scale = prefs->getString("/dialogs/import/scale");
    // std::cout << "GkdpixbufInput::open: "
    //           << " ask: " << ask
    //           << ", link: " << link 
    //           << ", forcexdpi: " << forcexdpi 
    //           << ", scale: " << scale << std::endl;
    // std::cout << "     in  preferences: "
    //           << " ask: " << !mod->get_param_bool("do_not_ask")
    //           << ", link: " << mod->get_param_optiongroup("link")
    //           << ", mod_dpi: " << mod->get_param_optiongroup("dpi")
    //           << ", scale: " << mod->get_param_optiongroup("scale") << std::endl;
    if( ask ) {
        Glib::ustring mod_link = mod->get_param_optiongroup("link");
        Glib::ustring mod_dpi = mod->get_param_optiongroup("dpi");
        bool mod_forcexdpi = ( mod_dpi.compare( "from_default" ) == 0 );
        Glib::ustring mod_scale = mod->get_param_optiongroup("scale");
        if( link.compare( mod_link ) != 0 ) {
            link = mod_link;
        }
        prefs->setString("/dialogs/import/link", link );
        if( forcexdpi != mod_forcexdpi ) {
            forcexdpi = mod_forcexdpi;
        }
        prefs->setBool("/dialogs/import/forcexdpi", forcexdpi );
        if( scale.compare( mod_scale ) != 0 ) {
            scale = mod_scale;
        }
        prefs->setString("/dialogs/import/scale", scale );
        prefs->setBool("/dialogs/import/ask", !mod->get_param_bool("do_not_ask") );
    }
    bool embed = ( link.compare( "embed" ) == 0 );
 
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
        //bool forcexdpi = prefs->getBool("/dialogs/import/forcexdpi");
        ImageResolution *ir = 0;
        double xscale = 1;
        double yscale = 1;


        if (!ir && !forcexdpi) {
            ir = new ImageResolution(uri);
        }
        if (ir && ir->ok()) {
            xscale = 960.0 / round(10.*ir->x());  // round-off to 0.1 dpi
            yscale = 960.0 / round(10.*ir->y());
            // prevent crash on image with too small dpi (bug 1479193)
            if (ir->x() <= .05)
                xscale = 960.0;
            if (ir->y() <= .05)
                yscale = 960.0;
        } else {
            xscale = 96.0 / defaultxdpi;
            yscale = 96.0 / defaultxdpi;
        }

        width *= xscale;
        height *= yscale;

        delete ir; // deleting NULL is safe

        // Create image node
        Inkscape::XML::Document *xml_doc = doc->getReprDoc();
        Inkscape::XML::Node *image_node = xml_doc->createElement("svg:image");
        sp_repr_set_svg_double(image_node, "width", width);
        sp_repr_set_svg_double(image_node, "height", height);

        // Added 11 Feb 2014 as we now honor "preserveAspectRatio" and this is
        // what Inkscaper's expect.
        image_node->setAttribute("preserveAspectRatio", "none");

        if( scale.compare( "auto" ) != 0 ) {
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, "image-rendering", scale.c_str());
            sp_repr_css_set(image_node, css, "style");
            sp_repr_css_attr_unref( css );
        }

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
            // std::cerr << "Viewbox not set, setting" << std::endl;
            doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDisplayUnit()), doc->getHeight().value(doc->getDisplayUnit())));
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

                  "<param name='link' type='optiongroup' appearance='full' _gui-text='" N_("Image Import Type:") "' _gui-description='" N_("Embed results in stand-alone, larger SVG files. Link references a file outside this SVG document and all files must be moved together.") "' >\n"
                    "<_option value='embed' >" N_("Embed") "</_option>\n"
                    "<_option value='link' >" N_("Link") "</_option>\n"
                  "</param>\n"

                  "<param name='dpi' type='optiongroup' appearance='full' _gui-text='" N_("Image DPI:") "' _gui-description='" N_("Take information from file or use default bitmap import resolution as defined in the preferences.") "' >\n"
                    "<_option value='from_file' >" N_("From file") "</_option>\n"
                    "<_option value='from_default' >" N_("Default import resolution") "</_option>\n"
                  "</param>\n"

                  "<param name='scale' type='optiongroup' appearance='full' _gui-text='" N_("Image Rendering Mode:") "' _gui-description='" N_("When an image is upscaled, apply smoothing or keep blocky (pixelated). (Will not work in all browsers.)") "' >\n"
                    "<_option value='auto' >" N_("None (auto)") "</_option>\n"
                    "<_option value='optimizeQuality' >" N_("Smooth (optimizeQuality)") "</_option>\n"
                    "<_option value='optimizeSpeed' >" N_("Blocky (optimizeSpeed)") "</_option>\n"
                  "</param>\n"

                  "<param name=\"do_not_ask\" _gui-description='" N_("Hide the dialog next time and always apply the same actions.") "' _gui-text=\"" N_("Don't ask again") "\" type=\"boolean\" >false</param>\n"
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

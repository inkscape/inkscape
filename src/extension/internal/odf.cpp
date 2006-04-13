/**
 * OpenDocument <drawing> input and output
 *
 * This is an an entry in the extensions mechanism to begin to enable
 * the inputting and outputting of OpenDocument Format (ODF) files from
 * within Inkscape.  Although the initial implementations will be very lossy
 * do to the differences in the models of SVG and ODF, they will hopefully
 * improve greatly with time.
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "odf.h"
#include "clear-n_.h"
#include "inkscape.h"
#include "sp-path.h"
#include <style.h>
#include "display/curve.h"
#include "libnr/n-art-bpath.h"
#include "extension/system.h"

#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "svg/svg.h"
#include "text-editing.h"

#include <vector>

#include "dom/dom.h"
#include "dom/util/ziptool.h"
#include "dom/io/domstream.h"
#include "dom/io/bufferstream.h"


//# Shorthand notation
typedef org::w3c::dom::DOMString DOMString;
typedef org::w3c::dom::io::OutputStreamWriter OutputStreamWriter;
typedef org::w3c::dom::io::BufferOutputStream BufferOutputStream;




namespace Inkscape
{
namespace Extension
{
namespace Internal
{





//########################################################################
//# O U T P U T
//########################################################################




/**
 * This function searches the Repr tree recursively from the given node,
 * and adds refs to all nodes with the given name, to the result vector
 */
void
OdfOutput::preprocess(ZipFile &zf, Inkscape::XML::Node *node)
{

    std::string nodeName = node->name();

    if (nodeName == "image")
        {
        char *hrefs = (char *)node->attribute("xlink:href");
        if (hrefs)
            {
            std::string oldName = hrefs;
            if (imageTable.find(oldName) == imageTable.end())
                {
                char buf[16];
                snprintf(buf, 15, "Pictures/image%d", imageTable.size());
                std::string newName = buf;
                imageTable[oldName] = newName;
                std::string comment = "old name was: ";
                comment.append(oldName);
                ZipEntry *ze = zf.addFile(oldName, comment);
                if (ze)
                    {
                    ze->setFileName(newName);
                    }
                else
                    {
                    g_warning("Could not load image file '%s'", oldName.c_str());
                    }
                }
            }
        }

#if 0

    //Look for style values in the svg element
    Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attr =
        node->attributeList();
    for ( ; attr ; ++attr)
        {
        std::string attrName  = (const char *)attr->key;
        std::string attrValue = (const char *)attr->value;
        StyleInfo si(attrName, attrValue);
        /*
        if (styleTable.find(styleValue) != styleTable.end())
            {
            g_message("duplicate style");
            }
        else
            {
            char buf[16];
            snprintf(buf, 15, "style%d", styleIndex++);
            std::string styleName  = buf;
            //Map from value-->name .   Looks backwards, i know
            styleTable[styleValue] = styleName;
            g_message("mapping '%s' to '%s'",
                styleValue.c_str(), styleName.c_str());
            }
        */
        }

#endif

    for (Inkscape::XML::Node *child = node->firstChild() ;
            child ; child = child->next())
        preprocess(zf, child);
}


/**
 * This function searches the Repr tree recursively from the given node,
 * and adds refs to all nodes with the given name, to the result vector
 */
void
OdfOutput::preprocess(ZipFile &zf, SPDocument *doc)
{
    styleTable.clear();
    preprocess(zf, doc->rroot);


}

bool OdfOutput::writeManifest(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">\n");
    outs.printf("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n");
    outs.printf("    <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.graphics\" manifest:full-path=\"/\"/>\n");
    outs.printf("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n");
    outs.printf("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"meta.xml\"/>\n");
    outs.printf("    <!--List our images here-->\n");
    std::map<std::string, std::string>::iterator iter;
    for (iter = imageTable.begin() ; iter!=imageTable.end() ; iter++)
        {
        std::string oldName = iter->first;
        std::string newName = iter->second;
        if (oldName.size() < 5)
            {
            g_warning("image file name too short:%s", oldName.c_str());
            return false;
            }
        std::string ext = oldName.substr(oldName.size() - 4);
        outs.printf("    <manifest:file-entry manifest:media-type=\"");
        if (ext == ".gif")
            outs.printf("image/gif");
        else if (ext == ".png")
            outs.printf("image/png");
        else if (ext == ".jpg" || ext == ".jpeg")
            outs.printf("image/jpeg");
        outs.printf("\" manifest:full-path=\"");
        outs.printf((char *)newName.c_str());
        outs.printf("\"/>\n");
        }
    outs.printf("</manifest:manifest>\n");

    outs.close();

    //Make our entry
    ZipEntry *ze = zf.newEntry("META-INF/manifest.xml", "ODF file manifest");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}





bool OdfOutput::writeStyle(Writer &outs)
{
    outs.printf("<office:automatic-styles>\n");
    outs.printf("<style:style style:name=\"dp1\" style:family=\"drawing-page\"/>\n");
    outs.printf("<style:style style:name=\"grx1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
    outs.printf("  <style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\" draw:textarea-horizontal-align=\"center\" draw:textarea-vertical-align=\"middle\" draw:color-mode=\"standard\" draw:luminance=\"0%\" draw:contrast=\"0%\" draw:gamma=\"100%\" draw:red=\"0%\" draw:green=\"0%\" draw:blue=\"0%\" fo:clip=\"rect(0cm 0cm 0cm 0cm)\" draw:image-opacity=\"100%\" style:mirror=\"none\"/>\n");
    outs.printf("</style:style>\n");
    outs.printf("<style:style style:name=\"P1\" style:family=\"paragraph\">\n");
    outs.printf("  <style:paragraph-properties fo:text-align=\"center\"/>\n");
    outs.printf("</style:style>\n");

    //##  Dump our style table
    /*
    std::map<std::string, std::string>::iterator iter;
    for (iter = styleTable.begin() ; iter != styleTable.end() ; iter++)
        {
        outs.printf("<style:style style:name=\"%s\"", iter->second);
        outs.printf(" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
        outs.printf("  <style:graphic-properties");
        outs.printf(" draw:fill=\"" + s.getFill() + "\"");
        if (!s.getFill().equals("none"))
            outs.printf(" draw:fill-color=\"" + s.getFillColor() + "\"");
        outs.printf(" draw:stroke=\"" + s.getStroke() + "\"");
        if (!s.getStroke().equals("none"))
            {
            outs.printf(" svg:stroke-width=\"" + s.getStrokeWidth() + "\"");
            outs.printf(" svg:stroke-color=\"" + s.getStrokeColor() + "\"");
            }
        outs.printf("/>\n");
        outs.printf("</style:style>\n");
        }
    */
    outs.printf("</office:automatic-styles>\n");
    outs.printf("\n");

    return true;
}




bool OdfOutput::writeTree(Writer &outs, Inkscape::XML::Node *node)
{
    //# Get the SPItem, if applicable
    SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(node);
    if (!reprobj)
        return true;
    if (!SP_IS_ITEM(reprobj))
        {
        return true;
        }
    SPItem *item = SP_ITEM(reprobj);

    //# Do our stuff
    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(item))
        {
        curve = sp_shape_get_curve(SP_SHAPE(item));
        }
    else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))
        {
        curve = te_get_layout(item)->convertToCurves();
        }

    if (!curve)
        return false;

    //Inkscape::XML::Node *repr = sp_repr_new("svg:path");
    /* Transformation */
    //repr->setAttribute("transform", SP_OBJECT_REPR(item)->attribute("transform"));

    /* Rotation center */
    //sp_repr_set_attr(repr, "inkscape:transform-center-x", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-x"));
    //sp_repr_set_attr(repr, "inkscape:transform-center-y", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-y"));

    /* Definition */
    gchar *def_str = sp_svg_write_path(curve->bpath);

    g_free(def_str);
    sp_curve_unref(curve);


    //# Iterate through the children
    for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
        {
        if (!writeTree(outs, child))
            return false;
        }
    return true;
}




bool OdfOutput::writeContent(ZipFile &zf, Inkscape::XML::Node *node)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("<office:document-content\n");
    outs.printf("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.printf("    xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n");
    outs.printf("    xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n");
    outs.printf("    xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n");
    outs.printf("    xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n");
    outs.printf("    xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n");
    outs.printf("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.printf("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.printf("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.printf("    xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n");
    outs.printf("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.printf("    xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n");
    outs.printf("    xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n");
    outs.printf("    xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n");
    outs.printf("    xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n");
    outs.printf("    xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n");
    outs.printf("    xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n");
    outs.printf("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.printf("    xmlns:ooow=\"http://openoffice.org/2004/writer\"\n");
    outs.printf("    xmlns:oooc=\"http://openoffice.org/2004/calc\"\n");
    outs.printf("    xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n");
    outs.printf("    xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n");
    outs.printf("    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    outs.printf("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    outs.printf("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.printf("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.printf("    office:version=\"1.0\">\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:scripts/>\n");

    //if (!writeStyle(outs))
    //    return false;

    //if (!writeTree(outs, node))
    //    return false;

    //Make our entry
    ZipEntry *ze = zf.newEntry("content.xml", "ODF master content file");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}




/**
 * Descends into the SVG tree, mapping things to ODF when appropriate
 */
void
OdfOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *uri)
{
    ZipFile zf;
    preprocess(zf, doc);
    if (!writeManifest(zf))
        {
        g_warning("Failed to write manifest");
        return;
        }
    if (!writeContent(zf, doc->rroot))
        {
        g_warning("Failed to write content");
        return;
        }
    if (!zf.writeFile(uri))
        {
        return;
        }
}


/**
 * This is the definition of PovRay output.  This function just
 * calls the extension system with the memory allocated XML that
 * describes the data.
*/
void
OdfOutput::init()
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
            "<name>" N_("OpenDocument Drawing Output") "</name>\n"
            "<id>org.inkscape.output.odf</id>\n"
            "<output>\n"
                "<extension>.odg</extension>\n"
                "<mimetype>text/x-povray-script</mimetype>\n"
                "<filetypename>" N_("OpenDocument drawing (*.odg)") "</filetypename>\n"
                "<filetypetooltip>" N_("OpenDocument drawing file") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>",
        new OdfOutput());
}

/**
 * Make sure that we are in the database
 */
bool
OdfOutput::check (Inkscape::Extension::Extension *module)
{
    /* We don't need a Key
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_POV))
        return FALSE;
    */

    return TRUE;
}

//########################################################################
//# I N P U T
//########################################################################




}  //namespace Internal
}  //namespace Extension
}  //namespace Inkscape


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

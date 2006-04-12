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

#include <vector>

#include "dom/dom.h"
#include "dom/util/ziptool.h"


//# Shorthand notation
typedef org::w3c::dom::DOMString DOMString;


namespace Inkscape
{
namespace Extension
{
namespace Internal
{




class ImageInfo
{
public:

    ImageInfo(const DOMString &nameArg,
              const DOMString &newNameArg,
              const std::vector<unsigned char> &bufArg)
        {
        name    = nameArg;
        newName = newNameArg;
        buf     = bufArg;
        }

    virtual ~ImageInfo()
        {}

    DOMString getName()
        {
        return name;
        }

    DOMString getNewName()
        {
        return newName;
        }


    std::vector<unsigned char> getBuf()
        {
        return buf;
        }

    DOMString name;
    DOMString newName;
    std::vector<unsigned char>buf;

};


class StyleInfo
{
public:

    StyleInfo(const DOMString &nameArg, const DOMString &styleArg)
        {
        name   = nameArg;
        style  = styleArg;
        fill   = "none";
        stroke = "none";
        }

    virtual ~StyleInfo()
        {}

    DOMString getName()
        {
        return name;
        }

    DOMString getCssStyle()
        {
        return cssStyle;
        }

    DOMString getStroke()
        {
        return stroke;
        }

    DOMString getStrokeColor()
        {
        return strokeColor;
        }

    DOMString getStrokeWidth()
        {
        return strokeWidth;
        }


    DOMString getFill()
        {
        return fill;
        }

    DOMString getFillColor()
        {
        return fillColor;
        }

    DOMString name;
    DOMString style;
    DOMString cssStyle;
    DOMString stroke;
    DOMString strokeColor;
    DOMString strokeWidth;
    DOMString fill;
    DOMString fillColor;

};


//########################################################################
//# O U T P U T
//########################################################################

void OdfOutput::po(char *str)
{
    if (str)
        while (*str)
            outs.put(*str++);
}






/**
 * This function searches the Repr tree recursively from the given node,
 * and adds refs to all nodes with the given name, to the result vector
 */
static void
findElementsByTagName(std::vector<Inkscape::XML::Node *> &results,
                      Inkscape::XML::Node *node,
                      char const *name)
{
    if ( !name || strcmp(node->name(), name) == 0 )
        {
        results.push_back(node);
        }

    for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
        findElementsByTagName( results, child, name );

}

/**
 * This function searches the Repr tree recursively from the given node,
 * and adds refs to all nodes with the given name, to the result vector
 */
void
OdfOutput::preprocess(Inkscape::XML::Node *node)
{
    //Look for style values in the svg element
    Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attr =
        node->attributeList();
    for ( ; attr ; ++attr)
        {
        DOMString attrName  = (const char *)attr->key;
        DOMString attrValue = (const char *)attr->value;
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



    for (Inkscape::XML::Node *child = node->firstChild() ;
            child ; child = child->next())
        preprocess(child);
}


/**
 * This function searches the Repr tree recursively from the given node,
 * and adds refs to all nodes with the given name, to the result vector
 */
void
OdfOutput::preprocess(SPDocument *doc)
{
    styleTable.clear();
    styleIndex = 0;
    preprocess(doc->rroot);

    outs.clear();

    po("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    po("<office:document-content\n");
    po("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    po("    xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n");
    po("    xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n");
    po("    xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n");
    po("    xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n");
    po("    xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n");
    po("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    po("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    po("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    po("    xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n");
    po("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    po("    xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n");
    po("    xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n");
    po("    xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n");
    po("    xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n");
    po("    xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n");
    po("    xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n");
    po("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    po("    xmlns:ooow=\"http://openoffice.org/2004/writer\"\n");
    po("    xmlns:oooc=\"http://openoffice.org/2004/calc\"\n");
    po("    xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n");
    po("    xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n");
    po("    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    po("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    po("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    po("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    po("    office:version=\"1.0\">\n");
    po("\n");
    po("\n");
    po("<office:scripts/>\n");
    po("<office:automatic-styles>\n");
    po("<style:style style:name=\"dp1\" style:family=\"drawing-page\"/>\n");
    po("<style:style style:name=\"grx1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
    po("  <style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\" draw:textarea-horizontal-align=\"center\" draw:textarea-vertical-align=\"middle\" draw:color-mode=\"standard\" draw:luminance=\"0%\" draw:contrast=\"0%\" draw:gamma=\"100%\" draw:red=\"0%\" draw:green=\"0%\" draw:blue=\"0%\" fo:clip=\"rect(0cm 0cm 0cm 0cm)\" draw:image-opacity=\"100%\" style:mirror=\"none\"/>\n");
    po("</style:style>\n");
    po("<style:style style:name=\"P1\" style:family=\"paragraph\">\n");
    po("  <style:paragraph-properties fo:text-align=\"center\"/>\n");
    po("</style:style>\n");

    //##  Dump our style table
    /*
    std::map<std::string, std::string>::iterator iter;
    for (iter = styleTable.begin() ; iter != styleTable.end() ; iter++)
        {
        po("<style:style style:name=\"%s\"", iter->second);
        po(" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
        po("  <style:graphic-properties");
        po(" draw:fill=\"" + s.getFill() + "\"");
        if (!s.getFill().equals("none"))
            po(" draw:fill-color=\"" + s.getFillColor() + "\"");
        po(" draw:stroke=\"" + s.getStroke() + "\"");
        if (!s.getStroke().equals("none"))
            {
            po(" svg:stroke-width=\"" + s.getStrokeWidth() + "\"");
            po(" svg:stroke-color=\"" + s.getStrokeColor() + "\"");
            }
        po("/>\n");
        po("</style:style>\n");
        }
    */
    po("</office:automatic-styles>\n");
    po("\n");

}


bool OdfOutput::writeTree(Inkscape::XML::Node *node)
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


    //# Iterate through the children
    for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
        {
        if (!writeTree(child))
            return false;
        }
    return true;
}



/**
 * Descends into the SVG tree, mapping things to ODF when appropriate
 */
void
OdfOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *uri)
{
    //# Preprocess the style entries.  ODF does not put styles
    //# directly on elements.  Rather, it uses class IDs.
    preprocess(doc);
    ZipFile zipFile;
    zipFile.writeFile(uri);
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

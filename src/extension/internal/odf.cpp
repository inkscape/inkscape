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

//# System includes
#include <stdio.h>
#include <time.h>
#include <vector>


//# Inkscape includes
#include "clear-n_.h"
#include "inkscape.h"
#include <style.h>
#include "display/curve.h"
#include "libnr/n-art-bpath.h"
#include "extension/system.h"

#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "sp-image.h"
#include "sp-path.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "svg/svg.h"
#include "text-editing.h"


//# DOM-specific includes
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


//#define pxToCm  0.0275
#define pxToCm  0.04
#define piToRad 0.0174532925
#define docHeightCm 22.86


//########################################################################
//# O U T P U T
//########################################################################

static std::string getAttribute( Inkscape::XML::Node *node, char *attrName)
{
    std::string val;
    char *valstr = (char *)node->attribute(attrName);
    if (valstr)
        val = (const char *)valstr;
    return val;
}


static std::string getExtension(const std::string &fname)
{
    std::string ext;

    unsigned int pos = fname.rfind('.');
    if (pos == fname.npos)
        {
        ext = "";
        }
    else
        {
        ext = fname.substr(pos);
        }
    return ext;
}


static std::string formatTransform(NR::Matrix &tf)
{
    std::string str;
    if (!tf.test_identity())
        {
        char buf[128];
        snprintf(buf, 127, "matrix(%.3f %.3f %.3f %.3f %.3f %.3f)",
                tf[0], tf[1], tf[2], tf[3], tf[4], tf[5]);
        str = buf;
        }
    return str;
}


/**
 * Method descends into the repr tree, converting image and style info
 * into forms compatible in ODF.
 */
void
OdfOutput::preprocess(ZipFile &zf, Inkscape::XML::Node *node)
{

    std::string nodeName = node->name();
    std::string id       = getAttribute(node, "id");

    if (nodeName == "image" || nodeName == "svg:image")
        {
        //g_message("image");
        std::string href = getAttribute(node, "xlink:href");
        if (href.size() > 0)
            {
            std::string oldName = href;
            std::string ext = getExtension(oldName);
            if (ext == ".jpeg")
                ext = ".jpg";
            if (imageTable.find(oldName) == imageTable.end())
                {
                char buf[64];
                snprintf(buf, 63, "Pictures/image%d%s",
                    imageTable.size(), ext.c_str());
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



    SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(node);
    if (!reprobj)
        return;
    if (!SP_IS_ITEM(reprobj))
        {
        return;
        }
    SPItem *item = SP_ITEM(reprobj);
    SPStyle *style = SP_OBJECT_STYLE(item);
    if (style && id.size()>0)
        {
        StyleInfo si;
        if (style->fill.type == SP_PAINT_TYPE_COLOR)
            {
            guint32 fillCol =
                sp_color_get_rgba32_ualpha(&style->fill.value.color, 0);
            char buf[16];
            int r = (fillCol >> 24) & 0xff;
            int g = (fillCol >> 16) & 0xff;
            int b = (fillCol >>  8) & 0xff;
            //g_message("## %s %lx", id.c_str(), (unsigned int)fillCol);
            snprintf(buf, 15, "#%02x%02x%02x", r, g, b);
            si.fillColor = buf;
            si.fill      = "solid";
            double opacityPercent = 100.0;
            snprintf(buf, 15, "%.2f%%", opacityPercent);
            si.fillOpacity = buf;
            }
        if (style->stroke.type == SP_PAINT_TYPE_COLOR)
            {
            guint32 strokeCol =
                sp_color_get_rgba32_ualpha(&style->stroke.value.color, 0);
            char buf[16];
            int r = (strokeCol >> 24) & 0xff;
            int g = (strokeCol >> 16) & 0xff;
            int b = (strokeCol >>  8) & 0xff;
            snprintf(buf, 15, "#%02x%02x%02x", r, g, b);
            si.strokeColor = buf;
            snprintf(buf, 15, "%.2fpt", style->stroke_width.value);
            si.strokeWidth = buf;
            si.stroke      = "solid";
            double opacityPercent = 100.0;
            snprintf(buf, 15, "%.2f%%", opacityPercent);
            si.strokeOpacity = buf;
            }

        //Look for existing identical style;
        bool styleMatch = false;
        std::map<std::string, StyleInfo>::iterator iter;
        for (iter=styleTable.begin() ; iter!=styleTable.end() ; iter++)
            {
            if (si.equals(iter->second))
                {
                //map to existing styleTable entry
                std::string styleName = iter->first;
                //g_message("found duplicate style:%s", styleName.c_str());
                styleLookupTable[id] = styleName;
                styleMatch = true;
                break;
                }
            }
        //None found, make a new pair or entries
        if (!styleMatch)
            {
            char buf[16];
            snprintf(buf, 15, "style%d", styleTable.size());
            std::string styleName = buf;
            styleTable[styleName] = si;
            styleLookupTable[id] = styleName;
            }
        }

    /*
    //Look for style values in the svg element
    Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attr =
        node->attributeList();
    for ( ; attr ; ++attr)
        {
        if (!attr->key || !attr->value)
            {
            g_warning("null key or value in attribute");
            continue;
            }
        //g_message("key:%s value:%s", g_quark_to_string(attr->key),
        //                             g_quark_to_string(attr->value)  );

        std::string attrName  = (const char *)g_quark_to_string(attr->key);
        std::string attrValue = (const char *)attr->value;
        //g_message("tag:'%s'    key:'%s'    value:'%s'",
        //    nodeName.c_str(), attrName.c_str(), attrValue.c_str()  );
        if (attrName == "style")
            {
            StyleInfo si(attrName, attrValue);
            if (styleTable.find(attrValue) != styleTable.end())
                {
                //g_message("duplicate style");
                }
            else
                {
                char buf[16];
                snprintf(buf, 15, "style%d", styleTable.size());
                std::string attrName  = buf;
                //Map from value-->name .   Looks backwards, i know
                styleTable[attrValue] = si;
                //g_message("mapping '%s' to '%s'",
                //    attrValue.c_str(), attrName.c_str());
                }
            }
        }
    */


    for (Inkscape::XML::Node *child = node->firstChild() ;
            child ; child = child->next())
        preprocess(zf, child);
}



bool OdfOutput::writeManifest(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  file:  manifest.xml\n");
    outs.printf("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.printf("  http://www.inkscape.org\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
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

        std::string ext = getExtension(oldName);
        if (ext == ".jpeg")
            ext = ".jpg";
        outs.printf("    <manifest:file-entry manifest:media-type=\"");
        if (ext == ".gif")
            outs.printf("image/gif");
        else if (ext == ".png")
            outs.printf("image/png");
        else if (ext == ".jpg")
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


bool OdfOutput::writeMeta(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  file:  meta.xml\n");
    outs.printf("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.printf("  http://www.inkscape.org\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:document-meta\n");
    outs.printf("xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.printf("xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.printf("xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.printf("xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.printf("xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.printf("xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.printf("xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.printf("xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.printf("office:version=\"1.0\">\n");
    outs.printf("<office:meta>\n");
    outs.printf("    <meta:generator>Inkscape.org - 0.44</meta:generator>\n");
    outs.printf("    <meta:initial-creator>clark kent</meta:initial-creator>\n");
    outs.printf("    <meta:creation-date>2006-04-13T17:12:29</meta:creation-date>\n");
    outs.printf("    <dc:creator>clark kent</dc:creator>\n");
    outs.printf("    <dc:date>2006-04-13T17:13:20</dc:date>\n");
    outs.printf("    <dc:language>en-US</dc:language>\n");
    outs.printf("    <meta:editing-cycles>2</meta:editing-cycles>\n");
    outs.printf("    <meta:editing-duration>PT56S</meta:editing-duration>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 1\"/>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 2\"/>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 3\"/>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 4\"/>\n");
    outs.printf("    <meta:document-statistic meta:object-count=\"2\"/>\n");
    outs.printf("</office:meta>\n");
    outs.printf("</office:document-meta>\n");
    outs.printf("\n");
    outs.printf("\n");


    outs.close();

    //Make our entry
    ZipEntry *ze = zf.newEntry("meta.xml", "ODF info file");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}


bool OdfOutput::writeStyle(Writer &outs)
{
    outs.printf("<office:automatic-styles>\n");
    outs.printf("<style:style style:name=\"dp1\" style:family=\"drawing-page\"/>\n");
    outs.printf("<style:style style:name=\"gr1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
    outs.printf("  <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"\n");
    outs.printf("       draw:textarea-horizontal-align=\"center\"\n");
    outs.printf("       draw:textarea-vertical-align=\"middle\" draw:color-mode=\"standard\"\n");
    outs.printf("       draw:luminance=\"0%\" draw:contrast=\"0%\" draw:gamma=\"100%\" draw:red=\"0%\"\n");
    outs.printf("       draw:green=\"0%\" draw:blue=\"0%\" fo:clip=\"rect(0cm 0cm 0cm 0cm)\"\n");
    outs.printf("       draw:image-opacity=\"100%\" style:mirror=\"none\"/>\n");
    outs.printf("</style:style>\n");
    outs.printf("<style:style style:name=\"P1\" style:family=\"paragraph\">\n");
    outs.printf("  <style:paragraph-properties fo:text-align=\"center\"/>\n");
    outs.printf("</style:style>\n");

    //##  Dump our style table
    std::map<std::string, StyleInfo>::iterator iter;
    for (iter = styleTable.begin() ; iter != styleTable.end() ; iter++)
        {
        outs.printf("<style:style style:name=\"%s\"", iter->first.c_str());
        StyleInfo s(iter->second);
        outs.printf(" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
        outs.printf("  <style:graphic-properties");
        outs.printf(" draw:fill=\"%s\" ", s.fill.c_str());
        if (s.fill != "none")
            {
            outs.printf(" draw:fill-color=\"%s\" ", s.fillColor.c_str());
            outs.printf(" draw:fill-opacity=\"%s\" ", s.fillOpacity.c_str());
            }
        outs.printf(" draw:stroke=\"%s\" ", s.stroke.c_str());
        if (s.stroke != "none")
            {
            outs.printf(" svg:stroke-width=\"%s\" ", s.strokeWidth.c_str());
            outs.printf(" svg:stroke-color=\"%s\" ", s.strokeColor.c_str());
            outs.printf(" svg:stroke-opacity=\"%s\" ", s.strokeOpacity.c_str());
            }
        outs.printf("/>\n");
        outs.printf("</style:style>\n");
        }

    outs.printf("</office:automatic-styles>\n");
    outs.printf("\n");

    return true;
}


static void
writePath(Writer &outs, NArtBpath const *bpath,
          NR::Matrix &tf, double xoff, double yoff)
{
    bool closed = false;
    NArtBpath *bp = (NArtBpath *)bpath;
    for (  ; bp->code != NR_END; bp++)
        {
        NR::Point const p1(bp->c(1) * tf);
	NR::Point const p2(bp->c(2) * tf);
	NR::Point const p3(bp->c(3) * tf);
	double x1 = (p1[NR::X] * pxToCm - xoff) * 1000.0;
	double y1 = (p1[NR::Y] * pxToCm - yoff) * 1000.0;
	double x2 = (p2[NR::X] * pxToCm - xoff) * 1000.0;
	double y2 = (p2[NR::Y] * pxToCm - yoff) * 1000.0;
	double x3 = (p3[NR::X] * pxToCm - xoff) * 1000.0;
	double y3 = (p3[NR::Y] * pxToCm - yoff) * 1000.0;

        switch (bp->code)
            {
            case NR_LINETO:
                outs.printf("L %.3f,%.3f ",  x3 , y3);
                break;

            case NR_CURVETO:
                outs.printf("C %.3f,%.3f %.3f,%.3f %.3f,%.3f ",
                              x1, y1, x2, y2, x3, y3);
                break;

            case NR_MOVETO_OPEN:
            case NR_MOVETO:
                if (closed)
                    outs.printf("z ");
                closed = ( bp->code == NR_MOVETO );
                outs.printf("M %.3f,%.3f ",  x3 , y3);
                break;

            default:
                break;

            }

        }

    if (closed)
        outs.printf("z");;

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


    std::string nodeName = node->name();
    std::string id       = getAttribute(node, "id");

    NR::Matrix tf  = sp_item_i2d_affine(item);
    NR::Rect bbox = sp_item_bbox_desktop(item);

    //Flip Y into document coordinates
    double doc_height = sp_document_height(SP_ACTIVE_DOCUMENT);
    NR::Matrix doc2dt_tf = NR::Matrix(NR::scale(1, -1));
    doc2dt_tf = doc2dt_tf * NR::Matrix(NR::translate(0, doc_height));
    //tf            = tf   * doc2dt_tf;
    //bbox          = bbox * doc2dt_tf;

    double x      = pxToCm * bbox.min()[NR::X];
    double y      = pxToCm * bbox.min()[NR::Y];
    double width  = pxToCm * ( bbox.max()[NR::X] - bbox.min()[NR::X] );
    double height = pxToCm * ( bbox.max()[NR::Y] - bbox.min()[NR::Y] );


    //# Do our stuff
    SPCurve *curve = NULL;

    //g_message("##### %s #####", nodeName.c_str());

    if (nodeName == "svg" || nodeName == "svg:svg")
        {
        //# Iterate through the children
        for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
            {
            if (!writeTree(outs, child))
                return false;
            }
        return true;
        }
    else if (nodeName == "g" || nodeName == "svg:g")
        {
        if (id.size() > 0)
            outs.printf("<draw:g id=\"%s\">", id.c_str());
        else
            outs.printf("<draw:g>\n");
        //# Iterate through the children
        for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
            {
            if (!writeTree(outs, child))
                return false;
            }
        outs.printf("</draw:g>\n");
        return true;
        }
    else if (nodeName == "image" || nodeName == "svg:image")
        {
        if (!SP_IS_IMAGE(item))
            {
            g_warning("<image> is not an SPImage.  Why?  ;-)");
            return false;
            }

        SPImage *img   = SP_IMAGE(item);
        double ix      = img->x.computed;
        double iy      = img->y.computed;
        double iwidth  = img->width.computed;
        double iheight = img->height.computed;

        NR::Rect ibbox(NR::Point(ix, iy), NR::Point(iwidth, iheight));
        ix      = pxToCm * ibbox.min()[NR::X];
        iy      = pxToCm * ibbox.min()[NR::Y];
        iwidth  = pxToCm * ( ibbox.max()[NR::X] - ibbox.min()[NR::X] );
        iheight = pxToCm * ( ibbox.max()[NR::Y] - ibbox.min()[NR::Y] );


        std::string itemTransformString = formatTransform(item->transform);

        std::string href = getAttribute(node, "xlink:href");
        std::map<std::string, std::string>::iterator iter = imageTable.find(href);
        if (iter == imageTable.end())
            {
            g_warning("image '%s' not in table", href.c_str());
            return false;
            }
        std::string newName = iter->second;


        outs.printf("<draw:frame ");
        if (id.size() > 0)
            outs.printf("id=\"%s\" ", id.c_str());
        outs.printf("draw:style-name=\"gr1\" draw:text-style-name=\"P1\" draw:layer=\"layout\" ");
        outs.printf("svg:x=\"%.3fcm\" svg:y=\"%.3fcm\" ",
                                  ix, iy);
        outs.printf("svg:width=\"%.3fcm\" svg:height=\"%.3fcm\" ",
                                  iwidth, iheight);
        if (itemTransformString.size() > 0)
            outs.printf("draw:transform=\"%s\" ", itemTransformString.c_str());

        outs.printf(">\n");
        outs.printf("    <draw:image xlink:href=\"%s\" xlink:type=\"simple\"\n",
                              newName.c_str());
        outs.printf("        xlink:show=\"embed\" xlink:actuate=\"onLoad\">\n");
        outs.printf("        <text:p/>\n");
        outs.printf("    </draw:image>\n");
        outs.printf("</draw:frame>\n");
        return true;
        }
    else if (SP_IS_SHAPE(item))
        {
        //g_message("### %s is a shape", nodeName.c_str());
        curve = sp_shape_get_curve(SP_SHAPE(item));
        }
    else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))
        {
        curve = te_get_layout(item)->convertToCurves();
        }

    if (curve)
        {
        //Inkscape::XML::Node *repr = sp_repr_new("svg:path");
        /* Transformation */
        //repr->setAttribute("transform", SP_OBJECT_REPR(item)->attribute("transform"));

        /* Rotation center */
        //sp_repr_set_attr(repr, "inkscape:transform-center-x", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-x"));
        //sp_repr_set_attr(repr, "inkscape:transform-center-y", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-y"));

        /* Definition */

        outs.printf("<draw:path ");
        if (id.size()>0)
            outs.printf("id=\"%s\" ", id.c_str());

        std::map<std::string, std::string>::iterator iter;
        iter = styleLookupTable.find(id);
        if (iter != styleLookupTable.end())
            {
            std::string styleName = iter->second;
            outs.printf("draw:style-name=\"%s\" ", styleName.c_str());
            }

        outs.printf("draw:layer=\"layout\" svg:x=\"%.3fcm\" svg:y=\"%.3fcm\" ",
                       x, y);
	outs.printf("svg:width=\"%.3fcm\" svg:height=\"%.3fcm\" ",
	               width, height);
	outs.printf("svg:viewBox=\"0.0 0.0 %.3f %.3f\"\n",
	               width * 1000.0, height * 1000.0);

	outs.printf("    svg:d=\"");
	writePath(outs, curve->bpath, tf, x, y);
	outs.printf("\"");

	outs.printf(">\n");
        outs.printf("</draw:path>\n");


        sp_curve_unref(curve);
        }

    return true;
}




bool OdfOutput::writeContent(ZipFile &zf, Inkscape::XML::Node *node)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  file:  content.xml\n");
    outs.printf("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.printf("  http://www.inkscape.org\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
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
    outs.printf("\n");
    outs.printf("\n");
    //AffineTransform trans = new AffineTransform();
    //trans.scale(12.0, 12.0);
    outs.printf("<!-- ######### CONVERSION FROM SVG STARTS ######## -->\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  S T Y L E S\n");
    outs.printf("  Style entries have been pulled from the svg style and\n");
    outs.printf("  representation attributes in the SVG tree.  The tree elements\n");
    outs.printf("  then refer to them by name, in the ODF manner\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");

    if (!writeStyle(outs))
        {
        g_warning("Failed to write styles");
        return false;
        }

    outs.printf("\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  D R A W I N G\n");
    outs.printf("  This section is the heart of SVG-ODF conversion.  We are\n");
    outs.printf("  starting with simple conversions, and will slowly evolve\n");
    outs.printf("  into a 'smarter' translation as time progresses.  Any help\n");
    outs.printf("  in improving .odg export is welcome.\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:body>\n");
    outs.printf("<office:drawing>\n");
    outs.printf("<draw:page draw:name=\"page1\" draw:style-name=\"dp1\"\n");
    outs.printf("        draw:master-page-name=\"Default\">\n");
    outs.printf("\n");
    outs.printf("<draw:g draw:transform=\"scale(1,-1)\">\n");
    outs.printf("\n");

    if (!writeTree(outs, node))
        {
        g_warning("Failed to convert SVG tree");
        return false;
        }

    outs.printf("\n");
    outs.printf("</draw:g>\n");
    outs.printf("\n");

    outs.printf("</draw:page>\n");
    outs.printf("</office:drawing>\n");

    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!-- ######### CONVERSION FROM SVG ENDS ######## -->\n");
    outs.printf("\n");
    outs.printf("\n");

    outs.printf("</office:body>\n");
    outs.printf("</office:document-content>\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  E N D    O F    F I L E\n");
    outs.printf("  Have a nice day  - ishmal\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");



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
    styleTable.clear();
    styleLookupTable.clear();
    imageTable.clear();
    preprocess(zf, doc->rroot);

    if (!writeManifest(zf))
        {
        g_warning("Failed to write manifest");
        return;
        }

    if (!writeMeta(zf))
        {
        g_warning("Failed to write metafile");
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



//#######################
//# L A T E R  !!!  :-)
//#######################













}  //namespace Internal
}  //namespace Extension
}  //namespace Inkscape


//########################################################################
//# E N D    O F    F I L E
//########################################################################

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

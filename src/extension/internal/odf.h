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

#ifndef EXTENSION_INTERNAL_ODG_OUT_H
#define EXTENSION_INTERNAL_ODG_OUT_H

#include <dom/dom.h>
#include <dom/io/stringstream.h>

#include <glib.h>
#include "extension/implementation/implementation.h"


#include <xml/repr.h>

#include <string>
#include <map>

#include <dom/util/ziptool.h>
#include <dom/io/domstream.h>

typedef org::w3c::dom::io::Writer Writer;

namespace Inkscape
{
namespace Extension
{
namespace Internal
{



class StyleInfo
{
public:

    StyleInfo(const std::string &nameArg, const std::string &styleArg)
        {
        name   = nameArg;
        style  = styleArg;
        fill   = "none";
        stroke = "none";
        }

    virtual ~StyleInfo()
        {}

    std::string getName()
        {
        return name;
        }

    std::string getCssStyle()
        {
        return cssStyle;
        }

    std::string getStroke()
        {
        return stroke;
        }

    std::string getStrokeColor()
        {
        return strokeColor;
        }

    std::string getStrokeWidth()
        {
        return strokeWidth;
        }


    std::string getFill()
        {
        return fill;
        }

    std::string getFillColor()
        {
        return fillColor;
        }

    std::string name;
    std::string style;
    std::string cssStyle;
    std::string stroke;
    std::string strokeColor;
    std::string strokeWidth;
    std::string fill;
    std::string fillColor;

};



class OdfOutput : public Inkscape::Extension::Implementation::Implementation
{

public:

    bool check (Inkscape::Extension::Extension * module);

    void save  (Inkscape::Extension::Output *mod,
	        SPDocument *doc,
	        const gchar *uri);

    static void   init  (void);

private:

    std::map<std::string, StyleInfo> styleTable;

    //for renaming image file names
    std::map<std::string, std::string> imageTable;

    void preprocess(ZipFile &zf, SPDocument *doc);
    void preprocess(ZipFile &zf, Inkscape::XML::Node *node);

    bool writeManifest(ZipFile &zf);

    bool writeStyle(Writer &outs);

    bool writeTree(Writer &outs, Inkscape::XML::Node *node);

    bool writeContent(ZipFile &zf, Inkscape::XML::Node *node);

};




}  //namespace Internal
}  //namespace Extension
}  //namespace Inkscape



#endif /* EXTENSION_INTERNAL_ODG_OUT_H */


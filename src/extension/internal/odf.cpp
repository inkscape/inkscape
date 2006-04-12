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

#include "io/sys.h"

#include "dom/util/ziptool.h"

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
 * Descends into the SVG tree, mapping things to ODF when appropriate
 */
void
OdfOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *uri)
{
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
                "<filetypename>" N_("OpenDocument drawing (*.odg)(placeholder)") "</filetypename>\n"
                "<filetypetooltip>" N_("OpenDocument drawing file") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>",
        new OdfOutput());
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

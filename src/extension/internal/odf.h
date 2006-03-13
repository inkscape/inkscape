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

#include <glib.h>
#include "extension/implementation/implementation.h"

namespace Inkscape
{
namespace Extension
{
namespace Internal
{


class OdfOutput : public Inkscape::Extension::Implementation::Implementation
{

    public:

	bool check (Inkscape::Extension::Extension * module);

	void          save  (Inkscape::Extension::Output *mod,
	                     SPDocument *doc,
	                     const gchar *uri);

	static void   init  (void);


};




}  //namespace Internal
}  //namespace Extension
}  //namespace Inkscape



#endif /* EXTENSION_INTERNAL_ODG_OUT_H */


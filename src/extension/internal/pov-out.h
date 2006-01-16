/*
 * A simple utility for exporting Inkscape svg Shapes as PovRay bezier
 * prisms.  Note that this is output-only, and would thus seem to be
 * better placed as an 'export' rather than 'output'.  However, Export
 * handles all or partial documents, while this outputs ALL shapes in
 * the current SVG document.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef EXTENSION_INTERNAL_POV_OUT_H
#define EXTENSION_INTERNAL_POV_OUT_H

#include <glib.h>
#include "extension/implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class PovOutput : public Inkscape::Extension::Implementation::Implementation
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



#endif /* EXTENSION_INTERNAL_POV_OUT_H */


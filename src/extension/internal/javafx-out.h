/*
 * A simple utility for exporting Inkscape svg Shapes as PovRay bezier
 * prisms.  Note that this is output-only, and would thus seem to be
 * better placed as an 'export' rather than 'output'.  However, Export
 * handles all or partial documents, while this outputs ALL shapes in
 * the current SVG document.
 *
 * Authors:
 *   Bob Jamison <ishmal@inkscape.org>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef EXTENSION_INTERNAL_JAVAFX_OUT_H
#define EXTENSION_INTERNAL_JAVAFX_OUT_H

#include <glib.h>
#include "extension/implementation/implementation.h"

namespace Inkscape
{
namespace Extension
{
namespace Internal
{



/**
 * Output the current svg document in JavaFX format.
 * 
 * For information, @see:  
 * https://openjfx.dev.java.net/
 */ 
class JavaFXOutput : public Inkscape::Extension::Implementation::Implementation
{


public:

    /**
     * Our internal String definition
     */
    typedef Glib::ustring String;


    /**
     * Check whether we can actually output using this module
     */
	virtual bool check (Inkscape::Extension::Extension * module);

    /**
     * API call to perform the output to a file
     */
	virtual void save (Inkscape::Extension::Output *mod,
	           SPDocument *doc, const gchar *uri);

    /**
     * Inkscape runtime startup call.
     */
	static void init(void);
	
    /**
     * Reset variables to initial state
     */
	void reset();
	
private:

    
    //For formatted output
	String outbuf;
	

	/**
	 * Format text to our output buffer
	 */
	void out(const char *fmt, ...) G_GNUC_PRINTF(2,3);

	//Output the parts of the file

    /**
     * Output the file header
     */
	bool doHeader(const String &name);

    /**
     * Output the SVG document's curve data as POV curves
     */
	bool doCurves(SPDocument *doc, const String &name);

    /**
     * Output the file footer
     */
	bool doTail(const String &name);



    /**
     * Actual method to save document
     */
	bool saveDocument(SPDocument *doc, const gchar *uri);

    //For statistics
    int nrNodes;
    int nrSegments;
    int nrShapes;

};




}  // namespace Internal
}  // namespace Extension
}  // namespace Inkscape



#endif /* EXTENSION_INTERNAL_POV_OUT_H */


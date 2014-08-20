/*
 * A simple utility for exporting an Inkscape svg image as a JavaFX
 * scene tree.
 *
 * Authors:
 *   Bob Jamison <ishmal@inkscape.org>
 *   Silveira Neto <silveiraneto@gmail.com>
 *   Jim Clarke <Jim.Clarke@sun.com>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef EXTENSION_INTERNAL_JAVAFX_OUT_H
#define EXTENSION_INTERNAL_JAVAFX_OUT_H

#include <glib.h>
#include "extension/implementation/implementation.h"
#include <document.h>
#include <sp-gradient.h>

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
    JavaFXOutput (void);
    
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
    virtual void save(Inkscape::Extension::Output *mod,
                      SPDocument *doc, gchar const *filename);

    /**
     * Inkscape runtime startup call.
     */
	static void init(void);
	
    /**
     * Reset variables to initial state
     */
	void reset();
	
private:

    //output class name
    String name;

    //For formatted output
	String outbuf;   //main output buffer
	String foutbuf;  //header function buffer


	/**
	 * Format text to our output buffer
	 */
	void out(const char *fmt, ...) G_GNUC_PRINTF(2,3);

	/**
	 * Format text to our function output buffer
	 */
	void fout(const char *fmt, ...) G_GNUC_PRINTF(2,3);

	//Output the parts of the file

    /**
     * Output the file header
     */
	bool doHeader();

    /**
     *  Output gradient information to the buffer
     */
    bool doGradient(SPGradient *grad, const String &id);

    /**
     *  Output an element's style attribute
     */
    bool doStyle(SPStyle *style);

    /**
     * Output the SVG document's curve data as JavaFX geometry types
     */
    bool doCurve(SPItem *item, const String &id);
    bool doTreeRecursive(SPDocument *doc, SPObject *obj);
    bool doTree(SPDocument *doc);

    bool doBody(SPDocument *doc, SPObject *obj);

    /**
     * Output the file footer
     */
	bool doTail();



    /**
     * Actual method to save document
     */
	bool saveDocument(SPDocument *doc, gchar const *filename);

    //For statistics
    int nrNodes;
    int nrShapes;
    
    int idindex;

    double minx;
    double miny;
    double maxx;
    double maxy;
    

};




}  // namespace Internal
}  // namespace Extension
}  // namespace Inkscape



#endif /* EXTENSION_INTERNAL_POV_OUT_H */


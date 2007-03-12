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

namespace Inkscape
{
namespace Extension
{
namespace Internal
{



class PovOutput : public Inkscape::Extension::Implementation::Implementation
{

typedef Glib::ustring String;


public:

	bool check (Inkscape::Extension::Extension * module);

	void          save  (Inkscape::Extension::Output *mod,
	                     SPDocument *doc,
	                     const gchar *uri);

	static void   init  (void);
	
	void reset();
	
	/**
	 * Format text to our output buffer
	 */     	
	void out(char *fmt, ...);

    void vec2(double a, double b);

    void vec3(double a, double b, double c);

    void vec4(double a, double b, double c, double d);

    void rgbf(double r, double g, double b, double f);

    void segment(int segNr, double a0, double a1,
                            double b0, double b1,
                            double c0, double c1,
                            double d0, double d1);


    void doHeader();

    void doTail();

    void doCurves(SPDocument *doc);

	String outbuf;
	
    char fmtbuf[2048];


    /**
     * used for saving information about shapes
     */
    class PovShapeInfo
    {
    public:
    PovShapeInfo()
        {}
    PovShapeInfo(const PovShapeInfo &other)
        { assign(other); }
    PovShapeInfo operator=(const PovShapeInfo &other)
        { assign(other); return *this; }
    virtual ~PovShapeInfo()
        {}
    String id;
    String color;

    private:
    void assign(const PovShapeInfo &other)
        {
        id    = other.id;
        color = other.color;
        }
    };

    //A list for saving information about the shapes
    std::vector<PovShapeInfo> povShapes;
    
    int nrNodes;
    int nrSegments;
    int nrShapes;

};




}  // namespace Internal
}  // namespace Extension
}  // namespace Inkscape



#endif /* EXTENSION_INTERNAL_POV_OUT_H */


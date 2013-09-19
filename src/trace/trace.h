/*
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004-2006 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_TRACE_H
#define SEEN_TRACE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include <glibmm/refptr.h>
#include <gdkmm/pixbuf.h>
#include <vector>
#include <sp-shape.h>

class SPImage;
class  SPItem;

namespace Inkscape {

namespace Trace {



/**
 *
 */
class TracingEngineResult
{

public:

    /**
     *
     */
    TracingEngineResult(const std::string &theStyle,
                        const std::string &thePathData,
                        long theNodeCount) :
	    style(theStyle),
	    pathData(thePathData),
	    nodeCount(theNodeCount)
        {}

    TracingEngineResult(const TracingEngineResult &other)
        { assign(other); }

    virtual TracingEngineResult &operator=(const TracingEngineResult &other)
        { assign(other); return *this; }


    /**
     *
     */
    virtual ~TracingEngineResult()
        { }


    /**
     *
     */
    std::string getStyle()
        { return style; }

    /**
     *
     */
    std::string getPathData()
        { return pathData; }

    /**
     *
     */
    long getNodeCount()
        { return nodeCount; }

private:

    void assign(const TracingEngineResult &other)
        {
        style = other.style;
        pathData = other.pathData;
        nodeCount = other.nodeCount;
        }

    std::string style;

    std::string pathData;

    long nodeCount;

};



/**
 * A generic interface for plugging different
 *  autotracers into Inkscape.
 */
class TracingEngine
{

    public:

    /**
     *
     */
    TracingEngine()
        {}

    /**
     *
     */
    virtual ~TracingEngine()
        {}

    /**
     *  This is the working method of this interface, and all
     *  implementing classes.  Take a GdkPixbuf, trace it, and
     *  return a style attribute and the path data that is
     *  compatible with the d="" attribute
     *  of an SVG <path> element.
     */
    virtual  std::vector<TracingEngineResult> trace(
                           Glib::RefPtr<Gdk::Pixbuf> /*pixbuf*/)
        { std::vector<TracingEngineResult> dummy;  return dummy; }


    /**
     *  Abort the thread that is executing getPathDataFromPixbuf()
     */
    virtual void abort()
        {}



};//class TracingEngine









/**
 *  This simple class allows a generic wrapper around a given
 *  TracingEngine object.  Its purpose is to provide a gateway
 *  to a variety of tracing engines, while maintaining a
 *  consistent interface.
 */
class Tracer
{

public:


    /**
     *
     */
    Tracer()
        {
        engine       = NULL;
        sioxEnabled  = false;
        }



    /**
     *
     */
    ~Tracer()
        {}


    /**
     *  A convenience method to allow other software to 'see' the
     *  same image that this class sees.
     */
    Glib::RefPtr<Gdk::Pixbuf> getSelectedImage();

    /**
     * This is the main working method.  Trace the selected image, if
     * any, and create a <path> element from it, inserting it into
     * the current document.
     */
    void trace(TracingEngine *engine);


    /**
     *  Abort the thread that is executing convertImageToPath()
     */
    void abort();

    /**
     *  Whether we want to enable SIOX subimage selection.
     */
    void enableSiox(bool enable);


private:

    /**
     * This is the single path code that is called by its counterpart above.
     * Threaded method that does single bitmap--->path conversion.
     */
    void traceThread();

    /**
     * This is true during execution. Setting it to false (like abort()
     * does) should inform the threaded code that it needs to stop
     */
    bool keepGoing;

    /**
     *  During tracing, this is Non-null, and refers to the
     *  engine that is currently doing the tracing.
     */
    TracingEngine *engine;

    /**
     * Get the selected image.  Also check for any SPItems over it, in
     * case the user wants SIOX pre-processing.
     */
    SPImage *getSelectedSPImage();

    std::vector<SPShape *> sioxShapes;

    bool sioxEnabled;

    /**
     * Process a GdkPixbuf, according to which areas have been
     * obscured in the GUI.
     */
    Glib::RefPtr<Gdk::Pixbuf> sioxProcessImage(SPImage *img, Glib::RefPtr<Gdk::Pixbuf> origPixbuf);

    Glib::RefPtr<Gdk::Pixbuf> lastSioxPixbuf;
    Glib::RefPtr<Gdk::Pixbuf> lastOrigPixbuf;

};//class Tracer




} // namespace Trace

} // namespace Inkscape



#endif // SEEN_TRACE_H

//#########################################################################
//# E N D   O F   F I L E
//#########################################################################


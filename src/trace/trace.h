/*
 * A generic interface for plugging different
 *  autotracers into Inkscape.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __TRACE_H__
#define __TRACE_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include <gdk/gdkpixbuf.h>


struct SPImage;
struct SPItem;

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
    TracingEngineResult(char *theStyle, char *thePathData, long theNodeCount)
        {
        next      = NULL;
        style     = strdup(theStyle);
        pathData  = strdup(thePathData);
        nodeCount = theNodeCount;
        }

    /**
     *
     */
    virtual ~TracingEngineResult()
        {
        if (next)
            delete next;
        if (style)
            free(style);
        if (pathData)
            free(pathData);
        }


    /**
     *
     */
    char *getStyle()
        { return style; }

    /**
     *
     */
    char *getPathData()
        { return pathData; }

    /**
     *
     */
    long getNodeCount()
        { return nodeCount; }

    /**
     *
     */
    TracingEngineResult *next;

private:

    char *style;

    char *pathData;

    long nodeCount;

};



/**
 *
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
    virtual  TracingEngineResult *trace(GdkPixbuf *pixbuf, int *nrPaths)
        { return NULL; }


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
        selectedItem = NULL;
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
    GdkPixbuf *getSelectedImage();

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


private:

    /**
     * This is the single path code that is called by its counterpart above.
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

    SPImage *getSelectedSPImage();

    SPItem *selectedItem;


};//class Tracer




} // namespace Trace

} // namespace Inkscape



#endif //__TRACE_H__

//#########################################################################
//# E N D   O F   F I L E
//#########################################################################


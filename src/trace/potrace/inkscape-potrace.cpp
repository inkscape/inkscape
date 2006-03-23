/*
 * This is the C++ glue between Inkscape and Potrace
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * Potrace, the wonderful tracer located at http://potrace.sourceforge.net,
 * is provided by the generosity of Peter Selinger, to whom we are grateful.
 *
 */

#include <glibmm/i18n.h>
#include <gtkmm/main.h>

#include "trace/filterset.h"
#include "trace/imagemap-gdk.h"

#include <inkscape.h>
#include <desktop-handles.h>
#include "message-stack.h"
#include <sp-path.h>
#include <svg/stringstream.h>
#include "curve.h"
#include "bitmap.h"

#include "inkscape-potrace.h"


static void updateGui()
{
   //## Allow the GUI to update
   Gtk::Main::iteration(false); //at least once, non-blocking
   while( Gtk::Main::events_pending() )
       Gtk::Main::iteration();

}


static void potraceStatusCallback(double progress, void *userData) /* callback fn */
{
    updateGui();

    if (!userData)
        return;

    //g_message("progress: %f\n", progress);

    //Inkscape::Trace::Potrace::PotraceTracingEngine *engine =
    //      (Inkscape::Trace::Potrace::PotraceTracingEngine *)userData;
}




//required by potrace
namespace Inkscape {

namespace Trace {

namespace Potrace {


/**
 *
 */
PotraceTracingEngine::PotraceTracingEngine()
{

    //##### Our defaults
    invert    = false;
    traceType = TRACE_BRIGHTNESS;

    quantizationNrColors = 8;

    brightnessThreshold  = 0.45;

    cannyHighThreshold   = 0.65;


}




typedef struct
{
    double x;
    double y;
} Point;


/**
 * Check a point against a list of points to see if it
 * has already occurred.
 */
static bool
hasPoint(std::vector<Point> &points, double x, double y)
{
    for (unsigned int i=0; i<points.size() ; i++)
        {
        Point p = points[i];
        if (p.x == x && p.y == y)
            return true;
        }
    return false;
}


/**
 *  Recursively descend the path_t node tree, writing paths in SVG
 *  format into the output stream.  The Point vector is used to prevent
 *  redundant paths.  Returns number of paths processed.
 */
static long
writePaths(PotraceTracingEngine *engine, potrace_path_t *plist,
            Inkscape::SVGOStringStream& data, std::vector<Point> &points)
{
    long nodeCount = 0L;

    potrace_path_t *node;
    for (node=plist; node ; node=node->sibling)
        {
        potrace_curve_t *curve = &(node->curve);
        //g_message("node->fm:%d\n", node->fm);
        if (!curve->n)
            continue;
        dpoint_t *pt = curve->c[curve->n - 1];
        double x0 = 0.0;
        double y0 = 0.0;
        double x1 = 0.0;
        double y1 = 0.0;
        double x2 = pt[2].x;
        double y2 = pt[2].y;
        //Have we been here already?
        if (hasPoint(points, x2, y2))
            {
            //g_message("duplicate point: (%f,%f)\n", x2, y2);
            continue;
            }
        else
            {
            Point p;
            p.x = x2; p.y = y2;
            points.push_back(p);
            }
        data << "M " << x2 << " " << y2 << " ";
        nodeCount++;

        for (int i=0 ; i<curve->n ; i++)
            {
            if (!engine->keepGoing)
                return 0L;
            pt = curve->c[i];
            x0 = pt[0].x;
            y0 = pt[0].y;
            x1 = pt[1].x;
            y1 = pt[1].y;
            x2 = pt[2].x;
            y2 = pt[2].y;
            switch (curve->tag[i])
                {
                case POTRACE_CORNER:
                    data << "L " << x1 << " " << y1 << " " ;
                    data << "L " << x2 << " " << y2 << " " ;
                break;
                case POTRACE_CURVETO:
                    data << "C " << x0 << " " << y0 << " "
                                 << x1 << " " << y1 << " "
                                 << x2 << " " << y2 << " ";

                break;
                default:
                break;
                }
            nodeCount++;
            }
        data << "z";

        for (path_t *child=node->childlist; child ; child=child->sibling)
            {
            nodeCount += writePaths(engine, child, data, points);
            }
        }

    return nodeCount;

}





static GrayMap *
filter(PotraceTracingEngine &engine, GdkPixbuf * pixbuf)
{
    if (!pixbuf)
        return NULL;

    GrayMap *newGm = NULL;

    /*### Color quantization -- banding ###*/
    if (engine.getTraceType() == TRACE_QUANT)
        {
        RgbMap *rgbmap = gdkPixbufToRgbMap(pixbuf);
        //rgbMap->writePPM(rgbMap, "rgb.ppm");
        newGm = quantizeBand(rgbmap,
                            engine.getQuantizationNrColors());
        rgbmap->destroy(rgbmap);
        //return newGm;
        }

    /*### Brightness threshold ###*/
    else if ( engine.getTraceType() == TRACE_BRIGHTNESS ||
              engine.getTraceType() == TRACE_BRIGHTNESS_MULTI )
        {
        GrayMap *gm = gdkPixbufToGrayMap(pixbuf);

        newGm = GrayMapCreate(gm->width, gm->height);
        double floor =  3.0 *
               ( engine.getBrightnessFloor() * 256.0 );
        double cutoff =  3.0 *
               ( engine.getBrightnessThreshold() * 256.0 );
        for (int y=0 ; y<gm->height ; y++)
            {
            for (int x=0 ; x<gm->width ; x++)
                {
                double brightness = (double)gm->getPixel(gm, x, y);
                if (brightness >= floor && brightness < cutoff)
                    newGm->setPixel(newGm, x, y, GRAYMAP_BLACK);  //black pixel
                else
                    newGm->setPixel(newGm, x, y, GRAYMAP_WHITE); //white pixel
                }
            }

        gm->destroy(gm);
        //newGm->writePPM(newGm, "brightness.ppm");
        //return newGm;
        }

    /*### Canny edge detection ###*/
    else if (engine.getTraceType() == TRACE_CANNY)
        {
        GrayMap *gm = gdkPixbufToGrayMap(pixbuf);
        newGm = grayMapCanny(gm, 0.1, engine.getCannyHighThreshold());
        gm->destroy(gm);
        //newGm->writePPM(newGm, "canny.ppm");
        //return newGm;
        }

    /*### Do I invert the image? ###*/
    if (newGm && engine.getInvert())
        {
        for (int y=0 ; y<newGm->height ; y++)
            {
            for (int x=0 ; x<newGm->width ; x++)
                {
                unsigned long brightness = newGm->getPixel(newGm, x, y);
                brightness = 765 - brightness;
                newGm->setPixel(newGm, x, y, brightness);
                }
            }
        }

    return newGm;//none of the above
}


static IndexedMap *
filterIndexed(PotraceTracingEngine &engine, GdkPixbuf * pixbuf)
{
    if (!pixbuf)
        return NULL;

    IndexedMap *newGm = NULL;

    /*### Color quant multiscan ###*/
    if (engine.getTraceType() == TRACE_QUANT_COLOR)
        {
        RgbMap *gm = gdkPixbufToRgbMap(pixbuf);
        if (engine.getMultiScanSmooth())
            {
            RgbMap *gaussMap = rgbMapGaussian(gm);
            newGm = rgbMapQuantize(gaussMap, 8, engine.getMultiScanNrColors());
            gaussMap->destroy(gaussMap);
            }
        else
            {
            newGm = rgbMapQuantize(gm, 8, engine.getMultiScanNrColors());
            }
        gm->destroy(gm);
        }

    /*### Quant multiscan ###*/
    else if (engine.getTraceType() == TRACE_QUANT_MONO)
        {
        RgbMap *gm = gdkPixbufToRgbMap(pixbuf);
        if (engine.getMultiScanSmooth())
            {
            RgbMap *gaussMap = rgbMapGaussian(gm);
            newGm = rgbMapQuantize(gaussMap, 8, engine.getMultiScanNrColors());
            gaussMap->destroy(gaussMap);
            }
        else
            {
            newGm = rgbMapQuantize(gm, 8, engine.getMultiScanNrColors());
            }
        gm->destroy(gm);

        //Turn to grays
        for (int i=0 ; i<newGm->nrColors ; i++)
            {
            RGB rgb = newGm->clut[i];
            int grayVal = (rgb.r + rgb.g + rgb.b) / 3;
            rgb.r = rgb.g = rgb.b = grayVal;
            newGm->clut[i] = rgb;
            }
        }

    return newGm;
}




GdkPixbuf *
PotraceTracingEngine::preview(GdkPixbuf * pixbuf)
{
    if ( traceType == TRACE_QUANT_COLOR ||
         traceType == TRACE_QUANT_MONO   )
        {
        IndexedMap *gm = filterIndexed(*this, pixbuf);
        if (!gm)
            return NULL;

        GdkPixbuf *newBuf = indexedMapToGdkPixbuf(gm);

        gm->destroy(gm);

        return newBuf;
        }
    else
        {
        GrayMap *gm = filter(*this, pixbuf);
        if (!gm)
            return NULL;

        GdkPixbuf *newBuf = grayMapToGdkPixbuf(gm);

        gm->destroy(gm);

        return newBuf;
        }
}


//*This is the core inkscape-to-potrace binding
char *PotraceTracingEngine::grayMapToPath(GrayMap *grayMap, long *nodeCount)
{

    /* get default parameters */
    potrace_param_t *potraceParams = potrace_param_default();

    potraceParams->progress.callback = potraceStatusCallback;
    potraceParams->progress.data     = (void *)this;

    potrace_bitmap_t *potraceBitmap = bm_new(grayMap->width, grayMap->height);
    bm_clear(potraceBitmap, 0);

    //##Read the data out of the GrayMap
    for (int y=0 ; y<grayMap->height ; y++)
        {
        for (int x=0 ; x<grayMap->width ; x++)
            {
            BM_UPUT(potraceBitmap, x, y, 
                  grayMap->getPixel(grayMap, x, y) ? 0 : 1);
            }
        }

    //##Debug
    /*
    FILE *f = fopen("poimage.pbm", "wb");
    bm_writepbm(f, bm);
    fclose(f);
    */

    if (!keepGoing)
        {
        g_warning("aborted");
        return NULL;
        }

    /* trace a bitmap*/
    potrace_state_t *potraceState = potrace_trace(potraceParams, 
			                          potraceBitmap);

    //## Free the Potrace bitmap
    bm_free(potraceBitmap);

    if (!keepGoing)
        {
        g_warning("aborted");
        potrace_state_free(potraceState);
        potrace_param_free(potraceParams);
        return NULL;
        }

    Inkscape::SVGOStringStream data;

    data << "";

    //## copy the path information into our d="" attribute string
    std::vector<Point> points;
    long thisNodeCount = writePaths(this, potraceState->plist, data, points);

    /* free a potrace items */
    potrace_state_free(potraceState);
    potrace_param_free(potraceParams);

    if (!keepGoing)
        return NULL;

    char *d = strdup((char *)data.str().c_str());
    if ( nodeCount)
        *nodeCount = thisNodeCount;

    
    return d;

}



/**
 *  This is called for a single scan
 */
TracingEngineResult *
PotraceTracingEngine::traceSingle(GdkPixbuf * thePixbuf, int *nrPaths)
{

    if (!thePixbuf)
        return NULL;

    brightnessFloor = 0.0; //important to set this

    GrayMap *grayMap = filter(*this, thePixbuf);
    if (!grayMap)
        return NULL;

    long nodeCount;
    char *d = grayMapToPath(grayMap, &nodeCount);

    grayMap->destroy(grayMap);

    if (!d)
        {
        *nrPaths = 0;
        return NULL;
        }
    char *style = "fill:#000000";

    //g_message("### GOT '%s' \n", d);
    TracingEngineResult *result = new TracingEngineResult(style, d, nodeCount);

    free(d);

    *nrPaths = 1;

    return result;
}


/**
 *  Called for multiple-scanning algorithms
 */
TracingEngineResult *
PotraceTracingEngine::traceBrightnessMulti(GdkPixbuf * thePixbuf, int *nrPaths)
{

    if (!thePixbuf)
        return NULL;

    double low     = 0.2; //bottom of range
    double high    = 0.9; //top of range
    double delta   = (high - low ) / ((double)multiScanNrColors);

    brightnessFloor = 0.0; //Set bottom to black

    int traceCount = 0;

    TracingEngineResult *results = NULL;
    for ( brightnessThreshold = low ;
          brightnessThreshold <= high ;
          brightnessThreshold += delta)

        {

        GrayMap *grayMap = filter(*this, thePixbuf);
        if (!grayMap)
            return NULL;

        long nodeCount;
        char *d = grayMapToPath(grayMap, &nodeCount);

        grayMap->destroy(grayMap);

        if (!d)
            {
            *nrPaths = 0;
            return NULL;
            }

        int grayVal = (int)(256.0 * brightnessThreshold);
        char style[31];
        sprintf(style, "fill-opacity:1.0;fill:#%02x%02x%02x",
                    grayVal, grayVal, grayVal);

        //g_message("### GOT '%s' \n", d);
        TracingEngineResult *result = new TracingEngineResult(style, d, nodeCount);

        free(d);

        if (!results)
            {
            results = result; //first one
            }
        else
            {
            //walk to end of list
            TracingEngineResult *r;
            for (r=results ; r->next ; r=r->next)
                {}
            r->next = result;
            }

        if (!multiScanStack)
            brightnessFloor = brightnessThreshold;

        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (desktop)
            {
            gchar *msg = g_strdup_printf(_("Trace: %d.  %ld nodes"), traceCount++, nodeCount);
            SP_DT_MSGSTACK(desktop)->flash(Inkscape::NORMAL_MESSAGE, msg);
            g_free(msg);
            }
        }

    //report the count of paths processed
    *nrPaths = multiScanNrColors;


    return results;
}


/**
 *  Quantization
 */
TracingEngineResult *
PotraceTracingEngine::traceQuant(GdkPixbuf * thePixbuf, int *nrPaths)
{

    if (!thePixbuf)
        return NULL;

    IndexedMap *iMap = filterIndexed(*this, thePixbuf);
    if (!iMap)
        return NULL;

    TracingEngineResult *results = NULL;

    //Create and clear a gray map
    GrayMap *gm = GrayMapCreate(iMap->width, iMap->height);
    for (int row=0 ; row<gm->height ; row++)
        for (int col=0 ; col<gm->width ; col++)
            gm->setPixel(gm, col, row, GRAYMAP_WHITE);


    for (int colorIndex=0 ; colorIndex<iMap->nrColors ; colorIndex++)
        {

        /*Make a gray map for each color index */
        for (int row=0 ; row<iMap->height ; row++)
            {
            for (int col=0 ; col<iMap->width ; col++)
                {
                int indx = (int) iMap->getPixel(iMap, col, row);
                if (indx == colorIndex)
                    {
                    gm->setPixel(gm, col, row, GRAYMAP_BLACK); //black
                    }
                else
                    {
                    if (!multiScanStack)
                        gm->setPixel(gm, col, row, GRAYMAP_WHITE);//white
                    }
                }
            }

        //## Now we have a traceable graymap
        long nodeCount;
        char *d = grayMapToPath(gm, &nodeCount);

        if (!d)
            {
            *nrPaths = 0;
            return NULL;
            }

        //### get style info
        char style[13];
        RGB rgb = iMap->clut[colorIndex];
        sprintf(style, "fill:#%02x%02x%02x", rgb.r, rgb.g, rgb.b);

        //g_message("### GOT '%s' \n", d);
        TracingEngineResult *result = new TracingEngineResult(style, d, nodeCount);

        free(d);

        if (!results)
            {
            results = result; //first one
            }
        else
            {
            //prepend
            /*
            result->next = results;
            results = result;
            */

            //append
            TracingEngineResult *r;
            for (r=results ; r->next ; r=r->next)
                {}
            r->next = result;
            }


        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (desktop)
            {
            gchar *msg = g_strdup_printf(_("Trace: %d.  %ld nodes"), colorIndex, nodeCount);
            SP_DT_MSGSTACK(desktop)->flash(Inkscape::NORMAL_MESSAGE, msg);
            g_free(msg);
            }


        }

    //report the count of paths processed
    *nrPaths = iMap->nrColors;

    gm->destroy(gm);
    iMap->destroy(iMap);

    return results;
}


/**
 *  This is the working method of this interface, and all
 *  implementing classes.  Take a GdkPixbuf, trace it, and
 *  return the path data that is compatible with the d="" attribute
 *  of an SVG <path> element.
 */
TracingEngineResult *
PotraceTracingEngine::trace(GdkPixbuf * thePixbuf, int *nrPaths)
{

    //Set up for messages
    keepGoing             = 1;

    if ( traceType == TRACE_QUANT_COLOR ||
         traceType == TRACE_QUANT_MONO   )
        {
        return traceQuant(thePixbuf, nrPaths);
        }
    else if ( traceType == TRACE_BRIGHTNESS_MULTI )
        {
        return traceBrightnessMulti(thePixbuf, nrPaths);
        }
    else
        {
        return traceSingle(thePixbuf, nrPaths);
        }
}





/**
 *  Abort the thread that is executing getPathDataFromPixbuf()
 */
void
PotraceTracingEngine::abort()
{
    //g_message("PotraceTracingEngine::abort()\n");
    keepGoing = 0;
}




}  // namespace Potrace
}  // namespace Trace
}  // namespace Inkscape


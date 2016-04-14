/*
 * A generic interface for plugging different
 *  autotracers into Inkscape.
 *
 * Authors:
 *   Bob Jamison <rjamison@earthlink.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2006 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "trace/potrace/inkscape-potrace.h"

#include "inkscape.h"
#include "desktop.h"

#include "document.h"
#include "document-undo.h"
#include "message-stack.h"
#include <glibmm/i18n.h>
#include <gtkmm/main.h>
#include "selection.h"
#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "sp-item.h"
#include "sp-shape.h"
#include "sp-image.h"
#include <2geom/transforms.h>
#include "verbs.h"

#include "display/cairo-utils.h"
#include "display/drawing.h"
#include "display/drawing-shape.h"

#include "siox.h"
#include "imagemap-gdk.h"

namespace Inkscape {
namespace Trace {

SPImage *Tracer::getSelectedSPImage()
{

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: No active desktop");
        return NULL;
        }

    Inkscape::MessageStack *msgStack = desktop->getMessageStack();

    Inkscape::Selection *sel = desktop->getSelection();
    if (!sel)
        {
        char *msg = _("Select an <b>image</b> to trace");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    if (sioxEnabled)
        {
        SPImage *img = NULL;
        std::vector<SPItem*> const list = sel->itemList();
        std::vector<SPItem *> items;
        sioxShapes.clear();

        /*
           First, things are selected top-to-bottom, so we need to invert
           them as bottom-to-top so that we can discover the image and any
           SPItems above it
        */
        for (std::vector<SPItem*>::const_iterator i=list.begin() ; list.end()!=i ; ++i)
            {
            if (!SP_IS_ITEM(*i))
                {
                continue;
                }
            SPItem *item = *i;
            items.insert(items.begin(), item);
            }
        std::vector<SPItem *>::iterator iter;
        for (iter = items.begin() ; iter!= items.end() ; ++iter)
            {
            SPItem *item = *iter;
            if (SP_IS_IMAGE(item))
                {
                if (img) //we want only one
                    {
                    char *msg = _("Select only one <b>image</b> to trace");
                    msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
                    return NULL;
                    }
                img = SP_IMAGE(item);
                }
            else // if (img) //# items -after- the image in tree (above it in Z)
                {
                if (SP_IS_SHAPE(item))
                    {
                    SPShape *shape = SP_SHAPE(item);
                    sioxShapes.push_back(shape);
                    }
                }
            }

        if (!img || sioxShapes.size() < 1)
            {
            char *msg = _("Select one image and one or more shapes above it");
            msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
            return NULL;
            }
        return img;
        }
    else
        //### SIOX not enabled.  We want exactly one image selected
        {
        SPItem *item = sel->singleItem();
        if (!item)
            {
            char *msg = _("Select an <b>image</b> to trace");  //same as above
            msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
            //g_warning(msg);
            return NULL;
            }

        if (!SP_IS_IMAGE(item))
            {
            char *msg = _("Select an <b>image</b> to trace");
            msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
            //g_warning(msg);
            return NULL;
            }

        SPImage *img = SP_IMAGE(item);

        return img;
        }

}



typedef org::siox::SioxImage SioxImage;
typedef org::siox::SioxObserver SioxObserver;
typedef org::siox::Siox Siox;


class TraceSioxObserver : public SioxObserver
{
public:

    /**
     *
     */
    TraceSioxObserver (void *contextArg) :
                     SioxObserver(contextArg)
        {}

    /**
     *
     */
    virtual ~TraceSioxObserver ()
        { }

    /**
     *  Informs the observer how much has been completed.
     *  Return false if the processing should be aborted.
     */
    virtual bool progress(float /*percentCompleted*/)
        {
        //Tracer *tracer = (Tracer *)context;
        //## Allow the GUI to update
        Gtk::Main::iteration(false); //at least once, non-blocking
        while( Gtk::Main::events_pending() )
            Gtk::Main::iteration();
        return true;
        }

    /**
     *  Send an error string to the Observer.  Processing will
     *  be halted.
     */
    virtual void error(const std::string &/*msg*/)
        {
        //Tracer *tracer = (Tracer *)context;
        }


};





Glib::RefPtr<Gdk::Pixbuf> Tracer::sioxProcessImage(SPImage *img, Glib::RefPtr<Gdk::Pixbuf>origPixbuf)
{
    if (!sioxEnabled)
        return origPixbuf;

    if (origPixbuf == lastOrigPixbuf)
        return lastSioxPixbuf;

    //g_message("siox: start");

    //Convert from gdk, so a format we know.  By design, the pixel
    //format in PackedPixelMap is identical to what is needed by SIOX
    SioxImage simage(origPixbuf->gobj());

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("%s", _("Trace: No active desktop"));
        return Glib::RefPtr<Gdk::Pixbuf>(NULL);
        }

    Inkscape::MessageStack *msgStack = desktop->getMessageStack();

    Inkscape::Selection *sel = desktop->getSelection();
    if (!sel)
        {
        char *msg = _("Select an <b>image</b> to trace");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return Glib::RefPtr<Gdk::Pixbuf>(NULL);
        }

    Inkscape::DrawingItem *aImg = img->get_arenaitem(desktop->dkey);
    //g_message("img: %d %d %d %d\n", aImg->bbox.x0, aImg->bbox.y0,
    //                                aImg->bbox.x1, aImg->bbox.y1);

    double width  = aImg->geometricBounds()->width();
    double height = aImg->geometricBounds()->height();

    double iwidth  = simage.getWidth();
    double iheight = simage.getHeight();

    double iwscale = width  / iwidth;
    double ihscale = height / iheight;

    std::vector<Inkscape::DrawingItem *> arenaItems;
    std::vector<SPShape *>::iterator iter;
    for (iter = sioxShapes.begin() ; iter!=sioxShapes.end() ; ++iter)
        {
        SPItem *item = *iter;
        Inkscape::DrawingItem *aItem = item->get_arenaitem(desktop->dkey);
        arenaItems.push_back(aItem);
        }

    //g_message("%d arena items\n", arenaItems.size());

    //PackedPixelMap *dumpMap = PackedPixelMapCreate(
    //                simage.getWidth(), simage.getHeight());

    //g_message("siox: start selection");

    for (int row=0 ; row<iheight ; row++)
        {
        double ypos = aImg->geometricBounds()->top() + ihscale * (double) row;
        for (int col=0 ; col<simage.getWidth() ; col++)
            {
            //Get absolute X,Y position
            double xpos = aImg->geometricBounds()->left() + iwscale * (double)col;
            Geom::Point point(xpos, ypos);
            point *= aImg->transform();
            //point *= imgMat;
            //point = desktop->doc2dt(point);
            //g_message("x:%f    y:%f\n", point[0], point[1]);
            bool weHaveAHit = false;
            std::vector<Inkscape::DrawingItem *>::iterator aIter;
            for (aIter = arenaItems.begin() ; aIter!=arenaItems.end() ; ++aIter)
                {
                Inkscape::DrawingItem *arenaItem = *aIter;
                arenaItem->drawing().update();
                if (arenaItem->pick(point, 1.0f, 1))
                    {
                    weHaveAHit = true;
                    break;
                    }
                }

            if (weHaveAHit)
                {
                //g_message("hit!\n");
                //dumpMap->setPixelLong(dumpMap, col, row, 0L);
                simage.setConfidence(col, row,
                        Siox::UNKNOWN_REGION_CONFIDENCE);
                }
            else
                {
                //g_message("miss!\n");
                //dumpMap->setPixelLong(dumpMap, col, row,
                //        simage.getPixel(col, row));
                simage.setConfidence(col, row,
                        Siox::CERTAIN_BACKGROUND_CONFIDENCE);
                }
            }
        }

    //g_message("siox: selection done");

    //dumpMap->writePPM(dumpMap, "siox1.ppm");
    //dumpMap->destroy(dumpMap);

    //## ok we have our pixel buf
    TraceSioxObserver observer(this);
    Siox sengine(&observer);
    SioxImage result = sengine.extractForeground(simage, 0xffffff);
    if (!result.isValid())
        {
        g_warning("%s", _("Invalid SIOX result"));
        return Glib::RefPtr<Gdk::Pixbuf>(NULL);
        }

    //result.writePPM("siox2.ppm");

    Glib::RefPtr<Gdk::Pixbuf> newPixbuf = Glib::wrap(result.getGdkPixbuf());

    //g_message("siox: done");

    lastSioxPixbuf = newPixbuf;

    return newPixbuf;
}


Glib::RefPtr<Gdk::Pixbuf> Tracer::getSelectedImage()
{


    SPImage *img = getSelectedSPImage();
    if (!img)
        return Glib::RefPtr<Gdk::Pixbuf>(NULL);

    if (!img->pixbuf)
        return Glib::RefPtr<Gdk::Pixbuf>(NULL);

    GdkPixbuf *raw_pb = img->pixbuf->getPixbufRaw(false);
    GdkPixbuf *trace_pb = gdk_pixbuf_copy(raw_pb);
    if (img->pixbuf->pixelFormat() == Inkscape::Pixbuf::PF_CAIRO) {
        convert_pixels_argb32_to_pixbuf(
            gdk_pixbuf_get_pixels(trace_pb),
            gdk_pixbuf_get_width(trace_pb),
            gdk_pixbuf_get_height(trace_pb),
            gdk_pixbuf_get_rowstride(trace_pb));
    }

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Glib::wrap(trace_pb, false);

    if (sioxEnabled)
        {
        Glib::RefPtr<Gdk::Pixbuf> sioxPixbuf =
             sioxProcessImage(img, pixbuf);
        if (!sioxPixbuf)
            {
            return pixbuf;
            }
        else
            {
            return sioxPixbuf;
            }
        }
    else
        {
        return pixbuf;
        }

}



//#########################################################################
//#  T R A C E
//#########################################################################

void Tracer::enableSiox(bool enable)
{
    sioxEnabled = enable;
}


void Tracer::traceThread()
{
    //## Remember. NEVER leave this method without setting
    //## engine back to NULL

    //## Prepare our kill flag.  We will watch this later to
    //## see if the main thread wants us to stop
    keepGoing = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: No active desktop\n");
        return;
        }

    Inkscape::MessageStack *msgStack = desktop->getMessageStack();

    Inkscape::Selection *selection = desktop->getSelection();

    if (!SP_ACTIVE_DOCUMENT)
        {
        char *msg = _("Trace: No active document");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    doc->ensureUpToDate();


    SPImage *img = getSelectedSPImage();
    if (!img)
        {
        engine = NULL;
        return;
        }

    GdkPixbuf *trace_pb = gdk_pixbuf_copy(img->pixbuf->getPixbufRaw(false));
    if (img->pixbuf->pixelFormat() == Inkscape::Pixbuf::PF_CAIRO) {
        convert_pixels_argb32_to_pixbuf(
            gdk_pixbuf_get_pixels(trace_pb),
            gdk_pixbuf_get_width(trace_pb),
            gdk_pixbuf_get_height(trace_pb),
            gdk_pixbuf_get_rowstride(trace_pb));
    }

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Glib::wrap(trace_pb, false);

    pixbuf = sioxProcessImage(img, pixbuf);

    if (!pixbuf)
        {
        char *msg = _("Trace: Image has no bitmap data");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }

    msgStack->flash(Inkscape::NORMAL_MESSAGE, _("Trace: Starting trace..."));
    desktop->updateCanvasNow();

    std::vector<TracingEngineResult> results =
                engine->trace(pixbuf);
    //printf("nrPaths:%d\n", results.size());
    int nrPaths = results.size();

    //### Check if we should stop
    if (!keepGoing || nrPaths<1)
        {
        engine = NULL;
        return;
        }

    //### Get pointers to the <image> and its parent
    //XML Tree being used directly here while it shouldn't be.
    Inkscape::XML::Node *imgRepr   = SP_OBJECT(img)->getRepr();
    Inkscape::XML::Node *par       = imgRepr->parent();

    //### Get some information for the new transform()
    double x      = 0.0;
    double y      = 0.0;
    double width  = 0.0;
    double height = 0.0;
    double dval   = 0.0;

    if (sp_repr_get_double(imgRepr, "x", &dval))
        x = dval;
    if (sp_repr_get_double(imgRepr, "y", &dval))
        y = dval;

    if (sp_repr_get_double(imgRepr, "width", &dval))
        width = dval;
    if (sp_repr_get_double(imgRepr, "height", &dval))
        height = dval;

    double iwidth  = (double)pixbuf->get_width();
    double iheight = (double)pixbuf->get_height();

    double iwscale = width  / iwidth;
    double ihscale = height / iheight;

    Geom::Translate trans(x, y);
    Geom::Scale scal(iwscale, ihscale);

    //# Convolve scale, translation, and the original transform
    Geom::Affine tf(scal * trans);
    tf *= img->transform;


    //#OK.  Now let's start making new nodes

    Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
    Inkscape::XML::Node *groupRepr = NULL;

    //# if more than 1, make a <g>roup of <path>s
    if (nrPaths > 1)
        {
        groupRepr = xml_doc->createElement("svg:g");
        par->addChild(groupRepr, imgRepr);
        }

    long totalNodeCount = 0L;

    for (unsigned int i=0 ; i<results.size() ; i++)
        {
        TracingEngineResult result = results[i];
        totalNodeCount += result.getNodeCount();

        Inkscape::XML::Node *pathRepr = xml_doc->createElement("svg:path");
        pathRepr->setAttribute("style", result.getStyle().c_str());
        pathRepr->setAttribute("d",     result.getPathData().c_str());

        if (nrPaths > 1)
            groupRepr->addChild(pathRepr, NULL);
        else
            par->addChild(pathRepr, imgRepr);

        //### Apply the transform from the image to the new shape
        SPObject *reprobj = doc->getObjectByRepr(pathRepr);
        if (reprobj)
            {
            SPItem *newItem = SP_ITEM(reprobj);
            newItem->doWriteTransform(pathRepr, tf, NULL);
            }
        if (nrPaths == 1)
            {
            selection->clear();
            selection->add(pathRepr);
            }
        Inkscape::GC::release(pathRepr);
        }

    // If we have a group, then focus on, then forget it
    if (nrPaths > 1)
        {
        selection->clear();
        selection->add(groupRepr);
        Inkscape::GC::release(groupRepr);
        }

    //## inform the document, so we can undo
    DocumentUndo::done(doc, SP_VERB_SELECTION_TRACE, _("Trace bitmap"));

    engine = NULL;

    char *msg = g_strdup_printf(_("Trace: Done. %ld nodes created"), totalNodeCount);
    msgStack->flash(Inkscape::NORMAL_MESSAGE, msg);
    g_free(msg);

}





void Tracer::trace(TracingEngine *theEngine)
{
    //Check if we are already running
    if (engine)
        return;

    engine = theEngine;

#if HAVE_THREADS
    //Ensure that thread support is running
    if (!Glib::thread_supported())
        Glib::thread_init();

    //Create our thread and run it
    Glib::Thread::create(
        sigc::mem_fun(*this, &Tracer::traceThread), false);
#else
    traceThread();
#endif

}





void Tracer::abort()
{

    //## Inform Trace's working thread
    keepGoing = false;

    if (engine)
        {
        engine->abort();
        }

}



} // namespace Trace

} // namespace Inkscape


//#########################################################################
//# E N D   O F   F I L E
//#########################################################################


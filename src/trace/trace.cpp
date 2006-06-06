/*
 * A generic interface for plugging different
 *  autotracers into Inkscape.
 *
 * Authors:
 *   Bob Jamison <rjamison@earthlink.net>
 *
 * Copyright (C) 2004-2006 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



#include "trace/potrace/inkscape-potrace.h"

#include <inkscape.h>
#include <desktop.h>
#include <desktop-handles.h>
#include <document.h>
#include <message-stack.h>
#include <glibmm/i18n.h>
#include <selection.h>
#include <xml/repr.h>
#include <xml/attribute-record.h>
#include <sp-item.h>
#include <sp-shape.h>
#include <sp-image.h>

#include <display/nr-arena.h>
#include <display/nr-arena-shape.h>

#include "siox.h"
#include "imagemap-gdk.h"



namespace Inkscape
{

namespace Trace
{






/**
 * Get the selected image.  Also check for any SPItems over it, in
 * case the user wants SIOX pre-processing.
 */
SPImage *
Tracer::getSelectedSPImage()
{

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: No active desktop\n");
        return NULL;
        }

    Inkscape::MessageStack *msgStack = sp_desktop_message_stack(desktop);

    Inkscape::Selection *sel = sp_desktop_selection(desktop);
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
        GSList const *list = sel->itemList();
        std::vector<SPItem *> items;
        sioxShapes.clear();

        /*
           First, things are selected top-to-bottom, so we need to invert
           them as bottom-to-top so that we can discover the image and any
           SPItems above it
        */
        for ( ; list ; list=list->next)
            {
            if (!SP_IS_ITEM(list->data))
                {
                continue;
                }
            SPItem *item = SP_ITEM(list->data);
            items.insert(items.begin(), item);
            }
        std::vector<SPItem *>::iterator iter;
        for (iter = items.begin() ; iter!= items.end() ; iter++)
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
typedef org::siox::Siox Siox;

GdkPixbuf *
Tracer::sioxProcessImage(SPImage *img, GdkPixbuf *origPixbuf)
{
    if (!sioxEnabled)
        return origPixbuf;

    //Convert from gdk, so a format we know.  By design, the pixel
    //format in PackedPixelMap is identical to what is needed by SIOX
    SioxImage simage(origPixbuf);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning(_("Trace: No active desktop"));
        return NULL;
        }

    Inkscape::MessageStack *msgStack = sp_desktop_message_stack(desktop);

    Inkscape::Selection *sel = sp_desktop_selection(desktop);
    if (!sel)
        {
        char *msg = _("Select an <b>image</b> to trace");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    NRArenaItem *aImg = sp_item_get_arenaitem(img, desktop->dkey);
    //g_message("img: %d %d %d %d\n", aImg->bbox.x0, aImg->bbox.y0,
    //                                aImg->bbox.x1, aImg->bbox.y1);

    double width  = (double)(aImg->bbox.x1 - aImg->bbox.x0);
    double height = (double)(aImg->bbox.y1 - aImg->bbox.y0);

    double iwidth  = (double)simage.getWidth();
    double iheight = (double)simage.getHeight();

    double iwscale = width  / iwidth;
    double ihscale = height / iheight;

    std::vector<NRArenaItem *> arenaItems;
    std::vector<SPShape *>::iterator iter;
    for (iter = sioxShapes.begin() ; iter!=sioxShapes.end() ; iter++)
        {
        SPItem *item = *iter;
        //### Create ArenaItems and set transform
        NRArenaItem *aItem = sp_item_get_arenaitem(item, desktop->dkey);

        //nr_arena_item_set_transform(aItem, item->transform);
        //g_message("%d %d %d %d\n", aItem->bbox.x0, aItem->bbox.y0,
        //                           aItem->bbox.x1, aItem->bbox.y1);
        arenaItems.push_back(aItem);
        }
    //g_message("%d arena items\n", arenaItems.size());

    PackedPixelMap *dumpMap = PackedPixelMapCreate(
                    simage.getWidth(), simage.getHeight());

    for (int row=0 ; row<simage.getHeight() ; row++)
        {
        double ypos = ((double)aImg->bbox.y0) + ihscale * (double) row;
        for (int col=0 ; col<simage.getWidth() ; col++)
            {
            //Get absolute X,Y position
            double xpos = ((double)aImg->bbox.x0) + iwscale * (double)col;
            NR::Point point(xpos, ypos);
            point *= aImg->transform;
            //point *= imgMat;
            //point = desktop->doc2dt(point);
            std::vector<SPShape *>::iterator iter;
            //g_message("x:%f    y:%f\n", point[0], point[1]);
            bool weHaveAHit = false;
            std::vector<NRArenaItem *>::iterator aIter;
            for (aIter = arenaItems.begin() ; aIter!=arenaItems.end() ; aIter++)
                {
                NRArenaItem *arenaItem = *aIter;
                NRArenaItemClass *arenaClass =
                    (NRArenaItemClass *) NR_OBJECT_GET_CLASS (arenaItem);
                if (arenaClass->pick(arenaItem, point, 1.0f, 1))
                    {
                    weHaveAHit = true;
                    break;
                    }
                }

            if (weHaveAHit)
                {
                //g_message("hit!\n");
                dumpMap->setPixelLong(dumpMap, col, row, 0L);
                simage.setConfidence(col, row, 
                        Siox::UNKNOWN_REGION_CONFIDENCE);
                }
            else
                {
                dumpMap->setPixelLong(dumpMap, col, row,
                        simage.getPixel(col, row));
                simage.setConfidence(col, row,
                        Siox::CERTAIN_BACKGROUND_CONFIDENCE);
                }
            }
        }

    //dumpMap->writePPM(dumpMap, "siox1.ppm");
    dumpMap->destroy(dumpMap);

    //## ok we have our pixel buf
    org::siox::Siox sengine;
    org::siox::SioxImage result =
            sengine.extractForeground(simage, 0xffffff);
    if (!result.isValid())
        {
        g_warning(_("Invalid SIOX result"));
        return NULL;
        }

    //result.writePPM("siox2.ppm");

    /* Free Arena and ArenaItem */
    /*
    std::vector<NRArenaItem *>::iterator aIter;
    for (aIter = arenaItems.begin() ; aIter!=arenaItems.end() ; aIter++)
       {
       NRArenaItem *arenaItem = *aIter;
       nr_arena_item_unref(arenaItem);
       }
    nr_object_unref((NRObject *) arena);
    */

    GdkPixbuf *newPixbuf = result.getGdkPixbuf();

    return newPixbuf;
}


/**
 *
 */
GdkPixbuf *
Tracer::getSelectedImage()
{

    SPImage *img = getSelectedSPImage();
    if (!img)
        return NULL;

    GdkPixbuf *pixbuf = img->pixbuf;
    if (!pixbuf)
        return NULL;

    if (sioxEnabled)
        {
        GdkPixbuf *sioxPixbuf = sioxProcessImage(img, pixbuf);
        if (!sioxPixbuf)
            {
            g_object_ref(pixbuf);
            return pixbuf;
            }
        else
            {
            return sioxPixbuf;
            }
        }
    else
        {
        g_object_ref(pixbuf);
        return pixbuf;
        }

}



//#########################################################################
//#  T R A C E
//#########################################################################

/**
 *  Whether we want to enable SIOX subimage selection
 */
void Tracer::enableSiox(bool enable)
{
    sioxEnabled = enable;
}


/**
 *  Threaded method that does single bitmap--->path conversion
 */
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

    Inkscape::MessageStack *msgStack = sp_desktop_message_stack(desktop);

    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    if (!SP_ACTIVE_DOCUMENT)
        {
        char *msg = _("Trace: No active document");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    sp_document_ensure_up_to_date(doc);


    SPImage *img = getSelectedSPImage();
    if (!img)
        {
        engine = NULL;
        return;
        }

    GdkPixbuf *pixbuf = img->pixbuf;
    g_object_ref(pixbuf);

    pixbuf = sioxProcessImage(img, pixbuf);

    if (!pixbuf)
        {
        char *msg = _("Trace: Image has no bitmap data");
        msgStack->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }

    int nrPaths;
    TracingEngineResult *results = engine->trace(pixbuf, &nrPaths);
    //printf("nrPaths:%d\n", nrPaths);

    //### Check if we should stop
    if (!keepGoing || !results || nrPaths<1)
        {
        engine = NULL;
        return;
        }

    //### Get pointers to the <image> and its parent
    Inkscape::XML::Node *imgRepr   = SP_OBJECT(img)->repr;
    Inkscape::XML::Node *par       = sp_repr_parent(imgRepr);

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

    NR::Matrix trans(NR::translate(x, y));

    double iwidth  = (double)gdk_pixbuf_get_width(pixbuf);
    double iheight = (double)gdk_pixbuf_get_height(pixbuf);

    double iwscale = width  / iwidth;
    double ihscale = height / iheight;

    NR::Matrix scal(NR::scale(iwscale, ihscale));

    //# Convolve scale, translation, and the original transform
    NR::Matrix tf(scal);
    tf *= trans;
    tf *= img->transform;


    //#OK.  Now let's start making new nodes

    Inkscape::XML::Node *groupRepr = NULL;

    //# if more than 1, make a <g>roup of <path>s
    if (nrPaths > 1)
        {
        groupRepr = sp_repr_new("svg:g");
        par->addChild(groupRepr, imgRepr);
        }

    long totalNodeCount = 0L;

    for (TracingEngineResult *result=results ;
                  result ; result=result->next)
        {
        totalNodeCount += result->getNodeCount();

        Inkscape::XML::Node *pathRepr = sp_repr_new("svg:path");
        pathRepr->setAttribute("style", result->getStyle());
        pathRepr->setAttribute("d",     result->getPathData());

        if (nrPaths > 1)
            groupRepr->addChild(pathRepr, NULL);
        else
            par->addChild(pathRepr, imgRepr);

        //### Apply the transform from the image to the new shape
        SPObject *reprobj = doc->getObjectByRepr(pathRepr);
        if (reprobj)
            {
            SPItem *newItem = SP_ITEM(reprobj);
            sp_item_write_transform(newItem, pathRepr, tf, NULL);
            }
        if (nrPaths == 1)
            {
            selection->clear();
            selection->add(pathRepr);
            }
        Inkscape::GC::release(pathRepr);
        }

    //release our pixbuf
    g_object_unref(pixbuf);

    delete results;

    // If we have a group, then focus on, then forget it
    if (nrPaths > 1)
        {
        selection->clear();
        selection->add(groupRepr);
        Inkscape::GC::release(groupRepr);
        }

    //## inform the document, so we can undo
    sp_document_done(doc);

    engine = NULL;

    char *msg = g_strdup_printf(_("Trace: Done. %ld nodes created"), totalNodeCount);
    msgStack->flash(Inkscape::NORMAL_MESSAGE, msg);
    g_free(msg);

}





/**
 *  Main tracing method
 */
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





/**
 *  Abort the thread that is executing trace()
 */
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


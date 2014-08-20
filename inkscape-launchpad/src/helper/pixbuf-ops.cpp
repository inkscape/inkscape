/*
 * Helpers for SPItem -> gdk_pixbuf related stuff
 *
 * Authors:
 *   John Cliff <simarilius@yahoo.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <png.h>
#include <boost/scoped_ptr.hpp>
#include <2geom/transforms.h>

#include "interface.h"
#include "helper/png-write.h"
#include "display/cairo-utils.h"
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "document.h"
#include "sp-item.h"
#include "sp-root.h"
#include "sp-use.h"
#include "sp-defs.h"
#include "util/units.h"

#include "helper/pixbuf-ops.h"

// TODO look for copy-n-past duplication of this function:
/**
 * Hide all items that are not listed in list, recursively, skipping groups and defs.
 */
static void hide_other_items_recursively(SPObject *o, GSList *list, unsigned dkey)
{
    if ( SP_IS_ITEM(o)
         && !SP_IS_DEFS(o)
         && !SP_IS_ROOT(o)
         && !SP_IS_GROUP(o)
         && !SP_IS_USE(o)
         && !g_slist_find(list, o) )
    {
        SP_ITEM(o)->invoke_hide(dkey);
    }

    // recurse
    if (!g_slist_find(list, o)) {
        for ( SPObject *child = o->firstChild() ; child; child = child->getNext() ) {
            hide_other_items_recursively(child, list, dkey);
        }
    }
}


// The following is a mutation of the flood fill code, the marker preview, and random other samplings.
// The dpi settings dont do anything yet, but I want them to, and was wanting to keep reasonably close
// to the call for the interface to the png writing.

bool sp_export_jpg_file(SPDocument *doc, gchar const *filename,
                        double x0, double y0, double x1, double y1,
                        unsigned width, unsigned height, double xdpi, double ydpi,
                        unsigned long bgcolor, double quality,GSList *items)
{
    boost::scoped_ptr<Inkscape::Pixbuf> pixbuf(
        sp_generate_internal_bitmap(doc, filename, x0, y0, x1, y1,
            width, height, xdpi, ydpi, bgcolor, items));

    gchar c[32];
    g_snprintf(c, 32, "%f", quality);
    gboolean saved = gdk_pixbuf_save(pixbuf->getPixbufRaw(), filename, "jpeg", NULL, "quality", c, NULL);
 
    return saved;
}

/**
    generates a bitmap from given items
    the bitmap is stored in RAM and not written to file
    @param x0
    @param y0
    @param x1
    @param y1
    @param width
    @param height
    @param xdpi
    @param ydpi
    @return the created GdkPixbuf structure or NULL if no memory is allocable
*/
Inkscape::Pixbuf *sp_generate_internal_bitmap(SPDocument *doc, gchar const */*filename*/,
                                       double x0, double y0, double x1, double y1,
                                       unsigned width, unsigned height, double xdpi, double ydpi,
                                       unsigned long /*bgcolor*/,
                                       GSList *items_only)

{
    if (width == 0 || height == 0) return NULL;

    Inkscape::Pixbuf *inkpb = NULL;
    /* Create new drawing for offscreen rendering*/
    Inkscape::Drawing drawing;
    drawing.setExact(true);
    unsigned dkey = SPItem::display_key_new(1);

    doc->ensureUpToDate();

    Geom::Rect screen=Geom::Rect(Geom::Point(x0,y0), Geom::Point(x1, y1));

    double padding = 1.0;

    Geom::Point origin(screen.min()[Geom::X],
                  doc->getHeight().value("px") - screen[Geom::Y].extent() - screen.min()[Geom::Y]);

    origin[Geom::X] = origin[Geom::X] + (screen[Geom::X].extent() * ((1 - padding) / 2));
    origin[Geom::Y] = origin[Geom::Y] + (screen[Geom::Y].extent() * ((1 - padding) / 2));

    Geom::Scale scale(Inkscape::Util::Quantity::convert(xdpi, "px", "in"), Inkscape::Util::Quantity::convert(ydpi, "px", "in"));
    Geom::Affine affine = scale * Geom::Translate(-origin * scale);

    /* Create ArenaItems and set transform */
    Inkscape::DrawingItem *root = doc->getRoot()->invoke_show( drawing, dkey, SP_ITEM_SHOW_DISPLAY);
    root->setTransform(affine);
    drawing.setRoot(root);

    // We show all and then hide all items we don't want, instead of showing only requested items,
    // because that would not work if the shown item references something in defs
    if (items_only) {
        hide_other_items_recursively(doc->getRoot(), items_only, dkey);
    }

    Geom::IntRect final_bbox = Geom::IntRect::from_xywh(0, 0, width, height);
    drawing.update(final_bbox);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
        Inkscape::DrawingContext dc(surface, Geom::Point(0,0));

        // render items
        drawing.render(dc, final_bbox, Inkscape::DrawingItem::RENDER_BYPASS_CACHE);

        inkpb = new Inkscape::Pixbuf(surface);
    }
    else
    {
        long long size = (long long) height * (long long) cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
        g_warning("sp_generate_internal_bitmap: not enough memory to create pixel buffer. Need %lld.", size);
        cairo_surface_destroy(surface);
    }
    doc->getRoot()->invoke_hide(dkey);

//    gdk_pixbuf_save (pixbuf, "C:\\temp\\internal.jpg", "jpeg", NULL, "quality","100", NULL);

    return inkpb;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

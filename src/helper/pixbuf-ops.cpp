#define __SP_PIXBUF_OPS_C__

/*
 * Helpers for SPItem -> gdk_pixbuf related stuff
 *
 * Authors:
 *   John Cliff <simarilius@yahoo.com>
 *
 * Copyright (C) 2008 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <interface.h>
#include <libnr/nr-pixops.h>
#include <glib.h>
#include <glib/gmessages.h>
#include <png.h>
#include "png-write.h"
#include <display/nr-arena-item.h>
#include <display/nr-arena.h>
#include <document.h>
#include <sp-item.h>
#include <sp-root.h>
#include <sp-defs.h>

#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-matrix-ops.h"
#include "libnr/nr-translate-scale-ops.h"

#include "pixbuf-ops.h"

/**
 * Hide all items that are not listed in list, recursively, skipping groups and defs.
 */
static void
hide_other_items_recursively(SPObject *o, GSList *list, unsigned dkey)
{
    if ( SP_IS_ITEM(o)
         && !SP_IS_DEFS(o)
         && !SP_IS_ROOT(o)
         && !SP_IS_GROUP(o)
         && !g_slist_find(list, o) )
    {
        sp_item_invoke_hide(SP_ITEM(o), dkey);
    }

    // recurse
    if (!g_slist_find(list, o)) {
        for (SPObject *child = sp_object_first_child(o) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            hide_other_items_recursively(child, list, dkey);
        }
    }
}


// The following is a mutation of the flood fill code, the marker preview, and random other samplings.
// The dpi settings dont do anything yet, but I want them to, and was wanting to keep reasonably close
// to the call for the interface to the png writing.

bool
sp_export_jpg_file(SPDocument *doc, gchar const *filename,
                   double x0, double y0, double x1, double y1,
                   unsigned width, unsigned height, double xdpi, double ydpi,
                   unsigned long bgcolor, double quality,GSList *items)

{


      GdkPixbuf* pixbuf;
      pixbuf = sp_generate_internal_bitmap(doc, filename, x0, y0, x1, y1,
                                         width, height, xdpi, ydpi,
                                         bgcolor, items );


     gchar c[32];
     g_snprintf(c, 32, "%f", quality);
     gboolean saved = gdk_pixbuf_save (pixbuf, filename, "jpeg", NULL, "quality", c, NULL);
     g_free(c);
     if (saved) return true;
     else return false;
}

GdkPixbuf*
sp_generate_internal_bitmap(SPDocument *doc, gchar const */*filename*/,
                            double x0, double y0, double x1, double y1,
                            unsigned width, unsigned height, double /*xdpi*/, double /*ydpi*/,
                            unsigned long bgcolor,
                            GSList *items_only)

{


     /* Create new arena */
     NRArena *arena = NRArena::create();
     unsigned dkey = sp_item_display_key_new(1);

     sp_document_ensure_up_to_date (doc);

     NR::Rect screen=NR::Rect(NR::Point(x0,y0), NR::Point(x1, y1));

     double zoom_scale = 1.0;
     double padding = 1.0;

     width = (int)ceil(screen.extent(NR::X) * zoom_scale * padding);
     height = (int)ceil(screen.extent(NR::Y) * zoom_scale * padding);

     NR::Point origin(screen.min()[NR::X],
                      sp_document_height(doc) - screen.extent(NR::Y) - screen.min()[NR::Y]);

     origin[NR::X] = origin[NR::X] + (screen.extent(NR::X) * ((1 - padding) / 2));
     origin[NR::Y] = origin[NR::Y] + (screen.extent(NR::Y) * ((1 - padding) / 2));

     NR::scale scale(zoom_scale, zoom_scale);
     NR::Matrix affine = scale * NR::translate(-origin * scale);

     /* Create ArenaItems and set transform */
     NRArenaItem *root = sp_item_invoke_show(SP_ITEM(sp_document_root(doc)), arena, dkey, SP_ITEM_SHOW_DISPLAY);
     nr_arena_item_set_transform(NR_ARENA_ITEM(root), affine);

     NRGC gc(NULL);
     gc.transform.set_identity();

     // We show all and then hide all items we don't want, instead of showing only requested items,
     // because that would not work if the shown item references something in defs
     if (items_only) {
         hide_other_items_recursively(sp_document_root(doc), items_only, dkey);
     }

     NRRectL final_bbox;
     final_bbox.x0 = 0;
     final_bbox.y0 = 0;//row;
     final_bbox.x1 = width;
     final_bbox.y1 = height;//row + num_rows;

     nr_arena_item_invoke_update(root, &final_bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

     guchar *px = g_new(guchar, 4 * width * height);

     NRPixBlock B;
     nr_pixblock_setup_extern( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                               final_bbox.x0, final_bbox.y0, final_bbox.x1, final_bbox.y1,
                               px, 4 * width, FALSE, FALSE );

     unsigned char dtc[4];
     dtc[0] = NR_RGBA32_R(bgcolor);
     dtc[1] = NR_RGBA32_G(bgcolor);
     dtc[2] = NR_RGBA32_B(bgcolor);
     dtc[3] = NR_RGBA32_A(bgcolor);

     for (unsigned int fy = 0; fy < height; fy++) {
         guchar *p = NR_PIXBLOCK_PX(&B) + fy * B.rs;
         for (unsigned int fx = 0; fx < width; fx++) {
             for (int i = 0; i < 4; i++) {
                 *p++ = dtc[i];
             }
         }
     }

     nr_arena_item_invoke_render(NULL, root, &final_bbox, &B, NR_ARENA_ITEM_RENDER_NO_CACHE );

     GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(px, GDK_COLORSPACE_RGB,
                                          TRUE,
                                          8, width, height, width * 4,
                                          (GdkPixbufDestroyNotify)g_free,
                                          NULL);

//    gdk_pixbuf_save (pixbuf, "C:\\temp\\internal.jpg", "jpeg", NULL, "quality","100", NULL);

    return pixbuf;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

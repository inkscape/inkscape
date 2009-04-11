/** \file
 * SPIcon: Generic icon widget
 */
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <glib/gmem.h>
#include <gtk/gtk.h>
#include <gtkmm.h>

#include "path-prefix.h"
#include "preferences.h"
#include "inkscape.h"
#include "document.h"
#include "sp-item.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "io/sys.h"

#include "icon.h"

static void sp_icon_class_init(SPIconClass *klass);
static void sp_icon_init(SPIcon *icon);
static void sp_icon_dispose(GObject *object);

static void sp_icon_reset(SPIcon *icon);
static void sp_icon_clear(SPIcon *icon);

static void sp_icon_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void sp_icon_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static int sp_icon_expose(GtkWidget *widget, GdkEventExpose *event);

static void sp_icon_paint(SPIcon *icon, GdkRectangle const *area);

static void sp_icon_screen_changed( GtkWidget *widget, GdkScreen *previous_screen );
static void sp_icon_style_set( GtkWidget *widget, GtkStyle *previous_style );
static void sp_icon_theme_changed( SPIcon *icon );

static GdkPixbuf *sp_icon_image_load_pixmap(gchar const *name, unsigned lsize, unsigned psize);
static GdkPixbuf *sp_icon_image_load_svg(gchar const *name, GtkIconSize lsize, unsigned psize);

static void sp_icon_overlay_pixels( guchar *px, int width, int height, int stride,
                                    unsigned r, unsigned g, unsigned b );

static void injectCustomSize();

static GtkWidgetClass *parent_class;

static bool sizeDirty = true;

static bool sizeMapDone = false;
static GtkIconSize iconSizeLookup[] = {
    GTK_ICON_SIZE_INVALID,
    GTK_ICON_SIZE_MENU,
    GTK_ICON_SIZE_SMALL_TOOLBAR,
    GTK_ICON_SIZE_LARGE_TOOLBAR,
    GTK_ICON_SIZE_BUTTON,
    GTK_ICON_SIZE_DND,
    GTK_ICON_SIZE_DIALOG,
    GTK_ICON_SIZE_MENU, // for Inkscape::ICON_SIZE_DECORATION
};

class IconCacheItem
{
public:
    IconCacheItem( GtkIconSize lsize, GdkPixbuf* pb ) :
        _lsize( lsize ),
        _pb( pb )
    {}
    GtkIconSize _lsize;
    GdkPixbuf* _pb;
};

static Glib::RefPtr<Gtk::IconFactory> inkyIcons;

GType
sp_icon_get_type()
{
    //TODO: switch to GObject
    // GtkType and such calls were deprecated a while back with the
    // introduction of GObject as a separate layer, with GType instead. --JonCruz

    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPIconClass),
            NULL,
            NULL,
            (GClassInitFunc) sp_icon_class_init,
            NULL,
            NULL,
            sizeof(SPIcon),
            0,
            (GInstanceInitFunc) sp_icon_init,
            NULL
        };
        type = g_type_register_static(GTK_TYPE_WIDGET, "SPIcon", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_icon_class_init(SPIconClass *klass)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;

    parent_class = (GtkWidgetClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_icon_dispose;

    widget_class->size_request = sp_icon_size_request;
    widget_class->size_allocate = sp_icon_size_allocate;
    widget_class->expose_event = sp_icon_expose;
    widget_class->screen_changed = sp_icon_screen_changed;
    widget_class->style_set = sp_icon_style_set;
}


static void
sp_icon_init(SPIcon *icon)
{
    GTK_WIDGET_FLAGS(icon) |= GTK_NO_WINDOW;
    icon->lsize = Inkscape::ICON_SIZE_BUTTON;
    icon->psize = 0;
    icon->name = 0;
    icon->pb = 0;
    icon->pb_faded = 0;
}

static void
sp_icon_dispose(GObject *object)
{
    SPIcon *icon = SP_ICON(object);
    sp_icon_clear(icon);
    if ( icon->name ) {
        g_free( icon->name );
        icon->name = 0;
    }

    ((GObjectClass *) (parent_class))->dispose(object);
}

static void sp_icon_reset( SPIcon *icon ) {
    icon->psize = 0;
    sp_icon_clear(icon);
}

static void sp_icon_clear( SPIcon *icon ) {
    if (icon->pb) {
        g_object_unref(G_OBJECT(icon->pb));
        icon->pb = NULL;
    }
    if (icon->pb_faded) {
        g_object_unref(G_OBJECT(icon->pb_faded));
        icon->pb_faded = NULL;
    }
}

static void
sp_icon_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    SPIcon const *icon = SP_ICON(widget);

    int const size = ( icon->psize
                       ? icon->psize
                       : sp_icon_get_phys_size(icon->lsize) );
    requisition->width = size;
    requisition->height = size;
}

static void
sp_icon_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    widget->allocation = *allocation;

    if (GTK_WIDGET_DRAWABLE(widget)) {
        gtk_widget_queue_draw(widget);
    }
}

static int sp_icon_expose(GtkWidget *widget, GdkEventExpose *event)
{
    if ( GTK_WIDGET_DRAWABLE(widget) ) {
        SPIcon *icon = SP_ICON(widget);
        if ( !icon->pb ) {
            sp_icon_fetch_pixbuf( icon );
        }

        sp_icon_paint(SP_ICON(widget), &event->area);
    }

    return TRUE;
}

// PUBLIC CALL:
void sp_icon_fetch_pixbuf( SPIcon *icon )
{
    if ( icon ) {
        if ( !icon->pb ) {
            icon->psize = sp_icon_get_phys_size(icon->lsize);
            GtkIconTheme *theme = gtk_icon_theme_get_default();

            GdkPixbuf *pb = NULL;
            if (gtk_icon_theme_has_icon(theme, icon->name)) {
                pb = gtk_icon_theme_load_icon(theme, icon->name, icon->psize, (GtkIconLookupFlags) 0, NULL);
            }
            if (!pb) {
                pb = sp_icon_image_load_svg( icon->name, Inkscape::getRegisteredIconSize(icon->lsize), icon->psize );
                // if this was loaded from SVG, add it as a builtin icon
                gtk_icon_theme_add_builtin_icon(icon->name, icon->psize, pb);
            }
            if (!pb) {
                pb = sp_icon_image_load_pixmap( icon->name, icon->lsize, icon->psize );
            }
            if ( pb ) {
                icon->pb = pb;
                icon->pb_faded = gdk_pixbuf_copy(icon->pb);
                gdk_pixbuf_saturate_and_pixelate(icon->pb, icon->pb_faded, 0.5, TRUE);
            } else {
                /* TODO: We should do something more useful if we can't load the image. */
                g_warning ("failed to load icon '%s'", icon->name);
            }
        }
    }
}

static void sp_icon_screen_changed( GtkWidget *widget, GdkScreen *previous_screen )
{
    if ( GTK_WIDGET_CLASS( parent_class )->screen_changed ) {
        GTK_WIDGET_CLASS( parent_class )->screen_changed( widget, previous_screen );
    }
    SPIcon *icon = SP_ICON(widget);
    sp_icon_theme_changed(icon);
}

static void sp_icon_style_set( GtkWidget *widget, GtkStyle *previous_style )
{
    if ( GTK_WIDGET_CLASS( parent_class )->style_set ) {
        GTK_WIDGET_CLASS( parent_class )->style_set( widget, previous_style );
    }
    SPIcon *icon = SP_ICON(widget);
    sp_icon_theme_changed(icon);
}

static void sp_icon_theme_changed( SPIcon *icon )
{
    //g_message("Got a change bump for this icon");
    sizeDirty = true;
    sp_icon_reset(icon);
    gtk_widget_queue_draw( GTK_WIDGET(icon) );
}


static Glib::ustring icon_cache_key(gchar const *name, unsigned psize);
static GdkPixbuf *get_cached_pixbuf(Glib::ustring const &key);

static GtkWidget *
sp_icon_new_full( Inkscape::IconSize lsize, gchar const *name )
{
    GtkWidget *widget = 0;
    gint trySize = CLAMP( static_cast<gint>(lsize), 0, static_cast<gint>(G_N_ELEMENTS(iconSizeLookup) - 1) );
    if ( !sizeMapDone ) {
        injectCustomSize();
    }
    GtkIconSize mappedSize = iconSizeLookup[trySize];

    GtkStockItem stock;
    gboolean stockFound = gtk_stock_lookup( name, &stock );

    if ( stockFound ) {
        GtkWidget *img = gtk_image_new_from_stock( name, mappedSize );
        widget = GTK_WIDGET(img);
    }

    if ( !widget ) {
        //g_message("Creating an SPIcon instance for %s:%d", name, (int)lsize);
        SPIcon *icon = (SPIcon *)g_object_new(SP_TYPE_ICON, NULL);
        icon->lsize = lsize;
        icon->name = g_strdup(name);
        icon->psize = sp_icon_get_phys_size(lsize);

        widget = GTK_WIDGET(icon);
    }

    return widget;
}

GtkWidget *
sp_icon_new( Inkscape::IconSize lsize, gchar const *name )
{
    return sp_icon_new_full( lsize, name );
}

// PUBLIC CALL:
Gtk::Widget *sp_icon_get_icon( Glib::ustring const &oid, Inkscape::IconSize size )
{
    Gtk::Widget *result = 0;
    GtkWidget *widget = sp_icon_new_full( static_cast<Inkscape::IconSize>(Inkscape::getRegisteredIconSize(size)), oid.c_str() );

    if ( widget ) {
        if ( GTK_IS_IMAGE(widget) ) {
            GtkImage *img = GTK_IMAGE(widget);
            result = Glib::wrap( img );
        } else {
            result = Glib::wrap( widget );
        }
    }

    return result;
}

GtkIconSize
sp_icon_get_gtk_size(int size)
{
    static GtkIconSize sizemap[64] = {(GtkIconSize)0};
    size = CLAMP(size, 4, 63);
    if (!sizemap[size]) {
        static int count = 0;
        char c[64];
        g_snprintf(c, 64, "InkscapeIcon%d", count++);
        sizemap[size] = gtk_icon_size_register(c, size, size);
    }
    return sizemap[size];
}


static void injectCustomSize()
{
    // TODO - still need to handle the case of theme changes and resize, especially as we can't re-register a string.
    if ( !sizeMapDone )
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool dump = prefs->getBool( "/debug/icons/dumpDefault");
        gint width = 0;
        gint height = 0;
        if ( gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height ) ) {
            gint newWidth = ((width * 3) / 4);
            gint newHeight = ((height * 3) / 4);
            GtkIconSize newSizeEnum = gtk_icon_size_register( "inkscape-decoration", newWidth, newHeight );
            if ( newSizeEnum ) {
                if ( dump ) {
                    g_message("Registered (%d, %d) <= (%d, %d) as index %d", newWidth, newHeight, width, height, newSizeEnum);
                }
                guint index = static_cast<guint>(Inkscape::ICON_SIZE_DECORATION);
                if ( index < G_N_ELEMENTS(iconSizeLookup) ) {
                    iconSizeLookup[index] = newSizeEnum;
                } else if ( dump ) {
                    g_message("size lookup array too small to store entry");
                }
            }
        }
        sizeMapDone = true;
    }

    static bool hit = false;
    if ( !hit ) {
        hit = true;
        inkyIcons = Gtk::IconFactory::create();
        inkyIcons->add_default();
    }
}

GtkIconSize Inkscape::getRegisteredIconSize( Inkscape::IconSize size )
{
    GtkIconSize other = GTK_ICON_SIZE_MENU;
    injectCustomSize();
    size = CLAMP( size, Inkscape::ICON_SIZE_MENU, Inkscape::ICON_SIZE_DECORATION );
    if ( size == Inkscape::ICON_SIZE_DECORATION ) {
        other = gtk_icon_size_from_name("inkscape-decoration");
    } else {
        other = static_cast<GtkIconSize>(size);
    }

    return other;
}


// PUBLIC CALL:
int sp_icon_get_phys_size(int size)
{
    static bool init = false;
    static int lastSys[Inkscape::ICON_SIZE_DECORATION + 1];
    static int vals[Inkscape::ICON_SIZE_DECORATION + 1];

    size = CLAMP( size, GTK_ICON_SIZE_MENU, Inkscape::ICON_SIZE_DECORATION );

    if ( !sizeMapDone ) {
        injectCustomSize();
    }

    if ( sizeDirty && init ) {
        GtkIconSize const gtkSizes[] = {
            GTK_ICON_SIZE_MENU,
            GTK_ICON_SIZE_SMALL_TOOLBAR,
            GTK_ICON_SIZE_LARGE_TOOLBAR,
            GTK_ICON_SIZE_BUTTON,
            GTK_ICON_SIZE_DND,
            GTK_ICON_SIZE_DIALOG,
            static_cast<guint>(Inkscape::ICON_SIZE_DECORATION) < G_N_ELEMENTS(iconSizeLookup) ?
                iconSizeLookup[static_cast<int>(Inkscape::ICON_SIZE_DECORATION)] :
                GTK_ICON_SIZE_MENU
        };
        for (unsigned i = 0; i < G_N_ELEMENTS(gtkSizes) && init; ++i) {
            guint const val_ix = (gtkSizes[i] <= GTK_ICON_SIZE_DIALOG) ? (guint)gtkSizes[i] : (guint)Inkscape::ICON_SIZE_DECORATION;

            g_assert( val_ix < G_N_ELEMENTS(vals) );

            gint width = 0;
            gint height = 0;
            if ( gtk_icon_size_lookup(gtkSizes[i], &width, &height ) ) {
                init &= (lastSys[val_ix] == std::max(width, height));
            }
        }
    }

    if ( !init ) {
        sizeDirty = false;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool dump = prefs->getBool("/debug/icons/dumpDefault");

        if ( dump ) {
            g_message( "Default icon sizes:" );
        }
        memset( vals, 0, sizeof(vals) );
        memset( lastSys, 0, sizeof(lastSys) );
        GtkIconSize const gtkSizes[] = {
            GTK_ICON_SIZE_MENU,
            GTK_ICON_SIZE_SMALL_TOOLBAR,
            GTK_ICON_SIZE_LARGE_TOOLBAR,
            GTK_ICON_SIZE_BUTTON,
            GTK_ICON_SIZE_DND,
            GTK_ICON_SIZE_DIALOG,
            static_cast<guint>(Inkscape::ICON_SIZE_DECORATION) < G_N_ELEMENTS(iconSizeLookup) ?
                iconSizeLookup[static_cast<int>(Inkscape::ICON_SIZE_DECORATION)] :
                GTK_ICON_SIZE_MENU
        };
        gchar const *const names[] = {
            "GTK_ICON_SIZE_MENU",
            "GTK_ICON_SIZE_SMALL_TOOLBAR",
            "GTK_ICON_SIZE_LARGE_TOOLBAR",
            "GTK_ICON_SIZE_BUTTON",
            "GTK_ICON_SIZE_DND",
            "GTK_ICON_SIZE_DIALOG",
            "inkscape-decoration"
        };

        GtkWidget *icon = (GtkWidget *)g_object_new(SP_TYPE_ICON, NULL);

        for (unsigned i = 0; i < G_N_ELEMENTS(gtkSizes); ++i) {
            guint const val_ix = (gtkSizes[i] <= GTK_ICON_SIZE_DIALOG) ? (guint)gtkSizes[i] : (guint)Inkscape::ICON_SIZE_DECORATION;

            g_assert( val_ix < G_N_ELEMENTS(vals) );

            gint width = 0;
            gint height = 0;
            bool used = false;
            if ( gtk_icon_size_lookup(gtkSizes[i], &width, &height ) ) {
                vals[val_ix] = std::max(width, height);
                lastSys[val_ix] = vals[val_ix];
                used = true;
            }
            if (dump) {
                g_message(" =--  %u  size:%d  %c(%d, %d)   '%s'",
                          i, gtkSizes[i],
                          ( used ? ' ' : 'X' ), width, height, names[i]);
            }
            gchar const *id = GTK_STOCK_OPEN;
            GdkPixbuf *pb = gtk_widget_render_icon( icon, id, gtkSizes[i], NULL);
            if (pb) {
                width = gdk_pixbuf_get_width(pb);
                height = gdk_pixbuf_get_height(pb);
                int newSize = std::max( width, height );
                // TODO perhaps check a few more stock icons to get a range on sizes.
                if ( newSize > 0 ) {
                    vals[val_ix] = newSize;
                }
                if (dump) {
                    g_message("      %u  size:%d   (%d, %d)", i, gtkSizes[i], width, height);
                }

                g_object_unref(G_OBJECT(pb));
            }
        }
        //g_object_unref(icon);
        init = true;
    }

    // Fixup workaround
    if ((size == GTK_ICON_SIZE_MENU) || (size == GTK_ICON_SIZE_SMALL_TOOLBAR) || (size == GTK_ICON_SIZE_LARGE_TOOLBAR)) {
        gint width = 0;
        gint height = 0;
        if ( gtk_icon_size_lookup( static_cast<GtkIconSize>(size), &width, &height ) ) {
            vals[size] = std::max( width, height );
        }
    }

    return vals[size];
}

static void sp_icon_paint(SPIcon *icon, GdkRectangle const *area)
{
    GtkWidget &widget = *GTK_WIDGET(icon);

    GdkPixbuf *image = GTK_WIDGET_IS_SENSITIVE(&widget) ? icon->pb : icon->pb_faded;

    if (image) {
        int const padx = ( ( widget.allocation.width > icon->psize )
                           ? ( widget.allocation.width - icon->psize ) / 2
                           : 0 );
        int const pady = ( ( widget.allocation.height > icon->psize )
                           ? ( widget.allocation.height - icon->psize ) / 2
                           : 0 );

        int const x0 = std::max(area->x, widget.allocation.x + padx);
        int const y0 = std::max(area->y, widget.allocation.y + pady);
        int const x1 = std::min(area->x + area->width,  widget.allocation.x + padx + static_cast<int>(icon->psize) );
        int const y1 = std::min(area->y + area->height, widget.allocation.y + pady + static_cast<int>(icon->psize) );

        int width = x1 - x0;
        int height = y1 - y0;
        // Limit drawing to when we actually have something. Avoids some crashes.
        if ( (width > 0) && (height > 0) ) {
            gdk_draw_pixbuf(GDK_DRAWABLE(widget.window), NULL, image,
                            x0 - widget.allocation.x - padx,
                            y0 - widget.allocation.y - pady,
                            x0, y0,
                            width, height,
                            GDK_RGB_DITHER_NORMAL, x0, y0);
        }
    }
}

GdkPixbuf *sp_icon_image_load_pixmap(gchar const *name, unsigned /*lsize*/, unsigned psize)
{
    gchar *path = (gchar *) g_strdup_printf("%s/%s.png", INKSCAPE_PIXMAPDIR, name);
    // TODO: bulia, please look over
    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    gchar *localFilename = g_filename_from_utf8( path,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
    GdkPixbuf *pb = gdk_pixbuf_new_from_file(localFilename, NULL);
    g_free(localFilename);
    g_free(path);
    if (!pb) {
        path = (gchar *) g_strdup_printf("%s/%s.xpm", INKSCAPE_PIXMAPDIR, name);
        // TODO: bulia, please look over
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error = NULL;
        gchar *localFilename = g_filename_from_utf8( path,
                                                     -1,
                                                     &bytesRead,
                                                     &bytesWritten,
                                                     &error);
        pb = gdk_pixbuf_new_from_file(localFilename, NULL);
        g_free(localFilename);
        g_free(path);
    }

    if (pb) {
        if (!gdk_pixbuf_get_has_alpha(pb)) {
            gdk_pixbuf_add_alpha(pb, FALSE, 0, 0, 0);
        }

        if ( ( static_cast<unsigned>(gdk_pixbuf_get_width(pb)) != psize )
             || ( static_cast<unsigned>(gdk_pixbuf_get_height(pb)) != psize ) ) {
            GdkPixbuf *spb = gdk_pixbuf_scale_simple(pb, psize, psize, GDK_INTERP_HYPER);
            g_object_unref(G_OBJECT(pb));
            pb = spb;
        }
    }

    return pb;
}

// takes doc, root, icon, and icon name to produce pixels
extern "C" guchar *
sp_icon_doc_icon( SPDocument *doc, NRArenaItem *root,
                  gchar const *name, unsigned psize )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool const dump = prefs->getBool("/debug/icons/dumpSvg");
    guchar *px = NULL;

    if (doc) {
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_ITEM(object)) {
            /* Find bbox in document */
            Geom::Matrix const i2doc(sp_item_i2doc_affine(SP_ITEM(object)));
            Geom::OptRect dbox = SP_ITEM(object)->getBounds(i2doc);

            if ( SP_OBJECT_PARENT(object) == NULL )
            {
                dbox = Geom::Rect(Geom::Point(0, 0),
                                Geom::Point(sp_document_width(doc), sp_document_height(doc)));
            }

            /* This is in document coordinates, i.e. pixels */
            if ( dbox ) {
                NRGC gc(NULL);
                /* Update to renderable state */
                double sf = 1.0;
                nr_arena_item_set_transform(root, (Geom::Matrix)Geom::Scale(sf, sf));
                gc.transform.setIdentity();
                nr_arena_item_invoke_update( root, NULL, &gc,
                                             NR_ARENA_ITEM_STATE_ALL,
                                             NR_ARENA_ITEM_STATE_NONE );
                /* Item integer bbox in points */
                NRRectL ibox;
                ibox.x0 = (int) floor(sf * dbox->min()[Geom::X] + 0.5);
                ibox.y0 = (int) floor(sf * dbox->min()[Geom::Y] + 0.5);
                ibox.x1 = (int) floor(sf * dbox->max()[Geom::X] + 0.5);
                ibox.y1 = (int) floor(sf * dbox->max()[Geom::Y] + 0.5);

                if ( dump ) {
                    g_message( "   box    --'%s'  (%f,%f)-(%f,%f)", name, (double)ibox.x0, (double)ibox.y0, (double)ibox.x1, (double)ibox.y1 );
                }

                /* Find button visible area */
                int width = ibox.x1 - ibox.x0;
                int height = ibox.y1 - ibox.y0;

                if ( dump ) {
                    g_message( "   vis    --'%s'  (%d,%d)", name, width, height );
                }

                {
                    int block = std::max(width, height);
                    if (block != static_cast<int>(psize) ) {
                        if ( dump ) {
                            g_message("      resizing" );
                        }
                        sf = (double)psize / (double)block;

                        nr_arena_item_set_transform(root, (Geom::Matrix)Geom::Scale(sf, sf));
                        gc.transform.setIdentity();
                        nr_arena_item_invoke_update( root, NULL, &gc,
                                                     NR_ARENA_ITEM_STATE_ALL,
                                                     NR_ARENA_ITEM_STATE_NONE );
                        /* Item integer bbox in points */
                        ibox.x0 = (int) floor(sf * dbox->min()[Geom::X] + 0.5);
                        ibox.y0 = (int) floor(sf * dbox->min()[Geom::Y] + 0.5);
                        ibox.x1 = (int) floor(sf * dbox->max()[Geom::X] + 0.5);
                        ibox.y1 = (int) floor(sf * dbox->max()[Geom::Y] + 0.5);

                        if ( dump ) {
                            g_message( "   box2   --'%s'  (%f,%f)-(%f,%f)", name, (double)ibox.x0, (double)ibox.y0, (double)ibox.x1, (double)ibox.y1 );
                        }

                        /* Find button visible area */
                        width = ibox.x1 - ibox.x0;
                        height = ibox.y1 - ibox.y0;
                        if ( dump ) {
                            g_message( "   vis2   --'%s'  (%d,%d)", name, width, height );
                        }
                    }
                }

                int dx, dy;
                //dx = (psize - width) / 2;
                //dy = (psize - height) / 2;
                dx=dy=psize;
                dx=(dx-width)/2; // watch out for psize, since 'unsigned'-'signed' can cause problems if the result is negative
                dy=(dy-height)/2;
                NRRectL area;
                area.x0 = ibox.x0 - dx;
                area.y0 = ibox.y0 - dy;
                area.x1 = area.x0 + psize;
                area.y1 = area.y0 + psize;
                /* Actual renderable area */
                NRRectL ua;
                ua.x0 = MAX(ibox.x0, area.x0);
                ua.y0 = MAX(ibox.y0, area.y0);
                ua.x1 = MIN(ibox.x1, area.x1);
                ua.y1 = MIN(ibox.y1, area.y1);

                if ( dump ) {
                    g_message( "   area   --'%s'  (%f,%f)-(%f,%f)", name, (double)area.x0, (double)area.y0, (double)area.x1, (double)area.y1 );
                    g_message( "   ua     --'%s'  (%f,%f)-(%f,%f)", name, (double)ua.x0, (double)ua.y0, (double)ua.x1, (double)ua.y1 );
                }
                /* Set up pixblock */
                px = g_new(guchar, 4 * psize * psize);
                memset(px, 0x00, 4 * psize * psize);
                /* Render */
                NRPixBlock B;
                nr_pixblock_setup_extern( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                                          ua.x0, ua.y0, ua.x1, ua.y1,
                                          px + 4 * psize * (ua.y0 - area.y0) +
                                          4 * (ua.x0 - area.x0),
                                          4 * psize, FALSE, FALSE );
                nr_arena_item_invoke_render(NULL, root, &ua, &B,
                                             NR_ARENA_ITEM_RENDER_NO_CACHE );
                nr_pixblock_release(&B);

                bool useOverlay = prefs->getBool("/debug/icons/overlaySvg");
                if ( useOverlay ) {
                    sp_icon_overlay_pixels( px, psize, psize, 4 * psize, 0x00, 0x00, 0xff );
                }
            }
        }
    }

    return px;
} // end of sp_icon_doc_icon()



struct svg_doc_cache_t
{
    SPDocument *doc;
    NRArenaItem *root;
};

static std::map<Glib::ustring, svg_doc_cache_t *> doc_cache;
static std::map<Glib::ustring, GdkPixbuf *> pb_cache;

Glib::ustring icon_cache_key(gchar const *name, unsigned psize)
{
    Glib::ustring key=name;
    key += ":";
    key += psize;
    return key;
}

GdkPixbuf *get_cached_pixbuf(Glib::ustring const &key) {
    std::map<Glib::ustring, GdkPixbuf *>::iterator found = pb_cache.find(key);
    if ( found != pb_cache.end() ) {
        return found->second;
    }
    return NULL;
}

static std::list<gchar*> &icons_svg_paths()
{
    static std::list<gchar *> sources;
    static bool initialized = false;
    if (!initialized) {
        // Fall back from user prefs dir into system locations.
        gchar *userdir = profile_path("icons");
        sources.push_back(g_build_filename(userdir,"icons.svg", NULL));
        sources.push_back(g_build_filename(INKSCAPE_PIXMAPDIR, "icons.svg", NULL));
        g_free(userdir);
        initialized = true;
    }
    return sources;
}

// this function renders icons from icons.svg and returns the pixels.
static guchar *load_svg_pixels(gchar const *name,
                               unsigned /*lsize*/, unsigned psize)
{
    SPDocument *doc = NULL;
    NRArenaItem *root = NULL;
    svg_doc_cache_t *info = NULL;

    std::list<gchar *> &sources = icons_svg_paths();

    // Try each document in turn until we successfully load the icon from one
    guchar *px=NULL;
    for (std::list<gchar*>::iterator i = sources.begin(); i != sources.end() && !px; ++i) {
        gchar *doc_filename = *i;

        // Did we already load this doc?
        Glib::ustring key(doc_filename);
        info = 0;
        {
            std::map<Glib::ustring, svg_doc_cache_t *>::iterator i = doc_cache.find(key);
            if ( i != doc_cache.end() ) {
                info = i->second;
            }
        }

        /* Try to load from document. */
        if (!info &&
            Inkscape::IO::file_test( doc_filename, G_FILE_TEST_IS_REGULAR ) &&
            (doc = sp_document_new( doc_filename, FALSE )) ) {
            
            //g_message("Loaded icon file %s", doc_filename);
            // prep the document
            sp_document_ensure_up_to_date(doc);
            /* Create new arena */
            NRArena *arena = NRArena::create();
            /* Create ArenaItem and set transform */
            unsigned visionkey = sp_item_display_key_new(1);
            /* fixme: Memory manage root if needed (Lauris) */
            root = sp_item_invoke_show( SP_ITEM(SP_DOCUMENT_ROOT(doc)),
                                        arena, visionkey, SP_ITEM_SHOW_DISPLAY );

            // store into the cache
            info = new svg_doc_cache_t;
            g_assert(info);

            info->doc=doc;
            info->root=root;
            doc_cache[key]=info;
        }
        if (info) {
            doc=info->doc;
            root=info->root;
        }

        // move on to the next document if we couldn't get anything
        if (!info && !doc) continue;

        px = sp_icon_doc_icon( doc, root, name, psize );
        //if (px) g_message("Found icon %s in %s", name, doc_filename);
    }

    //if (!px) g_message("Not found icon %s", name);
    return px;
}

static GdkPixbuf *sp_icon_image_load_svg(gchar const *name, GtkIconSize lsize, unsigned psize)
{
    Glib::ustring key = icon_cache_key(name, psize);

    // did we already load this icon at this scale/size?
    GdkPixbuf* pb = get_cached_pixbuf(key);
    if (!pb) {
        guchar *px = load_svg_pixels(name, lsize, psize);
        if (px) {
            pb = gdk_pixbuf_new_from_data(px, GDK_COLORSPACE_RGB, TRUE, 8,
                                          psize, psize, psize * 4,
                                          (GdkPixbufDestroyNotify)g_free, NULL);
            pb_cache[key] = pb;
        }
    }

    if ( pb ) {
        // increase refcount since we're handing out ownership
        g_object_ref(G_OBJECT(pb));
    }
    return pb;
}

void sp_icon_overlay_pixels(guchar *px, int width, int height, int stride,
                            unsigned r, unsigned g, unsigned b)
{
    int bytesPerPixel = 4;
    int spacing = 4;
    for ( int y = 0; y < height; y += spacing ) {
        guchar *ptr = px + y * stride;
        for ( int x = 0; x < width; x += spacing ) {
            *(ptr++) = r;
            *(ptr++) = g;
            *(ptr++) = b;
            *(ptr++) = 0xff;

            ptr += bytesPerPixel * (spacing - 1);
        }
    }

    if ( width > 1 && height > 1 ) {
        // point at the last pixel
        guchar *ptr = px + ((height-1) * stride) + ((width - 1) * bytesPerPixel);

        if ( width > 2 ) {
            px[4] = r;
            px[5] = g;
            px[6] = b;
            px[7] = 0xff;

            ptr[-12] = r;
            ptr[-11] = g;
            ptr[-10] = b;
            ptr[-9] = 0xff;
        }

        ptr[-4] = r;
        ptr[-3] = g;
        ptr[-2] = b;
        ptr[-1] = 0xff;

        px[0 + stride] = r;
        px[1 + stride] = g;
        px[2 + stride] = b;
        px[3 + stride] = 0xff;

        ptr[0 - stride] = r;
        ptr[1 - stride] = g;
        ptr[2 - stride] = b;
        ptr[3 - stride] = 0xff;

        if ( height > 2 ) {
            ptr[0 - stride * 3] = r;
            ptr[1 - stride * 3] = g;
            ptr[2 - stride * 3] = b;
            ptr[3 - stride * 3] = 0xff;
        }
    }
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

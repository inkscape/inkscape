/** \file
 * SPIcon: Generic icon widget
 */
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/icontheme.h>
#include <cstring>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <2geom/transforms.h>

#include "path-prefix.h"
#include "preferences.h"
#include "inkscape.h"
#include "document.h"
#include "sp-item.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "display/drawing.h"
#include "io/sys.h"
#include "sp-root.h"
#include "util/units.h"

#include "icon.h"
#include "ui/icon-names.h"

struct IconImpl {
    static GtkWidget *newFull( Inkscape::IconSize lsize, gchar const *name );

    static void dispose(GObject *object);

    static void reset(SPIcon *icon);
    static void clear(SPIcon *icon);

    static void sizeRequest(GtkWidget *widget, GtkRequisition *requisition);
    
    static void getPreferredWidth(GtkWidget *widget, 
		    gint *minimal_width,
		    gint *natural_width);

    static void getPreferredHeight(GtkWidget *widget, 
		    gint *minimal_height,
		    gint *natural_height);

    static void sizeAllocate(GtkWidget *widget, GtkAllocation *allocation);
    static gboolean draw(GtkWidget *widget, cairo_t *cr);

#if !GTK_CHECK_VERSION(3,0,0)
    static gboolean expose(GtkWidget *widget, GdkEventExpose *event);
#endif

    static void screenChanged( GtkWidget *widget, GdkScreen *previous_screen );
    static void styleSet( GtkWidget *widget, GtkStyle *previous_style );
    static void themeChanged( SPIcon *icon );

    static int getPhysSize(int size);
    static void fetchPixbuf( SPIcon *icon );

    static gboolean prerenderTask(gpointer data);
    static void addPreRender( GtkIconSize lsize, gchar const *name );
    static GdkPixbuf* renderup( gchar const* name, Inkscape::IconSize lsize, unsigned psize );


    static GdkPixbuf *loadPixmap(gchar const *name, unsigned lsize, unsigned psize);
    static GdkPixbuf *loadSvg(std::list<Glib::ustring> const &names, GtkIconSize lsize, unsigned psize);

    static void overlayPixels( guchar *px, int width, int height, int stride,
                               unsigned r, unsigned g, unsigned b );

    static void injectCustomSize();

    static void imageMapCB(GtkWidget* widget, gpointer user_data);
    static void imageMapNamedCB(GtkWidget* widget, gpointer user_data);
    static bool prerenderIcon(gchar const *name, GtkIconSize lsize, unsigned psize);


    static std::list<gchar*> &icons_svg_paths();
    static guchar *load_svg_pixels(std::list<Glib::ustring> const &names,
                                   unsigned psize, unsigned &stride);

    static std::string fileEscape( std::string const & str );
 
    static void validateCache();
    static void setupLegacyNaming();

private:
    static const std::string magicNumber;
    static std::map<Glib::ustring, Glib::ustring> legacyNames;
};

const std::string IconImpl::magicNumber = "1.0";
std::map<Glib::ustring, Glib::ustring> IconImpl::legacyNames;


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

static std::map<Glib::ustring, std::vector<IconCacheItem> > iconSetCache;
static std::set<Glib::ustring> internalNames;

G_DEFINE_TYPE(SPIcon, sp_icon, GTK_TYPE_WIDGET);

static void
sp_icon_class_init(SPIconClass *klass)
{
    GObjectClass   *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = IconImpl::dispose;

#if GTK_CHECK_VERSION(3,0,0)
    widget_class->get_preferred_width = IconImpl::getPreferredWidth;
    widget_class->get_preferred_height = IconImpl::getPreferredHeight;
    widget_class->draw = IconImpl::draw;
#else
    widget_class->size_request = IconImpl::sizeRequest;
    widget_class->expose_event = IconImpl::expose;
#endif
    widget_class->size_allocate = IconImpl::sizeAllocate;
    widget_class->screen_changed = IconImpl::screenChanged;
    widget_class->style_set = IconImpl::styleSet;
}

static void
sp_icon_init(SPIcon *icon)
{
    gtk_widget_set_has_window (GTK_WIDGET (icon), FALSE);
    icon->lsize = Inkscape::ICON_SIZE_BUTTON;
    icon->psize = 0;
    icon->name = NULL;
    icon->pb = NULL;
}

void IconImpl::dispose(GObject *object)
{
    SPIcon *icon = SP_ICON(object);
    clear(icon);
    if ( icon->name ) {
        g_free( icon->name );
        icon->name = NULL;
    }

    (G_OBJECT_CLASS(sp_icon_parent_class))->dispose(object);
}

void IconImpl::reset( SPIcon *icon )
{
    icon->psize = 0;
    clear(icon);
}

void IconImpl::clear( SPIcon *icon )
{
    if (icon->pb) {
        g_object_unref(G_OBJECT(icon->pb));
        icon->pb = NULL;
    }
}

void IconImpl::sizeRequest(GtkWidget *widget, GtkRequisition *requisition)
{
    SPIcon const *icon = SP_ICON(widget);

    int const size = ( icon->psize
                       ? icon->psize
                       : getPhysSize(icon->lsize) );
    requisition->width = size;
    requisition->height = size;
}

void IconImpl::getPreferredWidth(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	sizeRequest(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

void IconImpl::getPreferredHeight(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	sizeRequest(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}

void IconImpl::sizeAllocate(GtkWidget *widget, GtkAllocation *allocation)
{
    gtk_widget_set_allocation(widget, allocation);

    if (gtk_widget_is_drawable(widget)) {
        gtk_widget_queue_draw(widget);
    }
}

// GTK3 Only, Doesn't actually seem to be used.
gboolean IconImpl::draw(GtkWidget *widget, cairo_t* cr)
{
    SPIcon *icon = SP_ICON(widget);
    if ( !icon->pb ) {
        fetchPixbuf( icon );
    }
    
    GdkPixbuf *image = icon->pb;
    bool unref_image = false;

    /* copied from the expose function of GtkImage */
#if GTK_CHECK_VERSION(3,0,0)
    if (gtk_widget_get_state_flags (GTK_WIDGET(icon)) != GTK_STATE_FLAG_NORMAL && image) {
#else
    if (gtk_widget_get_state (GTK_WIDGET(icon)) != GTK_STATE_NORMAL && image) {
        std::cerr << "IconImpl::draw: Ooops! It is called in GTK2" << std::endl;
#endif
        std::cerr << "IconImpl::draw: No image, creating fallback" << std::endl;

#if GTK_CHECK_VERSION(3,0,0)
        // image = gtk_render_icon_pixbuf(gtk_widget_get_style_context(widget), 
        //                                source, 
        //                                (GtkIconSize)-1);

        // gtk_render_icon_pixbuf deprecated, replaced by:
        GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
        image = gtk_icon_theme_load_icon (icon_theme,
                                          "gtk-image",
                                          32,
                                          (GtkIconLookupFlags)0,
                                          NULL);
#else
        GtkIconSource *source = gtk_icon_source_new();
        gtk_icon_source_set_pixbuf(source, icon->pb);
        gtk_icon_source_set_size(source, GTK_ICON_SIZE_SMALL_TOOLBAR); // note: this is boilerplate and not used
        gtk_icon_source_set_size_wildcarded(source, FALSE);
        image = gtk_style_render_icon(gtk_widget_get_style(widget), source, 
			gtk_widget_get_direction(widget),
			(GtkStateType) gtk_widget_get_state(widget), 
			(GtkIconSize)-1, widget, "gtk-image");
        gtk_icon_source_free(source);
#endif

        unref_image = true;
    }

    if (image) {
        GtkAllocation allocation;
	GtkRequisition requisition;
	gtk_widget_get_allocation(widget, &allocation);

#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_get_preferred_size(widget, &requisition, NULL);
#else
	gtk_widget_get_requisition(widget, &requisition);
#endif

        int x = floor(allocation.x + ((allocation.width - requisition.width) * 0.5));
        int y = floor(allocation.y + ((allocation.height - requisition.height) * 0.5));
        int width = gdk_pixbuf_get_width(image);
        int height = gdk_pixbuf_get_height(image);
        // Limit drawing to when we actually have something. Avoids some crashes.
        if ( (width > 0) && (height > 0) ) {
		gdk_cairo_set_source_pixbuf(cr, image, x, y);
		cairo_paint(cr);
        }
    }

    if (unref_image) {
        g_object_unref(G_OBJECT(image));
    }
    
    return TRUE;
}

#if !GTK_CHECK_VERSION(3,0,0)
gboolean IconImpl::expose(GtkWidget *widget, GdkEventExpose * /*event*/)
{
    gboolean result = TRUE;

    if (gtk_widget_is_drawable(widget)) {
	cairo_t * cr = gdk_cairo_create(gtk_widget_get_window(widget));
        result = draw(widget, cr);
	cairo_destroy(cr);
    }

    return result;
}
#endif

// PUBLIC CALL:
void sp_icon_fetch_pixbuf( SPIcon *icon )
{
    return IconImpl::fetchPixbuf(icon);
}

void IconImpl::fetchPixbuf( SPIcon *icon )
{
    if ( icon ) {
        if ( !icon->pb ) {
            icon->psize = getPhysSize(icon->lsize);
            icon->pb = renderup(icon->name, icon->lsize, icon->psize);
        }
    }
}

GdkPixbuf* IconImpl::renderup( gchar const* name, Inkscape::IconSize lsize, unsigned psize ) {
    GtkIconTheme *theme = gtk_icon_theme_get_default();

    GdkPixbuf *pb = NULL;
    if (gtk_icon_theme_has_icon(theme, name)) {
        pb = gtk_icon_theme_load_icon(theme, name, psize, (GtkIconLookupFlags) 0, NULL);
    }
    if (!pb) {
        std::list<Glib::ustring> names;
        names.push_back(name);
        if ( legacyNames.find(name) != legacyNames.end() ) {
            if ( Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg") ) {
                g_message("Checking fallback [%s]->[%s]", name, legacyNames[name].c_str());
            }
            names.push_back(legacyNames[name]);
        }

        pb = loadSvg( names, Inkscape::getRegisteredIconSize(lsize), psize );

        // if this was loaded from SVG, add it as a builtin icon
        if (pb) {
            gtk_icon_theme_add_builtin_icon(name, psize, pb);
        }
    }
    if (!pb) {
        pb = loadPixmap( name, lsize, psize );
    }
    if ( !pb ) {
        // TODO: We should do something more useful if we can't load the image.
        g_warning ("failed to load icon '%s'", name);
    }
    return pb;
}

void IconImpl::screenChanged( GtkWidget *widget, GdkScreen *previous_screen )
{
    if ( GTK_WIDGET_CLASS( sp_icon_parent_class )->screen_changed ) {
        GTK_WIDGET_CLASS( sp_icon_parent_class )->screen_changed( widget, previous_screen );
    }
    SPIcon *icon = SP_ICON(widget);
    themeChanged(icon);
}

void IconImpl::styleSet( GtkWidget *widget, GtkStyle *previous_style )
{
    if ( GTK_WIDGET_CLASS( sp_icon_parent_class )->style_set ) {
        GTK_WIDGET_CLASS( sp_icon_parent_class )->style_set( widget, previous_style );
    }
    SPIcon *icon = SP_ICON(widget);
    themeChanged(icon);
}

void IconImpl::themeChanged( SPIcon *icon )
{
    bool const dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg");
    if ( dump ) {
        g_message("Got a change bump for this icon");
    }
    sizeDirty = true;
    reset(icon);
    gtk_widget_queue_draw( GTK_WIDGET(icon) );
}

std::string IconImpl::fileEscape( std::string const & str )
{
    std::string result;
    result.reserve(str.size());
    for ( size_t i = 0; i < str.size(); ++i ) {
        char ch = str[i];
        if ( (0x20 <= ch) && !(0x80 & ch) ) {
            result += ch;
        } else {
            result += "\\x";
            gchar *tmp = g_strdup_printf("%02X", (0x0ff & ch));
            result += tmp;
            g_free(tmp);
        }
    }
    return result;
}

static bool isSizedSubdir( std::string const &name )
{
    bool isSized = false;
    if ( (name.size() > 2) && (name.size() & 1) ) { // needs to be an odd length 3 or more
        size_t mid = (name.size() - 1) / 2;
        if ( (name[mid] == 'x') && (name.substr(0, mid) == name.substr(mid + 1)) ) {
            isSized = true;
            for ( size_t i = 0; (i < mid) && isSized; ++i ) {
                isSized &= g_ascii_isdigit(name[i]);
            }
        }
    }
    return isSized;
}

void IconImpl::validateCache()
{
    std::list<gchar *> &sources = icons_svg_paths();
    std::string iconCacheDir = Glib::build_filename(Glib::build_filename(Glib::get_user_cache_dir(), "inkscape"), "icons");
    std::string iconCacheFile = Glib::build_filename( iconCacheDir, "cache.info" );

    std::vector<std::string> filesFound;

    for (std::list<gchar*>::iterator i = sources.begin(); i != sources.end(); ++i) {
        gchar const* potentialFile = *i;
        if ( Glib::file_test(potentialFile, Glib::FILE_TEST_EXISTS) && Glib::file_test(potentialFile, Glib::FILE_TEST_IS_REGULAR) ) {
            filesFound.push_back(*i);
        }
    }

    unsigned long lastSeen = 0;
    std::ostringstream out;
    out << "Inkscape cache v" << std::hex << magicNumber << std::dec << std::endl;
    out << "Sourcefiles: " << filesFound.size() << std::endl; 
    for ( std::vector<std::string>::iterator it = filesFound.begin(); it != filesFound.end(); ++it ) {
        GStatBuf st;
        memset(&st, 0, sizeof(st));
        if ( !g_stat(it->c_str(), &st) ) {
            unsigned long when = st.st_mtime;
            lastSeen = std::max(lastSeen, when);
            out << std::hex << when << std::dec << " " << fileEscape(*it) << std::endl;
        } else {
            out << "0 " << fileEscape(*it) << std::endl;
        }
    }
    std::string wanted = out.str();

    std::string present;
    {
        gchar *contents = NULL;
        if ( g_file_get_contents(iconCacheFile.c_str(), &contents, 0, 0) ) {
            if ( contents ) {
                present = contents;
            }
            g_free(contents);
            contents = NULL;
        }
    }
    bool cacheValid = (present == wanted);

    if ( cacheValid ) {
        // Check if any cached rasters are out of date
        Glib::Dir dir(iconCacheDir);
        for ( Glib::DirIterator it = dir.begin(); cacheValid && (it != dir.end()); ++it ) {
            if ( isSizedSubdir(*it) ) {
                std::string subdirName = Glib::build_filename( iconCacheDir, *it );
                if ( Glib::file_test(subdirName, Glib::FILE_TEST_IS_DIR) ) {
                    Glib::Dir subdir(subdirName);
                    for ( Glib::DirIterator subit = subdir.begin(); cacheValid && (subit != subdir.end()); ++subit ) {
                        std::string fullpath = Glib::build_filename( subdirName, *subit );
                        if ( Glib::file_test(fullpath, Glib::FILE_TEST_EXISTS) && !Glib::file_test(fullpath, Glib::FILE_TEST_IS_DIR) ) {
                            GStatBuf st;
                            memset(&st, 0, sizeof(st));
                            if ( !g_stat(fullpath.c_str(), &st) ) {
                                unsigned long when = st.st_mtime;
                                if ( when < lastSeen ) {
                                    cacheValid = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !cacheValid ) {
        if ( Glib::file_test(iconCacheDir, Glib::FILE_TEST_EXISTS) ) {
            // Purge existing icons, but not possible future sub-directories.
            if ( Glib::file_test(iconCacheDir, Glib::FILE_TEST_IS_DIR) ) {
                Glib::Dir dir(iconCacheDir);
                for ( Glib::DirIterator it = dir.begin(); it != dir.end(); ++it ) {
                    if ( isSizedSubdir(*it) ) {
                        std::string subdirName = Glib::build_filename( iconCacheDir, *it );
                        if ( Glib::file_test(subdirName, Glib::FILE_TEST_IS_DIR) ) {
                            Glib::Dir subdir(subdirName);
                            for ( Glib::DirIterator subit = subdir.begin(); subit != subdir.end(); ++subit ) {
                                std::string fullpath = Glib::build_filename( subdirName, *subit );
                                if ( Glib::file_test(fullpath, Glib::FILE_TEST_EXISTS) && !Glib::file_test(fullpath, Glib::FILE_TEST_IS_DIR) ) {
                                    g_remove(fullpath.c_str());
                                }
                            }
                            g_rmdir( subdirName.c_str() );
                        }
                    }
                }
            }
        } else {
            g_mkdir_with_parents( iconCacheDir.c_str(), 0x1ED );
        }

        if ( g_file_set_contents(iconCacheFile.c_str(), wanted.c_str(), wanted.size(), 0) ) {
            // Caching may proceed
        } else {
            g_warning("Unable to write cache info file.");
        }
    }
}

static Glib::ustring icon_cache_key(Glib::ustring const &name, unsigned psize);
static GdkPixbuf *get_cached_pixbuf(Glib::ustring const &key);

void IconImpl::setupLegacyNaming() {
    legacyNames["document-import"] ="file_import";
    legacyNames["document-export"] ="file_export";
    legacyNames["document-import-ocal"] ="ocal_import";
    legacyNames["document-export-ocal"] ="ocal_export";
    legacyNames["document-metadata"] ="document_metadata";
    legacyNames["dialog-input-devices"] ="input_devices";
    legacyNames["edit-duplicate"] ="edit_duplicate";
    legacyNames["edit-clone"] ="edit_clone";
    legacyNames["edit-clone-unlink"] ="edit_unlink_clone";
    legacyNames["edit-select-original"] ="edit_select_original";
    legacyNames["edit-undo-history"] ="edit_undo_history";
    legacyNames["edit-paste-in-place"] ="selection_paste_in_place";
    legacyNames["edit-paste-style"] ="selection_paste_style";
    legacyNames["selection-make-bitmap-copy"] ="selection_bitmap";
    legacyNames["edit-select-all"] ="selection_select_all";
    legacyNames["edit-select-all-layers"] ="selection_select_all_in_all_layers";
    legacyNames["edit-select-invert"] ="selection_invert";
    legacyNames["edit-select-none"] ="selection_deselect";
    legacyNames["dialog-xml-editor"] ="xml_editor";
    legacyNames["zoom-original"] ="zoom_1_to_1";
    legacyNames["zoom-half-size"] ="zoom_1_to_2";
    legacyNames["zoom-double-size"] ="zoom_2_to_1";
    legacyNames["zoom-fit-selection"] ="zoom_select";
    legacyNames["zoom-fit-drawing"] ="zoom_draw";
    legacyNames["zoom-fit-page"] ="zoom_page";
    legacyNames["zoom-fit-width"] ="zoom_pagewidth";
    legacyNames["zoom-previous"] ="zoom_previous";
    legacyNames["zoom-next"] ="zoom_next";
    legacyNames["zoom-in"] ="zoom_in";
    legacyNames["zoom-out"] ="zoom_out";
    legacyNames["show-grid"] ="grid";
    legacyNames["show-guides"] ="guides";
    legacyNames["color-management"] ="color_management";
    legacyNames["show-dialogs"] ="dialog_toggle";
    legacyNames["dialog-messages"] ="messages";
    legacyNames["dialog-scripts"] ="scripts";
    legacyNames["window-previous"] ="window_previous";
    legacyNames["window-next"] ="window_next";
    legacyNames["dialog-icon-preview"] ="view_icon_preview";
    legacyNames["window-new"] ="view_new";
    legacyNames["view-fullscreen"] ="fullscreen";
    legacyNames["layer-new"] ="new_layer";
    legacyNames["layer-rename"] ="rename_layer";
    legacyNames["layer-previous"] ="switch_to_layer_above";
    legacyNames["layer-next"] ="switch_to_layer_below";
    legacyNames["selection-move-to-layer-above"] ="move_selection_above";
    legacyNames["selection-move-to-layer-below"] ="move_selection_below";
    legacyNames["layer-raise"] ="raise_layer";
    legacyNames["layer-lower"] ="lower_layer";
    legacyNames["layer-top"] ="layer_to_top";
    legacyNames["layer-bottom"] ="layer_to_bottom";
    legacyNames["layer-delete"] ="delete_layer";
    legacyNames["dialog-layers"] ="layers";
    legacyNames["dialog-fill-and-stroke"] ="fill_and_stroke";
    legacyNames["dialog-object-properties"] ="dialog_item_properties";
    legacyNames["object-group"] ="selection_group";
    legacyNames["object-ungroup"] ="selection_ungroup";
    legacyNames["selection-raise"] ="selection_up";
    legacyNames["selection-lower"] ="selection_down";
    legacyNames["selection-top"] ="selection_top";
    legacyNames["selection-bottom"] ="selection_bot";
    legacyNames["object-rotate-left"] ="object_rotate_90_CCW";
    legacyNames["object-rotate-right"] ="object_rotate_90_CW";
    legacyNames["object-flip-horizontal"] ="object_flip_hor";
    legacyNames["object-flip-vertical"] ="object_flip_ver";
    legacyNames["dialog-transform"] ="object_trans";
    legacyNames["dialog-align-and-distribute"] ="object_align";
    legacyNames["dialog-rows-and-columns"] ="grid_arrange";
    legacyNames["object-to-path"] ="object_tocurve";
    legacyNames["stroke-to-path"] ="stroke_tocurve";
    legacyNames["bitmap-trace"] ="selection_trace";
    legacyNames["path-union"] ="union";
    legacyNames["path-difference"] ="difference";
    legacyNames["path-intersection"] ="intersection";
    legacyNames["path-exclusion"] ="exclusion";
    legacyNames["path-division"] ="division";
    legacyNames["path-cut"] ="cut_path";
    legacyNames["path-combine"] ="selection_combine";
    legacyNames["path-break-apart"] ="selection_break";
    legacyNames["path-outset"] ="outset_path";
    legacyNames["path-inset"] ="inset_path";
    legacyNames["path-offset-dynamic"] ="dynamic_offset";
    legacyNames["path-offset-linked"] ="linked_offset";
    legacyNames["path-simplify"] ="simplify";
    legacyNames["path-reverse"] ="selection_reverse";
    legacyNames["dialog-text-and-font"] ="object_font";
    legacyNames["text-put-on-path"] ="put_on_path";
    legacyNames["text-remove-from-path"] ="remove_from_path";
    legacyNames["text-flow-into-frame"] ="flow_into_frame";
    legacyNames["text-unflow"] ="unflow";
    legacyNames["text-convert-to-regular"] ="convert_to_text";
    legacyNames["text-unkern"] ="remove_manual_kerns";
    legacyNames["help-keyboard-shortcuts"] ="help_keys";
    legacyNames["help-contents"] ="help_tutorials";
    legacyNames["inkscape-logo"] ="inkscape_options";
    legacyNames["dialog-memory"] ="about_memory";
    legacyNames["tool-pointer"] ="draw_select";
    legacyNames["tool-node-editor"] ="draw_node";
    legacyNames["tool-tweak"] ="draw_tweak";
    legacyNames["zoom"] ="draw_zoom";
    legacyNames["draw-rectangle"] ="draw_rect";
    legacyNames["draw-cuboid"] ="draw_3dbox";
    legacyNames["draw-ellipse"] ="draw_arc";
    legacyNames["draw-polygon-star"] ="draw_star";
    legacyNames["draw-spiral"] ="draw_spiral";
    legacyNames["draw-freehand"] ="draw_freehand";
    legacyNames["draw-path"] ="draw_pen";
    legacyNames["draw-calligraphic"] ="draw_calligraphic";
    legacyNames["draw-eraser"] ="draw_erase";
    legacyNames["color-fill"] ="draw_paintbucket";
    legacyNames["draw-text"] ="draw_text";
    legacyNames["draw-connector"] ="draw_connector";
    legacyNames["color-gradient"] ="draw_gradient";
    legacyNames["color-picker"] ="draw_dropper";
    legacyNames["transform-affect-stroke"] ="transform_stroke";
    legacyNames["transform-affect-rounded-corners"] ="transform_corners";
    legacyNames["transform-affect-gradient"] ="transform_gradient";
    legacyNames["transform-affect-pattern"] ="transform_pattern";
    legacyNames["node-add"] ="node_insert";
    legacyNames["node-delete"] ="node_delete";
    legacyNames["node-join"] ="node_join";
    legacyNames["node-break"] ="node_break";
    legacyNames["node-join-segment"] ="node_join_segment";
    legacyNames["node-delete-segment"] ="node_delete_segment";
    legacyNames["node-type-cusp"] ="node_cusp";
    legacyNames["node-type-smooth"] ="node_smooth";
    legacyNames["node-type-symmetric"] ="node_symmetric";
    legacyNames["node-type-auto-smooth"] ="node_auto";
    legacyNames["node-segment-curve"] ="node_curve";
    legacyNames["node-segment-line"] ="node_line";
    legacyNames["show-node-handles"] ="nodes_show_handles";
    legacyNames["path-effect-parameter-next"] ="edit_next_parameter";
    legacyNames["show-path-outline"] ="nodes_show_helperpath";
    legacyNames["path-clip-edit"] ="nodeedit-clippath";
    legacyNames["path-mask-edit"] ="nodeedit-mask";
    legacyNames["node-type-cusp"] ="node_cusp";
    legacyNames["object-tweak-push"] ="tweak_move_mode";
    legacyNames["object-tweak-attract"] ="tweak_move_mode_inout";
    legacyNames["object-tweak-randomize"] ="tweak_move_mode_jitter";
    legacyNames["object-tweak-shrink"] ="tweak_scale_mode";
    legacyNames["object-tweak-rotate"] ="tweak_rotate_mode";
    legacyNames["object-tweak-duplicate"] ="tweak_moreless_mode";
    legacyNames["object-tweak-push"] ="tweak_move_mode";
    legacyNames["path-tweak-push"] ="tweak_push_mode";
    legacyNames["path-tweak-shrink"] ="tweak_shrink_mode";
    legacyNames["path-tweak-attract"] ="tweak_attract_mode";
    legacyNames["path-tweak-roughen"] ="tweak_roughen_mode";
    legacyNames["object-tweak-paint"] ="tweak_colorpaint_mode";
    legacyNames["object-tweak-jitter-color"] ="tweak_colorjitter_mode";
    legacyNames["object-tweak-blur"] ="tweak_blur_mode";
    legacyNames["rectangle-make-corners-sharp"] ="squared_corner";
    legacyNames["perspective-parallel"] ="toggle_vp_x";
    legacyNames["draw-ellipse-whole"] ="reset_circle";
    legacyNames["draw-ellipse-segment"] ="circle_closed_arc";
    legacyNames["draw-ellipse-arc"] ="circle_open_arc";
    legacyNames["draw-polygon"] ="star_flat";
    legacyNames["draw-star"] ="star_angled";
    legacyNames["path-mode-bezier"] ="bezier_mode";
    legacyNames["path-mode-spiro"] ="spiro_splines_mode";
    legacyNames["path-mode-bspline"] ="bspline_mode";
    legacyNames["path-mode-polyline"] ="polylines_mode";
    legacyNames["path-mode-polyline-paraxial"] ="paraxial_lines_mode";
    legacyNames["draw-use-tilt"] ="guse_tilt";
    legacyNames["draw-use-pressure"] ="guse_pressure";
    legacyNames["draw-trace-background"] ="trace_background";
    legacyNames["draw-eraser-delete-objects"] ="delete_object";
    legacyNames["format-text-direction-vertical"] ="writing_mode_tb";
    legacyNames["format-text-direction-horizontal"] ="writing_mode_lr";
    legacyNames["connector-avoid"] ="connector_avoid";
    legacyNames["connector-ignore"] ="connector_ignore";
    legacyNames["object-fill"] ="controls_fill";
    legacyNames["object-stroke"] ="controls_stroke";
    legacyNames["snap"] ="toggle_snap_global";
    legacyNames["snap-bounding-box"] ="toggle_snap_bbox";
    legacyNames["snap-bounding-box-edges"] ="toggle_snap_to_bbox_path";
    legacyNames["snap-bounding-box-corners"] ="toggle_snap_to_bbox_node";
    legacyNames["snap-bounding-box-midpoints"] ="toggle_snap_to_bbox_edge_midpoints";
    legacyNames["snap-bounding-box-center"] ="toggle_snap_to_bbox_midpoints";
    legacyNames["snap-nodes"] ="toggle_snap_nodes";
    legacyNames["snap-nodes-path"] ="toggle_snap_to_paths";
    legacyNames["snap-nodes-cusp"] ="toggle_snap_to_nodes";
    legacyNames["snap-nodes-smooth"] ="toggle_snap_to_smooth_nodes";
    legacyNames["snap-nodes-midpoint"] ="toggle_snap_to_midpoints";
    legacyNames["snap-nodes-intersection"] ="toggle_snap_to_path_intersections";
    legacyNames["snap-nodes-center"] ="toggle_snap_to_bbox_midpoints-3";
    legacyNames["snap-nodes-rotation-center"] ="toggle_snap_center";
    legacyNames["snap-page"] ="toggle_snap_page_border";
    legacyNames["snap-grid-guide-intersections"] ="toggle_snap_grid_guide_intersections";
    legacyNames["align-horizontal-right-to-anchor"] ="al_left_out";
    legacyNames["align-horizontal-left"] ="al_left_in";
    legacyNames["align-horizontal-center"] ="al_center_hor";
    legacyNames["align-horizontal-right"] ="al_right_in";
    legacyNames["align-horizontal-left-to-anchor"] ="al_right_out";
    legacyNames["align-horizontal-baseline"] ="al_baselines_vert";
    legacyNames["align-vertical-bottom-to-anchor"] ="al_top_out";
    legacyNames["align-vertical-top"] ="al_top_in";
    legacyNames["align-vertical-center"] ="al_center_ver";
    legacyNames["align-vertical-bottom"] ="al_bottom_in";
    legacyNames["align-vertical-top-to-anchor"] ="al_bottom_out";
    legacyNames["align-vertical-baseline"] ="al_baselines_hor";
    legacyNames["distribute-horizontal-left"] ="distribute_left";
    legacyNames["distribute-horizontal-center"] ="distribute_hcentre";
    legacyNames["distribute-horizontal-right"] ="distribute_right";
    legacyNames["distribute-horizontal-baseline"] ="distribute_baselines_hor";
    legacyNames["distribute-vertical-bottom"] ="distribute_bottom";
    legacyNames["distribute-vertical-center"] ="distribute_vcentre";
    legacyNames["distribute-vertical-top"] ="distribute_top";
    legacyNames["distribute-vertical-baseline"] ="distribute_baselines_vert";
    legacyNames["distribute-randomize"] ="distribute_randomize";
    legacyNames["distribute-unclump"] ="unclump";
    legacyNames["distribute-graph"] ="graph_layout";
    legacyNames["distribute-graph-directed"] ="directed_graph";
    legacyNames["distribute-remove-overlaps"] ="remove_overlaps";
    legacyNames["align-horizontal-node"] ="node_valign";
    legacyNames["align-vertical-node"] ="node_halign";
    legacyNames["distribute-vertical-node"] ="node_vdistribute";
    legacyNames["distribute-horizontal-node"] ="node_hdistribute";
    legacyNames["xml-element-new"] ="add_xml_element_node";
    legacyNames["xml-text-new"] ="add_xml_text_node";
    legacyNames["xml-node-delete"] ="delete_xml_node";
    legacyNames["xml-node-duplicate"] ="duplicate_xml_node";
    legacyNames["xml-attribute-delete"] ="delete_xml_attribute";
    legacyNames["transform-move-horizontal"] ="arrows_hor";
    legacyNames["transform-move-vertical"] ="arrows_ver";
    legacyNames["transform-scale-horizontal"] ="transform_scale_hor";
    legacyNames["transform-scale-vertical"] ="transform_scale_ver";
    legacyNames["transform-skew-horizontal"] ="transform_scew_hor";
    legacyNames["transform-skew-vertical"] ="transform_scew_ver";
    legacyNames["object-fill"] ="properties_fill";
    legacyNames["object-stroke"] ="properties_stroke_paint";
    legacyNames["object-stroke-style"] ="properties_stroke";
    legacyNames["paint-none"] ="fill_none";
    legacyNames["paint-solid"] ="fill_solid";
    legacyNames["paint-gradient-linear"] ="fill_gradient";
    legacyNames["paint-gradient-radial"] ="fill_radial";
    legacyNames["paint-pattern"] ="fill_pattern";
    legacyNames["paint-unknown"] ="fill_unset";
    legacyNames["fill-rule-even-odd"] ="fillrule_evenodd";
    legacyNames["fill-rule-nonzero"] ="fillrule_nonzero";
    legacyNames["stroke-join-miter"] ="join_miter";
    legacyNames["stroke-join-bevel"] ="join_bevel";
    legacyNames["stroke-join-round"] ="join_round";
    legacyNames["stroke-cap-butt"] ="cap_butt";
    legacyNames["stroke-cap-square"] ="cap_square";
    legacyNames["stroke-cap-round"] ="cap_round";
    legacyNames["guides"] ="guide";
    legacyNames["grid-rectangular"] ="grid_xy";
    legacyNames["grid-axonometric"] ="grid_axonom";
    legacyNames["object-visible"] ="visible";
    legacyNames["object-hidden"] ="hidden";
    legacyNames["object-unlocked"] ="lock_unlocked";
    legacyNames["object-locked"] ="width_height_lock";
    legacyNames["zoom"] ="sticky_zoom";
}

GtkWidget *IconImpl::newFull( Inkscape::IconSize lsize, gchar const *name )
{
    static bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpGtk");

    GtkWidget *widget = NULL;
    gint trySize = CLAMP( static_cast<gint>(lsize), 0, static_cast<gint>(G_N_ELEMENTS(iconSizeLookup) - 1) );
    if (trySize != lsize ) {
        std::cerr << "GtkWidget *IconImple::newFull(): lsize != trySize: lsize: " << lsize
                  << " try Size: " << trySize << " " << (name?name:"NULL") << std::endl;
    }
    if ( !sizeMapDone ) {
        injectCustomSize();
    }
    GtkIconSize mappedSize = iconSizeLookup[trySize];

    if ( legacyNames.empty() ) {
        setupLegacyNaming();
    }

    GtkWidget *img = gtk_image_new_from_icon_name( name, mappedSize );
    if ( dump ) {
        g_message("gtk_image_new_from_icon_name( '%s', %d ) = %p", name, mappedSize, img);
        GtkImageType thing = gtk_image_get_storage_type(GTK_IMAGE(img));
        g_message("      Type is %d  %s", (int)thing, (thing == GTK_IMAGE_EMPTY ? "Empty" : "ok"));
    }

    if ( img ) {
        GtkImageType type = gtk_image_get_storage_type( GTK_IMAGE(img) );
        if ( type == GTK_IMAGE_ICON_NAME ) {
            widget = GTK_WIDGET(img);
            img = NULL;

            // Add a hook to render if set visible before prerender is done.
            g_signal_connect( G_OBJECT(widget), "map", G_CALLBACK(imageMapNamedCB), GINT_TO_POINTER(0) );

            if ( Inkscape::Preferences::get()->getBool("/options/iconrender/named_nodelay") ) {
                int psize = getPhysSize(lsize);
                // std::cout << "  name: " << name << " size: " << psize << std::endl;
                prerenderIcon(name, mappedSize, psize);
            } else {
                addPreRender( mappedSize, name );
            }
        } else {
            if ( dump ) {
                g_message( "skipped gtk '%s' %d  (not GTK_IMAGE_ICON_NAME)", name, lsize );
            }
            //g_object_unref(G_OBJECT(img));
            img = NULL;
        }
    }

    if ( !widget ) {
        //g_message("Creating an SPIcon instance for %s:%d", name, (int)lsize);
        SPIcon *icon = SP_ICON(g_object_new(SP_TYPE_ICON, NULL));
        icon->lsize = lsize;
        icon->name = g_strdup(name);
        icon->psize = getPhysSize(lsize);

        widget = GTK_WIDGET(icon);
    }

    return widget;
}

// PUBLIC CALL:
GtkWidget *sp_icon_new( Inkscape::IconSize lsize, gchar const *name )
{
    return IconImpl::newFull( lsize, name );
}

// PUBLIC CALL for when you REALLY need a pixbuf
GdkPixbuf *sp_pixbuf_new( Inkscape::IconSize lsize, gchar const *name )
{
    int psize = IconImpl::getPhysSize(lsize);
    return IconImpl::renderup(name, lsize, psize);
}

// PUBLIC CALL:
Gtk::Widget *sp_icon_get_icon( Glib::ustring const &oid, Inkscape::IconSize size )
{
    Gtk::Widget *result = NULL;
    GtkWidget *widget = IconImpl::newFull( static_cast<Inkscape::IconSize>(Inkscape::getRegisteredIconSize(size)), oid.c_str() );

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

void IconImpl::injectCustomSize()
{
    // TODO - still need to handle the case of theme changes and resize, especially as we can't re-register a string.
    if ( !sizeMapDone )
    {
        bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpDefault");
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
}

GtkIconSize Inkscape::getRegisteredIconSize( Inkscape::IconSize size )
{
    GtkIconSize other = GTK_ICON_SIZE_MENU;
    IconImpl::injectCustomSize();
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
    return IconImpl::getPhysSize(size);
}

int IconImpl::getPhysSize(int size)
{
    static bool init = false;
    static int lastSys[Inkscape::ICON_SIZE_DECORATION + 1];
    static int vals[Inkscape::ICON_SIZE_DECORATION + 1];

    size = CLAMP( size, static_cast<int>(GTK_ICON_SIZE_MENU), static_cast<int>(Inkscape::ICON_SIZE_DECORATION) );

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
        bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpDefault");
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

            // The following is needed due to this documented behavior of gtk_icon_size_lookup:
            //   "The rendered pixbuf may not even correspond to the width/height returned by
            //   gtk_icon_size_lookup(), because themes are free to render the pixbuf however
            //   they like, including changing the usual size."
            gchar const *id = INKSCAPE_ICON("document-open");
            GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
            GdkPixbuf *pb = gtk_icon_theme_load_icon (icon_theme,
                                                      id,
                                                      vals[val_ix],
                                                      (GtkIconLookupFlags)0,
                                                      NULL);
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
        init = true;
    }

    return vals[size];
}



GdkPixbuf *IconImpl::loadPixmap(gchar const *name, unsigned /*lsize*/, unsigned psize)
{
    gchar *path = g_strdup_printf("%s/%s.png", INKSCAPE_PIXMAPDIR, name);
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
        path = g_strdup_printf("%s/%s.xpm", INKSCAPE_PIXMAPDIR, name);
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

static Geom::IntRect round_rect(Geom::Rect const &r)
{
    using Geom::X;
    using Geom::Y;
    Geom::IntPoint a, b;
    a[X] = round(r.left());
    a[Y] = round(r.top());
    b[X] = round(r.right());
    b[Y] = round(r.bottom());
    Geom::IntRect ret(a, b);
    return ret;
}

// takes doc, drawing, icon, and icon name to produce pixels
extern "C" guchar *
sp_icon_doc_icon( SPDocument *doc, Inkscape::Drawing &drawing,
                  gchar const *name, unsigned psize,
                  unsigned &stride)
{
    bool const dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg");
    guchar *px = NULL;

    if (doc) {
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_ITEM(object)) {
            SPItem *item = SP_ITEM(object);
            // Find bbox in document
            Geom::OptRect dbox = item->documentVisualBounds();

            if ( object->parent == NULL )
            {
                dbox = Geom::Rect(Geom::Point(0, 0),
                                Geom::Point(doc->getWidth().value("px"), doc->getHeight().value("px")));
            }

            /* This is in document coordinates, i.e. pixels */
            if ( dbox ) {
                /* Update to renderable state */
                double sf = 1.0;
                drawing.root()->setTransform(Geom::Scale(sf));
                drawing.update();
                /* Item integer bbox in points */
                // NOTE: previously, each rect coordinate was rounded using floor(c + 0.5)
                Geom::IntRect ibox = round_rect(*dbox);

                if ( dump ) {
                    g_message( "   box    --'%s'  (%f,%f)-(%f,%f)", name, (double)ibox.left(), (double)ibox.top(), (double)ibox.right(), (double)ibox.bottom() );
                }

                /* Find button visible area */
                int width = ibox.width();
                int height = ibox.height();

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

                        drawing.root()->setTransform(Geom::Scale(sf));
                        drawing.update();

                        ibox = round_rect(*dbox * Geom::Scale(sf));
                        if ( dump ) {
                            g_message( "   box2   --'%s'  (%f,%f)-(%f,%f)", name, (double)ibox.left(), (double)ibox.top(), (double)ibox.right(), (double)ibox.bottom() );
                        }

                        /* Find button visible area */
                        width = ibox.width();
                        height = ibox.height();
                        if ( dump ) {
                            g_message( "   vis2   --'%s'  (%d,%d)", name, width, height );
                        }
                    }
                }

                Geom::IntPoint pdim(psize, psize);
                int dx, dy;
                //dx = (psize - width) / 2;
                //dy = (psize - height) / 2;
                dx=dy=psize;
                dx=(dx-width)/2; // watch out for psize, since 'unsigned'-'signed' can cause problems if the result is negative
                dy=(dy-height)/2;
                Geom::IntRect area = Geom::IntRect::from_xywh(ibox.min() - Geom::IntPoint(dx,dy), pdim);
                /* Actual renderable area */
                Geom::IntRect ua = *Geom::intersect(ibox, area);

                if ( dump ) {
                    g_message( "   area   --'%s'  (%f,%f)-(%f,%f)", name, (double)area.left(), (double)area.top(), (double)area.right(), (double)area.bottom() );
                    g_message( "   ua     --'%s'  (%f,%f)-(%f,%f)", name, (double)ua.left(), (double)ua.top(), (double)ua.right(), (double)ua.bottom() );
                }

                stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, psize);

                /* Set up pixblock */
                px = g_new(guchar, stride * psize);
                memset(px, 0x00, stride * psize);

                /* Render */
                cairo_surface_t *s = cairo_image_surface_create_for_data(px,
                    CAIRO_FORMAT_ARGB32, psize, psize, stride);
                Inkscape::DrawingContext dc(s, ua.min());

                drawing.render(dc, ua);
                cairo_surface_destroy(s);

                // convert to GdkPixbuf format
                convert_pixels_argb32_to_pixbuf(px, psize, psize, stride);

                if ( Inkscape::Preferences::get()->getBool("/debug/icons/overlaySvg") ) {
                    IconImpl::overlayPixels( px, psize, psize, stride, 0x00, 0x00, 0xff );
                }
            }
        }
    }

    return px;
} // end of sp_icon_doc_icon()



class SVGDocCache
{
public:
    SVGDocCache( SPDocument *doc )
        : doc(doc)
        , visionkey(SPItem::display_key_new(1))
    {
        doc->doRef();
        doc->ensureUpToDate();
        drawing.setRoot(doc->getRoot()->invoke_show(drawing, visionkey, SP_ITEM_SHOW_DISPLAY ));
    }
    ~SVGDocCache() {
        doc->getRoot()->invoke_hide(visionkey);
        doc->doUnref();
    }
    SPDocument *doc;
    Inkscape::Drawing drawing;
    unsigned visionkey;
};

static std::map<Glib::ustring, SVGDocCache *> doc_cache;
static std::map<Glib::ustring, GdkPixbuf *> pb_cache;

Glib::ustring icon_cache_key(Glib::ustring const & name, unsigned psize)
{
    Glib::ustring key = name;
    key += ":";
    key += psize;
    return key;
}

GdkPixbuf *get_cached_pixbuf(Glib::ustring const &key) {
    GdkPixbuf* pb = NULL;
    std::map<Glib::ustring, GdkPixbuf *>::iterator found = pb_cache.find(key);
    if ( found != pb_cache.end() ) {
        pb = found->second;
    }
    return pb;
}

std::list<gchar*> &IconImpl::icons_svg_paths()
{
    static std::list<gchar *> sources;
    static bool initialized = false;
    if (!initialized) {
        // Fall back from user prefs dir into system locations.
        gchar *userdir = Inkscape::Application::profile_path("icons");
        sources.push_back(g_build_filename(userdir,"icons.svg", NULL));
        sources.push_back(g_build_filename(INKSCAPE_PIXMAPDIR, "icons.svg", NULL));
        g_free(userdir);
        initialized = true;
    }
    return sources;
}

// this function renders icons from icons.svg and returns the pixels.
guchar *IconImpl::load_svg_pixels(std::list<Glib::ustring> const &names,
                                  unsigned psize, unsigned &stride)
{
    bool const dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg");
    std::list<gchar *> &sources = icons_svg_paths();

    // Try each document in turn until we successfully load the icon from one
    guchar *px = NULL;
    for (std::list<gchar*>::iterator i = sources.begin(); (i != sources.end()) && !px; ++i) {
        gchar *doc_filename = *i;
        SVGDocCache *info = NULL;

        // Did we already load this doc?
        Glib::ustring key(doc_filename);
        {
            std::map<Glib::ustring, SVGDocCache *>::iterator i = doc_cache.find(key);
            if ( i != doc_cache.end() ) {
                info = i->second;
            }
        }

        // Try to load from document.
        if (!info && Inkscape::IO::file_test( doc_filename, G_FILE_TEST_IS_REGULAR ) ) {
            SPDocument *doc = SPDocument::createNewDoc( doc_filename, FALSE );
            if ( doc ) {
                if ( dump ) {
                    g_message("Loaded icon file %s", doc_filename);
                }
                // store into the cache
                info = new SVGDocCache(doc);
                doc_cache[key] = info;
            }
        }
        if (info) {
            for (std::list<Glib::ustring>::const_iterator it = names.begin(); !px && (it != names.end()); ++it ) {
                px = sp_icon_doc_icon( info->doc, info->drawing, it->c_str(), psize, stride );
            }
        }
    }

    return px;
}

static void addToIconSet(GdkPixbuf* pb, gchar const* name, GtkIconSize lsize, unsigned psize) {
    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();
    bool icon_found = icon_theme->has_icon(name);
    if ( !icon_found ) {
        Gtk::IconTheme::add_builtin_icon( name, psize, Glib::wrap(pb) );
        static bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpGtk");
        if (dump) {
            g_message("    set in a builtin for %s:%d:%d", name, lsize, psize);
        }
    }
}

void Inkscape::queueIconPrerender( Glib::ustring const &name, Inkscape::IconSize lsize )
{
    gboolean themedFound = gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), name.c_str());
    if ( !themedFound ) {
        gint trySize = CLAMP( static_cast<gint>(lsize), 0, static_cast<gint>(G_N_ELEMENTS(iconSizeLookup) - 1) );
        if ( !sizeMapDone ) {
            IconImpl::injectCustomSize();
        }
        GtkIconSize mappedSize = iconSizeLookup[trySize];

        int psize = IconImpl::getPhysSize(lsize);
        // TODO place in a queue that is triggered by other map events
        IconImpl::prerenderIcon(name.c_str(), mappedSize, psize);
    }
}

static std::map<unsigned, Glib::ustring> sizePaths;

static std::string getDestDir( unsigned psize )
{
    if ( sizePaths.find(psize) == sizePaths.end() ) {
        gchar *tmp = g_strdup_printf("%dx%d", psize, psize);
        sizePaths[psize] = tmp;
        g_free(tmp);
    }

    return sizePaths[psize];
}

// returns true if icon needed preloading, false if nothing was done
bool IconImpl::prerenderIcon(gchar const *name, GtkIconSize lsize, unsigned psize)
{
    bool loadNeeded = false;
    static bool useCache = Inkscape::Preferences::get()->getBool("/debug/icons/useCache", true);
    static bool cacheValidated = false;
    if (!cacheValidated) {
        cacheValidated = true;
        if ( useCache ) {
            validateCache();
        }
    }

    Glib::ustring key = icon_cache_key(name, psize);
    if ( !get_cached_pixbuf(key) ) {
        static bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpGtk");
        if ((internalNames.find(name) != internalNames.end())
            || (!gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), name))) {
            if (dump) {
                g_message("prerenderIcon  [%s] %d:%d", name, lsize, psize);
            }

            std::string potentialFile;
            bool dataLoaded = false;
            if ( useCache ) {
                // In file encoding:
                std::string iconCacheDir = Glib::build_filename(Glib::build_filename(Glib::get_user_cache_dir(), "inkscape"), "icons");
                std::string subpart = getDestDir(psize);
                std::string subdir = Glib::build_filename( iconCacheDir, subpart );
                if ( !Glib::file_test(subdir, Glib::FILE_TEST_EXISTS) ) {
                    g_mkdir_with_parents( subdir.c_str(), 0x1ED );
                }
                potentialFile = Glib::build_filename( subdir, name );
                potentialFile += ".png";

                if ( Glib::file_test(potentialFile, Glib::FILE_TEST_EXISTS) && Glib::file_test(potentialFile, Glib::FILE_TEST_IS_REGULAR) ) {
                    bool badFile = false;
                    try {
                        Glib::RefPtr<Gdk::Pixbuf> pb = Gdk::Pixbuf::create_from_file(potentialFile);
                        if (pb) {
                            dataLoaded = true;
                            GdkPixbuf *obj = pb->gobj();
                            g_object_ref(obj);
                            pb_cache[key] = obj;
                            addToIconSet(obj, name, lsize, psize);
                            loadNeeded = true;
                            if (internalNames.find(name) == internalNames.end()) {
                                internalNames.insert(name);
                            }
                        }
                    } catch ( Glib::FileError &ex ) {
                        //g_warning("FileError    [%s]", ex.what().c_str());
                        badFile = true;
                    } catch ( Gdk::PixbufError &ex ) {
                        //g_warning("PixbufError  [%s]", ex.what().c_str());
                        // Invalid contents. Remove cached item
                        badFile = true;
                    }
                    if ( badFile ) {
                        g_remove(potentialFile.c_str());
                    }
                }
            }

            if (!dataLoaded) {
                std::list<Glib::ustring> names;
                names.push_back(name);
                if ( legacyNames.find(name) != legacyNames.end() ) {
                    names.push_back(legacyNames[name]);
                    if ( dump ) {
                        g_message("load_svg_pixels([%s] = %s, %d, %d)", name, legacyNames[name].c_str(), lsize, psize);
                    }
                }
                unsigned stride;
                guchar* px = load_svg_pixels(names, psize, stride);
                if (px) {
                    GdkPixbuf* pb = gdk_pixbuf_new_from_data( px, GDK_COLORSPACE_RGB, TRUE, 8,
                                                              psize, psize, stride,
                                                              reinterpret_cast<GdkPixbufDestroyNotify>(g_free), NULL );
                    pb_cache[key] = pb;
                    addToIconSet(pb, name, lsize, psize);
                    loadNeeded = true;
                    if (internalNames.find(name) == internalNames.end()) {
                        internalNames.insert(name);
                    }
                    if (useCache) {
                        g_object_ref(pb);
                        Glib::RefPtr<Gdk::Pixbuf> ppp = Glib::wrap(pb);
                        try {
                            ppp->save( potentialFile, "png" );
                        } catch ( Glib::FileError &ex ) {
                            //g_warning("FileError    [%s]", ex.what().c_str());
                        } catch ( Gdk::PixbufError &ex ) {
                            //g_warning("PixbufError  [%s]", ex.what().c_str());
                        }
                    }
                } else if (dump) {
                    g_message("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX  error!!! pixels not found for '%s'", name);
                }
            }
        }
        else if (dump) {
            g_message("prerenderIcon  [%s] %d NOT!!!!!!", name, psize);
        }
    }
    return loadNeeded;
}

GdkPixbuf *IconImpl::loadSvg(std::list<Glib::ustring> const &names, GtkIconSize lsize, unsigned psize)
{
    Glib::ustring key = icon_cache_key(*names.begin(), psize);

    // did we already load this icon at this scale/size?
    GdkPixbuf* pb = get_cached_pixbuf(key);
    if (!pb) {
        unsigned stride;
        guchar *px = load_svg_pixels(names, psize, stride);
        if (px) {
            pb = gdk_pixbuf_new_from_data(px, GDK_COLORSPACE_RGB, TRUE, 8,
                                          psize, psize, stride,
                                          (GdkPixbufDestroyNotify)g_free, NULL);
            pb_cache[key] = pb;
            addToIconSet(pb, names.begin()->c_str(), lsize, psize);
        }
    }

    if ( pb ) {
        // increase refcount since we're handing out ownership
        g_object_ref(G_OBJECT(pb));
    }
    return pb;
}

void IconImpl::overlayPixels(guchar *px, int width, int height, int stride,
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

class preRenderItem
{
public:
    preRenderItem( GtkIconSize lsize, gchar const *name ) :
        _lsize( lsize ),
        _name( name )
    {}
    GtkIconSize _lsize;
    Glib::ustring _name;
};


static std::vector<preRenderItem> pendingRenders;
static bool callbackHooked = false;

void IconImpl::addPreRender( GtkIconSize lsize, gchar const *name )
{
    if ( !callbackHooked )
    {
        callbackHooked = true;
        g_idle_add_full( G_PRIORITY_LOW, &prerenderTask, NULL, NULL );
    }

    pendingRenders.push_back(preRenderItem(lsize, name));
}

gboolean IconImpl::prerenderTask(gpointer /*data*/) {
    if ( Inkscape::Application::isCrashing() ) {
        // stop
    } else if (!pendingRenders.empty()) {
        bool workDone = false;
        do {
            preRenderItem single = pendingRenders.front();
            pendingRenders.erase(pendingRenders.begin());
            int psize = getPhysSize(single._lsize);
            workDone = prerenderIcon(single._name.c_str(), single._lsize, psize);
        } while (!pendingRenders.empty() && !workDone);
    }

    if (!Inkscape::Application::isCrashing() && !pendingRenders.empty()) {
        return TRUE;
    } else {
        callbackHooked = false;
        return FALSE;
    }
}


void IconImpl::imageMapCB(GtkWidget* widget, gpointer user_data)
{
    gchar const* id = NULL;
    GtkIconSize size = GTK_ICON_SIZE_INVALID;
    gtk_image_get_icon_name(GTK_IMAGE(widget), &id, &size);
    GtkIconSize lsize = static_cast<GtkIconSize>(GPOINTER_TO_INT(user_data));
    if ( id ) {
        int psize = getPhysSize(lsize);
        g_message("imageMapCB(%p) for [%s]:%d:%d", widget, id, lsize, psize);
        for ( std::vector<preRenderItem>::iterator it = pendingRenders.begin(); it != pendingRenders.end(); ++it ) {
            if ( (it->_name == id) && (it->_lsize == lsize) ) {
                prerenderIcon(id, lsize, psize);
                pendingRenders.erase(it);
                g_message("    prerender for %s:%d:%d", id, lsize, psize);
                if (lsize != size) {
                    int psize = getPhysSize(size);
                    prerenderIcon(id, size, psize);
                }
                break;
            }
        }
    }

    g_signal_handlers_disconnect_by_func(widget, (gpointer)imageMapCB, user_data);
}

void IconImpl::imageMapNamedCB(GtkWidget* widget, gpointer user_data)
{
    GtkImage* img = GTK_IMAGE(widget);
    gchar const* iconName = NULL;
    GtkIconSize size = GTK_ICON_SIZE_INVALID;
    gtk_image_get_icon_name(img, &iconName, &size);
    if ( iconName ) {
        GtkImageType type = gtk_image_get_storage_type( GTK_IMAGE(img) );
        if ( type == GTK_IMAGE_ICON_NAME ) {

            GtkIconSize iconSize = GTK_ICON_SIZE_INVALID;
            gchar const* iconName_two = NULL;
            {
                g_object_get(G_OBJECT(widget),
                             "icon-name", &iconName_two,
                             "icon-size", &iconSize,
                             NULL);
            }

            for ( std::vector<preRenderItem>::iterator it = pendingRenders.begin(); it != pendingRenders.end(); ++it ) {
                /// @todo  fix pointer string comparison here!!! "it->_name == iconName_two", that seems very bug-prone
                if ( (it->_name == iconName_two) && (it->_lsize == iconSize) ) {
                    int psize = getPhysSize(iconSize);
                    prerenderIcon(iconName_two, iconSize, psize);
                    pendingRenders.erase(it);
                    break;
                }
            }

            gtk_image_set_from_icon_name(img, "", iconSize);
            gtk_image_set_from_icon_name(img, iconName_two, iconSize);
        } else {
            g_warning("UNEXPECTED TYPE of %d", (int)type);
        }
    }

    g_signal_handlers_disconnect_by_func(widget, (gpointer)imageMapNamedCB, user_data);
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

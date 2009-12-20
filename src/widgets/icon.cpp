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

static gboolean icon_prerender_task(gpointer data);

static void addPreRender( GtkIconSize lsize, gchar const *name );

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

static std::map<Glib::ustring, Glib::ustring> legacyNames;

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

GType
sp_icon_get_type()
{
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

static GdkPixbuf* renderup( gchar const* name, Inkscape::IconSize lsize, unsigned psize );

// PUBLIC CALL:
void sp_icon_fetch_pixbuf( SPIcon *icon )
{
    if ( icon ) {
        if ( !icon->pb ) {
            icon->psize = sp_icon_get_phys_size(icon->lsize);
            icon->pb = renderup(icon->name, icon->lsize, icon->psize);
        }
    }
}

static GdkPixbuf* renderup( gchar const* name, Inkscape::IconSize lsize, unsigned psize ) {
    GtkIconTheme *theme = gtk_icon_theme_get_default();

    GdkPixbuf *pb = 0;
    if (gtk_icon_theme_has_icon(theme, name)) {
        pb = gtk_icon_theme_load_icon(theme, name, psize, (GtkIconLookupFlags) 0, NULL);
    }
    if (!pb) {
        pb = sp_icon_image_load_svg( name, Inkscape::getRegisteredIconSize(lsize), psize );
        if (!pb && (legacyNames.find(name) != legacyNames.end())) {
            if ( Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg") ) {
                g_message("Checking fallback [%s]->[%s]", name, legacyNames[name].c_str());
            }
            pb = sp_icon_image_load_svg( legacyNames[name].c_str(), Inkscape::getRegisteredIconSize(lsize), psize );
        }

        // if this was loaded from SVG, add it as a builtin icon
        if (pb) {
            gtk_icon_theme_add_builtin_icon(name, psize, pb);
        }
    }
    if (!pb) {
        pb = sp_icon_image_load_pixmap( name, lsize, psize );
    }
    if ( !pb ) {
        // TODO: We should do something more useful if we can't load the image.
        g_warning ("failed to load icon '%s'", name);
    }
    return pb;
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
    bool const dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg");
    if ( dump ) {
        g_message("Got a change bump for this icon");
    }
    sizeDirty = true;
    sp_icon_reset(icon);
    gtk_widget_queue_draw( GTK_WIDGET(icon) );
}


static void imageMapCB(GtkWidget* widget, gpointer user_data);
static void imageMapNamedCB(GtkWidget* widget, gpointer user_data);
static bool prerender_icon(gchar const *name, GtkIconSize lsize, unsigned psize);
static Glib::ustring icon_cache_key(gchar const *name, unsigned psize);
static GdkPixbuf *get_cached_pixbuf(Glib::ustring const &key);

static void setupLegacyNaming() {
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

static GtkWidget *
sp_icon_new_full( Inkscape::IconSize lsize, gchar const *name )
{
    static bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpGtk");

    GtkWidget *widget = 0;
    gint trySize = CLAMP( static_cast<gint>(lsize), 0, static_cast<gint>(G_N_ELEMENTS(iconSizeLookup) - 1) );
    if ( !sizeMapDone ) {
        injectCustomSize();
    }
    GtkIconSize mappedSize = iconSizeLookup[trySize];

    GtkStockItem stock;
    gboolean stockFound = gtk_stock_lookup( name, &stock );

    GtkWidget *img = 0;
    if ( legacyNames.empty() ) {
        setupLegacyNaming();
    }

    if ( stockFound ) {
        img = gtk_image_new_from_stock( name, mappedSize );
    } else {
        img = gtk_image_new_from_icon_name( name, mappedSize );
        if ( dump ) {
            g_message("gtk_image_new_from_icon_name( '%s', %d ) = %p", name, mappedSize, img);
            GtkImageType thing = gtk_image_get_storage_type(GTK_IMAGE(img));
            g_message("      Type is %d  %s", (int)thing, (thing == GTK_IMAGE_EMPTY ? "Empty" : "ok"));
        }
    }

    if ( img ) {
        GtkImageType type = gtk_image_get_storage_type( GTK_IMAGE(img) );
        if ( type == GTK_IMAGE_STOCK ) {
            if ( !stockFound ) {
                // It's not showing as a stock ID, so assume it will be present internally
                addPreRender( mappedSize, name );

                // Add a hook to render if set visible before prerender is done.
                g_signal_connect( G_OBJECT(img), "map", G_CALLBACK(imageMapCB), GINT_TO_POINTER(static_cast<int>(mappedSize)) );
                if ( dump ) {
                    g_message("      connecting %p for imageMapCB for [%s] %d", img, name, (int)mappedSize);
                }
            }
            widget = GTK_WIDGET(img);
            img = 0;
            if ( dump ) {
                g_message( "loaded gtk  '%s' %d  (GTK_IMAGE_STOCK) %s  on %p", name, mappedSize, (stockFound ? "STOCK" : "local"), widget );
            }
        } else if ( type == GTK_IMAGE_ICON_NAME ) {
            widget = GTK_WIDGET(img);
            img = 0;

            // Add a hook to render if set visible before prerender is done.
            g_signal_connect( G_OBJECT(widget), "map", G_CALLBACK(imageMapNamedCB), GINT_TO_POINTER(0) );

            if ( Inkscape::Preferences::get()->getBool("/options/iconrender/named_nodelay") ) {
                int psize = sp_icon_get_phys_size(lsize);
                prerender_icon(name, mappedSize, psize);
            } else {
                addPreRender( mappedSize, name );
            }
        } else {
            if ( dump ) {
                g_message( "skipped gtk '%s' %d  (not GTK_IMAGE_STOCK)", name, lsize );
            }
            //g_object_unref( (GObject *)img );
            img = 0;
        }
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

            // The following is needed due to this documented behavior of gtk_icon_size_lookup:
            //   "The rendered pixbuf may not even correspond to the width/height returned by
            //   gtk_icon_size_lookup(), because themes are free to render the pixbuf however
            //   they like, including changing the usual size."
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

    return vals[size];
}

static void sp_icon_paint(SPIcon *icon, GdkRectangle const */*area*/)
{
    GtkWidget &widget = *GTK_WIDGET(icon);
    GdkPixbuf *image = icon->pb;
    bool unref_image = false;

    /* copied from the expose function of GtkImage */
    if (GTK_WIDGET_STATE (icon) != GTK_STATE_NORMAL && image) {
        GtkIconSource *source = gtk_icon_source_new();
        gtk_icon_source_set_pixbuf(source, icon->pb);
        gtk_icon_source_set_size(source, GTK_ICON_SIZE_SMALL_TOOLBAR); // note: this is boilerplate and not used
        gtk_icon_source_set_size_wildcarded(source, FALSE);
        image = gtk_style_render_icon (widget.style, source, gtk_widget_get_direction(&widget),
            (GtkStateType) GTK_WIDGET_STATE(&widget), (GtkIconSize)-1, &widget, "gtk-image");
        gtk_icon_source_free(source);
        unref_image = true;
    }

    if (image) {
        int x = floor(widget.allocation.x + ((widget.allocation.width - widget.requisition.width) * 0.5));
        int y = floor(widget.allocation.y + ((widget.allocation.height - widget.requisition.height) * 0.5));
        int width = gdk_pixbuf_get_width(image);
        int height = gdk_pixbuf_get_height(image);
        // Limit drawing to when we actually have something. Avoids some crashes.
        if ( (width > 0) && (height > 0) ) {
            gdk_draw_pixbuf(GDK_DRAWABLE(widget.window), widget.style->black_gc, image,
                            0, 0, x, y, width, height,
                            GDK_RGB_DITHER_NORMAL, x, y);
        }
    }

    if (unref_image) {
        g_object_unref(G_OBJECT(image));
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
    bool const dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpSvg");
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

                if ( Inkscape::Preferences::get()->getBool("/debug/icons/overlaySvg") ) {
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
    GdkPixbuf* pb = 0;
    std::map<Glib::ustring, GdkPixbuf *>::iterator found = pb_cache.find(key);
    if ( found != pb_cache.end() ) {
        pb = found->second;
    }
    return pb;
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
        if (!info && !doc) {
            continue;
        }

        px = sp_icon_doc_icon( doc, root, name, psize );
//         if (px) {
//             g_message("Found icon %s in %s", name, doc_filename);
//         }
    }

//     if (!px) {
//         g_message("Not found icon %s", name);
//     }
    return px;
}

static void addToIconSet(GdkPixbuf* pb, gchar const* name, GtkIconSize lsize, unsigned psize) {
    static bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpGtk");
    GtkStockItem stock;
    gboolean stockFound = gtk_stock_lookup( name, &stock );
   if ( !stockFound ) {
        Gtk::IconTheme::add_builtin_icon( name, psize, Glib::wrap(pb) );
        if (dump) {
            g_message("    set in a builtin for %s:%d:%d", name, lsize, psize);
        }
    }
}

void Inkscape::queueIconPrerender( Glib::ustring const &name, Inkscape::IconSize lsize )
{
    GtkStockItem stock;
    gboolean stockFound = gtk_stock_lookup( name.c_str(), &stock );
    if (!stockFound && !gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), name.c_str()) ) {
        gint trySize = CLAMP( static_cast<gint>(lsize), 0, static_cast<gint>(G_N_ELEMENTS(iconSizeLookup) - 1) );
        if ( !sizeMapDone ) {
            injectCustomSize();
        }
        GtkIconSize mappedSize = iconSizeLookup[trySize];

        int psize = sp_icon_get_phys_size(lsize);
        // TODO place in a queue that is triggered by other map events
        prerender_icon(name.c_str(), mappedSize, psize);
    }
}

// returns true if icon needed preloading, false if nothing was done
bool prerender_icon(gchar const *name, GtkIconSize lsize, unsigned psize)
{
    bool loadNeeded = false;
    static bool dump = Inkscape::Preferences::get()->getBool("/debug/icons/dumpGtk");

    Glib::ustring key = icon_cache_key(name, psize);
    if ( !get_cached_pixbuf(key) ) {
        if ((internalNames.find(name) != internalNames.end())
            || (!gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), name))) {
            if (dump) {
                g_message("prerender_icon  [%s] %d:%d", name, lsize, psize);
            }
            guchar* px = load_svg_pixels(name, lsize, psize);
            if ( !px ) {
                // check for a fallback name
                if ( legacyNames.find(name) != legacyNames.end() ) {
                    if ( dump ) {
                        g_message("load_svg_pixels([%s]=%s, %d, %d)", name, legacyNames[name].c_str(), lsize, psize);
                    }
                    px = load_svg_pixels(legacyNames[name].c_str(), lsize, psize);
                }
            }
            if (px) {
                GdkPixbuf* pb = gdk_pixbuf_new_from_data( px, GDK_COLORSPACE_RGB, TRUE, 8,
                                                          psize, psize, psize * 4,
                                                          reinterpret_cast<GdkPixbufDestroyNotify>(g_free), NULL );
                pb_cache[key] = pb;
                addToIconSet(pb, name, lsize, psize);
                loadNeeded = true;
                if (internalNames.find(name) == internalNames.end()) {
                    internalNames.insert(name);
                }
            } else if (dump) {
                g_message("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX  error!!! pixels not found for '%s'", name);
            }
        }
        else if (dump) {
            g_message("prerender_icon  [%s] %d NOT!!!!!!", name, psize);
        }
    }
    return loadNeeded;
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
            addToIconSet(pb, name, lsize, psize);
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

static void addPreRender( GtkIconSize lsize, gchar const *name )
{
    if ( !callbackHooked )
    {
        callbackHooked = true;
        g_idle_add_full( G_PRIORITY_LOW, &icon_prerender_task, NULL, NULL );
    }

    pendingRenders.push_back(preRenderItem(lsize, name));
}

gboolean icon_prerender_task(gpointer /*data*/) {
    if (!pendingRenders.empty()) {
        bool workDone = false;
        do {
            preRenderItem single = pendingRenders.front();
            pendingRenders.erase(pendingRenders.begin());
            int psize = sp_icon_get_phys_size(single._lsize);
            workDone = prerender_icon(single._name.c_str(), single._lsize, psize);
        } while (!pendingRenders.empty() && !workDone);
    }

    if (!pendingRenders.empty()) {
        return TRUE;
    } else {
        callbackHooked = false;
        return FALSE;
    }
}


void imageMapCB(GtkWidget* widget, gpointer user_data) {
    gchar* id = 0;
    GtkIconSize size = GTK_ICON_SIZE_INVALID;
    gtk_image_get_stock(GTK_IMAGE(widget), &id, &size);
    GtkIconSize lsize = static_cast<GtkIconSize>(GPOINTER_TO_INT(user_data));
    if ( id ) {
        int psize = sp_icon_get_phys_size(lsize);
        g_message("imageMapCB(%p) for [%s]:%d:%d", widget, id, lsize, psize);
        for ( std::vector<preRenderItem>::iterator it = pendingRenders.begin(); it != pendingRenders.end(); ++it ) {
            if ( (it->_name == id) && (it->_lsize == lsize) ) {
                prerender_icon(id, lsize, psize);
                pendingRenders.erase(it);
                g_message("    prerender for %s:%d:%d", id, lsize, psize);
                if (lsize != size) {
                    int psize = sp_icon_get_phys_size(size);
                    prerender_icon(id, size, psize);
                }
                break;
            }
        }
    }

    g_signal_handlers_disconnect_by_func(widget, (gpointer)imageMapCB, user_data);
}

static void imageMapNamedCB(GtkWidget* widget, gpointer user_data) {
    GtkImage* img = GTK_IMAGE(widget);
    gchar const* iconName = 0;
    GtkIconSize size = GTK_ICON_SIZE_INVALID;
    gtk_image_get_icon_name(img, &iconName, &size);
    if ( iconName ) {
        GtkImageType type = gtk_image_get_storage_type( GTK_IMAGE(img) );
        if ( type == GTK_IMAGE_ICON_NAME ) {

            gint iconSize = 0;
            gchar* iconName = 0;
            {
                g_object_get(G_OBJECT(widget),
                             "icon-name", &iconName,
                             "icon-size", &iconSize,
                             NULL);
            }

            for ( std::vector<preRenderItem>::iterator it = pendingRenders.begin(); it != pendingRenders.end(); ++it ) {
                if ( (it->_name == iconName) && (it->_lsize == size) ) {
                    int psize = sp_icon_get_phys_size(size);
                    prerender_icon(iconName, size, psize);
                    pendingRenders.erase(it);
                    break;
                }
            }

            gtk_image_set_from_icon_name(img, "", (GtkIconSize)iconSize);
            gtk_image_set_from_icon_name(img, iconName, (GtkIconSize)iconSize);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

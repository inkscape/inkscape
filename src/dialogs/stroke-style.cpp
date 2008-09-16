#define __SP_STROKE_STYLE_C__

/**
 * \brief  Stroke style dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <brycehar@bryceharrington.org>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2001-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 * Copyright (C) 2008 Maximilian Albert (gtkmm-ification)
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_SS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif



#include <glib/gmem.h>
#include <gtk/gtk.h>

#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "svg/css-ostringstream.h"
#include "widgets/sp-widget.h"
#include "widgets/spw-utilities.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "marker.h"
#include "sp-pattern.h"
#include "widgets/paint-selector.h"
#include "widgets/dash-selector.h"
#include "style.h"
#include "gradient-chemistry.h"
#include "sp-namedview.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "selection.h"
#include "inkscape.h"
#include "inkscape-stock.h"
#include "dialogs/dialog-events.h"
#include "sp-text.h"
#include "sp-rect.h"
#include "document-private.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "path-prefix.h"
#include "widgets/icon.h"
#include "helper/stock-items.h"
#include "io/sys.h"
#include "ui/cache/svg_preview_cache.h"

#include "dialogs/stroke-style.h"


/* Paint */

static void sp_stroke_style_paint_selection_modified (SPWidget *spw, Inkscape::Selection *selection, guint flags, SPPaintSelector *psel);
static void sp_stroke_style_paint_selection_changed (SPWidget *spw, Inkscape::Selection *selection, SPPaintSelector *psel);
static void sp_stroke_style_paint_update(SPWidget *spw);

static void sp_stroke_style_paint_mode_changed(SPPaintSelector *psel, SPPaintSelectorMode mode, SPWidget *spw);
static void sp_stroke_style_paint_dragged(SPPaintSelector *psel, SPWidget *spw);
static void sp_stroke_style_paint_changed(SPPaintSelector *psel, SPWidget *spw);

static void sp_stroke_style_widget_change_subselection ( Inkscape::Application *inkscape, SPDesktop *desktop, SPWidget *spw );
static void sp_stroke_style_widget_transientize_callback(Inkscape::Application *inkscape,
                                                         SPDesktop *desktop,
                                                         SPWidget *spw );

/** Marker selection option menus */
static Gtk::OptionMenu * marker_start_menu = NULL;
static Gtk::OptionMenu * marker_mid_menu = NULL;
static Gtk::OptionMenu * marker_end_menu = NULL;

sigc::connection marker_start_menu_connection;
sigc::connection marker_mid_menu_connection;
sigc::connection marker_end_menu_connection;

static SPObject *ink_extract_marker_name(gchar const *n, SPDocument *doc);
static void      ink_markers_menu_update(Gtk::Container* spw, SPMarkerLoc const which);

static Inkscape::UI::Cache::SvgPreview svg_preview_cache;

/**
 * Create the stroke style widget, and hook up all the signals.
 */
GtkWidget *
sp_stroke_style_paint_widget_new(void)
{
    GtkWidget *spw, *psel;

    spw = sp_widget_new_global(INKSCAPE);

    psel = sp_paint_selector_new(false); // without fillrule selector
    gtk_widget_show(psel);
    gtk_container_add(GTK_CONTAINER(spw), psel);
    gtk_object_set_data(GTK_OBJECT(spw), "paint-selector", psel);

    gtk_signal_connect(GTK_OBJECT(spw), "modify_selection",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_selection_modified),
                       psel);
    gtk_signal_connect(GTK_OBJECT(spw), "change_selection",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_selection_changed),
                       psel);

    g_signal_connect (INKSCAPE, "change_subselection", G_CALLBACK (sp_stroke_style_widget_change_subselection), spw);

    g_signal_connect (G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK (sp_stroke_style_widget_transientize_callback), spw );

    gtk_signal_connect(GTK_OBJECT(psel), "mode_changed",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_mode_changed),
                       spw);
    gtk_signal_connect(GTK_OBJECT(psel), "dragged",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_dragged),
                       spw);
    gtk_signal_connect(GTK_OBJECT(psel), "changed",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_changed),
                       spw);

    sp_stroke_style_paint_update (SP_WIDGET(spw));
    return spw;
}

/**
 * On signal modified, invokes an update of the stroke style paint object.
 */
static void
sp_stroke_style_paint_selection_modified( SPWidget *spw,
                                          Inkscape::Selection */*selection*/,
                                          guint flags,
                                          SPPaintSelector */*psel*/ )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG |
                  SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
        sp_stroke_style_paint_update(spw);
    }
}


/**
 * On signal selection changed, invokes an update of the stroke style paint object.
 */
static void
sp_stroke_style_paint_selection_changed( SPWidget *spw,
                                         Inkscape::Selection */*selection*/,
                                         SPPaintSelector */*psel*/ )
{
    sp_stroke_style_paint_update (spw);
}


/**
 * On signal change subselection, invoke an update of the stroke style widget.
 */
static void
sp_stroke_style_widget_change_subselection( Inkscape::Application */*inkscape*/,
                                            SPDesktop */*desktop*/,
                                            SPWidget *spw )
{
    sp_stroke_style_paint_update (spw);
}

/**
 * Gets the active stroke style property, then sets the appropriate color, alpha, gradient,
 * pattern, etc. for the paint-selector.
 */
static void
sp_stroke_style_paint_update (SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    SPPaintSelector *psel = SP_PAINT_SELECTOR(gtk_object_get_data(GTK_OBJECT(spw), "paint-selector"));

    // create temporary style
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    // query into it
    int result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKE);

    switch (result) {
        case QUERY_STYLE_NOTHING:
        {
            /* No paint at all */
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_EMPTY);
            break;
        }

        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently, e.g. display "averaged" somewhere in paint selector
        case QUERY_STYLE_MULTIPLE_SAME:
        {
            SPPaintSelectorMode pselmode = sp_style_determine_paint_selector_mode (query, false);
            sp_paint_selector_set_mode (psel, pselmode);

            if (query->stroke.set && query->stroke.isPaintserver()) {

                SPPaintServer *server = SP_STYLE_STROKE_SERVER (query);

                if (SP_IS_LINEARGRADIENT (server)) {
                    SPGradient *vector = sp_gradient_get_vector (SP_GRADIENT (server), FALSE);
                    sp_paint_selector_set_gradient_linear (psel, vector);

                    SPLinearGradient *lg = SP_LINEARGRADIENT (server);
                    sp_paint_selector_set_gradient_properties (psel,
                                                       SP_GRADIENT_UNITS (lg),
                                                       SP_GRADIENT_SPREAD (lg));
                } else if (SP_IS_RADIALGRADIENT (server)) {
                    SPGradient *vector = sp_gradient_get_vector (SP_GRADIENT (server), FALSE);
                    sp_paint_selector_set_gradient_radial (psel, vector);

                    SPRadialGradient *rg = SP_RADIALGRADIENT (server);
                    sp_paint_selector_set_gradient_properties (psel,
                                                       SP_GRADIENT_UNITS (rg),
                                                       SP_GRADIENT_SPREAD (rg));
                } else if (SP_IS_PATTERN (server)) {
                    SPPattern *pat = pattern_getroot (SP_PATTERN (server));
                    sp_update_pattern_list (psel, pat);
                }
            } else if (query->stroke.set && query->stroke.isColor()) {
                sp_paint_selector_set_color_alpha (psel, &query->stroke.value.color, SP_SCALE24_TO_FLOAT (query->stroke_opacity.value));

            }
            break;
        }

        case QUERY_STYLE_MULTIPLE_DIFFERENT:
        {
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);
            break;
        }
    }

    sp_style_unref(query);

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}

/**
 * When the mode is changed, invoke a regular changed handler.
 */
static void
sp_stroke_style_paint_mode_changed( SPPaintSelector *psel,
                                    SPPaintSelectorMode /*mode*/,
                                    SPWidget *spw )
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    /* TODO: Does this work?
     * Not really, here we have to get old color back from object
     * Instead of relying on paint widget having meaningful colors set
     */
    sp_stroke_style_paint_changed(psel, spw);
}

static gchar const *const undo_label_1 = "stroke:flatcolor:1";
static gchar const *const undo_label_2 = "stroke:flatcolor:2";
static gchar const *undo_label = undo_label_1;

/**
 * When a drag callback occurs on a paint selector object, if it is a RGB or CMYK
 * color mode, then set the stroke opacity to psel's flat color.
 */
static void
sp_stroke_style_paint_dragged(SPPaintSelector *psel, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            sp_paint_selector_set_flat_color (psel, SP_ACTIVE_DESKTOP, "stroke", "stroke-opacity");
            sp_document_maybe_done (sp_desktop_document(SP_ACTIVE_DESKTOP), undo_label, SP_VERB_DIALOG_FILL_STROKE,
                                    _("Set stroke color"));
            break;
        }

        default:
            g_warning( "file %s: line %d: Paint %d should not emit 'dragged'",
                       __FILE__, __LINE__, psel->mode);
            break;
    }
}

/**
 * When the stroke style's paint settings change, this handler updates the
 * repr's stroke css style and applies the style to relevant drawing items.
 */
static void
sp_stroke_style_paint_changed(SPPaintSelector *psel, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }
    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPDocument *document = sp_desktop_document (desktop);
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    GSList const *items = selection->itemList();

    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_EMPTY:
            // This should not happen.
            g_warning ( "file %s: line %d: Paint %d should not emit 'changed'",
                        __FILE__, __LINE__, psel->mode);
            break;
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
            // This happens when you switch multiple objects with different gradients to flat color;
            // nothing to do here.
            break;

        case SP_PAINT_SELECTOR_MODE_NONE:
        {
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, "stroke", "none");

            sp_desktop_set_style (desktop, css);

            sp_repr_css_attr_unref(css);

            sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                             _("Remove stroke"));
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            sp_paint_selector_set_flat_color (psel, desktop, "stroke", "stroke-opacity");
            sp_document_maybe_done (sp_desktop_document(desktop), undo_label, SP_VERB_DIALOG_FILL_STROKE,
                                    _("Set stroke color"));

            // on release, toggle undo_label so that the next drag will not be lumped with this one
            if (undo_label == undo_label_1)
                undo_label = undo_label_2;
            else
                undo_label = undo_label_1;

            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
            if (items) {
                SPGradientType const gradient_type = ( psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR
                                                       ? SP_GRADIENT_TYPE_LINEAR
                                                       : SP_GRADIENT_TYPE_RADIAL );
                SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);
                if (!vector) {
                    /* No vector in paint selector should mean that we just changed mode */

                    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
                    int result = objects_query_fillstroke ((GSList *) items, query, false);
                    guint32 common_rgb = 0;
                    if (result == QUERY_STYLE_MULTIPLE_SAME) {
                        if (!query->fill.isColor()) {
                            common_rgb = sp_desktop_get_color(desktop, false);
                        } else {
                            common_rgb = query->stroke.value.color.toRGBA32( 0xff );
                        }
                        vector = sp_document_default_gradient_vector(document, common_rgb);
                    }
                    sp_style_unref(query);

                    for (GSList const *i = items; i != NULL; i = i->next) {
                        if (!vector) {
                            sp_item_set_gradient(SP_ITEM(i->data),
                                                 sp_gradient_vector_for_object(document, desktop, SP_OBJECT(i->data), false),
                                                 gradient_type, false);
                        } else {
                            sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, false);
                        }
                    }
                } else {
                    vector = sp_gradient_ensure_vector_normalized(vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        SPGradient *gr = sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, false);
                        sp_gradient_selector_attrs_to_gradient(gr, psel);
                    }
                }

                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 _("Set gradient on stroke"));
            }
            break;

        case SP_PAINT_SELECTOR_MODE_PATTERN:

            if (items) {

                SPPattern *pattern = sp_paint_selector_get_pattern (psel);
                if (!pattern) {

                    /* No Pattern in paint selector should mean that we just
                     * changed mode - dont do jack.
                     */

                } else {
                    Inkscape::XML::Node *patrepr = SP_OBJECT_REPR(pattern);
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    gchar *urltext = g_strdup_printf ("url(#%s)", patrepr->attribute("id"));
                    sp_repr_css_set_property (css, "stroke", urltext);

                    for (GSList const *i = items; i != NULL; i = i->next) {
                         Inkscape::XML::Node *selrepr = SP_OBJECT_REPR (i->data);
                         SPObject *selobj = SP_OBJECT (i->data);
                         if (!selrepr)
                             continue;

                         SPStyle *style = SP_OBJECT_STYLE (selobj);
                         if (style && style->stroke.isPaintserver()) {
                             SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (selobj);
                             if (SP_IS_PATTERN (server) && pattern_getroot (SP_PATTERN(server)) == pattern)
                                 // only if this object's pattern is not rooted in our selected pattern, apply
                                 continue;
                         }

                         sp_repr_css_change_recursive (selrepr, css, "style");
                     }

                    sp_repr_css_attr_unref (css);
                    g_free (urltext);

                } // end if

                sp_document_done (document, SP_VERB_DIALOG_FILL_STROKE,
                                  _("Set pattern on stroke"));
            } // end if

            break;

        case SP_PAINT_SELECTOR_MODE_UNSET:
            if (items) {
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    sp_repr_css_unset_property (css, "stroke");
                    sp_repr_css_unset_property (css, "stroke-opacity");
                    sp_repr_css_unset_property (css, "stroke-width");
                    sp_repr_css_unset_property (css, "stroke-miterlimit");
                    sp_repr_css_unset_property (css, "stroke-linejoin");
                    sp_repr_css_unset_property (css, "stroke-linecap");
                    sp_repr_css_unset_property (css, "stroke-dashoffset");
                    sp_repr_css_unset_property (css, "stroke-dasharray");

                    sp_desktop_set_style (desktop, css);
                    sp_repr_css_attr_unref (css);

                    sp_document_done (document, SP_VERB_DIALOG_FILL_STROKE,
                                      _("Unset stroke"));
            }
            break;

        default:
            g_warning( "file %s: line %d: Paint selector should not be in "
                       "mode %d",
                       __FILE__, __LINE__,
                       psel->mode );
            break;
    }

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
}





/* Line */

static void sp_stroke_style_line_selection_modified(SPWidget *spw, Inkscape::Selection *selection, guint flags, gpointer data);
static void sp_stroke_style_line_selection_changed(SPWidget *spw, Inkscape::Selection *selection, gpointer data);

static void sp_stroke_style_line_update(Gtk::Container *spw, Inkscape::Selection *sel);

static void sp_stroke_style_set_join_buttons(Gtk::Container *spw, Gtk::ToggleButton *active);

static void sp_stroke_style_set_cap_buttons(Gtk::Container *spw, Gtk::ToggleButton *active);

static void sp_stroke_style_width_changed(Gtk::Container *spw);
static void sp_stroke_style_miterlimit_changed(Gtk::Container *spw);
static void sp_stroke_style_any_toggled(Gtk::ToggleButton *tb, Gtk::Container *spw);
static void sp_stroke_style_line_dash_changed(Gtk::Container *spw);

static void sp_stroke_style_update_marker_menus(Gtk::Container *spw, GSList const *objects);


/**
 * Helper function for creating radio buttons.  This should probably be re-thought out
 * when reimplementing this with Gtkmm.
 */
static Gtk::RadioButton *
sp_stroke_radio_button(Gtk::RadioButton *tb, char const *icon,
                       Gtk::HBox *hb, Gtk::Container *spw,
                       gchar const *key, gchar const *data)
{
    g_assert(icon != NULL);
    g_assert(hb  != NULL);
    g_assert(spw != NULL);

    if (tb == NULL) {
        tb = new Gtk::RadioButton();
    } else {
        Gtk::RadioButtonGroup grp = tb->get_group();
        tb = new Gtk::RadioButton(grp);
    }

    tb->show();
    tb->set_mode(false);
    hb->pack_start(*tb, false, false, 0);
    spw->set_data(icon, tb);
    tb->set_data(key, (gpointer*)data);
    tb->signal_toggled().connect(sigc::bind<Gtk::RadioButton *, Gtk::Container *>(
                                     sigc::ptr_fun(&sp_stroke_style_any_toggled), tb, spw));
    Gtk::Widget *px = manage(Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_LARGE_TOOLBAR, icon)));
    g_assert(px != NULL);
    px->show();
    tb->add(*px);

    return tb;

}

static void
sp_stroke_style_widget_transientize_callback(Inkscape::Application */*inkscape*/,
                                             SPDesktop */*desktop*/,
                                             SPWidget */*spw*/ )
{
// TODO:  Either of these will cause crashes sometimes
//    sp_stroke_style_line_update( SP_WIDGET(spw), desktop ? sp_desktop_selection(desktop) : NULL);
//    ink_markers_menu_update(spw);
}

/**
 * Creates a copy of the marker named mname, determines its visible and renderable
 * area in menu_id's bounding box, and then renders it.  This allows us to fill in
 * preview images of each marker in the marker menu.
 */
static Gtk::Image *
sp_marker_prev_new(unsigned psize, gchar const *mname,
                   SPDocument *source, SPDocument *sandbox,
                   gchar const *menu_id, NRArena const */*arena*/, unsigned /*visionkey*/, NRArenaItem *root)
{
    // Retrieve the marker named 'mname' from the source SVG document
    SPObject const *marker = source->getObjectById(mname);
    if (marker == NULL)
        return NULL;

    // Create a copy repr of the marker with id="sample"
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(sandbox);
    Inkscape::XML::Node *mrepr = SP_OBJECT_REPR (marker)->duplicate(xml_doc);
    mrepr->setAttribute("id", "sample");

    // Replace the old sample in the sandbox by the new one
    Inkscape::XML::Node *defsrepr = SP_OBJECT_REPR (sandbox->getObjectById("defs"));
    SPObject *oldmarker = sandbox->getObjectById("sample");
    if (oldmarker)
        oldmarker->deleteObject(false);
    defsrepr->appendChild(mrepr);
    Inkscape::GC::release(mrepr);

// Uncomment this to get the sandbox documents saved (useful for debugging)
    //FILE *fp = fopen (g_strconcat(menu_id, mname, ".svg", NULL), "w");
    //sp_repr_save_stream (sp_document_repr_doc (sandbox), fp);
    //fclose (fp);

    // object to render; note that the id is the same as that of the menu we're building
    SPObject *object = sandbox->getObjectById(menu_id);
    sp_document_root (sandbox)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    sp_document_ensure_up_to_date(sandbox);

    if (object == NULL || !SP_IS_ITEM(object))
        return NULL; // sandbox broken?

    // Find object's bbox in document
    Geom::Matrix const i2doc(sp_item_i2doc_affine(SP_ITEM(object)));
    boost::optional<NR::Rect> dbox = SP_ITEM(object)->getBounds(i2doc);

    if (!dbox) {
        return NULL;
    }

    /* Update to renderable state */
    double sf = 0.8;

    gchar *cache_name = g_strconcat(menu_id, mname, NULL);
    Glib::ustring key = svg_preview_cache.cache_key(source->uri, cache_name, psize);
    g_free (cache_name);
    // TODO: is this correct?
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Glib::wrap(svg_preview_cache.get_preview_from_cache(key));

    if (!pixbuf) {
        pixbuf = Glib::wrap(render_pixbuf(root, sf, to_2geom(*dbox), psize));
        svg_preview_cache.set_preview_in_cache(key, pixbuf->gobj());
    }

    // Create widget
    Gtk::Image *pb = new Gtk::Image(pixbuf);

    return pb;
}

/**
 *  Returns a list of markers in the defs of the given source document as a GSList object
 *  Returns NULL if there are no markers in the document.
 */
GSList *
ink_marker_list_get (SPDocument *source)
{
    if (source == NULL)
        return NULL;

    GSList *ml   = NULL;
    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS (source);
    for ( SPObject *child = sp_object_first_child(SP_OBJECT(defs));
          child != NULL;
          child = SP_OBJECT_NEXT (child) )
    {
        if (SP_IS_MARKER(child)) {
            ml = g_slist_prepend (ml, child);
        }
    }
    return ml;
}

#define MARKER_ITEM_MARGIN 0

/**
 * Adds previews of markers in marker_list to the given menu widget
 */
static void
sp_marker_menu_build (Gtk::Menu *m, GSList *marker_list, SPDocument *source, SPDocument *sandbox, gchar const *menu_id)
{
    // Do this here, outside of loop, to speed up preview generation:
    NRArena const *arena = NRArena::create();
    unsigned const visionkey = sp_item_display_key_new(1);
    NRArenaItem *root =  sp_item_invoke_show(SP_ITEM(SP_DOCUMENT_ROOT (sandbox)), (NRArena *) arena, visionkey, SP_ITEM_SHOW_DISPLAY);

    for (; marker_list != NULL; marker_list = marker_list->next) {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) marker_list->data);
        Gtk::MenuItem *i = new Gtk::MenuItem();
        i->show();

        if (repr->attribute("inkscape:stockid"))
            i->set_data("stockid", (void *) "true");
        else
            i->set_data("stockid", (void *) "false");

        gchar const *markid = repr->attribute("id");
        i->set_data("marker", (void *) markid);

        Gtk::HBox *hb = new Gtk::HBox(false, MARKER_ITEM_MARGIN);
        hb->show();

        // generate preview

        Gtk::Image *prv = sp_marker_prev_new (22, markid, source, sandbox, menu_id, arena, visionkey, root);
        prv->show();
        hb->pack_start(*prv, false, false, 6);

        // create label
        Gtk::Label *l = new Gtk::Label(repr->attribute("id"));
        l->show();
        l->set_alignment(0.0, 0.5);

        hb->pack_start(*l, true, true, 0);

        hb->show();
        i->add(*hb);

        m->append(*i);
    }

    sp_item_invoke_hide(SP_ITEM(sp_document_root(sandbox)), visionkey);
    nr_object_unref((NRObject *) arena);
}

/**
 * sp_marker_list_from_doc()
 *
 * \brief Pick up all markers from source, except those that are in
 * current_doc (if non-NULL), and add items to the m menu
 *
 */
static void
sp_marker_list_from_doc (Gtk::Menu *m, SPDocument */*current_doc*/, SPDocument *source, SPDocument */*markers_doc*/, SPDocument *sandbox, gchar const *menu_id)
{
    GSList *ml = ink_marker_list_get(source);
    GSList *clean_ml = NULL;

    for (; ml != NULL; ml = ml->next) {
        if (!SP_IS_MARKER(ml->data))
            continue;

        // Add to the list of markers we really do wish to show
        clean_ml = g_slist_prepend (clean_ml, ml->data);
    }
    sp_marker_menu_build(m, clean_ml, source, sandbox, menu_id);

    g_slist_free (ml);
    g_slist_free (clean_ml);
}

/**
 * Returns a new document containing default start, mid, and end markers.
 */
SPDocument *
ink_markers_preview_doc ()
{
gchar const *buffer = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
"  <defs id=\"defs\" />"

"  <g id=\"marker-start\">"
"    <path style=\"fill:none;stroke:black;stroke-width:1.7;marker-start:url(#sample);marker-mid:none;marker-end:none\""
"       d=\"M 12.5,13 L 25,13\" id=\"path1\" />"
"    <rect style=\"fill:none;stroke:none\" id=\"rect2\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"0\" />"
"  </g>"

"  <g id=\"marker-mid\">"
"    <path style=\"fill:none;stroke:black;stroke-width:1.7;marker-start:none;marker-mid:url(#sample);marker-end:none\""
"       d=\"M 0,113 L 12.5,113 L 25,113\" id=\"path11\" />"
"    <rect style=\"fill:none;stroke:none\" id=\"rect22\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"100\" />"
"  </g>"

"  <g id=\"marker-end\">"
"    <path style=\"fill:none;stroke:black;stroke-width:1.7;marker-start:none;marker-mid:none;marker-end:url(#sample)\""
"       d=\"M 0,213 L 12.5,213\" id=\"path111\" />"
"    <rect style=\"fill:none;stroke:none\" id=\"rect222\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"200\" />"
"  </g>"

"</svg>";

    return sp_document_new_from_mem (buffer, strlen(buffer), FALSE);
}

static void
ink_marker_menu_create_menu(Gtk::Menu *m, gchar const *menu_id, SPDocument *doc, SPDocument *sandbox)
{
    static SPDocument *markers_doc = NULL;

    // add "None"
    Gtk::MenuItem *i = new Gtk::MenuItem();
    i->show();

    i->set_data("marker", (void *) "none");

    Gtk::HBox *hb = new Gtk::HBox(false,  MARKER_ITEM_MARGIN);
    hb->show();

    Gtk::Label *l = new Gtk::Label( _("None") );
    l->show();
    l->set_alignment(0.0, 0.5);

    hb->pack_start(*l, true, true, 0);

    hb->show();
    i->add(*hb);
    m->append(*i);

    // find and load markers.svg
    if (markers_doc == NULL) {
        char *markers_source = g_build_filename(INKSCAPE_MARKERSDIR, "markers.svg", NULL);
        if (Inkscape::IO::file_test(markers_source, G_FILE_TEST_IS_REGULAR)) {
            markers_doc = sp_document_new(markers_source, FALSE);
        }
        g_free(markers_source);
    }

    // suck in from current doc
    sp_marker_list_from_doc(m, NULL, doc, markers_doc, sandbox, menu_id);

    // add separator
    {
        //Gtk::Separator *i = gtk_separator_menu_item_new();
        Gtk::SeparatorMenuItem *i = new Gtk::SeparatorMenuItem();
        i->show();
        m->append(*i);
    }

    // suck in from markers.svg
    if (markers_doc) {
        sp_document_ensure_up_to_date(doc);
        sp_marker_list_from_doc(m, doc, markers_doc, NULL, sandbox, menu_id);
    }

}

/**
 * Creates a menu widget to display markers from markers.svg
 */
static Gtk::OptionMenu *
ink_marker_menu(Gtk::Widget */*tbl*/, gchar const *menu_id, SPDocument *sandbox)
{
    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *doc = sp_desktop_document(desktop);
    Gtk::OptionMenu *mnu = new Gtk::OptionMenu();

    /* Create new menu widget */
    Gtk::Menu *m = new Gtk::Menu();
    m->show();

    mnu->set_data("updating", (gpointer) FALSE);

    if (!doc) {
        Gtk::MenuItem *i = new Gtk::MenuItem(_("No document selected"));
        i->show();
        m->append(*i);
        mnu->set_sensitive(false);

    } else {
        ink_marker_menu_create_menu(m, menu_id, doc, sandbox);

        mnu->set_sensitive(true);
    }

    mnu->set_data("menu_id", const_cast<gchar *>(menu_id));
    mnu->set_menu(*m);

    /* Set history */
    mnu->set_history(0);

    return mnu;
}

/**
 * Handles when user selects one of the markers from the marker menu.
 * Defines a uri string to refer to it, then applies it to all selected
 * items in the current desktop.
 */
static void
sp_marker_select(Gtk::OptionMenu *mnu, Gtk::Container *spw, SPMarkerLoc const which)
{
    if (spw->get_data("update")) {
        return;
    }

    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *document = sp_desktop_document(desktop);
    if (!document) {
        return;
    }

    /* Get Marker */
    if (!mnu->get_menu()->get_active()->get_data("marker"))
    {
        return;
    }
    gchar *markid = static_cast<gchar *>(mnu->get_menu()->get_active()->get_data("marker"));
    gchar const *marker = "";
    if (strcmp(markid, "none")) {
       gchar *stockid = static_cast<gchar *>(mnu->get_menu()->get_active()->get_data("stockid"));

       gchar *markurn = markid;
       if (!strcmp(stockid,"true")) markurn = g_strconcat("urn:inkscape:marker:",markid,NULL);
       SPObject *mark = get_stock_item(markurn);
       if (mark) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR(mark);
            marker = g_strconcat("url(#", repr->attribute("id"), ")", NULL);
        }
    } else {
        marker = markid;
    }
    SPCSSAttr *css = sp_repr_css_attr_new();
    gchar const *menu_id = static_cast<gchar const *>(mnu->get_data("menu_id"));
    sp_repr_css_set_property(css, menu_id, marker);

    // Also update the marker dropdown menus, so the document's markers
    // show up at the top of the menu
//    sp_stroke_style_line_update( SP_WIDGET(spw), desktop ? sp_desktop_selection(desktop) : NULL);
    ink_markers_menu_update(spw, which);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
         SPItem *item = (SPItem *) items->data;
         if (!SP_IS_SHAPE(item) || SP_IS_RECT(item)) // can't set marker to rect, until it's converted to using <path>
             continue;
         Inkscape::XML::Node *selrepr = SP_OBJECT_REPR((SPItem *) items->data);
         if (selrepr) {
             sp_repr_css_change_recursive(selrepr, css, "style");
         }
         SP_OBJECT(items->data)->requestModified(SP_OBJECT_MODIFIED_FLAG);
         SP_OBJECT(items->data)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
     }

    sp_repr_css_attr_unref(css);

    sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                     _("Set markers"));

};

static unsigned int
ink_marker_menu_get_pos(Gtk::Menu *mnu, gchar const *markname)
{
    if (markname == NULL)
        markname = static_cast<gchar const *>(mnu->get_active()->get_data("marker"));

    if (markname == NULL)
        return 0;

    std::vector<Gtk::Widget *> kids = mnu->get_children();
    unsigned int i = 0;
    for (; i < kids.size();) {
        gchar const *mark = static_cast<gchar const *>(kids[i]->get_data("marker"));
        if (mark && strcmp(mark, markname) == 0) {
            break;
        }
        ++i;
    }

    return i;
}

static void
ink_markers_menu_update(Gtk::Container* /*spw*/, SPMarkerLoc const which) {
    SPDesktop  *desktop = inkscape_active_desktop();
    SPDocument *document = sp_desktop_document(desktop);
    SPDocument *sandbox = ink_markers_preview_doc ();
    Gtk::Menu  *m;
    int        pos;

    // TODO: this code can be shortened by abstracting out marker_(start|mid|end)_...
    switch (which) {
        case SP_MARKER_LOC_START:
            marker_start_menu_connection.block();
            pos = ink_marker_menu_get_pos(marker_start_menu->get_menu(), NULL);
            m = new Gtk::Menu();
            m->show();
            ink_marker_menu_create_menu(m, "marker-start", document, sandbox);
            marker_start_menu->remove_menu();
            marker_start_menu->set_menu(*m);
            marker_start_menu->set_history(pos);
            marker_start_menu_connection.unblock();
            break;

        case SP_MARKER_LOC_MID:
            marker_mid_menu_connection.block();
            pos = ink_marker_menu_get_pos(marker_mid_menu->get_menu(), NULL);
            m = new Gtk::Menu();
            m->show();
            ink_marker_menu_create_menu(m, "marker-mid", document, sandbox);
            marker_mid_menu->remove_menu();
            marker_mid_menu->set_menu(*m);
            marker_mid_menu->set_history(pos);
            marker_mid_menu_connection.unblock();
            break;

        case SP_MARKER_LOC_END:
            marker_end_menu_connection.block();
            pos = ink_marker_menu_get_pos(marker_end_menu->get_menu(), NULL);
            m = new Gtk::Menu();
            m->show();
            ink_marker_menu_create_menu(m, "marker-end", document, sandbox);
            marker_end_menu->remove_menu();
            marker_end_menu->set_menu(*m);
            marker_end_menu->set_history(pos);
            marker_end_menu_connection.unblock();
            break;
        default:
            g_assert_not_reached();
    }
}

/**
 * Sets the stroke width units for all selected items.
 * Also handles absolute and dimensionless units.
 */
static gboolean stroke_width_set_unit(SPUnitSelector *,
                                      SPUnit const *old,
                                      SPUnit const *new_units,
                                      Gtk::Container *spw)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (!desktop) {
        return FALSE;
    }

    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    if (selection->isEmpty())
        return FALSE;

    GSList const *objects = selection->itemList();

    if ((old->base == SP_UNIT_ABSOLUTE || old->base == SP_UNIT_DEVICE) &&
       (new_units->base == SP_UNIT_DIMENSIONLESS)) {

        /* Absolute to percentage */
        spw->set_data ("update", GUINT_TO_POINTER (TRUE));

        Gtk::Adjustment *a = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
        float w = sp_units_get_pixels (a->get_value(), *old);

        gdouble average = stroke_average_width (objects);

        if (average == NR_HUGE || average == 0)
            return FALSE;

        a->set_value (100.0 * w / average);

        spw->set_data ("update", GUINT_TO_POINTER (FALSE));
        return TRUE;

    } else if ((old->base == SP_UNIT_DIMENSIONLESS) &&
              (new_units->base == SP_UNIT_ABSOLUTE || new_units->base == SP_UNIT_DEVICE)) {

        /* Percentage to absolute */
        spw->set_data ("update", GUINT_TO_POINTER (TRUE));

        Gtk::Adjustment *a = static_cast<Gtk::Adjustment *>(spw->get_data ("width"));

        gdouble average = stroke_average_width (objects);

        a->set_value (sp_pixels_get_units (0.01 * a->get_value() * average, *new_units));

        spw->set_data ("update", GUINT_TO_POINTER (FALSE));
        return TRUE;
    }

    return FALSE;
}


/**
 * \brief  Creates a new widget for the line stroke style.
 *
 */
Gtk::Container *
sp_stroke_style_line_widget_new(void)
{
    Gtk::Widget *us;
    SPDashSelector *ds;
    GtkWidget *us_old, *spw_old;
    Gtk::Container *spw;
    Gtk::Table *t;
    Gtk::Adjustment *a;
    Gtk::SpinButton *sb;
    Gtk::RadioButton *tb;
    Gtk::HBox *f, *hb;

    Gtk::Tooltips *tt = new Gtk::Tooltips();

    spw_old = sp_widget_new_global(INKSCAPE);
    spw = dynamic_cast<Gtk::Container *>(manage(Glib::wrap(spw_old)));

    f = new Gtk::HBox(false, 0);
    f->show();
    spw->add(*f);

    t = new Gtk::Table(3, 6, false);
    t->show();
    t->set_border_width(4);
    t->set_row_spacings(4);
    f->add(*t);
    spw->set_data("stroke", t);

    gint i = 0;

    /* Stroke width */
    spw_label(t, _("Width:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

// TODO: when this is gtkmmified, use an Inkscape::UI::Widget::ScalarUnit instead of the separate
// spinbutton and unit selector for stroke width. In sp_stroke_style_line_update, use
// setHundredPercent to remember the aeraged width corresponding to 100%. Then the
// stroke_width_set_unit will be removed (because ScalarUnit takes care of conversions itself), and
// with it, the two remaining calls of stroke_average_width, allowing us to get rid of that
// function in desktop-style.

    a = new Gtk::Adjustment(1.0, 0.0, 1000.0, 0.1, 10.0, 10.0);
    spw->set_data("width", a);
    sb = new Gtk::SpinButton(*a, 0.1, 3);
    tt->set_tip(*sb, _("Stroke width"));
    sb->show();

    sp_dialog_defocus_on_enter_cpp(sb);

    hb->pack_start(*sb, false, false, 0);
    us_old = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
    us = manage(Glib::wrap(us_old));
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop)
        sp_unit_selector_set_unit (SP_UNIT_SELECTOR(us_old), sp_desktop_namedview(desktop)->doc_units);
    sp_unit_selector_add_unit(SP_UNIT_SELECTOR(us_old), &sp_unit_get_by_id(SP_UNIT_PERCENT), 0);
    g_signal_connect ( G_OBJECT (us_old), "set_unit", G_CALLBACK (stroke_width_set_unit), spw );
    us->show();
    sp_unit_selector_add_adjustment( SP_UNIT_SELECTOR(us_old), GTK_ADJUSTMENT(a->gobj()) );
    hb->pack_start(*us, FALSE, FALSE, 0);
    spw->set_data("units", us_old);

    a->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_width_changed), spw));
    i++;

    /* Join type */
    // TRANSLATORS: The line join style specifies the shape to be used at the
    //  corners of paths. It can be "miter", "round" or "bevel".
    spw_label(t, _("Join:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_MITER,
                                hb, spw, "join", "miter");

    // TRANSLATORS: Miter join: joining lines with a sharp (pointed) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    tt->set_tip(*tb, _("Miter join"));

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_ROUND,
                                hb, spw, "join", "round");

    // TRANSLATORS: Round join: joining lines with a rounded corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    tt->set_tip(*tb, _("Round join"));

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_BEVEL,
                                hb, spw, "join", "bevel");

    // TRANSLATORS: Bevel join: joining lines with a blunted (flattened) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    tt->set_tip(*tb, _("Bevel join"));

    i++;

    /* Miterlimit  */
    // TRANSLATORS: Miter limit: only for "miter join", this limits the length
    //  of the sharp "spike" when the lines connect at too sharp an angle.
    // When two line segments meet at a sharp angle, a miter join results in a
    //  spike that extends well beyond the connection point. The purpose of the
    //  miter limit is to cut off such spikes (i.e. convert them into bevels)
    //  when they become too long.
    spw_label(t, _("Miter limit:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    a = new Gtk::Adjustment(4.0, 0.0, 100.0, 0.1, 10.0, 10.0);
    spw->set_data("miterlimit", a);

    sb = new Gtk::SpinButton(*a, 0.1, 2);
    tt->set_tip(*sb, _("Maximum length of the miter (in units of stroke width)"));
    sb->show();
    spw->set_data("miterlimit_sb", sb);
    sp_dialog_defocus_on_enter_cpp(sb);

    hb->pack_start(*sb, false, false, 0);

    a->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_miterlimit_changed), spw));
    i++;

    /* Cap type */
    // TRANSLATORS: cap type specifies the shape for the ends of lines
    spw_label(t, _("Cap:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_BUTT,
                                hb, spw, "cap", "butt");

    // TRANSLATORS: Butt cap: the line shape does not extend beyond the end point
    //  of the line; the ends of the line are square
    tt->set_tip(*tb, _("Butt cap"));

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_ROUND,
                                hb, spw, "cap", "round");

    // TRANSLATORS: Round cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are rounded
    tt->set_tip(*tb, _("Round cap"));

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_SQUARE,
                                hb, spw, "cap", "square");

    // TRANSLATORS: Square cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are square
    tt->set_tip(*tb, _("Square cap"));

    i++;


    /* Dash */
    spw_label(t, _("Dashes:"), 0, i);
    ds = manage(new SPDashSelector(inkscape_get_repr(INKSCAPE, "palette.dashes")));

    ds->show();
    t->attach(*ds, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("dash", ds);
    ds->changed_signal.connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_line_dash_changed), spw));
    i++;

    /* Drop down marker selectors*/
    // TODO: this code can be shortened by iterating over the possible menus!

    // doing this here once, instead of for each preview, to speed things up
    SPDocument *sandbox = ink_markers_preview_doc ();

    // TRANSLATORS: Path markers are an SVG feature that allows you to attach arbitrary shapes
    // (arrowheads, bullets, faces, whatever) to the start, end, or middle nodes of a path.
    spw_label(t, _("Start Markers:"), 0, i);
    marker_start_menu = ink_marker_menu(spw ,"marker-start", sandbox);
    marker_start_menu_connection = marker_start_menu->signal_changed().connect(
        sigc::bind<Gtk::OptionMenu *, Gtk::Container *, SPMarkerLoc>(
            sigc::ptr_fun(&sp_marker_select), marker_start_menu, spw, SP_MARKER_LOC_START));
    marker_start_menu->show();
    t->attach(*marker_start_menu, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("start_mark_menu", marker_start_menu);

    i++;
    spw_label(t, _("Mid Markers:"), 0, i);
    marker_mid_menu = ink_marker_menu(spw ,"marker-mid", sandbox);
    marker_mid_menu_connection = marker_mid_menu->signal_changed().connect(
        sigc::bind<Gtk::OptionMenu *, Gtk::Container *, SPMarkerLoc>(
            sigc::ptr_fun(&sp_marker_select), marker_mid_menu,spw, SP_MARKER_LOC_MID));
    marker_mid_menu->show();
    t->attach(*marker_mid_menu, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("mid_mark_menu", marker_mid_menu);

    i++;
    spw_label(t, _("End Markers:"), 0, i);
    marker_end_menu = ink_marker_menu(spw ,"marker-end", sandbox);
    marker_end_menu_connection = marker_end_menu->signal_changed().connect(
        sigc::bind<Gtk::OptionMenu *, Gtk::Container *, SPMarkerLoc>(
            sigc::ptr_fun(&sp_marker_select), marker_end_menu, spw, SP_MARKER_LOC_END));
    marker_end_menu->show();
    t->attach(*marker_end_menu, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("end_mark_menu", marker_end_menu);

    i++;

    // FIXME: we cheat and still use gtk+ signals

    gtk_signal_connect(GTK_OBJECT(spw_old), "modify_selection",
                       GTK_SIGNAL_FUNC(sp_stroke_style_line_selection_modified),
                       spw);
    gtk_signal_connect(GTK_OBJECT(spw_old), "change_selection",
                       GTK_SIGNAL_FUNC(sp_stroke_style_line_selection_changed),
                       spw);

    sp_stroke_style_line_update(spw, desktop ? sp_desktop_selection(desktop) : NULL);

    return spw;
}

/**
 * Callback for when stroke style widget is modified.
 * Triggers update action.
 */
static void
sp_stroke_style_line_selection_modified(SPWidget *,
                                        Inkscape::Selection *selection,
                                        guint flags,
                                        gpointer data)
{
    Gtk::Container *spw = static_cast<Gtk::Container *>(data);
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        sp_stroke_style_line_update(spw, selection);
    }

}

/**
 * Callback for when stroke style widget is changed.
 * Triggers update action.
 */
static void
sp_stroke_style_line_selection_changed(SPWidget *,
                                       Inkscape::Selection *selection,
                                       gpointer data)
{
    Gtk::Container *spw = static_cast<Gtk::Container *>(data);
    sp_stroke_style_line_update(spw, selection);
}

/**
 * Sets selector widgets' dash style from an SPStyle object.
 */
static void
sp_dash_selector_set_from_style(SPDashSelector *dsel, SPStyle *style)
{
    if (style->stroke_dash.n_dash > 0) {
        double d[64];
        int len = MIN(style->stroke_dash.n_dash, 64);
        for (int i = 0; i < len; i++) {
            if (style->stroke_width.computed != 0)
                d[i] = style->stroke_dash.dash[i] / style->stroke_width.computed;
            else
                d[i] = style->stroke_dash.dash[i]; // is there a better thing to do for stroke_width==0?
        }
        dsel->set_dash(len, d, style->stroke_width.computed != 0 ?
                       style->stroke_dash.offset / style->stroke_width.computed  :
                       style->stroke_dash.offset);
    } else {
        dsel->set_dash(0, NULL, 0.0);
    }
}

/**
 * Sets the join type for a line, and updates the stroke style widget's buttons
 */
static void
sp_jointype_set (Gtk::Container *spw, unsigned const jointype)
{
    Gtk::RadioButton *tb = NULL;
    switch (jointype) {
        case SP_STROKE_LINEJOIN_MITER:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_JOIN_MITER));
            break;
        case SP_STROKE_LINEJOIN_ROUND:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_JOIN_ROUND));
            break;
        case SP_STROKE_LINEJOIN_BEVEL:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_JOIN_BEVEL));
            break;
        default:
            break;
    }
    sp_stroke_style_set_join_buttons(spw, tb);
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
static void
sp_captype_set (Gtk::Container *spw, unsigned const captype)
{
    Gtk::RadioButton *tb = NULL;
    switch (captype) {
        case SP_STROKE_LINECAP_BUTT:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_CAP_BUTT));
            break;
        case SP_STROKE_LINECAP_ROUND:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_CAP_ROUND));
            break;
        case SP_STROKE_LINECAP_SQUARE:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_CAP_SQUARE));
            break;
        default:
            break;
    }
    sp_stroke_style_set_cap_buttons(spw, tb);
}

/**
 * Callback for when stroke style widget is updated, including markers, cap type,
 * join type, etc.
 */
static void
sp_stroke_style_line_update(Gtk::Container *spw, Inkscape::Selection *sel)
{
    if (spw->get_data("update")) {
        return;
    }

    spw->set_data("update", GINT_TO_POINTER(TRUE));

    Gtk::Table *sset = static_cast<Gtk::Table *>(spw->get_data("stroke"));
    Gtk::Adjustment *width = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
    Gtk::Adjustment *ml = static_cast<Gtk::Adjustment *>(spw->get_data("miterlimit"));
    SPUnitSelector *us = SP_UNIT_SELECTOR(spw->get_data("units"));
    SPDashSelector *dsel = static_cast<SPDashSelector *>(spw->get_data("dash"));

    // create temporary style
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    // query into it
    int result_sw = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
    int result_ml = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKEMITERLIMIT);
    int result_cap = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKECAP);
    int result_join = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKEJOIN);

    if (result_sw == QUERY_STYLE_NOTHING) {
        /* No objects stroked, set insensitive */
        sset->set_sensitive(false);

        spw->set_data("update", GINT_TO_POINTER(FALSE));
        return;
    } else {
        sset->set_sensitive(true);

        SPUnit const *unit = sp_unit_selector_get_unit(us);

        if (result_sw == QUERY_STYLE_MULTIPLE_AVERAGED) {
            sp_unit_selector_set_unit(us, &sp_unit_get_by_id(SP_UNIT_PERCENT));
        } else {
            // same width, or only one object; no sense to keep percent, switch to absolute
            if (unit->base != SP_UNIT_ABSOLUTE && unit->base != SP_UNIT_DEVICE) {
                sp_unit_selector_set_unit(us, sp_desktop_namedview(SP_ACTIVE_DESKTOP)->doc_units);
            }
        }

        unit = sp_unit_selector_get_unit(us);

        if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
            double avgwidth = sp_pixels_get_units (query->stroke_width.computed, *unit);
            width->set_value(avgwidth);
        } else {
            width->set_value(100);
        }
    }

    if (result_ml != QUERY_STYLE_NOTHING)
        ml->set_value(query->stroke_miterlimit.value); // TODO: reflect averagedness?

    if (result_join != QUERY_STYLE_MULTIPLE_DIFFERENT) {
        sp_jointype_set(spw, query->stroke_linejoin.value);
    } else {
        sp_stroke_style_set_join_buttons(spw, NULL);
    }

    if (result_cap != QUERY_STYLE_MULTIPLE_DIFFERENT) {
        sp_captype_set (spw, query->stroke_linecap.value);
    } else {
        sp_stroke_style_set_cap_buttons(spw, NULL);
    }

    sp_style_unref(query);

    if (!sel || sel->isEmpty())
        return;

    GSList const *objects = sel->itemList();
    SPObject * const object = SP_OBJECT(objects->data);
    SPStyle * const style = SP_OBJECT_STYLE(object);

    /* Markers */
    sp_stroke_style_update_marker_menus(spw, objects); // FIXME: make this desktop query too

    /* Dash */
    sp_dash_selector_set_from_style(dsel, style); // FIXME: make this desktop query too

    sset->set_sensitive(true);

    spw->set_data("update", GINT_TO_POINTER(FALSE));
}

/**
 * Sets a line's dash properties in a CSS style object.
 */
static void
sp_stroke_style_set_scaled_dash(SPCSSAttr *css,
                                int ndash, double *dash, double offset,
                                double scale)
{
    if (ndash > 0) {
        Inkscape::CSSOStringStream osarray;
        for (int i = 0; i < ndash; i++) {
            osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
                osarray << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", osarray.str().c_str());

        Inkscape::CSSOStringStream osoffset;
        osoffset << offset * scale;
        sp_repr_css_set_property(css, "stroke-dashoffset", osoffset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", NULL);
    }
}

/**
 * Sets line properties like width, dashes, markers, etc. on all currently selected items.
 */
static void
sp_stroke_style_scale_line(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    spw->set_data("update", GINT_TO_POINTER(TRUE));

    Gtk::Adjustment *wadj = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
    SPUnitSelector *us = SP_UNIT_SELECTOR(spw->get_data("units"));
    SPDashSelector *dsel = static_cast<SPDashSelector *>(spw->get_data("dash"));
    Gtk::Adjustment *ml = static_cast<Gtk::Adjustment *>(spw->get_data("miterlimit"));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPDocument *document = sp_desktop_document (desktop);
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    GSList const *items = selection->itemList();

    /* TODO: Create some standardized method */
    SPCSSAttr *css = sp_repr_css_attr_new();

    if (items) {

        double width_typed = wadj->get_value();
        double const miterlimit = ml->get_value();

        SPUnit const *const unit = sp_unit_selector_get_unit(SP_UNIT_SELECTOR(us));

        double *dash, offset;
        int ndash;
        dsel->get_dash(&ndash, &dash, &offset);

        for (GSList const *i = items; i != NULL; i = i->next) {
            /* Set stroke width */
            double width;
            if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
                width = sp_units_get_pixels (width_typed, *unit);
            } else { // percentage
                gdouble old_w = SP_OBJECT_STYLE (i->data)->stroke_width.computed;
                width = old_w * width_typed / 100;
            }

            {
                Inkscape::CSSOStringStream os_width;
                os_width << width;
                sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());
            }

            {
                Inkscape::CSSOStringStream os_ml;
                os_ml << miterlimit;
                sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());
            }

            /* Set dash */
            sp_stroke_style_set_scaled_dash(css, ndash, dash, offset, width);

            sp_desktop_apply_css_recursive (SP_OBJECT(i->data), css, true);
        }

        g_free(dash);

        if (unit->base != SP_UNIT_ABSOLUTE && unit->base != SP_UNIT_DEVICE) {
            // reset to 100 percent
            wadj->set_value(100.0);
        }

    }

    // we have already changed the items, so set style without changing selection
    // FIXME: move the above stroke-setting stuff, including percentages, to desktop-style
    sp_desktop_set_style (desktop, css, false);

    sp_repr_css_attr_unref(css);

    sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                     _("Set stroke style"));

    spw->set_data("update", GINT_TO_POINTER(FALSE));
}

/**
 * Callback for when the stroke style's width changes.
 * Causes all line styles to be applied to all selected items.
 */
static void
sp_stroke_style_width_changed(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

/**
 * Callback for when the stroke style's miterlimit changes.
 * Causes all line styles to be applied to all selected items.
 */
static void
sp_stroke_style_miterlimit_changed(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

/**
 * Callback for when the stroke style's dash changes.
 * Causes all line styles to be applied to all selected items.
 */

static void
sp_stroke_style_line_dash_changed(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

/**
 * \brief  This routine handles toggle events for buttons in the stroke style
 *         dialog.
 * When activated, this routine gets the data for the various widgets, and then
 * calls the respective routines to update css properties, etc.
 *
 */
static void
sp_stroke_style_any_toggled(Gtk::ToggleButton *tb, Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    if (tb->get_active()) {

        gchar const *join
            = static_cast<gchar const *>(tb->get_data("join"));
        gchar const *cap
            = static_cast<gchar const *>(tb->get_data("cap"));

        if (join) {
            Gtk::SpinButton *ml = static_cast<Gtk::SpinButton *>(spw->get_data("miterlimit_sb"));
            ml->set_sensitive(!strcmp(join, "miter"));
        }

        SPDesktop *desktop = SP_ACTIVE_DESKTOP;

        /* TODO: Create some standardized method */
        SPCSSAttr *css = sp_repr_css_attr_new();

        if (join) {
            sp_repr_css_set_property(css, "stroke-linejoin", join);

            sp_desktop_set_style (desktop, css);

            sp_stroke_style_set_join_buttons(spw, tb);
        } else if (cap) {
            sp_repr_css_set_property(css, "stroke-linecap", cap);

            sp_desktop_set_style (desktop, css);

            sp_stroke_style_set_cap_buttons(spw, tb);
        }

        sp_repr_css_attr_unref(css);

        sp_document_done(sp_desktop_document(desktop), SP_VERB_DIALOG_FILL_STROKE,
                         _("Set stroke style"));
    }
}

/**
 * Updates the join style toggle buttons
 */
static void
sp_stroke_style_set_join_buttons(Gtk::Container *spw, Gtk::ToggleButton *active)
{
    Gtk::RadioButton *tb;

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_JOIN_MITER));
    tb->set_active(active == tb);

    Gtk::SpinButton *ml = static_cast<Gtk::SpinButton *>(spw->get_data("miterlimit_sb"));
    ml->set_sensitive(active == tb);

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_JOIN_ROUND));
    tb->set_active(active == tb);

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_JOIN_BEVEL));
    tb->set_active(active == tb);
}

/**
 * Updates the cap style toggle buttons
 */
static void
sp_stroke_style_set_cap_buttons(Gtk::Container *spw, Gtk::ToggleButton *active)
{
    Gtk::RadioButton *tb;

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_CAP_BUTT));
    tb->set_active(active == tb);
    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_CAP_ROUND));
    tb->set_active(active == tb);
    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_STOCK_CAP_SQUARE));
    tb->set_active(active == tb);
}

/**
 * Sets the current marker in the marker menu.
 */
static void
ink_marker_menu_set_current(SPObject *marker, Gtk::OptionMenu *mnu)
{
    mnu->set_data("update", GINT_TO_POINTER(TRUE));

    Gtk::Menu *m = mnu->get_menu();
    if (marker != NULL) {
        bool mark_is_stock = false;
        if (SP_OBJECT_REPR(marker)->attribute("inkscape:stockid"))
            mark_is_stock = true;

        gchar *markname;
        if (mark_is_stock)
            markname = g_strdup(SP_OBJECT_REPR(marker)->attribute("inkscape:stockid"));
        else
            markname = g_strdup(SP_OBJECT_REPR(marker)->attribute("id"));

        int markpos = ink_marker_menu_get_pos(m, markname);
        mnu->set_history(markpos);

        g_free (markname);
    }
    else {
        mnu->set_history(0);
    }
    mnu->set_data("update", GINT_TO_POINTER(FALSE));
}

/**
 * Updates the marker menus to highlight the appropriate marker and scroll to
 * that marker.
 */
static void
sp_stroke_style_update_marker_menus(Gtk::Container *spw, GSList const *objects)
{
    struct { char const *key; int loc; } const keyloc[] = {
        { "start_mark_menu", SP_MARKER_LOC_START },
        { "mid_mark_menu", SP_MARKER_LOC_MID },
        { "end_mark_menu", SP_MARKER_LOC_END }
    };

    bool all_texts = true;
    for (GSList *i = (GSList *) objects; i != NULL; i = i->next) {
        if (!SP_IS_TEXT (i->data)) {
            all_texts = false;
        }
    }

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        Gtk::OptionMenu *mnu = static_cast<Gtk::OptionMenu *>(spw->get_data(keyloc[i].key));
        // Per SVG spec, text objects cannot have markers; disable menus if only texts are selected
        mnu->set_sensitive(!all_texts);
    }

    // We show markers of the first object in the list only
    // FIXME: use the first in the list that has the marker of each type, if any
    SPObject *object = SP_OBJECT(objects->data);

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        // For all three marker types,

        // find the corresponding menu
        Gtk::OptionMenu *mnu = static_cast<Gtk::OptionMenu *>(spw->get_data(keyloc[i].key));

        // Quit if we're in update state
        if (mnu->get_data("update")) {
            return;
        }

        if (object->style->marker[keyloc[i].loc].value != NULL && !all_texts) {
            // If the object has this type of markers,

            // Extract the name of the marker that the object uses
            SPObject *marker = ink_extract_marker_name(object->style->marker[keyloc[i].loc].value, SP_OBJECT_DOCUMENT(object));
            // Scroll the menu to that marker
            ink_marker_menu_set_current(marker, mnu);

        } else {
            mnu->set_history(0);
        }
    }
}


/**
 * Extract the actual name of the link
 * e.g. get mTriangle from url(#mTriangle).
 * \return Buffer containing the actual name, allocated from GLib;
 * the caller should free the buffer when they no longer need it.
 */
static SPObject*
ink_extract_marker_name(gchar const *n, SPDocument *doc)
{
    gchar const *p = n;
    while (*p != '\0' && *p != '#') {
        p++;
    }

    if (*p == '\0' || p[1] == '\0') {
        return NULL;
    }

    p++;
    int c = 0;
    while (p[c] != '\0' && p[c] != ')') {
        c++;
    }

    if (p[c] == '\0') {
        return NULL;
    }

    gchar* b = g_strdup(p);
    b[c] = '\0';

    // FIXME: get the document from the object and let the caller pass it in
    SPObject *marker = doc->getObjectById(b);
    return marker;
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
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

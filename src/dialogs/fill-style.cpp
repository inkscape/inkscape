#define __SP_FILL_STYLE_C__

/**
 * \brief  Fill style widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_FS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "widgets/sp-widget.h"
#include "sp-linear-gradient.h"
#include "sp-pattern.h"
#include "sp-radial-gradient.h"
#include "widgets/paint-selector.h"
#include "style.h"
#include "gradient-chemistry.h"
#include "desktop-style.h"
#include "desktop-handles.h"
#include "selection.h"
#include "inkscape.h"
#include "document-private.h"
#include "xml/repr.h"
#include <glibmm/i18n.h>
#include "display/sp-canvas.h"


// These can be deleted once we sort out the libart dependence.

#define ART_WIND_RULE_NONZERO 0

static void sp_fill_style_widget_construct          ( SPWidget *spw,
                                                      SPPaintSelector *psel );

static void sp_fill_style_widget_modify_selection   ( SPWidget *spw,
                                                      Inkscape::Selection *selection,
                                                      guint flags,
                                                      SPPaintSelector *psel );

static void sp_fill_style_widget_change_subselection ( Inkscape::Application *inkscape, SPDesktop *desktop, SPWidget *spw );

static void sp_fill_style_widget_change_selection   ( SPWidget *spw,
                                                      Inkscape::Selection *selection,
                                                      SPPaintSelector *psel );

static void sp_fill_style_widget_update (SPWidget *spw);

static void sp_fill_style_widget_paint_mode_changed ( SPPaintSelector *psel,
                                                      SPPaintSelectorMode mode,
                                                      SPWidget *spw );
static void sp_fill_style_widget_fillrule_changed ( SPPaintSelector *psel,
                                          SPPaintSelectorFillRule mode,
                                                    SPWidget *spw );

static void sp_fill_style_widget_paint_dragged (SPPaintSelector *psel, SPWidget *spw );
static void sp_fill_style_widget_paint_changed (SPPaintSelector *psel, SPWidget *spw );

GtkWidget *
sp_fill_style_widget_new (void)
{
    GtkWidget *spw = sp_widget_new_global (INKSCAPE);

    GtkWidget *vb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vb);
    gtk_container_add (GTK_CONTAINER (spw), vb);

    GtkWidget *psel = sp_paint_selector_new (true); // with fillrule selector
    gtk_widget_show (psel);
    gtk_box_pack_start (GTK_BOX (vb), psel, TRUE, TRUE, 0);
    g_object_set_data (G_OBJECT (spw), "paint-selector", psel);

    g_signal_connect ( G_OBJECT (psel), "mode_changed",
                       G_CALLBACK (sp_fill_style_widget_paint_mode_changed),
                       spw );

    g_signal_connect ( G_OBJECT (psel), "dragged",
                       G_CALLBACK (sp_fill_style_widget_paint_dragged),
                       spw );

    g_signal_connect ( G_OBJECT (psel), "changed",
                       G_CALLBACK (sp_fill_style_widget_paint_changed),
                       spw );

    g_signal_connect ( G_OBJECT (psel), "fillrule_changed",
                       G_CALLBACK (sp_fill_style_widget_fillrule_changed),
                       spw );


    g_signal_connect ( G_OBJECT (spw), "construct",
                       G_CALLBACK (sp_fill_style_widget_construct), psel);

//FIXME: switch these from spw signals to global inkscape object signals; spw just retranslates
//those anyway; then eliminate spw
    g_signal_connect ( G_OBJECT (spw), "modify_selection",
                       G_CALLBACK (sp_fill_style_widget_modify_selection), psel);

    g_signal_connect ( G_OBJECT (spw), "change_selection",
                       G_CALLBACK (sp_fill_style_widget_change_selection), psel);

    g_signal_connect (INKSCAPE, "change_subselection", G_CALLBACK (sp_fill_style_widget_change_subselection), spw);

    sp_fill_style_widget_update (SP_WIDGET (spw));

    return spw;

} // end of sp_fill_style_widget_new()



static void
sp_fill_style_widget_construct( SPWidget *spw, SPPaintSelector */*psel*/ )
{
#ifdef SP_FS_VERBOSE
    g_print ( "Fill style widget constructed: inkscape %p repr %p\n",
              spw->inkscape, spw->repr );
#endif
    if (spw->inkscape) {
        sp_fill_style_widget_update (spw);
    }

} // end of sp_fill_style_widget_construct()

static void
sp_fill_style_widget_modify_selection( SPWidget *spw,
                                       Inkscape::Selection */*selection*/,
                                       guint flags,
                                       SPPaintSelector */*psel*/ )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                  SP_OBJECT_PARENT_MODIFIED_FLAG |
                  SP_OBJECT_STYLE_MODIFIED_FLAG) )
    {
        sp_fill_style_widget_update (spw);
    }
}

static void
sp_fill_style_widget_change_subselection( Inkscape::Application */*inkscape*/,
                                          SPDesktop */*desktop*/,
                                          SPWidget *spw )
{
    sp_fill_style_widget_update (spw);
}

static void
sp_fill_style_widget_change_selection( SPWidget *spw,
                                       Inkscape::Selection */*selection*/,
                                       SPPaintSelector */*psel*/ )
{
    sp_fill_style_widget_update (spw);
}

/**
* \param sel Selection to use, or NULL.
*/
static void
sp_fill_style_widget_update (SPWidget *spw)
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    if (g_object_get_data (G_OBJECT (spw), "local")) {
        g_object_set_data (G_OBJECT (spw), "local", GINT_TO_POINTER (FALSE)); // local change; do nothing, but reset the flag
        return;
    }

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    SPPaintSelector *psel = SP_PAINT_SELECTOR (g_object_get_data (G_OBJECT (spw), "paint-selector"));

    // create temporary style
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FILL); 

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
            SPPaintSelectorMode pselmode = sp_style_determine_paint_selector_mode (query, true);
            sp_paint_selector_set_mode (psel, pselmode);

            sp_paint_selector_set_fillrule (psel, query->fill_rule.computed == ART_WIND_RULE_NONZERO? 
                                     SP_PAINT_SELECTOR_FILLRULE_NONZERO : SP_PAINT_SELECTOR_FILLRULE_EVENODD);

            if (query->fill.set && query->fill.isColor()) {
                sp_paint_selector_set_color_alpha (psel, &query->fill.value.color, SP_SCALE24_TO_FLOAT (query->fill_opacity.value));
            } else if (query->fill.set && query->fill.isPaintserver()) {

                SPPaintServer *server = SP_STYLE_FILL_SERVER (query);

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

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));

}


static void
sp_fill_style_widget_paint_mode_changed ( SPPaintSelector *psel,
                                          SPPaintSelectorMode /*mode*/,
                                          SPWidget *spw )
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    /* TODO: Does this work? */
    /* TODO: Not really, here we have to get old color back from object */
    /* Instead of relying on paint widget having meaningful colors set */
    sp_fill_style_widget_paint_changed (psel, spw);
}

static void
sp_fill_style_widget_fillrule_changed ( SPPaintSelector */*psel*/,
                                          SPPaintSelectorFillRule mode,
                                          SPWidget *spw )
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "fill-rule", mode == SP_PAINT_SELECTOR_FILLRULE_EVENODD? "evenodd":"nonzero");

    sp_desktop_set_style (desktop, css);

    sp_repr_css_attr_unref (css);

    sp_document_done (SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_FILL_STROKE, 
                      _("Change fill rule"));
}

static gchar const *undo_label_1 = "fill:flatcolor:1";
static gchar const *undo_label_2 = "fill:flatcolor:2";
static gchar const *undo_label = undo_label_1;

/**
This is called repeatedly while you are dragging a color slider, only for flat color
modes. Previously it set the color in style but did not update the repr for efficiency, however
this was flakey and didn't buy us almost anything. So now it does the same as _changed, except
lumps all its changes for undo.
 */
static void
sp_fill_style_widget_paint_dragged (SPPaintSelector *psel, SPWidget *spw)
{
    if (!spw->inkscape) {
        return;
    }

    if (g_object_get_data (G_OBJECT (spw), "update")) {
        return;
    }

    if (g_object_get_data (G_OBJECT (spw), "local")) {
        // previous local flag not cleared yet; 
        // this means dragged events come too fast, so we better skip this one to speed up display 
        // (it's safe to do this in any case)
        return;
    }

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    switch (psel->mode) {

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            sp_paint_selector_set_flat_color (psel, SP_ACTIVE_DESKTOP, "fill", "fill-opacity");
            sp_document_maybe_done (sp_desktop_document(SP_ACTIVE_DESKTOP), undo_label, SP_VERB_DIALOG_FILL_STROKE, 
                                    _("Set fill color"));
            g_object_set_data (G_OBJECT (spw), "local", GINT_TO_POINTER (TRUE)); // local change, do not update from selection
            break;
        }

        default:
            g_warning ( "file %s: line %d: Paint %d should not emit 'dragged'",
                        __FILE__, __LINE__, psel->mode );
            break;

    }
    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
}


/**
This is called (at least) when:
1  paint selector mode is switched (e.g. flat color -> gradient)
2  you finished dragging a gradient node and released mouse
3  you changed a gradient selector parameter (e.g. spread)
Must update repr.
 */
static void
sp_fill_style_widget_paint_changed ( SPPaintSelector *psel,
                                     SPWidget *spw )
{
    if (g_object_get_data (G_OBJECT (spw), "update")) {
        return;
    }
    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }
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
            SPCSSAttr *css = sp_repr_css_attr_new ();
            sp_repr_css_set_property (css, "fill", "none");

            sp_desktop_set_style (desktop, css);

            sp_repr_css_attr_unref (css);

            sp_document_done (document, SP_VERB_DIALOG_FILL_STROKE, 
                              _("Remove fill"));
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in losing release events
            sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);

            sp_paint_selector_set_flat_color (psel, desktop, "fill", "fill-opacity");
            sp_document_maybe_done (sp_desktop_document(desktop), undo_label, SP_VERB_DIALOG_FILL_STROKE,
                                    _("Set fill color"));
            // resume interruptibility
            sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));

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

                // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                SPCSSAttr *css = sp_repr_css_attr_new();
                sp_repr_css_set_property(css, "fill-opacity", "1.0");

                SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);
                if (!vector) {
                    /* No vector in paint selector should mean that we just changed mode */

                    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
                    int result = objects_query_fillstroke ((GSList *) items, query, true);
                    guint32 common_rgb = 0;
                    if (result == QUERY_STYLE_MULTIPLE_SAME) {
                        if (!query->fill.isColor()) {
                            common_rgb = sp_desktop_get_color(desktop, true);
                        } else {
                            common_rgb = query->fill.value.color.toRGBA32( 0xff );
                        }
                        vector = sp_document_default_gradient_vector(document, common_rgb);
                    }
                    sp_style_unref(query);

                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        sp_repr_css_change_recursive(SP_OBJECT_REPR(i->data), css, "style");

                        if (!vector) {
                            sp_item_set_gradient(SP_ITEM(i->data),
                                                 sp_gradient_vector_for_object(document, desktop, SP_OBJECT(i->data), true),
                                                 gradient_type, true);
                        } else {
                            sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, true);
                        }
                    }
                } else {
                    /* We have changed from another gradient type, or modified spread/units within
                     * this gradient type. */
                    vector = sp_gradient_ensure_vector_normalized (vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        sp_repr_css_change_recursive (SP_OBJECT_REPR (i->data), css, "style");

                        SPGradient *gr = sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, true);
                        sp_gradient_selector_attrs_to_gradient (gr, psel);
                    }
                }

                sp_repr_css_attr_unref (css);

                sp_document_done (document, SP_VERB_DIALOG_FILL_STROKE, 
                                  _("Set gradient on fill"));
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
                    sp_repr_css_set_property (css, "fill", urltext);

                    // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                    sp_repr_css_set_property(css, "fill-opacity", "1.0");

                    // cannot just call sp_desktop_set_style, because we don't want to touch those
                    // objects who already have the same root pattern but through a different href
                    // chain. FIXME: move this to a sp_item_set_pattern
                    for (GSList const *i = items; i != NULL; i = i->next) {
                         SPObject *selobj = SP_OBJECT (i->data);

                         SPStyle *style = SP_OBJECT_STYLE (selobj);
                         if (style && style->fill.isPaintserver()) {
                             SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (selobj);
                             if (SP_IS_PATTERN (server) && pattern_getroot (SP_PATTERN(server)) == pattern)
                                // only if this object's pattern is not rooted in our selected pattern, apply
                                 continue;
                         }

                         sp_desktop_apply_css_recursive (selobj, css, true);
                     }

                    sp_repr_css_attr_unref (css);
                    g_free (urltext);

                } // end if

                sp_document_done (document, SP_VERB_DIALOG_FILL_STROKE, 
                                  _("Set pattern on fill"));

            } // end if

            break;

        case SP_PAINT_SELECTOR_MODE_UNSET:
            if (items) {
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    sp_repr_css_unset_property (css, "fill");

                    sp_desktop_set_style (desktop, css);
                    sp_repr_css_attr_unref (css);

                    sp_document_done (document, SP_VERB_DIALOG_FILL_STROKE, 
                                      _("Unset fill"));
            }
            break;

        default:
            g_warning ( "file %s: line %d: Paint selector should not be in "
                        "mode %d",
                        __FILE__, __LINE__, psel->mode );
            break;
    }

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
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

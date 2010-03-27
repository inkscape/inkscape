/** @file
 * @brief  Fill style widget
 */
/* Authors:
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

#include <glibmm/i18n.h>
#include <gtk/gtkvbox.h>

#include "desktop-handles.h"
#include "desktop-style.h"
#include "display/sp-canvas.h"
#include "document-private.h"
#include "gradient-chemistry.h"
#include "inkscape.h"
#include "selection.h"
#include "sp-linear-gradient.h"
#include "sp-pattern.h"
#include "sp-radial-gradient.h"
#include "style.h"
#include "widgets/paint-selector.h"
#include "xml/repr.h"

#include "fill-style.h"
#include "fill-n-stroke-factory.h"


// These can be deleted once we sort out the libart dependence.

#define ART_WIND_RULE_NONZERO 0

/* Fill */

static void fillnstroke_fillrule_changed(SPPaintSelector *psel, SPPaintSelector::FillRule mode, GtkWidget *spw);

static void fillnstroke_selection_modified(Inkscape::Application *inkscape, Inkscape::Selection *selection, guint flags, GtkWidget *spw);
static void fillnstroke_selection_changed(Inkscape::Application *inkscape, Inkscape::Selection *selection, GtkWidget *spw);
static void fillnstroke_subselection_changed(Inkscape::Application *inkscape, SPDesktop *desktop, GtkWidget *spw);

static void fillnstroke_paint_mode_changed(SPPaintSelector *psel, SPPaintSelector::Mode mode, GtkWidget *spw);
static void fillnstroke_paint_dragged(SPPaintSelector *psel, GtkWidget *spw);
static void fillnstroke_paint_changed(SPPaintSelector *psel, GtkWidget *spw);

static void fillnstroke_performUpdate(GtkWidget *spw);

GtkWidget *sp_fill_style_widget_new(void)
{
    return Inkscape::Widgets::createStyleWidget( FILL );
}

/**
 * Create the fill or stroke style widget, and hook up all the signals.
 */
GtkWidget *Inkscape::Widgets::createStyleWidget( FillOrStroke kind )
{
    Inkscape::Application *appInstance = INKSCAPE;
    GtkWidget *spw = gtk_vbox_new(FALSE, 0);

    // with or without fillrule selector
    GtkWidget *psel = sp_paint_selector_new(kind == FILL);
    gtk_widget_show(psel);
    gtk_container_add(GTK_CONTAINER(spw), psel);
    g_object_set_data(G_OBJECT(spw), "paint-selector", psel);
    g_object_set_data(G_OBJECT(spw), "kind", GINT_TO_POINTER(kind));

    g_signal_connect( G_OBJECT(appInstance), "modify_selection",
                      G_CALLBACK(fillnstroke_selection_modified),
                      spw );

    g_signal_connect( G_OBJECT(appInstance), "change_selection",
                      G_CALLBACK(fillnstroke_selection_changed),
                      spw );

    g_signal_connect( G_OBJECT(appInstance), "change_subselection",
                      G_CALLBACK(fillnstroke_subselection_changed),
                      spw );

    g_signal_connect( G_OBJECT(psel), "mode_changed",
                      G_CALLBACK(fillnstroke_paint_mode_changed),
                      spw );

    g_signal_connect( G_OBJECT(psel), "dragged",
                      G_CALLBACK(fillnstroke_paint_dragged),
                      spw );

    g_signal_connect( G_OBJECT(psel), "changed",
                      G_CALLBACK(fillnstroke_paint_changed),
                      spw );

    if (kind == FILL) {
        g_signal_connect( G_OBJECT(psel), "fillrule_changed",
                          G_CALLBACK(fillnstroke_fillrule_changed),
                          spw );
    }

    fillnstroke_performUpdate(spw);

    return spw;
}

/**
 * On signal modified, invokes an update of the fill or stroke style paint object.
 */
void fillnstroke_selection_modified( Inkscape::Application * /*inkscape*/,
                                     Inkscape::Selection * /*selection*/,
                                     guint flags,
                                     GtkWidget *spw )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                  SP_OBJECT_PARENT_MODIFIED_FLAG |
                  SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
#ifdef SP_FS_VERBOSE
        g_message("fillnstroke_selection_modified()");
#endif
        fillnstroke_performUpdate(spw);
    }
}

/**
 * On signal selection changed, invokes an update of the fill or stroke style paint object.
 */
void fillnstroke_selection_changed( Inkscape::Application * /*inkscape*/,
                                    Inkscape::Selection */*selection*/,
                                    GtkWidget *spw )
{
    fillnstroke_performUpdate(spw);
}

/**
 * On signal change subselection, invoke an update of the fill or stroke style widget.
 */
void fillnstroke_subselection_changed( Inkscape::Application * /*inkscape*/,
                                       SPDesktop * /*desktop*/,
                                       GtkWidget *spw )
{
    fillnstroke_performUpdate(spw);
}

/**
 * Gets the active fill or stroke style property, then sets the appropriate
 * color, alpha, gradient, pattern, etc. for the paint-selector.
 *
 * @param sel Selection to use, or NULL.
 */
void fillnstroke_performUpdate( GtkWidget *spw )
{
    if ( g_object_get_data(G_OBJECT(spw), "update") ) {
        return;
    }

    FillOrStroke kind = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(spw), "kind")) ? FILL : STROKE;

    if (kind == FILL) {
        if ( g_object_get_data(G_OBJECT(spw), "local") ) {
            g_object_set_data(G_OBJECT(spw), "local", GINT_TO_POINTER(FALSE)); // local change; do nothing, but reset the flag
            return;
        }
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    SPPaintSelector *psel = SP_PAINT_SELECTOR(g_object_get_data(G_OBJECT(spw), "paint-selector"));

    // create temporary style
    SPStyle *query = sp_style_new(SP_ACTIVE_DOCUMENT);

    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style(SP_ACTIVE_DESKTOP, query, (kind == FILL) ? QUERY_STYLE_PROPERTY_FILL : QUERY_STYLE_PROPERTY_STROKE);

    SPIPaint &targPaint = (kind == FILL) ? query->fill : query->stroke;
    SPIScale24 &targOpacity = (kind == FILL) ? query->fill_opacity : query->stroke_opacity;

    switch (result) {
        case QUERY_STYLE_NOTHING:
        {
            /* No paint at all */
            psel->setMode(SPPaintSelector::MODE_EMPTY);
            break;
        }

        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently, e.g. display "averaged" somewhere in paint selector
        case QUERY_STYLE_MULTIPLE_SAME:
        {
            SPPaintSelector::Mode pselmode = SPPaintSelector::getModeForStyle(*query, kind == FILL);
            psel->setMode(pselmode);

            if (kind == FILL) {
                psel->setFillrule(query->fill_rule.computed == ART_WIND_RULE_NONZERO?
                                  SPPaintSelector::FILLRULE_NONZERO : SPPaintSelector::FILLRULE_EVENODD);
            }

            if (targPaint.set && targPaint.isColor()) {
                psel->setColorAlpha(targPaint.value.color, SP_SCALE24_TO_FLOAT(targOpacity.value));
            } else if (targPaint.set && targPaint.isPaintserver()) {

                SPPaintServer *server = (kind == FILL) ? query->getFillPaintServer() : query->getStrokePaintServer();

                if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    psel->setSwatch( vector );
                } else if (SP_IS_LINEARGRADIENT(server)) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    psel->setGradientLinear( vector );

                    SPLinearGradient *lg = SP_LINEARGRADIENT(server);
                    psel->setGradientProperties( SP_GRADIENT_UNITS(lg),
                                                 SP_GRADIENT_SPREAD(lg) );
                } else if (SP_IS_RADIALGRADIENT(server)) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    psel->setGradientRadial( vector );

                    SPRadialGradient *rg = SP_RADIALGRADIENT(server);
                    psel->setGradientProperties( SP_GRADIENT_UNITS(rg),
                                                 SP_GRADIENT_SPREAD(rg) );
                } else if (SP_IS_PATTERN(server)) {
                    SPPattern *pat = pattern_getroot(SP_PATTERN(server));
                    psel->updatePatternList( pat );
                }
            }
            break;
        }

        case QUERY_STYLE_MULTIPLE_DIFFERENT:
        {
            psel->setMode(SPPaintSelector::MODE_MULTIPLE);
            break;
        }
    }

    sp_style_unref(query);

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}

/**
 * When the mode is changed, invoke a regular changed handler.
 */
void fillnstroke_paint_mode_changed( SPPaintSelector *psel,
                                     SPPaintSelector::Mode /*mode*/,
                                     GtkWidget *spw )
{
    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }

#ifdef SP_FS_VERBOSE
    g_message("fillnstroke_paint_mode_changed(psel:%p, mode, spw:%p)", psel, spw);
#endif

    /* TODO: Does this work?
     * Not really, here we have to get old color back from object
     * Instead of relying on paint widget having meaningful colors set
     */
    fillnstroke_paint_changed(psel, spw);
}

void fillnstroke_fillrule_changed( SPPaintSelector * /*psel*/,
                                   SPPaintSelector::FillRule mode,
                                   GtkWidget *spw )
{
    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_set_property(css, "fill-rule", mode == SPPaintSelector::FILLRULE_EVENODD? "evenodd":"nonzero");

    sp_desktop_set_style(desktop, css);

    sp_repr_css_attr_unref(css);
    css = 0;

    sp_document_done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_FILL_STROKE,
                     _("Change fill rule"));
}

static gchar const *undo_F_label_1 = "fill:flatcolor:1";
static gchar const *undo_F_label_2 = "fill:flatcolor:2";

static gchar const *undo_S_label_1 = "stroke:flatcolor:1";
static gchar const *undo_S_label_2 = "stroke:flatcolor:2";

static gchar const *undo_F_label = undo_F_label_1;
static gchar const *undo_S_label = undo_S_label_1;

/**
 * This is called repeatedly while you are dragging a color slider, only for flat color
 * modes. Previously it set the color in style but did not update the repr for efficiency, however
 * this was flakey and didn't buy us almost anything. So now it does the same as _changed, except
 * lumps all its changes for undo.
 */
void fillnstroke_paint_dragged(SPPaintSelector *psel, GtkWidget *spw)
{
    if (!INKSCAPE) {
        return;
    }

    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }

    FillOrStroke kind = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(spw), "kind")) ? FILL : STROKE;

    if (kind == FILL) {
        if (g_object_get_data(G_OBJECT(spw), "local")) {
            // previous local flag not cleared yet;
            // this means dragged events come too fast, so we better skip this one to speed up display
            // (it's safe to do this in any case)
            return;
        }
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    switch (psel->mode) {
        case SPPaintSelector::MODE_COLOR_RGB:
        case SPPaintSelector::MODE_COLOR_CMYK:
        {
            psel->setFlatColor( SP_ACTIVE_DESKTOP, (kind == FILL) ? "fill" : "stroke", (kind == FILL) ? "fill-opacity" : "stroke-opacity" );
            sp_document_maybe_done(sp_desktop_document(SP_ACTIVE_DESKTOP), (kind == FILL) ? undo_F_label : undo_S_label, SP_VERB_DIALOG_FILL_STROKE,
                                   (kind == FILL) ? _("Set fill color") : _("Set stroke color"));
            if (kind == FILL) {
                g_object_set_data(G_OBJECT(spw), "local", GINT_TO_POINTER(TRUE)); // local change, do not update from selection
            }
            break;
        }

        default:
            g_warning( "file %s: line %d: Paint %d should not emit 'dragged'",
                       __FILE__, __LINE__, psel->mode );
            break;
    }
    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}

/**
This is called (at least) when:
1  paint selector mode is switched (e.g. flat color -> gradient)
2  you finished dragging a gradient node and released mouse
3  you changed a gradient selector parameter (e.g. spread)
Must update repr.
 */
void fillnstroke_paint_changed( SPPaintSelector *psel, GtkWidget *spw )
{
#ifdef SP_FS_VERBOSE
    g_message("fillnstroke_paint_changed(psel:%p, spw:%p)", psel, spw);
#endif
    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }
    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    FillOrStroke kind = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(spw), "kind")) ? FILL : STROKE;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }
    SPDocument *document = sp_desktop_document(desktop);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    GSList const *items = selection->itemList();

    switch (psel->mode) {
        case SPPaintSelector::MODE_EMPTY:
            // This should not happen.
            g_warning( "file %s: line %d: Paint %d should not emit 'changed'",
                       __FILE__, __LINE__, psel->mode);
            break;
        case SPPaintSelector::MODE_MULTIPLE:
            // This happens when you switch multiple objects with different gradients to flat color;
            // nothing to do here.
            break;

        case SPPaintSelector::MODE_NONE:
        {
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, (kind == FILL) ? "fill" : "stroke", "none");

            sp_desktop_set_style(desktop, css);

            sp_repr_css_attr_unref(css);
            css = 0;

            sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                             (kind == FILL) ? _("Remove fill") : _("Remove stroke"));
            break;
        }

        case SPPaintSelector::MODE_COLOR_RGB:
        case SPPaintSelector::MODE_COLOR_CMYK:
        {
            if (kind == FILL) {
                // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in losing release events
                sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);
            }

            psel->setFlatColor( desktop,
                                (kind == FILL) ? "fill" : "stroke",
                                (kind == FILL) ? "fill-opacity" : "stroke-opacity" );
            sp_document_maybe_done(sp_desktop_document(desktop), (kind == FILL) ? undo_F_label : undo_S_label, SP_VERB_DIALOG_FILL_STROKE,
                                   (kind == FILL) ? _("Set fill color") : _("Set stroke color"));

            if (kind == FILL) {
                // resume interruptibility
                sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));
            }

            // on release, toggle undo_label so that the next drag will not be lumped with this one
            if (undo_F_label == undo_F_label_1) {
                undo_F_label = undo_F_label_2;
                undo_S_label = undo_S_label_2;
            } else {
                undo_F_label = undo_F_label_1;
                undo_S_label = undo_S_label_1;
            }

            break;
        }

        case SPPaintSelector::MODE_GRADIENT_LINEAR:
        case SPPaintSelector::MODE_GRADIENT_RADIAL:
        case SPPaintSelector::MODE_SWATCH:
            if (items) {
                SPGradientType const gradient_type = ( psel->mode != SPPaintSelector::MODE_GRADIENT_RADIAL
                                                       ? SP_GRADIENT_TYPE_LINEAR
                                                       : SP_GRADIENT_TYPE_RADIAL );

                SPCSSAttr *css = 0;
                if (kind == FILL) {
                    // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                    css = sp_repr_css_attr_new();
                    sp_repr_css_set_property(css, "fill-opacity", "1.0");
                }

                SPGradient *vector = psel->getGradientVector();
                if (!vector) {
                    /* No vector in paint selector should mean that we just changed mode */

                    SPStyle *query = sp_style_new(SP_ACTIVE_DOCUMENT);
                    int result = objects_query_fillstroke(const_cast<GSList *>(items), query, kind == FILL);
                    SPIPaint &targPaint = (kind == FILL) ? query->fill : query->stroke;
                    guint32 common_rgb = 0;
                    if (result == QUERY_STYLE_MULTIPLE_SAME) {
                        if (!targPaint.isColor()) {
                            common_rgb = sp_desktop_get_color(desktop, kind == FILL);
                        } else {
                            common_rgb = targPaint.value.color.toRGBA32( 0xff );
                        }
                        vector = sp_document_default_gradient_vector(document, common_rgb);
                    }
                    sp_style_unref(query);

                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        if (kind == FILL) {
                            sp_repr_css_change_recursive(SP_OBJECT_REPR(i->data), css, "style");
                        }

                        if (!vector) {
                            sp_item_set_gradient(SP_ITEM(i->data),
                                                 sp_gradient_vector_for_object(document, desktop, SP_OBJECT(i->data), kind == FILL),
                                                 gradient_type, kind == FILL);
                        } else {
                            sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, kind == FILL);
                        }
                    }
                } else {
                    // We have changed from another gradient type, or modified spread/units within
                    // this gradient type.
                    vector = sp_gradient_ensure_vector_normalized(vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        if (kind == FILL) {
                            sp_repr_css_change_recursive(SP_OBJECT_REPR(i->data), css, "style");
                        }

                        SPGradient *gr = sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, kind == FILL);
                        psel->pushAttrsToGradient( gr );
                    }
                }

                if (css) {
                    sp_repr_css_attr_unref(css);
                    css = 0;
                }

                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 (kind == FILL) ? _("Set gradient on fill") : _("Set gradient on stroke"));
            }
            break;

        case SPPaintSelector::MODE_PATTERN:

            if (items) {

                SPPattern *pattern = psel->getPattern();
                if (!pattern) {

                    /* No Pattern in paint selector should mean that we just
                     * changed mode - dont do jack.
                     */

                } else {
                    Inkscape::XML::Node *patrepr = SP_OBJECT_REPR(pattern);
                    SPCSSAttr *css = sp_repr_css_attr_new();
                    gchar *urltext = g_strdup_printf("url(#%s)", patrepr->attribute("id"));
                    sp_repr_css_set_property(css, (kind == FILL) ? "fill" : "stroke", urltext);

                    // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                    if (kind == FILL) {
                        sp_repr_css_set_property(css, "fill-opacity", "1.0");
                    }

                    // cannot just call sp_desktop_set_style, because we don't want to touch those
                    // objects who already have the same root pattern but through a different href
                    // chain. FIXME: move this to a sp_item_set_pattern
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        Inkscape::XML::Node *selrepr = SP_OBJECT_REPR(i->data);
                        if ( (kind == STROKE) && !selrepr) {
                            continue;
                        }
                        SPObject *selobj = SP_OBJECT(i->data);

                        SPStyle *style = SP_OBJECT_STYLE(selobj);
                        if (style && ((kind == FILL) ? style->fill : style->stroke).isPaintserver()) {
                            SPObject *server = (kind == FILL) ?
                                SP_OBJECT_STYLE_FILL_SERVER(selobj) :
                                SP_OBJECT_STYLE_STROKE_SERVER(selobj);
                            if (SP_IS_PATTERN(server) && pattern_getroot(SP_PATTERN(server)) == pattern)
                                // only if this object's pattern is not rooted in our selected pattern, apply
                                continue;
                        }

                        if (kind == FILL) {
                            sp_desktop_apply_css_recursive(selobj, css, true);
                        } else {
                            sp_repr_css_change_recursive(selrepr, css, "style");
                        }
                    }

                    sp_repr_css_attr_unref(css);
                    css = 0;
                    g_free(urltext);

                } // end if

                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 (kind == FILL) ? _("Set pattern on fill") :
                                 _("Set pattern on stroke"));
            } // end if

            break;

        case SPPaintSelector::MODE_UNSET:
            if (items) {
                SPCSSAttr *css = sp_repr_css_attr_new();
                if (kind == FILL) {
                    sp_repr_css_unset_property(css, "fill");
                } else {
                    sp_repr_css_unset_property(css, "stroke");
                    sp_repr_css_unset_property(css, "stroke-opacity");
                    sp_repr_css_unset_property(css, "stroke-width");
                    sp_repr_css_unset_property(css, "stroke-miterlimit");
                    sp_repr_css_unset_property(css, "stroke-linejoin");
                    sp_repr_css_unset_property(css, "stroke-linecap");
                    sp_repr_css_unset_property(css, "stroke-dashoffset");
                    sp_repr_css_unset_property(css, "stroke-dasharray");
                }

                sp_desktop_set_style(desktop, css);
                sp_repr_css_attr_unref(css);
                css = 0;

                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 (kind == FILL) ? _("Unset fill") : _("Unset stroke"));
            }
            break;

        default:
            g_warning( "file %s: line %d: Paint selector should not be in "
                       "mode %d",
                       __FILE__, __LINE__,
                       psel->mode );
            break;
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
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

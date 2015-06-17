/**
 * @file
 * Fill style widget.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_FS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <glibmm/i18n.h>

#include "verbs.h"

#include <gtk/gtk.h>

#include "desktop.h"
#include "selection.h"

#include "desktop-style.h"
#include "display/sp-canvas.h"
#include "document-private.h"
#include "document-undo.h"
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


Gtk::Widget *sp_fill_style_widget_new(void)
{
    return Inkscape::Widgets::createStyleWidget( FILL );
}


namespace Inkscape {

class FillNStroke : public Gtk::VBox
{
public:
    FillNStroke( FillOrStroke kind );
    ~FillNStroke();

    void setFillrule( SPPaintSelector::FillRule mode );

    void setDesktop(SPDesktop *desktop);

private:
    static void paintModeChangeCB(SPPaintSelector *psel, SPPaintSelector::Mode mode, FillNStroke *self);
    static void paintChangedCB(SPPaintSelector *psel, FillNStroke *self);
    static void paintDraggedCB(SPPaintSelector *psel, FillNStroke *self);
    static gboolean dragDelayCB(gpointer data);

    static void fillruleChangedCB( SPPaintSelector *psel, SPPaintSelector::FillRule mode, FillNStroke *self );

    void selectionModifiedCB(guint flags);
    void eventContextCB(SPDesktop *desktop, Inkscape::UI::Tools::ToolBase *eventcontext);

    void dragFromPaint();
    void updateFromPaint();

    void performUpdate();

    FillOrStroke kind;
    SPDesktop *desktop;
    SPPaintSelector *psel;
    guint32 lastDrag;
    guint dragId;
    bool update;
    sigc::connection selectChangedConn;
    sigc::connection subselChangedConn;
    sigc::connection selectModifiedConn;
    sigc::connection eventContextConn;
};

} // namespace Inkscape

void sp_fill_style_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop)
{
    Inkscape::FillNStroke *fs = dynamic_cast<Inkscape::FillNStroke*>(widget);
    if (fs) {
        fs->setDesktop(desktop);
    }
}

namespace Inkscape {

/**
 * Create the fill or stroke style widget, and hook up all the signals.
 */
Gtk::Widget *Inkscape::Widgets::createStyleWidget( FillOrStroke kind )
{
    FillNStroke *filler = new FillNStroke(kind);

    return filler;
}

FillNStroke::FillNStroke( FillOrStroke kind ) :
    Gtk::VBox(),
    kind(kind),
    desktop(0),
    psel(0),
    lastDrag(0),
    dragId(0),
    update(false),
    selectChangedConn(),
    subselChangedConn(),
    selectModifiedConn(),
    eventContextConn()
{
    // Add and connect up the paint selector widget:
    psel = sp_paint_selector_new(kind);
    gtk_widget_show(GTK_WIDGET(psel));
    gtk_container_add(GTK_CONTAINER(gobj()), GTK_WIDGET(psel));
    g_signal_connect( G_OBJECT(psel), "mode_changed",
                      G_CALLBACK(paintModeChangeCB),
                      this );

    g_signal_connect( G_OBJECT(psel), "dragged",
                      G_CALLBACK(paintDraggedCB),
                      this );

    g_signal_connect( G_OBJECT(psel), "changed",
                      G_CALLBACK(paintChangedCB),
                      this );
    if (kind == FILL) {
        g_signal_connect( G_OBJECT(psel), "fillrule_changed",
                          G_CALLBACK(fillruleChangedCB),
                          this );
    }

    performUpdate();
}

FillNStroke::~FillNStroke()
{
    if (dragId) {
        g_source_remove(dragId);
        dragId = 0;
    }
    psel = 0;
    selectModifiedConn.disconnect();
    subselChangedConn.disconnect();
    selectChangedConn.disconnect();
    eventContextConn.disconnect();
}

/**
 * On signal modified, invokes an update of the fill or stroke style paint object.
 */
void FillNStroke::selectionModifiedCB( guint flags )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                   SP_OBJECT_PARENT_MODIFIED_FLAG |
                   SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
#ifdef SP_FS_VERBOSE
        g_message("selectionModifiedCB(%d) on %p", flags, this);
#endif
        performUpdate();
    }
}

void FillNStroke::setDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (dragId) {
            g_source_remove(dragId);
            dragId = 0;
        }
        if (this->desktop) {
            selectModifiedConn.disconnect();
            subselChangedConn.disconnect();
            selectChangedConn.disconnect();
            eventContextConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &FillNStroke::performUpdate)));
            subselChangedConn = desktop->connectToolSubselectionChanged(sigc::hide(sigc::mem_fun(*this, &FillNStroke::performUpdate)));
            eventContextConn = desktop->connectEventContextChanged(sigc::hide(sigc::bind(sigc::mem_fun(*this, &FillNStroke::eventContextCB), (Inkscape::UI::Tools::ToolBase *)NULL)));

            // Must check flags, so can't call performUpdate() directly.
            selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &FillNStroke::selectionModifiedCB)));
        }
        performUpdate();
    }
}

/**
 * Listen to this "change in tool" event, in case a subselection tool (such as Gradient or Node) selection
 * is changed back to a selection tool - especially needed for selected gradient stops.
 */
void FillNStroke::eventContextCB(SPDesktop * /*desktop*/, Inkscape::UI::Tools::ToolBase * /*eventcontext*/)
{
    performUpdate();
}


/**
 * Gets the active fill or stroke style property, then sets the appropriate
 * color, alpha, gradient, pattern, etc. for the paint-selector.
 *
 * @param sel Selection to use, or NULL.
 */
void FillNStroke::performUpdate()
{
    if ( update || !desktop ) {
        return;
    }

    if ( dragId ) {
        // local change; do nothing, but reset the flag
        g_source_remove(dragId);
        dragId = 0;
        return;
    }

    update = true;

    // create temporary style
    SPStyle query(desktop->doc());

    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style(desktop, &query, (kind == FILL) ? QUERY_STYLE_PROPERTY_FILL : QUERY_STYLE_PROPERTY_STROKE);

    SPIPaint &targPaint = (kind == FILL) ? query.fill : query.stroke;
    SPIScale24 &targOpacity = (kind == FILL) ? query.fill_opacity : query.stroke_opacity;

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
            SPPaintSelector::Mode pselmode = SPPaintSelector::getModeForStyle(query, kind);
            psel->setMode(pselmode);

            if (kind == FILL) {
                psel->setFillrule(query.fill_rule.computed == ART_WIND_RULE_NONZERO?
                                  SPPaintSelector::FILLRULE_NONZERO : SPPaintSelector::FILLRULE_EVENODD);
            }

            if (targPaint.set && targPaint.isColor()) {
                psel->setColorAlpha(targPaint.value.color, SP_SCALE24_TO_FLOAT(targOpacity.value));
            } else if (targPaint.set && targPaint.isPaintserver()) {

                SPPaintServer *server = (kind == FILL) ? query.getFillPaintServer() : query.getStrokePaintServer();

                if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    psel->setSwatch( vector );
                } else if (SP_IS_LINEARGRADIENT(server)) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    psel->setGradientLinear( vector );

                    SPLinearGradient *lg = SP_LINEARGRADIENT(server);
                    psel->setGradientProperties( lg->getUnits(),
                                                 lg->getSpread() );
                } else if (SP_IS_RADIALGRADIENT(server)) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    psel->setGradientRadial( vector );

                    SPRadialGradient *rg = SP_RADIALGRADIENT(server);
                    psel->setGradientProperties( rg->getUnits(),
                                                 rg->getSpread() );
                } else if (SP_IS_PATTERN(server)) {
                    SPPattern *pat = SP_PATTERN(server)->rootPattern();
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

    update = false;
}

/**
 * When the mode is changed, invoke a regular changed handler.
 */
void FillNStroke::paintModeChangeCB( SPPaintSelector * /*psel*/,
                                     SPPaintSelector::Mode /*mode*/,
                                     FillNStroke *self )
{
#ifdef SP_FS_VERBOSE
    g_message("paintModeChangeCB(psel, mode, self:%p)", self);
#endif
    if (self && !self->update) {
        self->updateFromPaint();
    }
}

void FillNStroke::fillruleChangedCB( SPPaintSelector * /*psel*/,
                                     SPPaintSelector::FillRule mode,
                                     FillNStroke *self )
{
    if (self) {
        self->setFillrule(mode);
    }
}

void FillNStroke::setFillrule( SPPaintSelector::FillRule mode )
{
    if (!update && desktop) {
        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property(css, "fill-rule", (mode == SPPaintSelector::FILLRULE_EVENODD) ? "evenodd":"nonzero");

        sp_desktop_set_style(desktop, css);

        sp_repr_css_attr_unref(css);
        css = 0;

        DocumentUndo::done(desktop->doc(), SP_VERB_DIALOG_FILL_STROKE,
                           _("Change fill rule"));
    }
}

static gchar const *undo_F_label_1 = "fill:flatcolor:1";
static gchar const *undo_F_label_2 = "fill:flatcolor:2";

static gchar const *undo_S_label_1 = "stroke:flatcolor:1";
static gchar const *undo_S_label_2 = "stroke:flatcolor:2";

static gchar const *undo_F_label = undo_F_label_1;
static gchar const *undo_S_label = undo_S_label_1;


void FillNStroke::paintDraggedCB(SPPaintSelector * /*psel*/, FillNStroke *self)
{
#ifdef SP_FS_VERBOSE
    g_message("paintDraggedCB(psel, spw:%p)", self);
#endif
    if (self && !self->update) {
        self->dragFromPaint();
    }
}


gboolean FillNStroke::dragDelayCB(gpointer data)
{
    gboolean keepGoing = TRUE;
    if (data) {
        FillNStroke *self = reinterpret_cast<FillNStroke*>(data);
        if (!self->update) {
            if (self->dragId) {
                g_source_remove(self->dragId);
                self->dragId = 0;

                self->dragFromPaint();
                self->performUpdate();
            }
            keepGoing = FALSE;
        }
    } else {
        keepGoing = FALSE;
    }
    return keepGoing;
}

/**
 * This is called repeatedly while you are dragging a color slider, only for flat color
 * modes. Previously it set the color in style but did not update the repr for efficiency, however
 * this was flakey and didn't buy us almost anything. So now it does the same as _changed, except
 * lumps all its changes for undo.
 */
void FillNStroke::dragFromPaint()
{
    if (!desktop || update) {
        return;
    }

    guint32 when = gtk_get_current_event_time();

    // Don't attempt too many updates per second.
    // Assume a base 15.625ms resolution on the timer.
    if (!dragId && lastDrag && when && ((when - lastDrag) < 32)) {
        // local change, do not update from selection
        dragId = g_timeout_add_full(G_PRIORITY_DEFAULT, 33, dragDelayCB, this, 0);
    }

    if (dragId) {
        // previous local flag not cleared yet;
        // this means dragged events come too fast, so we better skip this one to speed up display
        // (it's safe to do this in any case)
        return;
    }
    lastDrag = when;

    update = true;

    switch (psel->mode) {
        case SPPaintSelector::MODE_SOLID_COLOR:
        {
            // local change, do not update from selection
            dragId = g_timeout_add_full(G_PRIORITY_DEFAULT, 100, dragDelayCB, this, 0);
            psel->setFlatColor( desktop, (kind == FILL) ? "fill" : "stroke", (kind == FILL) ? "fill-opacity" : "stroke-opacity" );
            DocumentUndo::maybeDone(desktop->doc(), (kind == FILL) ? undo_F_label : undo_S_label, SP_VERB_DIALOG_FILL_STROKE,
                                    (kind == FILL) ? _("Set fill color") : _("Set stroke color"));
            break;
        }

        default:
            g_warning( "file %s: line %d: Paint %d should not emit 'dragged'",
                       __FILE__, __LINE__, psel->mode );
            break;
    }
    update = false;
}

/**
This is called (at least) when:
1  paint selector mode is switched (e.g. flat color -> gradient)
2  you finished dragging a gradient node and released mouse
3  you changed a gradient selector parameter (e.g. spread)
Must update repr.
 */
void FillNStroke::paintChangedCB( SPPaintSelector * /*psel*/, FillNStroke *self )
{
#ifdef SP_FS_VERBOSE
    g_message("paintChangedCB(psel, spw:%p)", self);
#endif
    if (self && !self->update) {
        self->updateFromPaint();
    }
}

void FillNStroke::updateFromPaint()
{
    if (!desktop) {
        return;
    }
    update = true;

    SPDocument *document = desktop->getDocument();
    Inkscape::Selection *selection = desktop->getSelection();

    std::vector<SPItem*> const items = selection->itemList();

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

            DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                               (kind == FILL) ? _("Remove fill") : _("Remove stroke"));
            break;
        }

        case SPPaintSelector::MODE_SOLID_COLOR:
        {
            if (kind == FILL) {
                // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in losing release events
                desktop->getCanvas()->forceFullRedrawAfterInterruptions(0);
            }

            psel->setFlatColor( desktop,
                                (kind == FILL) ? "fill" : "stroke",
                                (kind == FILL) ? "fill-opacity" : "stroke-opacity" );
            DocumentUndo::maybeDone(desktop->getDocument(), (kind == FILL) ? undo_F_label : undo_S_label, SP_VERB_DIALOG_FILL_STROKE,
                                    (kind == FILL) ? _("Set fill color") : _("Set stroke color"));

            if (kind == FILL) {
                // resume interruptibility
                desktop->getCanvas()->endForcedFullRedraws();
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
            if (!items.empty()) {
                SPGradientType const gradient_type = ( psel->mode != SPPaintSelector::MODE_GRADIENT_RADIAL
                                                       ? SP_GRADIENT_TYPE_LINEAR
                                                       : SP_GRADIENT_TYPE_RADIAL );
                bool createSwatch = (psel->mode == SPPaintSelector::MODE_SWATCH);

                SPCSSAttr *css = 0;
                if (kind == FILL) {
                    // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                    css = sp_repr_css_attr_new();
                    sp_repr_css_set_property(css, "fill-opacity", "1.0");
                }

                SPGradient *vector = psel->getGradientVector();
                if (!vector) {
                    /* No vector in paint selector should mean that we just changed mode */

                    SPStyle query(desktop->doc());
                    int result = objects_query_fillstroke(items, &query, kind == FILL);
                    if (result == QUERY_STYLE_MULTIPLE_SAME) {
                        SPIPaint &targPaint = (kind == FILL) ? query.fill : query.stroke;
                        SPColor common;
                        if (!targPaint.isColor()) {
                            common = sp_desktop_get_color(desktop, kind == FILL);
                        } else {
                            common = targPaint.value.color;
                        }
                        vector = sp_document_default_gradient_vector( document, common, createSwatch );
                        if ( vector && createSwatch ) {
                            vector->setSwatch();
                        }
                    }

                    for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
                        //FIXME: see above
                        if (kind == FILL) {
                            sp_repr_css_change_recursive((*i)->getRepr(), css, "style");
                        }

                        if (!vector) {
                            SPGradient *gr = sp_gradient_vector_for_object( document,
                                                                            desktop,
                                                                            reinterpret_cast<SPObject*>(*i),
                                                                            (kind == FILL) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE,
                                                                            createSwatch );
                            if ( gr && createSwatch ) {
                                gr->setSwatch();
                            }
                            sp_item_set_gradient(*i,
                                                 gr,
                                                 gradient_type, (kind == FILL) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE);
                        } else {
                            sp_item_set_gradient(*i, vector, gradient_type, (kind == FILL) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE);
                        }
                    }
                } else {
                    // We have changed from another gradient type, or modified spread/units within
                    // this gradient type.
                    vector = sp_gradient_ensure_vector_normalized(vector);
                    for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
                        //FIXME: see above
                        if (kind == FILL) {
                            sp_repr_css_change_recursive((*i)->getRepr(), css, "style");
                        }

                        SPGradient *gr = sp_item_set_gradient(*i, vector, gradient_type, (kind == FILL) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE);
                        psel->pushAttrsToGradient( gr );
                    }
                }

                if (css) {
                    sp_repr_css_attr_unref(css);
                    css = 0;
                }

                DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                                   (kind == FILL) ? _("Set gradient on fill") : _("Set gradient on stroke"));
            }
            break;

        case SPPaintSelector::MODE_PATTERN:

            if (!items.empty()) {

                SPPattern *pattern = psel->getPattern();
                if (!pattern) {

                    /* No Pattern in paint selector should mean that we just
                     * changed mode - dont do jack.
                     */

                } else {
                    Inkscape::XML::Node *patrepr = pattern->getRepr();
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
                    for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
                        Inkscape::XML::Node *selrepr = (*i)->getRepr();
                        if ( (kind == STROKE) && !selrepr) {
                            continue;
                        }
                        SPObject *selobj = *i;

                        SPStyle *style = selobj->style;
                        if (style && ((kind == FILL) ? style->fill : style->stroke).isPaintserver()) {
                            SPPaintServer *server = (kind == FILL) ?
                                selobj->style->getFillPaintServer() :
                                selobj->style->getStrokePaintServer();
                            if (SP_IS_PATTERN(server) && SP_PATTERN(server)->rootPattern() == pattern)
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

                DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                                   (kind == FILL) ? _("Set pattern on fill") :
                                   _("Set pattern on stroke"));
            } // end if

            break;

        case SPPaintSelector::MODE_UNSET:
            if (!items.empty()) {
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

                DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
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

    update = false;
}

} // namespace Inkscape

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

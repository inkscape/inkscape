/**
 * @file
 * SPPaintSelector: Generic paint selector widget.
 */

/*
 * Authors:
 *   Lauris Kaplinski
 *   bulia byak <buliabyak@users.sf.net>
 *   John Cliff <simarilius@yahoo.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) Lauris Kaplinski 2002
 * Copyright (C) 2010 Authors
*/

#define noSP_PS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include "widgets/swatch-selector.h"
#include "sp-pattern.h"
#include <glibmm/i18n.h>
#include "widgets/icon.h"
#include "widgets/widget-sizes.h"
#include "xml/repr.h"

#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-mesh.h"
#include "sp-stop.h"
/* fixme: Move it from dialogs to here */
#include "gradient-selector.h"
#include <inkscape.h>
#include <document-private.h>
#include <desktop-style.h>
#include <style.h>
#include "svg/svg-color.h"
#include "svg/css-ostringstream.h"
#include "path-prefix.h"
#include "io/sys.h"
#include "helper/stock-items.h"
#include "ui/icon-names.h"
#include "ui/widget/color-notebook.h"

#include "paint-selector.h"

#ifdef SP_PS_VERBOSE
#include "svg/svg-icc-color.h"
#endif // SP_PS_VERBOSE

#include <gtk/gtk.h>

using Inkscape::Widgets::SwatchSelector;
using Inkscape::UI::SelectedColor;

enum {
    MODE_CHANGED,
    GRABBED,
    DRAGGED,
    RELEASED,
    CHANGED,
    FILLRULE_CHANGED,
    LAST_SIGNAL
};

static void sp_paint_selector_dispose(GObject *object);

static GtkWidget *sp_paint_selector_style_button_add(SPPaintSelector *psel, gchar const *px, SPPaintSelector::Mode mode, gchar const *tip);
static void sp_paint_selector_style_button_toggled(GtkToggleButton *tb, SPPaintSelector *psel);
static void sp_paint_selector_fillrule_toggled(GtkToggleButton *tb, SPPaintSelector *psel);

static void sp_paint_selector_set_mode_empty(SPPaintSelector *psel);
static void sp_paint_selector_set_mode_multiple(SPPaintSelector *psel);
static void sp_paint_selector_set_mode_none(SPPaintSelector *psel);
static void sp_paint_selector_set_mode_color(SPPaintSelector *psel, SPPaintSelector::Mode mode);
static void sp_paint_selector_set_mode_gradient(SPPaintSelector *psel, SPPaintSelector::Mode mode);
static void sp_paint_selector_set_mode_pattern(SPPaintSelector *psel, SPPaintSelector::Mode mode);
static void sp_paint_selector_set_mode_swatch(SPPaintSelector *psel, SPPaintSelector::Mode mode);
static void sp_paint_selector_set_mode_unset(SPPaintSelector *psel);


static void sp_paint_selector_set_style_buttons(SPPaintSelector *psel, GtkWidget *active);

static guint psel_signals[LAST_SIGNAL] = {0};

#ifdef SP_PS_VERBOSE
static gchar const* modeStrings[] = {
    "MODE_EMPTY",
    "MODE_MULTIPLE",
    "MODE_NONE",
    "MODE_SOLID_COLOR",
    "MODE_GRADIENT_LINEAR",
    "MODE_GRADIENT_RADIAL",
    "MODE_PATTERN",
    "MODE_SWATCH",
    "MODE_UNSET",
    ".",
    ".",
    ".",
};
#endif


static bool isPaintModeGradient(SPPaintSelector::Mode mode)
{
    bool isGrad = (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) ||
        (mode == SPPaintSelector::MODE_GRADIENT_RADIAL) ||
#ifdef WITH_MESH
        (mode == SPPaintSelector::MODE_GRADIENT_MESH) ||
#endif
        (mode == SPPaintSelector::MODE_SWATCH);

    return isGrad;
}

static SPGradientSelector *getGradientFromData(SPPaintSelector const *psel)
{
    SPGradientSelector *grad = 0;
    if (psel->mode == SPPaintSelector::MODE_SWATCH) {
        SwatchSelector *swatchsel = static_cast<SwatchSelector*>(g_object_get_data(G_OBJECT(psel->selector), "swatch-selector"));
        if (swatchsel) {
            grad = swatchsel->getGradientSelector();
        }
    } else {
        grad = reinterpret_cast<SPGradientSelector*>(g_object_get_data(G_OBJECT(psel->selector), "gradient-selector"));
    }
    return grad;
}

#if GTK_CHECK_VERSION(3,0,0)
G_DEFINE_TYPE(SPPaintSelector, sp_paint_selector, GTK_TYPE_BOX);
#else
G_DEFINE_TYPE(SPPaintSelector, sp_paint_selector, GTK_TYPE_VBOX);
#endif

static void
sp_paint_selector_class_init(SPPaintSelectorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    psel_signals[MODE_CHANGED] = g_signal_new("mode_changed",
                                                G_TYPE_FROM_CLASS(object_class),
                                                (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                                G_STRUCT_OFFSET(SPPaintSelectorClass, mode_changed),
                                                NULL, NULL,
                                                g_cclosure_marshal_VOID__UINT,
                                                G_TYPE_NONE, 1, G_TYPE_UINT);
    psel_signals[GRABBED] =  g_signal_new("grabbed",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPPaintSelectorClass, grabbed),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    psel_signals[DRAGGED] =  g_signal_new("dragged",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPPaintSelectorClass, dragged),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    psel_signals[RELEASED] = g_signal_new("released",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPPaintSelectorClass, released),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    psel_signals[CHANGED] =  g_signal_new("changed",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPPaintSelectorClass, changed),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    psel_signals[FILLRULE_CHANGED] = g_signal_new("fillrule_changed",
                                                    G_TYPE_FROM_CLASS(object_class),
                                                    (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                                    G_STRUCT_OFFSET(SPPaintSelectorClass, fillrule_changed),
                                                    NULL, NULL,
                                                    g_cclosure_marshal_VOID__UINT,
                                                    G_TYPE_NONE, 1, G_TYPE_UINT);

    object_class->dispose = sp_paint_selector_dispose;
}

#define XPAD 4
#define YPAD 1

static void
sp_paint_selector_init(SPPaintSelector *psel)
{
#if GTK_CHECK_VERSION(3,0,0)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(psel), GTK_ORIENTATION_VERTICAL);
#endif

    psel->mode = static_cast<SPPaintSelector::Mode>(-1); // huh?  do you mean 0xff?  --  I think this means "not in the enum"

    /* Paint style button box */
#if GTK_CHECK_VERSION(3,0,0)
    psel->style = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(psel->style), FALSE);
#else
    psel->style = gtk_hbox_new(FALSE, 0);
#endif
    gtk_widget_show(psel->style);
    gtk_container_set_border_width(GTK_CONTAINER(psel->style), 4);
    gtk_box_pack_start(GTK_BOX(psel), psel->style, FALSE, FALSE, 0);

    /* Buttons */
    psel->none = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-none"),
                                                    SPPaintSelector::MODE_NONE, _("No paint"));
    psel->solid = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-solid"),
                                                     SPPaintSelector::MODE_SOLID_COLOR, _("Flat color"));
    psel->gradient = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-gradient-linear"),
                                                        SPPaintSelector::MODE_GRADIENT_LINEAR, _("Linear gradient"));
    psel->radial = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-gradient-radial"),
                                                      SPPaintSelector::MODE_GRADIENT_RADIAL, _("Radial gradient"));
#ifdef WITH_MESH
    psel->mesh = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-gradient-mesh"),
                                                      SPPaintSelector::MODE_GRADIENT_MESH, _("Mesh gradient"));
#endif
    psel->pattern = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-pattern"),
                                                       SPPaintSelector::MODE_PATTERN, _("Pattern"));
    psel->swatch = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-swatch"),
                                                       SPPaintSelector::MODE_SWATCH, _("Swatch"));
    psel->unset = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON("paint-unknown"),
                                                     SPPaintSelector::MODE_UNSET, _("Unset paint (make it undefined so it can be inherited)"));

    /* Fillrule */
    {
#if GTK_CHECK_VERSION(3,0,0)
    psel->fillrulebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(psel->fillrulebox), FALSE);
#else
        psel->fillrulebox = gtk_hbox_new(FALSE, 0);
#endif
        gtk_box_pack_end(GTK_BOX(psel->style), psel->fillrulebox, FALSE, FALSE, 0);

        GtkWidget *w;
        psel->evenodd = gtk_radio_button_new(NULL);
        gtk_button_set_relief(GTK_BUTTON(psel->evenodd), GTK_RELIEF_NONE);
        gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(psel->evenodd), FALSE);
        // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
        gtk_widget_set_tooltip_text(psel->evenodd, _("Any path self-intersections or subpaths create holes in the fill (fill-rule: evenodd)"));
        g_object_set_data(G_OBJECT(psel->evenodd), "mode", GUINT_TO_POINTER(SPPaintSelector::FILLRULE_EVENODD));
        w = sp_icon_new(Inkscape::ICON_SIZE_DECORATION, INKSCAPE_ICON("fill-rule-even-odd"));
        gtk_container_add(GTK_CONTAINER(psel->evenodd), w);
        gtk_box_pack_start(GTK_BOX(psel->fillrulebox), psel->evenodd, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(psel->evenodd), "toggled", G_CALLBACK(sp_paint_selector_fillrule_toggled), psel);

        psel->nonzero = gtk_radio_button_new(gtk_radio_button_get_group(GTK_RADIO_BUTTON(psel->evenodd)));
        gtk_button_set_relief(GTK_BUTTON(psel->nonzero), GTK_RELIEF_NONE);
        gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(psel->nonzero), FALSE);
        // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
        gtk_widget_set_tooltip_text(psel->nonzero, _("Fill is solid unless a subpath is counterdirectional (fill-rule: nonzero)"));
        g_object_set_data(G_OBJECT(psel->nonzero), "mode", GUINT_TO_POINTER(SPPaintSelector::FILLRULE_NONZERO));
        w = sp_icon_new(Inkscape::ICON_SIZE_DECORATION, INKSCAPE_ICON("fill-rule-nonzero"));
        gtk_container_add(GTK_CONTAINER(psel->nonzero), w);
        gtk_box_pack_start(GTK_BOX(psel->fillrulebox), psel->nonzero, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(psel->nonzero), "toggled", G_CALLBACK(sp_paint_selector_fillrule_toggled), psel);
    }

    /* Frame */
    psel->label = gtk_label_new("");
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *lbbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(lbbox), FALSE);
#else
    GtkWidget *lbbox = gtk_hbox_new(FALSE, 4);
#endif
    gtk_widget_show(psel->label);
    gtk_box_pack_start(GTK_BOX(lbbox), psel->label, false, false, 4);
    gtk_box_pack_start(GTK_BOX(psel), lbbox, false, false, 4);

#if GTK_CHECK_VERSION(3,0,0)
    psel->frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(psel->frame), FALSE);
#else
    psel->frame = gtk_vbox_new(FALSE, 4);
#endif
    gtk_widget_show(psel->frame);
    //gtk_container_set_border_width(GTK_CONTAINER(psel->frame), 0);
    gtk_box_pack_start(GTK_BOX(psel), psel->frame, TRUE, TRUE, 0);


    /* Last used color */
    psel->selected_color = new SelectedColor;
    psel->updating_color = false;

    psel->selected_color->signal_grabbed.connect(sigc::mem_fun(psel, &SPPaintSelector::onSelectedColorGrabbed));
    psel->selected_color->signal_dragged.connect(sigc::mem_fun(psel, &SPPaintSelector::onSelectedColorDragged));
    psel->selected_color->signal_released.connect(sigc::mem_fun(psel, &SPPaintSelector::onSelectedColorReleased));
    psel->selected_color->signal_changed.connect(sigc::mem_fun(psel, &SPPaintSelector::onSelectedColorChanged));
}

static void sp_paint_selector_dispose(GObject *object)
{
    SPPaintSelector *psel = SP_PAINT_SELECTOR(object);

    // clean up our long-living pattern menu
    g_object_set_data(G_OBJECT(psel),"patternmenu",NULL);

    if (psel->selected_color) {
        delete psel->selected_color;
        psel->selected_color = NULL;
    }

    if ((G_OBJECT_CLASS(sp_paint_selector_parent_class))->dispose)
        (G_OBJECT_CLASS(sp_paint_selector_parent_class))->dispose(object);
}

static GtkWidget *sp_paint_selector_style_button_add(SPPaintSelector *psel,
                                                     gchar const *pixmap, SPPaintSelector::Mode mode,
                                                     gchar const *tip)
{
    GtkWidget *b, *w;

    b = gtk_toggle_button_new();
    gtk_widget_set_tooltip_text(b, tip);
    gtk_widget_show(b);

    gtk_container_set_border_width(GTK_CONTAINER(b), 0);

    gtk_button_set_relief(GTK_BUTTON(b), GTK_RELIEF_NONE);

    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(b), FALSE);
    g_object_set_data(G_OBJECT(b), "mode", GUINT_TO_POINTER(mode));

    w = sp_icon_new(Inkscape::ICON_SIZE_BUTTON, pixmap);
    gtk_widget_show(w);
    gtk_container_add(GTK_CONTAINER(b), w);

    gtk_box_pack_start(GTK_BOX(psel->style), b, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(b), "toggled", G_CALLBACK(sp_paint_selector_style_button_toggled), psel);

    return b;
}

static void
sp_paint_selector_style_button_toggled(GtkToggleButton *tb, SPPaintSelector *psel)
{
    if (!psel->update && gtk_toggle_button_get_active(tb)) {
        psel->setMode(static_cast<SPPaintSelector::Mode>(GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(tb), "mode"))));
    }
}

static void
sp_paint_selector_fillrule_toggled(GtkToggleButton *tb, SPPaintSelector *psel)
{
    if (!psel->update && gtk_toggle_button_get_active(tb)) {
        SPPaintSelector::FillRule fr = static_cast<SPPaintSelector::FillRule>(GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(tb), "mode")));
        g_signal_emit(G_OBJECT(psel), psel_signals[FILLRULE_CHANGED], 0, fr);
    }
}

static void
sp_paint_selector_show_fillrule(SPPaintSelector *psel, bool is_fill)
{
    if (psel->fillrulebox) {
        if (is_fill) {
            gtk_widget_show_all(psel->fillrulebox);
        } else {
            gtk_widget_destroy(psel->fillrulebox);
            psel->fillrulebox = NULL;
        }
    }
}


SPPaintSelector *sp_paint_selector_new(FillOrStroke kind)
{
    SPPaintSelector *psel = static_cast<SPPaintSelector*>(g_object_new(SP_TYPE_PAINT_SELECTOR, NULL));

    psel->setMode(SPPaintSelector::MODE_MULTIPLE);

    // This silliness is here because I don't know how to pass a parameter to the
    // GtkObject "constructor" (sp_paint_selector_init). Remove it when paint_selector
    // becomes a normal class.
    sp_paint_selector_show_fillrule(psel, kind == FILL);

    return psel;
}

void SPPaintSelector::setMode(Mode mode)
{
    if (this->mode != mode) {
        update = TRUE;
#ifdef SP_PS_VERBOSE
        g_print("Mode change %d -> %d   %s -> %s\n", this->mode, mode, modeStrings[this->mode], modeStrings[mode]);
#endif
        switch (mode) {
            case MODE_EMPTY:
                sp_paint_selector_set_mode_empty(this);
                break;
            case MODE_MULTIPLE:
                sp_paint_selector_set_mode_multiple(this);
                break;
            case MODE_NONE:
                sp_paint_selector_set_mode_none(this);
                break;
            case MODE_SOLID_COLOR:
                sp_paint_selector_set_mode_color(this, mode);
                break;
            case MODE_GRADIENT_LINEAR:
            case MODE_GRADIENT_RADIAL:
#ifdef WITH_MESH
            case MODE_GRADIENT_MESH:
#endif
                sp_paint_selector_set_mode_gradient(this, mode);
                break;
            case MODE_PATTERN:
                sp_paint_selector_set_mode_pattern(this, mode);
                break;
            case MODE_SWATCH:
                sp_paint_selector_set_mode_swatch(this, mode);
                break;
            case MODE_UNSET:
                sp_paint_selector_set_mode_unset(this);
                break;
            default:
                g_warning("file %s: line %d: Unknown paint mode %d", __FILE__, __LINE__, mode);
                break;
        }
        this->mode = mode;
        g_signal_emit(G_OBJECT(this), psel_signals[MODE_CHANGED], 0, this->mode);
        update = FALSE;
    }
}

void SPPaintSelector::setFillrule(FillRule fillrule)
{
    if (fillrulebox) {
        // TODO this flips widgets but does not use a member to store state. Revisit
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(evenodd), (fillrule == FILLRULE_EVENODD));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nonzero), (fillrule == FILLRULE_NONZERO));
    }
}

void SPPaintSelector::setColorAlpha(SPColor const &color, float alpha)
{
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );
/*
    guint32 rgba = 0;

    if ( sp_color_get_colorspace_type(color) == SP_COLORSPACE_TYPE_CMYK )
    {
#ifdef SP_PS_VERBOSE
        g_print("PaintSelector set CMYKA\n");
#endif
        sp_paint_selector_set_mode(psel, MODE_COLOR_CMYK);
    }
    else
*/
    {
#ifdef SP_PS_VERBOSE
        g_print("PaintSelector set RGBA\n");
#endif
        setMode(MODE_SOLID_COLOR);
    }

    updating_color = true;
    selected_color->setColorAlpha(color, alpha);
    updating_color = false;
    //rgba = color.toRGBA32( alpha );
}

void SPPaintSelector::setSwatch(SPGradient *vector )
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set SWATCH\n");
#endif
    setMode(MODE_SWATCH);

    SwatchSelector *swatchsel = static_cast<SwatchSelector*>(g_object_get_data(G_OBJECT(selector), "swatch-selector"));
    if (swatchsel) {
        swatchsel->setVector( (vector) ? vector->document : 0, vector );
    }
}

void SPPaintSelector::setGradientLinear(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT LINEAR\n");
#endif
    setMode(MODE_GRADIENT_LINEAR);

    SPGradientSelector *gsel = getGradientFromData(this);

    gsel->setMode(SPGradientSelector::MODE_LINEAR);
    gsel->setVector((vector) ? vector->document : 0, vector);
}

void SPPaintSelector::setGradientRadial(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT RADIAL\n");
#endif
    setMode(MODE_GRADIENT_RADIAL);

    SPGradientSelector *gsel = getGradientFromData(this);

    gsel->setMode(SPGradientSelector::MODE_RADIAL);

    gsel->setVector((vector) ? vector->document : 0, vector);
}

#ifdef WITH_MESH
void SPPaintSelector::setGradientMesh(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT MESH\n");
#endif
    setMode(MODE_GRADIENT_RADIAL);

    SPGradientSelector *gsel = getGradientFromData(this);

    gsel->setMode(SPGradientSelector::MODE_MESH);
    gsel->setVector((vector) ? vector->document : 0, vector);
}
#endif

void SPPaintSelector::setGradientProperties( SPGradientUnits units, SPGradientSpread spread )
{
    g_return_if_fail(isPaintModeGradient(mode));

    SPGradientSelector *gsel = getGradientFromData(this);
    gsel->setUnits(units);
    gsel->setSpread(spread);
}

void SPPaintSelector::getGradientProperties( SPGradientUnits &units, SPGradientSpread &spread) const
{
    g_return_if_fail(isPaintModeGradient(mode));

    SPGradientSelector *gsel = getGradientFromData(this);
    units = gsel->getUnits();
    spread = gsel->getSpread();
}

/**
 * \post (alpha == NULL) || (*alpha in [0.0, 1.0]).
 */
void SPPaintSelector::getColorAlpha(SPColor &color, gfloat &alpha) const
{
    selected_color->colorAlpha(color, alpha);

    g_assert( ( 0.0 <= alpha )
              && ( alpha <= 1.0 ) );
}

SPGradient *SPPaintSelector::getGradientVector()
{
    SPGradient* vect = 0;

    if (isPaintModeGradient(mode)) {
        SPGradientSelector *gsel = getGradientFromData(this);
        vect = gsel->getVector();
    }

    return vect;
}


void SPPaintSelector::pushAttrsToGradient( SPGradient *gr ) const
{
    SPGradientUnits units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
    SPGradientSpread spread = SP_GRADIENT_SPREAD_PAD;
    getGradientProperties( units, spread );
    gr->setUnits(units);
    gr->setSpread(spread);
    gr->updateRepr();
}

static void
sp_paint_selector_clear_frame(SPPaintSelector *psel)
{
    g_return_if_fail( psel != NULL);

    if (psel->selector) {

        //This is a hack to work around GtkNotebook bug in ColorSelector. Is sends signal switch-page on destroy
        //The widget is hidden firts so it can recognize that it should not process signals from notebook child
        gtk_widget_set_visible(psel->selector, false);
        gtk_widget_destroy(psel->selector);
        psel->selector = NULL;
    }
}

static void
sp_paint_selector_set_mode_empty(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, NULL);
    gtk_widget_set_sensitive(psel->style, FALSE);

    sp_paint_selector_clear_frame(psel);

    gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>No objects</b>"));
}

static void
sp_paint_selector_set_mode_multiple(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, NULL);
    gtk_widget_set_sensitive(psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Multiple styles</b>"));
}

static void
sp_paint_selector_set_mode_unset(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, psel->unset);
    gtk_widget_set_sensitive(psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Paint is undefined</b>"));
}

static void
sp_paint_selector_set_mode_none(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, psel->none);
    gtk_widget_set_sensitive(psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>No paint</b>"));

}

/* Color paint */

void SPPaintSelector::onSelectedColorGrabbed() {
    g_signal_emit(G_OBJECT(this), psel_signals[GRABBED], 0);
}

void SPPaintSelector::onSelectedColorDragged() {
    if (updating_color) {
        return;
    }
    g_signal_emit(G_OBJECT(this), psel_signals[DRAGGED], 0);
}

void SPPaintSelector::onSelectedColorReleased() {
    g_signal_emit(G_OBJECT(this), psel_signals[RELEASED], 0);
}

void SPPaintSelector::onSelectedColorChanged() {
    if (updating_color) {
        return;
    }

    if (mode == MODE_SOLID_COLOR) {
        g_signal_emit(G_OBJECT(this), psel_signals[CHANGED], 0);
    } else {
        g_warning("SPPaintSelector::onSelectedColorChanged(): selected color changed while not in color selection mode");
    }
}

static void sp_paint_selector_set_mode_color(SPPaintSelector *psel, SPPaintSelector::Mode /*mode*/)
{
    using Inkscape::UI::Widget::ColorNotebook;

    if ((psel->mode == SPPaintSelector::MODE_SWATCH) 
            || (psel->mode == SPPaintSelector::MODE_GRADIENT_LINEAR)
            || (psel->mode == SPPaintSelector::MODE_GRADIENT_RADIAL) ) {
        SPGradientSelector *gsel = getGradientFromData(psel);
        if (gsel) {
            SPGradient *gradient = gsel->getVector();

            // Gradient can be null if object paint is changed externally (ie. with a color picker tool)
            if (gradient)
            {
                SPColor color = gradient->getFirstStop()->specified_color;
                float alpha = gradient->getFirstStop()->opacity;
                psel->selected_color->setColorAlpha(color, alpha, false);
            }
        }
    }

    sp_paint_selector_set_style_buttons(psel, psel->solid);
    gtk_widget_set_sensitive(psel->style, TRUE);

    if ((psel->mode == SPPaintSelector::MODE_SOLID_COLOR)) {
        /* Already have color selector */
        // Do nothing
    } else {

        sp_paint_selector_clear_frame(psel);
        /* Create new color selector */
        /* Create vbox */
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(vb), FALSE);
#else
        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
#endif
        gtk_widget_show(vb);

        /* Color selector */
        Gtk::Widget *color_selector = Gtk::manage(new ColorNotebook(*(psel->selected_color)));
        color_selector->show();
        gtk_box_pack_start(GTK_BOX(vb), color_selector->gobj(), TRUE, TRUE, 0);

        /* Pack everything to frame */
        gtk_container_add(GTK_CONTAINER(psel->frame), vb);

        psel->selector = vb;
    }

    gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Flat color</b>"));

#ifdef SP_PS_VERBOSE
    g_print("Color req\n");
#endif
}

/* Gradient */

static void sp_paint_selector_gradient_grabbed(SPGradientSelector * /*csel*/, SPPaintSelector *psel)
{
    g_signal_emit(G_OBJECT(psel), psel_signals[GRABBED], 0);
}

static void sp_paint_selector_gradient_dragged(SPGradientSelector * /*csel*/, SPPaintSelector *psel)
{
    g_signal_emit(G_OBJECT(psel), psel_signals[DRAGGED], 0);
}

static void sp_paint_selector_gradient_released(SPGradientSelector * /*csel*/, SPPaintSelector *psel)
{
    g_signal_emit(G_OBJECT(psel), psel_signals[RELEASED], 0);
}

static void sp_paint_selector_gradient_changed(SPGradientSelector * /*csel*/, SPPaintSelector *psel)
{
    g_signal_emit(G_OBJECT(psel), psel_signals[CHANGED], 0);
}

static void sp_paint_selector_set_mode_gradient(SPPaintSelector *psel, SPPaintSelector::Mode mode)
{
    GtkWidget *gsel;

    /* fixme: We do not need function-wide gsel at all */

    if (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) {
        sp_paint_selector_set_style_buttons(psel, psel->gradient);
    } else if (mode == SPPaintSelector::MODE_GRADIENT_RADIAL) {
        sp_paint_selector_set_style_buttons(psel, psel->radial);
    }
#ifdef WITH_MESH
    else {
        sp_paint_selector_set_style_buttons(psel, psel->mesh);
    }
#endif
    gtk_widget_set_sensitive(psel->style, TRUE);

    if ((psel->mode == SPPaintSelector::MODE_GRADIENT_LINEAR) || (psel->mode == SPPaintSelector::MODE_GRADIENT_RADIAL)) {
        /* Already have gradient selector */
        gsel = GTK_WIDGET(g_object_get_data(G_OBJECT(psel->selector), "gradient-selector"));
    } else {
        sp_paint_selector_clear_frame(psel);
        /* Create new gradient selector */
        gsel = sp_gradient_selector_new();
        gtk_widget_show(gsel);
        g_signal_connect(G_OBJECT(gsel), "grabbed", G_CALLBACK(sp_paint_selector_gradient_grabbed), psel);
        g_signal_connect(G_OBJECT(gsel), "dragged", G_CALLBACK(sp_paint_selector_gradient_dragged), psel);
        g_signal_connect(G_OBJECT(gsel), "released", G_CALLBACK(sp_paint_selector_gradient_released), psel);
        g_signal_connect(G_OBJECT(gsel), "changed", G_CALLBACK(sp_paint_selector_gradient_changed), psel);
        /* Pack everything to frame */
        gtk_container_add(GTK_CONTAINER(psel->frame), gsel);
        psel->selector = gsel;
        g_object_set_data(G_OBJECT(psel->selector), "gradient-selector", gsel);
    }

    /* Actually we have to set option menu history here */
    if (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) {
        SP_GRADIENT_SELECTOR(gsel)->setMode(SPGradientSelector::MODE_LINEAR);
        //sp_gradient_selector_set_mode(SP_GRADIENT_SELECTOR(gsel), SP_GRADIENT_SELECTOR_MODE_LINEAR);
        gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Linear gradient</b>"));
    } else if (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) {
        SP_GRADIENT_SELECTOR(gsel)->setMode(SPGradientSelector::MODE_RADIAL);
        gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Radial gradient</b>"));
    }
#ifdef WITH_MESH
    else {
        SP_GRADIENT_SELECTOR(gsel)->setMode(SPGradientSelector::MODE_MESH);
        gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Mesh gradient</b>"));
    }
#endif

#ifdef SP_PS_VERBOSE
    g_print("Gradient req\n");
#endif
}

static void
sp_paint_selector_set_style_buttons(SPPaintSelector *psel, GtkWidget *active)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->none), (active == psel->none));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->solid), (active == psel->solid));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->gradient), (active == psel->gradient));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->radial), (active == psel->radial));
#ifdef WITH_MESH
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->mesh), (active == psel->mesh));
#endif
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->pattern), (active == psel->pattern));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->swatch), (active == psel->swatch));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(psel->unset), (active == psel->unset));
}

static void sp_psel_pattern_destroy(GtkWidget *widget, SPPaintSelector * /*psel*/)
{
    // drop our reference to the pattern menu widget
    g_object_unref( G_OBJECT(widget) );
}

static void sp_psel_pattern_change(GtkWidget * /*widget*/, SPPaintSelector *psel)
{
    g_signal_emit(G_OBJECT(psel), psel_signals[CHANGED], 0);
}



/**
 *  Returns a list of patterns in the defs of the given source document as a GSList object
 *  Returns NULL if there are no patterns in the document.
 */
static GSList *
ink_pattern_list_get (SPDocument *source)
{
    if (source == NULL)
        return NULL;

    GSList *pl = NULL;
    GSList const *patterns = source->getResourceList("pattern");
    for (GSList *l = const_cast<GSList *>(patterns); l != NULL; l = l->next) {
        if (SP_PATTERN(l->data) == SP_PATTERN(l->data)->rootPattern()) {  // only if this is a root pattern
            pl = g_slist_prepend(pl, l->data);
        }
    }

    pl = g_slist_reverse(pl);
    return pl;
}

/**
 * Adds menu items for pattern list - derived from marker code, left hb etc in to make addition of previews easier at some point.
 */
static void
sp_pattern_menu_build (GtkWidget *combo, GSList *pattern_list, SPDocument */*source*/)
{
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo)));
    GtkTreeIter iter;

    for (; pattern_list != NULL; pattern_list = pattern_list->next) {

        Inkscape::XML::Node *repr = reinterpret_cast<SPItem *>(pattern_list->data)->getRepr();

        // label for combobox
        gchar const *label;
        if (repr->attribute("inkscape:stockid")) {
            label = _(repr->attribute("inkscape:stockid"));
        } else {
            label = _(repr->attribute("id"));
        }

        gchar const *patid = repr->attribute("id");

        gboolean stockid = false;
        if (repr->attribute("inkscape:stockid")) {
            stockid = true;
        }

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                COMBO_COL_LABEL, label, COMBO_COL_STOCK, stockid, COMBO_COL_PATTERN, patid, COMBO_COL_SEP, FALSE, -1);

    }
}

/**
 * Pick up all patterns from source, except those that are in
 * current_doc (if non-NULL), and add items to the pattern menu.
 */
static void sp_pattern_list_from_doc(GtkWidget *combo, SPDocument * /*current_doc*/, SPDocument *source, SPDocument * /*pattern_doc*/)
{
    GSList *pl = ink_pattern_list_get(source);
    GSList *clean_pl = NULL;

    for (; pl != NULL; pl = pl->next) {
        if (!SP_IS_PATTERN(pl->data)) {
            continue;
        }

        // Add to the list of patterns we really do wish to show
        clean_pl = g_slist_prepend (clean_pl, pl->data);
    }

    sp_pattern_menu_build (combo, clean_pl, source);

    g_slist_free (pl);
    g_slist_free (clean_pl);
}


static void
ink_pattern_menu_populate_menu(GtkWidget *combo, SPDocument *doc)
{
    static SPDocument *patterns_doc = NULL;

    // find and load patterns.svg
    if (patterns_doc == NULL) {
        char *patterns_source = g_build_filename(INKSCAPE_PATTERNSDIR, "patterns.svg", NULL);
        if (Inkscape::IO::file_test(patterns_source, G_FILE_TEST_IS_REGULAR)) {
            patterns_doc = SPDocument::createNewDoc(patterns_source, FALSE);
        }
        g_free(patterns_source);
    }

    // suck in from current doc
    sp_pattern_list_from_doc ( combo, NULL, doc, patterns_doc );

    // add separator
    {
        GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo)));
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set(store, &iter,
                COMBO_COL_LABEL, "", COMBO_COL_STOCK, false, COMBO_COL_PATTERN, "", COMBO_COL_SEP, true, -1);
    }

    // suck in from patterns.svg
    if (patterns_doc) {
        doc->ensureUpToDate();
        sp_pattern_list_from_doc ( combo, doc, patterns_doc, NULL );
    }

}


static GtkWidget*
ink_pattern_menu(GtkWidget *combo)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo)));
    GtkTreeIter iter;

    if (!doc) {

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                COMBO_COL_LABEL, _("No document selected"), COMBO_COL_STOCK, false, COMBO_COL_PATTERN, "", COMBO_COL_SEP, false, -1);
        gtk_widget_set_sensitive(combo, FALSE);

    } else {

        ink_pattern_menu_populate_menu(combo, doc);
        gtk_widget_set_sensitive(combo, TRUE);

    }

    // Select the first item that is not a seperator
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store), &iter)) {
        gboolean sep = false;
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COMBO_COL_SEP, &sep, -1);
        if (sep) {
            gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
        }
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter);
    }

    return combo;
}


/*update pattern list*/
void SPPaintSelector::updatePatternList( SPPattern *pattern )
{
    if (update) {
        return;
    }
    GtkWidget *combo = GTK_WIDGET(g_object_get_data(G_OBJECT(this), "patternmenu"));
    g_assert( combo != NULL );

    /* Clear existing menu if any */
    GtkTreeModel *store = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
    gtk_list_store_clear(GTK_LIST_STORE(store));

    ink_pattern_menu(combo);

    /* Set history */

    if (pattern && !g_object_get_data(G_OBJECT(combo), "update")) {

        g_object_set_data(G_OBJECT(combo), "update", GINT_TO_POINTER(TRUE));
        gchar const *patname = pattern->getRepr()->attribute("id");

        // Find this pattern and set it active in the combo_box
        GtkTreeIter iter ;
        gchar *patid = NULL;
        bool valid = gtk_tree_model_get_iter_first (store, &iter);
        if (!valid) {
            return;
        }
        gtk_tree_model_get (store, &iter, COMBO_COL_PATTERN, &patid, -1);
        while (valid && strcmp(patid, patname)  != 0) {
            valid = gtk_tree_model_iter_next (store, &iter);
            gtk_tree_model_get (store, &iter, COMBO_COL_PATTERN, &patid, -1);
        }

        if (valid) {
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter);
        }

        g_object_set_data(G_OBJECT(combo), "update", GINT_TO_POINTER(FALSE));
    }
}

static void sp_paint_selector_set_mode_pattern(SPPaintSelector *psel, SPPaintSelector::Mode mode)
{
    if (mode == SPPaintSelector::MODE_PATTERN) {
        sp_paint_selector_set_style_buttons(psel, psel->pattern);
    }

    gtk_widget_set_sensitive(psel->style, TRUE);

    GtkWidget *tbl = NULL;

    if (psel->mode == SPPaintSelector::MODE_PATTERN) {
        /* Already have pattern menu */
        tbl = GTK_WIDGET(g_object_get_data(G_OBJECT(psel->selector), "pattern-selector"));
    } else {
        sp_paint_selector_clear_frame(psel);

        /* Create vbox */
#if GTK_CHECK_VERSION(3,0,0)
        tbl = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_box_set_homogeneous(GTK_BOX(tbl), FALSE);
#else
        tbl = gtk_vbox_new(FALSE, 4);
#endif
        gtk_widget_show(tbl);

        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new(FALSE, 1);
#endif

            /**
             * Create a combo_box and store with 4 columns,
             * The label, a pointer to the pattern, is stockid or not, is a separator or not.
             */
            GtkListStore *store = gtk_list_store_new (COMBO_N_COLS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_BOOLEAN);
            GtkWidget *combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
            gtk_combo_box_set_row_separator_func(GTK_COMBO_BOX(combo), SPPaintSelector::isSeparator, NULL, NULL);

            GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
            gtk_cell_renderer_set_padding (renderer, 2, 0);
            gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
            gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer, "text", COMBO_COL_LABEL, NULL);

            ink_pattern_menu(combo);
            g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(sp_psel_pattern_change), psel);
            g_signal_connect(G_OBJECT(combo), "destroy", G_CALLBACK(sp_psel_pattern_destroy), psel);
            g_object_set_data(G_OBJECT(psel), "patternmenu", combo);
            g_object_ref( G_OBJECT(combo));

            gtk_container_add(GTK_CONTAINER(hb), combo);
            gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

            g_object_unref( G_OBJECT(store));
        }

        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new(FALSE, 0);
#endif
            GtkWidget *l = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(l), _("Use the <b>Node tool</b> to adjust position, scale, and rotation of the pattern on canvas. Use <b>Object &gt; Pattern &gt; Objects to Pattern</b> to create a new pattern from selection."));
            gtk_label_set_line_wrap(GTK_LABEL(l), true);
            gtk_widget_set_size_request(l, 180, -1);
            gtk_box_pack_start(GTK_BOX(hb), l, TRUE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
            gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
        }

        gtk_widget_show_all(tbl);

        gtk_container_add(GTK_CONTAINER(psel->frame), tbl);
        psel->selector = tbl;
        g_object_set_data(G_OBJECT(psel->selector), "pattern-selector", tbl);

        gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Pattern fill</b>"));
    }
#ifdef SP_PS_VERBOSE
    g_print("Pattern req\n");
#endif
}

gboolean SPPaintSelector::isSeparator (GtkTreeModel *model, GtkTreeIter *iter, gpointer /*data*/) {

    gboolean sep = FALSE;
    gtk_tree_model_get(model, iter, COMBO_COL_SEP, &sep, -1);
    return sep;
}

SPPattern *SPPaintSelector::getPattern()
{
    SPPattern *pat = 0;
    g_return_val_if_fail((mode == MODE_PATTERN) , NULL);

    GtkWidget *combo = GTK_WIDGET(g_object_get_data(G_OBJECT(this), "patternmenu"));

    /* no pattern menu if we were just selected */
    if ( combo == NULL ) {
        return NULL;
    }

    GtkTreeModel *store = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));

    /* Get the selected pattern */
    GtkTreeIter  iter;
    if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX(combo), &iter) ||
            !gtk_list_store_iter_is_valid(GTK_LIST_STORE(store), &iter)) {
        return NULL;
    }

    gchar *patid = NULL;
    gboolean stockid = FALSE;
    gchar *label = NULL;
    gtk_tree_model_get (store, &iter, COMBO_COL_LABEL, &label, COMBO_COL_STOCK, &stockid, COMBO_COL_PATTERN, &patid,  -1);
    if (patid == NULL) {
        return NULL;
    }

    if (strcmp(patid, "none")){

        gchar *paturn;
        if (stockid) {
            paturn = g_strconcat("urn:inkscape:pattern:",patid,NULL);
        }
        else {
            paturn = g_strdup(patid);
        }
        SPObject *pat_obj = get_stock_item(paturn);
        if (pat_obj) {
            pat = SP_PATTERN(pat_obj);
        }
        g_free(paturn);
    } else {
        pat = SP_PATTERN(patid)->rootPattern();
    }

    if (pat && !SP_IS_PATTERN(pat)) {
        pat = 0;
    }

    return pat;
}

static void sp_paint_selector_set_mode_swatch(SPPaintSelector *psel, SPPaintSelector::Mode mode)
{
    if (mode == SPPaintSelector::MODE_SWATCH) {
        sp_paint_selector_set_style_buttons(psel, psel->swatch);
    }

    gtk_widget_set_sensitive(psel->style, TRUE);

    if (psel->mode == SPPaintSelector::MODE_SWATCH){
        // swatchsel = static_cast<SwatchSelector*>(g_object_get_data(G_OBJECT(psel->selector), "swatch-selector"));
    } else {
        sp_paint_selector_clear_frame(psel);
        // Create new gradient selector
        SwatchSelector *swatchsel = new SwatchSelector();
        swatchsel->show();

        swatchsel->connectGrabbedHandler( G_CALLBACK(sp_paint_selector_gradient_grabbed), psel );
        swatchsel->connectDraggedHandler( G_CALLBACK(sp_paint_selector_gradient_dragged), psel );
        swatchsel->connectReleasedHandler( G_CALLBACK(sp_paint_selector_gradient_released), psel );
        swatchsel->connectchangedHandler( G_CALLBACK(sp_paint_selector_gradient_changed), psel );

        // Pack everything to frame
        gtk_container_add(GTK_CONTAINER(psel->frame), GTK_WIDGET(swatchsel->gobj()));
        psel->selector = GTK_WIDGET(swatchsel->gobj());
        g_object_set_data(G_OBJECT(psel->selector), "swatch-selector", swatchsel);

        gtk_label_set_markup(GTK_LABEL(psel->label), _("<b>Swatch fill</b>"));
    }

#ifdef SP_PS_VERBOSE
    g_print("Swatch req\n");
#endif
}

// TODO this seems very bad to be taking in a desktop pointer to muck with. Logic probably belongs elsewhere
void SPPaintSelector::setFlatColor( SPDesktop *desktop, gchar const *color_property, gchar const *opacity_property )
{
    SPCSSAttr *css = sp_repr_css_attr_new();

    SPColor color;
    gfloat alpha = 0;
    getColorAlpha( color, alpha );

    std::string colorStr = color.toString();

#ifdef SP_PS_VERBOSE
    guint32 rgba = color.toRGBA32( alpha );
    g_message("sp_paint_selector_set_flat_color() to '%s' from 0x%08x::%s",
              colorStr.c_str(),
              rgba,
              (color.icc ? color.icc->colorProfile.c_str():"<null>") );
#endif // SP_PS_VERBOSE

    sp_repr_css_set_property(css, color_property, colorStr.c_str());
    Inkscape::CSSOStringStream osalpha;
    osalpha << alpha;
    sp_repr_css_set_property(css, opacity_property, osalpha.str().c_str());

    sp_desktop_set_style(desktop, css);

    sp_repr_css_attr_unref(css);
}

SPPaintSelector::Mode SPPaintSelector::getModeForStyle(SPStyle const & style, FillOrStroke kind)
{
    Mode mode = MODE_UNSET;
    SPIPaint const & target = (kind == FILL) ? style.fill : style.stroke;

    if ( !target.set ) {
        mode = MODE_UNSET;
    } else if ( target.isPaintserver() ) {
        SPPaintServer const *server = (kind == FILL) ? style.getFillPaintServer() : style.getStrokePaintServer();

#ifdef SP_PS_VERBOSE
        g_message("SPPaintSelector::getModeForStyle(%p, %d)", &style, kind);
        g_message("==== server:%p %s  grad:%s   swatch:%s", server, server->getId(), (SP_IS_GRADIENT(server)?"Y":"n"), (SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()?"Y":"n"));
#endif // SP_PS_VERBOSE


        if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
            mode = MODE_SWATCH;
        } else if (SP_IS_LINEARGRADIENT(server)) {
            mode = MODE_GRADIENT_LINEAR;
        } else if (SP_IS_RADIALGRADIENT(server)) {
            mode = MODE_GRADIENT_RADIAL;
#ifdef WITH_MESH
        } else if (SP_IS_MESH(server)) {
            mode = MODE_GRADIENT_MESH;
#endif
        } else if (SP_IS_PATTERN(server)) {
            mode = MODE_PATTERN;
        } else {
            g_warning( "file %s: line %d: Unknown paintserver", __FILE__, __LINE__ );
            mode = MODE_NONE;
        }
    } else if ( target.isColor() ) {
        // TODO this is no longer a valid assertion:
        mode = MODE_SOLID_COLOR; // so far only rgb can be read from svg
    } else if ( target.isNone() ) {
        mode = MODE_NONE;
    } else {
        g_warning( "file %s: line %d: Unknown paint type", __FILE__, __LINE__ );
        mode = MODE_NONE;
    }

    return mode;
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

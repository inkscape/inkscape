/** \file
 * SPPaintSelector: Generic paint selector widget.
 */

/*
 * Authors:
 *   Lauris Kaplinski
 *   bulia byak <buliabyak@users.sf.net>
 *   John Cliff <simarilius@yahoo.com>
 *   Jon A. Cruz <jon@joncruz.org>
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

#include <gtk/gtkhbox.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>

#include "../sp-pattern.h"
#include <glibmm/i18n.h>
#include "../widgets/icon.h"
#include "widgets/widget-sizes.h"
#include "xml/repr.h"

#include "sp-color-notebook.h"
#include "sp-linear-gradient-fns.h"
#include "sp-radial-gradient-fns.h"
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
#include "widgets/swatch-selector.h"

#include "paint-selector.h"

#ifdef SP_PS_VERBOSE
#include "svg/svg-icc-color.h"
#endif // SP_PS_VERBOSE


using Inkscape::Widgets::SwatchSelector;

enum {
    MODE_CHANGED,
    GRABBED,
    DRAGGED,
    RELEASED,
    CHANGED,
    FILLRULE_CHANGED,
    LAST_SIGNAL
};

static void sp_paint_selector_class_init(SPPaintSelectorClass *klass);
static void sp_paint_selector_init(SPPaintSelector *slider);
static void sp_paint_selector_destroy(GtkObject *object);

static GtkWidget *sp_paint_selector_style_button_add(SPPaintSelector *psel, gchar const *px, SPPaintSelector::Mode mode, GtkTooltips *tt, gchar const *tip);
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

static GtkVBoxClass *parent_class;
static guint psel_signals[LAST_SIGNAL] = {0};

#ifdef SP_PS_VERBOSE
static gchar const* modeStrings[] = {
    "MODE_EMPTY",
    "MODE_MULTIPLE",
    "MODE_NONE",
    "MODE_COLOR_RGB",
    "MODE_COLOR_CMYK",
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


static bool isPaintModeGradient( SPPaintSelector::Mode mode )
{
    bool isGrad = (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) ||
        (mode == SPPaintSelector::MODE_GRADIENT_RADIAL) ||
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
        grad = reinterpret_cast<SPGradientSelector*>(gtk_object_get_data(GTK_OBJECT(psel->selector), "gradient-selector"));
    }
    return grad;
}

GType sp_paint_selector_get_type(void)
{
    static GtkType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPPaintSelectorClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_paint_selector_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPPaintSelector),
            0, // n_preallocs
            (GInstanceInitFunc)sp_paint_selector_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_VBOX, "SPPaintSelector", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_paint_selector_class_init(SPPaintSelectorClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;

    parent_class = (GtkVBoxClass*)gtk_type_class(GTK_TYPE_VBOX);

    psel_signals[MODE_CHANGED] = gtk_signal_new("mode_changed",
                                                (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                                GTK_CLASS_TYPE(object_class),
                                                GTK_SIGNAL_OFFSET(SPPaintSelectorClass, mode_changed),
                                                gtk_marshal_NONE__UINT,
                                                GTK_TYPE_NONE, 1, GTK_TYPE_UINT);
    psel_signals[GRABBED] =  gtk_signal_new("grabbed",
                                            (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                            GTK_CLASS_TYPE(object_class),
                                            GTK_SIGNAL_OFFSET(SPPaintSelectorClass, grabbed),
                                            gtk_marshal_NONE__NONE,
                                            GTK_TYPE_NONE, 0);
    psel_signals[DRAGGED] =  gtk_signal_new("dragged",
                                            (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                            GTK_CLASS_TYPE(object_class),
                                            GTK_SIGNAL_OFFSET(SPPaintSelectorClass, dragged),
                                            gtk_marshal_NONE__NONE,
                                            GTK_TYPE_NONE, 0);
    psel_signals[RELEASED] = gtk_signal_new("released",
                                            (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                            GTK_CLASS_TYPE(object_class),
                                            GTK_SIGNAL_OFFSET(SPPaintSelectorClass, released),
                                            gtk_marshal_NONE__NONE,
                                            GTK_TYPE_NONE, 0);
    psel_signals[CHANGED] =  gtk_signal_new("changed",
                                            (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                            GTK_CLASS_TYPE(object_class),
                                            GTK_SIGNAL_OFFSET(SPPaintSelectorClass, changed),
                                            gtk_marshal_NONE__NONE,
                                            GTK_TYPE_NONE, 0);
    psel_signals[FILLRULE_CHANGED] = gtk_signal_new("fillrule_changed",
                                                    (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                                    GTK_CLASS_TYPE(object_class),
                                                    GTK_SIGNAL_OFFSET(SPPaintSelectorClass, fillrule_changed),
                                                    gtk_marshal_NONE__UINT,
                                                    GTK_TYPE_NONE, 1, GTK_TYPE_UINT);

    object_class->destroy = sp_paint_selector_destroy;
}

#define XPAD 4
#define YPAD 1

static void
sp_paint_selector_init(SPPaintSelector *psel)
{
    GtkTooltips *tt = gtk_tooltips_new();

    psel->mode = static_cast<SPPaintSelector::Mode>(-1); // huh?  do you mean 0xff?  --  I think this means "not in the enum"

    /* Paint style button box */
    psel->style = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(psel->style);
    gtk_container_set_border_width(GTK_CONTAINER(psel->style), 4);
    gtk_box_pack_start(GTK_BOX(psel), psel->style, FALSE, FALSE, 0);

    /* Buttons */
    psel->none = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_NONE,
                                                    SPPaintSelector::MODE_NONE, tt, _("No paint"));
    psel->solid = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_SOLID,
                                                     SPPaintSelector::MODE_COLOR_RGB, tt, _("Flat color"));
    psel->gradient = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_GRADIENT_LINEAR,
                                                        SPPaintSelector::MODE_GRADIENT_LINEAR, tt, _("Linear gradient"));
    psel->radial = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_GRADIENT_RADIAL,
                                                      SPPaintSelector::MODE_GRADIENT_RADIAL, tt, _("Radial gradient"));
    psel->pattern = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_PATTERN,
                                                       SPPaintSelector::MODE_PATTERN, tt, _("Pattern"));
    psel->swatch = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_SWATCH,
                                                       SPPaintSelector::MODE_SWATCH, tt, _("Swatch"));
    psel->unset = sp_paint_selector_style_button_add(psel, INKSCAPE_ICON_PAINT_UNKNOWN,
                                                     SPPaintSelector::MODE_UNSET, tt, _("Unset paint (make it undefined so it can be inherited)"));

    /* Fillrule */
    {
        psel->fillrulebox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_end(GTK_BOX(psel->style), psel->fillrulebox, FALSE, FALSE, 0);

        GtkWidget *w;
        psel->evenodd = gtk_radio_button_new(NULL);
        gtk_button_set_relief(GTK_BUTTON(psel->evenodd), GTK_RELIEF_NONE);
        gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(psel->evenodd), FALSE);
        // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
        gtk_tooltips_set_tip(tt, psel->evenodd, _("Any path self-intersections or subpaths create holes in the fill (fill-rule: evenodd)"), NULL);
        gtk_object_set_data(GTK_OBJECT(psel->evenodd), "mode", GUINT_TO_POINTER(SPPaintSelector::FILLRULE_EVENODD));
        w = sp_icon_new(Inkscape::ICON_SIZE_DECORATION, INKSCAPE_ICON_FILL_RULE_EVEN_ODD);
        gtk_container_add(GTK_CONTAINER(psel->evenodd), w);
        gtk_box_pack_start(GTK_BOX(psel->fillrulebox), psel->evenodd, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(psel->evenodd), "toggled", GTK_SIGNAL_FUNC(sp_paint_selector_fillrule_toggled), psel);

        psel->nonzero = gtk_radio_button_new(gtk_radio_button_group(GTK_RADIO_BUTTON(psel->evenodd)));
        gtk_button_set_relief(GTK_BUTTON(psel->nonzero), GTK_RELIEF_NONE);
        gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(psel->nonzero), FALSE);
        // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
        gtk_tooltips_set_tip(tt, psel->nonzero, _("Fill is solid unless a subpath is counterdirectional (fill-rule: nonzero)"), NULL);
        gtk_object_set_data(GTK_OBJECT(psel->nonzero), "mode", GUINT_TO_POINTER(SPPaintSelector::FILLRULE_NONZERO));
        w = sp_icon_new(Inkscape::ICON_SIZE_DECORATION, INKSCAPE_ICON_FILL_RULE_NONZERO);
        gtk_container_add(GTK_CONTAINER(psel->nonzero), w);
        gtk_box_pack_start(GTK_BOX(psel->fillrulebox), psel->nonzero, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(psel->nonzero), "toggled", GTK_SIGNAL_FUNC(sp_paint_selector_fillrule_toggled), psel);
    }

    /* Frame */
    psel->frame = gtk_frame_new("");
    gtk_widget_show(psel->frame);
    gtk_container_set_border_width(GTK_CONTAINER(psel->frame), 0);
    gtk_box_pack_start(GTK_BOX(psel), psel->frame, TRUE, TRUE, 0);

    /* Last used color */
    psel->color.set( 0.0, 0.0, 0.0 );
    psel->alpha = 1.0;
}

static void
sp_paint_selector_destroy(GtkObject *object)
{
    SPPaintSelector *psel = SP_PAINT_SELECTOR(object);

    // clean up our long-living pattern menu
    g_object_set_data(G_OBJECT(psel),"patternmenu",NULL);

    if (((GtkObjectClass *) parent_class)->destroy)
        (* ((GtkObjectClass *) parent_class)->destroy)(object);
}

static GtkWidget *sp_paint_selector_style_button_add(SPPaintSelector *psel,
                                                     gchar const *pixmap, SPPaintSelector::Mode mode,
                                                     GtkTooltips *tt, gchar const *tip)
{
    GtkWidget *b, *w;

    b = gtk_toggle_button_new();
    gtk_tooltips_set_tip(tt, b, tip, NULL);
    gtk_widget_show(b);

    gtk_container_set_border_width(GTK_CONTAINER(b), 0);

    gtk_button_set_relief(GTK_BUTTON(b), GTK_RELIEF_NONE);

    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(b), FALSE);
    gtk_object_set_data(GTK_OBJECT(b), "mode", GUINT_TO_POINTER(mode));

    w = sp_icon_new(Inkscape::ICON_SIZE_BUTTON, pixmap);
    gtk_widget_show(w);
    gtk_container_add(GTK_CONTAINER(b), w);

    gtk_box_pack_start(GTK_BOX(psel->style), b, FALSE, FALSE, 0);
    gtk_signal_connect(GTK_OBJECT(b), "toggled", GTK_SIGNAL_FUNC(sp_paint_selector_style_button_toggled), psel);

    return b;
}

static void
sp_paint_selector_style_button_toggled(GtkToggleButton *tb, SPPaintSelector *psel)
{
    if (!psel->update && gtk_toggle_button_get_active(tb)) {
        psel->setMode(static_cast<SPPaintSelector::Mode>(GPOINTER_TO_UINT(gtk_object_get_data(GTK_OBJECT(tb), "mode"))));
    }
}

static void
sp_paint_selector_fillrule_toggled(GtkToggleButton *tb, SPPaintSelector *psel)
{
    if (!psel->update && gtk_toggle_button_get_active(tb)) {
        SPPaintSelector::FillRule fr = static_cast<SPPaintSelector::FillRule>(GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(tb), "mode")));
        gtk_signal_emit(GTK_OBJECT(psel), psel_signals[FILLRULE_CHANGED], fr);
    }
}

void
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


GtkWidget *
sp_paint_selector_new(bool is_fill)
{
    SPPaintSelector *psel = static_cast<SPPaintSelector*>(gtk_type_new(SP_TYPE_PAINT_SELECTOR));

    psel->setMode(SPPaintSelector::MODE_MULTIPLE);

    // This silliness is here because I don't know how to pass a parameter to the
    // GtkObject "constructor" (sp_paint_selector_init). Remove it when paint_selector
    // becomes a normal class.
    sp_paint_selector_show_fillrule(psel, is_fill);

    return GTK_WIDGET(psel);
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
            case MODE_COLOR_RGB:
            case MODE_COLOR_CMYK:
                sp_paint_selector_set_mode_color(this, mode);
                break;
            case MODE_GRADIENT_LINEAR:
            case MODE_GRADIENT_RADIAL:
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
        gtk_signal_emit(GTK_OBJECT(this), psel_signals[MODE_CHANGED], this->mode);
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
    SPColorSelector *csel = 0;
    guint32 rgba = 0;

/*
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
        setMode(MODE_COLOR_RGB);
    }

    csel = reinterpret_cast<SPColorSelector*>(gtk_object_get_data(GTK_OBJECT(selector), "color-selector"));
    rgba = color.toRGBA32( alpha );
    csel->base->setColorAlpha( color, alpha );
}

void SPPaintSelector::setSwatch(SPGradient *vector )
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set SWATCH\n");
#endif
    setMode(MODE_SWATCH);

    SwatchSelector *swatchsel = static_cast<SwatchSelector*>(g_object_get_data(G_OBJECT(selector), "swatch-selector"));
    if (swatchsel) {
        swatchsel->setVector( (vector) ? SP_OBJECT_DOCUMENT(vector) : 0, vector );
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
    gsel->setVector((vector) ? SP_OBJECT_DOCUMENT(vector) : 0, vector);
}

void SPPaintSelector::setGradientRadial(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT RADIAL\n");
#endif
    setMode(MODE_GRADIENT_RADIAL);

    SPGradientSelector *gsel = getGradientFromData(this);

    gsel->setMode(SPGradientSelector::MODE_RADIAL);

    gsel->setVector((vector) ? SP_OBJECT_DOCUMENT(vector) : 0, vector);
}

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
    SPColorSelector *csel;

    csel = reinterpret_cast<SPColorSelector*>(g_object_get_data(G_OBJECT(selector), "color-selector"));

    csel->base->getColorAlpha( color, alpha );

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
    sp_gradient_set_units(gr, units);
    sp_gradient_set_spread(gr, spread);
    SP_OBJECT(gr)->updateRepr();
}

static void
sp_paint_selector_clear_frame(SPPaintSelector *psel)
{
    g_return_if_fail( psel != NULL);

    if (psel->selector) {

        /* before we destroy the frame contents, we must detach
         * the patternmenu so that Gtk doesn't gtk_widget_destroy
         * all the children of the menu.  (We also have a g_object_ref
         * count set on it too so that the gtk_container_remove doesn't
         * end up destroying it.
         */
        GtkWidget *patterns = (GtkWidget *)g_object_get_data(G_OBJECT(psel), "patternmenu");
        if (patterns != NULL) {
            GtkWidget * parent = gtk_widget_get_parent( GTK_WIDGET(patterns));
            if ( parent != NULL ) {
                g_assert( GTK_IS_CONTAINER(parent) );
                gtk_container_remove( GTK_CONTAINER(parent), patterns );
            }
        }

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

    gtk_frame_set_label(GTK_FRAME(psel->frame), _("No objects"));
}

static void
sp_paint_selector_set_mode_multiple(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, NULL);
    gtk_widget_set_sensitive(psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_frame_set_label(GTK_FRAME(psel->frame), _("Multiple styles"));
}

static void
sp_paint_selector_set_mode_unset(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, psel->unset);
    gtk_widget_set_sensitive(psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_frame_set_label(GTK_FRAME(psel->frame), _("Paint is undefined"));
}

static void
sp_paint_selector_set_mode_none(SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons(psel, psel->none);
    gtk_widget_set_sensitive(psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_frame_set_label(GTK_FRAME(psel->frame), _("No paint"));
}

/* Color paint */

static void sp_paint_selector_color_grabbed(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[GRABBED]);
}

static void sp_paint_selector_color_dragged(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[DRAGGED]);
}

static void sp_paint_selector_color_released(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[RELEASED]);
}

static void
sp_paint_selector_color_changed(SPColorSelector *csel, SPPaintSelector *psel)
{
    csel->base->getColorAlpha( psel->color, psel->alpha );

    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[CHANGED]);
}

static void sp_paint_selector_set_mode_color(SPPaintSelector *psel, SPPaintSelector::Mode /*mode*/)
{
    GtkWidget *csel;

    sp_paint_selector_set_style_buttons(psel, psel->solid);
    gtk_widget_set_sensitive(psel->style, TRUE);

    if ((psel->mode == SPPaintSelector::MODE_COLOR_RGB) || (psel->mode == SPPaintSelector::MODE_COLOR_CMYK)) {
        /* Already have color selector */
        csel = (GtkWidget*)gtk_object_get_data(GTK_OBJECT(psel->selector), "color-selector");
    } else {

        sp_paint_selector_clear_frame(psel);
        /* Create new color selector */
        /* Create vbox */
        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
        gtk_widget_show(vb);

        /* Color selector */
        csel = sp_color_selector_new( SP_TYPE_COLOR_NOTEBOOK );
        gtk_widget_show(csel);
        gtk_object_set_data(GTK_OBJECT(vb), "color-selector", csel);
        gtk_box_pack_start(GTK_BOX(vb), csel, TRUE, TRUE, 0);
        gtk_signal_connect(GTK_OBJECT(csel), "grabbed", GTK_SIGNAL_FUNC(sp_paint_selector_color_grabbed), psel);
        gtk_signal_connect(GTK_OBJECT(csel), "dragged", GTK_SIGNAL_FUNC(sp_paint_selector_color_dragged), psel);
        gtk_signal_connect(GTK_OBJECT(csel), "released", GTK_SIGNAL_FUNC(sp_paint_selector_color_released), psel);
        gtk_signal_connect(GTK_OBJECT(csel), "changed", GTK_SIGNAL_FUNC(sp_paint_selector_color_changed), psel);
        /* Pack everything to frame */
        gtk_container_add(GTK_CONTAINER(psel->frame), vb);
        psel->selector = vb;

        /* Set color */
        SP_COLOR_SELECTOR( csel )->base->setColorAlpha( psel->color, psel->alpha );

    }

    gtk_frame_set_label(GTK_FRAME(psel->frame), _("Flat color"));
#ifdef SP_PS_VERBOSE
    g_print("Color req\n");
#endif
}

/* Gradient */

static void sp_paint_selector_gradient_grabbed(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[GRABBED]);
}

static void sp_paint_selector_gradient_dragged(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[DRAGGED]);
}

static void sp_paint_selector_gradient_released(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[RELEASED]);
}

static void sp_paint_selector_gradient_changed(SPColorSelector * /*csel*/, SPPaintSelector *psel)
{
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[CHANGED]);
}

static void sp_paint_selector_set_mode_gradient(SPPaintSelector *psel, SPPaintSelector::Mode mode)
{
    GtkWidget *gsel;

    /* fixme: We do not need function-wide gsel at all */

    if (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) {
        sp_paint_selector_set_style_buttons(psel, psel->gradient);
    } else {
        sp_paint_selector_set_style_buttons(psel, psel->radial);
    }
    gtk_widget_set_sensitive(psel->style, TRUE);

    if ((psel->mode == SPPaintSelector::MODE_GRADIENT_LINEAR) || (psel->mode == SPPaintSelector::MODE_GRADIENT_RADIAL)) {
        /* Already have gradient selector */
        gsel = (GtkWidget*)gtk_object_get_data(GTK_OBJECT(psel->selector), "gradient-selector");
    } else {
        sp_paint_selector_clear_frame(psel);
        /* Create new gradient selector */
        gsel = sp_gradient_selector_new();
        gtk_widget_show(gsel);
        gtk_signal_connect(GTK_OBJECT(gsel), "grabbed", GTK_SIGNAL_FUNC(sp_paint_selector_gradient_grabbed), psel);
        gtk_signal_connect(GTK_OBJECT(gsel), "dragged", GTK_SIGNAL_FUNC(sp_paint_selector_gradient_dragged), psel);
        gtk_signal_connect(GTK_OBJECT(gsel), "released", GTK_SIGNAL_FUNC(sp_paint_selector_gradient_released), psel);
        gtk_signal_connect(GTK_OBJECT(gsel), "changed", GTK_SIGNAL_FUNC(sp_paint_selector_gradient_changed), psel);
        /* Pack everything to frame */
        gtk_container_add(GTK_CONTAINER(psel->frame), gsel);
        psel->selector = gsel;
        gtk_object_set_data(GTK_OBJECT(psel->selector), "gradient-selector", gsel);
    }

    /* Actually we have to set option menu history here */
    if (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) {
        SP_GRADIENT_SELECTOR(gsel)->setMode(SPGradientSelector::MODE_LINEAR);
        //sp_gradient_selector_set_mode(SP_GRADIENT_SELECTOR(gsel), SP_GRADIENT_SELECTOR_MODE_LINEAR);
        gtk_frame_set_label(GTK_FRAME(psel->frame), _("Linear gradient"));
    } else {
        SP_GRADIENT_SELECTOR(gsel)->setMode(SPGradientSelector::MODE_RADIAL);
        gtk_frame_set_label(GTK_FRAME(psel->frame), _("Radial gradient"));
    }
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
    gtk_signal_emit(GTK_OBJECT(psel), psel_signals[CHANGED]);
}



/**
 *  Returns a list of patterns in the defs of the given source document as a GSList object
 *  Returns NULL if there are no patterns in the document.
 */
GSList *
ink_pattern_list_get (SPDocument *source)
{
    if (source == NULL)
        return NULL;

    GSList *pl = NULL;
    GSList const *patterns = sp_document_get_resource_list(source, "pattern");
    for (GSList *l = (GSList *) patterns; l != NULL; l = l->next) {
        if (SP_PATTERN(l->data) == pattern_getroot(SP_PATTERN(l->data))) {  // only if this is a root pattern
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
sp_pattern_menu_build (GtkWidget *m, GSList *pattern_list, SPDocument */*source*/)
{

    for (; pattern_list != NULL; pattern_list = pattern_list->next) {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) pattern_list->data);
                GtkWidget *i = gtk_menu_item_new();
                gtk_widget_show(i);

        if (repr->attribute("inkscape:stockid"))
            g_object_set_data (G_OBJECT(i), "stockid", (void *) "true");
        else
            g_object_set_data (G_OBJECT(i), "stockid", (void *) "false");

        gchar const *patid = repr->attribute("id");
        g_object_set_data (G_OBJECT(i), "pattern", (void *) patid);

                GtkWidget *hb = gtk_hbox_new(FALSE, 4);
                gtk_widget_show(hb);

        // create label
                GtkWidget *l;
                if (repr->attribute("inkscape:stockid"))
                    l = gtk_label_new(_(repr->attribute("inkscape:stockid")));
                else
                    l = gtk_label_new(_(repr->attribute("id")));
                gtk_widget_show(l);
                gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.5);

                gtk_box_pack_start(GTK_BOX(hb), l, TRUE, TRUE, 0);

                gtk_widget_show(hb);
                gtk_container_add(GTK_CONTAINER(i), hb);

                gtk_menu_append(GTK_MENU(m), i);
            }
        }

/**
 * sp_pattern_list_from_doc()
 *
 * \brief Pick up all patterns from source, except those that are in
 * current_doc (if non-NULL), and add items to the pattern menu
 *
 */
static void sp_pattern_list_from_doc (GtkWidget *m, SPDocument * /*current_doc*/, SPDocument *source, SPDocument * /*pattern_doc*/)
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

    sp_pattern_menu_build (m, clean_pl, source);

    g_slist_free (pl);
    g_slist_free (clean_pl);
}




static void
ink_pattern_menu_populate_menu(GtkWidget *m, SPDocument *doc)
{
    static SPDocument *patterns_doc = NULL;

    // find and load patterns.svg
    if (patterns_doc == NULL) {
        char *patterns_source = g_build_filename(INKSCAPE_PATTERNSDIR, "patterns.svg", NULL);
        if (Inkscape::IO::file_test(patterns_source, G_FILE_TEST_IS_REGULAR)) {
            patterns_doc = sp_document_new(patterns_source, FALSE);
        }
        g_free(patterns_source);
    }

    // suck in from current doc
    sp_pattern_list_from_doc ( m, NULL, doc, patterns_doc );

    // add separator
    {
        GtkWidget *i = gtk_separator_menu_item_new();
        gchar const *patid = "";
        g_object_set_data (G_OBJECT(i), "pattern", (void *) patid);
        gtk_widget_show(i);
        gtk_menu_append(GTK_MENU(m), i);
    }

    // suck in from patterns.svg
    if (patterns_doc) {
        sp_document_ensure_up_to_date(doc);
        sp_pattern_list_from_doc ( m, doc, patterns_doc, NULL );
    }

}


static GtkWidget*
ink_pattern_menu(GtkWidget *mnu)
{
   /* Create new menu widget */
    GtkWidget *m = gtk_menu_new();
    gtk_widget_show(m);
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    if (!doc) {
        GtkWidget *i;
        i = gtk_menu_item_new_with_label(_("No document selected"));
        gtk_widget_show(i);
        gtk_menu_append(GTK_MENU(m), i);
        gtk_widget_set_sensitive(mnu, FALSE);
    } else {

       ink_pattern_menu_populate_menu(m, doc);
        gtk_widget_set_sensitive(mnu, TRUE);

    }
    gtk_option_menu_set_menu(GTK_OPTION_MENU(mnu), m);

    /* Set history */
    gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), 0);
    return mnu;
}


/*update pattern list*/
void SPPaintSelector::updatePatternList( SPPattern *pattern )
{
    if (update) {
        return;
    }
    GtkWidget *mnu = GTK_WIDGET(g_object_get_data(G_OBJECT(this), "patternmenu"));
    g_assert( mnu != NULL );

    /* Clear existing menu if any */
    gtk_option_menu_remove_menu(GTK_OPTION_MENU(mnu));

    ink_pattern_menu(mnu);

    /* Set history */

    if (pattern && !gtk_object_get_data(GTK_OBJECT(mnu), "update")) {

        gtk_object_set_data(GTK_OBJECT(mnu), "update", GINT_TO_POINTER(TRUE));

        gchar *patname = (gchar *) SP_OBJECT_REPR(pattern)->attribute("id");

        GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(mnu)));

        GList *kids = GTK_MENU_SHELL(m)->children;

        int patpos = 0;
        int i = 0;

        for (; kids != NULL; kids = kids->next) {

            gchar *men_pat = (gchar *) g_object_get_data(G_OBJECT(kids->data), "pattern");
            if ( strcmp(men_pat, patname) == 0 ) {
                patpos = i;
            }
            i++;
        }


        gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), patpos);
        gtk_object_set_data(GTK_OBJECT(mnu), "update", GINT_TO_POINTER(FALSE));
    }
    //gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), 0);
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
        tbl = (GtkWidget*)gtk_object_get_data(GTK_OBJECT(psel->selector), "pattern-selector");
    } else {
        sp_paint_selector_clear_frame(psel);

        /* Create vbox */
        tbl = gtk_vbox_new(FALSE, 4);
        gtk_widget_show(tbl);

        {
            GtkWidget *hb = gtk_hbox_new(FALSE, 1);

            GtkWidget *mnu = gtk_option_menu_new();
            ink_pattern_menu(mnu);
            gtk_signal_connect(GTK_OBJECT(mnu), "changed", GTK_SIGNAL_FUNC(sp_psel_pattern_change), psel);
            gtk_signal_connect(GTK_OBJECT(mnu), "destroy", GTK_SIGNAL_FUNC(sp_psel_pattern_destroy), psel);
            gtk_object_set_data(GTK_OBJECT(psel), "patternmenu", mnu);
            g_object_ref( G_OBJECT(mnu));

            gtk_container_add(GTK_CONTAINER(hb), mnu);
            gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
        }

        {
            GtkWidget *hb = gtk_hbox_new(FALSE, 0);
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
        gtk_object_set_data(GTK_OBJECT(psel->selector), "pattern-selector", tbl);

        gtk_frame_set_label(GTK_FRAME(psel->frame), _("Pattern fill"));
    }
#ifdef SP_PS_VERBOSE
    g_print("Pattern req\n");
#endif
}

SPPattern *SPPaintSelector::getPattern()
{
    SPPattern *pat = 0;
    g_return_val_if_fail((mode == MODE_PATTERN) , NULL);

    GtkWidget *patmnu = (GtkWidget *) g_object_get_data(G_OBJECT(this), "patternmenu");
    /* no pattern menu if we were just selected */
    if ( patmnu == NULL ) return NULL;

    GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(patmnu)));

    /* Get Pattern */
    if (!g_object_get_data(G_OBJECT(gtk_menu_get_active(m)), "pattern"))
    {
        return NULL;
    }
    gchar *patid = (gchar *) g_object_get_data(G_OBJECT(gtk_menu_get_active(m)),
                                               "pattern");
    //gchar *pattern = "";
    if (strcmp(patid, "none")){

        gchar *stockid = (gchar *) g_object_get_data(G_OBJECT(gtk_menu_get_active(m)),
                                                     "stockid");
        gchar *paturn = patid;
        if (!strcmp(stockid,"true")) paturn = g_strconcat("urn:inkscape:pattern:",patid,NULL);
        SPObject *pat_obj = get_stock_item(paturn);
        if (pat_obj) {
            pat = SP_PATTERN(pat_obj);
        }
    } else {
        pat = pattern_getroot(SP_PATTERN(g_object_get_data(G_OBJECT(gtk_menu_get_active(m)), "pattern")));
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

    SwatchSelector *swatchsel = 0;

    if (psel->mode == SPPaintSelector::MODE_SWATCH){
        /* Already have pattern menu */
        swatchsel = static_cast<SwatchSelector*>(g_object_get_data(G_OBJECT(psel->selector), "swatch-selector"));
    } else {
        sp_paint_selector_clear_frame(psel);
        /* Create new gradient selector */
        SwatchSelector *swatchsel = new SwatchSelector();
        swatchsel->show();

        swatchsel->connectGrabbedHandler( G_CALLBACK(sp_paint_selector_gradient_grabbed), psel );
        swatchsel->connectDraggedHandler( G_CALLBACK(sp_paint_selector_gradient_dragged), psel );
        swatchsel->connectReleasedHandler( G_CALLBACK(sp_paint_selector_gradient_released), psel );
        swatchsel->connectchangedHandler( G_CALLBACK(sp_paint_selector_gradient_changed), psel );

        // Pack everything to frame
        gtk_container_add(GTK_CONTAINER(psel->frame), GTK_WIDGET(swatchsel->gobj()));
        psel->selector = GTK_WIDGET(swatchsel->gobj());
        gtk_object_set_data(GTK_OBJECT(psel->selector), "swatch-selector", swatchsel);

        gtk_frame_set_label(GTK_FRAME(psel->frame), _("Swatch fill"));
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

SPPaintSelector::Mode sp_style_determine_paint_selector_mode(SPStyle *style, bool isfill)
{
    SPPaintSelector::Mode mode = SPPaintSelector::MODE_UNSET;
    SPIPaint& target = isfill ? style->fill : style->stroke;

    if ( !target.set ) {
        mode = SPPaintSelector::MODE_UNSET;
    } else if ( target.isPaintserver() ) {
        SPPaintServer *server = isfill? SP_STYLE_FILL_SERVER(style) : SP_STYLE_STROKE_SERVER(style);


#ifdef SP_PS_VERBOSE
        g_message("==== server:%p %s  grad:%s   swatch:%s", server, server->getId(), (SP_IS_GRADIENT(server)?"Y":"n"), (SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()?"Y":"n"));
#endif // SP_PS_VERBOSE


        if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
            mode = SPPaintSelector::MODE_SWATCH;
        } else if (SP_IS_LINEARGRADIENT(server)) {
            mode = SPPaintSelector::MODE_GRADIENT_LINEAR;
        } else if (SP_IS_RADIALGRADIENT(server)) {
            mode = SPPaintSelector::MODE_GRADIENT_RADIAL;
        } else if (SP_IS_PATTERN(server)) {
            mode = SPPaintSelector::MODE_PATTERN;
        } else {
            g_warning( "file %s: line %d: Unknown paintserver", __FILE__, __LINE__ );
            mode = SPPaintSelector::MODE_NONE;
        }
    } else if ( target.isColor() ) {
        // TODO this is no longer a valid assertion:
        mode = SPPaintSelector::MODE_COLOR_RGB; // so far only rgb can be read from svg
    } else if ( target.isNone() ) {
        mode = SPPaintSelector::MODE_NONE;
    } else {
        g_warning( "file %s: line %d: Unknown paint type", __FILE__, __LINE__ );
        mode = SPPaintSelector::MODE_NONE;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

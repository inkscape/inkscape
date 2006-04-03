#define __SP_PAINT_SELECTOR_C__

/** \file
 * SPPaintSelector: Generic paint selector widget.
 */

/*
 * Copyright (C) Lauris Kaplinski 2002
 *   bulia byak <buliabyak@users.sf.net>
*/

#define noSP_PS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif



#include <gtk/gtkhbox.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkmenuitem.h>

#include "../sp-pattern.h"
#include <glibmm/i18n.h>
#include "../widgets/icon.h"
#include "../inkscape-stock.h"
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

#include "paint-selector.h"

enum {
	MODE_CHANGED,
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	FILLRULE_CHANGED,
	LAST_SIGNAL
};

static void sp_paint_selector_class_init (SPPaintSelectorClass *klass);
static void sp_paint_selector_init (SPPaintSelector *slider);
static void sp_paint_selector_destroy (GtkObject *object);

static GtkWidget *sp_paint_selector_style_button_add (SPPaintSelector *psel, gchar const *px, SPPaintSelectorMode mode, GtkTooltips *tt, gchar const *tip);
static void sp_paint_selector_style_button_toggled (GtkToggleButton *tb, SPPaintSelector *psel);
static void sp_paint_selector_fillrule_toggled (GtkToggleButton *tb, SPPaintSelector *psel);

static void sp_paint_selector_set_mode_empty (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_multiple (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_none (SPPaintSelector *psel);
static void sp_paint_selector_set_mode_color (SPPaintSelector *psel, SPPaintSelectorMode mode);
static void sp_paint_selector_set_mode_gradient (SPPaintSelector *psel, SPPaintSelectorMode mode);
static void sp_paint_selector_set_mode_pattern (SPPaintSelector *psel, SPPaintSelectorMode mode);
static void sp_paint_selector_set_mode_unset (SPPaintSelector *psel);


static void sp_paint_selector_set_style_buttons (SPPaintSelector *psel, GtkWidget *active);

static GtkVBoxClass *parent_class;
static guint psel_signals[LAST_SIGNAL] = {0};

GtkType
sp_paint_selector_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPPaintSelector",
			sizeof (SPPaintSelector),
			sizeof (SPPaintSelectorClass),
			(GtkClassInitFunc) sp_paint_selector_class_init,
			(GtkObjectInitFunc) sp_paint_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_VBOX, &info);
	}
	return type;
}

static void
sp_paint_selector_class_init (SPPaintSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

	psel_signals[MODE_CHANGED] = gtk_signal_new ("mode_changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, mode_changed),
						 gtk_marshal_NONE__UINT,
						 GTK_TYPE_NONE, 1, GTK_TYPE_UINT);
	psel_signals[GRABBED] =  gtk_signal_new ("grabbed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, grabbed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[DRAGGED] =  gtk_signal_new ("dragged",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, dragged),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[RELEASED] = gtk_signal_new ("released",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, released),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[CHANGED] =  gtk_signal_new ("changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, changed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	psel_signals[FILLRULE_CHANGED] = gtk_signal_new ("fillrule_changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPPaintSelectorClass, fillrule_changed),
						 gtk_marshal_NONE__UINT,
						 GTK_TYPE_NONE, 1, GTK_TYPE_UINT);

	object_class->destroy = sp_paint_selector_destroy;
}

#define XPAD 4
#define YPAD 1

static void
sp_paint_selector_init (SPPaintSelector *psel)
{
	GtkTooltips *tt = gtk_tooltips_new();

	psel->mode = (SPPaintSelectorMode)-1; // huh?  do you mean 0xff?  --  I think this means "not in the enum"

	/* Paint style button box */
	psel->style = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (psel->style);
	gtk_container_set_border_width (GTK_CONTAINER (psel->style), 4);
	gtk_box_pack_start (GTK_BOX (psel), psel->style, FALSE, FALSE, 0);

	/* Buttons */
	psel->none = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_NONE,
							 SP_PAINT_SELECTOR_MODE_NONE, tt, _("No paint"));
	psel->solid = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_SOLID,
							  SP_PAINT_SELECTOR_MODE_COLOR_RGB, tt, _("Flat color"));
	psel->gradient = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_GRADIENT,
						     SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR, tt, _("Linear gradient"));
	psel->radial = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_RADIAL,
							 SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL, tt, _("Radial gradient")),
	psel->pattern = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_PATTERN,
                               SP_PAINT_SELECTOR_MODE_PATTERN, tt, _("Pattern"));
	psel->unset = sp_paint_selector_style_button_add (psel, INKSCAPE_STOCK_FILL_UNSET,
                               SP_PAINT_SELECTOR_MODE_UNSET, tt, _("Unset paint (make it undefined so it can be inherited)"));

	/* Fillrule */
	{
	psel->fillrulebox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (psel->style), psel->fillrulebox, FALSE, FALSE, 0);

	GtkWidget *w;
	psel->evenodd = gtk_radio_button_new (NULL);
	gtk_button_set_relief (GTK_BUTTON (psel->evenodd), GTK_RELIEF_NONE);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(psel->evenodd), FALSE);
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
	gtk_tooltips_set_tip (tt, psel->evenodd, _("Any path self-intersections or subpaths create holes in the fill (fill-rule: evenodd)"), NULL);
	gtk_object_set_data (GTK_OBJECT (psel->evenodd), "mode", GUINT_TO_POINTER (SP_PAINT_SELECTOR_FILLRULE_EVENODD));
	w = sp_icon_new (GTK_ICON_SIZE_MENU, "fillrule_evenodd");
	gtk_container_add (GTK_CONTAINER (psel->evenodd), w);
	gtk_box_pack_start (GTK_BOX (psel->fillrulebox), psel->evenodd, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (psel->evenodd), "toggled", GTK_SIGNAL_FUNC (sp_paint_selector_fillrule_toggled), psel);

	psel->nonzero = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (psel->evenodd)));
	gtk_button_set_relief (GTK_BUTTON (psel->nonzero), GTK_RELIEF_NONE);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(psel->nonzero), FALSE);
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
	gtk_tooltips_set_tip (tt, psel->nonzero, _("Fill is solid unless a subpath is counterdirectional (fill-rule: nonzero)"), NULL);
	gtk_object_set_data (GTK_OBJECT (psel->nonzero), "mode", GUINT_TO_POINTER (SP_PAINT_SELECTOR_FILLRULE_NONZERO));
	w = sp_icon_new (GTK_ICON_SIZE_MENU, "fillrule_nonzero");
	gtk_container_add (GTK_CONTAINER (psel->nonzero), w);
	gtk_box_pack_start (GTK_BOX (psel->fillrulebox), psel->nonzero, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (psel->nonzero), "toggled", GTK_SIGNAL_FUNC (sp_paint_selector_fillrule_toggled), psel);
	}

	/* Frame */
	psel->frame = gtk_frame_new ("");
	gtk_widget_show (psel->frame);
	gtk_container_set_border_width (GTK_CONTAINER (psel->frame), 0);
	gtk_box_pack_start (GTK_BOX (psel), psel->frame, TRUE, TRUE, 0);

	/* Last used color */
	sp_color_set_rgb_float (&psel->color, 0.0, 0.0, 0.0);
	psel->alpha = 1.0;
}

static void
sp_paint_selector_destroy (GtkObject *object)
{
	SPPaintSelector *psel = SP_PAINT_SELECTOR (object);

	// clean up our long-living pattern menu
	g_object_set_data (G_OBJECT(psel),"patternmenu",NULL);

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static GtkWidget *
sp_paint_selector_style_button_add (SPPaintSelector *psel, gchar const *pixmap, SPPaintSelectorMode mode, GtkTooltips *tt, gchar const *tip)
{
	GtkWidget *b, *w;

	b = gtk_toggle_button_new ();
	gtk_tooltips_set_tip (tt, b, tip, NULL);
	gtk_widget_show (b);

	gtk_container_set_border_width (GTK_CONTAINER (b), 0);

	gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);

	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
	gtk_object_set_data (GTK_OBJECT (b), "mode", GUINT_TO_POINTER (mode));

	w = sp_icon_new (GTK_ICON_SIZE_BUTTON, pixmap);
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (b), w);

	gtk_box_pack_start (GTK_BOX (psel->style), b, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (b), "toggled", GTK_SIGNAL_FUNC (sp_paint_selector_style_button_toggled), psel);

	return b;
}

static void
sp_paint_selector_style_button_toggled (GtkToggleButton *tb, SPPaintSelector *psel)
{
	if (!psel->update && gtk_toggle_button_get_active (tb)) {
		sp_paint_selector_set_mode (psel, (SPPaintSelectorMode)GPOINTER_TO_UINT (gtk_object_get_data (GTK_OBJECT (tb), "mode")));
	}
}

static void
sp_paint_selector_fillrule_toggled (GtkToggleButton *tb, SPPaintSelector *psel)
{
	if (!psel->update && gtk_toggle_button_get_active (tb)) {
		SPPaintSelectorFillRule fr = (SPPaintSelectorFillRule)GPOINTER_TO_UINT (gtk_object_get_data (GTK_OBJECT (tb), "mode"));
		gtk_signal_emit (GTK_OBJECT (psel), psel_signals[FILLRULE_CHANGED], fr);
	}
}

void
sp_paint_selector_show_fillrule (SPPaintSelector *psel, bool is_fill)
{
	if (psel->fillrulebox) {
		if (is_fill) {
			gtk_widget_show_all (psel->fillrulebox);
		} else {
			gtk_widget_destroy (psel->fillrulebox);
			psel->fillrulebox = NULL;
		}
	}
}


GtkWidget *
sp_paint_selector_new (bool is_fill)
{
	SPPaintSelector *psel;

	psel = (SPPaintSelector*)gtk_type_new (SP_TYPE_PAINT_SELECTOR);

	sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);

     // This silliness is here because I don't know how to pass a parameter to the
     // GtkObject "constructor" (sp_paint_selector_init). Remove it when paint_selector
     // becomes a normal class.
	sp_paint_selector_show_fillrule (psel, is_fill);

	return GTK_WIDGET (psel);
}

void
sp_paint_selector_set_mode (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	if (psel->mode != mode) {
		psel->update = TRUE;
#ifdef SP_PS_VERBOSE
		g_print ("Mode change %d -> %d\n", psel->mode, mode);
#endif
		switch (mode) {
		case SP_PAINT_SELECTOR_MODE_EMPTY:
			sp_paint_selector_set_mode_empty (psel);
			break;
		case SP_PAINT_SELECTOR_MODE_MULTIPLE:
			sp_paint_selector_set_mode_multiple (psel);
			break;
		case SP_PAINT_SELECTOR_MODE_NONE:
			sp_paint_selector_set_mode_none (psel);
			break;
		case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
		case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
			sp_paint_selector_set_mode_color (psel, mode);
			break;
		case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
		case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
			sp_paint_selector_set_mode_gradient (psel, mode);
			break;
		case SP_PAINT_SELECTOR_MODE_PATTERN:
			sp_paint_selector_set_mode_pattern (psel, mode);
			break;
		case SP_PAINT_SELECTOR_MODE_UNSET:
			sp_paint_selector_set_mode_unset (psel);
			break;
		default:
			g_warning ("file %s: line %d: Unknown paint mode %d", __FILE__, __LINE__, mode);
			break;
		}
		psel->mode = mode;
		gtk_signal_emit (GTK_OBJECT (psel), psel_signals[MODE_CHANGED], psel->mode);
		psel->update = FALSE;
	}
}

void
sp_paint_selector_set_fillrule (SPPaintSelector *psel, SPPaintSelectorFillRule fillrule)
{
	if (psel->fillrulebox) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->evenodd), (fillrule == SP_PAINT_SELECTOR_FILLRULE_EVENODD));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->nonzero), (fillrule == SP_PAINT_SELECTOR_FILLRULE_NONZERO));
	}
}

void
sp_paint_selector_set_color_alpha (SPPaintSelector *psel, SPColor const *color, float alpha)
{
	g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );
	SPColorSelector *csel;
	guint32 rgba;

	if ( sp_color_get_colorspace_type (color) == SP_COLORSPACE_TYPE_CMYK )
	{
#ifdef SP_PS_VERBOSE
		g_print ("PaintSelector set CMYKA\n");
#endif
		sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_COLOR_CMYK);
	}
	else
	{
#ifdef SP_PS_VERBOSE
		g_print ("PaintSelector set RGBA\n");
#endif
		sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_COLOR_RGB);
	}

	csel = (SPColorSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "color-selector");
	rgba = sp_color_get_rgba32_falpha( &*color, alpha );
	csel->base->setColorAlpha( *color, alpha );
}

void
sp_paint_selector_set_gradient_linear (SPPaintSelector *psel, SPGradient *vector)
{
	SPGradientSelector *gsel;
#ifdef SP_PS_VERBOSE
	g_print ("PaintSelector set GRADIENT LINEAR\n");
#endif
	sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_mode (gsel, SP_GRADIENT_SELECTOR_MODE_LINEAR);
	sp_gradient_selector_set_vector (gsel, (vector) ? SP_OBJECT_DOCUMENT (vector) : NULL, vector);
}

void
sp_paint_selector_set_gradient_radial (SPPaintSelector *psel, SPGradient *vector)
{
	SPGradientSelector *gsel;
#ifdef SP_PS_VERBOSE
	g_print ("PaintSelector set GRADIENT RADIAL\n");
#endif
	sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	sp_gradient_selector_set_mode (gsel, SP_GRADIENT_SELECTOR_MODE_RADIAL);
	sp_gradient_selector_set_vector (gsel, (vector) ? SP_OBJECT_DOCUMENT (vector) : NULL, vector);
}

void
sp_paint_selector_set_gradient_properties (SPPaintSelector *psel, SPGradientUnits units, SPGradientSpread spread)
{
	SPGradientSelector *gsel;
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));
	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");
	sp_gradient_selector_set_units (gsel, units);
	sp_gradient_selector_set_spread (gsel, spread);
}

void
sp_paint_selector_get_gradient_properties (SPPaintSelector *psel, SPGradientUnits *units, SPGradientSpread *spread)
{
	SPGradientSelector *gsel;
	g_return_if_fail (SP_IS_PAINT_SELECTOR (psel));
	g_return_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			  (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL));
	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");
	if (units) *units = sp_gradient_selector_get_units (gsel);
	if (spread) *spread = sp_gradient_selector_get_spread (gsel);
}

/**
 * \post (alpha == NULL) || (*alpha in [0.0, 1.0]).
 */
void
sp_paint_selector_get_color_alpha (SPPaintSelector *psel, SPColor *color, gfloat *alpha)
{
	SPColorSelector *csel;

	csel = (SPColorSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "color-selector");

	csel->base->getColorAlpha( *color, alpha );

	g_assert( !alpha
		  || ( ( 0.0 <= *alpha )
		       && ( *alpha <= 1.0 ) ) );
}

SPGradient *
sp_paint_selector_get_gradient_vector (SPPaintSelector *psel)
{
	SPGradientSelector *gsel;

	g_return_val_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) ||
			      (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL), NULL);

	gsel = (SPGradientSelector*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");

	return sp_gradient_selector_get_vector (gsel);
}

void
sp_gradient_selector_attrs_to_gradient (SPGradient *gr, SPPaintSelector *psel)
{
	SPGradientUnits units;
	SPGradientSpread spread;
	sp_paint_selector_get_gradient_properties (psel, &units, &spread);
	sp_gradient_set_units (gr, units);
	sp_gradient_set_spread (gr, spread);
	SP_OBJECT(gr)->updateRepr();
}

static void
sp_paint_selector_clear_frame(SPPaintSelector *psel)
{
	g_return_if_fail ( psel != NULL);

	if (psel->selector) {

		/* before we destroy the frame contents, we must detach
		 * the patternmenu so that Gtk doesn't gtk_widget_destroy
		 * all the children of the menu.  (We also have a g_object_ref
		 * count set on it too so that the gtk_container_remove doesn't
		 * end up destroying it.
		 */
		GtkWidget *patterns = (GtkWidget *)g_object_get_data (G_OBJECT(psel), "patternmenu");
		if (patterns != NULL) {
			GtkWidget * parent = gtk_widget_get_parent ( GTK_WIDGET (patterns));
			if ( parent != NULL ) {
				g_assert ( GTK_IS_CONTAINER (parent) );
				gtk_container_remove ( GTK_CONTAINER (parent), patterns );
			}
		}

		gtk_widget_destroy (psel->selector);
		psel->selector = NULL;
	}
}

static void
sp_paint_selector_set_mode_empty (SPPaintSelector *psel)
{
	sp_paint_selector_set_style_buttons (psel, NULL);
	gtk_widget_set_sensitive (psel->style, FALSE);

	sp_paint_selector_clear_frame(psel);

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("No objects"));
}

static void
sp_paint_selector_set_mode_multiple (SPPaintSelector *psel)
{
	sp_paint_selector_set_style_buttons (psel, NULL);
	gtk_widget_set_sensitive (psel->style, TRUE);

	sp_paint_selector_clear_frame(psel);

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("Multiple styles"));
}

static void
sp_paint_selector_set_mode_unset (SPPaintSelector *psel)
{
    sp_paint_selector_set_style_buttons (psel, psel->unset);
    gtk_widget_set_sensitive (psel->style, TRUE);

    sp_paint_selector_clear_frame(psel);

    gtk_frame_set_label (GTK_FRAME (psel->frame), _("Paint is undefined"));
}

static void
sp_paint_selector_set_mode_none (SPPaintSelector *psel)
{
	sp_paint_selector_set_style_buttons (psel, psel->none);
	gtk_widget_set_sensitive (psel->style, TRUE);

	sp_paint_selector_clear_frame(psel);

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("No paint"));
}

/* Color paint */

static void
sp_paint_selector_color_grabbed (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[GRABBED]);
}

static void
sp_paint_selector_color_dragged (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[DRAGGED]);
}

static void
sp_paint_selector_color_released (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[RELEASED]);
}

static void
sp_paint_selector_color_changed (SPColorSelector *csel, SPPaintSelector *psel)
{
	csel->base->getColorAlpha( psel->color, &psel->alpha );

	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[CHANGED]);
}

static void
sp_paint_selector_set_mode_color (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	GtkWidget *csel;

	sp_paint_selector_set_style_buttons (psel, psel->solid);
	gtk_widget_set_sensitive (psel->style, TRUE);

	if ((psel->mode == SP_PAINT_SELECTOR_MODE_COLOR_RGB) || (psel->mode == SP_PAINT_SELECTOR_MODE_COLOR_CMYK)) {
		/* Already have color selector */
		csel = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel->selector), "color-selector");
	} else {

		sp_paint_selector_clear_frame(psel);
		/* Create new color selector */
		/* Create vbox */
		GtkWidget *vb = gtk_vbox_new (FALSE, 4);
		gtk_widget_show (vb);

		/* Color selector */
		csel = sp_color_selector_new (SP_TYPE_COLOR_NOTEBOOK, SP_COLORSPACE_TYPE_NONE);
		gtk_widget_show (csel);
		gtk_object_set_data (GTK_OBJECT (vb), "color-selector", csel);
		gtk_box_pack_start (GTK_BOX (vb), csel, TRUE, TRUE, 0);
		gtk_signal_connect (GTK_OBJECT (csel), "grabbed", GTK_SIGNAL_FUNC (sp_paint_selector_color_grabbed), psel);
		gtk_signal_connect (GTK_OBJECT (csel), "dragged", GTK_SIGNAL_FUNC (sp_paint_selector_color_dragged), psel);
		gtk_signal_connect (GTK_OBJECT (csel), "released", GTK_SIGNAL_FUNC (sp_paint_selector_color_released), psel);
		gtk_signal_connect (GTK_OBJECT (csel), "changed", GTK_SIGNAL_FUNC (sp_paint_selector_color_changed), psel);
		/* Pack everything to frame */
		gtk_container_add (GTK_CONTAINER (psel->frame), vb);
		psel->selector = vb;

		/* Set color */
		SP_COLOR_SELECTOR( csel )->base->setColorAlpha( psel->color, psel->alpha );

	}

	gtk_frame_set_label (GTK_FRAME (psel->frame), _("Flat color"));
#ifdef SP_PS_VERBOSE
	g_print ("Color req\n");
#endif
}

/* Gradient */

static void
sp_paint_selector_gradient_grabbed (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[GRABBED]);
}

static void
sp_paint_selector_gradient_dragged (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[DRAGGED]);
}

static void
sp_paint_selector_gradient_released (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[RELEASED]);
}

static void
sp_paint_selector_gradient_changed (SPColorSelector *csel, SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[CHANGED]);
}

static void
sp_paint_selector_set_mode_gradient (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	GtkWidget *gsel;

	/* fixme: We do not need function-wide gsel at all */

	if (mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) {
		sp_paint_selector_set_style_buttons (psel, psel->gradient);
	} else {
		sp_paint_selector_set_style_buttons (psel, psel->radial);
	}
	gtk_widget_set_sensitive (psel->style, TRUE);

	if ((psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) || (psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL)) {
		/* Already have gradient selector */
		gsel = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel->selector), "gradient-selector");
	} else {
		sp_paint_selector_clear_frame(psel);
		/* Create new gradient selector */
		gsel = sp_gradient_selector_new ();
		gtk_widget_show (gsel);
		gtk_signal_connect (GTK_OBJECT (gsel), "grabbed", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_grabbed), psel);
		gtk_signal_connect (GTK_OBJECT (gsel), "dragged", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_dragged), psel);
		gtk_signal_connect (GTK_OBJECT (gsel), "released", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_released), psel);
		gtk_signal_connect (GTK_OBJECT (gsel), "changed", GTK_SIGNAL_FUNC (sp_paint_selector_gradient_changed), psel);
		/* Pack everything to frame */
		gtk_container_add (GTK_CONTAINER (psel->frame), gsel);
		psel->selector = gsel;
		gtk_object_set_data (GTK_OBJECT (psel->selector), "gradient-selector", gsel);
	}

	/* Actually we have to set optiomenu history here */
	if (mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR) {
		sp_gradient_selector_set_mode (SP_GRADIENT_SELECTOR (gsel), SP_GRADIENT_SELECTOR_MODE_LINEAR);
		gtk_frame_set_label (GTK_FRAME (psel->frame), _("Linear gradient"));
	} else {
		sp_gradient_selector_set_mode (SP_GRADIENT_SELECTOR (gsel), SP_GRADIENT_SELECTOR_MODE_RADIAL);
		gtk_frame_set_label (GTK_FRAME (psel->frame), _("Radial gradient"));
	}
#ifdef SP_PS_VERBOSE
	g_print ("Gradient req\n");
#endif
}

static void
sp_paint_selector_set_style_buttons (SPPaintSelector *psel, GtkWidget *active)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->none), (active == psel->none));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->solid), (active == psel->solid));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->gradient), (active == psel->gradient));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->radial), (active == psel->radial));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->pattern), (active == psel->pattern));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psel->unset), (active == psel->unset));
}

static void
sp_psel_pattern_destroy (GtkWidget *widget,  SPPaintSelector *psel)
{
	// drop our reference to the pattern menu widget
	g_object_unref ( G_OBJECT (widget) );
}

static void
sp_psel_pattern_change (GtkWidget *widget,  SPPaintSelector *psel)
{
	gtk_signal_emit (GTK_OBJECT (psel), psel_signals[CHANGED]);
}

static GtkWidget*
ink_pattern_menu (GtkWidget *mnu)
{
	/* Create new menu widget */
	GtkWidget *m = gtk_menu_new ();
	gtk_widget_show (m);

	/* Pick up all patterns  */
	SPDocument *doc = SP_ACTIVE_DOCUMENT;
	GSList *pl = NULL;
	GSList const *patterns = sp_document_get_resource_list (doc, "pattern");
	for (GSList *l = (GSList *) patterns; l != NULL; l = l->next) {
		if (SP_PATTERN (l->data) == pattern_getroot (SP_PATTERN (l->data))) {  // only if this is a root pattern
			pl = g_slist_prepend (pl, l->data);
		}
	}

	pl = g_slist_reverse (pl);

	if (!doc) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No document selected"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (mnu, FALSE);
	} else if (!pl) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No patterns in document"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (mnu, FALSE);
	} else {
		for (; pl != NULL; pl = pl->next){
			if (SP_IS_PATTERN(pl->data)){
				SPPattern *pat = SP_PATTERN (pl->data);
				GtkWidget *i = gtk_menu_item_new ();
				gtk_widget_show (i);
				g_object_set_data (G_OBJECT (i), "pattern", pat);
				GtkWidget *hb = gtk_hbox_new (FALSE, 4);
				gtk_widget_show (hb);
				Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) pl->data);
				GtkWidget *l = gtk_label_new (repr->attribute("id"));
				gtk_widget_show (l);
				gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
				gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
				gtk_widget_show (hb);
				gtk_container_add (GTK_CONTAINER (i), hb);
				gtk_menu_append (GTK_MENU (m), i);
			}
		}

		gtk_widget_set_sensitive (mnu, TRUE);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (mnu), m);

	/* Set history */
	//gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);

	g_slist_free (pl);
	return mnu;
}


/*update pattern list*/
void
sp_update_pattern_list ( SPPaintSelector *psel,  SPPattern *pattern)
{
	if (psel->update) return;
	GtkWidget *mnu = (GtkWidget *)g_object_get_data (G_OBJECT(psel), "patternmenu");
	g_assert ( mnu != NULL );

	/* Clear existing menu if any */
	gtk_option_menu_remove_menu (GTK_OPTION_MENU (mnu));

	ink_pattern_menu (mnu);

	/* Set history */

	if (pattern && !gtk_object_get_data(GTK_OBJECT(mnu), "update")) {

		gtk_object_set_data(GTK_OBJECT(mnu), "update", GINT_TO_POINTER(TRUE));

		gchar *patname = (gchar *) SP_OBJECT_REPR(pattern)->attribute("id");

		GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (GTK_OPTION_MENU(mnu)));
		GList *kids = GTK_MENU_SHELL(m)->children;

		int patpos = 0;
		int i = 0;

		for (; kids != NULL; kids = kids->next) {
			gchar *men_pat = (gchar *) SP_OBJECT_REPR(g_object_get_data(G_OBJECT(kids->data), "pattern"))->attribute("id");
			if ( strcmp(men_pat, patname) == 0 ) {
				patpos = i;
			}
			i++;
		}

		gtk_option_menu_set_history(GTK_OPTION_MENU (mnu), patpos);
		gtk_object_set_data(GTK_OBJECT (mnu), "update", GINT_TO_POINTER (FALSE));
	}
	//gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
}

static void
sp_paint_selector_set_mode_pattern (SPPaintSelector *psel, SPPaintSelectorMode mode)
{
	if (mode == SP_PAINT_SELECTOR_MODE_PATTERN)
		sp_paint_selector_set_style_buttons (psel, psel->pattern);

	gtk_widget_set_sensitive (psel->style, TRUE);

	GtkWidget *tbl=NULL;

	if (psel->mode == SP_PAINT_SELECTOR_MODE_PATTERN){
		/* Already have pattern menu */
		tbl = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (psel->selector), "pattern-selector");
	} else {
		sp_paint_selector_clear_frame(psel);

		/* Create vbox */
		tbl = gtk_vbox_new (FALSE, 4);
		gtk_widget_show (tbl);

		{
		GtkWidget *hb = gtk_hbox_new (FALSE, 1);

		GtkWidget *mnu = gtk_option_menu_new ();
		ink_pattern_menu (mnu);
		gtk_signal_connect (GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_psel_pattern_change), psel);
		gtk_signal_connect (GTK_OBJECT (mnu), "destroy", GTK_SIGNAL_FUNC (sp_psel_pattern_destroy), psel);
		gtk_object_set_data (GTK_OBJECT (psel), "patternmenu", mnu);
		g_object_ref( G_OBJECT (mnu));

		gtk_container_add (GTK_CONTAINER (hb), mnu);
		gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
		}

		{
		GtkWidget *hb = gtk_hbox_new (FALSE, 0);
		GtkWidget *l = gtk_label_new (NULL);
		gtk_label_set_markup(GTK_LABEL(l), _("Use <b>Edit &gt; Object(s) to Pattern</b> to create a new pattern from selection."));
		gtk_label_set_line_wrap (GTK_LABEL(l), true);
		gtk_widget_set_size_request (l, 180, -1);
		gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
		gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
		}

		gtk_widget_show_all (tbl);

		gtk_container_add (GTK_CONTAINER (psel->frame), tbl);
		psel->selector = tbl;
		gtk_object_set_data (GTK_OBJECT (psel->selector), "pattern-selector", tbl);

		gtk_frame_set_label (GTK_FRAME (psel->frame), _("Pattern fill"));
	}
#ifdef SP_PS_VERBOSE
	g_print ("Pattern req\n");
#endif
}

SPPattern *
sp_paint_selector_get_pattern (SPPaintSelector *psel)
{
	SPPattern *pat;

	g_return_val_if_fail ((psel->mode == SP_PAINT_SELECTOR_MODE_PATTERN) , NULL);

	GtkWidget *patmnu = (GtkWidget *) g_object_get_data (G_OBJECT(psel), "patternmenu");
	/* no pattern menu if we were just selected */
	if ( patmnu == NULL ) return NULL;

	GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (GTK_OPTION_MENU(patmnu)));

	pat = pattern_getroot (SP_PATTERN(g_object_get_data (G_OBJECT(gtk_menu_get_active (m)), "pattern")));

	return pat;
}

void
sp_paint_selector_set_flat_color (SPPaintSelector *psel, SPDesktop *desktop, gchar const *color_property, gchar const *opacity_property)
{
    SPCSSAttr *css = sp_repr_css_attr_new ();

    SPColor color;
    gfloat alpha;
    sp_paint_selector_get_color_alpha (psel, &color, &alpha);
    guint32 rgba = sp_color_get_rgba32_falpha (&color, alpha);

    gchar b[64];
    sp_svg_write_color (b, 64, rgba);

    sp_repr_css_set_property (css, color_property, b);
    Inkscape::CSSOStringStream osalpha;
    osalpha << alpha;
    sp_repr_css_set_property (css, opacity_property, osalpha.str().c_str());

    sp_desktop_set_style (desktop, css);

    sp_repr_css_attr_unref (css);
}

SPPaintSelectorMode
sp_style_determine_paint_selector_mode (SPStyle *style, bool isfill)
{
    unsigned set = isfill? style->fill.set : style->stroke.set;
    if (!set)
        return SP_PAINT_SELECTOR_MODE_UNSET;

    unsigned type = isfill? style->fill.type : style->stroke.type;
    switch (type) {

        case SP_PAINT_TYPE_NONE:
        {
            return SP_PAINT_SELECTOR_MODE_NONE;
        }

        case SP_PAINT_TYPE_COLOR:
        {
            return SP_PAINT_SELECTOR_MODE_COLOR_RGB; // so far only rgb can be read from svg
        }

        case SP_PAINT_TYPE_PAINTSERVER:
        {
            SPPaintServer *server = isfill? SP_STYLE_FILL_SERVER (style) : SP_STYLE_STROKE_SERVER (style);

            if (SP_IS_LINEARGRADIENT (server)) {
                return SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR;
            } else if (SP_IS_RADIALGRADIENT (server)) {
                return SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL;
            } else if (SP_IS_PATTERN (server)) {
                return SP_PAINT_SELECTOR_MODE_PATTERN;
            }

            g_warning ( "file %s: line %d: Unknown paintserver",
                        __FILE__, __LINE__ );
            return SP_PAINT_SELECTOR_MODE_NONE;
        }

        default:
            g_warning ( "file %s: line %d: Unknown paint type %d",
                        __FILE__, __LINE__, type );
            break;
    }

    return SP_PAINT_SELECTOR_MODE_NONE;
}

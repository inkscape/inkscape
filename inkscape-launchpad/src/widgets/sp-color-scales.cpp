/*
 *   bulia byak <buliabyak@users.sf.net>
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include "ui/dialog-events.h"
#include "sp-color-scales.h"
#include "sp-color-slider.h"
#include "svg/svg-icc-color.h"

#define CSC_CHANNEL_R (1 << 0)
#define CSC_CHANNEL_G (1 << 1)
#define CSC_CHANNEL_B (1 << 2)
#define CSC_CHANNEL_A (1 << 3)
#define CSC_CHANNEL_H (1 << 0)
#define CSC_CHANNEL_S (1 << 1)
#define CSC_CHANNEL_V (1 << 2)
#define CSC_CHANNEL_C (1 << 0)
#define CSC_CHANNEL_M (1 << 1)
#define CSC_CHANNEL_Y (1 << 2)
#define CSC_CHANNEL_K (1 << 3)
#define CSC_CHANNEL_CMYKA (1 << 4)

#define CSC_CHANNELS_ALL 0


G_BEGIN_DECLS

static void sp_color_scales_dispose(GObject *object);

static void sp_color_scales_show_all (GtkWidget *widget);
static void sp_color_scales_hide(GtkWidget *widget);

static const gchar *sp_color_scales_hue_map (void);

G_END_DECLS

#define XPAD 4
#define YPAD 1

#define noDUMP_CHANGE_INFO 1

G_DEFINE_TYPE(SPColorScales, sp_color_scales, SP_TYPE_COLOR_SELECTOR);

static void
sp_color_scales_class_init (SPColorScalesClass *klass)
{
	static const gchar* nameset[] = {N_("RGB"), N_("HSL"), N_("CMYK"), 0};
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	SPColorSelectorClass *selector_class = SP_COLOR_SELECTOR_CLASS (klass);

	selector_class->name = nameset;
	selector_class->submode_count = 3;

	object_class->dispose = sp_color_scales_dispose;

	widget_class->show_all = sp_color_scales_show_all;
	widget_class->hide = sp_color_scales_hide;
}

ColorScales::ColorScales( SPColorSelector* csel )
    : ColorSelector( csel ),
      _mode( SP_COLOR_SCALES_MODE_NONE ),
      _rangeLimit( 255.0 ),
      _updating( FALSE ),
      _dragging( FALSE )
{
    for (gint i = 0; i < 5; i++) {
        _l[i] = 0;
        _a[i] = 0;
        _s[i] = 0;
        _b[i] = 0;
    }
}

ColorScales::~ColorScales()
{
    for (gint i = 0; i < 5; i++) {
        _l[i] = 0;
        _a[i] = 0;
        _s[i] = 0;
        _b[i] = 0;
    }
}

void sp_color_scales_init (SPColorScales *cs)
{
    SP_COLOR_SELECTOR(cs)->base = new ColorScales( SP_COLOR_SELECTOR(cs) );

    if ( SP_COLOR_SELECTOR(cs)->base )
    {
        SP_COLOR_SELECTOR(cs)->base->init();
    }
}

void ColorScales::init()
{
	gint i;

	_updating = FALSE;
	_dragging = FALSE;

#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *t = gtk_grid_new();
#else
	GtkWidget *t = gtk_table_new (5, 3, FALSE);
#endif
	gtk_widget_show (t);
	gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 4);

	/* Create components */
	for (i = 0; i < static_cast< gint > (G_N_ELEMENTS(_a)) ; i++) {
		/* Label */
		_l[i] = gtk_label_new("");
		gtk_misc_set_alignment (GTK_MISC (_l[i]), 1.0, 0.5);
		gtk_widget_show (_l[i]);

#if GTK_CHECK_VERSION(3,0,0)
  #if GTK_CHECK_VERSION(3,12,0)
		gtk_widget_set_margin_start(_l[i], XPAD);
		gtk_widget_set_margin_end(_l[i], XPAD);
  #else
		gtk_widget_set_margin_left(_l[i], XPAD);
		gtk_widget_set_margin_right(_l[i], XPAD);
  #endif
		gtk_widget_set_margin_top(_l[i], YPAD);
		gtk_widget_set_margin_bottom(_l[i], YPAD);
		gtk_grid_attach(GTK_GRID(t), _l[i], 0, i, 1, 1);
#else
		gtk_table_attach (GTK_TABLE (t), _l[i], 0, 1, i, i + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#endif

		/* Adjustment */
		_a[i] = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, _rangeLimit, 1.0, 10.0, 10.0));
		/* Slider */
		_s[i] = sp_color_slider_new (_a[i]);
		gtk_widget_show (_s[i]);

#if GTK_CHECK_VERSION(3,0,0)
  #if GTK_CHECK_VERSION(3,12,0)
		gtk_widget_set_margin_start(_s[i], XPAD);
		gtk_widget_set_margin_end(_s[i], XPAD);
  #else
		gtk_widget_set_margin_left(_s[i], XPAD);
		gtk_widget_set_margin_right(_s[i], XPAD);
  #endif
		gtk_widget_set_margin_top(_s[i], YPAD);
		gtk_widget_set_margin_bottom(_s[i], YPAD);
		gtk_widget_set_hexpand(_s[i], TRUE);
		gtk_grid_attach(GTK_GRID(t), _s[i], 1, i, 1, 1);
#else
		gtk_table_attach (GTK_TABLE (t), _s[i], 1, 2, i, i + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, XPAD, YPAD);
#endif

		/* Spinbutton */
		_b[i] = gtk_spin_button_new (GTK_ADJUSTMENT (_a[i]), 1.0, 0);
		sp_dialog_defocus_on_enter (_b[i]);
		gtk_label_set_mnemonic_widget (GTK_LABEL(_l[i]), _b[i]);
		gtk_widget_show (_b[i]);

#if GTK_CHECK_VERSION(3,0,0)
  #if GTK_CHECK_VERSION(3,12,0)
		gtk_widget_set_margin_start(_b[i], XPAD);
		gtk_widget_set_margin_end(_b[i], XPAD);
  #else
		gtk_widget_set_margin_left(_b[i], XPAD);
		gtk_widget_set_margin_right(_b[i], XPAD);
  #endif
		gtk_widget_set_margin_top(_b[i], YPAD);
		gtk_widget_set_margin_bottom(_b[i], YPAD);
		gtk_widget_set_halign(_b[i], GTK_ALIGN_CENTER);
		gtk_widget_set_valign(_b[i], GTK_ALIGN_CENTER);
		gtk_grid_attach(GTK_GRID(t), _b[i], 2, i, 1, 1);
#else
		gtk_table_attach (GTK_TABLE (t), _b[i], 2, 3, i, i + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);
#endif

		/* Attach channel value to adjustment */
		g_object_set_data (G_OBJECT (_a[i]), "channel", GINT_TO_POINTER (i));
		/* Signals */
		g_signal_connect (G_OBJECT (_a[i]), "value_changed",
					G_CALLBACK (_adjustmentAnyChanged), _csel);
		g_signal_connect (G_OBJECT (_s[i]), "grabbed",
					G_CALLBACK (_sliderAnyGrabbed), _csel);
		g_signal_connect (G_OBJECT (_s[i]), "released",
					G_CALLBACK (_sliderAnyReleased), _csel);
		g_signal_connect (G_OBJECT (_s[i]), "changed",
					G_CALLBACK (_sliderAnyChanged), _csel);
	}

	/* Initial mode is none, so it works */
	setMode(SP_COLOR_SCALES_MODE_RGB);
}

static void sp_color_scales_dispose(GObject *object)
{
	if (G_OBJECT_CLASS(sp_color_scales_parent_class)->dispose)
            G_OBJECT_CLASS(sp_color_scales_parent_class)->dispose(object);
}

static void
sp_color_scales_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void sp_color_scales_hide(GtkWidget *widget)
{
	gtk_widget_hide(widget);
}

GtkWidget *sp_color_scales_new()
{
	SPColorScales *csel = SP_COLOR_SCALES(g_object_new (SP_TYPE_COLOR_SCALES, NULL));

	return GTK_WIDGET (csel);
}

void ColorScales::_recalcColor( gboolean changing )
{
    if ( changing )
    {
        SPColor color;
        gfloat alpha = 1.0;
        gfloat c[5];

        switch (_mode) {
        case SP_COLOR_SCALES_MODE_RGB:
        case SP_COLOR_SCALES_MODE_HSV:
            _getRgbaFloatv(c);
            color.set( c[0], c[1], c[2] );
            alpha = c[3];
            break;
        case SP_COLOR_SCALES_MODE_CMYK:
        {
            _getCmykaFloatv( c );

            float rgb[3];
            sp_color_cmyk_to_rgb_floatv( rgb, c[0], c[1], c[2], c[3] );
            color.set( rgb[0], rgb[1], rgb[2] );
            alpha = c[4];
            break;
        }
        default:
            g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, _mode);
            break;
        }

        /* Preserve ICC */
        color.icc = _color.icc ? new SVGICCColor(*_color.icc) : 0;

        _updateInternals( color, alpha, _dragging );
    }
    else
    {
        _updateInternals( _color, _alpha, _dragging );
    }
}

/* Helpers for setting color value */
gfloat ColorScales::getScaled( const GtkAdjustment *a )
{
    gfloat val = gtk_adjustment_get_value (const_cast<GtkAdjustment*>(a)) 
	    / gtk_adjustment_get_upper (const_cast<GtkAdjustment*>(a));
    return val;
}

void ColorScales::setScaled( GtkAdjustment *a, gfloat v )
{
    gfloat val = v * gtk_adjustment_get_upper (a);
    gtk_adjustment_set_value( a, val );
}

void ColorScales::_setRangeLimit( gdouble upper )
{
    _rangeLimit = upper;
    for ( gint i = 0; i < static_cast<gint>(G_N_ELEMENTS(_a)); i++ ) {
        gtk_adjustment_set_upper (_a[i], upper);
        gtk_adjustment_changed( _a[i] );
    }
}

void ColorScales::_colorChanged()
{
#ifdef DUMP_CHANGE_INFO
    g_message("ColorScales::_colorChanged( this=%p, %f, %f, %f,   %f)", this, _color.v.c[0], _color.v.c[1], _color.v.c[2], _alpha );
#endif
    gfloat tmp[3];
    gfloat c[5] = {0.0, 0.0, 0.0, 0.0};

    switch (_mode) {
    case SP_COLOR_SCALES_MODE_RGB:
        sp_color_get_rgb_floatv( &_color, c );
        c[3] = _alpha;
        c[4] = 0.0;
        break;
    case SP_COLOR_SCALES_MODE_HSV:
        sp_color_get_rgb_floatv( &_color, tmp );
        sp_color_rgb_to_hsl_floatv (c, tmp[0], tmp[1], tmp[2]);
        c[3] = _alpha;
        c[4] = 0.0;
        break;
    case SP_COLOR_SCALES_MODE_CMYK:
        sp_color_get_cmyk_floatv( &_color, c );
        c[4] = _alpha;
        break;
    default:
        g_warning ("file %s: line %d: Illegal color selector mode %d", __FILE__, __LINE__, _mode);
        break;
    }

    _updating = TRUE;
    setScaled( _a[0], c[0] );
    setScaled( _a[1], c[1] );
    setScaled( _a[2], c[2] );
    setScaled( _a[3], c[3] );
    setScaled( _a[4], c[4] );
    _updateSliders( CSC_CHANNELS_ALL );
    _updating = FALSE;
}

void ColorScales::_getRgbaFloatv( gfloat *rgba )
{
	g_return_if_fail (rgba != NULL);

	switch (_mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		rgba[0] = getScaled(_a[0]);
		rgba[1] = getScaled(_a[1]);
		rgba[2] = getScaled(_a[2]);
		rgba[3] = getScaled(_a[3]);
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		sp_color_hsl_to_rgb_floatv (rgba, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]));
		rgba[3] = getScaled(_a[3]);
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		sp_color_cmyk_to_rgb_floatv (rgba, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), getScaled(_a[3]));
		rgba[3] = getScaled(_a[4]);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

void ColorScales::_getCmykaFloatv( gfloat *cmyka )
{
	gfloat rgb[3];

	g_return_if_fail (cmyka != NULL);

	switch (_mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		sp_color_rgb_to_cmyk_floatv (cmyka, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]));
		cmyka[4] = getScaled(_a[3]);
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		sp_color_hsl_to_rgb_floatv (rgb, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]));
		sp_color_rgb_to_cmyk_floatv (cmyka, rgb[0], rgb[1], rgb[2]);
		cmyka[4] = getScaled(_a[3]);
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		cmyka[0] = getScaled(_a[0]);
		cmyka[1] = getScaled(_a[1]);
		cmyka[2] = getScaled(_a[2]);
		cmyka[3] = getScaled(_a[3]);
		cmyka[4] = getScaled(_a[4]);
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

guint32 ColorScales::_getRgba32()
{
	gfloat c[4];
	guint32 rgba;

	_getRgbaFloatv(c);

	rgba = SP_RGBA32_F_COMPOSE (c[0], c[1], c[2], c[3]);

	return rgba;
}

void ColorScales::setMode(SPColorScalesMode mode)
{
	gfloat rgba[4];
	gfloat c[4];

	if (_mode == mode) return;

	if ((_mode == SP_COLOR_SCALES_MODE_RGB) ||
		(_mode == SP_COLOR_SCALES_MODE_HSV) ||
		(_mode == SP_COLOR_SCALES_MODE_CMYK)) {
		_getRgbaFloatv(rgba);
	} else {
		rgba[0] = rgba[1] = rgba[2] = rgba[3] = 1.0;
	}

	_mode = mode;

	switch (mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		_setRangeLimit(255.0);
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[0]), _("_R:"));
		gtk_widget_set_tooltip_text (_s[0], _("Red"));
		gtk_widget_set_tooltip_text (_b[0], _("Red"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[1]), _("_G:"));
		gtk_widget_set_tooltip_text (_s[1], _("Green"));
		gtk_widget_set_tooltip_text (_b[1], _("Green"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[2]), _("_B:"));
		gtk_widget_set_tooltip_text (_s[2], _("Blue"));
		gtk_widget_set_tooltip_text (_b[2], _("Blue"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[3]), _("_A:"));
		gtk_widget_set_tooltip_text (_s[3], _("Alpha (opacity)"));
		gtk_widget_set_tooltip_text (_b[3], _("Alpha (opacity)"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (_s[0]), NULL);
		gtk_widget_hide (_l[4]);
		gtk_widget_hide (_s[4]);
		gtk_widget_hide (_b[4]);
		_updating = TRUE;
		setScaled( _a[0], rgba[0] );
		setScaled( _a[1], rgba[1] );
		setScaled( _a[2], rgba[2] );
		setScaled( _a[3], rgba[3] );
		_updating = FALSE;
		_updateSliders( CSC_CHANNELS_ALL );
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		_setRangeLimit(255.0);
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[0]), _("_H:"));
		gtk_widget_set_tooltip_text (_s[0], _("Hue"));
		gtk_widget_set_tooltip_text (_b[0], _("Hue"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[1]), _("_S:"));
		gtk_widget_set_tooltip_text (_s[1], _("Saturation"));
		gtk_widget_set_tooltip_text (_b[1], _("Saturation"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[2]), _("_L:"));
		gtk_widget_set_tooltip_text (_s[2], _("Lightness"));
		gtk_widget_set_tooltip_text (_b[2], _("Lightness"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[3]), _("_A:"));
		gtk_widget_set_tooltip_text (_s[3], _("Alpha (opacity)"));
		gtk_widget_set_tooltip_text (_b[3], _("Alpha (opacity)"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (_s[0]), (guchar *)(sp_color_scales_hue_map()));
		gtk_widget_hide (_l[4]);
		gtk_widget_hide (_s[4]);
		gtk_widget_hide (_b[4]);
		_updating = TRUE;
		c[0] = 0.0;
		sp_color_rgb_to_hsl_floatv (c, rgba[0], rgba[1], rgba[2]);
		setScaled( _a[0], c[0] );
		setScaled( _a[1], c[1] );
		setScaled( _a[2], c[2] );
		setScaled( _a[3], rgba[3] );
		_updating = FALSE;
		_updateSliders( CSC_CHANNELS_ALL );
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		_setRangeLimit(100.0);
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[0]), _("_C:"));
		gtk_widget_set_tooltip_text (_s[0], _("Cyan"));
		gtk_widget_set_tooltip_text (_b[0], _("Cyan"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[1]), _("_M:"));
		gtk_widget_set_tooltip_text (_s[1], _("Magenta"));
		gtk_widget_set_tooltip_text (_b[1], _("Magenta"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[2]), _("_Y:"));
		gtk_widget_set_tooltip_text (_s[2], _("Yellow"));
		gtk_widget_set_tooltip_text (_b[2], _("Yellow"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[3]), _("_K:"));
		gtk_widget_set_tooltip_text (_s[3], _("Black"));
		gtk_widget_set_tooltip_text (_b[3], _("Black"));
		gtk_label_set_markup_with_mnemonic (GTK_LABEL (_l[4]), _("_A:"));
		gtk_widget_set_tooltip_text (_s[4], _("Alpha (opacity)"));
		gtk_widget_set_tooltip_text (_b[4], _("Alpha (opacity)"));
		sp_color_slider_set_map (SP_COLOR_SLIDER (_s[0]), NULL);
		gtk_widget_show (_l[4]);
		gtk_widget_show (_s[4]);
		gtk_widget_show (_b[4]);
		_updating = TRUE;

                sp_color_rgb_to_cmyk_floatv (c, rgba[0], rgba[1], rgba[2]);
                setScaled( _a[0], c[0] );
                setScaled( _a[1], c[1] );
                setScaled( _a[2], c[2] );
                setScaled( _a[3], c[3] );

		setScaled( _a[4], rgba[3] );
		_updating = FALSE;
		_updateSliders( CSC_CHANNELS_ALL );
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}
}

SPColorScalesMode ColorScales::getMode() const
{
	return _mode;
}

void ColorScales::setSubmode( guint submode )
{
	g_return_if_fail (_csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SCALES (_csel));
	g_return_if_fail (submode < 3);

	switch ( submode )
	{
	default:
	case 0:
		setMode(SP_COLOR_SCALES_MODE_RGB);
		break;
	case 1:
		setMode(SP_COLOR_SCALES_MODE_HSV);
		break;
	case 2:
		setMode(SP_COLOR_SCALES_MODE_CMYK);
		break;
	}
}

guint ColorScales::getSubmode() const
{
	guint submode = 0;

	switch ( _mode )
	{
	case SP_COLOR_SCALES_MODE_HSV:
		submode = 1;
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		submode = 2;
		break;
	case SP_COLOR_SCALES_MODE_RGB:
	default:
		submode = 0;
	}

	return submode;
}

void ColorScales::_adjustmentAnyChanged( GtkAdjustment *adjustment, SPColorScales *cs )
{
	gint channel = GPOINTER_TO_INT (g_object_get_data(G_OBJECT (adjustment), "channel"));

	_adjustmentChanged(cs, channel);
}

void ColorScales::_sliderAnyGrabbed( SPColorSlider *slider, SPColorScales *cs )
{
    (void)slider;
    ColorScales* scales = static_cast<ColorScales*>(SP_COLOR_SELECTOR(cs)->base);
	if (!scales->_dragging) {
		scales->_dragging = TRUE;
        scales->_grabbed();
        scales->_recalcColor( FALSE );
	}
}

void ColorScales::_sliderAnyReleased( SPColorSlider *slider, SPColorScales *cs )
{
    (void)slider;
    ColorScales* scales = static_cast<ColorScales*>(SP_COLOR_SELECTOR(cs)->base);
	if (scales->_dragging) {
		scales->_dragging = FALSE;
        scales->_released();
        scales->_recalcColor( FALSE );
	}
}

void ColorScales::_sliderAnyChanged( SPColorSlider *slider, SPColorScales *cs )
{
    (void)slider;
    ColorScales* scales = static_cast<ColorScales*>(SP_COLOR_SELECTOR(cs)->base);

    scales->_recalcColor( TRUE );
}

void ColorScales::_adjustmentChanged( SPColorScales *cs, guint channel )
{
	ColorScales* scales = static_cast<ColorScales*>(SP_COLOR_SELECTOR(cs)->base);
	if (scales->_updating) return;

	scales->_updating = TRUE;

	scales->_updateSliders( (1 << channel) );

	scales->_recalcColor (TRUE);

	scales->_updating = FALSE;
}

void ColorScales::_updateSliders( guint channels )
{
	gfloat rgb0[3], rgbm[3], rgb1[3];
#ifdef SPCS_PREVIEW
	guint32 rgba;
#endif
	switch (_mode) {
	case SP_COLOR_SCALES_MODE_RGB:
		if ((channels != CSC_CHANNEL_R) && (channels != CSC_CHANNEL_A)) {
			/* Update red */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[0]),
							SP_RGBA32_F_COMPOSE (0.0, getScaled(_a[1]), getScaled(_a[2]), 1.0),
							SP_RGBA32_F_COMPOSE (0.5, getScaled(_a[1]), getScaled(_a[2]), 1.0),
							SP_RGBA32_F_COMPOSE (1.0, getScaled(_a[1]), getScaled(_a[2]), 1.0));
		}
		if ((channels != CSC_CHANNEL_G) && (channels != CSC_CHANNEL_A)) {
			/* Update green */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[1]),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), 0.0, getScaled(_a[2]), 1.0),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), 0.5, getScaled(_a[2]), 1.0),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), 1.0, getScaled(_a[2]), 1.0));
		}
		if ((channels != CSC_CHANNEL_B) && (channels != CSC_CHANNEL_A)) {
			/* Update blue */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[2]),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), getScaled(_a[1]), 0.0, 1.0),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), getScaled(_a[1]), 0.5, 1.0),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), getScaled(_a[1]), 1.0, 1.0));
		}
		if (channels != CSC_CHANNEL_A) {
			/* Update alpha */
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[3]),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 0.0),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 0.5),
							SP_RGBA32_F_COMPOSE (getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 1.0));
		}
		break;
	case SP_COLOR_SCALES_MODE_HSV:
		/* Hue is never updated */
		if ((channels != CSC_CHANNEL_S) && (channels != CSC_CHANNEL_A)) {
			/* Update saturation */
			sp_color_hsl_to_rgb_floatv (rgb0, getScaled(_a[0]), 0.0, getScaled(_a[2]));
			sp_color_hsl_to_rgb_floatv (rgbm, getScaled(_a[0]), 0.5, getScaled(_a[2]));
			sp_color_hsl_to_rgb_floatv (rgb1, getScaled(_a[0]), 1.0, getScaled(_a[2]));
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[1]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgbm[0], rgbm[1], rgbm[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_V) && (channels != CSC_CHANNEL_A)) {
			/* Update value */
			sp_color_hsl_to_rgb_floatv (rgb0, getScaled(_a[0]), getScaled(_a[1]), 0.0);
			sp_color_hsl_to_rgb_floatv (rgbm, getScaled(_a[0]), getScaled(_a[1]), 0.5);
			sp_color_hsl_to_rgb_floatv (rgb1, getScaled(_a[0]), getScaled(_a[1]), 1.0);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[2]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgbm[0], rgbm[1], rgbm[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if (channels != CSC_CHANNEL_A) {
			/* Update alpha */
			sp_color_hsl_to_rgb_floatv (rgb0, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]));
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[3]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.0),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.5),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0));
		}
		break;
	case SP_COLOR_SCALES_MODE_CMYK:
		if ((channels != CSC_CHANNEL_C) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update C */
			sp_color_cmyk_to_rgb_floatv (rgb0, 0.0, getScaled(_a[1]), getScaled(_a[2]), getScaled(_a[3]));
			sp_color_cmyk_to_rgb_floatv (rgbm, 0.5, getScaled(_a[1]), getScaled(_a[2]), getScaled(_a[3]));
			sp_color_cmyk_to_rgb_floatv (rgb1, 1.0, getScaled(_a[1]), getScaled(_a[2]), getScaled(_a[3]));
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[0]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgbm[0], rgbm[1], rgbm[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_M) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update M */
			sp_color_cmyk_to_rgb_floatv (rgb0, getScaled(_a[0]), 0.0, getScaled(_a[2]), getScaled(_a[3]));
			sp_color_cmyk_to_rgb_floatv (rgbm, getScaled(_a[0]), 0.5, getScaled(_a[2]), getScaled(_a[3]));
			sp_color_cmyk_to_rgb_floatv (rgb1, getScaled(_a[0]), 1.0, getScaled(_a[2]), getScaled(_a[3]));
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[1]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgbm[0], rgbm[1], rgbm[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_Y) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update Y */
			sp_color_cmyk_to_rgb_floatv (rgb0, getScaled(_a[0]), getScaled(_a[1]), 0.0, getScaled(_a[3]));
			sp_color_cmyk_to_rgb_floatv (rgbm, getScaled(_a[0]), getScaled(_a[1]), 0.5, getScaled(_a[3]));
			sp_color_cmyk_to_rgb_floatv (rgb1, getScaled(_a[0]), getScaled(_a[1]), 1.0, getScaled(_a[3]));
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[2]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgbm[0], rgbm[1], rgbm[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if ((channels != CSC_CHANNEL_K) && (channels != CSC_CHANNEL_CMYKA)) {
			/* Update K */
			sp_color_cmyk_to_rgb_floatv (rgb0, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 0.0);
			sp_color_cmyk_to_rgb_floatv (rgbm, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 0.5);
			sp_color_cmyk_to_rgb_floatv (rgb1, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), 1.0);
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[3]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgbm[0], rgbm[1], rgbm[2], 1.0),
							SP_RGBA32_F_COMPOSE (rgb1[0], rgb1[1], rgb1[2], 1.0));
		}
		if (channels != CSC_CHANNEL_CMYKA) {
			/* Update alpha */
			sp_color_cmyk_to_rgb_floatv (rgb0, getScaled(_a[0]), getScaled(_a[1]), getScaled(_a[2]), getScaled(_a[3]));
			sp_color_slider_set_colors (SP_COLOR_SLIDER (_s[4]),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.0),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 0.5),
							SP_RGBA32_F_COMPOSE (rgb0[0], rgb0[1], rgb0[2], 1.0));
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal color selector mode", __FILE__, __LINE__);
		break;
	}

	// Force the internal color to be updated
    if ( !_updating )
    {
        _recalcColor( TRUE );
    }

#ifdef SPCS_PREVIEW
	rgba = sp_color_scales_get_rgba32 (cs);
	sp_color_preview_set_rgba32 (SP_COLOR_PREVIEW (_p), rgba);
#endif
}

static const gchar *
sp_color_scales_hue_map (void)
{
	static gchar *map = NULL;

	if (!map) {
		gchar *p;
		gint h;
		map = g_new (gchar, 4 * 1024);
		p = map;
		for (h = 0; h < 1024; h++) {
			gfloat rgb[3];
			sp_color_hsl_to_rgb_floatv (rgb, h / 1024.0, 1.0, 0.5);
			*p++ = SP_COLOR_F_TO_U (rgb[0]);
			*p++ = SP_COLOR_F_TO_U (rgb[1]);
			*p++ = SP_COLOR_F_TO_U (rgb[2]);
			*p++ = 255;
		}
	}

	return map;
}



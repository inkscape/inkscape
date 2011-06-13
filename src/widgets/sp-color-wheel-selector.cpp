#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include "../dialogs/dialog-events.h"
#include "sp-color-wheel-selector.h"
#include "sp-color-scales.h"
#include "sp-color-icc-selector.h"
#include "../svg/svg-icc-color.h"

G_BEGIN_DECLS

static void sp_color_wheel_selector_class_init (SPColorWheelSelectorClass *klass);
static void sp_color_wheel_selector_init (SPColorWheelSelector *cs);
static void sp_color_wheel_selector_destroy (GtkObject *object);

static void sp_color_wheel_selector_show_all (GtkWidget *widget);
static void sp_color_wheel_selector_hide_all (GtkWidget *widget);


G_END_DECLS

static SPColorSelectorClass *parent_class;

#define XPAD 4
#define YPAD 1

GType
sp_color_wheel_selector_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof (SPColorWheelSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_color_wheel_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (SPColorWheelSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_color_wheel_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
                                       "SPColorWheelSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void
sp_color_wheel_selector_class_init (SPColorWheelSelectorClass *klass)
{
    static const gchar* nameset[] = {N_("Wheel"), 0};
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    SPColorSelectorClass *selector_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    selector_class = SP_COLOR_SELECTOR_CLASS (klass);

    parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

    selector_class->name = nameset;
    selector_class->submode_count = 1;

    object_class->destroy = sp_color_wheel_selector_destroy;

    widget_class->show_all = sp_color_wheel_selector_show_all;
    widget_class->hide_all = sp_color_wheel_selector_hide_all;
}

ColorWheelSelector::ColorWheelSelector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _updating( FALSE ),
      _dragging( FALSE ),
      _adj(0),
      _wheel(0),
      _sbtn(0),
      _label(0)
{
}

ColorWheelSelector::~ColorWheelSelector()
{
    _adj = 0;
    _wheel = 0;
    _sbtn = 0;
    _label = 0;
}

void sp_color_wheel_selector_init (SPColorWheelSelector *cs)
{
    SP_COLOR_SELECTOR(cs)->base = new ColorWheelSelector( SP_COLOR_SELECTOR(cs) );

    if ( SP_COLOR_SELECTOR(cs)->base )
    {
        SP_COLOR_SELECTOR(cs)->base->init();
    }
}

static void resizeHSVWheel( GtkHSV *hsv, GtkAllocation *allocation )
{
    gint diam = std::min(allocation->width, allocation->height);

    // drop a little for resizing
    diam -= 4;

    GtkStyle *style = gtk_widget_get_style( GTK_WIDGET(hsv) );
    if ( style ) {
        gint thick = std::max(style->xthickness, style->ythickness);
        if (thick > 0) {
            diam -= thick * 2;
        }
    }
    gint padding = -1;
    gtk_widget_style_get( GTK_WIDGET(hsv), 
                          "focus-padding", &padding,
                          NULL );
    if (padding > 0) {
        diam -= padding * 2;
    }
     
    diam = std::max(20, diam);
    gint ring = static_cast<gint>( static_cast<gdouble>(diam) / (4.0 * 1.618) );
    gtk_hsv_set_metrics( hsv, diam, ring );
}

static void handleWheelStyleSet(GtkHSV *hsv, GtkStyle* /*previous*/, gpointer /*userData*/)
{
    GtkAllocation allocation = {0, 0, 0, 0};
    gtk_widget_get_allocation( GTK_WIDGET(hsv), &allocation );
    resizeHSVWheel( hsv, &allocation );
}

static void handleWheelAllocation(GtkHSV *hsv, GtkAllocation *allocation, gpointer /*userData*/)
{
    resizeHSVWheel( hsv, allocation );
}

void ColorWheelSelector::init()
{
    GtkWidget *t;
    gint row = 0;

    _updating = FALSE;
    _dragging = FALSE;

    t = gtk_table_new (5, 3, FALSE);
    gtk_widget_show (t);
    gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 0);

    /* Create components */
    row = 0;

    _wheel = gtk_hsv_new();
    gtk_hsv_set_metrics( GTK_HSV(_wheel), 48, 8 );
    gtk_widget_show( _wheel );
    gtk_table_attach( GTK_TABLE(t), _wheel, 0, 3, row, row + 1,  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), XPAD, YPAD);

    row++;

    /* Label */
    _label = gtk_label_new_with_mnemonic (_("_A:"));
    gtk_misc_set_alignment (GTK_MISC (_label), 1.0, 0.5);
    gtk_widget_show (_label);
    gtk_table_attach (GTK_TABLE (t), _label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);

    /* Adjustment */
    _adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 255.0, 1.0, 10.0, 10.0);

    /* Slider */
    _slider = sp_color_slider_new (_adj);
    gtk_widget_set_tooltip_text (_slider, _("Alpha (opacity)"));
    gtk_widget_show (_slider);
    gtk_table_attach (GTK_TABLE (t), _slider, 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD);

    sp_color_slider_set_colors (SP_COLOR_SLIDER (_slider),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 0.0),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 0.5),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 1.0));


    /* Spinbutton */
    _sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (_adj), 1.0, 0);
    gtk_widget_set_tooltip_text (_sbtn, _("Alpha (opacity)"));
    sp_dialog_defocus_on_enter (_sbtn);
    gtk_label_set_mnemonic_widget (GTK_LABEL(_label), _sbtn);
    gtk_widget_show (_sbtn);
    gtk_table_attach (GTK_TABLE (t), _sbtn, 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);

    /* Signals */
    g_signal_connect (G_OBJECT (_adj), "value_changed",
                        G_CALLBACK (_adjustmentChanged), _csel);

    g_signal_connect (G_OBJECT (_slider), "grabbed",
                        G_CALLBACK (_sliderGrabbed), _csel);
    g_signal_connect (G_OBJECT (_slider), "released",
                        G_CALLBACK (_sliderReleased), _csel);
    g_signal_connect (G_OBJECT (_slider), "changed",
                        G_CALLBACK (_sliderChanged), _csel);

    g_signal_connect( G_OBJECT(_wheel), "changed",
                        G_CALLBACK (_wheelChanged), _csel );


    // GTK does not automatically scale the color wheel, so we have to add that in:
    g_signal_connect( G_OBJECT (_wheel), "size-allocate",
                        G_CALLBACK (handleWheelAllocation), _csel );
    g_signal_connect( G_OBJECT (_wheel), "style-set",
                        G_CALLBACK (handleWheelStyleSet), _csel );
}

static void
sp_color_wheel_selector_destroy (GtkObject *object)
{
    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_wheel_selector_show_all (GtkWidget *widget)
{
    gtk_widget_show (widget);
}

static void
sp_color_wheel_selector_hide_all (GtkWidget *widget)
{
    gtk_widget_hide (widget);
}

GtkWidget *
sp_color_wheel_selector_new (void)
{
    SPColorWheelSelector *csel;

    csel = (SPColorWheelSelector*)gtk_type_new (SP_TYPE_COLOR_WHEEL_SELECTOR);

    return GTK_WIDGET (csel);
}

/* Helpers for setting color value */

static void preserve_icc(SPColor *color, SPColorWheelSelector *cs){
    ColorSelector* selector = (ColorSelector*)(SP_COLOR_SELECTOR(cs)->base);
    color->icc = selector->getColor().icc ? new SVGICCColor(*selector->getColor().icc) : 0;
}

void ColorWheelSelector::_colorChanged()
{
#ifdef DUMP_CHANGE_INFO
    g_message("ColorWheelSelector::_colorChanged( this=%p, %f, %f, %f,   %f)", this, color.v.c[0], color.v.c[1], color.v.c[2], alpha );
#endif
    _updating = TRUE;
    {
        gdouble h = 0;
        gdouble s = 0;
        gdouble v = 0;
        gtk_rgb_to_hsv( _color.v.c[0], _color.v.c[1], _color.v.c[2], &h, &s, &v  );
        gtk_hsv_set_color( GTK_HSV(_wheel), h, s, v );
    }

    guint32 start = _color.toRGBA32( 0x00 );
    guint32 mid = _color.toRGBA32( 0x7f );
    guint32 end = _color.toRGBA32( 0xff );

    sp_color_slider_set_colors(SP_COLOR_SLIDER(_slider), start, mid, end);

    ColorScales::setScaled(_adj, _alpha);

    _updating = FALSE;
}

void ColorWheelSelector::_adjustmentChanged( GtkAdjustment *adjustment, SPColorWheelSelector *cs )
{
// TODO check this. It looks questionable:
    // if a value is entered between 0 and 1 exclusive, normalize it to (int) 0..255  or 0..100
    if (adjustment->value > 0.0 && adjustment->value < 1.0) {
        gtk_adjustment_set_value( adjustment, floor ((adjustment->value) * adjustment->upper + 0.5) );
    }

    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    if (wheelSelector->_updating) return;

    wheelSelector->_updating = TRUE;

    preserve_icc(&wheelSelector->_color, cs);
    wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );

    wheelSelector->_updating = FALSE;
}

void ColorWheelSelector::_sliderGrabbed( SPColorSlider *slider, SPColorWheelSelector *cs )
{
    (void)slider;
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    if (!wheelSelector->_dragging) {
        wheelSelector->_dragging = TRUE;
        wheelSelector->_grabbed();

        preserve_icc(&wheelSelector->_color, cs);
        wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );
    }
}

void ColorWheelSelector::_sliderReleased( SPColorSlider *slider, SPColorWheelSelector *cs )
{
    (void)slider;
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    if (wheelSelector->_dragging) {
        wheelSelector->_dragging = FALSE;
        wheelSelector->_released();

        preserve_icc(&wheelSelector->_color, cs);
        wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );
    }
}

void ColorWheelSelector::_sliderChanged( SPColorSlider *slider, SPColorWheelSelector *cs )
{
    (void)slider;
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);

    preserve_icc(&wheelSelector->_color, cs);
    wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );
}

void ColorWheelSelector::_wheelChanged( GtkHSV *hsv, SPColorWheelSelector *cs )
{
    ColorWheelSelector* wheelSelector = static_cast<ColorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);

    gdouble h = 0;
    gdouble s = 0;
    gdouble v = 0;
    gtk_hsv_get_color( hsv, &h, &s, &v );
    
    gdouble r = 0;
    gdouble g = 0;
    gdouble b = 0;
    gtk_hsv_to_rgb(h, s, v, &r, &g, &b);

    SPColor color(r, g, b);

    guint32 start = color.toRGBA32( 0x00 );
    guint32 mid = color.toRGBA32( 0x7f );
    guint32 end = color.toRGBA32( 0xff );

    sp_color_slider_set_colors (SP_COLOR_SLIDER(wheelSelector->_slider), start, mid, end);

    preserve_icc(&color, cs);
    wheelSelector->_updateInternals( color, wheelSelector->_alpha, gtk_hsv_is_adjusting( hsv ) );
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

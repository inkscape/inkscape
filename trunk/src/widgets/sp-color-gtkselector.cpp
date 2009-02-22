#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>
#include "sp-color-gtkselector.h"


static void sp_color_gtkselector_class_init (SPColorGtkselectorClass *klass);
static void sp_color_gtkselector_init (SPColorGtkselector *csel);
static void sp_color_gtkselector_destroy (GtkObject *object);

static void sp_color_gtkselector_show_all (GtkWidget *widget);
static void sp_color_gtkselector_hide_all (GtkWidget *widget);


static SPColorSelectorClass *parent_class;

#define XPAD 4
#define YPAD 1

GType
sp_color_gtkselector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPColorGtkselectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) sp_color_gtkselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SPColorGtkselector),
			0,	  /* n_preallocs */
			(GInstanceInitFunc) sp_color_gtkselector_init,
			NULL,
		};

		type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
									   "SPColorGtkselector",
									   &info,
									   static_cast< GTypeFlags > (0) );
	}
	return type;
}

static void
sp_color_gtkselector_class_init (SPColorGtkselectorClass *klass)
{
	static const gchar* nameset[] = {N_("System"), 0};
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPColorSelectorClass *selector_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	selector_class = SP_COLOR_SELECTOR_CLASS (klass);

	parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

	selector_class->name = nameset;
	selector_class->submode_count = 1;

	object_class->destroy = sp_color_gtkselector_destroy;

	widget_class->show_all = sp_color_gtkselector_show_all;
	widget_class->hide_all = sp_color_gtkselector_hide_all;
}

void sp_color_gtkselector_init (SPColorGtkselector *csel)
{
    SP_COLOR_SELECTOR(csel)->base = new ColorGtkselector( SP_COLOR_SELECTOR(csel) );

    if ( SP_COLOR_SELECTOR(csel)->base )
    {
        SP_COLOR_SELECTOR(csel)->base->init();
    }
}

void ColorGtkselector::init()
{
	GtkWidget *gtksel;

	gtksel = gtk_color_selection_new();
	gtk_widget_show (gtksel);
	_gtkThing = GTK_COLOR_SELECTION (gtksel);
	gtk_box_pack_start (GTK_BOX (_csel), gtksel, TRUE, TRUE, 0);

	_sigId = g_signal_connect( GTK_OBJECT(gtksel), "color-changed", GTK_SIGNAL_FUNC( _gtkChanged ), _csel);
}

static void
sp_color_gtkselector_destroy (GtkObject *object)
{
	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_gtkselector_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_gtkselector_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_gtkselector_new( GType )
{
	SPColorGtkselector *csel;

	csel = (SPColorGtkselector*)gtk_type_new (SP_TYPE_COLOR_GTKSELECTOR);

	return GTK_WIDGET (csel);
}

ColorGtkselector::ColorGtkselector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _gtkThing(0)
{
}

ColorGtkselector::~ColorGtkselector()
{
}

void ColorGtkselector::_colorChanged()
{
    GdkColor gcolor;

    gcolor.pixel = 0;
    gcolor.red   = static_cast< guint16 >(_color.v.c[0] * 65535);
    gcolor.green = static_cast< guint16 >(_color.v.c[1] * 65535);
    gcolor.blue  = static_cast< guint16 >(_color.v.c[2] * 65535);

//     g_message( "*****  _colorChanged %04x %04x %04x", gcolor.red, gcolor.green, gcolor.blue );
    g_signal_handler_block( _gtkThing, _sigId );
    gtk_color_selection_set_current_alpha( _gtkThing, static_cast<guint16>(65535 * _alpha) );
    gtk_color_selection_set_current_color( _gtkThing, &gcolor );
    g_signal_handler_unblock(_gtkThing, _sigId );
}

void ColorGtkselector::_gtkChanged( GtkColorSelection *colorselection, SPColorGtkselector *gtksel )
{
    GdkColor color;
    gtk_color_selection_get_current_color (colorselection, &color);

    guint16 alpha = gtk_color_selection_get_current_alpha (colorselection);

    SPColor ourColor( (color.red / 65535.0), (color.green / 65535.0), (color.blue / 65535.0) );

//     g_message( "*****  _gtkChanged   %04x %04x %04x", color.red, color.green, color.blue );

    ColorGtkselector* gtkInst = (ColorGtkselector*)(SP_COLOR_SELECTOR(gtksel)->base);
    gtkInst->_updateInternals( ourColor, static_cast< gfloat > (alpha) / 65535.0, gtk_color_selection_is_adjusting(colorselection) );
}

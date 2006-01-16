/*
 *   bulia byak <buliabyak@users.sf.net>
*/ 

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <gtk/gtksignal.h>
#include <glibmm/i18n.h>
#include "sp-color-selector.h"

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

#define noDUMP_CHANGE_INFO
#define FOO_NAME(x) g_type_name( G_TYPE_FROM_INSTANCE(x) )

static void sp_color_selector_class_init (SPColorSelectorClass *klass);
static void sp_color_selector_init (SPColorSelector *csel);
static void sp_color_selector_destroy (GtkObject *object);

static void sp_color_selector_show_all (GtkWidget *widget);
static void sp_color_selector_hide_all (GtkWidget *widget);

static GtkVBoxClass *parent_class;
static guint csel_signals[LAST_SIGNAL] = {0};

GType
sp_color_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPColorSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) sp_color_selector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SPColorSelector),
			0,	  /* n_preallocs */
			(GInstanceInitFunc) sp_color_selector_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_VBOX,
									   "SPColorSelector",
									   &info,
									   static_cast< GTypeFlags > (0) );
	}
	return type;
}

static void
sp_color_selector_class_init (SPColorSelectorClass *klass)
{
	static const gchar* nameset[] = {N_("Unnamed"), 0};
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

	csel_signals[GRABBED] = gtk_signal_new ("grabbed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, grabbed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[DRAGGED] = gtk_signal_new ("dragged",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, dragged),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[RELEASED] = gtk_signal_new ("released",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, released),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[CHANGED] = gtk_signal_new ("changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, changed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);

	klass->name = nameset;
	klass->submode_count = 1;

	object_class->destroy = sp_color_selector_destroy;

	widget_class->show_all = sp_color_selector_show_all;
	widget_class->hide_all = sp_color_selector_hide_all;

}

void sp_color_selector_init (SPColorSelector *csel)
{
    if ( csel->base )
    {
        csel->base->init();
    }
/*   gtk_signal_connect (GTK_OBJECT (csel->rgbae), "changed", GTK_SIGNAL_FUNC (sp_color_selector_rgba_entry_changed), csel); */
}

static void
sp_color_selector_destroy (GtkObject *object)
{
	SPColorSelector *csel;

	csel = SP_COLOR_SELECTOR (object);
    if ( csel->base )
    {
        delete csel->base;
        csel->base = 0;
    }

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_selector_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_selector_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_selector_new (GType selector_type, SPColorSpaceType colorspace)
{
	SPColorSelector *csel;
	g_return_val_if_fail (g_type_is_a (selector_type, SP_TYPE_COLOR_SELECTOR), NULL);

	csel = SP_COLOR_SELECTOR (g_object_new (selector_type, NULL));

	return GTK_WIDGET (csel);
}

double ColorSelector::_epsilon = 1e-4;

void ColorSelector::setSubmode( guint submode )
{
}

guint ColorSelector::getSubmode() const
{
    guint mode = 0;
    return mode;
}

SPColorSpaceType ColorSelector::getColorspace() const
{
    SPColorSpaceType type = SP_COLORSPACE_TYPE_UNKNOWN;

    return type;
}

gboolean ColorSelector::setColorspace( SPColorSpaceType colorspace )
{
    return false;
}

ColorSelector::ColorSelector( SPColorSelector* csel )
    : _csel(csel),
      _alpha(1.0),
      _held(FALSE)
{
    sp_color_set_rgb_rgba32( &_color, 0 );

    virgin = true;
}

ColorSelector::~ColorSelector()
{
}

void ColorSelector::init()
{
    _csel->base = new ColorSelector( _csel );
}

void ColorSelector::setColor( const SPColor& color )
{
    setColorAlpha( color, _alpha );
}

SPColor ColorSelector::getColor() const
{
    return _color;
}

void ColorSelector::setAlpha( gfloat alpha )
{
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );
    setColorAlpha( _color, alpha );
}

gfloat ColorSelector::getAlpha() const
{
    return _alpha;
}

/**
Called from the outside to set the color; optionally emits signal (only when called from
downstream, e.g. the RGBA value field, but not from the rest of the program)
*/
void ColorSelector::setColorAlpha( const SPColor& color, gfloat alpha, bool emit )
{
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );

    if ( virgin || !sp_color_is_close( &color, &_color, _epsilon ) ||
         (fabs ((_alpha) - (alpha)) >= _epsilon )) {

        virgin = false;

        sp_color_copy (&_color, &color);
        _alpha = alpha;
        _colorChanged( color, alpha );

        if (emit)
		gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[CHANGED]);
    }
}

void ColorSelector::_grabbed()
{
    _held = TRUE;
#ifdef DUMP_CHANGE_INFO
    g_message ("%s:%d: About to signal %s in %s", __FILE__, __LINE__,
               "GRABBED",
               FOO_NAME(_csel));
#endif
    gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[GRABBED]);
}

void ColorSelector::_released()
{
    _held = false;
#ifdef DUMP_CHANGE_INFO
    g_message ("%s:%d: About to signal %s in %s", __FILE__, __LINE__,
               "RELEASED",
               FOO_NAME(_csel));
#endif
    gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[RELEASED]);
    gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[CHANGED]);
}

// Called from subclasses to update color and broadcast if needed
void ColorSelector::_updateInternals( const SPColor& color, gfloat alpha, gboolean held )
{
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );
    gboolean colorDifferent = ( !sp_color_is_close( &color, &_color, _epsilon )
                                || ( fabs((_alpha) - (alpha)) >= _epsilon ) );

    gboolean grabbed = held && !_held;
    gboolean released = !held && _held;

    // Store these before emmiting any signals
    _held = held;
    if ( colorDifferent )
    {
        sp_color_copy (&_color, &color);
        _alpha = alpha;
    }

    if ( grabbed )
    {
#ifdef DUMP_CHANGE_INFO
        g_message ("%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                   "GRABBED",
                   sp_color_get_rgba32_falpha(&color,alpha), FOO_NAME(_csel));
#endif
        gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[GRABBED]);
    }
    else if ( released )
    {
#ifdef DUMP_CHANGE_INFO
        g_message ("%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                   "RELEASED",
                   sp_color_get_rgba32_falpha(&color,alpha), FOO_NAME(_csel));
#endif
        gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[RELEASED]);
    }

    if ( colorDifferent || released )
    {
#ifdef DUMP_CHANGE_INFO
        g_message ("%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                   (_held ? "CHANGED" : "DRAGGED" ),
                   sp_color_get_rgba32_falpha(&color,alpha), FOO_NAME(_csel));
#endif
        gtk_signal_emit (GTK_OBJECT (_csel), csel_signals[_held ? CHANGED : DRAGGED]);
    }
}

void ColorSelector::_colorChanged( const SPColor& color, gfloat alpha )
{
}

void ColorSelector::getColorAlpha( SPColor& color, gfloat* alpha ) const
{
    gint i = 0;

    sp_color_copy (&color, &_color);
    if ( alpha )
    {
        *alpha = _alpha;
    }

    // Try to catch uninitialized value usage
    if ( color.colorspace )
    {
        i++;
    }
    if ( color.v.c[0] )
    {
        i++;
    }
    if ( color.v.c[1] )
    {
        i++;
    }
    if ( color.v.c[2] )
    {
        i++;
    }
    if ( color.v.c[3] )
    {
        i++;
    }
    if ( alpha && *alpha )
    {
        i++;
    }
}

/*
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <gtk/gtk.h>
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

static void sp_color_selector_dispose(GObject *object);

static void sp_color_selector_show_all( GtkWidget *widget );
static void sp_color_selector_hide( GtkWidget *widget );

static guint csel_signals[LAST_SIGNAL] = {0};

double ColorSelector::_epsilon = 1e-4;

#if GTK_CHECK_VERSION(3,0,0)
G_DEFINE_TYPE(SPColorSelector, sp_color_selector, GTK_TYPE_BOX);
#else
G_DEFINE_TYPE(SPColorSelector, sp_color_selector, GTK_TYPE_VBOX);
#endif

void sp_color_selector_class_init( SPColorSelectorClass *klass )
{
    static const gchar* nameset[] = {N_("Unnamed"), 0};
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class;
    widget_class = GTK_WIDGET_CLASS(klass);

    csel_signals[GRABBED] = g_signal_new( "grabbed",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPColorSelectorClass, grabbed),
					    NULL, NULL,
					    g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0 );
    csel_signals[DRAGGED] = g_signal_new( "dragged",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPColorSelectorClass, dragged),
					    NULL, NULL,
					    g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0 );
    csel_signals[RELEASED] = g_signal_new( "released",
                                             G_TYPE_FROM_CLASS(object_class),
                                             (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                             G_STRUCT_OFFSET(SPColorSelectorClass, released),
					     NULL, NULL,
					     g_cclosure_marshal_VOID__VOID,
                                             G_TYPE_NONE, 0 );
    csel_signals[CHANGED] = g_signal_new( "changed",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPColorSelectorClass, changed),
					    NULL, NULL,
					    g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0 );

    klass->name = nameset;
    klass->submode_count = 1;

    object_class->dispose = sp_color_selector_dispose;

    widget_class->show_all = sp_color_selector_show_all;
    widget_class->hide = sp_color_selector_hide;

}

void sp_color_selector_init( SPColorSelector *csel )
{
#if GTK_CHECK_VERSION(3,0,0)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(csel), GTK_ORIENTATION_VERTICAL);
#endif

    if ( csel->base )
    {
        csel->base->init();
    }
/*   g_signal_connect(G_OBJECT(csel->rgbae), "changed", G_CALLBACK(sp_color_selector_rgba_entry_changed), csel); */
}

void sp_color_selector_dispose(GObject *object)
{
    SPColorSelector *csel = SP_COLOR_SELECTOR( object );
    if ( csel->base )
        {
            delete csel->base;
            csel->base = 0;
        }

    if ((G_OBJECT_CLASS(sp_color_selector_parent_class))->dispose ) {
        (G_OBJECT_CLASS(sp_color_selector_parent_class))->dispose(object);
    }
}

void sp_color_selector_show_all( GtkWidget *widget )
{
    gtk_widget_show( widget );
}

void sp_color_selector_hide(GtkWidget *widget)
{
    gtk_widget_hide( widget );
}

GtkWidget *sp_color_selector_new( GType selector_type )
{
    g_return_val_if_fail( g_type_is_a( selector_type, SP_TYPE_COLOR_SELECTOR ), NULL );

    SPColorSelector *csel = SP_COLOR_SELECTOR( g_object_new( selector_type, NULL ) );

    return GTK_WIDGET( csel );
}

void ColorSelector::setSubmode( guint /*submode*/ )
{
}

guint ColorSelector::getSubmode() const
{
    guint mode = 0;
    return mode;
}

ColorSelector::ColorSelector( SPColorSelector* csel )
    : _csel(csel),
      _color( 0 ),
      _alpha(1.0),
      _held(FALSE),
      virgin(true)
{
    g_return_if_fail( SP_IS_COLOR_SELECTOR(_csel) );
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

#include "svg/svg-icc-color.h"

/**
Called from the outside to set the color; optionally emits signal (only when called from
downstream, e.g. the RGBA value field, but not from the rest of the program)
*/
void ColorSelector::setColorAlpha( const SPColor& color, gfloat alpha, bool emit )
{
#ifdef DUMP_CHANGE_INFO
    g_message("ColorSelector::setColorAlpha( this=%p, %f, %f, %f, %s,   %f,   %s) in %s", this, color.v.c[0], color.v.c[1], color.v.c[2], (color.icc?color.icc->colorProfile.c_str():"<null>"), alpha, (emit?"YES":"no"), FOO_NAME(_csel));
#endif
    g_return_if_fail( _csel != NULL );
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );

#ifdef DUMP_CHANGE_INFO
    g_message("---- ColorSelector::setColorAlpha    virgin:%s   !close:%s    alpha is:%s in %s",
              (virgin?"YES":"no"),
              (!color.isClose( _color, _epsilon )?"YES":"no"),
              ((fabs((_alpha) - (alpha)) >= _epsilon )?"YES":"no"),
              FOO_NAME(_csel)
              );
#endif

    if ( virgin || !color.isClose( _color, _epsilon ) ||
         (fabs((_alpha) - (alpha)) >= _epsilon )) {

        virgin = false;

        _color = color;
        _alpha = alpha;
        _colorChanged();

        if (emit) {
            g_signal_emit(G_OBJECT(_csel), csel_signals[CHANGED], 0);
        }
#ifdef DUMP_CHANGE_INFO
    } else {
        g_message("++++ ColorSelector::setColorAlpha   color:%08x  ==>  _color:%08X   isClose:%s   in %s", color.toRGBA32(alpha), _color.toRGBA32(_alpha),
                  (color.isClose( _color, _epsilon )?"YES":"no"), FOO_NAME(_csel));
#endif
    }
}

void ColorSelector::_grabbed()
{
    _held = TRUE;
#ifdef DUMP_CHANGE_INFO
    g_message("%s:%d: About to signal %s in %s", __FILE__, __LINE__,
               "GRABBED",
               FOO_NAME(_csel));
#endif
    g_signal_emit(G_OBJECT(_csel), csel_signals[GRABBED], 0);
}

void ColorSelector::_released()
{
    _held = false;
#ifdef DUMP_CHANGE_INFO
    g_message("%s:%d: About to signal %s in %s", __FILE__, __LINE__,
               "RELEASED",
               FOO_NAME(_csel));
#endif
    g_signal_emit(G_OBJECT(_csel), csel_signals[RELEASED], 0);
    g_signal_emit(G_OBJECT(_csel), csel_signals[CHANGED], 0);
}

// Called from subclasses to update color and broadcast if needed
void ColorSelector::_updateInternals( const SPColor& color, gfloat alpha, gboolean held )
{
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );
    gboolean colorDifferent = ( !color.isClose( _color, _epsilon )
                                || ( fabs((_alpha) - (alpha)) >= _epsilon ) );

    gboolean grabbed = held && !_held;
    gboolean released = !held && _held;

    // Store these before emmiting any signals
    _held = held;
    if ( colorDifferent )
    {
        _color = color;
        _alpha = alpha;
    }

    if ( grabbed )
    {
#ifdef DUMP_CHANGE_INFO
        g_message("%s:%d: About to signal %s to color %08x::%s in %s", __FILE__, __LINE__,
                   "GRABBED",
                   color.toRGBA32( alpha ), (color.icc?color.icc->colorProfile.c_str():"<null>"), FOO_NAME(_csel));
#endif
        g_signal_emit(G_OBJECT(_csel), csel_signals[GRABBED], 0);
    }
    else if ( released )
    {
#ifdef DUMP_CHANGE_INFO
        g_message("%s:%d: About to signal %s to color %08x::%s in %s", __FILE__, __LINE__,
                   "RELEASED",
                   color.toRGBA32( alpha ), (color.icc?color.icc->colorProfile.c_str():"<null>"), FOO_NAME(_csel));
#endif
        g_signal_emit(G_OBJECT(_csel), csel_signals[RELEASED], 0);
    }

    if ( colorDifferent || released )
    {
#ifdef DUMP_CHANGE_INFO
        g_message("%s:%d: About to signal %s to color %08x::%s in %s", __FILE__, __LINE__,
                   (_held ? "CHANGED" : "DRAGGED" ),
                   color.toRGBA32( alpha ), (color.icc?color.icc->colorProfile.c_str():"<null>"), FOO_NAME(_csel));
#endif
        g_signal_emit(G_OBJECT(_csel), csel_signals[_held ? DRAGGED : CHANGED], 0);
    }
}

/**
 * Called once the color actually changes. Allows subclasses to react to changes.
 */
void ColorSelector::_colorChanged()
{
}

void ColorSelector::getColorAlpha( SPColor &color, gfloat &alpha ) const
{
    gint i = 0;

    color = _color;
    alpha = _alpha;

    // Try to catch uninitialized value usage
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
    if ( alpha )
    {
        i++;
    }
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

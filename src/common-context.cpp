
#include "common-context.h"

#include <gtk/gtk.h>

#include "config.h"

#include "forward.h"
#include "message-context.h"
#include "streq.h"

#define MIN_PRESSURE      0.0
#define MAX_PRESSURE      1.0
#define DEFAULT_PRESSURE  1.0

#define DRAG_MIN 0.0
#define DRAG_DEFAULT 1.0
#define DRAG_MAX 1.0


static void sp_common_context_class_init(SPCommonContextClass *klass);
static void sp_common_context_init(SPCommonContext *erc);
static void sp_common_context_dispose(GObject *object);

static void sp_common_context_setup(SPEventContext *ec);
static void sp_common_context_set(SPEventContext *ec, gchar const *key, gchar const *value);

static gint sp_common_context_root_handler(SPEventContext *event_context, GdkEvent *event);


static SPEventContextClass *common_parent_class = 0;

GType sp_common_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCommonContextClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_common_context_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPCommonContext),
            0, // n_preallocs
            (GInstanceInitFunc)sp_common_context_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPCommonContext", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}


static void sp_common_context_class_init(SPCommonContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    common_parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_common_context_dispose;

    event_context_class->setup = sp_common_context_setup;
    event_context_class->set = sp_common_context_set;
    event_context_class->root_handler = sp_common_context_root_handler;
}

static void sp_common_context_init(SPCommonContext *ctx)
{
//     ctx->cursor_shape = cursor_eraser_xpm;
//     ctx->hot_x = 4;
//     ctx->hot_y = 4;

    ctx->accumulated = 0;
    ctx->segments = 0;
    ctx->currentcurve = 0;
    ctx->currentshape = 0;
    ctx->npoints = 0;
    ctx->cal1 = 0;
    ctx->cal2 = 0;
    ctx->repr = 0;

    /* Common values */
    ctx->cur = Geom::Point(0,0);
    ctx->last = Geom::Point(0,0);
    ctx->vel = Geom::Point(0,0);
    ctx->vel_max = 0;
    ctx->acc = Geom::Point(0,0);
    ctx->ang = Geom::Point(0,0);
    ctx->del = Geom::Point(0,0);

    /* attributes */
    ctx->dragging = FALSE;

    ctx->mass = 0.3;
    ctx->drag = DRAG_DEFAULT;
    ctx->angle = 30.0;
    ctx->width = 0.2;
    ctx->pressure = DEFAULT_PRESSURE;

    ctx->vel_thin = 0.1;
    ctx->flatness = 0.9;
    ctx->cap_rounding = 0.0;

    ctx->abs_width = false;
}

static void sp_common_context_dispose(GObject *object)
{
    SPCommonContext *ctx = SP_COMMON_CONTEXT(object);

    if (ctx->accumulated) {
        ctx->accumulated = ctx->accumulated->unref();
        ctx->accumulated = 0;
    }

    while (ctx->segments) {
        gtk_object_destroy(GTK_OBJECT(ctx->segments->data));
        ctx->segments = g_slist_remove(ctx->segments, ctx->segments->data);
    }

    if (ctx->currentcurve) {
        ctx->currentcurve = ctx->currentcurve->unref();
        ctx->currentcurve = 0;
    }
    if (ctx->cal1) {
        ctx->cal1 = ctx->cal1->unref();
        ctx->cal1 = 0;
    }
    if (ctx->cal2) {
        ctx->cal2 = ctx->cal2->unref();
        ctx->cal2 = 0;
    }

    if (ctx->currentshape) {
        gtk_object_destroy(GTK_OBJECT(ctx->currentshape));
        ctx->currentshape = 0;
    }

    if (ctx->_message_context) {
        delete ctx->_message_context;
        ctx->_message_context = 0;
    }

    G_OBJECT_CLASS(common_parent_class)->dispose(object);
}


static void sp_common_context_setup(SPEventContext *ec)
{
    if ( common_parent_class->setup ) {
        common_parent_class->setup(ec);
    }
}

static inline bool
get_bool_value(char const *const val)
{
    return ( val && *val == '1' );
    /* The only possible values if written by inkscape are NULL (attribute not present)
     * or "0" or "1".
     *
     * For other values (e.g. if modified by a human without following the above rules), the
     * current implementation does the right thing for empty string, "no", "false", "n", "f"
     * but the wrong thing for "yes", "true", "y", "t". */
}

static void sp_common_context_set(SPEventContext *ec, gchar const *key, gchar const *value)
{
    SPCommonContext *ctx = SP_COMMON_CONTEXT(ec);

    if (streq(key, "mass")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : 0.2 );
        ctx->mass = CLAMP(dval, -1000.0, 1000.0);
    } else if (streq(key, "wiggle")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : (1 - DRAG_DEFAULT));
        ctx->drag = CLAMP((1 - dval), DRAG_MIN, DRAG_MAX); // drag is inverse to wiggle
    } else if (streq(key, "angle")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : 0.0);
        ctx->angle = CLAMP (dval, -90, 90);
    } else if (streq(key, "width")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : 0.1 );
        ctx->width = CLAMP(dval, -1000.0, 1000.0);
    } else if (streq(key, "thinning")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : 0.1 );
        ctx->vel_thin = CLAMP(dval, -1.0, 1.0);
    } else if (streq(key, "tremor")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : 0.0 );
        ctx->tremor = CLAMP(dval, 0.0, 1.0);
    } else if (streq(key, "flatness")) {
        double const dval = ( value ? g_ascii_strtod (value, NULL) : 1.0 );
        ctx->flatness = CLAMP(dval, 0, 1.0);
    } else if (streq(key, "usepressure")) {
        ctx->usepressure = get_bool_value(value);
    } else if (streq(key, "usetilt")) {
        ctx->usetilt = get_bool_value(value);
    } else if (streq(key, "abs_width")) {
        ctx->abs_width = get_bool_value(value);
    } else if (streq(key, "cap_rounding")) {
        ctx->cap_rounding = ( value ? g_ascii_strtod (value, NULL) : 0.0 );
    }
}

static gint sp_common_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    gint ret = FALSE;

    // TODO add common hanlding


    if ( !ret ) {
        if ( common_parent_class->root_handler ) {
            ret = common_parent_class->root_handler(event_context, event);
        }
    }

    return ret;
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

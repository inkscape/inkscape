#define __SP_COLOR_WHEEL_C__

/*
 * A wheel color widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   John Bintz <jcoswell@coswellproductions.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2003-2004 Authors
 *
 * This code is in public domain
 */

#include <cstring>
#include <string>

#include <gtk/gtksignal.h>
#include "sp-color-wheel.h"

#include "libnr/nr-rotate-ops.h"

#define WHEEL_SIZE 96

enum {
    CHANGED,
    LAST_SIGNAL
};

#define noDUMP_CHANGE_INFO
#define FOO_NAME(x) g_type_name( G_TYPE_FROM_INSTANCE(x) )

static void sp_color_wheel_class_init (SPColorWheelClass *klass);
static void sp_color_wheel_init (SPColorWheel *wheel);
static void sp_color_wheel_destroy (GtkObject *object);

static void sp_color_wheel_realize (GtkWidget *widget);
static void sp_color_wheel_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void sp_color_wheel_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gint sp_color_wheel_expose (GtkWidget *widget, GdkEventExpose *event);
static gint sp_color_wheel_button_press (GtkWidget *widget, GdkEventButton *event);
static gint sp_color_wheel_button_release (GtkWidget *widget, GdkEventButton *event);
static gint sp_color_wheel_motion_notify (GtkWidget *widget, GdkEventMotion *event);

static void sp_color_wheel_set_hue(SPColorWheel *wheel, gdouble hue);
static void sp_color_wheel_set_sv( SPColorWheel *wheel, gdouble sat, gdouble value );
static void sp_color_wheel_recalc_triangle(SPColorWheel *wheel);

static void sp_color_wheel_paint (SPColorWheel *wheel, GdkRectangle *area);
static void sp_color_wheel_render_hue_wheel (SPColorWheel *wheel);
static void sp_color_wheel_render_triangle (SPColorWheel *wheel);


static gboolean sp_color_wheel_focus(GtkWidget        *widget,
                                     GtkDirectionType  direction);

static void sp_color_wheel_process_in_triangle( SPColorWheel *wheel, gdouble x, gdouble y );

static GtkWidgetClass *parent_class;
static guint wheel_signals[LAST_SIGNAL] = {0};

/*
static double
get_time (void)
{
    GTimeVal tv;
    g_get_current_time (&tv);
    return tv.tv_sec + 1e-6 * tv.tv_usec;
}
*/

GType sp_color_wheel_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPColorWheelClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_color_wheel_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPColorWheel),
            0, // n_preallocs
            (GInstanceInitFunc)sp_color_wheel_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_WIDGET, "SPColorWheel", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_color_wheel_class_init (SPColorWheelClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;

    parent_class = (GtkWidgetClass*)gtk_type_class (GTK_TYPE_WIDGET);

    wheel_signals[CHANGED] = gtk_signal_new ("changed",
                          (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                          GTK_CLASS_TYPE(object_class),
                          GTK_SIGNAL_OFFSET (SPColorWheelClass, changed),
                          gtk_marshal_NONE__NONE,
                          GTK_TYPE_NONE, 0);

    object_class->destroy = sp_color_wheel_destroy;

    widget_class->realize = sp_color_wheel_realize;
    widget_class->size_request = sp_color_wheel_size_request;
    widget_class->size_allocate = sp_color_wheel_size_allocate;

    widget_class->focus = sp_color_wheel_focus;

    widget_class->expose_event = sp_color_wheel_expose;
    widget_class->button_press_event = sp_color_wheel_button_press;
    widget_class->button_release_event = sp_color_wheel_button_release;
    widget_class->motion_notify_event = sp_color_wheel_motion_notify;
}

static void
sp_color_wheel_init (SPColorWheel *wheel)
{
    /* We are widget with window */
    GTK_WIDGET_UNSET_FLAGS (wheel, GTK_NO_WINDOW);
    GTK_WIDGET_SET_FLAGS (wheel, GTK_CAN_FOCUS );

    wheel->dragging = FALSE;

    wheel->_inTriangle = FALSE;
    wheel->_triDirty = TRUE;
    wheel->_triangle = NULL;
    for ( guint i = 0; i < G_N_ELEMENTS(wheel->_triPoints); i++ )
    {
        wheel->_triPoints[i].x = 0;
        wheel->_triPoints[i].y = 0;
    }
    wheel->_triImage = NULL;
    wheel->_triBs = 0;

    wheel->_image = NULL;
    wheel->_bs = 0;
    wheel->_hue = 0.0;
    wheel->_sat = 0.9;
    wheel->_value = 0.25;
    wheel->_inner = 0;
    wheel->_center = 0;
    wheel->_spotValue = 1.0;
}

static void
sp_color_wheel_destroy (GtkObject *object)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (object);

    if ( wheel->_image )
    {
        g_free( wheel->_image );
        wheel->_image = NULL;
        wheel->_bs = 0;
    }

    if ( wheel->_triImage )
    {
        g_free( wheel->_triImage );
        wheel->_triImage = NULL;
        wheel->_triBs = 0;
    }

    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}


void sp_color_wheel_get_color( SPColorWheel *wheel, SPColor* color )
{
    float rgb[3];
    gint i;
    g_return_if_fail (SP_IS_COLOR_WHEEL (wheel));
    g_return_if_fail (wheel != NULL);
    g_return_if_fail (color != NULL);

    sp_color_hsv_to_rgb_floatv (rgb, wheel->_hue, 1.0, 1.0);
    for ( i = 0; i < 3; i++ )
    {
        rgb[i] = (rgb[i] * wheel->_sat) + (wheel->_value * (1.0 - wheel->_sat));
    }

    color->set( rgb[0], rgb[1], rgb[2] );
}

void sp_color_wheel_set_color( SPColorWheel *wheel, const SPColor* color )
{
#ifdef DUMP_CHANGE_INFO
    g_message("sp_color_wheel_set_color( wheel=%p, %f, %f, %f)", wheel, color->v.c[0], color->v.c[1], color->v.c[2] );
#endif
    g_return_if_fail (SP_IS_COLOR_WHEEL (wheel));
    g_return_if_fail (wheel != NULL);
    g_return_if_fail (color != NULL);

    float hue;
    float scratch[3];
    float rgb[3];

    sp_color_get_rgb_floatv (color, rgb);
    sp_color_rgb_to_hsv_floatv (scratch, rgb[0], rgb[1], rgb[2]);
    hue = scratch[0];

    sp_color_hsv_to_rgb_floatv (scratch, hue, 1.0, 1.0);

    gint lowInd = 0;
    gint hiInd = 0;
    for ( int i = 1; i < 3; i++ )
    {
        if ( scratch[i] < scratch[lowInd] )
        {
            lowInd = i;
        }
        if ( scratch[i] > scratch[hiInd] )
        {
            hiInd = i;
        }
    }
    // scratch[lowInd] should always be 0
    gdouble sat = (rgb[hiInd] - rgb[lowInd])/(scratch[hiInd]-scratch[lowInd]);
    gdouble val = sat < 1.0 ? (rgb[hiInd] - sat * scratch[hiInd])/(1.0-sat) : 0.0;


    sp_color_wheel_set_hue(wheel, hue);
    sp_color_wheel_set_sv(wheel, sat, val);
}

gboolean sp_color_wheel_is_adjusting( SPColorWheel *wheel )
{
    g_return_val_if_fail( SP_IS_COLOR_WHEEL(wheel), FALSE );
    return wheel->dragging;
}

static void
sp_color_wheel_realize (GtkWidget *widget)
{
    SPColorWheel *wheel;
    GdkWindowAttr attributes;
    gint attributes_mask;

    wheel = SP_COLOR_WHEEL (widget);

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gdk_rgb_get_visual ();
    attributes.colormap = gdk_rgb_get_cmap ();
    attributes.event_mask = gtk_widget_get_events (widget);
    attributes.event_mask |= (GDK_EXPOSURE_MASK |
                  GDK_BUTTON_PRESS_MASK |
                  GDK_BUTTON_RELEASE_MASK |
                  GDK_POINTER_MOTION_MASK |
                  GDK_ENTER_NOTIFY_MASK |
                  GDK_LEAVE_NOTIFY_MASK |
                  GDK_FOCUS_CHANGE_MASK );
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
}

static void
sp_color_wheel_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (widget);

    requisition->width = WHEEL_SIZE + widget->style->xthickness * 2;
    requisition->height = WHEEL_SIZE + widget->style->ythickness * 2;
}

static void
sp_color_wheel_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (widget);

    widget->allocation = *allocation;

    wheel->_center = MIN(allocation->width, allocation->height)/2;
    wheel->_inner = (3 * wheel->_center)/4;
    if ( wheel->_image )
    {
        g_free( wheel->_image );
        wheel->_image = NULL;
        wheel->_bs = 0;
    }

    // Need to render the gradient before we do the triangle over
    sp_color_wheel_render_hue_wheel(wheel);
    sp_color_wheel_recalc_triangle(wheel);
    sp_color_wheel_render_triangle(wheel);

    if (GTK_WIDGET_REALIZED (widget)) {
        /* Resize GdkWindow */
        gdk_window_move_resize (widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
    }
}

static gint
sp_color_wheel_expose (GtkWidget *widget, GdkEventExpose *event)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (widget);

    if (GTK_WIDGET_DRAWABLE (widget)) {
        gint width, height;
        width = widget->allocation.width;
        height = widget->allocation.height;
        sp_color_wheel_paint (wheel, &event->area);
    }

    return TRUE;
}

static gint
sp_color_wheel_button_press (GtkWidget *widget, GdkEventButton *event)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (widget);

    if (event->button == 1) {
        gint cx, cw;
        cx = widget->style->xthickness;
        cw = widget->allocation.width - 2 * cx;
        gboolean grabbed = FALSE;

        {
            double dx = event->x - wheel->_center;
            double dy = event->y - wheel->_center;
            gint hyp = static_cast<gint>(ABS(dx*dx) + ABS(dy*dy));
            if ( hyp <= (wheel->_center*wheel->_center) )
            {
                if ( hyp >= (wheel->_inner*wheel->_inner) )
                {
                    gdouble rot = atan2( dy, -dx );
                    sp_color_wheel_set_hue (wheel, (rot + M_PI) / (2.0 * M_PI));

                    wheel->_inTriangle = FALSE;
                    grabbed = TRUE;
                }
                else if ( wheel->_triangle && gdk_region_point_in( wheel->_triangle, (gint)event->x, (gint)event->y ) )
                {
                    wheel->_inTriangle = TRUE;
                    sp_color_wheel_process_in_triangle( wheel, event->x, event->y );
                    grabbed = TRUE;
                }
            }
        }

        if ( grabbed )
        {
            gtk_widget_queue_draw( widget );
            gtk_widget_grab_focus( widget );

            wheel->dragging = TRUE;
#ifdef DUMP_CHANGE_INFO
            {
                SPColor color;
                sp_color_wheel_get_color( wheel, &color );
                g_message( "%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                           "CHANGED",
                           color.toRGBA32( 0 ), FOO_NAME(wheel));
            }
#endif
            gtk_signal_emit (GTK_OBJECT (wheel), wheel_signals[CHANGED]);
            gdk_pointer_grab (widget->window, FALSE,
                              (GdkEventMask)(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
                              NULL, NULL, event->time);
        }
    }

    return TRUE;
}

static gint
sp_color_wheel_button_release (GtkWidget *widget, GdkEventButton *event)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (widget);

    if (event->button == 1) {
        gdk_pointer_ungrab (event->time);
        wheel->dragging = FALSE;

#ifdef DUMP_CHANGE_INFO
        {
            SPColor color;
            sp_color_wheel_get_color( wheel, &color );
            g_message( "%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                       "CHANGED",
                       color.toRGBA32( 0 ), FOO_NAME(wheel));
        }
#endif
        gtk_signal_emit (GTK_OBJECT (wheel), wheel_signals[CHANGED]);
    }

    return TRUE;
}

static gint
sp_color_wheel_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
    SPColorWheel *wheel;

    wheel = SP_COLOR_WHEEL (widget);

    if (wheel->dragging) {
        double dx = event->x - wheel->_center;
        double dy = event->y - wheel->_center;
        if ( !wheel->_inTriangle )
        {
            gdouble rot = atan2( dy, -dx );
            sp_color_wheel_set_hue (wheel, (rot + M_PI) / (2.0 * M_PI));
        }
        else
        {
            sp_color_wheel_process_in_triangle( wheel, event->x, event->y );
        }

#ifdef DUMP_CHANGE_INFO
        {
            SPColor color;
            sp_color_wheel_get_color( wheel, &color );
            g_message( "%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                       "CHANGED",
                       color.toRGBA32( 0 ), FOO_NAME(wheel));
        }
#endif
        gtk_signal_emit (GTK_OBJECT (wheel), wheel_signals[CHANGED]);
    }

    return TRUE;
}

GtkWidget *
sp_color_wheel_new ()
{
    SPColorWheel *wheel;

    wheel = (SPColorWheel*)gtk_type_new (SP_TYPE_COLOR_WHEEL);

    return GTK_WIDGET (wheel);
}

static void sp_color_wheel_set_hue(SPColorWheel *wheel, gdouble hue)
{
    g_return_if_fail (SP_IS_COLOR_WHEEL (wheel));

    if ( wheel->_hue != hue )
    {
        wheel->_hue = hue;

        sp_color_wheel_recalc_triangle(wheel);

        SPColor color;
        gfloat rgb[3];
        sp_color_wheel_get_color( wheel, &color );
        sp_color_get_rgb_floatv (&color, rgb);

        wheel->_spotValue = ( (0.299 * rgb[0]) + (0.587 * rgb[1]) + (0.114 * rgb[2]) );

        gtk_widget_queue_draw (GTK_WIDGET (wheel));
    }
}


static void sp_color_wheel_set_sv( SPColorWheel *wheel, gdouble sat, gdouble value )
{
    static gdouble epsilon = 1e-6;
    gboolean changed = FALSE;

    if ( ABS( wheel->_sat - sat ) > epsilon )
    {
        wheel->_sat = sat;
        changed = TRUE;
    }
    if ( ABS( wheel->_value - value ) > epsilon )
    {
        wheel->_value = value;
        changed = TRUE;
    }

    if ( changed )
    {
        SPColor color;
        gfloat rgb[3];
        sp_color_wheel_get_color( wheel, &color );
        sp_color_get_rgb_floatv (&color, rgb);

        wheel->_spotValue = ( (0.299 * rgb[0]) + (0.587 * rgb[1]) + (0.114 * rgb[2]) );

#ifdef DUMP_CHANGE_INFO
        {
            SPColor color;
            sp_color_wheel_get_color( wheel, &color );
            g_message( "%s:%d: About to signal %s to color %08x in %s", __FILE__, __LINE__,
                       "CHANGED",
                       color.toRGBA32( 0 ), FOO_NAME(wheel));
        }
#endif
        gtk_signal_emit (GTK_OBJECT (wheel), wheel_signals[CHANGED]);
    }
    gtk_widget_queue_draw (GTK_WIDGET (wheel));
}

static void sp_color_wheel_recalc_triangle(SPColorWheel *wheel)
{
    if ( wheel->_triangle )
    {
        gdk_region_destroy( wheel->_triangle );
        wheel->_triangle = NULL;
    }
    wheel->_triDirty = TRUE;

    if ( wheel->_center > 0 && wheel->_inner > 0 )
    {
        gdouble dx = cos( M_PI * 2 * wheel->_hue );
        gdouble dy = -sin( M_PI * 2 * wheel->_hue );

        wheel->_triPoints[0].x = wheel->_center + static_cast<gint>(dx * wheel->_inner);
        wheel->_triPoints[0].y = wheel->_center + static_cast<gint>(dy * wheel->_inner);

        dx = cos( M_PI * 2 * wheel->_hue + ((M_PI * 2)/ 3) );
        dy = -sin( M_PI * 2 * wheel->_hue + ((M_PI * 2) / 3) );
        wheel->_triPoints[1].x = wheel->_center + static_cast<gint>(dx * wheel->_inner);
        wheel->_triPoints[1].y = wheel->_center + static_cast<gint>(dy * wheel->_inner);

        dx = cos( M_PI * 2 * wheel->_hue - ((M_PI*2) / 3) );
        dy = -sin( M_PI * 2 * wheel->_hue - ((M_PI*2) / 3) );
        wheel->_triPoints[2].x = wheel->_center + static_cast<gint>(dx * wheel->_inner);
        wheel->_triPoints[2].y = wheel->_center + static_cast<gint>(dy * wheel->_inner);


        wheel->_triangle = gdk_region_polygon( wheel->_triPoints,
                                               3,
                                               GDK_EVEN_ODD_RULE );
    }
}

#define VERT_SWAP( X, Y ) { \
    gfloat tmpF; \
 \
    tmpF = point##X.x; \
    point##X.x = point##Y.x; \
    point##Y.x = tmpF; \
 \
    tmpF = point##X.y; \
    point##X.y = point##Y.y; \
    point##Y.y = tmpF; \
 \
    tmpF = rgb##X[0]; \
    rgb##X[0] = rgb##Y[0]; \
    rgb##Y[0] = tmpF; \
 \
    tmpF = rgb##X[1]; \
    rgb##X[1] = rgb##Y[1]; \
    rgb##Y[1] = tmpF; \
 \
    tmpF = rgb##X[2]; \
    rgb##X[2] = rgb##Y[2]; \
    rgb##Y[2] = tmpF; \
}

#define VERT_COPY( dst, src ) { \
    point##dst.x = point##src.x; \
 \
    point##dst.y = point##src.y; \
 \
    rgb##dst[0] = rgb##src[0]; \
    rgb##dst[1] = rgb##src[1]; \
    rgb##dst[2] = rgb##src[2]; \
}

typedef struct {
    gfloat x;
    gfloat y;
} PointF;

/**
 * Render the provided color wheel information as a triangle.
 */
static void sp_color_wheel_render_triangle (SPColorWheel *wheel)
{
    if ( wheel->_image )
    {
        if ( wheel->_triBs < wheel->_bs )
        {
            g_free( wheel->_triImage );
            wheel->_triImage = NULL;
        }

        if (wheel->_triDirty || !wheel->_triImage)
        {
            if ( !wheel->_triImage )
            {
                wheel->_triBs = wheel->_bs;
                wheel->_triImage = g_new (guchar, wheel->_triBs * 3);
                //g_message( "just allocated %fKB for tri", ((wheel->_triBs * 3.0)/1024.0) );
            }

            memcpy( wheel->_triImage, wheel->_image, wheel->_bs * 3 );

            PointF pointA, pointB, pointC;
            pointA.x = wheel->_triPoints[0].x;
            pointA.y = wheel->_triPoints[0].y;
            pointB.x = wheel->_triPoints[1].x;
            pointB.y = wheel->_triPoints[1].y;
            pointC.x = wheel->_triPoints[2].x;
            pointC.y = wheel->_triPoints[2].y;

            gfloat rgbA[3];
            gfloat rgbB[3] = {0.0, 0.0, 0.0};
            gfloat rgbC[3] = {1.0, 1.0, 1.0};

            sp_color_hsv_to_rgb_floatv (rgbA, wheel->_hue, 1.0, 1.0);

// Start of Gouraud fill ============================================================
            gint rowStride = wheel->_center * 2 * 3;
            guchar* dst = wheel->_triImage;

            if ( pointC.y < pointB.y )
                VERT_SWAP( C, B );

            if ( pointC.y < pointA.y )
                VERT_SWAP( C, A );

            if ( pointB.y < pointA.y )
                VERT_SWAP( B, A );

            if ( pointA.y == pointB.y && pointB.x < pointA.x )
                VERT_SWAP( A, B );

            gfloat dr, dg, db;

            gfloat dx1,dx2,dx3;
            gfloat dr1,dr2,dr3;
            gfloat dg1,dg2,dg3;
            gfloat db1,db2,db3;


            PointF pointS;
            PointF pointE;
            PointF pointP;
            gfloat rgbS[3];
            gfloat rgbE[3];
            gfloat rgbP[3];


            if (pointB.y-pointA.y > 0)
            {
                dx1=(pointB.x-pointA.x)/(pointB.y-pointA.y);
                dr1=(rgbB[0]-rgbA[0])/(pointB.y-pointA.y);
                dg1=(rgbB[1]-rgbA[1])/(pointB.y-pointA.y);
                db1=(rgbB[2]-rgbA[2])/(pointB.y-pointA.y);
            }
            else
            {
                dx1=dr1=dg1=db1=0;
            }

            if (pointC.y-pointA.y > 0)
            {
                dx2=(pointC.x-pointA.x)/(pointC.y-pointA.y);
                dr2=(rgbC[0]-rgbA[0])/(pointC.y-pointA.y);
                dg2=(rgbC[1]-rgbA[1])/(pointC.y-pointA.y);
                db2=(rgbC[2]-rgbA[2])/(pointC.y-pointA.y);
            }
            else
            {
                dx2=dr2=dg2=db2=0;
            }

            if (pointC.y-pointB.y > 0)
            {
                dx3=(pointC.x-pointB.x)/(pointC.y-pointB.y);
                dr3=(rgbC[0]-rgbB[0])/(pointC.y-pointB.y);
                dg3=(rgbC[1]-rgbB[1])/(pointC.y-pointB.y);
                db3=(rgbC[2]-rgbB[2])/(pointC.y-pointB.y);
            }
            else
            {
                dx3=dr3=dg3=db3=0;
            }

            VERT_COPY(S, A);
            VERT_COPY(E, A);

            int runs = 1; int fill_mode = 0;

            if ( dx1 == 0 )
            {
                fill_mode = 0;
            }
            else if ( dx1 > dx2 )
            {
                fill_mode = 1; runs = 2;
            }
            else if ( dx1 )
            {
                fill_mode = 2; runs = 2;
            }

            gfloat targetY = 0;
            int fill_direction_mode = 0;

            for (int current_run = 0; current_run < runs; current_run++)
            {
                targetY = pointC.y;
                switch (fill_mode)
                {
                    case 0:
                        VERT_COPY(E,B);
                        fill_direction_mode = 0;
                        break;
                    case 1:
                        if (current_run == 0) {
                            targetY = pointB.y;
                            fill_direction_mode = 1;
                        } else {
                            VERT_COPY(E,B);
                            fill_direction_mode = 0;
                        }
                        break;
                    case 2:
                        if (current_run == 0) {
                            targetY = pointB.y;
                            fill_direction_mode = 2;
                        } else {
                            VERT_COPY(S,B);
                            fill_direction_mode = 3;
                        }
                        break;
                }

                for(;pointS.y <= targetY; pointS.y++,pointE.y++)
                {
                    if (pointE.x-pointS.x > 0)
                    {
                        dr=(rgbE[0]-rgbS[0])/(pointE.x-pointS.x);
                        dg=(rgbE[1]-rgbS[1])/(pointE.x-pointS.x);
                        db=(rgbE[2]-rgbS[2])/(pointE.x-pointS.x);
                    }
                    else
                    {
                        dr=dg=db=0;
                    }
                    VERT_COPY(P,S);
                    dst = wheel->_triImage + (static_cast<gint>(pointP.y) * rowStride);
                    dst += static_cast<gint>(pointP.x) * 3;
                    for(;pointP.x < pointE.x;pointP.x++)
                    {
                        //putpixel(P);
                        dst[0] = SP_COLOR_F_TO_U(rgbP[0]);
                        dst[1] = SP_COLOR_F_TO_U(rgbP[1]);
                        dst[2] = SP_COLOR_F_TO_U(rgbP[2]);
                        dst += 3;
                        rgbP[0]+=dr; rgbP[1]+=dg; rgbP[2]+=db;
                    }

                    switch (fill_direction_mode) {
                        case 0:
                            pointS.x+=dx2; rgbS[0]+=dr2; rgbS[1]+=dg2; rgbS[2]+=db2;
                            pointE.x+=dx3; rgbE[0]+=dr3; rgbE[1]+=dg3; rgbE[2]+=db3;
                            break;
                        case 1:
                            pointS.x+=dx2; rgbS[0]+=dr2; rgbS[1]+=dg2; rgbS[2]+=db2;
                            pointE.x+=dx1; rgbE[0]+=dr1; rgbE[1]+=dg1; rgbE[2]+=db1;
                            break;
                        case 2:
                            pointS.x+=dx1; rgbS[0]+=dr1; rgbS[1]+=dg1; rgbS[2]+=db1;
                            pointE.x+=dx2; rgbE[0]+=dr2; rgbE[1]+=dg2; rgbE[2]+=db2;
                            break;
                        case 3:
                            pointS.x+=dx3; rgbS[0]+=dr3; rgbS[1]+=dg3; rgbS[2]+=db3;
                            pointE.x+=dx2; rgbE[0]+=dr2; rgbE[1]+=dg2; rgbE[2]+=db2;
                            break;
                    }
                }
            }


// End of Gouraud fill  ============================================================

            wheel->_triDirty = FALSE;
            //g_message( "Just updated triangle" );
        }
    }
}

static void
sp_color_wheel_paint (SPColorWheel *wheel, GdkRectangle *area)
{
    GtkWidget *widget;
    GdkRectangle warea, carea;
    GdkRectangle wpaint, cpaint;

    widget = GTK_WIDGET (wheel);

    /* Widget area */
    warea.x = 0;
    warea.y = 0;
    warea.width = widget->allocation.width;
    warea.height = widget->allocation.height;

    /* Color gradient area */
    carea.x = widget->style->xthickness;
    carea.y = widget->style->ythickness;
    carea.width = widget->allocation.width - 2 * carea.x;
    carea.height = widget->allocation.height - 2 * carea.y;

    /* Actual paintable area */
    if (!gdk_rectangle_intersect (area, &warea, &wpaint)) return;

    //g_message( "Painted as state %d", widget->state );

    /* Paintable part of color gradient area */
    if (gdk_rectangle_intersect (area, &carea, &cpaint)) {
        sp_color_wheel_render_hue_wheel (wheel);
        sp_color_wheel_render_triangle (wheel);
    }

/*
    gtk_draw_box (widget->style,
                  widget->window,
                  (GtkStateType)widget->state,
                  GTK_SHADOW_NONE,
                  warea.x,
                  warea.y,
                  warea.width,
                  warea.height);
*/

    gtk_style_apply_default_background( widget->style,
                                        widget->window,
                                        TRUE,
                                        (GtkStateType)widget->state,
                                        NULL,
                                        0,
                                        0,
                                        warea.width,
                                        warea.height);


    /* Draw shadow */
/*
    gtk_paint_shadow (widget->style, widget->window,
              (GtkStateType)widget->state, GTK_SHADOW_IN,
              NULL, widget, "colorwheel",
              0, 0,
              warea.width, warea.height);
*/


    /* Draw pixelstore */
    if (wheel->_triImage != NULL) {
        //gdouble start, end;
        //start = get_time();
        gdk_draw_rgb_image (widget->window, widget->style->black_gc,
                            0, 0,//cpaint.x, cpaint.y,
                            //cpaint.width, cpaint.height,
                            wheel->_center * 2, wheel->_center * 2,
                            GDK_RGB_DITHER_MAX,
                            wheel->_triImage, wheel->_center * 6);

        //end = get_time();
        //g_message( "blits took %f", (end-start) );
    }

    {
        gdouble dx = cos( M_PI * 2 * wheel->_hue );
        gdouble dy = -sin( M_PI * 2 * wheel->_hue );

        gfloat rgb[3];
        sp_color_hsv_to_rgb_floatv (rgb, wheel->_hue, 1.0, 1.0);

        GdkGC *line_gc = (( (0.299 * rgb[0]) + (0.587 * rgb[1]) + (0.114 * rgb[2]) ) < 0.5) ? widget->style->white_gc : widget->style->black_gc;

        gint inx = wheel->_center + static_cast<gint>(dx * wheel->_inner);
        gint iny = wheel->_center + static_cast<gint>(dy * wheel->_inner);


        gdk_draw_line (widget->window, line_gc,
                       inx, iny,
                       wheel->_center + static_cast<gint>(dx * wheel->_center), wheel->_center + static_cast<gint>(dy * wheel->_center) );


        GdkGCValues values;

        if ( GTK_WIDGET_HAS_FOCUS(wheel) )
        {
            line_gc = widget->style->black_gc;

            gdk_gc_get_values ( line_gc, &values );

            gdk_gc_set_line_attributes ( line_gc,
                                         3, // Line width
                                         values.line_style, //GDK_LINE_SOLID,
                                         values.cap_style, //GDK_CAP_BUTT,
                                         values.join_style ); //GDK_JOIN_MITER );

            if ( wheel->_inTriangle )
            {
                gdk_draw_line (widget->window, line_gc,
                               wheel->_triPoints[0].x, wheel->_triPoints[0].y,
                               wheel->_triPoints[1].x, wheel->_triPoints[1].y );

                gdk_draw_line (widget->window, line_gc,
                               wheel->_triPoints[1].x, wheel->_triPoints[1].y,
                               wheel->_triPoints[2].x, wheel->_triPoints[2].y );

                gdk_draw_line (widget->window, line_gc,
                               wheel->_triPoints[2].x, wheel->_triPoints[2].y,
                               wheel->_triPoints[0].x, wheel->_triPoints[0].y );
            }
            else
            {
                gdk_draw_arc (widget->window, line_gc,
                              FALSE, // filled
                              0, 0,
                              wheel->_center * 2, wheel->_center * 2,
                              0, 64 * 360 );

                gint diff = wheel->_center - wheel->_inner;

                gdk_draw_arc (widget->window, line_gc,
                              FALSE, // filled
                              diff, diff,
                              wheel->_inner * 2, wheel->_inner * 2,
                              0, 64 * 360 );

            }
            gdk_gc_set_line_attributes ( line_gc,
                                         values.line_width, // Line width
                                         values.line_style, //GDK_LINE_SOLID,
                                         values.cap_style, //GDK_CAP_BUTT,
                                         values.join_style ); //GDK_JOIN_MITER );
        }
// ==========

//        line_gc = (p[3] < 0x80) ? widget->style->white_gc : widget->style->black_gc;
        line_gc = (wheel->_spotValue < 0.5) ? widget->style->white_gc : widget->style->black_gc;

        gdk_gc_get_values ( line_gc, &values );

        gdk_gc_set_line_attributes ( line_gc,
                                     2, // Line width
                                     values.line_style, //GDK_LINE_SOLID,
                                     values.cap_style, //GDK_CAP_BUTT,
                                     values.join_style ); //GDK_JOIN_MITER );

        gint pointX = (gint)( (1.0 - wheel->_sat) * ((1.0-wheel->_value)*(gdouble)wheel->_triPoints[1].x + wheel->_value*(gdouble)wheel->_triPoints[2].x)
            + (wheel->_sat * wheel->_triPoints[0].x) );

        gint pointY = (gint)( (1.0 - wheel->_sat) * ((1.0-wheel->_value)*(gdouble)wheel->_triPoints[1].y + wheel->_value*(gdouble)wheel->_triPoints[2].y)
            + (wheel->_sat * wheel->_triPoints[0].y) );


        gdk_gc_set_line_attributes ( line_gc,
                                     values.line_width, // Line width
                                     values.line_style, //GDK_LINE_SOLID,
                                     values.cap_style, //GDK_CAP_BUTT,
                                     values.join_style ); //GDK_JOIN_MITER );

        gdk_draw_arc (widget->window, line_gc,
                      FALSE, // filled
                      pointX - 4, pointY - 4,
                      8, 8,
                      0, 64 * 360 );

        gdk_draw_arc (widget->window, line_gc,
                      FALSE, // filled
                      pointX - 3, pointY - 3,
                      6, 6,
                      0, 64 * 360 );



    }
}

/* Colors are << 16 */

static void
sp_color_wheel_render_hue_wheel (SPColorWheel *wheel)
{
    guchar *dp;
    gint x, y;
    guint r, g, b;
    gint size = wheel->_center * 2;
    gboolean dirty = FALSE;

    if (wheel->_image && (wheel->_bs < (size * size) )) {
        g_free (wheel->_image);
        wheel->_image = NULL;
        wheel->_bs = 0;

        if ( wheel->_triImage )
        {
            g_free( wheel->_triImage );
            wheel->_triImage = NULL;
            wheel->_triBs = 0;
            wheel->_triDirty = TRUE;
        }
    }

    if (!wheel->_image) {
        wheel->_image = g_new (guchar, size * size * 3);
        wheel->_bs = size * size;
        dirty = TRUE;
        //g_message( "just allocated %fKB for hue", ((wheel->_bs * 3.0)/1024.0) );
    }

    if ( dirty )
    {
        GtkWidget* widget = GTK_WIDGET (wheel);
        dp = wheel->_image;
        r = widget->style->bg[widget->state].red >> 8;
        g = widget->style->bg[widget->state].green >> 8;
        b = widget->style->bg[widget->state].blue >> 8;
        //g_message( "Rendered as state %d", widget->state );

        gint offset = wheel->_center;
        gint inner = wheel->_inner * wheel->_inner;
        gint rad = wheel->_center * wheel->_center;

        for (x = 0; x < size; x++) {
            guchar *d = dp;
            for (y = 0; y < size; y++) {
                gint dx = x - offset;
                gint dy = y - offset;
                gint hyp = (ABS(dx*dx) + ABS(dy*dy));
                if ( hyp >= inner && hyp <= rad)
                {
                    gdouble rot = atan2( static_cast<gdouble>(dy), static_cast<gdouble>(-dx) );

                    gfloat rgb[3];
                    sp_color_hsv_to_rgb_floatv (rgb, (rot + M_PI) / (2.0 * M_PI), 1.0, 1.0);

                    d[0] = SP_COLOR_F_TO_U (rgb[0]);
                    d[1] = SP_COLOR_F_TO_U (rgb[1]);
                    d[2] = SP_COLOR_F_TO_U (rgb[2]);
                }
                else
                {
                    /* Background value */
                    d[0] = r;
                    d[1] = g;
                    d[2] = b;
                }

                d += 3 * size;
            }
            dp += 3;
        }
    }
}

static gboolean sp_color_wheel_focus(GtkWidget        *widget,
                                     GtkDirectionType  direction)
{
    gboolean focusKept = FALSE;
    gboolean wasFocused = GTK_WIDGET_HAS_FOCUS(widget);
    SPColorWheel* wheel = SP_COLOR_WHEEL(widget);
    gboolean goingUp = FALSE;

    switch ( direction )
    {
    case GTK_DIR_TAB_FORWARD:
    case GTK_DIR_UP:
    case GTK_DIR_LEFT:
        goingUp = TRUE;
        break;

    case GTK_DIR_TAB_BACKWARD:
    case GTK_DIR_DOWN:
    case GTK_DIR_RIGHT:
        goingUp = FALSE;
        break;
    default:
        ;
    }

    if ( !wasFocused )
    {
        wheel->_inTriangle = !goingUp;
        gtk_widget_grab_focus (widget);
        focusKept = TRUE;
    }
    else if ( (!wheel->_inTriangle) == (!goingUp) )
    {
        focusKept = FALSE;
    }
    else
    {
        wheel->_inTriangle = !wheel->_inTriangle;
        gtk_widget_queue_draw( widget );
        focusKept = TRUE;
    }

    return focusKept;
}

static void sp_color_wheel_process_in_triangle( SPColorWheel *wheel, gdouble x, gdouble y )
{
// njh: dot(rot90(B-C), x) = saturation
// njh: dot(B-C, x) = value
    NR::Point delta( x - (((gdouble)(wheel->_triPoints[1].x + wheel->_triPoints[2].x)) / 2.0),
                     y - (((gdouble)(wheel->_triPoints[1].y + wheel->_triPoints[2].y)) / 2.0) );

    gdouble rot = (M_PI * 2 * wheel->_hue );

    NR::Point result = delta * NR::rotate(rot);

    gdouble sat = CLAMP( result[NR::X] / (wheel->_inner * 1.5), 0.0, 1.0 );

    gdouble halfHeight = (wheel->_inner * sin(M_PI/3.0)) * (1.0 - sat);
    gdouble value = CLAMP( ((result[NR::Y]+ halfHeight) / (2.0*halfHeight)), 0.0, 1.0 );

    wheel->_triDirty = TRUE;

    sp_color_wheel_set_sv( wheel, sat, value );
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

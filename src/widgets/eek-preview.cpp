/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Eek Preview Stuffs.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <algorithm>
using std::min;

#include <gtk/gtk.h>
#include "eek-preview.h"
#include "preferences.h"

#define PRIME_BUTTON_MAGIC_NUMBER 1

/* Keep in sync with last value in eek-preview.h */
#define PREVIEW_SIZE_LAST PREVIEW_SIZE_HUGE
#define PREVIEW_SIZE_NEXTFREE (PREVIEW_SIZE_HUGE + 1)

#define PREVIEW_MAX_RATIO 500

enum {
    PROP_0,
    PROP_FOCUS
};

typedef struct
{
    int          scaledW;
    int          scaledH;
    
    int          r;
    int          g;
    int          b;
    
    gboolean     hot;
    gboolean     within;
    gboolean     takesFocus;
    ViewType     view;
    PreviewSize  size;
    guint        ratio;
    guint        linked;
    guint        border;
    GdkPixbuf   *previewPixbuf;
    GdkPixbuf   *scaled;
} EekPreviewPrivate;

#define EEK_PREVIEW_GET_PRIVATE(preview) \
  G_TYPE_INSTANCE_GET_PRIVATE(preview, EEK_PREVIEW_TYPE, EekPreviewPrivate)

static void     eek_preview_class_init( EekPreviewClass *klass );
static void     eek_preview_init( EekPreview *preview );
static gboolean eek_preview_draw(GtkWidget* widget, cairo_t* cr);

G_DEFINE_TYPE(EekPreview, eek_preview, GTK_TYPE_DRAWING_AREA);

static GtkWidgetClass* parent_class = 0;

void eek_preview_set_color( EekPreview* preview, int r, int g, int b )
{
    EekPreviewPrivate *priv = EEK_PREVIEW_GET_PRIVATE(preview);

    priv->r = r;
    priv->g = g;
    priv->b = b;

    gtk_widget_queue_draw(GTK_WIDGET(preview));
}


void
eek_preview_set_pixbuf(EekPreview *preview,
                       GdkPixbuf  *pixbuf)
{
    EekPreviewPrivate *priv = EEK_PREVIEW_GET_PRIVATE(preview);

    priv->previewPixbuf     = pixbuf;

    gtk_widget_queue_draw(GTK_WIDGET(preview));

    if (priv->scaled)
    {
        g_object_unref(priv->scaled);
        priv->scaled = NULL;
    }

    priv->scaledW = gdk_pixbuf_get_width(priv->previewPixbuf);
    priv->scaledH = gdk_pixbuf_get_height(priv->previewPixbuf);
}

static gboolean setupDone = FALSE;
static GtkRequisition sizeThings[PREVIEW_SIZE_NEXTFREE];

void eek_preview_set_size_mappings( guint count, GtkIconSize const* sizes )
{
    gint width = 0;
    gint height = 0;
    gint smallest = 512;
    gint largest = 0;
    guint i = 0;
    guint delta = 0;

    for ( i = 0; i < count; ++i ) {
        gboolean worked = gtk_icon_size_lookup( sizes[i], &width, &height );
        if ( worked ) {
            if ( width < smallest ) {
                smallest = width;
            }
            if ( width > largest ) {
                largest = width;
            }
        }
    }

    smallest = (smallest * 3) / 4;

    delta = largest - smallest;

    for ( i = 0; i < G_N_ELEMENTS(sizeThings); ++i ) {
        guint val = smallest + ( (i * delta) / (G_N_ELEMENTS(sizeThings) - 1) );
        sizeThings[i].width = val;
        sizeThings[i].height = val;
    }

    setupDone = TRUE;
}

static void eek_preview_size_request( GtkWidget* widget, GtkRequisition* req )
{
    gint               width   = 0;
    gint               height  = 0;
    EekPreview        *preview = EEK_PREVIEW(widget);
    EekPreviewPrivate *priv    = EEK_PREVIEW_GET_PRIVATE(preview);

    if ( !setupDone ) {
        GtkIconSize sizes[] = {
            GTK_ICON_SIZE_MENU,
            GTK_ICON_SIZE_SMALL_TOOLBAR,
            GTK_ICON_SIZE_LARGE_TOOLBAR,
            GTK_ICON_SIZE_BUTTON,
            GTK_ICON_SIZE_DIALOG
        };
        eek_preview_set_size_mappings( G_N_ELEMENTS(sizes), sizes );
    }

    width = sizeThings[priv->size].width;
    height = sizeThings[priv->size].height;

    if ( priv->view == VIEW_TYPE_LIST ) {
        width *= 3;
    }

    if ( priv->ratio != 100 ) {
        width = (width * priv->ratio) / 100;
        if ( width < 0 ) {
            width = 1;
        }
    }

    req->width = width;
    req->height = height;
}

#if GTK_CHECK_VERSION(3,0,0)
static void eek_preview_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	eek_preview_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void eek_preview_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	eek_preview_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

enum {
  CLICKED_SIGNAL,
  ALTCLICKED_SIGNAL,
  LAST_SIGNAL
};


static guint eek_preview_signals[LAST_SIGNAL] = { 0 };

#if !GTK_CHECK_VERSION(3,0,0)
static gboolean eek_preview_expose_event( GtkWidget* widget, GdkEventExpose* /* event */ )
{
    gboolean result = FALSE;

    if (gtk_widget_is_drawable(widget)) {
	GdkWindow* window = gtk_widget_get_window(widget);
	cairo_t* cr = gdk_cairo_create(window);
	result = eek_preview_draw(widget, cr);
	cairo_destroy(cr);
    }

    return result;
}
#endif

static
gboolean eek_preview_draw(GtkWidget *widget,
                          cairo_t   *cr)
{

    EekPreview        *preview = EEK_PREVIEW(widget);
    EekPreviewPrivate *priv    = EEK_PREVIEW_GET_PRIVATE(preview);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
   
#if !GTK_CHECK_VERSION(3,0,0) 
    GdkColor fg = { 0,
                    static_cast<guint16>(priv->r),
                    static_cast<guint16>(priv->g),
                    static_cast<guint16>(priv->b)
                  };
#endif

    gint insetTop = 0, insetBottom = 0;
    gint insetLeft = 0, insetRight = 0;

    if (priv->border == BORDER_SOLID) {
        insetTop = 1;
        insetLeft = 1;
    }
    if (priv->border == BORDER_SOLID_LAST_ROW) {
        insetTop = insetBottom = 1;
        insetLeft = 1;
    }
    if (priv->border == BORDER_WIDE) {
        insetTop = insetBottom = 1;
        insetLeft = insetRight = 1;
    }


#if GTK_CHECK_VERSION(3,0,0)
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
	
    gtk_render_frame(context,
                     cr,
                     0, 0,
                     allocation.width, allocation.height);
	
    gtk_render_background(context,
                          cr,
                          0, 0,
                          allocation.width, allocation.height);
#else
    GtkStyle *style = gtk_widget_get_style(widget);
    GdkWindow *window = gtk_widget_get_window(widget);
        
    gtk_paint_flat_box( style,
                        window,
                        (GtkStateType)gtk_widget_get_state(widget),
                        GTK_SHADOW_NONE,
                        NULL,
                        widget,
                        NULL,
                        0, 0,
                        allocation.width, allocation.height);
        
    gdk_colormap_alloc_color( gdk_colormap_get_system(), &fg, FALSE, TRUE );
#endif

    // Border
    if (priv->border != BORDER_NONE) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
        cairo_fill(cr);
    }

    cairo_set_source_rgb(cr, priv->r/65535.0, priv->g/65535.0, priv->b/65535.0 );
    cairo_rectangle(cr, insetLeft, insetTop, allocation.width - (insetLeft + insetRight), allocation.height - (insetTop + insetBottom));
    cairo_fill(cr);

    if (priv->previewPixbuf )
    {
        GtkDrawingArea *da = &(preview->drawing);
        GdkWindow *da_window = gtk_widget_get_window(GTK_WIDGET(da));
        cairo_t *cr = gdk_cairo_create(da_window);

        gint w = gdk_window_get_width(da_window);
        gint h = gdk_window_get_height(da_window);

        if ((w != priv->scaledW) || (h != priv->scaledH)) {
            if (priv->scaled)
            {
                g_object_unref(priv->scaled);
            }

            priv->scaled = gdk_pixbuf_scale_simple(priv->previewPixbuf,
                                                   w - (insetLeft + insetRight),
                                                   h - (insetTop + insetBottom),
                                                   GDK_INTERP_BILINEAR);

            priv->scaledW = w - (insetLeft + insetRight);
            priv->scaledH = h - (insetTop + insetBottom);
        }

        GdkPixbuf *pix = (priv->scaled) ? priv->scaled : priv->previewPixbuf;

        // Border
        if (priv->border != BORDER_NONE) {
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
            cairo_fill(cr);
        }

        gdk_cairo_set_source_pixbuf(cr, pix, insetLeft, insetTop);
        cairo_paint(cr);
        cairo_destroy(cr);
    }

    if (priv->linked)
    {
        /* Draw arrow */
        GdkRectangle possible = {insetLeft,
                                 insetTop,
                                 (allocation.width - (insetLeft + insetRight)),
                                 (allocation.height - (insetTop + insetBottom))
                                };

        GdkRectangle area = {possible.x,
                             possible.y,
                             possible.width / 2,
                             possible.height / 2 };

        /* Make it square */
        if ( area.width > area.height )
            area.width = area.height;
        if ( area.height > area.width )
            area.height = area.width;

        /* Center it horizontally */
        if ( area.width < possible.width ) {
            int diff = (possible.width - area.width) / 2;
            area.x += diff;
        }

        if (priv->linked & PREVIEW_LINK_IN)
        {
#if GTK_CHECK_VERSION(3,0,0)
            gtk_render_arrow(context,
                             cr,
                             G_PI, // Down-pointing arrow
                             area.x, area.y,
                             min(area.width, area.height)
                            );
#else
            gtk_paint_arrow( style,
                             window,
                             gtk_widget_get_state (widget),
                             GTK_SHADOW_ETCHED_IN,
                             NULL, /* clip area.  &area, */
                             widget, /* may be NULL */
                             NULL, /* detail */
                             GTK_ARROW_DOWN,
                             FALSE,
                             area.x, area.y,
                             area.width, area.height
                           );
#endif
        }

        if (priv->linked & PREVIEW_LINK_OUT)
        {
            GdkRectangle otherArea = {area.x, area.y, area.width, area.height};
            if ( otherArea.height < possible.height ) {
                otherArea.y = possible.y + (possible.height - otherArea.height);
            }

#if GTK_CHECK_VERSION(3,0,0)
            gtk_render_arrow(context,
                             cr,
                             G_PI, // Down-pointing arrow
                             otherArea.x, otherArea.y,
                             min(otherArea.width, otherArea.height)
                            );
#else
            gtk_paint_arrow( style,
                             window,
                             gtk_widget_get_state (widget),
                             GTK_SHADOW_ETCHED_OUT,
                             NULL, /* clip area.  &area, */
                             widget, /* may be NULL */
                             NULL, /* detail */
                             GTK_ARROW_DOWN,
                             FALSE,
                             otherArea.x, otherArea.y,
                             otherArea.width, otherArea.height
                           );
#endif
        }

        if (priv->linked & PREVIEW_LINK_OTHER)
        {
            GdkRectangle otherArea = {insetLeft, area.y, area.width, area.height};
            if ( otherArea.height < possible.height ) {
                otherArea.y = possible.y + (possible.height - otherArea.height) / 2;
            }

#if GTK_CHECK_VERSION(3,0,0)
            gtk_render_arrow(context,
                             cr,
                             1.5*G_PI, // Left-pointing arrow
                             otherArea.x, otherArea.y,
                             min(otherArea.width, otherArea.height)
                            );
#else
            gtk_paint_arrow( style,
                             window,
                             gtk_widget_get_state (widget),
                             GTK_SHADOW_ETCHED_OUT,
                             NULL, /* clip area.  &area, */
                             widget, /* may be NULL */
                             NULL, /* detail */
                             GTK_ARROW_LEFT,
                             FALSE,
                             otherArea.x, otherArea.y,
                             otherArea.width, otherArea.height
                           );
#endif
        }


        if (priv->linked & PREVIEW_FILL)
        {
            GdkRectangle otherArea = {possible.x + ((possible.width / 4) - (area.width / 2)),
                                      area.y,
                                      area.width, area.height};
            if ( otherArea.height < possible.height ) {
                otherArea.y = possible.y + (possible.height - otherArea.height) / 2;
            }
#if GTK_CHECK_VERSION(3,0,0)
            gtk_render_check(context,
                             cr,
                             otherArea.x, otherArea.y,
                             otherArea.width, otherArea.height );
#else
            gtk_paint_check( style,
                             window,
                             gtk_widget_get_state (widget),
                             GTK_SHADOW_ETCHED_OUT,
                             NULL,
                             widget,
                             NULL,
                             otherArea.x, otherArea.y,
                             otherArea.width, otherArea.height );
#endif
        }

        if (priv->linked & PREVIEW_STROKE)
        {
            GdkRectangle otherArea = {possible.x + (((possible.width * 3) / 4) - (area.width / 2)),
                                      area.y,
                                      area.width, area.height};
            if ( otherArea.height < possible.height ) {
                otherArea.y = possible.y + (possible.height - otherArea.height) / 2;
            }
#if GTK_CHECK_VERSION(3,0,0)
            // This should be a diamond too?
            gtk_render_check(context,
                             cr,
                             otherArea.x, otherArea.y,
                             otherArea.width, otherArea.height );
#else
            gtk_paint_diamond( style,
                               window,
                               gtk_widget_get_state (widget),
                               GTK_SHADOW_ETCHED_OUT,
                               NULL,
                               widget,
                               NULL,
                               otherArea.x, otherArea.y,
                               otherArea.width, otherArea.height );
#endif
        }
    }


    if ( gtk_widget_has_focus(widget) ) {
        gtk_widget_get_allocation (widget, &allocation);

#if GTK_CHECK_VERSION(3,0,0)
        gtk_render_focus(context,
                         cr,
                         0 + 1, 0 + 1,
                         allocation.width - 2, allocation.height - 2 );
#else
        gtk_paint_focus( style,
                         window,
                         GTK_STATE_NORMAL,
                         NULL, /* GdkRectangle *area, */
                         widget,
                         NULL,
                         0 + 1, 0 + 1,
                         allocation.width - 2, allocation.height - 2 );
#endif
    }

    return FALSE;
}


static gboolean eek_preview_enter_cb( GtkWidget* widget, GdkEventCrossing* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget ) {
        EekPreview        *preview = EEK_PREVIEW(widget);
        EekPreviewPrivate *priv    = EEK_PREVIEW_GET_PRIVATE(preview);

        priv->within = TRUE;
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_state_flags( widget, priv->hot ? GTK_STATE_FLAG_ACTIVE : GTK_STATE_FLAG_PRELIGHT, false );
#else
        gtk_widget_set_state( widget, priv->hot ? GTK_STATE_ACTIVE : GTK_STATE_PRELIGHT );
#endif
    }

    return FALSE;
}

static gboolean eek_preview_leave_cb( GtkWidget* widget, GdkEventCrossing* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget ) {
        EekPreview        *preview = EEK_PREVIEW(widget);
        EekPreviewPrivate *priv    = EEK_PREVIEW_GET_PRIVATE(preview);

        priv->within = FALSE;
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_state_flags( widget, GTK_STATE_FLAG_NORMAL, false );
#else
        gtk_widget_set_state( widget, GTK_STATE_NORMAL );
#endif
    }

    return FALSE;
}

static gboolean eek_preview_button_press_cb( GtkWidget* widget, GdkEventButton* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget )
    {
        EekPreview        *preview = EEK_PREVIEW(widget);
        EekPreviewPrivate *priv    = EEK_PREVIEW_GET_PRIVATE(preview);

        if ( priv->takesFocus && !gtk_widget_has_focus(widget) )
        {
            gtk_widget_grab_focus(widget);
        }

        if ( event->button == PRIME_BUTTON_MAGIC_NUMBER ||
             event->button == 2 )
        {
            priv->hot = TRUE;

            if ( priv->within )
            {
#if GTK_CHECK_VERSION(3,0,0)
                gtk_widget_set_state_flags( widget, GTK_STATE_FLAG_ACTIVE, false );
#else
                gtk_widget_set_state( widget, GTK_STATE_ACTIVE );
#endif
            }
        }
    }

    return FALSE;
}

static gboolean eek_preview_button_release_cb( GtkWidget* widget, GdkEventButton* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget ) {
        EekPreview        *preview = EEK_PREVIEW(widget);
        EekPreviewPrivate *priv    = EEK_PREVIEW_GET_PRIVATE(preview);

        priv->hot = FALSE;
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_state_flags( widget, GTK_STATE_FLAG_NORMAL, false );
#else
        gtk_widget_set_state( widget, GTK_STATE_NORMAL );
#endif

        if ( priv->within &&
            (event->button == PRIME_BUTTON_MAGIC_NUMBER || 
             event->button == 2))
        {
            gboolean isAlt = ( ((event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) ||
                                (event->button == 2));

            if ( isAlt )
            {
                g_signal_emit( widget, eek_preview_signals[ALTCLICKED_SIGNAL], 0, 2 );
            }
            else
            {
                g_signal_emit( widget, eek_preview_signals[CLICKED_SIGNAL], 0 );
            }
        }
    }

    return FALSE;
}

static void eek_preview_get_property( GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
    EekPreview        *preview   = EEK_PREVIEW( object );
    EekPreviewPrivate *priv      = EEK_PREVIEW_GET_PRIVATE(preview);

    switch (prop_id)
    {
        case PROP_FOCUS:
            g_value_set_boolean( value, priv->takesFocus );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
eek_preview_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    EekPreview        *preview   = EEK_PREVIEW( object );

    switch (prop_id)
    {
        case PROP_FOCUS:
            eek_preview_set_focus_on_click(preview, g_value_get_boolean(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}


static gboolean eek_preview_popup_menu( GtkWidget* widget )
{
    gboolean blip = parent_class->popup_menu ? parent_class->popup_menu(widget) : FALSE;
    return blip;
}


static void eek_preview_class_init( EekPreviewClass *klass )
{
    GObjectClass*   gobjClass   = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widgetClass = (GtkWidgetClass*)klass;

    gobjClass->set_property = eek_preview_set_property;
    gobjClass->get_property = eek_preview_get_property;

    parent_class = (GtkWidgetClass*)g_type_class_peek_parent( klass );

#if GTK_CHECK_VERSION(3,0,0)
    widgetClass->get_preferred_width = eek_preview_get_preferred_width;
    widgetClass->get_preferred_height = eek_preview_get_preferred_height;
    widgetClass->draw = eek_preview_draw;
#else
    widgetClass->size_request = eek_preview_size_request;
    widgetClass->expose_event = eek_preview_expose_event;
#endif

    widgetClass->button_press_event = eek_preview_button_press_cb;
    widgetClass->button_release_event = eek_preview_button_release_cb;
    widgetClass->enter_notify_event = eek_preview_enter_cb;
    widgetClass->leave_notify_event = eek_preview_leave_cb;

    /* For keybindings: */
    widgetClass->popup_menu = eek_preview_popup_menu;

    g_type_class_add_private(gobjClass, sizeof(EekPreviewPrivate));

    eek_preview_signals[CLICKED_SIGNAL] =
        g_signal_new( "clicked",
                      G_TYPE_FROM_CLASS( klass ),
                      (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET( EekPreviewClass, clicked ),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0 );

    eek_preview_signals[ALTCLICKED_SIGNAL] =
        g_signal_new( "alt-clicked",
                      G_TYPE_FROM_CLASS( klass ),
                      (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET( EekPreviewClass, clicked ),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT, G_TYPE_NONE,
                      1, G_TYPE_INT );


    g_object_class_install_property( gobjClass,
                                     PROP_FOCUS,
                                     g_param_spec_boolean(
                                         "focus-on-click",
                                         NULL,
                                         "flag to grab focus when clicked",
                                         TRUE,
                                         (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)
                                         )
                                   );
}

void
eek_preview_set_linked(EekPreview *preview,
                       LinkType    link)
{
    EekPreviewPrivate *priv = EEK_PREVIEW_GET_PRIVATE(preview);

    g_return_if_fail(IS_EEK_PREVIEW(preview));
    
    link = (LinkType)(link & PREVIEW_LINK_ALL);

    if (link != (LinkType)priv->linked)
    {
        priv->linked = link;

        gtk_widget_queue_draw(GTK_WIDGET(preview));
    }
}

LinkType
eek_preview_get_linked(EekPreview *preview)
{
    g_return_val_if_fail(IS_EEK_PREVIEW(preview), PREVIEW_LINK_NONE);

    return (LinkType)EEK_PREVIEW_GET_PRIVATE(preview)->linked;
}

gboolean
eek_preview_get_focus_on_click(EekPreview* preview)
{
    g_return_val_if_fail(IS_EEK_PREVIEW(preview), FALSE);

    return EEK_PREVIEW_GET_PRIVATE(preview)->takesFocus;
}

void
eek_preview_set_focus_on_click(EekPreview *preview,
                               gboolean    focus_on_click)
{
    EekPreviewPrivate *priv = EEK_PREVIEW_GET_PRIVATE(preview);

    g_return_if_fail(IS_EEK_PREVIEW(preview));

    if (focus_on_click != priv->takesFocus)
    {
        priv->takesFocus = focus_on_click;
    }
}

void
eek_preview_set_details(EekPreview   *preview,
                        ViewType      view,
                        PreviewSize   size,
                        guint         ratio,
                        guint         border)
{
    EekPreviewPrivate *priv = EEK_PREVIEW_GET_PRIVATE(preview);
    
    g_return_if_fail(IS_EEK_PREVIEW(preview));

    priv->view  = view;

    if ( size > PREVIEW_SIZE_LAST )
    {
        size = PREVIEW_SIZE_LAST;
    }

    priv->size = size;

    if ( ratio > PREVIEW_MAX_RATIO )
    {
        ratio = PREVIEW_MAX_RATIO;
    }

    priv->ratio  = ratio;
    priv->border = border;

    gtk_widget_queue_draw(GTK_WIDGET(preview));
}

static void
eek_preview_init(EekPreview *preview)
{
    GtkWidget         *widg = GTK_WIDGET(preview);
    EekPreviewPrivate *priv = EEK_PREVIEW_GET_PRIVATE(preview);

    gtk_widget_set_can_focus( widg, TRUE );
    gtk_widget_set_receives_default( widg, TRUE );

    gtk_widget_set_sensitive( widg, TRUE );

    gtk_widget_add_events(widg, GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_KEY_PRESS_MASK
                          | GDK_KEY_RELEASE_MASK
                          | GDK_FOCUS_CHANGE_MASK
                          | GDK_ENTER_NOTIFY_MASK
                          | GDK_LEAVE_NOTIFY_MASK );

    priv->r             = 0x80;
    priv->g             = 0x80;
    priv->b             = 0xcc;
    priv->scaledW       = 0;
    priv->scaledH       = 0;

    priv->hot           = FALSE;
    priv->within        = FALSE;
    priv->takesFocus    = FALSE;

    priv->view          = VIEW_TYPE_LIST;
    priv->size          = PREVIEW_SIZE_SMALL;
    priv->ratio         = 100;
    priv->border        = BORDER_NONE;
    priv->previewPixbuf = 0;
    priv->scaled        = 0;
}


GtkWidget* eek_preview_new(void)
{
    return GTK_WIDGET( g_object_new( EEK_PREVIEW_TYPE, NULL ) );
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

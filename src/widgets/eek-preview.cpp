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

#define FOCUS_PROP_ID 1

/* Keep in sycn with last value in eek-preview.h */
#define PREVIEW_SIZE_LAST PREVIEW_SIZE_HUGE
#define PREVIEW_SIZE_NEXTFREE (PREVIEW_SIZE_HUGE + 1)

#define PREVIEW_MAX_RATIO 500

static void eek_preview_class_init( EekPreviewClass *klass );
static void eek_preview_init( EekPreview *preview );
static gboolean eek_preview_draw(GtkWidget* widget, cairo_t* cr);

static GtkWidgetClass* parent_class = 0;

void eek_preview_set_color( EekPreview* preview, int r, int g, int b )
{
    preview->_r = r;
    preview->_g = g;
    preview->_b = b;

    gtk_widget_queue_draw(GTK_WIDGET(preview));
}


void eek_preview_set_pixbuf( EekPreview* preview, GdkPixbuf* pixbuf )
{
    preview->_previewPixbuf = pixbuf;

    gtk_widget_queue_draw(GTK_WIDGET(preview));

    if (preview->_scaled) {
        g_object_unref(preview->_scaled);
        preview->_scaled = 0;
    }
    preview->_scaledW = gdk_pixbuf_get_width(preview->_previewPixbuf);
    preview->_scaledH = gdk_pixbuf_get_height(preview->_previewPixbuf);
}


GType eek_preview_get_type(void)
{
    static GType preview_type = 0;

    if (!preview_type) {
      static const GTypeInfo preview_info = {
          sizeof( EekPreviewClass ),
          NULL, /* base_init */
          NULL, /* base_finalize */
          (GClassInitFunc)eek_preview_class_init,
          NULL, /* class_finalize */
          NULL, /* class_data */
          sizeof( EekPreview ),
          0,    /* n_preallocs */
          (GInstanceInitFunc)eek_preview_init,
          NULL /* value_table */
      };


      preview_type = g_type_register_static( GTK_TYPE_DRAWING_AREA, "EekPreview", &preview_info, (GTypeFlags)0 );
    }

    return preview_type;
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
    gint width = 0;
    gint height = 0;
    EekPreview* preview = EEK_PREVIEW(widget);

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

    width = sizeThings[preview->_size].width;
    height = sizeThings[preview->_size].height;

    if ( preview->_view == VIEW_TYPE_LIST ) {
        width *= 3;
    }

    if ( preview->_ratio != 100 ) {
        width = (width * preview->_ratio) / 100;
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

static gboolean eek_preview_draw(GtkWidget* widget, cairo_t* cr)
{
    GtkStyle* style = gtk_widget_get_style(widget);
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    EekPreview* preview = EEK_PREVIEW(widget);
    
    GdkColor fg = { 0,
        static_cast<guint16>(preview->_r),
        static_cast<guint16>(preview->_g),
        static_cast<guint16>(preview->_b)};

    gint insetTop = 0, insetBottom = 0;
    gint insetLeft = 0, insetRight = 0;

    if (preview->_border == BORDER_SOLID) {
        insetTop = 1;
        insetLeft = 1;
    }
    if (preview->_border == BORDER_SOLID_LAST_ROW) {
        insetTop = insetBottom = 1;
        insetLeft = 1;
    }
    if (preview->_border == BORDER_WIDE) {
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
	GdkWindow* window = gtk_widget_get_window(widget);
        
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
	if (preview->_border != BORDER_NONE) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
        cairo_fill(cr);
	}

    cairo_set_source_rgb(cr, preview->_r/65535.0, preview->_g/65535.0, preview->_b/65535.0 );
    cairo_rectangle(cr, insetLeft, insetTop, allocation.width - (insetLeft + insetRight), allocation.height - (insetTop + insetBottom));
    cairo_fill(cr);

	if ( preview->_previewPixbuf ) {
        GtkDrawingArea *da = &(preview->drawing);
	    GdkWindow *da_window = gtk_widget_get_window(GTK_WIDGET(da));
	    cairo_t *cr = gdk_cairo_create(da_window);

	    gint w = gdk_window_get_width(da_window);
	    gint h = gdk_window_get_height(da_window);

	    if ((w != preview->_scaledW) || (h != preview->_scaledH)) {
            if (preview->_scaled) {
                g_object_unref(preview->_scaled);
            }
            preview->_scaled = gdk_pixbuf_scale_simple(preview->_previewPixbuf, w - (insetLeft + insetRight), h - (insetTop + insetBottom), GDK_INTERP_BILINEAR);
            preview->_scaledW = w - (insetLeft + insetRight);
            preview->_scaledH = h - (insetTop + insetBottom);
        }

        GdkPixbuf *pix = (preview->_scaled) ? preview->_scaled : preview->_previewPixbuf;

        // Border
        if (preview->_border != BORDER_NONE) {
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
            cairo_fill(cr);
        }

	    gdk_cairo_set_source_pixbuf(cr, pix, insetLeft, insetTop);
	    cairo_paint(cr);
	    cairo_destroy(cr);
    }

        if ( preview->_linked ) {
            /* Draw arrow */
            GdkRectangle possible = {insetLeft, insetTop, (allocation.width - (insetLeft + insetRight)), (allocation.height - (insetTop + insetBottom)) };
            GdkRectangle area = {possible.x, possible.y, possible.width / 2, possible.height / 2 };

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


            if ( preview->_linked & PREVIEW_LINK_IN ) {
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

            if ( preview->_linked & PREVIEW_LINK_OUT ) {
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

            if ( preview->_linked & PREVIEW_LINK_OTHER ) {
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


            if ( preview->_linked & PREVIEW_FILL ) {
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

            if ( preview->_linked & PREVIEW_STROKE ) {
                GdkRectangle otherArea = {possible.x + (((possible.width * 3) / 4) - (area.width / 2)),
                                          area.y,
                                          area.width, area.height};
                if ( otherArea.height < possible.height ) {
                    otherArea.y = possible.y + (possible.height - otherArea.height) / 2;
                }
#if GTK_CHECK_VERSION(3,0,0)
                gtk_paint_diamond( style,
                                   cr,
                                   gtk_widget_get_state (widget),
                                   GTK_SHADOW_ETCHED_OUT,
                                   widget,
                                   NULL,
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
        EekPreview* preview = EEK_PREVIEW(widget);
        preview->_within = TRUE;
        gtk_widget_set_state( widget, preview->_hot ? GTK_STATE_ACTIVE : GTK_STATE_PRELIGHT );
    }
    return FALSE;
}

static gboolean eek_preview_leave_cb( GtkWidget* widget, GdkEventCrossing* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget ) {
        EekPreview* preview = EEK_PREVIEW(widget);
        preview->_within = FALSE;
        gtk_widget_set_state( widget, GTK_STATE_NORMAL );
    }
    return FALSE;
}

/*
static gboolean eek_preview_focus_in_event( GtkWidget* widget, GdkEventFocus* event )
{
    g_message("focus IN");
    gboolean blip = parent_class->focus_in_event ? parent_class->focus_in_event(widget, event) : FALSE;
    return blip;
}

static gboolean eek_preview_focus_out_event( GtkWidget* widget, GdkEventFocus* event )
{
    g_message("focus OUT");
    gboolean blip = parent_class->focus_out_event ? parent_class->focus_out_event(widget, event) : FALSE;
    return blip;
}
*/

static gboolean eek_preview_button_press_cb( GtkWidget* widget, GdkEventButton* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget ) {
        EekPreview* preview = EEK_PREVIEW(widget);

        if ( preview->_takesFocus && !gtk_widget_has_focus(widget) ) {
            gtk_widget_grab_focus(widget);
        }

        if ( event->button == PRIME_BUTTON_MAGIC_NUMBER ||
                event->button == 2 ) {
            preview->_hot = TRUE;
            if ( preview->_within ) {
                gtk_widget_set_state( widget, GTK_STATE_ACTIVE );
            }
        }
    }

    return FALSE;
}

static gboolean eek_preview_button_release_cb( GtkWidget* widget, GdkEventButton* event )
{
    if ( gtk_get_event_widget( (GdkEvent*)event ) == widget ) {
        EekPreview* preview = EEK_PREVIEW(widget);
        preview->_hot = FALSE;
        gtk_widget_set_state( widget, GTK_STATE_NORMAL );
        if ( preview->_within &&
            (event->button == PRIME_BUTTON_MAGIC_NUMBER || event->button == 2)) {
            gboolean isAlt = ( ((event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) ||
                                (event->button == 2));

            if ( isAlt ) {
                g_signal_emit( widget, eek_preview_signals[ALTCLICKED_SIGNAL], 0, 2 );
            } else {
                g_signal_emit( widget, eek_preview_signals[CLICKED_SIGNAL], 0 );
            }
        }
    }
    return FALSE;
}

static void eek_preview_get_property( GObject *object,
                                      guint property_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
    GObjectClass* gobjClass = G_OBJECT_CLASS(parent_class);
    switch ( property_id ) {
        case FOCUS_PROP_ID:
        {
            EekPreview* preview = EEK_PREVIEW( object );
            g_value_set_boolean( value, preview->_takesFocus );
        }
        break;
        default:
        {
            if ( gobjClass->get_property ) {
                gobjClass->get_property( object, property_id, value, pspec );
            }
        }
    }
}

static void eek_preview_set_property( GObject *object,
                                      guint property_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
    GObjectClass* gobjClass = G_OBJECT_CLASS(parent_class);
    switch ( property_id ) {
        case FOCUS_PROP_ID:
        {
            EekPreview* preview = EEK_PREVIEW( object );
            gboolean val = g_value_get_boolean( value );
            if ( val != preview->_takesFocus ) {
                preview->_takesFocus = val;
            }
        }
        break;
        default:
        {
            if ( gobjClass->set_property ) {
                gobjClass->set_property( object, property_id, value, pspec );
            }
        }
    }
}


static gboolean eek_preview_popup_menu( GtkWidget* widget )
{
/*     g_message("Do the popup!"); */
    gboolean blip = parent_class->popup_menu ? parent_class->popup_menu(widget) : FALSE;
    return blip;
}


static void eek_preview_class_init( EekPreviewClass *klass )
{
    GObjectClass* gobjClass = G_OBJECT_CLASS(klass);
    /*GtkObjectClass* objectClass = (GtkObjectClass*)klass;*/
    GtkWidgetClass* widgetClass = (GtkWidgetClass*)klass;

    gobjClass->set_property = eek_preview_set_property;
    gobjClass->get_property = eek_preview_get_property;

    /*objectClass->destroy = eek_preview_destroy;*/

    parent_class = (GtkWidgetClass*)g_type_class_peek_parent( klass );

    /*widgetClass->map = ;*/
    /*widgetClass->unmap = ;*/
    /*widgetClass->realize = ;*/
    /*widgetClass->unrealize = ;*/
#if GTK_CHECK_VERSION(3,0,0)
    widgetClass->get_preferred_width = eek_preview_get_preferred_width;
    widgetClass->get_preferred_height = eek_preview_get_preferred_height;
    widgetClass->draw = eek_preview_draw;
#else
    widgetClass->size_request = eek_preview_size_request;
    widgetClass->expose_event = eek_preview_expose_event;
#endif
    /*widgetClass->size_allocate = ;*/
    /*widgetClass->state_changed = ;*/
    /*widgetClass->style_set = ;*/
    /*widgetClass->grab_notify = ;*/

    widgetClass->button_press_event = eek_preview_button_press_cb;
    widgetClass->button_release_event = eek_preview_button_release_cb;
    /*widgetClass->delete_event = ;*/
    /*widgetClass->destroy_event = ;*/
/*     widgetClass->key_press_event = eek_preview_key_press_event; */
/*     widgetClass->key_release_event = eek_preview_key_release_event; */
    widgetClass->enter_notify_event = eek_preview_enter_cb;
    widgetClass->leave_notify_event = eek_preview_leave_cb;
    /*widgetClass->configure_event = ;*/
    /*widgetClass->focus_in_event = eek_preview_focus_in_event;*/
    /*widgetClass->focus_out_event = eek_preview_focus_out_event;*/

    /* selection */
    /*widgetClass->selection_get = ;*/
    /*widgetClass->selection_received = ;*/


    /* drag source: */
    /*widgetClass->drag_begin = ;*/
    /*widgetClass->drag_end = ;*/
    /*widgetClass->drag_data_get = ;*/
    /*widgetClass->drag_data_delete = ;*/

    /* drag target: */
    /*widgetClass->drag_leave = ;*/
    /*widgetClass->drag_motion = ;*/
    /*widgetClass->drag_drop = ;*/
    /*widgetClass->drag_data_received = ;*/

    /* For keybindings: */
    widgetClass->popup_menu = eek_preview_popup_menu;
    /*widgetClass->show_help = ;*/

    /* Accessibility support: */
    /*widgetClass->get_accessible = ;*/
    /*widgetClass->screen_changed = ;*/
    /*widgetClass->can_activate_accel = ;*/


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
                                     FOCUS_PROP_ID,
                                     g_param_spec_boolean(
                                         "focus-on-click",
                                         NULL,
                                         "flag to grab focus when clicked",
                                         TRUE,
                                         (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT)
                                         )
        );
}

void eek_preview_set_linked( EekPreview* splat, LinkType link )
{
    link = (LinkType)(link & PREVIEW_LINK_ALL);
    if ( link != (LinkType)splat->_linked ) {
        splat->_linked = link;

        gtk_widget_queue_draw( GTK_WIDGET(splat) );
    }
}

LinkType eek_preview_get_linked( EekPreview* splat )
{
    return (LinkType)splat->_linked;
}

gboolean eek_preview_get_focus_on_click( EekPreview* preview )
{
    return preview->_takesFocus;
}

void eek_preview_set_focus_on_click( EekPreview* preview, gboolean focus_on_click )
{
    if ( focus_on_click != preview->_takesFocus ) {
        preview->_takesFocus = focus_on_click;
    }
}

void eek_preview_set_details( EekPreview* preview, PreviewStyle prevstyle, ViewType view, PreviewSize size, guint ratio, guint border )
{
    preview->_prevstyle = prevstyle;
    preview->_view = view;

    if ( size > PREVIEW_SIZE_LAST ) {
        size = PREVIEW_SIZE_LAST;
    }
    preview->_size = size;

    if ( ratio > PREVIEW_MAX_RATIO ) {
        ratio = PREVIEW_MAX_RATIO;
    }
    preview->_ratio = ratio;
    preview->_border = border;
    gtk_widget_queue_draw(GTK_WIDGET(preview));
}

static void eek_preview_init( EekPreview *preview )
{
    GtkWidget* widg = GTK_WIDGET(preview);
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

/*    gtk_widget_add_events( widg, GDK_ALL_EVENTS_MASK );*/

    preview->_r = 0x80;
    preview->_g = 0x80;
    preview->_b = 0xcc;
    preview->_scaledW = 0;
    preview->_scaledH = 0;

    preview->_hot = FALSE;
    preview->_within = FALSE;
    preview->_takesFocus = FALSE;

    preview->_prevstyle = PREVIEW_STYLE_ICON;
    preview->_view = VIEW_TYPE_LIST;
    preview->_size = PREVIEW_SIZE_SMALL;
    preview->_ratio = 100;
    preview->_border = BORDER_NONE;
    preview->_previewPixbuf = 0;
    preview->_scaled = 0;

/*
    GdkColor color = {0};
    color.red = (255 << 8) | 255;

    GdkColor whack = {0};
    whack.green = (255 << 8) | 255;

    gtk_widget_modify_bg( widg, GTK_STATE_NORMAL, &color );
    gtk_widget_modify_bg( widg, GTK_STATE_PRELIGHT, &whack );
*/

/*   GTK_STATE_ACTIVE, */
/*   GTK_STATE_PRELIGHT, */
/*   GTK_STATE_SELECTED, */
/*   GTK_STATE_INSENSITIVE */

    if ( 0 ) {
        GdkColor color = {0,0,0,0};

        color.red = 0xffff;
        color.green = 0;
        color.blue = 0xffff;
#if !GTK_CHECK_VERSION(3,0,0)
        gdk_colormap_alloc_color( gdk_colormap_get_system(), &color, FALSE, TRUE );
#endif
        gtk_widget_modify_bg(widg, GTK_STATE_ACTIVE, &color);

        color.red = 0;
        color.green = 0xffff;
        color.blue = 0;
#if !GTK_CHECK_VERSION(3,0,0)
        gdk_colormap_alloc_color( gdk_colormap_get_system(), &color, FALSE, TRUE );
#endif
        gtk_widget_modify_bg(widg, GTK_STATE_SELECTED, &color);

        color.red = 0xffff;
        color.green = 0;
        color.blue = 0;
#if !GTK_CHECK_VERSION(3,0,0)
        gdk_colormap_alloc_color( gdk_colormap_get_system(), &color, FALSE, TRUE );
#endif
        gtk_widget_modify_bg( widg, GTK_STATE_PRELIGHT, &color );
    }
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

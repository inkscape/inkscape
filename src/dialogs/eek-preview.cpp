/** @file
 * @brief EEK preview stuff
 */
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

#include <gtk/gtk.h>
#include "eek-preview.h"
#include "path-prefix.h"

#define PRIME_BUTTON_MAGIC_NUMBER 1

#define FOCUS_PROP_ID 1

/* Keep in sycn with last value in eek-preview.h */
#define PREVIEW_SIZE_LAST PREVIEW_SIZE_HUGE
#define PREVIEW_SIZE_NEXTFREE (PREVIEW_SIZE_HUGE + 1)

#define PREVIEW_MAX_RATIO 500

static void eek_preview_class_init( EekPreviewClass *klass );
static void eek_preview_init( EekPreview *preview );

static GtkWidgetClass* parent_class = 0;

void eek_preview_set_color( EekPreview* preview, int r, int g, int b )
{
    preview->_r = r;
    preview->_g = g;
    preview->_b = b;

    gtk_widget_queue_draw(GTK_WIDGET(preview));
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

enum {
  CLICKED_SIGNAL,
  ALTCLICKED_SIGNAL,
  LAST_SIGNAL
};


static guint eek_preview_signals[LAST_SIGNAL] = { 0 };


gboolean eek_preview_expose_event( GtkWidget* widget, GdkEventExpose* event )
{
/*     g_message("Exposed!!!   %s", GTK_WIDGET_HAS_FOCUS(widget) ? "XXX" : "---" ); */
    gint insetX = 0;
    gint insetY = 0;

    (void)event;
/*
    gint lower = widget->allocation.width;
    lower = (widget->allocation.height < lower) ? widget->allocation.height : lower;
    if ( lower > 16 ) {
        insetX++;
        if ( lower > 18 ) {
            insetX++;
            if ( lower > 22 ) {
                insetX++;
                if ( lower > 24 ) {
                    insetX++;
                    if ( lower > 32 ) {
                        insetX++;
                    }
                }
            }
        }
        insetY = insetX;
    }
*/

    if ( GTK_WIDGET_DRAWABLE( widget ) ) {
        GtkStyle* style = gtk_widget_get_style( widget );

        if ( insetX > 0 || insetY > 0 ) {
            gtk_paint_flat_box( style,
                                widget->window,
                                (GtkStateType)GTK_WIDGET_STATE(widget),
                                GTK_SHADOW_NONE,
                                NULL,
                                widget,
                                NULL,
                                0, 0,
                                widget->allocation.width, widget->allocation.height);
        }

        GdkGC *gc = gdk_gc_new( widget->window );
        EekPreview* preview = EEK_PREVIEW(widget);
        GdkColor fg = {0, preview->_r, preview->_g, preview->_b};

        gdk_colormap_alloc_color( gdk_colormap_get_system(), &fg, FALSE, TRUE );

        gdk_gc_set_foreground( gc, &fg );

        gdk_draw_rectangle( widget->window,
                            gc,
                            TRUE,
                            insetX, insetY,
                            widget->allocation.width - (insetX * 2), widget->allocation.height - (insetY * 2) );

        if ( preview->_linked ) {
            /* Draw arrow */
            GdkRectangle possible = {insetX, insetY, (widget->allocation.width - (insetX * 2)), (widget->allocation.height - (insetY * 2)) };
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
                gtk_paint_arrow( style,
                                 widget->window,
                                 (GtkStateType)widget->state,
                                 GTK_SHADOW_ETCHED_IN,
                                 NULL, /* clip area.  &area, */
                                 widget, /* may be NULL */
                                 NULL, /* detail */
                                 GTK_ARROW_DOWN,
                                 FALSE,
                                 area.x, area.y,
                                 area.width, area.height
                                 );
            }

            if ( preview->_linked & PREVIEW_LINK_OUT ) {
                GdkRectangle otherArea = {area.x, area.y, area.width, area.height};
                if ( otherArea.height < possible.height ) {
                    otherArea.y = possible.y + (possible.height - otherArea.height);
                }

                gtk_paint_arrow( style,
                                 widget->window,
                                 (GtkStateType)widget->state,
                                 GTK_SHADOW_ETCHED_OUT,
                                 NULL, /* clip area.  &area, */
                                 widget, /* may be NULL */
                                 NULL, /* detail */
                                 GTK_ARROW_UP,
                                 FALSE,
                                 otherArea.x, otherArea.y,
                                 otherArea.width, otherArea.height
                                 );
            }

            if ( preview->_linked & PREVIEW_LINK_OTHER ) {
                GdkRectangle otherArea = {insetX, area.y, area.width, area.height};
                if ( otherArea.height < possible.height ) {
                    otherArea.y = possible.y + (possible.height - otherArea.height) / 2;
                }

                gtk_paint_arrow( style,
                                 widget->window,
                                 (GtkStateType)widget->state,
                                 GTK_SHADOW_ETCHED_OUT,
                                 NULL, /* clip area.  &area, */
                                 widget, /* may be NULL */
                                 NULL, /* detail */
                                 GTK_ARROW_LEFT,
                                 FALSE,
                                 otherArea.x, otherArea.y,
                                 otherArea.width, otherArea.height
                                 );
            }
        }

        if (preview->_isRemove){
            GtkDrawingArea* da = &(preview->drawing);
            GdkDrawable* drawable = (GdkDrawable*) (((GtkWidget*)da)->window);
            gint w,h;
            gdk_drawable_get_size(drawable, &w, &h);
            GError *error = NULL;
            gchar *filepath = (gchar *) g_strdup_printf("%s/remove-color.png", INKSCAPE_PIXMAPDIR);
            gsize bytesRead = 0;
            gsize bytesWritten = 0;
            gchar *localFilename = g_filename_from_utf8( filepath,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_scale(localFilename, w, h, FALSE, &error);
            gdk_draw_pixbuf(drawable, 0, pixbuf, 0, 0, 0, 0, w, h, GDK_RGB_DITHER_NONE, 0, 0);
            g_free(localFilename);
            g_free(filepath);
        }


        if ( GTK_WIDGET_HAS_FOCUS(widget) ) {
            gtk_paint_focus( style,
                             widget->window,
                             GTK_STATE_NORMAL,
                             NULL, /* GdkRectangle *area, */
                             widget,
                             NULL,
                             0 + 1, 0 + 1,
                             widget->allocation.width - 2, widget->allocation.height - 2 );
        }
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

        if ( preview->_takesFocus && !GTK_WIDGET_HAS_FOCUS(widget) ) {
            gtk_widget_grab_focus(widget);
        }

        if ( event->button == PRIME_BUTTON_MAGIC_NUMBER ) {
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
        if ( preview->_within && event->button == PRIME_BUTTON_MAGIC_NUMBER ) {
            gboolean isAlt = (event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK;

            if ( isAlt ) {
                g_signal_emit( widget, eek_preview_signals[ALTCLICKED_SIGNAL], 0, 2 );
            } else {
                g_signal_emit( widget, eek_preview_signals[CLICKED_SIGNAL], 0 );
            }
        }
    }
    return FALSE;
}

gboolean eek_preview_key_press_event( GtkWidget* widget, GdkEventKey* event)
{
    (void)widget;
    (void)event;
    g_message("TICK");
    return FALSE;
}

gboolean eek_preview_key_release_event( GtkWidget* widget, GdkEventKey* event)
{
    (void)widget;
    (void)event;
    g_message("tock");
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
    widgetClass->size_request = eek_preview_size_request;
    /*widgetClass->size_allocate = ;*/
    /*widgetClass->state_changed = ;*/
    /*widgetClass->style_set = ;*/
    /*widgetClass->grab_notify = ;*/

    widgetClass->button_press_event = eek_preview_button_press_cb;
    widgetClass->button_release_event = eek_preview_button_release_cb;
    /*widgetClass->delete_event = ;*/
    /*widgetClass->destroy_event = ;*/
    widgetClass->expose_event = eek_preview_expose_event;
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

void eek_preview_set_details( EekPreview* preview, PreviewStyle prevstyle, ViewType view, PreviewSize size, guint ratio )
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

    gtk_widget_queue_draw(GTK_WIDGET(preview));
}

static void eek_preview_init( EekPreview *preview )
{
    GtkWidget* widg = GTK_WIDGET(preview);
    GTK_WIDGET_SET_FLAGS( widg, GTK_CAN_FOCUS );
    GTK_WIDGET_SET_FLAGS( widg, GTK_RECEIVES_DEFAULT );

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

    preview->_hot = FALSE;
    preview->_within = FALSE;
    preview->_takesFocus = FALSE;
    preview->_isRemove = FALSE;

    preview->_prevstyle = PREVIEW_STYLE_ICON;
    preview->_view = VIEW_TYPE_LIST;
    preview->_size = PREVIEW_SIZE_SMALL;
    preview->_ratio = 100;

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
        gdk_colormap_alloc_color( gdk_colormap_get_system(), &color, FALSE, TRUE );
        gtk_widget_modify_bg(widg, GTK_STATE_ACTIVE, &color);

        color.red = 0;
        color.green = 0xffff;
        color.blue = 0;
        gdk_colormap_alloc_color( gdk_colormap_get_system(), &color, FALSE, TRUE );
        gtk_widget_modify_bg(widg, GTK_STATE_SELECTED, &color);

        color.red = 0xffff;
        color.green = 0;
        color.blue = 0;
        gdk_colormap_alloc_color( gdk_colormap_get_system(), &color, FALSE, TRUE );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

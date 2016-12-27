#include "ink-action.h"
#include "widgets/icon.h"

#include <gtk/gtk.h>

static void ink_action_finalize( GObject* obj );
static void ink_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec );
static void ink_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec );

static GtkWidget* ink_action_create_menu_item( GtkAction* action );
static GtkWidget* ink_action_create_tool_item( GtkAction* action );

struct _InkActionPrivate
{
    gchar* iconId;
    Inkscape::IconSize iconSize;
};

#define INK_ACTION_GET_PRIVATE( o ) ( G_TYPE_INSTANCE_GET_PRIVATE( (o), INK_ACTION_TYPE, InkActionPrivate ) )

G_DEFINE_TYPE(InkAction, ink_action, GTK_TYPE_ACTION);

enum {
    PROP_INK_ID = 1,
    PROP_INK_SIZE
};

static void ink_action_class_init( InkActionClass* klass )
{
    if ( klass ) {
        GObjectClass * objClass = G_OBJECT_CLASS( klass );

        objClass->finalize = ink_action_finalize;
        objClass->get_property = ink_action_get_property;
        objClass->set_property = ink_action_set_property;

        klass->parent_class.create_menu_item = ink_action_create_menu_item;
        klass->parent_class.create_tool_item = ink_action_create_tool_item;
        /*klass->parent_class.connect_proxy = connect_proxy;*/
        /*klass->parent_class.disconnect_proxy = disconnect_proxy;*/

        g_object_class_install_property( objClass,
                                         PROP_INK_ID,
                                         g_param_spec_string( "iconId",
                                                              "Icon ID",
                                                              "The id for the icon",
                                                              "",
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_INK_SIZE,
                                         g_param_spec_int( "iconSize",
                                                           "Icon Size",
                                                           "The size the icon",
                                                           (int)Inkscape::ICON_SIZE_MENU,
                                                           (int)Inkscape::ICON_SIZE_DECORATION,
                                                           (int)Inkscape::ICON_SIZE_SMALL_TOOLBAR,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_type_class_add_private( klass, sizeof(InkActionClass) );
    }
}

static void ink_action_init( InkAction* action )
{
    action->private_data = INK_ACTION_GET_PRIVATE( action );
    action->private_data->iconId = 0;
    action->private_data->iconSize = Inkscape::ICON_SIZE_SMALL_TOOLBAR;
}

static void ink_action_finalize( GObject* obj )
{
    InkAction* action = INK_ACTION( obj );

    g_free( action->private_data->iconId );
    g_free( action->private_data );

}

//Any strings passed in should already be localised
InkAction* ink_action_new( const gchar *name,
                           const gchar *label,
                           const gchar *tooltip,
                           const gchar *inkId,
                           Inkscape::IconSize size )
{
    GObject* obj = (GObject*)g_object_new( INK_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "iconId", inkId,
                                           "iconSize", size,
                                           NULL );

    InkAction* action = INK_ACTION( obj );

    return action;
}

static void ink_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec )
{
    InkAction* action = INK_ACTION( obj );
    (void)action;
    switch ( propId ) {
        case PROP_INK_ID:
        {
            g_value_set_string( value, action->private_data->iconId );
        }
        break;

        case PROP_INK_SIZE:
        {
            g_value_set_int( value, action->private_data->iconSize );
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

void ink_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec )
{
    InkAction* action = INK_ACTION( obj );
    (void)action;
    switch ( propId ) {
        case PROP_INK_ID:
        {
            gchar* tmp = action->private_data->iconId;
            action->private_data->iconId = g_value_dup_string( value );
            g_free( tmp );
        }
        break;

        case PROP_INK_SIZE:
        {
            action->private_data->iconSize = (Inkscape::IconSize)g_value_get_int( value );
        }
        break;

        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
        }
    }
}

static GtkWidget* ink_action_create_menu_item( GtkAction* action )
{
    InkAction* act = INK_ACTION( action );
    GtkWidget* item = 0;

    if ( act->private_data->iconId ) {
        gchar* label = 0;
        g_object_get( G_OBJECT(act), "label", &label, NULL );
        item = gtk_image_menu_item_new_with_mnemonic( label );

        GtkWidget* child = sp_icon_new( Inkscape::ICON_SIZE_MENU, act->private_data->iconId );
        // TODO this work-around is until SPIcon will live properly inside of a popup menu
        if ( SP_IS_ICON(child) ) {
            SPIcon* icon = SP_ICON(child);
            sp_icon_fetch_pixbuf( icon );
            GdkPixbuf* target = icon->pb;
            if ( target ) {
                child = gtk_image_new_from_pixbuf( target );
                gtk_widget_set_sensitive(child, gtk_action_is_sensitive(action));
                gtk_widget_destroy( GTK_WIDGET(icon) );
            }
        }
        gtk_widget_show_all( child );
        gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(item), child );

        g_free( label );
        label = 0;
    } else {
        item = GTK_ACTION_CLASS(ink_action_parent_class)->create_menu_item( action );
    }

    return item;
}

static GtkWidget* ink_action_create_tool_item( GtkAction* action )
{
    InkAction* act = INK_ACTION( action );
    GtkWidget* item = GTK_ACTION_CLASS(ink_action_parent_class)->create_tool_item(action);

    if ( act->private_data->iconId ) {
        if ( GTK_IS_TOOL_BUTTON(item) ) {
            GtkToolButton* button = GTK_TOOL_BUTTON(item);

            GtkWidget* child = sp_icon_new( act->private_data->iconSize, act->private_data->iconId );
            gtk_tool_button_set_icon_widget( button, child );
        } else {
            // For now trigger a warning but don't do anything else
            GtkToolButton* button = GTK_TOOL_BUTTON(item);
            (void)button;
        }
    }

    // TODO investigate if needed
    gtk_widget_show_all( item );

    return item;
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

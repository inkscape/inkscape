


#include <glib/gi18n.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtktoolitem.h>
#include <gtk/gtktoggletoolbutton.h>
#include <gtk/gtkcheckmenuitem.h>

#include "icon-size.h"
#include "ink-action.h"

#include "widgets/button.h"
#include "widgets/icon.h"



static void ink_action_class_init( InkActionClass* klass );
static void ink_action_init( InkAction* action );
static void ink_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec );
static void ink_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec );

static GtkWidget* ink_action_create_menu_item( GtkAction* action );
static GtkWidget* ink_action_create_tool_item( GtkAction* action );

static GtkActionClass* gInkActionParentClass = 0;

GType ink_action_get_type( void )
{
    static GType myType = 0;
    if ( !myType ) {
        static const GTypeInfo myInfo = {
            sizeof( InkActionClass ),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)ink_action_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof( InkAction ),
            0, /* n_preallocs */
            (GInstanceInitFunc)ink_action_init,
            NULL
        };

        myType = g_type_register_static( GTK_TYPE_ACTION, "InkAction", &myInfo, (GTypeFlags)0 );
    }

    return myType;
}

enum {
    PROP_INK_ID = 1,
    PROP_INK_SIZE
};

static void ink_action_class_init( InkActionClass* klass )
{
    if ( klass ) {
        gInkActionParentClass = GTK_ACTION_CLASS( g_type_class_peek_parent( klass ) );
        GObjectClass * objClass = G_OBJECT_CLASS( klass );

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
    }
}

static void ink_action_init( InkAction* action )
{
}

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
            //g_value_set_pointer( value, action->private_data->adj );
        }
        break;

        case PROP_INK_SIZE:
        {
            //g_value_set_pointer( value, action->private_data->adj );
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
// 	    gchar* tmp = action->private_data->iconId;
// 	    action->private_data->iconId = g_value_dup_string( value );
// 	    g_free( tmp );
        }
        break;

        case PROP_INK_SIZE:
        {
// 	    action->private_data->iconSize = g_value_get_( value );
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

static GtkWidget* ink_action_create_menu_item( GtkAction* action )
{
    GtkWidget* item = 0;
    g_message("INK ACTION CREATE MENU ITEM");
    item = gInkActionParentClass->create_menu_item( action );
    return item;
}

static GtkWidget* ink_action_create_tool_item( GtkAction* action )
{
    GtkWidget* item = 0;
    g_message("INK ACTION CREATE TOOL ITEM");


    //item = gInkActionParentClass->create_tool_item( action );
    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                                 SP_BUTTON_TYPE_NORMAL,
                                                 NULL,
                                                 "use_pressure",
                                                 _("Use the pressure of the input device to alter the width of the pen"),
                                                 tt);
    //g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (sp_ddc_pressure_state_changed), NULL);
    //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("tools.calligraphic", "usepressure", 1));
    item = GTK_WIDGET(gtk_tool_item_new());
    gtk_container_add( GTK_CONTAINER(item), button );

    gtk_widget_show_all( item );

    return item;
}



/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */


static void ink_toggle_action_class_init( InkToggleActionClass* klass );
static void ink_toggle_action_init( InkToggleAction* action );
static void ink_toggle_action_finalize( GObject* obj );
static void ink_toggle_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec );
static void ink_toggle_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec );

static GtkWidget* ink_toggle_action_create_menu_item( GtkAction* action );
static GtkWidget* ink_toggle_action_create_tool_item( GtkAction* action );

static GtkToggleActionClass* gInkToggleActionParentClass = 0;

struct _InkToggleActionPrivate
{
    gchar* iconId;
    Inkscape::IconSize iconSize;
};

#define INK_TOGGLE_ACTION_GET_PRIVATE( o ) ( G_TYPE_INSTANCE_GET_PRIVATE( (o), INK_TOGGLE_ACTION_TYPE, InkToggleActionPrivate ) )

GType ink_toggle_action_get_type( void )
{
    static GType myType = 0;
    if ( !myType ) {
        static const GTypeInfo myInfo = {
            sizeof( InkToggleActionClass ),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)ink_toggle_action_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof( InkToggleAction ),
            0, /* n_preallocs */
            (GInstanceInitFunc)ink_toggle_action_init,
            NULL
        };

        myType = g_type_register_static( GTK_TYPE_TOGGLE_ACTION, "InkToggleAction", &myInfo, (GTypeFlags)0 );
    }

    return myType;
}


static void ink_toggle_action_class_init( InkToggleActionClass* klass )
{
    if ( klass ) {
        gInkToggleActionParentClass = GTK_TOGGLE_ACTION_CLASS( g_type_class_peek_parent( klass ) );
        GObjectClass * objClass = G_OBJECT_CLASS( klass );

	objClass->finalize = ink_toggle_action_finalize;
        objClass->get_property = ink_toggle_action_get_property;
        objClass->set_property = ink_toggle_action_set_property;

        klass->parent_class.parent_class.create_menu_item = ink_toggle_action_create_menu_item;
        klass->parent_class.parent_class.create_tool_item = ink_toggle_action_create_tool_item;
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

        g_type_class_add_private( klass, sizeof(InkToggleActionClass) );
    }
}

static void ink_toggle_action_init( InkToggleAction* action )
{
    action->private_data = INK_TOGGLE_ACTION_GET_PRIVATE( action );
    action->private_data->iconId = 0;
    action->private_data->iconSize = Inkscape::ICON_SIZE_SMALL_TOOLBAR;
}

static void ink_toggle_action_finalize( GObject* obj )
{
    InkToggleAction* action = INK_TOGGLE_ACTION( obj );

    g_free( action->private_data->iconId );
    g_free( action->private_data );

}

InkToggleAction* ink_toggle_action_new( const gchar *name,
                           const gchar *label,
                           const gchar *tooltip,
                           const gchar *inkId,
                           Inkscape::IconSize size )
{
    GObject* obj = (GObject*)g_object_new( INK_TOGGLE_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "iconId", inkId,
                                           "iconSize", size,
                                           NULL );

    InkToggleAction* action = INK_TOGGLE_ACTION( obj );

    return action;
}

static void ink_toggle_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec )
{
    InkToggleAction* action = INK_TOGGLE_ACTION( obj );
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

void ink_toggle_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec )
{
    InkToggleAction* action = INK_TOGGLE_ACTION( obj );
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

static GtkWidget* ink_toggle_action_create_menu_item( GtkAction* action )
{
    GtkWidget* item = gInkToggleActionParentClass->parent_class.create_menu_item(action);

    return item;
}

static GtkWidget* ink_toggle_action_create_tool_item( GtkAction* action )
{
    InkToggleAction* act = INK_TOGGLE_ACTION( action );
    GtkWidget* item = gInkToggleActionParentClass->parent_class.create_tool_item(action);

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

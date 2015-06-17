#ifndef SEEN_INK_ACTION
#define SEEN_INK_ACTION


#include <gtk/gtk.h>
#include "icon-size.h"
#include "attributes.h"

/* Equivalent to GTK Actions of the same type, but can support Inkscape SVG icons */

G_BEGIN_DECLS

#define INK_ACTION_TYPE                ( ink_action_get_type() )
#define INK_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), INK_ACTION_TYPE, InkAction) )
#define INK_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), INK_ACTION_TYPE, InkActionClass) )
#define IS_INK_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), INK_ACTION_TYPE) )
#define IS_INK_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), INK_ACTION_TYPE) )
#define INK_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), INK_ACTION_TYPE, InkActionClass) )

typedef struct _InkAction      InkAction;
typedef struct _InkActionClass InkActionClass;
typedef struct _InkActionPrivate InkActionPrivate;

struct _InkAction
{
    GtkAction action;
    InkActionPrivate *private_data;
};

struct _InkActionClass
{
    GtkActionClass parent_class;
};

GType ink_action_get_type( void );

InkAction* ink_action_new( const gchar *name,
                           const gchar *label,
                           const gchar *tooltip,
                           const gchar *inkId,
                           Inkscape::IconSize size );


/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */


#define INK_TOGGLE_ACTION_TYPE                ( ink_toggle_action_get_type() )
#define INK_TOGGLE_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), INK_TOGGLE_ACTION_TYPE, InkToggleAction) )
#define INK_TOGGLE_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), INK_TOGGLE_ACTION_TYPE, InkToggleActionClass) )
#define IS_INK_TOGGLE_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), INK_TOGGLE_ACTION_TYPE) )
#define IS_INK_TOGGLE_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), INK_TOGGLE_ACTION_TYPE) )
#define INK_TOGGLE_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), INK_TOGGLE_ACTION_TYPE, InkToggleActionClass) )

typedef struct _InkToggleAction      InkToggleAction;
typedef struct _InkToggleActionClass InkToggleActionClass;
typedef struct _InkToggleActionPrivate InkToggleActionPrivate;

struct _InkToggleAction
{
    GtkToggleAction action;
    InkToggleActionPrivate *private_data;
};

struct _InkToggleActionClass
{
    GtkToggleActionClass parent_class;
};

GType ink_toggle_action_get_type( void );

InkToggleAction* ink_toggle_action_new( const gchar *name,
                                        const gchar *label,
                                        const gchar *tooltip,
                                        const gchar *inkId,
                                        Inkscape::IconSize size,
                                        SPAttributeEnum attr = SP_ATTR_INVALID);


/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */


#define INK_RADIO_ACTION_TYPE                ( ink_radio_action_get_type() )
#define INK_RADIO_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), INK_RADIO_ACTION_TYPE, InkRadioAction) )
#define INK_RADIO_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), INK_RADIO_ACTION_TYPE, InkRadioActionClass) )
#define IS_INK_RADIO_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), INK_RADIO_ACTION_TYPE) )
#define IS_INK_RADIO_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), INK_RADIO_ACTION_TYPE) )
#define INK_RADIO_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), INK_RADIO_ACTION_TYPE, InkRadioActionClass) )

typedef struct _InkRadioAction      InkRadioAction;
typedef struct _InkRadioActionClass InkRadioActionClass;
typedef struct _InkRadioActionPrivate InkRadioActionPrivate;

struct _InkRadioAction
{
    GtkRadioAction action;
    InkRadioActionPrivate *private_data;
};

struct _InkRadioActionClass
{
    GtkRadioActionClass parent_class;
};

GType ink_radio_action_get_type( void );

InkRadioAction* ink_radio_action_new( const gchar *name,
                                      const gchar *label,
                                      const gchar *tooltip,
                                      const gchar *inkId,
                                      Inkscape::IconSize size );


/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

// ToolMenu Action is happily derived from http://www.gtkforums.com/viewtopic.php?t=4215

#define INK_TOOL_MENU_ACTION_TYPE                ( ink_tool_menu_action_get_type() )
#define INK_TOOL_MENU_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), INK_TOOL_MENU_ACTION_TYPE, InkToolMenuAction) )
#define INK_TOOL_MENU_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass),  INK_TOOL_MENU_ACTION_TYPE, InkToolMenuActionClass) )
#define IS_INK_TOOL_MENU_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), INK_TOOL_MENU_ACTION_TYPE) )
#define IS_INK_TOOL_MENU_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass),  INK_TOOL_MENU_ACTION_TYPE) )
#define INK_TOOL_MENU_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj),  INK_TOOL_MENU_ACTION_TYPE, InkToolMenuActionClass) )

typedef struct _InkToolMenuAction        InkToolMenuAction;
typedef struct _InkToolMenuActionClass   InkToolMenuActionClass;
typedef struct _InkToolMenuActionPrivate InkToolMenuActionPrivate;

struct _InkToolMenuAction
{
    InkAction action;
};

struct _InkToolMenuActionClass
{
    InkActionClass parent_class;
};

GType ink_tool_menu_action_get_type( void );

InkToolMenuAction* ink_tool_menu_action_new( const gchar *name,
                                             const gchar *label,
                                             const gchar *tooltip,
                                             const gchar *inkId,
                                             Inkscape::IconSize size );



G_END_DECLS

#endif /* SEEN_INK_ACTION */

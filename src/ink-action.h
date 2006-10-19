#ifndef SEEN_INK_ACTION
#define SEEN_INK_ACTION


#include <glib.h>
#include <gtk/gtkaction.h>
#include <gtk/gtktoggleaction.h>
#include <glib-object.h>

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
					Inkscape::IconSize size );



G_END_DECLS

#endif /* SEEN_INK_ACTION */



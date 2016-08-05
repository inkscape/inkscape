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


G_END_DECLS

#endif /* SEEN_INK_ACTION */
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

#ifndef INK_TOGGLE_ACTION_H
#define INK_TOGGLE_ACTION_H

#include <gtk/gtk.h>

#include "attributes.h"
#include "icon-size.h"

G_BEGIN_DECLS
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

G_END_DECLS

#endif // INK_TOGGLE_ACTION_H
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

#ifndef INK_RADIO_ACTION_H
#define INK_RADIO_ACTION_H

#include <gtk/gtk.h>

#include "icon.h"

G_BEGIN_DECLS

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

G_END_DECLS

#endif // INK_RADIO_ACTION_H
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

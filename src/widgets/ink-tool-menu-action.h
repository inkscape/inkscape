#ifndef INK_TOOL_MENU_ACTION_H
#define INK_TOOL_MENU_ACTION_H

#include "ink-action.h"

// ToolMenu Action is happily derived from http://www.gtkforums.com/viewtopic.php?t=4215

G_BEGIN_DECLS

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

#endif // INK_TOOL_MENU_ACTION_H

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

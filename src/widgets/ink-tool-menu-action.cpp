#include "ink-tool-menu-action.h"

// ToolMenu Action is happily derived from http://www.gtkforums.com/viewtopic.php?t=4215

G_DEFINE_TYPE(InkToolMenuAction, ink_tool_menu_action, INK_ACTION_TYPE);

static void
ink_tool_menu_action_class_init (InkToolMenuActionClass *klass)
{
   GtkActionClass *action_class = GTK_ACTION_CLASS (klass);
   action_class->toolbar_item_type = GTK_TYPE_MENU_TOOL_BUTTON;
}

static void
ink_tool_menu_action_init (InkToolMenuAction* /*tma*/)
{
}

InkToolMenuAction *
ink_tool_menu_action_new (const gchar *name,
                          const gchar *label,
                          const gchar *tooltip,
                          const gchar *inkId,
                          Inkscape::IconSize size )
{
    g_return_val_if_fail (name != NULL, NULL);

    GObject* obj = (GObject*)g_object_new( INK_TOOL_MENU_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "iconId", inkId,
                                           "iconSize", size,
                                           NULL );

    InkToolMenuAction* action = INK_TOOL_MENU_ACTION( obj );

    return action;
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

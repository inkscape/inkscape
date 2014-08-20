


#include <glib-object.h>
#include "libgdltypebuiltins.h"


/* enumerations from "gdl-dock-object.h" */
static const GFlagsValue _gdl_dock_param_flags_values[] = {
  { GDL_DOCK_PARAM_EXPORT, "GDL_DOCK_PARAM_EXPORT", "export" },
  { GDL_DOCK_PARAM_AFTER, "GDL_DOCK_PARAM_AFTER", "after" },
  { 0, NULL, NULL }
};

GType
gdl_dock_param_flags_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_flags_register_static ("GdlDockParamFlags", _gdl_dock_param_flags_values);

  return type;
}

static const GFlagsValue _gdl_dock_object_flags_values[] = {
  { GDL_DOCK_AUTOMATIC, "GDL_DOCK_AUTOMATIC", "automatic" },
  { GDL_DOCK_ATTACHED, "GDL_DOCK_ATTACHED", "attached" },
  { GDL_DOCK_IN_REFLOW, "GDL_DOCK_IN_REFLOW", "in-reflow" },
  { GDL_DOCK_IN_DETACH, "GDL_DOCK_IN_DETACH", "in-detach" },
  { 0, NULL, NULL }
};

GType
gdl_dock_object_flags_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_flags_register_static ("GdlDockObjectFlags", _gdl_dock_object_flags_values);

  return type;
}

static const GEnumValue _gdl_dock_placement_values[] = {
  { GDL_DOCK_NONE, "GDL_DOCK_NONE", "none" },
  { GDL_DOCK_TOP, "GDL_DOCK_TOP", "top" },
  { GDL_DOCK_BOTTOM, "GDL_DOCK_BOTTOM", "bottom" },
  { GDL_DOCK_RIGHT, "GDL_DOCK_RIGHT", "right" },
  { GDL_DOCK_LEFT, "GDL_DOCK_LEFT", "left" },
  { GDL_DOCK_CENTER, "GDL_DOCK_CENTER", "center" },
  { GDL_DOCK_FLOATING, "GDL_DOCK_FLOATING", "floating" },
  { 0, NULL, NULL }
};

GType
gdl_dock_placement_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GdlDockPlacement", _gdl_dock_placement_values);

  return type;
}


/* enumerations from "gdl-dock-master.h" */
static const GEnumValue _gdl_switcher_style_values[] = {
  { GDL_SWITCHER_STYLE_TEXT, "GDL_SWITCHER_STYLE_TEXT", "text" },
  { GDL_SWITCHER_STYLE_ICON, "GDL_SWITCHER_STYLE_ICON", "icon" },
  { GDL_SWITCHER_STYLE_BOTH, "GDL_SWITCHER_STYLE_BOTH", "both" },
  { GDL_SWITCHER_STYLE_TOOLBAR, "GDL_SWITCHER_STYLE_TOOLBAR", "toolbar" },
  { GDL_SWITCHER_STYLE_TABS, "GDL_SWITCHER_STYLE_TABS", "tabs" },
  { GDL_SWITCHER_STYLE_NONE, "GDL_SWITCHER_STYLE_NONE", "none" },
  { 0, NULL, NULL }
};

GType
gdl_switcher_style_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GdlSwitcherStyle", _gdl_switcher_style_values);

  return type;
}


/* enumerations from "gdl-dock-item.h" */
static const GFlagsValue _gdl_dock_item_behavior_values[] = {
  { GDL_DOCK_ITEM_BEH_NORMAL, "GDL_DOCK_ITEM_BEH_NORMAL", "normal" },
  { GDL_DOCK_ITEM_BEH_NEVER_FLOATING, "GDL_DOCK_ITEM_BEH_NEVER_FLOATING", "never-floating" },
  { GDL_DOCK_ITEM_BEH_NEVER_VERTICAL, "GDL_DOCK_ITEM_BEH_NEVER_VERTICAL", "never-vertical" },
  { GDL_DOCK_ITEM_BEH_NEVER_HORIZONTAL, "GDL_DOCK_ITEM_BEH_NEVER_HORIZONTAL", "never-horizontal" },
  { GDL_DOCK_ITEM_BEH_LOCKED, "GDL_DOCK_ITEM_BEH_LOCKED", "locked" },
  { GDL_DOCK_ITEM_BEH_CANT_DOCK_TOP, "GDL_DOCK_ITEM_BEH_CANT_DOCK_TOP", "cant-dock-top" },
  { GDL_DOCK_ITEM_BEH_CANT_DOCK_BOTTOM, "GDL_DOCK_ITEM_BEH_CANT_DOCK_BOTTOM", "cant-dock-bottom" },
  { GDL_DOCK_ITEM_BEH_CANT_DOCK_LEFT, "GDL_DOCK_ITEM_BEH_CANT_DOCK_LEFT", "cant-dock-left" },
  { GDL_DOCK_ITEM_BEH_CANT_DOCK_RIGHT, "GDL_DOCK_ITEM_BEH_CANT_DOCK_RIGHT", "cant-dock-right" },
  { GDL_DOCK_ITEM_BEH_CANT_DOCK_CENTER, "GDL_DOCK_ITEM_BEH_CANT_DOCK_CENTER", "cant-dock-center" },
  { GDL_DOCK_ITEM_BEH_CANT_CLOSE, "GDL_DOCK_ITEM_BEH_CANT_CLOSE", "cant-close" },
  { GDL_DOCK_ITEM_BEH_CANT_ICONIFY, "GDL_DOCK_ITEM_BEH_CANT_ICONIFY", "cant-iconify" },
  { GDL_DOCK_ITEM_BEH_NO_GRIP, "GDL_DOCK_ITEM_BEH_NO_GRIP", "no-grip" },
  { 0, NULL, NULL }
};

GType
gdl_dock_item_behavior_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_flags_register_static ("GdlDockItemBehavior", _gdl_dock_item_behavior_values);

  return type;
}

static const GFlagsValue _gdl_dock_item_flags_values[] = {
  { GDL_DOCK_IN_DRAG, "GDL_DOCK_IN_DRAG", "in-drag" },
  { GDL_DOCK_IN_PREDRAG, "GDL_DOCK_IN_PREDRAG", "in-predrag" },
  { GDL_DOCK_ICONIFIED, "GDL_DOCK_ICONIFIED", "iconified" },
  { GDL_DOCK_USER_ACTION, "GDL_DOCK_USER_ACTION", "user-action" },
  { 0, NULL, NULL }
};

GType
gdl_dock_item_flags_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_flags_register_static ("GdlDockItemFlags", _gdl_dock_item_flags_values);

  return type;
}


/* enumerations from "gdl-dock-bar.h" */
static const GEnumValue _gdl_dock_bar_style_values[] = {
  { GDL_DOCK_BAR_ICONS, "GDL_DOCK_BAR_ICONS", "icons" },
  { GDL_DOCK_BAR_TEXT, "GDL_DOCK_BAR_TEXT", "text" },
  { GDL_DOCK_BAR_BOTH, "GDL_DOCK_BAR_BOTH", "both" },
  { GDL_DOCK_BAR_AUTO, "GDL_DOCK_BAR_AUTO", "auto" },
  { 0, NULL, NULL }
};

GType
gdl_dock_bar_style_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("GdlDockBarStyle", _gdl_dock_bar_style_values);

  return type;
}





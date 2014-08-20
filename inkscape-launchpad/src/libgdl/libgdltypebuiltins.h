


#ifndef __LIBGDLTYPEBUILTINS_H__
#define __LIBGDLTYPEBUILTINS_H__ 1

#include "libgdl/gdl.h"

G_BEGIN_DECLS


/* --- gdl-dock-object.h --- */
#define GDL_TYPE_DOCK_PARAM_FLAGS gdl_dock_param_flags_get_type()
GType gdl_dock_param_flags_get_type (void);
#define GDL_TYPE_DOCK_OBJECT_FLAGS gdl_dock_object_flags_get_type()
GType gdl_dock_object_flags_get_type (void);
#define GDL_TYPE_DOCK_PLACEMENT gdl_dock_placement_get_type()
GType gdl_dock_placement_get_type (void);

/* --- gdl-dock-master.h --- */
#define GDL_TYPE_SWITCHER_STYLE gdl_switcher_style_get_type()
GType gdl_switcher_style_get_type (void);

/* --- gdl-dock-item.h --- */
#define GDL_TYPE_DOCK_ITEM_BEHAVIOR gdl_dock_item_behavior_get_type()
GType gdl_dock_item_behavior_get_type (void);
#define GDL_TYPE_DOCK_ITEM_FLAGS gdl_dock_item_flags_get_type()
GType gdl_dock_item_flags_get_type (void);

/* --- gdl-dock-bar.h --- */
#define GDL_TYPE_DOCK_BAR_STYLE gdl_dock_bar_style_get_type()
GType gdl_dock_bar_style_get_type (void);
G_END_DECLS

#endif /* __LIBGDLTYPEBUILTINS_H__ */




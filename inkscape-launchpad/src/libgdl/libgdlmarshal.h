
#ifndef __gdl_marshal_MARSHAL_H__
#define __gdl_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:VOID (./libgdlmarshal.list:1) */
#define gdl_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:ENUM (./libgdlmarshal.list:2) */
#define gdl_marshal_VOID__ENUM	g_cclosure_marshal_VOID__ENUM

/* VOID:INT,INT (./libgdlmarshal.list:3) */
extern void gdl_marshal_VOID__INT_INT (GClosure     *closure,
                                       GValue       *return_value,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint,
                                       gpointer      marshal_data);

/* VOID:UINT,UINT (./libgdlmarshal.list:4) */
extern void gdl_marshal_VOID__UINT_UINT (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data);

/* VOID:BOOLEAN (./libgdlmarshal.list:5) */
#define gdl_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

/* VOID:OBJECT,ENUM,BOXED (./libgdlmarshal.list:6) */
extern void gdl_marshal_VOID__OBJECT_ENUM_BOXED (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

/* VOID:BOXED (./libgdlmarshal.list:7) */
#define gdl_marshal_VOID__BOXED	g_cclosure_marshal_VOID__BOXED

G_END_DECLS

#endif /* __gdl_marshal_MARSHAL_H__ */


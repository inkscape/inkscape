#ifndef SEEN_SP_STOP_FNS_H
#define SEEN_SP_STOP_FNS_H

#include <glib-object.h>
struct SPStop;
struct SPStopClass;

#define SP_TYPE_STOP (sp_stop_get_type())
#define SP_STOP(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_STOP, SPStop))
#define SP_STOP_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_STOP, SPStopClass))
#define SP_IS_STOP(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_STOP))
#define SP_IS_STOP_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_STOP))

GType sp_stop_get_type();


#endif /* !SEEN_SP_STOP_FNS_H */

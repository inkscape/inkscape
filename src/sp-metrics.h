#ifndef SP_METRICS_H
#define SP_METRICS_H

#include <glib/gstring.h>
#include <glib/gtypes.h>
#include "sp-metric.h"

gdouble sp_absolute_metric_to_metric (gdouble length_src, const SPMetric metric_src, const SPMetric metric_dst);
GString * sp_metric_to_metric_string (gdouble length,  const SPMetric metric_src, const SPMetric metric_dst, gboolean m);

// convenience since we mostly deal with points
#define SP_METRIC_TO_PT(l,m) sp_absolute_metric_to_metric(l,m,SP_PT);
#define SP_PT_TO_METRIC(l,m) sp_absolute_metric_to_metric(l,SP_PT,m);

#define SP_PT_TO_METRIC_STRING(l,m) sp_metric_to_metric_string(l, SP_PT, m, TRUE)
#define SP_PT_TO_STRING(l,m) sp_metric_to_metric_string(l, SP_PT, m, FALSE)

#define SP_PX_TO_METRIC_STRING(l,m) sp_metric_to_metric_string(l, SP_PX, m, TRUE)
#define SP_PX_TO_STRING(l,m) sp_metric_to_metric_string(l, SP_PX, m, FALSE)

#endif

#include "sp-metrics.h"
#include "unit-constants.h"

/*
 * SPMetric handling and stuff
 * I hope this will be usefull :-)
 */

gdouble
sp_absolute_metric_to_metric (gdouble length_src, const SPMetric metric_src, const SPMetric metric_dst)
{
  gdouble src = 1;
  gdouble dst = 1;

  switch (metric_src) {
  case SP_M:
    src = M_PER_IN;
    break;
  case SP_MM:
    src = MM_PER_IN;
    break;
  case SP_CM:
    src = CM_PER_IN;
    break;
  case SP_IN:
    src = IN_PER_IN;
    break;
  case SP_FT:
    src = FT_PER_IN;
    break;
  case SP_PT:
    src = PT_PER_IN;
    break;
  case SP_PC:
    src = PC_PER_IN;
    break;
  case SP_PX:
    src = PX_PER_IN;
    break;
  case NONE:
    src = 1;
    break;
  }

  switch (metric_dst) {
  case SP_M:
    dst = M_PER_IN;
    break;
  case SP_MM:
    dst = MM_PER_IN;
    break;
  case SP_CM:
    dst = CM_PER_IN;
    break;
  case SP_IN:
    dst = IN_PER_IN;
    break;
  case SP_FT:
    dst = FT_PER_IN;
    break;
  case SP_PT:
    dst = PT_PER_IN;
    break;
  case SP_PC:
    dst = PC_PER_IN;
    break;
  case SP_PX:
    dst = PX_PER_IN;
    break;
  case NONE:
    dst = 1;
    break;
  }

  return length_src * (dst/src);
}

/**
 * Create a human-readable string suitable for status-bar display.
 */
GString *
sp_metric_to_metric_string(gdouble const length,
                           SPMetric const metric_src, SPMetric const metric_dst,
                           gboolean const m)
{
    gdouble const len = sp_absolute_metric_to_metric(length, metric_src, metric_dst);
    GString *str = g_string_new("");
    g_string_printf(str, "%0.02f", len);
    /* We need a fixed number of fractional digits, because otherwise the live statusbar display of
     * lengths will be too jerky */

    if (m) {
        char const *unit_str;
        switch (metric_dst) {
            case SP_M:  unit_str = " m"; break;
            case SP_MM: unit_str = " mm"; break;
            case SP_CM: unit_str = " cm"; break;
            case SP_IN: unit_str = "\""; break;
            case SP_PT: unit_str = " pt"; break;
            case SP_PX: unit_str = " px"; break;
            default: unit_str = NULL; break;
        }
        if (unit_str) {
            g_string_append(str, unit_str);
        }
    }
    return str;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

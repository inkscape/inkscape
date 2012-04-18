#ifndef __SP_RULER_H__
#define __SP_RULER_H__

/*
 * Customized ruler class for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include "sp-metric.h"
#include <iostream>
#include <glib.h>

#define GTK_DEPRECATED_TYPE_RULER            (gtk_deprecated_ruler_get_type ())
#define GTK_DEPRECATED_RULER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_DEPRECATED_TYPE_RULER, GtkDeprecatedRuler))
#define GTK_DEPRECATED_RULER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_DEPRECATED_TYPE_RULER, GtkDeprecatedRulerClass))
#define GTK_DEPRECATED_IS_RULER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_DEPRECATED_TYPE_RULER))
#define GTK_DEPRECATED_IS_RULER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_DEPRECATED_TYPE_RULER))
#define GTK_DEPRECATED_RULER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_DEPRECATED_TYPE_RULER, GtkDeprecatedRulerClass))


typedef struct _GtkDeprecatedRuler        GtkDeprecatedRuler;
typedef struct _GtkDeprecatedRulerClass   GtkDeprecatedRulerClass;
typedef struct _GtkDeprecatedRulerMetric  GtkDeprecatedRulerMetric;

/* All distances below are in 1/72nd's of an inch. (According to
 * Adobe that's a point, but points are really 1/72.27 in.)
 */
struct _GtkDeprecatedRuler
{
  GtkWidget widget;

  GdkPixmap *backing_store;
  GdkGC     *non_gr_exp_gc;		/* unused */
  GtkDeprecatedRulerMetric *metric;
  gint xsrc;
  gint ysrc;
  gint slider_size;

  /* The upper limit of the ruler (in points) */
  gdouble lower;
  /* The lower limit of the ruler */
  gdouble upper;
  /* The position of the mark on the ruler */
  gdouble position;
  /* The maximum size of the ruler */
  gdouble max_size;
};

struct _GtkDeprecatedRulerClass
{
  GtkWidgetClass parent_class;

  void (* draw_ticks) (GtkDeprecatedRuler *ruler);
  void (* draw_pos)   (GtkDeprecatedRuler *ruler);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

struct _GtkDeprecatedRulerMetric
{
  gchar const *metric_name;
  gchar const *abbrev;
  /* This should be points_per_unit. This is the size of the unit
   * in 1/72nd's of an inch and has nothing to do with screen pixels */
  gdouble pixels_per_unit;
  gdouble ruler_scale[10];
  gint subdivide[5];        /* five possible modes of subdivision */
};


GType           gtk_deprecated_ruler_get_type   (void) G_GNUC_CONST;
void            gtk_deprecated_ruler_set_metric (GtkDeprecatedRuler       *ruler,
                                                 GtkMetricType   metric);
GtkMetricType   gtk_deprecated_ruler_get_metric (GtkDeprecatedRuler       *ruler);
void            gtk_deprecated_ruler_set_range  (GtkDeprecatedRuler       *ruler,
                                                 gdouble         lower,
                                                 gdouble         upper,
                                                 gdouble         position,
                                                 gdouble         max_size);
void            gtk_deprecated_ruler_get_range  (GtkDeprecatedRuler       *ruler,
                                                 gdouble        *lower,
                                                 gdouble        *upper,
                                                 gdouble        *position,
                                                 gdouble        *max_size);

void            gtk_deprecated_ruler_draw_ticks (GtkDeprecatedRuler       *ruler);
void            gtk_deprecated_ruler_draw_pos   (GtkDeprecatedRuler       *ruler);






void sp_ruler_set_metric (GtkDeprecatedRuler * ruler, SPMetric  metric);


#define SP_HRULER(obj)          G_TYPE_CHECK_INSTANCE_CAST (obj, sp_hruler_get_type (), SPHRuler)
#define SP_HRULER_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, sp_hruler_get_type (), SPHRulerClass)
#define SP_IS_HRULER(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, sp_hruler_get_type ())


struct SPHRuler
{
  GtkDeprecatedRuler ruler;
};

struct SPHRulerClass
{
  GtkDeprecatedRulerClass parent_class;
};


GType    sp_hruler_get_type (void);
GtkWidget* sp_hruler_new      (void);



// vruler



#define SP_VRULER(obj)          G_TYPE_CHECK_INSTANCE_CAST (obj, sp_vruler_get_type (), SPVRuler)
#define SP_VRULER_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, sp_vruler_get_type (), SPVRulerClass)
#define SP_IS_VRULER(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, sp_vruler_get_type ())


struct SPVRuler
{
  GtkDeprecatedRuler ruler;
};

struct SPVRulerClass
{
  GtkDeprecatedRulerClass parent_class;
};


GType    sp_vruler_get_type (void);
GtkWidget* sp_vruler_new      (void);




#endif /* __SP_RULER_H__ */

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

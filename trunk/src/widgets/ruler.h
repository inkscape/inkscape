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

#include <gtk/gtkruler.h>
#include "sp-metric.h"
#include <iostream>
#include <glib.h>


void sp_ruler_set_metric (GtkRuler * ruler, SPMetric  metric);


#define SP_HRULER(obj)          GTK_CHECK_CAST (obj, sp_hruler_get_type (), SPHRuler)
#define SP_HRULER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, sp_hruler_get_type (), SPHRulerClass)
#define SP_IS_HRULER(obj)       GTK_CHECK_TYPE (obj, sp_hruler_get_type ())


struct SPHRuler
{
  GtkRuler ruler;
};

struct SPHRulerClass
{
  GtkRulerClass parent_class;
};


GtkType    sp_hruler_get_type (void);
GtkWidget* sp_hruler_new      (void);



// vruler



#define SP_VRULER(obj)          GTK_CHECK_CAST (obj, sp_vruler_get_type (), SPVRuler)
#define SP_VRULER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, sp_vruler_get_type (), SPVRulerClass)
#define SP_IS_VRULER(obj)       GTK_CHECK_TYPE (obj, sp_vruler_get_type ())


struct SPVRuler
{
  GtkRuler ruler;
};

struct SPVRulerClass
{
  GtkRulerClass parent_class;
};


GtkType    sp_vruler_get_type (void);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

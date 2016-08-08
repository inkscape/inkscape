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
#include <iostream>
#include <glib.h>

namespace Inkscape {
    namespace Util {
        class Unit;
    }
}

G_BEGIN_DECLS

#define SP_TYPE_RULER            (sp_ruler_get_type ())
#define SP_RULER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_RULER, SPRuler))
#define SP_RULER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_RULER, SPRulerClass))
#define SP_IS_RULER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_RULER))
#define SP_IS_RULER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_RULER))
#define SP_RULER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SP_TYPE_RULER, SPRulerClass))

typedef struct _SPRuler         SPRuler;
typedef struct _SPRulerClass    SPRulerClass;

struct _SPRuler
{
  GtkWidget parent_instance;
};

struct _SPRulerClass
{
  GtkWidgetClass parent_class;
};


GType           sp_ruler_get_type            (void) G_GNUC_CONST;

GtkWidget*      sp_ruler_new                 (GtkOrientation  orientation);

void            sp_ruler_add_track_widget    (SPRuler        *ruler,
		                              GtkWidget      *widget);
void            sp_ruler_remove_track_widget (SPRuler        *ruler,
		                              GtkWidget      *widget);

void            sp_ruler_set_unit            (SPRuler        *ruler,
                                              const Inkscape::Util::Unit *unit);
Inkscape::Util::Unit const * sp_ruler_get_unit            (SPRuler        *ruler);
void            sp_ruler_set_position        (SPRuler        *ruler,
                                              gdouble         set_position);
gdouble         sp_ruler_get_position        (SPRuler        *ruler);
void            sp_ruler_set_range           (SPRuler        *ruler,
                                              gdouble         lower,
                                              gdouble         upper,
                                              gdouble         max_size);
void            sp_ruler_get_range           (SPRuler        *ruler,
                                              gdouble        *lower,
                                              gdouble        *upper,
                                              gdouble        *max_size);

G_END_DECLS

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

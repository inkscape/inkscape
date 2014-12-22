#ifndef SEEN_SP_WIDGET_H
#define SEEN_SP_WIDGET_H

/*
 * Abstract base class for dynamic control widgets
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include "inkscape.h"

#define SP_TYPE_WIDGET (sp_widget_get_type())
#define SP_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_WIDGET, SPWidget))
#define SP_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_WIDGET, SPWidgetClass))
#define SP_IS_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_WIDGET))
#define SP_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_WIDGET))

namespace Inkscape {

class Selection;
class SPWidgetImpl;
}

struct SPWidget {
    friend class Inkscape::SPWidgetImpl;

    static GType getType();

    GtkBin bin;

    Inkscape::SPWidgetImpl *_impl;
private:
    sigc::connection selModified;
    sigc::connection selChanged;
    sigc::connection selSet;
};

struct SPWidgetClass {
    GtkBinClass bin_class;

    void (* construct) (SPWidget *spw);

    // Selection change handlers
    void (* modify_selection) (SPWidget *spw, Inkscape::Selection *selection, guint flags);
    void (* change_selection) (SPWidget *spw, Inkscape::Selection *selection);
    void (* set_selection) (SPWidget *spw, Inkscape::Selection *selection);
};

GType sp_widget_get_type();

/** Generic constructor for global widget. */
GtkWidget *sp_widget_new_global();

#endif // SEEN_SP_WIDGET_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

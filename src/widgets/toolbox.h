#ifndef SEEN_TOOLBOX_H
#define SEEN_TOOLBOX_H

/**
 * \brief Main toolbox
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkstyle.h>
#include <gtk/gtktooltips.h>
#include <glibmm/ustring.h>

#include "forward.h"
#include "icon-size.h"

namespace Inkscape {
namespace UI {

class ToolboxFactory
{
public:
    static void setToolboxDesktop(GtkWidget *toolbox, SPDesktop *desktop);
    static void setOrientation(GtkWidget* toolbox, GtkOrientation orientation);
    static void showAuxToolbox(GtkWidget* toolbox);

    static GtkWidget *createToolToolbox();
    static GtkWidget *createAuxToolbox();
    static GtkWidget *createCommandsToolbox();
    static GtkWidget *createSnapToolbox();

    static Glib::ustring getToolboxName(GtkWidget* toolbox);

    static void updateSnapToolbox(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

    static Inkscape::IconSize prefToSize(Glib::ustring const &path, int base = 0 );

private:
    ToolboxFactory();
};

} // namespace UI
} // namespace Inkscape


// utility

// TODO remove this:
void sp_toolbox_add_label(GtkWidget *tbl, gchar const *title, bool wide = true);


#endif /* !SEEN_TOOLBOX_H */

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

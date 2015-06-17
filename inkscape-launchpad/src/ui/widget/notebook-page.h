/*
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_NOTEBOOK_PAGE_H
#define INKSCAPE_UI_WIDGET_NOTEBOOK_PAGE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>

namespace Gtk {
#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * A tabbed notebook page for dialogs.
 */
class NotebookPage : public Gtk::VBox
{
public:

    NotebookPage();

    /**
     * Construct a NotebookPage.
     */
    NotebookPage(int n_rows, int n_columns, bool expand=false, bool fill=false, guint padding=0);

#if WITH_GTKMM_3_0
    Gtk::Grid& table() { return *_table; }
#else
    Gtk::Table& table() { return *_table; }
#endif

protected:

#if WITH_GTKMM_3_0
    Gtk::Grid *_table;
#else
    Gtk::Table *_table;
#endif
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_NOTEBOOK_PAGE_H

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

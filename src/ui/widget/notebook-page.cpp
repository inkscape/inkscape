/*
 * Notebook page widget.
 *
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#include "notebook-page.h"

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

namespace Inkscape {
namespace UI {
namespace Widget {

NotebookPage::NotebookPage(int n_rows, int n_columns, bool expand, bool fill, guint padding)
#if WITH_GTKMM_3_0
    :_table(Gtk::manage(new Gtk::Grid()))
#else
    :_table(Gtk::manage(new Gtk::Table(n_rows, n_columns)))
#endif
{
    set_border_width(2);

#if WITH_GTKMM_3_0
    _table->set_row_spacing(2);
    _table->set_column_spacing(2);
#else
    _table->set_spacings(2);
#endif

    pack_start(*_table, expand, fill, padding);
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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

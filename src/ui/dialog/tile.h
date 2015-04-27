/** @file
 * @brief Dialog for creating grid type arrangements of selected objects
 */
/* Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *   Declara Denis
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_DIALOG_TILE_H
#define SEEN_UI_DIALOG_TILE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>

#include "ui/widget/panel.h"

namespace Gtk {
class Button;

#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

namespace Inkscape {
namespace UI {
namespace Dialog {

class ArrangeTab;
class GridArrangeTab;
class PolarArrangeTab;

class ArrangeDialog : public UI::Widget::Panel {
private:
	Gtk::VBox       _arrangeBox;
	Gtk::Notebook   _notebook;

	GridArrangeTab  *_gridArrangeTab;
	PolarArrangeTab *_polarArrangeTab;

	Gtk::Button     *_arrangeButton;

public:
	ArrangeDialog();
	virtual ~ArrangeDialog() {};

    /**
     * Callback from Apply
     */
    virtual void _apply();

    virtual void on_show();

	static ArrangeDialog& getInstance() { return *new ArrangeDialog(); }
};

} //namespace Dialog
} //namespace UI
} //namespace Inkscape


#endif /* __TILEDIALOG_H__ */

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

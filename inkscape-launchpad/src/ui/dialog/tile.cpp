/*
 * A simple dialog for creating grid type arrangements of selected objects
 *
 * Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *   Declara Denis
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/dialog/grid-arrange-tab.h"
#include "ui/dialog/polar-arrange-tab.h"

#include <glibmm/i18n.h>

#include "tile.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

ArrangeDialog::ArrangeDialog()
	: UI::Widget::Panel("", "/dialogs/gridtiler", SP_VERB_SELECTION_ARRANGE),
	  _gridArrangeTab(new GridArrangeTab(this)),
      _polarArrangeTab(new PolarArrangeTab(this))
{
    Gtk::Box *contents = this->_getContents();

    _notebook.append_page(*_gridArrangeTab, C_("Arrange dialog", "Rectangular grid"));
    _notebook.append_page(*_polarArrangeTab, C_("Arrange dialog", "Polar Coordinates"));
    _arrangeBox.pack_start(_notebook);

    _arrangeButton = this->addResponseButton(C_("Arrange dialog","_Arrange"), GTK_RESPONSE_APPLY);
    _arrangeButton->set_use_underline(true);
    _arrangeButton->set_tooltip_text(_("Arrange selected objects"));
    contents->pack_start(_arrangeBox);
    //show_all_children();
}


void ArrangeDialog::on_show()
{
	UI::Widget::Panel::on_show();
	_polarArrangeTab->on_arrange_radio_changed();
}

void ArrangeDialog::_apply()
{
	switch(_notebook.get_current_page())
	{
	case 0:
		_gridArrangeTab->arrange();
		break;
	case 1:
		_polarArrangeTab->arrange();
		break;
	}
}

} //namespace Dialog
} //namespace UI
} //namespace Inkscape

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

/*
 * anchor-selector.h
 *
 *  Created on: Mar 22, 2012
 *      Author: denis
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef ANCHOR_SELECTOR_H_
#define ANCHOR_SELECTOR_H_

#if HAVE_CONFIG_H
 #include "config.h"
#endif

#include <gtkmm/alignment.h>
#include <gtkmm/togglebutton.h>

#if WITH_GTKMM_3_0
 #include <gtkmm/grid.h>
#else
 #include <gtkmm/table.h>
#endif

namespace Inkscape {
namespace UI {
namespace Widget {

class AnchorSelector : public Gtk::Alignment
{
private:
	Gtk::ToggleButton  _buttons[9];
	int                _selection;

#if WITH_GTKMM_3_0
	Gtk::Grid          _container;
#else
	Gtk::Table         _container;
#endif

	sigc::signal<void> _selectionChanged;

	void setupButton(const Glib::ustring &icon, Gtk::ToggleButton &button);
	void btn_activated(int index);

public:

	int getHorizontalAlignment() { return _selection % 3; }
	int getVerticalAlignment() { return _selection / 3; }

	sigc::signal<void> &on_selectionChanged() { return _selectionChanged; }

	void setAlignment(int horizontal, int vertical);

	AnchorSelector();
	virtual ~AnchorSelector();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif /* ANCHOR_SELECTOR_H_ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

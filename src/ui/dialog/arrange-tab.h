/**
 * @brief Arrange tools base class
 */
/* Authors:
 *    * Declara Denis
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_ARRANGE_TAB_H
#define INKSCAPE_UI_DIALOG_ARRANGE_TAB_H

#include <gtkmm/box.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * This interface should be implemented by each arrange mode.
 * The class is a Gtk::VBox and will be displayed as a tab in
 * the dialog
 */
class ArrangeTab : public Gtk::VBox
{
public:
	ArrangeTab() {};
	virtual ~ArrangeTab() {};

	/**
	 * Do the actual work! This method is invoked to actually arrange the
	 * selection
	 */
	virtual void arrange() = 0;
};

} //namespace Dialog
} //namespace UI
} //namespace Inkscape


#endif /* INKSCAPE_UI_DIALOG_ARRANGE_TAB_H */

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

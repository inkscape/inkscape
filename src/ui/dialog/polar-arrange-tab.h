/**
 * @brief Arranges Objects into a Circle/Ellipse
 */
/* Authors:
 *   Declara Denis
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_POLAR_ARRANGE_TAB_H
#define INKSCAPE_UI_DIALOG_POLAR_ARRANGE_TAB_H

#if HAVE_CONFIG_H
 #include "config.h"
#endif

#include "ui/widget/scalar-unit.h"
#include "ui/widget/anchor-selector.h"
#include "ui/dialog/arrange-tab.h"

#include <gtkmm/radiobutton.h>
#include <gtkmm/radiobuttongroup.h>

#if WITH_GTKMM_3_0
 #include <gtkmm/grid.h>
#else
 #include <gtkmm/table.h>
#endif

namespace Inkscape {
namespace UI {
namespace Dialog {

class ArrangeDialog;

/**
 * PolarArrangeTab is a Tab displayed in the Arrange dialog and contains
 * enables the user to arrange objects on a circular or elliptical shape
 */
class PolarArrangeTab : public ArrangeTab {
public:
	PolarArrangeTab(ArrangeDialog *parent_);
    virtual ~PolarArrangeTab() {};

    /**
     * Do the actual arrangement
     */
    virtual void arrange();

    /**
     * Respond to selection change
     */
    void updateSelection();

    void on_anchor_radio_changed();
    void on_arrange_radio_changed();

private:
    PolarArrangeTab(PolarArrangeTab const &d); // no copy
    void operator=(PolarArrangeTab const &d); // no assign

    ArrangeDialog         *parent;

    Gtk::Label             anchorPointLabel;

    Gtk::RadioButtonGroup  anchorRadioGroup;
    Gtk::RadioButton       anchorBoundingBoxRadio;
    Gtk::RadioButton       anchorObjectPivotRadio;
    Inkscape::UI::Widget::AnchorSelector anchorSelector;

    Gtk::Label             arrangeOnLabel;

    Gtk::RadioButtonGroup  arrangeRadioGroup;
    Gtk::RadioButton       arrangeOnFirstCircleRadio;
    Gtk::RadioButton       arrangeOnLastCircleRadio;
    Gtk::RadioButton       arrangeOnParametersRadio;

#if WITH_GTKMM_3_0
    Gtk::Grid              parametersTable;
#else
    Gtk::Table             parametersTable;
#endif

    Gtk::Label             centerLabel;
    Inkscape::UI::Widget::ScalarUnit centerY;
    Inkscape::UI::Widget::ScalarUnit centerX;

    Gtk::Label             radiusLabel;
    Inkscape::UI::Widget::ScalarUnit radiusY;
    Inkscape::UI::Widget::ScalarUnit radiusX;

    Gtk::Label             angleLabel;
    Inkscape::UI::Widget::ScalarUnit angleY;
    Inkscape::UI::Widget::ScalarUnit angleX;

    Gtk::CheckButton       rotateObjectsCheckBox;


};

} //namespace Dialog
} //namespace UI
} //namespace Inkscape

#endif /* INKSCAPE_UI_DIALOG_POLAR_ARRANGE_TAB_H */

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

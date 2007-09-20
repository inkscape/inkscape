/**
 * \brief Fill and Stroke dialog
 * based on sp_object_properties_dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_FILL_AND_STROKE_H
#define INKSCAPE_UI_DIALOG_FILL_AND_STROKE_H

#include <gtkmm/adjustment.h>
#include <gtkmm/alignment.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/filter-effect-chooser.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class FillAndStroke : public Dialog {
public:
    FillAndStroke(Behavior::BehaviorFactory behavior_factory);
    virtual ~FillAndStroke();

    static FillAndStroke *create(Behavior::BehaviorFactory behavior_factory)
    { return new FillAndStroke(behavior_factory); }

    void selectionChanged(Inkscape::Application *inkscape,
                          Inkscape::Selection *selection);

    void showPageFill();
    void showPageStrokePaint();
    void showPageStrokeStyle();

protected:
    Gtk::Notebook   _notebook;

    NotebookPage    _page_fill;
    NotebookPage    _page_stroke_paint;
    NotebookPage    _page_stroke_style;

    Gtk::VBox       _fe_vbox;
    Gtk::Alignment  _fe_alignment;

    Gtk::VBox       _opacity_vbox;
    Gtk::HBox       _opacity_label_box;
    Gtk::HBox       _opacity_hbox;
    Gtk::Label      _opacity_label;
    Gtk::Adjustment _opacity_adjustment;
    Gtk::HScale     _opacity_hscale;
    Gtk::SpinButton _opacity_spin_button;

    Gtk::HBox& _createPageTabLabel(const Glib::ustring& label, 
                                   const char *label_image);
    SimpleFilterModifier _fe_cb;

    void _layoutPageFill();
    void _layoutPageStrokePaint();
    void _layoutPageStrokeStyle();

    void _blendBlurValueChanged();
    void _opacityValueChanged();

    bool _blocked;

private:
    FillAndStroke(FillAndStroke const &d);
    FillAndStroke& operator=(FillAndStroke const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_FILL_AND_STROKE_H

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

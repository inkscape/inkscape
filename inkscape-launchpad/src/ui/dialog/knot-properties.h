/** @file
 * @brief
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_KNOT_PROPERTIES_H
#define INKSCAPE_DIALOG_KNOT_PROPERTIES_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm.h>
#include <2geom/point.h>
#include "knot.h"
#include "ui/tools/measure-tool.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialogs {


class KnotPropertiesDialog : public Gtk::Dialog {
 public:
	KnotPropertiesDialog();
    virtual ~KnotPropertiesDialog();

    Glib::ustring     getName() const { return "LayerPropertiesDialog"; }

    static void showDialog(SPDesktop *desktop, const SPKnot *pt, Glib::ustring const unit_name);

protected:

    SPDesktop *_desktop;
    SPKnot *_knotpoint;

    Gtk::Label        _knot_x_label;
    Gtk::SpinButton   _knot_x_entry;
    Gtk::Label        _knot_y_label;
    Gtk::SpinButton   _knot_y_entry;
    Gtk::Table        _layout_table;
    bool              _position_visible;

    Gtk::Button       _close_button;
    Gtk::Button       _apply_button;
    Glib::ustring _unit_name;

    sigc::connection    _destroy_connection;

    static KnotPropertiesDialog &_instance() {
        static KnotPropertiesDialog instance;
        return instance;
    }

    void _setDesktop(SPDesktop *desktop);
    void _setPt(const SPKnot *pt);

    void _apply();
    void _close();

    void _setKnotPoint(Geom::Point knotpoint, Glib::ustring const unit_name);
    void _prepareLabelRenderer(Gtk::TreeModel::const_iterator const &row);

    bool _handleKeyEvent(GdkEventKey *event);
    void _handleButtonEvent(GdkEventButton* event);
    friend class Inkscape::UI::Tools::MeasureTool;
    
private:
    KnotPropertiesDialog(KnotPropertiesDialog const &); // no copy
    KnotPropertiesDialog &operator=(KnotPropertiesDialog const &); // no assign
};

} // namespace
} // namespace
} // namespace


#endif //INKSCAPE_DIALOG_LAYER_PROPERTIES_H

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

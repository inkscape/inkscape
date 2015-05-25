/** @file
 * @brief  Dialog for renaming layers
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_POWERSTROKE_PROPERTIES_H
#define INKSCAPE_DIALOG_POWERSTROKE_PROPERTIES_H

#include <2geom/point.h>
#include <gtkmm.h>
#include "live_effects/parameter/powerstrokepointarray.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialogs {

class PowerstrokePropertiesDialog : public Gtk::Dialog {
 public:
	PowerstrokePropertiesDialog();
    virtual ~PowerstrokePropertiesDialog();

    Glib::ustring     getName() const { return "LayerPropertiesDialog"; }

    static void showDialog(SPDesktop *desktop, Geom::Point knotpoint, const Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity *pt);

protected:

    SPDesktop *_desktop;
    Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity *_knotpoint;

    Gtk::Label        _powerstroke_position_label;
    Gtk::SpinButton   _powerstroke_position_entry;
    Gtk::Label        _powerstroke_width_label;
    Gtk::SpinButton   _powerstroke_width_entry;
    Gtk::Table        _layout_table;
    bool              _position_visible;

    Gtk::Button       _close_button;
    Gtk::Button       _apply_button;

    sigc::connection    _destroy_connection;

    static PowerstrokePropertiesDialog &_instance() {
        static PowerstrokePropertiesDialog instance;
        return instance;
    }

    void _setDesktop(SPDesktop *desktop);
    void _setPt(const Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity *pt);

    void _apply();
    void _close();

    void _setKnotPoint(Geom::Point knotpoint);
    void _prepareLabelRenderer(Gtk::TreeModel::const_iterator const &row);

    bool _handleKeyEvent(GdkEventKey *event);
    void _handleButtonEvent(GdkEventButton* event);

    friend class Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity;

private:
    PowerstrokePropertiesDialog(PowerstrokePropertiesDialog const &); // no copy
    PowerstrokePropertiesDialog &operator=(PowerstrokePropertiesDialog const &); // no assign
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

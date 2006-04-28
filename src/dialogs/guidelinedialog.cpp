#define __GUIDELINE_CPP__

/*
 * simple guideline dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Andrius R. <knutux@gmail.com>
 *
 * Copyright (C) 1999-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "display/guideline.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "desktop.h"
#include "document.h"
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-handles.h"
#include "event-context.h"
#include "widgets/desktop-widget.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "dialogs/dialog-events.h"
#include "message-context.h"
#include "xml/repr.h"

#include "guidelinedialog.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

GuidelinePropertiesDialog::GuidelinePropertiesDialog(SPGuide *guide, SPDesktop *desktop)
: _desktop(desktop), _guide(guide),
  _a(0.0, -SP_DESKTOP_SCROLL_LIMIT, SP_DESKTOP_SCROLL_LIMIT, 1.0, 10.0, 10.0),
  _u(NULL), _mode(true), _oldpos(0.0)
{
}

GuidelinePropertiesDialog::~GuidelinePropertiesDialog() {
    if ( NULL != _u) {
        //g_free(_u);
    }
}

void GuidelinePropertiesDialog::showDialog(SPGuide *guide, SPDesktop *desktop) {
    GuidelinePropertiesDialog *dialog = new GuidelinePropertiesDialog(guide, desktop);
    dialog->_setup();
    dialog->run();
    delete dialog;
}

void GuidelinePropertiesDialog::_modeChanged()
{
    if (_mode) {
        // TRANSLATORS: This string appears when double-clicking on a guide.
        // This is the distance by which the guide is to be moved.
        _m.set_label(_(" relative by "));
        _mode = false;
    } else {
        // TRANSLATORS: This string appears when double-clicking on a guide.
        // This is the target location where the guide is to be moved.
        _m.set_label(_(" absolute to "));
        _mode = true;
    }
}

void GuidelinePropertiesDialog::_onApply()
{
    gdouble const raw_dist = _e.get_value();
    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(_u));
    gdouble const points = sp_units_get_pixels(raw_dist, unit);
    gdouble const newpos = ( _mode
                             ? points
                             : _guide->position + points );
    sp_guide_moveto(*_guide, newpos, true);
    sp_document_done(SP_OBJECT_DOCUMENT(_guide));
}

void GuidelinePropertiesDialog::_onOK()
{
    _onApply();
}

void GuidelinePropertiesDialog::_onDelete()
{
    SPDocument *doc = SP_OBJECT_DOCUMENT(_guide);
    sp_guide_remove(_guide);
    sp_document_done(doc);
}

void GuidelinePropertiesDialog::_response(gint response)
{
    switch (response) {
	case Gtk::RESPONSE_OK:
            _onOK();
            break;
	case -12:
            _onDelete();
            break;
	case Gtk::RESPONSE_CLOSE:
            break;
	case Gtk::RESPONSE_DELETE_EVENT:
            break;
/*	case GTK_RESPONSE_APPLY:
        _onApply();
        break;
*/
	default:
            g_assert_not_reached();
    }
}

void GuidelinePropertiesDialog::_setup() {
    set_title(_("Guideline"));
    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::DELETE, -12);
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

    Gtk::VBox *mainVBox = get_vbox();

    _b1.set_homogeneous(false);
    _b1.set_spacing(4);
    mainVBox->pack_start(_b1, false, false, 0);
    _b1.set_border_width(4);

    _b2.set_homogeneous(false);
    _b2.set_spacing(4);
    _b1.pack_start(_b2, true, true, 0);

    //labels
    _b3.set_homogeneous(false);
    _b3.set_spacing(4);
    _b2.pack_start(_b3, true, true, 0);

    _l1.set_label("foo1");
    _b3.pack_start(_l1, true, true, 0);
    _l1.set_alignment(1.0, 0.5);

    _l2.set_label("foo2");
    _b3.pack_start(_l2, true, true, 0);
    _l2.set_alignment(0.0, 0.5);

    _b4.set_homogeneous(false);
    _b4.set_spacing(4);
    _b2.pack_start(_b4, false, false, 0);

    // mode button
    _but.set_relief(Gtk::RELIEF_NONE);
    _b4.pack_start(_but, false, true, 0);
    _but.signal_clicked().connect(sigc::mem_fun(*this, &GuidelinePropertiesDialog::_modeChanged));
    _m.set_label(_(" absolute to "));
    _but.add(_m);

    // unitmenu
    /* fixme: We should allow percents here too, as percents of the canvas size */
    _u = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
    sp_unit_selector_set_unit(SP_UNIT_SELECTOR(_u), _desktop->namedview->doc_units);

    // spinbutton
    sp_unit_selector_add_adjustment(SP_UNIT_SELECTOR(_u), GTK_ADJUSTMENT(_a.gobj()));
    _e.configure(_a, 1.0 , 2);
    _e.set_numeric(TRUE);
    _b4.pack_start(_e, true, true, 0);
    gtk_signal_connect_object(GTK_OBJECT(_e.gobj()), "activate",
                              GTK_SIGNAL_FUNC(gtk_window_activate_default),
                              gobj());

    gtk_box_pack_start(GTK_BOX(_b4.gobj()), _u, FALSE, FALSE, 0);


    // dialog
    set_default_response(Gtk::RESPONSE_OK);
    signal_response().connect(sigc::mem_fun(*this, &GuidelinePropertiesDialog::_response));

    // initialize dialog
    _oldpos = _guide->position;
    {
        gchar *guide_description = sp_guide_description(_guide);
        gchar *label = g_strdup_printf(_("Move %s"), guide_description);
        g_free(guide_description);
        _l1.set_label(label);
        g_free(label);
    }

    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(_u));
    gdouble const val = sp_pixels_get_units(_oldpos, unit);
    _e.set_value(val);
    _e.grab_focus();
    _e.select_region(0, 20);
    set_position(Gtk::WIN_POS_MOUSE);

    show_all_children();
    set_modal(true);
    _desktop->setWindowTransient (gobj());
    property_destroy_with_parent() = true;
}

}
}
}

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


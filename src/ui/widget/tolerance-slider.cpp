/** \file
 *
 Implementation of tolerance slider widget.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de> 
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "xml/repr.h"
#include "svg/stringstream.h"

#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-namedview.h"

#include "registry.h"
#include "tolerance-slider.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

//---------------------------------------------------



//====================================================

ToleranceSlider::ToleranceSlider()
: _vbox(0)
{
}

ToleranceSlider::~ToleranceSlider()
{
    if (_vbox) delete _vbox;
    _scale_changed_connection.disconnect();
}

void
ToleranceSlider::init (const Glib::ustring& label1, const Glib::ustring& label2, const Glib::ustring& tip1, const Glib::ustring& tip2, const Glib::ustring& key, Registry& wr)
{
    _vbox = new Gtk::VBox;
    _hbox = manage (new Gtk::HBox);
    Gtk::Label *theLabel1 = manage (new Gtk::Label (label1));
    theLabel1->set_use_underline();
    _hbox->add (*theLabel1);
    _hscale = manage (new Gtk::HScale (0.4, 50.1, 0.1));
    theLabel1->set_mnemonic_widget (*_hscale);
    _hscale->set_draw_value (true);
    _hscale->set_value_pos (Gtk::POS_RIGHT);
    _hscale->set_size_request (100, -1);
    _tt.set_tip (*_hscale, tip1);
    _hbox->add (*_hscale);
    _vbox->add (*_hbox);
    Gtk::Label *theLabel2 = manage (new Gtk::Label (label2));
    theLabel2->set_use_underline();
    _button = manage (new Gtk::CheckButton);
    _tt.set_tip (*_button, tip2);
    _button->add (*theLabel2);
    _button->set_alignment (0.0, 0.5);
    _vbox->add (*_button);
    _key = key;
    _scale_changed_connection = _hscale->signal_value_changed().connect (sigc::mem_fun (*this, &ToleranceSlider::on_scale_changed));
    _btn_toggled_connection = _button->signal_toggled().connect (sigc::mem_fun (*this, &ToleranceSlider::on_toggled));
    _wr = &wr;
    _vbox->show_all_children();
}

void 
ToleranceSlider::setValue (double val, bool is_absolute)
{
    Gtk::Adjustment *adj = _hscale->get_adjustment();
    if (is_absolute) 
    { 
        adj->set_lower (1.0); 
        adj->set_upper (51.0);
        adj->set_step_increment (1.0);
    }
    else             
    { 
        adj->set_lower (0.4); 
        adj->set_upper (50.1);
        adj->set_step_increment (0.1);
    }

    if (val > 9999.9) // magic value 10000.0
    {
        _button->set_active (true);
        _hbox->set_sensitive (false);
        val = 50.0;
    }
    else
    {
        _button->set_active (false);
        _hbox->set_sensitive (true);
    }
    _hscale->set_value (val);
    _hbox->show_all();
}

void
ToleranceSlider::setLimits (double theMin, double theMax)
{
    _hscale->set_range (theMin, theMax);
    _hscale->get_adjustment()->set_step_increment (1);
}

void
ToleranceSlider::on_scale_changed()
{
    update (_hscale->get_value());
}

void
ToleranceSlider::on_toggled()
{
    if (_button->get_active())
    {
        _hbox->set_sensitive (false);
        _hbox->show_all();
        setValue (10000.0);
        update (10000.0);
    }
    else
    {
        _hbox->set_sensitive (true);
        _hbox->show_all();
        setValue (50.0);
        update (50.0);
    }
}

void
ToleranceSlider::update (double val)
{
    if (_wr->isUpdating())
        return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) 
        return;

    Inkscape::SVGOStringStream os;
    os << val;

    _wr->setUpdating (true);

    SPDocument *doc = sp_desktop_document(dt);
    gboolean saved = sp_document_get_undo_sensitive (doc);
    sp_document_set_undo_sensitive (doc, FALSE);
    Inkscape::XML::Node *repr = SP_OBJECT_REPR (sp_desktop_namedview(dt));
    repr->setAttribute(_key.c_str(), os.str().c_str());
    doc->rroot->setAttribute("sodipodi:modified", "true");
    sp_document_set_undo_sensitive (doc, saved);
    sp_document_done (doc);
    
    _wr->setUpdating (false);
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

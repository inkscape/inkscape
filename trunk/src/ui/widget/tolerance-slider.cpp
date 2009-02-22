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
ToleranceSlider::init (const Glib::ustring& label1, const Glib::ustring& label2, const Glib::ustring& label3, const Glib::ustring& tip1, const Glib::ustring& tip2, const Glib::ustring& tip3, const Glib::ustring& key, Registry& wr)
{
    // hbox = label + slider
    //
    // e.g. 
    //
    // snap distance |-------X---| 37
    
    // vbox = checkbutton
    //             +
    //           hbox
    
    _vbox = new Gtk::VBox;
    _hbox = manage (new Gtk::HBox);
    
    Gtk::Label *theLabel1 = manage (new Gtk::Label (label1));
    theLabel1->set_use_underline();
    theLabel1->set_alignment(0, 0.5);
    // align the label with the checkbox text above by indenting 22 px.
    _hbox->pack_start(*theLabel1, Gtk::PACK_EXPAND_WIDGET, 22); 
    _hscale = manage (new Gtk::HScale (1.0, 51, 1.0));
    theLabel1->set_mnemonic_widget (*_hscale);
    _hscale->set_draw_value (true);
    _hscale->set_value_pos (Gtk::POS_RIGHT);
    _hscale->set_size_request (100, -1);
    _old_val = 10;
    _hscale->set_value (_old_val);
    _tt.set_tip (*_hscale, tip1);
    _hbox->add (*_hscale);    
    
    
    Gtk::Label *theLabel2 = manage (new Gtk::Label (label2));
    theLabel2->set_use_underline();
    Gtk::Label *theLabel3 = manage (new Gtk::Label (label3));
    theLabel3->set_use_underline();    
    _button1 = manage (new Gtk::RadioButton);
    _radio_button_group = _button1->get_group();
    _button2 = manage (new Gtk::RadioButton);
    _button2->set_group(_radio_button_group);    
    _tt.set_tip (*_button1, tip2);
    _tt.set_tip (*_button2, tip3);    
    _button1->add (*theLabel3);
    _button1->set_alignment (0.0, 0.5);    
    _button2->add (*theLabel2);
    _button2->set_alignment (0.0, 0.5);
    
    _vbox->add (*_button1);
    _vbox->add (*_button2);    
    // Here we need some extra pixels to get the vertical spacing right. Why? 
    _vbox->pack_end(*_hbox, true, true, 3); // add 3 px.  
    _key = key;
    _scale_changed_connection = _hscale->signal_value_changed().connect (sigc::mem_fun (*this, &ToleranceSlider::on_scale_changed));
    _btn_toggled_connection = _button2->signal_toggled().connect (sigc::mem_fun (*this, &ToleranceSlider::on_toggled));
    _wr = &wr;
    _vbox->show_all_children();
}

void 
ToleranceSlider::setValue (double val)
{
    Gtk::Adjustment *adj = _hscale->get_adjustment();

    adj->set_lower (1.0);
    adj->set_upper (51.0);
    adj->set_step_increment (1.0);

    if (val > 9999.9) // magic value 10000.0
    {
        _button1->set_active (true);
        _button2->set_active (false);        
        _hbox->set_sensitive (false);
        val = 50.0;
    }
    else
    {
        _button1->set_active (false);
        _button2->set_active (true);        
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
    if (!_button2->get_active())
    {
        _old_val = _hscale->get_value();
        _hbox->set_sensitive (false);
        _hbox->show_all();
        setValue (10000.0);
        update (10000.0);
    }
    else
    {
        _hbox->set_sensitive (true);
        _hbox->show_all();
        setValue (_old_val);
        update (_old_val);
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
    bool saved = sp_document_get_undo_sensitive (doc);
    sp_document_set_undo_sensitive (doc, false);
    Inkscape::XML::Node *repr = SP_OBJECT_REPR (sp_desktop_namedview(dt));
    repr->setAttribute(_key.c_str(), os.str().c_str());
    sp_document_set_undo_sensitive (doc, saved);

    doc->setModifiedSinceSave();
    
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

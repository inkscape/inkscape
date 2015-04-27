/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de> 
 *   Abhishek Sharma
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
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>

#include "xml/repr.h"
#include "svg/stringstream.h"

#include "inkscape.h"
#include "document.h"
#include "document-undo.h"
#include "desktop.h"

#include "sp-namedview.h"

#include "registry.h"
#include "tolerance-slider.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

//---------------------------------------------------



//====================================================

ToleranceSlider::ToleranceSlider(const Glib::ustring& label1, const Glib::ustring& label2, const Glib::ustring& label3, const Glib::ustring& tip1, const Glib::ustring& tip2, const Glib::ustring& tip3, const Glib::ustring& key, Registry& wr)
: _vbox(0)
{
    init(label1, label2, label3, tip1, tip2, tip3, key, wr);
}

ToleranceSlider::~ToleranceSlider()
{
    if (_vbox) delete _vbox;
    _scale_changed_connection.disconnect();
}

void ToleranceSlider::init (const Glib::ustring& label1, const Glib::ustring& label2, const Glib::ustring& label3, const Glib::ustring& tip1, const Glib::ustring& tip2, const Glib::ustring& tip3, const Glib::ustring& key, Registry& wr)
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
    _hbox = Gtk::manage(new Gtk::HBox);
    
    Gtk::Label *theLabel1 = Gtk::manage(new Gtk::Label(label1));
    theLabel1->set_use_underline();
    theLabel1->set_alignment(0, 0.5);
    // align the label with the checkbox text above by indenting 22 px.
    _hbox->pack_start(*theLabel1, Gtk::PACK_EXPAND_WIDGET, 22);

#if WITH_GTKMM_3_0
    _hscale = Gtk::manage(new Gtk::Scale(Gtk::ORIENTATION_HORIZONTAL));
    _hscale->set_range(1.0, 51.0);
#else 
    _hscale = Gtk::manage (new Gtk::HScale (1.0, 51, 1.0));
#endif

    theLabel1->set_mnemonic_widget (*_hscale);
    _hscale->set_draw_value (true);
    _hscale->set_value_pos (Gtk::POS_RIGHT);
    _hscale->set_size_request (100, -1);
    _old_val = 10;
    _hscale->set_value (_old_val);
    _hscale->set_tooltip_text (tip1);
    _hbox->add (*_hscale);    
    
    
    Gtk::Label *theLabel2 = Gtk::manage(new Gtk::Label(label2));
    theLabel2->set_use_underline();
    Gtk::Label *theLabel3 = Gtk::manage(new Gtk::Label(label3));
    theLabel3->set_use_underline();    
    _button1 = Gtk::manage(new Gtk::RadioButton);
    _radio_button_group = _button1->get_group();
    _button2 = Gtk::manage(new Gtk::RadioButton);
    _button2->set_group(_radio_button_group);    
    _button1->set_tooltip_text (tip2);
    _button2->set_tooltip_text (tip3);    
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

void ToleranceSlider::setValue (double val)
{
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> adj = _hscale->get_adjustment();
#else
    Gtk::Adjustment *adj = _hscale->get_adjustment();
#endif

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

void ToleranceSlider::setLimits (double theMin, double theMax)
{
    _hscale->set_range (theMin, theMax);
    _hscale->get_adjustment()->set_step_increment (1);
}

void ToleranceSlider::on_scale_changed()
{
    update (_hscale->get_value());
}

void ToleranceSlider::on_toggled()
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

void ToleranceSlider::update (double val)
{
    if (_wr->isUpdating())
        return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) 
        return;

    Inkscape::SVGOStringStream os;
    os << val;

    _wr->setUpdating (true);

    SPDocument *doc = dt->getDocument();
    bool saved = DocumentUndo::getUndoSensitive(doc);
    DocumentUndo::setUndoSensitive(doc, false);
    Inkscape::XML::Node *repr = dt->getNamedView()->getRepr();
    repr->setAttribute(_key.c_str(), os.str().c_str());
    DocumentUndo::setUndoSensitive(doc, saved);

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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

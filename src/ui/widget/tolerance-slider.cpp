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
: _hbox(0)
{
}

ToleranceSlider::~ToleranceSlider()
{
    if (_hbox) delete _hbox;
    _scale_changed_connection.disconnect();
}

void
ToleranceSlider::init (const Glib::ustring& label1, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr)
{
    _hbox = new Gtk::HBox;
    Gtk::Label *theLabel1 = manage (new Gtk::Label (label1));
    theLabel1->set_use_underline();
    _hbox->add (*theLabel1);
    _hscale = manage (new Gtk::HScale (0.4, 50.1, 0.1));
    theLabel1->set_mnemonic_widget (*_hscale);
    _hscale->set_draw_value (true);
    _hscale->set_value_pos (Gtk::POS_RIGHT);
    _hscale->set_size_request (100, -1);
    _tt.set_tip (*_hscale, tip);
    _hbox->add (*_hscale);
//    Gtk::Label *theLabel2 = manage (new Gtk::Label (label2));
//    _hbox->add (*theLabel2);
    _key = key;
    _scale_changed_connection = _hscale->signal_value_changed().connect (sigc::mem_fun (*this, &ToleranceSlider::update));
    _wr = &wr;
}

void 
ToleranceSlider::setValue (double val, bool is_absolute)
{
    _hscale->set_value (val);
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
    update();
}

void
ToleranceSlider::setLimits (double theMin, double theMax)
{
    _hscale->set_range (theMin, theMax);
    _hscale->get_adjustment()->set_step_increment (1);
}

void
ToleranceSlider::update()
{
    if (_wr->isUpdating())
        return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) 
        return;

    Inkscape::SVGOStringStream os;
    os << _hscale->get_value();

    _wr->setUpdating (true);

    SPDocument *doc = SP_DT_DOCUMENT(dt);
    gboolean saved = sp_document_get_undo_sensitive (doc);
    sp_document_set_undo_sensitive (doc, FALSE);
    Inkscape::XML::Node *repr = SP_OBJECT_REPR (SP_DT_NAMEDVIEW(dt));
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

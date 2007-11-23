/** \file
 *
 *
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ui/widget/color-picker.h"
#include "ui/widget/registry.h"
#include "ui/widget/scalar-unit.h"
#include "ui/widget/point.h"
#include "ui/widget/random.h"
#include "widgets/spinbutton-events.h"

#include "helper/units.h"
#include "xml/repr.h"
#include "svg/svg-color.h"
#include "svg/stringstream.h"

#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-namedview.h"

#include "registered-widget.h"
#include "verbs.h"

// for interruptability bug:
#include "display/sp-canvas.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

//---------------------------------------------------


void
RegisteredWidget::write_to_xml(const char * svgstr)
{
    // Use local repr here. When repr is specified, use that one, but
    // if repr==NULL, get the repr of namedview of active desktop.
    Inkscape::XML::Node *local_repr = repr;
    SPDocument *local_doc = doc;
    if (!local_repr) {
        // no repr specified, use active desktop's namedview's repr
        SPDesktop* dt = SP_ACTIVE_DESKTOP;
        local_repr = SP_OBJECT_REPR (sp_desktop_namedview(dt));
        local_doc = sp_desktop_document(dt);
    }

    bool saved = sp_document_get_undo_sensitive (local_doc);
    sp_document_set_undo_sensitive (local_doc, false);

    if (!write_undo) local_repr->setAttribute(_key.c_str(), svgstr);
    local_doc->rroot->setAttribute("sodipodi:modified", "true");

    sp_document_set_undo_sensitive (local_doc, saved);
    if (write_undo) {
        local_repr->setAttribute(_key.c_str(), svgstr);
        sp_document_done (local_doc, event_type, event_description);
    }
}


//====================================================

RegisteredCheckButton::RegisteredCheckButton()
: _button(0),
   setProgrammatically(false)
{
}

RegisteredCheckButton::~RegisteredCheckButton()
{
    _toggled_connection.disconnect();
    if (_button) delete _button;
}

void
RegisteredCheckButton::init (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
{
    init_parent(key, wr, repr_in, doc_in);

    _button = new Gtk::CheckButton;
    _tt.set_tip (*_button, tip);
    Gtk::Label *l = new Gtk::Label (label);
    l->set_use_underline (true);
    _button->add (*manage (l));
    _button->set_alignment (right? 1.0 : 0.0, 0.5);
    _toggled_connection = _button->signal_toggled().connect (sigc::mem_fun (*this, &RegisteredCheckButton::on_toggled));
}

void
RegisteredCheckButton::setActive (bool b)
{
    setProgrammatically = true;
    _button->set_active (b);
    //The slave button is greyed out if the master button is unchecked
    for (std::list<Gtk::ToggleButton*>::const_iterator i = _slavebuttons.begin(); i != _slavebuttons.end(); i++) {
        (*i)->set_sensitive(b);
    }
    setProgrammatically = false;
}

void
RegisteredCheckButton::on_toggled()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating())
        return;
    _wr->setUpdating (true);

    write_to_xml(_button->get_active() ? "true" : "false");
    //The slave button is greyed out if the master button is unchecked
    for (std::list<Gtk::ToggleButton*>::const_iterator i = _slavebuttons.begin(); i != _slavebuttons.end(); i++) {
        (*i)->set_sensitive(_button->get_active());
    }

    _wr->setUpdating (false);
}

RegisteredUnitMenu::RegisteredUnitMenu()
: _label(0), _sel(0)
{
}

RegisteredUnitMenu::~RegisteredUnitMenu()
{
    _changed_connection.disconnect();
    if (_label) delete _label;
    if (_sel) delete _sel;
}

void
RegisteredUnitMenu::init (const Glib::ustring& label, const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
{
    init_parent(key, wr, repr_in, doc_in);

    _label = new Gtk::Label (label, 1.0, 0.5);
    _label->set_use_underline (true);
    _sel = new UnitMenu ();
    _label->set_mnemonic_widget (*_sel);
    _sel->setUnitType (UNIT_TYPE_LINEAR);
    _changed_connection = _sel->signal_changed().connect (sigc::mem_fun (*this, &RegisteredUnitMenu::on_changed));
}

void
RegisteredUnitMenu::setUnit (const SPUnit* unit)
{
    _sel->setUnit (sp_unit_get_abbreviation (unit));
}

void
RegisteredUnitMenu::on_changed()
{
    if (_wr->isUpdating())
        return;

    Inkscape::SVGOStringStream os;
    os << _sel->getUnitAbbr();

    _wr->setUpdating (true);

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}


RegisteredScalarUnit::RegisteredScalarUnit()
: _widget(0), _um(0)
{
}

RegisteredScalarUnit::~RegisteredScalarUnit()
{
    if (_widget) delete _widget;
    _value_changed_connection.disconnect();
}

void
RegisteredScalarUnit::init (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, const RegisteredUnitMenu &rum, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
{
    init_parent(key, wr, repr_in, doc_in);

    _widget = new ScalarUnit (label, tip, UNIT_TYPE_LINEAR, "", "", rum._sel);
    _widget->initScalar (-1e6, 1e6);
    _widget->setUnit (rum._sel->getUnitAbbr());
    _widget->setDigits (2);
    _um = rum._sel;
    _value_changed_connection = _widget->signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredScalarUnit::on_value_changed));
}

ScalarUnit*
RegisteredScalarUnit::getSU()
{
    return _widget;
}

void
RegisteredScalarUnit::setValue (double val)
{
    if (_widget)
        _widget->setValue (val);
}

void
RegisteredScalarUnit::on_value_changed()
{
g_message("on_value_changed");
    if (_widget->setProgrammatically) {
        _widget->setProgrammatically = false;
        return;
    }
g_message("on_value_changed1");

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << _widget->getValue("");
    if (_um)
        os << _um->getUnitAbbr();

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}


RegisteredScalar::RegisteredScalar()
{
    _widget = NULL;
}

RegisteredScalar::~RegisteredScalar()
{
    if (_widget) 
        delete _widget;

    _value_changed_connection.disconnect();
}

void
RegisteredScalar::init ( const Glib::ustring& label, const Glib::ustring& tip, 
                         const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                         SPDocument * doc_in )
{
    init_parent(key, wr, repr_in, doc_in);

    _widget = new Scalar (label, tip);
    _widget->setRange (-1e6, 1e6);
    _widget->setDigits (2);
    _widget->setIncrements(0.1, 1.0);
    _value_changed_connection = _widget->signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredScalar::on_value_changed));
}

Scalar*
RegisteredScalar::getS()
{
    return _widget;
}

void
RegisteredScalar::setValue (double val)
{
    if (_widget)
        _widget->setValue (val);
}

void
RegisteredScalar::on_value_changed()
{
    if (_widget->setProgrammatically) {
        _widget->setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating()) {
        return;
    }
    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << _widget->getValue();

    _widget->set_sensitive(false);
    write_to_xml(os.str().c_str());
    _widget->set_sensitive(true);

    _wr->setUpdating (false);
}


RegisteredColorPicker::RegisteredColorPicker()
: _label(0), _cp(0)
{
}

RegisteredColorPicker::~RegisteredColorPicker()
{
    _changed_connection.disconnect();
    if (_cp) delete _cp;
    if (_label) delete _label;
}

void
RegisteredColorPicker::init (const Glib::ustring& label, const Glib::ustring& title, const Glib::ustring& tip, const Glib::ustring& ckey, const Glib::ustring& akey, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
{
    init_parent("", wr, repr_in, doc_in);

    _label = new Gtk::Label (label, 1.0, 0.5);
    _label->set_use_underline (true);
    _cp = new ColorPicker (title,tip,0,true);
    _label->set_mnemonic_widget (*_cp);
    _ckey = ckey;
    _akey = akey;
    _changed_connection = _cp->connectChanged (sigc::mem_fun (*this, &RegisteredColorPicker::on_changed));
}

void
RegisteredColorPicker::setRgba32 (guint32 rgba)
{
    _cp->setRgba32 (rgba);
}

void
RegisteredColorPicker::closeWindow()
{
    _cp->closeWindow();
}

void
RegisteredColorPicker::on_changed (guint32 rgba)
{
    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    // Use local repr here. When repr is specified, use that one, but
    // if repr==NULL, get the repr of namedview of active desktop.
    Inkscape::XML::Node *local_repr = repr;
    SPDocument *local_doc = doc;
    if (!local_repr) {
        // no repr specified, use active desktop's namedview's repr
        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        if (!dt)
            return;
        local_repr = SP_OBJECT_REPR (sp_desktop_namedview(dt));
        local_doc = sp_desktop_document(dt);
    }

    gchar c[32];
    sp_svg_write_color(c, sizeof(c), rgba);
    bool saved = sp_document_get_undo_sensitive (local_doc);
    sp_document_set_undo_sensitive (local_doc, false);
    local_repr->setAttribute(_ckey.c_str(), c);
    sp_repr_set_css_double(local_repr, _akey.c_str(), (rgba & 0xff) / 255.0);
    local_doc->rroot->setAttribute("sodipodi:modified", "true");
    sp_document_set_undo_sensitive (local_doc, saved);
    sp_document_done (local_doc, SP_VERB_NONE,
                      /* TODO: annotate */ "registered-widget.cpp: RegisteredColorPicker::on_changed");

    _wr->setUpdating (false);
}

RegisteredSuffixedInteger::RegisteredSuffixedInteger()
: _label(0),
  setProgrammatically(false),
  _sb(0),
  _adj(0.0,0.0,100.0,1.0,1.0,1.0),
  _suffix(0)
{
}

RegisteredSuffixedInteger::~RegisteredSuffixedInteger()
{
    _changed_connection.disconnect();
    if (_label) delete _label;
    if (_suffix) delete _suffix;
    if (_sb) delete _sb;
}

void
RegisteredSuffixedInteger::init (const Glib::ustring& label, const Glib::ustring& suffix, const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
{
    init_parent(key, wr, repr_in, doc_in);

    _label = new Gtk::Label (label);
    _label->set_alignment (1.0, 0.5);
    _label->set_use_underline();
    _sb = new Gtk::SpinButton (_adj, 1.0, 0);
    _sb->set_numeric();
    _label->set_mnemonic_widget (*_sb);
    _suffix = new Gtk::Label (suffix);
    _hbox.pack_start (*_sb, true, true, 0);
    _hbox.pack_start (*_suffix, false, false, 0);

    _changed_connection = _adj.signal_value_changed().connect (sigc::mem_fun(*this, &RegisteredSuffixedInteger::on_value_changed));
}

void
RegisteredSuffixedInteger::setValue (int i)
{
    setProgrammatically = true;
    _adj.set_value (i);
}

void
RegisteredSuffixedInteger::on_value_changed()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    int value = int(_adj.get_value());
    os << value;

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}

RegisteredRadioButtonPair::RegisteredRadioButtonPair()
: _hbox(0),
   setProgrammatically(false)
{
}

RegisteredRadioButtonPair::~RegisteredRadioButtonPair()
{
    _changed_connection.disconnect();
}

void
RegisteredRadioButtonPair::init (const Glib::ustring& label,
const Glib::ustring& label1, const Glib::ustring& label2,
const Glib::ustring& tip1, const Glib::ustring& tip2,
const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
{
    init_parent(key, wr, repr_in, doc_in);

    _hbox = new Gtk::HBox;
    _hbox->add (*manage (new Gtk::Label (label)));
    _rb1 = manage (new Gtk::RadioButton (label1, true));
    _hbox->add (*_rb1);
    Gtk::RadioButtonGroup group = _rb1->get_group();
    _rb2 = manage (new Gtk::RadioButton (group, label2, true));
    _hbox->add (*_rb2);
    _rb2->set_active();
    _tt.set_tip (*_rb1, tip1);
    _tt.set_tip (*_rb2, tip2);
    _changed_connection = _rb1->signal_toggled().connect (sigc::mem_fun (*this, &RegisteredRadioButtonPair::on_value_changed));
}

void
RegisteredRadioButtonPair::setValue (bool second)
{
    if (!_rb1 || !_rb2)
        return;

    setProgrammatically = true;
    if (second) {
        _rb2->set_active();
    } else {
        _rb1->set_active();
    }
}

void
RegisteredRadioButtonPair::on_value_changed()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    bool second = _rb2->get_active();
    write_to_xml(second ? "true" : "false");

    _wr->setUpdating (false);
}

/*#########################################
 * Registered POINT
 */

RegisteredPoint::RegisteredPoint()
{
    _widget = NULL;
}

RegisteredPoint::~RegisteredPoint()
{
    if (_widget) 
        delete _widget;

    _value_x_changed_connection.disconnect();
    _value_y_changed_connection.disconnect();
}

void
RegisteredPoint::init ( const Glib::ustring& label, const Glib::ustring& tip, 
                        const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, 
                        SPDocument* doc_in )
{
    init_parent(key, wr, repr_in, doc_in);

    _widget = new Point (label, tip);
    _widget->setRange (-1e6, 1e6);
    _widget->setDigits (2);
    _widget->setIncrements(0.1, 1.0);
    _value_x_changed_connection = _widget->signal_x_value_changed().connect (sigc::mem_fun (*this, &RegisteredPoint::on_value_changed));
    _value_y_changed_connection = _widget->signal_y_value_changed().connect (sigc::mem_fun (*this, &RegisteredPoint::on_value_changed));
}

Point*
RegisteredPoint::getPoint()
{
    return _widget;
}

void
RegisteredPoint::setValue (double xval, double yval)
{
    if (_widget)
        _widget->setValue(xval, yval);
}

void
RegisteredPoint::on_value_changed()
{
    if (_widget->setProgrammatically()) {
        _widget->clearProgrammatically();
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << _widget->getXValue() << "," << _widget->getYValue();

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}

/*#########################################
 * Registered RANDOM
 */

RegisteredRandom::RegisteredRandom()
{
    _widget = NULL;
}

RegisteredRandom::~RegisteredRandom()
{
    if (_widget) 
        delete _widget;

    _value_changed_connection.disconnect();
    _reseeded_connection.disconnect();
}

void
RegisteredRandom::init ( const Glib::ustring& label, const Glib::ustring& tip, 
                         const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                         SPDocument * doc_in )
{
    init_parent(key, wr, repr_in, doc_in);

    _widget = new Random (label, tip);
    _widget->setRange (-1e6, 1e6);
    _widget->setDigits (2);
    _widget->setIncrements(0.1, 1.0);
    _value_changed_connection = _widget->signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredRandom::on_value_changed));
    _reseeded_connection = _widget->signal_reseeded.connect(sigc::mem_fun(*this, &RegisteredRandom::on_value_changed));
}

Random*
RegisteredRandom::getR()
{
    return _widget;
}

void
RegisteredRandom::setValue (double val, long startseed)
{
    if (!_widget)
        return;

    _widget->setValue (val);
    _widget->setStartSeed(startseed);
}

void
RegisteredRandom::on_value_changed()
{
    if (_widget->setProgrammatically) {
        _widget->setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating()) {
        return;
    }
    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << _widget->getValue() << ';' << _widget->getStartSeed();

    _widget->set_sensitive(false);
    write_to_xml(os.str().c_str());
    _widget->set_sensitive(true);

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

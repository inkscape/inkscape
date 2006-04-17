/** \file
 *
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ui/widget/color-picker.h"
#include "ui/widget/registry.h"
#include "ui/widget/scalar-unit.h"

#include "helper/units.h"
#include "xml/repr.h"
#include "svg/svg-color.h"
#include "svg/stringstream.h"

#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-namedview.h"

#include "registered-widget.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

//---------------------------------------------------



//====================================================

RegisteredCheckButton::RegisteredCheckButton()
: _button(0)
{
}

RegisteredCheckButton::~RegisteredCheckButton()
{
    _toggled_connection.disconnect();
    if (_button) delete _button;
}

void
RegisteredCheckButton::init (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right)
{
    _button = new Gtk::CheckButton;
    _tt.set_tip (*_button, tip);
    Gtk::Label *l = new Gtk::Label (label);
    l->set_use_underline (true);
    _button->add (*manage (l));
    _button->set_alignment (right? 1.0 : 0.0, 0.5);
    _key = key;
    _wr = &wr;
    _toggled_connection = _button->signal_toggled().connect (sigc::mem_fun (*this, &RegisteredCheckButton::on_toggled));
}

void
RegisteredCheckButton::setActive (bool b)
{
    _button->set_active (b);
}

void
RegisteredCheckButton::on_toggled()
{
    if (_wr->isUpdating())
        return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }

    SPDocument *doc = sp_desktop_document(dt);

    Inkscape::XML::Node *repr = SP_OBJECT_REPR (sp_desktop_namedview(dt));
    _wr->setUpdating (true);

    gboolean saved = sp_document_get_undo_sensitive (doc);
    sp_document_set_undo_sensitive (doc, FALSE);
    sp_repr_set_boolean(repr, _key.c_str(), _button->get_active());
    doc->rroot->setAttribute("sodipodi:modified", "true");
    sp_document_set_undo_sensitive (doc, saved);
    sp_document_done (doc);
    
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
RegisteredUnitMenu::init (const Glib::ustring& label, const Glib::ustring& key, Registry& wr)
{
    _label = new Gtk::Label (label, 1.0, 0.5);
    _label->set_use_underline (true);
    _sel = new UnitMenu ();
    _label->set_mnemonic_widget (*_sel);
    _sel->setUnitType (UNIT_TYPE_LINEAR);
    _wr = &wr;
    _key = key;
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

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) 
        return;

    Inkscape::SVGOStringStream os;
    os << _sel->getUnitAbbr();

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
RegisteredScalarUnit::init (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, const RegisteredUnitMenu &rum, Registry& wr)
{
    _widget = new ScalarUnit (label, tip, UNIT_TYPE_LINEAR, "", "", rum._sel);
    _widget->initScalar (-1e6, 1e6);
    _widget->setUnit (rum._sel->getUnitAbbr());
    _widget->setDigits (2);
    _key = key;
    _um = rum._sel;
    _value_changed_connection = _widget->signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredScalarUnit::on_value_changed));
    _wr = &wr;
}

ScalarUnit*
RegisteredScalarUnit::getSU()
{
    return _widget;
}

void 
RegisteredScalarUnit::setValue (double val)
{
    _widget->setValue (val);
    on_value_changed();
}

void
RegisteredScalarUnit::on_value_changed()
{
    if (_wr->isUpdating())
        return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) 
        return;

    Inkscape::SVGOStringStream os;
    os << _widget->getValue("");
    if (_um)
        os << _um->getUnitAbbr();

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
RegisteredColorPicker::init (const Glib::ustring& label, const Glib::ustring& title, const Glib::ustring& tip, const Glib::ustring& ckey, const Glib::ustring& akey, Registry& wr)
{
    _label = new Gtk::Label (label, 1.0, 0.5);
    _label->set_use_underline (true);
    _cp = new ColorPicker (title,tip,0,true);
    _label->set_mnemonic_widget (*_cp);
    _ckey = ckey;
    _akey = akey;
    _wr = &wr;
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
    if (_wr->isUpdating() || !SP_ACTIVE_DESKTOP)
        return;

    _wr->setUpdating (true);
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(SP_ACTIVE_DESKTOP));
    gchar c[32];
    sp_svg_write_color(c, 32, rgba);
    repr->setAttribute(_ckey.c_str(), c);
    sp_repr_set_css_double(repr, _akey.c_str(), (rgba & 0xff) / 255.0);
    _wr->setUpdating (false);
}

RegisteredSuffixedInteger::RegisteredSuffixedInteger()
: _label(0), _sb(0),
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
RegisteredSuffixedInteger::init (const Glib::ustring& label, const Glib::ustring& suffix, const Glib::ustring& key, Registry& wr)
{
    _key = key;
    _label = new Gtk::Label (label);
    _label->set_alignment (1.0, 0.5);
    _label->set_use_underline();
    _sb = new Gtk::SpinButton (_adj, 1.0, 0);
    _label->set_mnemonic_widget (*_sb);
    _suffix = new Gtk::Label (suffix);
    _hbox.pack_start (*_sb, true, true, 0);
    _hbox.pack_start (*_suffix, false, false, 0);

    _changed_connection = _adj.signal_value_changed().connect (sigc::mem_fun(*this, &RegisteredSuffixedInteger::on_value_changed));
    _wr = &wr;
}

void 
RegisteredSuffixedInteger::setValue (int i)
{
    _adj.set_value (i);
}

void
RegisteredSuffixedInteger::on_value_changed()
{
    if (_wr->isUpdating() || !SP_ACTIVE_DESKTOP)
        return;

    _wr->setUpdating (true);
    
    SPDesktop* dt = SP_ACTIVE_DESKTOP;
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(dt));
    Inkscape::SVGOStringStream os;
    int value = int(_adj.get_value());
    os << value;

    repr->setAttribute(_key.c_str(), os.str().c_str());
    sp_document_done(sp_desktop_document(dt));
    
    _wr->setUpdating (false);
}

RegisteredRadioButtonPair::RegisteredRadioButtonPair()
: _hbox(0)
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
const Glib::ustring& key, Registry& wr)
{
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
    _key = key;
    _wr = &wr;
    _changed_connection = _rb1->signal_toggled().connect (sigc::mem_fun (*this, &RegisteredRadioButtonPair::on_value_changed));
}

void 
RegisteredRadioButtonPair::setValue (bool second)
{
    if (second) _rb2->set_active();
    else        _rb1->set_active();
}

void
RegisteredRadioButtonPair::on_value_changed()
{
    if (_wr->isUpdating())
        return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) 
        return;

    _wr->setUpdating (true);
    
    bool second = _rb2->get_active();
    SPDocument *doc = sp_desktop_document(dt);
    gboolean saved = sp_document_get_undo_sensitive (doc);
    sp_document_set_undo_sensitive (doc, FALSE);
    Inkscape::XML::Node *repr = SP_OBJECT_REPR (sp_desktop_namedview(dt));
    repr->setAttribute(_key.c_str(), second ? "true" : "false");
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

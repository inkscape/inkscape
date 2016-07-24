/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *   Abhishek Sharma
 *
 * Copyright (C) 2000 - 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "registered-widget.h"
#include <gtkmm/radiobutton.h>

#include "ui/widget/color-picker.h"
#include "ui/widget/registry.h"
#include "ui/widget/scalar-unit.h"
#include "ui/widget/point.h"
#include "ui/widget/random.h"
#include "widgets/spinbutton-events.h"

#include "xml/repr.h"
#include "svg/svg-color.h"
#include "svg/stringstream.h"

#include "verbs.h"

// for interruptability bug:
#include "display/sp-canvas.h"

#include "desktop.h"


#include "sp-root.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/*#########################################
 * Registered CHECKBUTTON
 */

RegisteredCheckButton::~RegisteredCheckButton()
{
    _toggled_connection.disconnect();
}

RegisteredCheckButton::RegisteredCheckButton (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right, Inkscape::XML::Node* repr_in, SPDocument *doc_in, char const *active_str, char const *inactive_str)
    : RegisteredWidget<Gtk::CheckButton>()
    , _active_str(active_str)
    , _inactive_str(inactive_str)
{
    init_parent(key, wr, repr_in, doc_in);

    setProgrammatically = false;

    set_tooltip_text (tip);
    Gtk::Label *l = new Gtk::Label (label);
    l->set_use_underline (true);
    add (*manage (l));
    set_alignment (right? 1.0 : 0.0, 0.5);
    _toggled_connection = signal_toggled().connect (sigc::mem_fun (*this, &RegisteredCheckButton::on_toggled));
}

void
RegisteredCheckButton::setActive (bool b)
{
    setProgrammatically = true;
    set_active (b);
    //The slave button is greyed out if the master button is unchecked
    for (std::list<Gtk::Widget*>::const_iterator i = _slavewidgets.begin(); i != _slavewidgets.end(); ++i) {
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

    write_to_xml(get_active() ? _active_str : _inactive_str);
    //The slave button is greyed out if the master button is unchecked
    for (std::list<Gtk::Widget*>::const_iterator i = _slavewidgets.begin(); i != _slavewidgets.end(); ++i) {
        (*i)->set_sensitive(get_active());
    }

    _wr->setUpdating (false);
}

/*#########################################
 * Registered TOGGLEBUTTON
 */

RegisteredToggleButton::~RegisteredToggleButton()
{
    _toggled_connection.disconnect();
}

RegisteredToggleButton::RegisteredToggleButton (const Glib::ustring& /*label*/, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right, Inkscape::XML::Node* repr_in, SPDocument *doc_in, char const *icon_active, char const *icon_inactive)
    : RegisteredWidget<Gtk::ToggleButton>()
{
    init_parent(key, wr, repr_in, doc_in);
    setProgrammatically = false;
    set_tooltip_text (tip);
    set_alignment (right? 1.0 : 0.0, 0.5);
    _toggled_connection = signal_toggled().connect (sigc::mem_fun (*this, &RegisteredToggleButton::on_toggled));
}

void
RegisteredToggleButton::setActive (bool b)
{
    setProgrammatically = true;
    set_active (b);
    //The slave button is greyed out if the master button is untoggled
    for (std::list<Gtk::Widget*>::const_iterator i = _slavewidgets.begin(); i != _slavewidgets.end(); ++i) {
        (*i)->set_sensitive(b);
    }
    setProgrammatically = false;
}

void
RegisteredToggleButton::on_toggled()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating())
        return;
    _wr->setUpdating (true);

    write_to_xml(get_active() ? "true" : "false");
    //The slave button is greyed out if the master button is untoggled
    for (std::list<Gtk::Widget*>::const_iterator i = _slavewidgets.begin(); i != _slavewidgets.end(); ++i) {
        (*i)->set_sensitive(get_active());
    }

    _wr->setUpdating (false);
}

/*#########################################
 * Registered UNITMENU
 */

RegisteredUnitMenu::~RegisteredUnitMenu()
{
    _changed_connection.disconnect();
}

RegisteredUnitMenu::RegisteredUnitMenu (const Glib::ustring& label, const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
    :  RegisteredWidget<Labelled> (label, "" /*tooltip*/, new UnitMenu())
{
    init_parent(key, wr, repr_in, doc_in);

    getUnitMenu()->setUnitType (UNIT_TYPE_LINEAR);
    _changed_connection = getUnitMenu()->signal_changed().connect (sigc::mem_fun (*this, &RegisteredUnitMenu::on_changed));
}

void
RegisteredUnitMenu::setUnit (Glib::ustring unit)
{
    getUnitMenu()->setUnit(unit);
}

void
RegisteredUnitMenu::on_changed()
{
    if (_wr->isUpdating())
        return;

    Inkscape::SVGOStringStream os;
    os << getUnitMenu()->getUnitAbbr();

    _wr->setUpdating (true);

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}


/*#########################################
 * Registered SCALARUNIT
 */

RegisteredScalarUnit::~RegisteredScalarUnit()
{
    _value_changed_connection.disconnect();
}

RegisteredScalarUnit::RegisteredScalarUnit (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, const RegisteredUnitMenu &rum, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in, RSU_UserUnits user_units)
    : RegisteredWidget<ScalarUnit>(label, tip, UNIT_TYPE_LINEAR, "", "", rum.getUnitMenu()),
      _um(0)
{
    init_parent(key, wr, repr_in, doc_in);

    setProgrammatically = false;

    initScalar (-1e6, 1e6);
    setUnit (rum.getUnitMenu()->getUnitAbbr());
    setDigits (2);
    _um = rum.getUnitMenu();
    _user_units = user_units;
    _value_changed_connection = signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredScalarUnit::on_value_changed));
}


void
RegisteredScalarUnit::on_value_changed()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    if (_user_units != RSU_none) {
        // Output length in 'user units', taking into account scale in 'x' or 'y'.
        double scale = 1.0;
        if (doc) {
            SPRoot *root = doc->getRoot();
            if (root->viewBox_set) {
                // check to see if scaling is uniform
                if(Geom::are_near((root->viewBox.width() * root->height.computed) / (root->width.computed * root->viewBox.height()), 1.0, Geom::EPSILON)) {
                    scale = (root->viewBox.width() / root->width.computed + root->viewBox.height() / root->height.computed)/2.0;
                } else if (_user_units == RSU_x) { 
                    scale = root->viewBox.width() / root->width.computed;
                } else {
                    scale = root->viewBox.height() / root->height.computed;
                }
            }
        }
        os << getValue("px") * scale;
    } else {
        // Output using unit identifiers.
        os << getValue("");
        if (_um)
            os << _um->getUnitAbbr();
    }

    write_to_xml(os.str().c_str());
    _wr->setUpdating (false);
}


/*#########################################
 * Registered SCALAR
 */

RegisteredScalar::~RegisteredScalar()
{
    _value_changed_connection.disconnect();
}

RegisteredScalar::RegisteredScalar ( const Glib::ustring& label, const Glib::ustring& tip,
                         const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                         SPDocument * doc_in )
    : RegisteredWidget<Scalar>(label, tip)
{
    init_parent(key, wr, repr_in, doc_in);

    setProgrammatically = false;

    setRange (-1e6, 1e6);
    setDigits (2);
    setIncrements(0.1, 1.0);
    _value_changed_connection = signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredScalar::on_value_changed));
}

void
RegisteredScalar::on_value_changed()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating()) {
        return;
    }
    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << getValue();

    set_sensitive(false);
    write_to_xml(os.str().c_str());
    set_sensitive(true);

    _wr->setUpdating (false);
}


/*#########################################
 * Registered TEXT
 */

RegisteredText::~RegisteredText()
{
    _activate_connection.disconnect();
}

RegisteredText::RegisteredText ( const Glib::ustring& label, const Glib::ustring& tip,
                         const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                         SPDocument * doc_in )
    : RegisteredWidget<Text>(label, tip)
{
    init_parent(key, wr, repr_in, doc_in);

    setProgrammatically = false;

    setText("");
    _activate_connection = signal_activate().connect (sigc::mem_fun (*this, &RegisteredText::on_activate));
}

void
RegisteredText::on_activate()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating()) {
        return;
    }
    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << getText();

    set_sensitive(false);
    write_to_xml(os.str().c_str());
    set_sensitive(true);

    setText(os.str().c_str());

    _wr->setUpdating (false);
}


/*#########################################
 * Registered COLORPICKER
 */

RegisteredColorPicker::RegisteredColorPicker(const Glib::ustring& label,
                                             const Glib::ustring& title,
                                             const Glib::ustring& tip,
                                             const Glib::ustring& ckey,
                                             const Glib::ustring& akey,
                                             Registry& wr,
                                             Inkscape::XML::Node* repr_in,
                                             SPDocument *doc_in)
    : RegisteredWidget<ColorPicker> (title, tip, 0, true)
{
    init_parent("", wr, repr_in, doc_in);

    _label = new Gtk::Label (label, 1.0, 0.5);
    _label->set_use_underline (true);
    _label->set_mnemonic_widget (*this);
    _ckey = ckey;
    _akey = akey;
    _changed_connection = connectChanged (sigc::mem_fun (*this, &RegisteredColorPicker::on_changed));
}

RegisteredColorPicker::~RegisteredColorPicker()
{
    _changed_connection.disconnect();
}

void
RegisteredColorPicker::setRgba32 (guint32 rgba)
{
    ColorPicker::setRgba32 (rgba);
}

void
RegisteredColorPicker::closeWindow()
{
    ColorPicker::closeWindow();
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
        local_repr = dt->getNamedView()->getRepr();
        local_doc = dt->getDocument();
    }

    gchar c[32];
    sp_svg_write_color(c, sizeof(c), rgba);
    bool saved = DocumentUndo::getUndoSensitive(local_doc);
    DocumentUndo::setUndoSensitive(local_doc, false);
    local_repr->setAttribute(_ckey.c_str(), c);
    sp_repr_set_css_double(local_repr, _akey.c_str(), (rgba & 0xff) / 255.0);
    DocumentUndo::setUndoSensitive(local_doc, saved);

    local_doc->setModifiedSinceSave();
    DocumentUndo::done(local_doc, SP_VERB_NONE,
                       /* TODO: annotate */ "registered-widget.cpp: RegisteredColorPicker::on_changed");

    _wr->setUpdating (false);
}


/*#########################################
 * Registered SUFFIXEDINTEGER
 */

RegisteredSuffixedInteger::~RegisteredSuffixedInteger()
{
    _changed_connection.disconnect();
}

RegisteredSuffixedInteger::RegisteredSuffixedInteger (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& suffix, const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
    : RegisteredWidget<Scalar>(label, tip, 0, suffix),
      setProgrammatically(false)
{
    init_parent(key, wr, repr_in, doc_in);

    setRange (0, 1e6);
    setDigits (0);
    setIncrements(1, 10);

    _changed_connection = signal_value_changed().connect (sigc::mem_fun(*this, &RegisteredSuffixedInteger::on_value_changed));
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
    os << getValue();

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}


/*#########################################
 * Registered RADIOBUTTONPAIR
 */

RegisteredRadioButtonPair::~RegisteredRadioButtonPair()
{
    _changed_connection.disconnect();
}

RegisteredRadioButtonPair::RegisteredRadioButtonPair (const Glib::ustring& label,
        const Glib::ustring& label1, const Glib::ustring& label2,
        const Glib::ustring& tip1, const Glib::ustring& tip2,
        const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
    : RegisteredWidget<Gtk::HBox>(),
      _rb1(NULL),
      _rb2(NULL)
{
    init_parent(key, wr, repr_in, doc_in);

    setProgrammatically = false;

    add(*Gtk::manage(new Gtk::Label(label)));
    _rb1 = Gtk::manage(new Gtk::RadioButton(label1, true));
    add (*_rb1);
    Gtk::RadioButtonGroup group = _rb1->get_group();
    _rb2 = Gtk::manage(new Gtk::RadioButton(group, label2, true));
    add (*_rb2);
    _rb2->set_active();
    _rb1->set_tooltip_text(tip1);
    _rb2->set_tooltip_text(tip2);
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

RegisteredPoint::~RegisteredPoint()
{
    _value_x_changed_connection.disconnect();
    _value_y_changed_connection.disconnect();
}

RegisteredPoint::RegisteredPoint ( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                        SPDocument* doc_in )
    : RegisteredWidget<Point> (label, tip)
{
    init_parent(key, wr, repr_in, doc_in);

    setRange (-1e6, 1e6);
    setDigits (2);
    setIncrements(0.1, 1.0);
    _value_x_changed_connection = signal_x_value_changed().connect (sigc::mem_fun (*this, &RegisteredPoint::on_value_changed));
    _value_y_changed_connection = signal_y_value_changed().connect (sigc::mem_fun (*this, &RegisteredPoint::on_value_changed));
}

void
RegisteredPoint::on_value_changed()
{
    if (setProgrammatically()) {
        clearProgrammatically();
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << getXValue() << "," << getYValue();

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}

/*#########################################
 * Registered TRANSFORMEDPOINT
 */

RegisteredTransformedPoint::~RegisteredTransformedPoint()
{
    _value_x_changed_connection.disconnect();
    _value_y_changed_connection.disconnect();
}

RegisteredTransformedPoint::RegisteredTransformedPoint ( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                        SPDocument* doc_in )
    : RegisteredWidget<Point> (label, tip),
      to_svg(Geom::identity())
{
    init_parent(key, wr, repr_in, doc_in);

    setRange (-1e6, 1e6);
    setDigits (2);
    setIncrements(0.1, 1.0);
    _value_x_changed_connection = signal_x_value_changed().connect (sigc::mem_fun (*this, &RegisteredTransformedPoint::on_value_changed));
    _value_y_changed_connection = signal_y_value_changed().connect (sigc::mem_fun (*this, &RegisteredTransformedPoint::on_value_changed));
}

void
RegisteredTransformedPoint::setValue(Geom::Point const & p)
{
    Geom::Point new_p = p * to_svg.inverse();
    Point::setValue(new_p);  // the Point widget should display things in canvas coordinates
}

void
RegisteredTransformedPoint::setTransform(Geom::Affine const & canvas_to_svg)
{
    // check if matrix is singular / has inverse
    if ( ! canvas_to_svg.isSingular() ) {
        to_svg = canvas_to_svg;
    } else {
        // set back to default
        to_svg = Geom::identity();
    }
}

void
RegisteredTransformedPoint::on_value_changed()
{
    if (setProgrammatically()) {
        clearProgrammatically();
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Geom::Point pos = getValue() * to_svg;

    Inkscape::SVGOStringStream os;
    os << pos;

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}

/*#########################################
 * Registered TRANSFORMEDPOINT
 */

RegisteredVector::~RegisteredVector()
{
    _value_x_changed_connection.disconnect();
    _value_y_changed_connection.disconnect();
}

RegisteredVector::RegisteredVector ( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                        SPDocument* doc_in )
    : RegisteredWidget<Point> (label, tip),
      _polar_coords(false)
{
    init_parent(key, wr, repr_in, doc_in);

    setRange (-1e6, 1e6);
    setDigits (2);
    setIncrements(0.1, 1.0);
    _value_x_changed_connection = signal_x_value_changed().connect (sigc::mem_fun (*this, &RegisteredVector::on_value_changed));
    _value_y_changed_connection = signal_y_value_changed().connect (sigc::mem_fun (*this, &RegisteredVector::on_value_changed));
}

void
RegisteredVector::setValue(Geom::Point const & p)
{
    if (!_polar_coords) {
        Point::setValue(p);
    } else {
        Geom::Point polar;
        polar[Geom::X] = atan2(p) *180/M_PI;
        polar[Geom::Y] = p.length();
        Point::setValue(polar);
    }
}

void
RegisteredVector::setValue(Geom::Point const & p, Geom::Point const & origin)
{
    RegisteredVector::setValue(p);
    _origin = origin;
}

void RegisteredVector::setPolarCoords(bool polar_coords)
{
    _polar_coords = polar_coords;
    if (polar_coords) {
        xwidget.setLabelText("Angle:");
        ywidget.setLabelText("Distance:");
    } else {
        xwidget.setLabelText("X:");
        ywidget.setLabelText("Y:");
    }
}

void
RegisteredVector::on_value_changed()
{
    if (setProgrammatically()) {
        clearProgrammatically();
        return;
    }

    if (_wr->isUpdating())
        return;

    _wr->setUpdating (true);

    Geom::Point origin = _origin;
    Geom::Point vector = getValue();
    if (_polar_coords) {
        vector = Geom::Point::polar(vector[Geom::X]*M_PI/180, vector[Geom::Y]);
    }

    Inkscape::SVGOStringStream os;
    os << origin << " , " << vector;

    write_to_xml(os.str().c_str());

    _wr->setUpdating (false);
}

/*#########################################
 * Registered RANDOM
 */

RegisteredRandom::~RegisteredRandom()
{
    _value_changed_connection.disconnect();
    _reseeded_connection.disconnect();
}

RegisteredRandom::RegisteredRandom ( const Glib::ustring& label, const Glib::ustring& tip,
                         const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in,
                         SPDocument * doc_in )
    : RegisteredWidget<Random> (label, tip)
{
    init_parent(key, wr, repr_in, doc_in);

    setProgrammatically = false;

    setRange (-1e6, 1e6);
    setDigits (2);
    setIncrements(0.1, 1.0);
    _value_changed_connection = signal_value_changed().connect (sigc::mem_fun (*this, &RegisteredRandom::on_value_changed));
    _reseeded_connection = signal_reseeded.connect(sigc::mem_fun(*this, &RegisteredRandom::on_value_changed));
}

void
RegisteredRandom::setValue (double val, long startseed)
{
    Scalar::setValue (val);
    setStartSeed(startseed);
}

void
RegisteredRandom::on_value_changed()
{
    if (setProgrammatically) {
        setProgrammatically = false;
        return;
    }

    if (_wr->isUpdating()) {
        return;
    }
    _wr->setUpdating (true);

    Inkscape::SVGOStringStream os;
    os << getValue() << ';' << getStartSeed();

    set_sensitive(false);
    write_to_xml(os.str().c_str());
    set_sensitive(true);

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

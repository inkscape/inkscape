/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Abhishek Sharma
 *
 * Copyright (C) 2005-2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_REGISTERED_WIDGET__H_
#define INKSCAPE_UI_WIDGET_REGISTERED_WIDGET__H_

#include "ui/widget/scalar.h"
#include <2geom/affine.h>
#include "xml/node.h"
#include "registry.h"

#include "ui/widget/scalar-unit.h"
#include "ui/widget/point.h"
#include "ui/widget/text.h"
#include "ui/widget/random.h"
#include "ui/widget/unit-menu.h"
#include "ui/widget/color-picker.h"
#include "inkscape.h"

#include "document.h"
#include "document-undo.h"
#include "desktop.h"
#include "sp-namedview.h"

#include <gtkmm/checkbutton.h>

class SPDocument;

namespace Gtk {
    class HScale;
    class RadioButton;
    class SpinButton;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class Registry;

template <class W>
class RegisteredWidget : public W {
public:
    void set_undo_parameters(const unsigned int _event_type, Glib::ustring _event_description)
    {
        event_type = _event_type;
        event_description = _event_description;
        write_undo = true;
    }
    void set_xml_target(Inkscape::XML::Node *xml_node, SPDocument *document)
    {
        repr = xml_node;
        doc = document;
    }

    bool is_updating() {if (_wr) return _wr->isUpdating(); else return false;}

protected:
    RegisteredWidget() : W() { construct(); }
    template< typename A >
    explicit RegisteredWidget( A& a ): W( a ) { construct(); }
    template< typename A, typename B >
    RegisteredWidget( A& a, B& b ): W( a, b ) { construct(); }
    template< typename A, typename B, typename C >
    RegisteredWidget( A& a, B& b, C* c ): W( a, b, c ) { construct(); }
    template< typename A, typename B, typename C >
    RegisteredWidget( A& a, B& b, C& c ): W( a, b, c ) { construct(); }
    template< typename A, typename B, typename C, typename D >
    RegisteredWidget( A& a, B& b, C c, D d ): W( a, b, c, d ) { construct(); }
    template< typename A, typename B, typename C, typename D, typename E , typename F>
    RegisteredWidget( A& a, B& b, C c, D& d, E& e, F* f): W( a, b, c, d, e, f) { construct(); }

    virtual ~RegisteredWidget() {};

    void init_parent(const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
    {
        _wr = &wr;
        _key = key;
        repr = repr_in;
        doc = doc_in;
        if (repr && !doc)  // doc cannot be NULL when repr is not NULL
            g_warning("Initialization of registered widget using defined repr but with doc==NULL");
    }

    void write_to_xml(const char * svgstr)
    {
        // Use local repr here. When repr is specified, use that one, but
        // if repr==NULL, get the repr of namedview of active desktop.
        Inkscape::XML::Node *local_repr = repr;
        SPDocument *local_doc = doc;
        if (!local_repr) {
            // no repr specified, use active desktop's namedview's repr
            SPDesktop* dt = SP_ACTIVE_DESKTOP;
            local_repr = reinterpret_cast<SPObject *>(dt->getNamedView())->getRepr();
            local_doc = dt->getDocument();
        }

        bool saved = DocumentUndo::getUndoSensitive(local_doc);
        DocumentUndo::setUndoSensitive(local_doc, false);
        if (!write_undo) {
            local_repr->setAttribute(_key.c_str(), svgstr);
        }
        DocumentUndo::setUndoSensitive(local_doc, saved);

        local_doc->setModifiedSinceSave();

        if (write_undo) {
            local_repr->setAttribute(_key.c_str(), svgstr);
            DocumentUndo::done(local_doc, event_type, event_description);
        }
    }

    Registry * _wr;
    Glib::ustring _key;
    Inkscape::XML::Node * repr;
    SPDocument * doc;
    unsigned int event_type;
    Glib::ustring event_description;
    bool write_undo;

private:
    void construct() {
        _wr = NULL;
        repr = NULL;
        doc = NULL;
        write_undo = false;
        event_type = -1;
    }
};

//#######################################################

class RegisteredCheckButton : public RegisteredWidget<Gtk::CheckButton> {
public:
    virtual ~RegisteredCheckButton();
    RegisteredCheckButton (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right=true, Inkscape::XML::Node* repr_in=NULL, SPDocument *doc_in=NULL, char const *active_str = "true", char const *inactive_str = "false");

    void setActive (bool);

    std::list<Gtk::Widget*> _slavewidgets;

    // a slave button is only sensitive when the master button is active
    // i.e. a slave button is greyed-out when the master button is not checked

    void setSlaveWidgets(std::list<Gtk::Widget*> btns) {
        _slavewidgets = btns;
    }

    bool setProgrammatically; // true if the value was set by setActive, not changed by the user;
                                // if a callback checks it, it must reset it back to false

protected:
    char const *_active_str, *_inactive_str;
    sigc::connection  _toggled_connection;
    void on_toggled();
};

class RegisteredToggleButton : public RegisteredWidget<Gtk::ToggleButton> {
public:
    virtual ~RegisteredToggleButton();
    RegisteredToggleButton (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right=true, Inkscape::XML::Node* repr_in=NULL, SPDocument *doc_in=NULL, char const *icon_active = "true", char const *icon_inactive = "false");

    void setActive (bool);

    std::list<Gtk::Widget*> _slavewidgets;

    // a slave button is only sensitive when the master button is active
    // i.e. a slave button is greyed-out when the master button is not checked

    void setSlaveWidgets(std::list<Gtk::Widget*> btns) {
        _slavewidgets = btns;
    }

    bool setProgrammatically; // true if the value was set by setActive, not changed by the user;
                                // if a callback checks it, it must reset it back to false

protected:
    sigc::connection  _toggled_connection;
    void on_toggled();
};

class RegisteredUnitMenu : public RegisteredWidget<Labelled> {
public:
    ~RegisteredUnitMenu();
    RegisteredUnitMenu ( const Glib::ustring& label,
                         const Glib::ustring& key,
                         Registry& wr,
                         Inkscape::XML::Node* repr_in = NULL,
                         SPDocument *doc_in = NULL );

    void setUnit (const Glib::ustring);
    Unit const * getUnit() const { return static_cast<UnitMenu*>(_widget)->getUnit(); };
    UnitMenu* getUnitMenu() const { return static_cast<UnitMenu*>(_widget); };
    sigc::connection _changed_connection;

protected:
    void on_changed();
};

// Allow RegisteredScalarUnit to output lengths in 'user units' (which may have direction dependent
// scale factors).
enum RSU_UserUnits {
    RSU_none,
    RSU_x,
    RSU_y
};

class RegisteredScalarUnit : public RegisteredWidget<ScalarUnit> {
public:
    ~RegisteredScalarUnit();
    RegisteredScalarUnit ( const Glib::ustring& label,
                           const Glib::ustring& tip,
                           const Glib::ustring& key,
                           const RegisteredUnitMenu &rum,
                           Registry& wr,
                           Inkscape::XML::Node* repr_in = NULL,
                           SPDocument *doc_in = NULL,
                           RSU_UserUnits _user_units = RSU_none );

protected:
    sigc::connection  _value_changed_connection;
    UnitMenu         *_um;
    void on_value_changed();
    RSU_UserUnits _user_units;
};

class RegisteredScalar : public RegisteredWidget<Scalar> {
public:
    virtual ~RegisteredScalar();
    RegisteredScalar (const Glib::ustring& label,
            const Glib::ustring& tip,
            const Glib::ustring& key,
            Registry& wr,
            Inkscape::XML::Node* repr_in = NULL,
            SPDocument *doc_in = NULL );

protected:
    sigc::connection  _value_changed_connection;
    void on_value_changed();
};

class RegisteredText : public RegisteredWidget<Text> {
public:
    virtual ~RegisteredText();
    RegisteredText (const Glib::ustring& label,
            const Glib::ustring& tip,
            const Glib::ustring& key,
            Registry& wr,
            Inkscape::XML::Node* repr_in = NULL,
            SPDocument *doc_in = NULL );

protected:
    sigc::connection  _activate_connection;
    void on_activate();
};

class RegisteredColorPicker : public RegisteredWidget<ColorPicker> {
public:
    virtual ~RegisteredColorPicker();

    RegisteredColorPicker (const Glib::ustring& label,
                           const Glib::ustring& title,
                           const Glib::ustring& tip,
                           const Glib::ustring& ckey,
                           const Glib::ustring& akey,
                           Registry& wr,
                           Inkscape::XML::Node* repr_in = NULL,
                           SPDocument *doc_in = NULL);

    void setRgba32 (guint32);
    void closeWindow();

    Gtk::Label *_label;

protected:
    Glib::ustring _ckey, _akey;
    void on_changed (guint32);
    sigc::connection _changed_connection;
};

class RegisteredSuffixedInteger : public RegisteredWidget<Scalar> {
public:
    virtual ~RegisteredSuffixedInteger();
    RegisteredSuffixedInteger ( const Glib::ustring& label,
                                const Glib::ustring& tip, 
                                const Glib::ustring& suffix,
                                const Glib::ustring& key,
                                Registry& wr,
                                Inkscape::XML::Node* repr_in = NULL,
                                SPDocument *doc_in = NULL );

    bool setProgrammatically; // true if the value was set by setValue, not changed by the user;
                                // if a callback checks it, it must reset it back to false

protected:
    sigc::connection _changed_connection;
    void on_value_changed();
};

class RegisteredRadioButtonPair : public RegisteredWidget<Gtk::HBox> {
public:
    virtual ~RegisteredRadioButtonPair();
    RegisteredRadioButtonPair ( const Glib::ustring& label,
                                const Glib::ustring& label1,
                                const Glib::ustring& label2,
                                const Glib::ustring& tip1,
                                const Glib::ustring& tip2,
                                const Glib::ustring& key,
                                Registry& wr,
                                Inkscape::XML::Node* repr_in = NULL,
                                SPDocument *doc_in = NULL );

    void setValue (bool second);

    bool setProgrammatically; // true if the value was set by setValue, not changed by the user;
                                    // if a callback checks it, it must reset it back to false
protected:
    Gtk::RadioButton *_rb1, *_rb2;
    sigc::connection _changed_connection;
    void on_value_changed();
};

class RegisteredPoint : public RegisteredWidget<Point> {
public:
    virtual ~RegisteredPoint();
    RegisteredPoint ( const Glib::ustring& label,
                      const Glib::ustring& tip,
                      const Glib::ustring& key,
                      Registry& wr,
                      Inkscape::XML::Node* repr_in = NULL,
                      SPDocument *doc_in = NULL );

protected:
    sigc::connection  _value_x_changed_connection;
    sigc::connection  _value_y_changed_connection;
    void on_value_changed();
};


class RegisteredTransformedPoint : public RegisteredWidget<Point> {
public:
    virtual ~RegisteredTransformedPoint();
    RegisteredTransformedPoint (  const Glib::ustring& label,
                                  const Glib::ustring& tip,
                                  const Glib::ustring& key,
                                  Registry& wr,
                                  Inkscape::XML::Node* repr_in = NULL,
                                  SPDocument *doc_in = NULL );

    // redefine setValue, because transform must be applied
    void setValue(Geom::Point const & p);

    void setTransform(Geom::Affine const & canvas_to_svg);

protected:
    sigc::connection  _value_x_changed_connection;
    sigc::connection  _value_y_changed_connection;
    void on_value_changed();

    Geom::Affine to_svg;
};


class RegisteredVector : public RegisteredWidget<Point> {
public:
    virtual ~RegisteredVector();
    RegisteredVector (const Glib::ustring& label,
                      const Glib::ustring& tip,
                      const Glib::ustring& key,
                      Registry& wr,
                      Inkscape::XML::Node* repr_in = NULL,
                      SPDocument *doc_in = NULL );

    // redefine setValue, because transform must be applied
    void setValue(Geom::Point const & p);
    void setValue(Geom::Point const & p, Geom::Point const & origin);

    /**
     * Changes the widgets text to polar coordinates. The SVG output will still be a normal carthesian vector.
     * Careful: when calling getValue(), the return value's X-coord will be the angle, Y-value will be the distance/length. 
     * After changing the coords type (polar/non-polar), the value has to be reset (setValue).
     */
    void setPolarCoords(bool polar_coords = true);

protected:
    sigc::connection  _value_x_changed_connection;
    sigc::connection  _value_y_changed_connection;
    void on_value_changed();

    Geom::Point _origin;
    bool _polar_coords;
};


class RegisteredRandom : public RegisteredWidget<Random> {
public:
    virtual ~RegisteredRandom();
    RegisteredRandom ( const Glib::ustring& label,
                       const Glib::ustring& tip,
                       const Glib::ustring& key,
                       Registry& wr,
                       Inkscape::XML::Node* repr_in = NULL,
                       SPDocument *doc_in = NULL);

    void setValue (double val, long startseed);

protected:
    sigc::connection  _value_changed_connection;
    sigc::connection  _reseeded_connection;
    void on_value_changed();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_REGISTERED_WIDGET__H_

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

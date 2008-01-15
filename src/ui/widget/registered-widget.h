/** \file
 * \brief
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2005-2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_REGISTERED_WIDGET__H_
#define INKSCAPE_UI_WIDGET_REGISTERED_WIDGET__H_

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/togglebutton.h>

#include "xml/node.h"
#include "registry.h"

#include "ui/widget/point.h"
#include "ui/widget/random.h"

class SPUnit;
class SPDocument;

namespace Gtk {
    class HScale;
    class RadioButton;
    class SpinButton;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorPicker;
class Registry;
class Scalar;
class ScalarUnit;
class UnitMenu;

template <class W>
class RegisteredWidget : public W {
public:
    void set_undo_parameters(const unsigned int _event_type, Glib::ustring _event_description)
    {
        event_type = _event_type;
        event_description = _event_description;
        write_undo = true;
    }

    bool is_updating() {if (_wr) return _wr->isUpdating(); else return false;}

protected:
    RegisteredWidget() : W() { construct(); }
    template< typename A >
    explicit RegisteredWidget( A& a ): W( a ) { construct(); }
    template< typename A, typename B >
    RegisteredWidget( A& a, B& b ): W( a, b ) { construct(); }
    template< typename A, typename B, typename C >
    RegisteredWidget( A& a, B& b, C& c ): W( a, b, c ) { construct(); }

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

    void write_to_xml(const char * svgstr);

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
    }
};

class RegisteredWdg {
public:
    void set_undo_parameters(const unsigned int _event_type, Glib::ustring _event_description)
    {
        event_type = _event_type;
        event_description = _event_description;
        write_undo = true;
    }

    bool is_updating() {if (_wr) return _wr->isUpdating(); else return false;}

protected:
    RegisteredWdg()
    {
        _wr = NULL;
        repr = NULL;
        doc = NULL;
        write_undo = false;
    }

    void init_parent(const Glib::ustring& key, Registry& wr, Inkscape::XML::Node* repr_in, SPDocument *doc_in)
    {
        _wr = &wr;
        _key = key;
        repr = repr_in;
        doc = doc_in;
        if (repr && !doc)  // doc cannot be NULL when repr is not NULL
            g_warning("Initialization of registered widget using defined repr but with doc==NULL");
    }

    void write_to_xml(const char * svgstr);

    Registry * _wr;
    Glib::ustring _key;
    Inkscape::XML::Node * repr;
    SPDocument * doc;
    unsigned int event_type;
    Glib::ustring event_description;
    bool write_undo;
};

//#######################################################

class RegisteredCheckButton : public RegisteredWidget<Gtk::CheckButton> {
public:
    virtual ~RegisteredCheckButton();
    RegisteredCheckButton (const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Registry& wr, bool right=true, Inkscape::XML::Node* repr_in=NULL, SPDocument *doc_in=NULL);

    void setActive (bool);

    std::list<Gtk::ToggleButton*> _slavebuttons;

    // a slave button is only sensitive when the master button is active
    // i.e. a slave button is greyed-out when the master button is not checked

    void setSlaveButton(std::list<Gtk::ToggleButton*> btns) {
        _slavebuttons = btns;
    }

    bool setProgrammatically; // true if the value was set by setActive, not changed by the user;
                                // if a callback checks it, it must reset it back to false


protected:
    Gtk::Tooltips     _tt;
    sigc::connection  _toggled_connection;
    void on_toggled();
};

class RegisteredUnitMenu : public RegisteredWdg {
public:
    RegisteredUnitMenu();
    ~RegisteredUnitMenu();
    void init ( const Glib::ustring& label,
                const Glib::ustring& key,
                Registry& wr,
                Inkscape::XML::Node* repr_in,
                SPDocument *doc_in);
    inline void init ( const Glib::ustring& label, 
                       const Glib::ustring& key, 
                       Registry& wr)
        { init(label, key, wr, NULL, NULL); };

    void setUnit (const SPUnit*);
    Gtk::Label   *_label;
    UnitMenu     *_sel;
    sigc::connection _changed_connection;

protected:
    void on_changed();
};

class RegisteredScalarUnit : public RegisteredWdg {
public:
    RegisteredScalarUnit();
    ~RegisteredScalarUnit();
    void init (const Glib::ustring& label,
            const Glib::ustring& tip,
            const Glib::ustring& key,
            const RegisteredUnitMenu &rum,
            Registry& wr,
            Inkscape::XML::Node* repr_in,
            SPDocument *doc_in);
    inline void init ( const Glib::ustring& label,
                       const Glib::ustring& tip,
                       const Glib::ustring& key,
                       const RegisteredUnitMenu &rum,
                       Registry& wr)
        { init(label, tip, key, rum, wr, NULL, NULL); };

    ScalarUnit* getSU();
    void setValue (double);

protected:
    ScalarUnit   *_widget;
    sigc::connection  _value_changed_connection;
    UnitMenu         *_um;
    void on_value_changed();
};

class RegisteredScalar : public RegisteredWdg {
public:
    RegisteredScalar();
    ~RegisteredScalar();
    void init (const Glib::ustring& label,
            const Glib::ustring& tip,
            const Glib::ustring& key,
            Registry& wr,
            Inkscape::XML::Node* repr_in,
            SPDocument *doc_in);
    inline void init ( const Glib::ustring& label,
                       const Glib::ustring& tip,
                       const Glib::ustring& key,
                       Registry& wr)
        { init(label, tip, key, wr, NULL, NULL); };

    Scalar* getS();
    void setValue (double);

protected:
    Scalar   *_widget;
    sigc::connection  _value_changed_connection;
    void on_value_changed();
};

class RegisteredColorPicker : public RegisteredWdg {
public:
    RegisteredColorPicker();
    ~RegisteredColorPicker();
    void init (const Glib::ustring& label, 
            const Glib::ustring& title, 
            const Glib::ustring& tip, 
            const Glib::ustring& ckey, 
            const Glib::ustring& akey,
            Registry& wr,
            Inkscape::XML::Node* repr_in,
            SPDocument *doc_in);
    inline void init ( const Glib::ustring& label, 
                       const Glib::ustring& title, 
                       const Glib::ustring& tip, 
                       const Glib::ustring& ckey, 
                       const Glib::ustring& akey, 
                       Registry& wr)
        { init(label, title, tip, ckey, akey, wr, NULL, NULL); };

    void setRgba32 (guint32);
    void closeWindow();

    Gtk::Label *_label;
    ColorPicker *_cp;

protected:
    Glib::ustring _ckey, _akey;
    void on_changed (guint32);
    sigc::connection _changed_connection;
};

class RegisteredSuffixedInteger : public RegisteredWdg {
public:
    RegisteredSuffixedInteger();
    ~RegisteredSuffixedInteger();
    void init (const Glib::ustring& label1, 
               const Glib::ustring& label2, 
               const Glib::ustring& key,
               Registry& wr,
               Inkscape::XML::Node* repr_in,
               SPDocument *doc_in);
    inline void init ( const Glib::ustring& label1, 
                       const Glib::ustring& label2, 
                       const Glib::ustring& key, 
                       Registry& wr)
        { init(label1, label2, key, wr, NULL, NULL); };

    void setValue (int);
    Gtk::Label *_label;
    Gtk::HBox _hbox;
    bool setProgrammatically; // true if the value was set by setValue, not changed by the user; 
                                // if a callback checks it, it must reset it back to false

protected:
    Gtk::SpinButton *_sb;
    Gtk::Adjustment _adj;
    Gtk::Label      *_suffix;
    sigc::connection _changed_connection;
    void on_value_changed();
};

class RegisteredRadioButtonPair : public RegisteredWdg {
public:
    RegisteredRadioButtonPair();
    ~RegisteredRadioButtonPair();
    void init (const Glib::ustring& label, 
               const Glib::ustring& label1, 
               const Glib::ustring& label2, 
               const Glib::ustring& tip1, 
               const Glib::ustring& tip2, 
               const Glib::ustring& key,
               Registry& wr,
               Inkscape::XML::Node* repr_in,
               SPDocument *doc_in);
    inline void init ( const Glib::ustring& label, 
                       const Glib::ustring& label1, 
                       const Glib::ustring& label2, 
                       const Glib::ustring& tip1, 
                       const Glib::ustring& tip2, 
                       const Glib::ustring& key, 
                       Registry& wr)
        { init(label, label1, label2, tip1, tip2, key, wr, NULL, NULL); };

    void setValue (bool second);
    Gtk::HBox *_hbox;

    bool setProgrammatically; // true if the value was set by setValue, not changed by the user; 
                                    // if a callback checks it, it must reset it back to false
protected:
    Gtk::RadioButton *_rb1, *_rb2;
    Gtk::Tooltips     _tt;
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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

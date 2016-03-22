/*
 * Inkscape Preferences dialog.
 *
 * Authors:
 *   Marco Scholten
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Copyright (C) 2004, 2006, 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include <gtkmm/scale.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>

#include "preferences.h"
#include "ui/widget/preferences-widget.h"
#include "verbs.h"
#include "selcue.h"
#include "io/sys.h"
#include <iostream>
#include "desktop.h"
#include "enums.h"
#include "inkscape.h"

#include "message-stack.h"
#include "style.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "ui/dialog/filedialog.h"
#include "xml/repr.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>

#ifdef WIN32
#include <windows.h>
#endif

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Widget {

DialogPage::DialogPage()
{
    set_border_width(12);

#if WITH_GTKMM_3_0
    set_orientation(Gtk::ORIENTATION_VERTICAL);
    set_column_spacing(12);
    set_row_spacing(6);
#else
    set_col_spacings(12);
    set_row_spacings(6);
#endif
}

/**
 * Add a widget to the bottom row of the dialog page
 *
 * \param[in] indent         Whether the widget should be indented by one column
 * \param[in] label          The label text for the widget
 * \param[in] widget         The widget to add to the page
 * \param[in] suffix         Text for an optional label at the right of the widget
 * \param[in] tip            Tooltip text for the widget
 * \param[in] expand_widget  Whether to expand the widget horizontally
 * \param[in] other_widget   An optional additional widget to display at the right of the first one
 */
void DialogPage::add_line(bool                 indent,
                          Glib::ustring const &label,
                          Gtk::Widget         &widget,
                          Glib::ustring const &suffix,
                          const Glib::ustring &tip,
                          bool                 expand_widget,
                          Gtk::Widget         *other_widget)
{
    if (tip != "")
        widget.set_tooltip_text (tip);
    
    Gtk::Alignment* label_alignment = Gtk::manage(new Gtk::Alignment());
    
    Gtk::HBox* hb = Gtk::manage(new Gtk::HBox());
    hb->set_spacing(12);
    hb->pack_start(widget, expand_widget, expand_widget);
        
    // Pack an additional widget into a box with the widget if desired 
    if (other_widget)
        hb->pack_start(*other_widget, expand_widget, expand_widget);
    
    // Pack the widget into an alignment container so that it can
    // be indented if desired
    Gtk::Alignment* w_alignment = Gtk::manage(new Gtk::Alignment());
    w_alignment->add(*hb);

#if WITH_GTKMM_3_0
    w_alignment->set_valign(Gtk::ALIGN_CENTER);
#else
    guint row = property_n_rows();
#endif
    
    // Add a label in the first column if provided
    if (label != "")
    {
        Gtk::Label* label_widget = Gtk::manage(new Gtk::Label(label, Gtk::ALIGN_START,
                                                              Gtk::ALIGN_CENTER, true));
        label_widget->set_mnemonic_widget(widget);
        
        // Pack the label into an alignment container so that we can indent it
        // if necessary
        label_alignment->add(*label_widget);
 
        if (indent)
            label_alignment->set_padding(0, 0, 12, 0);

#if WITH_GTKMM_3_0
        label_alignment->set_valign(Gtk::ALIGN_CENTER);
        add(*label_alignment);
        attach_next_to(*w_alignment, *label_alignment, Gtk::POS_RIGHT, 1, 1);
#else
        attach(*label_alignment, 0, 1, row, row + 1, Gtk::FILL, Gtk::AttachOptions(), 0, 0);
#endif
    }

    // Now add the widget to the bottom of the dialog
#if WITH_GTKMM_3_0
    if (label == "")
    {
        if (indent)
            w_alignment->set_padding(0, 0, 12, 0);

        add(*w_alignment);
        
        GValue width = G_VALUE_INIT;
        g_value_init(&width, G_TYPE_INT);
        g_value_set_int(&width, 2);
        gtk_container_child_set_property(GTK_CONTAINER(gobj()), GTK_WIDGET(w_alignment->gobj()), "width", &width);
    }
#else
    // The widget should span two columns if there is no label
    int w_col_span = 1;
    if (label == "")
        w_col_span = 2;
    
    attach(*w_alignment, 2 - w_col_span, 2, row, row + 1,
            Gtk::FILL | Gtk::EXPAND,
            Gtk::AttachOptions(),
            0, 0);
#endif

    // Add a label on the right of the widget if desired
    if (suffix != "")
    {
        Gtk::Label* suffix_widget = Gtk::manage(new Gtk::Label(suffix , Gtk::ALIGN_START , Gtk::ALIGN_CENTER, true));
        hb->pack_start(*suffix_widget,false,false);
    }

}

void DialogPage::add_group_header(Glib::ustring name)
{
    if (name != "")
    {
        Gtk::Label* label_widget = Gtk::manage(new Gtk::Label(Glib::ustring(/*"<span size='large'>*/"<b>") + name +
                                               Glib::ustring("</b>"/*</span>"*/) , Gtk::ALIGN_START , Gtk::ALIGN_CENTER, true));
        
        label_widget->set_use_markup(true);
        
#if WITH_GTKMM_3_0
        label_widget->set_valign(Gtk::ALIGN_CENTER);
        add(*label_widget);
//        if (row != 1)
  //          set_row_spacing(row - 1, 18);
#else
        int row = property_n_rows();
        attach(*label_widget , 0, 4, row, row + 1, Gtk::FILL, Gtk::AttachOptions(), 0, 0);
        if (row != 1)
            set_row_spacing(row - 1, 18);
#endif
    }
}

void DialogPage::set_tip(Gtk::Widget& widget, Glib::ustring const &tip)
{
    widget.set_tooltip_text (tip);
}

void PrefCheckButton::init(Glib::ustring const &label, Glib::ustring const &prefs_path,
    bool default_value)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    this->set_label(label);
    this->set_active( prefs->getBool(_prefs_path, default_value) );
}

void PrefCheckButton::on_toggled()
{
    this->changed_signal.emit(this->get_active());
    if (this->get_visible()) //only take action if the user toggled it
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool(_prefs_path, this->get_active());
    }
}

void PrefRadioButton::init(Glib::ustring const &label, Glib::ustring const &prefs_path,
    Glib::ustring const &string_value, bool default_value, PrefRadioButton* group_member)
{
    _prefs_path = prefs_path;
    _value_type = VAL_STRING;
    _string_value = string_value;
    (void)default_value;
    this->set_label(label);
    if (group_member)
    {
        Gtk::RadioButtonGroup rbg = group_member->get_group();
        this->set_group(rbg);
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring val = prefs->getString(_prefs_path);
    if ( !val.empty() )
        this->set_active(val == _string_value);
    else
        this->set_active( false );
}

void PrefRadioButton::init(Glib::ustring const &label, Glib::ustring const &prefs_path,
    int int_value, bool default_value, PrefRadioButton* group_member)
{
    _prefs_path = prefs_path;
    _value_type = VAL_INT;
    _int_value = int_value;
    this->set_label(label);
    if (group_member)
    {
        Gtk::RadioButtonGroup rbg = group_member->get_group();
        this->set_group(rbg);
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (default_value)
        this->set_active( prefs->getInt(_prefs_path, int_value) == _int_value );
    else
        this->set_active( prefs->getInt(_prefs_path, int_value + 1) == _int_value );
}

void PrefRadioButton::on_toggled()
{
    this->changed_signal.emit(this->get_active());
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (this->get_visible() && this->get_active() ) //only take action if toggled by user (to active)
    {
        if ( _value_type == VAL_STRING )
            prefs->setString(_prefs_path, _string_value);
        else if ( _value_type == VAL_INT )
            prefs->setInt(_prefs_path, _int_value);
    }
}

void PrefSpinButton::init(Glib::ustring const &prefs_path,
              double lower, double upper, double step_increment, double /*page_increment*/,
              double default_value, bool is_int, bool is_percent)
{
    _prefs_path = prefs_path;
    _is_int = is_int;
    _is_percent = is_percent;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double value;
    if (is_int) {
        if (is_percent) {
            value = 100 * prefs->getDoubleLimited(prefs_path, default_value, lower/100.0, upper/100.0);
        } else {
            value = (double) prefs->getIntLimited(prefs_path, (int) default_value, (int) lower, (int) upper);
        }
    } else {
        value = prefs->getDoubleLimited(prefs_path, default_value, lower, upper);
    }

    this->set_range (lower, upper);
    this->set_increments (step_increment, 0);
    this->set_value (value);
    this->set_width_chars(6);
    if (is_int)
        this->set_digits(0);
    else if (step_increment < 0.1)
        this->set_digits(4);
    else
        this->set_digits(2);

}

void PrefSpinButton::on_value_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (this->get_visible()) //only take action if user changed value
    {
        if (_is_int) {
            if (_is_percent) {
                prefs->setDouble(_prefs_path, this->get_value()/100.0);
            } else {
                prefs->setInt(_prefs_path, (int) this->get_value());
            }
        } else {
            prefs->setDouble(_prefs_path, this->get_value());
        }
    }
}

void PrefSpinUnit::init(Glib::ustring const &prefs_path,
              double lower, double upper, double step_increment,
              double default_value, UnitType unit_type, Glib::ustring const &default_unit)
{
    _prefs_path = prefs_path;
    _is_percent = (unit_type == UNIT_TYPE_DIMENSIONLESS);

    resetUnitType(unit_type);
    setUnit(default_unit);
    setRange (lower, upper); /// @fixme  this disregards changes of units
    setIncrements (step_increment, 0);
    if (step_increment < 0.1) {
        setDigits(4);
    } else {
        setDigits(2);
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double value = prefs->getDoubleLimited(prefs_path, default_value, lower, upper);
    Glib::ustring unitstr = prefs->getUnit(prefs_path);
    if (unitstr.length() == 0) {
        unitstr = default_unit;
        // write the assumed unit to preferences:
        prefs->setDoubleUnit(_prefs_path, value, unitstr);
    }
    setValue(value, unitstr);

    signal_value_changed().connect_notify(sigc::mem_fun(*this, &PrefSpinUnit::on_my_value_changed));
}

void PrefSpinUnit::on_my_value_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (getWidget()->get_visible()) //only take action if user changed value
    {
        prefs->setDoubleUnit(_prefs_path, getValue(getUnit()->abbr), getUnit()->abbr);
    }
}

const double ZoomCorrRuler::textsize = 7;
const double ZoomCorrRuler::textpadding = 5;

ZoomCorrRuler::ZoomCorrRuler(int width, int height) :
    _unitconv(1.0),
    _border(5)
{
    set_size(width, height);
}

void ZoomCorrRuler::set_size(int x, int y)
{
    _min_width = x;
    _height = y;
    set_size_request(x + _border*2, y + _border*2);
}

// The following two functions are borrowed from 2geom's toy-framework-2; if they are useful in
// other locations, we should perhaps make them (or adapted versions of them) publicly available
static void
draw_text(cairo_t *cr, Geom::Point loc, const char* txt, bool bottom = false,
          double fontsize = ZoomCorrRuler::textsize, std::string fontdesc = "Sans") {
    PangoLayout* layout = pango_cairo_create_layout (cr);
    pango_layout_set_text(layout, txt, -1);

    // set font and size
    std::ostringstream sizestr;
    sizestr << fontsize;
    fontdesc = fontdesc + " " + sizestr.str();
    PangoFontDescription *font_desc = pango_font_description_from_string(fontdesc.c_str());
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free (font_desc);

    PangoRectangle logical_extent;
    pango_layout_get_pixel_extents(layout, NULL, &logical_extent);
    cairo_move_to(cr, loc[Geom::X], loc[Geom::Y] - (bottom ? logical_extent.height : 0));
    pango_cairo_show_layout(cr, layout);
}

static void
draw_number(cairo_t *cr, Geom::Point pos, double num) {
    std::ostringstream number;
    number << num;
    draw_text(cr, pos, number.str().c_str(), true);
}

/*
 * \arg dist The distance between consecutive minor marks
 * \arg major_interval Number of marks after which to draw a major mark
 */
void
ZoomCorrRuler::draw_marks(Cairo::RefPtr<Cairo::Context> cr, double dist, int major_interval) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    const double zoomcorr = prefs->getDouble("/options/zoomcorrection/value", 1.0);
    double mark = 0;
    int i = 0;
    while (mark <= _drawing_width) {
        cr->move_to(mark, _height);
        if ((i % major_interval) == 0) {
            // major mark
            cr->line_to(mark, 0);
            Geom::Point textpos(mark + 3, ZoomCorrRuler::textsize + ZoomCorrRuler::textpadding);
            draw_number(cr->cobj(), textpos, dist * i);
        } else {
            // minor mark
            cr->line_to(mark, ZoomCorrRuler::textsize + 2 * ZoomCorrRuler::textpadding);
        }
        mark += dist * zoomcorr / _unitconv;
        ++i;
    }
}

#if !WITH_GTKMM_3_0
bool
ZoomCorrRuler::on_expose_event(GdkEventExpose *event) {
    bool result = false;

    if(get_is_drawable())
    {
        Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();
        cr->rectangle(event->area.x, event->area.y,
                      event->area.width, event->area.height);
        cr->clip();
        result = on_draw(cr);
    }

    return result;
}
#endif

bool
ZoomCorrRuler::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    Glib::RefPtr<Gdk::Window> window = get_window();

    int w = window->get_width();
    _drawing_width = w - _border * 2;

    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->set_fill_rule(Cairo::FILL_RULE_WINDING);
    cr->rectangle(0, 0, w, _height + _border*2);
    cr->fill();

    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->set_line_width(0.5);

    cr->translate(_border, _border); // so that we have a small white border around the ruler
    cr->move_to (0, _height);
    cr->line_to (_drawing_width, _height);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring abbr = prefs->getString("/options/zoomcorrection/unit");
    if (abbr == "cm") {
        draw_marks(cr, 0.1, 10);
    } else if (abbr == "in") {
        draw_marks(cr, 0.25, 4);
    } else if (abbr == "mm") {
        draw_marks(cr, 10, 10);
    } else if (abbr == "pc") {
        draw_marks(cr, 1, 10);
    } else if (abbr == "pt") {
        draw_marks(cr, 10, 10);
    } else if (abbr == "px") {
        draw_marks(cr, 10, 10);
    } else {
        draw_marks(cr, 1, 1);
    }
    cr->stroke();

    return true;
}


void
ZoomCorrRulerSlider::on_slider_value_changed()
{
    if (this->get_visible() || freeze) //only take action if user changed value
    {
        freeze = true;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/options/zoomcorrection/value", _slider->get_value() / 100.0);
        _sb.set_value(_slider->get_value());
        _ruler.queue_draw();
        freeze = false;
    }
}

void
ZoomCorrRulerSlider::on_spinbutton_value_changed()
{
    if (this->get_visible() || freeze) //only take action if user changed value
    {
        freeze = true;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/options/zoomcorrection/value", _sb.get_value() / 100.0);
        _slider->set_value(_sb.get_value());
        _ruler.queue_draw();
        freeze = false;
    }
}

void
ZoomCorrRulerSlider::on_unit_changed() {
    if (GPOINTER_TO_INT(_unit.get_data("sensitive")) == 0) {
        // when the unit menu is initialized, the unit is set to the default but
        // it needs to be reset later so we don't perform the change in this case
        return;
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString("/options/zoomcorrection/unit", _unit.getUnitAbbr());
    double conv = _unit.getConversion(_unit.getUnitAbbr(), "px");
    _ruler.set_unit_conversion(conv);
    if (_ruler.get_visible()) {
        _ruler.queue_draw();
    }
}

bool ZoomCorrRulerSlider::on_mnemonic_activate ( bool group_cycling )
{
    return _sb.mnemonic_activate ( group_cycling );
}


void
ZoomCorrRulerSlider::init(int ruler_width, int ruler_height, double lower, double upper,
                      double step_increment, double page_increment, double default_value)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double value = prefs->getDoubleLimited("/options/zoomcorrection/value", default_value, lower, upper) * 100.0;

    freeze = false;

    _ruler.set_size(ruler_width, ruler_height);

#if WITH_GTKMM_3_0
    _slider = Gtk::manage(new Gtk::Scale(Gtk::ORIENTATION_HORIZONTAL));
#else
    _slider = Gtk::manage(new Gtk::HScale());
#endif

    _slider->set_size_request(_ruler.width(), -1);
    _slider->set_range (lower, upper);
    _slider->set_increments (step_increment, page_increment);
    _slider->set_value (value);
    _slider->set_digits(2);

    _slider->signal_value_changed().connect(sigc::mem_fun(*this, &ZoomCorrRulerSlider::on_slider_value_changed));
    _sb.signal_value_changed().connect(sigc::mem_fun(*this, &ZoomCorrRulerSlider::on_spinbutton_value_changed));
    _unit.signal_changed().connect(sigc::mem_fun(*this, &ZoomCorrRulerSlider::on_unit_changed));

    _sb.set_range (lower, upper);
    _sb.set_increments (step_increment, 0);
    _sb.set_value (value);
    _sb.set_digits(2);

    _unit.set_data("sensitive", GINT_TO_POINTER(0));
    _unit.setUnitType(UNIT_TYPE_LINEAR);
    _unit.set_data("sensitive", GINT_TO_POINTER(1));
    _unit.setUnit(prefs->getString("/options/zoomcorrection/unit"));

    Gtk::Alignment *alignment1 = Gtk::manage(new Gtk::Alignment(0.5,1,0,0));
    Gtk::Alignment *alignment2 = Gtk::manage(new Gtk::Alignment(0.5,1,0,0));
    alignment1->add(_sb);
    alignment2->add(_unit);

#if WITH_GTKMM_3_0
    Gtk::Grid *table = Gtk::manage(new Gtk::Grid());
    table->attach(*_slider,    0, 0, 1, 1);
    alignment1->set_halign(Gtk::ALIGN_CENTER);
    table->attach(*alignment1, 1, 0, 1, 1);
    table->attach(_ruler,      0, 1, 1, 1);
    alignment2->set_halign(Gtk::ALIGN_CENTER);
    table->attach(*alignment2, 1, 1, 1, 1);
#else
    Gtk::Table *table = Gtk::manage(new Gtk::Table());
    table->attach(*_slider,    0, 1, 0, 1);
    table->attach(*alignment1, 1, 2, 0, 1, static_cast<Gtk::AttachOptions>(0));
    table->attach(_ruler,      0, 1, 1, 2);
    table->attach(*alignment2, 1, 2, 1, 2, static_cast<Gtk::AttachOptions>(0));
#endif

    pack_start(*table, Gtk::PACK_SHRINK);
}

void
PrefSlider::on_slider_value_changed()
{
    if (this->get_visible() || freeze) //only take action if user changed value
    {
        freeze = true;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(_prefs_path, _slider->get_value());
        _sb.set_value(_slider->get_value());
        freeze = false;
    }
}

void
PrefSlider::on_spinbutton_value_changed()
{
    if (this->get_visible() || freeze) //only take action if user changed value
    {
        freeze = true;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(_prefs_path, _sb.get_value());
        _slider->set_value(_sb.get_value());
        freeze = false;
    }
}

bool PrefSlider::on_mnemonic_activate ( bool group_cycling )
{
    return _sb.mnemonic_activate ( group_cycling );
}

void
PrefSlider::init(Glib::ustring const &prefs_path,
                 double lower, double upper, double step_increment, double page_increment, double default_value, int digits)
{
    _prefs_path = prefs_path;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double value = prefs->getDoubleLimited(prefs_path, default_value, lower, upper);

    freeze = false;

#if WITH_GTKMM_3_0
    _slider = Gtk::manage(new Gtk::Scale(Gtk::ORIENTATION_HORIZONTAL));
#else
    _slider = Gtk::manage(new Gtk::HScale());
#endif

    _slider->set_range (lower, upper);
    _slider->set_increments (step_increment, page_increment);
    _slider->set_value (value);
    _slider->set_digits(digits);
    _slider->signal_value_changed().connect(sigc::mem_fun(*this, &PrefSlider::on_slider_value_changed));

    _sb.signal_value_changed().connect(sigc::mem_fun(*this, &PrefSlider::on_spinbutton_value_changed));
    _sb.set_range (lower, upper);
    _sb.set_increments (step_increment, 0);
    _sb.set_value (value);
    _sb.set_digits(digits);

    Gtk::Alignment *alignment1 = Gtk::manage(new Gtk::Alignment(0.5,1,0,0));
    alignment1->add(_sb);

#if WITH_GTKMM_3_0
    Gtk::Grid *table = Gtk::manage(new Gtk::Grid());
    _slider->set_hexpand();
    table->attach(*_slider,    0, 0, 1, 1);
    alignment1->set_halign(Gtk::ALIGN_CENTER);
    table->attach(*alignment1, 1, 0, 1, 1);
#else
    Gtk::Table *table = Gtk::manage(new Gtk::Table());
    table->attach(*_slider,    0, 1, 0, 1);
    table->attach(*alignment1, 1, 2, 0, 1, static_cast<Gtk::AttachOptions>(0));
#endif

    this->pack_start(*table, Gtk::PACK_EXPAND_WIDGET);
}

void PrefCombo::init(Glib::ustring const &prefs_path,
                     Glib::ustring labels[], int values[], int num_items, int default_value)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int row = 0;
    int value = prefs->getInt(_prefs_path, default_value);

    for (int i = 0 ; i < num_items; ++i)
    {
        this->append(labels[i]);
        _values.push_back(values[i]);
        if (value == values[i])
            row = i;
    }
    this->set_active(row);
}

void PrefCombo::init(Glib::ustring const &prefs_path,
                     Glib::ustring labels[], Glib::ustring values[], int num_items, Glib::ustring default_value)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int row = 0;
    Glib::ustring value = prefs->getString(_prefs_path);
    if(value.empty())
    {
        value = default_value;
    }

    for (int i = 0 ; i < num_items; ++i)
    {
        this->append(labels[i]);
        _ustr_values.push_back(values[i]);
        if (value == values[i])
            row = i;
    }
    this->set_active(row);
}

void PrefCombo::on_changed()
{
    if (this->get_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if(!_values.empty())
        {
            prefs->setInt(_prefs_path, _values[this->get_active_row_number()]);
        }
        else
        {
            prefs->setString(_prefs_path, _ustr_values[this->get_active_row_number()]);
        }
    }
}

void PrefEntryButtonHBox::init(Glib::ustring const &prefs_path,
            bool visibility, Glib::ustring const &default_string)
{
    _prefs_path = prefs_path;
    _default_string = default_string;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    relatedEntry = new Gtk::Entry();
    relatedButton = new Gtk::Button(_("Reset"));
    relatedEntry->set_invisible_char('*');
    relatedEntry->set_visibility(visibility);
    relatedEntry->set_text(prefs->getString(_prefs_path));
    this->pack_start(*relatedEntry);
    this->pack_start(*relatedButton);
    relatedButton->signal_clicked().connect(
            sigc::mem_fun(*this, &PrefEntryButtonHBox::onRelatedButtonClickedCallback));
    relatedEntry->signal_changed().connect(
            sigc::mem_fun(*this, &PrefEntryButtonHBox::onRelatedEntryChangedCallback));
}

void PrefEntryButtonHBox::onRelatedEntryChangedCallback()
{
    if (this->get_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, relatedEntry->get_text());
    }
}

void PrefEntryButtonHBox::onRelatedButtonClickedCallback()
{
    if (this->get_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, _default_string);
        relatedEntry->set_text(_default_string);
    }
}

bool PrefEntryButtonHBox::on_mnemonic_activate ( bool group_cycling )
{
    return relatedEntry->mnemonic_activate ( group_cycling );
}

void PrefEntryFileButtonHBox::init(Glib::ustring const &prefs_path,
            bool visibility)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    
    relatedEntry = new Gtk::Entry();
    relatedEntry->set_invisible_char('*');
    relatedEntry->set_visibility(visibility);
    relatedEntry->set_text(prefs->getString(_prefs_path));
    
    relatedButton = new Gtk::Button();
    Gtk::HBox* pixlabel = new Gtk::HBox(false, 3);
    Gtk::Image *im = new Gtk::Image(Gtk::StockID(Gtk::Stock::INDEX),
            Gtk::ICON_SIZE_BUTTON);
    pixlabel->pack_start(*im);
    Gtk::Label *l = new Gtk::Label();
    l->set_markup_with_mnemonic(_("_Browse..."));
    pixlabel->pack_start(*l);
    relatedButton->add(*pixlabel); 

    this->pack_end(*relatedButton, false, false, 4);
    this->pack_start(*relatedEntry, true, true, 0);

    relatedButton->signal_clicked().connect(
            sigc::mem_fun(*this, &PrefEntryFileButtonHBox::onRelatedButtonClickedCallback));
    relatedEntry->signal_changed().connect(
            sigc::mem_fun(*this, &PrefEntryFileButtonHBox::onRelatedEntryChangedCallback));
}

void PrefEntryFileButtonHBox::onRelatedEntryChangedCallback()
{
    if (this->get_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, relatedEntry->get_text());
    }
}

static Inkscape::UI::Dialog::FileOpenDialog * selectPrefsFileInstance = NULL;

void PrefEntryFileButtonHBox::onRelatedButtonClickedCallback()
{
    if (this->get_visible()) //only take action if user changed value
    {
        //# Get the current directory for finding files
        static Glib::ustring open_path;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();


        Glib::ustring attr = prefs->getString(_prefs_path);
        if (!attr.empty()) open_path = attr;
        
        //# Test if the open_path directory exists
        if (!Inkscape::IO::file_test(open_path.c_str(),
                  (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
            open_path = "";

#ifdef WIN32
        //# If no open path, default to our win32 documents folder
        if (open_path.empty())
        {
            // The path to the My Documents folder is read from the
            // value "HKEY_CURRENT_USER\Software\Windows\CurrentVersion\Explorer\Shell Folders\Personal"
            HKEY key = NULL;
            if(RegOpenKeyExA(HKEY_CURRENT_USER,
                "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
            {
                WCHAR utf16path[_MAX_PATH];
                DWORD value_type;
                DWORD data_size = sizeof(utf16path);
                if(RegQueryValueExW(key, L"Personal", NULL, &value_type,
                    (BYTE*)utf16path, &data_size) == ERROR_SUCCESS)
                {
                    g_assert(value_type == REG_SZ);
                    gchar *utf8path = g_utf16_to_utf8(
                        (const gunichar2*)utf16path, -1, NULL, NULL, NULL);
                    if(utf8path)
                    {
                        open_path = Glib::ustring(utf8path);
                        g_free(utf8path);
                    }
                }
            }
        }
#endif

        //# If no open path, default to our home directory
        if (open_path.empty())
        {
            open_path = g_get_home_dir();
            open_path.append(G_DIR_SEPARATOR_S);
        }

        //# Create a dialog
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (!selectPrefsFileInstance) {
        selectPrefsFileInstance =
              Inkscape::UI::Dialog::FileOpenDialog::create(
                 *desktop->getToplevel(),
                 open_path,
                 Inkscape::UI::Dialog::EXE_TYPES,
                 _("Select a bitmap editor"));
        }
        
        //# Show the dialog
        bool const success = selectPrefsFileInstance->show();
        
        if (!success) {
            return;
        }
        
        //# User selected something.  Get name and type
        Glib::ustring fileName = selectPrefsFileInstance->getFilename();

        if (!fileName.empty())
        {
            Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

            if ( newFileName.size() > 0)
                open_path = newFileName;
            else
                g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );

            prefs->setString(_prefs_path, open_path);
        }
        
        relatedEntry->set_text(fileName);
    }
}

bool PrefEntryFileButtonHBox::on_mnemonic_activate ( bool group_cycling )
{
    return relatedEntry->mnemonic_activate ( group_cycling );
}

void PrefFileButton::init(Glib::ustring const &prefs_path)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    select_filename(Glib::filename_from_utf8(prefs->getString(_prefs_path)));

    signal_selection_changed().connect(sigc::mem_fun(*this, &PrefFileButton::onFileChanged));
}

void PrefFileButton::onFileChanged()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString(_prefs_path, Glib::filename_to_utf8(get_filename()));
}

void PrefEntry::init(Glib::ustring const &prefs_path, bool visibility)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    this->set_invisible_char('*');
    this->set_visibility(visibility);
    this->set_text(prefs->getString(_prefs_path));
}

void PrefEntry::on_changed()
{
    if (this->get_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, this->get_text());
    }
}

void PrefColorPicker::init(Glib::ustring const &label, Glib::ustring const &prefs_path,
                           guint32 default_rgba)
{
    _prefs_path = prefs_path;
    _title = label;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    this->setRgba32( prefs->getInt(_prefs_path, (int)default_rgba) );
}

void PrefColorPicker::on_changed (guint32 rgba)
{
    if (this->get_visible()) //only take action if the user toggled it
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt(_prefs_path, (int) rgba);
    }
}

void PrefUnit::init(Glib::ustring const &prefs_path)
{
    _prefs_path = prefs_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    setUnitType(UNIT_TYPE_LINEAR);
    setUnit(prefs->getString(_prefs_path));
}

void PrefUnit::on_changed()
{
    if (this->get_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, getUnitAbbr());
    }
}

} // namespace Widget
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

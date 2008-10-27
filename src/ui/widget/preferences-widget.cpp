/**
 * \brief Inkscape Preferences dialog
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

#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include <gtkmm/box.h>

#include "preferences.h"
#include "ui/widget/preferences-widget.h"
#include "verbs.h"
#include "selcue.h"
#include <iostream>
#include "enums.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "message-stack.h"
#include "style.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "xml/repr.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Widget {

DialogPage::DialogPage()
{
    this->set_border_width(12);
    this->set_col_spacings(12);
    this->set_row_spacings(6);
}

void DialogPage::add_line(bool indent, Glib::ustring const &label, Gtk::Widget &widget, Glib::ustring const &suffix, const Glib::ustring &tip, bool expand_widget)
{
    int start_col;
    int row = this->property_n_rows();
    Gtk::Widget* w;
    if (expand_widget)
    {
        w = &widget;
    }
    else
    {
        Gtk::HBox* hb = Gtk::manage(new Gtk::HBox());
        hb->set_spacing(12);
        hb->pack_start(widget,false,false);
        w = (Gtk::Widget*) hb;
    }
    if (label != "")
    {
        Gtk::Label* label_widget;
        label_widget = Gtk::manage(new Gtk::Label(label , Gtk::ALIGN_LEFT , Gtk::ALIGN_CENTER, true));
        label_widget->set_mnemonic_widget(widget);
        if (indent)
        {
            Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
            alignment->set_padding(0, 0, 12, 0);
            alignment->add(*label_widget);
            this->attach(*alignment , 0, 1, row, row + 1, Gtk::FILL, Gtk::AttachOptions(), 0, 0);
        }
        else
            this->attach(*label_widget , 0, 1, row, row + 1, Gtk::FILL, Gtk::AttachOptions(), 0, 0);
        start_col = 1;
    }
    else
        start_col = 0;

    if (start_col == 0 && indent) //indent this widget
    {
        Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
        alignment->set_padding(0, 0, 12, 0);
        alignment->add(*w);
        this->attach(*alignment, start_col, 2, row, row + 1, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(),  0, 0);
    }
    else
    {
        this->attach(*w, start_col, 2, row, row + 1, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(),  0, 0);
    }

    if (suffix != "")
    {
        Gtk::Label* suffix_widget = Gtk::manage(new Gtk::Label(suffix , Gtk::ALIGN_LEFT , Gtk::ALIGN_CENTER, true));
        if (expand_widget)
            this->attach(*suffix_widget, 2, 3, row, row + 1, Gtk::FILL,  Gtk::AttachOptions(), 0, 0);
        else
            ((Gtk::HBox*)w)->pack_start(*suffix_widget,false,false);
    }

    if (tip != "")
    {
        _tooltips.set_tip (widget, tip);
    }

}

void DialogPage::add_group_header(Glib::ustring name)
{
    int row = this->property_n_rows();
    if (name != "")
    {
        Gtk::Label* label_widget = Gtk::manage(new Gtk::Label(Glib::ustring(/*"<span size='large'>*/"<b>") + name +
                                               Glib::ustring("</b>"/*</span>"*/) , Gtk::ALIGN_LEFT , Gtk::ALIGN_CENTER, true));
        label_widget->set_use_markup(true);
        this->attach(*label_widget , 0, 4, row, row + 1, Gtk::FILL, Gtk::AttachOptions(), 0, 0);
        if (row != 1)
            this->set_row_spacing(row - 1, 18);
    }
}

void DialogPage::set_tip(Gtk::Widget& widget, Glib::ustring const &tip)
{
    _tooltips.set_tip (widget, tip);
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
    if (this->is_visible()) //only take action if the user toggled it
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
    
    if (this->is_visible() && this->get_active() ) //only take action if toggled by user (to active)
    {
        if ( _value_type == VAL_STRING )
            prefs->setString(_prefs_path, _string_value);
        else if ( _value_type == VAL_INT )
            prefs->setInt(_prefs_path, _int_value);
    }
}

void PrefSpinButton::init(Glib::ustring const &prefs_path,
              double lower, double upper, double step_increment, double page_increment,
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
    this->set_increments (step_increment, page_increment);
    this->set_numeric();
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
    if (this->is_visible()) //only take action if user changed value
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

void
ZoomCorrRuler::redraw() {
    Glib::RefPtr<Gdk::Window> window = get_window();
    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

    int w, h;
    window->get_size(w, h);
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
    } else if (abbr == "ft") {
        draw_marks(cr, 1/12.0, 12);
    } else if (abbr == "in") {
        draw_marks(cr, 0.25, 4);
    } else if (abbr == "m") {
        draw_marks(cr, 1/10.0, 10);
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
}

bool
ZoomCorrRuler::on_expose_event(GdkEventExpose */*event*/) {
    this->redraw();
    return true;
}

void
ZoomCorrRulerSlider::on_slider_value_changed()
{
    if (this->is_visible() || freeze) //only take action if user changed value
    {
        freeze = true;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/options/zoomcorrection/value", _slider.get_value() / 100.0);
        _sb.set_value(_slider.get_value());
        _ruler.redraw();
        freeze = false;
    }
}

void
ZoomCorrRulerSlider::on_spinbutton_value_changed()
{
    if (this->is_visible() || freeze) //only take action if user changed value
    {
        freeze = true;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/options/zoomcorrection/value", _sb.get_value() / 100.0);
        _slider.set_value(_sb.get_value());
        _ruler.redraw();
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
    if (_ruler.is_visible()) {
        _ruler.redraw();
    }
}

void
ZoomCorrRulerSlider::init(int ruler_width, int ruler_height, double lower, double upper,
                      double step_increment, double page_increment, double default_value)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double value = prefs->getDoubleLimited("/options/zoomcorrection/value", default_value, lower, upper) * 100.0;

    freeze = false;

    _ruler.set_size(ruler_width, ruler_height);

    _slider.set_size_request(_ruler.width(), -1);
    _slider.set_range (lower, upper);
    _slider.set_increments (step_increment, page_increment);
    _slider.set_value (value);
    _slider.set_digits(2);

    _slider.signal_value_changed().connect(sigc::mem_fun(*this, &ZoomCorrRulerSlider::on_slider_value_changed));
    _sb.signal_value_changed().connect(sigc::mem_fun(*this, &ZoomCorrRulerSlider::on_spinbutton_value_changed));
    _unit.signal_changed().connect(sigc::mem_fun(*this, &ZoomCorrRulerSlider::on_unit_changed));

    _sb.set_range (lower, upper);
    _sb.set_increments (step_increment, page_increment);
    _sb.set_value (value);
    _sb.set_digits(2);

    _unit.set_data("sensitive", GINT_TO_POINTER(0));
    _unit.setUnitType(UNIT_TYPE_LINEAR);
    _unit.set_data("sensitive", GINT_TO_POINTER(1));
    _unit.setUnit(prefs->getString("/options/zoomcorrection/unit"));

    Gtk::Table *table = Gtk::manage(new Gtk::Table());
    Gtk::Alignment *alignment1 = Gtk::manage(new Gtk::Alignment(0.5,1,0,0));
    Gtk::Alignment *alignment2 = Gtk::manage(new Gtk::Alignment(0.5,1,0,0));
    alignment1->add(_sb);
    alignment2->add(_unit);

    table->attach(_slider,     0, 1, 0, 1);
    table->attach(*alignment1, 1, 2, 0, 1, static_cast<Gtk::AttachOptions>(0));
    table->attach(_ruler,      0, 1, 1, 2);
    table->attach(*alignment2, 1, 2, 1, 2, static_cast<Gtk::AttachOptions>(0));

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
        this->append_text(labels[i]);
        _values.push_back(values[i]);
        if (value == values[i])
            row = i;
    }
    this->set_active(row);
}

void PrefCombo::on_changed()
{
    if (this->is_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt(_prefs_path, _values[this->get_active_row_number()]);
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
    if (this->is_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, relatedEntry->get_text());
    }
}

void PrefEntryButtonHBox::onRelatedButtonClickedCallback()
{
    if (this->is_visible()) //only take action if user changed value
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(_prefs_path, _default_string);
        relatedEntry->set_text(_default_string);
    }
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
    if (this->is_visible()) //only take action if user changed value
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
    if (this->is_visible()) //only take action if the user toggled it
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
    if (this->is_visible()) //only take action if user changed value
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

/**
 * @file
 * A notebook with RGB, CMYK, CMS, HSL, and Wheel pages - implementation
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Tomasz Boczkowski <penginsbacon@gmail.com> (c++-sification)
 *
 * Copyright (C) 2001-2014 Authors
 *
 * This code is in public domain
 */

#undef SPCS_PREVIEW
#define noDUMP_CHANGE_INFO

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "widgets/icon.h"
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstddef>
#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>

#include "preferences.h"
#include "widgets/spw-utilities.h"
#include "svg/svg-icc-color.h"
#include "inkscape.h"
#include "document.h"
#include "profile-manager.h"
#include "color-profile.h"
#include "cms-system.h"
#include "ui/dialog-events.h"
#include "ui/tools-switch.h"
#include "ui/tools/tool-base.h"
#include "ui/widget/color-entry.h"
#include "ui/widget/color-icc-selector.h"
#include "ui/widget/color-notebook.h"
#include "ui/widget/color-scales.h"
#include "ui/widget/color-wheel-selector.h"

using Inkscape::CMSSystem;

#define XPAD 4
#define YPAD 1

namespace Inkscape {
namespace UI {
namespace Widget {


ColorNotebook::ColorNotebook(SelectedColor &color)
#if GTK_CHECK_VERSION(3, 0, 0)
    : Gtk::Grid()
#else
    : Gtk::Table(2, 3, false)
#endif
    , _selected_color(color)

{
    Page *page;

    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_RGB), true);
    _available_pages.push_back(page);
    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_HSV), true);
    _available_pages.push_back(page);
    page = new Page(new ColorScalesFactory(SP_COLOR_SCALES_MODE_CMYK), true);
    _available_pages.push_back(page);
    page = new Page(new ColorWheelSelectorFactory, true);
    _available_pages.push_back(page);
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    page = new Page(new ColorICCSelectorFactory, true);
    _available_pages.push_back(page);
#endif

    _initUI();

    _selected_color.signal_changed.connect(sigc::mem_fun(this, &ColorNotebook::_onSelectedColorChanged));
    _selected_color.signal_dragged.connect(sigc::mem_fun(this, &ColorNotebook::_onSelectedColorChanged));
}

ColorNotebook::~ColorNotebook()
{
    if (_buttons) {
        delete[] _buttons;
        _buttons = 0;
    }
}

ColorNotebook::Page::Page(Inkscape::UI::ColorSelectorFactory *selector_factory, bool enabled_full)
    : selector_factory(selector_factory)
    , enabled_full(enabled_full)
{
}


void ColorNotebook::_initUI()
{
    guint row = 0;

    Gtk::Notebook *notebook = Gtk::manage(new Gtk::Notebook);
    notebook->show();
    notebook->set_show_border(false);
    notebook->set_show_tabs(false);
    _book = GTK_WIDGET(notebook->gobj());

#if GTK_CHECK_VERSION(3, 0, 0)
    _buttonbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_set_homogeneous(GTK_BOX(_buttonbox), TRUE);
#else
    _buttonbox = gtk_hbox_new(TRUE, 2);
#endif

    gtk_widget_show(_buttonbox);
    _buttons = new GtkWidget *[_available_pages.size()];

    for (int i = 0; static_cast<size_t>(i) < _available_pages.size(); i++) {
        _addPage(_available_pages[i]);
    }

    sp_set_font_size_smaller(_buttonbox);

#if GTK_CHECK_VERSION(3, 0, 0)
  #if GTK_CHECK_VERSION(3, 12, 0)
    gtk_widget_set_margin_start(_buttonbox, XPAD);
    gtk_widget_set_margin_end(_buttonbox, XPAD);
  #else
    gtk_widget_set_margin_left(_buttonbox, XPAD);
    gtk_widget_set_margin_right(_buttonbox, XPAD);
  #endif
    gtk_widget_set_margin_top(_buttonbox, YPAD);
    gtk_widget_set_margin_bottom(_buttonbox, YPAD);
    gtk_widget_set_hexpand(_buttonbox, TRUE);
    gtk_widget_set_valign(_buttonbox, GTK_ALIGN_CENTER);
    attach(*Glib::wrap(_buttonbox), 0, row, 2, 1);
#else
    attach(*Glib::wrap(_buttonbox), 0, 2, row, row + 1, Gtk::EXPAND | Gtk::FILL, static_cast<Gtk::AttachOptions>(0),
           XPAD, YPAD);
#endif

    row++;

#if GTK_CHECK_VERSION(3, 0, 0)
#if GTK_CHECK_VERSION(3, 12, 0)
    gtk_widget_set_margin_start(_book, XPAD * 2);
    gtk_widget_set_margin_end(_book, XPAD * 2);
#else
    gtk_widget_set_margin_left(_book, XPAD * 2);
    gtk_widget_set_margin_right(_book, XPAD * 2);
#endif
    gtk_widget_set_margin_top(_book, YPAD);
    gtk_widget_set_margin_bottom(_book, YPAD);
    gtk_widget_set_hexpand(_book, TRUE);
    gtk_widget_set_vexpand(_book, TRUE);
    attach(*notebook, 0, row, 2, 1);
#else
    attach(*notebook, 0, 2, row, row + 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, XPAD * 2, YPAD);
#endif

    // restore the last active page
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _setCurrentPage(prefs->getInt("/colorselector/page", 0));
    row++;

#if GTK_CHECK_VERSION(3, 0, 0)
    GtkWidget *rgbabox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
    GtkWidget *rgbabox = gtk_hbox_new(FALSE, 0);
#endif

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    /* Create color management icons */
    _box_colormanaged = gtk_event_box_new();
    GtkWidget *colormanaged = gtk_image_new_from_icon_name("color-management-icon", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(_box_colormanaged), colormanaged);
    gtk_widget_set_tooltip_text(_box_colormanaged, _("Color Managed"));
    gtk_widget_set_sensitive(_box_colormanaged, false);
    gtk_box_pack_start(GTK_BOX(rgbabox), _box_colormanaged, FALSE, FALSE, 2);

    _box_outofgamut = gtk_event_box_new();
    GtkWidget *outofgamut = gtk_image_new_from_icon_name("out-of-gamut-icon", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(_box_outofgamut), outofgamut);
    gtk_widget_set_tooltip_text(_box_outofgamut, _("Out of gamut!"));
    gtk_widget_set_sensitive(_box_outofgamut, false);
    gtk_box_pack_start(GTK_BOX(rgbabox), _box_outofgamut, FALSE, FALSE, 2);

    _box_toomuchink = gtk_event_box_new();
    GtkWidget *toomuchink = gtk_image_new_from_icon_name("too-much-ink-icon", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(_box_toomuchink), toomuchink);
    gtk_widget_set_tooltip_text(_box_toomuchink, _("Too much ink!"));
    gtk_widget_set_sensitive(_box_toomuchink, false);
    gtk_box_pack_start(GTK_BOX(rgbabox), _box_toomuchink, FALSE, FALSE, 2);
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)


    /* Color picker */
    GtkWidget *picker = gtk_image_new_from_icon_name("color-picker", GTK_ICON_SIZE_SMALL_TOOLBAR);
    _btn_picker = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(_btn_picker), GTK_RELIEF_NONE);
    gtk_container_add(GTK_CONTAINER(_btn_picker), picker);
    gtk_widget_set_tooltip_text(_btn_picker, _("Pick colors from image"));
    gtk_box_pack_start(GTK_BOX(rgbabox), _btn_picker, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(_btn_picker), "clicked", G_CALLBACK(ColorNotebook::_onPickerClicked), this);

    /* Create RGBA entry and color preview */
    _rgbal = gtk_label_new_with_mnemonic(_("RGBA_:"));
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(_rgbal, GTK_ALIGN_END);
#else
    gtk_misc_set_alignment(GTK_MISC(_rgbal), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(rgbabox), _rgbal, TRUE, TRUE, 2);

    ColorEntry *rgba_entry = Gtk::manage(new ColorEntry(_selected_color));
    sp_dialog_defocus_on_enter(GTK_WIDGET(rgba_entry->gobj()));
    gtk_box_pack_start(GTK_BOX(rgbabox), GTK_WIDGET(rgba_entry->gobj()), FALSE, FALSE, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(_rgbal), GTK_WIDGET(rgba_entry->gobj()));

    sp_set_font_size_smaller(rgbabox);
    gtk_widget_show_all(rgbabox);

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    // the "too much ink" icon is initially hidden
    gtk_widget_hide(GTK_WIDGET(_box_toomuchink));
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if GTK_CHECK_VERSION(3, 0, 0)
  #if GTK_CHECK_VERSION(3, 12, 0)
    gtk_widget_set_margin_start(rgbabox, XPAD);
    gtk_widget_set_margin_end(rgbabox, XPAD);
  #else
    gtk_widget_set_margin_left(rgbabox, XPAD);
    gtk_widget_set_margin_right(rgbabox, XPAD);
  #endif
    gtk_widget_set_margin_top(rgbabox, YPAD);
    gtk_widget_set_margin_bottom(rgbabox, YPAD);
    attach(*Glib::wrap(rgbabox), 0, row, 2, 1);
#else
    attach(*Glib::wrap(rgbabox), 0, 2, row, row + 1, Gtk::FILL, Gtk::SHRINK, XPAD, YPAD);
#endif

#ifdef SPCS_PREVIEW
    _p = sp_color_preview_new(0xffffffff);
    gtk_widget_show(_p);
    attach(*Glib::wrap(_p), 2, 3, row, row + 1, Gtk::FILL, Gtk::FILL, XPAD, YPAD);
#endif

    g_signal_connect(G_OBJECT(_book), "switch-page", G_CALLBACK(ColorNotebook::_onPageSwitched), this);
}

void ColorNotebook::_onPickerClicked(GtkWidget * /*widget*/, ColorNotebook * /*colorbook*/)
{
    // Set the dropper into a "one click" mode, so it reverts to the previous tool after a click
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/dropper/onetimepick", true);
    Inkscape::UI::Tools::sp_toggle_dropper(SP_ACTIVE_DESKTOP);
}

void ColorNotebook::_onButtonClicked(GtkWidget *widget, ColorNotebook *nb)
{
    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        return;
    }

    for (gint i = 0; i < gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb->_book)); i++) {
        if (nb->_buttons[i] == widget) {
            gtk_notebook_set_current_page(GTK_NOTEBOOK(nb->_book), i);
        }
    }
}

void ColorNotebook::_onSelectedColorChanged() { _updateICCButtons(); }

void ColorNotebook::_onPageSwitched(GtkNotebook *notebook, GtkWidget *page, guint page_num, ColorNotebook *colorbook)
{
    if (colorbook->get_visible()) {
        // remember the page we switched to
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt("/colorselector/page", page_num);
    }
}


// TODO pass in param so as to avoid the need for SP_ACTIVE_DOCUMENT
void ColorNotebook::_updateICCButtons()
{
    SPColor color = _selected_color.color();
    gfloat alpha = _selected_color.alpha();

    g_return_if_fail((0.0 <= alpha) && (alpha <= 1.0));

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    /* update color management icon*/
    gtk_widget_set_sensitive(_box_colormanaged, color.icc != NULL);

    /* update out-of-gamut icon */
    gtk_widget_set_sensitive(_box_outofgamut, false);
    if (color.icc) {
        Inkscape::ColorProfile *target_profile =
            SP_ACTIVE_DOCUMENT->profileManager->find(color.icc->colorProfile.c_str());
        if (target_profile)
            gtk_widget_set_sensitive(_box_outofgamut, target_profile->GamutCheck(color));
    }

    /* update too-much-ink icon */
    gtk_widget_set_sensitive(_box_toomuchink, false);
    if (color.icc) {
        Inkscape::ColorProfile *prof = SP_ACTIVE_DOCUMENT->profileManager->find(color.icc->colorProfile.c_str());
        if (prof && CMSSystem::isPrintColorSpace(prof)) {
            gtk_widget_show(GTK_WIDGET(_box_toomuchink));
            double ink_sum = 0;
            for (unsigned int i = 0; i < color.icc->colors.size(); i++) {
                ink_sum += color.icc->colors[i];
            }

            /* Some literature states that when the sum of paint values exceed 320%, it is considered to be a satured
               color,
                which means the paper can get too wet due to an excessive ammount of ink. This may lead to several
               issues
                such as misalignment and poor quality of printing in general.*/
            if (ink_sum > 3.2)
                gtk_widget_set_sensitive(_box_toomuchink, true);
        }
        else {
            gtk_widget_hide(GTK_WIDGET(_box_toomuchink));
        }
    }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
}

void ColorNotebook::_setCurrentPage(int i)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(_book), i);

    if (_buttons && (static_cast<size_t>(i) < _available_pages.size())) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_buttons[i]), TRUE);
    }
}

void ColorNotebook::_addPage(Page &page)
{
    Gtk::Widget *selector_widget;

    selector_widget = page.selector_factory->createWidget(_selected_color);
    if (selector_widget) {
        selector_widget->show();

        Glib::ustring mode_name = page.selector_factory->modeName();
        Gtk::Widget *tab_label = Gtk::manage(new Gtk::Label(mode_name));
        gint page_num = gtk_notebook_append_page(GTK_NOTEBOOK(_book), selector_widget->gobj(), tab_label->gobj());

        _buttons[page_num] = gtk_radio_button_new_with_label(NULL, mode_name.c_str());
        gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(_buttons[page_num]), FALSE);
        if (page_num > 0) {
            GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(_buttons[0]));
            gtk_radio_button_set_group(GTK_RADIO_BUTTON(_buttons[page_num]), group);
        }
        gtk_widget_show(_buttons[page_num]);
        gtk_box_pack_start(GTK_BOX(_buttonbox), _buttons[page_num], TRUE, TRUE, 0);

        g_signal_connect(G_OBJECT(_buttons[page_num]), "clicked", G_CALLBACK(_onButtonClicked), this);
    }
}
}
}
}

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

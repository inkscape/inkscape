/**
 * @file
 * A notebook with RGB, CMYK, CMS, HSL, and Wheel pages
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
#ifndef SEEN_SP_COLOR_NOTEBOOK_H
#define SEEN_SP_COLOR_NOTEBOOK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <boost/ptr_container/ptr_vector.hpp>
#if WITH_GTKMM_3_0
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif
#include <gtk/gtk.h>
#include <glib.h>

#include "color.h"
#include "ui/selected-color.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorNotebook
#if GTK_CHECK_VERSION(3, 0, 0)
    : public Gtk::Grid
#else
    : public Gtk::Table
#endif
{
public:
    ColorNotebook(SelectedColor &color);
    virtual ~ColorNotebook();

protected:
    struct Page {
        Page(Inkscape::UI::ColorSelectorFactory *selector_factory, bool enabled_full);

        Inkscape::UI::ColorSelectorFactory *selector_factory;
        bool enabled_full;
    };

    virtual void _initUI();
    void _addPage(Page &page);

    static void _onButtonClicked(GtkWidget *widget, ColorNotebook *colorbook);
    static void _onPickerClicked(GtkWidget *widget, ColorNotebook *colorbook);
    static void _onPageSwitched(GtkNotebook *notebook, GtkWidget *page, guint page_num, ColorNotebook *colorbook);
    virtual void _onSelectedColorChanged();

    void _updateICCButtons();
    void _setCurrentPage(int i);

    Inkscape::UI::SelectedColor &_selected_color;
    gulong _entryId;
    GtkWidget *_book;
    GtkWidget *_buttonbox;
    GtkWidget **_buttons;
    GtkWidget *_rgbal; /* RGBA entry */
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    GtkWidget *_box_outofgamut, *_box_colormanaged, *_box_toomuchink;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    GtkWidget *_btn_picker;
    GtkWidget *_p; /* Color preview */
    boost::ptr_vector<Page> _available_pages;

private:
    // By default, disallow copy constructor and assignment operator
    ColorNotebook(const ColorNotebook &obj);
    ColorNotebook &operator=(const ColorNotebook &obj);
};
}
}
}
#endif // SEEN_SP_COLOR_NOTEBOOK_H
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


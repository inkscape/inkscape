/**
 * \brief Fill and Stroke dialog,
 * based on sp_object_properties_dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "fill-and-stroke.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "inkscape-stock.h"
#include "selection.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "verbs.h"
#include "xml/repr.h"
#include "widgets/icon.h"

#include "dialogs/fill-style.h"
#include "dialogs/stroke-style.h"

#include <widgets/paint-selector.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

FillAndStroke::FillAndStroke()
    : UI::Widget::Panel ("", "dialogs.fillstroke", SP_VERB_DIALOG_FILL_STROKE),
      _page_fill(1, 1, true, true),
      _page_stroke_paint(1, 1, true, true),
      _page_stroke_style(1, 1, true, true),
      _composite_settings(SP_VERB_DIALOG_FILL_STROKE, "fillstroke", SimpleFilterModifier::BLUR)
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(0);

    contents->pack_start(_notebook, true, true);

    _notebook.append_page(_page_fill, _createPageTabLabel(_("Fill"), INKSCAPE_STOCK_PROPERTIES_FILL_PAGE));
    _notebook.append_page(_page_stroke_paint, _createPageTabLabel(_("Stroke _paint"), INKSCAPE_STOCK_PROPERTIES_STROKE_PAINT_PAGE));
    _notebook.append_page(_page_stroke_style, _createPageTabLabel(_("Stroke st_yle"), INKSCAPE_STOCK_PROPERTIES_STROKE_PAGE));

    _layoutPageFill();
    _layoutPageStrokePaint();
    _layoutPageStrokeStyle();

    contents->pack_start(_composite_settings, false, false, 0);

    show_all_children();

    _composite_settings.setSubject(&_subject);
}

FillAndStroke::~FillAndStroke()
{
    _composite_settings.setSubject(NULL);
}

void
FillAndStroke::_layoutPageFill()
{
    Gtk::Widget *fs = manage(Glib::wrap(sp_fill_style_widget_new()));
    _page_fill.table().attach(*fs, 0, 1, 0, 1);
}

void
FillAndStroke::_layoutPageStrokePaint()
{
    Gtk::Widget *ssp = manage(Glib::wrap(sp_stroke_style_paint_widget_new()));
    _page_stroke_paint.table().attach(*ssp, 0, 1, 0, 1);
}

void
FillAndStroke::_layoutPageStrokeStyle()
{
    //Gtk::Widget *ssl = manage(Glib::wrap(sp_stroke_style_line_widget_new()));
    //Gtk::Widget *ssl = static_cast<Gtk::Widget *>(sp_stroke_style_line_widget_new());
    Gtk::Widget *ssl = sp_stroke_style_line_widget_new();
    _page_stroke_style.table().attach(*ssl, 0, 1, 0, 1);
}

void
FillAndStroke::showPageFill()
{
    present();
    _notebook.set_current_page(0);
}

void
FillAndStroke::showPageStrokePaint()
{
    present();
    _notebook.set_current_page(1);
}

void
FillAndStroke::showPageStrokeStyle()
{
    present();
    _notebook.set_current_page(2);
}

Gtk::HBox&
FillAndStroke::_createPageTabLabel(const Glib::ustring& label, const char *label_image)
{
    Gtk::HBox *_tab_label_box = manage(new Gtk::HBox(false, 0));
    _tab_label_box->pack_start(*Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_DECORATION,
                                                       label_image)));

    Gtk::Label *_tab_label = manage(new Gtk::Label(label, true));
    _tab_label_box->pack_start(*_tab_label);
    _tab_label_box->show_all();

    return *_tab_label_box;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

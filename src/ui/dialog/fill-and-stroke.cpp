/** @file
 * @brief Fill and Stroke dialog - implementation
 *
 * Based on the old sp_object_properties_dialog.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004--2007 Authors
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "fill-and-stroke.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "selection.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "ui/icon-names.h"
#include "verbs.h"
#include "widgets/fill-style.h"
#include "widgets/icon.h"
#include "widgets/paint-selector.h"
#include "widgets/stroke-style.h"
#include "xml/repr.h"

#include "ui/view/view-widget.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

FillAndStroke::FillAndStroke()
    : UI::Widget::Panel ("", "/dialogs/fillstroke", SP_VERB_DIALOG_FILL_STROKE),
      _page_fill(1, 1, true, true),
      _page_stroke_paint(1, 1, true, true),
      _page_stroke_style(1, 1, true, true),
      _composite_settings(SP_VERB_DIALOG_FILL_STROKE, "fillstroke", SimpleFilterModifier::BLUR),
      hierID(0),
      trackActive(false),
      targetDesktop(0),
      fillWdgt(0),
      strokeWdgt(0)
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(0);

    contents->pack_start(_notebook, true, true);

    _notebook.append_page(_page_fill, _createPageTabLabel(_("Fill"), INKSCAPE_ICON_OBJECT_FILL));
    _notebook.append_page(_page_stroke_paint, _createPageTabLabel(_("Stroke _paint"), INKSCAPE_ICON_OBJECT_STROKE));
    _notebook.append_page(_page_stroke_style, _createPageTabLabel(_("Stroke st_yle"), INKSCAPE_ICON_OBJECT_STROKE_STYLE));

    _layoutPageFill();
    _layoutPageStrokePaint();
    _layoutPageStrokeStyle();

    contents->pack_start(_composite_settings, false, false, 0);

    show_all_children();

    _composite_settings.setSubject(&_subject);

    // Use C/gobject callbacks to avoid gtkmm rewrap-during-destruct issues:
    hierID = g_signal_connect( G_OBJECT(gobj()), "hierarchy-changed", G_CALLBACK(hierarchyChangeCB), this );

    g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK( activateDesktopCB ), this );
}

FillAndStroke::~FillAndStroke()
{
    _composite_settings.setSubject(NULL);
    if (hierID) {
        g_signal_handler_disconnect(G_OBJECT(gobj()), hierID);
        hierID = 0;
    }
}

gboolean FillAndStroke::activateDesktopCB(Inkscape::Application */*inkscape*/, SPDesktop *desktop, FillAndStroke *self )
{
    if (self && self->trackActive) {
        self->setTargetDesktop(desktop);
    }
    return FALSE;
}

bool FillAndStroke::hierarchyChangeCB(GtkWidget *widget, GtkWidget* /*prev*/, FillAndStroke *self)
{
    if (self) {
        GtkWidget *ww = gtk_widget_get_ancestor(widget, SP_TYPE_VIEW_WIDGET);
        bool newFlag = (ww == 0);
        if (newFlag != self->trackActive) {
            self->trackActive = newFlag;
            if (self->trackActive) {
                self->setTargetDesktop(SP_ACTIVE_DESKTOP);
            } else {
                self->setTargetDesktop(self->getDesktop());
            }
        }
    }
    return false;
}

void FillAndStroke::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    setTargetDesktop(desktop);
}

void FillAndStroke::setTargetDesktop(SPDesktop *desktop)
{
    if (fillWdgt) {
        sp_fill_style_widget_set_desktop(fillWdgt, desktop);
    }
    if (strokeWdgt) {
        sp_stroke_style_widget_set_desktop(strokeWdgt, desktop);
    }
}

void
FillAndStroke::_layoutPageFill()
{
    fillWdgt = manage(sp_fill_style_widget_new());
    _page_fill.table().attach(*fillWdgt, 0, 1, 0, 1);
}

void
FillAndStroke::_layoutPageStrokePaint()
{
    strokeWdgt = manage(sp_stroke_style_paint_widget_new());
    _page_stroke_paint.table().attach(*strokeWdgt, 0, 1, 0, 1);
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

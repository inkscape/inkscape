#define __COLOR_PICKER_C__

/** \file
 * \brief  Color picker button & window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) Authors 2000-2005
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "inkscape.h"
#include "desktop-handles.h"
#include "document.h"
#include "dialogs/dialog-events.h"

#include "widgets/sp-color-notebook.h"

#include "color-picker.h"

static bool _in_use = false;

namespace Inkscape {
namespace UI {
namespace Widget {

void sp_color_picker_color_mod(SPColorSelector *csel, GObject *cp);

ColorPicker::ColorPicker (const Glib::ustring& title, const Glib::ustring& tip,
                          guint32 rgba, bool undo)
          : _preview(rgba), _title(title), _rgba(rgba), _undo(undo),
           _colorSelectorDialog("dialogs.colorpickerwindow")
{
    setupDialog(title);
    set_relief (Gtk::RELIEF_NONE);
    _preview.show();
    add (_preview);
    _tt.set_tip (*this, tip);
}

ColorPicker::~ColorPicker()
{
    closeWindow();
    _colorSelector = NULL;
}

void
ColorPicker::setupDialog(const Glib::ustring &title)
{
    GtkWidget *dlg = GTK_WIDGET(_colorSelectorDialog.gobj());
    sp_transientize(dlg);

    _colorSelectorDialog.hide();
    _colorSelectorDialog.set_title (title);
    _colorSelectorDialog.set_border_width (4);
    _colorSelector = (SPColorSelector*)sp_color_selector_new(SP_TYPE_COLOR_NOTEBOOK);
    _colorSelectorDialog.get_vbox()->pack_start (
              *Glib::wrap(&_colorSelector->vbox), true, true, 0);

    g_signal_connect(G_OBJECT(_colorSelector), "dragged",
                         G_CALLBACK(sp_color_picker_color_mod), (void *)this);
    g_signal_connect(G_OBJECT(_colorSelector), "released",
                         G_CALLBACK(sp_color_picker_color_mod), (void *)this);
    g_signal_connect(G_OBJECT(_colorSelector), "changed",
                         G_CALLBACK(sp_color_picker_color_mod), (void *)this);

    gtk_widget_show(GTK_WIDGET(_colorSelector));

}




void
ColorPicker::setRgba32 (guint32 rgba)
{
    if (_in_use) return;

    _preview.setRgba32 (rgba);
    _rgba = rgba;
    if (_colorSelector)
    {
        SPColor color;
        color.set( rgba );
        _colorSelector->base->setColorAlpha(color, SP_RGBA32_A_F(rgba));
    }
}

void
ColorPicker::closeWindow()
{
    _colorSelectorDialog.hide();
}

void
ColorPicker::on_clicked()
{
    if (_colorSelector)
    {
        SPColor color;
        color.set( _rgba );
        _colorSelector->base->setColorAlpha(color, SP_RGBA32_A_F(_rgba));
    }
    _colorSelectorDialog.show();
}

void
ColorPicker::on_changed (guint32)
{
}

void
sp_color_picker_color_mod(SPColorSelector *csel, GObject *cp)
{
    if (_in_use) {
        return;
    } else {
        _in_use = true;
    }

    SPColor color;
    float alpha = 0;
    csel->base->getColorAlpha(color, alpha);
    guint32 rgba = color.toRGBA32( alpha );

    ColorPicker *ptr = (ColorPicker *)(cp);

    (ptr->_preview).setRgba32 (rgba);

    if (ptr->_undo && SP_ACTIVE_DESKTOP)
        sp_document_done(sp_desktop_document(SP_ACTIVE_DESKTOP), SP_VERB_NONE,
                         /* TODO: annotate */ "color-picker.cpp:130");

    ptr->on_changed (rgba);
    _in_use = false;
    ptr->_changed_signal.emit (rgba);
    ptr->_rgba = rgba;
}


}//namespace Widget
}//namespace UI
}//namespace Inkscape


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

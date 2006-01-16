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

#include "ui/dialog/dialog.h"

#include "widgets/sp-color-notebook.h"

#include "color-picker.h"

namespace Inkscape {
    namespace UI {
        namespace Widget {

struct CPPointer {
    ColorPicker *ptr;
};

class ColorPickerWindow : public Inkscape::UI::Dialog::Dialog {
public:
    ColorPickerWindow (ColorPicker* cp, const Glib::ustring &title);
    virtual ~ColorPickerWindow();
    void setRgba32 (guint32);
    
    SPColorSelector *_csel;
    CPPointer _cpp;
};

ColorPicker::ColorPicker (const Glib::ustring& title, const Glib::ustring& tip, 
                          guint32 rgba, bool undo)
: _preview(rgba), _window(0), 
  _title(title), _rgba(rgba), _undo(undo)
{
    set_relief (Gtk::RELIEF_NONE);
    _preview.show();
    add (_preview);
    _tt.set_tip (*this, tip);
}

ColorPicker::~ColorPicker()
{
    closeWindow();
}

void
ColorPicker::setRgba32 (guint32 rgba)
{
    _preview.setRgba32 (rgba);
    _rgba = rgba;
    if (_window)
        _window->setRgba32 (rgba);
}

void
ColorPicker::closeWindow()
{
    if (_window)
    {
        delete _window;
        _window = 0;
    }
}

void
ColorPicker::on_clicked()
{
    static int _x, _y;
    if (_window)
    { 
        _window->move(_x,_y);
        _window->present();
        return;
    }

    _window =new ColorPickerWindow (this, _title);
    _window->setRgba32 (_rgba);
    _window->show_all();
    _window->get_position (_x, _y);
}

void
ColorPicker::on_changed (guint32)
{
}

void
sp_color_picker_color_mod(SPColorSelector *csel, GObject *cp)
{
    SPColor color;
    float alpha;
    csel->base->getColorAlpha(color, &alpha);
    guint32 rgba = sp_color_get_rgba32_falpha(&color, alpha);
    
    ColorPicker *ptr = static_cast<CPPointer*>((void*)cp)->ptr;

    (ptr->_preview).setRgba32 (rgba);

    if (ptr->_undo && SP_ACTIVE_DESKTOP)
        sp_document_done(SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP));

    ptr->on_changed (rgba);
    ptr->_changed_signal.emit (rgba);
}

//==============================================================

ColorPickerWindow::ColorPickerWindow (ColorPicker *cp, const Glib::ustring &title)
    : Dialog("dialogs.colorpickerwindow")
{
    set_title (title);
    set_border_width (4);
    _csel = (SPColorSelector*)sp_color_selector_new(SP_TYPE_COLOR_NOTEBOOK,
                                  SP_COLORSPACE_TYPE_UNKNOWN);
    get_vbox()->pack_start (*Glib::wrap(&_csel->vbox), true, true, 0);

    _cpp.ptr = cp;
    g_signal_connect(G_OBJECT(_csel), "dragged",
                         G_CALLBACK(sp_color_picker_color_mod), &_cpp);
    g_signal_connect(G_OBJECT(_csel), "changed",
                         G_CALLBACK(sp_color_picker_color_mod), &_cpp);

    show();
} 

ColorPickerWindow::~ColorPickerWindow()
{
    _csel = 0;
}
    
void
ColorPickerWindow::setRgba32 (guint32 rgba)
{
    if (_csel) 
    {
        SPColor color;
        sp_color_set_rgb_rgba32(&color, rgba);
        _csel->base->setColorAlpha(color, SP_RGBA32_A_F(rgba));
    }
}

}}}

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

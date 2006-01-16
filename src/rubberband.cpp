#define __RUBBERBAND_C__

/**
 * \file src/rubberband.cpp
 * \brief Rubberbanding selector
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/sodipodi-ctrlrect.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "rubberband.h"

Inkscape::Rubberband *Inkscape::Rubberband::_instance = NULL;

Inkscape::Rubberband::Rubberband()
    : _desktop(NULL), _canvas(NULL)
{
    
}

void Inkscape::Rubberband::start(SPDesktop *d, NR::Point const &p)
{
    stop();
    _desktop = d;
    _start = p;
}

void Inkscape::Rubberband::stop()
{
    if (_canvas) {
        gtk_object_destroy((GtkObject *) _canvas);
        _canvas = NULL;
    }
}

void Inkscape::Rubberband::move(NR::Point const &p)
{
    if (_canvas == NULL) {
        _canvas = static_cast<CtrlRect *>(sp_canvas_item_new(SP_DT_CONTROLS(_desktop), SP_TYPE_CTRLRECT, NULL));
    }

    _desktop->scroll_to_point(&p);
    _end = p;

    _canvas->setRectangle(NR::Rect(_start, _end));
}

NR::Maybe<NR::Rect> Inkscape::Rubberband::getRectangle() const
{
    if (_canvas == NULL) {
        return NR::Nothing();
    }

    return NR::Rect(_start, _end);
}

Inkscape::Rubberband *Inkscape::Rubberband::get()
{
    if (_instance == NULL) {
        _instance = new Inkscape::Rubberband;
    }

    return _instance;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

#ifndef __INKSCAPE_CTRLRECT_H__
#define __INKSCAPE_CTRLRECT_H__

/**
 * \file sodipodi-ctrlrect.h
 * \brief Simple non-transformed rectangle, usable for rubberband
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include <glib/gtypes.h>
#include "sp-canvas.h"

#define SP_TYPE_CTRLRECT (sp_ctrlrect_get_type ())
#define SP_CTRLRECT(obj) (GTK_CHECK_CAST((obj), SP_TYPE_CTRLRECT, CtrlRect))
#define SP_CTRLRECT_CLASS(c) (GTK_CHECK_CLASS_CAST((c), SP_TYPE_CTRLRECT, SPCtrlRectClass))
#define SP_IS_CTRLRECT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CTRLRECT))
#define SP_IS_CTRLRECT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CTRLRECT))

class CtrlRect : public SPCanvasItem
{
public:

    void init();
    void setColor(guint32 b, bool h, guint f);
    void setShadow(int s, guint c);
    void setRectangle(Geom::Rect const &r);
    void setDashed(bool d);

    void render(SPCanvasBuf *buf);
    void update(Geom::Matrix const &affine, unsigned int flags);
    
private:
    void _requestUpdate();
    
    Geom::Rect _rect;
    bool _has_fill;
    bool _dashed;
    NRRectL _area;
    gint _shadow_size;
    guint32 _border_color;
    guint32 _fill_color;
    guint32 _shadow_color;    
    int _shadow;
};

struct SPCtrlRectClass : public SPCanvasItemClass {};

GtkType sp_ctrlrect_get_type();

#endif

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

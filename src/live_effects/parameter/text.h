#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_TEXT_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_TEXT_H

/*
 * Inkscape::LivePathEffectParameters
 *
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>

//#include "display/canvas-text.h"
#include "display/canvas-bpath.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {

namespace LivePathEffect {

class TextParam : public Parameter {
public:
    TextParam( const Glib::ustring& label,
               const Glib::ustring& tip,
               const Glib::ustring& key,
               Inkscape::UI::Widget::Registry* wr,
               Effect* effect,
               const Glib::ustring default_value = "");
    virtual ~TextParam();

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;

    void param_setValue(const Glib::ustring newvalue);
    virtual void param_set_default();
    void setPos(Geom::Point pos);
    void setAnchor(double x_value, double y_value);

    const Glib::ustring get_value() { return defvalue; };

private:
    TextParam(const TextParam&);
    TextParam& operator=(const TextParam&);
    double anchor_x;
    double anchor_y;

    Glib::ustring defvalue;

    SPCanvasText *canvas_text;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

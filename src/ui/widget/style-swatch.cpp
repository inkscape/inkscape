/**
 * \brief Static style swatch (fill, stroke, opacity)
 *
 * Author:
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2005 author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "style-swatch.h"

#include "widgets/spw-utilities.h"
#include "ui/widget/color-preview.h"

#include "style.h"
#include "sp-linear-gradient-fns.h"
#include "sp-radial-gradient-fns.h"
#include "sp-pattern.h"
#include "xml/repr.h"
#include "widgets/widget-sizes.h"
#include "helper/units.h"

enum {
    SS_FILL,
    SS_STROKE
};


namespace Inkscape {
namespace UI {
namespace Widget {

StyleSwatch::StyleSwatch(SPCSSAttr *css)
    : _css (NULL),

      _table(2, 6),

      _sw_unit(NULL),

      _tooltips ()
{
    _label[SS_FILL].set_markup(_("F:"));
    _label[SS_STROKE].set_markup(_("S:"));

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        _label[i].set_alignment(0.0, 0.5);
        _label[i].set_padding(0, 0);

        _color_preview[i] = new Inkscape::UI::Widget::ColorPreview (0);
    }

    _opacity_value.set_alignment(0.0, 0.5);
    _opacity_value.set_padding(0, 0);

    _table.set_col_spacings (2);
    _table.set_row_spacings (0);

    _stroke.pack_start(_place[SS_STROKE]);
    _stroke_width_place.add(_stroke_width);
    _stroke.pack_start(_stroke_width_place, Gtk::PACK_SHRINK);

    _table.attach(_label[SS_FILL], 0,1, 0,1, Gtk::SHRINK, Gtk::SHRINK);
    _table.attach(_label[SS_STROKE], 0,1, 1,2, Gtk::SHRINK, Gtk::SHRINK);

    _table.attach(_place[SS_FILL], 1,2, 0,1);
    _table.attach(_stroke, 1,2, 1,2);

    _opacity_place.add(_opacity_value);
    _table.attach(_opacity_place, 2,3, 0,2, Gtk::SHRINK, Gtk::SHRINK);

    pack_start(_table, true, true, 0);

    set_size_request (STYLE_SWATCH_WIDTH, -1);

    sp_set_font_size_smaller (GTK_WIDGET(_opacity_value.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_stroke_width.gobj()));
    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        sp_set_font_size_smaller (GTK_WIDGET(_value[i].gobj()));
        sp_set_font_size_smaller (GTK_WIDGET(_place[i].gobj()));
        sp_set_font_size_smaller (GTK_WIDGET(_label[i].gobj()));
    }

    setStyle (css);
}

StyleSwatch::~StyleSwatch()
{
    if (_css) 
        sp_repr_css_attr_unref (_css);

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        delete _color_preview[i];
    }
}

void
StyleSwatch::setStyle(SPCSSAttr *css)
{
    if (_css) 
        sp_repr_css_attr_unref (_css);
    _css = sp_repr_css_attr_new();
    sp_repr_css_merge(_css, css);

    gchar const *css_string = sp_repr_css_write_string (_css);
    SPStyle *temp_spstyle = sp_style_new();
    sp_style_merge_from_style_string (temp_spstyle, css_string);

    setStyle (temp_spstyle);

    sp_style_unref (temp_spstyle);
}

void
StyleSwatch::setStyle(SPStyle *query)
{
    _place[SS_FILL].remove();
    _place[SS_STROKE].remove();

    bool has_stroke = true;

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        Gtk::EventBox *place = &(_place[i]);

        SPIPaint *paint;
        if (i == SS_FILL) {
            paint = &(query->fill);
        } else {
            paint = &(query->stroke);
        }

        if (paint->set && paint->type == SP_PAINT_TYPE_COLOR) {
            guint32 color = sp_color_get_rgba32_falpha (&(paint->value.color), 
                                                        SP_SCALE24_TO_FLOAT ((i == SS_FILL)? query->fill_opacity.value : query->stroke_opacity.value));
            ((Inkscape::UI::Widget::ColorPreview*)_color_preview[i])->setRgba32 (color);
            _color_preview[i]->show_all();
            place->add(*_color_preview[i]);
            gchar *tip;
            if (i == SS_FILL) {
                tip = g_strdup_printf ("Fill: %06x/%.3g", color >> 8, SP_RGBA32_A_F(color));
            } else {
                tip = g_strdup_printf ("Stroke: %06x/%.3g", color >> 8, SP_RGBA32_A_F(color));
            }
            _tooltips.set_tip(*place, tip);
            g_free (tip);
        } else if (paint->set && paint->type == SP_PAINT_TYPE_PAINTSERVER) {
            SPPaintServer *server = (i == SS_FILL)? SP_STYLE_FILL_SERVER (query) : SP_STYLE_STROKE_SERVER (query);

            if (SP_IS_LINEARGRADIENT (server)) {
                _value[i].set_markup(_("L Gradient"));
                place->add(_value[i]);
                _tooltips.set_tip(*place, (i == SS_FILL)? (_("Linear gradient fill")) : (_("Linear gradient stroke")));
            } else if (SP_IS_RADIALGRADIENT (server)) {
                _value[i].set_markup(_("R Gradient"));
                place->add(_value[i]);
                _tooltips.set_tip(*place, (i == SS_FILL)? (_("Radial gradient fill")) : (_("Radial gradient stroke")));
            } else if (SP_IS_PATTERN (server)) {
                _value[i].set_markup(_("Pattern"));
                place->add(_value[i]);
                _tooltips.set_tip(*place, (i == SS_FILL)? (_("Pattern fill")) : (_("Pattern stroke")));
            }

        } else if (paint->set && paint->type == SP_PAINT_TYPE_NONE) {
            _value[i].set_markup(_("None"));
            place->add(_value[i]);
            _tooltips.set_tip(*place, (i == SS_FILL)? (_("No fill")) : (_("No stroke")));
            if (i == SS_STROKE) has_stroke = false;
        } else if (!paint->set) {
            _value[i].set_markup(_("Unset"));
            place->add(_value[i]);
            _tooltips.set_tip(*place, (i == SS_FILL)? (_("Unset fill")) : (_("Unset stroke")));
            if (i == SS_STROKE) has_stroke = false;
        }
    }

// Now query stroke_width
    if (has_stroke) {
        double w;
        if (_sw_unit) {
            w = sp_pixels_get_units(query->stroke_width.computed, *_sw_unit);
        } else {
            w = query->stroke_width.computed;
        }

        {
            gchar *str = g_strdup_printf(" %.3g", w);
            _stroke_width.set_markup(str);
            g_free (str);
        }
        {
            gchar *str = g_strdup_printf(_("Stroke width: %.5g%s"), 
                                         w, 
                                         _sw_unit? sp_unit_get_abbreviation(_sw_unit) : "px");
            _tooltips.set_tip(_stroke_width_place, str);
            g_free (str);
        }
    } else {
        _tooltips.unset_tip(_stroke_width_place);
        _stroke_width.set_markup ("");
    }

    gdouble op = SP_SCALE24_TO_FLOAT(query->opacity.value);
    if (op != 1) {
        {
            gchar *str;
            if (op == 0)
                str = g_strdup_printf(_("0:%.3g"), op);
            else 
                str = g_strdup_printf(_("0:.%d"), (int) (op*10));
            _opacity_value.set_markup (str);
            g_free (str);
        }
        {
            gchar *str = g_strdup_printf(_("Opacity: %.3g"), op);
            _tooltips.set_tip(_opacity_place, str);
            g_free (str);
        }
    } else {
        _tooltips.unset_tip(_opacity_place);
        _opacity_value.set_markup ("");
    }

    show_all();
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

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

#include <cstring>
#include <string>

#include "style-swatch.h"

#include "widgets/spw-utilities.h"
#include "ui/widget/color-preview.h"

#include "style.h"
#include "sp-linear-gradient-fns.h"
#include "sp-radial-gradient-fns.h"
#include "sp-pattern.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "widgets/widget-sizes.h"
#include "helper/units.h"
#include "helper/action.h"
#include "inkscape.h"

enum {
    SS_FILL,
    SS_STROKE
};

static void style_swatch_attr_changed( Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const */*old_value*/, gchar const */*new_value*/,
                                       bool /*is_interactive*/, gpointer data)
{
    Inkscape::UI::Widget::StyleSwatch *ss = (Inkscape::UI::Widget::StyleSwatch *) data;

    if (!strcmp (name, "style")) { // FIXME: watching only for the style attr, no CSS attrs
        SPCSSAttr *css = sp_repr_css_attr_inherited(repr, "style");
        ss->setStyle (css);
    }
}


static Inkscape::XML::NodeEventVector style_swatch_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    style_swatch_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


static void style_swatch_tool_attr_changed( Inkscape::XML::Node */*repr*/, gchar const *name,
                                            gchar const */*old_value*/, gchar const *new_value,
                                            bool /*is_interactive*/, gpointer data)
{
    Inkscape::UI::Widget::StyleSwatch *ss = (Inkscape::UI::Widget::StyleSwatch *) data;

    if (!strcmp (name, "usecurrent")) { // FIXME: watching only for the style attr, no CSS attrs
        if (!strcmp (new_value, "1")) {
            ss->setWatched (inkscape_get_repr(INKSCAPE, "desktop"), inkscape_get_repr(INKSCAPE, ss->_tool_path));
        } else {
            ss->setWatched (inkscape_get_repr(INKSCAPE, ss->_tool_path), NULL);
        }
        // UGLY HACK: we have to reconnect to the watched tool repr again, retrieving it from the stored
        // tool_path, because the actual repr keeps shifting with each change, no idea why
        ss->setWatchedTool(ss->_tool_path, false);
    }
}

static Inkscape::XML::NodeEventVector style_swatch_tool_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    style_swatch_tool_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

namespace Inkscape {
namespace UI {
namespace Widget {

StyleSwatch::StyleSwatch(SPCSSAttr *css, gchar const *main_tip)
    :
      _tool_path(NULL),
      _desktop(0),
      _verb_t(0),
      _css (NULL),

      _watched(NULL),
      _watched_tool(NULL),

      _table(2, 6),

      _sw_unit(NULL),

      _tooltips ()
{
    _label[SS_FILL].set_markup(_("Fill:"));
    _label[SS_STROKE].set_markup(_("Stroke:"));

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

    _table.attach(_label[SS_FILL], 0,1, 0,1, Gtk::FILL, Gtk::SHRINK);
    _table.attach(_label[SS_STROKE], 0,1, 1,2, Gtk::FILL, Gtk::SHRINK);

    _table.attach(_place[SS_FILL], 1,2, 0,1);
    _table.attach(_stroke, 1,2, 1,2);

    _opacity_place.add(_opacity_value);
    _table.attach(_opacity_place, 2,3, 0,2, Gtk::SHRINK, Gtk::SHRINK);

    _swatch.add(_table);
    pack_start(_swatch, true, true, 0);

    set_size_request (STYLE_SWATCH_WIDTH, -1);

    sp_set_font_size_smaller (GTK_WIDGET(_opacity_value.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_stroke_width.gobj()));
    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        sp_set_font_size_smaller (GTK_WIDGET(_value[i].gobj()));
        sp_set_font_size_smaller (GTK_WIDGET(_place[i].gobj()));
        sp_set_font_size_smaller (GTK_WIDGET(_label[i].gobj()));
    }

    setStyle (css);

    _swatch.signal_button_press_event().connect(sigc::mem_fun(*this, &StyleSwatch::on_click));

    _tooltips.set_tip(_swatch, main_tip);
}

void StyleSwatch::setClickVerb(sp_verb_t verb_t) {
    _verb_t = verb_t;
}

void StyleSwatch::setDesktop(SPDesktop *desktop) {
    _desktop = desktop;
}

bool
StyleSwatch::on_click(GdkEventButton */*event*/)
{
    if (this->_desktop && this->_verb_t != SP_VERB_NONE) {
        Inkscape::Verb *verb = Inkscape::Verb::get(this->_verb_t);
        SPAction *action = verb->get_action((Inkscape::UI::View::View *) this->_desktop);
        sp_action_perform (action, NULL);
        return true;
    }
    return false;
}

StyleSwatch::~StyleSwatch()
{
    if (_css)
        sp_repr_css_attr_unref (_css);

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        delete _color_preview[i];
    }

    if (_watched) {
        sp_repr_remove_listener_by_data(_watched, this);
        Inkscape::GC::release(_watched);
        _watched = NULL;
    }

    if (_watched_tool) {
        sp_repr_remove_listener_by_data(_watched_tool, this);
        Inkscape::GC::release(_watched_tool);
        _watched_tool = NULL;
        _tool_path = NULL;
    }
}

void
StyleSwatch::setWatched(Inkscape::XML::Node *watched, Inkscape::XML::Node *secondary)
{
    if (_watched) {
        sp_repr_remove_listener_by_data(_watched, this);
        Inkscape::GC::release(_watched);
        _watched = NULL;
    }

    if (watched) {
        _watched = watched;
        Inkscape::GC::anchor(_watched);
        sp_repr_add_listener(_watched, &style_swatch_repr_events, this);
        sp_repr_synthesize_events(_watched, &style_swatch_repr_events, this);

        // If desktop's last-set style is empty, a tool uses its own fixed style even if set to use
        // last-set (so long as it's empty). To correctly show this, we're passed the second repr,
        // that of the tool prefs node, from which we now setStyle if the watched repr's style is
        // empty.
        if (secondary) {
            SPCSSAttr *css = sp_repr_css_attr_inherited(watched, "style");
            if (!css->attributeList()) { // is css empty?
                SPCSSAttr *css_secondary = sp_repr_css_attr_inherited(secondary, "style");
                this->setStyle (css_secondary);
            }
        }
    }
}

void
StyleSwatch::setWatchedTool(const char *path, bool synthesize)
{
    if (_watched_tool) {
        sp_repr_remove_listener_by_data(_watched_tool, this);
        Inkscape::GC::release(_watched_tool);
        _watched_tool = NULL;
        _tool_path = NULL;
    }

    if (path) {
        _tool_path = (char *) path;
        Inkscape::XML::Node *watched_tool = inkscape_get_repr(INKSCAPE, path);
        if (watched_tool) {
            _watched_tool = watched_tool;
            Inkscape::GC::anchor(_watched_tool);
            sp_repr_add_listener(_watched_tool, &style_swatch_tool_repr_events, this);
            if (synthesize) {
                sp_repr_synthesize_events(_watched_tool, &style_swatch_tool_repr_events, this);
            }
        }
    }

}


void
StyleSwatch::setStyle(SPCSSAttr *css)
{
    if (_css)
        sp_repr_css_attr_unref (_css);

    if (!css)
        return;

    _css = sp_repr_css_attr_new();
    sp_repr_css_merge(_css, css);

    gchar const *css_string = sp_repr_css_write_string (_css);
    SPStyle *temp_spstyle = sp_style_new(SP_ACTIVE_DOCUMENT);
    if (css_string)
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

        if (paint->set && paint->isPaintserver()) {
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

        } else if (paint->set && paint->isColor()) {
            guint32 color = paint->value.color.toRGBA32( SP_SCALE24_TO_FLOAT ((i == SS_FILL)? query->fill_opacity.value : query->stroke_opacity.value) );
            ((Inkscape::UI::Widget::ColorPreview*)_color_preview[i])->setRgba32 (color);
            _color_preview[i]->show_all();
            place->add(*_color_preview[i]);
            gchar *tip;
            if (i == SS_FILL) {
                tip = g_strdup_printf (_("Fill: %06x/%.3g"), color >> 8, SP_RGBA32_A_F(color));
            } else {
                tip = g_strdup_printf (_("Stroke: %06x/%.3g"), color >> 8, SP_RGBA32_A_F(color));
            }
            _tooltips.set_tip(*place, tip);
            g_free (tip);
        } else if (paint->set && paint->isNone()) {
            _value[i].set_markup(_("<i>None</i>"));
            place->add(_value[i]);
            _tooltips.set_tip(*place, (i == SS_FILL)? (_("No fill")) : (_("No stroke")));
            if (i == SS_STROKE) has_stroke = false;
        } else if (!paint->set) {
            _value[i].set_markup(_("<b>Unset</b>"));
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
                str = g_strdup_printf(_("O:%.3g"), op);
            else
                str = g_strdup_printf(_("O:.%d"), (int) (op*10));
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

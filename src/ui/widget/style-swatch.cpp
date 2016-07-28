/**
 * @file
 * Static style swatch (fill, stroke, opacity).
 */
/* Authors:
 *   buliabyak@gmail.com
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2005-2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "style-swatch.h"

#include <cstring>
#include <string>

#include "widgets/spw-utilities.h"
#include "ui/widget/color-preview.h"

#include "style.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-pattern.h"
#include "xml/repr.h"
#include "xml/sp-css-attr.h"
#include "widgets/widget-sizes.h"
#include "util/units.h"
#include "helper/action.h"
#include "helper/action-context.h"
#include "preferences.h"
#include "inkscape.h"
#include "verbs.h"
#include <glibmm/i18n.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

enum {
    SS_FILL,
    SS_STROKE
};

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Watches whether the tool uses the current style.
 */
class StyleSwatch::ToolObserver : public Inkscape::Preferences::Observer {
public:
    ToolObserver(Glib::ustring const &path, StyleSwatch &ss) : 
        Observer(path),
        _style_swatch(ss)
    {}
    virtual void notify(Inkscape::Preferences::Entry const &val);
private:
    StyleSwatch &_style_swatch;
};

/**
 * Watches for changes in the observed style pref.
 */
class StyleSwatch::StyleObserver : public Inkscape::Preferences::Observer {
public:
    StyleObserver(Glib::ustring const &path, StyleSwatch &ss) :
        Observer(path),
        _style_swatch(ss)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        this->notify(prefs->getEntry(path));
    }
    virtual void notify(Inkscape::Preferences::Entry const &val) {
        SPCSSAttr *css = val.getInheritedStyle();
        _style_swatch.setStyle(css);
        sp_repr_css_attr_unref(css);
    }
private:
    StyleSwatch &_style_swatch;
};

void StyleSwatch::ToolObserver::notify(Inkscape::Preferences::Entry const &val)
{
    bool usecurrent = val.getBool();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (_style_swatch._style_obs) delete _style_swatch._style_obs;

    if (usecurrent) {
        _style_swatch._style_obs = new StyleObserver("/desktop/style", _style_swatch);
        
        // If desktop's last-set style is empty, a tool uses its own fixed style even if set to use
        // last-set (so long as it's empty). To correctly show this, we get the tool's style
        // if the desktop's style is empty.
        SPCSSAttr *css = prefs->getStyle("/desktop/style");
        if (!css->attributeList()) {
            SPCSSAttr *css2 = prefs->getInheritedStyle(_style_swatch._tool_path + "/style");
            _style_swatch.setStyle(css2);
            sp_repr_css_attr_unref(css2);
        }
        sp_repr_css_attr_unref(css);
    } else {
        _style_swatch._style_obs = new StyleObserver(_style_swatch._tool_path + "/style", _style_swatch);
    }
    prefs->addObserver(*_style_swatch._style_obs);
}

StyleSwatch::StyleSwatch(SPCSSAttr *css, gchar const *main_tip)
    :
      _desktop(NULL),
      _verb_t(0),
      _css(NULL),
      _tool_obs(NULL),
      _style_obs(NULL),
#if WITH_GTKMM_3_0
      _table(Gtk::manage(new Gtk::Grid())),
#else
      _table(Gtk::manage(new Gtk::Table(2, 6))),
#endif
      _sw_unit(NULL)
{
    set_name("StyleSwatch");
    
    _label[SS_FILL].set_markup(_("Fill:"));
    _label[SS_STROKE].set_markup(_("Stroke:"));

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        _label[i].set_alignment(0.0, 0.5);
        _label[i].set_padding(0, 0);

        _color_preview[i] = new Inkscape::UI::Widget::ColorPreview (0);
    }

    _opacity_value.set_alignment(0.0, 0.5);
    _opacity_value.set_padding(0, 0);

#if WITH_GTKMM_3_0
    _table->set_column_spacing(2);
    _table->set_row_spacing(0);
#else
    _table->set_col_spacings(2);
    _table->set_row_spacings(0);
#endif

    _stroke.pack_start(_place[SS_STROKE]);
    _stroke_width_place.add(_stroke_width);
    _stroke.pack_start(_stroke_width_place, Gtk::PACK_SHRINK);
    
    _opacity_place.add(_opacity_value);

#if WITH_GTKMM_3_0
    _table->attach(_label[SS_FILL],   0, 0, 1, 1);
    _table->attach(_label[SS_STROKE], 0, 1, 1, 1);
    _table->attach(_place[SS_FILL],   1, 0, 1, 1);
    _table->attach(_stroke,           1, 1, 1, 1);
    _table->attach(_opacity_place,    2, 0, 1, 2);
#else
    _table->attach(_label[SS_FILL], 0,1, 0,1, Gtk::FILL, Gtk::SHRINK);
    _table->attach(_label[SS_STROKE], 0,1, 1,2, Gtk::FILL, Gtk::SHRINK);
    _table->attach(_place[SS_FILL], 1,2, 0,1);
    _table->attach(_stroke, 1,2, 1,2);
    _table->attach(_opacity_place, 2,3, 0,2, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _swatch.add(*_table);
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

    if (main_tip)
    {
        _swatch.set_tooltip_text(main_tip);
    }
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
        SPAction *action = verb->get_action(Inkscape::ActionContext((Inkscape::UI::View::View *) this->_desktop));
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

    if (_style_obs) delete _style_obs;
    if (_tool_obs) delete _tool_obs;
}

void
StyleSwatch::setWatchedTool(const char *path, bool synthesize)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    
    if (_tool_obs) {
        delete _tool_obs;
        _tool_obs = NULL;
    }

    if (path) {
        _tool_path = path;
        _tool_obs = new ToolObserver(_tool_path + "/usecurrent", *this);
        prefs->addObserver(*_tool_obs);
    } else {
        _tool_path = "";
    }
    
    // hack until there is a real synthesize events function for prefs,
    // which shouldn't be hard to write once there is sufficient need for it
    if (synthesize && _tool_obs) {
        _tool_obs->notify(prefs->getEntry(_tool_path + "/usecurrent"));
    }
}


void StyleSwatch::setStyle(SPCSSAttr *css)
{
    if (_css)
        sp_repr_css_attr_unref (_css);

    if (!css)
        return;

    _css = sp_repr_css_attr_new();
    sp_repr_css_merge(_css, css);

    Glib::ustring css_string;
    sp_repr_css_write_string (_css, css_string);

    SPStyle style(SP_ACTIVE_DOCUMENT);
    if (!css_string.empty()) {
        style.mergeString(css_string.c_str());
    }
    setStyle (&style);
}

void StyleSwatch::setStyle(SPStyle *query)
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
                place->set_tooltip_text((i == SS_FILL)? (_("Linear gradient fill")) : (_("Linear gradient stroke")));
            } else if (SP_IS_RADIALGRADIENT (server)) {
                _value[i].set_markup(_("R Gradient"));
                place->add(_value[i]);
                place->set_tooltip_text((i == SS_FILL)? (_("Radial gradient fill")) : (_("Radial gradient stroke")));
            } else if (SP_IS_PATTERN (server)) {
                _value[i].set_markup(_("Pattern"));
                place->add(_value[i]);
                place->set_tooltip_text((i == SS_FILL)? (_("Pattern fill")) : (_("Pattern stroke")));
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
            place->set_tooltip_text(tip);
            g_free (tip);
        } else if (paint->set && paint->isNone()) {
            _value[i].set_markup(C_("Fill and stroke", "<i>None</i>"));
            place->add(_value[i]);
            place->set_tooltip_text((i == SS_FILL)? (C_("Fill and stroke", "No fill")) : (C_("Fill and stroke", "No stroke")));
            if (i == SS_STROKE) has_stroke = false;
        } else if (!paint->set) {
            _value[i].set_markup(_("<b>Unset</b>"));
            place->add(_value[i]);
            place->set_tooltip_text((i == SS_FILL)? (_("Unset fill")) : (_("Unset stroke")));
            if (i == SS_STROKE) has_stroke = false;
        }
    }

// Now query stroke_width
    if (has_stroke) {
        double w;
        if (_sw_unit) {
            w = Inkscape::Util::Quantity::convert(query->stroke_width.computed, "px", _sw_unit);
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
                                         _sw_unit? _sw_unit->abbr.c_str() : "px");
            _stroke_width_place.set_tooltip_text(str);
            g_free (str);
        }
    } else {
        _stroke_width_place.set_tooltip_text("");
        _stroke_width.set_markup("");
        _stroke_width.set_has_tooltip(false);
    }

    gdouble op = SP_SCALE24_TO_FLOAT(query->opacity.value);
    if (op != 1) {
        {
            gchar *str;
            str = g_strdup_printf(_("O: %2.0f"), (op*100.0));
            _opacity_value.set_markup (str);
            g_free (str);
        }
        {
            gchar *str = g_strdup_printf(_("Opacity: %2.1f %%"), (op*100.0));
            _opacity_place.set_tooltip_text(str);
            g_free (str);
        }
    } else {
        _opacity_place.set_tooltip_text("");
        _opacity_value.set_markup("");
        _opacity_value.set_has_tooltip(false);
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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

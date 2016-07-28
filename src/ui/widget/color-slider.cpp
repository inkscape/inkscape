/**
 * @file
 * A slider with colored background - implementation.
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gdkmm/cursor.h>
#include <gdkmm/screen.h>
#include <gdkmm/general.h>
#include <gtkmm/adjustment.h>
#if WITH_GTKMM_3_0
#include <gtkmm/stylecontext.h>
#else
#include <gtkmm/style.h>
#endif
#include <gtk/gtk.h>

#include "ui/widget/color-scales.h"
#include "ui/widget/color-slider.h"
#include "preferences.h"

static const gint SLIDER_WIDTH = 96;
static const gint SLIDER_HEIGHT = 8;
static const gint ARROW_SIZE = 7;

static const guchar *sp_color_slider_render_gradient(gint x0, gint y0, gint width, gint height, gint c[], gint dc[],
                                                     guint b0, guint b1, guint mask);
static const guchar *sp_color_slider_render_map(gint x0, gint y0, gint width, gint height, guchar *map, gint start,
                                                gint step, guint b0, guint b1, guint mask);

namespace Inkscape {
namespace UI {
namespace Widget {

#if GTK_CHECK_VERSION(3, 0, 0)
ColorSlider::ColorSlider(Glib::RefPtr<Gtk::Adjustment> adjustment)
    : _dragging(false)
#else
ColorSlider::ColorSlider(Gtk::Adjustment *adjustment)
    : _dragging(false)
    , _adjustment(NULL)
#endif
    , _value(0.0)
    , _oldvalue(0.0)
    , _mapsize(0)
    , _map(NULL)
{
    _c0[0] = 0x00;
    _c0[1] = 0x00;
    _c0[2] = 0x00;
    _c0[3] = 0xff;

    _cm[0] = 0xff;
    _cm[1] = 0x00;
    _cm[2] = 0x00;
    _cm[3] = 0xff;

    _c0[0] = 0xff;
    _c0[1] = 0xff;
    _c0[2] = 0xff;
    _c0[3] = 0xff;

    _b0 = 0x5f;
    _b1 = 0xa0;
    _bmask = 0x08;

    setAdjustment(adjustment);
}

ColorSlider::~ColorSlider()
{
    if (_adjustment) {
        _adjustment_changed_connection.disconnect();
        _adjustment_value_changed_connection.disconnect();
#if GTK_CHECK_VERSION(3, 0, 0)
        _adjustment.reset();
#else
        _adjustment->unreference();
        _adjustment = NULL;
#endif
    }
}

void ColorSlider::on_realize()
{
    set_realized();

    if (!_gdk_window) {
        GdkWindowAttr attributes;
        gint attributes_mask;
        Gtk::Allocation allocation = get_allocation();

        memset(&attributes, 0, sizeof(attributes));
        attributes.x = allocation.get_x();
        attributes.y = allocation.get_y();
        attributes.width = allocation.get_width();
        attributes.height = allocation.get_height();
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.visual = gdk_screen_get_system_visual(gdk_screen_get_default());
#if !GTK_CHECK_VERSION(3, 0, 0)
        attributes.colormap = gdk_screen_get_system_colormap(gdk_screen_get_default());
#endif
        attributes.event_mask = get_events();
        attributes.event_mask |= (Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                                  Gdk::POINTER_MOTION_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);

#if GTK_CHECK_VERSION(3, 0, 0)
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
#else
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
#endif

        _gdk_window = Gdk::Window::create(get_parent_window(), &attributes, attributes_mask);
        set_window(_gdk_window);
        _gdk_window->set_user_data(gobj());

#if !GTK_CHECK_VERSION(3, 0, 0)
        style_attach();
#endif
    }
}

void ColorSlider::on_unrealize()
{
    _gdk_window.reset();

    Gtk::Widget::on_unrealize();
}

void ColorSlider::on_size_allocate(Gtk::Allocation &allocation)
{
    set_allocation(allocation);

    if (get_realized()) {
        _gdk_window->move_resize(allocation.get_x(), allocation.get_y(), allocation.get_width(),
                                 allocation.get_height());
    }
}

#if GTK_CHECK_VERSION(3, 0, 0)

void ColorSlider::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
    Gtk::Border padding = style_context->get_padding(get_state_flags());
    int width = SLIDER_WIDTH + padding.get_left() + padding.get_right();
    minimum_width = natural_width = width;
}

void ColorSlider::get_preferred_width_for_height_vfunc(int /*height*/, int &minimum_width, int &natural_width) const
{
    get_preferred_width(minimum_width, natural_width);
}

void ColorSlider::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
    Gtk::Border padding = style_context->get_padding(get_state_flags());
    int height = SLIDER_HEIGHT + padding.get_top() + padding.get_bottom();
    minimum_height = natural_height = height;
}

void ColorSlider::get_preferred_height_for_width_vfunc(int /*width*/, int &minimum_height, int &natural_height) const
{
    get_preferred_height(minimum_height, natural_height);
}

#else

void ColorSlider::on_size_request(Gtk::Requisition *requisition)
{
    GtkStyle *style = gtk_widget_get_style(gobj());
    requisition->width = SLIDER_WIDTH + style->xthickness * 2;
    requisition->height = SLIDER_HEIGHT + style->ythickness * 2;
}

bool ColorSlider::on_expose_event(GdkEventExpose *event)
{
    bool result = false;

    if (get_is_drawable()) {
        Cairo::RefPtr<Cairo::Context> cr = _gdk_window->create_cairo_context();
        result = on_draw(cr);
    }
    return result;
}

#endif

bool ColorSlider::on_button_press_event(GdkEventButton *event)
{
    if (event->button == 1) {
        Gtk::Allocation allocation = get_allocation();
        gint cx, cw;
#if GTK_CHECK_VERSION(3, 0, 0)
        cx = get_style_context()->get_padding(get_state_flags()).get_left();
#else
        cx = get_style()->get_xthickness();
#endif
        cw = allocation.get_width() - 2 * cx;
        signal_grabbed.emit();
        _dragging = true;
        _oldvalue = _value;
        ColorScales::setScaled(_adjustment->gobj(), CLAMP((gfloat)(event->x - cx) / cw, 0.0, 1.0));
        signal_dragged.emit();

#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_device_grab(
            gdk_event_get_device(reinterpret_cast<GdkEvent *>(event)), _gdk_window->gobj(), GDK_OWNERSHIP_NONE, FALSE,
            static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK), NULL, event->time);
#else
        gdk_pointer_grab(get_window()->gobj(), FALSE,
                         static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK), NULL, NULL,
                         event->time);
#endif
    }

    return false;
}

bool ColorSlider::on_button_release_event(GdkEventButton *event)
{
    if (event->button == 1) {

#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_device_ungrab(gdk_event_get_device(reinterpret_cast<GdkEvent *>(event)),
                          gdk_event_get_time(reinterpret_cast<GdkEvent *>(event)));
#else
        get_window()->pointer_ungrab(event->time);
#endif

        _dragging = false;
        signal_released.emit();
        if (_value != _oldvalue) {
            signal_value_changed.emit();
        }
    }

    return false;
}

bool ColorSlider::on_motion_notify_event(GdkEventMotion *event)
{
    if (_dragging) {
        gint cx, cw;
        Gtk::Allocation allocation = get_allocation();
#if GTK_CHECK_VERSION(3, 0, 0)
        cx = get_style_context()->get_padding(get_state_flags()).get_left();
#else
        cx = get_style()->get_xthickness();
#endif
        cw = allocation.get_width() - 2 * cx;
        ColorScales::setScaled(_adjustment->gobj(), CLAMP((gfloat)(event->x - cx) / cw, 0.0, 1.0));
        signal_dragged.emit();
    }

    return false;
}

#if GTK_CHECK_VERSION(3, 0, 0)
void ColorSlider::setAdjustment(Glib::RefPtr<Gtk::Adjustment> adjustment)
{
#else
void ColorSlider::setAdjustment(Gtk::Adjustment *adjustment)
{
#endif
    if (!adjustment) {
#if GTK_CHECK_VERSION(3, 0, 0)
        _adjustment = Gtk::Adjustment::create(0.0, 0.0, 1.0, 0.01, 0.0, 0.0);
#else
        _adjustment = Gtk::manage(new Gtk::Adjustment(0.0, 0.0, 1.0, 0.01, 0.0, 0.0));
#endif
    }
    else {
        adjustment->set_page_increment(0.0);
        adjustment->set_page_size(0.0);
    }

    if (_adjustment != adjustment) {
        if (_adjustment) {
            _adjustment_changed_connection.disconnect();
            _adjustment_value_changed_connection.disconnect();
#if !GTK_CHECK_VERSION(3, 0, 0)
            _adjustment->unreference();
#endif
        }

        _adjustment = adjustment;
        _adjustment_changed_connection =
            _adjustment->signal_changed().connect(sigc::mem_fun(this, &ColorSlider::_onAdjustmentChanged));
        _adjustment_value_changed_connection =
            _adjustment->signal_value_changed().connect(sigc::mem_fun(this, &ColorSlider::_onAdjustmentValueChanged));

        _value = ColorScales::getScaled(_adjustment->gobj());

        _onAdjustmentChanged();
    }
}

void ColorSlider::_onAdjustmentChanged() { queue_draw(); }

void ColorSlider::_onAdjustmentValueChanged()
{
    if (_value != ColorScales::getScaled(_adjustment->gobj())) {
        gint cx, cy, cw, ch;
#if GTK_CHECK_VERSION(3, 0, 0)
        Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
        Gtk::Allocation allocation = get_allocation();
        Gtk::Border padding = style_context->get_padding(get_state_flags());
        cx = padding.get_left();
        cy = padding.get_top();
#else
        Glib::RefPtr<Gtk::Style> style = get_style();
        Gtk::Allocation allocation = get_allocation();
        cx = style->get_xthickness();
        cy = style->get_ythickness();
#endif
        cw = allocation.get_width() - 2 * cx;
        ch = allocation.get_height() - 2 * cy;
        if ((gint)(ColorScales::getScaled(_adjustment->gobj()) * cw) != (gint)(_value * cw)) {
            gint ax, ay;
            gfloat value;
            value = _value;
            _value = ColorScales::getScaled(_adjustment->gobj());
            ax = (int)(cx + value * cw - ARROW_SIZE / 2 - 2);
            ay = cy;
            queue_draw_area(ax, ay, ARROW_SIZE + 4, ch);
            ax = (int)(cx + _value * cw - ARROW_SIZE / 2 - 2);
            ay = cy;
            queue_draw_area(ax, ay, ARROW_SIZE + 4, ch);
        }
        else {
            _value = ColorScales::getScaled(_adjustment->gobj());
        }
    }
}

void ColorSlider::setColors(guint32 start, guint32 mid, guint32 end)
{
    // Remove any map, if set
    _map = 0;

    _c0[0] = start >> 24;
    _c0[1] = (start >> 16) & 0xff;
    _c0[2] = (start >> 8) & 0xff;
    _c0[3] = start & 0xff;

    _cm[0] = mid >> 24;
    _cm[1] = (mid >> 16) & 0xff;
    _cm[2] = (mid >> 8) & 0xff;
    _cm[3] = mid & 0xff;

    _c1[0] = end >> 24;
    _c1[1] = (end >> 16) & 0xff;
    _c1[2] = (end >> 8) & 0xff;
    _c1[3] = end & 0xff;

    queue_draw();
}

void ColorSlider::setMap(const guchar *map)
{
    _map = const_cast<guchar *>(map);

    queue_draw();
}

void ColorSlider::setBackground(guint dark, guint light, guint size)
{
    _b0 = dark;
    _b1 = light;
    _bmask = size;

    queue_draw();
}

bool ColorSlider::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    gboolean colorsOnTop = Inkscape::Preferences::get()->getBool("/options/workarounds/colorsontop", false);

    Gtk::Allocation allocation = get_allocation();

#if GTK_CHECK_VERSION(3, 0, 0)
    Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
#else
    Glib::RefPtr<Gdk::Window> window = get_window();
    Glib::RefPtr<Gtk::Style> style = get_style();
#endif

    // Draw shadow
    if (colorsOnTop) {
#if GTK_CHECK_VERSION(3, 0, 0)
        style_context->render_frame(cr, 0, 0, allocation.get_width(), allocation.get_height());
#else
        gtk_paint_shadow(style->gobj(), window->gobj(), gtk_widget_get_state(gobj()), GTK_SHADOW_IN, NULL, gobj(),
                         "colorslider", 0, 0, allocation.get_width(), allocation.get_height());
#endif
    }

    /* Paintable part of color gradient area */
    Gdk::Rectangle carea;

#if GTK_CHECK_VERSION(3, 0, 0)
    Gtk::Border padding;

    padding = style_context->get_padding(get_state_flags());

    carea.set_x(padding.get_left());
    carea.set_y(padding.get_top());
    ;
#else
    carea.set_x(style->get_xthickness());
    carea.set_y(style->get_ythickness());
#endif

    carea.set_width(allocation.get_width() - 2 * carea.get_x());
    carea.set_height(allocation.get_height() - 2 * carea.get_y());

    if (_map) {
        /* Render map pixelstore */
        gint d = (1024 << 16) / carea.get_width();
        gint s = 0;

        const guchar *b =
            sp_color_slider_render_map(0, 0, carea.get_width(), carea.get_height(), _map, s, d, _b0, _b1, _bmask);

        if (b != NULL && carea.get_width() > 0) {
            Glib::RefPtr<Gdk::Pixbuf> pb = Gdk::Pixbuf::create_from_data(
                b, Gdk::COLORSPACE_RGB, false, 8, carea.get_width(), carea.get_height(), carea.get_width() * 3);

            Gdk::Cairo::set_source_pixbuf(cr, pb, carea.get_x(), carea.get_y());
            cr->paint();
        }
    }
    else {
        gint c[4], dc[4];

        /* Render gradient */

        // part 1: from c0 to cm
        if (carea.get_width() > 0) {
            for (gint i = 0; i < 4; i++) {
                c[i] = _c0[i] << 16;
                dc[i] = ((_cm[i] << 16) - c[i]) / (carea.get_width() / 2);
            }
            guint wi = carea.get_width() / 2;
            const guchar *b = sp_color_slider_render_gradient(0, 0, wi, carea.get_height(), c, dc, _b0, _b1, _bmask);

            /* Draw pixelstore 1 */
            if (b != NULL && wi > 0) {
                Glib::RefPtr<Gdk::Pixbuf> pb =
                    Gdk::Pixbuf::create_from_data(b, Gdk::COLORSPACE_RGB, false, 8, wi, carea.get_height(), wi * 3);

                Gdk::Cairo::set_source_pixbuf(cr, pb, carea.get_x(), carea.get_y());
                cr->paint();
            }
        }

        // part 2: from cm to c1
        if (carea.get_width() > 0) {
            for (gint i = 0; i < 4; i++) {
                c[i] = _cm[i] << 16;
                dc[i] = ((_c1[i] << 16) - c[i]) / (carea.get_width() / 2);
            }
            guint wi = carea.get_width() / 2;
            const guchar *b = sp_color_slider_render_gradient(carea.get_width() / 2, 0, wi, carea.get_height(), c, dc,
                                                              _b0, _b1, _bmask);

            /* Draw pixelstore 2 */
            if (b != NULL && wi > 0) {
                Glib::RefPtr<Gdk::Pixbuf> pb =
                    Gdk::Pixbuf::create_from_data(b, Gdk::COLORSPACE_RGB, false, 8, wi, carea.get_height(), wi * 3);

                Gdk::Cairo::set_source_pixbuf(cr, pb, carea.get_width() / 2 + carea.get_x(), carea.get_y());
                cr->paint();
            }
        }
    }

    /* Draw shadow */
    if (!colorsOnTop) {
#if GTK_CHECK_VERSION(3, 0, 0)
        style_context->render_frame(cr, 0, 0, allocation.get_width(), allocation.get_height());
#else
        gtk_paint_shadow(style->gobj(), window->gobj(), gtk_widget_get_state(gobj()), GTK_SHADOW_IN, NULL, gobj(),
                         "colorslider", 0, 0, allocation.get_width(), allocation.get_height());
#endif
    }

    /* Draw arrow */
    gint x = (int)(_value * (carea.get_width() - 1) - ARROW_SIZE / 2 + carea.get_x());
    gint y1 = carea.get_y();
    gint y2 = carea.get_y() + carea.get_height() - 1;
    cr->set_line_width(1.0);

    // Define top arrow
    cr->move_to(x - 0.5, y1 + 0.5);
    cr->line_to(x + ARROW_SIZE - 0.5, y1 + 0.5);
    cr->line_to(x + (ARROW_SIZE - 1) / 2.0, y1 + ARROW_SIZE / 2.0 + 0.5);
    cr->line_to(x - 0.5, y1 + 0.5);

    // Define bottom arrow
    cr->move_to(x - 0.5, y2 + 0.5);
    cr->line_to(x + ARROW_SIZE - 0.5, y2 + 0.5);
    cr->line_to(x + (ARROW_SIZE - 1) / 2.0, y2 - ARROW_SIZE / 2.0 + 0.5);
    cr->line_to(x - 0.5, y2 + 0.5);

    // Render both arrows
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->stroke_preserve();
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->fill();

    return false;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* Colors are << 16 */

static const guchar *sp_color_slider_render_gradient(gint x0, gint y0, gint width, gint height, gint c[], gint dc[],
                                                     guint b0, guint b1, guint mask)
{
    static guchar *buf = NULL;
    static gint bs = 0;
    guchar *dp;
    gint x, y;
    guint r, g, b, a;

    if (buf && (bs < width * height)) {
        g_free(buf);
        buf = NULL;
    }
    if (!buf) {
        buf = g_new(guchar, width * height * 3);
        bs = width * height;
    }

    dp = buf;
    r = c[0];
    g = c[1];
    b = c[2];
    a = c[3];
    for (x = x0; x < x0 + width; x++) {
        gint cr, cg, cb, ca;
        guchar *d;
        cr = r >> 16;
        cg = g >> 16;
        cb = b >> 16;
        ca = a >> 16;
        d = dp;
        for (y = y0; y < y0 + height; y++) {
            guint bg, fc;
            /* Background value */
            bg = ((x & mask) ^ (y & mask)) ? b0 : b1;
            fc = (cr - bg) * ca;
            d[0] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
            fc = (cg - bg) * ca;
            d[1] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
            fc = (cb - bg) * ca;
            d[2] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
            d += 3 * width;
        }
        r += dc[0];
        g += dc[1];
        b += dc[2];
        a += dc[3];
        dp += 3;
    }

    return buf;
}

/* Positions are << 16 */

static const guchar *sp_color_slider_render_map(gint x0, gint y0, gint width, gint height, guchar *map, gint start,
                                                gint step, guint b0, guint b1, guint mask)
{
    static guchar *buf = NULL;
    static gint bs = 0;
    guchar *dp;
    gint x, y;

    if (buf && (bs < width * height)) {
        g_free(buf);
        buf = NULL;
    }
    if (!buf) {
        buf = g_new(guchar, width * height * 3);
        bs = width * height;
    }

    dp = buf;
    for (x = x0; x < x0 + width; x++) {
        gint cr, cg, cb, ca;
        guchar *d = dp;
        guchar *sp = map + 4 * (start >> 16);
        cr = *sp++;
        cg = *sp++;
        cb = *sp++;
        ca = *sp++;
        for (y = y0; y < y0 + height; y++) {
            guint bg, fc;
            /* Background value */
            bg = ((x & mask) ^ (y & mask)) ? b0 : b1;
            fc = (cr - bg) * ca;
            d[0] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
            fc = (cg - bg) * ca;
            d[1] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
            fc = (cb - bg) * ca;
            d[2] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
            d += 3 * width;
        }
        dp += 3;
        start += step;
    }

    return buf;
}

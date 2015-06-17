#ifndef SEEN_SP_COLOR_SCALES_H
#define SEEN_SP_COLOR_SCALES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if WITH_GTKMM_3_0
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif

#include "ui/selected-color.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorSlider;

typedef enum {
    SP_COLOR_SCALES_MODE_NONE = 0,
    SP_COLOR_SCALES_MODE_RGB = 1,
    SP_COLOR_SCALES_MODE_HSV = 2,
    SP_COLOR_SCALES_MODE_CMYK = 3
} SPColorScalesMode;

class ColorScales
#if GTK_CHECK_VERSION(3, 0, 0)
    : public Gtk::Grid
#else
    : public Gtk::Table
#endif
{
public:
    static const gchar *SUBMODE_NAMES[];

    static gfloat getScaled(const GtkAdjustment *a);
    static void setScaled(GtkAdjustment *a, gfloat v);

    ColorScales(SelectedColor &color, SPColorScalesMode mode);
    virtual ~ColorScales();

    virtual void _initUI(SPColorScalesMode mode);

    void setMode(SPColorScalesMode mode);
    SPColorScalesMode getMode() const;

protected:
    void _onColorChanged();
    void on_show();

    static void _adjustmentAnyChanged(GtkAdjustment *adjustment, ColorScales *cs);
    void _sliderAnyGrabbed();
    void _sliderAnyReleased();
    void _sliderAnyChanged();
    static void _adjustmentChanged(ColorScales *cs, guint channel);

    void _getRgbaFloatv(gfloat *rgba);
    void _getCmykaFloatv(gfloat *cmyka);
    guint32 _getRgba32();
    void _updateSliders(guint channels);
    void _recalcColor();
    void _updateDisplay();

    void _setRangeLimit(gdouble upper);

    SelectedColor &_color;
    SPColorScalesMode _mode;
    gdouble _rangeLimit;
    gboolean _updating : 1;
    gboolean _dragging : 1;
    GtkAdjustment *_a[5];                     /* Channel adjustments */
    Inkscape::UI::Widget::ColorSlider *_s[5]; /* Channel sliders */
    GtkWidget *_b[5];                         /* Spinbuttons */
    GtkWidget *_l[5];                         /* Labels */

private:
    // By default, disallow copy constructor and assignment operator
    ColorScales(ColorScales const &obj);
    ColorScales &operator=(ColorScales const &obj);
};

class ColorScalesFactory : public Inkscape::UI::ColorSelectorFactory
{
public:
    ColorScalesFactory(SPColorScalesMode submode);
    ~ColorScalesFactory();

    Gtk::Widget *createWidget(Inkscape::UI::SelectedColor &color) const;
    Glib::ustring modeName() const;

private:
    SPColorScalesMode _submode;
};

}
}
}

#endif /* !SEEN_SP_COLOR_SCALES_H */
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

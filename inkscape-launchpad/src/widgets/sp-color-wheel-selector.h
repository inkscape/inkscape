#ifndef SEEN_SP_COLOR_WHEEL_SELECTOR_H
#define SEEN_SP_COLOR_WHEEL_SELECTOR_H

#include <glib.h>
#include <gtk/gtk.h>

#include "sp-color-slider.h"
#include "sp-color-selector.h"



typedef struct _GimpColorWheel GimpColorWheel;
struct SPColorWheelSelector;
struct SPColorWheelSelectorClass;

class ColorWheelSelector: public ColorSelector
{
public:
    ColorWheelSelector( SPColorSelector* csel );
    virtual ~ColorWheelSelector();

    virtual void init();

protected:
    virtual void _colorChanged();

    static void _adjustmentChanged ( GtkAdjustment *adjustment, SPColorWheelSelector *cs );

    static void _sliderGrabbed( SPColorSlider *slider, SPColorWheelSelector *cs );
    static void _sliderReleased( SPColorSlider *slider, SPColorWheelSelector *cs );
    static void _sliderChanged( SPColorSlider *slider, SPColorWheelSelector *cs );
    static void _wheelChanged( GimpColorWheel *wheel, SPColorWheelSelector *cs );

    static void _fooChanged( GtkWidget foo, SPColorWheelSelector *cs );

    void _recalcColor( gboolean changing );

    gboolean _updating : 1;
    gboolean _dragging : 1;
    GtkAdjustment* _adj; // Channel adjustment
    GtkWidget* _wheel;
    GtkWidget* _slider;
    GtkWidget* _sbtn; // Spinbutton
    GtkWidget* _label; // Label

private:
    // By default, disallow copy constructor and assignment operator
    ColorWheelSelector( const ColorWheelSelector& obj );
    ColorWheelSelector& operator=( const ColorWheelSelector& obj );
};



#define SP_TYPE_COLOR_WHEEL_SELECTOR (sp_color_wheel_selector_get_type ())
#define SP_COLOR_WHEEL_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_COLOR_WHEEL_SELECTOR, SPColorWheelSelector))
#define SP_COLOR_WHEEL_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_WHEEL_SELECTOR, SPColorWheelSelectorClass))
#define SP_IS_COLOR_WHEEL_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_COLOR_WHEEL_SELECTOR))
#define SP_IS_COLOR_WHEEL_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_WHEEL_SELECTOR))

struct SPColorWheelSelector {
    SPColorSelector parent;
};

struct SPColorWheelSelectorClass {
    SPColorSelectorClass parent_class;
};

GType sp_color_wheel_selector_get_type (void);

GtkWidget *sp_color_wheel_selector_new (void);



#endif // SEEN_SP_COLOR_WHEEL_SELECTOR_H

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

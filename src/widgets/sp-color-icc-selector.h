#ifndef SEEN_SP_COLOR_ICC_SELECTOR_H
#define SEEN_SP_COLOR_ICC_SELECTOR_H

#include <glib/gtypes.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkvbox.h>

#include "../color.h"
#include "sp-color-slider.h"
#include "sp-color-selector.h"

#if ENABLE_LCMS
#include "color-profile.h"
#endif // ENABLE_LCMS



struct SPColorICCSelector;
struct SPColorICCSelectorClass;


class ColorICCSelector: public ColorSelector
{
public:
    ColorICCSelector( SPColorSelector* csel );
    virtual ~ColorICCSelector();

    virtual void init();

protected:
    virtual void _colorChanged();

    static void _adjustmentChanged ( GtkAdjustment *adjustment, SPColorICCSelector *cs );

    static void _sliderGrabbed( SPColorSlider *slider, SPColorICCSelector *cs );
    static void _sliderReleased( SPColorSlider *slider, SPColorICCSelector *cs );
    static void _sliderChanged( SPColorSlider *slider, SPColorICCSelector *cs );

    static void _fixupHit( GtkWidget* src, gpointer data );
    static void _profileSelected( GtkWidget* src, gpointer data );

    void _recalcColor( gboolean changing );
#if ENABLE_LCMS
    void _setProfile( SVGICCColor* profile );
    void _switchToProfile( gchar const* name );
#endif // ENABLE_LCMS
    void _updateSliders( gint ignore );
    void _profilesChanged( std::string const & name );

    gboolean _updating : 1;
    gboolean _dragging : 1;

    guint32 _fixupNeeded;
    GtkWidget* _fixupBtn;
    GtkWidget* _profileSel;

    guint _fooCount;
    guint const* _fooScales;
    GtkAdjustment** _fooAdj;
    GtkWidget** _fooSlider;
    GtkWidget** _fooBtn;
    GtkWidget** _fooLabel;
    guchar** _fooMap;

    GtkAdjustment* _adj; /* Channel adjustment */
    GtkWidget* _slider;
    GtkWidget* _sbtn; /* Spinbutton */
    GtkWidget* _label; /* Label */

    GtkTooltips* _tt; /* tooltip container */

#if ENABLE_LCMS
    std::string _profileName;
    Inkscape::ColorProfile* _prof;
    guint _profChannelCount;
    gulong _profChangedID;
#endif // ENABLE_LCMS

private:
    // By default, disallow copy constructor and assignment operator
    ColorICCSelector( const ColorICCSelector& obj );
    ColorICCSelector& operator=( const ColorICCSelector& obj );
};



#define SP_TYPE_COLOR_ICC_SELECTOR (sp_color_icc_selector_get_type ())
#define SP_COLOR_ICC_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_ICC_SELECTOR, SPColorICCSelector))
#define SP_COLOR_ICC_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_ICC_SELECTOR, SPColorICCSelectorClass))
#define SP_IS_COLOR_ICC_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_ICC_SELECTOR))
#define SP_IS_COLOR_ICC_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_ICC_SELECTOR))

struct SPColorICCSelector {
    SPColorSelector parent;
};

struct SPColorICCSelectorClass {
    SPColorSelectorClass parent_class;
};

GType sp_color_icc_selector_get_type (void);

GtkWidget *sp_color_icc_selector_new (void);



#endif // SEEN_SP_COLOR_ICC_SELECTOR_H

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

#ifndef SEEN_SP_COLOR_ICC_SELECTOR_H
#define SEEN_SP_COLOR_ICC_SELECTOR_H

#include <glib.h>
#include "sp-color-selector.h"

namespace Inkscape {
class ColorProfile;
}

struct SPColorICCSelector;
struct SPColorICCSelectorClass;

class ColorICCSelectorImpl;

class ColorICCSelector: public ColorSelector
{
public:
    ColorICCSelector( SPColorSelector* csel );
    virtual ~ColorICCSelector();

    virtual void init();

protected:
    virtual void _colorChanged();

    void _recalcColor( gboolean changing );

private:
    friend class ColorICCSelectorImpl;

    // By default, disallow copy constructor and assignment operator
    ColorICCSelector( const ColorICCSelector& obj );
    ColorICCSelector& operator=( const ColorICCSelector& obj );

    ColorICCSelectorImpl *_impl;
};



#define SP_TYPE_COLOR_ICC_SELECTOR (sp_color_icc_selector_get_type())
#define SP_COLOR_ICC_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_COLOR_ICC_SELECTOR, SPColorICCSelector))
#define SP_COLOR_ICC_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_COLOR_ICC_SELECTOR, SPColorICCSelectorClass))
#define SP_IS_COLOR_ICC_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_COLOR_ICC_SELECTOR))
#define SP_IS_COLOR_ICC_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_COLOR_ICC_SELECTOR))

struct SPColorICCSelector {
    SPColorSelector parent;
};

struct SPColorICCSelectorClass {
    SPColorSelectorClass parent_class;
};

GType sp_color_icc_selector_get_type(void);

GtkWidget *sp_color_icc_selector_new(void);



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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

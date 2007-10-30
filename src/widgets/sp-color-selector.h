#ifndef SEEN_SP_COLOR_SELECTOR_H
#define SEEN_SP_COLOR_SELECTOR_H

#include <gtk/gtkvbox.h>
#include "../color.h"

#include <glib.h>




struct SPColorSelector;

class ColorSelector
{
public:
    ColorSelector( SPColorSelector* csel );
    virtual ~ColorSelector();

    virtual void init();

    void setColor( const SPColor& color );
    SPColor getColor() const;

    void setAlpha( gfloat alpha );
    gfloat getAlpha() const;

    void setColorAlpha( const SPColor& color, gfloat alpha, bool emit = false );
    void getColorAlpha( SPColor& color, gfloat* alpha ) const;

    virtual void setSubmode( guint submode );
    virtual guint getSubmode() const;

protected:
    void _grabbed();
    void _released();
    void _updateInternals( const SPColor& color, gfloat alpha, gboolean held );
    gboolean _isHeld() const { return _held; }

    virtual void _colorChanged();

    static double _epsilon;

    SPColorSelector* _csel;
    SPColor _color;
    gfloat _alpha;      // guaranteed to be in [0, 1].

private:
    // By default, disallow copy constructor and assignment operator
    ColorSelector( const ColorSelector& obj );
    ColorSelector& operator=( const ColorSelector& obj );

    gboolean _held;

    bool virgin; // if true, no color is set yet
};



#define SP_TYPE_COLOR_SELECTOR (sp_color_selector_get_type ())
#define SP_COLOR_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_SELECTOR, SPColorSelector))
#define SP_COLOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_SELECTOR, SPColorSelectorClass))
#define SP_IS_COLOR_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_SELECTOR))
#define SP_IS_COLOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_SELECTOR))
#define SP_COLOR_SELECTOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SP_TYPE_COLOR_SELECTOR, SPColorSelectorClass))

struct SPColorSelector {
    GtkVBox vbox;

    ColorSelector* base;
};

struct SPColorSelectorClass {
    GtkVBoxClass parent_class;

    const gchar **name;
    guint submode_count;

    void (* grabbed) (SPColorSelector *rgbsel);
    void (* dragged) (SPColorSelector *rgbsel);
    void (* released) (SPColorSelector *rgbsel);
    void (* changed) (SPColorSelector *rgbsel);
};

GType sp_color_selector_get_type(void);

GtkWidget *sp_color_selector_new( GType selector_type );



#endif // SEEN_SP_COLOR_SELECTOR_H

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

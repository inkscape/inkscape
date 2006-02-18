
#ifndef SEEN_SWATCHES_H
#define SEEN_SWATCHES_H
/*
 * A simple dialog for previewing icon representation.
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/textview.h>
#include <gtkmm/tooltips.h>

#include "ui/widget/panel.h"
#include "ui/previewholder.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {



/**
 * The color swatch you see on screen as a clickable box.
 */
class ColorItem : public Inkscape::UI::Previewable
{
public:
    ColorItem( unsigned int r, unsigned int g, unsigned int b,
               Glib::ustring& name );
    virtual ~ColorItem();
    ColorItem(ColorItem const &other);
    virtual ColorItem &operator=(ColorItem const &other);
    virtual Gtk::Widget* getPreview(PreviewStyle style,
                                    ViewType view,
                                    Gtk::BuiltinIconSize size);
    void buttonClicked(bool secondary = false);
    unsigned int _r;
    unsigned int _g;
    unsigned int _b;
    Glib::ustring _name;
    
private:
    Gtk::Tooltips tips;
};

	

/**
 * A panel that displays color swatches.
 */
class SwatchesPanel : public Inkscape::UI::Widget::Panel
{
public:
    SwatchesPanel();
    virtual ~SwatchesPanel();

    static SwatchesPanel& getInstance();

    void Temp();

protected:
    virtual void _handleAction( int setId, int itemId );

private:
    SwatchesPanel(SwatchesPanel const &); // no copy
    SwatchesPanel &operator=(SwatchesPanel const &); // no assign

    static SwatchesPanel* instance;

    PreviewHolder* _holder;
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_SWATCHES_H

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

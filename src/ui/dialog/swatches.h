/** @file
 * @brief Color swatches dialog
 */
/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_DIALOGS_SWATCHES_H
#define SEEN_DIALOGS_SWATCHES_H

#include <gtkmm/textview.h>
#include <gtkmm/tooltips.h>

#include "ui/widget/panel.h"
#include "ui/previewholder.h"
#include "widgets/ege-paint-def.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

class ColorItem;

/**
 * A panel that displays color swatches.
 */
class SwatchesPanel : public Inkscape::UI::Widget::Panel
{
public:
    SwatchesPanel(gchar const* prefsPath = "/dialogs/swatches");
    virtual ~SwatchesPanel();

    static SwatchesPanel& getInstance();

    virtual void setOrientation( Gtk::AnchorType how );

    virtual void setDesktop( SPDesktop* desktop );
    virtual SPDesktop* getDesktop() {return _currentDesktop;}

    virtual int getSelectedIndex() {return _currentIndex;} // temporary
    virtual void handleGradientsChange(); // temporary

protected:
    virtual void _handleAction( int setId, int itemId );
    virtual void _setDocument( SPDocument *document );
    virtual void _rebuild();

private:
    SwatchesPanel(SwatchesPanel const &); // no copy
    SwatchesPanel &operator=(SwatchesPanel const &); // no assign

    PreviewHolder* _holder;
    ColorItem* _clear;
    ColorItem* _remove;
    int _currentIndex;
    SPDesktop*  _currentDesktop;
    SPDocument* _currentDocument;
    void* _ptr;

    sigc::connection _documentConnection;
    sigc::connection _resourceConnection;
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

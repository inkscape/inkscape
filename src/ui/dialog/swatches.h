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

#include "ui/widget/panel.h"
#include "enums.h"

namespace Inkscape {
namespace UI {

class PreviewHolder;

namespace Dialogs {

class ColorItem;
class SwatchPage;
class DocTrack;

/**
 * A panel that displays paint swatches.
 */
class SwatchesPanel : public Inkscape::UI::Widget::Panel
{
public:
    SwatchesPanel(gchar const* prefsPath = "/dialogs/swatches");
    virtual ~SwatchesPanel();

    static SwatchesPanel& getInstance();

    virtual void setOrientation(SPAnchorType how);

    virtual void setDesktop( SPDesktop* desktop );
    virtual SPDesktop* getDesktop() {return _currentDesktop;}

    virtual int getSelectedIndex() {return _currentIndex;} // temporary

protected:
    static void handleGradientsChange(SPDocument *document);

    virtual void _updateFromSelection();
    virtual void _handleAction( int setId, int itemId );
    virtual void _setDocument( SPDocument *document );
    virtual void _rebuild();

    virtual std::vector<SwatchPage*> _getSwatchSets() const;

private:
    SwatchesPanel(SwatchesPanel const &); // no copy
    SwatchesPanel &operator=(SwatchesPanel const &); // no assign

    static void _trackDocument( SwatchesPanel *panel, SPDocument *document );
    static void handleDefsModified(SPDocument *document);

    PreviewHolder* _holder;
    ColorItem* _clear;
    ColorItem* _remove;
    int _currentIndex;
    SPDesktop*  _currentDesktop;
    SPDocument* _currentDocument;

    sigc::connection _documentConnection;
    sigc::connection _selChanged;

    friend class DocTrack;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

/** @file
 * @brief A simple dialog for previewing icon representation.
 */
/* Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004,2005 The Inkscape Organization
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_ICON_PREVIEW_H
#define SEEN_ICON_PREVIEW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/image.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/toggletoolbutton.h>

#include "ui/widget/panel.h"
#include "desktop-tracker.h"

class SPObject;
namespace Glib {
class Timer;
}

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * A panel that displays an icon preview
 */
class IconPreviewPanel : public UI::Widget::Panel
{
public:
    IconPreviewPanel();
    //IconPreviewPanel(Glib::ustring const &label);
    ~IconPreviewPanel();

    static IconPreviewPanel& getInstance();

    void setDesktop( SPDesktop* desktop );
    void refreshPreview();
    void modeToggled();

private:
    IconPreviewPanel(IconPreviewPanel const &); // no copy
    IconPreviewPanel &operator=(IconPreviewPanel const &); // no assign


    DesktopTracker deskTrack;
    SPDesktop *desktop;
    SPDocument *document;
    Glib::Timer *timer;
    Glib::Timer *renderTimer;
    bool pending;
    gdouble minDelay;

    Gtk::VBox       iconBox;

#if WITH_GTKMM_3_0
    Gtk::Paned      splitter;
#else
    Gtk::HPaned     splitter;
#endif

    Glib::ustring targetId;
    int hot;
    int numEntries;
    int* sizes;

    Gtk::Image      magnified;
    Gtk::Label      magLabel;

    Gtk::ToggleButton     *selectionButton;

    guchar** pixMem;
    Gtk::Image** images;
    Glib::ustring** labels;
    Gtk::ToggleToolButton** buttons;
    sigc::connection desktopChangeConn;
    sigc::connection docReplacedConn;
    sigc::connection docModConn;
    sigc::connection selChangedConn;


    void setDocument( SPDocument *document );
    void on_button_clicked(int which);
    void renderPreview( SPObject* obj );
    void updateMagnify();
    void queueRefresh();
    bool refreshCB();
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_ICON_PREVIEW_H

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

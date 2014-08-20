/** @file
 * @brief Bitmap tracing settings dialog
 */
/* Authors:
 *   Bob Jamison
 *   Vinícius dos Santos Oliveira <vini.ipsmaker@gmail.com>
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __PIXELARTDIALOG_H__
#define __PIXELARTDIALOG_H__

#include "ui/widget/panel.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * A dialog that displays log messages
 */
class PixelArtDialog : public UI::Widget::Panel
{

public:

    PixelArtDialog() : 
        UI::Widget::Panel("", "/dialogs/pixelart", SP_VERB_SELECTION_PIXEL_ART)
    {}


    static PixelArtDialog &getInstance();

    virtual ~PixelArtDialog() {};
};


} //namespace Dialog
} //namespace UI
} //namespace Inkscape

#endif /* __PIXELARTDIALOGDIALOG_H__ */

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

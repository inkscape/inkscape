/** @file
 * @brief Memory statistics dialog
 */
/* Authors:
 *     MenTaLguY <mental@rydia.net>
 * 
 * Copyright 2005 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_UI_DIALOG_MEMORY_H
#define SEEN_INKSCAPE_UI_DIALOG_MEMORY_H

#include "ui/widget/panel.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class Memory : public UI::Widget::Panel {
public:
    Memory();
    ~Memory();

    static Memory &getInstance() { return *new Memory(); }

protected:
    void _apply();

private:
    Memory(Memory const &d); // no copy
    void operator=(Memory const &d); // no assign

    struct Private;
    Private &_private;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif

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

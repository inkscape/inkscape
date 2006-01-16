/**
 * \brief Layer Editor dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_LAYER_EDITOR_H
#define INKSCAPE_UI_DIALOG_LAYER_EDITOR_H

#include "dialog.h"

#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

class LayerEditor : public Dialog {
public:
    LayerEditor();
    virtual ~LayerEditor();

    static LayerEditor *create() { return new LayerEditor(); }

protected:

private:
    LayerEditor(LayerEditor const &d);
    LayerEditor& operator=(LayerEditor const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_LAYER_EDITOR_H

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

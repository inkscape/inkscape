/**
 * \brief XML Editor dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_DIALOG_XML_EDITOR_H
#define INKSCAPE_DIALOG_XML_EDITOR_H

#include "ui/widget/panel.h"

#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

class XmlEditor : public UI::Widget::Panel {
public:
    XmlEditor();
    virtual ~XmlEditor();

    static XmlEditor &getInstance() { return *new XmlEditor(); }

protected:

private:
    XmlEditor(XmlEditor const &d);
    XmlEditor& operator=(XmlEditor const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_XML_EDITOR_H

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

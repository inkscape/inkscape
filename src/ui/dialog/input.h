
#ifndef INKSCAPE_UI_DIALOG_INPUT_H
#define INKSCAPE_UI_DIALOG_INPUT_H


#include "verbs.h"
#include "ui/widget/panel.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


class InputDialog : public UI::Widget::Panel
{
public:
    static InputDialog &getInstance();

    InputDialog() : UI::Widget::Panel("", "dialogs.inputdevices2", SP_VERB_DIALOG_INPUT2) {}
    virtual ~InputDialog() {}
};

} // namespace Dialog
} // namesapce UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_INPUT_H

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

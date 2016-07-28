/*
 * ActionContext implementation.
 *
 * Author:
 *   Eric Greveson <eric@greveson.co.uk>
 *
 * Copyright (C) 2013 Eric Greveson
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "desktop.h"
#include "document.h"
#include "layer-model.h"
#include "selection.h"
#include "helper/action-context.h"
#include "ui/view/view.h"

namespace Inkscape {

ActionContext::ActionContext()
  : _selection(NULL)
  , _view(NULL)
{
}

ActionContext::ActionContext(Selection *selection)
  : _selection(selection)
  , _view(NULL)
{
}

ActionContext::ActionContext(UI::View::View *view)
  : _selection(NULL)
  , _view(view)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(view);
    if (desktop) {
        _selection = desktop->selection;
    }
}

SPDocument *ActionContext::getDocument() const
{
    if (_selection == NULL) {
        return NULL;
    }

    // Should be the same as the view's document, if view is non-NULL
    return _selection->layers()->getDocument();
}

Selection *ActionContext::getSelection() const
{
    return _selection;
}

UI::View::View *ActionContext::getView() const
{
    return _view;
}

SPDesktop *ActionContext::getDesktop() const
{
    // TODO: this slightly horrible storage of a UI::View::View*, and 
    // casting to an SPDesktop*, is only done because that's what was
    // already the norm in the Inkscape codebase. This seems wrong. Surely
    // we should store an SPDesktop* in the first place? Is there a case
    // of actions being carried out on a View that is not an SPDesktop?
    return static_cast<SPDesktop *>(_view);
}

} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

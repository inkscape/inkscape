/*
    Author:  Ted Gould <ted@gould.cx>
    Copyright (c) 2003-2005,2007

    This code is licensed under the GNU GPL.  See COPYING for details.

    This file is the backend to the extensions system.  These are
    the parts of the system that most users will never see, but are
    important for implementing the extensions themselves.  This file
    contains the base class for all of that.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "implementation.h"

#include <extension/output.h>
#include <extension/input.h>
#include <extension/effect.h>

#include "selection.h"
#include "desktop.h"

#include "ui/view/view.h"

namespace Inkscape {
namespace Extension {
namespace Implementation {

Gtk::Widget *
Implementation::prefs_input(Inkscape::Extension::Input *module, gchar const */*filename*/) {
    return module->autogui(NULL, NULL);
}

Gtk::Widget *
Implementation::prefs_output(Inkscape::Extension::Output *module) {
    return module->autogui(NULL, NULL);
}

Gtk::Widget *Implementation::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal, ImplementationDocumentCache * /*docCache*/)
{
    if (module->param_visible_count() == 0) {
        return NULL;
    }

    SPDocument * current_document = view->doc();

    std::vector<SPItem*> selected = ((SPDesktop *)view)->getSelection()->itemList();
    Inkscape::XML::Node const* first_select = NULL;
    if (!selected.empty()) {
        const SPItem * item = selected[0];
        first_select = item->getRepr();
    }

    // TODO deal with this broken const correctness:
    return module->autogui(current_document, const_cast<Inkscape::XML::Node *>(first_select), changeSignal);
} // Implementation::prefs_effect

}  /* namespace Implementation */
}  /* namespace Extension */
}  /* namespace Inkscape */

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

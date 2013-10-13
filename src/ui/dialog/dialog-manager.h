/** @file
 * @brief Object for managing a set of dialogs, including their signals and
 *        construction/caching/destruction of them.
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Jon Phillips <jon@rejon.org>
 *   
 * Copyright (C) 2004, 2005 Authors
 * 
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_MANAGER_H
#define INKSCAPE_UI_DIALOG_MANAGER_H

#include "dialog.h"
#include <map>

namespace Inkscape {
namespace UI {
namespace Dialog {

class DialogManager {
public:
    typedef Dialog *(*DialogFactory)();

    DialogManager();
    virtual ~DialogManager();

    static DialogManager &getInstance();

    sigc::signal<void> show_dialogs;
    sigc::signal<void> show_f12;
    sigc::signal<void> hide_dialogs;
    sigc::signal<void> hide_f12;
    sigc::signal<void> transientize;

    /* generic dialog management start */
    typedef std::map<GQuark, DialogFactory> FactoryMap;
    typedef std::map<GQuark, Dialog*> DialogMap;

    void registerFactory(gchar const *name, DialogFactory factory);
    void registerFactory(GQuark name, DialogFactory factory);
    Dialog *getDialog(gchar const* dlgName); 
    Dialog *getDialog(GQuark dlgName); 
    void showDialog(gchar const *name, bool grabfocus=true);
    void showDialog(GQuark name, bool grabfocus=true);

protected:
    DialogManager(DialogManager const &d); // no copy
    DialogManager& operator=(DialogManager const &d); // no assign

    FactoryMap _factory_map; //< factories to create dialogs
    DialogMap _dialog_map; //< map of already created dialogs
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_MANAGER_H

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

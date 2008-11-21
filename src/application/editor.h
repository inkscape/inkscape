/** @file
 * @brief  Singleton class to manage an application used for editing SVG
 *         documents using GUI views
 */
/*
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_APPLICATION_EDITOR_H
#define INKSCAPE_APPLICATION_EDITOR_H

#include <sigc++/sigc++.h>
#include <glib/gslist.h>
#include <glibmm/ustring.h>
#include <set>
#include "app-prototype.h"

class SPDesktop;
class SPDocument;
class SPEventContext;

namespace Inkscape {
    class Selection;
    namespace XML {
        class Document;
    }
    namespace UI {
        namespace View {
            class Edit;
        }
    }
    namespace NSApplication {

class Editor : public AppPrototype
{
public:
    static Editor *create (int argc, char **argv);
    virtual ~Editor();

    void*           getWindow();

    void            toggleDialogs();
    void            nextDesktop();
    void            prevDesktop();

    void            refreshDisplay();
    void            exit();

    bool        lastViewOfDocument(SPDocument* doc, SPDesktop* view) const;

    bool        addView(SPDesktop* view);
    bool        deleteView(SPDesktop* view);

    static Inkscape::XML::Document *getPreferences();
    static SPDesktop* getActiveDesktop();
    static bool isDesktopActive (SPDesktop* dt) { return getActiveDesktop()==dt; }
    static SPDesktop* createDesktop (SPDocument* doc);
    static void addDesktop (SPDesktop* dt);
    static void removeDesktop (SPDesktop* dt);
    static void activateDesktop (SPDesktop* dt);
    static void reactivateDesktop (SPDesktop* dt);
    static bool isDuplicatedView (SPDesktop* dt);

    static SPDocument* getActiveDocument();
    static void addDocument (SPDocument* doc);
    static void removeDocument (SPDocument* doc);

    static void selectionModified (Inkscape::Selection*, guint);
    static void selectionChanged (Inkscape::Selection*);
    static void subSelectionChanged (SPDesktop*);
    static void selectionSet (Inkscape::Selection*);
    static void eventContextSet (SPEventContext*);
    static void hideDialogs();
    static void unhideDialogs();

    static sigc::connection connectSelectionModified (const sigc::slot<void, Inkscape::Selection*, guint> &slot);
    static sigc::connection connectSelectionChanged (const sigc::slot<void, Inkscape::Selection*> &slot);
    static sigc::connection connectSubselectionChanged (const sigc::slot<void, SPDesktop*> &slot);
    static sigc::connection connectSelectionSet (const sigc::slot<void, Inkscape::Selection*> &slot);
    static sigc::connection connectEventContextSet (const sigc::slot<void, SPEventContext*> &slot);
    static sigc::connection connectDesktopActivated (const sigc::slot<void, SPDesktop*> &slot);
    static sigc::connection connectDesktopDeactivated (const sigc::slot<void, SPDesktop*> &slot);
    static sigc::connection connectShutdown (const sigc::slot<void> &slot);
    static sigc::connection connectDialogsHidden (const sigc::slot<void> &slot);
    static sigc::connection connectDialogsUnhidden (const sigc::slot<void> &slot);
    static sigc::connection connectExternalChange (const sigc::slot<void> &slot);


protected:
    Editor(Editor const &);
    Editor& operator=(Editor const &);

    std::multiset<SPDocument *> _document_set;
    GSList         *_documents;
    GSList         *_desktops;
    gchar          *_argv0;

    bool       _dialogs_toggle;

    sigc::signal <void, Inkscape::Selection*, guint> _selection_modified_signal;
    sigc::signal <void, Inkscape::Selection*>        _selection_changed_signal;
    sigc::signal <void, SPDesktop*>                  _subselection_changed_signal;
    sigc::signal <void, Inkscape::Selection*>        _selection_set_signal;
    sigc::signal <void, SPEventContext*>             _event_context_set_signal;
    sigc::signal <void, SPDesktop*>                  _desktop_activated_signal;
    sigc::signal <void, SPDesktop*>                  _desktop_deactivated_signal;
    sigc::signal <void> _shutdown_signal;
    sigc::signal <void> _dialogs_hidden_signal;
    sigc::signal <void> _dialogs_unhidden_signal;
    sigc::signal <void> _external_change_signal;

private:
    Editor(int argc, char **argv);
    bool init();
};

#define ACTIVE_DESKTOP Inkscape::NSApplication::Editor::getActiveDesktop()

} // namespace NSApplication
} // namespace Inkscape


#endif /* !INKSCAPE_APPLICATION_EDITOR_H */

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

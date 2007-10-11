/** \file
 * \brief  Document Metadata dialog
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005, 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_DOCUMENT_METADATA_H
#define INKSCAPE_UI_DIALOG_DOCUMENT_METADATA_H

#include <list>
#include <sigc++/sigc++.h>
#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "ui/widget/licensor.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/registry.h"
#include "dialog.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
    namespace XML {
        class Node;
    }
    namespace UI {
        namespace Widget {
            class EntityEntry;
        }
        namespace Dialog {

typedef std::list<EntityEntry*> RDElist;

class DocumentMetadata : public Inkscape::UI::Dialog::Dialog {
public:
    void  update();
    static DocumentMetadata *create(Behavior::BehaviorFactory behavior_factory);
    static void destroy();
    sigc::connection _doc_replaced_connection;

protected:
    void  build_metadata();
    void  init();
    virtual void  on_response (int);

    Gtk::Tooltips _tt;
    Gtk::Notebook  _notebook;

    NotebookPage   _page_metadata1, _page_metadata2;

    //---------------------------------------------------------------
    RDElist _rdflist;
    Licensor _licensor;

    gchar const *_prefs_path;
    Registry _wr;

private:
    DocumentMetadata(Behavior::BehaviorFactory behavior_factory);
    virtual ~DocumentMetadata();
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCUMENT_METADATA_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

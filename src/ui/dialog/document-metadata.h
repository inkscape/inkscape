/** \file
 * \brief  Document Metadata dialog
 */
/* Authors:
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

#include "ui/widget/panel.h"
#include "ui/widget/licensor.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/registry.h"

namespace Inkscape {
    namespace XML {
        class Node;
    }
    namespace UI {
        namespace Widget {
            class EntityEntry;
        }
        namespace Dialog {

typedef std::list<UI::Widget::EntityEntry*> RDElist;

class DocumentMetadata : public Inkscape::UI::Widget::Panel {
public:
    void  update();

    static DocumentMetadata &getInstance();

    static void destroy();

protected:
    void  build_metadata();
    void  init();

    void _handleDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    void _handleActivateDesktop(Inkscape::Application *application, SPDesktop *desktop);
    void _handleDeactivateDesktop(Inkscape::Application *application, SPDesktop *desktop);

    Gtk::Tooltips _tt;
    Gtk::Notebook  _notebook;

    UI::Widget::NotebookPage _page_metadata1;
    UI::Widget::NotebookPage _page_metadata2;

    //---------------------------------------------------------------
    RDElist _rdflist;
    UI::Widget::Licensor _licensor;

    UI::Widget::Registry _wr;

private:
    virtual ~DocumentMetadata();
    DocumentMetadata();
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCUMENT_METADATA_H

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

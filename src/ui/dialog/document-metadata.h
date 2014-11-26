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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <list>
#include <stddef.h>
#include "ui/widget/panel.h"
#include <gtkmm/notebook.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include "inkscape.h"
#include "ui/widget/licensor.h"
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
    void _handleActivateDesktop(SPDesktop *desktop);
    void _handleDeactivateDesktop(SPDesktop *desktop);

    Gtk::Notebook  _notebook;

#if WITH_GTKMM_3_0
    Gtk::Grid     _page_metadata1;
    Gtk::Grid     _page_metadata2;
#else
    Gtk::Table     _page_metadata1;
    Gtk::Table     _page_metadata2;
#endif

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

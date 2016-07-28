/**
 * @file
 * Document metadata dialog, Gtkmm-style.
 */
/* Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "document-metadata.h"
#include "desktop.h"

#include "inkscape.h"
#include "rdf.h"
#include "sp-namedview.h"
#include "ui/widget/entity-entry.h"
#include "verbs.h"
#include "xml/node-event-vector.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 15

//===================================================

//---------------------------------------------------

static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);

static Inkscape::XML::NodeEventVector const _repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


DocumentMetadata &
DocumentMetadata::getInstance()
{
    DocumentMetadata &instance = *new DocumentMetadata();
    instance.init();
    return instance;
}


DocumentMetadata::DocumentMetadata()
#if WITH_GTKMM_3_0
    : UI::Widget::Panel ("", "/dialogs/documentmetadata", SP_VERB_DIALOG_METADATA)
#else
    : UI::Widget::Panel ("", "/dialogs/documentmetadata", SP_VERB_DIALOG_METADATA),
      _page_metadata1(1, 1), _page_metadata2(1, 1)
#endif
{
    hide();
    _getContents()->set_spacing (4);
    _getContents()->pack_start(_notebook, true, true);

    _page_metadata1.set_border_width(2);
    _page_metadata2.set_border_width(2);
   
#if WITH_GTKMM_3_0
    _page_metadata1.set_column_spacing(2);
    _page_metadata2.set_column_spacing(2);
    _page_metadata1.set_row_spacing(2);
    _page_metadata2.set_row_spacing(2);
#else 
    _page_metadata1.set_spacings(2);
    _page_metadata2.set_spacings(2);
#endif
    
    _notebook.append_page(_page_metadata1, _("Metadata"));
    _notebook.append_page(_page_metadata2, _("License"));

    signalDocumentReplaced().connect(sigc::mem_fun(*this, &DocumentMetadata::_handleDocumentReplaced));
    signalActivateDesktop().connect(sigc::mem_fun(*this, &DocumentMetadata::_handleActivateDesktop));
    signalDeactiveDesktop().connect(sigc::mem_fun(*this, &DocumentMetadata::_handleDeactivateDesktop));

    build_metadata();
}

void
DocumentMetadata::init()
{
    update();

    Inkscape::XML::Node *repr = getDesktop()->getNamedView()->getRepr();
    repr->addListener (&_repr_events, this);

    show_all_children();
}

DocumentMetadata::~DocumentMetadata()
{
    Inkscape::XML::Node *repr = getDesktop()->getNamedView()->getRepr();
    repr->removeListenerByData (this);

    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it)
        delete (*it);
}

void
DocumentMetadata::build_metadata()
{
    using Inkscape::UI::Widget::EntityEntry;

    _page_metadata1.show();

    Gtk::Label *label = Gtk::manage (new Gtk::Label);
    label->set_markup (_("<b>Dublin Core Entities</b>"));
    label->set_alignment (0.0);

#if WITH_GTKMM_3_0
    label->set_valign(Gtk::ALIGN_CENTER);
    _page_metadata1.attach(*label, 0, 0, 3, 1);
#else
    _page_metadata1.attach(*label, 0,3,0,1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
#endif

     /* add generic metadata entry areas */
    struct rdf_work_entity_t * entity;
    int row = 1;
    for (entity = rdf_work_entities; entity && entity->name; entity++, row++) {
        if ( entity->editable == RDF_EDIT_GENERIC ) {
            EntityEntry *w = EntityEntry::create (entity, _wr);
            _rdflist.push_back (w);
            Gtk::HBox *space = Gtk::manage (new Gtk::HBox);
            space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
            space->set_valign(Gtk::ALIGN_CENTER);
            _page_metadata1.attach(*space, 0, row, 1, 1);

            w->_label.set_valign(Gtk::ALIGN_CENTER);
            _page_metadata1.attach(w->_label, 1, row, 1, 1);

            w->_packable->set_hexpand();
            w->_packable->set_valign(Gtk::ALIGN_CENTER);
            _page_metadata1.attach(*w->_packable, 2, row, 1, 1);
#else
            _page_metadata1.attach(*space, 0,1, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
            _page_metadata1.attach(w->_label, 1,2, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
            _page_metadata1.attach(*w->_packable, 2,3, row, row+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
        }
    }

    _page_metadata2.show();

    row = 0;
    Gtk::Label *llabel = Gtk::manage (new Gtk::Label);
    llabel->set_markup (_("<b>License</b>"));
    llabel->set_alignment (0.0);

#if WITH_GTKMM_3_0
    llabel->set_valign(Gtk::ALIGN_CENTER);
    _page_metadata2.attach(*llabel, 0, row, 3, 1);
#else
    _page_metadata2.attach(*llabel, 0,3, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
#endif

    /* add license selector pull-down and URI */
    ++row;
    _licensor.init (_wr);
    Gtk::HBox *space = Gtk::manage (new Gtk::HBox);
    space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
    space->set_valign(Gtk::ALIGN_CENTER);
    _page_metadata2.attach(*space, 0, row, 1, 1);

    _licensor.set_hexpand();
    _licensor.set_valign(Gtk::ALIGN_CENTER);
    _page_metadata2.attach(_licensor, 1, row, 2, 1);
#else
    _page_metadata2.attach(*space, 0,1, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    _page_metadata2.attach(_licensor, 1,3, row, row+1, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
#endif
}

/**
 * Update dialog widgets from desktop.
 */
void DocumentMetadata::update()
{
    if (_wr.isUpdating()) return;

    _wr.setUpdating (true);
    set_sensitive (true);

    //-----------------------------------------------------------meta pages
    /* update the RDF entities */
    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it)
        (*it)->update (SP_ACTIVE_DOCUMENT);

    _licensor.update (SP_ACTIVE_DOCUMENT);

    _wr.setUpdating (false);
}

void 
DocumentMetadata::_handleDocumentReplaced(SPDesktop* desktop, SPDocument *)
{
    Inkscape::XML::Node *repr = desktop->getNamedView()->getRepr();
    repr->addListener (&_repr_events, this);
    update();
}

void 
DocumentMetadata::_handleActivateDesktop(SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = desktop->getNamedView()->getRepr();
    repr->addListener(&_repr_events, this);
    update();
}

void
DocumentMetadata::_handleDeactivateDesktop(SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = desktop->getNamedView()->getRepr();
    repr->removeListenerByData(this);
}

//--------------------------------------------------------------------

/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void on_repr_attr_changed(Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer data)
{
    if (DocumentMetadata *dialog = static_cast<DocumentMetadata *>(data))
	dialog->update();
}

} // namespace Dialog
} // namespace UI
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

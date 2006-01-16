/** \file
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/entry.h>

#include "ui/widget/registry.h"

#include "dialogs/rdf.h"

#include "inkscape.h"

#include "entity-entry.h"

namespace Inkscape {
namespace UI {
namespace Widget {

//===================================================

//---------------------------------------------------

EntityEntry*
EntityEntry::create (rdf_work_entity_t* ent, Gtk::Tooltips& tt, Registry& wr)
{
    g_assert (ent);
    EntityEntry* obj = 0;
    switch (ent->format)
    {
        case RDF_FORMAT_LINE: 
            obj = new EntityLineEntry (ent, tt, wr);
            break;
        case RDF_FORMAT_MULTILINE: 
            obj = new EntityMultiLineEntry (ent, tt, wr);
            break;
        default:
            g_warning ("Can't happen.");
    }

    obj->_label.show();
    return obj;
}

EntityEntry::EntityEntry (rdf_work_entity_t* ent, Gtk::Tooltips& tt, Registry& wr)
: _label(Glib::ustring(_(ent->title))+":", 1.0, 0.5), _packable(0), 
  _entity(ent), _tt(&tt), _wr(&wr)
{
}

EntityEntry::~EntityEntry()
{
    _changed_connection.disconnect();
}

EntityLineEntry::EntityLineEntry (rdf_work_entity_t* ent, Gtk::Tooltips& tt, Registry& wr)
: EntityEntry (ent, tt, wr)
{
    Gtk::Entry *e = new Gtk::Entry;
    tt.set_tip (*e, _(ent->tip));
    _packable = e;
    _changed_connection = e->signal_changed().connect (sigc::mem_fun (*this, &EntityLineEntry::on_changed));
}

EntityLineEntry::~EntityLineEntry()
{
    delete reinterpret_cast<Gtk::Entry*>(_packable);
}

void 
EntityLineEntry::update (SPDocument *doc)
{
    const char *text = rdf_get_work_entity (doc, _entity);
    reinterpret_cast<Gtk::Entry*>(_packable)->set_text (text ? text : "");
}

void
EntityLineEntry::on_changed()
{
    if (_wr->isUpdating()) return;

    _wr->setUpdating (true);
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    char const *text = reinterpret_cast<Gtk::Entry*>(_packable)->get_text().c_str();
    if (rdf_set_work_entity (doc, _entity, text))
        sp_document_done (doc);
    _wr->setUpdating (false);
}

EntityMultiLineEntry::EntityMultiLineEntry (rdf_work_entity_t* ent, Gtk::Tooltips& tt, Registry& wr)
: EntityEntry (ent, tt, wr)
{
    Gtk::ScrolledWindow *s = new Gtk::ScrolledWindow;
    s->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    s->set_shadow_type (Gtk::SHADOW_IN);
    _packable = s;
    _v.set_size_request (-1, 5);
    _v.set_wrap_mode (Gtk::WRAP_WORD);
    _v.set_accepts_tab (false);
    s->add (_v);
    tt.set_tip (_v, _(ent->tip));
    _changed_connection = _v.get_buffer()->signal_changed().connect (sigc::mem_fun (*this, &EntityMultiLineEntry::on_changed));
}

EntityMultiLineEntry::~EntityMultiLineEntry()
{
    delete reinterpret_cast<Gtk::ScrolledWindow*>(_packable);
}

void 
EntityMultiLineEntry::update (SPDocument *doc)
{
    const char *text = rdf_get_work_entity (doc, _entity);
    Gtk::ScrolledWindow *s = reinterpret_cast<Gtk::ScrolledWindow*>(_packable);
    Gtk::TextView *tv = reinterpret_cast<Gtk::TextView*>(s->get_child());
    tv->get_buffer()->set_text (text ? text : "");
}

void
EntityMultiLineEntry::on_changed()
{
    if (_wr->isUpdating()) return;

    _wr->setUpdating (true);
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    Gtk::ScrolledWindow *s = reinterpret_cast<Gtk::ScrolledWindow*>(_packable);
    Gtk::TextView *tv = reinterpret_cast<Gtk::TextView*>(s->get_child());
    char const *text = tv->get_buffer()->get_text().c_str();
    if (rdf_set_work_entity (doc, _entity, text))
        sp_document_done (doc);
    _wr->setUpdating (false);
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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

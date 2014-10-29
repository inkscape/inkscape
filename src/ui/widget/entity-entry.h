/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_ENTITY_ENTRY__H
#define INKSCAPE_UI_WIDGET_ENTITY_ENTRY__H

#include <gtkmm/textview.h>

struct rdf_work_entity_t;
class SPDocument;

namespace Gtk {
class TextBuffer;
}

namespace Inkscape {
namespace UI {
namespace Widget {

class Registry;

class EntityEntry {
public:
    static EntityEntry* create (rdf_work_entity_t* ent, Registry& wr);
    virtual ~EntityEntry() = 0;
    virtual void update (SPDocument *doc) = 0;
    virtual void on_changed() = 0;
    virtual void load_from_preferences() = 0;
    void save_to_preferences(SPDocument *doc);
    Gtk::Label _label;
    Gtk::Widget *_packable;

protected: 
    EntityEntry (rdf_work_entity_t* ent, Registry& wr);
    sigc::connection _changed_connection;
    rdf_work_entity_t *_entity;
    Registry *_wr;
};

class EntityLineEntry : public EntityEntry {
public:
    EntityLineEntry (rdf_work_entity_t* ent, Registry& wr);
    ~EntityLineEntry();
    void update (SPDocument *doc);
    void load_from_preferences();

protected:
    virtual void on_changed();
};

class EntityMultiLineEntry : public EntityEntry {
public:
    EntityMultiLineEntry (rdf_work_entity_t* ent, Registry& wr);
    ~EntityMultiLineEntry();
    void update (SPDocument *doc);
    void load_from_preferences();

protected: 
    virtual void on_changed();
    Gtk::TextView _v;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_ENTITY_ENTRY__H

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

#ifndef SP_TREF_H
#define SP_TREF_H

/** \file
 * SVG <tref> implementation, see sp-tref.cpp.
 * 
 * This file was created based on skeleton.h
 */
/*
 * Authors:
 *   Gail Banaszkiewicz <Gail.Banaszkiewicz@gmail.com>
 *
 * Copyright (C) 2007 Gail Banaszkiewicz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-item.h"
#include "sp-tref-reference.h"
#include "text-tag-attributes.h"


/* tref base class */

#define SP_TYPE_TREF (sp_tref_get_type())
#define SP_TREF(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_TREF, SPTRef))
#define SP_TREF_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_TREF, SPTSpanClass))
#define SP_IS_TREF(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_TREF))
#define SP_IS_TREF_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_TREF))

class SPTRef;
class SPTRef;

struct SPTRef : public SPItem {
    // Attributes that are used in the same way they would be in a tspan
    TextTagAttributes attributes;
    
    // Text stored in the xlink:href attribute
    gchar *href;
    
    // URI reference to original object
    SPTRefReference *uriOriginalRef;
    
    // Shortcut pointer to the child of the tref (which is a copy
    // of the character data stored at and/or below the node
    // referenced by uriOriginalRef)
    SPObject *stringChild;
    
    // The sigc connections for various notifications
    sigc::connection _delete_connection;
    sigc::connection _changed_connection;
    
    SPObject * getObjectReferredTo();
};

struct SPTRefClass {
    SPItemClass parent_class;
};

GType sp_tref_get_type();

void sp_tref_update_text(SPTRef *tref);
bool sp_tref_reference_allowed(SPTRef *tref, SPObject *possible_ref);
bool sp_tref_fully_contained(SPObject *start_item, Glib::ustring::iterator &start, 
                             SPObject *end_item, Glib::ustring::iterator &end);
SPObject * sp_tref_convert_to_tspan(SPObject *item);


#endif /* !SP_TREF_H */

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

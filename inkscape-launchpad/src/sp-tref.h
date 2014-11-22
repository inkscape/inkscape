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

#define SP_TREF(obj) (dynamic_cast<SPTRef*>((SPObject*)obj))
#define SP_IS_TREF(obj) (dynamic_cast<const SPTRef*>((SPObject*)obj) != NULL)

class SPTRef : public SPItem {
public:
	SPTRef();
	virtual ~SPTRef();

    // Attributes that are used in the same way they would be in a tspan
    TextTagAttributes attributes;
    
    // Text stored in the xlink:href attribute
    char *href;
    
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
    SPObject const *getObjectReferredTo() const;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();
	virtual void set(unsigned int key, char const* value);
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);

	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type) const;
        virtual const char* displayName() const;
	virtual char* description() const;
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

#ifndef __SP_TAG_USE_H__
#define __SP_TAG_USE_H__

/*
 * SVG <inkscape:tagref> implementation
 *
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <stddef.h>
#include <sigc++/sigc++.h>
#include "svg/svg-length.h"
#include "sp-object.h"


#define SP_TAG_USE(obj)            (dynamic_cast<SPTagUse*> (obj))
#define SP_IS_TAG_USE(obj)         (dynamic_cast<SPTagUse*> (obj) != NULL)

class SPTagUse;
class SPTagUseReference;

class SPTagUse : public SPObject {

public:
    // item built from the original's repr (the visible clone)
    // relative to the SPUse itself, it is treated as a child, similar to a grouped item relative to its group
    SPObject *child;
    gchar *href;
public:
    SPTagUse();
    virtual ~SPTagUse();
    
    virtual void build(SPDocument *doc, Inkscape::XML::Node *repr);
    virtual void set(unsigned key, gchar const *value);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
    virtual void release();
    
    virtual void href_changed(SPObject* old_ref, SPObject* ref);
    
    //virtual SPItem* unlink();
    virtual SPItem* get_original();
    virtual SPItem* root();

    // the reference to the original object
    SPTagUseReference *ref;
    sigc::connection _changed_connection;
};

#endif

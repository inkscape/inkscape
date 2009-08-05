#ifndef INKSCAPE_LIVEPATHEFFECT_OBJECT_H
#define INKSCAPE_LIVEPATHEFFECT_OBJECT_H

/*
 * Inkscape::LivePathEffect
 *
* Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#include "sp-object.h"
#include "effect.h"

namespace Inkscape {
namespace XML {
class Node;
class Document;
}
}

#define TYPE_LIVEPATHEFFECT  (LivePathEffectObject::livepatheffect_get_type())
#define LIVEPATHEFFECT(o)    (G_TYPE_CHECK_INSTANCE_CAST((o), TYPE_LIVEPATHEFFECT, LivePathEffectObject))
#define IS_LIVEPATHEFFECT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), TYPE_LIVEPATHEFFECT))

/// The LivePathEffect vtable.
struct LivePathEffectObjectClass {
    SPObjectClass parent_class;
};

class LivePathEffectObject : public SPObject {
public:
    Inkscape::LivePathEffect::EffectType effecttype;

    bool effecttype_set;

    LivePathEffectObject * fork_private_if_necessary(unsigned int nr_of_allowed_users = 1);

    /* Note that the returned pointer can be NULL in a valid LivePathEffectObject contained in a valid list of lpeobjects in an lpeitem!
     * So one should always check whether the returned value is NULL or not */
    Inkscape::LivePathEffect::Effect * get_lpe() { return lpe; };

private:
    Inkscape::LivePathEffect::Effect *lpe; // this can be NULL in a valid LivePathEffectObject

    /* C-style class functions: */
public:
    static GType livepatheffect_get_type();
private:
    static void livepatheffect_class_init(LivePathEffectObjectClass *klass);
    static void livepatheffect_init(LivePathEffectObject *stop);
    static void livepatheffect_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
    static void livepatheffect_release(SPObject *object);
    static void livepatheffect_set(SPObject *object, unsigned key, gchar const *value);
    static Inkscape::XML::Node *livepatheffect_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
};


#endif

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

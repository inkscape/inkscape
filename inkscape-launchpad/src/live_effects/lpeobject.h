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
#include "effect-enum.h"

namespace Inkscape {
    namespace XML {
        class Node;
        struct Document;
    }
    namespace LivePathEffect {
        class Effect;
    }
}

#define LIVEPATHEFFECT(obj) ((LivePathEffectObject*)obj)
#define IS_LIVEPATHEFFECT(obj) (dynamic_cast<const LivePathEffectObject*>((SPObject*)obj))

class LivePathEffectObject : public SPObject {
public:
	LivePathEffectObject();
	virtual ~LivePathEffectObject();

    Inkscape::LivePathEffect::EffectType effecttype;

    bool effecttype_set;

    LivePathEffectObject * fork_private_if_necessary(unsigned int nr_of_allowed_users = 1);

    /* Note that the returned pointer can be NULL in a valid LivePathEffectObject contained in a valid list of lpeobjects in an lpeitem!
     * So one should always check whether the returned value is NULL or not */
    Inkscape::LivePathEffect::Effect * get_lpe() { return lpe; };
    Inkscape::LivePathEffect::Effect const * get_lpe() const { return lpe; };

    Inkscape::LivePathEffect::Effect *lpe; // this can be NULL in a valid LivePathEffectObject

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, char const* value);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

#ifndef SP_LPE_ITEM_H_SEEN
#define SP_LPE_ITEM_H_SEEN

/** \file
 * Base class for live path effect items
 */
/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Bastien Bouclet <bgkweb@gmail.com>
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <list>
#include <string>
#include "sp-item.h"

#define SP_LPE_ITEM(obj) (dynamic_cast<SPLPEItem*>((SPObject*)obj))
#define SP_IS_LPE_ITEM(obj) (dynamic_cast<const SPLPEItem*>((SPObject*)obj) != NULL)

class LivePathEffectObject;
class SPCurve;
class SPDesktop;

namespace Inkscape{ 
    namespace Display {
        class TemporaryItem;
    }
    namespace LivePathEffect{
        class LPEObjectReference;
        class Effect;
    }
}

typedef std::list<Inkscape::LivePathEffect::LPEObjectReference *> PathEffectList;

class SPLPEItem : public SPItem {
public:
    SPLPEItem();
    virtual ~SPLPEItem();

    int path_effects_enabled;

    PathEffectList* path_effect_list;
    std::list<sigc::connection> *lpe_modified_connection_list; // this list contains the connections for listening to lpeobject parameter changes

    Inkscape::LivePathEffect::LPEObjectReference* current_path_effect;
    std::vector<Inkscape::Display::TemporaryItem*> lpe_helperpaths;

    void replacePathEffects( std::vector<LivePathEffectObject const *> const &old_lpeobjs,
                             std::vector<LivePathEffectObject const *> const &new_lpeobjs );


    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void release();

    virtual void set(unsigned int key, char const* value);

    virtual void update(SPCtx* ctx, unsigned int flags);
    virtual void modified(unsigned int flags);

    virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
    virtual void remove_child(Inkscape::XML::Node* child);

    virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

    virtual void update_patheffect(bool write);

    bool performPathEffect(SPCurve *curve, bool is_clip_or_mask = false);

    bool pathEffectsEnabled() const;
    bool hasPathEffect() const;
    bool hasPathEffectOfType(int const type) const;
    bool hasPathEffectRecursive() const;
    Inkscape::LivePathEffect::Effect* getPathEffectOfType(int type);
    Inkscape::LivePathEffect::Effect const* getPathEffectOfType(int type) const;
    bool hasBrokenPathEffect() const;

    PathEffectList getEffectList();
    PathEffectList const getEffectList() const;

    void downCurrentPathEffect();
    void upCurrentPathEffect();
    Inkscape::LivePathEffect::LPEObjectReference* getCurrentLPEReference();
    Inkscape::LivePathEffect::Effect* getCurrentLPE();
    bool setCurrentPathEffect(Inkscape::LivePathEffect::LPEObjectReference* lperef);
    void removeCurrentPathEffect(bool keep_paths);
    void removeAllPathEffects(bool keep_paths);
    void addPathEffect(std::string value, bool reset);
    void addPathEffect(LivePathEffectObject * new_lpeobj);
    void apply_to_mask(SPItem * item);
    void apply_to_clippath(SPItem * item);
    void apply_to_clip_or_mask(SPItem * clip_mask, SPItem * item);
    bool forkPathEffectsIfNecessary(unsigned int nr_of_allowed_users = 1);

    void editNextParamOncanvas(SPDesktop *dt);
};

void sp_lpe_item_update_patheffect (SPLPEItem *lpeitem, bool wholetree, bool write); // careful, class already has method with *very* similar name!

#endif /* !SP_LPE_ITEM_H_SEEN */

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

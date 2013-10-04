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

#include "sp-item.h"

#include <list>

#define SP_LPE_ITEM(obj) (dynamic_cast<SPLPEItem*>((SPObject*)obj))
#define SP_IS_LPE_ITEM(obj) (dynamic_cast<const SPLPEItem*>((SPObject*)obj) != NULL)

class CLPEItem;
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

	virtual void set(unsigned int key, gchar const* value);

	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);

	virtual void update_patheffect(bool write);

    bool pathEffectsEnabled() const;
    bool hasPathEffect() const;
    bool hasPathEffectOfType(int const type) const;
    bool hasPathEffectRecursive() const;
    Inkscape::LivePathEffect::Effect* getPathEffectOfType(int type);
    bool hasBrokenPathEffect() const;

    PathEffectList getEffectList();
    PathEffectList const getEffectList() const;

    void downCurrentPathEffect();
    void upCurrentPathEffect();
};

void sp_lpe_item_update_patheffect (SPLPEItem *lpeitem, bool wholetree, bool write);
bool sp_lpe_item_perform_path_effect(SPLPEItem *lpeitem, SPCurve *curve);
void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, gchar *value, bool reset);
void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * new_lpeobj);
bool sp_lpe_item_fork_path_effects_if_necessary(SPLPEItem *lpeitem, unsigned int nr_of_allowed_users = 1);
void sp_lpe_item_remove_all_path_effects(SPLPEItem *lpeitem, bool keep_paths);
void sp_lpe_item_remove_current_path_effect(SPLPEItem *lpeitem, bool keep_paths);
void sp_lpe_item_edit_next_param_oncanvas(SPLPEItem *lpeitem, SPDesktop *dt);
Inkscape::LivePathEffect::LPEObjectReference* sp_lpe_item_get_current_lpereference(SPLPEItem *lpeitem);
Inkscape::LivePathEffect::Effect* sp_lpe_item_get_current_lpe(SPLPEItem *lpeitem);
bool sp_lpe_item_set_current_path_effect(SPLPEItem *lpeitem, Inkscape::LivePathEffect::LPEObjectReference* lperef);

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

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
#include "display/display-forward.h"

#include <list>

#define SP_TYPE_LPE_ITEM (sp_lpe_item_get_type())
#define SP_LPE_ITEM(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_LPE_ITEM, SPLPEItem))
#define SP_LPE_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_LPE_ITEM, SPLPEItemClass))
#define SP_IS_LPE_ITEM(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_LPE_ITEM))
#define SP_IS_LPE_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_LPE_ITEM))

struct LivePathEffectObject;
namespace Inkscape{ 
namespace LivePathEffect{
    class LPEObjectReference;
    class Effect;
};
};

typedef std::list<Inkscape::LivePathEffect::LPEObjectReference *> PathEffectList;

struct SPLPEItem : public SPItem {
    int path_effects_enabled;

    PathEffectList* path_effect_list;
    Inkscape::LivePathEffect::LPEObjectReference* current_path_effect;
    std::vector<Inkscape::Display::TemporaryItem*> lpe_helperpaths;

    bool adding_helperpaths;
    bool removing_helperpaths;

    sigc::connection lpe_modified_connection;
};

struct SPLPEItemClass {
    SPItemClass parent_class;

    void (* update_patheffect) (SPLPEItem *lpeitem, bool write);
};

GType sp_lpe_item_get_type();

void sp_lpe_item_update_patheffect (SPLPEItem *lpeitem, bool wholetree, bool write);
void sp_lpe_item_perform_path_effect(SPLPEItem *lpeitem, SPCurve *curve);
void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, gchar *value, bool reset);
void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * new_lpeobj);
void sp_lpe_item_replace_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * old_lpeobj,
                                        LivePathEffectObject * new_lpeobj);
void sp_lpe_item_remove_all_path_effects(SPLPEItem *lpeitem, bool keep_paths);
void sp_lpe_item_remove_current_path_effect(SPLPEItem *lpeitem, bool keep_paths);
void sp_lpe_item_down_current_path_effect(SPLPEItem *lpeitem);
void sp_lpe_item_up_current_path_effect(SPLPEItem *lpeitem);
bool sp_lpe_item_has_path_effect(SPLPEItem *lpeitem);
bool sp_lpe_item_has_path_effect_recursive(SPLPEItem *lpeitem);
void sp_lpe_item_edit_next_param_oncanvas(SPLPEItem *lpeitem, SPDesktop *dt);
PathEffectList sp_lpe_item_get_effect_list(SPLPEItem *lpeitem);
Inkscape::LivePathEffect::LPEObjectReference* sp_lpe_item_get_current_lpereference(SPLPEItem *lpeitem);
Inkscape::LivePathEffect::Effect* sp_lpe_item_get_current_lpe(SPLPEItem *lpeitem);
bool sp_lpe_item_set_current_path_effect(SPLPEItem *lpeitem, Inkscape::LivePathEffect::LPEObjectReference* lperef);
bool sp_lpe_item_path_effects_enabled(SPLPEItem *lpeitem);
void sp_lpe_item_add_temporary_canvasitems(SPLPEItem *lpeitem, SPDesktop *desktop);
void sp_lpe_item_remove_temporary_canvasitems(SPLPEItem *lpeitem, SPDesktop *desktop);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

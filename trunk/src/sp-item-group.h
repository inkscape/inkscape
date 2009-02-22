#ifndef __SP_ITEM_GROUP_H__
#define __SP_ITEM_GROUP_H__

/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include "sp-lpe-item.h"

#define SP_TYPE_GROUP            (sp_group_get_type ())
#define SP_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GROUP, SPGroup))
#define SP_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GROUP, SPGroupClass))
#define SP_IS_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GROUP))
#define SP_IS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GROUP))

class CGroup;

struct SPGroup : public SPLPEItem {
    enum LayerMode { GROUP, LAYER };

    LayerMode _layer_mode;
    std::map<unsigned int, LayerMode> _display_modes;

    LayerMode layerMode() const { return _layer_mode; }
    void setLayerMode(LayerMode mode);

    LayerMode effectiveLayerMode(unsigned int display_key) const {
        if ( _layer_mode == LAYER ) {
            return LAYER;
        } else {
            return layerDisplayMode(display_key);
        }
    }

    LayerMode layerDisplayMode(unsigned int display_key) const;
    void setLayerDisplayMode(unsigned int display_key, LayerMode mode);
    void translateChildItems(Geom::Translate const &tr);

    CGroup *group;
    
private:
    void _updateLayerMode(unsigned int display_key=0);
};

struct SPGroupClass {
    SPLPEItemClass parent_class;
};

/*
 * Virtual methods of SPGroup
 */
class CGroup {
public:
    CGroup(SPGroup *group);
    virtual ~CGroup();
    
    virtual void onChildAdded(Inkscape::XML::Node *child);
    virtual void onChildRemoved(Inkscape::XML::Node *child);
    virtual void onUpdate(SPCtx *ctx, unsigned int flags);
    virtual void onModified(guint flags);
    virtual void calculateBBox(NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
    virtual void onPrint(SPPrintContext *ctx);
    virtual void onOrderChanged(Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref);
    virtual gchar *getDescription();
    virtual NRArenaItem *show (NRArena *arena, unsigned int key, unsigned int flags);
    virtual void hide (unsigned int key);

    gint getItemCount();

protected:
    virtual void _showChildren (NRArena *arena, NRArenaItem *ai, unsigned int key, unsigned int flags);

    SPGroup *_group;
};

GType sp_group_get_type (void);

void sp_item_group_ungroup (SPGroup *group, GSList **children, bool do_done = true);


GSList *sp_item_group_item_list (SPGroup *group);
SPObject *sp_item_group_get_child_by_name (SPGroup *group, SPObject *ref, const gchar *name);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

#ifndef SEEN_SP_ITEM_GROUP_H
#define SEEN_SP_ITEM_GROUP_H

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

#define SP_GROUP(obj) (dynamic_cast<SPGroup*>((SPObject*)obj))
#define SP_IS_GROUP(obj) (dynamic_cast<const SPGroup*>((SPObject*)obj) != NULL)

#define SP_IS_LAYER(obj)         (SP_IS_GROUP(obj) && SP_GROUP(obj)->layerMode() == SPGroup::LAYER)

namespace Inkscape {

class Drawing;
class DrawingItem;

} // namespace Inkscape

class SPGroup : public SPLPEItem {
public:
	SPGroup();
	virtual ~SPGroup();

    enum LayerMode { GROUP, LAYER, MASK_HELPER };

    bool _expanded;
    bool _insertBottom;
    LayerMode _layer_mode;
    std::map<unsigned int, LayerMode> _display_modes;

    LayerMode layerMode() const { return _layer_mode; }
    void setLayerMode(LayerMode mode);

    bool expanded() const { return _expanded; }
    void setExpanded(bool isexpanded);
    
    bool insertBottom() const { return _insertBottom; }
    void setInsertBottom(bool insertbottom);

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
    void scaleChildItemsRec(Geom::Scale const &sc, Geom::Point const &p, bool noRecurse);

    int getItemCount() const;
    virtual void _showChildren (Inkscape::Drawing &drawing, Inkscape::DrawingItem *ai, unsigned int key, unsigned int flags);

private:
    void _updateLayerMode(unsigned int display_key=0);

public:
    virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
   	virtual void release();

    virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
    virtual void remove_child(Inkscape::XML::Node *child);
    virtual void order_changed(Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref);

    virtual void update(SPCtx *ctx, unsigned int flags);
    virtual void modified(unsigned int flags);
    virtual void set(unsigned int key, char const* value);

    virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

    virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType bboxtype) const;
    virtual void print(SPPrintContext *ctx);
    virtual const char* displayName() const;
    virtual char *description() const;
    virtual Inkscape::DrawingItem *show (Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
    virtual void hide (unsigned int key);

    virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;

    virtual void update_patheffect(bool write);
};


/**
 * finds clones of a child of the group going out of the group; and inverse the group transform on its clones
 * Also called when moving objects between different layers
 * @param group current group
 * @param parent original parent
 * @param g transform
 */
void sp_item_group_ungroup_handle_clones(SPItem *parent, Geom::Affine const g);

void sp_item_group_ungroup (SPGroup *group, std::vector<SPItem*> &children, bool do_done = true);


std::vector<SPItem*> sp_item_group_item_list (SPGroup *group);

SPObject *sp_item_group_get_child_by_name (SPGroup *group, SPObject *ref, const char *name);

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

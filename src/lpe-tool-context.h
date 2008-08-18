#ifndef SP_LPETOOL_CONTEXT_H_SEEN
#define SP_LPETOOL_CONTEXT_H_SEEN

/*
 * LPEToolContext: a context for a generic tool composed of subtools that are given by LPEs
 *
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "pen-context.h"
#include "helper/units.h"

#define SP_TYPE_LPETOOL_CONTEXT (sp_lpetool_context_get_type())
#define SP_LPETOOL_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_LPETOOL_CONTEXT, SPLPEToolContext))
#define SP_LPETOOL_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_LPETOOL_CONTEXT, SPLPEToolContextClass))
#define SP_IS_LPETOOL_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_LPETOOL_CONTEXT))
#define SP_IS_LPETOOL_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_LPETOOL_CONTEXT))

class SPLPEToolContext;
class SPLPEToolContextClass;

/* This is the list of subtools from which the toolbar of the LPETool is built automatically */
extern const int num_subtools;

extern Inkscape::LivePathEffect::EffectType lpesubtools[];

enum LPEToolState {
    LPETOOL_STATE_PEN,
    LPETOOL_STATE_NODE
};

namespace Inkscape {
class Selection;
}

class ShapeEditor;

struct SPLPEToolContext : public SPPenContext {
    ShapeEditor* shape_editor;
    SPCanvasItem *canvas_bbox;
    Inkscape::LivePathEffect::EffectType mode;

    std::map<SPPath *, SPCanvasItem*> *measuring_items;

    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;
};

struct SPLPEToolContextClass : public SPEventContextClass{};

int lpetool_mode_to_index(Inkscape::LivePathEffect::EffectType const type);
int lpetool_item_has_construction(SPLPEToolContext *lc, SPItem *item);
bool lpetool_try_construction(SPLPEToolContext *lc, Inkscape::LivePathEffect::EffectType const type);
void lpetool_context_switch_mode(SPLPEToolContext *lc, Inkscape::LivePathEffect::EffectType const type);
void lpetool_get_limiting_bbox_corners(SPDocument *document, Geom::Point &A, Geom::Point &B);
void lpetool_context_reset_limiting_bbox(SPLPEToolContext *lc);
void lpetool_create_measuring_items(SPLPEToolContext *lc, Inkscape::Selection *selection = NULL);
void lpetool_delete_measuring_items(SPLPEToolContext *lc);
void lpetool_update_measuring_items(SPLPEToolContext *lc);
void lpetool_show_measuring_info(SPLPEToolContext *lc, bool show = true);

GType sp_lpetool_context_get_type(void);

#endif // SP_LPETOOL_CONTEXT_H_SEEN

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

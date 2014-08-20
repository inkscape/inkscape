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

#include "ui/tools/pen-tool.h"

#define SP_LPETOOL_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::LpeTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_LPETOOL_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::LpeTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

/* This is the list of subtools from which the toolbar of the LPETool is built automatically */
extern const int num_subtools;

struct SubtoolEntry {
    Inkscape::LivePathEffect::EffectType type;
    gchar const *icon_name;
};

extern SubtoolEntry lpesubtools[];

enum LPEToolState {
    LPETOOL_STATE_PEN,
    LPETOOL_STATE_NODE
};

namespace Inkscape {
class Selection;
}

class ShapeEditor;

namespace Inkscape {
namespace UI {
namespace Tools {

class LpeTool : public PenTool {
public:
	LpeTool();
	virtual ~LpeTool();

    ShapeEditor* shape_editor;
    SPCanvasItem *canvas_bbox;
    Inkscape::LivePathEffect::EffectType mode;

    std::map<SPPath *, SPCanvasItem*> *measuring_items;

    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;

	static const std::string prefsPath;

	virtual const std::string& getPrefsPath();

protected:
	virtual void setup();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);
};

int lpetool_mode_to_index(Inkscape::LivePathEffect::EffectType const type);
int lpetool_item_has_construction(LpeTool *lc, SPItem *item);
bool lpetool_try_construction(LpeTool *lc, Inkscape::LivePathEffect::EffectType const type);
void lpetool_context_switch_mode(LpeTool *lc, Inkscape::LivePathEffect::EffectType const type);
void lpetool_get_limiting_bbox_corners(SPDocument *document, Geom::Point &A, Geom::Point &B);
void lpetool_context_reset_limiting_bbox(LpeTool *lc);
void lpetool_create_measuring_items(LpeTool *lc, Inkscape::Selection *selection = NULL);
void lpetool_delete_measuring_items(LpeTool *lc);
void lpetool_update_measuring_items(LpeTool *lc);
void lpetool_show_measuring_info(LpeTool *lc, bool show = true);

}
}
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

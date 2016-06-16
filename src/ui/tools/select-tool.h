#ifndef __SP_SELECT_CONTEXT_H__
#define __SP_SELECT_CONTEXT_H__

/*
 * Select tool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/tools/tool-base.h"

#define SP_SELECT_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::SelectTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_SELECT_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::SelectTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

struct SPCanvasItem;

namespace Inkscape {
  class MessageContext;
  class SelTrans;
  class SelectionDescriber;
}

namespace Inkscape {
namespace UI {
namespace Tools {

class SelectTool : public ToolBase {
public:
	SelectTool();
	virtual ~SelectTool();

	bool dragging;
	bool moved;
	bool button_press_shift;
	bool button_press_ctrl;
	bool button_press_alt;

        std::vector<SPItem *> cycling_items;
        std::vector<SPItem *> cycling_items_cmp;
        SPItem *cycling_cur_item;
	bool cycling_wrap;

	SPItem *item;
	SPCanvasItem *grabbed;
	Inkscape::SelTrans *_seltrans;
	Inkscape::SelectionDescriber *_describer;

	static const std::string prefsPath;

	virtual void setup();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	bool sp_select_context_abort();
	void sp_select_context_cycle_through_items(Inkscape::Selection *selection, GdkEventScroll *scroll_event, bool shift_pressed);
	void sp_select_context_reset_opacities();
};

}
}
}

#endif

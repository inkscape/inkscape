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

#include "event-context.h"
#include <gtk/gtk.h>

#define SP_SELECT_CONTEXT(obj) (dynamic_cast<SPSelectContext*>((SPEventContext*)obj))
#define SP_IS_SELECT_CONTEXT(obj) (dynamic_cast<const SPSelectContext*>((const SPEventContext*)obj) != NULL)

struct SPCanvasItem;

namespace Inkscape {
  class MessageContext;
  class SelTrans;
  class SelectionDescriber;
}

class SPSelectContext : public SPEventContext {
public:
	SPSelectContext();
	virtual ~SPSelectContext();

	guint dragging : 1;
	guint moved : 1;
	bool button_press_shift;
	bool button_press_ctrl;
	bool button_press_alt;

	GList *cycling_items;
	GList *cycling_items_cmp;
	GList *cycling_items_selected_before;
	GList *cycling_cur_item;
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
};

#endif

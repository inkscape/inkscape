#ifndef __SP_FLOOD_CONTEXT_H__
#define __SP_FLOOD_CONTEXT_H__

/*
 * Flood fill drawing context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   John Bintz <jcoswell@coswellproductions.org>
 *
 * Released under GNU GPL
 */

#include <sigc++/connection.h>
#include "ui/tools/tool-base.h"

#define SP_FLOOD_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::FloodTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_FLOOD_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::FloodTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {
namespace Tools {

class FloodTool : public ToolBase {
public:
	FloodTool();
	virtual ~FloodTool();

	SPItem *item;

	sigc::connection sel_changed_connection;

	static const std::string prefsPath;

	virtual void setup();
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

	static void set_channels(gint channels);

private:
	void selection_changed(Inkscape::Selection* selection);
	void finishItem();
};

GList* flood_channels_dropdown_items_list (void);
GList* flood_autogap_dropdown_items_list (void);

enum PaintBucketChannels {
    FLOOD_CHANNELS_RGB,
    FLOOD_CHANNELS_R,
    FLOOD_CHANNELS_G,
    FLOOD_CHANNELS_B,
    FLOOD_CHANNELS_H,
    FLOOD_CHANNELS_S,
    FLOOD_CHANNELS_L,
    FLOOD_CHANNELS_ALPHA
};

}
}
}

#endif

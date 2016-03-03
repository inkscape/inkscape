#ifndef SP_ERASER_CONTEXT_H_SEEN
#define SP_ERASER_CONTEXT_H_SEEN

/*
 * Handwriting-like drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/point.h>

#include "ui/tools/dynamic-base.h"

#define ERC_MIN_PRESSURE      0.0
#define ERC_MAX_PRESSURE      1.0
#define ERC_DEFAULT_PRESSURE  1.0

#define ERC_MIN_TILT         -1.0
#define ERC_MAX_TILT          1.0
#define ERC_DEFAULT_TILT      0.0

namespace Inkscape {
namespace UI {
namespace Tools {

class EraserTool : public DynamicBase {
public:
	EraserTool();
	virtual ~EraserTool();

	static const std::string prefsPath;

	virtual void setup();
	virtual bool root_handler(GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	void reset(Geom::Point p);
	void extinput(GdkEvent *event);
	bool apply(Geom::Point p);
	void brush();
	void cancel();
	void clear_current();
	void set_to_accumulated();
	void accumulate();
	void fit_and_split(bool release);
	void draw_temporary_box();
	bool nowidth;
};

}
}
}

#endif // SP_ERASER_CONTEXT_H_SEEN

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

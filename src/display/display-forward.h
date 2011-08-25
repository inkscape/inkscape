#ifndef SEEN_DISPLAY_DISPLAY_FORWARD_H
#define SEEN_DISPLAY_DISPLAY_FORWARD_H

#include <glib-object.h>

struct SPCanvas;
struct SPCanvasClass;
struct SPCanvasItem;
typedef struct _SPCanvasItemClass SPCanvasItemClass;
struct SPCanvasGroup;
struct SPCanvasGroupClass;
class SPCurve;
typedef struct _SPCanvasArena SPCanvasArena;

namespace Inkscape {
class Drawing;
class DrawingItem;
class DrawingGroup;
class DrawingImage;
class DrawingShape;
class DrawingGlyphs;
class DrawingText;
class UpdateContext;

class DrawingContext;
class DrawingSurface;
class DrawingCache;

namespace Display {
    class TemporaryItem;
    class TemporaryItemList;
}

namespace Filters {
    class Filter;
}
}

#endif /* !SEEN_DISPLAY_DISPLAY_FORWARD_H */

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

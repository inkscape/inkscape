#include <gdk/gdkevents.h>
#include "libnr/nr-rect.h"
struct SPDesktop;
struct SPItem;

const double goldenratio = 1.61803398874989484820; // golden ratio

namespace Inkscape
{

class MessageContext;
class MessageStack;

extern bool have_viable_layer(SPDesktop *desktop, MessageContext *message);
extern bool have_viable_layer(SPDesktop *desktop, MessageStack *message);
::NR::Rect snap_rectangular_box(SPDesktop const *desktop, SPItem *item,
                              NR::Point const &pt, NR::Point const &center, int state);
NR::Point setup_for_drag_start(SPDesktop *desktop, SPEventContext* ec, GdkEvent *ev);

}

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

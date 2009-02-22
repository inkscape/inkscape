#ifndef __SP_ITEM_NOTIFY_MOVETO_H__
#define __SP_ITEM_NOTIFY_MOVETO_H__

#include <forward.h>

void sp_item_notify_moveto(SPItem &item, SPGuide const &g, int const snappoint_ix,
                           double position, bool const commit);


#endif /* !__SP_ITEM_NOTIFY_MOVETO_H__ */


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

#ifndef SEEN_SP_ITEM_NOTIFY_MOVETO_H
#define SEEN_SP_ITEM_NOTIFY_MOVETO_H

class SPItem;
class SPGuide;

void sp_item_notify_moveto(SPItem &item, SPGuide const &g, int const snappoint_ix,
                           double position, bool const commit);


#endif // SEEN_SP_ITEM_NOTIFY_MOVETO_H


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

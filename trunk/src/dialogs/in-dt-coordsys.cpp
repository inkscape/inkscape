#include "sp-root.h"

/** Returns true iff \a item is suitable to be included in the selection, in particular
    whether it has a bounding box in the desktop coordinate system for rendering resize handles.

    Descendents of <defs> nodes (markers etc.) return false, for example.
*/
bool in_dt_coordsys(SPObject const &item)
{
    /* Definition based on sp_item_i2doc_affine. */
    SPObject const *child = &item;
    g_return_val_if_fail(child != NULL, false);
    for(;;) {
        if (!SP_IS_ITEM(child)) {
            return false;
        }
        SPObject const * const parent = SP_OBJECT_PARENT(child);
        if (parent == NULL) {
            break;
        }
        child = parent;
    }
    g_assert(SP_IS_ROOT(child));
    /* Relevance: Otherwise, I'm not sure whether to return true or false. */
    return true;
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

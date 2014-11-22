/*
 * Provides a class that can contain active TemporaryItem's on a desktop
 * When the object is deleted, it also deletes the canvasitem it contains!
 * This object should be created/managed by a TemporaryItemList.
 * After its lifetime, it fires the timeout signal, afterwards *it deletes itself*.
 *
 * (part of code inspired by message-stack.cpp)
 *
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/canvas-temporary-item.h"

#include <glib.h>
#include "display/sp-canvas-item.h"

namespace Inkscape {
namespace Display {

/** lifetime is measured in milliseconds
 */
TemporaryItem::TemporaryItem(SPCanvasItem *item, guint lifetime, bool deselect_destroy)
    : canvasitem(item),
      timeout_id(0),
      destroy_on_deselect(deselect_destroy)
{
    if (lifetime > 0 && destroy_on_deselect) {
        g_print ("Warning: lifetime should be 0 when destroy_on_deselect is true\n");
        lifetime = 0;
    }
    // zero lifetime means stay forever, so do not add timeout event.
    if (lifetime > 0) {
        timeout_id = g_timeout_add(lifetime, &TemporaryItem::_timeout, this);
    }
}

TemporaryItem::~TemporaryItem()
{
    // when it has not expired yet...
    if (timeout_id) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }

    if (canvasitem) {
        // destroying the item automatically hides it
        sp_canvas_item_destroy(canvasitem);
        canvasitem = NULL;
    }
}

/* static method */
int TemporaryItem::_timeout(void* data) {
    TemporaryItem *tempitem = static_cast<TemporaryItem *>(data);
    tempitem->timeout_id = 0;
    tempitem->signal_timeout.emit(tempitem);
    delete tempitem;
    return FALSE;
}


} //namespace Display
} /* namespace Inkscape */

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

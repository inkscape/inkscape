#ifndef INKSCAPE_CANVAS_TEMPORARY_ITEM_H
#define INKSCAPE_CANVAS_TEMPORARY_ITEM_H

/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <cstddef>
#include <sigc++/signal.h>

struct SPCanvasItem;

namespace Inkscape {
namespace Display {

/**
 * Provides a class to put a canvasitem temporarily on-canvas.
 */
class TemporaryItem  {
public:
    TemporaryItem(SPCanvasItem *item, unsigned int lifetime, bool destroy_on_deselect = false);
    virtual ~TemporaryItem();

    sigc::signal<void, TemporaryItem *> signal_timeout;

protected:
    friend class TemporaryItemList;

    SPCanvasItem * canvasitem;   /** The item we are holding on to */
    unsigned int timeout_id;     /** ID by which glib knows the timeout event */
    bool destroy_on_deselect; // only destroy when parent item is deselected, not when mouse leaves

    static int _timeout(void* data); ///< callback for when lifetime expired

private:
    TemporaryItem(const TemporaryItem&);
    TemporaryItem& operator=(const TemporaryItem&);
};

} //namespace Display
} //namespace Inkscape

#endif

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

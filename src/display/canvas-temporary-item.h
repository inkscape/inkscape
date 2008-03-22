#ifndef INKSCAPE_CANVAS_TEMPORARY_ITEM_H
#define INKSCAPE_CANVAS_TEMPORARY_ITEM_H

/** \file
 * Provides a class to put a canvasitem temporarily on-canvas.
 *
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"

#include <sigc++/sigc++.h>

namespace Inkscape {
namespace Display {

class TemporaryItem  {
public:
    TemporaryItem(SPCanvasItem *item, guint lifetime);
    virtual ~TemporaryItem();

    sigc::signal<void, TemporaryItem *> signal_timeout;

protected:
    friend class TemporaryItemList;

    SPCanvasItem * canvasitem;   /** The item we are holding on to */
    guint timeout_id;     /** ID by which glib knows the timeout event */

    static gboolean _timeout(gpointer data); ///< callback for when lifetime expired

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

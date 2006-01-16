#ifndef __SP_DESKTOP_HANDLES_H__
#define __SP_DESKTOP_HANDLES_H__

/*
 * Frontends
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"
#include "forward.h"

namespace Inkscape { 
    class MessageStack;
    class Selection; 
}

#define SP_DESKTOP_SCROLL_LIMIT 4000.0
#define SP_DESKTOP_ZOOM_MAX 256.0
#define SP_DESKTOP_ZOOM_MIN 0.01

#define SP_COORDINATES_UNDERLINE_NONE (0)
#define SP_COORDINATES_UNDERLINE_X (1 << NR::X)
#define SP_COORDINATES_UNDERLINE_Y (1 << NR::Y)

#define SP_DT_EVENTCONTEXT(d) sp_desktop_event_context (d)
#define SP_DT_SELECTION(d) sp_desktop_selection (d)
#define SP_DT_DOCUMENT(d) sp_desktop_document (d)
#define SP_DT_CANVAS(d) sp_desktop_canvas (d)
#define SP_DT_ACETATE(d) sp_desktop_acetate (d)
#define SP_DT_MAIN(d) sp_desktop_main (d)
#define SP_DT_GRID(d) sp_desktop_grid (d)
#define SP_DT_GUIDES(d) sp_desktop_guides (d)
#define SP_DT_DRAWING(d) sp_desktop_drawing (d)
#define SP_DT_SKETCH(d) sp_desktop_sketch (d)
#define SP_DT_CONTROLS(d) sp_desktop_controls (d)
#define SP_DT_MSGSTACK(d) sp_desktop_message_stack (d)
#define SP_DT_NAMEDVIEW(d) sp_desktop_namedview (d)

SPEventContext * sp_desktop_event_context (SPDesktop const * desktop);
Inkscape::Selection * sp_desktop_selection (SPDesktop const * desktop);
SPDocument * sp_desktop_document (SPDesktop const * desktop);
SPCanvas * sp_desktop_canvas (SPDesktop const * desktop);
SPCanvasItem * sp_desktop_acetate (SPDesktop const * desktop);
SPCanvasGroup * sp_desktop_main (SPDesktop const * desktop);
SPCanvasGroup * sp_desktop_grid (SPDesktop const * desktop);
SPCanvasGroup * sp_desktop_guides (SPDesktop const * desktop);
SPCanvasItem *sp_desktop_drawing (SPDesktop const *desktop);
SPCanvasGroup * sp_desktop_sketch (SPDesktop const * desktop);
SPCanvasGroup * sp_desktop_controls (SPDesktop const * desktop);
Inkscape::MessageStack * sp_desktop_message_stack (SPDesktop const * desktop);
SPNamedView * sp_desktop_namedview (SPDesktop const * desktop);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

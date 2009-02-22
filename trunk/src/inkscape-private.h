#ifndef __INKSCAPE_PRIVATE_H__
#define __INKSCAPE_PRIVATE_H__

/*
 * Some forward declarations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_INKSCAPE (inkscape_get_type ())
#define SP_INKSCAPE(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_INKSCAPE, Inkscape))
#define SP_INKSCAPE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_INKSCAPE, InkscapeClass))
#define SP_IS_INKSCAPE(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_INKSCAPE))
#define SP_IS_INKSCAPE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_INKSCAPE))

#include "forward.h"
#include "inkscape.h"

namespace Inkscape { class Selection; }

GType inkscape_get_type (void);

void inkscape_ref (void);
void inkscape_unref (void);

guint inkscape_mapalt();
void inkscape_mapalt(guint);

/*
 * These are meant solely for desktop, document etc. implementations
 */

void inkscape_selection_modified (Inkscape::Selection *selection, guint flags);
void inkscape_selection_changed (Inkscape::Selection * selection);
void inkscape_selection_set (Inkscape::Selection * selection);
void inkscape_eventcontext_set (SPEventContext * eventcontext);
void inkscape_add_desktop (SPDesktop * desktop);
void inkscape_remove_desktop (SPDesktop * desktop);
void inkscape_activate_desktop (SPDesktop * desktop);
void inkscape_reactivate_desktop (SPDesktop * desktop);
void inkscape_add_document (SPDocument *document);
bool inkscape_remove_document (SPDocument *document);

void inkscape_set_color (SPColor *color, float opacity);

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

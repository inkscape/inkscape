#ifndef SEEN_CONTEXT_MENU_H
#define SEEN_CONTEXT_MENU_H

/*
 * Unser-interface related object extension
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * This code is in public domain
 */

#include <gtk/gtk.h>

#include "forward.h"
#include "sp-object.h"
/* Append object-specific part to context menu */

void sp_object_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

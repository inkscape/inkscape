#ifndef __SP_DASH_SELECTOR_H__
#define __SP_DASH_SELECTOR_H__

/*
 * Optionmenu for selecting dash patterns
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

struct SPDashSelector;

#include <gtk/gtkwidget.h>
#include "../xml/repr.h"

#include <glib.h>



#define SP_TYPE_DASH_SELECTOR (sp_dash_selector_get_type ())
#define SP_DASH_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_DASH_SELECTOR, SPDashSelector))
#define SP_IS_DASH_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_DASH_SELECTOR))

GtkType sp_dash_selector_get_type (void);

GtkWidget *sp_dash_selector_new (Inkscape::XML::Node *repr);

void sp_dash_selector_set_dash (SPDashSelector *dsel, int ndash, double *dash, double offset);
void sp_dash_selector_get_dash (SPDashSelector *dsel, int *ndash, double **dash, double *offset);



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

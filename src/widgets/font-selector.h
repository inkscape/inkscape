#ifndef SP_FONT_SELECTOR_H
#define SP_FONT_SELECTOR_H

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

struct SPFontSelector;

#define SP_TYPE_FONT_SELECTOR (sp_font_selector_get_type ())
#define SP_FONT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_FONT_SELECTOR, SPFontSelector))
#define SP_IS_FONT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_FONT_SELECTOR))

class font_instance;

/* SPFontSelector */

GType sp_font_selector_get_type (void);

GtkWidget *sp_font_selector_new (void);

void sp_font_selector_set_font (SPFontSelector *fsel, font_instance *font, double size);

font_instance *sp_font_selector_get_font (SPFontSelector *fsel);
double  sp_font_selector_get_size (SPFontSelector *fsel);

unsigned int sp_font_selector_get_best_style (font_instance *font, GList *list);

#endif // SP_FONT_SELECTOR_H

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

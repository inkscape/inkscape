#ifndef SP_FONT_SELECTOR_H
#define SP_FONT_SELECTOR_H

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 1999-2013 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

struct SPFontSelector;

#define SP_TYPE_FONT_SELECTOR (sp_font_selector_get_type ())
#define SP_FONT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_FONT_SELECTOR, SPFontSelector))
#define SP_IS_FONT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_FONT_SELECTOR))

/*
 * The routines here create and manage a font selector widget with three parts,
 * one each for font-family, font-style, and font-size.
 *
 * It is used by the TextEdit  and Glyphs panel dialogs. The FontLister class is used
 * to access the list of font-families and their associated styles for fonts either
 * on the system or in the document. The FontLister class is also used by the Text
 * toolbar. Fonts are kept track of by their "fontspecs"  which are the same as the
 * strings that Pango generates.
 *
 * The main functions are:
 *   Create the font-seletor widget.
 *   Update the lists when a new text selection is made.
 *   Update the Style list when a new font-family is selected, highlighting the
 *     best match to the original font style (as not all fonts have the same style options).
 *   Emit a signal when any change is made so that the Text Preview can be updated.
 *   Provide the currently selected values.
 */

/* SPFontSelector */

GType sp_font_selector_get_type (void);

GtkWidget *sp_font_selector_new (void);

void sp_font_selector_set_fontspec (SPFontSelector *fsel, Glib::ustring fontspec, double size);
Glib::ustring sp_font_selector_get_fontspec (SPFontSelector *fsel);

double  sp_font_selector_get_size (SPFontSelector *fsel);

#endif // !SP_FONT_SELECTOR_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

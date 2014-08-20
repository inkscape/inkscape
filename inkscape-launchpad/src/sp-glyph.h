#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef __SP_GLYPH_H__
#define __SP_GLYPH_H__

/*
 * SVG <glyph> element implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_GLYPH(obj) (dynamic_cast<SPGlyph*>((SPObject*)obj))
#define SP_IS_GLYPH(obj) (dynamic_cast<const SPGlyph*>((SPObject*)obj) != NULL)

enum glyphArabicForm {
    GLYPH_ARABIC_FORM_INITIAL,
    GLYPH_ARABIC_FORM_MEDIAL,
    GLYPH_ARABIC_FORM_TERMINAL,
    GLYPH_ARABIC_FORM_ISOLATED,
};

enum glyphOrientation {
    GLYPH_ORIENTATION_HORIZONTAL,
    GLYPH_ORIENTATION_VERTICAL,
    GLYPH_ORIENTATION_BOTH
};

class SPGlyph : public SPObject {
public:
	SPGlyph();
	virtual ~SPGlyph();

    Glib::ustring unicode;
    Glib::ustring glyph_name;
    char* d;
    glyphOrientation orientation;
    glyphArabicForm arabic_form;
    char* lang;
    double horiz_adv_x;
    double vert_origin_x;
    double vert_origin_y;
    double vert_adv_y;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

#endif //#ifndef __SP_GLYPH_H__

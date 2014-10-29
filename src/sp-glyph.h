/**
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SP_GLYPH_H
#define SEEN_SP_GLYPH_H

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

/*
 * SVG <glyph> element
 */

class SPGlyph : public SPObject {
public:
    SPGlyph();
    virtual ~SPGlyph() {}

    // FIXME encapsulation
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
    virtual void set(unsigned int key, const char* value);
    virtual void update(SPCtx* ctx, unsigned int flags);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);

};

#endif // !SEEN_SP_GLYPH_H

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

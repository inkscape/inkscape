/*
 * SVG <hkern> and <vkern> elements implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SP_GLYPH_KERNING_H
#define SEEN_SP_GLYPH_KERNING_H

#include "sp-object.h"
#include "unicoderange.h"

#define SP_HKERN(obj) (dynamic_cast<SPHkern*>(obj))
#define SP_IS_HKERN(obj) (dynamic_cast<const SPHkern*>(obj) != NULL)

#define SP_VKERN(obj) (dynamic_cast<SPVkern*>(obj))
#define SP_IS_VKERN(obj) (dynamic_cast<const SPVkern*>(obj) != NULL)

// CPPIFY: These casting macros are buggy, as Vkern and Hkern aren't "real" classes.

class GlyphNames {
public: 
    GlyphNames(char const* value);
    ~GlyphNames();
    bool contains(char const* name);
private:
    char* names;
};

class SPGlyphKerning : public SPObject {
public:
    SPGlyphKerning();
    virtual ~SPGlyphKerning() {}

    // FIXME encapsulation
    UnicodeRange* u1;
    GlyphNames* g1;
    UnicodeRange* u2;
    GlyphNames* g2;
    double k;

protected:
    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void release();
    virtual void set(unsigned int key, char const* value);
    virtual void update(SPCtx* ctx, unsigned int flags);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

class SPHkern : public SPGlyphKerning {
    virtual ~SPHkern() {}
};

class SPVkern : public SPGlyphKerning {
    virtual ~SPVkern() {}
};

#endif // !SEEN_SP_GLYPH_KERNING_H

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

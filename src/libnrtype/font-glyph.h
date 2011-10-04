#ifndef SEEN_LIBNRTYPE_FONT_GLYPH_H
#define SEEN_LIBNRTYPE_FONT_GLYPH_H

#include <2geom/forward.h>

// the info for a glyph in a font. it's totally resolution- and fontsize-independent
struct font_glyph {
    double         h_advance, h_width; // width != advance because of kerning adjustements
    double         v_advance, v_width;
    double         bbox[4];            // bbox of the path (and the artbpath), not the bbox of the glyph
																			 // as the fonts sometimes contain
    Geom::PathVector* pathvector;      // outline as 2geom pathvector, for text->curve stuff (should be unified with livarot)
};


#endif /* !SEEN_LIBNRTYPE_FONT_GLYPH_H */

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

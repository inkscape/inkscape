#ifndef SEEN_FONT_STYLE_TO_POS_H
#define SEEN_FONT_STYLE_TO_POS_H

#include <libnrtype/nr-type-pos-def.h>

class SPStyle;

NRTypePosDef font_style_to_pos(SPStyle const &style);

#endif // SEEN_FONT_STYLE_TO_POS_H

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

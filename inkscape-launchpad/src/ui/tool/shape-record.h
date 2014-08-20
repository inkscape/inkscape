/** @file
 * Structures that store data needed for shape editing which are not contained
 * directly in the XML node
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_SHAPE_RECORD_H
#define SEEN_UI_TOOL_SHAPE_RECORD_H

#include <glibmm/ustring.h>
#include <boost/operators.hpp>
#include <2geom/affine.h>

class SPItem;
namespace Inkscape {
namespace UI {

/** Role of the shape in the drawing - affects outline display and color */
enum ShapeRole {
    SHAPE_ROLE_NORMAL,
    SHAPE_ROLE_CLIPPING_PATH,
    SHAPE_ROLE_MASK,
    SHAPE_ROLE_LPE_PARAM // implies edit_original set to true in ShapeRecord
};

struct ShapeRecord :
    public boost::totally_ordered<ShapeRecord>
{
    SPItem *item; // SP node for the edited shape
    Geom::Affine edit_transform; // how to transform controls - used for clipping paths and masks
    ShapeRole role;
    Glib::ustring lpe_key; // name of LPE shape param being edited

    inline bool operator==(ShapeRecord const &o) const {
        return item == o.item && lpe_key == o.lpe_key;
    }
    inline bool operator<(ShapeRecord const &o) const {
        return item == o.item ? (lpe_key < o.lpe_key) : (item < o.item);
    }
};

} // namespace UI
} // namespace Inkscape

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

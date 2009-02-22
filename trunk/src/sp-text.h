#ifndef __SP_TEXT_H__
#define __SP_TEXT_H__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>
#include <sigc++/sigc++.h>
#include "sp-item.h"
#include "sp-string.h"
#include "text-tag-attributes.h"
#include "libnr/nr-point.h"
#include "libnrtype/Layout-TNG.h"


#define SP_TYPE_TEXT (sp_text_get_type())
#define SP_TEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_TEXT, SPText))
#define SP_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_TEXT, SPTextClass))
#define SP_IS_TEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_TEXT))
#define SP_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_TEXT))

/* Text specific flags */
#define SP_TEXT_CONTENT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A
#define SP_TEXT_LAYOUT_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A


/* SPText */

struct SPText : public SPItem {
    /** Converts the text object to its component curves */
    SPCurve *getNormalizedBpath() const
        {return layout.convertToCurves();}

    /** Completely recalculates the layout. */
    void rebuildLayout();

//semiprivate:  (need to be accessed by the C-style functions still)
    TextTagAttributes attributes;
    Inkscape::Text::Layout layout;
	
    /** when the object is transformed it's nicer to change the font size
    and coordinates when we can, rather than just applying a matrix
    transform. is_root is used to indicate to the function that it should
    extend zero-length position vectors to length 1 in order to record the
    new position. This is necessary to convert from objects whose position is
    completely specified by transformations. */
    static void _adjustCoordsRecursive(SPItem *item, Geom::Matrix const &m, double ex, bool is_root = true);
    static void _adjustFontsizeRecursive(SPItem *item, double ex, bool is_root = true);
	
    /** discards the NRArena objects representing this text. */
    void _clearFlow(NRArenaGroup *in_arena);

private:
    /** Recursively walks the xml tree adding tags and their contents. The
    non-trivial code does two things: firstly, it manages the positioning
    attributes and their inheritance rules, and secondly it keeps track of line
    breaks and makes sure both that they are assigned the correct SPObject and
    that we don't get a spurious extra one at the end of the flow. */
    unsigned _buildLayoutInput(SPObject *root, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_optional_attrs, unsigned parent_attrs_offset, bool in_textpath);
};

struct SPTextClass {
    SPItemClass parent_class;
};

GType sp_text_get_type();

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

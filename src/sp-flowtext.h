#ifndef __SP_ITEM_FLOWTEXT_H__
#define __SP_ITEM_FLOWTEXT_H__

/*
 */

#include "sp-item.h"

#include "display/nr-arena-forward.h"
#include <2geom/forward.h>
#include "libnrtype/Layout-TNG.h"

#define SP_TYPE_FLOWTEXT            (sp_flowtext_get_type ())
#define SP_FLOWTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWTEXT, SPFlowtext))
#define SP_FLOWTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWTEXT, SPFlowtextClass))
#define SP_IS_FLOWTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWTEXT))
#define SP_IS_FLOWTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWTEXT))

struct SPFlowtext : public SPItem {
    /** Completely recalculates the layout. */
    void rebuildLayout();

    /** Converts the flowroot in into a \<text\> tree, keeping all the formatting and positioning,
    but losing the automatic wrapping ability. */
    Inkscape::XML::Node *getAsText();

    SPItem *get_frame(SPItem *after);

    bool has_internal_frame();

//semiprivate:  (need to be accessed by the C-style functions still)
    Inkscape::Text::Layout layout;

    /** discards the NRArena objects representing this text. */
    void _clearFlow(NRArenaGroup* in_arena);

    double par_indent;

private:
    /** Recursively walks the xml tree adding tags and their contents. */
    void _buildLayoutInput(SPObject *root, Shape const *exclusion_shape, std::list<Shape> *shapes, SPObject **pending_line_break_object);

    /** calculates the union of all the \<flowregionexclude\> children
    of this flowroot. */
    Shape* _buildExclusionShape() const;

};

struct SPFlowtextClass {
    SPItemClass parent_class;
};

GType sp_flowtext_get_type (void);

SPItem *create_flowtext_with_internal_frame (SPDesktop *desktop, Geom::Point p1, Geom::Point p2);

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

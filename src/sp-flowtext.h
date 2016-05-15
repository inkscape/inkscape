#ifndef SEEN_SP_ITEM_FLOWTEXT_H
#define SEEN_SP_ITEM_FLOWTEXT_H

/*
 */

#include <2geom/forward.h>

#include "libnrtype/Layout-TNG.h"
#include "sp-item.h"

#define SP_FLOWTEXT(obj) (dynamic_cast<SPFlowtext*>((SPObject*)obj))
#define SP_IS_FLOWTEXT(obj) (dynamic_cast<const SPFlowtext*>((SPObject*)obj) != NULL)


namespace Inkscape {

class DrawingGroup;

} // namespace Inkscape

class SPFlowtext : public SPItem {
public:
	SPFlowtext();
	virtual ~SPFlowtext();

    /** Completely recalculates the layout. */
    void rebuildLayout();

    /** Converts the flowroot in into a \<text\> tree, keeping all the formatting and positioning,
    but losing the automatic wrapping ability. */
    Inkscape::XML::Node *getAsText();

    // TODO check if these should return SPRect instead of SPItem

    SPItem *get_frame(SPItem const *after);

    SPItem const *get_frame(SPItem const *after) const;

    bool has_internal_frame() const;

//semiprivate:  (need to be accessed by the C-style functions still)
    Inkscape::Text::Layout layout;

    /** discards the drawing objects representing this text. */
    void _clearFlow(Inkscape::DrawingGroup* in_arena);

    double par_indent;

    bool _optimizeScaledText;

	/** Converts the text object to its component curves */
	SPCurve *getNormalizedBpath() const {
		return layout.convertToCurves();
	}

    /** Optimize scaled flow text on next set_transform. */
    void optimizeScaledText()
        {_optimizeScaledText = true;}

private:
    /** Recursively walks the xml tree adding tags and their contents. */
    void _buildLayoutInput(SPObject *root, Shape const *exclusion_shape, std::list<Shape> *shapes, SPObject **pending_line_break_object);

    /** calculates the union of all the \<flowregionexclude\> children
    of this flowroot. */
    Shape* _buildExclusionShape() const;

public:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual void set(unsigned int key, const char* value);
	virtual Geom::Affine set_transform(Geom::Affine const& xform);

	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);

	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type) const;
	virtual void print(SPPrintContext *ctx);
        virtual const char* displayName() const;
	virtual char* description() const;
	virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
	virtual void hide(unsigned int key);
    virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;
};

SPItem *create_flowtext_with_internal_frame (SPDesktop *desktop, Geom::Point p1, Geom::Point p2);

#endif // SEEN_SP_ITEM_FLOWTEXT_H

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

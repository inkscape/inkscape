#ifndef INKSCAPE_SP_TEXTPATH_H
#define INKSCAPE_SP_TEXTPATH_H

#include "svg/svg-length.h"
#include "sp-item.h"
#include "sp-text.h"

class SPUsePath;
class Path;

#define SP_TEXTPATH(obj) (dynamic_cast<SPTextPath*>((SPObject*)obj))
#define SP_IS_TEXTPATH(obj) (dynamic_cast<const SPTextPath*>((SPObject*)obj) != NULL)

class SPTextPath : public SPItem {
public:
	SPTextPath();
	virtual ~SPTextPath();

    TextTagAttributes attributes;
    SVGLength startOffset;

    Path *originalPath;
    bool isUpdating;
    SPUsePath *sourcePath;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();
	virtual void set(unsigned int key, const char* value);
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

#define SP_IS_TEXT_TEXTPATH(obj) (SP_IS_TEXT(obj) && obj->firstChild() && SP_IS_TEXTPATH(obj->firstChild()))

SPItem *sp_textpath_get_path_item(SPTextPath *tp);
void sp_textpath_to_text(SPObject *tp);


#endif /* !INKSCAPE_SP_TEXTPATH_H */

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

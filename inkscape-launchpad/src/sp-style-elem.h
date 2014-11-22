#ifndef INKSCAPE_SP_STYLE_ELEM_H
#define INKSCAPE_SP_STYLE_ELEM_H

#include "sp-object.h"
#include "media.h"

class SPStyleElem : public SPObject {
public:
	SPStyleElem();
	virtual ~SPStyleElem();

    Media media;
    bool is_css;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void set(unsigned int key, char const* value);
	virtual void read_content();
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};


#endif /* !INKSCAPE_SP_STYLE_ELEM_H */

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

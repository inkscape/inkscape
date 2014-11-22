/** \file
 * SPRoot: SVG \<svg\> implementation.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_ROOT_H_SEEN
#define SP_ROOT_H_SEEN

#include "version.h"
#include "svg/svg-length.h"
#include "sp-item-group.h"
#include "viewbox.h"

#define SP_ROOT(obj) (dynamic_cast<SPRoot*>((SPObject*)obj))
#define SP_IS_ROOT(obj) (dynamic_cast<const SPRoot*>((SPObject*)obj) != NULL)

class SPDefs;

/** \<svg\> element */
class SPRoot : public SPGroup, public SPViewBox {
public:
	SPRoot();
	virtual ~SPRoot();

    struct {
        Inkscape::Version svg;
        Inkscape::Version inkscape;
    } version, original;

    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;

    char *onload;

    /**
     * Primary \<defs\> element where we put new defs (patterns, gradients etc.).
     *
     * At the time of writing, this is chosen as the first \<defs\> child of
     * this \<svg\> element: see writers of this member in sp-root.cpp.
     */
    SPDefs *defs;

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void set(unsigned int key, char const* value);
	virtual void update(SPCtx *ctx, unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

	virtual void modified(unsigned int flags);
	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
	virtual void print(SPPrintContext *ctx);
    virtual const char* displayName() const;
};

#endif /* !SP_ROOT_H_SEEN */

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

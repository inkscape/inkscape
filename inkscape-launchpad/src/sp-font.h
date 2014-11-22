#ifndef SP_FONT_H_SEEN
#define SP_FONT_H_SEEN

/*
 * SVG <font> element implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_FONT(obj) (dynamic_cast<SPFont*>((SPObject*)obj))
#define SP_IS_FONT(obj) (dynamic_cast<const SPFont*>((SPObject*)obj) != NULL)

class SPFont : public SPObject {
public:
	SPFont();
	virtual ~SPFont();

    double horiz_origin_x;
    double horiz_origin_y;
    double horiz_adv_x;
    double vert_origin_x;
    double vert_origin_y;
    double vert_adv_y;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual void set(unsigned int key, char const* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

#endif //#ifndef SP_FONT_H_SEEN

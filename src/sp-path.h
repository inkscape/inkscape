#ifndef SEEN_SP_PATH_H
#define SEEN_SP_PATH_H

/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ximian, Inc.
 *   Johan Engelen
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 1999-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"
#include "sp-conn-end-pair.h"

class SPCurve;

#define SP_PATH(obj) (dynamic_cast<SPPath*>((SPObject*)obj))
#define SP_IS_PATH(obj) (dynamic_cast<const SPPath*>((SPObject*)obj) != NULL)

/**
 * SVG <path> implementation
 */
class SPPath : public SPShape {
public:
	SPPath();
	virtual ~SPPath();

    int nodesInPath() const;

    // still in lowercase because the names should be clearer on whether curve, curve->copy or curve-ref is returned.
    void     set_original_curve (SPCurve *curve, unsigned int owner, bool write);
    SPCurve* get_original_curve () const;
    SPCurve* get_curve_for_edit () const;
    const SPCurve* get_curve_reference() const;

public: // should be made protected
    SPCurve* get_curve();
    friend class SPConnEndPair;

public:
    SPConnEndPair connEndPair;

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

        virtual const char* displayName() const;
	virtual char* description() const;
	virtual Geom::Affine set_transform(Geom::Affine const &transform);
    virtual void convert_to_guides() const;

    virtual void update_patheffect(bool write);
};

#endif // SEEN_SP_PATH_H

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

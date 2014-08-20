#ifndef __SP_SCRIPT_H__
#define __SP_SCRIPT_H__

/*
 * SVG <script> implementation
 *
 * Author:
 *   Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Author
 *
 * Released under GNU GPL version 2 or later, read the file 'COPYING' for more information
 */

#include "sp-object.h"
#include "document.h"

#define SP_SCRIPT(obj) (dynamic_cast<SPScript*>((SPObject*)obj))
#define SP_IS_SCRIPT(obj) (dynamic_cast<const SPScript*>((SPObject*)obj) != NULL)

/* SPScript */
class SPScript : public SPObject {
public:
	SPScript();
	virtual ~SPScript();

	gchar *xlinkhref;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();
	virtual void set(unsigned int key, const gchar* value);
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

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

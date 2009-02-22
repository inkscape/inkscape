#ifndef __SP_PATH_H__
#define __SP_PATH_H__

/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"
#include "sp-conn-end-pair.h"


#define SP_TYPE_PATH (sp_path_get_type ())
#define SP_PATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_PATH, SPPath))
#define SP_IS_PATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_PATH))

struct SPPath : public SPShape {
    SPCurve *original_curve;

    SPConnEndPair connEndPair;
};

struct SPPathClass {
    SPShapeClass shape_class;
};

GType sp_path_get_type (void);
gint sp_nodes_in_path(SPPath *path);

void     sp_path_set_original_curve (SPPath *path, SPCurve *curve, unsigned int owner, bool write);
SPCurve* sp_path_get_original_curve (SPPath *path);
SPCurve* sp_path_get_curve_for_edit (SPPath *path);
const SPCurve* sp_path_get_curve_reference (SPPath *path);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

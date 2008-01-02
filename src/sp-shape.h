#ifndef __SP_SHAPE_H__
#define __SP_SHAPE_H__

/*
 * Base class for shapes, including <path> element
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"
#include "sp-item.h"
#include "sp-marker-loc.h"

#include <sigc++/connection.h>

#define SP_TYPE_SHAPE (sp_shape_get_type ())
#define SP_SHAPE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_SHAPE, SPShape))
#define SP_SHAPE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_SHAPE, SPShapeClass))
#define SP_IS_SHAPE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_SHAPE))
#define SP_IS_SHAPE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_SHAPE))

#define SP_SHAPE_WRITE_PATH (1 << 2)

struct SPDesktop;
struct LivePathEffectObject;
namespace Inkscape{ 
namespace LivePathEffect{
    class LPEObjectReference;
    class Effect;
};
};


struct SPShape : public SPItem {
    SPCurve *curve;

      SPObject *marker[SP_MARKER_LOC_QTY];
      sigc::connection release_connect [SP_MARKER_LOC_QTY];
      sigc::connection modified_connect [SP_MARKER_LOC_QTY];

    gchar *path_effect_href;
    Inkscape::LivePathEffect::LPEObjectReference *path_effect_ref;
    sigc::connection lpe_modified_connection;
};

struct SPShapeClass {
	SPItemClass item_class;

	/* Build bpath from extra shape attributes */
	void (* set_shape) (SPShape *shape);

    void (* update_patheffect) (SPShape *shape, bool write);
};

GType sp_shape_get_type (void);

void sp_shape_set_shape (SPShape *shape);

/* Return duplicate of curve or NULL */
SPCurve *sp_shape_get_curve (SPShape *shape);

// sets a curve, updates display
void sp_shape_set_curve (SPShape *shape, SPCurve *curve, unsigned int owner);

// same as sp_shape_set_curve, but without updating display
void sp_shape_set_curve_insync (SPShape *shape, SPCurve *curve, unsigned int owner);

// markers API
void sp_shape_set_marker (SPObject *object, unsigned int key, const gchar *value);
int sp_shape_has_markers (SPShape const *shape);
int sp_shape_number_of_markers (SPShape* Shape, int type);
NR::Matrix sp_shape_marker_get_transform(SPShape const *shape, NArtBpath const *bp);
bool sp_shape_marker_required(SPShape const *shape, int const m, NArtBpath *bp);

LivePathEffectObject * sp_shape_get_livepatheffectobject(SPShape *shape);
Inkscape::LivePathEffect::Effect * sp_shape_get_livepatheffect(SPShape *shape);
void sp_shape_update_patheffect (SPShape *shape, bool write);
void sp_shape_perform_path_effect(SPCurve *curve, SPShape *shape);

void sp_shape_set_path_effect(SPShape *shape, gchar *value);
void sp_shape_set_path_effect(SPShape *shape, LivePathEffectObject * new_lpeobj);
void sp_shape_remove_path_effect(SPShape *shape);
bool sp_shape_has_path_effect(SPShape *shape);

void sp_shape_edit_next_param_oncanvas(SPShape *shape, SPDesktop *dt);

#endif

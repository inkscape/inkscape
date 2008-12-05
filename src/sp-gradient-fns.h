#ifndef SEEN_SP_GRADIENT_FNS_H
#define SEEN_SP_GRADIENT_FNS_H

/** \file
 * Macros and fn declarations related to gradients.
 */

#include <glib/gtypes.h>
#include <glib-object.h>
#include <2geom/forward.h>
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"

class SPGradient;

namespace Inkscape {
namespace XML {
class Node;
}
}

#define SP_TYPE_GRADIENT (sp_gradient_get_type())
#define SP_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GRADIENT, SPGradient))
#define SP_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GRADIENT, SPGradientClass))
#define SP_IS_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GRADIENT))
#define SP_IS_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GRADIENT))

#define SP_GRADIENT_STATE_IS_SET(g) (SP_GRADIENT(g)->state != SP_GRADIENT_STATE_UNKNOWN)
#define SP_GRADIENT_IS_VECTOR(g) (SP_GRADIENT(g)->state == SP_GRADIENT_STATE_VECTOR)
#define SP_GRADIENT_IS_PRIVATE(g) (SP_GRADIENT(g)->state == SP_GRADIENT_STATE_PRIVATE)
#define SP_GRADIENT_HAS_STOPS(g) (SP_GRADIENT(g)->has_stops)
#define SP_GRADIENT_SPREAD(g) (SP_GRADIENT(g)->spread)
#define SP_GRADIENT_UNITS(g) (SP_GRADIENT(g)->units)

GType sp_gradient_get_type();

/** Forces vector to be built, if not present (i.e. changed) */
void sp_gradient_ensure_vector(SPGradient *gradient);

/** Ensures that color array is populated */
void sp_gradient_ensure_colors(SPGradient *gradient);

void sp_gradient_set_units(SPGradient *gr, SPGradientUnits units);
void sp_gradient_set_spread(SPGradient *gr, SPGradientSpread spread);

SPGradient *sp_gradient_get_vector (SPGradient *gradient, bool force_private);
SPGradientSpread sp_gradient_get_spread (SPGradient *gradient);

/* Gradient repr methods */
void sp_gradient_repr_write_vector(SPGradient *gr);
void sp_gradient_repr_clear_vector(SPGradient *gr);

void sp_gradient_render_vector_block_rgba(SPGradient *gr, guchar *px, gint w, gint h, gint rs, gint pos, gint span, bool horizontal);
void sp_gradient_render_vector_block_rgb(SPGradient *gr, guchar *px, gint w, gint h, gint rs, gint pos, gint span, bool horizontal);

/** Transforms to/from gradient position space in given environment */
Geom::Matrix sp_gradient_get_g2d_matrix(SPGradient const *gr, Geom::Matrix const &ctm,
                                      Geom::Rect const &bbox);
Geom::Matrix sp_gradient_get_gs2d_matrix(SPGradient const *gr, Geom::Matrix const &ctm,
                                       Geom::Rect const &bbox);
void sp_gradient_set_gs2d_matrix(SPGradient *gr, Geom::Matrix const &ctm, Geom::Rect const &bbox,
                                 Geom::Matrix const &gs2d);


#endif /* !SEEN_SP_GRADIENT_FNS_H */

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

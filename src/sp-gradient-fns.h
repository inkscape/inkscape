#ifndef SEEN_SP_GRADIENT_FNS_H
#define SEEN_SP_GRADIENT_FNS_H

/** \file
 * Macros and fn declarations related to gradients.
 */

#include <glib.h>
#include <glib-object.h>
#include <2geom/forward.h>
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"

class SPGradient;
struct SPMeshGradient;

SPGradientSpread sp_gradient_get_spread (SPGradient *gradient);

/* Gradient repr methods */
void sp_gradient_repr_write_vector(SPGradient *gr);
void sp_gradient_repr_clear_vector(SPGradient *gr);

void sp_meshgradient_repr_write(SPMeshGradient *mg);

cairo_pattern_t *sp_gradient_create_preview_pattern(SPGradient *gradient, double width);

/** Transforms to/from gradient position space in given environment */
Geom::Affine sp_gradient_get_g2d_matrix(SPGradient const *gr, Geom::Affine const &ctm,
                                      Geom::Rect const &bbox);
Geom::Affine sp_gradient_get_gs2d_matrix(SPGradient const *gr, Geom::Affine const &ctm,
                                       Geom::Rect const &bbox);
void sp_gradient_set_gs2d_matrix(SPGradient *gr, Geom::Affine const &ctm, Geom::Rect const &bbox,
                                 Geom::Affine const &gs2d);


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

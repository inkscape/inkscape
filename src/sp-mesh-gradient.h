#ifndef SP_MESH_GRADIENT_H
#define SP_MESH_GRADIENT_H

/** \file
 * SPMeshGradient: SVG <meshgradient> implementation.
 */

#include "svg/svg-length.h"
#include "sp-gradient.h"
#include "sp-mesh-gradient-fns.h"

/** Mesh gradient. */
struct SPMeshGradient : public SPGradient {
    SVGLength x;  // Upper left corner of mesh
    SVGLength y;  // Upper right corner of mesh
};

/// The SPMeshGradient vtable.
struct SPMeshGradientClass {
    SPGradientClass parent_class;
};


#endif /* !SP_MESH_GRADIENT_H */

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

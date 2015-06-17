#ifndef SP_MESH_H
#define SP_MESH_H

/** \file
 * SPMesh: SVG <mesh> implementation.
 */

#include "svg/svg-length.h"
#include "sp-gradient.h"

#define SP_MESH(obj) (dynamic_cast<SPMesh*>((SPObject*)obj))
#define SP_IS_MESH(obj) (dynamic_cast<const SPMesh*>((SPObject*)obj) != NULL)

/** Mesh gradient. */
class SPMesh : public SPGradient {
public:
	SPMesh();
	virtual ~SPMesh();

    SVGLength x;  // Upper left corner of mesh
    SVGLength y;  // Upper right corner of mesh
    SPMeshType type;
    bool type_set;
    virtual cairo_pattern_t* pattern_new(cairo_t *ct, Geom::OptRect const &bbox, double opacity);

protected:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void set(unsigned key, char const *value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

#endif /* !SP_MESH_H */

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

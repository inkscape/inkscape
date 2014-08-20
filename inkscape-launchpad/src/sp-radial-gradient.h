#ifndef SP_RADIAL_GRADIENT_H
#define SP_RADIAL_GRADIENT_H

/** \file
 * SPRadialGradient: SVG <radialgradient> implementtion.
 */

#include <glib.h>
#include "sp-gradient.h"
#include "svg/svg-length.h"

#define SP_RADIALGRADIENT(obj) (dynamic_cast<SPRadialGradient*>((SPObject*)obj))
#define SP_IS_RADIALGRADIENT(obj) (dynamic_cast<const SPRadialGradient*>((SPObject*)obj) != NULL)

/** Radial gradient. */
class SPRadialGradient : public SPGradient {
public:
	SPRadialGradient();
	virtual ~SPRadialGradient();

    SVGLength cx;
    SVGLength cy;
    SVGLength r;
    SVGLength fx;
    SVGLength fy;

    virtual cairo_pattern_t* pattern_new(cairo_t *ct, Geom::OptRect const &bbox, double opacity);

protected:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void set(unsigned key, gchar const *value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);
};

#endif /* !SP_RADIAL_GRADIENT_H */

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

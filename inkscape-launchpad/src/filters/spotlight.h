#ifndef SP_FESPOTLIGHT_H_SEEN
#define SP_FESPOTLIGHT_H_SEEN

/** \file
 * SVG <filter> implementation, see sp-filter.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_FESPOTLIGHT(obj) (dynamic_cast<SPFeSpotLight*>((SPObject*)obj))
#define SP_IS_FESPOTLIGHT(obj) (dynamic_cast<const SPFeSpotLight*>((SPObject*)obj) != NULL)

class SPFeSpotLight : public SPObject {
public:
	SPFeSpotLight();
	virtual ~SPFeSpotLight();

    /** x coordinate of the light source */
    float x; 
    unsigned int x_set : 1;
    /** y coordinate of the light source */
    float y; 
    unsigned int y_set : 1;
    /** z coordinate of the light source */
    float z; 
    unsigned int z_set : 1;
    /** x coordinate of the point the source is pointing at */
    float pointsAtX;
    unsigned int pointsAtX_set : 1;
    /** y coordinate of the point the source is pointing at */
    float pointsAtY;
    unsigned int pointsAtY_set : 1;
    /** z coordinate of the point the source is pointing at */
    float pointsAtZ;
    unsigned int pointsAtZ_set : 1;
    /** specular exponent (focus of the light) */
    float specularExponent;
    unsigned int specularExponent_set : 1;
    /** limiting cone angle */
    float limitingConeAngle;
    unsigned int limitingConeAngle_set : 1;
    //other fields

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, char const* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

#endif /* !SP_FESPOTLIGHT_H_SEEN */

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

#ifndef __NR_LIGHT_H__
#define __NR_LIGHT_H__
/** \file
 * These classes provide tools to compute interesting objects relative to light
 * sources. Each class provides a constructor converting information contained
 * in a sp light object into information useful in the current setting, a
 * method to get the light vector (at a given point) and a method to get the 
 * light color components (at a given point).
 */

#include <gdk/gdktypes.h>
#include "display/nr-3dutils.h"
#include "display/nr-light-types.h"
#include <2geom/forward.h>

struct SPFeDistantLight;
struct SPFePointLight;
struct SPFeSpotLight;

namespace Inkscape {
namespace Filters {

enum LightComponent {
    LIGHT_RED = 0,
    LIGHT_GREEN,
    LIGHT_BLUE
};

class DistantLight {
    public:
        /**
         * Constructor
         *
         * \param light the sp light object
         * \param lighting_color the lighting_color used
         */
        DistantLight(SPFeDistantLight *light, guint32 lighting_color);
        virtual ~DistantLight();
        
        /**
         * Computes the light vector of the distant light
         *
         * \param v a Fvector referece where we store the result
         */
        void light_vector(NR::Fvector &v);
        
        /**
         * Computes the light components of the distant light
         *
         * \param lc a Fvector referece where we store the result, X=R, Y=G, Z=B
         */
        void light_components(NR::Fvector &lc);

    private:
        guint32 color;
        gdouble azimuth; //azimuth in rad
        gdouble elevation; //elevation in rad
};

class PointLight {
    public:
        /**
         * Constructor
         *
         * \param light the sp light object
         * \param lighting_color the lighting_color used
         * \param trans the transformation between absolute coordinate (those
         * employed in the sp light object) and current coordinate (those
         * employed in the rendering)
         */
        PointLight(SPFePointLight *light, guint32 lighting_color, const Geom::Matrix &trans);
        virtual ~PointLight();
        /**
         * Computes the light vector of the distant light at point (x,y,z).
         * x, y and z are given in the arena_item coordinate, they are used as
         * is
         *
         * \param v a Fvector referece where we store the result
         * \param x x coordinate of the current point
         * \param y y coordinate of the current point
         * \param z z coordinate of the current point
         */
        void light_vector(NR::Fvector &v, gdouble x, gdouble y, gdouble z);
        
        /**
         * Computes the light components of the distant light
         *
         * \param lc a Fvector referece where we store the result, X=R, Y=G, Z=B
         */
        void light_components(NR::Fvector &lc);

    private:
        guint32 color;
        //light position coordinates in render setting
        gdouble l_x;
        gdouble l_y;
        gdouble l_z;
};

class SpotLight {
    public:
        /**
         * Constructor
         *
         * \param light the sp light object
         * \param lighting_color the lighting_color used
         * \param trans the transformation between absolute coordinate (those
         * employed in the sp light object) and current coordinate (those
         * employed in the rendering)
         */
        SpotLight(SPFeSpotLight *light, guint32 lighting_color, const Geom::Matrix &trans);
        virtual ~SpotLight();

        /**
         * Computes the light vector of the distant light at point (x,y,z).
         * x, y and z are given in the arena_item coordinate, they are used as
         * is
         *
         * \param v a Fvector referece where we store the result
         * \param x x coordinate of the current point
         * \param y y coordinate of the current point
         * \param z z coordinate of the current point
         */
        void light_vector(NR::Fvector &v, gdouble x, gdouble y, gdouble z);

        /**
         * Computes the light components of the distant light at the current
         * point. We only need the light vector to compute theses
         *
         * \param lc a Fvector referece where we store the result, X=R, Y=G, Z=B
         * \param L the light vector of the current point
         */
        void light_components(NR::Fvector &lc, const NR::Fvector &L);

    private:
        guint32 color;
        //light position coordinates in render setting
        gdouble l_x;
        gdouble l_y;
        gdouble l_z;
        gdouble cos_lca; //cos of the limiting cone angle
        gdouble speExp; //specular exponent;
        NR::Fvector S; //unit vector from light position in the direction
                   //the spot point at
};


} /* namespace Filters */
} /* namespace Inkscape */

#endif // __NR_LIGHT_H__
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

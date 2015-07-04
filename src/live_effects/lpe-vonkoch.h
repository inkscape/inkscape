#ifndef INKSCAPE_LPE_VONKOCH_H
#define INKSCAPE_LPE_VONKOCH_H

/*
 * Inkscape::LPEVonKoch
 *
 * Copyright (C) JF Barraud 2007 <jf.barraud@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/lpegroupbbox.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/point.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

class VonKochPathParam : public PathParam{
public:
    VonKochPathParam ( const Glib::ustring& label,
		       const Glib::ustring& tip,
		       const Glib::ustring& key,
		       Inkscape::UI::Widget::Registry* wr,
		       Effect* effect,
		       const gchar * default_value = "M0,0 L1,1"):PathParam(label,tip,key,wr,effect,default_value){}
    virtual ~VonKochPathParam(){}
    virtual void param_setup_nodepath(Inkscape::NodePath::Path *np);  
  };

  //FIXME: a path is used here instead of 2 points to work around path/point param incompatibility bug.
class VonKochRefPathParam : public PathParam{
public:
    VonKochRefPathParam ( const Glib::ustring& label,
		       const Glib::ustring& tip,
		       const Glib::ustring& key,
		       Inkscape::UI::Widget::Registry* wr,
		       Effect* effect,
		       const gchar * default_value = "M0,0 L1,1"):PathParam(label,tip,key,wr,effect,default_value){}
    virtual ~VonKochRefPathParam(){}
    virtual void param_setup_nodepath(Inkscape::NodePath::Path *np);  
    virtual bool param_readSVGValue(const gchar * strvalue);  
  };
 
class LPEVonKoch : public Effect, GroupBBoxEffect {
public:
    LPEVonKoch(LivePathEffectObject *lpeobject);
    virtual ~LPEVonKoch();

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

    virtual void resetDefaults(SPItem const* item);

    virtual void doBeforeEffect(SPLPEItem const* item);

    //Usefull??
    //    protected: 
    //virtual void addCanvasIndicators(SPLPEItem const *lpeitem, std::vector<Geom::PathVector> &hp_vec); 

private:
    ScalarParam  nbgenerations;
    VonKochPathParam    generator;
    BoolParam    similar_only;
    BoolParam    drawall;
    //BoolParam    draw_boxes;
    //FIXME: a path is used here instead of 2 points to work around path/point param incompatibility bug.
    VonKochRefPathParam    ref_path;
    //    PointParam   refA;
    //    PointParam   refB;
    ScalarParam  maxComplexity;

    LPEVonKoch(const LPEVonKoch&);
    LPEVonKoch& operator=(const LPEVonKoch&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif

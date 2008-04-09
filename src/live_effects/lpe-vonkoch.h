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
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/bool.h"

#include "live_effects/n-art-bpath-2geom.h"

// needed for on-canvas editting:
#include "tools-switch.h"
#include "shape-editor.h"
#include "node-context.h"
#include "desktop-handles.h"
#include "selection.h"
#include "nodepath.h"


namespace Inkscape {
namespace LivePathEffect {

enum VonKochRefType {
    VKREF_BBOX = 0,
    VKREF_SEG,
    VKREF_END // This must be last
};

class VonKochPathParam : public PathParam{
public:
    VonKochPathParam ( const Glib::ustring& label,
		       const Glib::ustring& tip,
		       const Glib::ustring& key,
		       Inkscape::UI::Widget::Registry* wr,
		       Effect* effect,
		       const gchar * default_value = "M0,0 L1,1"):PathParam(label,tip,key,wr,effect,default_value){};
    virtual ~VonKochPathParam();
    virtual void param_setup_nodepath(Inkscape::NodePath::Path *np);  
  };

class LPEVonKoch : public Effect {
public:
    LPEVonKoch(LivePathEffectObject *lpeobject);
    virtual ~LPEVonKoch();

    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

    virtual void resetDefaults(SPItem * item);

    virtual void transform_multiply(Geom::Matrix const& postmul, bool set);

private:
    ScalarParam  nbgenerations;
    VonKochPathParam    generator;
    BoolParam    drawall;
    EnumParam<VonKochRefType> reftype;
    ScalarParam  maxComplexity;

    //void on_pattern_pasted();

    LPEVonKoch(const LPEVonKoch&);
    LPEVonKoch& operator=(const LPEVonKoch&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif

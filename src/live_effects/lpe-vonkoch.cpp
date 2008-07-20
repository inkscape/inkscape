#define INKSCAPE_LPE_VONKOCH_CPP

/*
 * Copyright (C) JF Barraud 2007 <jf.barraud@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdio>

#include "live_effects/lpe-vonkoch.h"
#include "svg/svg.h"
#include "ui/widget/scalar.h"
#include "nodepath.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include <algorithm>
using std::vector;


namespace Inkscape {
namespace LivePathEffect {

VonKochPathParam::~VonKochPathParam()
{  
}

void
VonKochPathParam::param_setup_nodepath(Inkscape::NodePath::Path *np)
{  
    PathParam::param_setup_nodepath(np);
    sp_nodepath_make_straight_path(np);
}

static const Util::EnumData<VonKochRefType> VonKochRefTypeData[VKREF_END] = {
    {VKREF_BBOX,     N_("Bounding box"),               "bbox"},
    {VKREF_SEG, N_("Last gen. segment"), "lastseg"},
};
static const Util::EnumDataConverter<VonKochRefType> VonKochRefTypeConverter(VonKochRefTypeData, VKREF_END);

LPEVonKoch::LPEVonKoch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    nbgenerations(_("Nb of generations"), _("Depth of the recursion --- keep low!!"), "nbgenerations", &wr, this, 1),
    generator(_("Generating path"), _("Path whos segments define the fractal"), "generator", &wr, this, "M0,0 L3,0 M0,1 L1,1 M 2,1 L3,1"),
    drawall(_("Draw all generations"), _("If unchecked, draw only the last generation"), "drawall", &wr, this, true),
    reftype(_("Reference"), _("Generating path segments define transforms in reference to bbox or last segment"), "reftype", VonKochRefTypeConverter, &wr, this, VKREF_BBOX),
    maxComplexity(_("Max complexity"), _("Disable effect if the output is too complex"), "maxComplexity", &wr, this, 1000)
{
    registerParameter( dynamic_cast<Parameter *>(&generator) );
    registerParameter( dynamic_cast<Parameter *>(&nbgenerations) );
    registerParameter( dynamic_cast<Parameter *>(&drawall) );
    registerParameter( dynamic_cast<Parameter *>(&reftype) );
    registerParameter( dynamic_cast<Parameter *>(&maxComplexity) );

    nbgenerations.param_make_integer();
    nbgenerations.param_set_range(0, NR_HUGE);
    maxComplexity.param_make_integer();
    maxComplexity.param_set_range(0, NR_HUGE);
}

LPEVonKoch::~LPEVonKoch()
{

}

std::vector<Geom::Path>
LPEVonKoch::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    using namespace Geom;
    std::vector<Geom::Path> generating_path = path_from_piecewise(generator.get_pwd2(),.01);//TODO what should that tolerance be?

    //Collect transform matrices.
    //FIXME: fusing/cutting nodes mix up component order in the path. This is why the last segment is used.
    Matrix m0;
    VonKochRefType type = reftype.get_value();
    if (type==VKREF_BBOX){
        Rect bbox = path_in[0].boundsExact();
        for(unsigned i=1; i < path_in.size(); i++){
            bbox.unionWith(path_in[i].boundsExact());
        }
        m0 = Matrix(bbox[X].extent(),0,0,bbox[X].extent(), bbox.min()[X], (bbox.min()[Y]+bbox.max()[Y])/2);
    }else{
        if (generating_path.size()==0) return path_in;
        Point p = generating_path.back().back().pointAt(0);
        Point u = generating_path.back().back().pointAt(0.999)-p;
        m0 = Matrix(u[X], u[Y],-u[Y], u[X], p[X], p[Y]);
    }
    m0 = m0.inverse();

    std::vector<Matrix> transforms;
    for (unsigned i=0; i<generating_path.size(); i++){
        unsigned end = generating_path[i].size(); 
        if(type==VKREF_SEG && i==generating_path.size()-1) end-=1;
        for (unsigned j=0; j<end; j++){   
            Point p = generating_path[i].pointAt(j);
            Point u = generating_path[i].pointAt(j+0.999)-generating_path[i].pointAt(j+0.001);
            Matrix m = Matrix(u[X], u[Y],-u[Y], u[X], p[X], p[Y]);
            m = m0*m;
            transforms.push_back(m);
        }
    }
    if (transforms.size()==0) return path_in;

    //Do nothing if the output is too complex... 
    int path_in_complexity = 0;
    for (unsigned k = 0; k < path_in.size(); k++){
            path_in_complexity+=path_in[k].size();
    }    
    double complexity = pow(transforms.size(),nbgenerations)*path_in_complexity;
    if (drawall.get_value()){
        int k = transforms.size();
        if(k>1){
            complexity = (pow(k,nbgenerations+1)-1)/(k-1)*path_in_complexity;
        }else{
            complexity = nbgenerations*k*path_in_complexity;
        }
    }else{
        complexity = pow(transforms.size(),nbgenerations)*path_in_complexity;
    }
    if (complexity > double(maxComplexity)){
        return path_in;
    }

    //Generate path:
    std::vector<Geom::Path> pathi = path_in;
    std::vector<Geom::Path> path_out = path_in;
    
    for (unsigned i = 0; i<nbgenerations; i++){
        if (drawall.get_value()){
            path_out =  path_in;
            complexity = path_in_complexity;
        }else{
            path_out = std::vector<Geom::Path>();
            complexity = 0;
        }
        for (unsigned j = 0; j<transforms.size(); j++){
            for (unsigned k = 0; k<pathi.size() && complexity < maxComplexity; k++){
                path_out.push_back(pathi[k]*transforms[j]); 
                complexity+=pathi[k].size();
            }
        }
        pathi = path_out;
    }
    return path_out;
}

void
LPEVonKoch::resetDefaults(SPItem * item)
{
    if (!SP_IS_PATH(item)) return;

    using namespace Geom;

    // set the bend path to run horizontally in the middle of the bounding box of the original path
    Piecewise<D2<SBasis> > pwd2;
    std::vector<Geom::Path> temppath = sp_svg_read_pathv( SP_OBJECT_REPR(item)->attribute("inkscape:original-d"));
    for (unsigned int i=0; i < temppath.size(); i++) {
        pwd2.concat( temppath[i].toPwSb() );
    }

    D2<Piecewise<SBasis> > d2pw = make_cuts_independent(pwd2);
    Interval bndsX = bounds_exact(d2pw[0]);
    Interval bndsY = bounds_exact(d2pw[1]);
    Point start(bndsX.min(), (bndsY.max()+bndsY.min())/2);
    Point end(bndsX.max(), (bndsY.max()+bndsY.min())/2);

    std::vector<Geom::Path> paths;
    Geom::Path path;
    path = Geom::Path();
    path.start( start );
    path.appendNew<Geom::LineSegment>( end );
    paths.push_back(path * Matrix(1./3,0,0,1./3,start[X]*2./3,start[Y]*2./3 + bndsY.extent()/2));
    paths.push_back(path * Matrix(1./3,0,0,1./3,  end[X]*2./3,  end[Y]*2./3 + bndsY.extent()/2));
    paths.push_back(path);

    generator.set_new_value(paths, true);

    


//     Piecewise<D2<SBasis> > default_gen;
//     default_gen.concat(Piecewise<D2<SBasis> >(D2<SBasis>(Linear(bndsX.min(),bndsX.max()),Linear((bndsY.min()+bndsY.max())/2))));
//     default_gen.concat(Piecewise<D2<SBasis> >(D2<SBasis>(Linear(bndsX.max(),bndsX.max()+bndsX.extent()/2),Linear((bndsY.min()+bndsY.max())/2))));
//     generator.set_new_value(default_gen, true);
}

void
LPEVonKoch::transform_multiply(Geom::Matrix const& postmul, bool set)
{
    // TODO: implement correct transformation instead of this default behavior
    Effect::transform_multiply(postmul, set);
}


} // namespace LivePathEffect
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

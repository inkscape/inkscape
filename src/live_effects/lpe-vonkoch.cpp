#define INKSCAPE_LPE_VONKOCH_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-vonkoch.h"
#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-fns.h>
#include "live_effects/n-art-bpath-2geom.h"
#include "svg/svg.h"
#include "ui/widget/scalar.h"

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

LPEVonKoch::LPEVonKoch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    generator(_("Generating path"), _("Path whos segments define the fractal"), "generator", &wr, this, "M0,0 L3,0 M0,1 L1,1 M 2,1 L3,1"),
    nbgenerations(_("Nb of generations"), _("Depth of the recursion --- keep low!!"), "nbgenerations", &wr, this, 1),
    drawall(_("Draw all generations"), _("If unchecked, draw only the last generation"), "drawall", &wr, this, false),
    vertical_pattern(_("Original path is vertical"), _("Rotates the original 90 degrees, before generating the fractal"), "vertical", &wr, this, false)
{
    registerParameter( dynamic_cast<Parameter *>(&generator) );
    registerParameter( dynamic_cast<Parameter *>(&nbgenerations) );
    registerParameter( dynamic_cast<Parameter *>(&drawall) );
    registerParameter( dynamic_cast<Parameter *>(&vertical_pattern) );

    nbgenerations.param_make_integer();
    nbgenerations.param_set_range(0, NR_HUGE);
}

LPEVonKoch::~LPEVonKoch()
{

}


std::vector<Geom::Path>
LPEVonKoch::doEffect_path (std::vector<Geom::Path> & path_in)
{
    using namespace Geom;
    std::vector<Geom::Path> generating_path = path_from_piecewise(generator,.01);//TODO what should that tolerance be?
    Point p = generating_path[0].pointAt(0.001);
    Point u = generating_path[0].pointAt(0.999)-generating_path[0].pointAt(0.001);
    Matrix m0 = Matrix(u[X], u[Y],-u[Y], u[X], p[X], p[Y]);
    m0 = m0.inverse();

    std::vector<Matrix> transforms;
    for (unsigned i=0; i<generating_path.size(); i++){
        for (unsigned j = (i==0)?1:0; j<generating_path[i].size(); j++){   
            p = generating_path[i].pointAt(j);
            u = generating_path[i].pointAt(j+0.999)-generating_path[i].pointAt(j+0.001);
            Matrix m = Matrix(u[X], u[Y],-u[Y], u[X], p[X], p[Y]);
            m = m0*m;
            transforms.push_back(m);
        }
    }
/*
    assert(generating_path.size()>0);

    std::vector<Matrix> transforms;
    transforms.push_back(Matrix(.5, 0., 0., .5,   0.,50.));
    transforms.push_back(Matrix(.5, 0., 0., .5, 100., 0.));
*/
    
    std::vector<Geom::Path> pathi = path_in;
    std::vector<Geom::Path> path_out = path_in;
   
    for (unsigned i = 0; i<nbgenerations; i++){
        path_out = (drawall.get_value())? path_in : std::vector<Geom::Path>();
        for (unsigned j = 0; j<transforms.size(); j++){
            for (unsigned k = 0; k<pathi.size(); k++){
                path_out.push_back(pathi[k]*transforms[j]); 
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
    std::vector<Geom::Path> temppath = SVGD_to_2GeomPath( SP_OBJECT_REPR(item)->attribute("inkscape:original-d"));
    for (unsigned int i=0; i < temppath.size(); i++) {
        pwd2.concat( temppath[i].toPwSb() );
    }

    D2<Piecewise<SBasis> > d2pw = make_cuts_independant(pwd2);
    Interval bndsX = bounds_exact(d2pw[0]);
    Interval bndsY = bounds_exact(d2pw[1]);
    Point start(bndsX.min(), (bndsY.max()+bndsY.min())/2);
    Point end(bndsX.max(), (bndsY.max()+bndsY.min())/2);

    std::vector<Geom::Path> paths;
    Geom::Path path;
    path = Geom::Path();
    path.start( start );
    path.appendNew<Geom::LineSegment>( end );
    paths.push_back(path);
    paths.push_back(path * Matrix(1./3,0,0,1./3,start[X]*2./3,start[Y]*2./3 + bndsY.extent()/2));
    paths.push_back(path * Matrix(1./3,0,0,1./3,  end[X]*2./3,  end[Y]*2./3 + bndsY.extent()/2));

    //generator.param_set_and_write_new_value( path.toPwSb() );
    generator.param_set_and_write_new_value( paths_to_pw(paths) );


//     Piecewise<D2<SBasis> > default_gen;
//     default_gen.concat(Piecewise<D2<SBasis> >(D2<SBasis>(Linear(bndsX.min(),bndsX.max()),Linear((bndsY.min()+bndsY.max())/2))));
//     default_gen.concat(Piecewise<D2<SBasis> >(D2<SBasis>(Linear(bndsX.max(),bndsX.max()+bndsX.extent()/2),Linear((bndsY.min()+bndsY.max())/2))));
//     generator.param_set_and_write_new_value( default_gen );
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

/*
 * Copyright (C) JF Barraud 2007 <jf.barraud@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-vonkoch.h"

#include <glibmm/i18n.h>

#include <2geom/transforms.h>

//using std::vector;
namespace Inkscape {
namespace LivePathEffect {

void
VonKochPathParam::param_setup_nodepath(Inkscape::NodePath::Path *np)
{  
    PathParam::param_setup_nodepath(np);
    //sp_nodepath_make_straight_path(np);
}

//FIXME: a path is used here instead of 2 points to work around path/point param incompatibility bug.
void
VonKochRefPathParam::param_setup_nodepath(Inkscape::NodePath::Path *np)
{  
    PathParam::param_setup_nodepath(np);
    //sp_nodepath_make_straight_path(np);
}
bool
VonKochRefPathParam::param_readSVGValue(const gchar * strvalue)
{  
    Geom::PathVector old = _pathvector;
    bool res = PathParam::param_readSVGValue(strvalue);
    if (res && _pathvector.size()==1 && _pathvector.front().size()==1){
        return true;
    }else{
        _pathvector = old;
        return false;
    }
}

LPEVonKoch::LPEVonKoch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    nbgenerations(_("N_r of generations:"), _("Depth of the recursion --- keep low!!"), "nbgenerations", &wr, this, 1),
    generator(_("Generating path:"), _("Path whose segments define the iterated transforms"), "generator", &wr, this, "M0,0 L30,0 M0,10 L10,10 M 20,10 L30,10"),
    similar_only(_("_Use uniform transforms only"), _("2 consecutive segments are used to reverse/preserve orientation only (otherwise, they define a general transform)."), "similar_only", &wr, this, false),
    drawall(_("Dra_w all generations"), _("If unchecked, draw only the last generation"), "drawall", &wr, this, true),
    //,draw_boxes(_("Display boxes"), _("Display boxes instead of paths only"), "draw_boxes", &wr, this, true)
    ref_path(_("Reference segment:"), _("The reference segment. Defaults to the horizontal midline of the bbox."), "ref_path", &wr, this, "M0,0 L10,0"),
    //refA(_("Ref Start"), _("Left side middle of the reference box"), "refA", &wr, this),
    //refB(_("Ref End"), _("Right side middle of the reference box"), "refB", &wr, this),
    //FIXME: a path is used here instead of 2 points to work around path/point param incompatibility bug.
    maxComplexity(_("_Max complexity:"), _("Disable effect if the output is too complex"), "maxComplexity", &wr, this, 1000)
{
    //FIXME: a path is used here instead of 2 points to work around path/point param incompatibility bug.
    registerParameter( dynamic_cast<Parameter *>(&ref_path) );
    //registerParameter( dynamic_cast<Parameter *>(&refA) );
    //registerParameter( dynamic_cast<Parameter *>(&refB) );
    registerParameter( dynamic_cast<Parameter *>(&generator) );
    registerParameter( dynamic_cast<Parameter *>(&similar_only) );
    registerParameter( dynamic_cast<Parameter *>(&nbgenerations) );
    registerParameter( dynamic_cast<Parameter *>(&drawall) );
    registerParameter( dynamic_cast<Parameter *>(&maxComplexity) );
    //registerParameter( dynamic_cast<Parameter *>(&draw_boxes) );
    apply_to_clippath_and_mask = true;
    nbgenerations.param_make_integer();
    nbgenerations.param_set_range(0, Geom::infinity());
    maxComplexity.param_make_integer();
    maxComplexity.param_set_range(0, Geom::infinity());
}

LPEVonKoch::~LPEVonKoch()
{

}

Geom::PathVector
LPEVonKoch::doEffect_path (Geom::PathVector const & path_in)
{
    using namespace Geom;

    Geom::PathVector generating_path = generator.get_pathvector();
    
    if (generating_path.empty()) {
        return path_in;
    }

    //Collect transform matrices.
    Affine m0;
    Geom::Path refpath = ref_path.get_pathvector().front();
    Point A = refpath.pointAt(0);
    Point B = refpath.pointAt(refpath.size());
    Point u = B-A;
    m0 = Affine(u[X], u[Y],-u[Y], u[X], A[X], A[Y]);
    
    //FIXME: a path is used as ref instead of 2 points to work around path/point param incompatibility bug.
    //Point u = refB-refA;
    //m0 = Affine(u[X], u[Y],-u[Y], u[X], refA[X], refA[Y]);
    m0 = m0.inverse();

    std::vector<Affine> transforms;
    for (unsigned i=0; i<generating_path.size(); i++){
        Affine m;
        if(generating_path[i].size()==1){
            Point p = generating_path[i].pointAt(0);
            Point u = generating_path[i].pointAt(1)-p;
            m = Affine(u[X], u[Y],-u[Y], u[X], p[X], p[Y]);
            m = m0*m;
            transforms.push_back(m);
        }else if(generating_path[i].size()>=2){
            Point p = generating_path[i].pointAt(1);
            Point u = generating_path[i].pointAt(2)-p;
            Point v = p-generating_path[i].pointAt(0);
            if (similar_only.get_value()){
                int sign = (u[X]*v[Y]-u[Y]*v[X]>=0?1:-1);
                v[X] = -u[Y]*sign;
                v[Y] =  u[X]*sign;
            }
            m = Affine(u[X], u[Y],v[X], v[Y], p[X], p[Y]);
            m = m0*m;
            transforms.push_back(m);
        }
    }

    if (transforms.empty()){
        return path_in;
    }

    //Do nothing if the output is too complex... 
    int path_in_complexity = 0;
    for (unsigned k = 0; k < path_in.size(); k++){
            path_in_complexity+=path_in[k].size();
    }
    double complexity = std::pow(transforms.size(), nbgenerations) * path_in_complexity;
    if (drawall.get_value()){
        int k = transforms.size();
        if(k>1){
            complexity = (std::pow(k,nbgenerations+1)-1)/(k-1)*path_in_complexity;
        }else{
            complexity = nbgenerations*k*path_in_complexity;
        }
    }
    if (complexity > double(maxComplexity)){
        g_warning("VonKoch lpe's output too complex. Effect bypassed.");
        return path_in;
    }

    //Generate path:
    Geom::PathVector pathi = path_in;
    Geom::PathVector path_out = path_in;
    
    for (unsigned i = 0; i<nbgenerations; i++){
        if (drawall.get_value()){
            path_out =  path_in;
            complexity = path_in_complexity;
        }else{
            path_out = Geom::PathVector();
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


//Usefull?? 
//void 
//LPEVonKoch::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
/*{
    using namespace Geom;
    if (draw_boxes.get_value()){
        double ratio = .5;
        if (similar_only.get_value()) ratio = boundingbox_Y.extent()/boundingbox_X.extent()/2;
        
        Point BB1,BB2,BB3,BB4,v;
        
        //Draw the reference box  (ref_path is supposed to consist in one line segment)
        //FIXME: a path is used as ref instead of 2 points to work around path/point param incompatibility bug.
        Geom::Path refpath = ref_path.get_pathvector().front();
        if (refpath.size()==1){
            BB1 = BB4 = refpath.front().pointAt(0);
            BB2 = BB3 = refpath.front().pointAt(1);
            v = rot90(BB2 - BB1)*ratio;
            BB1 -= v;
            BB2 -= v;
            BB3 += v;
            BB4 += v;
            Geom::Path refbox(BB1);
            refbox.appendNew<LineSegment>(BB2);
            refbox.appendNew<LineSegment>(BB3);
            refbox.appendNew<LineSegment>(BB4);
            refbox.close();
            PathVector refbox_as_vect;
            refbox_as_vect.push_back(refbox);
            hp_vec.push_back(refbox_as_vect);
        }
        //Draw the transformed boxes
        Geom::PathVector generating_path = generator.get_pathvector();
        for (unsigned i=0;i<generating_path.size(); i++){
            if (generating_path[i].size()==0){
                //Ooops! this should not happen.
            }else if (generating_path[i].size()==1){
                BB1 = BB4 = generating_path[i].pointAt(0);
                BB2 = BB3 = generating_path[i].pointAt(1);
                v = rot90(BB2 - BB1)*ratio;
            }else{//Only tak the first 2 segments into account.
                BB1 = BB4 = generating_path[i].pointAt(1);
                BB2 = BB3 = generating_path[i].pointAt(2);
                if(similar_only.get_value()){
                    v = rot90(BB2 - BB1)*ratio;
            }else{
                    v = (generating_path[i].pointAt(0) - BB1)*ratio;
                }
            }
            BB1 -= v;
            BB2 -= v;
            BB3 += v;
            BB4 += v;
            Geom::Path path(BB1);
            path.appendNew<LineSegment>(BB2);
            path.appendNew<LineSegment>(BB3);
            path.appendNew<LineSegment>(BB4);
            path.close();
            PathVector pathv;
            pathv.push_back(path);
            hp_vec.push_back(pathv);
        }
    }
}
*/

void
LPEVonKoch::doBeforeEffect (SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);
    
    Geom::PathVector paths = ref_path.get_pathvector();
    Geom::Point A,B;
    if (paths.empty()||paths.front().size()==0){
        //FIXME: a path is used as ref instead of 2 points to work around path/point param incompatibility bug.
        //refA.param_setValue( Geom::Point(boundingbox_X.min(), boundingbox_Y.middle()) );
        //refB.param_setValue( Geom::Point(boundingbox_X.max(), boundingbox_Y.middle()) );
        A = Point(boundingbox_X.min(), boundingbox_Y.middle());
        B = Point(boundingbox_X.max(), boundingbox_Y.middle());
    }else{
        A = paths.front().pointAt(0);
        B = paths.front().pointAt(paths.front().size());
    }
    if (paths.size()!=1||paths.front().size()!=1){
        Geom::Path tmp_path(A);
        tmp_path.appendNew<LineSegment>(B);
        Geom::PathVector tmp_pathv;
        tmp_pathv.push_back(tmp_path);
        ref_path.set_new_value(tmp_pathv,true);
    }
}


void
LPEVonKoch::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);

    using namespace Geom;
    original_bbox(SP_LPE_ITEM(item));

    Point A,B;
    A[Geom::X] = boundingbox_X.min();
    A[Geom::Y] = boundingbox_Y.middle();
    B[Geom::X] = boundingbox_X.max();
    B[Geom::Y] = boundingbox_Y.middle();

    Geom::PathVector paths,refpaths;
    Geom::Path path = Geom::Path(A);
    path.appendNew<Geom::LineSegment>(B);

    refpaths.push_back(path);
    ref_path.set_new_value(refpaths, true);

    paths.push_back(path * Affine(1./3,0,0,1./3, A[X]*2./3, A[Y]*2./3 + boundingbox_Y.extent()/2));
    paths.push_back(path * Affine(1./3,0,0,1./3, B[X]*2./3, B[Y]*2./3 + boundingbox_Y.extent()/2));
    generator.set_new_value(paths, true);

    //FIXME: a path is used as ref instead of 2 points to work around path/point param incompatibility bug.
    //refA[Geom::X] = boundingbox_X.min();
    //refA[Geom::Y] = boundingbox_Y.middle();
    //refB[Geom::X] = boundingbox_X.max();
    //refB[Geom::Y] = boundingbox_Y.middle();
    //Geom::PathVector paths;
    //Geom::Path path = Geom::Path( (Point) refA);
    //path.appendNew<Geom::LineSegment>( (Point) refB );
    //paths.push_back(path * Affine(1./3,0,0,1./3, refA[X]*2./3, refA[Y]*2./3 + boundingbox_Y.extent()/2));
    //paths.push_back(path * Affine(1./3,0,0,1./3, refB[X]*2./3, refB[Y]*2./3 + boundingbox_Y.extent()/2));
    //paths.push_back(path);
    //generator.set_new_value(paths, true);
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

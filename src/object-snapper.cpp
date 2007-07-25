/**
 *  \file object-snapper.cpp
 *  \brief Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/n-art-bpath.h"
#include "libnr/nr-rect-ops.h"
#include "document.h"
#include "sp-namedview.h"
#include "sp-path.h"
#include "sp-image.h"
#include "sp-item-group.h"
#include "sp-item.h"
#include "sp-use.h"
#include "display/curve.h"
#include "desktop.h"
#include "inkscape.h"
#include "splivarot.h"
#include "prefs-utils.h"


Inkscape::ObjectSnapper::ObjectSnapper(SPNamedView const *nv, NR::Coord const d)
    : Snapper(nv, d), _snap_to_nodes(true), _snap_to_paths(true)
{

}


/**
 *  \param p Point we are trying to snap (desktop coordinates)
 */

void Inkscape::ObjectSnapper::_findCandidates(std::list<SPItem*>& c,
                                              SPObject* r,
                                              std::list<SPItem const *> const &it,
                                              NR::Point const &p) const
{
    if (ThisSnapperMightSnap()) {    
        SPDesktop const *desktop = SP_ACTIVE_DESKTOP;
        for (SPObject* o = sp_object_first_child(r); o != NULL; o = SP_OBJECT_NEXT(o)) {
            if (SP_IS_ITEM(o) && !SP_ITEM(o)->isLocked() && !desktop->itemIsHidden(SP_ITEM(o))) {
    
                /* See if this item is on the ignore list */
                std::list<SPItem const *>::const_iterator i = it.begin();
                while (i != it.end() && *i != o) {
                    i++;
                }
    
                if (i == it.end()) {
                    /* See if the item is within range */
                    if (SP_IS_GROUP(o)) {
                        _findCandidates(c, o, it, p);
                    } else {
                        NR::Maybe<NR::Rect> b = sp_item_bbox_desktop(SP_ITEM(o));
                        if ( b && NR::expand(*b, -getDistance()).contains(p) ) {
                            c.push_back(SP_ITEM(o));
                        }
                    }
                }
    
            }
        }
    }
}


void Inkscape::ObjectSnapper::_snapNodes(Inkscape::SnappedPoint &s,
                                         NR::Point const &p,
                                         std::list<SPItem*> const &cand) const
{
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;
    
    // Determine the type of bounding box we should snap to
    //TODO if (_snap_to_bbox) ???? -> will save some cpu time 
	gchar const *prefs_bbox = prefs_get_string_attribute("tools.select", "bounding_box");
	SPItem::BBoxType bbox_type = (prefs_bbox != NULL && strcmp(prefs_bbox, "geometric")==0)? SPItem::GEOMETRIC_BBOX : SPItem::APPROXIMATE_BBOX;        

    for (std::list<SPItem*>::const_iterator i = cand.begin(); i != cand.end(); i++) {
        
        NR::Matrix i2doc(NR::identity());
        SPItem *root_item = NULL;
        if (SP_IS_USE(*i)) {
            i2doc = sp_use_get_root_transform(SP_USE(*i));
            root_item = sp_use_root(SP_USE(*i));
        } else { 
            i2doc = sp_item_i2doc_affine(*i);
            root_item = *i;
        }
        
        SPCurve *curve = NULL;
        
        if (SP_IS_SHAPE(root_item)) {
            SPShape const *sh = SP_SHAPE(root_item);
            curve = sh->curve;
        } else if (SP_IS_IMAGE(root_item)) {
            SPImage const *im = SP_IMAGE(root_item);
            curve = im->curve;
        }
            
		std::list<NR::Point> points_to_snap_to;
        
        //Collect all nodes so we can snap to them
        if (curve) {
            int j = 0;
            while (SP_CURVE_BPATH(curve)[j].code != NR_END) {        
                /* Get this node in desktop coordinates */
                NArtBpath const &bp = SP_CURVE_BPATH(curve)[j];
                points_to_snap_to.push_back(desktop->doc2dt(bp.c(3) * i2doc));
                j++;
            }
        }
        
        //Collect the bounding box's corners so we can snap to them
        NR::Maybe<NR::Rect> b = sp_item_bbox_desktop(root_item, bbox_type);
        if (b) {
	        for ( unsigned k = 0 ; k < 4 ; k++ ) {
	            points_to_snap_to.push_back(b->corner(k));
	        }
        }        
        
        //Do the snapping, using all the nodes and corners collected above
        for (std::list<NR::Point>::const_iterator k = points_to_snap_to.begin(); k != points_to_snap_to.end(); k++) {
    	    /* Try to snap to this node of the path */
            NR::Coord const dist = NR::L2(*k - p);
            if (dist < getDistance() && dist < s.getDistance()) {
                s = SnappedPoint(*k, dist);
            }       
        }
    }
}


void Inkscape::ObjectSnapper::_snapPaths(Inkscape::SnappedPoint &s,
                                         NR::Point const &p,
                                         std::list<SPItem*> const &cand) const
{
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;

    NR::Point const p_doc = desktop->dt2doc(p);
    
    // Determine the type of bounding box we should snap to
    //TODO if (_snap_to_bbox) ???? -> will save some cpu time 
	gchar const *prefs_bbox = prefs_get_string_attribute("tools.select", "bounding_box");
	SPItem::BBoxType bbox_type = (prefs_bbox != NULL && strcmp(prefs_bbox, "geometric")==0)? SPItem::GEOMETRIC_BBOX : SPItem::APPROXIMATE_BBOX;        
    
    for (std::list<SPItem*>::const_iterator i = cand.begin(); i != cand.end(); i++) {

        /* Transform the requested snap point to this item's coordinates */
        NR::Matrix i2doc(NR::identity());
        SPItem *root_item = NULL;
        /* We might have a clone at hand, so make sure we get the root item */
        if (SP_IS_USE(*i)) {
            i2doc = sp_use_get_root_transform(SP_USE(*i));
            root_item = sp_use_root(SP_USE(*i));
        } else {
            i2doc = sp_item_i2doc_affine(*i);
            root_item = *i;
        }
        
        //Build a list of all paths considered for snapping to
        std::list<Path*> paths_to_snap_to;
        
        //Add the item's path to snap to
        paths_to_snap_to.push_back(Path_for_item(root_item, true, true));
                
        //Add the item's bounding box to snap to
        //This will get ugly... rect -> curve -> bpath
        NRRect rect;
        sp_item_invoke_bbox(root_item, &rect, i2doc, TRUE, bbox_type);
        NR::Maybe<NR::Rect> bbox = rect.upgrade();
        SPCurve *curve = sp_curve_new_from_rect(bbox);
        NArtBpath *bpath = SP_CURVE_BPATH(curve);
        Path *path = bpath_to_Path(bpath);  
        paths_to_snap_to.push_back(path);
        delete curve;
        delete bpath;
        
        //Now we can finally do the real snapping, using the paths collected above        
        for (std::list<Path*>::const_iterator k = paths_to_snap_to.begin(); k != paths_to_snap_to.end(); k++) {
	        if (*k) {
	            (*k)->ConvertWithBackData(0.01);
	    	
		        /* Look for the nearest position on this SPItem to our snap point */
		        NR::Maybe<Path::cut_position> const o = get_nearest_position_on_Path(*k, p_doc);
		        if (o && o->t >= 0 && o->t <= 1) {
		
		            /* Convert the nearest point back to desktop coordinates */
		            NR::Point const o_it = get_point_on_Path(*k, o->piece, o->t);		            
		            NR::Point const o_dt = desktop->doc2dt(o_it);
					
		            NR::Coord const dist = NR::L2(o_dt - p);
		            if (dist < getDistance() && dist < s.getDistance()) {
		                s = SnappedPoint(o_dt, dist);
		            }
		        }
		
		        delete *k;
	        }
	    }
    }
}


Inkscape::SnappedPoint Inkscape::ObjectSnapper::_doFreeSnap(NR::Point const &p,
                                                            std::list<SPItem const *> const &it) const
{
    if ( NULL == _named_view ) {
        return SnappedPoint(p, NR_HUGE);
    }

    /* Get a list of all the SPItems that we will try to snap to */
    std::list<SPItem*> cand;
    _findCandidates(cand, sp_document_root(_named_view->document), it, p);

    SnappedPoint s(p, NR_HUGE);

    if (_snap_to_nodes) {
        _snapNodes(s, p, cand);
    }
    if (_snap_to_paths) {
        _snapPaths(s, p, cand);
    }

    return s;
}



Inkscape::SnappedPoint Inkscape::ObjectSnapper::_doConstrainedSnap(NR::Point const &p,
                                                                   ConstraintLine const &c,
                                                                   std::list<SPItem const *> const &it) const
{
    /* FIXME: this needs implementing properly; I think we have to do the
    ** intersection of c with the objects.
    */
    return _doFreeSnap(p, it);
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::ObjectSnapper::ThisSnapperMightSnap() const
{
    return (_enabled && _snap_to != 0 && (_snap_to_paths || _snap_to_nodes));
}


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

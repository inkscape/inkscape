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
#include "sp-image.h"
#include "sp-item-group.h"
#include "sp-item.h"
#include "sp-use.h"
#include "display/curve.h"
#include "desktop.h"
#include "inkscape.h"
#include "prefs-utils.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"

Inkscape::ObjectSnapper::ObjectSnapper(SPNamedView const *nv, NR::Coord const d)
    : Snapper(nv, d), _snap_to_itemnode(true), _snap_to_itempath(true), 
    _snap_to_bboxnode(true), _snap_to_bboxpath(true), _strict_snapping(true),
    _include_item_center(false)
{
	_candidates = new std::vector<SPItem*>;
	_points_to_snap_to = new std::vector<NR::Point>;
	_paths_to_snap_to = new std::vector<Path*>;
}

Inkscape::ObjectSnapper::~ObjectSnapper() 
{
	_candidates->clear(); //Don't delete the candidates themselves, as these are not ours!
	delete _candidates;	
	
	_points_to_snap_to->clear();
	delete _points_to_snap_to;
	
	for (std::vector<Path*>::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
		delete *k;
    }
    _paths_to_snap_to->clear();
	delete _paths_to_snap_to;
}

/**
 *	Find all items within snapping range.  
 * 	\param r Pointer to the current document
 *  \param it List of items to ignore 
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param DimensionToSnap Snap in X, Y, or both directions.
 */

void Inkscape::ObjectSnapper::_findCandidates(SPObject* r,
                                              std::list<SPItem const *> const &it,
                                              bool const &first_point,
                                              std::vector<NR::Point> &points_to_snap,
                                              DimensionToSnap const snap_dim) const
{
    if (ThisSnapperMightSnap()) {    
        SPDesktop const *desktop = SP_ACTIVE_DESKTOP;
        
        if (first_point) { 
			_candidates->clear();
        }	    
                
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
                        _findCandidates(o, it, false, points_to_snap, snap_dim);
                    } else {
                        // Now let's see if any of the snapping points is within
                        // snapping range of this object  
                        NR::Maybe<NR::Rect> b = sp_item_bbox_desktop(SP_ITEM(o));
                        if (b) {
	                        for (std::vector<NR::Point>::const_iterator i = points_to_snap.begin(); i != points_to_snap.end(); i++) {
		                    	NR::Point b_min = b->min();
	                        	NR::Point b_max = b->max();		                        
		                        double d = getDistance();
		                        bool withinX = ((*i)[NR::X] >= b_min[NR::X] - d) && ((*i)[NR::X] <= b_max[NR::X] + d); 
		                        bool withinY = ((*i)[NR::Y] >= b_min[NR::Y] - d) && ((*i)[NR::Y] <= b_max[NR::Y] + d);
	                        	if (snap_dim == SNAP_X && withinX || snap_dim == SNAP_Y && withinY || snap_dim == SNAP_XY && withinX && withinY) {
	                         	   //We've found a point that is within snapping range 
	                         	   //of this object, so record it as a candidate
	                         	   _candidates->push_back(SP_ITEM(o));
	                         	   break;
	                        	}	
	                        }	
                        }
                    }
                }    
            }
        }
    }
}


bool Inkscape::ObjectSnapper::_snapNodes(Inkscape::Snapper::PointType const &t,
										 Inkscape::SnappedPoint &s,
                                         NR::Point const &p,
                                         bool const &first_point,
                                         DimensionToSnap const snap_dim) const
{
    bool success = false;
    
    // Determine the type of bounding box we should snap to
    SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX; 
	if (_snap_to_bboxnode) {	
		gchar const *prefs_bbox = prefs_get_string_attribute("tools.select", "bounding_box");
		bbox_type = (prefs_bbox != NULL && strcmp(prefs_bbox, "geometric")==0)? SPItem::GEOMETRIC_BBOX : SPItem::APPROXIMATE_BBOX;
	}        
	
	bool p_is_a_node = t & Inkscape::Snapper::SNAPPOINT_NODE;        
	bool p_is_a_bbox = t & Inkscape::Snapper::SNAPPOINT_BBOX; 
	bool p_is_a_guide = t & Inkscape::Snapper::SNAPPOINT_GUIDE;
	
	// A point considered for snapping should be either a node, a bbox corner or a guide. Pick only ONE! 
	g_assert(!(p_is_a_node && p_is_a_bbox || p_is_a_bbox && p_is_a_guide || p_is_a_node && p_is_a_guide));	

    // Now, let's first collect all points to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This dramatically improves the performance
	if (first_point) {
		_points_to_snap_to->clear();
	    for (std::vector<SPItem*>::const_iterator i = _candidates->begin(); i != _candidates->end(); i++) {
	        
	        //NR::Matrix i2doc(NR::identity());
	        SPItem *root_item = *i;
	        if (SP_IS_USE(*i)) {
	            root_item = sp_use_root(SP_USE(*i));
	        }
	          
			//Collect all nodes so we can snap to them
	        if (_snap_to_itemnode) {
	        	if (!(_strict_snapping && !p_is_a_node) || p_is_a_guide) {
			        sp_item_snappoints(root_item, _include_item_center, SnapPointsIter(*_points_to_snap_to));
				}
	        }
	        
	        //Collect the bounding box's corners so we can snap to them
	        if (_snap_to_bboxnode) {
	        	if (!(_strict_snapping && !p_is_a_bbox) || p_is_a_guide) {
			        NR::Maybe<NR::Rect> b = sp_item_bbox_desktop(root_item, bbox_type);
			        if (b) {
				        for ( unsigned k = 0 ; k < 4 ; k++ ) {
				            _points_to_snap_to->push_back(b->corner(k));
				        }
			        }
	        	}        
	        }
	    }
	}
    
    //Do the snapping, using all the nodes and corners collected above
    for (std::vector<NR::Point>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); k++) {
	    /* Try to snap to this node of the path */
        NR::Coord dist = NR_HUGE;
        NR::Point snapped_point;
        switch (snap_dim) {
        	case SNAP_X:
        		dist = fabs((*k)[NR::X] - p[NR::X]);
        		snapped_point = NR::Point((*k)[NR::X], p[NR::Y]); 
        		break;
        	case SNAP_Y:
        		dist = fabs((*k)[NR::Y] - p[NR::Y]);
        		snapped_point = NR::Point(p[NR::X], (*k)[NR::Y]);
        		break;
        	case SNAP_XY:
        		dist = NR::L2(*k - p);
        		snapped_point = *k;
        		break;
        }
        
        if (dist < getDistance() && dist < s.getDistance()) {
            s = SnappedPoint(snapped_point, dist);
            success = true;
        }       
    }
    
    return success;
}


bool Inkscape::ObjectSnapper::_snapPaths(Inkscape::Snapper::PointType const &t,
										 Inkscape::SnappedPoint &s,
                                         NR::Point const &p,
                                         bool const &first_point) const
{
    bool success = false;
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;

    NR::Point const p_doc = desktop->dt2doc(p);
    
    // Determine the type of bounding box we should snap to
    SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX; 
	if (_snap_to_bboxpath) {	
    	gchar const *prefs_bbox = prefs_get_string_attribute("tools.select", "bounding_box");
		bbox_type = (prefs_bbox != NULL && strcmp(prefs_bbox, "geometric")==0)? SPItem::GEOMETRIC_BBOX : SPItem::APPROXIMATE_BBOX;        
	}
	
	bool p_is_a_node = t & Inkscape::Snapper::SNAPPOINT_NODE;        
	
    // Now, let's first collect all paths to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This dramatically improves the performance
    if (first_point) {	    
	    for (std::vector<Path*>::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
    		delete *k;
	    }
	    _paths_to_snap_to->clear();
	    for (std::vector<SPItem*>::const_iterator i = _candidates->begin(); i != _candidates->end(); i++) {
	
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
	        
	        //Add the item's path to snap to
	        if (_snap_to_itempath) {
	        	if (!(_strict_snapping && !p_is_a_node)) {
	        		// Snapping to the path of characters is very cool, but for a large
					// chunk of text this will take ages! So limit snapping to text paths
					// containing max. 240 characters. Snapping the bbox will not be affected
					bool very_lenghty_prose = false;
					if (SP_IS_TEXT(root_item) || SP_IS_FLOWTEXT(root_item)) {
						very_lenghty_prose =  sp_text_get_length(SP_TEXT(root_item)) > 240; 
					}
					// On my AMD 3000+, the snapping lag becomes annoying at approx. 240 chars
					// which corresponds to a lag of 500 msec. This is for snapping a rect
					// to a single line of text. 
					
					// Snapping for example to a traced bitmap is also very stressing for 
					// the CPU, so we'll only snap to paths having no more than 500 nodes
					// This also leads to a lag of approx. 500 msec (in my lousy test set-up). 
					bool very_complex_path = false;
					if (SP_IS_PATH(root_item)) {
						very_complex_path = sp_nodes_in_path(SP_PATH(root_item)) > 500; 
					}								 		 
					
				 	if (!very_lenghty_prose && !very_complex_path) {
        				_paths_to_snap_to->push_back(Path_for_item(root_item, true, true));
				 	}
	        	}
	        }
	                
	        //Add the item's bounding box to snap to
	        if (_snap_to_bboxpath) {
	        	if (!(_strict_snapping && p_is_a_node)) {    	        
			        //This will get ugly... rect -> curve -> bpath
			        NRRect rect;
			        sp_item_invoke_bbox(root_item, &rect, i2doc, TRUE, bbox_type);
			        NR::Maybe<NR::Rect> bbox = rect.upgrade();
			        SPCurve *curve = sp_curve_new_from_rect(bbox);
			        if (curve) {
				        NArtBpath *bpath = SP_CURVE_BPATH(curve);
				        if (bpath) {
					        Path *path = bpath_to_Path(bpath);  
					        if (path) {
				        		_paths_to_snap_to->push_back(path);
					        }
					        delete bpath;
				        }    
				        delete curve;				        
			        }
	        	}
	        }
	    }
    }

    //Now we can finally do the real snapping, using the paths collected above        
    for (std::vector<Path*>::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
        if (*k) {
            if (first_point) {
            	(*k)->ConvertWithBackData(0.01); //This is extremely time consuming!
            }
    	
    		/* Look for the nearest position on this SPItem to our snap point */
	        NR::Maybe<Path::cut_position> const o = get_nearest_position_on_Path(*k, p_doc);
	        if (o && o->t >= 0 && o->t <= 1) {
	
	            /* Convert the nearest point back to desktop coordinates */
	            NR::Point const o_it = get_point_on_Path(*k, o->piece, o->t);		            
	            NR::Point const o_dt = desktop->doc2dt(o_it);
				
	            NR::Coord const dist = NR::L2(o_dt - p);
	            if (dist < getDistance() && dist < s.getDistance()) {
	                s = SnappedPoint(o_dt, dist);
	                success = true;
	            }
	        }
        }        
    }
    
    return success;
}


void Inkscape::ObjectSnapper::_doFreeSnap(SnappedConstraints &sc,
											Inkscape::Snapper::PointType const &t,
											NR::Point const &p,
											bool const &first_point,
	                         				std::vector<NR::Point> &points_to_snap,
	                                        std::list<SPItem const *> const &it) const
{
    if ( NULL == _named_view ) {
        return;
    }

    /* Get a list of all the SPItems that we will try to snap to */
	if (first_point) {
		_findCandidates(sp_document_root(_named_view->document), it, first_point, points_to_snap, SNAP_XY);
	}

	SnappedPoint s(p, NR_HUGE);
	bool snapped_to_node = false;
	bool snapped_to_path = false;

    if (_snap_to_itemnode || _snap_to_bboxnode) {
        snapped_to_node = _snapNodes(t, s, p, first_point, SNAP_XY);
    }
    if (_snap_to_itempath || _snap_to_bboxpath) {
        snapped_to_path = _snapPaths(t, s, p, first_point);
    }

    if (snapped_to_node || snapped_to_path) {
    	sc.points.push_back(s);
    }
}



void Inkscape::ObjectSnapper::_doConstrainedSnap(SnappedConstraints &sc,
												   Inkscape::Snapper::PointType const &t,
												   NR::Point const &p,
												   bool const &first_point,
				             					   std::vector<NR::Point> &points_to_snap,
				                                   ConstraintLine const &c,
				                                   std::list<SPItem const *> const &it) const
{
    /* FIXME: this needs implementing properly; I think we have to do the
    ** intersection of c with the objects.
    */
    _doFreeSnap(sc, t, p, first_point, points_to_snap, it);
}



Inkscape::SnappedPoint Inkscape::ObjectSnapper::guideSnap(NR::Point const &p,
														  DimensionToSnap const snap_dim) const
{
    if ( NULL == _named_view ) {
        return SnappedPoint(p, NR_HUGE);
    }

    /* Get a list of all the SPItems that we will try to snap to */
    std::vector<SPItem*> cand;
    std::list<SPItem const *> const it; //just an empty list
    
    std::vector<NR::Point> points_to_snap;
    points_to_snap.push_back(p);
    
    _findCandidates(sp_document_root(_named_view->document), it, true, points_to_snap, snap_dim);

    SnappedPoint s(p, NR_HUGE);
    _snapNodes(Inkscape::Snapper::SNAPPOINT_GUIDE, s, p, true, snap_dim);
    
    return s;
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::ObjectSnapper::ThisSnapperMightSnap() const
{
    bool snap_to_something = _snap_to_itempath || _snap_to_itemnode || _snap_to_bboxpath || _snap_to_bboxnode;
    return (_enabled && _snap_from != 0 && snap_to_something);
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

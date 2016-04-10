/** \file
 * Implementation of <path sodipodi:type="inkscape:offset">.
 */

/*
 * Authors: (of the sp-spiral.c upon which this file was constructed):
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include "svg/svg.h"
#include "attributes.h"
#include "display/curve.h"
#include <glibmm/i18n.h>

#include "livarot/Path.h"
#include "livarot/Shape.h"

#include "enums.h"
#include "preferences.h"
#include "sp-text.h"
#include "sp-offset.h"
#include "sp-use-reference.h"
#include "uri.h"

#include <2geom/affine.h>
#include <2geom/pathvector.h>

#include "xml/repr.h"

class SPDocument;

#define noOFFSET_VERBOSE

/** \note
 * SPOffset is a derivative of SPShape, much like the SPSpiral or SPRect.
 * The goal is to have a source shape (= originalPath), an offset (= radius)
 * and compute the offset of the source by the radius. To get it to work,
 * one needs to know what the source is and what the radius is, and how it's
 * stored in the xml representation. The object itself is a "path" element,
 * to get lots of shape functionality for free. The source is the easy part:
 * it's stored in a "inkscape:original" attribute in the path. In case of
 * "linked" offset, as they've been dubbed, there is an additional
 * "inkscape:href" that contains the id of an element of the svg.
 * When built, the object will attach a listener vector to that object and
 * rebuild the "inkscape:original" whenever the href'd object changes. This
 * is of course grossly inefficient, and also does not react to changes
 * to the href'd during context stuff (like changing the shape of a star by
 * dragging control points) unless the path of that object is changed during
 * the context (seems to be the case for SPEllipse). The computation of the
 * offset is done in sp_offset_set_shape(), a function that is called whenever
 * a change occurs to the offset (change of source or change of radius).
 * just like the sp-star and other, this path derivative can make control
 * points, or more precisely one control point, that's enough to define the
 * radius (look in object-edit).
 */

static void refresh_offset_source(SPOffset* offset);

static void sp_offset_start_listening(SPOffset *offset,SPObject* to);
static void sp_offset_quit_listening(SPOffset *offset);
static void sp_offset_href_changed(SPObject *old_ref, SPObject *ref, SPOffset *offset);
static void sp_offset_move_compensate(Geom::Affine const *mp, SPItem *original, SPOffset *self);
static void sp_offset_delete_self(SPObject *deleted, SPOffset *self);
static void sp_offset_source_modified (SPObject *iSource, guint flags, SPItem *item);


// slow= source path->polygon->offset of polygon->polygon->path
// fast= source path->offset of source path->polygon->path
// fast is not mathematically correct, because computing the offset of a single
// cubic bezier patch is not trivial; in particular, there are problems with holes
// reappearing in offset when the radius becomes too large
//TODO: need fix for bug: #384688 with fix released in r.14156
//but reverted because bug #1507049 seems has more priority.
static bool   use_slow_but_correct_offset_method = false;

SPOffset::SPOffset() : SPShape() {
    this->rad = 1.0;
    this->original = NULL;
    this->originalPath = NULL;
    this->knotSet = false;
    this->sourceDirty=false;
    this->isUpdating=false;
    // init various connections
    this->sourceHref = NULL;
    this->sourceRepr = NULL;
    this->sourceObject = NULL;

    // set up the uri reference
    this->sourceRef = new SPUseReference(this);
    this->_changed_connection = this->sourceRef->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_offset_href_changed), this));
}

SPOffset::~SPOffset() {
    delete this->sourceRef;

    this->_modified_connection.disconnect();
    this->_delete_connection.disconnect();
    this->_changed_connection.disconnect();
    this->_transformed_connection.disconnect();
}

void SPOffset::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPShape::build(document, repr);

    //XML Tree being used directly here while it shouldn't be.
    if (this->getRepr()->attribute("inkscape:radius")) {
        this->readAttr( "inkscape:radius" );
    } else {
        //XML Tree being used directly here (as object->getRepr) 
        //in all the below lines in the block while it shouldn't be.
        gchar const *oldA = this->getRepr()->attribute("sodipodi:radius");
        this->getRepr()->setAttribute("inkscape:radius",oldA);
        this->getRepr()->setAttribute("sodipodi:radius",NULL);

        this->readAttr( "inkscape:radius" );
    }

    if (this->getRepr()->attribute("inkscape:original")) {
        this->readAttr( "inkscape:original" );
    } else {
        gchar const *oldA = this->getRepr()->attribute("sodipodi:original");
        this->getRepr()->setAttribute("inkscape:original",oldA);
        this->getRepr()->setAttribute("sodipodi:original",NULL);

        this->readAttr( "inkscape:original" );
    }

    if (this->getRepr()->attribute("xlink:href")) {
        this->readAttr( "xlink:href" );
    } else {
        gchar const *oldA = this->getRepr()->attribute("inkscape:href");

        if (oldA) {
            size_t lA = strlen(oldA);
            char *nA=(char*)malloc((1+lA+1)*sizeof(char));

            memcpy(nA+1,oldA,lA*sizeof(char));

            nA[0]='#';
            nA[lA+1]=0;

            this->getRepr()->setAttribute("xlink:href",nA);

            free(nA);

            this->getRepr()->setAttribute("inkscape:href",NULL);
        }

        this->readAttr( "xlink:href" );
    }
}

Inkscape::XML::Node* SPOffset::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:path");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        /** \todo
         * Fixme: we may replace these attributes by
         * inkscape:offset="cx cy exp revo rad arg t0"
         */
        repr->setAttribute("sodipodi:type", "inkscape:offset");
        sp_repr_set_svg_double(repr, "inkscape:radius", this->rad);
        repr->setAttribute("inkscape:original", this->original);
        repr->setAttribute("inkscape:href", this->sourceHref);
    }


    // Make sure the offset has curve
    SPCurve *curve = SP_SHAPE (this)->getCurve();

    if (curve == NULL) {
        this->set_shape();
    }

    // write that curve to "d"
    char *d = sp_svg_write_path (this->_curve->get_pathvector());
    repr->setAttribute("d", d);
    g_free (d);

    SPShape::write(xml_doc, repr, flags | SP_SHAPE_WRITE_PATH);

    return repr;
}

void SPOffset::release() {
    if (this->original) {
    	free (this->original);
    }

    if (this->originalPath) {
    	delete ((Path *) this->originalPath);
    }

    this->original = NULL;
    this->originalPath = NULL;

    sp_offset_quit_listening(this);

    this->_changed_connection.disconnect();

    g_free(this->sourceHref);

    this->sourceHref = NULL;
    this->sourceRef->detach();

    SPShape::release();
}

void SPOffset::set(unsigned int key, const gchar* value) {
    if ( this->sourceDirty ) {
    	refresh_offset_source(this);
    }

    /* fixme: we should really collect updates */
    switch (key)
    {
        case SP_ATTR_INKSCAPE_ORIGINAL:
        case SP_ATTR_SODIPODI_ORIGINAL:
            if (value == NULL) {
            } else {
                if (this->original) {
                    free (this->original);
                    delete ((Path *) this->originalPath);

                    this->original = NULL;
                    this->originalPath = NULL;
                }

                this->original = strdup (value);

                Geom::PathVector pv = sp_svg_read_pathv(this->original);

                this->originalPath = new Path;
                reinterpret_cast<Path *>(this->originalPath)->LoadPathVector(pv);

                this->knotSet = false;

                if ( this->isUpdating == false ) {
                	this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                }
            }
            break;

        case SP_ATTR_INKSCAPE_RADIUS:
        case SP_ATTR_SODIPODI_RADIUS:
            if (!sp_svg_length_read_computed_absolute (value, &this->rad)) {
                if (fabs (this->rad) < 0.01) {
                    this->rad = (this->rad < 0) ? -0.01 : 0.01;
                }

                this->knotSet = false; // knotset=false because it's not set from the context
            }

            if ( this->isUpdating == false ) {
            	this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_INKSCAPE_HREF:
        case SP_ATTR_XLINK_HREF:
            if ( value == NULL ) {
                sp_offset_quit_listening(this);
                if ( this->sourceHref ) {
                	g_free(this->sourceHref);
                }

                this->sourceHref = NULL;
                this->sourceRef->detach();
            } else {
                if ( this->sourceHref && ( strcmp(value, this->sourceHref) == 0 ) ) {
                } else {
                    if ( this->sourceHref ) {
                    	g_free(this->sourceHref);
                    }

                    this->sourceHref = g_strdup(value);

                    try {
                        this->sourceRef->attach(Inkscape::URI(value));
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        this->sourceRef->detach();
                    }
                }
            }
            break;
            
        default:
            SPShape::set(key, value);
            break;
    }
}

void SPOffset::update(SPCtx *ctx, guint flags) {
    this->isUpdating=true; // prevent sp_offset_set from requesting updates
    
    if ( this->sourceDirty ) {
    	refresh_offset_source(this);
    }
    
    if (flags &
        (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
         SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
    	
        this->set_shape();
    }
    
    this->isUpdating=false;

    SPShape::update(ctx, flags);
}

const char* SPOffset::displayName() const {
    if ( this->sourceHref ) {
        return _("Linked Offset");
    } else {
        return _("Dynamic Offset");
    }
}

gchar* SPOffset::description() const {
    // TRANSLATORS COMMENT: %s is either "outset" or "inset" depending on sign
    return g_strdup_printf(_("%s by %f pt"), (this->rad >= 0) ?
            _("outset") : _("inset"), fabs (this->rad));
}

void SPOffset::set_shape() {
    if ( this->originalPath == NULL ) {
        // oops : no path?! (the offset object should do harakiri)
        return;
    }
#ifdef OFFSET_VERBOSE
    g_print ("rad=%g\n", offset->rad);
#endif
    // au boulot

    if ( fabs(this->rad) < 0.01 ) {
        // grosso modo: 0
        // just put the source this as the offseted one, no one will notice
        // it's also useless to compute the offset with a 0 radius

        //XML Tree being used directly here while it shouldn't be.
        const char *res_d = this->getRepr()->attribute("inkscape:original");

        if ( res_d ) {
            Geom::PathVector pv = sp_svg_read_pathv(res_d);
            SPCurve *c = new SPCurve(pv);
            g_assert(c != NULL);

            this->setCurveInsync (c, TRUE);
            this->setCurveBeforeLPE(c);

            c->unref();
        }

        return;
    }

    // extra paraniac careful check. the preceding if () should take care of this case
    if (fabs (this->rad) < 0.01) {
    	this->rad = (this->rad < 0) ? -0.01 : 0.01;
    }

    Path *orig = new Path;
    orig->Copy ((Path *)this->originalPath);

    if ( use_slow_but_correct_offset_method == false ) {
        // version par outline
        Shape *theShape = new Shape;
        Shape *theRes = new Shape;
        Path *originaux[1];
        Path *res = new Path;
        res->SetBackData (false);

        // and now: offset
        float o_width;
        if (this->rad >= 0)
        {
            o_width = this->rad;
            orig->OutsideOutline (res, o_width, join_round, butt_straight, 20.0);
        }
        else
        {
            o_width = -this->rad;
            orig->OutsideOutline (res, -o_width, join_round, butt_straight, 20.0);
        }

        if (o_width >= 1.0)
        {
            //      res->ConvertForOffset (1.0, orig, offset->rad);
            res->ConvertWithBackData (1.0);
        }
        else
        {
            //      res->ConvertForOffset (o_width, orig, offset->rad);
            res->ConvertWithBackData (o_width);
        }
        res->Fill (theShape, 0);
        theRes->ConvertToShape (theShape, fill_positive);
        originaux[0] = res;

        theRes->ConvertToForme (orig, 1, originaux);

        Geom::OptRect bbox = this->desktopVisualBounds();

        if ( bbox ) {
            gdouble size = L2(bbox->dimensions());
            gdouble const exp = this->transform.descrim();

            if (exp != 0) {
                size /= exp;
            }

            orig->Coalesce (size * 0.001);
            //g_print ("coa %g    exp %g    item %p\n", size * 0.001, exp, item);
        }


        //  if (o_width >= 1.0)
        //  {
        //    orig->Coalesce (0.1);  // small treshhold, since we only want to get rid of small segments
        // the curve should already be computed by the Outline() function
        //   orig->ConvertEvenLines (1.0);
        //   orig->Simplify (0.5);
        //  }
        //  else
        //  {
        //          orig->Coalesce (0.1*o_width);
        //   orig->ConvertEvenLines (o_width);
        //   orig->Simplify (0.5 * o_width);
        //  }

        delete theShape;
        delete theRes;
        delete res;
    } else {
        // version par makeoffset
        Shape *theShape = new Shape;
        Shape *theRes = new Shape;


        // and now: offset
        float o_width;
        if (this->rad >= 0)
        {
            o_width = this->rad;
        }
        else
        {
            o_width = -this->rad;
        }

        // one has to have a measure of the details
        if (o_width >= 1.0)
        {
            orig->ConvertWithBackData (0.5);
        }
        else
        {
            orig->ConvertWithBackData (0.5*o_width);
        }

        orig->Fill (theShape, 0);
        theRes->ConvertToShape (theShape, fill_positive);

        Path *originaux[1];
        originaux[0]=orig;

        Path *res = new Path;
        theRes->ConvertToForme (res, 1, originaux);

        int    nbPart=0;
        Path** parts=res->SubPaths(nbPart,true);
        char   *holes=(char*)malloc(nbPart*sizeof(char));

        // we offset contours separately, because we can.
        // this way, we avoid doing a unique big ConvertToShape when dealing with big shapes with lots of holes
        {
            Shape* onePart=new Shape;
            Shape* oneCleanPart=new Shape;

            theShape->Reset();

            for (int i=0;i<nbPart;i++) {
                double partSurf=parts[i]->Surface();
                parts[i]->Convert(1.0);

                {
                    // raffiner si besoin
                    double  bL,bT,bR,bB;
                    parts[i]->PolylineBoundingBox(bL,bT,bR,bB);
                    double  mesure=((bR-bL)+(bB-bT))*0.5;
                    if ( mesure < 10.0 ) {
                        parts[i]->Convert(0.02*mesure);
                    }
                }

                if ( partSurf < 0 ) { // inverse par rapport a la realite
                    // plein
                    holes[i]=0;
                    parts[i]->Fill(oneCleanPart,0);
                    onePart->ConvertToShape(oneCleanPart,fill_positive); // there aren't intersections in that one, but maybe duplicate points and null edges
                    oneCleanPart->MakeOffset(onePart,this->rad,join_round,20.0);
                    onePart->ConvertToShape(oneCleanPart,fill_positive);

                    onePart->CalcBBox();
                    double  typicalSize=0.5*((onePart->rightX-onePart->leftX)+(onePart->bottomY-onePart->topY));

                    if ( typicalSize < 0.05 ) {
                    	typicalSize=0.05;
                    }

                    typicalSize*=0.01;

                    if ( typicalSize > 1.0 ) {
                    	typicalSize=1.0;
                    }

                    onePart->ConvertToForme (parts[i]);
                    parts[i]->ConvertEvenLines (typicalSize);
                    parts[i]->Simplify (typicalSize);

                    double nPartSurf=parts[i]->Surface();

                    if ( nPartSurf >= 0 ) {
                        // inversion de la surface -> disparait
                        delete parts[i];
                        parts[i]=NULL;
                    } else {
                    }

/*          int  firstP=theShape->nbPt;
            for (int j=0;j<onePart->nbPt;j++) theShape->AddPoint(onePart->pts[j].x);
            for (int j=0;j<onePart->nbAr;j++) theShape->AddEdge(firstP+onePart->aretes[j].st,firstP+onePart->aretes[j].en);*/
                } else {
                    // trou
                    holes[i]=1;
                    parts[i]->Fill(oneCleanPart,0,false,true,true);
                    onePart->ConvertToShape(oneCleanPart,fill_positive);
                    oneCleanPart->MakeOffset(onePart,-this->rad,join_round,20.0);
                    onePart->ConvertToShape(oneCleanPart,fill_positive);
//          for (int j=0;j<onePart->nbAr;j++) onePart->Inverse(j); // pas oublier de reinverser

                    onePart->CalcBBox();
                    double  typicalSize=0.5*((onePart->rightX-onePart->leftX)+(onePart->bottomY-onePart->topY));

                    if ( typicalSize < 0.05 ) {
                    	typicalSize=0.05;
                    }

                    typicalSize*=0.01;

                    if ( typicalSize > 1.0 ) {
                    	typicalSize=1.0;
                    }

                    onePart->ConvertToForme (parts[i]);
                    parts[i]->ConvertEvenLines (typicalSize);
                    parts[i]->Simplify (typicalSize);
                    double nPartSurf=parts[i]->Surface();

                    if ( nPartSurf >= 0 ) {
                        // inversion de la surface -> disparait
                        delete parts[i];
                        parts[i]=NULL;
                    } else {
                    }

                    /*         int  firstP=theShape->nbPt;
                               for (int j=0;j<onePart->nbPt;j++) theShape->AddPoint(onePart->pts[j].x);
                               for (int j=0;j<onePart->nbAr;j++) theShape->AddEdge(firstP+onePart->aretes[j].en,firstP+onePart->aretes[j].st);*/
                }
//        delete parts[i];
            }
//      theShape->MakeOffset(theRes,offset->rad,join_round,20.0);
            delete onePart;
            delete oneCleanPart;
        }

        if ( nbPart > 1 ) {
            theShape->Reset();

            for (int i=0;i<nbPart;i++) {
                if ( parts[i] ) {
                    parts[i]->ConvertWithBackData(1.0);

                    if ( holes[i] ) {
                        parts[i]->Fill(theShape,i,true,true,true);
                    } else {
                        parts[i]->Fill(theShape,i,true,true,false);
                    }
                }
            }

            theRes->ConvertToShape (theShape, fill_positive);
            theRes->ConvertToForme (orig,nbPart,parts);

            for (int i=0;i<nbPart;i++) {
            	if ( parts[i] ) {
            		delete parts[i];
            	}
            }
        } else if ( nbPart == 1 ) {
            orig->Copy(parts[0]);

            for (int i=0;i<nbPart;i++) {
            	if ( parts[i] ) {
            		delete parts[i];
            	}
            }
        } else {
            orig->Reset();
        }
//    theRes->ConvertToShape (theShape, fill_positive);
//    theRes->ConvertToForme (orig);

/*    if (o_width >= 1.0) {
      orig->ConvertEvenLines (1.0);
      orig->Simplify (1.0);
      } else {
      orig->ConvertEvenLines (1.0*o_width);
      orig->Simplify (1.0 * o_width);
      }*/

        if ( parts ) {
        	free(parts);
        }

        if ( holes ) {
        	free(holes);
        }

        delete res;
        delete theShape;
        delete theRes;
    }
    {
        char *res_d = NULL;

        if (orig->descr_cmd.size() <= 1)
        {
            // Aie.... nothing left.
            res_d = strdup ("M 0 0 L 0 0 z");
            //printf("%s\n",res_d);
        }
        else
        {

            res_d = orig->svg_dump_path ();
        }

        delete orig;

        Geom::PathVector pv = sp_svg_read_pathv(res_d);
        SPCurve *c = new SPCurve(pv);
        g_assert(c != NULL);

        this->setCurveInsync (c, TRUE);
        this->setCurveBeforeLPE(c);
        c->unref();

        free (res_d);
    }
}

void SPOffset::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const {
    SPShape::snappoints(p, snapprefs);
}


// utilitaires pour les poignees
// used to get the distance to the shape: distance to polygon give the fabs(radius), we still need
// the sign. for edges, it's easy to determine which side the point is on, for points of the polygon
// it's trickier: we need to identify which angle the point is in; to that effect, we take each
// successive clockwise angle (A,C) and check if the vector B given by the point is in the angle or
// outside.
// another method would be to use the Winding() function to test whether the point is inside or outside
// the polygon (it would be wiser to do so, in fact, but i like being stupid)

/**
 *
 * \todo
 * FIXME: This can be done using linear operations, more stably and
 *  faster.  method: transform A and C into B's space, A should be
 *  negative and B should be positive in the orthogonal component.  I
 *  think this is equivalent to
 *  dot(A, rot90(B))*dot(C, rot90(B)) == -1.
 *    -- njh
 */
static bool
vectors_are_clockwise (Geom::Point A, Geom::Point B, Geom::Point C)
{
    using Geom::rot90;
    double ab_s = dot(A, rot90(B));
    double ab_c = dot(A, B);
    double bc_s = dot(B, rot90(C));
    double bc_c = dot(B, C);
    double ca_s = dot(C, rot90(A));
    double ca_c = dot(C, A);

    double ab_a = acos (ab_c);

    if (ab_c <= -1.0) {
        ab_a = M_PI;
    }

    if (ab_c >= 1.0) {
        ab_a = 0;
    }

    if (ab_s < 0) {
        ab_a = 2 * M_PI - ab_a;
    }

    double bc_a = acos (bc_c);

    if (bc_c <= -1.0) {
        bc_a = M_PI;
    }

    if (bc_c >= 1.0) {
        bc_a = 0;
    }

    if (bc_s < 0) {
        bc_a = 2 * M_PI - bc_a;
    }

    double ca_a = acos (ca_c);

    if (ca_c <= -1.0) {
        ca_a = M_PI;
    }

    if (ca_c >= 1.0) {
        ca_a = 0;
    }

    if (ca_s < 0) {
        ca_a = 2 * M_PI - ca_a;
    }

    double lim = 2 * M_PI - ca_a;

    if (ab_a < lim) {
        return true;
    }

    return false;
}

/**
 * Distance to the original path; that function is called from object-edit
 * to set the radius when the control knot moves.
 *
 * The sign of the result is the radius we're going to offset the shape with,
 * so result > 0 ==outset and result < 0 ==inset. thus result<0 means
 * 'px inside source'.
 */
double
sp_offset_distance_to_original (SPOffset * offset, Geom::Point px)
{
    if (offset == NULL || offset->originalPath == NULL || ((Path *) offset->originalPath)->descr_cmd.size() <= 1) {
        return 1.0;
    }

    double dist = 1.0;
    Shape *theShape = new Shape;
    Shape *theRes = new Shape;

    /** \todo
     * Awfully damn stupid method: uncross the source path EACH TIME you
     * need to compute the distance. The good way to do this would be to
     * store the uncrossed source path somewhere, and delete it when the
     * context is finished. Hopefully this part is much faster than actually
     * computing the offset (which happen just after), so the time spent in
     * this function should end up being negligible with respect to the
     * delay of one context.
     */
    // move
    ((Path *) offset->originalPath)->Convert (1.0);
    ((Path *) offset->originalPath)->Fill (theShape, 0);
    theRes->ConvertToShape (theShape, fill_oddEven);

    if (theRes->numberOfEdges() <= 1)
    {

    }
    else
    {
        double ptDist = -1.0;
        bool ptSet = false;
        double arDist = -1.0;
        bool arSet = false;

        // first get the minimum distance to the points
        for (int i = 0; i < theRes->numberOfPoints(); i++)
        {
            if (theRes->getPoint(i).totalDegree() > 0)
            {
                Geom::Point nx = theRes->getPoint(i).x;
                Geom::Point nxpx = px-nx;
                double ndist = sqrt (dot(nxpx,nxpx));

                if (ptSet == false || fabs (ndist) < fabs (ptDist))
                {
                    // we have a new minimum distance
                    // now we need to wheck if px is inside or outside (for the sign)
                    nx = px - theRes->getPoint(i).x;
                    double nlen = sqrt (dot(nx , nx));
                    nx /= nlen;
                    int pb, cb, fb;
                    fb = theRes->getPoint(i).incidentEdge[LAST];
                    pb = theRes->getPoint(i).incidentEdge[LAST];
                    cb = theRes->getPoint(i).incidentEdge[FIRST];

                    do
                    {
                        // one angle
                        Geom::Point prx, nex;
                        prx = theRes->getEdge(pb).dx;
                        nlen = sqrt (dot(prx, prx));
                        prx /= nlen;
                        nex = theRes->getEdge(cb).dx;
                        nlen = sqrt (dot(nex , nex));
                        nex /= nlen;

                        if (theRes->getEdge(pb).en == i)
                        {
                            prx = -prx;
                        }

                        if (theRes->getEdge(cb).en == i)
                        {
                            nex = -nex;
                        }

                        if (vectors_are_clockwise (nex, nx, prx))
                        {
                            // we're in that angle. set the sign, and exit that loop
                            if (theRes->getEdge(cb).st == i)
                            {
                                ptDist = -ndist;
                                ptSet = true;
                            }
                            else
                            {
                                ptDist = ndist;
                                ptSet = true;
                            }
                            break;
                        }

                        pb = cb;
                        cb = theRes->NextAt (i, cb);
                    }

                    while (cb >= 0 && pb >= 0 && pb != fb);
                }
            }
        }

        // loop over the edges to try to improve the distance
        for (int i = 0; i < theRes->numberOfEdges(); i++)
        {
            Geom::Point sx = theRes->getPoint(theRes->getEdge(i).st).x;
            Geom::Point ex = theRes->getPoint(theRes->getEdge(i).en).x;
            Geom::Point nx = ex - sx;
            double len = sqrt (dot(nx,nx));

            if (len > 0.0001)
            {
                Geom::Point   pxsx=px-sx;
                double ab = dot(nx,pxsx);

                if (ab > 0 && ab < len * len)
                {
                    // we're in the zone of influence of the segment
                    double ndist = (cross(nx, pxsx)) / len;

                    if (arSet == false || fabs (ndist) < fabs (arDist))
                    {
                        arDist = ndist;
                        arSet = true;
                    }
                }
            }
        }

        if (arSet || ptSet)
        {
            if (arSet == false) {
                arDist = ptDist;
            }

            if (ptSet == false) {
                ptDist = arDist;
            }

            if (fabs (ptDist) < fabs (arDist)) {
                dist = ptDist;
            } else {
                dist = arDist;
            }
        }
    }

    delete theShape;
    delete theRes;

    return dist;
}

/**
 * Computes a point on the offset;  used to set a "seed" position for
 * the control knot.
 *
 * \return the topmost point on the offset.
 */
void
sp_offset_top_point (SPOffset const * offset, Geom::Point *px)
{
    (*px) = Geom::Point(0, 0);

    if (offset == NULL) {
        return;
    }

    if (offset->knotSet)
    {
        (*px) = offset->knot;
        return;
    }

    SPCurve *curve = SP_SHAPE (offset)->getCurve();

    if (curve == NULL)
    {
    	// CPPIFY
        //offset->set_shape();
    	const_cast<SPOffset*>(offset)->set_shape();

        curve = SP_SHAPE (offset)->getCurve();

        if (curve == NULL)
            return;
    }

    if (curve->is_empty())
    {
        curve->unref();
        return;
    }

    Path *finalPath = new Path;
    finalPath->LoadPathVector(curve->get_pathvector());

    Shape *theShape = new Shape;

    finalPath->Convert (1.0);
    finalPath->Fill (theShape, 0);

    if (theShape->hasPoints())
    {
        theShape->SortPoints ();
        *px = theShape->getPoint(0).x;
    }

    delete theShape;
    delete finalPath;
    curve->unref();
}

// the listening functions
static void sp_offset_start_listening(SPOffset *offset,SPObject* to)
{
    if ( to == NULL ) {
        return;
    }

    offset->sourceObject = to;
    offset->sourceRepr = to->getRepr();

    offset->_delete_connection = to->connectDelete(sigc::bind(sigc::ptr_fun(&sp_offset_delete_self), offset));
    offset->_transformed_connection = SP_ITEM(to)->connectTransformed(sigc::bind(sigc::ptr_fun(&sp_offset_move_compensate), offset));
    offset->_modified_connection = to->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_offset_source_modified), offset));
}

static void sp_offset_quit_listening(SPOffset *offset)
{
    if ( offset->sourceObject == NULL ) {
        return;
    }

    offset->_modified_connection.disconnect();
    offset->_delete_connection.disconnect();
    offset->_transformed_connection.disconnect();

    offset->sourceRepr = NULL;
    offset->sourceObject = NULL;
}

static void
sp_offset_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, SPOffset *offset)
{
    sp_offset_quit_listening(offset);

    if (offset->sourceRef) {
        SPItem *refobj = offset->sourceRef->getObject();

        if (refobj) {
        	sp_offset_start_listening(offset,refobj);
        }

        offset->sourceDirty=true;
        offset->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    }
}

static void sp_offset_move_compensate(Geom::Affine const *mp, SPItem */*original*/, SPOffset *self)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint mode = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_PARALLEL);

    Geom::Affine m(*mp);

    if (!(m.isTranslation()) || mode == SP_CLONE_COMPENSATION_NONE) {
        self->sourceDirty=true;
        self->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        return;
    }

    // calculate the compensation matrix and the advertized movement matrix
    self->readAttr("transform");

    Geom::Affine t = self->transform;
    Geom::Affine offset_move = t.inverse() * m * t;

    Geom::Affine advertized_move;
    if (mode == SP_CLONE_COMPENSATION_PARALLEL) {
        offset_move = offset_move.inverse() * m;
        advertized_move = m;
    } else if (mode == SP_CLONE_COMPENSATION_UNMOVED) {
        offset_move = offset_move.inverse();
        advertized_move.setIdentity();
    } else {
        g_assert_not_reached();
    }

    self->sourceDirty=true;

    // commit the compensation
    self->transform *= offset_move;
    self->doWriteTransform(self->getRepr(), self->transform, &advertized_move);
    self->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_offset_delete_self(SPObject */*deleted*/, SPOffset *offset)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint const mode = prefs->getInt("/options/cloneorphans/value", SP_CLONE_ORPHANS_UNLINK);

    if (mode == SP_CLONE_ORPHANS_UNLINK) {
        // leave it be. just forget about the source
        sp_offset_quit_listening(offset);

        if ( offset->sourceHref ) {
        	g_free(offset->sourceHref);
        }

        offset->sourceHref = NULL;
        offset->sourceRef->detach();
    } else if (mode == SP_CLONE_ORPHANS_DELETE) {
        offset->deleteObject();
    }
}

static void
sp_offset_source_modified (SPObject */*iSource*/, guint flags, SPItem *item)
{
    SPOffset *offset = SP_OFFSET(item);
    offset->sourceDirty=true;

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG)) {
        offset->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    }
}

static void
refresh_offset_source(SPOffset* offset)
{
    if ( offset == NULL ) {
    	return;
    }

    offset->sourceDirty=false;

    // le mauvais cas: pas d'attribut d => il faut verifier que c'est une SPShape puis prendre le contour
    // The bad case: no d attribute.  Must check that it's an SPShape and then take the outline.
    SPObject *refobj=offset->sourceObject;

    if ( refobj == NULL ) {
    	return;
    }

    SPItem  *item  = SP_ITEM (refobj);
    SPCurve *curve = NULL;

    if (SP_IS_SHAPE (item)) {
        curve = SP_SHAPE (item)->getCurve ();
    }
    else if (SP_IS_TEXT (item)) {
        curve = SP_TEXT (item)->getNormalizedBpath ();
    }
    else {
        return;
    }

    if (curve == NULL) {
        return;
    }

    Path *orig = new Path;
    orig->LoadPathVector(curve->get_pathvector());
    curve->unref();

    if (!item->transform.isIdentity()) {
        gchar const *t_attr = item->getRepr()->attribute("transform");

        if (t_attr) {
            Geom::Affine t;

            if (sp_svg_transform_read(t_attr, &t)) {
                orig->Transform(t);
            }
        }
    }

    // Finish up.
    {
        SPCSSAttr *css;
        const gchar *val;
        Shape *theShape = new Shape;
        Shape *theRes = new Shape;

        orig->ConvertWithBackData (1.0);
        orig->Fill (theShape, 0);

        css = sp_repr_css_attr (offset->sourceRepr , "style");
        val = sp_repr_css_property (css, "fill-rule", NULL);

        if (val && strcmp (val, "nonzero") == 0)
        {
            theRes->ConvertToShape (theShape, fill_nonZero);
        }
        else if (val && strcmp (val, "evenodd") == 0)
        {
            theRes->ConvertToShape (theShape, fill_oddEven);
        }
        else
        {
            theRes->ConvertToShape (theShape, fill_nonZero);
        }

        Path *originaux[1];
        originaux[0] = orig;
        Path *res = new Path;
        theRes->ConvertToForme (res, 1, originaux);

        delete theShape;
        delete theRes;

        char *res_d = res->svg_dump_path ();
        delete res;
        delete orig;

        // TODO fix:
        //XML Tree being used diectly here while it shouldn't be.
        offset->getRepr()->setAttribute("inkscape:original", res_d);

        free (res_d);
    }
}

SPItem *
sp_offset_get_source (SPOffset *offset)
{
    if (offset && offset->sourceRef) {
        SPItem *refobj = offset->sourceRef->getObject();

        if (SP_IS_ITEM (refobj)) {
            return (SPItem *) refobj;
        }
    }

    return NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

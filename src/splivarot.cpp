#define __SP_LIVAROT_C__
/*
 *  splivarot.cpp
 *  Inkscape
 *
 *  Created by fred on Fri Dec 05 2003.
 *  tweaked endlessly by bulia byak <buliabyak@users.sf.net>
 *  public domain
 *
 */

/*
 * contains lots of stitched pieces of path-chemistry.c
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cstring>
#include <string>
#include <vector>
#include <glib/gmem.h>
#include "xml/repr.h"
#include "svg/svg.h"
#include "sp-path.h"
#include "sp-shape.h"
#include "sp-image.h"
#include "marker.h"
#include "enums.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
#include "sp-item-group.h"
#include "style.h"
#include "document.h"
#include "message-stack.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "display/canvas-bpath.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include "preferences.h"

#include "xml/repr.h"
#include "xml/repr-sorting.h"
#include <2geom/pathvector.h>
#include <libnr/nr-scale-matrix-ops.h>
#include "helper/geom.h"

#include "livarot/Path.h"
#include "livarot/Shape.h"

#include "splivarot.h"

bool   Ancetre(Inkscape::XML::Node *a, Inkscape::XML::Node *who);

void sp_selected_path_boolop(SPDesktop *desktop, bool_op bop, const unsigned int verb=SP_VERB_NONE, const Glib::ustring description="");
void sp_selected_path_do_offset(SPDesktop *desktop, bool expand, double prefOffset);
void sp_selected_path_create_offset_object(SPDesktop *desktop, int expand, bool updating);

void
sp_selected_path_union(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_union, SP_VERB_SELECTION_UNION, _("Union"));
}

void
sp_selected_path_union_skip_undo(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_union, SP_VERB_NONE, _("Union"));
}

void
sp_selected_path_intersect(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_inters, SP_VERB_SELECTION_INTERSECT, _("Intersection"));
}

void
sp_selected_path_diff(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_diff, SP_VERB_SELECTION_DIFF, _("Difference"));
}

void
sp_selected_path_diff_skip_undo(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_diff, SP_VERB_NONE, _("Difference"));
}

void
sp_selected_path_symdiff(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_symdiff, SP_VERB_SELECTION_SYMDIFF, _("Exclusion"));
}
void
sp_selected_path_cut(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_cut, SP_VERB_SELECTION_CUT, _("Division"));
}
void
sp_selected_path_slice(SPDesktop *desktop)
{
    sp_selected_path_boolop(desktop, bool_op_slice, SP_VERB_SELECTION_SLICE,  _("Cut path"));
}


// boolean operations
// take the source paths from the file, do the operation, delete the originals and add the results
void
sp_selected_path_boolop(SPDesktop *desktop, bool_op bop, const unsigned int verb, const Glib::ustring description)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    
    GSList *il = (GSList *) selection->itemList();
    
    // allow union on a single object for the purpose of removing self overlapse (svn log, revision 13334)
    if ( (g_slist_length(il) < 2) && (bop != bool_op_union)) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Select <b>at least 2 paths</b> to perform a boolean operation."));
        return;
    }
    else if ( g_slist_length(il) < 1 ) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Select <b>at least 1 path</b> to perform a boolean union."));
        return;
    }

    if (g_slist_length(il) > 2) {
        if (bop == bool_op_diff || bop == bool_op_symdiff || bop == bool_op_cut || bop == bool_op_slice ) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Select <b>exactly 2 paths</b> to perform difference, XOR, division, or path cut."));
            return;
        }
    }

    // reverseOrderForOp marks whether the order of the list is the top->down order
    // it's only used when there are 2 objects, and for operations who need to know the
    // topmost object (differences, cuts)
    bool reverseOrderForOp = false;

    // mettre les elements de la liste dans l'ordre pour ces operations
    if (bop == bool_op_diff || bop == bool_op_symdiff || bop == bool_op_cut || bop == bool_op_slice) {
        // check in the tree to find which element of the selection list is topmost (for 2-operand commands only)
        Inkscape::XML::Node *a = SP_OBJECT_REPR(il->data);
        Inkscape::XML::Node *b = SP_OBJECT_REPR(il->next->data);

        if (a == NULL || b == NULL) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Unable to determine the <b>z-order</b> of the objects selected for difference, XOR, division, or path cut."));
            return;
        }

        if (Ancetre(a, b)) {
            // a is the parent of b, already in the proper order
        } else if (Ancetre(b, a)) {
            // reverse order
            reverseOrderForOp = true;
        } else {

            // objects are not in parent/child relationship;
            // find their lowest common ancestor
            Inkscape::XML::Node *dad = LCA(a, b);
            if (dad == NULL) {
                desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Unable to determine the <b>z-order</b> of the objects selected for difference, XOR, division, or path cut."));
                return;
            }

            // find the children of the LCA that lead from it to the a and b
            Inkscape::XML::Node *as = AncetreFils(a, dad);
            Inkscape::XML::Node *bs = AncetreFils(b, dad);

            // find out which comes first
            for (Inkscape::XML::Node *child = dad->firstChild(); child; child = child->next()) {
                if (child == as) {
                    /* a first, so reverse. */
                    reverseOrderForOp = true;
                    break;
                }
                if (child == bs)
                    break;
            }
        }
    }

    il = g_slist_copy(il);

    // first check if all the input objects have shapes
    // otherwise bail out
    for (GSList *l = il; l != NULL; l = l->next)
    {
        SPItem *item = SP_ITEM(l->data);
        if (!SP_IS_SHAPE(item) && !SP_IS_TEXT(item) && !SP_IS_FLOWTEXT(item))
        {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("One of the objects is <b>not a path</b>, cannot perform boolean operation."));
            g_slist_free(il);
            return;
        }
    }

    // extract the livarot Paths from the source objects
    // also get the winding rule specified in the style
    int nbOriginaux = g_slist_length(il);
    std::vector<Path *> originaux(nbOriginaux);
    std::vector<FillRule> origWind(nbOriginaux);
    int curOrig;
    {
        curOrig = 0;
        for (GSList *l = il; l != NULL; l = l->next)
        {
            SPCSSAttr *css = sp_repr_css_attr(SP_OBJECT_REPR(il->data), "style");
            gchar const *val = sp_repr_css_property(css, "fill-rule", NULL);
            if (val && strcmp(val, "nonzero") == 0) {
                origWind[curOrig]= fill_nonZero;
            } else if (val && strcmp(val, "evenodd") == 0) {
                origWind[curOrig]= fill_oddEven;
            } else {
                origWind[curOrig]= fill_nonZero;
            }

            originaux[curOrig] = Path_for_item((SPItem *) l->data, true, true);
            if (originaux[curOrig] == NULL || originaux[curOrig]->descr_cmd.size() <= 1)
            {
                for (int i = curOrig; i >= 0; i--) delete originaux[i];
                g_slist_free(il);
                return;
            }
            curOrig++;
        }
    }
    // reverse if needed
    // note that the selection list keeps its order
    if ( reverseOrderForOp ) {
        Path* swap=originaux[0];originaux[0]=originaux[1];originaux[1]=swap;
        FillRule swai=origWind[0]; origWind[0]=origWind[1]; origWind[1]=swai;
    }

    // and work
    // some temporary instances, first
    Shape *theShapeA = new Shape;
    Shape *theShapeB = new Shape;
    Shape *theShape = new Shape;
    Path *res = new Path;
    res->SetBackData(false);
    Path::cut_position  *toCut=NULL;
    int                  nbToCut=0;

    if ( bop == bool_op_inters || bop == bool_op_union || bop == bool_op_diff || bop == bool_op_symdiff ) {
        // true boolean op
        // get the polygons of each path, with the winding rule specified, and apply the operation iteratively
        originaux[0]->ConvertWithBackData(0.1);

        originaux[0]->Fill(theShape, 0);

        theShapeA->ConvertToShape(theShape, origWind[0]);

        curOrig = 1;
        for (GSList *l = il->next; l != NULL; l = l->next) {
            originaux[curOrig]->ConvertWithBackData(0.1);

            originaux[curOrig]->Fill(theShape, curOrig);

            theShapeB->ConvertToShape(theShape, origWind[curOrig]);

            // les elements arrivent en ordre inverse dans la liste
            theShape->Booleen(theShapeB, theShapeA, bop);

            {
                Shape *swap = theShape;
                theShape = theShapeA;
                theShapeA = swap;
            }
            curOrig++;
        }

        {
            Shape *swap = theShape;
            theShape = theShapeA;
            theShapeA = swap;
        }

    } else if ( bop == bool_op_cut ) {
        // cuts= sort of a bastard boolean operation, thus not the axact same modus operandi
        // technically, the cut path is not necessarily a polygon (thus has no winding rule)
        // it is just uncrossed, and cleaned from duplicate edges and points
        // then it's fed to Booleen() which will uncross it against the other path
        // then comes the trick: each edge of the cut path is duplicated (one in each direction),
        // thus making a polygon. the weight of the edges of the cut are all 0, but
        // the Booleen need to invert the ones inside the source polygon (for the subsequent
        // ConvertToForme)

        // the cut path needs to have the highest pathID in the back data
        // that's how the Booleen() function knows it's an edge of the cut

        // FIXME: this gives poor results, the final paths are full of extraneous nodes. Decreasing
        // ConvertWithBackData parameter below simply increases the number of nodes, so for now I
        // left it at 1.0. Investigate replacing this by a combination of difference and
        // intersection of the same two paths. -- bb
        {
            Path* swap=originaux[0];originaux[0]=originaux[1];originaux[1]=swap;
            int   swai=origWind[0];origWind[0]=origWind[1];origWind[1]=(fill_typ)swai;
        }
        originaux[0]->ConvertWithBackData(1.0);

        originaux[0]->Fill(theShape, 0);

        theShapeA->ConvertToShape(theShape, origWind[0]);

        originaux[1]->ConvertWithBackData(1.0);

        originaux[1]->Fill(theShape, 1,false,false,false); //do not closeIfNeeded

        theShapeB->ConvertToShape(theShape, fill_justDont); // fill_justDont doesn't computes winding numbers

        // les elements arrivent en ordre inverse dans la liste
        theShape->Booleen(theShapeB, theShapeA, bool_op_cut, 1);

    } else if ( bop == bool_op_slice ) {
        // slice is not really a boolean operation
        // you just put the 2 shapes in a single polygon, uncross it
        // the points where the degree is > 2 are intersections
        // just check it's an intersection on the path you want to cut, and keep it
        // the intersections you have found are then fed to ConvertPositionsToMoveTo() which will
        // make new subpath at each one of these positions
        // inversion pour l'opration
        {
            Path* swap=originaux[0];originaux[0]=originaux[1];originaux[1]=swap;
            int   swai=origWind[0];origWind[0]=origWind[1];origWind[1]=(fill_typ)swai;
        }
        originaux[0]->ConvertWithBackData(1.0);

        originaux[0]->Fill(theShapeA, 0,false,false,false); // don't closeIfNeeded

        originaux[1]->ConvertWithBackData(1.0);

        originaux[1]->Fill(theShapeA, 1,true,false,false);// don't closeIfNeeded and just dump in the shape, don't reset it

        theShape->ConvertToShape(theShapeA, fill_justDont);

        if ( theShape->hasBackData() ) {
            // should always be the case, but ya never know
            {
                for (int i = 0; i < theShape->numberOfPoints(); i++) {
                    if ( theShape->getPoint(i).totalDegree() > 2 ) {
                        // possibly an intersection
                        // we need to check that at least one edge from the source path is incident to it
                        // before we declare it's an intersection
                        int cb = theShape->getPoint(i).incidentEdge[FIRST];
                        int   nbOrig=0;
                        int   nbOther=0;
                        int   piece=-1;
                        float t=0.0;
                        while ( cb >= 0 && cb < theShape->numberOfEdges() ) {
                            if ( theShape->ebData[cb].pathID == 0 ) {
                                // the source has an edge incident to the point, get its position on the path
                                piece=theShape->ebData[cb].pieceID;
                                if ( theShape->getEdge(cb).st == i ) {
                                    t=theShape->ebData[cb].tSt;
                                } else {
                                    t=theShape->ebData[cb].tEn;
                                }
                                nbOrig++;
                            }
                            if ( theShape->ebData[cb].pathID == 1 ) nbOther++; // the cut is incident to this point
                            cb=theShape->NextAt(i, cb);
                        }
                        if ( nbOrig > 0 && nbOther > 0 ) {
                            // point incident to both path and cut: an intersection
                            // note that you only keep one position on the source; you could have degenerate
                            // cases where the source crosses itself at this point, and you wouyld miss an intersection
                            toCut=(Path::cut_position*)realloc(toCut, (nbToCut+1)*sizeof(Path::cut_position));
                            toCut[nbToCut].piece=piece;
                            toCut[nbToCut].t=t;
                            nbToCut++;
                        }
                    }
                }
            }
            {
                // i think it's useless now
                int i = theShape->numberOfEdges() - 1;
                for (;i>=0;i--) {
                    if ( theShape->ebData[i].pathID == 1 ) {
                        theShape->SubEdge(i);
                    }
                }
            }

        }
    }

    int*    nesting=NULL;
    int*    conts=NULL;
    int     nbNest=0;
    // pour compenser le swap juste avant
    if ( bop == bool_op_slice ) {
//    theShape->ConvertToForme(res, nbOriginaux, originaux, true);
//    res->ConvertForcedToMoveTo();
        res->Copy(originaux[0]);
        res->ConvertPositionsToMoveTo(nbToCut, toCut); // cut where you found intersections
        free(toCut);
    } else if ( bop == bool_op_cut ) {
        // il faut appeler pour desallouer PointData (pas vital, mais bon)
        // the Booleen() function did not deallocated the point_data array in theShape, because this
        // function needs it.
        // this function uses the point_data to get the winding number of each path (ie: is a hole or not)
        // for later reconstruction in objects, you also need to extract which path is parent of holes (nesting info)
        theShape->ConvertToFormeNested(res, nbOriginaux, &originaux[0], 1, nbNest, nesting, conts);
    } else {
        theShape->ConvertToForme(res, nbOriginaux, &originaux[0]);
    }

    delete theShape;
    delete theShapeA;
    delete theShapeB;
    for (int i = 0; i < nbOriginaux; i++)  delete originaux[i];

    if (res->descr_cmd.size() <= 1)
    {
        // only one command, presumably a moveto: it isn't a path
        for (GSList *l = il; l != NULL; l = l->next)
        {
            SP_OBJECT(l->data)->deleteObject();
        }
        sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE, 
                         description);
        selection->clear();

        delete res;
        g_slist_free(il);
        return;
    }

    // get the source path object
    SPObject *source;
    if ( bop == bool_op_diff || bop == bool_op_symdiff || bop == bool_op_cut || bop == bool_op_slice ) {
        if (reverseOrderForOp) {
             source = SP_OBJECT(il->data);
        } else {
             source = SP_OBJECT(il->next->data);
        }
    } else {
        // find out the bottom object
        GSList *sorted = g_slist_copy((GSList *) selection->reprList());

        sorted = g_slist_sort(sorted, (GCompareFunc) sp_repr_compare_position);

        source = sp_desktop_document(desktop)->
            getObjectByRepr((Inkscape::XML::Node *)sorted->data);

        g_slist_free(sorted);
    }

    // adjust style properties that depend on a possible transform in the source object in order
    // to get a correct style attribute for the new path
    SPItem* item_source = SP_ITEM(source);
    Geom::Matrix i2doc(sp_item_i2doc_affine(item_source));
    sp_item_adjust_stroke(item_source, i2doc.descrim());
    sp_item_adjust_pattern(item_source, i2doc);
    sp_item_adjust_gradient(item_source, i2doc);
    sp_item_adjust_livepatheffect(item_source, i2doc);

    Inkscape::XML::Node *repr_source = SP_OBJECT_REPR(source);

    // remember important aspects of the source path, to be restored
    gint pos = repr_source->position();
    Inkscape::XML::Node *parent = sp_repr_parent(repr_source);
    gchar const *id = repr_source->attribute("id");
    gchar const *style = repr_source->attribute("style");
    gchar const *mask = repr_source->attribute("mask");
    gchar const *clip_path = repr_source->attribute("clip-path");
    gchar *title = source->title();
    gchar *desc = source->desc();
    // remove source paths
    selection->clear();
    for (GSList *l = il; l != NULL; l = l->next) {
        // if this is the bottommost object,
        if (!strcmp(SP_OBJECT_REPR(l->data)->attribute("id"), id)) {
            // delete it so that its clones don't get alerted; this object will be restored shortly, with the same id
            SP_OBJECT(l->data)->deleteObject(false);
        } else {
            // delete the object for real, so that its clones can take appropriate action
            SP_OBJECT(l->data)->deleteObject();
        }
    }
    g_slist_free(il);

    // premultiply by the inverse of parent's repr
    SPItem *parent_item = SP_ITEM(sp_desktop_document(desktop)->getObjectByRepr(parent));
    Geom::Matrix local (sp_item_i2doc_affine(parent_item));
    gchar *transform = sp_svg_transform_write(local.inverse());

    // now that we have the result, add it on the canvas
    if ( bop == bool_op_cut || bop == bool_op_slice ) {
        int    nbRP=0;
        Path** resPath;
        if ( bop == bool_op_slice ) {
            // there are moveto's at each intersection, but it's still one unique path
            // so break it down and add each subpath independently
            // we could call break_apart to do this, but while we have the description...
            resPath=res->SubPaths(nbRP, false);
        } else {
            // cut operation is a bit wicked: you need to keep holes
            // that's why you needed the nesting
            // ConvertToFormeNested() dumped all the subpath in a single Path "res", so we need
            // to get the path for each part of the polygon. that's why you need the nesting info:
            // to know in wich subpath to add a subpath
            resPath=res->SubPathsWithNesting(nbRP, true, nbNest, nesting, conts);

            // cleaning
            if ( conts ) free(conts);
            if ( nesting ) free(nesting);
        }

        // add all the pieces resulting from cut or slice
        for (int i=0;i<nbRP;i++) {
            gchar *d = resPath[i]->svg_dump_path();

            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
            Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
            repr->setAttribute("style", style);
            if (mask)
                repr->setAttribute("mask", mask);
            if (clip_path)
                repr->setAttribute("clip-path", clip_path);

            repr->setAttribute("d", d);
            g_free(d);

            // for slice, remove fill
            if (bop == bool_op_slice) {
                SPCSSAttr *css;

                css = sp_repr_css_attr_new();
                sp_repr_css_set_property(css, "fill", "none");

                sp_repr_css_change(repr, css, "style");

                sp_repr_css_attr_unref(css);
            }

            // we assign the same id on all pieces, but it on adding to document, it will be changed on all except one
            // this means it's basically random which of the pieces inherits the original's id and clones
            // a better algorithm might figure out e.g. the biggest piece
            repr->setAttribute("id", id);

            repr->setAttribute("transform", transform);

            // add the new repr to the parent
            parent->appendChild(repr);

            // move to the saved position
            repr->setPosition(pos > 0 ? pos : 0);

            selection->add(repr);
            Inkscape::GC::release(repr);

            delete resPath[i];
        }
        if ( resPath ) free(resPath);

    } else {
        gchar *d = res->svg_dump_path();

        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        repr->setAttribute("style", style);

        if ( mask )
            repr->setAttribute("mask", mask);

        if ( clip_path )
            repr->setAttribute("clip-path", clip_path);

        repr->setAttribute("d", d);
        g_free(d);

        repr->setAttribute("transform", transform);

        repr->setAttribute("id", id);
        parent->appendChild(repr);
        if (title) {
        	sp_desktop_document(desktop)->getObjectByRepr(repr)->setTitle(title);
        }            
        if (desc) {
        	sp_desktop_document(desktop)->getObjectByRepr(repr)->setDesc(desc);
        }
		repr->setPosition(pos > 0 ? pos : 0);

        selection->add(repr);
        Inkscape::GC::release(repr);
    }

    g_free(transform);
    if (title) g_free(title);
    if (desc) g_free(desc);

    if (verb != SP_VERB_NONE) {
        sp_document_done(sp_desktop_document(desktop), verb, description);
    }

    delete res;
}

static
void sp_selected_path_outline_add_marker( SPObject *marker_object, Geom::Matrix marker_transform,
                                          Geom::Scale stroke_scale, Geom::Matrix transform,
                                          Inkscape::XML::Node *g_repr, Inkscape::XML::Document *xml_doc, SPDocument * doc )
{
    SPMarker* marker = SP_MARKER (marker_object);
    SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (marker_object));

    Geom::Matrix tr(marker_transform);

    if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
        tr = stroke_scale * tr;
    }

    // total marker transform
    tr = marker_item->transform * marker->c2p * tr * transform;

    if (SP_OBJECT_REPR(marker_item)) {
        Inkscape::XML::Node *m_repr = SP_OBJECT_REPR(marker_item)->duplicate(xml_doc);
        g_repr->appendChild(m_repr);
        SPItem *marker_item = (SPItem *) doc->getObjectByRepr(m_repr);
        sp_item_write_transform(marker_item, m_repr, tr);
    }
}

void
sp_selected_path_outline(SPDesktop *desktop)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>stroked path(s)</b> to convert stroke to path."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (!SP_IS_SHAPE(item) && !SP_IS_TEXT(item))
            continue;

        SPCurve *curve = NULL;
        if (SP_IS_SHAPE(item)) {
            curve = sp_shape_get_curve(SP_SHAPE(item));
            if (curve == NULL)
                continue;
        }
        if (SP_IS_TEXT(item)) {
            curve = SP_TEXT(item)->getNormalizedBpath();
            if (curve == NULL)
                continue;
        }

        // pas de stroke pas de chocolat
        if (!SP_OBJECT_STYLE(item) || SP_OBJECT_STYLE(item)->stroke.noneSet) {
            curve->unref();
            continue;
        }

        // remember old stroke style, to be set on fill
        SPStyle *i_style = SP_OBJECT_STYLE(item);
        SPCSSAttr *ncss;
        {
            ncss = sp_css_attr_from_style(i_style, SP_STYLE_FLAG_ALWAYS);
            gchar const *s_val = sp_repr_css_property(ncss, "stroke", NULL);
            gchar const *s_opac = sp_repr_css_property(ncss, "stroke-opacity", NULL);

            sp_repr_css_set_property(ncss, "stroke", "none");
            sp_repr_css_set_property(ncss, "stroke-opacity", "1.0");
            sp_repr_css_set_property(ncss, "fill", s_val);
            if ( s_opac ) {
                sp_repr_css_set_property(ncss, "fill-opacity", s_opac);
            } else {
                sp_repr_css_set_property(ncss, "fill-opacity", "1.0");
            }
            sp_repr_css_unset_property(ncss, "marker-start");
            sp_repr_css_unset_property(ncss, "marker-mid");
            sp_repr_css_unset_property(ncss, "marker-end");
        }

        Geom::Matrix const transform(item->transform);
        float const scale = transform.descrim();
        gchar const *mask = SP_OBJECT_REPR(item)->attribute("mask");
        gchar const *clip_path = SP_OBJECT_REPR(item)->attribute("clip-path");

        float o_width, o_miter;
        JoinType o_join;
        ButtType o_butt;

        {
            int jointype, captype;

            jointype = i_style->stroke_linejoin.computed;
            captype = i_style->stroke_linecap.computed;
            o_width = i_style->stroke_width.computed;

            switch (jointype) {
                case SP_STROKE_LINEJOIN_MITER:
                    o_join = join_pointy;
                    break;
                case SP_STROKE_LINEJOIN_ROUND:
                    o_join = join_round;
                    break;
                default:
                    o_join = join_straight;
                    break;
            }

            switch (captype) {
                case SP_STROKE_LINECAP_SQUARE:
                    o_butt = butt_square;
                    break;
                case SP_STROKE_LINECAP_ROUND:
                    o_butt = butt_round;
                    break;
                default:
                    o_butt = butt_straight;
                    break;
            }

            if (o_width < 0.1)
                o_width = 0.1;
            o_miter = i_style->stroke_miterlimit.value * o_width;
        }

        SPCurve *curvetemp = curve_for_item(item);
        if (curvetemp == NULL) {
            curve->unref();
            continue;
        }
        // Livarots outline of arcs is broken. So convert the path to linear and cubics only, for which the outline is created correctly.
        Geom::PathVector pathv = pathv_to_linear_and_cubic_beziers( curvetemp->get_pathvector() );
        curvetemp->unref();

        Path *orig = new Path;
        orig->LoadPathVector(pathv);

        Path *res = new Path;
        res->SetBackData(false);

        if (i_style->stroke_dash.n_dash) {
            // For dashed strokes, use Stroke method, because Outline can't do dashes
            // However Stroke adds lots of extra nodes _or_ makes the path crooked, so consider this a temporary workaround

            orig->ConvertWithBackData(0.1);

            orig->DashPolylineFromStyle(i_style, scale, 0);

            Shape* theShape = new Shape;
            orig->Stroke(theShape, false, 0.5*o_width, o_join, o_butt,
                         0.5 * o_miter);
            orig->Outline(res, 0.5 * o_width, o_join, o_butt, 0.5 * o_miter);

            Shape *theRes = new Shape;

            theRes->ConvertToShape(theShape, fill_positive);

            Path *originaux[1];
            originaux[0] = res;
            theRes->ConvertToForme(orig, 1, originaux);

            res->Coalesce(5.0);

            delete theShape;
            delete theRes;

        } else {

            orig->Outline(res, 0.5 * o_width, o_join, o_butt, 0.5 * o_miter);

            orig->Coalesce(0.5 * o_width);

            Shape *theShape = new Shape;
            Shape *theRes = new Shape;

            res->ConvertWithBackData(1.0);
            res->Fill(theShape, 0);
            theRes->ConvertToShape(theShape, fill_positive);

            Path *originaux[1];
            originaux[0] = res;
            theRes->ConvertToForme(orig, 1, originaux);

            delete theShape;
            delete theRes;
        }

        if (orig->descr_cmd.size() <= 1) {
            // ca a merd, ou bien le resultat est vide
            delete res;
            delete orig;
            continue;
        }

        did = true;

        // remember the position of the item
        gint pos = SP_OBJECT_REPR(item)->position();
        // remember parent
        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        // remember id
        char const *id = SP_OBJECT_REPR(item)->attribute("id");
        // remember title
        gchar *title = item->title();
        // remember description
        gchar *desc = item->desc();
        
        if (res->descr_cmd.size() > 1) { // if there's 0 or 1 node left, drop this path altogether

            SPDocument * doc = sp_desktop_document(desktop);
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
            Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

            // restore old style, but set old stroke style on fill
            sp_repr_css_change(repr, ncss, "style");

            sp_repr_css_attr_unref(ncss);

            gchar *str = orig->svg_dump_path();
            repr->setAttribute("d", str);
            g_free(str);

            if (mask)
                repr->setAttribute("mask", mask);
            if (clip_path)
                repr->setAttribute("clip-path", clip_path);

            if (SP_IS_SHAPE(item) && sp_shape_has_markers (SP_SHAPE(item))) {

                Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
                Inkscape::XML::Node *g_repr = xml_doc->createElement("svg:g");

                // add the group to the parent
                parent->appendChild(g_repr);
                // move to the saved position
                g_repr->setPosition(pos > 0 ? pos : 0);

                g_repr->appendChild(repr);
                // restore title, description, id, transform
                repr->setAttribute("id", id);
                SPItem *newitem = (SPItem *) doc->getObjectByRepr(repr);
                sp_item_write_transform(newitem, repr, transform);
                if (title) {
                	newitem->setTitle(title);
                }
                if (desc) {
                	newitem->setDesc(desc);
                }
                
                SPShape *shape = SP_SHAPE(item);

                Geom::PathVector const & pathv = curve->get_pathvector();
                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    for (int i = 0; i < 2; i++) {  // SP_MARKER_LOC and SP_MARKER_LOC_START
                        if ( SPObject *marker_obj = shape->marker[i] ) {
                            Geom::Matrix const m (sp_shape_marker_get_transform_at_start(path_it->front()));
                            sp_selected_path_outline_add_marker( marker_obj, m,
                                                                 Geom::Scale(i_style->stroke_width.computed), transform,
                                                                 g_repr, xml_doc, doc );
                        }
                    }

                    for (int i = 0; i < 3; i += 2) {  // SP_MARKER_LOC and SP_MARKER_LOC_MID
                        SPObject *midmarker_obj = shape->marker[i];
                        if ( midmarker_obj && (path_it->size_default() > 1) ) {
                            Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
                            Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
                            while (curve_it2 != path_it->end_default())
                            {
                                /* Put marker between curve_it1 and curve_it2.
                                 * Loop to end_default (so including closing segment), because when a path is closed,
                                 * there should be a midpoint marker between last segment and closing straight line segment
                                 */
                                Geom::Matrix const m (sp_shape_marker_get_transform(*curve_it1, *curve_it2));
                                sp_selected_path_outline_add_marker(midmarker_obj, m,
                                                                    Geom::Scale(i_style->stroke_width.computed), transform,
                                                                    g_repr, xml_doc, doc);

                                ++curve_it1;
                                ++curve_it2;
                            }
                        }
                    }

                    for (int i = 0; i < 4; i += 3) {  // SP_MARKER_LOC and SP_MARKER_LOC_END
                        if ( SPObject *marker_obj = shape->marker[i] ) {
                            /* Get reference to last curve in the path.
                             * For moveto-only path, this returns the "closing line segment". */
                            unsigned int index = path_it->size_default();
                            if (index > 0) {
                                index--;
                            }
                            Geom::Curve const &lastcurve = (*path_it)[index];

                            Geom::Matrix const m = sp_shape_marker_get_transform_at_end(lastcurve);
                            sp_selected_path_outline_add_marker( marker_obj, m,
                                                                 Geom::Scale(i_style->stroke_width.computed), transform,
                                                                 g_repr, xml_doc, doc );
                        }
                    }
                }

                selection->add(g_repr);

                Inkscape::GC::release(g_repr);


            } else {

                // add the new repr to the parent
                parent->appendChild(repr);

                // move to the saved position
                repr->setPosition(pos > 0 ? pos : 0);

                // restore title, description, id, transform
                repr->setAttribute("id", id);

                SPItem *newitem = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);
                sp_item_write_transform(newitem, repr, transform);
                if (title) {
                	newitem->setTitle(title);
                }
                if (desc) {
                	newitem->setDesc(desc);
                }
                
                selection->add(repr);

            }

            Inkscape::GC::release(repr);

            curve->unref();
            selection->remove(item);
            SP_OBJECT(item)->deleteObject(false);

        }
        if (title) g_free(title);
        if (desc) g_free(desc);

        delete res;
        delete orig;
    }

    if (did) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_OUTLINE, 
                         _("Convert stroke to path"));
    } else {
        // TRANSLATORS: "to outline" means "to convert stroke to path"
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No stroked paths</b> in the selection."));
        return;
    }
}


void
sp_selected_path_offset(SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double prefOffset = prefs->getDouble("/options/defaultoffsetwidth/value", 1.0);

    sp_selected_path_do_offset(desktop, true, prefOffset);
}
void
sp_selected_path_inset(SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double prefOffset = prefs->getDouble("/options/defaultoffsetwidth/value", 1.0);

    sp_selected_path_do_offset(desktop, false, prefOffset);
}

void
sp_selected_path_offset_screen(SPDesktop *desktop, double pixels)
{
    sp_selected_path_do_offset(desktop, true,  pixels / desktop->current_zoom());
}

void
sp_selected_path_inset_screen(SPDesktop *desktop, double pixels)
{
    sp_selected_path_do_offset(desktop, false,  pixels / desktop->current_zoom());
}


void sp_selected_path_create_offset_object_zero(SPDesktop *desktop)
{
    sp_selected_path_create_offset_object(desktop, 0, false);
}

void sp_selected_path_create_offset(SPDesktop *desktop)
{
    sp_selected_path_create_offset_object(desktop, 1, false);
}
void sp_selected_path_create_inset(SPDesktop *desktop)
{
    sp_selected_path_create_offset_object(desktop, -1, false);
}

void sp_selected_path_create_updating_offset_object_zero(SPDesktop *desktop)
{
    sp_selected_path_create_offset_object(desktop, 0, true);
}

void sp_selected_path_create_updating_offset(SPDesktop *desktop)
{
    sp_selected_path_create_offset_object(desktop, 1, true);
}
void sp_selected_path_create_updating_inset(SPDesktop *desktop)
{
    sp_selected_path_create_offset_object(desktop, -1, true);
}

void
sp_selected_path_create_offset_object(SPDesktop *desktop, int expand, bool updating)
{
    Inkscape::Selection *selection;
    Inkscape::XML::Node *repr;
    SPItem *item;
    SPCurve *curve;
    gchar *style, *str;
    float o_width, o_miter;
    JoinType o_join;
    ButtType o_butt;

    curve = NULL;

    selection = sp_desktop_selection(desktop);

    item = selection->singleItem();

    if (item == NULL || ( !SP_IS_SHAPE(item) && !SP_IS_TEXT(item) ) ) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Selected object is <b>not a path</b>, cannot inset/outset."));
        return;
    }
    if (SP_IS_SHAPE(item))
    {
        curve = sp_shape_get_curve(SP_SHAPE(item));
        if (curve == NULL)
            return;
    }
    if (SP_IS_TEXT(item))
    {
        curve = SP_TEXT(item)->getNormalizedBpath();
        if (curve == NULL)
            return;
    }

    Geom::Matrix const transform(item->transform);

    sp_item_write_transform(item, SP_OBJECT_REPR(item), Geom::identity());

    style = g_strdup(SP_OBJECT(item)->repr->attribute("style"));

    // remember the position of the item
    gint pos = SP_OBJECT_REPR(item)->position();
    // remember parent
    Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();

    {
        SPStyle *i_style = SP_OBJECT(item)->style;
        int jointype, captype;

        jointype = i_style->stroke_linejoin.value;
        captype = i_style->stroke_linecap.value;
        o_width = i_style->stroke_width.computed;
        if (jointype == SP_STROKE_LINEJOIN_MITER)
        {
            o_join = join_pointy;
        }
        else if (jointype == SP_STROKE_LINEJOIN_ROUND)
        {
            o_join = join_round;
        }
        else
        {
            o_join = join_straight;
        }
        if (captype == SP_STROKE_LINECAP_SQUARE)
        {
            o_butt = butt_square;
        }
        else if (captype == SP_STROKE_LINECAP_ROUND)
        {
            o_butt = butt_round;
        }
        else
        {
            o_butt = butt_straight;
        }

        {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            o_width = prefs->getDouble("/options/defaultoffsetwidth/value", 1.0);
        }

        if (o_width < 0.01)
            o_width = 0.01;
        o_miter = i_style->stroke_miterlimit.value * o_width;
    }

    Path *orig = Path_for_item(item, true, false);
    if (orig == NULL)
    {
        g_free(style);
        curve->unref();
        return;
    }

    Path *res = new Path;
    res->SetBackData(false);

    {
        Shape *theShape = new Shape;
        Shape *theRes = new Shape;

        orig->ConvertWithBackData(1.0);
        orig->Fill(theShape, 0);

        SPCSSAttr *css = sp_repr_css_attr(SP_OBJECT_REPR(item), "style");
        gchar const *val = sp_repr_css_property(css, "fill-rule", NULL);
        if (val && strcmp(val, "nonzero") == 0)
        {
            theRes->ConvertToShape(theShape, fill_nonZero);
        }
        else if (val && strcmp(val, "evenodd") == 0)
        {
            theRes->ConvertToShape(theShape, fill_oddEven);
        }
        else
        {
            theRes->ConvertToShape(theShape, fill_nonZero);
        }

        Path *originaux[1];
        originaux[0] = orig;
        theRes->ConvertToForme(res, 1, originaux);

        delete theShape;
        delete theRes;
    }

    curve->unref();

    if (res->descr_cmd.size() <= 1)
    {
        // pas vraiment de points sur le resultat
        // donc il ne reste rien
        sp_document_done(sp_desktop_document(desktop), 
                         (updating ? SP_VERB_SELECTION_LINKED_OFFSET 
                          : SP_VERB_SELECTION_DYNAMIC_OFFSET),
                         (updating ? _("Create linked offset")
                          : _("Create dynamic offset")));
        selection->clear();

        delete res;
        delete orig;
        g_free(style);
        return;
    }

    {
        gchar tstr[80];

        tstr[79] = '\0';

        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
        repr = xml_doc->createElement("svg:path");
        repr->setAttribute("sodipodi:type", "inkscape:offset");
        sp_repr_set_svg_double(repr, "inkscape:radius", ( expand > 0
                                                          ? o_width
                                                          : expand < 0
                                                          ? -o_width
                                                          : 0 ));

        str = res->svg_dump_path();
        repr->setAttribute("inkscape:original", str);
        g_free(str);

        if ( updating ) {
            char const *id = SP_OBJECT(item)->repr->attribute("id");
            char const *uri = g_strdup_printf("#%s", id);
            repr->setAttribute("xlink:href", uri);
            g_free((void *) uri);
        } else {
            repr->setAttribute("inkscape:href", NULL);
        }

        repr->setAttribute("style", style);

        // add the new repr to the parent
        parent->appendChild(repr);

        // move to the saved position
        repr->setPosition(pos > 0 ? pos : 0);

        SPItem *nitem = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);

        if ( updating ) {
            // on conserve l'original
            // we reapply the transform to the original (offset will feel it)
            sp_item_write_transform(item, SP_OBJECT_REPR(item), transform);
        } else {
            // delete original, apply the transform to the offset
            SP_OBJECT(item)->deleteObject(false);
            sp_item_write_transform(nitem, repr, transform);
        }

        // The object just created from a temporary repr is only a seed.
        // We need to invoke its write which will update its real repr (in particular adding d=)
        SP_OBJECT(nitem)->updateRepr();

        Inkscape::GC::release(repr);

        selection->set(nitem);
    }

    sp_document_done(sp_desktop_document(desktop), 
                     (updating ? SP_VERB_SELECTION_LINKED_OFFSET 
                      : SP_VERB_SELECTION_DYNAMIC_OFFSET),
                     (updating ? _("Create linked offset")
                      : _("Create dynamic offset")));

    delete res;
    delete orig;

    g_free(style);
}












void
sp_selected_path_do_offset(SPDesktop *desktop, bool expand, double prefOffset)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>path(s)</b> to inset/outset."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (!SP_IS_SHAPE(item) && !SP_IS_TEXT(item))
            continue;

        SPCurve *curve = NULL;
        if (SP_IS_SHAPE(item)) {
            curve = sp_shape_get_curve(SP_SHAPE(item));
            if (curve == NULL)
                continue;
        }
        if (SP_IS_TEXT(item)) {
            curve = SP_TEXT(item)->getNormalizedBpath();
            if (curve == NULL)
                continue;
        }

        Geom::Matrix const transform(item->transform);

        sp_item_write_transform(item, SP_OBJECT_REPR(item), Geom::identity());

        gchar *style = g_strdup(SP_OBJECT_REPR(item)->attribute("style"));

        float o_width, o_miter;
        JoinType o_join;
        ButtType o_butt;

        {
            SPStyle *i_style = SP_OBJECT(item)->style;
            int jointype, captype;

            jointype = i_style->stroke_linejoin.value;
            captype = i_style->stroke_linecap.value;
            o_width = i_style->stroke_width.computed;

            switch (jointype) {
                case SP_STROKE_LINEJOIN_MITER:
                    o_join = join_pointy;
                    break;
                case SP_STROKE_LINEJOIN_ROUND:
                    o_join = join_round;
                    break;
                default:
                    o_join = join_straight;
                    break;
            }

            switch (captype) {
                case SP_STROKE_LINECAP_SQUARE:
                    o_butt = butt_square;
                    break;
                case SP_STROKE_LINECAP_ROUND:
                    o_butt = butt_round;
                    break;
                default:
                    o_butt = butt_straight;
                    break;
            }

            o_width = prefOffset;

            if (o_width < 0.1)
                o_width = 0.1;
            o_miter = i_style->stroke_miterlimit.value * o_width;
        }

        Path *orig = Path_for_item(item, false);
        if (orig == NULL) {
            g_free(style);
            curve->unref();
            continue;
        }

        Path *res = new Path;
        res->SetBackData(false);

        {
            Shape *theShape = new Shape;
            Shape *theRes = new Shape;

            orig->ConvertWithBackData(0.03);
            orig->Fill(theShape, 0);

            SPCSSAttr *css = sp_repr_css_attr(SP_OBJECT_REPR(item), "style");
            gchar const *val = sp_repr_css_property(css, "fill-rule", NULL);
            if (val && strcmp(val, "nonzero") == 0)
            {
                theRes->ConvertToShape(theShape, fill_nonZero);
            }
            else if (val && strcmp(val, "evenodd") == 0)
            {
                theRes->ConvertToShape(theShape, fill_oddEven);
            }
            else
            {
                theRes->ConvertToShape(theShape, fill_nonZero);
            }

            // et maintenant: offset
            // methode inexacte
/*			Path *originaux[1];
			originaux[0] = orig;
			theRes->ConvertToForme(res, 1, originaux);

			if (expand) {
                        res->OutsideOutline(orig, 0.5 * o_width, o_join, o_butt, o_miter);
			} else {
                        res->OutsideOutline(orig, -0.5 * o_width, o_join, o_butt, o_miter);
			}

			orig->ConvertWithBackData(1.0);
			orig->Fill(theShape, 0);
			theRes->ConvertToShape(theShape, fill_positive);
			originaux[0] = orig;
			theRes->ConvertToForme(res, 1, originaux);

			if (o_width >= 0.5) {
                        //     res->Coalesce(1.0);
                        res->ConvertEvenLines(1.0);
                        res->Simplify(1.0);
			} else {
                        //      res->Coalesce(o_width);
                        res->ConvertEvenLines(1.0*o_width);
                        res->Simplify(1.0 * o_width);
			}    */
            // methode par makeoffset

            if (expand)
            {
                theShape->MakeOffset(theRes, o_width, o_join, o_miter);
            }
            else
            {
                theShape->MakeOffset(theRes, -o_width, o_join, o_miter);
            }
            theRes->ConvertToShape(theShape, fill_positive);

            res->Reset();
            theRes->ConvertToForme(res);

            if (o_width >= 1.0)
            {
                res->ConvertEvenLines(1.0);
                res->Simplify(1.0);
            }
            else
            {
                res->ConvertEvenLines(1.0*o_width);
                res->Simplify(1.0 * o_width);
            }

            delete theShape;
            delete theRes;
        }

        did = true;

        curve->unref();
        // remember the position of the item
        gint pos = SP_OBJECT_REPR(item)->position();
        // remember parent
        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        // remember id
        char const *id = SP_OBJECT_REPR(item)->attribute("id");

        selection->remove(item);
        SP_OBJECT(item)->deleteObject(false);

        if (res->descr_cmd.size() > 1) { // if there's 0 or 1 node left, drop this path altogether

            gchar tstr[80];

            tstr[79] = '\0';

            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
            Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

            repr->setAttribute("style", style);

            gchar *str = res->svg_dump_path();
            repr->setAttribute("d", str);
            g_free(str);

            // add the new repr to the parent
            parent->appendChild(repr);

            // move to the saved position
            repr->setPosition(pos > 0 ? pos : 0);

            SPItem *newitem = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);

            // reapply the transform
            sp_item_write_transform(newitem, repr, transform);

            repr->setAttribute("id", id);

            selection->add(repr);

            Inkscape::GC::release(repr);
        }

        delete orig;
        delete res;
    }

    if (did) {
        sp_document_done(sp_desktop_document(desktop), 
                         (expand ? SP_VERB_SELECTION_OFFSET : SP_VERB_SELECTION_INSET),
                         (expand ? _("Outset path") : _("Inset path")));
    } else {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No paths</b> to inset/outset in the selection."));
        return;
    }
}


static bool
sp_selected_path_simplify_items(SPDesktop *desktop,
                                Inkscape::Selection *selection, GSList *items,
                                float threshold,  bool justCoalesce,
                                float angleLimit, bool breakableAngles,
                                bool modifySelection);


//return true if we changed something, else false
bool
sp_selected_path_simplify_item(SPDesktop *desktop,
                 Inkscape::Selection *selection, SPItem *item,
                 float threshold,  bool justCoalesce,
                 float angleLimit, bool breakableAngles,
                 gdouble size,     bool modifySelection)
{
    if (!(SP_IS_GROUP(item) || SP_IS_SHAPE(item) || SP_IS_TEXT(item)))
        return false;

    //If this is a group, do the children instead
    if (SP_IS_GROUP(item)) {
        GSList *items = sp_item_group_item_list(SP_GROUP(item));
        
        return sp_selected_path_simplify_items(desktop, selection, items,
                                               threshold, justCoalesce,
                                               angleLimit, breakableAngles,
                                               false);
    }


    SPCurve *curve = NULL;

    if (SP_IS_SHAPE(item)) {
        curve = sp_shape_get_curve(SP_SHAPE(item));
        if (!curve)
            return false;
    }

    if (SP_IS_TEXT(item)) {
        curve = SP_TEXT(item)->getNormalizedBpath();
        if (!curve)
            return false;
    }

    // correct virtual size by full transform (bug #166937)
    size /= sp_item_i2doc_affine(item).descrim();

    // save the transform, to re-apply it after simplification
    Geom::Matrix const transform(item->transform);

    /*
       reset the transform, effectively transforming the item by transform.inverse();
       this is necessary so that the item is transformed twice back and forth,
       allowing all compensations to cancel out regardless of the preferences
    */
    sp_item_write_transform(item, SP_OBJECT_REPR(item), Geom::identity());

    gchar *style = g_strdup(SP_OBJECT_REPR(item)->attribute("style"));
    gchar *mask = g_strdup(SP_OBJECT_REPR(item)->attribute("mask"));
    gchar *clip_path = g_strdup(SP_OBJECT_REPR(item)->attribute("clip-path"));

    Path *orig = Path_for_item(item, false);
    if (orig == NULL) {
        g_free(style);
        curve->unref();
        return false;
    }

    curve->unref();
    // remember the position of the item
    gint pos = SP_OBJECT_REPR(item)->position();
    // remember parent
    Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
    // remember id
    char const *id = SP_OBJECT_REPR(item)->attribute("id");
    // remember path effect
    char const *patheffect = SP_OBJECT_REPR(item)->attribute("inkscape:path-effect");
    // remember title
    gchar *title = item->title();
    // remember description
    gchar *desc = item->desc();
    
    //If a group was selected, to not change the selection list
    if (modifySelection)
        selection->remove(item);

    SP_OBJECT(item)->deleteObject(false);

    if ( justCoalesce ) {
        orig->Coalesce(threshold * size);
    } else {
        orig->ConvertEvenLines(threshold * size);
        orig->Simplify(threshold * size);
    }

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

    // restore style, mask and clip-path
    repr->setAttribute("style", style);
    g_free(style);

    if ( mask ) {
        repr->setAttribute("mask", mask);
        g_free(mask);
    }

    if ( clip_path ) {
        repr->setAttribute("clip-path", clip_path);
        g_free(clip_path);
    }

    // path
    gchar *str = orig->svg_dump_path();
    if (patheffect)
        repr->setAttribute("inkscape:original-d", str);
    else 
        repr->setAttribute("d", str);
    g_free(str);

    // restore id
    repr->setAttribute("id", id);

    // add the new repr to the parent
    parent->appendChild(repr);

    // move to the saved position
    repr->setPosition(pos > 0 ? pos : 0);

    SPItem *newitem = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);

    // reapply the transform
    sp_item_write_transform(newitem, repr, transform);

    // restore path effect
    repr->setAttribute("inkscape:path-effect", patheffect);
    
    // restore title & description
    if (title) {
    	newitem->setTitle(title);
        g_free(title);
    }
    if (desc) {
    	newitem->setDesc(desc);
        g_free(desc);
    }
    
    //If we are not in a selected group
    if (modifySelection)
        selection->add(repr);

    Inkscape::GC::release(repr);

    // clean up
    if (orig) delete orig;

    return true;
}


bool
sp_selected_path_simplify_items(SPDesktop *desktop,
                                Inkscape::Selection *selection, GSList *items,
                                float threshold,  bool justCoalesce,
                                float angleLimit, bool breakableAngles,
                                bool modifySelection)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool simplifyIndividualPaths = prefs->getBool("/options/simplifyindividualpaths/value");

    gchar *simplificationType;
    if (simplifyIndividualPaths) {
        simplificationType = _("Simplifying paths (separately):");
    } else {
        simplificationType = _("Simplifying paths:");
    }

    bool didSomething = false;

    Geom::OptRect selectionBbox = selection->bounds();
    if (!selectionBbox) {
        return false;
    }
    gdouble selectionSize  = L2(selectionBbox->dimensions());

    gdouble simplifySize  = selectionSize;

    int pathsSimplified = 0;
    int totalPathCount  = g_slist_length(items);

    // set "busy" cursor
    desktop->setWaitingCursor();

    for (; items != NULL; items = items->next) {
        SPItem *item = (SPItem *) items->data;

        if (!(SP_IS_GROUP(item) || SP_IS_SHAPE(item) || SP_IS_TEXT(item)))
          continue;

        if (simplifyIndividualPaths) {
            Geom::OptRect itemBbox = item->getBounds(sp_item_i2d_affine(item));
            if (itemBbox) {
                simplifySize      = L2(itemBbox->dimensions());
            } else {
                simplifySize      = 0;
            }
        }

        pathsSimplified++;

        if (pathsSimplified % 20 == 0) {
            gchar *message = g_strdup_printf(_("%s <b>%d</b> of <b>%d</b> paths simplified..."),
                simplificationType, pathsSimplified, totalPathCount);
            desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, message);
        }

        didSomething |= sp_selected_path_simplify_item(desktop, selection, item,
            threshold, justCoalesce, angleLimit, breakableAngles, simplifySize, modifySelection);
    }

    desktop->clearWaitingCursor();

    if (pathsSimplified > 20) {
        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, g_strdup_printf(_("<b>%d</b> paths simplified."), pathsSimplified));
    }

    return didSomething;
}

void
sp_selected_path_simplify_selection(SPDesktop *desktop, float threshold, bool justCoalesce,
                                    float angleLimit, bool breakableAngles)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE,
                         _("Select <b>path(s)</b> to simplify."));
        return;
    }

    GSList *items = g_slist_copy((GSList *) selection->itemList());

    bool didSomething = sp_selected_path_simplify_items(desktop, selection,
                                                        items, threshold,
                                                        justCoalesce,
                                                        angleLimit,
                                                        breakableAngles, true);

    if (didSomething)
        sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_SIMPLIFY, 
                         _("Simplify"));
    else
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No paths</b> to simplify in the selection."));

}


// globals for keeping track of accelerated simplify
static double previousTime      = 0.0;
static gdouble simplifyMultiply = 1.0;

void
sp_selected_path_simplify(SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble simplifyThreshold =
        prefs->getDouble("/options/simplifythreshold/value", 0.003);
    bool simplifyJustCoalesce = prefs->getBool("/options/simplifyjustcoalesce/value", 0);

    //Get the current time
    GTimeVal currentTimeVal;
    g_get_current_time(&currentTimeVal);
    double currentTime = currentTimeVal.tv_sec * 1000000 +
                currentTimeVal.tv_usec;

    //Was the previous call to this function recent? (<0.5 sec)
    if (previousTime > 0.0 && currentTime - previousTime < 500000.0) {

        // add to the threshold 1/2 of its original value
        simplifyMultiply  += 0.5;
        simplifyThreshold *= simplifyMultiply;

    } else {
        // reset to the default
        simplifyMultiply = 1;
    }

    //remember time for next call
    previousTime = currentTime;

    //g_print("%g\n", simplify_threshold);

    //Make the actual call
    sp_selected_path_simplify_selection(desktop, simplifyThreshold,
                                        simplifyJustCoalesce, 0.0, false);
}



// fonctions utilitaires

bool
Ancetre(Inkscape::XML::Node *a, Inkscape::XML::Node *who)
{
    if (who == NULL || a == NULL)
        return false;
    if (who == a)
        return true;
    return Ancetre(sp_repr_parent(a), who);
}

Path *
Path_for_item(SPItem *item, bool doTransformation, bool transformFull)
{
    SPCurve *curve = curve_for_item(item);

    if (curve == NULL)
        return NULL;
    
    Geom::PathVector *pathv = pathvector_for_curve(item, curve, doTransformation, transformFull, Geom::identity(), Geom::identity());
    curve->unref();
    
    Path *dest = new Path;
    dest->LoadPathVector(*pathv);    
    delete pathv;
    
    return dest;
}

/* 
 * NOTE: Returns empty pathvector if curve == NULL
 * TODO: see if calling this method can be optimized. All the pathvector copying might be slow.
 */
Geom::PathVector*
pathvector_for_curve(SPItem *item, SPCurve *curve, bool doTransformation, bool transformFull, Geom::Matrix extraPreAffine, Geom::Matrix extraPostAffine)
{
    if (curve == NULL)
        return NULL;

    Geom::PathVector *dest = new Geom::PathVector;    
    *dest = curve->get_pathvector(); // Make a copy; must be freed by the caller!
    
    if (doTransformation) {
        if (transformFull) {
            *dest *= extraPreAffine * sp_item_i2doc_affine(item) * extraPostAffine;
        } else {
            *dest *= extraPreAffine * (Geom::Matrix)item->transform * extraPostAffine;
        }
    } else {
        *dest *= extraPreAffine * extraPostAffine;
    }
    
    return dest;
}

SPCurve* curve_for_item(SPItem *item)
{
    if (!item) 
        return NULL;
    
    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(item)) {
        if (SP_IS_PATH(item)) {
            curve = sp_path_get_curve_for_edit(SP_PATH(item));
        } else {
            curve = sp_shape_get_curve(SP_SHAPE(item));
        }
    }
    else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))
    {
        curve = te_get_layout(item)->convertToCurves();
    }
    else if (SP_IS_IMAGE(item))
    {
    curve = sp_image_get_curve(SP_IMAGE(item));
    }
    
    return curve; // do not forget to unref the curve at some point!
}

boost::optional<Path::cut_position> get_nearest_position_on_Path(Path *path, Geom::Point p, unsigned seg)
{
    //get nearest position on path
    Path::cut_position pos = path->PointToCurvilignPosition(p, seg);
    return pos;
}

Geom::Point get_point_on_Path(Path *path, int piece, double t)
{
    Geom::Point p;
    path->PointAt(piece, t, p);
    return p;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

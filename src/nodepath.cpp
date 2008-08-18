#define __SP_NODEPATH_C__

/** \file
 * Path handler in node edit mode
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Portions of this code are in public domain; node sculpting functions written by bulia byak are under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gdk/gdkkeysyms.h>
#include "display/canvas-bpath.h"
#include "display/curve.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include "display/sp-canvas-util.h"
#include <glibmm/i18n.h>
#include <2geom/pathvector.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include "helper/units.h"
#include "helper/geom.h"
#include "knot.h"
#include "inkscape.h"
#include "document.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "snap.h"
#include "message-stack.h"
#include "message-context.h"
#include "node-context.h"
#include "lpe-tool-context.h"
#include "shape-editor.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "xml/repr.h"
#include "prefs-utils.h"
#include "sp-metrics.h"
#include "sp-path.h"
#include "libnr/nr-matrix-ops.h"
#include "svg/svg.h"
#include "verbs.h"
#include "display/bezier-utils.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <string>
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/path.h"
#include "util/mathfns.h"
#include "display/snap-indicator.h"
#include "snapped-point.h"

class NR::Matrix;

/// \todo
/// evil evil evil. FIXME: conflict of two different Path classes!
/// There is a conflict in the namespace between two classes named Path.
/// #include "sp-flowtext.h"
/// #include "sp-flowregion.h"

#define SP_TYPE_FLOWREGION            (sp_flowregion_get_type ())
#define SP_IS_FLOWREGION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWREGION))
GType sp_flowregion_get_type (void);
#define SP_TYPE_FLOWTEXT            (sp_flowtext_get_type ())
#define SP_IS_FLOWTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWTEXT))
GType sp_flowtext_get_type (void);
// end evil workaround

#include "helper/stlport.h"


/// \todo fixme: Implement these via preferences */

#define NODE_FILL          0xbfbfbf00
#define NODE_STROKE        0x000000ff
#define NODE_FILL_HI       0xff000000
#define NODE_STROKE_HI     0x000000ff
#define NODE_FILL_SEL      0x0000ffff
#define NODE_STROKE_SEL    0x000000ff
#define NODE_FILL_SEL_HI   0xff000000
#define NODE_STROKE_SEL_HI 0x000000ff
#define KNOT_FILL          0xffffffff
#define KNOT_STROKE        0x000000ff
#define KNOT_FILL_HI       0xff000000
#define KNOT_STROKE_HI     0x000000ff

static GMemChunk *nodechunk = NULL;

/* Creation from object */

static void subpaths_from_pathvector(Inkscape::NodePath::Path *np, Geom::PathVector const & pathv, Inkscape::NodePath::NodeType const *t);
static Inkscape::NodePath::NodeType * parse_nodetypes(gchar const *types, guint length);

/* Object updating */

static void stamp_repr(Inkscape::NodePath::Path *np);
static SPCurve *create_curve(Inkscape::NodePath::Path *np);
static gchar *create_typestr(Inkscape::NodePath::Path *np);

static void sp_node_update_handles(Inkscape::NodePath::Node *node, bool fire_move_signals = true);

static void sp_nodepath_node_select(Inkscape::NodePath::Node *node, gboolean incremental, gboolean override);

static void sp_node_set_selected(Inkscape::NodePath::Node *node, gboolean selected);

static Inkscape::NodePath::Node *sp_nodepath_set_node_type(Inkscape::NodePath::Node *node, Inkscape::NodePath::NodeType type);

/* Adjust handle placement, if the node or the other handle is moved */
static void sp_node_adjust_handle(Inkscape::NodePath::Node *node, gint which_adjust);
static void sp_node_adjust_handles(Inkscape::NodePath::Node *node);

/* Node event callbacks */
static void node_clicked(SPKnot *knot, guint state, gpointer data);
static void node_grabbed(SPKnot *knot, guint state, gpointer data);
static void node_ungrabbed(SPKnot *knot, guint state, gpointer data);
static gboolean node_request(SPKnot *knot, NR::Point *p, guint state, gpointer data);

/* Handle event callbacks */
static void node_handle_clicked(SPKnot *knot, guint state, gpointer data);
static void node_handle_grabbed(SPKnot *knot, guint state, gpointer data);
static void node_handle_ungrabbed(SPKnot *knot, guint state, gpointer data);
static gboolean node_handle_request(SPKnot *knot, NR::Point *p, guint state, gpointer data);
static void node_handle_moved(SPKnot *knot, NR::Point *p, guint state, gpointer data);
static gboolean node_handle_event(SPKnot *knot, GdkEvent *event, Inkscape::NodePath::Node *n);

/* Constructors and destructors */

static Inkscape::NodePath::SubPath *sp_nodepath_subpath_new(Inkscape::NodePath::Path *nodepath);
static void sp_nodepath_subpath_destroy(Inkscape::NodePath::SubPath *subpath);
static void sp_nodepath_subpath_close(Inkscape::NodePath::SubPath *sp);
static void sp_nodepath_subpath_open(Inkscape::NodePath::SubPath *sp,Inkscape::NodePath::Node *n);
static Inkscape::NodePath::Node * sp_nodepath_node_new(Inkscape::NodePath::SubPath *sp,Inkscape::NodePath::Node *next,Inkscape::NodePath::NodeType type, NRPathcode code,
                                         NR::Point *ppos, NR::Point *pos, NR::Point *npos);
static void sp_nodepath_node_destroy(Inkscape::NodePath::Node *node);

/* Helpers */

static Inkscape::NodePath::NodeSide *sp_node_get_side(Inkscape::NodePath::Node *node, gint which);
static Inkscape::NodePath::NodeSide *sp_node_opposite_side(Inkscape::NodePath::Node *node,Inkscape::NodePath::NodeSide *me);
static NRPathcode sp_node_path_code_from_side(Inkscape::NodePath::Node *node,Inkscape::NodePath::NodeSide *me);

static SPCurve* sp_nodepath_object_get_curve(SPObject *object, const gchar *key);
static void sp_nodepath_set_curve (Inkscape::NodePath::Path *np, SPCurve *curve);

// active_node indicates mouseover node
Inkscape::NodePath::Node * Inkscape::NodePath::Path::active_node = NULL;

static SPCanvasItem *
sp_nodepath_make_helper_item(Inkscape::NodePath::Path *np, /*SPDesktop *desktop, */const SPCurve *curve, bool show = false) {
    SPCurve *helper_curve = curve->copy();
    helper_curve->transform(np->i2d);
    SPCanvasItem *helper_path = sp_canvas_bpath_new(sp_desktop_controls(np->desktop), helper_curve);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(helper_path), np->helperpath_rgba, np->helperpath_width, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(helper_path), 0, SP_WIND_RULE_NONZERO);
    sp_canvas_item_move_to_z(helper_path, 0);
    if (show) {
        sp_canvas_item_show(helper_path);
    }
    helper_curve->unref();
    return helper_path;
}

static SPCanvasItem *
canvasitem_from_pathvec(Inkscape::NodePath::Path *np, Geom::PathVector const &pathv, bool show) {
    SPCurve *helper_curve = new SPCurve(pathv);
    return sp_nodepath_make_helper_item(np, helper_curve, show);
}

static void
sp_nodepath_create_helperpaths(Inkscape::NodePath::Path *np) {
    //std::map<Inkscape::LivePathEffect::Effect *, std::vector<SPCanvasItem *> >* helper_path_vec;
    if (!SP_IS_LPE_ITEM(np->item)) {
        g_print ("Only LPEItems can have helperpaths!\n");
        return;
    }

    SPLPEItem *lpeitem = SP_LPE_ITEM(np->item);
    PathEffectList lpelist = sp_lpe_item_get_effect_list(lpeitem);
    for (PathEffectList::iterator i = lpelist.begin(); i != lpelist.end(); ++i) {
        Inkscape::LivePathEffect::LPEObjectReference *lperef = (*i);
        Inkscape::LivePathEffect::Effect *lpe = lperef->lpeobject->lpe;
        // create new canvas items from the effect's helper paths
        std::vector<Geom::PathVector> hpaths = lpe->getHelperPaths(lpeitem);
        for (std::vector<Geom::PathVector>::iterator j = hpaths.begin(); j != hpaths.end(); ++j) {
            (*np->helper_path_vec)[lpe].push_back(canvasitem_from_pathvec(np, *j, true));
        }
    }
}

void
sp_nodepath_update_helperpaths(Inkscape::NodePath::Path *np) {
    //std::map<Inkscape::LivePathEffect::Effect *, std::vector<SPCanvasItem *> >* helper_path_vec;
    if (!SP_IS_LPE_ITEM(np->item)) {
        g_print ("Only LPEItems can have helperpaths!\n");
        return;
    }

    SPLPEItem *lpeitem = SP_LPE_ITEM(np->item);
    PathEffectList lpelist = sp_lpe_item_get_effect_list(lpeitem);
    for (PathEffectList::iterator i = lpelist.begin(); i != lpelist.end(); ++i) {
        Inkscape::LivePathEffect::Effect *lpe = (*i)->lpeobject->lpe;
        /* update canvas items from the effect's helper paths; note that this code relies on the
         * fact that getHelperPaths() will always return the same number of helperpaths in the same
         * order as during their creation in sp_nodepath_create_helperpaths
         */
        std::vector<Geom::PathVector> hpaths = lpe->getHelperPaths(lpeitem);
        for (unsigned int j = 0; j < hpaths.size(); ++j) {
            SPCurve *curve = new SPCurve(hpaths[j]);
            curve->transform(np->i2d);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(((*np->helper_path_vec)[lpe])[j]), curve);
            curve = curve->unref();
        }
    }
}

static void
sp_nodepath_destroy_helperpaths(Inkscape::NodePath::Path *np) {
    for (HelperPathList::iterator i = np->helper_path_vec->begin(); i != np->helper_path_vec->end(); ++i) {
        for (std::vector<SPCanvasItem *>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
            GtkObject *temp = *j;
            *j = NULL;
            gtk_object_destroy(temp);
        }
    }
}


/**
 * \brief Creates new nodepath from item
 */
Inkscape::NodePath::Path *sp_nodepath_new(SPDesktop *desktop, SPObject *object, bool show_handles, const char * repr_key_in, SPItem *item)
{
    Inkscape::XML::Node *repr = object->repr;

    /** \todo
     * FIXME: remove this. We don't want to edit paths inside flowtext.
     * Instead we will build our flowtext with cloned paths, so that the
     * real paths are outside the flowtext and thus editable as usual.
     */
    if (SP_IS_FLOWTEXT(object)) {
        for (SPObject *child = sp_object_first_child(object) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if SP_IS_FLOWREGION(child) {
                SPObject *grandchild = sp_object_first_child(SP_OBJECT(child));
                if (grandchild && SP_IS_PATH(grandchild)) {
                    object = SP_ITEM(grandchild);
                    break;
                }
            }
        }
    }

    SPCurve *curve = sp_nodepath_object_get_curve(object, repr_key_in);

    if (curve == NULL)
        return NULL;

    if (curve->get_segment_count() < 1) {
        curve->unref();
        return NULL; // prevent crash for one-node paths
    }

    //Create new nodepath
    Inkscape::NodePath::Path *np = g_new(Inkscape::NodePath::Path, 1);
    if (!np) {
        curve->unref();
        return NULL;
    }

    // Set defaults
    np->desktop     = desktop;
    np->object      = object;
    np->subpaths    = NULL;
    np->selected    = NULL;
    np->shape_editor = NULL; //Let the shapeeditor that makes this set it
    np->local_change = 0;
    np->show_handles = show_handles;
    np->helper_path = NULL;
    np->helper_path_vec = new HelperPathList;
    np->helperpath_rgba = prefs_get_int_attribute("tools.nodes", "highlight_color", 0xff0000ff);
    np->helperpath_width = 1.0;
    np->curve = curve->copy();
    np->show_helperpath = (prefs_get_int_attribute ("tools.nodes", "show_helperpath",  0) == 1);
    if (SP_IS_LPE_ITEM(object)) {
        Inkscape::LivePathEffect::Effect *lpe = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(object));
        if (lpe && lpe->isVisible() && lpe->showOrigPath()) {
            np->show_helperpath = true;
        }            
    }
    np->straight_path = false;
    if (IS_LIVEPATHEFFECT(object) && item) {
        np->item = item;
    } else {
        np->item = SP_ITEM(object);
    }

    // we need to update item's transform from the repr here,
    // because they may be out of sync when we respond
    // to a change in repr by regenerating nodepath     --bb
    sp_object_read_attr(SP_OBJECT(np->item), "transform");

    np->i2d  = sp_item_i2d_affine(np->item);
    np->d2i  = np->i2d.inverse();

    np->repr = repr;
    if (repr_key_in) { // apparantly the object is an LPEObject
        np->repr_key = g_strdup(repr_key_in);
        np->repr_nodetypes_key = g_strconcat(np->repr_key, "-nodetypes", NULL);
        Inkscape::LivePathEffect::Parameter *lpeparam = LIVEPATHEFFECT(object)->lpe->getParameter(repr_key_in);
        if (lpeparam) {
            lpeparam->param_setup_nodepath(np);
        }
    } else {
        np->repr_nodetypes_key = g_strdup("sodipodi:nodetypes");
        if ( sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(np->object)) ) {
            np->repr_key = g_strdup("inkscape:original-d");

            Inkscape::LivePathEffect::Effect* lpe = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(np->object));
            if (lpe) {
                lpe->setup_nodepath(np);
            }
        } else {
            np->repr_key = g_strdup("d");
        }
    }

    /* Calculate length of the nodetype string. The closing/starting point for closed paths is counted twice.
     * So for example a closed rectangle has a nodetypestring of length 5.
     * To get the correct count, one can count all segments in the paths, and then add the total number of (non-empty) paths. */
    Geom::PathVector pathv_sanitized = pathv_to_linear_and_cubic_beziers(np->curve->get_pathvector());
    np->curve->set_pathvector(pathv_sanitized);
    guint length = np->curve->get_segment_count();
    for (Geom::PathVector::const_iterator pit = pathv_sanitized.begin(); pit != pathv_sanitized.end(); ++pit) {
        length += pit->empty() ? 0 : 1;
    }

    gchar const *nodetypes = np->repr->attribute(np->repr_nodetypes_key);
    Inkscape::NodePath::NodeType *typestr = parse_nodetypes(nodetypes, length);

    // create the subpath(s) from the bpath
    subpaths_from_pathvector(np, pathv_sanitized, typestr);

    // reverse the list, because sp_nodepath_subpath_new() used g_list_prepend instead of append (for speed)
    np->subpaths = g_list_reverse(np->subpaths);

    delete[] typestr;
    curve->unref();

    // Draw helper curve
    if (np->show_helperpath) {
        np->helper_path = sp_nodepath_make_helper_item(np, /*desktop, */np->curve, true);
    }

    sp_nodepath_create_helperpaths(np);

    return np;
}

/**
 * Destroys nodepath's subpaths, then itself, also tell parent ShapeEditor about it.
 */
void sp_nodepath_destroy(Inkscape::NodePath::Path *np) {

    if (!np)  //soft fail, like delete
        return;

    while (np->subpaths) {
        sp_nodepath_subpath_destroy((Inkscape::NodePath::SubPath *) np->subpaths->data);
    }

    //Inform the ShapeEditor that made me, if any, that I am gone.
    if (np->shape_editor)
        np->shape_editor->nodepath_destroyed();

    g_assert(!np->selected);

    if (np->helper_path) {
        GtkObject *temp = np->helper_path;
        np->helper_path = NULL;
        gtk_object_destroy(temp);
    }
    if (np->curve) {
        np->curve->unref();
        np->curve = NULL;
    }

    if (np->repr_key) {
        g_free(np->repr_key);
        np->repr_key = NULL;
    }
    if (np->repr_nodetypes_key) {
        g_free(np->repr_nodetypes_key);
        np->repr_nodetypes_key = NULL;
    }

    sp_nodepath_destroy_helperpaths(np);
    delete np->helper_path_vec;
    np->helper_path_vec = NULL;

    np->desktop = NULL;

    g_free(np);
}

/**
 *  Return the node count of a given NodeSubPath.
 */
static gint sp_nodepath_subpath_get_node_count(Inkscape::NodePath::SubPath *subpath)
{
    if (!subpath)
        return 0;
    gint nodeCount = g_list_length(subpath->nodes);
    return nodeCount;
}

/**
 *  Return the node count of a given NodePath.
 */
static gint sp_nodepath_get_node_count(Inkscape::NodePath::Path *np)
{
    if (!np)
        return 0;
    gint nodeCount = 0;
    for (GList *item = np->subpaths ; item ; item=item->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *)item->data;
        nodeCount += g_list_length(subpath->nodes);
    }
    return nodeCount;
}

/**
 *  Return the subpath count of a given NodePath.
 */
static gint sp_nodepath_get_subpath_count(Inkscape::NodePath::Path *np)
{
    if (!np)
        return 0;
    return g_list_length (np->subpaths);
}

/**
 *  Return the selected node count of a given NodePath.
 */
static gint sp_nodepath_selection_get_node_count(Inkscape::NodePath::Path *np)
{
    if (!np)
        return 0;
    return g_list_length (np->selected);
}

/**
 *  Return the number of subpaths where nodes are selected in a given NodePath.
 */
static gint sp_nodepath_selection_get_subpath_count(Inkscape::NodePath::Path *np)
{
    if (!np)
        return 0;
    if (!np->selected)
        return 0;
    if (!np->selected->next)
        return 1;
    gint count = 0;
    for (GList *spl = np->subpaths; spl != NULL; spl = spl->next) {
        Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
            Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
            if (node->selected) {
                count ++;
                break;
            }
        }
    }
    return count;
}

/**
 * Clean up a nodepath after editing.
 *
 * Currently we are deleting trivial subpaths.
 */
static void sp_nodepath_cleanup(Inkscape::NodePath::Path *nodepath)
{
    GList *badSubPaths = NULL;

    //Check all closed subpaths to be >=1 nodes, all open subpaths to be >= 2 nodes
    for (GList *l = nodepath->subpaths; l ; l=l->next) {
       Inkscape::NodePath::SubPath *sp = (Inkscape::NodePath::SubPath *)l->data;
       if ((sp_nodepath_subpath_get_node_count(sp)<2 && !sp->closed) || (sp_nodepath_subpath_get_node_count(sp)<1 && sp->closed))
            badSubPaths = g_list_append(badSubPaths, sp);
    }

    //Delete them.  This second step is because sp_nodepath_subpath_destroy()
    //also removes the subpath from nodepath->subpaths
    for (GList *l = badSubPaths; l ; l=l->next) {
       Inkscape::NodePath::SubPath *sp = (Inkscape::NodePath::SubPath *)l->data;
        sp_nodepath_subpath_destroy(sp);
    }

    g_list_free(badSubPaths);
}

/**
 * Create new nodepaths from pathvector, make it subpaths of np.
 * \param t The node type array.
 */
static void subpaths_from_pathvector(Inkscape::NodePath::Path *np, Geom::PathVector const & pathv, Inkscape::NodePath::NodeType const *t)
{
    guint i = 0;  // index into node type array
    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit) {
        if (pit->empty())
            continue;  // don't add single knot paths

        Inkscape::NodePath::SubPath *sp = sp_nodepath_subpath_new(np);

        NR::Point ppos = pit->initialPoint() * (Geom::Matrix)np->i2d;
        NRPathcode pcode = NR_MOVETO;

        /* Johan: Note that this is pretty arcane code. I am pretty sure it is working correctly, be very certain to change it! (better to just rewrite this whole method)*/
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_closed(); ++cit) {
            if( dynamic_cast<Geom::LineSegment const*>(&*cit) ||
                dynamic_cast<Geom::HLineSegment const*>(&*cit) ||
                dynamic_cast<Geom::VLineSegment const*>(&*cit) )
            {
                NR::Point pos = cit->initialPoint() * (Geom::Matrix)np->i2d;
                sp_nodepath_node_new(sp, NULL, t[i++], pcode, &ppos, &pos, &pos);

                ppos = cit->finalPoint() * (Geom::Matrix)np->i2d;
                pcode = NR_LINETO;
            }
            else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const*>(&*cit)) {
                std::vector<Geom::Point> points = cubic_bezier->points();
                NR::Point pos = points[0] * (Geom::Matrix)np->i2d;
                NR::Point npos = points[1] * (Geom::Matrix)np->i2d;
                sp_nodepath_node_new(sp, NULL, t[i++], pcode, &ppos, &pos, &npos);

                ppos = points[2] * (Geom::Matrix)np->i2d;
                pcode = NR_CURVETO;
            }
        }

        if (pit->closed()) {
            // Add last knot (because sp_nodepath_subpath_close kills the last knot)
            /* Remember that last closing segment is always a lineto, but its length can be zero if the path is visually closed already
             * If the length is zero, don't add it to the nodepath. */
            Geom::Curve const &closing_seg = pit->back_closed();
            if ( ! closing_seg.isDegenerate() ) {
                NR::Point pos = closing_seg.finalPoint() * (Geom::Matrix)np->i2d;
                sp_nodepath_node_new(sp, NULL, t[i++], NR_LINETO, &pos, &pos, &pos);
            }

            sp_nodepath_subpath_close(sp);
        }
    }
}

/**
 * Convert from sodipodi:nodetypes to new style type array.
 */
static
Inkscape::NodePath::NodeType * parse_nodetypes(gchar const *types, guint length)
{
    Inkscape::NodePath::NodeType *typestr = new Inkscape::NodePath::NodeType[length + 1];

    guint pos = 0;

    if (types) {
        for (guint i = 0; types[i] && ( i < length ); i++) {
            while ((types[i] > '\0') && (types[i] <= ' ')) i++;
            if (types[i] != '\0') {
                switch (types[i]) {
                    case 's':
                        typestr[pos++] =Inkscape::NodePath::NODE_SMOOTH;
                        break;
                    case 'z':
                        typestr[pos++] =Inkscape::NodePath::NODE_SYMM;
                        break;
                    case 'c':
                        typestr[pos++] =Inkscape::NodePath::NODE_CUSP;
                        break;
                    default:
                        typestr[pos++] =Inkscape::NodePath::NODE_NONE;
                        break;
                }
            }
        }
    }

    while (pos < length) typestr[pos++] =Inkscape::NodePath::NODE_NONE;

    return typestr;
}

/**
 * Make curve out of nodepath, write it into that nodepath's SPShape item so that display is
 * updated but repr is not (for speed). Used during curve and node drag.
 */
static void update_object(Inkscape::NodePath::Path *np)
{
    g_assert(np);

    np->curve->unref();
    np->curve = create_curve(np);

    sp_nodepath_set_curve(np, np->curve);

    if (np->show_helperpath) {
        SPCurve * helper_curve = np->curve->copy();
        helper_curve->transform(np->i2d);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(np->helper_path), helper_curve);
        helper_curve->unref();
    }

    // updating helperpaths of LPEItems is now done in sp_lpe_item_update();
    //sp_nodepath_update_helperpaths(np);

    // now that nodepath and knotholder can be enabled simultaneously, we must update the knotholder, too
    // TODO: this should be done from ShapeEditor!! nodepath should be oblivious of knotholder!
    np->shape_editor->update_knotholder();
}

/**
 * Update XML path node with data from path object.
 */
static void update_repr_internal(Inkscape::NodePath::Path *np)
{
    g_assert(np);

    Inkscape::XML::Node *repr = np->object->repr;

    np->curve->unref();
    np->curve = create_curve(np);

    gchar *typestr = create_typestr(np);
    gchar *svgpath = sp_svg_write_path(np->curve->get_pathvector());

    // determine if path has an effect applied and write to correct "d" attribute.
    if (repr->attribute(np->repr_key) == NULL || strcmp(svgpath, repr->attribute(np->repr_key))) { // d changed
        np->local_change++;
        repr->setAttribute(np->repr_key, svgpath);
    }

    if (repr->attribute(np->repr_nodetypes_key) == NULL || strcmp(typestr, repr->attribute(np->repr_nodetypes_key))) { // nodetypes changed
        np->local_change++;
        repr->setAttribute(np->repr_nodetypes_key, typestr);
    }

    g_free(svgpath);
    g_free(typestr);

    if (np->show_helperpath) {
        SPCurve * helper_curve = np->curve->copy();
        helper_curve->transform(np->i2d);
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(np->helper_path), helper_curve);
        helper_curve->unref();
    }

    // TODO: do we need this call here? after all, update_object() should have been called just before
    //sp_nodepath_update_helperpaths(np);
}

/**
 * Update XML path node with data from path object, commit changes forever.
 */
void sp_nodepath_update_repr(Inkscape::NodePath::Path *np, const gchar *annotation)
{
    //fixme: np can be NULL, so check before proceeding
    g_return_if_fail(np != NULL);

    update_repr_internal(np);
    sp_canvas_end_forced_full_redraws(np->desktop->canvas);

    sp_document_done(sp_desktop_document(np->desktop), SP_VERB_CONTEXT_NODE,
                     annotation);
}

/**
 * Update XML path node with data from path object, commit changes with undo.
 */
static void sp_nodepath_update_repr_keyed(Inkscape::NodePath::Path *np, gchar const *key, const gchar *annotation)
{
    update_repr_internal(np);
    sp_document_maybe_done(sp_desktop_document(np->desktop), key, SP_VERB_CONTEXT_NODE,
                           annotation);
}

/**
 * Make duplicate of path, replace corresponding XML node in tree, commit.
 */
static void stamp_repr(Inkscape::NodePath::Path *np)
{
    g_assert(np);

    Inkscape::XML::Node *old_repr = np->object->repr;
    Inkscape::XML::Node *new_repr = old_repr->duplicate(old_repr->document());

    // remember the position of the item
    gint pos = old_repr->position();
    // remember parent
    Inkscape::XML::Node *parent = sp_repr_parent(old_repr);

    SPCurve *curve = create_curve(np);
    gchar *typestr = create_typestr(np);

    gchar *svgpath = sp_svg_write_path(curve->get_pathvector());

    new_repr->setAttribute(np->repr_key, svgpath);
    new_repr->setAttribute(np->repr_nodetypes_key, typestr);

    // add the new repr to the parent
    parent->appendChild(new_repr);
    // move to the saved position
    new_repr->setPosition(pos > 0 ? pos : 0);

    sp_document_done(sp_desktop_document(np->desktop), SP_VERB_CONTEXT_NODE,
                     _("Stamp"));

    Inkscape::GC::release(new_repr);
    g_free(svgpath);
    g_free(typestr);
    curve->unref();
}

/**
 * Create curve from path.
 */
static SPCurve *create_curve(Inkscape::NodePath::Path *np)
{
    SPCurve *curve = new SPCurve();

    for (GList *spl = np->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *sp = (Inkscape::NodePath::SubPath *) spl->data;
        curve->moveto(sp->first->pos * np->d2i);
       Inkscape::NodePath::Node *n = sp->first->n.other;
        while (n) {
            NR::Point const end_pt = n->pos * np->d2i;
            switch (n->code) {
                case NR_LINETO:
                    curve->lineto(end_pt);
                    break;
                case NR_CURVETO:
                    curve->curveto(n->p.other->n.pos * np->d2i,
                                     n->p.pos * np->d2i,
                                     end_pt);
                    break;
                default:
                    g_assert_not_reached();
                    break;
            }
            if (n != sp->last) {
                n = n->n.other;
            } else {
                n = NULL;
            }
        }
        if (sp->closed) {
            curve->closepath();
        }
    }

    return curve;
}

/**
 * Convert path type string to sodipodi:nodetypes style.
 */
static gchar *create_typestr(Inkscape::NodePath::Path *np)
{
    gchar *typestr = g_new(gchar, 32);
    gint len = 32;
    gint pos = 0;

    for (GList *spl = np->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *sp = (Inkscape::NodePath::SubPath *) spl->data;

        if (pos >= len) {
            typestr = g_renew(gchar, typestr, len + 32);
            len += 32;
        }

        typestr[pos++] = 'c';

       Inkscape::NodePath::Node *n;
        n = sp->first->n.other;
        while (n) {
            gchar code;

            switch (n->type) {
                case Inkscape::NodePath::NODE_CUSP:
                    code = 'c';
                    break;
                case Inkscape::NodePath::NODE_SMOOTH:
                    code = 's';
                    break;
                case Inkscape::NodePath::NODE_SYMM:
                    code = 'z';
                    break;
                default:
                    g_assert_not_reached();
                    code = '\0';
                    break;
            }

            if (pos >= len) {
                typestr = g_renew(gchar, typestr, len + 32);
                len += 32;
            }

            typestr[pos++] = code;

            if (n != sp->last) {
                n = n->n.other;
            } else {
                n = NULL;
            }
        }
    }

    if (pos >= len) {
        typestr = g_renew(gchar, typestr, len + 1);
        len += 1;
    }

    typestr[pos++] = '\0';

    return typestr;
}

// Returns different message contexts depending on the current context. This function should only
// be called when ec is either a SPNodeContext or SPLPEToolContext, thus we return NULL in all
// other cases.
static Inkscape::MessageContext *
get_message_context(SPEventContext *ec)
{
    Inkscape::MessageContext *mc;
    if (SP_IS_NODE_CONTEXT(ec)) {
        mc = SP_NODE_CONTEXT(ec)->_node_message_context;
    } else if (SP_IS_LPETOOL_CONTEXT(ec)) {
        mc = SP_LPETOOL_CONTEXT(ec)->_lpetool_message_context;
    } else {
        g_warning ("Nodepath should only be present in Node tool or Geometric tool.");
        return NULL;
    }
}

/**
 \brief Fills node and handle positions for three nodes, splitting line
  marked by end at distance t.
 */
static void sp_nodepath_line_midpoint(Inkscape::NodePath::Node *new_path,Inkscape::NodePath::Node *end, gdouble t)
{
    g_assert(new_path != NULL);
    g_assert(end      != NULL);

    g_assert(end->p.other == new_path);
   Inkscape::NodePath::Node *start = new_path->p.other;
    g_assert(start);

    if (end->code == NR_LINETO) {
        new_path->type =Inkscape::NodePath::NODE_CUSP;
        new_path->code = NR_LINETO;
        new_path->pos = new_path->n.pos = new_path->p.pos = (t * start->pos + (1 - t) * end->pos);
    } else {
        new_path->type =Inkscape::NodePath::NODE_SMOOTH;
        new_path->code = NR_CURVETO;
        gdouble s      = 1 - t;
        for (int dim = 0; dim < 2; dim++) {
            NR::Coord const f000 = start->pos[dim];
            NR::Coord const f001 = start->n.pos[dim];
            NR::Coord const f011 = end->p.pos[dim];
            NR::Coord const f111 = end->pos[dim];
            NR::Coord const f00t = s * f000 + t * f001;
            NR::Coord const f01t = s * f001 + t * f011;
            NR::Coord const f11t = s * f011 + t * f111;
            NR::Coord const f0tt = s * f00t + t * f01t;
            NR::Coord const f1tt = s * f01t + t * f11t;
            NR::Coord const fttt = s * f0tt + t * f1tt;
            start->n.pos[dim]    = f00t;
            new_path->p.pos[dim] = f0tt;
            new_path->pos[dim]   = fttt;
            new_path->n.pos[dim] = f1tt;
            end->p.pos[dim]      = f11t;
        }
    }
}

/**
 * Adds new node on direct line between two nodes, activates handles of all
 * three nodes.
 */
static Inkscape::NodePath::Node *sp_nodepath_line_add_node(Inkscape::NodePath::Node *end, gdouble t)
{
    g_assert(end);
    g_assert(end->subpath);
    g_assert(g_list_find(end->subpath->nodes, end));

   Inkscape::NodePath::Node *start = end->p.other;
    g_assert( start->n.other == end );
   Inkscape::NodePath::Node *newnode = sp_nodepath_node_new(end->subpath,
                                               end,
                                               (NRPathcode)end->code == NR_LINETO?
                                                  Inkscape::NodePath::NODE_CUSP : Inkscape::NodePath::NODE_SMOOTH,
                                               (NRPathcode)end->code,
                                               &start->pos, &start->pos, &start->n.pos);
    sp_nodepath_line_midpoint(newnode, end, t);

    sp_node_adjust_handles(start);
    sp_node_update_handles(start);
    sp_node_update_handles(newnode);
    sp_node_adjust_handles(end);
    sp_node_update_handles(end);

    return newnode;
}

/**
\brief Break the path at the node: duplicate the argument node, start a new subpath with the duplicate, and copy all nodes after the argument node to it
*/
static Inkscape::NodePath::Node *sp_nodepath_node_break(Inkscape::NodePath::Node *node)
{
    g_assert(node);
    g_assert(node->subpath);
    g_assert(g_list_find(node->subpath->nodes, node));

   Inkscape::NodePath::SubPath *sp = node->subpath;
    Inkscape::NodePath::Path *np    = sp->nodepath;

    if (sp->closed) {
        sp_nodepath_subpath_open(sp, node);
        return sp->first;
    } else {
        // no break for end nodes
        if (node == sp->first) return NULL;
        if (node == sp->last ) return NULL;

        // create a new subpath
       Inkscape::NodePath::SubPath *newsubpath = sp_nodepath_subpath_new(np);

        // duplicate the break node as start of the new subpath
        Inkscape::NodePath::Node *newnode = sp_nodepath_node_new(newsubpath, NULL, (Inkscape::NodePath::NodeType)node->type, NR_MOVETO, &node->pos, &node->pos, &node->n.pos);

        // attach rest of curve to new node
        g_assert(node->n.other);
        newnode->n.other = node->n.other; node->n.other = NULL;
        newnode->n.other->p.other = newnode;
        newsubpath->last = sp->last;
        sp->last = node;
        node = newnode;
        while (node->n.other) {
            node = node->n.other;
            node->subpath = newsubpath;
            sp->nodes = g_list_remove(sp->nodes, node);
            newsubpath->nodes = g_list_prepend(newsubpath->nodes, node);
        }


        return newnode;
    }
}

/**
 * Duplicate node and connect to neighbours.
 */
static Inkscape::NodePath::Node *sp_nodepath_node_duplicate(Inkscape::NodePath::Node *node)
{
    g_assert(node);
    g_assert(node->subpath);
    g_assert(g_list_find(node->subpath->nodes, node));

   Inkscape::NodePath::SubPath *sp = node->subpath;

    NRPathcode code = (NRPathcode) node->code;
    if (code == NR_MOVETO) { // if node is the endnode,
        node->code = NR_LINETO; // new one is inserted before it, so change that to line
    }

    Inkscape::NodePath::Node *newnode = sp_nodepath_node_new(sp, node, (Inkscape::NodePath::NodeType)node->type, code, &node->p.pos, &node->pos, &node->n.pos);

    if (!node->n.other || !node->p.other) // if node is an endnode, select it
        return node;
    else
        return newnode; // otherwise select the newly created node
}

static void sp_node_handle_mirror_n_to_p(Inkscape::NodePath::Node *node)
{
    node->p.pos = (node->pos + (node->pos - node->n.pos));
}

static void sp_node_handle_mirror_p_to_n(Inkscape::NodePath::Node *node)
{
    node->n.pos = (node->pos + (node->pos - node->p.pos));
}

/**
 * Change line type at node, with side effects on neighbours.
 */
static void sp_nodepath_set_line_type(Inkscape::NodePath::Node *end, NRPathcode code)
{
    g_assert(end);
    g_assert(end->subpath);
    g_assert(end->p.other);

    if (end->code == static_cast< guint > ( code ) )
        return;

   Inkscape::NodePath::Node *start = end->p.other;

    end->code = code;

    if (code == NR_LINETO) {
        if (start->code == NR_LINETO) {
            sp_nodepath_set_node_type (start, Inkscape::NodePath::NODE_CUSP);
        }
        if (end->n.other) {
            if (end->n.other->code == NR_LINETO) {
                sp_nodepath_set_node_type (end, Inkscape::NodePath::NODE_CUSP);
            }
        }
    } else {
        NR::Point delta = end->pos - start->pos;
        start->n.pos = start->pos + delta / 3;
        end->p.pos = end->pos - delta / 3;
        sp_node_adjust_handle(start, 1);
        sp_node_adjust_handle(end, -1);
    }

    sp_node_update_handles(start);
    sp_node_update_handles(end);
}

/**
 * Change node type, and its handles accordingly.
 */
static Inkscape::NodePath::Node *sp_nodepath_set_node_type(Inkscape::NodePath::Node *node, Inkscape::NodePath::NodeType type)
{
    g_assert(node);
    g_assert(node->subpath);

    if ((node->p.other != NULL) && (node->n.other != NULL)) {
        if ((node->code == NR_LINETO) && (node->n.other->code == NR_LINETO)) {
            type =Inkscape::NodePath::NODE_CUSP;
        }
    }

    node->type = type;

    if (node->type == Inkscape::NodePath::NODE_CUSP) {
        node->knot->setShape (SP_KNOT_SHAPE_DIAMOND);
        node->knot->setSize (node->selected? 11 : 9);
        sp_knot_update_ctrl(node->knot);
    } else {
        node->knot->setShape (SP_KNOT_SHAPE_SQUARE);
        node->knot->setSize (node->selected? 9 : 7);
        sp_knot_update_ctrl(node->knot);
    }

    // if one of handles is mouseovered, preserve its position
    if (node->p.knot && SP_KNOT_IS_MOUSEOVER(node->p.knot)) {
        sp_node_adjust_handle(node, 1);
    } else if (node->n.knot && SP_KNOT_IS_MOUSEOVER(node->n.knot)) {
        sp_node_adjust_handle(node, -1);
    } else {
        sp_node_adjust_handles(node);
    }

    sp_node_update_handles(node);

    sp_nodepath_update_statusbar(node->subpath->nodepath);

    return node;
}

bool
sp_node_side_is_line (Inkscape::NodePath::Node *node, Inkscape::NodePath::NodeSide *side)
{
        Inkscape::NodePath::Node *othernode = side->other;
        if (!othernode)
            return false;
        NRPathcode const code = sp_node_path_code_from_side(node, side);
        if (code == NR_LINETO)
            return true;
        Inkscape::NodePath::NodeSide *other_to_me = NULL;
        if (&node->p == side) {
            other_to_me = &othernode->n;
        } else if (&node->n == side) {
            other_to_me = &othernode->p;
        } 
        if (!other_to_me)
            return false;
        bool is_line = 
             (NR::L2(othernode->pos - other_to_me->pos) < 1e-6 &&
              NR::L2(node->pos - side->pos) < 1e-6);
        return is_line;
}

/**
 * Same as sp_nodepath_set_node_type(), but also converts, if necessary, adjacent segments from
 * lines to curves.  If adjacent to one line segment, pulls out or rotates opposite handle to align
 * with that segment, procucing half-smooth node. If already half-smooth, pull out the second handle too. 
 * If already cusp and set to cusp, retracts handles.
*/
void sp_nodepath_convert_node_type(Inkscape::NodePath::Node *node, Inkscape::NodePath::NodeType type)
{
    if (type == Inkscape::NodePath::NODE_SYMM || type == Inkscape::NodePath::NODE_SMOOTH) {

/* 
  Here's the algorithm of converting node to smooth (Shift+S or toolbar button), in pseudocode:
 
        if (two_handles) {
            // do nothing, adjust_handles called via set_node_type will line them up
        } else if (one_handle) {
            if (opposite_to_handle_is_line) {
                if (lined_up) {
                    // already half-smooth; pull opposite handle too making it fully smooth
                } else {
                    // do nothing, adjust_handles will line the handle  up, producing a half-smooth node
                }
            } else {
                // pull opposite handle in line with the existing one
            }
        } else if (no_handles) {
            if (both_segments_are_lines OR both_segments_are_curves) {
                //pull both handles
            } else {
                // pull the handle opposite to line segment, making node half-smooth
            }
        }
*/
        bool p_has_handle = (NR::L2(node->pos  - node->p.pos) > 1e-6);
        bool n_has_handle = (NR::L2(node->pos  - node->n.pos) > 1e-6);
        bool p_is_line = sp_node_side_is_line(node, &node->p);
        bool n_is_line = sp_node_side_is_line(node, &node->n);

        if (p_has_handle && n_has_handle) {
            // do nothing, adjust_handles will line them up
        } else if (p_has_handle || n_has_handle) {
            if (p_has_handle && n_is_line) {
                Radial line (node->n.other->pos - node->pos);
                Radial handle (node->pos - node->p.pos);
                if (fabs(line.a - handle.a) < 1e-3) { // lined up
                    // already half-smooth; pull opposite handle too making it fully smooth
                    node->n.pos = node->pos + (node->n.other->pos - node->pos) / 3;
                } else {
                    // do nothing, adjust_handles will line the handle  up, producing a half-smooth node
                }
            } else if (n_has_handle && p_is_line) {
                Radial line (node->p.other->pos - node->pos);
                Radial handle (node->pos - node->n.pos);
                if (fabs(line.a - handle.a) < 1e-3) { // lined up
                    // already half-smooth; pull opposite handle too making it fully smooth
                    node->p.pos = node->pos + (node->p.other->pos - node->pos) / 3;
                } else {
                    // do nothing, adjust_handles will line the handle  up, producing a half-smooth node
                }
            } else if (p_has_handle && node->n.other) {
                // pull n handle
                node->n.other->code = NR_CURVETO;
                double len =  (type == Inkscape::NodePath::NODE_SYMM)?
                    NR::L2(node->p.pos - node->pos) :
                    NR::L2(node->n.other->pos - node->pos) / 3;
                node->n.pos = node->pos - (len / NR::L2(node->p.pos - node->pos)) * (node->p.pos - node->pos);
            } else if (n_has_handle && node->p.other) {
                // pull p handle
                node->code = NR_CURVETO;
                double len =  (type == Inkscape::NodePath::NODE_SYMM)?
                    NR::L2(node->n.pos - node->pos) :
                    NR::L2(node->p.other->pos - node->pos) / 3;
                node->p.pos = node->pos - (len / NR::L2(node->n.pos - node->pos)) * (node->n.pos - node->pos);
            }
        } else if (!p_has_handle && !n_has_handle) {
            if ((p_is_line && n_is_line) || (!p_is_line && node->p.other && !n_is_line && node->n.other)) {
                // no handles, but both segments are either lnes or curves:
                //pull both handles

                // convert both to curves:
                node->code = NR_CURVETO;
                node->n.other->code = NR_CURVETO;

                NR::Point leg_prev = node->pos - node->p.other->pos;
                NR::Point leg_next = node->pos - node->n.other->pos;

                double norm_leg_prev = L2(leg_prev);
                double norm_leg_next = L2(leg_next);

                NR::Point delta;
                if (norm_leg_next > 0.0) {
                    delta = (norm_leg_prev / norm_leg_next) * leg_next - leg_prev;
                    (&delta)->normalize();
                }

                if (type == Inkscape::NodePath::NODE_SYMM) {
                    double norm_leg_avg = (norm_leg_prev + norm_leg_next) / 2;
                    node->p.pos = node->pos + 0.3 * norm_leg_avg * delta;
                    node->n.pos = node->pos - 0.3 * norm_leg_avg * delta;
                } else {
                    // length of handle is proportional to distance to adjacent node
                    node->p.pos = node->pos + 0.3 * norm_leg_prev * delta;
                    node->n.pos = node->pos - 0.3 * norm_leg_next * delta;
                }

            } else {
                // pull the handle opposite to line segment, making it half-smooth
                if (p_is_line && node->n.other) {
                    if (type != Inkscape::NodePath::NODE_SYMM) {
                        // pull n handle
                        node->n.other->code = NR_CURVETO;
                        double len =  NR::L2(node->n.other->pos - node->pos) / 3;
                        node->n.pos = node->pos + (len / NR::L2(node->p.other->pos - node->pos)) * (node->p.other->pos - node->pos);
                    }
                } else if (n_is_line && node->p.other) {
                    if (type != Inkscape::NodePath::NODE_SYMM) {
                        // pull p handle
                        node->code = NR_CURVETO;
                        double len =  NR::L2(node->p.other->pos - node->pos) / 3;
                        node->p.pos = node->pos + (len / NR::L2(node->n.other->pos - node->pos)) * (node->n.other->pos - node->pos);
                    }
                }
            }
        }
    } else if (type == Inkscape::NodePath::NODE_CUSP && node->type == Inkscape::NodePath::NODE_CUSP) {
        // cusping a cusp: retract nodes
        node->p.pos = node->pos;
        node->n.pos = node->pos;
    }

    sp_nodepath_set_node_type (node, type);
}

/**
 * Move node to point, and adjust its and neighbouring handles.
 */
void sp_node_moveto(Inkscape::NodePath::Node *node, NR::Point p)
{
    NR::Point delta = p - node->pos;
    node->pos = p;

    node->p.pos += delta;
    node->n.pos += delta;

    Inkscape::NodePath::Node *node_p = NULL;
    Inkscape::NodePath::Node *node_n = NULL;

    if (node->p.other) {
        if (node->code == NR_LINETO) {
            sp_node_adjust_handle(node, 1);
            sp_node_adjust_handle(node->p.other, -1);
            node_p = node->p.other;
        }
    }
    if (node->n.other) {
        if (node->n.other->code == NR_LINETO) {
            sp_node_adjust_handle(node, -1);
            sp_node_adjust_handle(node->n.other, 1);
            node_n = node->n.other;
        }
    }

    // this function is only called from batch movers that will update display at the end
    // themselves, so here we just move all the knots without emitting move signals, for speed
    sp_node_update_handles(node, false);
    if (node_n) {
        sp_node_update_handles(node_n, false);
    }
    if (node_p) {
        sp_node_update_handles(node_p, false);
    }
}

/**
 * Call sp_node_moveto() for node selection and handle possible snapping.
 */
static void sp_nodepath_selected_nodes_move(Inkscape::NodePath::Path *nodepath, NR::Coord dx, NR::Coord dy,
                                            bool const snap, bool constrained = false, 
                                            Inkscape::Snapper::ConstraintLine const &constraint = Geom::Point())
{
    NR::Coord best = NR_HUGE;
    NR::Point delta(dx, dy);
    NR::Point best_pt = delta;
    Inkscape::SnappedPoint best_abs;
    
    if (snap) {    
        /* When dragging a (selected) node, it should only snap to other nodes (i.e. unselected nodes), and
         * not to itself. The snapper however can not tell which nodes are selected and which are not, so we 
         * must provide that information. */
          
        // Build a list of the unselected nodes to which the snapper should snap 
        std::vector<Geom::Point> unselected_nodes;
        for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
            Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
            for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
                Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
                if (!node->selected) {
                    unselected_nodes.push_back(to_2geom(node->pos));
                }    
            }
        }        
        
        SnapManager &m = nodepath->desktop->namedview->snap_manager;
        
        for (GList *l = nodepath->selected; l != NULL; l = l->next) {
            Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
            m.setup(NULL, SP_PATH(n->subpath->nodepath->item), &unselected_nodes);
            Inkscape::SnappedPoint s;
            if (constrained) {
                Inkscape::Snapper::ConstraintLine dedicated_constraint = constraint;
                dedicated_constraint.setPoint(n->pos);
                s = m.constrainedSnap(Inkscape::Snapper::SNAPPOINT_NODE, to_2geom(n->pos + delta), dedicated_constraint);
            } else {
                s = m.freeSnap(Inkscape::Snapper::SNAPPOINT_NODE, to_2geom(n->pos + delta));
            }            
            if (s.getSnapped() && (s.getDistance() < best)) {
                best = s.getDistance();
                best_abs = s;
                best_pt = from_2geom(s.getPoint()) - n->pos;
            }
        }
                        
        if (best_abs.getSnapped()) {
            nodepath->desktop->snapindicator->set_new_snappoint(best_abs);
        } else {
            nodepath->desktop->snapindicator->remove_snappoint();    
        }
    }

    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
        Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
        sp_node_moveto(n, n->pos + best_pt);
    }

    // do not update repr here so that node dragging is acceptably fast
    update_object(nodepath);
}

/**
Function mapping x (in the range 0..1) to y (in the range 1..0) using a smooth half-bell-like
curve; the parameter alpha determines how blunt (alpha > 1) or sharp (alpha < 1) will be the curve
near x = 0.
 */
double
sculpt_profile (double x, double alpha, guint profile)
{
    if (x >= 1)
        return 0;
    if (x <= 0)
        return 1;

    switch (profile) {
        case SCULPT_PROFILE_LINEAR:
        return 1 - x;
        case SCULPT_PROFILE_BELL:
        return (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5);
        case SCULPT_PROFILE_ELLIPTIC:
        return sqrt(1 - x*x);
    }

    return 1;
}

double
bezier_length (NR::Point a, NR::Point ah, NR::Point bh, NR::Point b)
{
    // extremely primitive for now, don't have time to look for the real one
    double lower = NR::L2(b - a);
    double upper = NR::L2(ah - a) + NR::L2(bh - ah) + NR::L2(bh - b);
    return (lower + upper)/2;
}

void
sp_nodepath_move_node_and_handles (Inkscape::NodePath::Node *n, NR::Point delta, NR::Point delta_n, NR::Point delta_p)
{
    n->pos = n->origin + delta;
    n->n.pos = n->n.origin + delta_n;
    n->p.pos = n->p.origin + delta_p;
    sp_node_adjust_handles(n);
    sp_node_update_handles(n, false);
}

/**
 * Displace selected nodes and their handles by fractions of delta (from their origins), depending
 * on how far they are from the dragged node n.
 */
static void
sp_nodepath_selected_nodes_sculpt(Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::Node *n, NR::Point delta)
{
    g_assert (n);
    g_assert (nodepath);
    g_assert (n->subpath->nodepath == nodepath);

    double pressure = n->knot->pressure;
    if (pressure == 0)
        pressure = 0.5; // default
    pressure = CLAMP (pressure, 0.2, 0.8);

    // map pressure to alpha = 1/5 ... 5
    double alpha = 1 - 2 * fabs(pressure - 0.5);
    if (pressure > 0.5)
        alpha = 1/alpha;

    guint profile = prefs_get_int_attribute("tools.nodes", "sculpting_profile", SCULPT_PROFILE_BELL);

    if (sp_nodepath_selection_get_subpath_count(nodepath) <= 1) {
        // Only one subpath has selected nodes:
        // use linear mode, where the distance from n to node being dragged is calculated along the path

        double n_sel_range = 0, p_sel_range = 0;
        guint n_nodes = 0, p_nodes = 0;
        guint n_sel_nodes = 0, p_sel_nodes = 0;

        // First pass: calculate ranges (TODO: we could cache them, as they don't change while dragging)
        {
            double n_range = 0, p_range = 0;
            bool n_going = true, p_going = true;
            Inkscape::NodePath::Node *n_node = n;
            Inkscape::NodePath::Node *p_node = n;
            do {
                // Do one step in both directions from n, until reaching the end of subpath or bumping into each other
                if (n_node && n_going)
                    n_node = n_node->n.other;
                if (n_node == NULL) {
                    n_going = false;
                } else {
                    n_nodes ++;
                    n_range += bezier_length (n_node->p.other->origin, n_node->p.other->n.origin, n_node->p.origin, n_node->origin);
                    if (n_node->selected) {
                        n_sel_nodes ++;
                        n_sel_range = n_range;
                    }
                    if (n_node == p_node) {
                        n_going = false;
                        p_going = false;
                    }
                }
                if (p_node && p_going)
                    p_node = p_node->p.other;
                if (p_node == NULL) {
                    p_going = false;
                } else {
                    p_nodes ++;
                    p_range += bezier_length (p_node->n.other->origin, p_node->n.other->p.origin, p_node->n.origin, p_node->origin);
                    if (p_node->selected) {
                        p_sel_nodes ++;
                        p_sel_range = p_range;
                    }
                    if (p_node == n_node) {
                        n_going = false;
                        p_going = false;
                    }
                }
            } while (n_going || p_going);
        }

        // Second pass: actually move nodes in this subpath
        sp_nodepath_move_node_and_handles (n, delta, delta, delta);
        {
            double n_range = 0, p_range = 0;
            bool n_going = true, p_going = true;
            Inkscape::NodePath::Node *n_node = n;
            Inkscape::NodePath::Node *p_node = n;
            do {
                // Do one step in both directions from n, until reaching the end of subpath or bumping into each other
                if (n_node && n_going)
                    n_node = n_node->n.other;
                if (n_node == NULL) {
                    n_going = false;
                } else {
                    n_range += bezier_length (n_node->p.other->origin, n_node->p.other->n.origin, n_node->p.origin, n_node->origin);
                    if (n_node->selected) {
                        sp_nodepath_move_node_and_handles (n_node,
                                                           sculpt_profile (n_range / n_sel_range, alpha, profile) * delta,
                                                           sculpt_profile ((n_range + NR::L2(n_node->n.origin - n_node->origin)) / n_sel_range, alpha, profile) * delta,
                                                           sculpt_profile ((n_range - NR::L2(n_node->p.origin - n_node->origin)) / n_sel_range, alpha, profile) * delta);
                    }
                    if (n_node == p_node) {
                        n_going = false;
                        p_going = false;
                    }
                }
                if (p_node && p_going)
                    p_node = p_node->p.other;
                if (p_node == NULL) {
                    p_going = false;
                } else {
                    p_range += bezier_length (p_node->n.other->origin, p_node->n.other->p.origin, p_node->n.origin, p_node->origin);
                    if (p_node->selected) {
                        sp_nodepath_move_node_and_handles (p_node,
                                                           sculpt_profile (p_range / p_sel_range, alpha, profile) * delta,
                                                           sculpt_profile ((p_range - NR::L2(p_node->n.origin - p_node->origin)) / p_sel_range, alpha, profile) * delta,
                                                           sculpt_profile ((p_range + NR::L2(p_node->p.origin - p_node->origin)) / p_sel_range, alpha, profile) * delta);
                    }
                    if (p_node == n_node) {
                        n_going = false;
                        p_going = false;
                    }
                }
            } while (n_going || p_going);
        }

    } else {
        // Multiple subpaths have selected nodes:
        // use spatial mode, where the distance from n to node being dragged is measured directly as NR::L2.
        // TODO: correct these distances taking into account their angle relative to the bisector, so as to
        // fix the pear-like shape when sculpting e.g. a ring

        // First pass: calculate range
        gdouble direct_range = 0;
        for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
            Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
            for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
                Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
                if (node->selected) {
                    direct_range = MAX(direct_range, NR::L2(node->origin - n->origin));
                }
            }
        }

        // Second pass: actually move nodes
        for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
            Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
            for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
                Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
                if (node->selected) {
                    if (direct_range > 1e-6) {
                        sp_nodepath_move_node_and_handles (node,
                                                       sculpt_profile (NR::L2(node->origin - n->origin) / direct_range, alpha, profile) * delta,
                                                       sculpt_profile (NR::L2(node->n.origin - n->origin) / direct_range, alpha, profile) * delta,
                                                       sculpt_profile (NR::L2(node->p.origin - n->origin) / direct_range, alpha, profile) * delta);
                    } else {
                        sp_nodepath_move_node_and_handles (node, delta, delta, delta);
                    }

                }
            }
        }
    }

    // do not update repr here so that node dragging is acceptably fast
    update_object(nodepath);
}


/**
 * Move node selection to point, adjust its and neighbouring handles,
 * handle possible snapping, and commit the change with possible undo.
 */
void
sp_node_selected_move(Inkscape::NodePath::Path *nodepath, gdouble dx, gdouble dy)
{
    if (!nodepath) return;

    sp_nodepath_selected_nodes_move(nodepath, dx, dy, false);

    if (dx == 0) {
        sp_nodepath_update_repr_keyed(nodepath, "node:move:vertical", _("Move nodes vertically"));
    } else if (dy == 0) {
        sp_nodepath_update_repr_keyed(nodepath, "node:move:horizontal", _("Move nodes horizontally"));
    } else {
        sp_nodepath_update_repr(nodepath, _("Move nodes"));
    }
}

/**
 * Move node selection off screen and commit the change.
 */
void
sp_node_selected_move_screen(SPDesktop *desktop, Inkscape::NodePath::Path *nodepath, gdouble dx, gdouble dy)
{
    // borrowed from sp_selection_move_screen in selection-chemistry.c
    // we find out the current zoom factor and divide deltas by it

    gdouble zoom = desktop->current_zoom();
    gdouble zdx = dx / zoom;
    gdouble zdy = dy / zoom;

    if (!nodepath) return;

    sp_nodepath_selected_nodes_move(nodepath, zdx, zdy, false);

    if (dx == 0) {
        sp_nodepath_update_repr_keyed(nodepath, "node:move:vertical", _("Move nodes vertically"));
    } else if (dy == 0) {
        sp_nodepath_update_repr_keyed(nodepath, "node:move:horizontal", _("Move nodes horizontally"));
    } else {
        sp_nodepath_update_repr(nodepath, _("Move nodes"));
    }
}

/**
 * Move selected nodes to the absolute position given
 */
void sp_node_selected_move_absolute(Inkscape::NodePath::Path *nodepath, Geom::Coord val, Geom::Dim2 axis)
{
    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
        Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
        Geom::Point npos(axis == Geom::X ? val : n->pos[Geom::X], axis == Geom::Y ? val : n->pos[Geom::Y]);
        sp_node_moveto(n, npos);
    }

    sp_nodepath_update_repr(nodepath, _("Move nodes"));
}

/**
 * If the coordinates of all selected nodes coincide, return the common coordinate; otherwise return NR::Nothing
 */
boost::optional<Geom::Coord> sp_node_selected_common_coord (Inkscape::NodePath::Path *nodepath, Geom::Dim2 axis)
{
    boost::optional<Geom::Coord> no_coord;
    g_return_val_if_fail(nodepath->selected, no_coord);

    // determine coordinate of first selected node
    GList *nsel = nodepath->selected;
    Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) nsel->data;
    NR::Coord coord = n->pos[axis];
    bool coincide = true;

    // compare it to the coordinates of all the other selected nodes
    for (GList *l = nsel->next; l != NULL; l = l->next) {
        n = (Inkscape::NodePath::Node *) l->data;
        if (n->pos[axis] != coord) {
            coincide = false;
        }
    }
    if (coincide) {
        return coord;
    } else {
        Geom::Rect bbox = sp_node_selected_bbox(nodepath);
        // currently we return the coordinate of the bounding box midpoint because I don't know how
        // to erase the spin button entry field :), but maybe this can be useful behaviour anyway
        return bbox.midpoint()[axis];
    }
}

/** If they don't yet exist, creates knot and line for the given side of the node */
static void sp_node_ensure_knot_exists (SPDesktop *desktop, Inkscape::NodePath::Node *node, Inkscape::NodePath::NodeSide *side)
{
    if (!side->knot) {
        side->knot = sp_knot_new(desktop, _("<b>Node handle</b>: drag to shape the curve; with <b>Ctrl</b> to snap angle; with <b>Alt</b> to lock length; with <b>Shift</b> to rotate both handles"));

        side->knot->setShape (SP_KNOT_SHAPE_CIRCLE);
        side->knot->setSize (7);
        side->knot->setAnchor (GTK_ANCHOR_CENTER);
        side->knot->setFill(KNOT_FILL, KNOT_FILL_HI, KNOT_FILL_HI);
        side->knot->setStroke(KNOT_STROKE, KNOT_STROKE_HI, KNOT_STROKE_HI);
        sp_knot_update_ctrl(side->knot);

        g_signal_connect(G_OBJECT(side->knot), "clicked", G_CALLBACK(node_handle_clicked), node);
        g_signal_connect(G_OBJECT(side->knot), "grabbed", G_CALLBACK(node_handle_grabbed), node);
        g_signal_connect(G_OBJECT(side->knot), "ungrabbed", G_CALLBACK(node_handle_ungrabbed), node);
        g_signal_connect(G_OBJECT(side->knot), "request", G_CALLBACK(node_handle_request), node);
        g_signal_connect(G_OBJECT(side->knot), "moved", G_CALLBACK(node_handle_moved), node);
        g_signal_connect(G_OBJECT(side->knot), "event", G_CALLBACK(node_handle_event), node);
    }

    if (!side->line) {
        side->line = sp_canvas_item_new(sp_desktop_controls(desktop),
                                        SP_TYPE_CTRLLINE, NULL);
    }
}

/**
 * Ensure the given handle of the node is visible/invisible, update its screen position
 */
static void sp_node_update_handle(Inkscape::NodePath::Node *node, gint which, gboolean show_handle, bool fire_move_signals)
{
    g_assert(node != NULL);

   Inkscape::NodePath::NodeSide *side = sp_node_get_side(node, which);
    NRPathcode code = sp_node_path_code_from_side(node, side);

    show_handle = show_handle && (code == NR_CURVETO) && (NR::L2(side->pos - node->pos) > 1e-6);

    if (show_handle) {
        if (!side->knot) { // No handle knot at all
            sp_node_ensure_knot_exists(node->subpath->nodepath->desktop, node, side);
            // Just created, so we shouldn't fire the node_moved callback - instead set the knot position directly
            side->knot->pos = side->pos;
            if (side->knot->item)
                SP_CTRL(side->knot->item)->moveto(side->pos);
            sp_ctrlline_set_coords(SP_CTRLLINE(side->line), node->pos, side->pos);
            sp_knot_show(side->knot);
        } else {
            if (side->knot->pos != to_2geom(side->pos)) { // only if it's really moved
                if (fire_move_signals) {
                    sp_knot_set_position(side->knot, side->pos, 0); // this will set coords of the line as well
                } else {
                    sp_knot_moveto(side->knot, side->pos);
                    sp_ctrlline_set_coords(SP_CTRLLINE(side->line), node->pos, side->pos);
                }
            }
            if (!SP_KNOT_IS_VISIBLE(side->knot)) {
                sp_knot_show(side->knot);
            }
        }
        sp_canvas_item_show(side->line);
    } else {
        if (side->knot) {
            if (SP_KNOT_IS_VISIBLE(side->knot)) {
                sp_knot_hide(side->knot);
            }
        }
        if (side->line) {
            sp_canvas_item_hide(side->line);
        }
    }
}

/**
 * Ensure the node itself is visible, its handles and those of the neighbours of the node are
 * visible if selected, update their screen positions. If fire_move_signals, move the node and its
 * handles so that the corresponding signals are fired, callbacks are activated, and curve is
 * updated; otherwise, just move the knots silently (used in batch moves).
 */
static void sp_node_update_handles(Inkscape::NodePath::Node *node, bool fire_move_signals)
{
    g_assert(node != NULL);

    if (!SP_KNOT_IS_VISIBLE(node->knot)) {
        sp_knot_show(node->knot);
    }

    if (node->knot->pos != to_2geom(node->pos)) { // visible knot is in a different position, need to update
        if (fire_move_signals)
            sp_knot_set_position(node->knot, node->pos, 0);
        else
            sp_knot_moveto(node->knot, node->pos);
    }

    gboolean show_handles = node->selected;
    if (node->p.other != NULL) {
        if (node->p.other->selected) show_handles = TRUE;
    }
    if (node->n.other != NULL) {
        if (node->n.other->selected) show_handles = TRUE;
    }

    if (node->subpath->nodepath->show_handles == false)
        show_handles = FALSE;

    sp_node_update_handle(node, -1, show_handles, fire_move_signals);
    sp_node_update_handle(node, 1, show_handles, fire_move_signals);
}

/**
 * Call sp_node_update_handles() for all nodes on subpath.
 */
static void sp_nodepath_subpath_update_handles(Inkscape::NodePath::SubPath *subpath)
{
    g_assert(subpath != NULL);

    for (GList *l = subpath->nodes; l != NULL; l = l->next) {
        sp_node_update_handles((Inkscape::NodePath::Node *) l->data);
    }
}

/**
 * Call sp_nodepath_subpath_update_handles() for all subpaths of nodepath.
 */
static void sp_nodepath_update_handles(Inkscape::NodePath::Path *nodepath)
{
    g_assert(nodepath != NULL);

    for (GList *l = nodepath->subpaths; l != NULL; l = l->next) {
        sp_nodepath_subpath_update_handles((Inkscape::NodePath::SubPath *) l->data);
    }
}

void
sp_nodepath_show_handles(Inkscape::NodePath::Path *nodepath, bool show)
{
    if (nodepath == NULL) return;

    nodepath->show_handles = show;
    sp_nodepath_update_handles(nodepath);
}

/**
 * Adds all selected nodes in nodepath to list.
 */
void Inkscape::NodePath::Path::selection(std::list<Node *> &l)
{
    StlConv<Node *>::list(l, selected);
/// \todo this adds a copying, rework when the selection becomes a stl list
}

/**
 * Align selected nodes on the specified axis.
 */
void sp_nodepath_selected_align(Inkscape::NodePath::Path *nodepath, NR::Dim2 axis)
{
    if ( !nodepath || !nodepath->selected ) { // no nodepath, or no nodes selected
        return;
    }

    if ( !nodepath->selected->next ) { // only one node selected
        return;
    }
   Inkscape::NodePath::Node *pNode = reinterpret_cast<Inkscape::NodePath::Node *>(nodepath->selected->data);
    NR::Point dest(pNode->pos);
    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
        pNode = reinterpret_cast<Inkscape::NodePath::Node *>(l->data);
        if (pNode) {
            dest[axis] = pNode->pos[axis];
            sp_node_moveto(pNode, dest);
        }
    }

    sp_nodepath_update_repr(nodepath, _("Align nodes"));
}

/// Helper struct.
struct NodeSort
{
   Inkscape::NodePath::Node *_node;
    NR::Coord _coord;
    /// \todo use vectorof pointers instead of calling copy ctor
    NodeSort(Inkscape::NodePath::Node *node, NR::Dim2 axis) :
        _node(node), _coord(node->pos[axis])
    {}

};

static bool operator<(NodeSort const &a, NodeSort const &b)
{
    return (a._coord < b._coord);
}

/**
 * Distribute selected nodes on the specified axis.
 */
void sp_nodepath_selected_distribute(Inkscape::NodePath::Path *nodepath, NR::Dim2 axis)
{
    if ( !nodepath || !nodepath->selected ) { // no nodepath, or no nodes selected
        return;
    }

    if ( ! (nodepath->selected->next && nodepath->selected->next->next) ) { // less than 3 nodes selected
        return;
    }

   Inkscape::NodePath::Node *pNode = reinterpret_cast<Inkscape::NodePath::Node *>(nodepath->selected->data);
    std::vector<NodeSort> sorted;
    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
        pNode = reinterpret_cast<Inkscape::NodePath::Node *>(l->data);
        if (pNode) {
            NodeSort n(pNode, axis);
            sorted.push_back(n);
            //dest[axis] = pNode->pos[axis];
            //sp_node_moveto(pNode, dest);
        }
    }
    std::sort(sorted.begin(), sorted.end());
    unsigned int len = sorted.size();
    //overall bboxes span
    float dist = (sorted.back()._coord -
                  sorted.front()._coord);
    //new distance between each bbox
    float step = (dist) / (len - 1);
    float pos = sorted.front()._coord;
    for ( std::vector<NodeSort> ::iterator it(sorted.begin());
          it < sorted.end();
          it ++ )
    {
        NR::Point dest((*it)._node->pos);
        dest[axis] = pos;
        sp_node_moveto((*it)._node, dest);
        pos += step;
    }

    sp_nodepath_update_repr(nodepath, _("Distribute nodes"));
}


/**
 * Call sp_nodepath_line_add_node() for all selected segments.
 */
void
sp_node_selected_add_node(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) {
        return;
    }

    GList *nl = NULL;

    int n_added = 0;

    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
       Inkscape::NodePath::Node *t = (Inkscape::NodePath::Node *) l->data;
        g_assert(t->selected);
        if (t->p.other && t->p.other->selected) {
            nl = g_list_prepend(nl, t);
        }
    }

    while (nl) {
       Inkscape::NodePath::Node *t = (Inkscape::NodePath::Node *) nl->data;
       Inkscape::NodePath::Node *n = sp_nodepath_line_add_node(t, 0.5);
       sp_nodepath_node_select(n, TRUE, FALSE);
       n_added ++;
       nl = g_list_remove(nl, t);
    }

    /** \todo fixme: adjust ? */
    sp_nodepath_update_handles(nodepath);

    if (n_added > 1) {
        sp_nodepath_update_repr(nodepath, _("Add nodes"));
    } else if (n_added > 0) {
        sp_nodepath_update_repr(nodepath, _("Add node"));
    }

    sp_nodepath_update_statusbar(nodepath);
}

/**
 * Select segment nearest to point
 */
void
sp_nodepath_select_segment_near_point(Inkscape::NodePath::Path *nodepath, NR::Point p, bool toggle)
{
    if (!nodepath) {
        return;
    }

    SPCurve *curve = create_curve(nodepath);   // perhaps we can use nodepath->curve here instead?
    Geom::PathVector const &pathv = curve->get_pathvector();
    Geom::PathVectorPosition pvpos = Geom::nearestPoint(pathv, p);

    // calculate index for nodepath's representation.
    unsigned int segment_index = floor(pvpos.t) + 1;
    for (unsigned int i = 0; i < pvpos.path_nr; ++i) {
        segment_index += pathv[i].size() + 1;
        if (pathv[i].closed()) {
            segment_index += 1;
        }
    }

    curve->unref();

    //find segment to segment
    Inkscape::NodePath::Node *e = sp_nodepath_get_node_by_index(nodepath, segment_index);

    //fixme: this can return NULL, so check before proceeding.
    g_return_if_fail(e != NULL);

    gboolean force = FALSE;
    if (!(e->selected && (!e->p.other || e->p.other->selected))) {
        force = TRUE;
    }
    sp_nodepath_node_select(e, (gboolean) toggle, force);
    if (e->p.other)
        sp_nodepath_node_select(e->p.other, TRUE, force);

    sp_nodepath_update_handles(nodepath);

    sp_nodepath_update_statusbar(nodepath);
}

/**
 * Add a node nearest to point
 */
void
sp_nodepath_add_node_near_point(Inkscape::NodePath::Path *nodepath, NR::Point p)
{
    if (!nodepath) {
        return;
    }

    SPCurve *curve = create_curve(nodepath);   // perhaps we can use nodepath->curve here instead?
    Geom::PathVector const &pathv = curve->get_pathvector();
    Geom::PathVectorPosition pvpos = Geom::nearestPoint(pathv, p);

    // calculate index for nodepath's representation.
    double int_part;
    double t = std::modf(pvpos.t, &int_part);
    unsigned int segment_index = (unsigned int)int_part + 1;
    for (unsigned int i = 0; i < pvpos.path_nr; ++i) {
        segment_index += pathv[i].size() + 1;
        if (pathv[i].closed()) {
            segment_index += 1;
        }
    }

    curve->unref();

    //find segment to split
    Inkscape::NodePath::Node *e = sp_nodepath_get_node_by_index(nodepath, segment_index);

    //don't know why but t seems to flip for lines
    if (sp_node_path_code_from_side(e, sp_node_get_side(e, -1)) == NR_LINETO) {
        t = 1.0 - t;
    }

    Inkscape::NodePath::Node *n = sp_nodepath_line_add_node(e, t);
    sp_nodepath_node_select(n, FALSE, TRUE);

    /* fixme: adjust ? */
    sp_nodepath_update_handles(nodepath);

    sp_nodepath_update_repr(nodepath, _("Add node"));

    sp_nodepath_update_statusbar(nodepath);
}

/*
 * Adjusts a segment so that t moves by a certain delta for dragging
 * converts lines to curves
 *
 * method and idea borrowed from Simon Budig  <simon@gimp.org> and the GIMP
 * cf. app/vectors/gimpbezierstroke.c, gimp_bezier_stroke_point_move_relative()
 */
void
sp_nodepath_curve_drag(Inkscape::NodePath::Path *nodepath, int node, double t, NR::Point delta)
{
    Inkscape::NodePath::Node *e = sp_nodepath_get_node_by_index(nodepath, node);

    //fixme: e and e->p can be NULL, so check for those before proceeding
    g_return_if_fail(e != NULL);
    g_return_if_fail(&e->p != NULL);

    /* feel good is an arbitrary parameter that distributes the delta between handles
     * if t of the drag point is less than 1/6 distance form the endpoint only
     * the corresponding hadle is adjusted. This matches the behavior in GIMP
     */
    double feel_good;
    if (t <= 1.0 / 6.0)
        feel_good = 0;
    else if (t <= 0.5)
        feel_good = (pow((6 * t - 1) / 2.0, 3)) / 2;
    else if (t <= 5.0 / 6.0)
        feel_good = (1 - pow((6 * (1-t) - 1) / 2.0, 3)) / 2 + 0.5;
    else
        feel_good = 1;

    //if we're dragging a line convert it to a curve
    if (sp_node_path_code_from_side(e, sp_node_get_side(e, -1))==NR_LINETO) {
        sp_nodepath_set_line_type(e, NR_CURVETO);
    }

    NR::Point offsetcoord0 = ((1-feel_good)/(3*t*(1-t)*(1-t))) * delta;
    NR::Point offsetcoord1 = (feel_good/(3*t*t*(1-t))) * delta;
    e->p.other->n.pos += offsetcoord0;
    e->p.pos += offsetcoord1;

    // adjust handles of adjacent nodes where necessary
    sp_node_adjust_handle(e,1);
    sp_node_adjust_handle(e->p.other,-1);

    sp_nodepath_update_handles(e->subpath->nodepath);

    update_object(e->subpath->nodepath);

    sp_nodepath_update_statusbar(e->subpath->nodepath);
}


/**
 * Call sp_nodepath_break() for all selected segments.
 */
void sp_node_selected_break(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) return;

    GList *tempin = g_list_copy(nodepath->selected);
    GList *temp = NULL;
    for (GList *l = tempin; l != NULL; l = l->next) {
       Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
       Inkscape::NodePath::Node *nn = sp_nodepath_node_break(n);
        if (nn == NULL) continue; // no break, no new node
        temp = g_list_prepend(temp, nn);
    }
    g_list_free(tempin);

    if (temp) {
        sp_nodepath_deselect(nodepath);
    }
    for (GList *l = temp; l != NULL; l = l->next) {
        sp_nodepath_node_select((Inkscape::NodePath::Node *) l->data, TRUE, TRUE);
    }

    sp_nodepath_update_handles(nodepath);

    sp_nodepath_update_repr(nodepath, _("Break path"));
}

/**
 * Duplicate the selected node(s).
 */
void sp_node_selected_duplicate(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) {
        return;
    }

    GList *temp = NULL;
    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
       Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
       Inkscape::NodePath::Node *nn = sp_nodepath_node_duplicate(n);
        if (nn == NULL) continue; // could not duplicate
        temp = g_list_prepend(temp, nn);
    }

    if (temp) {
        sp_nodepath_deselect(nodepath);
    }
    for (GList *l = temp; l != NULL; l = l->next) {
        sp_nodepath_node_select((Inkscape::NodePath::Node *) l->data, TRUE, TRUE);
    }

    sp_nodepath_update_handles(nodepath);

    sp_nodepath_update_repr(nodepath, _("Duplicate node"));
}

/**
 *  Internal function to join two nodes by merging them into one.
 */
static void do_node_selected_join(Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::Node *a, Inkscape::NodePath::Node *b)
{
    /* a and b are endpoints */

    // if one of the two nodes is mouseovered, fix its position
    NR::Point c;
    if (a->knot && SP_KNOT_IS_MOUSEOVER(a->knot)) {
        c = a->pos;
    } else if (b->knot && SP_KNOT_IS_MOUSEOVER(b->knot)) {
        c = b->pos;
    } else {
        // otherwise, move joined node to the midpoint
        c = (a->pos + b->pos) / 2;
    }

    if (a->subpath == b->subpath) {
       Inkscape::NodePath::SubPath *sp = a->subpath;
        sp_nodepath_subpath_close(sp);
        sp_node_moveto (sp->first, c);

        sp_nodepath_update_handles(sp->nodepath);
        sp_nodepath_update_repr(nodepath, _("Close subpath"));
        return;
    }

    /* a and b are separate subpaths */
    Inkscape::NodePath::SubPath *sa = a->subpath;
    Inkscape::NodePath::SubPath *sb = b->subpath;
    NR::Point p;
    Inkscape::NodePath::Node *n;
    NRPathcode code;
    if (a == sa->first) {
        // we will now reverse sa, so that a is its last node, not first, and drop that node
        p = sa->first->n.pos;
        code = (NRPathcode)sa->first->n.other->code;
        // create new subpath
       Inkscape::NodePath::SubPath *t = sp_nodepath_subpath_new(sa->nodepath);
       // create a first moveto node on it
        n = sa->last;
        sp_nodepath_node_new(t, NULL,Inkscape::NodePath::NODE_CUSP, NR_MOVETO, &n->n.pos, &n->pos, &n->p.pos);
        n = n->p.other;
        if (n == sa->first) n = NULL;
        while (n) {
            // copy the rest of the nodes from sa to t, going backwards
            sp_nodepath_node_new(t, NULL, (Inkscape::NodePath::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
            n = n->p.other;
            if (n == sa->first) n = NULL;
        }
        // replace sa with t
        sp_nodepath_subpath_destroy(sa);
        sa = t;
    } else if (a == sa->last) {
        // a is already last, just drop it
        p = sa->last->p.pos;
        code = (NRPathcode)sa->last->code;
        sp_nodepath_node_destroy(sa->last);
    } else {
        code = NR_END;
        g_assert_not_reached();
    }

    if (b == sb->first) {
        // copy all nodes from b to a, forward 
        sp_nodepath_node_new(sa, NULL,Inkscape::NodePath::NODE_CUSP, code, &p, &c, &sb->first->n.pos);
        for (n = sb->first->n.other; n != NULL; n = n->n.other) {
            sp_nodepath_node_new(sa, NULL, (Inkscape::NodePath::NodeType)n->type, (NRPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
        }
    } else if (b == sb->last) {
        // copy all nodes from b to a, backward 
        sp_nodepath_node_new(sa, NULL,Inkscape::NodePath::NODE_CUSP, code, &p, &c, &sb->last->p.pos);
        for (n = sb->last->p.other; n != NULL; n = n->p.other) {
            sp_nodepath_node_new(sa, NULL, (Inkscape::NodePath::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
        }
    } else {
        g_assert_not_reached();
    }
    /* and now destroy sb */

    sp_nodepath_subpath_destroy(sb);

    sp_nodepath_update_handles(sa->nodepath);

    sp_nodepath_update_repr(nodepath, _("Join nodes"));

    sp_nodepath_update_statusbar(nodepath);
}

/**
 *  Internal function to join two nodes by adding a segment between them.
 */
static void do_node_selected_join_segment(Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::Node *a, Inkscape::NodePath::Node *b)
{
    if (a->subpath == b->subpath) {
       Inkscape::NodePath::SubPath *sp = a->subpath;

        /*similar to sp_nodepath_subpath_close(sp), without the node destruction*/
        sp->closed = TRUE;

        sp->first->p.other = sp->last;
        sp->last->n.other  = sp->first;

        sp_node_handle_mirror_p_to_n(sp->last);
        sp_node_handle_mirror_n_to_p(sp->first);

        sp->first->code = sp->last->code;
        sp->first       = sp->last;

        sp_nodepath_update_handles(sp->nodepath);

        sp_nodepath_update_repr(nodepath, _("Close subpath by segment"));

        return;
    }

    /* a and b are separate subpaths */
    Inkscape::NodePath::SubPath *sa = a->subpath;
    Inkscape::NodePath::SubPath *sb = b->subpath;

    Inkscape::NodePath::Node *n;
    NR::Point p;
    NRPathcode code;
    if (a == sa->first) {
        code = (NRPathcode) sa->first->n.other->code;
       Inkscape::NodePath::SubPath *t = sp_nodepath_subpath_new(sa->nodepath);
        n = sa->last;
        sp_nodepath_node_new(t, NULL,Inkscape::NodePath::NODE_CUSP, NR_MOVETO, &n->n.pos, &n->pos, &n->p.pos);
        for (n = n->p.other; n != NULL; n = n->p.other) {
            sp_nodepath_node_new(t, NULL, (Inkscape::NodePath::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
        }
        sp_nodepath_subpath_destroy(sa);
        sa = t;
    } else if (a == sa->last) {
        code = (NRPathcode)sa->last->code;
    } else {
        code = NR_END;
        g_assert_not_reached();
    }

    if (b == sb->first) {
        n = sb->first;
        sp_node_handle_mirror_p_to_n(sa->last);
        sp_nodepath_node_new(sa, NULL,Inkscape::NodePath::NODE_CUSP, code, &n->p.pos, &n->pos, &n->n.pos);
        sp_node_handle_mirror_n_to_p(sa->last);
        for (n = n->n.other; n != NULL; n = n->n.other) {
            sp_nodepath_node_new(sa, NULL, (Inkscape::NodePath::NodeType)n->type, (NRPathcode)n->code, &n->p.pos, &n->pos, &n->n.pos);
        }
    } else if (b == sb->last) {
        n = sb->last;
        sp_node_handle_mirror_p_to_n(sa->last);
        sp_nodepath_node_new(sa, NULL,Inkscape::NodePath::NODE_CUSP, code, &p, &n->pos, &n->p.pos);
        sp_node_handle_mirror_n_to_p(sa->last);
        for (n = n->p.other; n != NULL; n = n->p.other) {
            sp_nodepath_node_new(sa, NULL, (Inkscape::NodePath::NodeType)n->type, (NRPathcode)n->n.other->code, &n->n.pos, &n->pos, &n->p.pos);
        }
    } else {
        g_assert_not_reached();
    }
    /* and now destroy sb */

    sp_nodepath_subpath_destroy(sb);

    sp_nodepath_update_handles(sa->nodepath);

    sp_nodepath_update_repr(nodepath, _("Join nodes by segment"));
}

enum NodeJoinType { NODE_JOIN_ENDPOINTS, NODE_JOIN_SEGMENT };

/**
 * Internal function to handle joining two nodes.
 */
static void node_do_selected_join(Inkscape::NodePath::Path *nodepath, NodeJoinType mode)
{
    if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

    if (g_list_length(nodepath->selected) != 2) {
        nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("To join, you must have <b>two endnodes</b> selected."));
        return;
    }

    Inkscape::NodePath::Node *a = (Inkscape::NodePath::Node *) nodepath->selected->data;
    Inkscape::NodePath::Node *b = (Inkscape::NodePath::Node *) nodepath->selected->next->data;

    g_assert(a != b);
    if (!(a->p.other || a->n.other) || !(b->p.other || b->n.other)) {
        // someone tried to join an orphan node (i.e. a single-node subpath).
        // this is not worth an error message, just fail silently.
        return;
    }

    if (((a->subpath->closed) || (b->subpath->closed)) || (a->p.other && a->n.other) || (b->p.other && b->n.other)) {
        nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("To join, you must have <b>two endnodes</b> selected."));
        return;
    }

    switch(mode) {
        case NODE_JOIN_ENDPOINTS:
            do_node_selected_join(nodepath, a, b);
            break;
        case NODE_JOIN_SEGMENT:
            do_node_selected_join_segment(nodepath, a, b);
            break;
    }
}

/**
 *  Join two nodes by merging them into one.
 */
void sp_node_selected_join(Inkscape::NodePath::Path *nodepath)
{
    node_do_selected_join(nodepath, NODE_JOIN_ENDPOINTS);
}

/**
 *  Join two nodes by adding a segment between them.
 */
void sp_node_selected_join_segment(Inkscape::NodePath::Path *nodepath)
{
    node_do_selected_join(nodepath, NODE_JOIN_SEGMENT);
}

/**
 * Delete one or more selected nodes and preserve the shape of the path as much as possible.
 */
void sp_node_delete_preserve(GList *nodes_to_delete)
{
    GSList *nodepaths = NULL;

    while (nodes_to_delete) {
        Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node*) g_list_first(nodes_to_delete)->data;
        Inkscape::NodePath::SubPath *sp = node->subpath;
        Inkscape::NodePath::Path *nodepath = sp->nodepath;
        Inkscape::NodePath::Node *sample_cursor = NULL;
        Inkscape::NodePath::Node *sample_end = NULL;
        Inkscape::NodePath::Node *delete_cursor = node;
        bool just_delete = false;

        //find the start of this contiguous selection
        //move left to the first node that is not selected
        //or the start of the non-closed path
        for (Inkscape::NodePath::Node *curr=node->p.other; curr && curr!=node && g_list_find(nodes_to_delete, curr); curr=curr->p.other) {
            delete_cursor = curr;
        }

        //just delete at the beginning of an open path
        if (!delete_cursor->p.other) {
            sample_cursor = delete_cursor;
            just_delete = true;
        } else {
            sample_cursor = delete_cursor->p.other;
        }

        //calculate points for each segment
        int rate = 5;
        float period = 1.0 / rate;
        std::vector<NR::Point> data;
        if (!just_delete) {
            data.push_back(sample_cursor->pos);
            for (Inkscape::NodePath::Node *curr=sample_cursor; curr; curr=curr->n.other) {
                //just delete at the end of an open path
                if (!sp->closed && curr == sp->last) {
                    just_delete = true;
                    break;
                }

                //sample points on the contiguous selected segment
                NR::Point *bez;
                bez = new NR::Point [4];
                bez[0] = curr->pos;
                bez[1] = curr->n.pos;
                bez[2] = curr->n.other->p.pos;
                bez[3] = curr->n.other->pos;
                for (int i=1; i<rate; i++) {
                    gdouble t = i * period;
                    NR::Point p = bezier_pt(3, bez, t);
                    data.push_back(p);
                }
                data.push_back(curr->n.other->pos);

                sample_end = curr->n.other;
                //break if we've come full circle or hit the end of the selection
                if (!g_list_find(nodes_to_delete, curr->n.other) || curr->n.other==sample_cursor) {
                    break;
                }
            }
        }

        if (!just_delete) {
            //calculate the best fitting single segment and adjust the endpoints
            NR::Point *adata;
            adata = new NR::Point [data.size()];
            copy(data.begin(), data.end(), adata);

            NR::Point *bez;
            bez = new NR::Point [4];
            //would decreasing error create a better fitting approximation?
            gdouble error = 1.0;
            gint ret;
            ret = sp_bezier_fit_cubic (bez, adata, data.size(), error);

            //if these nodes are smooth or symmetrical, the endpoints will be thrown out of sync.
            //make sure these nodes are changed to cusp nodes so that, once the endpoints are moved,
            //the resulting nodes behave as expected.
            if (sample_cursor->type != Inkscape::NodePath::NODE_CUSP)
                sp_nodepath_convert_node_type(sample_cursor, Inkscape::NodePath::NODE_CUSP);
            if (sample_end->type != Inkscape::NodePath::NODE_CUSP)
                sp_nodepath_convert_node_type(sample_end, Inkscape::NodePath::NODE_CUSP);

            //adjust endpoints
            sample_cursor->n.pos = bez[1];
            sample_end->p.pos = bez[2];
        }

        //destroy this contiguous selection
        while (delete_cursor && g_list_find(nodes_to_delete, delete_cursor)) {
            Inkscape::NodePath::Node *temp = delete_cursor;
            if (delete_cursor->n.other == delete_cursor) {
                // delete_cursor->n points to itself, which means this is the last node on a closed subpath
                delete_cursor = NULL;
            } else {
                delete_cursor = delete_cursor->n.other;
            }
            nodes_to_delete = g_list_remove(nodes_to_delete, temp);
            sp_nodepath_node_destroy(temp);
        }

        sp_nodepath_update_handles(nodepath);

        if (!g_slist_find(nodepaths, nodepath))
            nodepaths = g_slist_prepend (nodepaths, nodepath);
    }

    for (GSList *i = nodepaths; i; i = i->next) {
        // FIXME: when/if we teach node tool to have more than one nodepath, deleting nodes from
        // different nodepaths will give us one undo event per nodepath
        Inkscape::NodePath::Path *nodepath = (Inkscape::NodePath::Path *) i->data;

        // if the entire nodepath is removed, delete the selected object.
        if (nodepath->subpaths == NULL ||
            //FIXME: a closed path CAN legally have one node, it's only an open one which must be
            //at least 2
            sp_nodepath_get_node_count(nodepath) < 2) {
            SPDocument *document = sp_desktop_document (nodepath->desktop);
            //FIXME: The following line will be wrong when we have mltiple nodepaths: we only want to
            //delete this nodepath's object, not the entire selection! (though at this time, this
            //does not matter)
            sp_selection_delete(nodepath->desktop);
            sp_document_done (document, SP_VERB_CONTEXT_NODE,
                              _("Delete nodes"));
        } else {
            sp_nodepath_update_repr(nodepath, _("Delete nodes preserving shape"));
            sp_nodepath_update_statusbar(nodepath);
        }
    }

    g_slist_free (nodepaths);
}

/**
 * Delete one or more selected nodes.
 */
void sp_node_selected_delete(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) return;
    if (!nodepath->selected) return;

    /** \todo fixme: do it the right way */
    while (nodepath->selected) {
       Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nodepath->selected->data;
        sp_nodepath_node_destroy(node);
    }


    //clean up the nodepath (such as for trivial subpaths)
    sp_nodepath_cleanup(nodepath);

    sp_nodepath_update_handles(nodepath);

    // if the entire nodepath is removed, delete the selected object.
    if (nodepath->subpaths == NULL ||
        sp_nodepath_get_node_count(nodepath) < 2) {
        SPDocument *document = sp_desktop_document (nodepath->desktop);
        sp_selection_delete(nodepath->desktop);
        sp_document_done (document, SP_VERB_CONTEXT_NODE,
                          _("Delete nodes"));
        return;
    }

    sp_nodepath_update_repr(nodepath, _("Delete nodes"));

    sp_nodepath_update_statusbar(nodepath);
}

/**
 * Delete one or more segments between two selected nodes.
 * This is the code for 'split'.
 */
void
sp_node_selected_delete_segment(Inkscape::NodePath::Path *nodepath)
{
   Inkscape::NodePath::Node *start, *end;     //Start , end nodes.  not inclusive
   Inkscape::NodePath::Node *curr, *next;     //Iterators

    if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

    if (g_list_length(nodepath->selected) != 2) {
        nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE,
                                                 _("Select <b>two non-endpoint nodes</b> on a path between which to delete segments."));
        return;
    }

    //Selected nodes, not inclusive
   Inkscape::NodePath::Node *a = (Inkscape::NodePath::Node *) nodepath->selected->data;
   Inkscape::NodePath::Node *b = (Inkscape::NodePath::Node *) nodepath->selected->next->data;

    if ( ( a==b)                       ||  //same node
         (a->subpath  != b->subpath )  ||  //not the same path
         (!a->p.other || !a->n.other)  ||  //one of a's sides does not have a segment
         (!b->p.other || !b->n.other) )    //one of b's sides does not have a segment
    {
        nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE,
                                                 _("Select <b>two non-endpoint nodes</b> on a path between which to delete segments."));
        return;
    }

    //###########################################
    //# BEGIN EDITS
    //###########################################
    //##################################
    //# CLOSED PATH
    //##################################
    if (a->subpath->closed) {


        gboolean reversed = FALSE;

        //Since we can go in a circle, we need to find the shorter distance.
        //  a->b or b->a
        start = end = NULL;
        int distance    = 0;
        int minDistance = 0;
        for (curr = a->n.other ; curr && curr!=a ; curr=curr->n.other) {
            if (curr==b) {
                //printf("a to b:%d\n", distance);
                start = a;//go from a to b
                end   = b;
                minDistance = distance;
                //printf("A to B :\n");
                break;
            }
            distance++;
        }

        //try again, the other direction
        distance = 0;
        for (curr = b->n.other ; curr && curr!=b ; curr=curr->n.other) {
            if (curr==a) {
                //printf("b to a:%d\n", distance);
                if (distance < minDistance) {
                    start    = b;  //we go from b to a
                    end      = a;
                    reversed = TRUE;
                    //printf("B to A\n");
                }
                break;
            }
            distance++;
        }


        //Copy everything from 'end' to 'start' to a new subpath
       Inkscape::NodePath::SubPath *t = sp_nodepath_subpath_new(nodepath);
        for (curr=end ; curr ; curr=curr->n.other) {
            NRPathcode code = (NRPathcode) curr->code;
            if (curr == end)
                code = NR_MOVETO;
            sp_nodepath_node_new(t, NULL,
                                 (Inkscape::NodePath::NodeType)curr->type, code,
                                 &curr->p.pos, &curr->pos, &curr->n.pos);
            if (curr == start)
                break;
        }
        sp_nodepath_subpath_destroy(a->subpath);


    }



    //##################################
    //# OPEN PATH
    //##################################
    else {

        //We need to get the direction of the list between A and B
        //Can we walk from a to b?
        start = end = NULL;
        for (curr = a->n.other ; curr && curr!=a ; curr=curr->n.other) {
            if (curr==b) {
                start = a;  //did it!  we go from a to b
                end   = b;
                //printf("A to B\n");
                break;
            }
        }
        if (!start) {//didn't work?  let's try the other direction
            for (curr = b->n.other ; curr && curr!=b ; curr=curr->n.other) {
                if (curr==a) {
                    start = b;  //did it!  we go from b to a
                    end   = a;
                    //printf("B to A\n");
                    break;
                }
            }
        }
        if (!start) {
            nodepath->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE,
                                                     _("Cannot find path between nodes."));
            return;
        }



        //Copy everything after 'end' to a new subpath
       Inkscape::NodePath::SubPath *t = sp_nodepath_subpath_new(nodepath);
        for (curr=end ; curr ; curr=curr->n.other) {
            NRPathcode code = (NRPathcode) curr->code;
            if (curr == end)
                code = NR_MOVETO;
            sp_nodepath_node_new(t, NULL, (Inkscape::NodePath::NodeType)curr->type, code,
                                 &curr->p.pos, &curr->pos, &curr->n.pos);
        }

        //Now let us do our deletion.  Since the tail has been saved, go all the way to the end of the list
        for (curr = start->n.other ; curr  ; curr=next) {
            next = curr->n.other;
            sp_nodepath_node_destroy(curr);
        }

    }
    //###########################################
    //# END EDITS
    //###########################################

    //clean up the nodepath (such as for trivial subpaths)
    sp_nodepath_cleanup(nodepath);

    sp_nodepath_update_handles(nodepath);

    sp_nodepath_update_repr(nodepath, _("Delete segment"));

    sp_nodepath_update_statusbar(nodepath);
}

/**
 * Call sp_nodepath_set_line() for all selected segments.
 */
void
sp_node_selected_set_line_type(Inkscape::NodePath::Path *nodepath, NRPathcode code)
{
    if (nodepath == NULL) return;

    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
       Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
        g_assert(n->selected);
        if (n->p.other && n->p.other->selected) {
            sp_nodepath_set_line_type(n, code);
        }
    }

    sp_nodepath_update_repr(nodepath, _("Change segment type"));
}

/**
 * Call sp_nodepath_convert_node_type() for all selected nodes.
 */
void
sp_node_selected_set_type(Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::NodeType type)
{
    if (nodepath == NULL) return;

    if (nodepath->straight_path) return; // don't change type when it is a straight path!

    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
        sp_nodepath_convert_node_type((Inkscape::NodePath::Node *) l->data, type);
    }

    sp_nodepath_update_repr(nodepath, _("Change node type"));
}

/**
 * Change select status of node, update its own and neighbour handles.
 */
static void sp_node_set_selected(Inkscape::NodePath::Node *node, gboolean selected)
{
    node->selected = selected;

    if (selected) {
        node->knot->setSize ((node->type == Inkscape::NodePath::NODE_CUSP) ? 11 : 9);
        node->knot->setFill(NODE_FILL_SEL, NODE_FILL_SEL_HI, NODE_FILL_SEL_HI);
        node->knot->setStroke(NODE_STROKE_SEL, NODE_STROKE_SEL_HI, NODE_STROKE_SEL_HI);
        sp_knot_update_ctrl(node->knot);
    } else {
        node->knot->setSize ((node->type == Inkscape::NodePath::NODE_CUSP) ? 9 : 7);
        node->knot->setFill(NODE_FILL, NODE_FILL_HI, NODE_FILL_HI);
        node->knot->setStroke(NODE_STROKE, NODE_STROKE_HI, NODE_STROKE_HI);
        sp_knot_update_ctrl(node->knot);
    }

    sp_node_update_handles(node);
    if (node->n.other) sp_node_update_handles(node->n.other);
    if (node->p.other) sp_node_update_handles(node->p.other);
}

/**
\brief Select a node
\param node     The node to select
\param incremental   If true, add to selection, otherwise deselect others
\param override   If true, always select this node, otherwise toggle selected status
*/
static void sp_nodepath_node_select(Inkscape::NodePath::Node *node, gboolean incremental, gboolean override)
{
    Inkscape::NodePath::Path *nodepath = node->subpath->nodepath;

    if (incremental) {
        if (override) {
            if (!g_list_find(nodepath->selected, node)) {
                nodepath->selected = g_list_prepend(nodepath->selected, node);
            }
            sp_node_set_selected(node, TRUE);
        } else { // toggle
            if (node->selected) {
                g_assert(g_list_find(nodepath->selected, node));
                nodepath->selected = g_list_remove(nodepath->selected, node);
            } else {
                g_assert(!g_list_find(nodepath->selected, node));
                nodepath->selected = g_list_prepend(nodepath->selected, node);
            }
            sp_node_set_selected(node, !node->selected);
        }
    } else {
        sp_nodepath_deselect(nodepath);
        nodepath->selected = g_list_prepend(nodepath->selected, node);
        sp_node_set_selected(node, TRUE);
    }

    sp_nodepath_update_statusbar(nodepath);
}


/**
\brief Deselect all nodes in the nodepath
*/
void
sp_nodepath_deselect(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

    while (nodepath->selected) {
        sp_node_set_selected((Inkscape::NodePath::Node *) nodepath->selected->data, FALSE);
        nodepath->selected = g_list_remove(nodepath->selected, nodepath->selected->data);
    }
    sp_nodepath_update_statusbar(nodepath);
}

/**
\brief Select or invert selection of all nodes in the nodepath
*/
void
sp_nodepath_select_all(Inkscape::NodePath::Path *nodepath, bool invert)
{
    if (!nodepath) return;

    for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
           Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
           sp_nodepath_node_select(node, TRUE, invert? !node->selected : TRUE);
        }
    }
}

/**
 * If nothing selected, does the same as sp_nodepath_select_all();
 * otherwise selects/inverts all nodes in all subpaths that have selected nodes
 * (i.e., similar to "select all in layer", with the "selected" subpaths
 * being treated as "layers" in the path).
 */
void
sp_nodepath_select_all_from_subpath(Inkscape::NodePath::Path *nodepath, bool invert)
{
    if (!nodepath) return;

    if (g_list_length (nodepath->selected) == 0) {
        sp_nodepath_select_all (nodepath, invert);
        return;
    }

    GList *copy = g_list_copy (nodepath->selected); // copy initial selection so that selecting in the loop does not affect us
    GSList *subpaths = NULL;

    for (GList *l = copy; l != NULL; l = l->next) {
        Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
        Inkscape::NodePath::SubPath *subpath = n->subpath;
        if (!g_slist_find (subpaths, subpath))
            subpaths = g_slist_prepend (subpaths, subpath);
    }

    for (GSList *sp = subpaths; sp != NULL; sp = sp->next) {
        Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) sp->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
            Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
            sp_nodepath_node_select(node, TRUE, invert? !g_list_find(copy, node) : TRUE);
        }
    }

    g_slist_free (subpaths);
    g_list_free (copy);
}

/**
 * \brief Select the node after the last selected; if none is selected,
 * select the first within path.
 */
void sp_nodepath_select_next(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

   Inkscape::NodePath::Node *last = NULL;
    if (nodepath->selected) {
        for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
           Inkscape::NodePath::SubPath *subpath, *subpath_next;
            subpath = (Inkscape::NodePath::SubPath *) spl->data;
            for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
               Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
                if (node->selected) {
                    if (node->n.other == (Inkscape::NodePath::Node *) subpath->last) {
                        if (node->n.other == (Inkscape::NodePath::Node *) subpath->first) { // closed subpath
                            if (spl->next) { // there's a next subpath
                                subpath_next = (Inkscape::NodePath::SubPath *) spl->next->data;
                                last = subpath_next->first;
                            } else if (spl->prev) { // there's a previous subpath
                                last = NULL; // to be set later to the first node of first subpath
                            } else {
                                last = node->n.other;
                            }
                        } else {
                            last = node->n.other;
                        }
                    } else {
                        if (node->n.other) {
                            last = node->n.other;
                        } else {
                            if (spl->next) { // there's a next subpath
                                subpath_next = (Inkscape::NodePath::SubPath *) spl->next->data;
                                last = subpath_next->first;
                            } else if (spl->prev) { // there's a previous subpath
                                last = NULL; // to be set later to the first node of first subpath
                            } else {
                                last = (Inkscape::NodePath::Node *) subpath->first;
                            }
                        }
                    }
                }
            }
        }
        sp_nodepath_deselect(nodepath);
    }

    if (last) { // there's at least one more node after selected
        sp_nodepath_node_select((Inkscape::NodePath::Node *) last, TRUE, TRUE);
    } else { // no more nodes, select the first one in first subpath
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) nodepath->subpaths->data;
        sp_nodepath_node_select((Inkscape::NodePath::Node *) subpath->first, TRUE, TRUE);
    }
}

/**
 * \brief Select the node before the first selected; if none is selected,
 * select the last within path
 */
void sp_nodepath_select_prev(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath) return; // there's no nodepath when editing rects, stars, spirals or ellipses

   Inkscape::NodePath::Node *last = NULL;
    if (nodepath->selected) {
        for (GList *spl = g_list_last(nodepath->subpaths); spl != NULL; spl = spl->prev) {
           Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
            for (GList *nl = g_list_last(subpath->nodes); nl != NULL; nl = nl->prev) {
               Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
                if (node->selected) {
                    if (node->p.other == (Inkscape::NodePath::Node *) subpath->first) {
                        if (node->p.other == (Inkscape::NodePath::Node *) subpath->last) { // closed subpath
                            if (spl->prev) { // there's a prev subpath
                               Inkscape::NodePath::SubPath *subpath_prev = (Inkscape::NodePath::SubPath *) spl->prev->data;
                                last = subpath_prev->last;
                            } else if (spl->next) { // there's a next subpath
                                last = NULL; // to be set later to the last node of last subpath
                            } else {
                                last = node->p.other;
                            }
                        } else {
                            last = node->p.other;
                        }
                    } else {
                        if (node->p.other) {
                            last = node->p.other;
                        } else {
                            if (spl->prev) { // there's a prev subpath
                               Inkscape::NodePath::SubPath *subpath_prev = (Inkscape::NodePath::SubPath *) spl->prev->data;
                                last = subpath_prev->last;
                            } else if (spl->next) { // there's a next subpath
                                last = NULL; // to be set later to the last node of last subpath
                            } else {
                                last = (Inkscape::NodePath::Node *) subpath->last;
                            }
                        }
                    }
                }
            }
        }
        sp_nodepath_deselect(nodepath);
    }

    if (last) { // there's at least one more node before selected
        sp_nodepath_node_select((Inkscape::NodePath::Node *) last, TRUE, TRUE);
    } else { // no more nodes, select the last one in last subpath
        GList *spl = g_list_last(nodepath->subpaths);
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        sp_nodepath_node_select((Inkscape::NodePath::Node *) subpath->last, TRUE, TRUE);
    }
}

/**
 * \brief Select all nodes that are within the rectangle.
 */
void sp_nodepath_select_rect(Inkscape::NodePath::Path *nodepath, NR::Rect const &b, gboolean incremental)
{
    if (!incremental) {
        sp_nodepath_deselect(nodepath);
    }

    for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
           Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;

            if (b.contains(node->pos)) {
                sp_nodepath_node_select(node, TRUE, TRUE);
            }
        }
    }
}


void
nodepath_grow_selection_linearly (Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::Node *n, int grow)
{
    g_assert (n);
    g_assert (nodepath);
    g_assert (n->subpath->nodepath == nodepath);

    if (g_list_length (nodepath->selected) == 0) {
        if (grow > 0) {
            sp_nodepath_node_select(n, TRUE, TRUE);
        }
        return;
    }

    if (g_list_length (nodepath->selected) == 1) {
        if (grow < 0) {
            sp_nodepath_deselect (nodepath);
            return;
        }
    }

        double n_sel_range = 0, p_sel_range = 0;
            Inkscape::NodePath::Node *farthest_n_node = n;
            Inkscape::NodePath::Node *farthest_p_node = n;

        // Calculate ranges
        {
            double n_range = 0, p_range = 0;
            bool n_going = true, p_going = true;
            Inkscape::NodePath::Node *n_node = n;
            Inkscape::NodePath::Node *p_node = n;
            do {
                // Do one step in both directions from n, until reaching the end of subpath or bumping into each other
                if (n_node && n_going)
                    n_node = n_node->n.other;
                if (n_node == NULL) {
                    n_going = false;
                } else {
                    n_range += bezier_length (n_node->p.other->pos, n_node->p.other->n.pos, n_node->p.pos, n_node->pos);
                    if (n_node->selected) {
                        n_sel_range = n_range;
                        farthest_n_node = n_node;
                    }
                    if (n_node == p_node) {
                        n_going = false;
                        p_going = false;
                    }
                }
                if (p_node && p_going)
                    p_node = p_node->p.other;
                if (p_node == NULL) {
                    p_going = false;
                } else {
                    p_range += bezier_length (p_node->n.other->pos, p_node->n.other->p.pos, p_node->n.pos, p_node->pos);
                    if (p_node->selected) {
                        p_sel_range = p_range;
                        farthest_p_node = p_node;
                    }
                    if (p_node == n_node) {
                        n_going = false;
                        p_going = false;
                    }
                }
            } while (n_going || p_going);
        }

    if (grow > 0) {
        if (n_sel_range < p_sel_range && farthest_n_node && farthest_n_node->n.other && !(farthest_n_node->n.other->selected)) {
                sp_nodepath_node_select(farthest_n_node->n.other, TRUE, TRUE);
        } else if (farthest_p_node && farthest_p_node->p.other && !(farthest_p_node->p.other->selected)) {
                sp_nodepath_node_select(farthest_p_node->p.other, TRUE, TRUE);
        }
    } else {
        if (n_sel_range > p_sel_range && farthest_n_node && farthest_n_node->selected) {
                sp_nodepath_node_select(farthest_n_node, TRUE, FALSE);
        } else if (farthest_p_node && farthest_p_node->selected) {
                sp_nodepath_node_select(farthest_p_node, TRUE, FALSE);
        }
    }
}

void
nodepath_grow_selection_spatially (Inkscape::NodePath::Path *nodepath, Inkscape::NodePath::Node *n, int grow)
{
    g_assert (n);
    g_assert (nodepath);
    g_assert (n->subpath->nodepath == nodepath);

    if (g_list_length (nodepath->selected) == 0) {
        if (grow > 0) {
            sp_nodepath_node_select(n, TRUE, TRUE);
        }
        return;
    }

    if (g_list_length (nodepath->selected) == 1) {
        if (grow < 0) {
            sp_nodepath_deselect (nodepath);
            return;
        }
    }

    Inkscape::NodePath::Node *farthest_selected = NULL;
    double farthest_dist = 0;

    Inkscape::NodePath::Node *closest_unselected = NULL;
    double closest_dist = NR_HUGE;

    for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
           Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
           if (node == n)
               continue;
           if (node->selected) {
               if (NR::L2(node->pos - n->pos) > farthest_dist) {
                   farthest_dist = NR::L2(node->pos - n->pos);
                   farthest_selected = node;
               }
           } else {
               if (NR::L2(node->pos - n->pos) < closest_dist) {
                   closest_dist = NR::L2(node->pos - n->pos);
                   closest_unselected = node;
               }
           }
        }
    }

    if (grow > 0) {
        if (closest_unselected) {
            sp_nodepath_node_select(closest_unselected, TRUE, TRUE);
        }
    } else {
        if (farthest_selected) {
            sp_nodepath_node_select(farthest_selected, TRUE, FALSE);
        }
    }
}


/**
\brief  Saves all nodes' and handles' current positions in their origin members
*/
void
sp_nodepath_remember_origins(Inkscape::NodePath::Path *nodepath)
{
    for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
           Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) nl->data;
           n->origin = n->pos;
           n->p.origin = n->p.pos;
           n->n.origin = n->n.pos;
        }
    }
}

/**
\brief  Saves selected nodes in a nodepath into a list containing integer positions of all selected nodes
*/
GList *save_nodepath_selection(Inkscape::NodePath::Path *nodepath)
{
    if (!nodepath->selected) {
        return NULL;
    }

    GList *r = NULL;
    guint i = 0;
    for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
           Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
            i++;
            if (node->selected) {
                r = g_list_append(r, GINT_TO_POINTER(i));
            }
        }
    }
    return r;
}

/**
\brief  Restores selection by selecting nodes whose positions are in the list
*/
void restore_nodepath_selection(Inkscape::NodePath::Path *nodepath, GList *r)
{
    sp_nodepath_deselect(nodepath);

    guint i = 0;
    for (GList *spl = nodepath->subpaths; spl != NULL; spl = spl->next) {
       Inkscape::NodePath::SubPath *subpath = (Inkscape::NodePath::SubPath *) spl->data;
        for (GList *nl = subpath->nodes; nl != NULL; nl = nl->next) {
           Inkscape::NodePath::Node *node = (Inkscape::NodePath::Node *) nl->data;
            i++;
            if (g_list_find(r, GINT_TO_POINTER(i))) {
                sp_nodepath_node_select(node, TRUE, TRUE);
            }
        }
    }
}


/**
\brief Adjusts handle according to node type and line code.
*/
static void sp_node_adjust_handle(Inkscape::NodePath::Node *node, gint which_adjust)
{
    g_assert(node);

   Inkscape::NodePath::NodeSide *me = sp_node_get_side(node, which_adjust);
   Inkscape::NodePath::NodeSide *other = sp_node_opposite_side(node, me);

   // nothing to do if we are an end node
    if (me->other == NULL) return;
    if (other->other == NULL) return;

    // nothing to do if we are a cusp node
    if (node->type == Inkscape::NodePath::NODE_CUSP) return;

    // nothing to do if it's a line from the specified side of the node (i.e. no handle to adjust)
    NRPathcode mecode;
    if (which_adjust == 1) {
        mecode = (NRPathcode)me->other->code;
    } else {
        mecode = (NRPathcode)node->code;
    }
    if (mecode == NR_LINETO) return;

    if (sp_node_side_is_line(node, other)) {
        // other is a line, and we are either smooth or symm
       Inkscape::NodePath::Node *othernode = other->other;
        double len = NR::L2(me->pos - node->pos);
        NR::Point delta = node->pos - othernode->pos;
        double linelen = NR::L2(delta);
        if (linelen < 1e-18)
            return;
        me->pos = node->pos + (len / linelen)*delta;
        return;
    }

    if (node->type == Inkscape::NodePath::NODE_SYMM) {
        // symmetrize 
        me->pos = 2 * node->pos - other->pos;
        return;
    } else {
        // smoothify
        double len = NR::L2(me->pos - node->pos);
        NR::Point delta = other->pos - node->pos;
        double otherlen = NR::L2(delta);
        if (otherlen < 1e-18) return;
        me->pos = node->pos - (len / otherlen) * delta;
    }
}

/**
 \brief Adjusts both handles according to node type and line code
 */
static void sp_node_adjust_handles(Inkscape::NodePath::Node *node)
{
    g_assert(node);

    if (node->type == Inkscape::NodePath::NODE_CUSP) return;

    /* we are either smooth or symm */

    if (node->p.other == NULL) return;
    if (node->n.other == NULL) return;

    if (sp_node_side_is_line(node, &node->p)) {
        sp_node_adjust_handle(node, 1);
        return;
    }

    if (sp_node_side_is_line(node, &node->n)) {
        sp_node_adjust_handle(node, -1);
        return;
    }

    /* both are curves */
    NR::Point const delta( node->n.pos - node->p.pos );

    if (node->type == Inkscape::NodePath::NODE_SYMM) {
        node->p.pos = node->pos - delta / 2;
        node->n.pos = node->pos + delta / 2;
        return;
    }

    /* We are smooth */
    double plen = NR::L2(node->p.pos - node->pos);
    if (plen < 1e-18) return;
    double nlen = NR::L2(node->n.pos - node->pos);
    if (nlen < 1e-18) return;
    node->p.pos = node->pos - (plen / (plen + nlen)) * delta;
    node->n.pos = node->pos + (nlen / (plen + nlen)) * delta;
}

/**
 * Node event callback.
 */
static gboolean node_event(SPKnot */*knot*/, GdkEvent *event, Inkscape::NodePath::Node *n)
{
    gboolean ret = FALSE;
    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            Inkscape::NodePath::Path::active_node = n;
            break;
        case GDK_LEAVE_NOTIFY:
            Inkscape::NodePath::Path::active_node = NULL;
            break;
        case GDK_SCROLL:
            if ((event->scroll.state & GDK_CONTROL_MASK) && !(event->scroll.state & GDK_SHIFT_MASK)) { // linearly
                switch (event->scroll.direction) {
                    case GDK_SCROLL_UP:
                        nodepath_grow_selection_linearly (n->subpath->nodepath, n, +1);
                        break;
                    case GDK_SCROLL_DOWN:
                        nodepath_grow_selection_linearly (n->subpath->nodepath, n, -1);
                        break;
                    default:
                        break;
                }
                ret = TRUE;
            } else if (!(event->scroll.state & GDK_SHIFT_MASK)) { // spatially
                switch (event->scroll.direction) {
                    case GDK_SCROLL_UP:
                        nodepath_grow_selection_spatially (n->subpath->nodepath, n, +1);
                        break;
                    case GDK_SCROLL_DOWN:
                        nodepath_grow_selection_spatially (n->subpath->nodepath, n, -1);
                        break;
                    default:
                        break;
                }
                ret = TRUE;
            }
            break;
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_space:
                    if (event->key.state & GDK_BUTTON1_MASK) {
                        Inkscape::NodePath::Path *nodepath = n->subpath->nodepath;
                        stamp_repr(nodepath);
                        ret = TRUE;
                    }
                    break;
                case GDK_Page_Up:
                    if (event->key.state & GDK_CONTROL_MASK) {
                        nodepath_grow_selection_linearly (n->subpath->nodepath, n, +1);
                    } else {
                        nodepath_grow_selection_spatially (n->subpath->nodepath, n, +1);
                    }
                    break;
                case GDK_Page_Down:
                    if (event->key.state & GDK_CONTROL_MASK) {
                        nodepath_grow_selection_linearly (n->subpath->nodepath, n, -1);
                    } else {
                        nodepath_grow_selection_spatially (n->subpath->nodepath, n, -1);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return ret;
}

/**
 * Handle keypress on node; directly called.
 */
gboolean node_key(GdkEvent *event)
{
    Inkscape::NodePath::Path *np;

    // there is no way to verify nodes so set active_node to nil when deleting!!
    if (Inkscape::NodePath::Path::active_node == NULL) return FALSE;

    if ((event->type == GDK_KEY_PRESS) && !(event->key.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))) {
        gint ret = FALSE;
        switch (get_group0_keyval (&event->key)) {
            /// \todo FIXME: this does not seem to work, the keys are stolen by tool contexts!
            case GDK_BackSpace:
                np = Inkscape::NodePath::Path::active_node->subpath->nodepath;
                sp_nodepath_node_destroy(Inkscape::NodePath::Path::active_node);
                sp_nodepath_update_repr(np, _("Delete node"));
                Inkscape::NodePath::Path::active_node = NULL;
                ret = TRUE;
                break;
            case GDK_c:
                sp_nodepath_set_node_type(Inkscape::NodePath::Path::active_node,Inkscape::NodePath::NODE_CUSP);
                ret = TRUE;
                break;
            case GDK_s:
                sp_nodepath_set_node_type(Inkscape::NodePath::Path::active_node,Inkscape::NodePath::NODE_SMOOTH);
                ret = TRUE;
                break;
            case GDK_y:
                sp_nodepath_set_node_type(Inkscape::NodePath::Path::active_node,Inkscape::NodePath::NODE_SYMM);
                ret = TRUE;
                break;
            case GDK_b:
                sp_nodepath_node_break(Inkscape::NodePath::Path::active_node);
                ret = TRUE;
                break;
        }
        return ret;
    }
    return FALSE;
}

/**
 * Mouseclick on node callback.
 */
static void node_clicked(SPKnot */*knot*/, guint state, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    if (state & GDK_CONTROL_MASK) {
        Inkscape::NodePath::Path *nodepath = n->subpath->nodepath;

        if (!(state & GDK_MOD1_MASK)) { // ctrl+click: toggle node type
            if (n->type == Inkscape::NodePath::NODE_CUSP) {
                sp_nodepath_convert_node_type (n,Inkscape::NodePath::NODE_SMOOTH);
            } else if (n->type == Inkscape::NodePath::NODE_SMOOTH) {
                sp_nodepath_convert_node_type (n,Inkscape::NodePath::NODE_SYMM);
            } else {
                sp_nodepath_convert_node_type (n,Inkscape::NodePath::NODE_CUSP);
            }
            sp_nodepath_update_repr(nodepath, _("Change node type"));
            sp_nodepath_update_statusbar(nodepath);

        } else { //ctrl+alt+click: delete node
            GList *node_to_delete = NULL;
            node_to_delete = g_list_append(node_to_delete, n);
            sp_node_delete_preserve(node_to_delete);
        }

    } else {
        sp_nodepath_node_select(n, (state & GDK_SHIFT_MASK), FALSE);
    }
}

/**
 * Mouse grabbed node callback.
 */
static void node_grabbed(SPKnot */*knot*/, guint state, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    if (!n->selected) {
        sp_nodepath_node_select(n, (state & GDK_SHIFT_MASK), FALSE);
    }

    n->is_dragging = true;
    sp_canvas_force_full_redraw_after_interruptions(n->subpath->nodepath->desktop->canvas, 5);

    sp_nodepath_remember_origins (n->subpath->nodepath);
}

/**
 * Mouse ungrabbed node callback.
 */
static void node_ungrabbed(SPKnot */*knot*/, guint /*state*/, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

   n->dragging_out = NULL;
   n->is_dragging = false;
   sp_canvas_end_forced_full_redraws(n->subpath->nodepath->desktop->canvas);

   sp_nodepath_update_repr(n->subpath->nodepath, _("Move nodes"));
}

/**
 * The point on a line, given by its angle, closest to the given point.
 * \param p  A point.
 * \param a  Angle of the line; it is assumed to go through coordinate origin.
 * \param closest  Pointer to the point struct where the result is stored.
 * \todo FIXME: use dot product perhaps?
 */
static void point_line_closest(NR::Point *p, double a, NR::Point *closest)
{
    if (a == HUGE_VAL) { // vertical
        *closest = NR::Point(0, (*p)[NR::Y]);
    } else {
        (*closest)[NR::X] = ( a * (*p)[NR::Y] + (*p)[NR::X]) / (a*a + 1);
        (*closest)[NR::Y] = a * (*closest)[NR::X];
    }
}

/**
 * Distance from the point to a line given by its angle.
 * \param p  A point.
 * \param a  Angle of the line; it is assumed to go through coordinate origin.
 */
static double point_line_distance(NR::Point *p, double a)
{
    NR::Point c;
    point_line_closest(p, a, &c);
    return sqrt(((*p)[NR::X] - c[NR::X])*((*p)[NR::X] - c[NR::X]) + ((*p)[NR::Y] - c[NR::Y])*((*p)[NR::Y] - c[NR::Y]));
}

/**
 * Callback for node "request" signal.
 * \todo fixme: This goes to "moved" event? (lauris)
 */
static gboolean
node_request(SPKnot */*knot*/, NR::Point *p, guint state, gpointer data)
{
    double yn, xn, yp, xp;
    double an, ap, na, pa;
    double d_an, d_ap, d_na, d_pa;
    gboolean collinear = FALSE;
    NR::Point c;
    NR::Point pr;

    Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    n->subpath->nodepath->desktop->snapindicator->remove_snappoint();

    // If either (Shift and some handle retracted), or (we're already dragging out a handle)
    if ( (!n->subpath->nodepath->straight_path) &&
         ( ((state & GDK_SHIFT_MASK) && ((n->n.other && n->n.pos == n->pos) || (n->p.other && n->p.pos == n->pos)))
           || n->dragging_out ) )
    {
       NR::Point mouse = (*p);

       if (!n->dragging_out) {
           // This is the first drag-out event; find out which handle to drag out
           double appr_n = (n->n.other ? NR::L2(n->n.other->pos - n->pos) - NR::L2(n->n.other->pos - (*p)) : -HUGE_VAL);
           double appr_p = (n->p.other ? NR::L2(n->p.other->pos - n->pos) - NR::L2(n->p.other->pos - (*p)) : -HUGE_VAL);

           if (appr_p == -HUGE_VAL && appr_n == -HUGE_VAL) // orphan node?
               return FALSE;

           Inkscape::NodePath::NodeSide *opposite;
           if (appr_p > appr_n) { // closer to p
               n->dragging_out = &n->p;
               opposite = &n->n;
               n->code = NR_CURVETO;
           } else if (appr_p < appr_n) { // closer to n
               n->dragging_out = &n->n;
               opposite = &n->p;
               n->n.other->code = NR_CURVETO;
           } else { // p and n nodes are the same
               if (n->n.pos != n->pos) { // n handle already dragged, drag p
                   n->dragging_out = &n->p;
                   opposite = &n->n;
                   n->code = NR_CURVETO;
               } else if (n->p.pos != n->pos) { // p handle already dragged, drag n
                   n->dragging_out = &n->n;
                   opposite = &n->p;
                   n->n.other->code = NR_CURVETO;
               } else { // find out to which handle of the adjacent node we're closer; note that n->n.other == n->p.other
                   double appr_other_n = (n->n.other ? NR::L2(n->n.other->n.pos - n->pos) - NR::L2(n->n.other->n.pos - (*p)) : -HUGE_VAL);
                   double appr_other_p = (n->n.other ? NR::L2(n->n.other->p.pos - n->pos) - NR::L2(n->n.other->p.pos - (*p)) : -HUGE_VAL);
                   if (appr_other_p > appr_other_n) { // closer to other's p handle
                       n->dragging_out = &n->n;
                       opposite = &n->p;
                       n->n.other->code = NR_CURVETO;
                   } else { // closer to other's n handle
                       n->dragging_out = &n->p;
                       opposite = &n->n;
                       n->code = NR_CURVETO;
                   }
               }
           }

           // if there's another handle, make sure the one we drag out starts parallel to it
           if (opposite->pos != n->pos) {
               mouse = n->pos - NR::L2(mouse - n->pos) * NR::unit_vector(opposite->pos - n->pos);
           }

           // knots might not be created yet!
           sp_node_ensure_knot_exists (n->subpath->nodepath->desktop, n, n->dragging_out);
           sp_node_ensure_knot_exists (n->subpath->nodepath->desktop, n, opposite);
       }

       // pass this on to the handle-moved callback
       node_handle_moved(n->dragging_out->knot, &mouse, state, (gpointer) n);
       sp_node_update_handles(n);
       return TRUE;
   }

    if (state & GDK_CONTROL_MASK) { // constrained motion

        // calculate relative distances of handles
        // n handle:
        yn = n->n.pos[NR::Y] - n->pos[NR::Y];
        xn = n->n.pos[NR::X] - n->pos[NR::X];
        // if there's no n handle (straight line), see if we can use the direction to the next point on path
        if ((n->n.other && n->n.other->code == NR_LINETO) || fabs(yn) + fabs(xn) < 1e-6) {
            if (n->n.other) { // if there is the next point
                if (L2(n->n.other->p.pos - n->n.other->pos) < 1e-6) // and the next point has no handle either
                    yn = n->n.other->origin[NR::Y] - n->origin[NR::Y]; // use origin because otherwise the direction will change as you drag
                    xn = n->n.other->origin[NR::X] - n->origin[NR::X];
            }
        }
        if (xn < 0) { xn = -xn; yn = -yn; } // limit the angle to between 0 and pi
        if (yn < 0) { xn = -xn; yn = -yn; }

        // p handle:
        yp = n->p.pos[NR::Y] - n->pos[NR::Y];
        xp = n->p.pos[NR::X] - n->pos[NR::X];
        // if there's no p handle (straight line), see if we can use the direction to the prev point on path
        if (n->code == NR_LINETO || fabs(yp) + fabs(xp) < 1e-6) {
            if (n->p.other) {
                if (L2(n->p.other->n.pos - n->p.other->pos) < 1e-6)
                    yp = n->p.other->origin[NR::Y] - n->origin[NR::Y];
                    xp = n->p.other->origin[NR::X] - n->origin[NR::X];
            }
        }
        if (xp < 0) { xp = -xp; yp = -yp; } // limit the angle to between 0 and pi
        if (yp < 0) { xp = -xp; yp = -yp; }

        if (state & GDK_MOD1_MASK && !(xn == 0 && xp == 0)) {
            // sliding on handles, only if at least one of the handles is non-vertical
            // (otherwise it's the same as ctrl+drag anyway)

            // calculate angles of the handles
            if (xn == 0) {
                if (yn == 0) { // no handle, consider it the continuation of the other one
                    an = 0;
                    collinear = TRUE;
                }
                else an = 0; // vertical; set the angle to horizontal
            } else an = yn/xn;

            if (xp == 0) {
                if (yp == 0) { // no handle, consider it the continuation of the other one
                    ap = an;
                }
                else ap = 0; // vertical; set the angle to horizontal
            } else  ap = yp/xp;

            if (collinear) an = ap;

            // angles of the perpendiculars; HUGE_VAL means vertical
            if (an == 0) na = HUGE_VAL; else na = -1/an;
            if (ap == 0) pa = HUGE_VAL; else pa = -1/ap;

            // mouse point relative to the node's original pos
            pr = (*p) - n->origin;

            // distances to the four lines (two handles and two perpendiculars)
            d_an = point_line_distance(&pr, an);
            d_na = point_line_distance(&pr, na);
            d_ap = point_line_distance(&pr, ap);
            d_pa = point_line_distance(&pr, pa);

            // find out which line is the closest, save its closest point in c
            if (d_an <= d_na && d_an <= d_ap && d_an <= d_pa) {
                point_line_closest(&pr, an, &c);
            } else if (d_ap <= d_an && d_ap <= d_na && d_ap <= d_pa) {
                point_line_closest(&pr, ap, &c);
            } else if (d_na <= d_an && d_na <= d_ap && d_na <= d_pa) {
                point_line_closest(&pr, na, &c);
            } else if (d_pa <= d_an && d_pa <= d_ap && d_pa <= d_na) {
                point_line_closest(&pr, pa, &c);
            }

            // move the node to the closest point
            sp_nodepath_selected_nodes_move(n->subpath->nodepath,
                                            n->origin[NR::X] + c[NR::X] - n->pos[NR::X],
                                            n->origin[NR::Y] + c[NR::Y] - n->pos[NR::Y], 
                                            true);

        } else {  // constraining to hor/vert

            if (fabs((*p)[NR::X] - n->origin[NR::X]) > fabs((*p)[NR::Y] - n->origin[NR::Y])) { // snap to hor
                sp_nodepath_selected_nodes_move(n->subpath->nodepath,
                                                (*p)[NR::X] - n->pos[NR::X], 
                                                n->origin[NR::Y] - n->pos[NR::Y],
                                                true, 
                                                true, Inkscape::Snapper::ConstraintLine(component_vectors[NR::X]));
            } else { // snap to vert
                sp_nodepath_selected_nodes_move(n->subpath->nodepath,
                                                n->origin[NR::X] - n->pos[NR::X],
                                                (*p)[NR::Y] - n->pos[NR::Y],
                                                true,
                                                true, Inkscape::Snapper::ConstraintLine(component_vectors[NR::Y]));
            }
        }
    } else { // move freely
        if (n->is_dragging) {
            if (state & GDK_MOD1_MASK) { // sculpt
                sp_nodepath_selected_nodes_sculpt(n->subpath->nodepath, n, (*p) - n->origin);
            } else {
                sp_nodepath_selected_nodes_move(n->subpath->nodepath,
                                            (*p)[NR::X] - n->pos[NR::X],
                                            (*p)[NR::Y] - n->pos[NR::Y],
                                            (state & GDK_SHIFT_MASK) == 0);
            }
        }
    }

    n->subpath->nodepath->desktop->scroll_to_point(p);

    return TRUE;
}

/**
 * Node handle clicked callback.
 */
static void node_handle_clicked(SPKnot *knot, guint state, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    if (state & GDK_CONTROL_MASK) { // "delete" handle
        if (n->p.knot == knot) {
            n->p.pos = n->pos;
        } else if (n->n.knot == knot) {
            n->n.pos = n->pos;
        }
        sp_node_update_handles(n);
        Inkscape::NodePath::Path *nodepath = n->subpath->nodepath;
        sp_nodepath_update_repr(nodepath, _("Retract handle"));
        sp_nodepath_update_statusbar(nodepath);

    } else { // just select or add to selection, depending in Shift
        sp_nodepath_node_select(n, (state & GDK_SHIFT_MASK), FALSE);
    }
}

/**
 * Node handle grabbed callback.
 */
static void node_handle_grabbed(SPKnot *knot, guint state, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    if (!n->selected) {
        sp_nodepath_node_select(n, (state & GDK_SHIFT_MASK), FALSE);
    }

    // remember the origin point of the handle
    if (n->p.knot == knot) {
        n->p.origin_radial = n->p.pos - n->pos;
    } else if (n->n.knot == knot) {
        n->n.origin_radial = n->n.pos - n->pos;
    } else {
        g_assert_not_reached();
    }

    sp_canvas_force_full_redraw_after_interruptions(n->subpath->nodepath->desktop->canvas, 5);
}

/**
 * Node handle ungrabbed callback.
 */
static void node_handle_ungrabbed(SPKnot *knot, guint state, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    // forget origin and set knot position once more (because it can be wrong now due to restrictions)
    if (n->p.knot == knot) {
        n->p.origin_radial.a = 0;
        sp_knot_set_position(knot, n->p.pos, state);
    } else if (n->n.knot == knot) {
        n->n.origin_radial.a = 0;
        sp_knot_set_position(knot, n->n.pos, state);
    } else {
        g_assert_not_reached();
    }

    sp_nodepath_update_repr(n->subpath->nodepath, _("Move node handle"));
}

/**
 * Node handle "request" signal callback.
 */
static gboolean node_handle_request(SPKnot *knot, NR::Point *p, guint state, gpointer data)
{
    Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

    Inkscape::NodePath::NodeSide *me, *opposite;
    gint which;
    if (n->p.knot == knot) {
        me = &n->p;
        opposite = &n->n;
        which = -1;
    } else if (n->n.knot == knot) {
        me = &n->n;
        opposite = &n->p;
        which = 1;
    } else {
        me = opposite = NULL;
        which = 0;
        g_assert_not_reached();
    }

    SPDesktop *desktop = n->subpath->nodepath->desktop;
    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, n->subpath->nodepath->item);
    Inkscape::SnappedPoint s;
    
    if ((state & GDK_SHIFT_MASK) != 0) {
    	// We will not try to snap when the shift-key is pressed
    	// so remove the old snap indicator and don't wait for it to time-out  
    	desktop->snapindicator->remove_snappoint();   	
    }

    Inkscape::NodePath::Node *othernode = opposite->other;
    if (othernode) {
        if ((n->type != Inkscape::NodePath::NODE_CUSP) && sp_node_side_is_line(n, opposite)) {
            /* We are smooth node adjacent with line */
            NR::Point const delta = *p - n->pos;
            NR::Coord const len = NR::L2(delta);
            Inkscape::NodePath::Node *othernode = opposite->other;
            NR::Point const ndelta = n->pos - othernode->pos;
            NR::Coord const linelen = NR::L2(ndelta);
            if (len > NR_EPSILON && linelen > NR_EPSILON) {
                NR::Coord const scal = dot(delta, ndelta) / linelen;
                (*p) = n->pos + (scal / linelen) * ndelta;
            }
            if ((state & GDK_SHIFT_MASK) == 0) {
            	s = m.constrainedSnap(Inkscape::Snapper::SNAPPOINT_NODE, to_2geom(*p), Inkscape::Snapper::ConstraintLine(*p, ndelta));
            }
        } else {
        	if ((state & GDK_SHIFT_MASK) == 0) {
        		s = m.freeSnap(Inkscape::Snapper::SNAPPOINT_NODE, to_2geom(*p));
        	}
        }
    } else {
    	if ((state & GDK_SHIFT_MASK) == 0) {
    		s = m.freeSnap(Inkscape::Snapper::SNAPPOINT_NODE, to_2geom(*p));
    	}
    }
    
    Geom::Point pt2g = *p;
    s.getPoint(pt2g);
    *p = pt2g;
    
    sp_node_adjust_handle(n, -which);

    return FALSE;
}

/**
 * Node handle moved callback.
 */
static void node_handle_moved(SPKnot *knot, NR::Point *p, guint state, gpointer data)
{
   Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) data;

   Inkscape::NodePath::NodeSide *me;
   Inkscape::NodePath::NodeSide *other;
    if (n->p.knot == knot) {
        me = &n->p;
        other = &n->n;
    } else if (n->n.knot == knot) {
        me = &n->n;
        other = &n->p;
    } else {
        me = NULL;
        other = NULL;
        g_assert_not_reached();
    }

    // calculate radial coordinates of the grabbed handle, its other handle, and the mouse point
    Radial rme(me->pos - n->pos);
    Radial rother(other->pos - n->pos);
    Radial rnew(*p - n->pos);

    if (state & GDK_CONTROL_MASK && rnew.a != HUGE_VAL) {
        int const snaps = prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12);
        /* 0 interpreted as "no snapping". */

        // 1. Snap to the closest PI/snaps angle, starting from zero.
        double a_snapped = floor(rnew.a/(M_PI/snaps) + 0.5) * (M_PI/snaps);

        // 2. Snap to the original angle, its opposite and perpendiculars
        if (me->origin_radial.a != HUGE_VAL) { // otherwise ortho doesn't exist: original handle was zero length
            /* The closest PI/2 angle, starting from original angle */
            double const a_ortho = me->origin_radial.a + floor((rnew.a - me->origin_radial.a)/(M_PI/2) + 0.5) * (M_PI/2);

            // Snap to the closest.
            a_snapped = ( fabs(a_snapped - rnew.a) < fabs(a_ortho - rnew.a)
                       ? a_snapped
                       : a_ortho );
        }

        // 3. Snap to the angle of the opposite line, if any
        Inkscape::NodePath::Node *othernode = other->other;
        if (othernode) {
            NR::Point other_to_snap(0,0);
            if (sp_node_side_is_line(n, other)) {
                other_to_snap = othernode->pos - n->pos;
            } else {
                other_to_snap = other->pos - n->pos;
            }
            if (NR::L2(other_to_snap) > 1e-3) {
                Radial rother_to_snap(other_to_snap);
                /* The closest PI/2 angle, starting from the angle of the opposite line segment */
                double const a_oppo = rother_to_snap.a + floor((rnew.a - rother_to_snap.a)/(M_PI/2) + 0.5) * (M_PI/2);

                // Snap to the closest.
                a_snapped = ( fabs(a_snapped - rnew.a) < fabs(a_oppo - rnew.a)
                       ? a_snapped
                       : a_oppo );
            }
        }

        rnew.a = a_snapped;
    }

    if (state & GDK_MOD1_MASK) {
        // lock handle length
        rnew.r = me->origin_radial.r;
    }

    if (( n->type !=Inkscape::NodePath::NODE_CUSP || (state & GDK_SHIFT_MASK))
        && rme.a != HUGE_VAL && rnew.a != HUGE_VAL && (fabs(rme.a - rnew.a) > 0.001 || n->type ==Inkscape::NodePath::NODE_SYMM)) {
        // rotate the other handle correspondingly, if both old and new angles exist and are not the same
        rother.a += rnew.a - rme.a;
        other->pos = NR::Point(rother) + n->pos;
        if (other->knot) {
            sp_ctrlline_set_coords(SP_CTRLLINE(other->line), n->pos, other->pos);
            sp_knot_moveto(other->knot, other->pos);
        }
    }

    me->pos = NR::Point(rnew) + n->pos;
    sp_ctrlline_set_coords(SP_CTRLLINE(me->line), n->pos, me->pos);

    // move knot, but without emitting the signal:
    // we cannot emit a "moved" signal because we're now processing it
    sp_knot_moveto(me->knot, me->pos);

    update_object(n->subpath->nodepath);

    /* status text */
    SPDesktop *desktop = n->subpath->nodepath->desktop;
    if (!desktop) return;
    SPEventContext *ec = desktop->event_context;
    if (!ec) return;
    Inkscape::MessageContext *mc = get_message_context(ec);
    if (!mc) return;

    double degrees = 180 / M_PI * rnew.a;
    if (degrees > 180) degrees -= 360;
    if (degrees < -180) degrees += 360;
    if (prefs_get_int_attribute("options.compassangledisplay", "value", 0) != 0)
        degrees = angle_to_compass (degrees);

    GString *length = SP_PX_TO_METRIC_STRING(rnew.r, desktop->namedview->getDefaultMetric());

    mc->setF(Inkscape::IMMEDIATE_MESSAGE,
         _("<b>Node handle</b>: angle %0.2f&#176;, length %s; with <b>Ctrl</b> to snap angle; with <b>Alt</b> to lock length; with <b>Shift</b> to rotate both handles"), degrees, length->str);

    g_string_free(length, TRUE);
}

/**
 * Node handle event callback.
 */
static gboolean node_handle_event(SPKnot *knot, GdkEvent *event,Inkscape::NodePath::Node *n)
{
    gboolean ret = FALSE;
    switch (event->type) {
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_space:
                    if (event->key.state & GDK_BUTTON1_MASK) {
                        Inkscape::NodePath::Path *nodepath = n->subpath->nodepath;
                        stamp_repr(nodepath);
                        ret = TRUE;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GDK_ENTER_NOTIFY:
            // we use an experimentally determined threshold that seems to work fine
            if (NR::L2(n->pos - knot->pos) < 0.75)
                Inkscape::NodePath::Path::active_node = n;
            break;
        case GDK_LEAVE_NOTIFY:
            // we use an experimentally determined threshold that seems to work fine
            if (NR::L2(n->pos - knot->pos) < 0.75)
                Inkscape::NodePath::Path::active_node = NULL;
            break;
        default:
            break;
    }

    return ret;
}

static void node_rotate_one_internal(Inkscape::NodePath::Node const &n, gdouble const angle,
                                 Radial &rme, Radial &rother, gboolean const both)
{
    rme.a += angle;
    if ( both
         || ( n.type == Inkscape::NodePath::NODE_SMOOTH )
         || ( n.type == Inkscape::NodePath::NODE_SYMM )  )
    {
        rother.a += angle;
    }
}

static void node_rotate_one_internal_screen(Inkscape::NodePath::Node const &n, gdouble const angle,
                                        Radial &rme, Radial &rother, gboolean const both)
{
    gdouble const norm_angle = angle / n.subpath->nodepath->desktop->current_zoom();

    gdouble r;
    if ( both
         || ( n.type == Inkscape::NodePath::NODE_SMOOTH )
         || ( n.type == Inkscape::NodePath::NODE_SYMM )  )
    {
        r = MAX(rme.r, rother.r);
    } else {
        r = rme.r;
    }

    gdouble const weird_angle = atan2(norm_angle, r);
/* Bulia says norm_angle is just the visible distance that the
 * object's end must travel on the screen.  Left as 'angle' for want of
 * a better name.*/

    rme.a += weird_angle;
    if ( both
         || ( n.type == Inkscape::NodePath::NODE_SMOOTH )
         || ( n.type == Inkscape::NodePath::NODE_SYMM )  )
    {
        rother.a += weird_angle;
    }
}

/**
 * Rotate one node.
 */
static void node_rotate_one (Inkscape::NodePath::Node *n, gdouble angle, int which, gboolean screen)
{
    Inkscape::NodePath::NodeSide *me, *other;
    bool both = false;

    double xn = n->n.other? n->n.other->pos[NR::X] : n->pos[NR::X];
    double xp = n->p.other? n->p.other->pos[NR::X] : n->pos[NR::X];

    if (!n->n.other) { // if this is an endnode, select its single handle regardless of "which"
        me = &(n->p);
        other = &(n->n);
    } else if (!n->p.other) {
        me = &(n->n);
        other = &(n->p);
    } else {
        if (which > 0) { // right handle
            if (xn > xp) {
                me = &(n->n);
                other = &(n->p);
            } else {
                me = &(n->p);
                other = &(n->n);
            }
        } else if (which < 0){ // left handle
            if (xn <= xp) {
                me = &(n->n);
                other = &(n->p);
            } else {
                me = &(n->p);
                other = &(n->n);
            }
        } else { // both handles
            me = &(n->n);
            other = &(n->p);
            both = true;
        }
    }

    Radial rme(me->pos - n->pos);
    Radial rother(other->pos - n->pos);

    if (screen) {
        node_rotate_one_internal_screen (*n, angle, rme, rother, both);
    } else {
        node_rotate_one_internal (*n, angle, rme, rother, both);
    }

    me->pos = n->pos + NR::Point(rme);

    if (both || n->type == Inkscape::NodePath::NODE_SMOOTH || n->type == Inkscape::NodePath::NODE_SYMM) {
        other->pos =  n->pos + NR::Point(rother);
    }

    // this function is only called from sp_nodepath_selected_nodes_rotate that will update display at the end,
    // so here we just move all the knots without emitting move signals, for speed
    sp_node_update_handles(n, false);
}

/**
 * Rotate selected nodes.
 */
void sp_nodepath_selected_nodes_rotate(Inkscape::NodePath::Path *nodepath, gdouble angle, int which, bool screen)
{
    if (!nodepath || !nodepath->selected) return;

    if (g_list_length(nodepath->selected) == 1) {
       Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) nodepath->selected->data;
        node_rotate_one (n, angle, which, screen);
    } else {
       // rotate as an object:

        Inkscape::NodePath::Node *n0 = (Inkscape::NodePath::Node *) nodepath->selected->data;
        NR::Rect box (n0->pos, n0->pos); // originally includes the first selected node
        for (GList *l = nodepath->selected; l != NULL; l = l->next) {
            Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
            box.expandTo (n->pos); // contain all selected nodes
        }

        gdouble rot;
        if (screen) {
            gdouble const zoom = nodepath->desktop->current_zoom();
            gdouble const zmove = angle / zoom;
            gdouble const r = NR::L2(box.max() - box.midpoint());
            rot = atan2(zmove, r);
        } else {
            rot = angle;
        }

        NR::Point rot_center;
        if (Inkscape::NodePath::Path::active_node == NULL)
            rot_center = box.midpoint();
        else
            rot_center = Inkscape::NodePath::Path::active_node->pos;

        NR::Matrix t =
            NR::Matrix (NR::translate(-rot_center)) *
            NR::Matrix (NR::rotate(rot)) *
            NR::Matrix (NR::translate(rot_center));

        for (GList *l = nodepath->selected; l != NULL; l = l->next) {
            Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
            n->pos *= t;
            n->n.pos *= t;
            n->p.pos *= t;
            sp_node_update_handles(n, false);
        }
    }

    sp_nodepath_update_repr_keyed(nodepath, angle > 0 ? "nodes:rot:p" : "nodes:rot:n", _("Rotate nodes"));
}

/**
 * Scale one node.
 */
static void node_scale_one (Inkscape::NodePath::Node *n, gdouble grow, int which)
{
    bool both = false;
    Inkscape::NodePath::NodeSide *me, *other;

    double xn = n->n.other? n->n.other->pos[NR::X] : n->pos[NR::X];
    double xp = n->p.other? n->p.other->pos[NR::X] : n->pos[NR::X];

    if (!n->n.other) { // if this is an endnode, select its single handle regardless of "which"
        me = &(n->p);
        other = &(n->n);
        n->code = NR_CURVETO;
    } else if (!n->p.other) {
        me = &(n->n);
        other = &(n->p);
        if (n->n.other)
            n->n.other->code = NR_CURVETO;
    } else {
        if (which > 0) { // right handle
            if (xn > xp) {
                me = &(n->n);
                other = &(n->p);
                if (n->n.other)
                    n->n.other->code = NR_CURVETO;
            } else {
                me = &(n->p);
                other = &(n->n);
                n->code = NR_CURVETO;
            }
        } else if (which < 0){ // left handle
            if (xn <= xp) {
                me = &(n->n);
                other = &(n->p);
                if (n->n.other)
                    n->n.other->code = NR_CURVETO;
            } else {
                me = &(n->p);
                other = &(n->n);
                n->code = NR_CURVETO;
            }
        } else { // both handles
            me = &(n->n);
            other = &(n->p);
            both = true;
            n->code = NR_CURVETO;
            if (n->n.other)
                n->n.other->code = NR_CURVETO;
        }
    }

    Radial rme(me->pos - n->pos);
    Radial rother(other->pos - n->pos);

    rme.r += grow;
    if (rme.r < 0) rme.r = 0;
    if (rme.a == HUGE_VAL) {
        if (me->other) { // if direction is unknown, initialize it towards the next node
            Radial rme_next(me->other->pos - n->pos);
            rme.a = rme_next.a;
        } else { // if there's no next, initialize to 0
            rme.a = 0;
        }
    }
    if (both || n->type == Inkscape::NodePath::NODE_SYMM) {
        rother.r += grow;
        if (rother.r < 0) rother.r = 0;
        if (rother.a == HUGE_VAL) {
            rother.a = rme.a + M_PI;
        }
    }

    me->pos = n->pos + NR::Point(rme);

    if (both || n->type == Inkscape::NodePath::NODE_SYMM) {
        other->pos = n->pos + NR::Point(rother);
    }

    // this function is only called from sp_nodepath_selected_nodes_scale that will update display at the end,
    // so here we just move all the knots without emitting move signals, for speed
    sp_node_update_handles(n, false);
}

/**
 * Scale selected nodes.
 */
void sp_nodepath_selected_nodes_scale(Inkscape::NodePath::Path *nodepath, gdouble const grow, int const which)
{
    if (!nodepath || !nodepath->selected) return;

    if (g_list_length(nodepath->selected) == 1) {
        // scale handles of the single selected node
        Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) nodepath->selected->data;
        node_scale_one (n, grow, which);
    } else {
        // scale nodes as an "object":

        Inkscape::NodePath::Node *n0 = (Inkscape::NodePath::Node *) nodepath->selected->data;
        NR::Rect box (n0->pos, n0->pos); // originally includes the first selected node
        for (GList *l = nodepath->selected; l != NULL; l = l->next) {
            Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
            box.expandTo (n->pos); // contain all selected nodes
        }

        double scale = (box.maxExtent() + grow)/box.maxExtent();

        NR::Point scale_center;
        if (Inkscape::NodePath::Path::active_node == NULL)
            scale_center = box.midpoint();
        else
            scale_center = Inkscape::NodePath::Path::active_node->pos;

        NR::Matrix t =
            NR::Matrix (NR::translate(-scale_center)) *
            NR::Matrix (NR::scale(scale, scale)) *
            NR::Matrix (NR::translate(scale_center));

        for (GList *l = nodepath->selected; l != NULL; l = l->next) {
            Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
            n->pos *= t;
            n->n.pos *= t;
            n->p.pos *= t;
            sp_node_update_handles(n, false);
        }
    }

    sp_nodepath_update_repr_keyed(nodepath, grow > 0 ? "nodes:scale:p" : "nodes:scale:n", _("Scale nodes"));
}

void sp_nodepath_selected_nodes_scale_screen(Inkscape::NodePath::Path *nodepath, gdouble const grow, int const which)
{
    if (!nodepath) return;
    sp_nodepath_selected_nodes_scale(nodepath, grow / nodepath->desktop->current_zoom(), which);
}

/**
 * Flip selected nodes horizontally/vertically.
 */
void sp_nodepath_flip (Inkscape::NodePath::Path *nodepath, NR::Dim2 axis, boost::optional<NR::Point> center)
{
    if (!nodepath || !nodepath->selected) return;

    if (g_list_length(nodepath->selected) == 1 && !center) {
        // flip handles of the single selected node
        Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) nodepath->selected->data;
        double temp = n->p.pos[axis];
        n->p.pos[axis] = n->n.pos[axis];
        n->n.pos[axis] = temp;
        sp_node_update_handles(n, false);
    } else {
        // scale nodes as an "object":

        Geom::Rect box = sp_node_selected_bbox (nodepath);
        if (!center) {
            center = box.midpoint();
        }
        NR::Matrix t =
            NR::Matrix (NR::translate(- *center)) *
            NR::Matrix ((axis == NR::X)? NR::scale(-1, 1) : NR::scale(1, -1)) *
            NR::Matrix (NR::translate(*center));

        for (GList *l = nodepath->selected; l != NULL; l = l->next) {
            Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
            n->pos *= t;
            n->n.pos *= t;
            n->p.pos *= t;
            sp_node_update_handles(n, false);
        }
    }

    sp_nodepath_update_repr(nodepath, _("Flip nodes"));
}

Geom::Rect sp_node_selected_bbox (Inkscape::NodePath::Path *nodepath)
{
    g_assert (nodepath->selected);

    Inkscape::NodePath::Node *n0 = (Inkscape::NodePath::Node *) nodepath->selected->data;
    Geom::Rect box (n0->pos, n0->pos); // originally includes the first selected node
    for (GList *l = nodepath->selected; l != NULL; l = l->next) {
        Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node *) l->data;
        box.expandTo (n->pos); // contain all selected nodes
    }
    return box;
}

//-----------------------------------------------
/**
 * Return new subpath under given nodepath.
 */
static Inkscape::NodePath::SubPath *sp_nodepath_subpath_new(Inkscape::NodePath::Path *nodepath)
{
    g_assert(nodepath);
    g_assert(nodepath->desktop);

   Inkscape::NodePath::SubPath *s = g_new(Inkscape::NodePath::SubPath, 1);

    s->nodepath = nodepath;
    s->closed = FALSE;
    s->nodes = NULL;
    s->first = NULL;
    s->last = NULL;

    // using prepend here saves up to 10% of time on paths with many subpaths, but requires that
    // the caller reverses the list after it's ready (this is done in sp_nodepath_new)
    nodepath->subpaths = g_list_prepend (nodepath->subpaths, s);

    return s;
}

/**
 * Destroy nodes in subpath, then subpath itself.
 */
static void sp_nodepath_subpath_destroy(Inkscape::NodePath::SubPath *subpath)
{
    g_assert(subpath);
    g_assert(subpath->nodepath);
    g_assert(g_list_find(subpath->nodepath->subpaths, subpath));

    while (subpath->nodes) {
        sp_nodepath_node_destroy((Inkscape::NodePath::Node *) subpath->nodes->data);
    }

    subpath->nodepath->subpaths = g_list_remove(subpath->nodepath->subpaths, subpath);

    g_free(subpath);
}

/**
 * Link head to tail in subpath.
 */
static void sp_nodepath_subpath_close(Inkscape::NodePath::SubPath *sp)
{
    g_assert(!sp->closed);
    g_assert(sp->last != sp->first);
    g_assert(sp->first->code == NR_MOVETO);

    sp->closed = TRUE;

    //Link the head to the tail
    sp->first->p.other = sp->last;
    sp->last->n.other  = sp->first;
    sp->last->n.pos    = sp->last->pos + (sp->first->n.pos - sp->first->pos);
    sp->first          = sp->last;

    //Remove the extra end node
    sp_nodepath_node_destroy(sp->last->n.other);
}

/**
 * Open closed (loopy) subpath at node.
 */
static void sp_nodepath_subpath_open(Inkscape::NodePath::SubPath *sp,Inkscape::NodePath::Node *n)
{
    g_assert(sp->closed);
    g_assert(n->subpath == sp);
    g_assert(sp->first == sp->last);

    /* We create new startpoint, current node will become last one */

   Inkscape::NodePath::Node *new_path = sp_nodepath_node_new(sp, n->n.other,Inkscape::NodePath::NODE_CUSP, NR_MOVETO,
                                                &n->pos, &n->pos, &n->n.pos);


    sp->closed        = FALSE;

    //Unlink to make a head and tail
    sp->first         = new_path;
    sp->last          = n;
    n->n.other        = NULL;
    new_path->p.other = NULL;
}

/**
 * Return new node in subpath with given properties.
 * \param pos Position of node.
 * \param ppos Handle position in previous direction
 * \param npos Handle position in previous direction
 */
Inkscape::NodePath::Node *
sp_nodepath_node_new(Inkscape::NodePath::SubPath *sp, Inkscape::NodePath::Node *next, Inkscape::NodePath::NodeType type, NRPathcode code, NR::Point *ppos, NR::Point *pos, NR::Point *npos)
{
    g_assert(sp);
    g_assert(sp->nodepath);
    g_assert(sp->nodepath->desktop);

    if (nodechunk == NULL)
        nodechunk = g_mem_chunk_create(Inkscape::NodePath::Node, 32, G_ALLOC_AND_FREE);

    Inkscape::NodePath::Node *n = (Inkscape::NodePath::Node*)g_mem_chunk_alloc(nodechunk);

    n->subpath  = sp;

    if (type != Inkscape::NodePath::NODE_NONE) {
        // use the type from sodipodi:nodetypes
        n->type = type;
    } else {
        if (fabs (Inkscape::Util::triangle_area (*pos, *ppos, *npos)) < 1e-2) {
            // points are (almost) collinear
            if (NR::L2(*pos - *ppos) < 1e-6 || NR::L2(*pos - *npos) < 1e-6) {
                // endnode, or a node with a retracted handle
                n->type = Inkscape::NodePath::NODE_CUSP;
            } else {
                n->type = Inkscape::NodePath::NODE_SMOOTH;
            }
        } else {
            n->type = Inkscape::NodePath::NODE_CUSP;
        }
    }

    n->code     = code;
    n->selected = FALSE;
    n->pos      = *pos;
    n->p.pos    = *ppos;
    n->n.pos    = *npos;

    n->dragging_out = NULL;

    Inkscape::NodePath::Node *prev;
    if (next) {
        //g_assert(g_list_find(sp->nodes, next));
        prev = next->p.other;
    } else {
        prev = sp->last;
    }

    if (prev)
        prev->n.other = n;
    else
        sp->first = n;

    if (next)
        next->p.other = n;
    else
        sp->last = n;

    n->p.other = prev;
    n->n.other = next;

    n->knot = sp_knot_new(sp->nodepath->desktop, _("<b>Node</b>: drag to edit the path; with <b>Ctrl</b> to snap to horizontal/vertical; with <b>Ctrl+Alt</b> to snap to handles' directions"));
    sp_knot_set_position(n->knot, *pos, 0);

    n->knot->setShape ((n->type == Inkscape::NodePath::NODE_CUSP)? SP_KNOT_SHAPE_DIAMOND : SP_KNOT_SHAPE_SQUARE);
    n->knot->setSize ((n->type == Inkscape::NodePath::NODE_CUSP)? 9 : 7);
    n->knot->setAnchor (GTK_ANCHOR_CENTER);
    n->knot->setFill(NODE_FILL, NODE_FILL_HI, NODE_FILL_HI);
    n->knot->setStroke(NODE_STROKE, NODE_STROKE_HI, NODE_STROKE_HI);
    sp_knot_update_ctrl(n->knot);

    g_signal_connect(G_OBJECT(n->knot), "event", G_CALLBACK(node_event), n);
    g_signal_connect(G_OBJECT(n->knot), "clicked", G_CALLBACK(node_clicked), n);
    g_signal_connect(G_OBJECT(n->knot), "grabbed", G_CALLBACK(node_grabbed), n);
    g_signal_connect(G_OBJECT(n->knot), "ungrabbed", G_CALLBACK(node_ungrabbed), n);
    g_signal_connect(G_OBJECT(n->knot), "request", G_CALLBACK(node_request), n);
    sp_knot_show(n->knot);

    // We only create handle knots and lines on demand
    n->p.knot = NULL;
    n->p.line = NULL;
    n->n.knot = NULL;
    n->n.line = NULL;

    sp->nodes = g_list_prepend(sp->nodes, n);

    return n;
}

/**
 * Destroy node and its knots, link neighbors in subpath.
 */
static void sp_nodepath_node_destroy(Inkscape::NodePath::Node *node)
{
    g_assert(node);
    g_assert(node->subpath);
    g_assert(SP_IS_KNOT(node->knot));

   Inkscape::NodePath::SubPath *sp = node->subpath;

    if (node->selected) { // first, deselect
        g_assert(g_list_find(node->subpath->nodepath->selected, node));
        node->subpath->nodepath->selected = g_list_remove(node->subpath->nodepath->selected, node);
    }

    node->subpath->nodes = g_list_remove(node->subpath->nodes, node);

    g_signal_handlers_disconnect_by_func(G_OBJECT(node->knot), (gpointer) G_CALLBACK(node_event), node);
    g_signal_handlers_disconnect_by_func(G_OBJECT(node->knot), (gpointer) G_CALLBACK(node_clicked), node);
    g_signal_handlers_disconnect_by_func(G_OBJECT(node->knot), (gpointer) G_CALLBACK(node_grabbed), node);
    g_signal_handlers_disconnect_by_func(G_OBJECT(node->knot), (gpointer) G_CALLBACK(node_ungrabbed), node);
    g_signal_handlers_disconnect_by_func(G_OBJECT(node->knot), (gpointer) G_CALLBACK(node_request), node);
    g_object_unref(G_OBJECT(node->knot));

    if (node->p.knot) {
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->p.knot), (gpointer) G_CALLBACK(node_handle_clicked), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->p.knot), (gpointer) G_CALLBACK(node_handle_grabbed), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->p.knot), (gpointer) G_CALLBACK(node_handle_ungrabbed), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->p.knot), (gpointer) G_CALLBACK(node_handle_request), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->p.knot), (gpointer) G_CALLBACK(node_handle_moved), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->p.knot), (gpointer) G_CALLBACK(node_handle_event), node);
        g_object_unref(G_OBJECT(node->p.knot));
        node->p.knot = NULL;
    }

    if (node->n.knot) {
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->n.knot), (gpointer) G_CALLBACK(node_handle_clicked), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->n.knot), (gpointer) G_CALLBACK(node_handle_grabbed), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->n.knot), (gpointer) G_CALLBACK(node_handle_ungrabbed), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->n.knot), (gpointer) G_CALLBACK(node_handle_request), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->n.knot), (gpointer) G_CALLBACK(node_handle_moved), node);
        g_signal_handlers_disconnect_by_func(G_OBJECT(node->n.knot), (gpointer) G_CALLBACK(node_handle_event), node);
        g_object_unref(G_OBJECT(node->n.knot));
        node->n.knot = NULL;
    }

    if (node->p.line)
        gtk_object_destroy(GTK_OBJECT(node->p.line));
    if (node->n.line)
        gtk_object_destroy(GTK_OBJECT(node->n.line));

    if (sp->nodes) { // there are others nodes on the subpath
        if (sp->closed) {
            if (sp->first == node) {
                g_assert(sp->last == node);
                sp->first = node->n.other;
                sp->last = sp->first;
            }
            node->p.other->n.other = node->n.other;
            node->n.other->p.other = node->p.other;
        } else {
            if (sp->first == node) {
                sp->first = node->n.other;
                sp->first->code = NR_MOVETO;
            }
            if (sp->last == node) sp->last = node->p.other;
            if (node->p.other) node->p.other->n.other = node->n.other;
            if (node->n.other) node->n.other->p.other = node->p.other;
        }
    } else { // this was the last node on subpath
        sp->nodepath->subpaths = g_list_remove(sp->nodepath->subpaths, sp);
    }

    g_mem_chunk_free(nodechunk, node);
}

/**
 * Returns one of the node's two sides.
 * \param which Indicates which side.
 * \return Pointer to previous node side if which==-1, next if which==1.
 */
static Inkscape::NodePath::NodeSide *sp_node_get_side(Inkscape::NodePath::Node *node, gint which)
{
    g_assert(node);

    switch (which) {
        case -1:
            return &node->p;
        case 1:
            return &node->n;
        default:
            break;
    }

    g_assert_not_reached();

    return NULL;
}

/**
 * Return the other side of the node, given one of its sides.
 */
static Inkscape::NodePath::NodeSide *sp_node_opposite_side(Inkscape::NodePath::Node *node, Inkscape::NodePath::NodeSide *me)
{
    g_assert(node);

    if (me == &node->p) return &node->n;
    if (me == &node->n) return &node->p;

    g_assert_not_reached();

    return NULL;
}

/**
 * Return NRPathcode on the given side of the node.
 */
static NRPathcode sp_node_path_code_from_side(Inkscape::NodePath::Node *node,Inkscape::NodePath::NodeSide *me)
{
    g_assert(node);

    if (me == &node->p) {
        if (node->p.other) return (NRPathcode)node->code;
        return NR_MOVETO;
    }

    if (me == &node->n) {
        if (node->n.other) return (NRPathcode)node->n.other->code;
        return NR_MOVETO;
    }

    g_assert_not_reached();

    return NR_END;
}

/**
 * Return node with the given index
 */
Inkscape::NodePath::Node *
sp_nodepath_get_node_by_index(Inkscape::NodePath::Path *nodepath, int index)
{
    Inkscape::NodePath::Node *e = NULL;

    if (!nodepath) {
        return e;
    }

    //find segment
    for (GList *l = nodepath->subpaths; l ; l=l->next) {

        Inkscape::NodePath::SubPath *sp = (Inkscape::NodePath::SubPath *)l->data;
        int n = g_list_length(sp->nodes);
        if (sp->closed) {
            n++;
        }

        //if the piece belongs to this subpath grab it
        //otherwise move onto the next subpath
        if (index < n) {
            e = sp->first;
            for (int i = 0; i < index; ++i) {
                e = e->n.other;
            }
            break;
        } else {
            if (sp->closed) {
                index -= (n+1);
            } else {
                index -= n;
            }
        }
    }

    return e;
}

/**
 * Returns plain text meaning of node type.
 */
static gchar const *sp_node_type_description(Inkscape::NodePath::Node *node)
{
    unsigned retracted = 0;
    bool endnode = false;

    for (int which = -1; which <= 1; which += 2) {
        Inkscape::NodePath::NodeSide *side = sp_node_get_side(node, which);
        if (side->other && NR::L2(side->pos - node->pos) < 1e-6)
            retracted ++;
        if (!side->other)
            endnode = true;
    }

    if (retracted == 0) {
        if (endnode) {
                // TRANSLATORS: "end" is an adjective here (NOT a verb)
                return _("end node");
        } else {
            switch (node->type) {
                case Inkscape::NodePath::NODE_CUSP:
                    // TRANSLATORS: "cusp" means "sharp" (cusp node); see also the Advanced Tutorial
                    return _("cusp");
                case Inkscape::NodePath::NODE_SMOOTH:
                    // TRANSLATORS: "smooth" is an adjective here
                    return _("smooth");
                case Inkscape::NodePath::NODE_SYMM:
                    return _("symmetric");
            }
        }
    } else if (retracted == 1) {
        if (endnode) {
            // TRANSLATORS: "end" is an adjective here (NOT a verb)
            return _("end node, handle retracted (drag with <b>Shift</b> to extend)");
        } else {
            return _("one handle retracted (drag with <b>Shift</b> to extend)");
        }
    } else {
        return _("both handles retracted (drag with <b>Shift</b> to extend)");
    }

    return NULL;
}

/**
 * Handles content of statusbar as long as node tool is active.
 */
void
sp_nodepath_update_statusbar(Inkscape::NodePath::Path *nodepath)//!!!move to ShapeEditorsCollection
{
    gchar const *when_selected = _("<b>Drag</b> nodes or node handles; <b>Alt+drag</b> nodes to sculpt; <b>arrow</b> keys to move nodes, <b>&lt; &gt;</b> to scale, <b>[ ]</b> to rotate");
    gchar const *when_selected_one = _("<b>Drag</b> the node or its handles; <b>arrow</b> keys to move the node");

    gint total_nodes = sp_nodepath_get_node_count(nodepath);
    gint selected_nodes = sp_nodepath_selection_get_node_count(nodepath);
    gint total_subpaths = sp_nodepath_get_subpath_count(nodepath);
    gint selected_subpaths = sp_nodepath_selection_get_subpath_count(nodepath);

    SPDesktop *desktop = NULL;
    if (nodepath) {
        desktop = nodepath->desktop;
    } else {
        desktop = SP_ACTIVE_DESKTOP; // when this is eliminated also remove #include "inkscape.h" above
    }

    SPEventContext *ec = desktop->event_context;
    if (!ec) return;
    Inkscape::MessageContext *mc = get_message_context(ec);
    if (!mc) return;

    inkscape_active_desktop()->emitToolSubselectionChanged(NULL);

    if (selected_nodes == 0) {
        Inkscape::Selection *sel = desktop->selection;
        if (!sel || sel->isEmpty()) {
            mc->setF(Inkscape::NORMAL_MESSAGE,
                     _("Select a single object to edit its nodes or handles."));
        } else {
            if (nodepath) {
            mc->setF(Inkscape::NORMAL_MESSAGE,
                     ngettext("<b>0</b> out of <b>%i</b> node selected. <b>Click</b>, <b>Shift+click</b>, or <b>drag around</b> nodes to select.",
                              "<b>0</b> out of <b>%i</b> nodes selected. <b>Click</b>, <b>Shift+click</b>, or <b>drag around</b> nodes to select.",
                              total_nodes),
                     total_nodes);
            } else {
                if (g_slist_length((GSList *)sel->itemList()) == 1) {
                    mc->setF(Inkscape::NORMAL_MESSAGE, _("Drag the handles of the object to modify it."));
                } else {
                    mc->setF(Inkscape::NORMAL_MESSAGE, _("Select a single object to edit its nodes or handles."));
                }
            }
        }
    } else if (nodepath && selected_nodes == 1) {
        mc->setF(Inkscape::NORMAL_MESSAGE,
                 ngettext("<b>%i</b> of <b>%i</b> node selected; %s. %s.",
                          "<b>%i</b> of <b>%i</b> nodes selected; %s. %s.",
                          total_nodes),
                 selected_nodes, total_nodes, sp_node_type_description((Inkscape::NodePath::Node *) nodepath->selected->data), when_selected_one);
    } else {
        if (selected_subpaths > 1) {
            mc->setF(Inkscape::NORMAL_MESSAGE,
                     ngettext("<b>%i</b> of <b>%i</b> node selected in <b>%i</b> of <b>%i</b> subpaths. %s.",
                              "<b>%i</b> of <b>%i</b> nodes selected in <b>%i</b> of <b>%i</b> subpaths. %s.",
                              total_nodes),
                     selected_nodes, total_nodes, selected_subpaths, total_subpaths, when_selected);
        } else {
            mc->setF(Inkscape::NORMAL_MESSAGE,
                     ngettext("<b>%i</b> of <b>%i</b> node selected. %s.",
                              "<b>%i</b> of <b>%i</b> nodes selected. %s.",
                              total_nodes),
                     selected_nodes, total_nodes, when_selected);
        }
    }
}

/*
 * returns a *copy* of the curve of that object.
 */
SPCurve* sp_nodepath_object_get_curve(SPObject *object, const gchar *key) {
    if (!object)
        return NULL;

    SPCurve *curve = NULL;
    if (SP_IS_PATH(object)) {
        SPCurve *curve_new = sp_path_get_curve_for_edit(SP_PATH(object));
        curve = curve_new->copy();
    } else if ( IS_LIVEPATHEFFECT(object) && key) {
        const gchar *svgd = object->repr->attribute(key);
        if (svgd) {
            Geom::PathVector pv = sp_svg_read_pathv(svgd);
            SPCurve *curve_new = new SPCurve(pv);
            if (curve_new) {
                curve = curve_new; // don't do curve_copy because curve_new is already only created for us!
            }
        }
    }

    return curve;
}

void sp_nodepath_set_curve (Inkscape::NodePath::Path *np, SPCurve *curve) {
    if (!np || !np->object || !curve)
        return;

    if (SP_IS_PATH(np->object)) {
        if (sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(np->object))) {
            sp_path_set_original_curve(SP_PATH(np->object), curve, true, false);
        } else {
            sp_shape_set_curve(SP_SHAPE(np->object), curve, true);
        }
    } else if ( IS_LIVEPATHEFFECT(np->object) ) {
        Inkscape::LivePathEffect::PathParam *pathparam = dynamic_cast<Inkscape::LivePathEffect::PathParam *>( LIVEPATHEFFECT(np->object)->lpe->getParameter(np->repr_key) );
        if (pathparam) {
            pathparam->set_new_value(np->curve->get_pathvector(), false); // do not write to SVG
            np->object->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

/**
SPCanvasItem *
sp_nodepath_path_to_canvasitem(Inkscape::NodePath::Path *np, SPPath *path) {
    return sp_nodepath_make_helper_item(np, sp_path_get_curve_for_edit(path));
}
**/

/**
SPCanvasItem *
sp_nodepath_generate_helperpath(SPDesktop *desktop, SPCurve *curve, const SPItem *item, guint32 color = 0xff0000ff) {
    SPCurve *flash_curve = curve->copy();
    Geom::Matrix i2d = item ? sp_item_i2d_affine(item) : Geom::identity();
    flash_curve->transform(i2d);
    SPCanvasItem * canvasitem = sp_canvas_bpath_new(sp_desktop_tempgroup(desktop), flash_curve);
    // would be nice if its color could be XORed or something, now it is invisible for red stroked objects...
    // unless we also flash the nodes...
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(canvasitem), color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(canvasitem), 0, SP_WIND_RULE_NONZERO);
    sp_canvas_item_show(canvasitem);
    flash_curve->unref();
    return canvasitem;
}

SPCanvasItem *
sp_nodepath_generate_helperpath(SPDesktop *desktop, SPPath *path) {
    return sp_nodepath_generate_helperpath(desktop, sp_path_get_curve_for_edit(path), SP_ITEM(path),
                                           prefs_get_int_attribute("tools.nodes", "highlight_color", 0xff0000ff));
}
**/

SPCanvasItem *
sp_nodepath_helperpath_from_path(SPDesktop *desktop, SPPath *path) {
    SPCurve *flash_curve = sp_path_get_curve_for_edit(path)->copy();
    Geom::Matrix i2d = sp_item_i2d_affine(SP_ITEM(path));
    flash_curve->transform(i2d);
    SPCanvasItem * canvasitem = sp_canvas_bpath_new(sp_desktop_tempgroup(desktop), flash_curve);
    // would be nice if its color could be XORed or something, now it is invisible for red stroked objects...
    // unless we also flash the nodes...
    guint32 color = prefs_get_int_attribute("tools.nodes", "highlight_color", 0xff0000ff);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(canvasitem), color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(canvasitem), 0, SP_WIND_RULE_NONZERO);
    sp_canvas_item_show(canvasitem);
    flash_curve->unref();
    return canvasitem;
}

// TODO: Merge this with sp_nodepath_make_helper_item()!
void sp_nodepath_show_helperpath(Inkscape::NodePath::Path *np, bool show) {
    np->show_helperpath = show;

    if (show) {
        SPCurve *helper_curve = np->curve->copy();
        helper_curve->transform(np->i2d);
        if (!np->helper_path) {
            //np->helper_path = sp_nodepath_make_helper_item(np, desktop, helper_curve, true); // Caution: this applies the transform np->i2d twice!!

            np->helper_path = sp_canvas_bpath_new(sp_desktop_controls(np->desktop), helper_curve);
            sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(np->helper_path), np->helperpath_rgba, np->helperpath_width, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
            sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(np->helper_path), 0, SP_WIND_RULE_NONZERO);
            sp_canvas_item_move_to_z(np->helper_path, 0);
            sp_canvas_item_show(np->helper_path);
        } else {
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(np->helper_path), helper_curve);
        }
        helper_curve->unref();
    } else {
        if (np->helper_path) {
            GtkObject *temp = np->helper_path;
            np->helper_path = NULL;
            gtk_object_destroy(temp);
        }
    }
}

/* sp_nodepath_make_straight_path:
 *   Prevents user from curving the path by dragging a segment or activating handles etc.
 *   The resulting path is a linear interpolation between nodal points, with only straight segments.
 * !!! this function does not work completely yet: it does not actively straighten the path, only prevents the path from being curved
 */
void sp_nodepath_make_straight_path(Inkscape::NodePath::Path *np) {
    np->straight_path = true;
    np->show_handles = false;
    g_message("add code to make the path straight.");
    // do sp_nodepath_convert_node_type on all nodes?
    // coding tip: search for this text : "Make selected segments lines"
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

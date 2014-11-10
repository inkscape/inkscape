/*
 * On-canvas gradient dragging
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005,2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm/i18n.h>
#include <cstring>
#include <string>

#include "desktop-handles.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-style.h"
#include "desktop-handles.h"
#include "document.h"
#include "document-undo.h"
#include "display/sp-ctrlline.h"
#include "display/sp-ctrlcurve.h"
#include "display/sp-canvas-util.h"
#include "xml/repr.h"
#include "xml/sp-css-attr.h"
#include "svg/css-ostringstream.h"
#include "svg/svg.h"
#include "preferences.h"
#include "inkscape.h"
#include "sp-item.h"
#include "style.h"
#include "knot.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-mesh-gradient.h"
#include "sp-mesh-row.h"
#include "sp-mesh-patch.h"
#include "gradient-chemistry.h"
#include "gradient-drag.h"
#include "sp-stop.h"
#include "snap.h"
#include "sp-namedview.h"
#include "selection-chemistry.h"
#include "verbs.h"
#include "display/sp-canvas.h"
#include "ui/control-manager.h"
#include "ui/tools/tool-base.h"

using Inkscape::ControlManager;
using Inkscape::CtrlLineType;
using Inkscape::DocumentUndo;
using Inkscape::allPaintTargets;
using Inkscape::CTLINE_PRIMARY;
using Inkscape::CTLINE_SECONDARY;

#define GR_KNOT_COLOR_NORMAL 0xffffff00
#define GR_KNOT_COLOR_MOUSEOVER 0xff000000
#define GR_KNOT_COLOR_SELECTED 0x0000ff00

#define GR_LINE_COLOR_FILL 0x0000ff7f
#define GR_LINE_COLOR_STROKE 0x9999007f

// screen pixels between knots when they snap:
#define SNAP_DIST 5

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

// knot shapes corresponding to GrPointType enum (in sp-gradient.h)
SPKnotShapeType gr_knot_shapes [] = {
        SP_KNOT_SHAPE_SQUARE,  // POINT_LG_BEGIN
        SP_KNOT_SHAPE_CIRCLE,  // POINT_LG_END
        SP_KNOT_SHAPE_DIAMOND, // POINT_LG_MID
        SP_KNOT_SHAPE_SQUARE,  // POINT_RG_CENTER
        SP_KNOT_SHAPE_CIRCLE,  // POINT_RG_R1
        SP_KNOT_SHAPE_CIRCLE,  // POINT_RG_R2
        SP_KNOT_SHAPE_CROSS,   // POINT_RG_FOCUS
        SP_KNOT_SHAPE_DIAMOND, // POINT_RG_MID1
        SP_KNOT_SHAPE_DIAMOND, // POINT_RG_MID2
        SP_KNOT_SHAPE_DIAMOND, // POINT_MG_CORNER
        SP_KNOT_SHAPE_CIRCLE,  // POINT_MG_HANDLE
        SP_KNOT_SHAPE_SQUARE   // POINT_MG_TENSOR
};

const gchar *gr_knot_descr [] = {
    N_("Linear gradient <b>start</b>"), //POINT_LG_BEGIN
    N_("Linear gradient <b>end</b>"),
    N_("Linear gradient <b>mid stop</b>"),
    N_("Radial gradient <b>center</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>focus</b>"), // POINT_RG_FOCUS
    N_("Radial gradient <b>mid stop</b>"),
    N_("Radial gradient <b>mid stop</b>"),
    N_("Mesh gradient <b>corner</b>"),
    N_("Mesh gradient <b>handle</b>"),
    N_("Mesh gradient <b>tensor</b>")
};

static void
gr_drag_sel_changed(Inkscape::Selection */*selection*/, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;
    drag->updateDraggers ();
    drag->updateLines ();
    drag->updateLevels ();
}

static void gr_drag_sel_modified(Inkscape::Selection */*selection*/, guint /*flags*/, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;
    if (drag->local_change) {
        drag->local_change = false;
    } else {
        drag->updateDraggers ();
    }
    drag->updateLines ();
    drag->updateLevels ();
}

/**
 * When a _query_style_signal is received, check that \a property requests fill/stroke/opacity (otherwise
 * skip), and fill the \a style with the averaged color of all draggables of the selected dragger, if
 * any.
 */
static int gr_drag_style_query(SPStyle *style, int property, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;

    if (property != QUERY_STYLE_PROPERTY_FILL && property != QUERY_STYLE_PROPERTY_STROKE && property != QUERY_STYLE_PROPERTY_MASTEROPACITY) {
        return QUERY_STYLE_NOTHING;
    }

    if (!drag->selected) {
        return QUERY_STYLE_NOTHING;
    } else {
        int ret = QUERY_STYLE_NOTHING;

        float cf[4];
        cf[0] = cf[1] = cf[2] = cf[3] = 0;

        int count = 0;

        for (GList *i = drag->selected; i != NULL; i = i->next) { // for all selected draggers
            GrDragger *d = (GrDragger *) i->data;
            for (GSList const* j = d->draggables; j != NULL; j = j->next) { // for all draggables of dragger
                GrDraggable *draggable = (GrDraggable *) j->data;

                if (ret == QUERY_STYLE_NOTHING) {
                    ret = QUERY_STYLE_SINGLE;
                } else if (ret == QUERY_STYLE_SINGLE) {
                    ret = QUERY_STYLE_MULTIPLE_AVERAGED;
                }

                guint32 c = sp_item_gradient_stop_query_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
                cf[0] += SP_RGBA32_R_F (c);
                cf[1] += SP_RGBA32_G_F (c);
                cf[2] += SP_RGBA32_B_F (c);
                cf[3] += SP_RGBA32_A_F (c);

                count ++;
            }
        }

        if (count) {
            cf[0] /= count;
            cf[1] /= count;
            cf[2] /= count;
            cf[3] /= count;

            // set both fill and stroke with our stop-color and stop-opacity
            style->fill.clear();
            style->fill.setColor( cf[0], cf[1], cf[2] );
            style->fill.set = TRUE;
            style->stroke.clear();
            style->stroke.setColor( cf[0], cf[1], cf[2] );
            style->stroke.set = TRUE;

            style->fill_opacity.value = SP_SCALE24_FROM_FLOAT (cf[3]);
            style->fill_opacity.set = TRUE;
            style->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (cf[3]);
            style->stroke_opacity.set = TRUE;

            style->opacity.value = SP_SCALE24_FROM_FLOAT (cf[3]);
            style->opacity.set = TRUE;
        }

        return ret;
    }
}

Glib::ustring GrDrag::makeStopSafeColor( gchar const *str, bool &isNull )
{
    Glib::ustring colorStr;
    if ( str ) {
        isNull = false;
        colorStr = str;
        Glib::ustring::size_type pos = colorStr.find("url(#");
        if ( pos != Glib::ustring::npos ) {
            Glib::ustring targetName = colorStr.substr(pos + 5, colorStr.length() - 6);
            const GSList *gradients = desktop->doc()->getResourceList("gradient");
            for (const GSList *item = gradients; item; item = item->next) {
                SPGradient* grad = SP_GRADIENT(item->data);
                if ( targetName == grad->getId() ) {
                    SPGradient *vect = grad->getVector();
                    SPStop *firstStop = (vect) ? vect->getFirstStop() : grad->getFirstStop();
                    if (firstStop) {
                        Glib::ustring stopColorStr;
                        if (firstStop->currentColor) {
                            stopColorStr = firstStop->getStyleProperty("color", NULL);
                        } else {
                            stopColorStr = firstStop->specified_color.toString();
                        }
                        if ( !stopColorStr.empty() ) {
                            colorStr = stopColorStr;
                        }
                    }
                    break;
                }
            }
        }
    } else {
        isNull = true;
    }

    return colorStr;
}

bool GrDrag::styleSet( const SPCSSAttr *css )
{
    if (!selected) {
        return false;
    }

    SPCSSAttr *stop = sp_repr_css_attr_new();

    // See if the css contains interesting properties, and if so, translate them into the format
    // acceptable for gradient stops

    // any of color properties, in order of increasing priority:
    if (css->attribute("flood-color")) {
        sp_repr_css_set_property (stop, "stop-color", css->attribute("flood-color"));
    }

    if (css->attribute("lighting-color")) {
        sp_repr_css_set_property (stop, "stop-color", css->attribute("lighting-color"));
    }

    if (css->attribute("color")) {
        sp_repr_css_set_property (stop, "stop-color", css->attribute("color"));
    }

    if (css->attribute("stroke") && strcmp(css->attribute("stroke"), "none")) {
        sp_repr_css_set_property (stop, "stop-color", css->attribute("stroke"));
    }

    if (css->attribute("fill") && strcmp(css->attribute("fill"), "none")) {
        sp_repr_css_set_property (stop, "stop-color", css->attribute("fill"));
    }

    if (css->attribute("stop-color")) {
        sp_repr_css_set_property (stop, "stop-color", css->attribute("stop-color"));
    }

    // Make sure the style is allowed for gradient stops.
    if ( !sp_repr_css_property_is_unset( stop, "stop-color") ) {
        bool stopIsNull = false;
        Glib::ustring tmp = makeStopSafeColor( sp_repr_css_property( stop, "stop-color", "" ), stopIsNull );
        if ( !stopIsNull && !tmp.empty() ) {
            sp_repr_css_set_property( stop, "stop-color", tmp.c_str() );
        }
    }


    if (css->attribute("stop-opacity")) { // direct setting of stop-opacity has priority
        sp_repr_css_set_property(stop, "stop-opacity", css->attribute("stop-opacity"));
    } else {  // multiply all opacity properties:
        gdouble accumulated = 1.0;
        accumulated *= sp_svg_read_percentage(css->attribute("flood-opacity"), 1.0);
        accumulated *= sp_svg_read_percentage(css->attribute("opacity"), 1.0);
        accumulated *= sp_svg_read_percentage(css->attribute("stroke-opacity"), 1.0);
        accumulated *= sp_svg_read_percentage(css->attribute("fill-opacity"), 1.0);

        Inkscape::CSSOStringStream os;
        os << accumulated;
        sp_repr_css_set_property(stop, "stop-opacity", os.str().c_str());

        if ((css->attribute("fill") && !css->attribute("stroke") && !strcmp(css->attribute("fill"), "none")) ||
            (css->attribute("stroke") && !css->attribute("fill") && !strcmp(css->attribute("stroke"), "none"))) {
            sp_repr_css_set_property(stop, "stop-opacity", "0"); // if a single fill/stroke property is set to none, don't change color, set opacity to 0
        }
    }

    if (!stop->attributeList()) { // nothing for us here, pass it on
        sp_repr_css_attr_unref(stop);
        return false;
    }

    for (GList const* sel = selected; sel != NULL; sel = sel->next) { // for all selected draggers
        GrDragger* dragger = reinterpret_cast<GrDragger*>(sel->data);
        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
            GrDraggable *draggable = reinterpret_cast<GrDraggable *>(i->data);

            local_change = true;
            sp_item_gradient_stop_set_style(draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke, stop);
        }
    }

    //sp_repr_css_print(stop);
    sp_repr_css_attr_unref(stop);
    return true;
}

guint32 GrDrag::getColor()
{
    if (!selected) return 0;

    float cf[4];
    cf[0] = cf[1] = cf[2] = cf[3] = 0;

    int count = 0;

    for (GList *i = selected; i != NULL; i = i->next) { // for all selected draggers
        GrDragger *d = (GrDragger *) i->data;
        for (GSList const* j = d->draggables; j != NULL; j = j->next) { // for all draggables of dragger
            GrDraggable *draggable = (GrDraggable *) j->data;

            guint32 c = sp_item_gradient_stop_query_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
            cf[0] += SP_RGBA32_R_F (c);
            cf[1] += SP_RGBA32_G_F (c);
            cf[2] += SP_RGBA32_B_F (c);
            cf[3] += SP_RGBA32_A_F (c);

            count ++;
        }
    }

    if (count) {
        cf[0] /= count;
        cf[1] /= count;
        cf[2] /= count;
        cf[3] /= count;
    }

    return SP_RGBA32_F_COMPOSE(cf[0], cf[1], cf[2], cf[3]);
}

// TODO refactor early returns
SPStop *GrDrag::addStopNearPoint(SPItem *item, Geom::Point mouse_p, double tolerance)
{
    gfloat new_stop_offset = 0; // type of SPStop.offset = gfloat
    SPGradient *gradient = 0;
    //bool r1_knot = false;

    // For Mesh
    int divide_row = -1;
    int divide_column = -1;
    double divide_coord = 0.5;

    bool addknot = false;

    for (std::vector<Inkscape::PaintTarget>::const_iterator it = allPaintTargets().begin(); (it != allPaintTargets().end()) && !addknot; ++it)
    {
        Inkscape::PaintTarget fill_or_stroke = *it;
        gradient = getGradient(item, fill_or_stroke);
        if (SP_IS_LINEARGRADIENT(gradient)) {
            Geom::Point begin   = getGradientCoords(item, POINT_LG_BEGIN, 0, fill_or_stroke);
            Geom::Point end     = getGradientCoords(item, POINT_LG_END, 0, fill_or_stroke);
            Geom::LineSegment ls(begin, end);
            double offset = ls.nearestPoint(mouse_p);
            Geom::Point nearest = ls.pointAt(offset);
            double dist_screen = Geom::distance(mouse_p, nearest);
            if ( dist_screen < tolerance ) {
                // calculate the new stop offset
                new_stop_offset = distance(begin, nearest) / distance(begin, end);
                // add the knot
                addknot = true;
            }
        } else if (SP_IS_RADIALGRADIENT(gradient)) {
            Geom::Point begin = getGradientCoords(item, POINT_RG_CENTER, 0, fill_or_stroke);
            Geom::Point end   = getGradientCoords(item, POINT_RG_R1, 0, fill_or_stroke);
            Geom::LineSegment ls(begin, end);
            double offset = ls.nearestPoint(mouse_p);
            Geom::Point nearest = ls.pointAt(offset);
            double dist_screen = Geom::distance(mouse_p, nearest);
            if ( dist_screen < tolerance ) {
                // calculate the new stop offset
                new_stop_offset = distance(begin, nearest) / distance(begin, end);
                // add the knot
                addknot = true;
                //r1_knot = true;
            } else {
                end = getGradientCoords(item, POINT_RG_R2, 0, fill_or_stroke);
                ls = Geom::LineSegment(begin, end);
                offset = ls.nearestPoint(mouse_p);
                nearest = ls.pointAt(offset);
                dist_screen = Geom::distance(mouse_p, nearest);
                if ( dist_screen < tolerance ) {
                    // calculate the new stop offset
                    new_stop_offset = distance(begin, nearest) / distance(begin, end);
                    // add the knot
                    addknot = true;
                    //r1_knot = false;
                }
            }
        } else if (SP_IS_MESHGRADIENT(gradient)) {

            // add_stop_near_point()
            // Find out which curve pointer is over and use that curve to determine
            // which row or column will be divided.
            // This is silly as we already should know which line we are over...
            // but that information is not saved (sp_gradient_context_is_over_line).

            SPMeshGradient *mg = SP_MESHGRADIENT(gradient);
            Geom::Affine transform = Geom::Affine(mg->gradientTransform)*(Geom::Affine)item->i2dt_affine();

            guint rows    = mg->array.patch_rows();
            guint columns = mg->array.patch_columns();

            double closest = 1e10;
            for( guint i = 0; i < rows; ++i ) {
                for( guint j = 0; j < columns; ++j ) {

                    SPMeshPatchI patch( &(mg->array.nodes), i, j );
                    Geom::Point p[4];

                    // Top line
                    {
                        p[0] = patch.getPoint( 0, 0 ) * transform; 
                        p[1] = patch.getPoint( 0, 1 ) * transform; 
                        p[2] = patch.getPoint( 0, 2 ) * transform; 
                        p[3] = patch.getPoint( 0, 3 ) * transform; 
                        Geom::BezierCurveN<3> b( p[0], p[1], p[2], p[3] );
                        Geom::Coord coord = b.nearestPoint( mouse_p );
                        Geom::Point nearest = b( coord );
                        double dist_screen = Geom::L2 ( mouse_p - nearest );
                        if ( dist_screen < closest ) {
                            closest = dist_screen;
                            divide_row = -1;
                            divide_column = j;
                            divide_coord = coord;
                        }
                    }

                    // Right line (only for last column)
                    if( j == columns - 1 ) {
                        p[0] = patch.getPoint( 1, 0 ) * transform; 
                        p[1] = patch.getPoint( 1, 1 ) * transform; 
                        p[2] = patch.getPoint( 1, 2 ) * transform; 
                        p[3] = patch.getPoint( 1, 3 ) * transform; 
                        Geom::BezierCurveN<3> b( p[0], p[1], p[2], p[3] );
                        Geom::Coord coord = b.nearestPoint( mouse_p );
                        Geom::Point nearest = b( coord );
                        double dist_screen = Geom::L2 ( mouse_p - nearest );
                        if ( dist_screen < closest ) {
                            closest = dist_screen;
                            divide_row = i;
                            divide_column = -1;
                            divide_coord = coord;
                        }
                    }

                    // Bottom line (only for last row)
                    if( i == rows - 1 ) {
                        p[0] = patch.getPoint( 2, 0 ) * transform; 
                        p[1] = patch.getPoint( 2, 1 ) * transform; 
                        p[2] = patch.getPoint( 2, 2 ) * transform; 
                        p[3] = patch.getPoint( 2, 3 ) * transform; 
                        Geom::BezierCurveN<3> b( p[0], p[1], p[2], p[3] );
                        Geom::Coord coord = b.nearestPoint( mouse_p );
                        Geom::Point nearest = b( coord );
                        double dist_screen = Geom::L2 ( mouse_p - nearest );
                        if ( dist_screen < closest ) {
                            closest = dist_screen;
                            divide_row = -1;
                            divide_column = j;
                            divide_coord = 1.0 - coord;
                        }
                    }

                    // Left line
                    {
                        p[0] = patch.getPoint( 3, 0 ) * transform; 
                        p[1] = patch.getPoint( 3, 1 ) * transform; 
                        p[2] = patch.getPoint( 3, 2 ) * transform; 
                        p[3] = patch.getPoint( 3, 3 ) * transform; 
                        Geom::BezierCurveN<3> b( p[0], p[1], p[2], p[3] );
                        Geom::Coord coord = b.nearestPoint( mouse_p );
                        Geom::Point nearest = b( coord );
                        double dist_screen = Geom::L2 ( mouse_p - nearest );
                        if ( dist_screen < closest ) {
                            closest = dist_screen;
                            divide_row = i;
                            divide_column = -1;
                            divide_coord = 1.0 - coord;
                        }
                    }

                } // End loop over columns
            } // End loop rows

            if( closest < tolerance ) {
                addknot = true;
            }

        } // End if mesh

    }

    if (addknot) {

        if( SP_IS_LINEARGRADIENT(gradient) || SP_IS_RADIALGRADIENT( gradient ) ) {
            SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (gradient, false);
            SPStop* prev_stop = vector->getFirstStop();
            SPStop* next_stop = prev_stop->getNextStop();
            guint i = 1;
            while ( (next_stop) && (next_stop->offset < new_stop_offset) ) {
                prev_stop = next_stop;
                next_stop = next_stop->getNextStop();
                i++;
            }
            if (!next_stop) {
                // logical error: the endstop should have offset 1 and should always be more than this offset here
                return NULL;
            }


            SPStop *newstop = sp_vector_add_stop (vector, prev_stop, next_stop, new_stop_offset);
            gradient->ensureVector();
            updateDraggers();

            // so that it does not automatically update draggers in idle loop, as this would deselect
            local_change = true;

            // select the newly created stop
            selectByStop(newstop);

            return newstop;

        } else {

            SPMeshGradient *mg = SP_MESHGRADIENT(gradient);

            if( divide_row > -1 ) {
                mg->array.split_row( divide_row, divide_coord );
            } else {
                mg->array.split_column( divide_column, divide_coord );
            }

            // Update repr
            sp_meshgradient_repr_write( mg );
            mg->array.built = false;
            mg->ensureArray();
            // How do we do this?
            DocumentUndo::done(sp_desktop_document (desktop), SP_VERB_CONTEXT_MESH,
                               _("Added patch row or column"));

        } // Mesh
    }

    return NULL;
}


bool GrDrag::dropColor(SPItem */*item*/, gchar const *c, Geom::Point p)
{
    // Note: not sure if a null pointer can come in for the style, but handle that just in case
    bool stopIsNull = false;
    Glib::ustring toUse = makeStopSafeColor( c, stopIsNull );

    // first, see if we can drop onto one of the existing draggers
    for (GList *i = draggers; i != NULL; i = i->next) { // for all draggables of dragger
        GrDragger *d = (GrDragger *) i->data;

        if (Geom::L2(p - d->point)*desktop->current_zoom() < 5) {
           SPCSSAttr *stop = sp_repr_css_attr_new ();
           sp_repr_css_set_property( stop, "stop-color", stopIsNull ? 0 : toUse.c_str() );
           sp_repr_css_set_property( stop, "stop-opacity", "1" );
           for (GSList *j = d->draggables; j != NULL; j = j->next) { // for all draggables of dragger
               GrDraggable *draggable = (GrDraggable *) j->data;
               local_change = true;
               sp_item_gradient_stop_set_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke, stop);
           }
           sp_repr_css_attr_unref(stop);
           return true;
        }
    }

    // now see if we're over line and create a new stop
    bool over_line = false;
    if (lines) {
        for (GSList *l = lines; (l != NULL) && (!over_line); l = l->next) {
            SPCtrlLine *line = (SPCtrlLine*) l->data;
            Geom::LineSegment ls(line->s, line->e);
            Geom::Point nearest = ls.pointAt(ls.nearestPoint(p));
            double dist_screen = Geom::L2(p - nearest) * desktop->current_zoom();
            if (line->item && dist_screen < 5) {
                SPStop *stop = addStopNearPoint(line->item, p, 5/desktop->current_zoom());
                if (stop) {
                    SPCSSAttr *css = sp_repr_css_attr_new();
                    sp_repr_css_set_property( css, "stop-color", stopIsNull ? 0 : toUse.c_str() );
                    sp_repr_css_set_property( css, "stop-opacity", "1" );
                    sp_repr_css_change(stop->getRepr(), css, "style");
                    return true;
                }
            }
        }
    }

    return false;
}


GrDrag::GrDrag(SPDesktop *desktop) :
    selected(0),
    keep_selection(false),
    local_change(false),
    desktop(desktop),
    hor_levels(),
    vert_levels(),
    draggers(0),
    lines(0),
    selection(sp_desktop_selection(desktop)),
    sel_changed_connection(),
    sel_modified_connection(),
    style_set_connection(),
    style_query_connection()
{
    sel_changed_connection = selection->connectChangedFirst(
        sigc::bind(
            sigc::ptr_fun(&gr_drag_sel_changed),
            (gpointer)this )

        );
    sel_modified_connection = selection->connectModifiedFirst(
        sigc::bind(
            sigc::ptr_fun(&gr_drag_sel_modified),
            (gpointer)this )
        );

    style_set_connection = desktop->connectSetStyle( sigc::mem_fun(*this, &GrDrag::styleSet) );

    style_query_connection = desktop->connectQueryStyle(
        sigc::bind(
            sigc::ptr_fun(&gr_drag_style_query),
            (gpointer)this )
        );

    updateDraggers();
    updateLines();
    updateLevels();

    if (desktop->gr_item) {
        GrDragger *dragger = getDraggerFor(desktop->gr_item, desktop->gr_point_type, desktop->gr_point_i, desktop->gr_fill_or_stroke);
        if (dragger) {
            setSelected(dragger);
        }
    }
}

GrDrag::~GrDrag()
{
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();
    this->style_set_connection.disconnect();
    this->style_query_connection.disconnect();

    if (this->selected) {
        GrDraggable *draggable = (GrDraggable *)   ((GrDragger*)this->selected->data)->draggables->data;
        desktop->gr_item = draggable->item;
        desktop->gr_point_type = draggable->point_type;
        desktop->gr_point_i = draggable->point_i;
        desktop->gr_fill_or_stroke = draggable->fill_or_stroke;
    } else {
        desktop->gr_item = NULL;
        desktop->gr_point_type = POINT_LG_BEGIN;
        desktop->gr_point_i = 0;
        desktop->gr_fill_or_stroke = Inkscape::FOR_FILL;
    }

    deselect_all();
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        delete ((GrDragger *) l->data);
    }
    g_list_free (this->draggers);
    this->draggers = NULL;
    this->selected = NULL;

    for (GSList *l = this->lines; l != NULL; l = l->next) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(l->data));
    }
    g_slist_free (this->lines);
    this->lines = NULL;
}

GrDraggable::GrDraggable(SPItem *item, GrPointType point_type, guint point_i, Inkscape::PaintTarget fill_or_stroke) :
    item(item),
    point_type(point_type),
    point_i(point_i),
    fill_or_stroke(fill_or_stroke)
{
    //g_object_ref(G_OBJECT(item));
	sp_object_ref(item);
}

GrDraggable::~GrDraggable()
{
    //g_object_unref (G_OBJECT (this->item));
	sp_object_unref(this->item);
}


SPObject *GrDraggable::getServer()
{
    SPObject *server = 0;
    if (item) {
        switch (fill_or_stroke) {
            case Inkscape::FOR_FILL:
                server = item->style->getFillPaintServer();
                break;
            case Inkscape::FOR_STROKE:
                server = item->style->getStrokePaintServer();
                break;
        }
    }

    return server;
}

static void gr_knot_moved_handler(SPKnot *knot, Geom::Point const &ppointer, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;
    GrDrag *drag = dragger->parent;

    Geom::Point p = ppointer;

    SPDesktop *desktop = dragger->parent->desktop;
    SnapManager &m = desktop->namedview->snap_manager;
    double snap_dist = m.snapprefs.getObjectTolerance() / dragger->parent->desktop->current_zoom();

    if (state & GDK_SHIFT_MASK) {
        // with Shift; unsnap if we carry more than one draggable
        if (dragger->draggables && dragger->draggables->next) {
            // create a new dragger
            GrDragger *dr_new = new GrDragger (dragger->parent, dragger->point, NULL);
            dragger->parent->draggers = g_list_prepend (dragger->parent->draggers, dr_new);
            // relink to it all but the first draggable in the list
            for (GSList const* i = dragger->draggables->next; i != NULL; i = i->next) {
                GrDraggable *draggable = (GrDraggable *) i->data;
                dr_new->addDraggable (draggable);
            }
            dr_new->updateKnotShape();
            g_slist_free (dragger->draggables->next);
            dragger->draggables->next = NULL;
            dragger->updateKnotShape();
            dragger->updateTip();
        }
    } else if (!(state & GDK_CONTROL_MASK)) {
        // without Shift or Ctrl; see if we need to snap to another dragger
        for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
            GrDragger *d_new = (GrDragger *) di->data;
            if (dragger->mayMerge(d_new) && Geom::L2 (d_new->point - p) < snap_dist) {

                // Merge draggers:
                for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
                    GrDraggable *draggable = (GrDraggable *) i->data;
                    // copy draggable to d_new:
                    GrDraggable *da_new = new GrDraggable (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
                    d_new->addDraggable (da_new);
                }

                // unlink and delete this dragger
                dragger->parent->draggers = g_list_remove (dragger->parent->draggers, dragger);
                d_new->parent->draggers = g_list_remove (d_new->parent->draggers, dragger);
                d_new->parent->selected = g_list_remove (d_new->parent->selected, dragger);
                delete dragger;

                // throw out delayed snap context 
                Inkscape::UI::Tools::sp_event_context_discard_delayed_snap_event(SP_ACTIVE_DESKTOP->event_context);

                // update the new merged dragger
                d_new->fireDraggables(true, false, true);
                d_new->parent->updateLines();
                d_new->parent->setSelected (d_new);
                d_new->updateKnotShape ();
                d_new->updateTip ();
                d_new->updateDependencies(true);
                DocumentUndo::done(sp_desktop_document (d_new->parent->desktop), SP_VERB_CONTEXT_GRADIENT, _("Merge gradient handles"));
                return;
            }
        }
    }

    if (!((state & GDK_SHIFT_MASK) || (state & GDK_CONTROL_MASK))) {
        m.setup(desktop);
        Inkscape::SnappedPoint s = m.freeSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_OTHER_HANDLE));
        m.unSetup();
        if (s.getSnapped()) {
            p = s.getPoint();
            knot->moveto(p);
        }
    } else if (state & GDK_CONTROL_MASK) {
        IntermSnapResults isr;
        Inkscape::SnapCandidatePoint scp = Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        unsigned snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));
        /* 0 means no snapping. */

        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) {
            GrDraggable *draggable = (GrDraggable *) i->data;

            Geom::Point dr_snap(Geom::infinity(), Geom::infinity());

            if (draggable->point_type == POINT_LG_BEGIN || draggable->point_type == POINT_LG_END) {
                for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item,
                                    draggable->point_type == POINT_LG_BEGIN? POINT_LG_END : POINT_LG_BEGIN,
                                    draggable->fill_or_stroke)) {
                        // found the other end of the linear gradient;
                        if (state & GDK_SHIFT_MASK) {
                            // moving linear around center
                            Geom::Point center = Geom::Point (0.5*(d_new->point + dragger->point));
                            dr_snap = center;
                        } else {
                            // moving linear around the other end
                            dr_snap = d_new->point;
                        }
                    }
                }
            } else if (draggable->point_type == POINT_RG_R1 || draggable->point_type == POINT_RG_R2 || draggable->point_type == POINT_RG_FOCUS) {
                for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item,
                                    POINT_RG_CENTER,
                                    draggable->fill_or_stroke)) {
                        // found the center of the radial gradient;
                        dr_snap = d_new->point;
                    }
                }
            } else if (draggable->point_type == POINT_RG_CENTER) {
                // radial center snaps to hor/vert relative to its original position
                dr_snap = dragger->point_original;
            } else if (draggable->point_type == POINT_MG_CORNER ||
                       draggable->point_type == POINT_MG_HANDLE ||
                       draggable->point_type == POINT_MG_TENSOR ) {
                // std::cout << " gr_knot_moved_handler: Got mesh point!" << std::endl;
            }

            // dr_snap contains the origin of the gradient, whereas p will be the new endpoint which we will try to snap now
            Inkscape::SnappedPoint sp;
            if (dr_snap.isFinite()) {
                m.setup(desktop);
                if (state & GDK_MOD1_MASK) {
                    // with Alt, snap to the original angle and its perpendiculars
                    sp = m.constrainedAngularSnap(scp, dragger->point_original, dr_snap, 2);
                } else {
                    // with Ctrl, snap to M_PI/snaps
                    sp = m.constrainedAngularSnap(scp, boost::optional<Geom::Point>(), dr_snap, snaps);
                }
                m.unSetup();
                isr.points.push_back(sp);
            }
        }

        m.setup(desktop, false); // turn of the snap indicator temporarily
        Inkscape::SnappedPoint bsp = m.findBestSnap(scp, isr, true);
        m.unSetup();
        if (!bsp.getSnapped()) {
            // If we didn't truly snap to an object or to a grid, then we will still have to look for the
            // closest projection onto one of the constraints. findBestSnap() will not do this for us
            for (std::list<Inkscape::SnappedPoint>::const_iterator i = isr.points.begin(); i != isr.points.end(); ++i) {
                if (i == isr.points.begin() || (Geom::L2((*i).getPoint() - p) < Geom::L2(bsp.getPoint() - p))) {
                    bsp.setPoint((*i).getPoint());
                    bsp.setTarget(Inkscape::SNAPTARGET_CONSTRAINED_ANGLE);
                }
            }
        }
        //p = isr.points.front().getPoint();
        p = bsp.getPoint();
        knot->moveto(p);
    }

    drag->keep_selection = (bool) g_list_find(drag->selected, dragger);
    bool scale_radial = (state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK);

    if (drag->keep_selection) {
        Geom::Point diff = p - dragger->point;
        drag->selected_move_nowrite (diff[Geom::X], diff[Geom::Y], scale_radial);
    } else {
        Geom::Point p_old = dragger->point;
        dragger->point = p;
        dragger->fireDraggables (false, scale_radial);
        dragger->updateDependencies(false);
        dragger->updateHandles( p_old, MG_NODE_NO_SCALE );
    }
}


static void gr_midpoint_limits(GrDragger *dragger, SPObject *server, Geom::Point *begin, Geom::Point *end, Geom::Point *low_lim, Geom::Point *high_lim, GSList **moving)
{

    GrDrag *drag = dragger->parent;
    // a midpoint dragger can (logically) only contain one GrDraggable
    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;

    // get begin and end points between which dragging is allowed:
    // the draglimits are between knot(lowest_i - 1) and knot(highest_i + 1)
    *moving = g_slist_append(*moving, dragger);

    guint lowest_i = draggable->point_i;
    guint highest_i = draggable->point_i;
    GrDragger *lowest_dragger = dragger;
    GrDragger *highest_dragger = dragger;
    if (dragger->isSelected()) {
        GrDragger* d_add;
        while ( true )
        {
            d_add = drag->getDraggerFor(draggable->item, draggable->point_type, lowest_i - 1, draggable->fill_or_stroke);
            if ( d_add && g_list_find(drag->selected, d_add) ) {
                lowest_i = lowest_i - 1;
                *moving = g_slist_prepend(*moving, d_add);
                lowest_dragger = d_add;
            } else {
                break;
            }
        }

        while ( true )
        {
            d_add = drag->getDraggerFor(draggable->item, draggable->point_type, highest_i + 1, draggable->fill_or_stroke);
            if ( d_add && g_list_find(drag->selected, d_add) ) {
                highest_i = highest_i + 1;
                *moving = g_slist_append(*moving, d_add);
                highest_dragger = d_add;
            } else {
                break;
            }
        }
    }

    if ( SP_IS_LINEARGRADIENT(server) ) {
        guint num = SP_LINEARGRADIENT(server)->vector.stops.size();
        GrDragger *d_temp;
        if (lowest_i == 1) {
            d_temp = drag->getDraggerFor (draggable->item, POINT_LG_BEGIN, 0, draggable->fill_or_stroke);
        } else {
            d_temp = drag->getDraggerFor (draggable->item, POINT_LG_MID, lowest_i - 1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *begin = d_temp->point;

        d_temp = drag->getDraggerFor (draggable->item, POINT_LG_MID, highest_i + 1, draggable->fill_or_stroke);
        if (d_temp == NULL) {
            d_temp = drag->getDraggerFor (draggable->item, POINT_LG_END, num-1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *end = d_temp->point;
    } else if ( SP_IS_RADIALGRADIENT(server) ) {
        guint num = SP_RADIALGRADIENT(server)->vector.stops.size();
        GrDragger *d_temp;
        if (lowest_i == 1) {
            d_temp = drag->getDraggerFor (draggable->item, POINT_RG_CENTER, 0, draggable->fill_or_stroke);
        } else {
            d_temp = drag->getDraggerFor (draggable->item, draggable->point_type, lowest_i - 1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *begin = d_temp->point;

        d_temp = drag->getDraggerFor (draggable->item, draggable->point_type, highest_i + 1, draggable->fill_or_stroke);
        if (d_temp == NULL) {
            d_temp = drag->getDraggerFor (draggable->item, (draggable->point_type==POINT_RG_MID1) ? POINT_RG_R1 : POINT_RG_R2, num-1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *end = d_temp->point;
    }

    *low_lim  = dragger->point - (lowest_dragger->point - *begin);
    *high_lim = dragger->point - (highest_dragger->point - *end);
}

/**
 * Called when a midpoint knot is dragged.
 */
static void gr_knot_moved_midpoint_handler(SPKnot */*knot*/, Geom::Point const &ppointer, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;
    GrDrag *drag = dragger->parent;
    // a midpoint dragger can (logically) only contain one GrDraggable
    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;

    // FIXME: take from prefs
    double snap_fraction = 0.1;

    Geom::Point p = ppointer;
    Geom::Point begin(0,0), end(0,0);
    Geom::Point low_lim(0,0), high_lim(0,0);

    SPObject *server = draggable->getServer();

    GSList *moving = NULL;
    gr_midpoint_limits(dragger, server, &begin, &end, &low_lim, &high_lim, &moving);

    if (state & GDK_CONTROL_MASK) {
        Geom::LineSegment ls(low_lim, high_lim);
        p = ls.pointAt(round(ls.nearestPoint(p) / snap_fraction) * snap_fraction);
    } else {
        Geom::LineSegment ls(low_lim, high_lim);
        p = ls.pointAt(ls.nearestPoint(p));
        if (!(state & GDK_SHIFT_MASK)) {
            Inkscape::Snapper::SnapConstraint cl(low_lim, high_lim - low_lim);
            SPDesktop *desktop = dragger->parent->desktop;
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);
            m.constrainedSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE, cl);
            m.unSetup();
        }
    }
    Geom::Point displacement = p - dragger->point;

    for (GSList const* i = moving; i != NULL; i = i->next) {
        GrDragger *drg = (GrDragger*) i->data;
        SPKnot *drgknot = drg->knot;
        Geom::Point this_move = displacement;
        if (state & GDK_MOD1_MASK) {
            // FIXME: unify all these profiles (here, in nodepath, in tweak) in one place
            double alpha = 1.0;
            if (Geom::L2(drg->point - dragger->point) + Geom::L2(drg->point - begin) - 1e-3 > Geom::L2(dragger->point - begin)) { // drg is on the end side from dragger
                double x = Geom::L2(drg->point - dragger->point)/Geom::L2(end - dragger->point);
                this_move = (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5) * this_move;
            } else { // drg is on the begin side from dragger
                double x = Geom::L2(drg->point - dragger->point)/Geom::L2(begin - dragger->point);
                this_move = (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5) * this_move;
            }
        }
        drg->point += this_move;
        drgknot->moveto(drg->point);
        drg->fireDraggables (false);
        drg->updateDependencies(false);
    }

    g_slist_free(moving);

    drag->keep_selection = dragger->isSelected();
}



static void gr_knot_grabbed_handler(SPKnot */*knot*/, unsigned int /*state*/, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->parent->desktop->canvas->forceFullRedrawAfterInterruptions(5);
}

/**
 * Called when the mouse releases a dragger knot; changes gradient writing to repr, updates other draggers if needed.
 */
static void gr_knot_ungrabbed_handler(SPKnot *knot, unsigned int state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->parent->desktop->canvas->endForcedFullRedraws();

    dragger->point_original = dragger->point = knot->pos;

    if ((state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK)) {
        dragger->fireDraggables (true, true);
    } else {
        dragger->fireDraggables (true);
    }
    dragger->updateHandles( dragger->point_original, MG_NODE_NO_SCALE );

    for (GList *i = dragger->parent->selected; i != NULL; i = i->next) {
        GrDragger *d = (GrDragger *) i->data;
        if (d == dragger)
            continue;
        d->fireDraggables (true);
    }

    // make this dragger selected
    if (!dragger->parent->keep_selection) {
        dragger->parent->setSelected (dragger);
    }
    dragger->parent->keep_selection = false;

    dragger->updateDependencies(true);

    // we did an undoable action
    DocumentUndo::done(sp_desktop_document (dragger->parent->desktop), SP_VERB_CONTEXT_GRADIENT,
                       _("Move gradient handle"));
}

/**
 * Called when a dragger knot is clicked; selects the dragger or deletes it depending on the
 * state of the keyboard keys.
 */
static void gr_knot_clicked_handler(SPKnot */*knot*/, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;
    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;
    if (!draggable) return;

    if ( (state & GDK_CONTROL_MASK) && (state & GDK_MOD1_MASK ) ) {
    // delete this knot from vector
        SPGradient *gradient = getGradient(draggable->item, draggable->fill_or_stroke);
        gradient = gradient->getVector();
        if (gradient->vector.stops.size() > 2) { // 2 is the minimum
            SPStop *stop = NULL;
            switch (draggable->point_type) {  // if we delete first or last stop, move the next/previous to the edge

                case POINT_LG_BEGIN:
                case POINT_RG_CENTER:
                    stop = gradient->getFirstStop();
                    {
                        SPStop *next = stop->getNextStop();
                        if (next) {
                            next->offset = 0;
                            sp_repr_set_css_double(next->getRepr(), "offset", 0);
                        }
                    }
                    break;

                case POINT_LG_END:
                case POINT_RG_R1:
                case POINT_RG_R2:
                    stop = sp_last_stop(gradient);
                    {
                        SPStop *prev = stop->getPrevStop();
                        if (prev) {
                            prev->offset = 1;
                            sp_repr_set_css_double(prev->getRepr(), "offset", 1);
                        }
                    }
                    break;

                case POINT_LG_MID:
                case POINT_RG_MID1:
                case POINT_RG_MID2:
                    stop = sp_get_stop_i(gradient, draggable->point_i);
                    break;

                default:
                    break;

            }

            gradient->getRepr()->removeChild(stop->getRepr());
            DocumentUndo::done(gradient->document, SP_VERB_CONTEXT_GRADIENT,
                               _("Delete gradient stop"));
        }
    } else {
    // select the dragger

        dragger->point_original = dragger->point;

        if ( state & GDK_SHIFT_MASK ) {
            dragger->parent->setSelected (dragger, true, false);
        } else {
            dragger->parent->setSelected (dragger);
        }
    }
}

/**
 * Called when a dragger knot is doubleclicked;
 */
static void gr_knot_doubleclicked_handler(SPKnot */*knot*/, guint /*state*/, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->point_original = dragger->point;

    if (dragger->draggables == NULL)
        return;

    /*
     *  2012/4 - Do nothing (gradient editor to be disabled)
     */
    //GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;
    //sp_item_gradient_edit_stop (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
}

/**
 * Act upon all draggables of the dragger, setting them to the dragger's point.
 */
void GrDragger::fireDraggables(bool write_repr, bool scale_radial, bool merging_focus)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;

        // set local_change flag so that selection_changed callback does not regenerate draggers
        this->parent->local_change = true;

        // change gradient, optionally writing to repr; prevent focus from moving if it's snapped
        // to the center, unless it's the first update upon merge when we must snap it to the point
        if (merging_focus ||
            !(draggable->point_type == POINT_RG_FOCUS && this->isA(draggable->item, POINT_RG_CENTER, draggable->point_i, draggable->fill_or_stroke)))
        {
            sp_item_gradient_set_coords (draggable->item, draggable->point_type, draggable->point_i, this->point, draggable->fill_or_stroke, write_repr, scale_radial);
        }
    }
}

/**
 * Checks if the dragger has a draggable with this point_type.
 */
bool GrDragger::isA(GrPointType point_type)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = reinterpret_cast<GrDraggable *>(i->data);
        if (draggable->point_type == point_type) {
            return true;
        }
    }
    return false;
}

/**
 * Checks if the dragger has a draggable with this item, point_type + point_i (number), fill_or_stroke.
 */
bool GrDragger::isA(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if ( (draggable->point_type == point_type) && (draggable->point_i == point_i) && (draggable->item == item) && (draggable->fill_or_stroke == fill_or_stroke) ) {
            return true;
        }
    }
    return false;
}

/**
 * Checks if the dragger has a draggable with this item, point_type, fill_or_stroke.
 */
bool GrDragger::isA(SPItem *item, GrPointType point_type, Inkscape::PaintTarget fill_or_stroke)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if ( (draggable->point_type == point_type) && (draggable->item == item) && (draggable->fill_or_stroke == fill_or_stroke) ) {
            return true;
        }
    }
    return false;
}

bool GrDraggable::mayMerge(GrDraggable *da2)
{
    if ((this->item == da2->item) && (this->fill_or_stroke == da2->fill_or_stroke)) {
        // we must not merge the points of the same gradient!
        if (!((this->point_type == POINT_RG_FOCUS && da2->point_type == POINT_RG_CENTER) ||
              (this->point_type == POINT_RG_CENTER && da2->point_type == POINT_RG_FOCUS))) {
            // except that we can snap center and focus together
            return false;
        }
    }
    // disable merging of midpoints.
    if ( (this->point_type == POINT_LG_MID) || (da2->point_type == POINT_LG_MID)
         || (this->point_type == POINT_RG_MID1) || (da2->point_type == POINT_RG_MID1)
         || (this->point_type == POINT_RG_MID2) || (da2->point_type == POINT_RG_MID2) )
        return false;

    return true;
}

bool GrDragger::mayMerge(GrDragger *other)
{
    if (this == other)
        return false;

    for (GSList const* i = this->draggables; i != NULL; i = i->next) { // for all draggables of this
        GrDraggable *da1 = (GrDraggable *) i->data;
        for (GSList const* j = other->draggables; j != NULL; j = j->next) { // for all draggables of other
            GrDraggable *da2 = (GrDraggable *) j->data;
            if (!da1->mayMerge(da2))
                return false;
        }
    }
    return true;
}

bool GrDragger::mayMerge(GrDraggable *da2)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) { // for all draggables of this
        GrDraggable *da1 = (GrDraggable *) i->data;
        if (!da1->mayMerge(da2))
            return false;
    }
    return true;
}

/**
 * Update mesh handles when mesh corner is moved.
 * pc_old: old position of corner (could be changed to dp if we figure out transforms).
 * op: how other nodes (handles, tensors) should be moved.
 * Scaling takes place only between a selected and an unselected corner,
 * other wise a handle is displaced the same distance as the adjacent corner.
 * If a side is a line, then the handles are always placed 1/3 of side length
 * from each corner.
 *
 * Ooops, needs to be reimplemented.
 */
void
GrDragger::updateHandles ( Geom::Point pc_old,  MeshNodeOperation op )
{

    // This routine might more properly be in mesh-context.cpp but moving knots is
    // handled here rather than there.

    // We need to update two places:
    //  1. In SPMeshArrayI with object coordinates
    //  2. In Drager/Knots with desktop coordinates.

    // This routine is more complicated than it might need to be inorder to allow
    // corner points to be selected in multiple meshes at the same time... with some
    // sharing the same dragger (overkill, perhaps?).

    // If no corner point in GrDragger then do nothing.
    if( !isA (POINT_MG_CORNER ) ) return;

    GrDrag *drag = this->parent;

    // We need a list of selected corners per mesh if scaling.
    std::map<SPGradient*, std::vector<guint> > selected_corners;
    bool scale = false;
    if( scale == true ) {

        for ( GList *i = drag->selected; i != NULL; i = i->next ) {
            GrDragger *dragger = (GrDragger *) i->data;
            for ( GSList *j = dragger->draggables; j != NULL; j = j->next ) {
                GrDraggable *draggable = (GrDraggable *) j->data;

                // Check draggable is of type POINT_MG_CORNER (don't allow selection of POINT_MG_HANDLE)
                if( draggable->point_type != POINT_MG_CORNER ) continue;

                // Must be a mesh gradient
                SPGradient *gradient = getGradient(draggable->item, draggable->fill_or_stroke);
                if ( !SP_IS_MESHGRADIENT( gradient ) ) continue;

                selected_corners[ gradient ].push_back( draggable->point_i );
            }
        }
    }

    // Now we do the handle moves.

    // Loop over all draggables in moved corner
    std::map<SPGradient*, std::vector<guint> > dragger_corners;
    for ( GSList *j = draggables; j != NULL; j = j->next ) {
        GrDraggable *draggable = (GrDraggable *) j->data;

        SPItem *item           = draggable->item;
        gint    point_type     = draggable->point_type;
        gint    point_i        = draggable->point_i;
        Inkscape::PaintTarget
                fill_or_stroke = draggable->fill_or_stroke;

        // Check draggable is of type POINT_MG_CORNER (don't allow selection of POINT_MG_HANDLE)
        if( point_type != POINT_MG_CORNER ) continue;

        // Must be a mesh gradient
        SPGradient *gradient = getGradient(item, fill_or_stroke);
        if ( !SP_IS_MESHGRADIENT( gradient ) ) continue;
        SPMeshGradient *mg = SP_MESHGRADIENT( gradient );

        // pc_old is the old corner position in desktop coordinates, we need it in gradient coordinate.
        gradient = sp_gradient_convert_to_userspace (gradient, item, (fill_or_stroke == Inkscape::FOR_FILL) ? "fill" : "stroke");
        Geom::Affine i2d ( item->i2dt_affine() );
        Geom::Point pcg_old = pc_old * i2d.inverse();
        pcg_old *= (gradient->gradientTransform).inverse();

        mg->array.update_handles( point_i, selected_corners[ gradient ], pcg_old, op );

        // Move on-screen knots
        for( guint i = 0; i < mg->array.handles.size(); ++i ) {
             GrDragger *handle = drag->getDraggerFor( item, POINT_MG_HANDLE, i, fill_or_stroke ); 
            SPKnot *knot = handle->knot;
            Geom::Point pk = getGradientCoords( item, POINT_MG_HANDLE, i, fill_or_stroke );
            knot->moveto(pk);

        }

        for( guint i = 0; i < mg->array.tensors.size(); ++i ) {

            GrDragger *handle = drag->getDraggerFor( item, POINT_MG_TENSOR, i, fill_or_stroke ); 
            SPKnot *knot = handle->knot;
            Geom::Point pk = getGradientCoords( item, POINT_MG_TENSOR, i, fill_or_stroke );
            knot->moveto(pk);

        }

    } // Loop over draggables.
}


/**
 * Updates the statusbar tip of the dragger knot, based on its draggables.
 */
void GrDragger::updateTip()
{
    if (this->knot && this->knot->tip) {
        g_free (this->knot->tip);
        this->knot->tip = NULL;
    }

    if (g_slist_length (this->draggables) == 1) {
        GrDraggable *draggable = (GrDraggable *) this->draggables->data;
        char *item_desc = draggable->item->detailedDescription();
        switch (draggable->point_type) {
            case POINT_LG_MID:
            case POINT_RG_MID1:
            case POINT_RG_MID2:
                this->knot->tip = g_strdup_printf (_("%s %d for: %s%s; drag with <b>Ctrl</b> to snap offset; click with <b>Ctrl+Alt</b> to delete stop"),
                                                   _(gr_knot_descr[draggable->point_type]),
                                                   draggable->point_i,
                                                   item_desc,
                                                   (draggable->fill_or_stroke == Inkscape::FOR_STROKE) ? _(" (stroke)") : "");
                break;

            default:
                this->knot->tip = g_strdup_printf (_("%s for: %s%s; drag with <b>Ctrl</b> to snap angle, with <b>Ctrl+Alt</b> to preserve angle, with <b>Ctrl+Shift</b> to scale around center"),
                                                   _(gr_knot_descr[draggable->point_type]),
                                                   item_desc,
                                                   (draggable->fill_or_stroke == Inkscape::FOR_STROKE) ? _(" (stroke)") : "");
                break;
        }
        g_free(item_desc);
    } else if (g_slist_length (draggables) == 2 && isA (POINT_RG_CENTER) && isA (POINT_RG_FOCUS)) {
        this->knot->tip = g_strdup_printf ("%s", _("Radial gradient <b>center</b> and <b>focus</b>; drag with <b>Shift</b> to separate focus"));
    } else {
        int length = g_slist_length (this->draggables);
        this->knot->tip = g_strdup_printf (ngettext("Gradient point shared by <b>%d</b> gradient; drag with <b>Shift</b> to separate",
                                                    "Gradient point shared by <b>%d</b> gradients; drag with <b>Shift</b> to separate",
                                                    length),
                                           length);
    }
}

/**
 * Adds a draggable to the dragger.
 */
void GrDragger::updateKnotShape()
{
    if (!draggables)
        return;
    GrDraggable *last = (GrDraggable *) g_slist_last(draggables)->data;
    g_object_set (G_OBJECT (this->knot->item), "shape", gr_knot_shapes[last->point_type], NULL);
}

/**
 * Adds a draggable to the dragger.
 */
void GrDragger::addDraggable(GrDraggable *draggable)
{
    this->draggables = g_slist_prepend (this->draggables, draggable);

    this->updateTip();
}


/**
 * Moves this dragger to the point of the given draggable, acting upon all other draggables.
 */
void GrDragger::moveThisToDraggable(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke, bool write_repr)
{
    GrDraggable *dr_first = reinterpret_cast<GrDraggable *>(draggables->data);
    if (!dr_first) {
        return;
    }

    this->point = getGradientCoords(dr_first->item, dr_first->point_type, dr_first->point_i, dr_first->fill_or_stroke);
    this->point_original = this->point;

    this->knot->moveto(this->point);

    for (GSList const* i = draggables; i != NULL; i = i->next) {
        GrDraggable *da = (GrDraggable *) i->data;
        if ( (da->item == item) &&
             (point_type == -1 || da->point_type == point_type) &&
             (point_i == -1 || da->point_i == point_i) &&
             (da->fill_or_stroke == fill_or_stroke) ) {
            // Don't move initial draggable
            continue;
        }
        sp_item_gradient_set_coords(da->item, da->point_type, da->point_i, this->point, da->fill_or_stroke, write_repr, false);
    }
    // FIXME: here we should also call this->updateDependencies(write_repr); to propagate updating, but how to prevent loops?
}


/**
 * Moves all midstop draggables that depend on this one.
 */
void GrDragger::updateMidstopDependencies(GrDraggable *draggable, bool write_repr)
{
    SPObject *server = draggable->getServer();
    if (!server)
        return;
    guint num = SP_GRADIENT(server)->vector.stops.size();
    if (num <= 2) return;

    if ( SP_IS_LINEARGRADIENT(server) ) {
        for ( guint i = 1; i < num - 1; i++ ) {
            this->moveOtherToDraggable (draggable->item, POINT_LG_MID, i, draggable->fill_or_stroke, write_repr);
        }
    } else  if ( SP_IS_RADIALGRADIENT(server) ) {
        for ( guint i = 1; i < num - 1; i++ ) {
            this->moveOtherToDraggable (draggable->item, POINT_RG_MID1, i, draggable->fill_or_stroke, write_repr);
            this->moveOtherToDraggable (draggable->item, POINT_RG_MID2, i, draggable->fill_or_stroke, write_repr);
        }
    }
}


/**
 * Moves all draggables that depend on this one.
 */
void GrDragger::updateDependencies(bool write_repr)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        switch (draggable->point_type) {
            case POINT_LG_BEGIN:
                {
                    // the end point is dependent only when dragging with ctrl+shift
                    this->moveOtherToDraggable (draggable->item, POINT_LG_END, -1, draggable->fill_or_stroke, write_repr);

                    this->updateMidstopDependencies (draggable, write_repr);
                }
                break;
            case POINT_LG_END:
                {
                    // the begin point is dependent only when dragging with ctrl+shift
                    this->moveOtherToDraggable (draggable->item, POINT_LG_BEGIN, 0, draggable->fill_or_stroke, write_repr);

                    this->updateMidstopDependencies (draggable, write_repr);
                }
                break;
            case POINT_LG_MID:
                // no other nodes depend on mid points.
                break;
            case POINT_RG_R2:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R1, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, -1, draggable->fill_or_stroke, write_repr);
                this->updateMidstopDependencies (draggable, write_repr);
                break;
            case POINT_RG_R1:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R2, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, -1, draggable->fill_or_stroke, write_repr);
                this->updateMidstopDependencies (draggable, write_repr);
                break;
            case POINT_RG_CENTER:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R1, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_R2, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, -1, draggable->fill_or_stroke, write_repr);
                this->updateMidstopDependencies (draggable, write_repr);
                break;
            case POINT_RG_FOCUS:
                // nothing can depend on that
                break;
            case POINT_RG_MID1:
                this->moveOtherToDraggable (draggable->item, POINT_RG_MID2, draggable->point_i, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_RG_MID2:
                this->moveOtherToDraggable (draggable->item, POINT_RG_MID1, draggable->point_i, draggable->fill_or_stroke, write_repr);
                break;
            default:
                break;
        }
    }
}



GrDragger::GrDragger(GrDrag *parent, Geom::Point p, GrDraggable *draggable)
  : point(p),
    point_original(p)
{
    this->draggables = NULL;

    this->parent = parent;

    // create the knot
    this->knot = new SPKnot(parent->desktop, NULL);
    this->knot->setMode(SP_KNOT_MODE_XOR);
    this->knot->setFill(GR_KNOT_COLOR_NORMAL, GR_KNOT_COLOR_MOUSEOVER, GR_KNOT_COLOR_MOUSEOVER);
    this->knot->setStroke(0x0000007f, 0x0000007f, 0x0000007f);
    this->knot->updateCtrl();

    // move knot to the given point
    this->knot->setPosition(p, SP_KNOT_STATE_NORMAL);
    this->knot->show();

    // connect knot's signals
    if ( (draggable)  // it can be NULL if a node in unsnapped (eg. focus point unsnapped from center)
                       // luckily, midstops never snap to other nodes so are never unsnapped...
         && ( (draggable->point_type == POINT_LG_MID)
              || (draggable->point_type == POINT_RG_MID1)
              || (draggable->point_type == POINT_RG_MID2) ) )
    {
        this->_moved_connection = this->knot->moved_signal.connect(sigc::bind(sigc::ptr_fun(gr_knot_moved_midpoint_handler), this));
    } else {
        this->_moved_connection = this->knot->moved_signal.connect(sigc::bind(sigc::ptr_fun(gr_knot_moved_handler), this));
    }

    this->_clicked_connection = this->knot->click_signal.connect(sigc::bind(sigc::ptr_fun(gr_knot_clicked_handler), this));
    this->_doubleclicked_connection = this->knot->doubleclicked_signal.connect(sigc::bind(sigc::ptr_fun(gr_knot_doubleclicked_handler), this));
    this->_grabbed_connection = this->knot->grabbed_signal.connect(sigc::bind(sigc::ptr_fun(gr_knot_grabbed_handler), this));
    this->_ungrabbed_connection = this->knot->ungrabbed_signal.connect(sigc::bind(sigc::ptr_fun(gr_knot_ungrabbed_handler), this));

    // add the initial draggable
    if (draggable) {
        this->addDraggable (draggable);
    }

    updateKnotShape();
}

GrDragger::~GrDragger()
{
    // unselect if it was selected
    // Hmm, this causes a race condition as it triggers a call to gradient_selection_changed which
    // can be executed while a list of draggers is being deleted. It doesn't acutally seem to be
    // necessary.
    //this->parent->setDeselected(this);

    // disconnect signals
    this->_moved_connection.disconnect();
    this->_clicked_connection.disconnect();
    this->_doubleclicked_connection.disconnect();
    this->_grabbed_connection.disconnect();
    this->_ungrabbed_connection.disconnect();

    /* unref should call destroy */
    knot_unref(this->knot);

    // delete all draggables
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        delete ((GrDraggable *) i->data);
    }

    g_slist_free (this->draggables);
    this->draggables = NULL;
}

/**
 * Select the dragger which has the given draggable.
 */
GrDragger *GrDrag::getDraggerFor(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke)
{
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {
            GrDraggable *da2 = (GrDraggable *) j->data;
            if ( (da2->item == item) &&
                 (point_type == -1 || da2->point_type == point_type) && // -1 means this does not matter
                 (point_i == -1 || da2->point_i == point_i) && // -1 means this does not matter
                 (da2->fill_or_stroke == fill_or_stroke)) {
                return (dragger);
            }
        }
    }
    return NULL;
}


void GrDragger::moveOtherToDraggable(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke, bool write_repr)
{
    GrDragger *d = this->parent->getDraggerFor(item, point_type, point_i, fill_or_stroke);
    if (d && d !=  this) {
        d->moveThisToDraggable(item, point_type, point_i, fill_or_stroke, write_repr);
    }
}


/**
 * Draw this dragger as selected.
 */
void GrDragger::select()
{
    this->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_SELECTED;
    g_object_set (G_OBJECT (this->knot->item), "fill_color", GR_KNOT_COLOR_SELECTED, NULL);
    //if( isA(POINT_MG_CORNER) ) {
    //    for (GSList * drgble = this->draggables; drgble != NULL; drgble = drgble->next) {
    //        GrDraggable *draggable = (GrDraggable*) drgble->data;
    //        //if( draggable != NULL ) std::cout << "   draggable" << std::endl;
    //        // MESH FIXME: TURN ON CORRESPONDING SIDE/TENSOR NODE VISIBILITY
    //    }
    //}
}

/**
 * Draw this dragger as normal (deselected).
 */
void GrDragger::deselect()
{
    this->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;
    g_object_set (G_OBJECT (this->knot->item), "fill_color", GR_KNOT_COLOR_NORMAL, NULL);
            // MESH FIXME: TURN OFF CORRESPONDING SIDE/TENSOR NODE VISIBILITY
}

bool
GrDragger::isSelected()
{
    return g_list_find (parent->selected, this);
}

/**
 * Deselect all stops/draggers (private).
 */
void GrDrag::deselect_all()
{
    while (selected) {
        ( (GrDragger*) selected->data)->deselect();
        selected = g_list_remove(selected, selected->data);
    }
}

/**
 * Deselect all stops/draggers (public; emits signal).
 */
void GrDrag::deselectAll()
{
    deselect_all();
    this->desktop->emitToolSubselectionChanged(NULL);
}

/**
 * Select all stops/draggers.
 */
void GrDrag::selectAll()
{
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *d = ((GrDragger *) l->data);
        setSelected (d, true, true);
    }
}

/**
 * Select all stops/draggers that match the coords.
 */
void GrDrag::selectByCoords(std::vector<Geom::Point> coords)
{
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *d = ((GrDragger *) l->data);
        for (guint k = 0; k < coords.size(); k++) {
            if (Geom::L2 (d->point - coords[k]) < 1e-4) {
                setSelected (d, true, true);
            }
        }
    }
}

/**
 * Select draggers by stop
 */
void GrDrag::selectByStop(SPStop *stop, bool add_to_selection, bool override )
{
    for (GList *i = this->draggers; i != NULL; i = i->next) {

        GrDragger *dragger = (GrDragger *) i->data;
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {

            GrDraggable *d = (GrDraggable *) j->data;
            SPGradient *gradient = getGradient(d->item, d->fill_or_stroke);
            SPGradient *vector = gradient->getVector(false);
            SPStop *stop_i = sp_get_stop_i(vector, d->point_i);

            if (stop_i == stop) {
                setSelected(dragger, add_to_selection, override);
            }
        }
    }
}
/**
 * Select all stops/draggers that fall within the rect.
 */
void GrDrag::selectRect(Geom::Rect const &r)
{
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *d = ((GrDragger *) l->data);
        if (r.contains(d->point)) {
           setSelected (d, true, true);
        }
    }
}

/**
 * Select a dragger.
 * @param dragger       The dragger to select.
 * @param add_to_selection   If true, add to selection, otherwise deselect others.
 * @param override      If true, always select this node, otherwise toggle selected status.
*/
void GrDrag::setSelected(GrDragger *dragger, bool add_to_selection, bool override)
{
    GrDragger *seldragger = NULL;

    // Don't allow selecting a mesh handle or mesh tensor.
    // We might want to rethink since a dragger can have draggables of different types.
    if ( dragger->isA( POINT_MG_HANDLE ) || dragger->isA( POINT_MG_TENSOR ) ) return;

    if (add_to_selection) {
        if (!dragger) return;
        if (override) {
            if (!g_list_find(selected, dragger)) {
                selected = g_list_prepend(selected, dragger);
            }
            dragger->select();
            seldragger = dragger;
        } else { // toggle
            if (g_list_find(selected, dragger)) {
                selected = g_list_remove(selected, dragger);
                dragger->deselect();
                if (selected) {
                    seldragger = (GrDragger*) selected->data; // select the dragger that is first in the list
                }
            } else {
                selected = g_list_prepend(selected, dragger);
                dragger->select();
                seldragger = dragger;
            }
        }
    } else {
        deselect_all();
        if (dragger) {
            selected = g_list_prepend(selected, dragger);
            dragger->select();
            seldragger = dragger;
        }
    }
    if (seldragger) {
        this->desktop->emitToolSubselectionChanged((gpointer) seldragger);
    }
}

/**
 * Deselect a dragger.
 * @param dragger       The dragger to deselect.
 */
void GrDrag::setDeselected(GrDragger *dragger)
{
    if (g_list_find(selected, dragger)) {
        selected = g_list_remove(selected, dragger);
        dragger->deselect();
    }
    this->desktop->emitToolSubselectionChanged((gpointer) (selected ? selected->data : NULL ));
}



/**
 * Create a line from p1 to p2 and add it to the lines list.
 */
void GrDrag::addLine(SPItem *item, Geom::Point p1, Geom::Point p2, Inkscape::PaintTarget fill_or_stroke)
{
    CtrlLineType type = (fill_or_stroke == Inkscape::FOR_FILL) ? CTLINE_PRIMARY : CTLINE_SECONDARY;
    SPCtrlLine *line = ControlManager::getManager().createControlLine(sp_desktop_controls(this->desktop), p1, p2, type);

    sp_canvas_item_move_to_z(line, 0);
    line->item = item;
    sp_canvas_item_show(line);
    this->lines = g_slist_append(this->lines, line);
}



/**
 * Create a curve from p0 to p3 and add it to the lines list. Used for mesh sides.
 */
void GrDrag::addCurve(SPItem *item, Geom::Point p0, Geom::Point p1, Geom::Point p2, Geom::Point p3, Inkscape::PaintTarget fill_or_stroke)
{
    CtrlLineType type = (fill_or_stroke == Inkscape::FOR_FILL) ? CTLINE_PRIMARY : CTLINE_SECONDARY;
    SPCtrlCurve *line = ControlManager::getManager().createControlCurve(sp_desktop_controls(this->desktop), p0, p1, p2, p3, type); 

    sp_canvas_item_move_to_z(line, 0);
    line->item = item;
    sp_canvas_item_show (line);
    this->lines = g_slist_append (this->lines, line);
}


/**
 * If there already exists a dragger within MERGE_DIST of p, add the draggable to it; otherwise create
 * new dragger and add it to draggers list.
 */
void GrDrag::addDragger(GrDraggable *draggable)
{
    Geom::Point p = getGradientCoords(draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);

    for (GList *i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        if (dragger->mayMerge (draggable) && Geom::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addDraggable (draggable);
            dragger->updateKnotShape();
            return;
        }
    }

    GrDragger *new_dragger = new GrDragger(this, p, draggable);
    // fixme: draggers should be added AFTER the last one: this way tabbing through them will be from begin to end.
    this->draggers = g_list_append (this->draggers, new_dragger);
}

/**
 * Add draggers for the radial gradient rg on item.
 */
void GrDrag::addDraggersRadial(SPRadialGradient *rg, SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
    rg->ensureVector();
    addDragger (new GrDraggable (item, POINT_RG_CENTER, 0, fill_or_stroke));
    guint num = rg->vector.stops.size();
    if (num > 2) {
        for ( guint i = 1; i < num - 1; i++ ) {
            addDragger (new GrDraggable (item, POINT_RG_MID1, i, fill_or_stroke));
        }
    }
    addDragger (new GrDraggable (item, POINT_RG_R1, num-1, fill_or_stroke));
    if (num > 2) {
        for ( guint i = 1; i < num - 1; i++ ) {
            addDragger (new GrDraggable (item, POINT_RG_MID2, i, fill_or_stroke));
        }
    }
    addDragger (new GrDraggable (item, POINT_RG_R2, num - 1, fill_or_stroke));
    addDragger (new GrDraggable (item, POINT_RG_FOCUS, 0, fill_or_stroke));
}

/**
 * Add draggers for the linear gradient lg on item.
 */
void GrDrag::addDraggersLinear(SPLinearGradient *lg, SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
    lg->ensureVector();
    addDragger(new GrDraggable (item, POINT_LG_BEGIN, 0, fill_or_stroke));
    guint num = lg->vector.stops.size();
    if (num > 2) {
        for ( guint i = 1; i < num - 1; i++ ) {
            addDragger(new GrDraggable (item, POINT_LG_MID, i, fill_or_stroke));
        }
    }
    addDragger(new GrDraggable (item, POINT_LG_END, num - 1, fill_or_stroke));
}

/**
 *Add draggers for the mesh gradient mg on item
 */
void GrDrag::addDraggersMesh(SPMeshGradient *mg, SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
    std::vector< std::vector< SPMeshNode* > > nodes = mg->array.nodes;

    // Show/hide mesh on fill/stroke. This doesn't work at the moment... and prevents node color updating.
    
    //Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool edit_fill    = true; //abs(prefs->getBool("/tools/mesh/edit_fill",    true));
    bool edit_stroke  = true; //abs(prefs->getBool("/tools/mesh/edit_stroke",  true));
    bool show_handles = true; //abs(prefs->getBool("/tools/mesh/show_handles", true));

    if( (fill_or_stroke == Inkscape::FOR_FILL   && !edit_fill) ||
        (fill_or_stroke == Inkscape::FOR_STROKE && !edit_stroke) ) {
        return;
    }

    // Make sure we have at least one patch defined.
    if( mg->array.patch_rows() == 0 || mg->array.patch_columns() == 0 ) {

        std::cout << "Empty Mesh Gradient, No Draggers to Add" << std::endl;
        return;
    }

    guint icorner = 0;
    guint ihandle = 0;
    guint itensor = 0;
    mg->array.corners.clear();
    mg->array.handles.clear();
    mg->array.tensors.clear();


    for( guint i = 0; i < nodes.size(); ++i ) {
        for( guint j = 0; j < nodes[i].size(); ++j ) {

            // std::cout << " Draggers: " << i << " " << j << " " << nodes[i][j]->node_type << std::endl;

            if( nodes[i][j]->set ) {
                switch ( nodes[i][j]->node_type ) {

                    case MG_NODE_TYPE_CORNER:
                    {
                        mg->array.corners.push_back( nodes[i][j] );
                        GrDraggable *corner = new GrDraggable (item, POINT_MG_CORNER, icorner, fill_or_stroke);
                        addDragger ( corner );
                        nodes[i][j]->draggable = icorner;
                        ++icorner;
                        break;
                    }

                    case MG_NODE_TYPE_HANDLE:
                    {
                        if( show_handles ) {
                            mg->array.handles.push_back( nodes[i][j] );
                            GrDraggable *handle = new GrDraggable (item, POINT_MG_HANDLE, ihandle, fill_or_stroke);
                            addDragger ( handle );
                            nodes[i][j]->draggable = ihandle;
                            ++ihandle;
                            break;
                        }
                    }

                    case MG_NODE_TYPE_TENSOR:
                    {
                        if( show_handles ) {
                            mg->array.tensors.push_back( nodes[i][j] );
                            GrDraggable *tensor = new GrDraggable (item, POINT_MG_TENSOR, itensor, fill_or_stroke);
                            addDragger ( tensor );
                            nodes[i][j]->draggable = itensor;
                            ++itensor;
                            break;
                        }
                    }

                    default:
                        std::cout << "Bad Mesh Gradient draggable type" << std::endl;
                        break;
                }
            }
        }
    }

    mg->array.drag_valid = true;
}

/**
 * Artificially grab the knot of this dragger; used by the gradient context.
 * Not used at the moment.
 */
void GrDrag::grabKnot(GrDragger *dragger, gint x, gint y, guint32 etime)
{
    if (dragger) {
        dragger->knot->startDragging(dragger->point, x, y, etime);
    }
}

/**
 * Artificially grab the knot of the dragger with this draggable; used by the gradient context.
 * This allows setting the final point from the end of the drag when creating a new gradient.
 */
void GrDrag::grabKnot(SPItem *item, GrPointType point_type, gint point_i, Inkscape::PaintTarget fill_or_stroke, gint x, gint y, guint32 etime)
{
    GrDragger *dragger = getDraggerFor(item, point_type, point_i, fill_or_stroke);
    if (dragger) {
        dragger->knot->startDragging(dragger->point, x, y, etime);
    }
}

/**
 * Regenerates the draggers list from the current selection; is called when selection is changed or
 * modified, also when a radial dragger needs to update positions of other draggers in the gradient.
 */
void GrDrag::updateDraggers()
{
    while (selected) {
        selected = g_list_remove(selected, selected->data);
    }
    // delete old draggers
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        delete static_cast<GrDragger *>(i->data);
    }
    g_list_free(this->draggers);
    this->draggers = NULL;

    g_return_if_fail(this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        SPStyle *style = item->style;

        if (style && (style->fill.isPaintserver())) {
            SPPaintServer *server = style->getFillPaintServer();
            if ( server && SP_IS_GRADIENT( server ) ) {
                if ( server->isSolid()
                     || (SP_GRADIENT(server)->getVector() && SP_GRADIENT(server)->getVector()->isSolid())) {
                    // Suppress "gradientness" of solid paint
                } else if ( SP_IS_LINEARGRADIENT(server) ) {
                    addDraggersLinear( SP_LINEARGRADIENT(server), item, Inkscape::FOR_FILL );
                } else if ( SP_IS_RADIALGRADIENT(server) ) {
                    addDraggersRadial( SP_RADIALGRADIENT(server), item, Inkscape::FOR_FILL );
                } else if ( SP_IS_MESHGRADIENT(server) ) {
                    addDraggersMesh(   SP_MESHGRADIENT(server),   item, Inkscape::FOR_FILL );
                }
            }
        }

        if (style && (style->stroke.isPaintserver())) {
            SPPaintServer *server = style->getStrokePaintServer();
            if ( server && SP_IS_GRADIENT( server ) ) {
                if ( server->isSolid()
                     || (SP_GRADIENT(server)->getVector() && SP_GRADIENT(server)->getVector()->isSolid())) {
                    // Suppress "gradientness" of solid paint
                } else if ( SP_IS_LINEARGRADIENT(server) ) {
                    addDraggersLinear( SP_LINEARGRADIENT(server), item, Inkscape::FOR_STROKE );
                } else if ( SP_IS_RADIALGRADIENT(server) ) {
                    addDraggersRadial( SP_RADIALGRADIENT(server), item, Inkscape::FOR_STROKE );
                } else if ( SP_IS_MESHGRADIENT(server) ) {
                    addDraggersMesh(   SP_MESHGRADIENT(server),   item, Inkscape::FOR_STROKE );
                }
            }
        }
    }
}


/**
 * Returns true if at least one of the draggers' knots has the mouse hovering above it.
 */
bool GrDrag::mouseOver()
{
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        GrDragger *d = (GrDragger *) i->data;
        if (d->knot && (d->knot->flags & SP_KNOT_MOUSEOVER)) {
            return true;
        }
    }
    return false;
}

/**
 * Regenerates the lines list from the current selection; is called on each move of a dragger, so that
 * lines are always in sync with the actual gradient.
 */
void GrDrag::updateLines()
{
    // delete old lines
    for (GSList const *i = this->lines; i != NULL; i = i->next) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(i->data));
    }
    g_slist_free(this->lines);
    this->lines = NULL;

    g_return_if_fail(this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {

        SPItem *item = SP_ITEM(i->data);

        SPStyle *style = item->style;

        if (style && (style->fill.isPaintserver())) {
            SPPaintServer *server = item->style->getFillPaintServer();
            if ( server && SP_IS_GRADIENT( server ) ) {
                if ( server->isSolid()
                     || (SP_GRADIENT(server)->getVector() && SP_GRADIENT(server)->getVector()->isSolid())) {
                    // Suppress "gradientness" of solid paint
                } else if ( SP_IS_LINEARGRADIENT(server) ) {
                    addLine(item, getGradientCoords(item, POINT_LG_BEGIN, 0, Inkscape::FOR_FILL), getGradientCoords(item, POINT_LG_END, 0, Inkscape::FOR_FILL), Inkscape::FOR_FILL);
                } else if ( SP_IS_RADIALGRADIENT(server) ) {
                    Geom::Point center = getGradientCoords(item, POINT_RG_CENTER, 0, Inkscape::FOR_FILL);
                    addLine(item, center, getGradientCoords(item, POINT_RG_R1, 0, Inkscape::FOR_FILL), Inkscape::FOR_FILL);
                    addLine(item, center, getGradientCoords(item, POINT_RG_R2, 0, Inkscape::FOR_FILL), Inkscape::FOR_FILL);
                } else if ( SP_IS_MESHGRADIENT(server) ) {

                    SPMeshGradient *mg = SP_MESHGRADIENT(server);

                    guint rows    = mg->array.patch_rows();
                    guint columns = mg->array.patch_columns();
                    for ( guint i = 0; i < rows; ++i ) {
                        for ( guint j = 0; j < columns; ++j ) {

                            std::vector<Geom::Point> h;

                            SPMeshPatchI patch( &(mg->array.nodes), i, j );

                            // Top line
                            h = patch.getPointsForSide( 0 );
                            for( guint p = 0; p < 4; ++p ) {
                                h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                            }
                            addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_FILL );

                            // Right line
                            if( j == columns - 1 ) {
                                h = patch.getPointsForSide( 1 );
                                for( guint p = 0; p < 4; ++p ) {
                                    h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                                }
                                addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_FILL );
                            }

                            // Bottom line
                            if( i == rows    - 1 ) {
                                h = patch.getPointsForSide( 2 );
                                for( guint p = 0; p < 4; ++p ) {
                                    h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                                }
                                addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_FILL );
                            }

                            // Left line
                            h = patch.getPointsForSide( 3 );
                            for( guint p = 0; p < 4; ++p ) {
                                h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                            }
                            addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_FILL );
                        }
                    }
                }                        
            }
        }

        if (style && (style->stroke.isPaintserver())) {
            SPPaintServer *server = item->style->getStrokePaintServer();
            if ( server && SP_IS_GRADIENT( server ) ) {
                if ( server->isSolid()
                     || (SP_GRADIENT(server)->getVector() && SP_GRADIENT(server)->getVector()->isSolid())) {
                    // Suppress "gradientness" of solid paint
                } else if ( SP_IS_LINEARGRADIENT(server) ) {
                    addLine(item, getGradientCoords(item, POINT_LG_BEGIN, 0, Inkscape::FOR_STROKE), getGradientCoords(item, POINT_LG_END, 0, Inkscape::FOR_STROKE), Inkscape::FOR_STROKE);
                } else if ( SP_IS_RADIALGRADIENT(server) ) {
                    Geom::Point center = getGradientCoords(item, POINT_RG_CENTER, 0, Inkscape::FOR_STROKE);
                    addLine(item, center, getGradientCoords(item, POINT_RG_R1, 0, Inkscape::FOR_STROKE), Inkscape::FOR_STROKE);
                    addLine(item, center, getGradientCoords(item, POINT_RG_R2, 0, Inkscape::FOR_STROKE), Inkscape::FOR_STROKE);
                } else if ( SP_IS_MESHGRADIENT(server) ) {

                    // MESH FIXME: TURN ROUTINE INTO FUNCTION AND CALL FOR BOTH FILL AND STROKE.
                    SPMeshGradient *mg = SP_MESHGRADIENT(server);

                    guint rows    = mg->array.patch_rows();
                    guint columns = mg->array.patch_columns();
                    for ( guint i = 0; i < rows; ++i ) {
                        for ( guint j = 0; j < columns; ++j ) {

                            std::vector<Geom::Point> h;

                            SPMeshPatchI patch( &(mg->array.nodes), i, j );

                            // Top line
                            h = patch.getPointsForSide( 0 );
                            for( guint p = 0; p < 4; ++p ) {
                                h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                            }
                            addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_STROKE );

                            // Right line
                            if( j == columns - 1 ) {
                                h = patch.getPointsForSide( 1 );
                                for( guint p = 0; p < 4; ++p ) {
                                    h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                                }
                                addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_STROKE );
                            }

                            // Bottom line
                            if( i == rows    - 1 ) {
                                h = patch.getPointsForSide( 2 );
                                for( guint p = 0; p < 4; ++p ) {
                                    h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                                }
                                addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_STROKE );
                            }

                            // Left line
                            h = patch.getPointsForSide( 3 );
                            for( guint p = 0; p < 4; ++p ) {
                                h[p] *= Geom::Affine(mg->gradientTransform) * (Geom::Affine)item->i2dt_affine();
                            }
                            addCurve (item, h[0], h[1], h[2], h[3], Inkscape::FOR_STROKE );
                        }
                    }                        
                }
            }
        }
    }
}

/**
 * Regenerates the levels list from the current selection.
 * Levels correspond to bounding box edges and midpoints.
 */
void GrDrag::updateLevels()
{
    hor_levels.clear();
    vert_levels.clear();

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        Geom::OptRect rect = item->desktopVisualBounds();
        if (rect) {
            // Remember the edges of the bbox and the center axis
            hor_levels.push_back(rect->min()[Geom::Y]);
            hor_levels.push_back(rect->max()[Geom::Y]);
            hor_levels.push_back(rect->midpoint()[Geom::Y]);
            vert_levels.push_back(rect->min()[Geom::X]);
            vert_levels.push_back(rect->max()[Geom::X]);
            vert_levels.push_back(rect->midpoint()[Geom::X]);
        }
    }
}

void GrDrag::selected_reverse_vector()
{
    if (selected == NULL)
        return;

    for (GSList const* i = ( (GrDragger*) selected->data )->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;

        sp_item_gradient_reverse_vector (draggable->item, draggable->fill_or_stroke);
    }
}

void GrDrag::selected_move_nowrite(double x, double y, bool scale_radial)
{
    selected_move (x, y, false, scale_radial);
}

void GrDrag::selected_move(double x, double y, bool write_repr, bool scale_radial)
{
    if (selected == NULL)
        return;

    bool did = false;

    for (GList *i = selected; i != NULL; i = i->next) {
        GrDragger *d = (GrDragger *) i->data;

        if (!d->isA(POINT_LG_MID) && !d->isA(POINT_RG_MID1) && !d->isA(POINT_RG_MID2)) {
            // if this is an endpoint,

            // Moving an rg center moves its focus and radii as well.
            // therefore, if this is a focus or radius and if selection
            // contains the center as well, do not move this one
            if (d->isA(POINT_RG_R1) || d->isA(POINT_RG_R2) ||
                (d->isA(POINT_RG_FOCUS) && !d->isA(POINT_RG_CENTER))) {
                bool skip_radius_with_center = false;
                for (GList *di = selected; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new->isA (((GrDraggable *) d->draggables->data)->item,
                                    POINT_RG_CENTER,
                                    0,
                                    ((GrDraggable *) d->draggables->data)->fill_or_stroke)) {
                        // FIXME: here we take into account only the first draggable!
                        skip_radius_with_center = true;
                    }
                }
                if (skip_radius_with_center)
                    continue;
            }

            did = true;
            Geom::Point p_old = d->point;
            d->point += Geom::Point (x, y);
            d->point_original = d->point;
            d->knot->moveto(d->point);

            d->fireDraggables (write_repr, scale_radial);
            d->updateHandles( p_old, MG_NODE_NO_SCALE );
            d->updateDependencies(write_repr);
        }
    }

    if (write_repr && did) {
        // we did an undoable action
        DocumentUndo::maybeDone(sp_desktop_document (desktop), "grmoveh", SP_VERB_CONTEXT_GRADIENT,
                                _("Move gradient handle(s)"));
        return;
    }

    if (!did) { // none of the end draggers are selected, so let's try to move the mids

        GrDragger *dragger = (GrDragger *) selected->data;
        // a midpoint dragger can (logically) only contain one GrDraggable
        GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;

        Geom::Point begin(0,0), end(0,0);
        Geom::Point low_lim(0,0), high_lim(0,0);

        SPObject *server = draggable->getServer();
        GSList *moving = NULL;
        gr_midpoint_limits(dragger, server, &begin, &end, &low_lim, &high_lim, &moving);

        Geom::LineSegment ls(low_lim, high_lim);
        Geom::Point p = ls.pointAt(ls.nearestPoint(dragger->point + Geom::Point(x,y)));
        Geom::Point displacement = p - dragger->point;

        for (GSList const* i = moving; i != NULL; i = i->next) {
            GrDragger *drg = (GrDragger*) i->data;
            SPKnot *drgknot = drg->knot;
            drg->point += displacement;
            drgknot->moveto(drg->point);
            drg->fireDraggables (true);
            drg->updateDependencies(true);
            did = true;
        }

        g_slist_free(moving);

        if (write_repr && did) {
            // we did an undoable action
            DocumentUndo::maybeDone(sp_desktop_document (desktop), "grmovem", SP_VERB_CONTEXT_GRADIENT,
                                    _("Move gradient mid stop(s)"));
        }
    }
}

void GrDrag::selected_move_screen(double x, double y)
{
    gdouble zoom = desktop->current_zoom();
    gdouble zx = x / zoom;
    gdouble zy = y / zoom;

    selected_move (zx, zy);
}

/**
 * Select the knot next to the last selected one and deselect all other selected.
 */
GrDragger *GrDrag::select_next()
{
    GrDragger *d = NULL;
    if (selected == NULL || g_list_find(draggers, selected->data)->next == NULL) {
        if (draggers)
            d = (GrDragger *) draggers->data;
    } else {
        d = (GrDragger *) g_list_find(draggers, selected->data)->next->data;
    }
    if (d)
        setSelected (d);
    return d;
}

/**
 * Select the knot previous from the last selected one and deselect all other selected.
 */
GrDragger *GrDrag::select_prev()
{
    GrDragger *d = NULL;
    if (selected == NULL || g_list_find(draggers, selected->data)->prev == NULL) {
        if (draggers)
            d = (GrDragger *) g_list_last (draggers)->data;
    } else {
        d = (GrDragger *) g_list_find(draggers, selected->data)->prev->data;
    }
    if (d)
        setSelected (d);
    return d;
}


// FIXME: i.m.o. an ugly function that I just made to work, but... aargh! (Johan)
void GrDrag::deleteSelected(bool just_one)
{
    if (!selected) return;

    SPDocument *document = NULL;

    struct StructStopInfo {
        SPStop * spstop;
        GrDraggable * draggable;
        SPGradient * gradient;
        SPGradient * vector;
    };

    GSList *midstoplist = NULL;  // list of stops that must be deleted (will be deleted first)
    GSList *endstoplist = NULL;  // list of stops that must be deleted
    while (selected) {
        GrDragger *dragger = (GrDragger*) selected->data;
        for (GSList * drgble = dragger->draggables; drgble != NULL; drgble = drgble->next) {
            GrDraggable *draggable = (GrDraggable*) drgble->data;
            SPGradient *gradient = getGradient(draggable->item, draggable->fill_or_stroke);
            SPGradient *vector   = sp_gradient_get_forked_vector_if_necessary (gradient, false);

            switch (draggable->point_type) {
                case POINT_LG_MID:
                case POINT_RG_MID1:
                case POINT_RG_MID2:
                    {
                        SPStop *stop = sp_get_stop_i(vector, draggable->point_i);
                        // check if already present in list. (e.g. when both RG_MID1 and RG_MID2 were selected)
                        bool present = false;
                        for (GSList const * l = midstoplist; l != NULL; l = l->next) {
                            if ( (SPStop*)l->data == stop ) {
                                present = true;
                                break; // no need to search further.
                            }
                        }
                        if (!present)
                            midstoplist = g_slist_append(midstoplist, stop);
                    }
                    break;
                case POINT_LG_BEGIN:
                case POINT_LG_END:
                case POINT_RG_CENTER:
                case POINT_RG_R1:
                case POINT_RG_R2:
                    {
                        SPStop *stop = NULL;
                        if ( (draggable->point_type == POINT_LG_BEGIN) || (draggable->point_type == POINT_RG_CENTER) ) {
                            stop = vector->getFirstStop();
                        } else {
                            stop = sp_last_stop(vector);
                        }
                        if (stop) {
                            StructStopInfo *stopinfo = new StructStopInfo;
                            stopinfo->spstop = stop;
                            stopinfo->draggable = draggable;
                            stopinfo->gradient = gradient;
                            stopinfo->vector = vector;
                            // check if already present in list. (e.g. when both R1 and R2 were selected)
                            bool present = false;
                            for (GSList const * l = endstoplist; l != NULL; l = l->next) {
                                if ( ((StructStopInfo*)l->data)->spstop == stopinfo->spstop ) {
                                    present = true;
                                    break; // no need to search further.
                                }
                            }
                            if (!present)
                                endstoplist = g_slist_append(endstoplist, stopinfo);
                        }
                    }
                    break;

                default:
                    break;
            }
        }
        selected = g_list_remove(selected, dragger);
        if ( just_one ) break; // iterate once if just_one is set.
    }
    while (midstoplist) {
        SPStop *stop = (SPStop*) midstoplist->data;
        document = stop->document;
        Inkscape::XML::Node * parent = stop->getRepr()->parent();
        parent->removeChild(stop->getRepr());
        midstoplist = g_slist_remove(midstoplist, stop);
    }
    while (endstoplist) {
        StructStopInfo *stopinfo  = (StructStopInfo*) endstoplist->data;
        document = stopinfo->spstop->document;

        // 2 is the minimum, cannot delete more than that without deleting the whole vector
        // cannot use vector->vector.stops.size() because the vector might be invalidated by deletion of a midstop
        // manually count the children, don't know if there already exists a function for this...
        int len = 0;
        for ( SPObject *child = (stopinfo->vector)->firstChild() ; child ; child = child->getNext() )
        {
            if ( SP_IS_STOP(child) ) {
                len ++;
            }
        }
        if (len > 2)
        {
            switch (stopinfo->draggable->point_type) {
                case POINT_LG_BEGIN:
                    {
                        stopinfo->vector->getRepr()->removeChild(stopinfo->spstop->getRepr());

                        SPLinearGradient *lg = SP_LINEARGRADIENT(stopinfo->gradient);
                        Geom::Point oldbegin = Geom::Point (lg->x1.computed, lg->y1.computed);
                        Geom::Point end = Geom::Point (lg->x2.computed, lg->y2.computed);
                        SPStop *stop = stopinfo->vector->getFirstStop();
                        gdouble offset = stop->offset;
                        Geom::Point newbegin = oldbegin + offset * (end - oldbegin);
                        lg->x1.computed = newbegin[Geom::X];
                        lg->y1.computed = newbegin[Geom::Y];

                        Inkscape::XML::Node *repr = stopinfo->gradient->getRepr();
                        sp_repr_set_svg_double(repr, "x1", lg->x1.computed);
                        sp_repr_set_svg_double(repr, "y1", lg->y1.computed);
                        stop->offset = 0;
                        sp_repr_set_css_double(stop->getRepr(), "offset", 0);

                        // iterate through midstops to set new offset values such that they won't move on canvas.
                        SPStop *laststop = sp_last_stop(stopinfo->vector);
                        stop = stop->getNextStop();
                        while ( stop != laststop ) {
                            stop->offset = (stop->offset - offset)/(1 - offset);
                            sp_repr_set_css_double(stop->getRepr(), "offset", stop->offset);
                            stop = stop->getNextStop();
                        }
                    }
                    break;
                case POINT_LG_END:
                    {
                        stopinfo->vector->getRepr()->removeChild(stopinfo->spstop->getRepr());

                        SPLinearGradient *lg = SP_LINEARGRADIENT(stopinfo->gradient);
                        Geom::Point begin = Geom::Point (lg->x1.computed, lg->y1.computed);
                        Geom::Point oldend = Geom::Point (lg->x2.computed, lg->y2.computed);
                        SPStop *laststop = sp_last_stop(stopinfo->vector);
                        gdouble offset = laststop->offset;
                        Geom::Point newend = begin + offset * (oldend - begin);
                        lg->x2.computed = newend[Geom::X];
                        lg->y2.computed = newend[Geom::Y];

                        Inkscape::XML::Node *repr = stopinfo->gradient->getRepr();
                        sp_repr_set_svg_double(repr, "x2", lg->x2.computed);
                        sp_repr_set_svg_double(repr, "y2", lg->y2.computed);
                        laststop->offset = 1;
                        sp_repr_set_css_double(laststop->getRepr(), "offset", 1);

                        // iterate through midstops to set new offset values such that they won't move on canvas.
                        SPStop *stop = stopinfo->vector->getFirstStop();
                        stop = stop->getNextStop();
                        while ( stop != laststop ) {
                            stop->offset = stop->offset / offset;
                            sp_repr_set_css_double(stop->getRepr(), "offset", stop->offset);
                            stop = stop->getNextStop();
                        }
                    }
                    break;
                case POINT_RG_CENTER:
                    {
                        SPStop *newfirst = stopinfo->spstop->getNextStop();
                        if (newfirst) {
                            newfirst->offset = 0;
                            sp_repr_set_css_double(newfirst->getRepr(), "offset", 0);
                        }
                        stopinfo->vector->getRepr()->removeChild(stopinfo->spstop->getRepr());
                    }
                    break;
                case POINT_RG_R1:
                case POINT_RG_R2:
                    {
                        stopinfo->vector->getRepr()->removeChild(stopinfo->spstop->getRepr());

                        SPRadialGradient *rg = SP_RADIALGRADIENT(stopinfo->gradient);
                        double oldradius = rg->r.computed;
                        SPStop *laststop = sp_last_stop(stopinfo->vector);
                        gdouble offset = laststop->offset;
                        double newradius = offset * oldradius;
                        rg->r.computed = newradius;

                        Inkscape::XML::Node *repr = rg->getRepr();
                        sp_repr_set_svg_double(repr, "r", rg->r.computed);
                        laststop->offset = 1;
                        sp_repr_set_css_double(laststop->getRepr(), "offset", 1);

                        // iterate through midstops to set new offset values such that they won't move on canvas.
                        SPStop *stop = stopinfo->vector->getFirstStop();
                        stop = stop->getNextStop();
                        while ( stop != laststop ) {
                            stop->offset = stop->offset / offset;
                            sp_repr_set_css_double(stop->getRepr(), "offset", stop->offset);
                            stop = stop->getNextStop();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        else
        { // delete the gradient from the object. set fill to unset  FIXME: set to fill of unselected node?
            SPCSSAttr *css = sp_repr_css_attr_new ();

            // stopinfo->spstop is the selected stop
            Inkscape::XML::Node *unselectedrepr = stopinfo->vector->getRepr()->firstChild();
            if (unselectedrepr == stopinfo->spstop->getRepr() ) {
                unselectedrepr = unselectedrepr->next();
            }

            if (unselectedrepr == NULL) {
                if (stopinfo->draggable->fill_or_stroke == Inkscape::FOR_FILL) {
                    sp_repr_css_unset_property (css, "fill");
                } else {
                    sp_repr_css_unset_property (css, "stroke");
                }
            } else {
                SPCSSAttr *stopcss = sp_repr_css_attr(unselectedrepr, "style");
                if (stopinfo->draggable->fill_or_stroke == Inkscape::FOR_FILL) {
                    sp_repr_css_set_property(css, "fill", sp_repr_css_property(stopcss, "stop-color", "inkscape:unset"));
                    sp_repr_css_set_property(css, "fill-opacity", sp_repr_css_property(stopcss, "stop-opacity", "1"));
                } else {
                    sp_repr_css_set_property(css, "stroke", sp_repr_css_property(stopcss, "stop-color", "inkscape:unset"));
                    sp_repr_css_set_property(css, "stroke-opacity", sp_repr_css_property(stopcss, "stop-opacity", "1"));
                }
                sp_repr_css_attr_unref (stopcss);
            }

            sp_repr_css_change(stopinfo->draggable->item->getRepr(), css, "style");
            sp_repr_css_attr_unref (css);
        }

        endstoplist = g_slist_remove(endstoplist, stopinfo);
        delete stopinfo;
    }

    if (document) {
        DocumentUndo::done( document, SP_VERB_CONTEXT_GRADIENT, _("Delete gradient stop(s)") );
    }
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

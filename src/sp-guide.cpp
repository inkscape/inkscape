#define __SP_GUIDE_C__

/*
 * Inkscape guideline implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *   Johan Engelen
 *
 * Copyright (C) 2000-2002 authors
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <algorithm>
#include <cstring>
#include <string>
#include "desktop-handles.h"
#include "display/guideline.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "attributes.h"
#include "sp-guide.h"
#include <sp-item-notify-moveto.h>
#include <sp-item.h>
#include <sp-guide-constraint.h>
#include <glibmm/i18n.h>
#include <xml/repr.h>
#include <remove-last.h>
#include "sp-metrics.h"
#include "inkscape.h"
#include "desktop.h"
#include "sp-namedview.h"
#include <2geom/angle.h>
#include "document.h"

using std::vector;

enum {
    PROP_0,
    PROP_COLOR,
    PROP_HICOLOR
};

static void sp_guide_class_init(SPGuideClass *gc);
static void sp_guide_init(SPGuide *guide);
static void sp_guide_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void sp_guide_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void sp_guide_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_guide_release(SPObject *object);
static void sp_guide_set(SPObject *object, unsigned int key, const gchar *value);

static SPObjectClass *parent_class;

GType sp_guide_get_type(void)
{
    static GType guide_type = 0;

    if (!guide_type) {
        GTypeInfo guide_info = {
            sizeof(SPGuideClass),
            NULL, NULL,
            (GClassInitFunc) sp_guide_class_init,
            NULL, NULL,
            sizeof(SPGuide),
            16,
            (GInstanceInitFunc) sp_guide_init,
            NULL,	/* value_table */
        };
        guide_type = g_type_register_static(SP_TYPE_OBJECT, "SPGuide", &guide_info, (GTypeFlags) 0);
    }

    return guide_type;
}

static void sp_guide_class_init(SPGuideClass *gc)
{
    GObjectClass *gobject_class = (GObjectClass *) gc;
    SPObjectClass *sp_object_class = (SPObjectClass *) gc;

    parent_class = (SPObjectClass*) g_type_class_ref(SP_TYPE_OBJECT);

    gobject_class->set_property = sp_guide_set_property;
    gobject_class->get_property = sp_guide_get_property;

    sp_object_class->build = sp_guide_build;
    sp_object_class->release = sp_guide_release;
    sp_object_class->set = sp_guide_set;

    g_object_class_install_property(gobject_class,
                                    PROP_COLOR,
                                    g_param_spec_uint("color", "Color", "Color",
                                                      0,
                                                      0xffffffff,
                                                      0xff000000,
                                                      (GParamFlags) G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class,
                                    PROP_HICOLOR,
                                    g_param_spec_uint("hicolor", "HiColor", "HiColor",
                                                      0,
                                                      0xffffffff,
                                                      0xff000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
}

static void sp_guide_init(SPGuide *guide)
{
    guide->normal_to_line = component_vectors[Geom::Y];
    guide->point_on_line = Geom::Point(0.,0.);
    guide->color = 0x0000ff7f;
    guide->hicolor = 0xff00007f;
}

static void sp_guide_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec */*pspec*/)
{
    SPGuide &guide = *SP_GUIDE(object);

    switch (prop_id) {
        case PROP_COLOR:
            guide.color = g_value_get_uint(value);
            for (GSList *l = guide.views; l != NULL; l = l->next) {
                sp_guideline_set_color(SP_GUIDELINE(l->data), guide.color);
            }
            break;

        case PROP_HICOLOR:
            guide.hicolor = g_value_get_uint(value);
            break;
    }
}

static void sp_guide_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec */*pspec*/)
{
    SPGuide const &guide = *SP_GUIDE(object);

    switch (prop_id) {
        case PROP_COLOR:
            g_value_set_uint(value, guide.color);
            break;
        case PROP_HICOLOR:
            g_value_set_uint(value, guide.hicolor);
            break;
    }
}

static void sp_guide_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        (* ((SPObjectClass *) (parent_class))->build)(object, document, repr);
    }

    sp_object_read_attr(object, "orientation");
    sp_object_read_attr(object, "position");
}

static void sp_guide_release(SPObject *object)
{
    SPGuide *guide = (SPGuide *) object;

    while (guide->views) {
        gtk_object_destroy(GTK_OBJECT(guide->views->data));
        guide->views = g_slist_remove(guide->views, guide->views->data);
    }

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

static void sp_guide_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPGuide *guide = SP_GUIDE(object);

    switch (key) {
    case SP_ATTR_ORIENTATION:
        {
            if (value && !strcmp(value, "horizontal")) {
                /* Visual representation of a horizontal line, constrain vertically (y coordinate). */
                guide->normal_to_line = component_vectors[Geom::Y];
            } else if (value && !strcmp(value, "vertical")) {
                guide->normal_to_line = component_vectors[Geom::X];
            } else if (value) {
                gchar ** strarray = g_strsplit(value, ",", 2);
                double newx, newy;
                unsigned int success = sp_svg_number_read_d(strarray[0], &newx);
                success += sp_svg_number_read_d(strarray[1], &newy);
                g_strfreev (strarray);
                if (success == 2 && (fabs(newx) > 1e-6 || fabs(newy) > 1e-6)) {
                    Geom::Point direction(newx, newy);
                    direction.normalize();
                    guide->normal_to_line = direction;
                } else {
                    // default to vertical line for bad arguments
                    guide->normal_to_line = component_vectors[Geom::X];
                }
            } else {
                // default to vertical line for bad arguments
                guide->normal_to_line = component_vectors[Geom::X];
            }
            sp_guide_set_normal(*guide, guide->normal_to_line, false);
        }
        break;
    case SP_ATTR_POSITION:
        {
            gchar ** strarray = g_strsplit(value, ",", 2);
            double newx, newy;
            unsigned int success = sp_svg_number_read_d(strarray[0], &newx);
            success += sp_svg_number_read_d(strarray[1], &newy);
            g_strfreev (strarray);
            if (success == 2) {
                guide->point_on_line = Geom::Point(newx, newy);
            } else if (success == 1) {
                // before 0.46 style guideline definition.
                const gchar *attr = SP_OBJECT_REPR(object)->attribute("orientation");
                if (attr && !strcmp(attr, "horizontal")) {
                    guide->point_on_line = Geom::Point(0, newx);
                } else {
                    guide->point_on_line = Geom::Point(newx, 0);
                }
            }

            // update position in non-committing way
            // fixme: perhaps we need to add an update method instead, and request_update here
            sp_guide_moveto(*guide, guide->point_on_line, false);
        }
        break;
    default:
            if (((SPObjectClass *) (parent_class))->set) {
                ((SPObjectClass *) (parent_class))->set(object, key, value);
            }
            break;
    }
}

SPGuide *
sp_guide_create(SPDesktop *desktop, Geom::Point const &pt1, Geom::Point const &pt2) {
    SPDocument *doc=sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::XML::Node *repr = xml_doc->createElement("sodipodi:guide");

    Geom::Point n = Geom::rot90(pt2 - pt1);

    sp_repr_set_point(repr, "position", pt1);
    sp_repr_set_point(repr, "orientation", n);

    SP_OBJECT_REPR(desktop->namedview)->appendChild(repr);
    Inkscape::GC::release(repr);

    SPGuide *guide= SP_GUIDE(doc->getObjectByRepr(repr));
    return guide;
}

void
sp_guide_pt_pairs_to_guides(SPDesktop *dt, std::list<std::pair<Geom::Point, Geom::Point> > &pts) {
    for (std::list<std::pair<Geom::Point, Geom::Point> >::iterator i = pts.begin(); i != pts.end(); ++i) {
        sp_guide_create(dt, (*i).first, (*i).second);
    }
}

void
sp_guide_create_guides_around_page(SPDesktop *dt) {
    SPDocument *doc=sp_desktop_document(dt);
    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Point A(0, 0);
    Geom::Point C(sp_document_width(doc), sp_document_height(doc));
    Geom::Point B(C[Geom::X], 0);
    Geom::Point D(0, C[Geom::Y]);

    pts.push_back(std::make_pair<Geom::Point, Geom::Point>(A, B));
    pts.push_back(std::make_pair<Geom::Point, Geom::Point>(B, C));
    pts.push_back(std::make_pair<Geom::Point, Geom::Point>(C, D));
    pts.push_back(std::make_pair<Geom::Point, Geom::Point>(D, A));

    sp_guide_pt_pairs_to_guides(dt, pts);

    sp_document_done (doc, SP_VERB_NONE, _("Guides around page"));
}

void sp_guide_show(SPGuide *guide, SPCanvasGroup *group, GCallback handler)
{
    SPCanvasItem *item = sp_guideline_new(group, guide->point_on_line, guide->normal_to_line);
    sp_guideline_set_color(SP_GUIDELINE(item), guide->color);

    g_signal_connect(G_OBJECT(item), "event", G_CALLBACK(handler), guide);

    guide->views = g_slist_prepend(guide->views, item);
}

void sp_guide_hide(SPGuide *guide, SPCanvas *canvas)
{
    g_assert(guide != NULL);
    g_assert(SP_IS_GUIDE(guide));
    g_assert(canvas != NULL);
    g_assert(SP_IS_CANVAS(canvas));

    for (GSList *l = guide->views; l != NULL; l = l->next) {
        if (canvas == SP_CANVAS_ITEM(l->data)->canvas) {
            gtk_object_destroy(GTK_OBJECT(l->data));
            guide->views = g_slist_remove(guide->views, l->data);
            return;
        }
    }

    g_assert_not_reached();
}

void sp_guide_sensitize(SPGuide *guide, SPCanvas *canvas, gboolean sensitive)
{
    g_assert(guide != NULL);
    g_assert(SP_IS_GUIDE(guide));
    g_assert(canvas != NULL);
    g_assert(SP_IS_CANVAS(canvas));

    for (GSList *l = guide->views; l != NULL; l = l->next) {
        if (canvas == SP_CANVAS_ITEM(l->data)->canvas) {
            sp_guideline_set_sensitive(SP_GUIDELINE(l->data), sensitive);
            return;
        }
    }

    g_assert_not_reached();
}

Geom::Point sp_guide_position_from_pt(SPGuide const *guide, Geom::Point const &pt)
{
    return -(pt - guide->point_on_line);
}

double sp_guide_distance_from_pt(SPGuide const *guide, Geom::Point const &pt)
{
    return Geom::dot(pt - guide->point_on_line, guide->normal_to_line);
}

/**
 * \arg commit False indicates temporary moveto in response to motion event while dragging,
 *      true indicates a "committing" version: in response to button release event after
 *      dragging a guideline, or clicking OK in guide editing dialog.
 */
void sp_guide_moveto(SPGuide const &guide, Geom::Point const point_on_line, bool const commit)
{
    g_assert(SP_IS_GUIDE(&guide));

    for (GSList *l = guide.views; l != NULL; l = l->next) {
        sp_guideline_set_position(SP_GUIDELINE(l->data), point_on_line);
    }

    /* Calling sp_repr_set_point must precede calling sp_item_notify_moveto in the commit
       case, so that the guide's new position is available for sp_item_rm_unsatisfied_cns. */
    if (commit) {
        sp_repr_set_point(SP_OBJECT(&guide)->repr, "position", point_on_line);
    }

/*  DISABLED CODE BECAUSE  SPGuideAttachment  IS NOT USE AT THE MOMENT (johan)
    for (vector<SPGuideAttachment>::const_iterator i(guide.attached_items.begin()),
             iEnd(guide.attached_items.end());
         i != iEnd; ++i)
    {
        SPGuideAttachment const &att = *i;
        sp_item_notify_moveto(*att.item, guide, att.snappoint_ix, position, commit);
    }
*/
}

/**
 * \arg commit False indicates temporary moveto in response to motion event while dragging,
 *      true indicates a "committing" version: in response to button release event after
 *      dragging a guideline, or clicking OK in guide editing dialog.
 */
void sp_guide_set_normal(SPGuide const &guide, Geom::Point const normal_to_line, bool const commit)
{
    g_assert(SP_IS_GUIDE(&guide));

    for (GSList *l = guide.views; l != NULL; l = l->next) {
        sp_guideline_set_normal(SP_GUIDELINE(l->data), normal_to_line);
    }

    /* Calling sp_repr_set_svg_point must precede calling sp_item_notify_moveto in the commit
       case, so that the guide's new position is available for sp_item_rm_unsatisfied_cns. */
    if (commit) {
        sp_repr_set_point(SP_OBJECT(&guide)->repr, "orientation", normal_to_line);
    }

/*  DISABLED CODE BECAUSE  SPGuideAttachment  IS NOT USE AT THE MOMENT (johan)
    for (vector<SPGuideAttachment>::const_iterator i(guide.attached_items.begin()),
             iEnd(guide.attached_items.end());
         i != iEnd; ++i)
    {
        SPGuideAttachment const &att = *i;
        sp_item_notify_moveto(*att.item, guide, att.snappoint_ix, position, commit);
    }
*/
}

/**
 * Returns a human-readable description of the guideline for use in dialog boxes and status bar.
 *
 * The caller is responsible for freeing the string.
 */
char *sp_guide_description(SPGuide const *guide)
{
    using Geom::X;
    using Geom::Y;
            
    GString *position_string_x = SP_PX_TO_METRIC_STRING(guide->point_on_line[X], SP_ACTIVE_DESKTOP->namedview->getDefaultMetric());
    GString *position_string_y = SP_PX_TO_METRIC_STRING(guide->point_on_line[Y], SP_ACTIVE_DESKTOP->namedview->getDefaultMetric());

    if ( are_near(guide->normal_to_line, component_vectors[X]) ||
         are_near(guide->normal_to_line, -component_vectors[X]) ) {
        return g_strdup_printf(_("vertical, at %s"), position_string_x->str);
    } else if ( are_near(guide->normal_to_line, component_vectors[Y]) ||
                are_near(guide->normal_to_line, -component_vectors[Y]) ) {
        return g_strdup_printf(_("horizontal, at %s"), position_string_y->str);
    } else {
        double const radians = guide->angle();
        double const degrees = Geom::rad_to_deg(radians);
        int const degrees_int = (int) round(degrees);
        return g_strdup_printf(_("at %d degrees, through (%s,%s); <b>Ctrl</b>+click to delete"), degrees_int, position_string_x->str, position_string_y->str);
    }

    g_string_free(position_string_x, TRUE);
    g_string_free(position_string_y, TRUE);
}

void sp_guide_remove(SPGuide *guide)
{
    g_assert(SP_IS_GUIDE(guide));

    for (vector<SPGuideAttachment>::const_iterator i(guide->attached_items.begin()),
             iEnd(guide->attached_items.end());
         i != iEnd; ++i)
    {
        SPGuideAttachment const &att = *i;
        remove_last(att.item->constraints, SPGuideConstraint(guide, att.snappoint_ix));
    }
    guide->attached_items.clear();

    sp_repr_unparent(SP_OBJECT(guide)->repr);
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

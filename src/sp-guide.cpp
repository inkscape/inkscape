/*
 * Inkscape guideline implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *   Johan Engelen
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
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
#include "display/sp-canvas.h"
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
#include "inkscape.h"
#include "desktop.h"
#include "sp-namedview.h"
#include <2geom/angle.h>
#include "document.h"
#include "document-undo.h"
#include "verbs.h"

using Inkscape::DocumentUndo;
using std::vector;

//enum {
//    PROP_0,
//    PROP_COLOR,
//    PROP_HICOLOR
//};
//
//static void sp_guide_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
//static void sp_guide_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

#include "sp-factory.h"

namespace {
	SPObject* createGuide() {
		return new SPGuide();
	}

	bool guideRegistered = SPFactory::instance().registerObject("sodipodi:guide", createGuide);
}

//static void sp_guide_class_init(SPGuideClass *gc)
//{
//    GObjectClass *gobject_class = (GObjectClass *) gc;
//
//    gobject_class->set_property = sp_guide_set_property;
//    gobject_class->get_property = sp_guide_get_property;
//
//    g_object_class_install_property(gobject_class,
//                                    PROP_COLOR,
//                                    g_param_spec_uint("color", "Color", "Color",
//                                                      0,
//                                                      0xffffffff,
//                                                      0xff000000,
//                                                      (GParamFlags) G_PARAM_READWRITE));
//
//    g_object_class_install_property(gobject_class,
//                                    PROP_HICOLOR,
//                                    g_param_spec_uint("hicolor", "HiColor", "HiColor",
//                                                      0,
//                                                      0xffffffff,
//                                                      0xff000000,
//                                                      (GParamFlags) G_PARAM_READWRITE));
//}
// CPPIFY: properties!

SPGuide::SPGuide() : SPObject() {
	this->label = NULL;
	this->views = NULL;

    this->normal_to_line = Geom::Point(0.,1.);
    this->point_on_line = Geom::Point(0.,0.);
    this->color = 0x0000ff7f;
    this->hicolor = 0xff00007f;
}

SPGuide::~SPGuide() {
}

guint32 SPGuide::getColor() const {
	return color;
}

guint32 SPGuide::getHiColor() const {
	return hicolor;
}

void SPGuide::setColor(guint32 c) {
	color = c;

	for (GSList *l = this->views; l != NULL; l = l->next) {
		sp_guideline_set_color(SP_GUIDELINE(l->data), this->color);
	}
}

void SPGuide::setHiColor(guint32 h) {
	this->hicolor = h;
}

//static void sp_guide_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec */*pspec*/)
//{
//    SPGuide &guide = *SP_GUIDE(object);
//
//    switch (prop_id) {
//        case PROP_COLOR:
//            guide.color = g_value_get_uint(value);
//            for (GSList *l = guide.views; l != NULL; l = l->next) {
//                sp_guideline_set_color(SP_GUIDELINE(l->data), guide.color);
//            }
//            break;
//
//        case PROP_HICOLOR:
//            guide.hicolor = g_value_get_uint(value);
//            break;
//    }
//}
//
//static void sp_guide_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec */*pspec*/)
//{
//    SPGuide const &guide = *SP_GUIDE(object);
//
//    switch (prop_id) {
//        case PROP_COLOR:
//            g_value_set_uint(value, guide.color);
//            break;
//        case PROP_HICOLOR:
//            g_value_set_uint(value, guide.hicolor);
//            break;
//    }
//}

void SPGuide::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPObject::build(document, repr);

    this->readAttr( "inkscape:label" );
    this->readAttr( "orientation" );
    this->readAttr( "position" );

    /* Register */
    document->addResource("guide", this);
}

void SPGuide::release() {
    while (this->views) {
        sp_guideline_delete(SP_GUIDELINE(this->views->data));
        this->views = g_slist_remove(this->views, this->views->data);
    }

    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("guide", this);
    }

    SPObject::release();
}

void SPGuide::set(unsigned int key, const gchar *value) {
    switch (key) {
    case SP_ATTR_INKSCAPE_LABEL:
        if (this->label) g_free(this->label);
        
        if (value) {
            this->label = g_strdup(value);
        } else {
            this->label = NULL;
        }

        sp_guide_set_label(*this, this->label, false);
        break;
    case SP_ATTR_ORIENTATION:
        {
            if (value && !strcmp(value, "horizontal")) {
                /* Visual representation of a horizontal line, constrain vertically (y coordinate). */
                this->normal_to_line = Geom::Point(0., 1.);
            } else if (value && !strcmp(value, "vertical")) {
                this->normal_to_line = Geom::Point(1., 0.);
            } else if (value) {
                gchar ** strarray = g_strsplit(value, ",", 2);
                double newx, newy;
                unsigned int success = sp_svg_number_read_d(strarray[0], &newx);
                success += sp_svg_number_read_d(strarray[1], &newy);
                g_strfreev (strarray);
                if (success == 2 && (fabs(newx) > 1e-6 || fabs(newy) > 1e-6)) {
                    Geom::Point direction(newx, newy);
                    direction.normalize();
                    this->normal_to_line = direction;
                } else {
                    // default to vertical line for bad arguments
                    this->normal_to_line = Geom::Point(1., 0.);
                }
            } else {
                // default to vertical line for bad arguments
                this->normal_to_line = Geom::Point(1., 0.);
            }
            sp_guide_set_normal(*this, this->normal_to_line, false);
        }
        break;
    case SP_ATTR_POSITION:
        {
            if (value) {
                gchar ** strarray = g_strsplit(value, ",", 2);
                double newx, newy;
                unsigned int success = sp_svg_number_read_d(strarray[0], &newx);
                success += sp_svg_number_read_d(strarray[1], &newy);
                g_strfreev (strarray);
                if (success == 2) {
                    this->point_on_line = Geom::Point(newx, newy);
                } else if (success == 1) {
                    // before 0.46 style guideline definition.
                    const gchar *attr = this->getRepr()->attribute("orientation");
                    if (attr && !strcmp(attr, "horizontal")) {
                        this->point_on_line = Geom::Point(0, newx);
                    } else {
                        this->point_on_line = Geom::Point(newx, 0);
                    }
                }
            } else {
                // default to (0,0) for bad arguments
                this->point_on_line = Geom::Point(0,0);
            }
            // update position in non-committing way
            // fixme: perhaps we need to add an update method instead, and request_update here
            sp_guide_moveto(*this, this->point_on_line, false);
        }
        break;
    default:
    	SPObject::set(key, value);
            break;
    }
}

SPGuide *SPGuide::createSPGuide(SPDocument *doc, Geom::Point const &pt1, Geom::Point const &pt2)
{
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::XML::Node *repr = xml_doc->createElement("sodipodi:guide");

    Geom::Point n = Geom::rot90(pt2 - pt1);

    sp_repr_set_point(repr, "position", pt1);
    sp_repr_set_point(repr, "orientation", n);

    SPNamedView *namedview = sp_document_namedview(doc, NULL);
    if (namedview) {
        namedview->appendChild(repr);
    }
    Inkscape::GC::release(repr);

    SPGuide *guide= SP_GUIDE(doc->getObjectByRepr(repr));
    return guide;
}

void
sp_guide_pt_pairs_to_guides(SPDocument *doc, std::list<std::pair<Geom::Point, Geom::Point> > &pts) {
    for (std::list<std::pair<Geom::Point, Geom::Point> >::iterator i = pts.begin(); i != pts.end(); ++i) {
        SPGuide::createSPGuide(doc, (*i).first, (*i).second);
    }
}

void
sp_guide_create_guides_around_page(SPDesktop *dt) {
    SPDocument *doc=sp_desktop_document(dt);
    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Point A(0, 0);
    Geom::Point C(doc->getWidth().value("px"), doc->getHeight().value("px"));
    Geom::Point B(C[Geom::X], 0);
    Geom::Point D(0, C[Geom::Y]);

    pts.push_back(std::pair<Geom::Point, Geom::Point>(A, B));
    pts.push_back(std::pair<Geom::Point, Geom::Point>(B, C));
    pts.push_back(std::pair<Geom::Point, Geom::Point>(C, D));
    pts.push_back(std::pair<Geom::Point, Geom::Point>(D, A));

    sp_guide_pt_pairs_to_guides(doc, pts);

    DocumentUndo::done(doc, SP_VERB_NONE, _("Create Guides Around the Page"));
}

void
sp_guide_delete_all_guides(SPDesktop *dt) {
    SPDocument *doc=sp_desktop_document(dt);
    const GSList *current;
    while ( (current = doc->getResourceList("guide")) ) {
        SPGuide* guide = SP_GUIDE(current->data);
        sp_guide_remove(guide);
    }

    DocumentUndo::done(doc, SP_VERB_NONE, _("Delete All Guides"));
}

void SPGuide::showSPGuide(SPCanvasGroup *group, GCallback handler)
{
    SPCanvasItem *item = sp_guideline_new(group, label, point_on_line, normal_to_line);
    sp_guideline_set_color(SP_GUIDELINE(item), color);

    g_signal_connect(G_OBJECT(item), "event", G_CALLBACK(handler), this);

    views = g_slist_prepend(views, item);
}

void SPGuide::hideSPGuide(SPCanvas *canvas)
{
    g_assert(canvas != NULL);
    g_assert(SP_IS_CANVAS(canvas));

    for (GSList *l = views; l != NULL; l = l->next) {
        if (canvas == SP_CANVAS_ITEM(l->data)->canvas) {
            sp_guideline_delete(SP_GUIDELINE(l->data));
            views = g_slist_remove(views, l->data);
            return;
        }
    }

    g_assert_not_reached();
}

void SPGuide::sensitize(SPCanvas *canvas, gboolean sensitive)
{
    g_assert(canvas != NULL);
    g_assert(SP_IS_CANVAS(canvas));

    for (GSList *l = views; l != NULL; l = l->next) {
        if (canvas == SP_CANVAS_ITEM(l->data)->canvas) {
            sp_guideline_set_sensitive(SP_GUIDELINE(l->data), sensitive);
            return;
        }
    }

    g_assert_not_reached();
}

Geom::Point SPGuide::getPositionFrom(Geom::Point const &pt) const
{
    return -(pt - point_on_line);
}

double SPGuide::getDistanceFrom(Geom::Point const &pt) const
{
    return Geom::dot(pt - point_on_line, normal_to_line);
}

/**
 * \arg commit False indicates temporary moveto in response to motion event while dragging,
 *      true indicates a "committing" version: in response to button release event after
 *      dragging a guideline, or clicking OK in guide editing dialog.
 */
void sp_guide_moveto(SPGuide &guide, Geom::Point const point_on_line, bool const commit)
{
    g_assert(SP_IS_GUIDE(&guide));

    for (GSList *l = guide.views; l != NULL; l = l->next) {
        sp_guideline_set_position(SP_GUIDELINE(l->data), point_on_line);
    }

    /* Calling sp_repr_set_point must precede calling sp_item_notify_moveto in the commit
       case, so that the guide's new position is available for sp_item_rm_unsatisfied_cns. */
    if (commit) {
        //XML Tree being used here directly while it shouldn't be.
        sp_repr_set_point(guide.getRepr(), "position", point_on_line);
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
void sp_guide_set_normal(SPGuide &guide, Geom::Point const normal_to_line, bool const commit)
{
    g_assert(SP_IS_GUIDE(&guide));

    for (GSList *l = guide.views; l != NULL; l = l->next) {
        sp_guideline_set_normal(SP_GUIDELINE(l->data), normal_to_line);
    }

    /* Calling sp_repr_set_svg_point must precede calling sp_item_notify_moveto in the commit
       case, so that the guide's new position is available for sp_item_rm_unsatisfied_cns. */
    if (commit) {
        //XML Tree being used directly while it shouldn't be
        sp_repr_set_point(guide.getRepr(), "orientation", normal_to_line);
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

void sp_guide_set_color(SPGuide &guide, const unsigned r, const unsigned g, const unsigned b, bool const commit)
{
    g_assert(SP_IS_GUIDE(&guide));
    guide.color = (r << 24) | (g << 16) | (b << 8) | 0x7f;

    if (guide.views){
        sp_guideline_set_color(SP_GUIDELINE(guide.views->data), guide.color);
    }

    if (commit){
        std::ostringstream os;
        os << "rgb(" << r << "," << g << "," << b << ")";
        //XML Tree being used directly while it shouldn't be
        guide.getRepr()->setAttribute("inkscape:color", os.str().c_str());
    }
}

void sp_guide_set_label(SPGuide &guide, const char* label, bool const commit)
{
    g_assert(SP_IS_GUIDE(&guide));
    if (guide.views){
        sp_guideline_set_label(SP_GUIDELINE(guide.views->data), label);
    }

    if (commit){
        //XML Tree being used directly while it shouldn't be
        guide.getRepr()->setAttribute("inkscape:label", label);
    }
}

/**
 * Returns a human-readable description of the guideline for use in dialog boxes and status bar.
 * If verbose is false, only positioning information is included (useful for dialogs).
 *
 * The caller is responsible for freeing the string.
 */
char *sp_guide_description(SPGuide const *guide, const bool verbose)
{
    using Geom::X;
    using Geom::Y;

    char *descr = 0;
    if ( !guide->document ) {
        // Guide has probably been deleted and no longer has an attached namedview.
        descr = g_strdup_printf("%s", _("Deleted"));
    } else {
        SPNamedView *namedview = sp_document_namedview(guide->document, NULL);

        Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(guide->point_on_line[X], "px");
        Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(guide->point_on_line[Y], "px");
        GString *position_string_x = g_string_new(x_q.string(namedview->doc_units).c_str());
        GString *position_string_y = g_string_new(y_q.string(namedview->doc_units).c_str());

        gchar *shortcuts = g_strdup_printf("; %s", _("<b>Shift+drag</b> to rotate, <b>Ctrl+drag</b> to move origin, <b>Del</b> to delete"));

        if ( are_near(guide->normal_to_line, Geom::Point(1., 0.)) ||
             are_near(guide->normal_to_line, -Geom::Point(1., 0.)) ) {
            descr = g_strdup_printf(_("vertical, at %s"), position_string_x->str);
        } else if ( are_near(guide->normal_to_line, Geom::Point(0., 1.)) ||
                    are_near(guide->normal_to_line, -Geom::Point(0., 1.)) ) {
            descr = g_strdup_printf(_("horizontal, at %s"), position_string_y->str);
        } else {
            double const radians = guide->angle();
            double const degrees = Geom::rad_to_deg(radians);
            int const degrees_int = (int) round(degrees);
            descr = g_strdup_printf(_("at %d degrees, through (%s,%s)"), 
                                    degrees_int, position_string_x->str, position_string_y->str);
        }

        g_string_free(position_string_x, TRUE);
        g_string_free(position_string_y, TRUE);

        if (verbose) {
            gchar *oldDescr = descr;
            descr = g_strconcat(oldDescr, shortcuts, NULL);
            g_free(oldDescr);
        }
        g_free(shortcuts);
    }

    return descr;
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

    //XML Tree being used directly while it shouldn't be.
    sp_repr_unparent(guide->getRepr());
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

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

#include "display/sp-canvas.h"
#include "display/guideline.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
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
#include "sp-root.h"
#include "sp-namedview.h"
#include <2geom/angle.h>
#include "document.h"
#include "document-undo.h"
#include "helper-fns.h"
#include "verbs.h"

using Inkscape::DocumentUndo;
using std::vector;

SPGuide::SPGuide()
    : SPObject()
    , label(NULL)
    , locked(0)
    , normal_to_line(Geom::Point(0.,1.))
    , point_on_line(Geom::Point(0.,0.))
    , color(0x0000ff7f)
    , hicolor(0xff00007f)
{}

void SPGuide::setColor(guint32 c)
{
    color = c;

    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        sp_guideline_set_color(*it, this->color);
    }
}

void SPGuide::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    SPObject::build(document, repr);

    this->readAttr( "inkscape:color" );
    this->readAttr( "inkscape:label" );
    this->readAttr( "inkscape:locked" );
    this->readAttr( "orientation" );
    this->readAttr( "position" );

    /* Register */
    document->addResource("guide", this);
}

void SPGuide::release()
{
    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        sp_guideline_delete(*it);
    }
    this->views.clear();

    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("guide", this);
    }

    SPObject::release();
}

void SPGuide::set(unsigned int key, const gchar *value) {
    switch (key) {
    case SP_ATTR_INKSCAPE_COLOR:
        if (value) {
            this->setColor(sp_svg_read_color(value, 0x0000ff00) | 0x7f);
        }
        break;
    case SP_ATTR_INKSCAPE_LABEL:
        // this->label already freed in sp_guideline_set_label (src/display/guideline.cpp)
        // see bug #1498444, bug #1469514
        if (value) {
            this->label = g_strdup(value);
        } else {
            this->label = NULL;
        }

        this->set_label(this->label, false);
        break;
    case SP_ATTR_INKSCAPE_LOCKED:
        this->locked = helperfns_read_bool(value, false);
        if (value) {
            this->set_locked(this->locked, false);
        }
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
        this->set_normal(this->normal_to_line, false);
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
                // If root viewBox set, interpret guides in terms of viewBox (90/96)
                SPRoot *root = document->getRoot();
                if( root->viewBox_set ) {
                    if(Geom::are_near((root->width.computed * root->viewBox.height()) / (root->viewBox.width() * root->height.computed), 1.0, Geom::EPSILON)) {
                        // for uniform scaling, try to reduce numerical error
                        double vbunit2px = (root->width.computed / root->viewBox.width() + root->height.computed / root->viewBox.height())/2.0;
                        newx = newx * vbunit2px;
                        newy = newy * vbunit2px;
                    } else {
                        newx = newx * root->width.computed  / root->viewBox.width();
                        newy = newy * root->height.computed / root->viewBox.height();
                    }
                }
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
        this->moveto(this->point_on_line, false);
    }
    break;
    default:
    	SPObject::set(key, value);
        break;
    }
}

/* Only used internally and in sp-line.cpp */
SPGuide *SPGuide::createSPGuide(SPDocument *doc, Geom::Point const &pt1, Geom::Point const &pt2)
{
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::XML::Node *repr = xml_doc->createElement("sodipodi:guide");

    Geom::Point n = Geom::rot90(pt2 - pt1);

    // If root viewBox set, interpret guides in terms of viewBox (90/96)
    double newx = pt1.x();
    double newy = pt1.y();

    SPRoot *root = doc->getRoot();
    if( root->viewBox_set ) {
        // check to see if scaling is uniform
        if(Geom::are_near((root->viewBox.width() * root->height.computed) / (root->width.computed * root->viewBox.height()), 1.0, Geom::EPSILON)) {
            double px2vbunit = (root->viewBox.width()/root->width.computed + root->viewBox.height()/root->height.computed)/2.0;
            newx = newx * px2vbunit;
            newy = newy * px2vbunit;
        } else {
            newx = newx * root->viewBox.width()  / root->width.computed;
            newy = newy * root->viewBox.height() / root->height.computed;
        }
    }

    sp_repr_set_point(repr, "position", Geom::Point( newx, newy ));
    sp_repr_set_point(repr, "orientation", n);

    SPNamedView *namedview = sp_document_namedview(doc, NULL);
    if (namedview) {
        namedview->appendChild(repr);
    }
    Inkscape::GC::release(repr);

    SPGuide *guide= SP_GUIDE(doc->getObjectByRepr(repr));
    return guide;
}

void sp_guide_pt_pairs_to_guides(SPDocument *doc, std::list<std::pair<Geom::Point, Geom::Point> > &pts)
{
    for (std::list<std::pair<Geom::Point, Geom::Point> >::iterator i = pts.begin(); i != pts.end(); ++i) {
        SPGuide::createSPGuide(doc, (*i).first, (*i).second);
    }
}

void sp_guide_create_guides_around_page(SPDesktop *dt)
{
    SPDocument *doc=dt->getDocument();
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

void sp_guide_delete_all_guides(SPDesktop *dt)
{
    SPDocument *doc=dt->getDocument();
    std::vector<SPObject *> current = doc->getResourceList("guide");
    while (!current.empty()){
        SPGuide* guide = SP_GUIDE(*(current.begin()));
        sp_guide_remove(guide);
        current = doc->getResourceList("guide");
    }

    DocumentUndo::done(doc, SP_VERB_NONE, _("Delete All Guides"));
}

void SPGuide::showSPGuide(SPCanvasGroup *group, GCallback handler)
{
    SPCanvasItem *item = sp_guideline_new(group, label, point_on_line, normal_to_line);
    sp_guideline_set_color(SP_GUIDELINE(item), color);

    g_signal_connect(G_OBJECT(item), "event", G_CALLBACK(handler), this);

    views.push_back(SP_GUIDELINE(item));
}

void SPGuide::showSPGuide()
{
    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        sp_canvas_item_show(SP_CANVAS_ITEM(*it));
        if((*it)->origin) {
          sp_canvas_item_show(SP_CANVAS_ITEM((*it)->origin));
        }  else {
            //reposition to same place to show knots
            sp_guideline_set_position(*it, point_on_line);
        }
    }
}

void SPGuide::hideSPGuide(SPCanvas *canvas)
{
    g_assert(canvas != NULL);
    g_assert(SP_IS_CANVAS(canvas));
    for(std::vector<SPGuideLine *>::iterator it = this->views.begin(); it != this->views.end(); ++it) {
        if (canvas == SP_CANVAS_ITEM(*it)->canvas) {
            sp_guideline_delete(*it);
            views.erase(it);
            return;
        }
    }

    assert(false);
}

void SPGuide::hideSPGuide()
{
    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        sp_canvas_item_hide(SP_CANVAS_ITEM(*it));
        if ((*it)->origin) {
            sp_canvas_item_hide(SP_CANVAS_ITEM((*it)->origin));
        }
    }
}

void SPGuide::sensitize(SPCanvas *canvas, bool sensitive)
{
    g_assert(canvas != NULL);
    g_assert(SP_IS_CANVAS(canvas));

    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        if (canvas == SP_CANVAS_ITEM(*it)->canvas) {
            sp_guideline_set_sensitive(*it, sensitive);
            return;
        }
    }

    assert(false);
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
void SPGuide::moveto(Geom::Point const point_on_line, bool const commit)
{
    if(this->locked) {
        return;
    }
    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        sp_guideline_set_position(*it, point_on_line);
    }

    /* Calling sp_repr_set_point must precede calling sp_item_notify_moveto in the commit
       case, so that the guide's new position is available for sp_item_rm_unsatisfied_cns. */
    if (commit) {
        // If root viewBox set, interpret guides in terms of viewBox (90/96)
        double newx = point_on_line.x();
        double newy = point_on_line.y();

        SPRoot *root = document->getRoot();
        if( root->viewBox_set ) {
            // check to see if scaling is uniform
            if(Geom::are_near((root->viewBox.width() * root->height.computed) / (root->width.computed * root->viewBox.height()), 1.0, Geom::EPSILON)) {
                double px2vbunit = (root->viewBox.width()/root->width.computed + root->viewBox.height()/root->height.computed)/2.0;
                newx = newx * px2vbunit;
                newy = newy * px2vbunit;
            } else {
                newx = newx * root->viewBox.width()  / root->width.computed;
                newy = newy * root->viewBox.height() / root->height.computed;
            }
        }

        //XML Tree being used here directly while it shouldn't be.
        sp_repr_set_point(getRepr(), "position", Geom::Point(newx, newy) );
    }

/*  DISABLED CODE BECAUSE  SPGuideAttachment  IS NOT USE AT THE MOMENT (johan)
    for (vector<SPGuideAttachment>::const_iterator i(attached_items.begin()),
             iEnd(attached_items.end());
         i != iEnd; ++i)
    {
        SPGuideAttachment const &att = *i;
        sp_item_notify_moveto(*att.item, this, att.snappoint_ix, position, commit);
    }
*/
}

/**
 * \arg commit False indicates temporary moveto in response to motion event while dragging,
 *      true indicates a "committing" version: in response to button release event after
 *      dragging a guideline, or clicking OK in guide editing dialog.
 */
void SPGuide::set_normal(Geom::Point const normal_to_line, bool const commit)
{
    if(this->locked) {
        return;
    }
    for(std::vector<SPGuideLine *>::const_iterator it = this->views.begin(); it != this->views.end(); ++it) {
        sp_guideline_set_normal(*it, normal_to_line);
    }

    /* Calling sp_repr_set_svg_point must precede calling sp_item_notify_moveto in the commit
       case, so that the guide's new position is available for sp_item_rm_unsatisfied_cns. */
    if (commit) {
        //XML Tree being used directly while it shouldn't be
        sp_repr_set_point(getRepr(), "orientation", normal_to_line);
    }

/*  DISABLED CODE BECAUSE  SPGuideAttachment  IS NOT USE AT THE MOMENT (johan)
    for (vector<SPGuideAttachment>::const_iterator i(attached_items.begin()),
             iEnd(attached_items.end());
         i != iEnd; ++i)
    {
        SPGuideAttachment const &att = *i;
        sp_item_notify_moveto(*att.item, this, att.snappoint_ix, position, commit);
    }
*/
}

void SPGuide::set_color(const unsigned r, const unsigned g, const unsigned b, bool const commit)
{
    this->color = (r << 24) | (g << 16) | (b << 8) | 0x7f;

    if (! views.empty()) {
        sp_guideline_set_color(views[0], this->color);
    }

    if (commit) {
        std::ostringstream os;
        os << "rgb(" << r << "," << g << "," << b << ")";
        //XML Tree being used directly while it shouldn't be
        getRepr()->setAttribute("inkscape:color", os.str().c_str());
    }
}

void SPGuide::set_locked(const bool locked, bool const commit)
{
    this->locked = locked;
    if ( !views.empty() ) {
        sp_guideline_set_locked(views[0], locked);
    }

    if (commit) {
        getRepr()->setAttribute("inkscape:locked", g_strdup(locked ? "true" : "false"));
    }
}

void SPGuide::set_label(const char* label, bool const commit)
{
    if (!views.empty()) {
        sp_guideline_set_label(views[0], label);
    }

    if (commit) {
        //XML Tree being used directly while it shouldn't be
        getRepr()->setAttribute("inkscape:label", label);
    }
}

/**
 * Returns a human-readable description of the guideline for use in dialog boxes and status bar.
 * If verbose is false, only positioning information is included (useful for dialogs).
 *
 * The caller is responsible for freeing the string.
 */
char* SPGuide::description(bool const verbose) const
{
    using Geom::X;
    using Geom::Y;

    char *descr = NULL;
    if ( !this->document ) {
        // Guide has probably been deleted and no longer has an attached namedview.
        descr = g_strdup(_("Deleted"));
    } else {
        SPNamedView *namedview = sp_document_namedview(this->document, NULL);

        Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(this->point_on_line[X], "px");
        Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(this->point_on_line[Y], "px");
        GString *position_string_x = g_string_new(x_q.string(namedview->display_units).c_str());
        GString *position_string_y = g_string_new(y_q.string(namedview->display_units).c_str());

        gchar *shortcuts = g_strdup_printf("; %s", _("<b>Shift+drag</b> to rotate, <b>Ctrl+drag</b> to move origin, <b>Del</b> to delete"));

        if ( are_near(this->normal_to_line, Geom::Point(1., 0.)) ||
             are_near(this->normal_to_line, -Geom::Point(1., 0.)) ) {
            descr = g_strdup_printf(_("vertical, at %s"), position_string_x->str);
        } else if ( are_near(this->normal_to_line, Geom::Point(0., 1.)) ||
                    are_near(this->normal_to_line, -Geom::Point(0., 1.)) ) {
            descr = g_strdup_printf(_("horizontal, at %s"), position_string_y->str);
        } else {
            double const radians = this->angle();
            double const degrees = Geom::deg_from_rad(radians);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

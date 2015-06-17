/*
 * Gradient drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"

#include "message-context.h"
#include "message-stack.h"
#include "pixmaps/cursor-gradient.xpm"
#include "pixmaps/cursor-gradient-add.xpm"
#include "ui/tools/gradient-tool.h"
#include "gradient-chemistry.h"
#include <glibmm/i18n.h>
#include "preferences.h"
#include "gradient-drag.h"
#include "gradient-chemistry.h"
#include "xml/repr.h"
#include "sp-item.h"
#include "display/sp-ctrlline.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-stop.h"
#include "svg/css-ostringstream.h"
#include "svg/svg-color.h"
#include "snap.h"
#include "sp-namedview.h"
#include "rubberband.h"
#include "document-undo.h"
#include "verbs.h"
#include "selection-chemistry.h"

using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Tools {

static void sp_gradient_drag(GradientTool &rc, Geom::Point const pt, guint state, guint32 etime);

const std::string& GradientTool::getPrefsPath() {
	return GradientTool::prefsPath;
}

const std::string GradientTool::prefsPath = "/tools/gradient";


GradientTool::GradientTool()
    : ToolBase(cursor_gradient_xpm, 4, 4)
    , cursor_addnode(false)
    , node_added(false)
// TODO: Why are these connections stored as pointers?
    , selcon(NULL)
    , subselcon(NULL)
{
	// TODO: This value is overwritten in the root handler
    this->tolerance = 6;
}

GradientTool::~GradientTool() {
    this->enableGrDrag(false);

    this->selcon->disconnect();
    delete this->selcon;

    this->subselcon->disconnect();
    delete this->subselcon;
}

const gchar *gr_handle_descr [] = {
    N_("Linear gradient <b>start</b>"), //POINT_LG_BEGIN
    N_("Linear gradient <b>end</b>"),
    N_("Linear gradient <b>mid stop</b>"),
    N_("Radial gradient <b>center</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>focus</b>"), // POINT_RG_FOCUS
    N_("Radial gradient <b>mid stop</b>"),
    N_("Radial gradient <b>mid stop</b>")
};

void GradientTool::selection_changed(Inkscape::Selection*) {
    GradientTool *rc = (GradientTool *) this;

    GrDrag *drag = rc->_grdrag;
    Inkscape::Selection *selection = this->desktop->getSelection();
    if (selection == NULL) {
        return;
    }
    guint n_obj = selection->itemList().size();

    if (!drag->isNonEmpty() || selection->isEmpty())
        return;
    guint n_tot = drag->numDraggers();
    guint n_sel = drag->numSelected();

    //The use of ngettext in the following code is intentional even if the English singular form would never be used
    if (n_sel == 1) {
        if (drag->singleSelectedDraggerNumDraggables() == 1) {
            gchar * message = g_strconcat(
                //TRANSLATORS: %s will be substituted with the point name (see previous messages); This is part of a compound message
                _("%s selected"),
                //TRANSLATORS: Mind the space in front. This is part of a compound message
                ngettext(" out of %d gradient handle"," out of %d gradient handles",n_tot),
                ngettext(" on %d selected object"," on %d selected objects",n_obj),NULL);
            rc->message_context->setF(Inkscape::NORMAL_MESSAGE,
                                       message,_(gr_handle_descr[drag->singleSelectedDraggerSingleDraggableType()]), n_tot, n_obj);
        } else {
            gchar * message = g_strconcat(
                //TRANSLATORS: This is a part of a compound message (out of two more indicating: grandint handle count & object count)
                ngettext("One handle merging %d stop (drag with <b>Shift</b> to separate) selected",
                         "One handle merging %d stops (drag with <b>Shift</b> to separate) selected",drag->singleSelectedDraggerNumDraggables()),
                ngettext(" out of %d gradient handle"," out of %d gradient handles",n_tot),
                ngettext(" on %d selected object"," on %d selected objects",n_obj),NULL);
            rc->message_context->setF(Inkscape::NORMAL_MESSAGE,message,drag->singleSelectedDraggerNumDraggables(), n_tot, n_obj);
        }
    } else if (n_sel > 1) {
        //TRANSLATORS: The plural refers to number of selected gradient handles. This is part of a compound message (part two indicates selected object count)
        gchar * message = g_strconcat(ngettext("<b>%d</b> gradient handle selected out of %d","<b>%d</b> gradient handles selected out of %d",n_sel),
                                      //TRANSLATORS: Mind the space in front. (Refers to gradient handles selected). This is part of a compound message
                                      ngettext(" on %d selected object"," on %d selected objects",n_obj),NULL);
        rc->message_context->setF(Inkscape::NORMAL_MESSAGE,message, n_sel, n_tot, n_obj);
    } else if (n_sel == 0) {
        rc->message_context->setF(Inkscape::NORMAL_MESSAGE,
                                   //TRANSLATORS: The plural refers to number of selected objects
                                   ngettext("<b>No</b> gradient handles selected out of %d on %d selected object",
                                            "<b>No</b> gradient handles selected out of %d on %d selected objects",n_obj), n_tot, n_obj);
    }
}

void GradientTool::setup() {
    ToolBase::setup();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    
    if (prefs->getBool("/tools/gradient/selcue", true)) {
        this->enableSelectionCue();
    }

    this->enableGrDrag();
    Inkscape::Selection *selection = this->desktop->getSelection();

    this->selcon = new sigc::connection(selection->connectChanged(
    	sigc::mem_fun(this, &GradientTool::selection_changed)
    ));

    this->subselcon = new sigc::connection(this->desktop->connectToolSubselectionChanged(
    	sigc::hide(sigc::bind(
    		sigc::mem_fun(this, &GradientTool::selection_changed),
    		(Inkscape::Selection*)NULL
    	))
    ));

    this->selection_changed(selection);
}

void
sp_gradient_context_select_next (ToolBase *event_context)
{
    GrDrag *drag = event_context->_grdrag;
    g_assert (drag);

    GrDragger *d = drag->select_next();

    event_context->desktop->scroll_to_point(d->point, 1.0);
}

void
sp_gradient_context_select_prev (ToolBase *event_context)
{
    GrDrag *drag = event_context->_grdrag;
    g_assert (drag);

    GrDragger *d = drag->select_prev();

    event_context->desktop->scroll_to_point(d->point, 1.0);
}

static bool
sp_gradient_context_is_over_line (GradientTool *rc, SPItem *item, Geom::Point event_p)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT (rc)->desktop;

    //Translate mouse point into proper coord system
    rc->mousepoint_doc = desktop->w2d(event_p);

    if (SP_IS_CTRLLINE(item)) {
        SPCtrlLine* line = SP_CTRLLINE(item);

        Geom::LineSegment ls(line->s, line->e);
        Geom::Point nearest = ls.pointAt(ls.nearestPoint(rc->mousepoint_doc));
        double dist_screen = Geom::L2 (rc->mousepoint_doc - nearest) * desktop->current_zoom();

        double tolerance = (double) SP_EVENT_CONTEXT(rc)->tolerance;

        bool close = (dist_screen < tolerance);

        return close;
    }
    return false;
}

static std::vector<Geom::Point>
sp_gradient_context_get_stop_intervals (GrDrag *drag, GSList **these_stops, GSList **next_stops)
{
    std::vector<Geom::Point> coords;

    // for all selected draggers
    for (GList *i = drag->selected; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        // remember the coord of the dragger to reselect it later
        coords.push_back(dragger->point);
        // for all draggables of dragger
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {
            GrDraggable *d = (GrDraggable *) j->data;

            // find the gradient
            SPGradient *gradient = getGradient(d->item, d->fill_or_stroke);
            SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (gradient, false);

            // these draggable types cannot have a next draggabe to insert a stop between them
            if (d->point_type == POINT_LG_END ||
                d->point_type == POINT_RG_FOCUS ||
                d->point_type == POINT_RG_R1 ||
                d->point_type == POINT_RG_R2) {
                continue;
            }

            // from draggables to stops
            SPStop *this_stop = sp_get_stop_i (vector, d->point_i);
            SPStop *next_stop = this_stop->getNextStop();
            SPStop *last_stop = sp_last_stop (vector);

            Inkscape::PaintTarget fs = d->fill_or_stroke;
            SPItem *item = d->item;
            gint type = d->point_type;
            gint p_i = d->point_i;

            // if there's a next stop,
            if (next_stop) {
                GrDragger *dnext = NULL;
                // find its dragger
                // (complex because it may have different types, and because in radial,
                // more than one dragger may correspond to a stop, so we must distinguish)
                if (type == POINT_LG_BEGIN || type == POINT_LG_MID) {
                    if (next_stop == last_stop) {
                        dnext = drag->getDraggerFor(item, POINT_LG_END, p_i+1, fs);
                    } else {
                        dnext = drag->getDraggerFor(item, POINT_LG_MID, p_i+1, fs);
                    }
                } else { // radial
                    if (type == POINT_RG_CENTER || type == POINT_RG_MID1) {
                        if (next_stop == last_stop) {
                            dnext = drag->getDraggerFor(item, POINT_RG_R1, p_i+1, fs);
                        } else {
                            dnext = drag->getDraggerFor(item, POINT_RG_MID1, p_i+1, fs);
                        }
                    }
                    if ((type == POINT_RG_MID2) ||
                        (type == POINT_RG_CENTER && dnext && !dnext->isSelected())) {
                        if (next_stop == last_stop) {
                            dnext = drag->getDraggerFor(item, POINT_RG_R2, p_i+1, fs);
                        } else {
                            dnext = drag->getDraggerFor(item, POINT_RG_MID2, p_i+1, fs);
                        }
                    }
                }

                // if both adjacent draggers selected,
                if (!g_slist_find(*these_stops, this_stop) && dnext && dnext->isSelected()) {

                    // remember the coords of the future dragger to select it
                    coords.push_back(0.5*(dragger->point + dnext->point));

                    // do not insert a stop now, it will confuse the loop;
                    // just remember the stops
                    *these_stops = g_slist_prepend (*these_stops, this_stop);
                    *next_stops = g_slist_prepend (*next_stops, next_stop);
                }
            }
        }
    }
    return coords;
}

void
sp_gradient_context_add_stops_between_selected_stops (GradientTool *rc)
{
    SPDocument *doc = NULL;
    GrDrag *drag = rc->_grdrag;

    GSList *these_stops = NULL;
    GSList *next_stops = NULL;

    std::vector<Geom::Point> coords = sp_gradient_context_get_stop_intervals (drag, &these_stops, &next_stops);

    if (g_slist_length(these_stops) == 0 && drag->numSelected() == 1) {
        // if a single stop is selected, add between that stop and the next one
        GrDragger *dragger = (GrDragger *) drag->selected->data;
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {
            GrDraggable *d = (GrDraggable *) j->data;
            if (d->point_type == POINT_RG_FOCUS) {
                /*
                 *  There are 2 draggables at the center (start) of a radial gradient
                 *  To avoid creating 2 separate stops, ignore this draggable point type
                 */
                continue;
            }
            SPGradient *gradient = getGradient(d->item, d->fill_or_stroke);
            SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (gradient, false);
            SPStop *this_stop = sp_get_stop_i (vector, d->point_i);
            if (this_stop) {
                SPStop *next_stop = this_stop->getNextStop();
                if (next_stop) {
                    these_stops = g_slist_prepend (these_stops, this_stop);
                    next_stops = g_slist_prepend (next_stops, next_stop);
                }
            }
        }
    }

    // now actually create the new stops
    GSList *i = these_stops;
    GSList *j = next_stops;
    GSList *new_stops = NULL;

    for (; i != NULL && j != NULL; i = i->next, j = j->next) {
        SPStop *this_stop = (SPStop *) i->data;
        SPStop *next_stop = (SPStop *) j->data;
        gfloat offset = 0.5*(this_stop->offset + next_stop->offset);
        SPObject *parent = this_stop->parent;
        if (SP_IS_GRADIENT (parent)) {
            doc = parent->document;
            SPStop *new_stop = sp_vector_add_stop (SP_GRADIENT (parent), this_stop, next_stop, offset);
            new_stops = g_slist_prepend (new_stops, new_stop);
            SP_GRADIENT(parent)->ensureVector();
        }
    }

    if (g_slist_length(these_stops) > 0 && doc) {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_GRADIENT, _("Add gradient stop"));
        drag->updateDraggers();
        // so that it does not automatically update draggers in idle loop, as this would deselect
        drag->local_change = true;

        // select the newly created stops
        for (GSList *s = new_stops; s != NULL; s = s->next) {
            drag->selectByStop((SPStop *)s->data);
        }

    }

    g_slist_free (these_stops);
    g_slist_free (next_stops);
    g_slist_free (new_stops);
}

static double sqr(double x) {return x*x;}

static void
sp_gradient_simplify(GradientTool *rc, double tolerance)
{
    SPDocument *doc = NULL;
    GrDrag *drag = rc->_grdrag;

    GSList *these_stops = NULL;
    GSList *next_stops = NULL;

    std::vector<Geom::Point> coords = sp_gradient_context_get_stop_intervals (drag, &these_stops, &next_stops);

    GSList *todel = NULL;

    GSList *i = these_stops;
    GSList *j = next_stops;
    for (; i != NULL && j != NULL; i = i->next, j = j->next) {
        SPStop *stop0 = (SPStop *) i->data;
        SPStop *stop1 = (SPStop *) j->data;

        gint i1 = g_slist_index(these_stops, stop1);
        if (i1 != -1) {
            GSList *next_next = g_slist_nth (next_stops, i1);
            if (next_next) {
                SPStop *stop2 = (SPStop *) next_next->data;

                if (g_slist_find(todel, stop0) || g_slist_find(todel, stop2))
                    continue;

                guint32 const c0 = stop0->get_rgba32();
                guint32 const c2 = stop2->get_rgba32();
                guint32 const c1r = stop1->get_rgba32();
                guint32 c1 = average_color (c0, c2,
                       (stop1->offset - stop0->offset) / (stop2->offset - stop0->offset));

                double diff =
                    sqr(SP_RGBA32_R_F(c1) - SP_RGBA32_R_F(c1r)) +
                    sqr(SP_RGBA32_G_F(c1) - SP_RGBA32_G_F(c1r)) +
                    sqr(SP_RGBA32_B_F(c1) - SP_RGBA32_B_F(c1r)) +
                    sqr(SP_RGBA32_A_F(c1) - SP_RGBA32_A_F(c1r));

                if (diff < tolerance)
                    todel = g_slist_prepend (todel, stop1);
            }
        }
    }

    for (i = todel; i != NULL; i = i->next) {
        SPStop *stop = (SPStop*) i->data;
        doc = stop->document;
        Inkscape::XML::Node * parent = stop->getRepr()->parent();
        parent->removeChild( stop->getRepr() );
    }

    if (g_slist_length(todel) > 0) {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_GRADIENT, _("Simplify gradient"));
        drag->local_change = true;
        drag->updateDraggers();
        drag->selectByCoords(coords);
    }

    g_slist_free (todel);
    g_slist_free (these_stops);
    g_slist_free (next_stops);
}


static void
sp_gradient_context_add_stop_near_point (GradientTool *rc, SPItem *item,  Geom::Point mouse_p, guint32 /*etime*/)
{
    // item is the selected item. mouse_p the location in doc coordinates of where to add the stop

    ToolBase *ec = SP_EVENT_CONTEXT(rc);
    SPDesktop *desktop = SP_EVENT_CONTEXT (rc)->desktop;

    double tolerance = (double) ec->tolerance;

    SPStop *newstop = ec->get_drag()->addStopNearPoint (item, mouse_p, tolerance/desktop->current_zoom());

    DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_GRADIENT,
                       _("Add gradient stop"));

    ec->get_drag()->updateDraggers();
    ec->get_drag()->local_change = true;
    ec->get_drag()->selectByStop(newstop);
}

bool GradientTool::root_handler(GdkEvent* event) {
    static bool dragging;

    Inkscape::Selection *selection = desktop->getSelection();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    this->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
    double const nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000, "px"); // in px

    GrDrag *drag = this->_grdrag;
    g_assert (drag);

    gint ret = FALSE;

    switch (event->type) {
    case GDK_2BUTTON_PRESS:
        if ( event->button.button == 1 ) {
            bool over_line = false;
            SPCtrlLine *line = NULL;

            if (drag->lines) {
                for (GSList *l = drag->lines; (l != NULL) && (!over_line); l = l->next) {
                    line = (SPCtrlLine*) l->data;
                    over_line |= sp_gradient_context_is_over_line (this, (SPItem*) line, Geom::Point(event->motion.x, event->motion.y));
                }
            }

            if (over_line) {
                // we take the first item in selection, because with doubleclick, the first click
                // always resets selection to the single object under cursor
                sp_gradient_context_add_stop_near_point(this, SP_ITEM(selection->itemList().front()), this->mousepoint_doc, event->button.time);
            } else {
            	std::vector<SPItem*>  items=selection->itemList();
                for (std::vector<SPItem*>::const_iterator i = items.begin();i!=items.end();i++) {
                    SPItem *item = *i;
                    SPGradientType new_type = (SPGradientType) prefs->getInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR);
                    Inkscape::PaintTarget fsmode = (prefs->getInt("/tools/gradient/newfillorstroke", 1) != 0) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE;

                    SPGradient *vector = sp_gradient_vector_for_object(desktop->getDocument(), desktop, item, fsmode);

                    SPGradient *priv = sp_item_set_gradient(item, vector, new_type, fsmode);
                    sp_gradient_reset_to_userspace(priv, item);
                }

                DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_GRADIENT,
                                   _("Create default gradient"));
            }
            ret = TRUE;
        }
        break;

    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 && !this->space_panning ) {
            Geom::Point button_w(event->button.x, event->button.y);

            // save drag origin
            this->xp = (gint) button_w[Geom::X];
            this->yp = (gint) button_w[Geom::Y];
            this->within_tolerance = true;

            dragging = true;

            Geom::Point button_dt = desktop->w2d(button_w);
            if (event->button.state & GDK_SHIFT_MASK) {
                Inkscape::Rubberband::get(desktop)->start(desktop, button_dt);
            } else {
                // remember clicked item, disregarding groups, honoring Alt; do nothing with Crtl to
                // enable Ctrl+doubleclick of exactly the selected item(s)
                if (!(event->button.state & GDK_CONTROL_MASK)) {
                    this->item_to_select = sp_event_context_find_item (desktop, button_w, event->button.state & GDK_MOD1_MASK, TRUE);
                }

                if (!selection->isEmpty()) {
                    SnapManager &m = desktop->namedview->snap_manager;
                    m.setup(desktop);
                    m.freeSnapReturnByRef(button_dt, Inkscape::SNAPSOURCE_NODE_HANDLE);
                    m.unSetup();
                }

                this->origin = button_dt;
            }

            ret = TRUE;
        }
        break;

    case GDK_MOTION_NOTIFY:
        if (dragging && ( event->motion.state & GDK_BUTTON1_MASK ) && !this->space_panning) {
            if ( this->within_tolerance
                 && ( abs( (gint) event->motion.x - this->xp ) < this->tolerance )
                 && ( abs( (gint) event->motion.y - this->yp ) < this->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            this->within_tolerance = false;

            Geom::Point const motion_w(event->motion.x,
                                     event->motion.y);
            Geom::Point const motion_dt = this->desktop->w2d(motion_w);

            if (Inkscape::Rubberband::get(desktop)->is_started()) {
                Inkscape::Rubberband::get(desktop)->move(motion_dt);
                this->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Draw around</b> handles to select them"));
            } else {
                sp_gradient_drag(*this, motion_dt, event->motion.state, event->motion.time);
            }

            gobble_motion_events(GDK_BUTTON1_MASK);

            ret = TRUE;
        } else {
            if (!drag->mouseOver() && !selection->isEmpty()) {
                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const motion_dt = this->desktop->w2d(motion_w);

                m.preSnap(Inkscape::SnapCandidatePoint(motion_dt, Inkscape::SNAPSOURCE_OTHER_HANDLE));
                m.unSetup();
            }

            bool over_line = false;

            if (drag->lines) {
                for (GSList *l = drag->lines; l != NULL; l = l->next) {
                    over_line |= sp_gradient_context_is_over_line (this, (SPItem*) l->data, Geom::Point(event->motion.x, event->motion.y));
                }
            }

            if (this->cursor_addnode && !over_line) {
                this->cursor_shape = cursor_gradient_xpm;
                this->sp_event_context_update_cursor();
                this->cursor_addnode = false;
            } else if (!this->cursor_addnode && over_line) {
                this->cursor_shape = cursor_gradient_add_xpm;
                this->sp_event_context_update_cursor();
                this->cursor_addnode = true;
            }
        }
        break;

    case GDK_BUTTON_RELEASE:
        this->xp = this->yp = 0;

        if ( event->button.button == 1 && !this->space_panning ) {
            bool over_line = false;
            SPCtrlLine *line = NULL;

            if (drag->lines) {
                for (GSList *l = drag->lines; (l != NULL) && (!over_line); l = l->next) {
                    line = (SPCtrlLine*) l->data;
                    over_line = sp_gradient_context_is_over_line (this, (SPItem*) line, Geom::Point(event->motion.x, event->motion.y));
                    if (over_line)
                        break;
                }
            }

            if ( (event->button.state & GDK_CONTROL_MASK) && (event->button.state & GDK_MOD1_MASK ) ) {
                if (over_line && line) {
                    sp_gradient_context_add_stop_near_point(this, line->item, this->mousepoint_doc, 0);
                    ret = TRUE;
                }
            } else {
                dragging = false;

                // unless clicked with Ctrl (to enable Ctrl+doubleclick).
                if (event->button.state & GDK_CONTROL_MASK) {
                    ret = TRUE;
                    break;
                }

                if (!this->within_tolerance) {
                    // we've been dragging, either do nothing (grdrag handles that),
                    // or rubberband-select if we have rubberband
                    Inkscape::Rubberband *r = Inkscape::Rubberband::get(desktop);

                    if (r->is_started() && !this->within_tolerance) {
                        // this was a rubberband drag
                        if (r->getMode() == RUBBERBAND_MODE_RECT) {
                            Geom::OptRect const b = r->getRectangle();
                            drag->selectRect(*b);
                        }
                    }
                } else if (this->item_to_select) {
                    if (over_line && line) {
                        // Clicked on an existing gradient line, dont change selection. This stops
                        // possible change in selection during a double click with overlapping objects
                    } else {
                        // no dragging, select clicked item if any
                        if (event->button.state & GDK_SHIFT_MASK) {
                            selection->toggle(this->item_to_select);
                        } else {
                            drag->deselectAll();
                            selection->set(this->item_to_select);
                        }
                    }
                } else {
                    // click in an empty space; do the same as Esc
                    if (drag->selected) {
                        drag->deselectAll();
                    } else {
                        selection->clear();
                    }
                }

                this->item_to_select = NULL;
                ret = TRUE;
            }

            Inkscape::Rubberband::get(desktop)->stop();
        }
        break;

    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
        case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_KEY_Meta_R:
            sp_event_show_modifier_tip (this->defaultMessageContext(), event,
                                        _("<b>Ctrl</b>: snap gradient angle"),
                                        _("<b>Shift</b>: draw gradient around the starting point"),
                                        NULL);
            break;

        case GDK_KEY_x:
        case GDK_KEY_X:
            if (MOD__ALT_ONLY(event)) {
                desktop->setToolboxFocusTo ("altx-grad");
                ret = TRUE;
            }
            break;

        case GDK_KEY_A:
        case GDK_KEY_a:
            if (MOD__CTRL_ONLY(event) && drag->isNonEmpty()) {
                drag->selectAll();
                ret = TRUE;
            }
            break;

        case GDK_KEY_L:
        case GDK_KEY_l:
            if (MOD__CTRL_ONLY(event) && drag->isNonEmpty() && drag->hasSelection()) {
                sp_gradient_simplify(this, 1e-4);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Escape:
            if (drag->selected) {
                drag->deselectAll();
            } else {
                Inkscape::SelectionHelper::selectNone(desktop);
            }
            ret = TRUE;
            //TODO: make dragging escapable by Esc
            break;

        case GDK_KEY_Left: // move handle left
        case GDK_KEY_KP_Left:
        case GDK_KEY_KP_4:
            if (!MOD__CTRL(event)) { // not ctrl
                gint mul = 1 + gobble_key_events(
                    get_group0_keyval(&event->key), 0); // with any mask
                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move_screen(mul*-10, 0); // shift
                    } else {
                    	drag->selected_move_screen(mul*-1, 0); // no shift
                    }
                } else { // no alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move(mul*-10*nudge, 0); // shift
                    } else {
                    	drag->selected_move(mul*-nudge, 0); // no shift
                    }
                }
                ret = TRUE;
            }
            break;

        case GDK_KEY_Up: // move handle up
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_8:
            if (!MOD__CTRL(event)) { // not ctrl
                gint mul = 1 + gobble_key_events(
                    get_group0_keyval(&event->key), 0); // with any mask
                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move_screen(0, mul*10); // shift
                    } else {
                    	drag->selected_move_screen(0, mul*1); // no shift
                    }
                } else { // no alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move(0, mul*10*nudge); // shift
                    } else {
                    	drag->selected_move(0, mul*nudge); // no shift
                    }
                }

                ret = TRUE;
            }
            break;

        case GDK_KEY_Right: // move handle right
        case GDK_KEY_KP_Right:
        case GDK_KEY_KP_6:
            if (!MOD__CTRL(event)) { // not ctrl
                gint mul = 1 + gobble_key_events(
                    get_group0_keyval(&event->key), 0); // with any mask

                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move_screen(mul*10, 0); // shift
                    } else {
                    	drag->selected_move_screen(mul*1, 0); // no shift
                    }
                } else { // no alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move(mul*10*nudge, 0); // shift
                    } else {
                    	drag->selected_move(mul*nudge, 0); // no shift
                    }
                }

                ret = TRUE;
            }
            break;

        case GDK_KEY_Down: // move handle down
        case GDK_KEY_KP_Down:
        case GDK_KEY_KP_2:
            if (!MOD__CTRL(event)) { // not ctrl
                gint mul = 1 + gobble_key_events(
                    get_group0_keyval(&event->key), 0); // with any mask

                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move_screen(0, mul*-10); // shift
                    } else {
                    	drag->selected_move_screen(0, mul*-1); // no shift
                    }
                } else { // no alt
                    if (MOD__SHIFT(event)) {
                    	drag->selected_move(0, mul*-10*nudge); // shift
                    } else {
                    	drag->selected_move(0, mul*-nudge); // no shift
                    }
                }

                ret = TRUE;
            }
            break;

        case GDK_KEY_r:
        case GDK_KEY_R:
            if (MOD__SHIFT_ONLY(event)) {
                sp_gradient_reverse_selected_gradients(desktop);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Insert:
        case GDK_KEY_KP_Insert:
            // with any modifiers:
            sp_gradient_context_add_stops_between_selected_stops (this);
            ret = TRUE;
            break;

        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
        case GDK_KEY_BackSpace:
            ret = this->deleteSelectedDrag(MOD__CTRL_ONLY(event));
            break;

        default:
            break;
        }
        break;

    case GDK_KEY_RELEASE:
        switch (get_group0_keyval (&event->key)) {
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
        case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt
        case GDK_KEY_Meta_R:
            this->defaultMessageContext()->clear();
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    if (!ret) {
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}

static void sp_gradient_drag(GradientTool &rc, Geom::Point const pt, guint /*state*/, guint32 etime)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(&rc)->desktop;
    Inkscape::Selection *selection = desktop->getSelection();
    SPDocument *document = desktop->getDocument();
    ToolBase *ec = SP_EVENT_CONTEXT(&rc);

    if (!selection->isEmpty()) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int type = prefs->getInt("/tools/gradient/newgradient", 1);
        Inkscape::PaintTarget fill_or_stroke = (prefs->getInt("/tools/gradient/newfillorstroke", 1) != 0) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE;

        SPGradient *vector;
        if (ec->item_to_select) {
            // pick color from the object where drag started
            vector = sp_gradient_vector_for_object(document, desktop, ec->item_to_select, fill_or_stroke);
        } else {
            // Starting from empty space:
            // Sort items so that the topmost comes last
        	std::vector<SPItem*> items(selection->itemList());
            sort(items.begin(),items.end(),sp_item_repr_compare_position);
            // take topmost
            vector = sp_gradient_vector_for_object(document, desktop, SP_ITEM(items.back()), fill_or_stroke);
        }

        // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property(css, "fill-opacity", "1.0");

        std::vector<SPItem*> itemlist = selection->itemList();
        for (std::vector<SPItem*>::const_iterator i = itemlist.begin();i!=itemlist.end();i++) {

            //FIXME: see above
            sp_repr_css_change_recursive((*i)->getRepr(), css, "style");

            sp_item_set_gradient(*i, vector, (SPGradientType) type, fill_or_stroke);

            if (type == SP_GRADIENT_TYPE_LINEAR) {
                sp_item_gradient_set_coords (*i, POINT_LG_BEGIN, 0, rc.origin, fill_or_stroke, true, false);
                sp_item_gradient_set_coords (*i, POINT_LG_END, 0, pt, fill_or_stroke, true, false);
            } else if (type == SP_GRADIENT_TYPE_RADIAL) {
                sp_item_gradient_set_coords (*i, POINT_RG_CENTER, 0, rc.origin, fill_or_stroke, true, false);
                sp_item_gradient_set_coords (*i, POINT_RG_R1, 0, pt, fill_or_stroke, true, false);
            }
            (*i)->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        if (ec->_grdrag) {
            ec->_grdrag->updateDraggers();
            // prevent regenerating draggers by selection modified signal, which sometimes
            // comes too late and thus destroys the knot which we will now grab:
            ec->_grdrag->local_change = true;
            // give the grab out-of-bounds values of xp/yp because we're already dragging
            // and therefore are already out of tolerance
            ec->_grdrag->grabKnot (selection->itemList()[0],
                                   type == SP_GRADIENT_TYPE_LINEAR? POINT_LG_END : POINT_RG_R1,
                                   -1, // ignore number (though it is always 1)
                                   fill_or_stroke, 99999, 99999, etime);
        }
        // We did an undoable action, but SPDocumentUndo::done will be called by the knot when released

        // status text; we do not track coords because this branch is run once, not all the time
        // during drag
        int n_objects = selection->itemList().size();
        rc.message_context->setF(Inkscape::NORMAL_MESSAGE,
                                  ngettext("<b>Gradient</b> for %d object; with <b>Ctrl</b> to snap angle",
                                           "<b>Gradient</b> for %d objects; with <b>Ctrl</b> to snap angle", n_objects),
                                  n_objects);
    } else {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>objects</b> on which to create gradient."));
    }
}

}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

/*
 * tweaking paths without node editing
 *
 * Authors:
 *   bulia byak
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

#include <numeric>

#include "svg/svg.h"

#include <glib.h>
#include "macros.h"
#include "document.h"
#include "document-undo.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-tweak-move.xpm"
#include "pixmaps/cursor-tweak-move-in.xpm"
#include "pixmaps/cursor-tweak-move-out.xpm"
#include "pixmaps/cursor-tweak-move-jitter.xpm"
#include "pixmaps/cursor-tweak-scale-up.xpm"
#include "pixmaps/cursor-tweak-scale-down.xpm"
#include "pixmaps/cursor-tweak-rotate-clockwise.xpm"
#include "pixmaps/cursor-tweak-rotate-counterclockwise.xpm"
#include "pixmaps/cursor-tweak-more.xpm"
#include "pixmaps/cursor-tweak-less.xpm"
#include "pixmaps/cursor-thin.xpm"
#include "pixmaps/cursor-thicken.xpm"
#include "pixmaps/cursor-attract.xpm"
#include "pixmaps/cursor-repel.xpm"
#include "pixmaps/cursor-push.xpm"
#include "pixmaps/cursor-roughen.xpm"
#include "pixmaps/cursor-color.xpm"
#include <boost/optional.hpp>
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"
#include "color.h"
#include "svg/svg-color.h"
#include "splivarot.h"
#include "sp-item-group.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "path-chemistry.h"
#include "sp-gradient.h"
#include "sp-stop.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "gradient-chemistry.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "display/sp-canvas.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "display/curve.h"
#include "livarot/Shape.h"
#include <2geom/transforms.h>
#include <2geom/circle.h>
#include "preferences.h"
#include "style.h"
#include "box3d.h"
#include "sp-item-transform.h"
#include "filter-chemistry.h"
#include "filters/gaussian-blur.h"
#include "verbs.h"

#include "ui/tools/tweak-tool.h"

using Inkscape::DocumentUndo;

#define DDC_RED_RGBA 0xff0000ff

#define DYNA_MIN_WIDTH 1.0e-6

#include "ui/tool-factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

namespace {
	ToolBase* createTweakContext() {
		return new TweakTool();
	}

	bool tweakContextRegistered = ToolFactory::instance().registerObject("/tools/tweak", createTweakContext);
}

const std::string& TweakTool::getPrefsPath() {
	return TweakTool::prefsPath;
}

const std::string TweakTool::prefsPath = "/tools/tweak";

TweakTool::TweakTool()
    : ToolBase(cursor_push_xpm, 4, 4)
    , pressure(TC_DEFAULT_PRESSURE)
    , dragging(false)
    , usepressure(false)
    , usetilt(false)
    , width(0.2)
    , force(0.2)
    , fidelity(0)
    , mode(0)
    , is_drawing(false)
    , is_dilating(false)
    , has_dilated(false)
    , dilate_area(NULL)
    , do_h(true)
    , do_s(true)
    , do_l(true)
    , do_o(false)
{
}

TweakTool::~TweakTool() {
    this->enableGrDrag(false);
    
    this->style_set_connection.disconnect();

    if (this->dilate_area) {
        sp_canvas_item_destroy(this->dilate_area);
        this->dilate_area = NULL;
    }
}

static bool is_transform_mode (gint mode)
{
    return (mode == TWEAK_MODE_MOVE || 
            mode == TWEAK_MODE_MOVE_IN_OUT || 
            mode == TWEAK_MODE_MOVE_JITTER || 
            mode == TWEAK_MODE_SCALE || 
            mode == TWEAK_MODE_ROTATE || 
            mode == TWEAK_MODE_MORELESS);
}

static bool is_color_mode (gint mode)
{
    return (mode == TWEAK_MODE_COLORPAINT || mode == TWEAK_MODE_COLORJITTER || mode == TWEAK_MODE_BLUR);
}

void TweakTool::update_cursor (bool with_shift) {
    guint num = 0;
    gchar *sel_message = NULL;

    if (!desktop->selection->isEmpty()) {
        num = g_slist_length(const_cast<GSList *>(desktop->selection->itemList()));
        sel_message = g_strdup_printf(ngettext("<b>%i</b> object selected","<b>%i</b> objects selected",num), num);
    } else {
        sel_message = g_strdup_printf("%s", _("<b>Nothing</b> selected"));
    }

   switch (this->mode) {
       case TWEAK_MODE_MOVE:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag to <b>move</b>."), sel_message);
           this->cursor_shape = cursor_tweak_move_xpm;
           break;
       case TWEAK_MODE_MOVE_IN_OUT:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>move in</b>; with Shift to <b>move out</b>."), sel_message);
           if (with_shift) {
               this->cursor_shape = cursor_tweak_move_out_xpm;
           } else {
               this->cursor_shape = cursor_tweak_move_in_xpm;
           }
           break;
       case TWEAK_MODE_MOVE_JITTER:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>move randomly</b>."), sel_message);
           this->cursor_shape = cursor_tweak_move_jitter_xpm;
           break;
       case TWEAK_MODE_SCALE:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>scale down</b>; with Shift to <b>scale up</b>."), sel_message);
           if (with_shift) {
               this->cursor_shape = cursor_tweak_scale_up_xpm;
           } else {
               this->cursor_shape = cursor_tweak_scale_down_xpm;
           }
           break;
       case TWEAK_MODE_ROTATE:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>rotate clockwise</b>; with Shift, <b>counterclockwise</b>."), sel_message);
           if (with_shift) {
               this->cursor_shape = cursor_tweak_rotate_counterclockwise_xpm;
           } else {
               this->cursor_shape = cursor_tweak_rotate_clockwise_xpm;
           }
           break;
       case TWEAK_MODE_MORELESS:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>duplicate</b>; with Shift, <b>delete</b>."), sel_message);
           if (with_shift) {
               this->cursor_shape = cursor_tweak_less_xpm;
           } else {
               this->cursor_shape = cursor_tweak_more_xpm;
           }
           break;
       case TWEAK_MODE_PUSH:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag to <b>push paths</b>."), sel_message);
           this->cursor_shape = cursor_push_xpm;
           break;
       case TWEAK_MODE_SHRINK_GROW:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>inset paths</b>; with Shift to <b>outset</b>."), sel_message);
           if (with_shift) {
               this->cursor_shape = cursor_thicken_xpm;
           } else {
               this->cursor_shape = cursor_thin_xpm;
           }
           break;
       case TWEAK_MODE_ATTRACT_REPEL:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>attract paths</b>; with Shift to <b>repel</b>."), sel_message);
           if (with_shift) {
               this->cursor_shape = cursor_repel_xpm;
           } else {
               this->cursor_shape = cursor_attract_xpm;
           }
           break;
       case TWEAK_MODE_ROUGHEN:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>roughen paths</b>."), sel_message);
           this->cursor_shape = cursor_roughen_xpm;
           break;
       case TWEAK_MODE_COLORPAINT:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>paint objects</b> with color."), sel_message);
           this->cursor_shape = cursor_color_xpm;
           break;
       case TWEAK_MODE_COLORJITTER:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>randomize colors</b>."), sel_message);
           this->cursor_shape = cursor_color_xpm;
           break;
       case TWEAK_MODE_BLUR:
           this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag or click to <b>increase blur</b>; with Shift to <b>decrease</b>."), sel_message);
           this->cursor_shape = cursor_color_xpm;
           break;
   }

   this->sp_event_context_update_cursor();
   g_free(sel_message);
}

bool TweakTool::set_style(const SPCSSAttr* css) {
    if (this->mode == TWEAK_MODE_COLORPAINT) { // intercept color setting only in this mode
        // we cannot store properties with uris
        css = sp_css_attr_unset_uris(const_cast<SPCSSAttr *>(css));
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setStyle("/tools/tweak/style", const_cast<SPCSSAttr *>(css));
        return true;
    }

    return false;
}

void TweakTool::setup() {
    ToolBase::setup();

    {
        /* TODO: have a look at sp_dyna_draw_context_setup where the same is done.. generalize? at least make it an arcto! */
        Geom::PathVector path;
        Geom::Circle(0, 0, 1).getPath(path);

        SPCurve *c = new SPCurve(path);

        this->dilate_area = sp_canvas_bpath_new(sp_desktop_controls(this->desktop), c);
        c->unref();
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(this->dilate_area), 0x00000000,(SPWindRule)0);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(this->dilate_area), 0xff9900ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_item_hide(this->dilate_area);
    }

    this->is_drawing = false;

    sp_event_context_read(this, "width");
    sp_event_context_read(this, "mode");
    sp_event_context_read(this, "fidelity");
    sp_event_context_read(this, "force");
    sp_event_context_read(this, "usepressure");
    sp_event_context_read(this, "doh");
    sp_event_context_read(this, "dol");
    sp_event_context_read(this, "dos");
    sp_event_context_read(this, "doo");

    this->style_set_connection = this->desktop->connectSetStyle( // catch style-setting signal in this tool
        //sigc::bind(sigc::ptr_fun(&sp_tweak_context_style_set), this)
   		sigc::mem_fun(this, &TweakTool::set_style)
    );
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/tweak/selcue")) {
        this->enableSelectionCue();
    }
    if (prefs->getBool("/tools/tweak/gradientdrag")) {
        this->enableGrDrag();
    }
}

void TweakTool::set(const Inkscape::Preferences::Entry& val) {
    Glib::ustring path = val.getEntryName();

    if (path == "width") {
        this->width = CLAMP(val.getDouble(0.1), -1000.0, 1000.0);
    } else if (path == "mode") {
        this->mode = val.getInt();
        this->update_cursor(false);
    } else if (path == "fidelity") {
        this->fidelity = CLAMP(val.getDouble(), 0.0, 1.0);
    } else if (path == "force") {
        this->force = CLAMP(val.getDouble(1.0), 0, 1.0);
    } else if (path == "usepressure") {
        this->usepressure = val.getBool();
    } else if (path == "doh") {
        this->do_h = val.getBool();
    } else if (path == "dos") {
        this->do_s = val.getBool();
    } else if (path == "dol") {
        this->do_l = val.getBool();
    } else if (path == "doo") {
        this->do_o = val.getBool();
    }
}

static void
sp_tweak_extinput(TweakTool *tc, GdkEvent *event)
{
    if (gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &tc->pressure)) {
        tc->pressure = CLAMP (tc->pressure, TC_MIN_PRESSURE, TC_MAX_PRESSURE);
    } else {
        tc->pressure = TC_DEFAULT_PRESSURE;
    }
}

static double
get_dilate_radius (TweakTool *tc)
{
    // 10 times the pen width:
    return 500 * tc->width/SP_EVENT_CONTEXT(tc)->desktop->current_zoom();
}

static double
get_path_force (TweakTool *tc)
{
    double force = 8 * (tc->usepressure? tc->pressure : TC_DEFAULT_PRESSURE)
        /sqrt(SP_EVENT_CONTEXT(tc)->desktop->current_zoom());
    if (force > 3) {
        force += 4 * (force - 3);
    }
    return force * tc->force;
}

static double
get_move_force (TweakTool *tc)
{
    double force = (tc->usepressure? tc->pressure : TC_DEFAULT_PRESSURE);
    return force * tc->force;
}

static bool
sp_tweak_dilate_recursive (Inkscape::Selection *selection, SPItem *item, Geom::Point p, Geom::Point vector, gint mode, double radius, double force, double fidelity, bool reverse)
{
    bool did = false;

    {
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box && !is_transform_mode(mode) && !is_color_mode(mode)) {
            // convert 3D boxes to ordinary groups before tweaking their shapes
            item = box3d_convert_to_group(box);
            selection->add(item);
        }
    }

    if (dynamic_cast<SPText *>(item) || dynamic_cast<SPFlowtext *>(item)) {
        GSList *items = g_slist_prepend (NULL, item);
        GSList *selected = NULL;
        GSList *to_select = NULL;
        SPDocument *doc = item->document;
        sp_item_list_to_curves (items, &selected, &to_select);
        g_slist_free (items);
        SPObject* newObj = doc->getObjectByRepr(static_cast<Inkscape::XML::Node *>(to_select->data));
        g_slist_free (to_select);
        item = dynamic_cast<SPItem *>(newObj);
        g_assert(item != NULL);
        selection->add(item);
    }

    if (dynamic_cast<SPGroup *>(item) && !dynamic_cast<SPBox3D *>(item)) {
        GSList *children = NULL;
        for (SPObject *child = item->firstChild() ; child; child = child->getNext() ) {
            if (dynamic_cast<SPItem *>(static_cast<SPObject *>(child))) {
                children = g_slist_prepend(children, child);
            }
        }

        for (GSList *i = children; i; i = i->next) {
            SPItem *child = dynamic_cast<SPItem *>(static_cast<SPObject *>(i->data));
            g_assert(child != NULL);
            if (sp_tweak_dilate_recursive (selection, child, p, vector, mode, radius, force, fidelity, reverse)) {
                did = true;
            }
        }

        g_slist_free(children);

    } else {
        if (mode == TWEAK_MODE_MOVE) {

            Geom::OptRect a = item->documentVisualBounds();
            if (a) {
                double x = Geom::L2(a->midpoint() - p)/radius;
                if (a->contains(p)) x = 0;
                if (x < 1) {
                    Geom::Point move = force * 0.5 * (cos(M_PI * x) + 1) * vector;
                    sp_item_move_rel(item, Geom::Translate(move[Geom::X], -move[Geom::Y]));
                    did = true;
                }
            }

        } else if (mode == TWEAK_MODE_MOVE_IN_OUT) {

            Geom::OptRect a = item->documentVisualBounds();
            if (a) {
                double x = Geom::L2(a->midpoint() - p)/radius;
                if (a->contains(p)) x = 0;
                if (x < 1) {
                    Geom::Point move = force * 0.5 * (cos(M_PI * x) + 1) * 
                        (reverse? (a->midpoint() - p) : (p - a->midpoint()));
                    sp_item_move_rel(item, Geom::Translate(move[Geom::X], -move[Geom::Y]));
                    did = true;
                }
            }

        } else if (mode == TWEAK_MODE_MOVE_JITTER) {

            Geom::OptRect a = item->documentVisualBounds();
            if (a) {
                double dp = g_random_double_range(0, M_PI*2);
                double dr = g_random_double_range(0, radius);
                double x = Geom::L2(a->midpoint() - p)/radius;
                if (a->contains(p)) x = 0;
                if (x < 1) {
                    Geom::Point move = force * 0.5 * (cos(M_PI * x) + 1) * Geom::Point(cos(dp)*dr, sin(dp)*dr);
                    sp_item_move_rel(item, Geom::Translate(move[Geom::X], -move[Geom::Y]));
                    did = true;
                }
            }

        } else if (mode == TWEAK_MODE_SCALE) {

            Geom::OptRect a = item->documentVisualBounds();
            if (a) {
                double x = Geom::L2(a->midpoint() - p)/radius;
                if (a->contains(p)) x = 0;
                if (x < 1) {
                    double scale = 1 + (reverse? force : -force) * 0.05 * (cos(M_PI * x) + 1);
                    sp_item_scale_rel(item, Geom::Scale(scale, scale));
                    did = true;
                }
            }

        } else if (mode == TWEAK_MODE_ROTATE) {

            Geom::OptRect a = item->documentVisualBounds();
            if (a) {
                double x = Geom::L2(a->midpoint() - p)/radius;
                if (a->contains(p)) x = 0;
                if (x < 1) {
                    double angle = (reverse? force : -force) * 0.05 * (cos(M_PI * x) + 1) * M_PI;
                    sp_item_rotate_rel(item, Geom::Rotate(angle));
                    did = true;
                }
            }

        } else if (mode == TWEAK_MODE_MORELESS) {

            Geom::OptRect a = item->documentVisualBounds();
            if (a) {
                double x = Geom::L2(a->midpoint() - p)/radius;
                if (a->contains(p)) x = 0;
                if (x < 1) {
                    double prob = force * 0.5 * (cos(M_PI * x) + 1);
                    double chance = g_random_double_range(0, 1);
                    if (chance <= prob) {
                        if (reverse) { // delete
                            sp_object_ref(item, NULL);
                            item->deleteObject(true, true);
                            sp_object_unref(item, NULL);
                        } else { // duplicate
                            SPDocument *doc = item->document;
                            Inkscape::XML::Document* xml_doc = doc->getReprDoc();
                            Inkscape::XML::Node *old_repr = item->getRepr();
                            SPObject *old_obj = doc->getObjectByRepr(old_repr);
                            Inkscape::XML::Node *parent = old_repr->parent();
                            Inkscape::XML::Node *copy = old_repr->duplicate(xml_doc);
                            parent->appendChild(copy);
                            SPObject *new_obj = doc->getObjectByRepr(copy);
                            if (selection->includes(old_obj)) {
                                selection->add(new_obj);
                            }
                            Inkscape::GC::release(copy);
                        }
                        did = true;
                    }
                }
            }

        } else if (dynamic_cast<SPPath *>(item) || dynamic_cast<SPShape *>(item)) {

            Inkscape::XML::Node *newrepr = NULL;
            gint pos = 0;
            Inkscape::XML::Node *parent = NULL;
            char const *id = NULL;
            if (!dynamic_cast<SPPath *>(item)) {
                newrepr = sp_selected_item_to_curved_repr(item, 0);
                if (!newrepr) {
                    return false;
                }

                // remember the position of the item
                pos = item->getRepr()->position();
                // remember parent
                parent = item->getRepr()->parent();
                // remember id
                id = item->getRepr()->attribute("id");
            }

            // skip those paths whose bboxes are entirely out of reach with our radius
            Geom::OptRect bbox = item->documentVisualBounds();
            if (bbox) {
                bbox->expandBy(radius);
                if (!bbox->contains(p)) {
                    return false;
                }
            }

            Path *orig = Path_for_item(item, false);
            if (orig == NULL) {
                return false;
            }

            Path *res = new Path;
            res->SetBackData(false);

            Shape *theShape = new Shape;
            Shape *theRes = new Shape;
            Geom::Affine i2doc(item->i2doc_affine());

            orig->ConvertWithBackData((0.08 - (0.07 * fidelity)) / i2doc.descrim()); // default 0.059
            orig->Fill(theShape, 0);

            SPCSSAttr *css = sp_repr_css_attr(item->getRepr(), "style");
            gchar const *val = sp_repr_css_property(css, "fill-rule", NULL);
            if (val && strcmp(val, "nonzero") == 0) {
                theRes->ConvertToShape(theShape, fill_nonZero);
            } else if (val && strcmp(val, "evenodd") == 0) {
                theRes->ConvertToShape(theShape, fill_oddEven);
            } else {
                theRes->ConvertToShape(theShape, fill_nonZero);
            }

            if (Geom::L2(vector) != 0) {
                vector = 1/Geom::L2(vector) * vector;
            }

            bool did_this = false;
            if (mode == TWEAK_MODE_SHRINK_GROW) {
                if (theShape->MakeTweak(tweak_mode_grow, theRes,
                                     reverse? force : -force,
                                     join_straight, 4.0,
                                     true, p, Geom::Point(0,0), radius, &i2doc) == 0) // 0 means the shape was actually changed
                  did_this = true;
            } else if (mode == TWEAK_MODE_ATTRACT_REPEL) {
                if (theShape->MakeTweak(tweak_mode_repel, theRes,
                                     reverse? force : -force,
                                     join_straight, 4.0,
                                     true, p, Geom::Point(0,0), radius, &i2doc) == 0)
                  did_this = true;
            } else if (mode == TWEAK_MODE_PUSH) {
                if (theShape->MakeTweak(tweak_mode_push, theRes,
                                     1.0,
                                     join_straight, 4.0,
                                     true, p, force*2*vector, radius, &i2doc) == 0)
                  did_this = true;
            } else if (mode == TWEAK_MODE_ROUGHEN) {
                if (theShape->MakeTweak(tweak_mode_roughen, theRes,
                                     force,
                                     join_straight, 4.0,
                                     true, p, Geom::Point(0,0), radius, &i2doc) == 0)
                  did_this = true;
            }

            // the rest only makes sense if we actually changed the path
            if (did_this) {
                theRes->ConvertToShape(theShape, fill_positive);

                res->Reset();
                theRes->ConvertToForme(res);

                double th_max = (0.6 - 0.59*sqrt(fidelity)) / i2doc.descrim();
                double threshold = MAX(th_max, th_max*force);
                res->ConvertEvenLines(threshold);
                res->Simplify(threshold / (selection->desktop()->current_zoom()));

                if (newrepr) { // converting to path, need to replace the repr
                    bool is_selected = selection->includes(item);
                    if (is_selected) {
                        selection->remove(item);
                    }

                    // It's going to resurrect, so we delete without notifying listeners.
                    item->deleteObject(false);

                    // restore id
                    newrepr->setAttribute("id", id);
                    // add the new repr to the parent
                    parent->appendChild(newrepr);
                    // move to the saved position
                    newrepr->setPosition(pos > 0 ? pos : 0);

                    if (is_selected)
                        selection->add(newrepr);
                }

                if (res->descr_cmd.size() > 1) {
                    gchar *str = res->svg_dump_path();
                    if (newrepr) {
                        newrepr->setAttribute("d", str);
                    } else {
                        SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
                        if (lpeitem && lpeitem->hasPathEffectRecursive()) {
                            item->getRepr()->setAttribute("inkscape:original-d", str);
                        } else {
                            item->getRepr()->setAttribute("d", str);
                        }
                    }
                    g_free(str);
                } else {
                    // TODO: if there's 0 or 1 node left, delete this path altogether
                }

                if (newrepr) {
                    Inkscape::GC::release(newrepr);
                    newrepr = NULL;
                }
            }

            delete theShape;
            delete theRes;
            delete orig;
            delete res;

            if (did_this) {
                did = true;
            }
        }

    }

    return did;
}

static void
tweak_colorpaint (float *color, guint32 goal, double force, bool do_h, bool do_s, bool do_l)
{
    float rgb_g[3];

    if (!do_h || !do_s || !do_l) {
        float hsl_g[3];
        sp_color_rgb_to_hsl_floatv (hsl_g, SP_RGBA32_R_F(goal), SP_RGBA32_G_F(goal), SP_RGBA32_B_F(goal));
        float hsl_c[3];
        sp_color_rgb_to_hsl_floatv (hsl_c, color[0], color[1], color[2]);
        if (!do_h) {
            hsl_g[0] = hsl_c[0];
        }
        if (!do_s) {
            hsl_g[1] = hsl_c[1];
        }
        if (!do_l) {
            hsl_g[2] = hsl_c[2];
        }
        sp_color_hsl_to_rgb_floatv (rgb_g, hsl_g[0], hsl_g[1], hsl_g[2]);
    } else {
        rgb_g[0] = SP_RGBA32_R_F(goal);
        rgb_g[1] = SP_RGBA32_G_F(goal);
        rgb_g[2] = SP_RGBA32_B_F(goal);
    }

    for (int i = 0; i < 3; i++) {
        double d = rgb_g[i] - color[i];
        color[i] += d * force;
    }
}

static void
tweak_colorjitter (float *color, double force, bool do_h, bool do_s, bool do_l)
{
    float hsl_c[3];
    sp_color_rgb_to_hsl_floatv (hsl_c, color[0], color[1], color[2]);

    if (do_h) {
        hsl_c[0] += g_random_double_range(-0.5, 0.5) * force;
        if (hsl_c[0] > 1) {
            hsl_c[0] -= 1;
        }
        if (hsl_c[0] < 0) {
            hsl_c[0] += 1;
        }
    }
    if (do_s) {
        hsl_c[1] += g_random_double_range(-hsl_c[1], 1 - hsl_c[1]) * force;
    }
    if (do_l) {
        hsl_c[2] += g_random_double_range(-hsl_c[2], 1 - hsl_c[2]) * force;
    }

    sp_color_hsl_to_rgb_floatv (color, hsl_c[0], hsl_c[1], hsl_c[2]);
}

static void
tweak_color (guint mode, float *color, guint32 goal, double force, bool do_h, bool do_s, bool do_l)
{
    if (mode == TWEAK_MODE_COLORPAINT) {
        tweak_colorpaint (color, goal, force, do_h, do_s, do_l);
    } else if (mode == TWEAK_MODE_COLORJITTER) {
        tweak_colorjitter (color, force, do_h, do_s, do_l);
    }
}

static void
tweak_opacity (guint mode, SPIScale24 *style_opacity, double opacity_goal, double force)
{
    double opacity = SP_SCALE24_TO_FLOAT (style_opacity->value);

    if (mode == TWEAK_MODE_COLORPAINT) {
        double d = opacity_goal - opacity;
        opacity += d * force;
    } else if (mode == TWEAK_MODE_COLORJITTER) {
        opacity += g_random_double_range(-opacity, 1 - opacity) * force;
    }

    style_opacity->value = SP_SCALE24_FROM_FLOAT(opacity);
}


static double
tweak_profile (double dist, double radius)
{
    if (radius == 0) {
        return 0;
    }
    double x = dist / radius;
    double alpha = 1;
    if (x >= 1) {
        return 0;
    } else if (x <= 0) {
        return 1;
    } else {
        return (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5);
    }
}

static void tweak_colors_in_gradient(SPItem *item, Inkscape::PaintTarget fill_or_stroke,
                                     guint32 const rgb_goal, Geom::Point p_w, double radius, double force, guint mode,
                                     bool do_h, bool do_s, bool do_l, bool /*do_o*/)
{
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (!gradient || !dynamic_cast<SPGradient *>(gradient)) {
        return;
    }

    Geom::Affine i2d (item->i2doc_affine ());
    Geom::Point p = p_w * i2d.inverse();
    p *= (gradient->gradientTransform).inverse();
    // now p is in gradient's original coordinates

    double pos = 0;
    double r = 0;

    SPLinearGradient *lg = dynamic_cast<SPLinearGradient *>(gradient);
    if (lg) {
        Geom::Point p1(lg->x1.computed, lg->y1.computed);
        Geom::Point p2(lg->x2.computed, lg->y2.computed);
        Geom::Point pdiff(p2 - p1);
        double vl = Geom::L2(pdiff);

        // This is the matrix which moves and rotates the gradient line
        // so it's oriented along the X axis:
        Geom::Affine norm = Geom::Affine(Geom::Translate(-p1)) * Geom::Affine(Geom::Rotate(-atan2(pdiff[Geom::Y], pdiff[Geom::X])));

        // Transform the mouse point by it to find out its projection onto the gradient line:
        Geom::Point pnorm = p * norm;

        // Scale its X coordinate to match the length of the gradient line:
        pos = pnorm[Geom::X] / vl;
        // Calculate radius in lenfth-of-gradient-line units
        r = radius / vl;

    } else {
        SPRadialGradient *rg = dynamic_cast<SPRadialGradient *>(gradient);
        if (rg) {
            Geom::Point c (rg->cx.computed, rg->cy.computed);
            pos = Geom::L2(p - c) / rg->r.computed;
            r = radius / rg->r.computed;
        }
    }

    // Normalize pos to 0..1, taking into accound gradient spread:
    double pos_e = pos;
    if (gradient->getSpread() == SP_GRADIENT_SPREAD_PAD) {
        if (pos > 1) {
            pos_e = 1;
        }
        if (pos < 0) {
            pos_e = 0;
        }
    } else if (gradient->getSpread() == SP_GRADIENT_SPREAD_REPEAT) {
        if (pos > 1 || pos < 0) {
            pos_e = pos - floor(pos);
        }
    } else if (gradient->getSpread() == SP_GRADIENT_SPREAD_REFLECT) {
        if (pos > 1 || pos < 0) {
            bool odd = ((int)(floor(pos)) % 2 == 1);
            pos_e = pos - floor(pos);
            if (odd) {
                pos_e = 1 - pos_e;
            }
        }
    }

    SPGradient *vector = sp_gradient_get_forked_vector_if_necessary(gradient, false);

    double offset_l = 0;
    double offset_h = 0;
    SPObject *child_prev = NULL;
    for (SPObject *child = vector->firstChild(); child; child = child->getNext()) {
        SPStop *stop = dynamic_cast<SPStop *>(child);
        if (!stop) {
            continue;
        }

        offset_h = stop->offset;

        if (child_prev) {
            SPStop *prevStop = dynamic_cast<SPStop *>(child_prev);
            g_assert(prevStop != NULL);

            if (offset_h - offset_l > r && pos_e >= offset_l && pos_e <= offset_h) {
                // the summit falls in this interstop, and the radius is small,
                // so it only affects the ends of this interstop;
                // distribute the force between the two endstops so that they
                // get all the painting even if they are not touched by the brush
                tweak_color (mode, stop->specified_color.v.c, rgb_goal,
                                  force * (pos_e - offset_l) / (offset_h - offset_l),
                                  do_h, do_s, do_l);
                tweak_color(mode, prevStop->specified_color.v.c, rgb_goal,
                            force * (offset_h - pos_e) / (offset_h - offset_l),
                            do_h, do_s, do_l);
                stop->updateRepr();
                child_prev->updateRepr();
                break;
            } else {
                // wide brush, may affect more than 2 stops,
                // paint each stop by the force from the profile curve
                if (offset_l <= pos_e && offset_l > pos_e - r) {
                    tweak_color(mode, prevStop->specified_color.v.c, rgb_goal,
                                force * tweak_profile (fabs (pos_e - offset_l), r),
                                do_h, do_s, do_l);
                    child_prev->updateRepr();
                }

                if (offset_h >= pos_e && offset_h < pos_e + r) {
                    tweak_color (mode, stop->specified_color.v.c, rgb_goal,
                                 force * tweak_profile (fabs (pos_e - offset_h), r),
                                 do_h, do_s, do_l);
                    stop->updateRepr();
                }
            }
        }

        offset_l = offset_h;
        child_prev = child;
    }
}

static bool
sp_tweak_color_recursive (guint mode, SPItem *item, SPItem *item_at_point,
                          guint32 fill_goal, bool do_fill,
                          guint32 stroke_goal, bool do_stroke,
                          float opacity_goal, bool do_opacity,
                          bool do_blur, bool reverse,
                          Geom::Point p, double radius, double force,
                          bool do_h, bool do_s, bool do_l, bool do_o)
{
    bool did = false;

    if (dynamic_cast<SPGroup *>(item)) {
        for (SPObject *child = item->firstChild() ; child; child = child->getNext() ) {
            SPItem *childItem = dynamic_cast<SPItem *>(child);
            if (childItem) {
                if (sp_tweak_color_recursive (mode, childItem, item_at_point,
                                          fill_goal, do_fill,
                                          stroke_goal, do_stroke,
                                          opacity_goal, do_opacity,
                                          do_blur, reverse,
                                          p, radius, force, do_h, do_s, do_l, do_o)) {
                    did = true;
                }
            }
        }

    } else {
        SPStyle *style = item->style;
        if (!style) {
            return false;
        }
        Geom::OptRect bbox = item->documentGeometricBounds();
        if (!bbox) {
            return false;
        }

        Geom::Rect brush(p - Geom::Point(radius, radius), p + Geom::Point(radius, radius));

        Geom::Point center = bbox->midpoint();
        double this_force;

// if item == item_at_point, use max force
        if (item == item_at_point) {
            this_force = force;
// else if no overlap of bbox and brush box, skip:
        } else if (!bbox->intersects(brush)) {
            return false;
//TODO:
// else if object > 1.5 brush: test 4/8/16 points in the brush on hitting the object, choose max
        //} else if (bbox->maxExtent() > 3 * radius) {
        //}
// else if object > 0.5 brush: test 4 corners of bbox and center on being in the brush, choose max
// else if still smaller, then check only the object center:
        } else {
            this_force = force * tweak_profile (Geom::L2 (p - center), radius);
        }

        if (this_force > 0.002) {

            if (do_blur) {
                Geom::OptRect bbox = item->documentGeometricBounds();
                if (!bbox) {
                    return did;
                }

                double blur_now = 0;
                Geom::Affine i2dt = item->i2dt_affine ();
                if (style->filter.set && style->getFilter()) {
                    //cycle through filter primitives
                    SPObject *primitive_obj = style->getFilter()->children;
                    while (primitive_obj) {
                        SPFilterPrimitive *primitive = dynamic_cast<SPFilterPrimitive *>(primitive_obj);
                        if (primitive) {
                            //if primitive is gaussianblur
                            SPGaussianBlur * spblur = dynamic_cast<SPGaussianBlur *>(primitive);
                            if (spblur) {
                                float num = spblur->stdDeviation.getNumber();
                                blur_now += num * i2dt.descrim(); // sum all blurs in the filter
                            }
                        }
                        primitive_obj = primitive_obj->next;
                    }
                }
                double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];
                blur_now = blur_now / perimeter;

                double blur_new;
                if (reverse) {
                    blur_new = blur_now - 0.06 * force;
                } else {
                    blur_new = blur_now + 0.06 * force;
                }
                if (blur_new < 0.0005 && blur_new < blur_now) {
                    blur_new = 0;
                }
                if (blur_new == 0) {
                    remove_filter(item, false);
                } else {
                    double radius = blur_new * perimeter;
                    SPFilter *filter = modify_filter_gaussian_blur_from_item(item->document, item, radius);
                    sp_style_set_property_url(item, "filter", filter, false);
                }
                return true; // do not do colors, blur is a separate mode
            }

            if (do_fill) {
                if (style->fill.isPaintserver()) {
                    tweak_colors_in_gradient(item, Inkscape::FOR_FILL, fill_goal, p, radius, this_force, mode, do_h, do_s, do_l, do_o);
                    did = true;
                } else if (style->fill.isColor()) {
                    tweak_color (mode, style->fill.value.color.v.c, fill_goal, this_force, do_h, do_s, do_l);
                    item->updateRepr();
                    did = true;
                }
            }
            if (do_stroke) {
                if (style->stroke.isPaintserver()) {
                    tweak_colors_in_gradient(item, Inkscape::FOR_STROKE, stroke_goal, p, radius, this_force, mode, do_h, do_s, do_l, do_o);
                    did = true;
                } else if (style->stroke.isColor()) {
                    tweak_color (mode, style->stroke.value.color.v.c, stroke_goal, this_force, do_h, do_s, do_l);
                    item->updateRepr();
                    did = true;
                }
            }
            if (do_opacity && do_o) {
                tweak_opacity (mode, &style->opacity, opacity_goal, this_force);
            }
        }
    }

    return did;
}


static bool
sp_tweak_dilate (TweakTool *tc, Geom::Point event_p, Geom::Point p, Geom::Point vector, bool reverse)
{
    Inkscape::Selection *selection = sp_desktop_selection(SP_EVENT_CONTEXT(tc)->desktop);
    SPDesktop *desktop = SP_EVENT_CONTEXT(tc)->desktop;

    if (selection->isEmpty()) {
        return false;
    }

    bool did = false;
    double radius = get_dilate_radius(tc);

    SPItem *item_at_point = SP_EVENT_CONTEXT(tc)->desktop->getItemAtPoint(event_p, TRUE);

    bool do_fill = false, do_stroke = false, do_opacity = false;
    guint32 fill_goal = sp_desktop_get_color_tool(desktop, "/tools/tweak", true, &do_fill);
    guint32 stroke_goal = sp_desktop_get_color_tool(desktop, "/tools/tweak", false, &do_stroke);
    double opacity_goal = sp_desktop_get_master_opacity_tool(desktop, "/tools/tweak", &do_opacity);
    if (reverse) {
#if 0
        // HSL inversion 
        float hsv[3];
        float rgb[3];
        sp_color_rgb_to_hsv_floatv (hsv, 
                                    SP_RGBA32_R_F(fill_goal),
                                    SP_RGBA32_G_F(fill_goal),
                                    SP_RGBA32_B_F(fill_goal));
        sp_color_hsv_to_rgb_floatv (rgb, hsv[0]<.5? hsv[0]+.5 : hsv[0]-.5, 1 - hsv[1], 1 - hsv[2]);
        fill_goal = SP_RGBA32_F_COMPOSE(rgb[0], rgb[1], rgb[2], 1);
        sp_color_rgb_to_hsv_floatv (hsv, 
                                    SP_RGBA32_R_F(stroke_goal),
                                    SP_RGBA32_G_F(stroke_goal),
                                    SP_RGBA32_B_F(stroke_goal));
        sp_color_hsv_to_rgb_floatv (rgb, hsv[0]<.5? hsv[0]+.5 : hsv[0]-.5, 1 - hsv[1], 1 - hsv[2]);
        stroke_goal = SP_RGBA32_F_COMPOSE(rgb[0], rgb[1], rgb[2], 1);
#else
        // RGB inversion 
        fill_goal = SP_RGBA32_U_COMPOSE(
            (255 - SP_RGBA32_R_U(fill_goal)),
            (255 - SP_RGBA32_G_U(fill_goal)),
            (255 - SP_RGBA32_B_U(fill_goal)),
            (255 - SP_RGBA32_A_U(fill_goal)));
        stroke_goal = SP_RGBA32_U_COMPOSE(
            (255 - SP_RGBA32_R_U(stroke_goal)),
            (255 - SP_RGBA32_G_U(stroke_goal)),
            (255 - SP_RGBA32_B_U(stroke_goal)),
            (255 - SP_RGBA32_A_U(stroke_goal)));
#endif
        opacity_goal = 1 - opacity_goal;
    }

    double path_force = get_path_force(tc);
    if (radius == 0 || path_force == 0) {
        return false;
    }
    double move_force = get_move_force(tc);
    double color_force = MIN(sqrt(path_force)/20.0, 1);

    for (GSList *items = g_slist_copy(const_cast<GSList *>(selection->itemList()));
         items != NULL;
         items = items->next) {

        SPItem *item = dynamic_cast<SPItem *>(static_cast<SPObject *>(items->data));

        if (is_color_mode (tc->mode)) {
            if (do_fill || do_stroke || do_opacity) {
                if (sp_tweak_color_recursive (tc->mode, item, item_at_point,
                                          fill_goal, do_fill,
                                          stroke_goal, do_stroke,
                                          opacity_goal, do_opacity,
                                          tc->mode == TWEAK_MODE_BLUR, reverse,
                                          p, radius, color_force, tc->do_h, tc->do_s, tc->do_l, tc->do_o)) {
                    did = true;
                }
            }
        } else if (is_transform_mode(tc->mode)) {
            if (sp_tweak_dilate_recursive (selection, item, p, vector, tc->mode, radius, move_force, tc->fidelity, reverse)) {
                did = true;
            }
        } else {
            if (sp_tweak_dilate_recursive (selection, item, p, vector, tc->mode, radius, path_force, tc->fidelity, reverse)) {
                did = true;
            }
        }
    }

    return did;
}

static void
sp_tweak_update_area (TweakTool *tc)
{
        double radius = get_dilate_radius(tc);
        Geom::Affine const sm (Geom::Scale(radius, radius) * Geom::Translate(SP_EVENT_CONTEXT(tc)->desktop->point()));
        sp_canvas_item_affine_absolute(tc->dilate_area, sm);
        sp_canvas_item_show(tc->dilate_area);
}

static void
sp_tweak_switch_mode (TweakTool *tc, gint mode, bool with_shift)
{
    SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue ("tweak_tool_mode", mode);
    // need to set explicitly, because the prefs may not have changed by the previous
    tc->mode = mode;
    tc->update_cursor(with_shift);
}

static void
sp_tweak_switch_mode_temporarily (TweakTool *tc, gint mode, bool with_shift)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
   // Juggling about so that prefs have the old value but tc->mode and the button show new mode:
   gint now_mode = prefs->getInt("/tools/tweak/mode", 0);
   SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue ("tweak_tool_mode", mode);
   // button has changed prefs, restore
   prefs->setInt("/tools/tweak/mode", now_mode);
   // changing prefs changed tc->mode, restore back :)
   tc->mode = mode;
   tc->update_cursor(with_shift);
}

bool TweakTool::root_handler(GdkEvent* event) {
    gint ret = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            sp_canvas_item_show(this->dilate_area);
            break;
        case GDK_LEAVE_NOTIFY:
            sp_canvas_item_hide(this->dilate_area);
            break;
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !this->space_panning) {

                if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
                    return TRUE;
                }

                Geom::Point const button_w(event->button.x,
                                         event->button.y);
                Geom::Point const button_dt(desktop->w2d(button_w));
                this->last_push = desktop->dt2doc(button_dt);

                sp_tweak_extinput(this, event);

                desktop->canvas->forceFullRedrawAfterInterruptions(3);
                this->is_drawing = true;
                this->is_dilating = true;
                this->has_dilated = false;

                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            Geom::Point const motion_w(event->motion.x,
                                     event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));
            Geom::Point motion_doc(desktop->dt2doc(motion_dt));
            sp_tweak_extinput(this, event);

            // draw the dilating cursor
                double radius = get_dilate_radius(this);
                Geom::Affine const sm (Geom::Scale(radius, radius) * Geom::Translate(desktop->w2d(motion_w)));
                sp_canvas_item_affine_absolute(this->dilate_area, sm);
                sp_canvas_item_show(this->dilate_area);

                guint num = 0;
                if (!desktop->selection->isEmpty()) {
                    num = g_slist_length(const_cast<GSList *>(desktop->selection->itemList()));
                }
                if (num == 0) {
                    this->message_context->flash(Inkscape::ERROR_MESSAGE, _("<b>Nothing selected!</b> Select objects to tweak."));
                }

            // dilating:
            if (this->is_drawing && ( event->motion.state & GDK_BUTTON1_MASK )) {
                sp_tweak_dilate (this, motion_w, motion_doc, motion_doc - this->last_push, event->button.state & GDK_SHIFT_MASK? true : false);
                //this->last_push = motion_doc;
                this->has_dilated = true;
                // it's slow, so prevent clogging up with events
                gobble_motion_events(GDK_BUTTON1_MASK);
                return TRUE;
            }

        }
        break;
        case GDK_BUTTON_RELEASE:
        {
            Geom::Point const motion_w(event->button.x, event->button.y);
            Geom::Point const motion_dt(desktop->w2d(motion_w));

            desktop->canvas->endForcedFullRedraws();
            this->is_drawing = false;

            if (this->is_dilating && event->button.button == 1 && !this->space_panning) {
                if (!this->has_dilated) {
                    // if we did not rub, do a light tap
                    this->pressure = 0.03;
                    sp_tweak_dilate (this, motion_w, desktop->dt2doc(motion_dt), Geom::Point(0,0), MOD__SHIFT(event));
                }
                this->is_dilating = false;
                this->has_dilated = false;
                switch (this->mode) {
                    case TWEAK_MODE_MOVE:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Move tweak"));
                        break;
                    case TWEAK_MODE_MOVE_IN_OUT:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Move in/out tweak"));
                        break;
                    case TWEAK_MODE_MOVE_JITTER:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Move jitter tweak"));
                        break;
                    case TWEAK_MODE_SCALE:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Scale tweak"));
                        break;
                    case TWEAK_MODE_ROTATE:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Rotate tweak"));
                        break;
                    case TWEAK_MODE_MORELESS:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Duplicate/delete tweak"));
                        break;
                    case TWEAK_MODE_PUSH:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Push path tweak"));
                        break;
                    case TWEAK_MODE_SHRINK_GROW:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Shrink/grow path tweak"));
                        break;
                    case TWEAK_MODE_ATTRACT_REPEL:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Attract/repel path tweak"));
                        break;
                    case TWEAK_MODE_ROUGHEN:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Roughen path tweak"));
                        break;
                    case TWEAK_MODE_COLORPAINT:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Color paint tweak"));
                        break;
                    case TWEAK_MODE_COLORJITTER:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Color jitter tweak"));
                        break;
                    case TWEAK_MODE_BLUR:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(this)->desktop),
                                           SP_VERB_CONTEXT_TWEAK, _("Blur tweak"));
                        break;
                }
            }
            break;
        }
        case GDK_KEY_PRESS:
        {
            switch (get_group0_keyval (&event->key)) {
                case GDK_KEY_m:
                case GDK_KEY_M:
                case GDK_KEY_0:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_MOVE, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_i:
                case GDK_KEY_I:
                case GDK_KEY_1:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_MOVE_IN_OUT, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_z:
                case GDK_KEY_Z:
                case GDK_KEY_2:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_MOVE_JITTER, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_less:
                case GDK_KEY_comma:
                case GDK_KEY_greater:
                case GDK_KEY_period:
                case GDK_KEY_3:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_SCALE, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_bracketright:
                case GDK_KEY_bracketleft:
                case GDK_KEY_4:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_ROTATE, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_d:
                case GDK_KEY_D:
                case GDK_KEY_5:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_MORELESS, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_p:
                case GDK_KEY_P:
                case GDK_KEY_6:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_PUSH, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_s:
                case GDK_KEY_S:
                case GDK_KEY_7:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_SHRINK_GROW, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_a:
                case GDK_KEY_A:
                case GDK_KEY_8:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_ATTRACT_REPEL, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_r:
                case GDK_KEY_R:
                case GDK_KEY_9:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_ROUGHEN, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_c:
                case GDK_KEY_C:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_COLORPAINT, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_j:
                case GDK_KEY_J:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_COLORJITTER, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_b:
                case GDK_KEY_B:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_tweak_switch_mode(this, TWEAK_MODE_BLUR, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;

                case GDK_KEY_Up:
                case GDK_KEY_KP_Up:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->force += 0.05;
                        if (this->force > 1.0) {
                            this->force = 1.0;
                        }
                        desktop->setToolboxAdjustmentValue ("tweak-force", this->force * 100);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Down:
                case GDK_KEY_KP_Down:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->force -= 0.05;
                        if (this->force < 0.0) {
                            this->force = 0.0;
                        }
                        desktop->setToolboxAdjustmentValue ("tweak-force", this->force * 100);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Right:
                case GDK_KEY_KP_Right:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->width += 0.01;
                        if (this->width > 1.0) {
                            this->width = 1.0;
                        }
                        desktop->setToolboxAdjustmentValue ("altx-tweak", this->width * 100); // the same spinbutton is for alt+x
                        sp_tweak_update_area(this);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Left:
                case GDK_KEY_KP_Left:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->width -= 0.01;
                        if (this->width < 0.01) {
                            this->width = 0.01;
                        }
                        desktop->setToolboxAdjustmentValue ("altx-tweak", this->width * 100);
                        sp_tweak_update_area(this);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Home:
                case GDK_KEY_KP_Home:
                    this->width = 0.01;
                    desktop->setToolboxAdjustmentValue ("altx-tweak", this->width * 100);
                    sp_tweak_update_area(this);
                    ret = TRUE;
                    break;
                case GDK_KEY_End:
                case GDK_KEY_KP_End:
                    this->width = 1.0;
                    desktop->setToolboxAdjustmentValue ("altx-tweak", this->width * 100);
                    sp_tweak_update_area(this);
                    ret = TRUE;
                    break;
                case GDK_KEY_x:
                case GDK_KEY_X:
                    if (MOD__ALT_ONLY(event)) {
                        desktop->setToolboxFocusTo ("altx-tweak");
                        ret = TRUE;
                    }
                    break;

                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    this->update_cursor(true);
                    break;

                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                    sp_tweak_switch_mode_temporarily(this, TWEAK_MODE_SHRINK_GROW, MOD__SHIFT(event));
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
        }
        case GDK_KEY_RELEASE: {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            switch (get_group0_keyval(&event->key)) {
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    this->update_cursor(false);
                    break;
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                    sp_tweak_switch_mode (this, prefs->getInt("/tools/tweak/mode"), MOD__SHIFT(event));
                    this->message_context->clear();
                    break;
                default:
                    sp_tweak_switch_mode (this, prefs->getInt("/tools/tweak/mode"), MOD__SHIFT(event));
                    break;
            }
        }
        default:
            break;
    }

    if (!ret) {
    	ret = ToolBase::root_handler(event);
    }

    return ret;
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

/*
 * TextTool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/clipboard.h>
#include <display/sp-ctrlline.h>
#include <display/sodipodi-ctrlrect.h>
#include <display/sp-ctrlquadr.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>
#include <sstream>

#include "context-fns.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "macros.h"
#include "message-context.h"
#include "message-stack.h"
#include "pixmaps/cursor-text-insert.xpm"
#include "pixmaps/cursor-text.xpm"
#include "preferences.h"
#include "rubberband.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "ui/shape-editor.h"
#include "sp-flowtext.h"
#include "sp-namedview.h"
#include "sp-text.h"
#include "style.h"
#include "ui/tools/text-tool.h"
#include "text-editing.h"
#include "ui/control-manager.h"
#include "verbs.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include <gtk/gtk.h>
#include "ui/tool-factory.h"

using Inkscape::ControlManager;
using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Tools {

static void sp_text_context_validate_cursor_iterators(TextTool *tc);
static void sp_text_context_update_cursor(TextTool *tc, bool scroll_to_see = true);
static void sp_text_context_update_text_selection(TextTool *tc);
static gint sp_text_context_timeout(TextTool *tc);
static void sp_text_context_forget_text(TextTool *tc);

static gint sptc_focus_in(GtkWidget *widget, GdkEventFocus *event, TextTool *tc);
static gint sptc_focus_out(GtkWidget *widget, GdkEventFocus *event, TextTool *tc);
static void sptc_commit(GtkIMContext *imc, gchar *string, TextTool *tc);

namespace {
    ToolBase* createTextContext() {
        return new TextTool();
    }

    bool textContextRegistered = ToolFactory::instance().registerObject("/tools/text", createTextContext);
}

const std::string& TextTool::getPrefsPath() {
    return TextTool::prefsPath;
}

const std::string TextTool::prefsPath = "/tools/text";


TextTool::TextTool()
    : ToolBase(cursor_text_xpm, 7, 7)
    , imc(NULL)
    , text(NULL)
    , pdoc(0, 0)
    , unimode(false)
    , unipos(0)
    , cursor(NULL)
    , indicator(NULL)
    , frame(NULL)
    , timeout(0)
    , show(false)
    , phase(false)
    , nascent_object(false)
    , over_text(false)
    , dragging(0)
    , creating(false)
    , grabbed(NULL)
    , preedit_string(NULL)
{
}

TextTool::~TextTool() {
    delete this->shape_editor;
    this->shape_editor = NULL;

    if (this->grabbed) {
        sp_canvas_item_ungrab(this->grabbed, GDK_CURRENT_TIME);
        this->grabbed = NULL;
    }

    Inkscape::Rubberband::get(this->desktop)->stop();
}

void TextTool::setup() {
    GtkSettings* settings = gtk_settings_get_default();
    gint timeout = 0;
    g_object_get( settings, "gtk-cursor-blink-time", &timeout, NULL );
    
    if (timeout < 0) {
        timeout = 200;
    } else {
        timeout /= 2;
    }

    this->cursor = ControlManager::getManager().createControlLine(sp_desktop_controls(desktop), Geom::Point(100, 0), Geom::Point(100, 100));
    this->cursor->setRgba32(0x000000ff);
    sp_canvas_item_hide(this->cursor);

    this->indicator = sp_canvas_item_new(sp_desktop_controls(desktop), SP_TYPE_CTRLRECT, NULL);
    SP_CTRLRECT(this->indicator)->setRectangle(Geom::Rect(Geom::Point(0, 0), Geom::Point(100, 100)));
    SP_CTRLRECT(this->indicator)->setColor(0x0000ff7f, false, 0);
    SP_CTRLRECT(this->indicator)->setShadow(1, 0xffffff7f);
    sp_canvas_item_hide(this->indicator);

    this->frame = sp_canvas_item_new(sp_desktop_controls(desktop), SP_TYPE_CTRLRECT, NULL);
    SP_CTRLRECT(this->frame)->setRectangle(Geom::Rect(Geom::Point(0, 0), Geom::Point(100, 100)));
    SP_CTRLRECT(this->frame)->setColor(0x0000ff7f, false, 0);
    sp_canvas_item_hide(this->frame);

    this->timeout = g_timeout_add(timeout, (GSourceFunc) sp_text_context_timeout, this);

    this->imc = gtk_im_multicontext_new();
    if (this->imc) {
        GtkWidget *canvas = GTK_WIDGET(sp_desktop_canvas(desktop));

        /* im preedit handling is very broken in inkscape for
         * multi-byte characters.  See bug 1086769.
         * We need to let the IM handle the preediting, and
         * just take in the characters when they're finished being
         * entered.
         */
        gtk_im_context_set_use_preedit(this->imc, FALSE);
        gtk_im_context_set_client_window(this->imc, 
            gtk_widget_get_window (canvas));

        g_signal_connect(G_OBJECT(canvas), "focus_in_event", G_CALLBACK(sptc_focus_in), this);
        g_signal_connect(G_OBJECT(canvas), "focus_out_event", G_CALLBACK(sptc_focus_out), this);
        g_signal_connect(G_OBJECT(this->imc), "commit", G_CALLBACK(sptc_commit), this);

        if (gtk_widget_has_focus(canvas)) {
            sptc_focus_in(canvas, NULL, this);
        }
    }

    ToolBase::setup();

    this->shape_editor = new ShapeEditor(this->desktop);

    SPItem *item = sp_desktop_selection(this->desktop)->singleItem();
    if (item && SP_IS_FLOWTEXT(item) && SP_FLOWTEXT(item)->has_internal_frame()) {
        this->shape_editor->set_item(item);
    }

    this->sel_changed_connection = sp_desktop_selection(desktop)->connectChangedFirst(
        sigc::mem_fun(*this, &TextTool::_selectionChanged)
    );
    this->sel_modified_connection = sp_desktop_selection(desktop)->connectModifiedFirst(
        sigc::mem_fun(*this, &TextTool::_selectionModified)
    );
    this->style_set_connection = desktop->connectSetStyle(
        sigc::mem_fun(*this, &TextTool::_styleSet)
    );
    this->style_query_connection = desktop->connectQueryStyle(
        sigc::mem_fun(*this, &TextTool::_styleQueried)
    );

    _selectionChanged(sp_desktop_selection(desktop));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/text/selcue")) {
        this->enableSelectionCue();
    }
    if (prefs->getBool("/tools/text/gradientdrag")) {
        this->enableGrDrag();
    }
}

void TextTool::finish() {
    if (this->desktop) {
        sp_signal_disconnect_by_data(sp_desktop_canvas(this->desktop), this);
    }

    this->enableGrDrag(false);

    this->style_set_connection.disconnect();
    this->style_query_connection.disconnect();
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();

    sp_text_context_forget_text(SP_TEXT_CONTEXT(this));

    if (this->imc) {
        g_object_unref(G_OBJECT(this->imc));
        this->imc = NULL;
    }

    if (this->timeout) {
        g_source_remove(this->timeout);
        this->timeout = 0;
    }

    if (this->cursor) {
        sp_canvas_item_destroy(this->cursor);
        this->cursor = NULL;
    }

    if (this->indicator) {
        sp_canvas_item_destroy(this->indicator);
        this->indicator = NULL;
    }

    if (this->frame) {
        sp_canvas_item_destroy(this->frame);
        this->frame = NULL;
    }

    for (std::vector<SPCanvasItem*>::iterator it = this->text_selection_quads.begin() ;
         it != this->text_selection_quads.end() ; ++it) {
        sp_canvas_item_hide(*it);
        sp_canvas_item_destroy(*it);
    }
    
    this->text_selection_quads.clear();

    ToolBase::finish();
}

bool TextTool::item_handler(SPItem* item, GdkEvent* event) {
    SPItem *item_ungrouped;

    gint ret = FALSE;

    sp_text_context_validate_cursor_iterators(this);
    Inkscape::Text::Layout::iterator old_start = this->text_sel_start;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !this->space_panning) {
                // find out clicked item, disregarding groups
                item_ungrouped = desktop->getItemAtPoint(Geom::Point(event->button.x, event->button.y), TRUE);
                if (SP_IS_TEXT(item_ungrouped) || SP_IS_FLOWTEXT(item_ungrouped)) {
                    sp_desktop_selection(desktop)->set(item_ungrouped);
                    if (this->text) {
                        // find out click point in document coordinates
                        Geom::Point p = desktop->w2d(Geom::Point(event->button.x, event->button.y));
                        // set the cursor closest to that point
                        if (event->button.state & GDK_SHIFT_MASK) {
                            this->text_sel_start = old_start;
                            this->text_sel_end = sp_te_get_position_by_coords(this->text, p);
                        } else {
                            this->text_sel_start = this->text_sel_end = sp_te_get_position_by_coords(this->text, p);
                        }
                        // update display
                        sp_text_context_update_cursor(this);
                        sp_text_context_update_text_selection(this);
                        this->dragging = 1;
                    }
                    ret = TRUE;
                }
            }
            break;
        case GDK_2BUTTON_PRESS:
            if (event->button.button == 1 && this->text) {
                Inkscape::Text::Layout const *layout = te_get_layout(this->text);
                if (layout) {
                    if (!layout->isStartOfWord(this->text_sel_start))
                        this->text_sel_start.prevStartOfWord();
                    if (!layout->isEndOfWord(this->text_sel_end))
                        this->text_sel_end.nextEndOfWord();
                    sp_text_context_update_cursor(this);
                    sp_text_context_update_text_selection(this);
                    this->dragging = 2;
                    ret = TRUE;
                }
            }
            break;
        case GDK_3BUTTON_PRESS:
            if (event->button.button == 1 && this->text) {
                this->text_sel_start.thisStartOfLine();
                this->text_sel_end.thisEndOfLine();
                sp_text_context_update_cursor(this);
                sp_text_context_update_text_selection(this);
                this->dragging = 3;
                ret = TRUE;
            }
            break;
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && this->dragging && !this->space_panning) {
                this->dragging = 0;
                sp_event_context_discard_delayed_snap_event(this);
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if ((event->motion.state & GDK_BUTTON1_MASK) && this->dragging && !this->space_panning) {
                Inkscape::Text::Layout const *layout = te_get_layout(this->text);
                if (!layout) break;
                // find out click point in document coordinates
                Geom::Point p = desktop->w2d(Geom::Point(event->button.x, event->button.y));
                // set the cursor closest to that point
                Inkscape::Text::Layout::iterator new_end = sp_te_get_position_by_coords(this->text, p);
                if (this->dragging == 2) {
                    // double-click dragging: go by word
                    if (new_end < this->text_sel_start) {
                        if (!layout->isStartOfWord(new_end))
                            new_end.prevStartOfWord();
                    } else
                        if (!layout->isEndOfWord(new_end))
                            new_end.nextEndOfWord();
                } else if (this->dragging == 3) {
                    // triple-click dragging: go by line
                    if (new_end < this->text_sel_start)
                        new_end.thisStartOfLine();
                    else
                        new_end.thisEndOfLine();
                }
                // update display
                if (this->text_sel_end != new_end) {
                    this->text_sel_end = new_end;
                    sp_text_context_update_cursor(this);
                    sp_text_context_update_text_selection(this);
                }
                gobble_motion_events(GDK_BUTTON1_MASK);
                ret = TRUE;
                break;
            }
            // find out item under mouse, disregarding groups
            item_ungrouped = desktop->getItemAtPoint(Geom::Point(event->button.x, event->button.y), TRUE);
            if (SP_IS_TEXT(item_ungrouped) || SP_IS_FLOWTEXT(item_ungrouped)) {

                Inkscape::Text::Layout const *layout = te_get_layout(item_ungrouped);
                if (layout->inputTruncated()) {
                    SP_CTRLRECT(this->indicator)->setColor(0xff0000ff, false, 0);
                } else {
                    SP_CTRLRECT(this->indicator)->setColor(0x0000ff7f, false, 0);
                }
                Geom::OptRect ibbox = item_ungrouped->desktopVisualBounds();
                if (ibbox) {
                    SP_CTRLRECT(this->indicator)->setRectangle(*ibbox);
                }
                sp_canvas_item_show(this->indicator);

                this->cursor_shape = cursor_text_insert_xpm;
                this->hot_x = 7;
                this->hot_y = 10;
                this->sp_event_context_update_cursor();
                sp_text_context_update_text_selection(this);

                if (SP_IS_TEXT (item_ungrouped)) {
                    desktop->event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> to edit the text, <b>drag</b> to select part of the text."));
                } else {
                    desktop->event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> to edit the flowed text, <b>drag</b> to select part of the text."));
                }

                this->over_text = true;

                ret = TRUE;
            }
            break;
        default:
            break;
    }

    if (!ret) {
        ret = ToolBase::item_handler(item, event);
    }

    return ret;
}

static void sp_text_context_setup_text(TextTool *tc)
{
    ToolBase *ec = SP_EVENT_CONTEXT(tc);

    /* Create <text> */
    Inkscape::XML::Document *xml_doc = ec->desktop->doc()->getReprDoc();
    Inkscape::XML::Node *rtext = xml_doc->createElement("svg:text");
    rtext->setAttribute("xml:space", "preserve"); // we preserve spaces in the text objects we create

    /* Set style */
    sp_desktop_apply_style_tool(ec->desktop, rtext, "/tools/text", true);

    sp_repr_set_svg_double(rtext, "x", tc->pdoc[Geom::X]);
    sp_repr_set_svg_double(rtext, "y", tc->pdoc[Geom::Y]);

    /* Create <tspan> */
    Inkscape::XML::Node *rtspan = xml_doc->createElement("svg:tspan");
    rtspan->setAttribute("sodipodi:role", "line"); // otherwise, why bother creating the tspan?
    rtext->addChild(rtspan, NULL);
    Inkscape::GC::release(rtspan);

    /* Create TEXT */
    Inkscape::XML::Node *rstring = xml_doc->createTextNode("");
    rtspan->addChild(rstring, NULL);
    Inkscape::GC::release(rstring);
    SPItem *text_item = SP_ITEM(ec->desktop->currentLayer()->appendChildRepr(rtext));
    /* fixme: Is selection::changed really immediate? */
    /* yes, it's immediate .. why does it matter? */
    sp_desktop_selection(ec->desktop)->set(text_item);
    Inkscape::GC::release(rtext);
    text_item->transform = SP_ITEM(ec->desktop->currentLayer())->i2doc_affine().inverse();

    text_item->updateRepr();
    text_item->doWriteTransform(text_item->getRepr(), text_item->transform, NULL, true);
    DocumentUndo::done(sp_desktop_document(ec->desktop), SP_VERB_CONTEXT_TEXT,
               _("Create text"));
}

/**
 * Insert the character indicated by tc.uni to replace the current selection,
 * and reset tc.uni/tc.unipos to empty string.
 *
 * \pre tc.uni/tc.unipos non-empty.
 */
static void insert_uni_char(TextTool *const tc)
{
    g_return_if_fail(tc->unipos
                     && tc->unipos < sizeof(tc->uni)
                     && tc->uni[tc->unipos] == '\0');
    unsigned int uv;
    std::stringstream ss;
    ss << std::hex << tc->uni;
    ss >> uv;
    tc->unipos = 0;
    tc->uni[tc->unipos] = '\0';

    if ( !g_unichar_isprint(static_cast<gunichar>(uv))
         && !(g_unichar_validate(static_cast<gunichar>(uv)) && (g_unichar_type(static_cast<gunichar>(uv)) == G_UNICODE_PRIVATE_USE) ) ) {
        // This may be due to bad input, so it goes to statusbar.
        tc->desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE,
                                           _("Non-printable character"));
    } else {
        if (!tc->text) { // printable key; create text if none (i.e. if nascent_object)
            sp_text_context_setup_text(tc);
            tc->nascent_object = 0; // we don't need it anymore, having created a real <text>
        }

        gchar u[10];
        guint const len = g_unichar_to_utf8(uv, u);
        u[len] = '\0';

        tc->text_sel_start = tc->text_sel_end = sp_te_replace(tc->text, tc->text_sel_start, tc->text_sel_end, u);
        sp_text_context_update_cursor(tc);
        sp_text_context_update_text_selection(tc);
        DocumentUndo::done(sp_desktop_document(tc->desktop), SP_VERB_DIALOG_TRANSFORM,
               _("Insert Unicode character"));
    }
}

static void hex_to_printable_utf8_buf(char const *const ehex, char *utf8)
{
    unsigned int uv;
    std::stringstream ss;
    ss << std::hex << ehex;
    ss >> uv;
    if (!g_unichar_isprint((gunichar) uv)) {
        uv = 0xfffd;
    }
    guint const len = g_unichar_to_utf8(uv, utf8);
    utf8[len] = '\0';
}

static void show_curr_uni_char(TextTool *const tc)
{
    g_return_if_fail(tc->unipos < sizeof(tc->uni)
                     && tc->uni[tc->unipos] == '\0');
    if (tc->unipos) {
        char utf8[10];
        hex_to_printable_utf8_buf(tc->uni, utf8);

        /* Status bar messages are in pango markup, so we need xml escaping. */
        if (utf8[1] == '\0') {
            switch(utf8[0]) {
                case '<': strcpy(utf8, "&lt;"); break;
                case '>': strcpy(utf8, "&gt;"); break;
                case '&': strcpy(utf8, "&amp;"); break;
                default: break;
            }
        }
        tc->defaultMessageContext()->setF(Inkscape::NORMAL_MESSAGE,
                                          _("Unicode (<b>Enter</b> to finish): %s: %s"), tc->uni, utf8);
    } else {
        tc->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("Unicode (<b>Enter</b> to finish): "));
    }
}

bool TextTool::root_handler(GdkEvent* event) {
    sp_canvas_item_hide(this->indicator);

    sp_text_context_validate_cursor_iterators(this);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    this->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !this->space_panning) {

                if (Inkscape::have_viable_layer(desktop, desktop->messageStack()) == false) {
                    return TRUE;
                }

                // save drag origin
                this->xp = (gint) event->button.x;
                this->yp = (gint) event->button.y;
                this->within_tolerance = true;

                Geom::Point const button_pt(event->button.x, event->button.y);
                Geom::Point button_dt(desktop->w2d(button_pt));

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                m.freeSnapReturnByRef(button_dt, Inkscape::SNAPSOURCE_NODE_HANDLE);
                m.unSetup();

                this->p0 = button_dt;
                Inkscape::Rubberband::get(desktop)->start(desktop, this->p0);
                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    GDK_KEY_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
                                    NULL, event->button.time);
                this->grabbed = SP_CANVAS_ITEM(desktop->acetate);
                this->creating = 1;

                /* Processed */
                return TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (this->over_text) {
                this->over_text = 0;
                // update cursor and statusbar: we are not over a text object now
                this->cursor_shape = cursor_text_xpm;
                this->hot_x = 7;
                this->hot_y = 7;
                this->sp_event_context_update_cursor();
                desktop->event_context->defaultMessageContext()->clear();
            }

            if (this->creating && (event->motion.state & GDK_BUTTON1_MASK) && !this->space_panning) {
                if ( this->within_tolerance
                     && ( abs( (gint) event->motion.x - this->xp ) < this->tolerance )
                     && ( abs( (gint) event->motion.y - this->yp ) < this->tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to draw, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                this->within_tolerance = false;

                Geom::Point const motion_pt(event->motion.x, event->motion.y);
                Geom::Point p = desktop->w2d(motion_pt);

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_NODE_HANDLE);
                m.unSetup();

                Inkscape::Rubberband::get(desktop)->move(p);
                gobble_motion_events(GDK_BUTTON1_MASK);

                // status text
                Inkscape::Util::Quantity x_q = Inkscape::Util::Quantity(fabs((p - this->p0)[Geom::X]), "px");
                Inkscape::Util::Quantity y_q = Inkscape::Util::Quantity(fabs((p - this->p0)[Geom::Y]), "px");
                GString *xs = g_string_new(x_q.string(desktop->namedview->display_units).c_str());
                GString *ys = g_string_new(y_q.string(desktop->namedview->display_units).c_str());
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Flowed text frame</b>: %s &#215; %s"), xs->str, ys->str);

                g_string_free(xs, FALSE);
                g_string_free(ys, FALSE);

            } else if (!this->sp_event_context_knot_mouseover()) {
                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point motion_dt(desktop->w2d(motion_w));
                m.preSnap(Inkscape::SnapCandidatePoint(motion_dt, Inkscape::SNAPSOURCE_OTHER_HANDLE));
                m.unSetup();
            }
            break;
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && !this->space_panning) {
                sp_event_context_discard_delayed_snap_event(this);

                Geom::Point p1 = desktop->w2d(Geom::Point(event->button.x, event->button.y));

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                m.freeSnapReturnByRef(p1, Inkscape::SNAPSOURCE_NODE_HANDLE);
                m.unSetup();

                if (this->grabbed) {
                    sp_canvas_item_ungrab(this->grabbed, GDK_CURRENT_TIME);
                    this->grabbed = NULL;
                }

                Inkscape::Rubberband::get(desktop)->stop();

                if (this->creating && this->within_tolerance) {
                    /* Button 1, set X & Y & new item */
                    sp_desktop_selection(desktop)->clear();
                    this->pdoc = desktop->dt2doc(p1);
                    this->show = TRUE;
                    this->phase = 1;
                    this->nascent_object = 1; // new object was just created

                    /* Cursor */
                    sp_canvas_item_show(this->cursor);
                    // Cursor height is defined by the new text object's font size; it needs to be set
                    // artificially here, for the text object does not exist yet:
                    double cursor_height = sp_desktop_get_font_size_tool(desktop);
                    this->cursor->setCoords(p1, p1 + Geom::Point(0, cursor_height));
                    if (this->imc) {
                        GdkRectangle im_cursor;
                        Geom::Point const top_left = SP_EVENT_CONTEXT(this)->desktop->get_display_area().corner(3);
                        Geom::Point const cursor_size(0, cursor_height);
                        Geom::Point const im_position = SP_EVENT_CONTEXT(this)->desktop->d2w(p1 + cursor_size - top_left);
                        im_cursor.x = (int) floor(im_position[Geom::X]);
                        im_cursor.y = (int) floor(im_position[Geom::Y]);
                        im_cursor.width = 0;
                        im_cursor.height = (int) -floor(SP_EVENT_CONTEXT(this)->desktop->d2w(cursor_size)[Geom::Y]);
                        gtk_im_context_set_cursor_location(this->imc, &im_cursor);
                    }
                    this->message_context->set(Inkscape::NORMAL_MESSAGE, _("Type text; <b>Enter</b> to start new line.")); // FIXME:: this is a copy of a string from _update_cursor below, do not desync

                    this->within_tolerance = false;
                } else if (this->creating) {
                    double cursor_height = sp_desktop_get_font_size_tool(desktop);
                    if (fabs(p1[Geom::Y] - this->p0[Geom::Y]) > cursor_height) {
                        // otherwise even one line won't fit; most probably a slip of hand (even if bigger than tolerance)
                        SPItem *ft = create_flowtext_with_internal_frame (desktop, this->p0, p1);
                        /* Set style */
                        sp_desktop_apply_style_tool(desktop, ft->getRepr(), "/tools/text", true);
                        sp_desktop_selection(desktop)->set(ft);
                        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Flowed text is created."));
                        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Create flowed text"));
                    } else {
                        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("The frame is <b>too small</b> for the current font size. Flowed text not created."));
                    }
                }
                this->creating = false;
                return TRUE;
            }
            break;
        case GDK_KEY_PRESS: {
            guint const group0_keyval = get_group0_keyval(&event->key);

            if (group0_keyval == GDK_KEY_KP_Add ||
                group0_keyval == GDK_KEY_KP_Subtract) {
                if (!(event->key.state & GDK_MOD2_MASK)) // mod2 is NumLock; if on, type +/- keys
                    break; // otherwise pass on keypad +/- so they can zoom
            }

            if ((this->text) || (this->nascent_object)) {
                // there is an active text object in this context, or a new object was just created

                if (this->unimode || !this->imc
                    || (MOD__CTRL(event) && MOD__SHIFT(event))    // input methods tend to steal this for unimode,
                                                    // but we have our own so make sure they don't swallow it
                    || !gtk_im_context_filter_keypress(this->imc, (GdkEventKey*) event)) {
                    //IM did not consume the key, or we're in unimode

                    if (!MOD__CTRL_ONLY(event) && this->unimode) {
                            /* TODO: ISO 14755 (section 3 Definitions) says that we should also
                               accept the first 6 characters of alphabets other than the latin
                               alphabet "if the Latin alphabet is not used".  The below is also
                               reasonable (viz. hope that the user's keyboard includes latin
                               characters and force latin interpretation -- just as we do for our
                               keyboard shortcuts), but differs from the ISO 14755
                               recommendation. */
                            switch (group0_keyval) {
                                case GDK_KEY_space:
                                case GDK_KEY_KP_Space: {
                                    if (this->unipos) {
                                        insert_uni_char(this);
                                    }
                                    /* Stay in unimode. */
                                    show_curr_uni_char(this);
                                    return TRUE;
                                }

                                case GDK_KEY_BackSpace: {
                                    g_return_val_if_fail(this->unipos < sizeof(this->uni), TRUE);
                                    if (this->unipos) {
                                        this->uni[--this->unipos] = '\0';
                                    }
                                    show_curr_uni_char(this);
                                    return TRUE;
                                }

                                case GDK_KEY_Return:
                                case GDK_KEY_KP_Enter: {
                                    if (this->unipos) {
                                        insert_uni_char(this);
                                    }
                                    /* Exit unimode. */
                                    this->unimode = false;
                                    this->defaultMessageContext()->clear();
                                    return TRUE;
                                }

                                case GDK_KEY_Escape: {
                                    // Cancel unimode.
                                    this->unimode = false;
                                    gtk_im_context_reset(this->imc);
                                    this->defaultMessageContext()->clear();
                                    return TRUE;
                                }

                                case GDK_KEY_Shift_L:
                                case GDK_KEY_Shift_R:
                                    break;

                                default: {
                                    if (g_ascii_isxdigit(group0_keyval)) {
                                        g_return_val_if_fail(this->unipos < sizeof(this->uni) - 1, TRUE);
                                        this->uni[this->unipos++] = group0_keyval;
                                        this->uni[this->unipos] = '\0';
                                        if (this->unipos == 8) {
                                            /* This behaviour is partly to allow us to continue to
                                               use a fixed-length buffer for tc->uni.  Reason for
                                               choosing the number 8 is that it's the length of
                                               ``canonical form'' mentioned in the ISO 14755 spec.
                                               An advantage over choosing 6 is that it allows using
                                               backspace for typos & misremembering when entering a
                                               6-digit number. */
                                            insert_uni_char(this);
                                        }
                                        show_curr_uni_char(this);
                                        return TRUE;
                                    } else {
                                        /* The intent is to ignore but consume characters that could be
                                           typos for hex digits.  Gtk seems to ignore & consume all
                                           non-hex-digits, and we do similar here.  Though note that some
                                           shortcuts (like keypad +/- for zoom) get processed before
                                           reaching this code. */
                                        return TRUE;
                                    }
                                }
                            }
                        }

                        Inkscape::Text::Layout::iterator old_start = this->text_sel_start;
                        Inkscape::Text::Layout::iterator old_end = this->text_sel_end;
                        bool cursor_moved = false;
                        int screenlines = 1;
                        if (this->text) {
                            double spacing = sp_te_get_average_linespacing(this->text);
                            Geom::Rect const d = desktop->get_display_area();
                            screenlines = (int) floor(fabs(d.min()[Geom::Y] - d.max()[Geom::Y])/spacing) - 1;
                            if (screenlines <= 0)
                                screenlines = 1;
                        }

                        /* Neither unimode nor IM consumed key; process text tool shortcuts */
                        switch (group0_keyval) {
                            case GDK_KEY_x:
                            case GDK_KEY_X:
                                if (MOD__ALT_ONLY(event)) {
                                    desktop->setToolboxFocusTo ("altx-text");
                                    return TRUE;
                                }
                                break;
                            case GDK_KEY_space:
                                if (MOD__CTRL_ONLY(event)) {
                                    /* No-break space */
                                    if (!this->text) { // printable key; create text if none (i.e. if nascent_object)
                                        sp_text_context_setup_text(this);
                                        this->nascent_object = 0; // we don't need it anymore, having created a real <text>
                                    }
                                    this->text_sel_start = this->text_sel_end = sp_te_replace(this->text, this->text_sel_start, this->text_sel_end, "\302\240");
                                    sp_text_context_update_cursor(this);
                                    sp_text_context_update_text_selection(this);
                                    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("No-break space"));
                                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Insert no-break space"));
                                    return TRUE;
                                }
                                break;
                            case GDK_KEY_U:
                            case GDK_KEY_u:
                                if (MOD__CTRL_ONLY(event) || (MOD__CTRL(event) && MOD__SHIFT(event))) {
                                    if (this->unimode) {
                                        this->unimode = false;
                                        this->defaultMessageContext()->clear();
                                    } else {
                                        this->unimode = true;
                                        this->unipos = 0;
                                        this->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("Unicode (<b>Enter</b> to finish): "));
                                    }
                                    if (this->imc) {
                                        gtk_im_context_reset(this->imc);
                                    }
                                    return TRUE;
                                }
                                break;
                            case GDK_KEY_B:
                            case GDK_KEY_b:
                                if (MOD__CTRL_ONLY(event) && this->text) {
                                    SPStyle const *style = sp_te_style_at_position(this->text, std::min(this->text_sel_start, this->text_sel_end));
                                    SPCSSAttr *css = sp_repr_css_attr_new();
                                    if (style->font_weight.computed == SP_CSS_FONT_WEIGHT_NORMAL
                                        || style->font_weight.computed == SP_CSS_FONT_WEIGHT_100
                                        || style->font_weight.computed == SP_CSS_FONT_WEIGHT_200
                                        || style->font_weight.computed == SP_CSS_FONT_WEIGHT_300
                                        || style->font_weight.computed == SP_CSS_FONT_WEIGHT_400)
                                        sp_repr_css_set_property(css, "font-weight", "bold");
                                    else
                                        sp_repr_css_set_property(css, "font-weight", "normal");
                                    sp_te_apply_style(this->text, this->text_sel_start, this->text_sel_end, css);
                                    sp_repr_css_attr_unref(css);
                                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Make bold"));
                                    sp_text_context_update_cursor(this);
                                    sp_text_context_update_text_selection(this);
                                    return TRUE;
                                }
                                break;
                            case GDK_KEY_I:
                            case GDK_KEY_i:
                                if (MOD__CTRL_ONLY(event) && this->text) {
                                    SPStyle const *style = sp_te_style_at_position(this->text, std::min(this->text_sel_start, this->text_sel_end));
                                    SPCSSAttr *css = sp_repr_css_attr_new();
                                    if (style->font_style.computed != SP_CSS_FONT_STYLE_NORMAL)
                                        sp_repr_css_set_property(css, "font-style", "normal");
                                    else
                                        sp_repr_css_set_property(css, "font-style", "italic");
                                    sp_te_apply_style(this->text, this->text_sel_start, this->text_sel_end, css);
                                    sp_repr_css_attr_unref(css);
                                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Make italic"));
                                    sp_text_context_update_cursor(this);
                                    sp_text_context_update_text_selection(this);
                                    return TRUE;
                                }
                                break;

                            case GDK_KEY_A:
                            case GDK_KEY_a:
                                if (MOD__CTRL_ONLY(event) && this->text) {
                                    Inkscape::Text::Layout const *layout = te_get_layout(this->text);
                                    if (layout) {
                                        this->text_sel_start = layout->begin();
                                        this->text_sel_end = layout->end();
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        return TRUE;
                                    }
                                }
                                break;

                            case GDK_KEY_Return:
                            case GDK_KEY_KP_Enter:
                            {
                                if (!this->text) { // printable key; create text if none (i.e. if nascent_object)
                                    sp_text_context_setup_text(this);
                                    this->nascent_object = 0; // we don't need it anymore, having created a real <text>
                                }

                                iterator_pair enter_pair;
                                bool success = sp_te_delete(this->text, this->text_sel_start, this->text_sel_end, enter_pair);
                                (void)success; // TODO cleanup
                                this->text_sel_start = this->text_sel_end = enter_pair.first;

                                this->text_sel_start = this->text_sel_end = sp_te_insert_line(this->text, this->text_sel_start);

                                sp_text_context_update_cursor(this);
                                sp_text_context_update_text_selection(this);
                                DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("New line"));
                                return TRUE;
                            }
                            case GDK_KEY_BackSpace:
                                if (this->text) { // if nascent_object, do nothing, but return TRUE; same for all other delete and move keys

                                    bool noSelection = false;

                                    if (MOD__CTRL(event)) {
                                        this->text_sel_start = this->text_sel_end;
                                    }

                                    if (this->text_sel_start == this->text_sel_end) {
                                        if (MOD__CTRL(event)) {
                                            this->text_sel_start.prevStartOfWord();
                                        } else {
                                            this->text_sel_start.prevCursorPosition();
                                        }
                                        noSelection = true;
                                    }

                                    iterator_pair bspace_pair;
                                    bool success = sp_te_delete(this->text, this->text_sel_start, this->text_sel_end, bspace_pair);

                                    if (noSelection) {
                                        if (success) {
                                            this->text_sel_start = this->text_sel_end = bspace_pair.first;
                                        } else { // nothing deleted
                                            this->text_sel_start = this->text_sel_end = bspace_pair.second;
                                        }
                                    } else {
                                        if (success) {
                                            this->text_sel_start = this->text_sel_end = bspace_pair.first;
                                        } else { // nothing deleted
                                            this->text_sel_start = bspace_pair.first;
                                            this->text_sel_end = bspace_pair.second;
                                        }
                                    }

                                    sp_text_context_update_cursor(this);
                                    sp_text_context_update_text_selection(this);
                                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Backspace"));
                                }
                                return TRUE;
                            case GDK_KEY_Delete:
                            case GDK_KEY_KP_Delete:
                                if (this->text) {
                                    bool noSelection = false;

                                    if (MOD__CTRL(event)) {
                                        this->text_sel_start = this->text_sel_end;
                                    }

                                    if (this->text_sel_start == this->text_sel_end) {
                                        if (MOD__CTRL(event)) {
                                            this->text_sel_end.nextEndOfWord();
                                        } else {
                                            this->text_sel_end.nextCursorPosition();
                                        }
                                        noSelection = true;
                                    }

                                    iterator_pair del_pair;
                                    bool success = sp_te_delete(this->text, this->text_sel_start, this->text_sel_end, del_pair);

                                    if (noSelection) {
                                        this->text_sel_start = this->text_sel_end = del_pair.first;
                                    } else {
                                        if (success) {
                                            this->text_sel_start = this->text_sel_end = del_pair.first;
                                        } else { // nothing deleted
                                            this->text_sel_start = del_pair.first;
                                            this->text_sel_end = del_pair.second;
                                        }
                                    }


                                    sp_text_context_update_cursor(this);
                                    sp_text_context_update_text_selection(this);
                                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Delete"));
                                }
                                return TRUE;
                            case GDK_KEY_Left:
                            case GDK_KEY_KP_Left:
                            case GDK_KEY_KP_4:
                                if (this->text) {
                                    if (MOD__ALT(event)) {
                                        gint mul = 1 + gobble_key_events(
                                            get_group0_keyval(&event->key), 0); // with any mask
                                        if (MOD__SHIFT(event))
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(mul*-10, 0));
                                        else
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(mul*-1, 0));
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        DocumentUndo::maybeDone(sp_desktop_document(desktop), "kern:left", SP_VERB_CONTEXT_TEXT, _("Kern to the left"));
                                    } else {
                                        if (MOD__CTRL(event))
                                            this->text_sel_end.cursorLeftWithControl();
                                        else
                                            this->text_sel_end.cursorLeft();
                                        cursor_moved = true;
                                        break;
                                    }
                                }
                                return TRUE;
                            case GDK_KEY_Right:
                            case GDK_KEY_KP_Right:
                            case GDK_KEY_KP_6:
                                if (this->text) {
                                    if (MOD__ALT(event)) {
                                        gint mul = 1 + gobble_key_events(
                                            get_group0_keyval(&event->key), 0); // with any mask
                                        if (MOD__SHIFT(event))
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(mul*10, 0));
                                        else
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(mul*1, 0));
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        DocumentUndo::maybeDone(sp_desktop_document(desktop), "kern:right", SP_VERB_CONTEXT_TEXT, _("Kern to the right"));
                                    } else {
                                        if (MOD__CTRL(event))
                                            this->text_sel_end.cursorRightWithControl();
                                        else
                                            this->text_sel_end.cursorRight();
                                        cursor_moved = true;
                                        break;
                                    }
                                }
                                return TRUE;
                            case GDK_KEY_Up:
                            case GDK_KEY_KP_Up:
                            case GDK_KEY_KP_8:
                                if (this->text) {
                                    if (MOD__ALT(event)) {
                                        gint mul = 1 + gobble_key_events(
                                            get_group0_keyval(&event->key), 0); // with any mask
                                        if (MOD__SHIFT(event))
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(0, mul*-10));
                                        else
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(0, mul*-1));
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        DocumentUndo::maybeDone(sp_desktop_document(desktop), "kern:up", SP_VERB_CONTEXT_TEXT, _("Kern up"));
                                    } else {
                                        if (MOD__CTRL(event))
                                            this->text_sel_end.cursorUpWithControl();
                                        else
                                            this->text_sel_end.cursorUp();
                                        cursor_moved = true;
                                        break;
                                    }
                                }
                                return TRUE;
                            case GDK_KEY_Down:
                            case GDK_KEY_KP_Down:
                            case GDK_KEY_KP_2:
                                if (this->text) {
                                    if (MOD__ALT(event)) {
                                        gint mul = 1 + gobble_key_events(
                                            get_group0_keyval(&event->key), 0); // with any mask
                                        if (MOD__SHIFT(event))
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(0, mul*10));
                                        else
                                            sp_te_adjust_kerning_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, Geom::Point(0, mul*1));
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        DocumentUndo::maybeDone(sp_desktop_document(desktop), "kern:down", SP_VERB_CONTEXT_TEXT, _("Kern down"));
                                    } else {
                                        if (MOD__CTRL(event))
                                            this->text_sel_end.cursorDownWithControl();
                                        else
                                            this->text_sel_end.cursorDown();
                                        cursor_moved = true;
                                        break;
                                    }
                                }
                                return TRUE;
                            case GDK_KEY_Home:
                            case GDK_KEY_KP_Home:
                                if (this->text) {
                                    if (MOD__CTRL(event))
                                        this->text_sel_end.thisStartOfShape();
                                    else
                                        this->text_sel_end.thisStartOfLine();
                                    cursor_moved = true;
                                    break;
                                }
                                return TRUE;
                            case GDK_KEY_End:
                            case GDK_KEY_KP_End:
                                if (this->text) {
                                    if (MOD__CTRL(event))
                                        this->text_sel_end.nextStartOfShape();
                                    else
                                        this->text_sel_end.thisEndOfLine();
                                    cursor_moved = true;
                                    break;
                                }
                                return TRUE;
                            case GDK_KEY_Page_Down:
                            case GDK_KEY_KP_Page_Down:
                                if (this->text) {
                                    this->text_sel_end.cursorDown(screenlines);
                                    cursor_moved = true;
                                    break;
                                }
                                return TRUE;
                            case GDK_KEY_Page_Up:
                            case GDK_KEY_KP_Page_Up:
                                if (this->text) {
                                    this->text_sel_end.cursorUp(screenlines);
                                    cursor_moved = true;
                                    break;
                                }
                                return TRUE;
                            case GDK_KEY_Escape:
                                if (this->creating) {
                                    this->creating = 0;
                                    if (this->grabbed) {
                                        sp_canvas_item_ungrab(this->grabbed, GDK_CURRENT_TIME);
                                        this->grabbed = NULL;
                                    }
                                    Inkscape::Rubberband::get(desktop)->stop();
                                } else {
                                    sp_desktop_selection(desktop)->clear();
                                }
                                this->nascent_object = FALSE;
                                return TRUE;
                            case GDK_KEY_bracketleft:
                                if (this->text) {
                                    if (MOD__ALT(event) || MOD__CTRL(event)) {
                                        if (MOD__ALT(event)) {
                                            if (MOD__SHIFT(event)) {
                                                // FIXME: alt+shift+[] does not work, don't know why
                                                sp_te_adjust_rotation_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, -10);
                                            } else {
                                                sp_te_adjust_rotation_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, -1);
                                            }
                                        } else {
                                            sp_te_adjust_rotation(this->text, this->text_sel_start, this->text_sel_end, desktop, -90);
                                        }
                                        DocumentUndo::maybeDone(sp_desktop_document(desktop), "textrot:ccw", SP_VERB_CONTEXT_TEXT, _("Rotate counterclockwise"));
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        return TRUE;
                                    }
                                }
                                break;
                            case GDK_KEY_bracketright:
                                if (this->text) {
                                    if (MOD__ALT(event) || MOD__CTRL(event)) {
                                        if (MOD__ALT(event)) {
                                            if (MOD__SHIFT(event)) {
                                                // FIXME: alt+shift+[] does not work, don't know why
                                                sp_te_adjust_rotation_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, 10);
                                            } else {
                                                sp_te_adjust_rotation_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, 1);
                                            }
                                        } else {
                                            sp_te_adjust_rotation(this->text, this->text_sel_start, this->text_sel_end, desktop, 90);
                                        }
                                        DocumentUndo::maybeDone(sp_desktop_document(desktop), "textrot:cw", SP_VERB_CONTEXT_TEXT, _("Rotate clockwise"));
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        return TRUE;
                                    }
                                }
                                break;
                            case GDK_KEY_less:
                            case GDK_KEY_comma:
                                if (this->text) {
                                    if (MOD__ALT(event)) {
                                        if (MOD__CTRL(event)) {
                                            if (MOD__SHIFT(event))
                                                sp_te_adjust_linespacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, -10);
                                            else
                                                sp_te_adjust_linespacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, -1);
                                            DocumentUndo::maybeDone(sp_desktop_document(desktop), "linespacing:dec", SP_VERB_CONTEXT_TEXT, _("Contract line spacing"));
                                        } else {
                                            if (MOD__SHIFT(event))
                                                sp_te_adjust_tspan_letterspacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, -10);
                                            else
                                                sp_te_adjust_tspan_letterspacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, -1);
                                            DocumentUndo::maybeDone(sp_desktop_document(desktop), "letterspacing:dec", SP_VERB_CONTEXT_TEXT, _("Contract letter spacing"));
                                        }
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        return TRUE;
                                    }
                                }
                                break;
                            case GDK_KEY_greater:
                            case GDK_KEY_period:
                                if (this->text) {
                                    if (MOD__ALT(event)) {
                                        if (MOD__CTRL(event)) {
                                            if (MOD__SHIFT(event))
                                                sp_te_adjust_linespacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, 10);
                                            else
                                                sp_te_adjust_linespacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, 1);
                                            DocumentUndo::maybeDone(sp_desktop_document(desktop), "linespacing:inc", SP_VERB_CONTEXT_TEXT, _("Expand line spacing"));
                                        } else {
                                            if (MOD__SHIFT(event))
                                                sp_te_adjust_tspan_letterspacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, 10);
                                            else
                                                sp_te_adjust_tspan_letterspacing_screen(this->text, this->text_sel_start, this->text_sel_end, desktop, 1);
                                            DocumentUndo::maybeDone(sp_desktop_document(desktop), "letterspacing:inc", SP_VERB_CONTEXT_TEXT, _("Expand letter spacing"));\
                                        }
                                        sp_text_context_update_cursor(this);
                                        sp_text_context_update_text_selection(this);
                                        return TRUE;
                                    }
                                }
                                break;
                            default:
                                break;
                        }

                        if (cursor_moved) {
                            if (!MOD__SHIFT(event))
                                this->text_sel_start = this->text_sel_end;
                            if (old_start != this->text_sel_start || old_end != this->text_sel_end) {
                                sp_text_context_update_cursor(this);
                                sp_text_context_update_text_selection(this);
                            }
                            return TRUE;
                        }

                } else return TRUE; // return the "I took care of it" value if it was consumed by the IM
            } else { // do nothing if there's no object to type in - the key will be sent to parent context,
                // except up/down that are swallowed to prevent the zoom field from activation
                if ((group0_keyval == GDK_KEY_Up    ||
                     group0_keyval == GDK_KEY_Down  ||
                     group0_keyval == GDK_KEY_KP_Up ||
                     group0_keyval == GDK_KEY_KP_Down )
                    && !MOD__CTRL_ONLY(event)) {
                    return TRUE;
                } else if (group0_keyval == GDK_KEY_Escape) { // cancel rubberband
                    if (this->creating) {
                        this->creating = 0;
                        if (this->grabbed) {
                            sp_canvas_item_ungrab(this->grabbed, GDK_CURRENT_TIME);
                            this->grabbed = NULL;
                        }
                        Inkscape::Rubberband::get(desktop)->stop();
                    }
                } else if ((group0_keyval == GDK_KEY_x || group0_keyval == GDK_KEY_X) && MOD__ALT_ONLY(event)) {
                    desktop->setToolboxFocusTo ("altx-text");
                    return TRUE;
                }
            }
            break;
        }

        case GDK_KEY_RELEASE:
            if (!this->unimode && this->imc && gtk_im_context_filter_keypress(this->imc, (GdkEventKey*) event)) {
                return TRUE;
            }
            break;
        default:
            break;
    }

    // if nobody consumed it so far
//    if ((SP_EVENT_CONTEXT_CLASS(sp_text_context_parent_class))->root_handler) { // and there's a handler in parent context,
//        return (SP_EVENT_CONTEXT_CLASS(sp_text_context_parent_class))->root_handler(event_context, event); // send event to parent
//    } else {
//        return FALSE; // return "I did nothing" value so that global shortcuts can be activated
//    }
    return ToolBase::root_handler(event);

}

/**
 Attempts to paste system clipboard into the currently edited text, returns true on success
 */
bool sp_text_paste_inline(ToolBase *ec)
{
    if (!SP_IS_TEXT_CONTEXT(ec))
        return false;

    TextTool *tc = SP_TEXT_CONTEXT(ec);

    if ((tc->text) || (tc->nascent_object)) {
        // there is an active text object in this context, or a new object was just created

        Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
        Glib::ustring const clip_text = refClipboard->wait_for_text();

        if (!clip_text.empty()) {
            // Fix for 244940
            // The XML standard defines the following as valid characters
            // (Extensible Markup Language (XML) 1.0 (Fourth Edition) paragraph 2.2)
            // char ::=     #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
            // Since what comes in off the paste buffer will go right into XML, clean
            // the text here.
            Glib::ustring text(clip_text);
            Glib::ustring::iterator itr = text.begin();
            gunichar paste_string_uchar;

            while(itr != text.end())
            {
                paste_string_uchar = *itr;

                // Make sure we don't have a control character. We should really check
                // for the whole range above... Add the rest of the invalid cases from
                // above if we find additional issues
                if(paste_string_uchar >= 0x00000020 ||
                   paste_string_uchar == 0x00000009 ||
                   paste_string_uchar == 0x0000000A ||
                   paste_string_uchar == 0x0000000D) {
                    ++itr;
                } else {
                    itr = text.erase(itr);
                }
            }

            if (!tc->text) { // create text if none (i.e. if nascent_object)
                sp_text_context_setup_text(tc);
                tc->nascent_object = 0; // we don't need it anymore, having created a real <text>
            }

            // using indices is slow in ustrings. Whatever.
            Glib::ustring::size_type begin = 0;
            for ( ; ; ) {
                Glib::ustring::size_type end = text.find('\n', begin);
                if (end == Glib::ustring::npos) {
                    if (begin != text.length())
                        tc->text_sel_start = tc->text_sel_end = sp_te_replace(tc->text, tc->text_sel_start, tc->text_sel_end, text.substr(begin).c_str());
                    break;
                }
                tc->text_sel_start = tc->text_sel_end = sp_te_replace(tc->text, tc->text_sel_start, tc->text_sel_end, text.substr(begin, end - begin).c_str());
                tc->text_sel_start = tc->text_sel_end = sp_te_insert_line(tc->text, tc->text_sel_start);
                begin = end + 1;
            }
            DocumentUndo::done(sp_desktop_document(ec->desktop), SP_VERB_CONTEXT_TEXT,
                               _("Paste text"));

            return true;
        }
    } // FIXME: else create and select a new object under cursor!

    return false;
}

/**
 Gets the raw characters that comprise the currently selected text, converting line
 breaks into lf characters.
*/
Glib::ustring sp_text_get_selected_text(ToolBase const *ec)
{
    if (!SP_IS_TEXT_CONTEXT(ec))
        return "";
    TextTool const *tc = SP_TEXT_CONTEXT(ec);
    if (tc->text == NULL)
        return "";

    return sp_te_get_string_multiline(tc->text, tc->text_sel_start, tc->text_sel_end);
}

SPCSSAttr *sp_text_get_style_at_cursor(ToolBase const *ec)
{
    if (!SP_IS_TEXT_CONTEXT(ec))
        return NULL;
    TextTool const *tc = SP_TEXT_CONTEXT(ec);
    if (tc->text == NULL)
        return NULL;

    SPObject const *obj = sp_te_object_at_position(tc->text, tc->text_sel_end);

    if (obj) {
        return take_style_from_item(const_cast<SPObject*>(obj));
    }

    return NULL;
}

/**
 Deletes the currently selected characters. Returns false if there is no
 text selection currently.
*/
bool sp_text_delete_selection(ToolBase *ec)
{
    if (!SP_IS_TEXT_CONTEXT(ec))
        return false;
    TextTool *tc = SP_TEXT_CONTEXT(ec);
    if (tc->text == NULL)
        return false;

    if (tc->text_sel_start == tc->text_sel_end)
        return false;

    iterator_pair pair;
    bool success = sp_te_delete(tc->text, tc->text_sel_start, tc->text_sel_end, pair);


    if (success) {
        tc->text_sel_start = tc->text_sel_end = pair.first;
    } else { // nothing deleted
        tc->text_sel_start = pair.first;
        tc->text_sel_end = pair.second;
    }

    sp_text_context_update_cursor(tc);
    sp_text_context_update_text_selection(tc);

    return true;
}

/**
 * \param selection Should not be NULL.
 */
void TextTool::_selectionChanged(Inkscape::Selection *selection)
{
    g_assert(selection != NULL);

    ToolBase *ec = SP_EVENT_CONTEXT(this);

    ec->shape_editor->unset_item();
    SPItem *item = selection->singleItem();
    if (item && SP_IS_FLOWTEXT(item) && SP_FLOWTEXT(item)->has_internal_frame()) {
        ec->shape_editor->set_item(item);
    }

    if (this->text && (item != this->text)) {
        sp_text_context_forget_text(this);
    }
    this->text = NULL;

    if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
        this->text = item;
        Inkscape::Text::Layout const *layout = te_get_layout(this->text);
        if (layout)
            this->text_sel_start = this->text_sel_end = layout->end();
    } else {
        this->text = NULL;
    }

    // we update cursor without scrolling, because this position may not be final;
    // item_handler moves cusros to the point of click immediately
    sp_text_context_update_cursor(this, false);
    sp_text_context_update_text_selection(this);
}

void TextTool::_selectionModified(Inkscape::Selection */*selection*/, guint /*flags*/)
{
    sp_text_context_update_cursor(this);
    sp_text_context_update_text_selection(this);
}

bool TextTool::_styleSet(SPCSSAttr const *css)
{
    if (this->text == NULL)
        return false;
    if (this->text_sel_start == this->text_sel_end)
        return false;    // will get picked up by the parent and applied to the whole text object

    sp_te_apply_style(this->text, this->text_sel_start, this->text_sel_end, css);
    DocumentUndo::done(sp_desktop_document(this->desktop), SP_VERB_CONTEXT_TEXT,
               _("Set text style"));
    sp_text_context_update_cursor(this);
    sp_text_context_update_text_selection(this);

    return true;
}

int TextTool::_styleQueried(SPStyle *style, int property)
{
    if (this->text == NULL) {
        return QUERY_STYLE_NOTHING;
    }
    const Inkscape::Text::Layout *layout = te_get_layout(this->text);
    if (layout == NULL) {
        return QUERY_STYLE_NOTHING;
    }
    sp_text_context_validate_cursor_iterators(this);

    GSList *styles_list = NULL;

    Inkscape::Text::Layout::iterator begin_it, end_it;
    if (this->text_sel_start < this->text_sel_end) {
        begin_it = this->text_sel_start;
        end_it = this->text_sel_end;
    } else {
        begin_it = this->text_sel_end;
        end_it = this->text_sel_start;
    }
    if (begin_it == end_it) {
        if (!begin_it.prevCharacter()) {
            end_it.nextCharacter();
        }
    }
    for (Inkscape::Text::Layout::iterator it = begin_it ; it < end_it ; it.nextStartOfSpan()) {
        SPObject const *pos_obj = 0;
        void *rawptr = 0;
        layout->getSourceOfCharacter(it, &rawptr);
        if (!rawptr || !SP_IS_OBJECT(rawptr)) {
            continue;
        }
        pos_obj = SP_OBJECT(rawptr);
        while (SP_IS_STRING(pos_obj) && pos_obj->parent) {
           pos_obj = pos_obj->parent;   // SPStrings don't have style
        }
        styles_list = g_slist_prepend(styles_list, (gpointer)pos_obj);
    }

    int result = sp_desktop_query_style_from_list (styles_list, style, property);

    g_slist_free(styles_list);
    return result;
}

static void sp_text_context_validate_cursor_iterators(TextTool *tc)
{
    if (tc->text == NULL)
        return;
    Inkscape::Text::Layout const *layout = te_get_layout(tc->text);
    if (layout) {     // undo can change the text length without us knowing it
        layout->validateIterator(&tc->text_sel_start);
        layout->validateIterator(&tc->text_sel_end);
    }
}

static void sp_text_context_update_cursor(TextTool *tc,  bool scroll_to_see)
{
    // due to interruptible display, tc may already be destroyed during a display update before
    // the cursor update (can't do both atomically, alas)
    if (!tc->desktop) return;

    if (tc->text) {
        Geom::Point p0, p1;
        sp_te_get_cursor_coords(tc->text, tc->text_sel_end, p0, p1);
        Geom::Point const d0 = p0 * tc->text->i2dt_affine();
        Geom::Point const d1 = p1 * tc->text->i2dt_affine();

        // scroll to show cursor
        if (scroll_to_see) {
            Geom::Point const center = SP_EVENT_CONTEXT(tc)->desktop->get_display_area().midpoint();
            if (Geom::L2(d0 - center) > Geom::L2(d1 - center))
                // unlike mouse moves, here we must scroll all the way at first shot, so we override the autoscrollspeed
                SP_EVENT_CONTEXT(tc)->desktop->scroll_to_point(d0, 1.0);
            else
                SP_EVENT_CONTEXT(tc)->desktop->scroll_to_point(d1, 1.0);
        }

        sp_canvas_item_show(tc->cursor);
        tc->cursor->setCoords(d0, d1);

        /* fixme: ... need another transformation to get canvas widget coordinate space? */
        if (tc->imc) {
            GdkRectangle im_cursor = { 0, 0, 1, 1 };
            Geom::Point const top_left = SP_EVENT_CONTEXT(tc)->desktop->get_display_area().corner(3);
            Geom::Point const im_d0 = SP_EVENT_CONTEXT(tc)->desktop->d2w(d0 - top_left);
            Geom::Point const im_d1 = SP_EVENT_CONTEXT(tc)->desktop->d2w(d1 - top_left);
            im_cursor.x = (int) floor(im_d0[Geom::X]);
            im_cursor.y = (int) floor(im_d1[Geom::Y]);
            im_cursor.width = (int) floor(im_d1[Geom::X]) - im_cursor.x;
            im_cursor.height = (int) floor(im_d0[Geom::Y]) - im_cursor.y;
            gtk_im_context_set_cursor_location(tc->imc, &im_cursor);
        }

        tc->show = TRUE;
        tc->phase = 1;

        Inkscape::Text::Layout const *layout = te_get_layout(tc->text);
        int const nChars = layout->iteratorToCharIndex(layout->end());
        char const *trunc = "";
        bool truncated = false;
        if (layout->inputTruncated()) {
            truncated = true;
            trunc = _(" [truncated]");
        }
        if (SP_IS_FLOWTEXT(tc->text)) {
            SPItem *frame = SP_FLOWTEXT(tc->text)->get_frame (NULL); // first frame only
            if (frame) {
                if (truncated) {
                    SP_CTRLRECT(tc->frame)->setColor(0xff0000ff, false, 0);
                } else {
                    SP_CTRLRECT(tc->frame)->setColor(0x0000ff7f, false, 0);
                }
                sp_canvas_item_show(tc->frame);
                Geom::OptRect frame_bbox = frame->desktopVisualBounds();
                if (frame_bbox) {
                    SP_CTRLRECT(tc->frame)->setRectangle(*frame_bbox);
                }
            }

            SP_EVENT_CONTEXT(tc)->message_context->setF(Inkscape::NORMAL_MESSAGE, ngettext("Type or edit flowed text (%d character%s); <b>Enter</b> to start new paragraph.", "Type or edit flowed text (%d characters%s); <b>Enter</b> to start new paragraph.", nChars), nChars, trunc);
        } else {
            SP_EVENT_CONTEXT(tc)->message_context->setF(Inkscape::NORMAL_MESSAGE, ngettext("Type or edit text (%d character%s); <b>Enter</b> to start new line.", "Type or edit text (%d characters%s); <b>Enter</b> to start new line.", nChars), nChars, trunc);
        }

    } else {
        sp_canvas_item_hide(tc->cursor);
        sp_canvas_item_hide(tc->frame);
        tc->show = FALSE;
        if (!tc->nascent_object) {
            SP_EVENT_CONTEXT(tc)->message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> to select or create text, <b>drag</b> to create flowed text; then type.")); // FIXME: this is a copy of string from tools-switch, do not desync
        }
    }

    SP_EVENT_CONTEXT(tc)->desktop->emitToolSubselectionChanged((gpointer)tc);
}

static void sp_text_context_update_text_selection(TextTool *tc)
{
    // due to interruptible display, tc may already be destroyed during a display update before
    // the selection update (can't do both atomically, alas)
    if (!tc->desktop) return;

    for (std::vector<SPCanvasItem*>::iterator it = tc->text_selection_quads.begin() ; it != tc->text_selection_quads.end() ; ++it) {
        sp_canvas_item_hide(*it);
        sp_canvas_item_destroy(*it);
    }
    tc->text_selection_quads.clear();

    std::vector<Geom::Point> quads;
    if (tc->text != NULL)
        quads = sp_te_create_selection_quads(tc->text, tc->text_sel_start, tc->text_sel_end, (tc->text)->i2dt_affine());
    for (unsigned i = 0 ; i < quads.size() ; i += 4) {
        SPCanvasItem *quad_canvasitem;
        quad_canvasitem = sp_canvas_item_new(sp_desktop_controls(tc->desktop), SP_TYPE_CTRLQUADR, NULL);
        // FIXME: make the color settable in prefs
        // for now, use semitrasparent blue, as cairo cannot do inversion :(
        sp_ctrlquadr_set_rgba32(SP_CTRLQUADR(quad_canvasitem), 0x00777777);
        sp_ctrlquadr_set_coords(SP_CTRLQUADR(quad_canvasitem), quads[i], quads[i+1], quads[i+2], quads[i+3]);
        sp_canvas_item_show(quad_canvasitem);
        tc->text_selection_quads.push_back(quad_canvasitem);
    }
}

static gint sp_text_context_timeout(TextTool *tc)
{
    if (tc->show) {
        sp_canvas_item_show(tc->cursor);
        if (tc->phase) {
            tc->phase = 0;
            tc->cursor->setRgba32(0x000000ff);
        } else {
            tc->phase = 1;
            tc->cursor->setRgba32(0xffffffff);
        }
    }

    return TRUE;
}

static void sp_text_context_forget_text(TextTool *tc)
{
    if (! tc->text) return;
    SPItem *ti = tc->text;
    (void)ti;
    /* We have to set it to zero,
     * or selection changed signal messes everything up */
    tc->text = NULL;

/* FIXME: this automatic deletion when nothing is inputted crashes the XML edittor and also crashes when duplicating an empty flowtext.
    So don't create an empty flowtext in the first place? Create it when first character is typed.
    */
/*
    if ((SP_IS_TEXT(ti) || SP_IS_FLOWTEXT(ti)) && sp_te_input_is_empty(ti)) {
        Inkscape::XML::Node *text_repr = ti->getRepr();
        // the repr may already have been unparented
        // if we were called e.g. as the result of
        // an undo or the element being removed from
        // the XML editor
        if ( text_repr && text_repr->parent() ) {
            sp_repr_unparent(text_repr);
            SPDocumentUndo::done(sp_desktop_document(tc->desktop), SP_VERB_CONTEXT_TEXT,
                     _("Remove empty text"));
        }
    }
*/
}

gint sptc_focus_in(GtkWidget */*widget*/, GdkEventFocus */*event*/, TextTool *tc)
{
    gtk_im_context_focus_in(tc->imc);
    return FALSE;
}

gint sptc_focus_out(GtkWidget */*widget*/, GdkEventFocus */*event*/, TextTool *tc)
{
    gtk_im_context_focus_out(tc->imc);
    return FALSE;
}

static void sptc_commit(GtkIMContext */*imc*/, gchar *string, TextTool *tc)
{
    if (!tc->text) {
        sp_text_context_setup_text(tc);
        tc->nascent_object = 0; // we don't need it anymore, having created a real <text>
    }

    tc->text_sel_start = tc->text_sel_end = sp_te_replace(tc->text, tc->text_sel_start, tc->text_sel_end, string);
    sp_text_context_update_cursor(tc);
    sp_text_context_update_text_selection(tc);

    DocumentUndo::done(tc->text->document, SP_VERB_CONTEXT_TEXT,
               _("Type text"));
}

void sp_text_context_place_cursor (TextTool *tc, SPObject *text, Inkscape::Text::Layout::iterator where)
{
    tc->desktop->selection->set (text);
    tc->text_sel_start = tc->text_sel_end = where;
    sp_text_context_update_cursor(tc);
    sp_text_context_update_text_selection(tc);
}

void sp_text_context_place_cursor_at (TextTool *tc, SPObject *text, Geom::Point const p)
{
    tc->desktop->selection->set (text);
    sp_text_context_place_cursor (tc, text, sp_te_get_position_by_coords(tc->text, p));
}

Inkscape::Text::Layout::iterator *sp_text_context_get_cursor_position(TextTool *tc, SPObject *text)
{
    if (text != tc->text)
        return NULL;
    return &(tc->text_sel_end);
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

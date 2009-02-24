/** @file
 * @brief  Spellcheck dialog
 */
/* Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "widgets/icon.h"
#include "message-stack.h"

#include <gtk/gtk.h>

#include <glibmm/i18n.h>
#include "helper/window.h"
#include "macros.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "desktop-handles.h"
#include "dialog-events.h"
#include "tools-switch.h"
#include "text-context.h"
#include "../interface.h"
#include "../preferences.h"
#include "../sp-text.h"
#include "../sp-flowtext.h"
#include "../text-editing.h"
#include "../sp-tspan.h"
#include "../sp-tref.h"
#include "../sp-defs.h"
#include "../selection-chemistry.h"
#include <xml/repr.h>
#include "display/canvas-bpath.h"
#include "display/curve.h"

#ifdef HAVE_ASPELL
#include <aspell.h>

#ifdef WIN32
#include <windows.h>
#endif

#define MIN_ONSCREEN_DISTANCE 50

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static Glib::ustring const prefs_path = "/dialogs/spellcheck/";

// C++ for the poor: instead of creating a formal C++ class, I just treat this entire file as a
// class, with the globals as its data fields. In such a simple case as this, when no inheritance
// or encapsulation are necessary, this is much simpler and less verbose, and mixes easily with
// plain-C GTK callbacks.

static SPDesktop *_desktop = NULL;
static SPObject *_root;

static AspellSpeller *_speller = NULL;
static AspellSpeller *_speller2 = NULL;
static AspellSpeller *_speller3 = NULL;

// list of canvasitems (currently just rects) that mark misspelled things on canvas
static GSList *_rects = NULL;

// list of text objects we have already checked in this session
static GSList *_seen_objects = NULL;

// the object currently being checked
static SPItem *_text = NULL;
// its layout
static Inkscape::Text::Layout const *_layout = NULL;

// iterators for the start and end of the current word
static Inkscape::Text::Layout::iterator _begin_w;
static Inkscape::Text::Layout::iterator _end_w;

// the word we're checking
static Glib::ustring _word;

// counters for the number of stops and dictionary adds
static int _stops = 0;
static int _adds = 0;

// true if we are in the middle of a check
static bool _working = false;

// connect to the object being checked in case it is modified or deleted by user
static sigc::connection *_modified_connection = NULL;
static sigc::connection *_release_connection = NULL;

// true if the spell checker dialog has changed text, to suppress modified callback
static bool _local_change = false;

static Inkscape::Preferences *_prefs = NULL;

static gchar *_lang = NULL;
static gchar *_lang2 = NULL;
static gchar *_lang3 = NULL;


void spellcheck_clear_rects()
{
    for (GSList *it = _rects; it; it = it->next) {
        sp_canvas_item_hide((SPCanvasItem*) it->data);
        gtk_object_destroy((SPCanvasItem*) it->data);
    }
    g_slist_free(_rects);
    _rects = NULL;
}

void spellcheck_clear_langs()
{
    if (_lang) {
        g_free(_lang);
        _lang = NULL;
    }
    if (_lang2) {
        g_free(_lang2);
        _lang2 = NULL;
    }
    if (_lang3) {
        g_free(_lang3);
        _lang3 = NULL;
    }
}

void
spellcheck_disconnect()
{
    if (_release_connection) {
        _release_connection->disconnect();
        delete _release_connection;
        _release_connection = NULL;
    }
    if (_modified_connection) {
        _modified_connection->disconnect();
        delete _modified_connection;
        _modified_connection = NULL;
    }
}

static void sp_spellcheck_dialog_destroy(GtkObject *object, gpointer)
{
    spellcheck_clear_rects();
    spellcheck_clear_langs();
    spellcheck_disconnect();

    sp_signal_disconnect_by_data (INKSCAPE, object);
    wd.win = dlg = NULL;
    wd.stop = 0;
}


static gboolean sp_spellcheck_dialog_delete(GtkObject *, GdkEvent *, gpointer /*data*/)
{
    spellcheck_clear_rects();
    spellcheck_clear_langs();
    spellcheck_disconnect();

    gtk_window_get_position (GTK_WINDOW (dlg), &x, &y);
    gtk_window_get_size (GTK_WINDOW (dlg), &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    _prefs->setInt(prefs_path + "x", x);
    _prefs->setInt(prefs_path + "y", y);
    _prefs->setInt(prefs_path + "w", w);
    _prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it
}

void
sp_spellcheck_new_button (GtkWidget *dlg, GtkWidget *hb, const gchar *label, GtkTooltips *tt, const gchar *tip, void (*function) (GObject *, GObject *), const gchar *cookie)
{
    GtkWidget *b = gtk_button_new_with_mnemonic (label);
    gtk_tooltips_set_tip (tt, b, tip, NULL);
    gtk_box_pack_start (GTK_BOX (hb), b, TRUE, TRUE, 0);
    g_signal_connect ( G_OBJECT (b), "clicked", G_CALLBACK (function), dlg );
    gtk_object_set_data (GTK_OBJECT (dlg), cookie, b);
    gtk_widget_show (b);
}



GSList *
all_text_items (SPObject *r, GSList *l, bool hidden, bool locked)
{
    if (!_desktop)
        return l; // no desktop to check

    if (SP_IS_DEFS(r))
        return l; // we're not interested in items in defs

    if (!strcmp (SP_OBJECT_REPR (r)->name(), "svg:metadata"))
        return l; // we're not interested in metadata

    for (SPObject *child = sp_object_first_child(r); child; child = SP_OBJECT_NEXT (child)) {
        if (SP_IS_ITEM (child) && !SP_OBJECT_IS_CLONED (child) && !_desktop->isLayer(SP_ITEM(child))) {
                if ((hidden || !_desktop->itemIsHidden(SP_ITEM(child))) && (locked || !SP_ITEM(child)->isLocked())) {
                    if (SP_IS_TEXT(child) || SP_IS_FLOWTEXT(child))
                        l = g_slist_prepend (l, child);
                }
        }
        l = all_text_items (child, l, hidden, locked);
    }
    return l;
}

bool
spellcheck_text_is_valid (SPObject *root, SPItem *text)
{
    GSList *l = NULL;
    l = all_text_items (root, l, false, true);
    for (GSList *i = l; i; i = i->next) {
        SPItem *item = (SPItem *) i->data;
        if (item == text) {
            g_slist_free (l);
            return true;
        }
    }
    g_slist_free (l);
    return false;
}

gint compare_text_bboxes (gconstpointer a, gconstpointer b)
{
    SPItem *i1 = SP_ITEM(a);
    SPItem *i2 = SP_ITEM(b);

    Geom::OptRect bbox1 = i1->getBounds(sp_item_i2d_affine(i1));
    Geom::OptRect bbox2 = i2->getBounds(sp_item_i2d_affine(i2));
    if (!bbox1 || !bbox2) {
        return 0;
    }

    // vector between top left corners
    Geom::Point diff = Geom::Point(bbox2->min()[Geom::X], bbox2->max()[Geom::Y]) -
                       Geom::Point(bbox1->min()[Geom::X], bbox1->max()[Geom::Y]);

    // sort top to bottom, left to right, but:
    // if i2 is higher only 0.2 or less times it is righter than i1, put i1 first
    if (diff[Geom::Y] > 0.2 * diff[Geom::X])
        return 1;
    else
        return -1;

    return 0;
}

// we regenerate and resort the list every time, because user could have changed it while the
// dialog was waiting
SPItem *spellcheck_get_text (SPObject *root)
{
    GSList *l = NULL;
    l = all_text_items (root, l, false, true);
    l = g_slist_sort(l, compare_text_bboxes);

    for (GSList *i = l; i; i = i->next) {
        SPItem *item = (SPItem *) i->data;
        if (!g_slist_find (_seen_objects, item)) {
            _seen_objects = g_slist_prepend(_seen_objects, item);
            g_slist_free(l);
            return item;
        }
    }

    g_slist_free(l);
    return NULL;
}

void
spellcheck_sensitive (const gchar *cookie, gboolean gray)
{
   GtkWidget *l = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (dlg), cookie));
   gtk_widget_set_sensitive(l, gray);
}

static void spellcheck_enable_accept(GtkTreeSelection */*selection*/,
                                     void */*??*/)
{
    spellcheck_sensitive ("b_accept", TRUE);
}

static void spellcheck_obj_modified (SPObject *obj, guint /*flags*/, gpointer /*data*/);
static void spellcheck_obj_released (SPObject *obj, gpointer /*data*/);

void
spellcheck_next_text()
{
    spellcheck_disconnect();

    _text = spellcheck_get_text(_root);
    if (_text) {
        _release_connection = new sigc::connection (SP_OBJECT(_text)->connectRelease(
             sigc::bind<1>(sigc::ptr_fun(&spellcheck_obj_released), dlg)));

        _modified_connection = new sigc::connection (SP_OBJECT(_text)->connectModified(
             sigc::bind<2>(sigc::ptr_fun(&spellcheck_obj_modified), dlg)));

        _layout = te_get_layout (_text);
        _begin_w = _layout->begin();
    }
    _end_w = _begin_w;
    _word.clear();
}

bool
spellcheck_init(SPDesktop *desktop)
{
    _desktop = desktop;

    spellcheck_sensitive("suggestions", FALSE);
    spellcheck_sensitive("b_accept", FALSE);
    spellcheck_sensitive("b_ignore", FALSE);
    spellcheck_sensitive("b_ignore_once", FALSE);
    spellcheck_sensitive("b_add", FALSE);
    spellcheck_sensitive("addto_langs", FALSE);
    spellcheck_sensitive("b_start", FALSE);

#ifdef WIN32
    // on windows, dictionaries are in a lib/aspell-0.60 subdir off inkscape's executable dir;
    // this is some black magick to find out the executable path to give it to aspell
    char exeName[MAX_PATH+1];
    GetModuleFileName(NULL, exeName, MAX_PATH);
    char *slashPos = strrchr(exeName, '\\');
    if (slashPos)
        *slashPos = '\0';
    g_print ("%s\n", exeName);
#endif

    _stops = 0;
    _adds = 0;
    spellcheck_clear_rects();

    {
    AspellConfig *config = new_aspell_config();
#ifdef WIN32
    aspell_config_replace(config, "prefix", exeName);
#endif
    aspell_config_replace(config, "lang", _lang);
    aspell_config_replace(config, "encoding", "UTF-8");
    AspellCanHaveError *ret = new_aspell_speller(config);
    delete_aspell_config(config);
    if (aspell_error(ret) != 0) {
        g_warning("Error: %s\n", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
        return false;
    }
    _speller = to_aspell_speller(ret);
    }

    if (_lang2) {
    AspellConfig *config = new_aspell_config();
#ifdef WIN32
    aspell_config_replace(config, "prefix", exeName);
#endif
    aspell_config_replace(config, "lang", _lang2);
    aspell_config_replace(config, "encoding", "UTF-8");
    AspellCanHaveError *ret = new_aspell_speller(config);
    delete_aspell_config(config);
    if (aspell_error(ret) != 0) {
        g_warning("Error: %s\n", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
        return false;
    }
    _speller2 = to_aspell_speller(ret);
    }

    if (_lang3) {
    AspellConfig *config = new_aspell_config();
#ifdef WIN32
    aspell_config_replace(config, "prefix", exeName);
#endif
    aspell_config_replace(config, "lang", _lang3);
    aspell_config_replace(config, "encoding", "UTF-8");
    AspellCanHaveError *ret = new_aspell_speller(config);
    delete_aspell_config(config);
    if (aspell_error(ret) != 0) {
        g_warning("Error: %s\n", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
        return false;
    }
    _speller3 = to_aspell_speller(ret);
    }

    _root = SP_DOCUMENT_ROOT (sp_desktop_document (desktop));

    // empty the list of objects we've checked
    g_slist_free (_seen_objects);
    _seen_objects = NULL;

    // grab first text
    spellcheck_next_text();

    _working = true;

    return true;
}

void
spellcheck_finished ()
{
    aspell_speller_save_all_word_lists(_speller);
    delete_aspell_speller(_speller);
    _speller = NULL;
    if (_speller2) {
        aspell_speller_save_all_word_lists(_speller2);
        delete_aspell_speller(_speller2);
        _speller2 = NULL;
    }
    if (_speller3) {
        aspell_speller_save_all_word_lists(_speller3);
        delete_aspell_speller(_speller3);
        _speller3 = NULL;
    }

    spellcheck_clear_rects();
    spellcheck_disconnect();

    _desktop->clearWaitingCursor();

    spellcheck_sensitive("suggestions", FALSE);
    spellcheck_sensitive("b_accept", FALSE);
    spellcheck_sensitive("b_ignore", FALSE);
    spellcheck_sensitive("b_ignore_once", FALSE);
    spellcheck_sensitive("b_add", FALSE);
    spellcheck_sensitive("addto_langs", FALSE);
    spellcheck_sensitive("b_stop", FALSE);
    spellcheck_sensitive("b_start", TRUE);

    {
        GtkWidget *l = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (dlg), "banner"));
        gchar *label;
        if (_stops)
            label = g_strdup_printf(_("<b>Finished</b>, <b>%d</b> words added to dictionary"), _adds);
        else
            label = g_strdup_printf(_("<b>Finished</b>, nothing suspicious found"));
        gtk_label_set_markup (GTK_LABEL(l), label);
        g_free(label);
    }

    g_slist_free(_seen_objects);
    _seen_objects = NULL;

    _desktop = NULL;
    _root = NULL;

    _working = false;
}

bool
spellcheck_next_word()
{
    if (!_working)
        return false;

    if (!_text) {
        spellcheck_finished();
        return false;
    }
    _word.clear();

    while (_word.size() == 0) {
        _begin_w = _end_w;

        if (!_layout || _begin_w == _layout->end()) {
            spellcheck_next_text();
            return false;
        }

        if (!_layout->isStartOfWord(_begin_w)) {
            _begin_w.nextStartOfWord();
        }

        _end_w = _begin_w;
        _end_w.nextEndOfWord();
        _word = sp_te_get_string_multiline (_text, _begin_w, _end_w);
    }

    // try to link this word with the next if separated by '
    void *rawptr;
    Glib::ustring::iterator text_iter;
    _layout->getSourceOfCharacter(_end_w, &rawptr, &text_iter);
    SPObject *char_item = SP_OBJECT(rawptr);
    if (SP_IS_STRING(char_item)) {
        int this_char = *text_iter;
        if (this_char == '\'' || this_char == 0x2019) {
            Inkscape::Text::Layout::iterator end_t = _end_w;
            end_t.nextCharacter();
            _layout->getSourceOfCharacter(end_t, &rawptr, &text_iter);
            SPObject *char_item = SP_OBJECT(rawptr);
            if (SP_IS_STRING(char_item)) {
                int this_char = *text_iter;
                if (g_ascii_isalpha(this_char)) { // 's
                    _end_w.nextEndOfWord();
                    _word = sp_te_get_string_multiline (_text, _begin_w, _end_w);
                }
            }
        }
    }

    // skip words containing digits
    if (_prefs->getInt(prefs_path + "ignorenumbers") != 0) {
        bool digits = false;
        for (unsigned int i = 0; i < _word.size(); i++) {
            if (g_unichar_isdigit(_word[i])) {
               digits = true;
               break;
            }
        }
        if (digits) {
            return false;
        }
    }

    // skip ALL-CAPS words 
    if (_prefs->getInt(prefs_path + "ignoreallcaps") != 0) {
        bool allcaps = true;
        for (unsigned int i = 0; i < _word.size(); i++) {
            if (!g_unichar_isupper(_word[i])) {
               allcaps = false;
               break;
            }
        }
        if (allcaps) {
            return false;
        }
    }

    // run it by all active spellers
    int have = aspell_speller_check(_speller, _word.c_str(), -1);
    if (_speller2)
        have += aspell_speller_check(_speller2, _word.c_str(), -1);
    if (_speller3)
        have += aspell_speller_check(_speller3, _word.c_str(), -1);

    if (have == 0) { // not found in any!
        _stops ++;

        _desktop->clearWaitingCursor();

        // display it in window
        {
            GtkWidget *l = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (dlg), "banner"));
            Glib::ustring langs = _lang;
            if (_lang2)
                langs = langs + ", " + _lang2;
            if (_lang3)
                langs = langs + ", " + _lang3;
            gchar *label = g_strdup_printf(_("Not in dictionary (%s): <b>%s</b>"), langs.c_str(), _word.c_str());
            gtk_label_set_markup (GTK_LABEL(l), label);
            g_free(label);
        }

        spellcheck_sensitive("suggestions", TRUE);
        spellcheck_sensitive("b_ignore", TRUE);
        spellcheck_sensitive("b_ignore_once", TRUE);
        spellcheck_sensitive("b_add", TRUE);
        spellcheck_sensitive("addto_langs", TRUE);
        spellcheck_sensitive("b_stop", TRUE);

        // draw rect
        std::vector<Geom::Point> points =
            _layout->createSelectionShape(_begin_w, _end_w, sp_item_i2d_affine(_text));
        Geom::Point tl, br;
        tl = br = points.front();
        for (unsigned i = 0 ; i < points.size() ; i ++) {
            if (points[i][Geom::X] < tl[Geom::X])
                tl[Geom::X] = points[i][Geom::X];
            if (points[i][Geom::Y] < tl[Geom::Y])
                tl[Geom::Y] = points[i][Geom::Y];
            if (points[i][Geom::X] > br[Geom::X])
                br[Geom::X] = points[i][Geom::X];
            if (points[i][Geom::Y] > br[Geom::Y])
                br[Geom::Y] = points[i][Geom::Y];
        }

        // expand slightly
        Geom::Rect area = Geom::Rect(tl, br);
        double mindim = fabs(tl[Geom::Y] - br[Geom::Y]);
        if (fabs(tl[Geom::X] - br[Geom::X]) < mindim)
            mindim = fabs(tl[Geom::X] - br[Geom::X]);
        area.expandBy(MAX(0.05 * mindim, 1));

        // create canvas path rectangle, red stroke
        SPCanvasItem *rect = sp_canvas_bpath_new(sp_desktop_sketch(_desktop), NULL);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(rect), 0xff0000ff, 3.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(rect), 0, SP_WIND_RULE_NONZERO);
        SPCurve *curve = new SPCurve();
        curve->moveto(area.corner(0));
        curve->lineto(area.corner(1));
        curve->lineto(area.corner(2));
        curve->lineto(area.corner(3));
        curve->lineto(area.corner(0));
        sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(rect), curve);
        sp_canvas_item_show(rect);
        _rects = g_slist_prepend(_rects, rect);

        // scroll to make it all visible
        Geom::Point const center = _desktop->get_display_area().midpoint();
        area.expandBy(0.5 * mindim);
        Geom::Point scrollto;
        double dist = 0;
        for (unsigned corner = 0; corner < 4; corner ++) {
            if (Geom::L2(area.corner(corner) - center) > dist) {
                dist = Geom::L2(area.corner(corner) - center);
                scrollto = area.corner(corner);
            }
        }
        _desktop->scroll_to_point (scrollto, 1.0);

        // select text; if in Text tool, position cursor to the beginning of word
        // unless it is already in the word
        if (_desktop->selection->singleItem() != _text)
            _desktop->selection->set (_text);
        if (tools_isactive(_desktop, TOOLS_TEXT)) {
            Inkscape::Text::Layout::iterator *cursor =
                sp_text_context_get_cursor_position(SP_TEXT_CONTEXT(_desktop->event_context), _text);
            if (!cursor) // some other text is selected there
                _desktop->selection->set (_text);
            else if (*cursor <= _begin_w || *cursor >= _end_w)
                sp_text_context_place_cursor (SP_TEXT_CONTEXT(_desktop->event_context), _text, _begin_w);
        } 

        // get suggestions
        {
            GtkTreeView *tree_view =
                GTK_TREE_VIEW(gtk_object_get_data (GTK_OBJECT (dlg), "suggestions"));
            GtkListStore *model = gtk_list_store_new (1, G_TYPE_STRING);
            gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

            {
            const AspellWordList *wl = aspell_speller_suggest(_speller, _word.c_str(), -1);
            AspellStringEnumeration * els = aspell_word_list_elements(wl);
            const char *sugg;
            GtkTreeIter iter;
            while ((sugg = aspell_string_enumeration_next(els)) != 0) {
                gtk_list_store_append (GTK_LIST_STORE (model), &iter);
                gtk_list_store_set (GTK_LIST_STORE (model),
                                    &iter,
                                    0, sugg,
                                    -1);
            }
            delete_aspell_string_enumeration(els);
            }

            if (_speller2) {
            const AspellWordList *wl = aspell_speller_suggest(_speller2, _word.c_str(), -1);
            AspellStringEnumeration * els = aspell_word_list_elements(wl);
            const char *sugg;
            GtkTreeIter iter;
            while ((sugg = aspell_string_enumeration_next(els)) != 0) {
                gtk_list_store_append (GTK_LIST_STORE (model), &iter);
                gtk_list_store_set (GTK_LIST_STORE (model),
                                    &iter,
                                    0, sugg,
                                    -1);
            }
            delete_aspell_string_enumeration(els);
            }

            if (_speller3) {
            const AspellWordList *wl = aspell_speller_suggest(_speller3, _word.c_str(), -1);
            AspellStringEnumeration * els = aspell_word_list_elements(wl);
            const char *sugg;
            GtkTreeIter iter;
            while ((sugg = aspell_string_enumeration_next(els)) != 0) {
                gtk_list_store_append (GTK_LIST_STORE (model), &iter);
                gtk_list_store_set (GTK_LIST_STORE (model),
                                    &iter,
                                    0, sugg,
                                    -1);
            }
            delete_aspell_string_enumeration(els);
            }

            spellcheck_sensitive("b_accept", FALSE); // gray it out until something is chosen
        }

        return true;

    }
    return false;
}



void
spellcheck_delete_last_rect ()
{
    if (_rects) {
        sp_canvas_item_hide(SP_CANVAS_ITEM(_rects->data));
        gtk_object_destroy(GTK_OBJECT(_rects->data));
        _rects = _rects->next; // pop latest-prepended rect
    }
}

void
do_spellcheck ()
{
    GtkWidget *l = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (dlg), "banner"));
    gtk_label_set_markup (GTK_LABEL(l), _("<i>Checking...</i>"));
    gtk_widget_queue_draw(GTK_WIDGET(dlg));
    gdk_window_process_updates(GTK_WIDGET(dlg)->window, TRUE);

    _desktop->setWaitingCursor();

    while (_working)
        if (spellcheck_next_word())
            break;
}

static void
spellcheck_obj_modified (SPObject */*obj*/, guint /*flags*/, gpointer /*data*/)
{
    if (_local_change) { // this was a change by this dialog, i.e. an Accept, skip it
        _local_change = false;
        return;
    }

    if (_working && _root) {
        // user may have edited the text we're checking; try to do the most sensible thing in this
        // situation

        // just in case, re-get text's layout
        _layout = te_get_layout (_text);

        // re-get the word
        _layout->validateIterator(&_begin_w);
        _end_w = _begin_w;
        _end_w.nextEndOfWord();
        Glib::ustring word_new = sp_te_get_string_multiline (_text, _begin_w, _end_w);
        if (word_new != _word) {
            _end_w = _begin_w;
            spellcheck_delete_last_rect ();
            do_spellcheck (); // recheck this word and go ahead if it's ok
        }
    }
}

static void
spellcheck_obj_released (SPObject */*obj*/, gpointer /*data*/)
{
    if (_working && _root) {
        // the text object was deleted
        spellcheck_delete_last_rect ();
        spellcheck_next_text();
        do_spellcheck (); // get next text and continue
    }
}

void
sp_spellcheck_accept (GObject *, GObject *dlg)
{
    // insert chosen suggestion
    GtkTreeView *tv =
        GTK_TREE_VIEW(gtk_object_get_data (GTK_OBJECT (dlg), "suggestions"));
    GtkTreeSelection *ts = gtk_tree_view_get_selection(tv);
    GtkTreeModel *model = 0;
    GtkTreeIter   iter;
    if (gtk_tree_selection_get_selected(ts, &model, &iter)) {
        gchar *sugg;
        gtk_tree_model_get (model, &iter, 0, &sugg, -1);
        if (sugg) {
            //g_print("chosen: %s\n", sugg);
            _local_change = true;
            sp_te_replace(_text, _begin_w, _end_w, sugg);
            // find the end of the word anew
            _end_w = _begin_w;
            _end_w.nextEndOfWord();
            sp_document_done (sp_desktop_document(_desktop), SP_VERB_CONTEXT_TEXT,
                              _("Fix spelling"));
        }
    }

    spellcheck_delete_last_rect ();

    do_spellcheck(); // next word or end
}

void
sp_spellcheck_ignore (GObject */*obj*/, GObject */*dlg*/)
{
    aspell_speller_add_to_session(_speller, _word.c_str(), -1);
    if (_speller2)
        aspell_speller_add_to_session(_speller2, _word.c_str(), -1);
    if (_speller3)
        aspell_speller_add_to_session(_speller3, _word.c_str(), -1);
    spellcheck_delete_last_rect ();

    do_spellcheck(); // next word or end
}

void
sp_spellcheck_ignore_once (GObject */*obj*/, GObject */*dlg*/)
{
    spellcheck_delete_last_rect ();

    do_spellcheck(); // next word or end
}

void
sp_spellcheck_add (GObject */*obj*/, GObject */*dlg*/)
{
    _adds++;
    GtkMenu *m =
        GTK_MENU(gtk_object_get_data (GTK_OBJECT (dlg), "addto_langs"));
    GtkWidget *mi = gtk_menu_get_active (m);
    unsigned int num = GPOINTER_TO_UINT(gtk_object_get_data (GTK_OBJECT (mi), "number"));
    switch (num) {
        case 1:
            aspell_speller_add_to_personal(_speller, _word.c_str(), -1);
            break;
        case 2:
            if (_speller2)
                aspell_speller_add_to_personal(_speller2, _word.c_str(), -1);
            break;
        case 3:
            if (_speller3)
                aspell_speller_add_to_personal(_speller3, _word.c_str(), -1);
            break;
        default:
            break;
    }

    spellcheck_delete_last_rect ();

    do_spellcheck(); // next word or end
}

void
sp_spellcheck_stop (GObject */*obj*/, GObject */*dlg*/)
{
    spellcheck_finished();
}

void
sp_spellcheck_start (GObject *, GObject *)
{
    if (spellcheck_init (SP_ACTIVE_DESKTOP))
        do_spellcheck(); // next word or end
}

static gboolean spellcheck_desktop_deactivated(Inkscape::Application */*application*/, SPDesktop *desktop, void */*data*/)
{
    if (_working) {
        if (_desktop == desktop) {
            spellcheck_finished();
        }
    }
    return FALSE;
}


void
sp_spellcheck_dialog (void)
{
    _prefs = Inkscape::Preferences::get();

    // take languages from prefs
    _lang = _lang2 = _lang3 = NULL;
    Glib::ustring lang = _prefs->getString(prefs_path + "lang");
    if (lang != "")
        _lang = g_strdup(lang.c_str());
    else
        _lang = g_strdup("en");
    lang = _prefs->getString(prefs_path + "lang2");
    if (lang != "")
        _lang2 = g_strdup(lang.c_str());
    lang = _prefs->getString(prefs_path + "lang3");
    if (lang != "")
        _lang3 = g_strdup(lang.c_str());

    if  (!dlg)
    {
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_SPELLCHECK), title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = _prefs->getInt(prefs_path + "x", -1000);
            y = _prefs->getInt(prefs_path + "y", -1000);
        }
        if (w ==0 || h == 0) {
            w = _prefs->getInt(prefs_path + "w", 0);
            h = _prefs->getInt(prefs_path + "h", 0);
        }

        if (w && h)
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE))) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }

        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd );

        g_signal_connect( G_OBJECT(INKSCAPE), "deactivate_desktop", G_CALLBACK( spellcheck_desktop_deactivated ), NULL);


        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_spellcheck_dialog_destroy), NULL );
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_spellcheck_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_spellcheck_dialog_delete), dlg);

        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

        GtkTooltips *tt = gtk_tooltips_new ();

        gtk_container_set_border_width (GTK_CONTAINER (dlg), 4);

        /* Toplevel vbox */
        GtkWidget *vb = gtk_vbox_new (FALSE, 4);
        gtk_container_add (GTK_CONTAINER (dlg), vb);

        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            GtkWidget *l = gtk_label_new (NULL);
            gtk_object_set_data (GTK_OBJECT (dlg), "banner", l);
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
        }

        {
            GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
            gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                            GTK_POLICY_AUTOMATIC,
                                            GTK_POLICY_AUTOMATIC);

            GtkListStore *model = gtk_list_store_new (1, G_TYPE_STRING);
            GtkWidget *tree_view = gtk_tree_view_new ();
            gtk_object_set_data (GTK_OBJECT (dlg), "suggestions", tree_view);
            gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
                                                   tree_view);
            gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
            GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(tree_view));
            g_signal_connect (G_OBJECT(selection), "changed",
                              G_CALLBACK (spellcheck_enable_accept), NULL);
            gtk_widget_show (tree_view);
            GtkCellRenderer *cell = gtk_cell_renderer_text_new ();
            GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (_("Suggestions:"),
                                                                                  cell,
                                                                                  "text", 0,
                                                                                  NULL);
            gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                                         GTK_TREE_VIEW_COLUMN (column));
            gtk_box_pack_start (GTK_BOX (vb), scrolled_window, TRUE, TRUE, 0);
        }


        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            sp_spellcheck_new_button (dlg, hb, _("_Accept"), tt, _("Accept the chosen suggestion"),
                                      sp_spellcheck_accept, "b_accept");
            sp_spellcheck_new_button (dlg, hb, _("_Ignore once"), tt, _("Ignore this word only once"),
                                      sp_spellcheck_ignore_once, "b_ignore_once");
            sp_spellcheck_new_button (dlg, hb, _("_Ignore"), tt, _("Ignore this word in this session"),
                                      sp_spellcheck_ignore, "b_ignore");
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
        }

        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            sp_spellcheck_new_button (dlg, hb, _("A_dd to dictionary:"), tt, _("Add this word to the chosen dictionary"),
                                      sp_spellcheck_add, "b_add");
            GtkWidget *cbox = gtk_menu_new ();
            {
                GtkWidget *mi = gtk_menu_item_new_with_label(_lang);
                g_object_set_data (G_OBJECT (mi), "number", GUINT_TO_POINTER (1));
                gtk_menu_append (GTK_MENU (cbox), mi);
            }
            if (_lang2) {
                GtkWidget *mi = gtk_menu_item_new_with_label(_lang2);
                g_object_set_data (G_OBJECT (mi), "number", GUINT_TO_POINTER (2));
                gtk_menu_append (GTK_MENU (cbox), mi);
            }
            if (_lang3) {
                GtkWidget *mi = gtk_menu_item_new_with_label(_lang3);
                g_object_set_data (G_OBJECT (mi), "number", GUINT_TO_POINTER (3));
                gtk_menu_append (GTK_MENU (cbox), mi);
            }
            gtk_widget_show_all (cbox);
            GtkWidget *mnu = gtk_option_menu_new();
            gtk_option_menu_set_menu(GTK_OPTION_MENU(mnu), cbox);
            g_object_set_data (G_OBJECT (dlg), "addto_langs", cbox);
            gtk_box_pack_start (GTK_BOX (hb), mnu, TRUE, TRUE, 0);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
        }

        {
            GtkWidget *hs = gtk_hseparator_new ();
            gtk_box_pack_start (GTK_BOX (vb), hs, FALSE, FALSE, 0);
        }

        {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            sp_spellcheck_new_button (dlg, hb, _("_Stop"), tt, _("Stop the check"),
                                      sp_spellcheck_stop, "b_stop");
            sp_spellcheck_new_button (dlg, hb, _("_Start"), tt, _("Start the check"),
                                      sp_spellcheck_start, "b_start");
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
        }

        gtk_widget_show_all (vb);
    }

    gtk_window_present ((GtkWindow *) dlg);

    // run it at once
    sp_spellcheck_start (NULL, NULL);
}

#else 

void sp_spellcheck_dialog (void) {}

#endif // HAVE_ASPELL


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

/**
 * @file
 * Spellcheck dialog.
 */
/* Authors:
 *   bulia byak <bulia@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "spellcheck.h"
#include "widgets/icon.h"
#include "message-stack.h"

#include "helper/window.h"
#include "macros.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"

#include "ui/tools-switch.h"
#include "ui/tools/text-tool.h"
#include "ui/interface.h"
#include "preferences.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "sp-defs.h"
#include "selection-chemistry.h"
#include <xml/repr.h>
#include "display/canvas-bpath.h"
#include "display/curve.h"
#include "document-undo.h"
#include "sp-root.h"
#include "verbs.h"
#include <glibmm/i18n.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


namespace Inkscape {
namespace UI {
namespace Dialog {


SpellCheck::SpellCheck (void) :
    UI::Widget::Panel ("", "/dialogs/spellcheck/", SP_VERB_DIALOG_SPELLCHECK),
    _rects(NULL),
    _seen_objects(NULL),
    _text(NULL),
    _layout(NULL),
    _stops(0),
    _adds(0),
    _working(false),
    _local_change(false),
    _prefs(NULL),
    _lang("en"),
    _lang2(""),
    _lang3(""),
    accept_button(_("_Accept"), true),
    ignoreonce_button(_("_Ignore once"), true),
    ignore_button(_("_Ignore"), true),
    add_button(_("A_dd"), true),
    dictionary_hbox(false, 0),
    stop_button(_("_Stop"), true),
    start_button(_("_Start"), true),
    desktop(NULL),
    deskTrack()
{

#ifdef HAVE_ASPELL
    _speller = NULL;
    _speller2 = NULL;
    _speller3 = NULL;
#endif /* HAVE_ASPELL */

    _prefs = Inkscape::Preferences::get();

    // take languages from prefs
    _lang  = _prefs->getString(_prefs_path + "lang");
    _lang2 = _prefs->getString(_prefs_path + "lang2");
    _lang3 = _prefs->getString(_prefs_path + "lang3");
    if (_lang == "")
        _lang = "en";

    banner_hbox.set_layout(Gtk::BUTTONBOX_START);
    banner_hbox.add(banner_label);

    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window.set_shadow_type(Gtk::SHADOW_IN);
    scrolled_window.set_size_request(120, 96);
    scrolled_window.add(tree_view);

    model = Gtk::ListStore::create(tree_columns);
    tree_view.set_model(model);
    tree_view.append_column(_("Suggestions:"), tree_columns.suggestions);

    {
        dictionary_combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dictionary_combo),  _lang.c_str());
        if (_lang2 != "") {
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dictionary_combo), _lang2.c_str());
        }
        if (_lang3 != "") {
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dictionary_combo), _lang3.c_str());
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX (dictionary_combo), 0);
        gtk_widget_show_all (dictionary_combo);
    }

    accept_button.set_tooltip_text(_("Accept the chosen suggestion"));
    ignoreonce_button.set_tooltip_text(_("Ignore this word only once"));
    ignore_button.set_tooltip_text(_("Ignore this word in this session"));
    add_button.set_tooltip_text(_("Add this word to the chosen dictionary"));

    dictionary_hbox.pack_start(add_button, true, true, 0);
    dictionary_hbox.pack_start(*Gtk::manage(Glib::wrap(dictionary_combo)), false, false, 0);

    changebutton_vbox.set_spacing(4);
    changebutton_vbox.pack_start(accept_button, false, false, 0);
    changebutton_vbox.pack_start(ignoreonce_button, false, false, 0);
    changebutton_vbox.pack_start(ignore_button, false, false, 0);
    changebutton_vbox.pack_start(dictionary_hbox, false, false, 0);

    suggestion_hbox.pack_start (scrolled_window, true, true, 4);
    suggestion_hbox.pack_end (changebutton_vbox, false, false, 0);

    stop_button.set_tooltip_text(_("Stop the check"));
    start_button.set_tooltip_text(_("Start the check"));

    actionbutton_hbox.set_layout(Gtk::BUTTONBOX_END);
    actionbutton_hbox.set_spacing(4);
    actionbutton_hbox.add(stop_button);
    actionbutton_hbox.add(start_button);

    /*
     * Main dialog
     */
    Gtk::Box *contents = _getContents();
    contents->set_spacing(6);
    contents->pack_start (banner_hbox, false, false, 0);
    contents->pack_start (suggestion_hbox, true, true, 0);
    contents->pack_start (action_sep, false, false, 6);
    contents->pack_start (actionbutton_hbox, false, false, 0);

    /*
     * Signal handlers
     */
    accept_button.signal_clicked().connect(sigc::mem_fun(*this, &SpellCheck::onAccept));
    ignoreonce_button.signal_clicked().connect(sigc::mem_fun(*this, &SpellCheck::onIgnoreOnce));
    ignore_button.signal_clicked().connect(sigc::mem_fun(*this, &SpellCheck::onIgnore));
    add_button.signal_clicked().connect(sigc::mem_fun(*this, &SpellCheck::onAdd));
    start_button.signal_clicked().connect(sigc::mem_fun(*this, &SpellCheck::onStart));
    stop_button.signal_clicked().connect(sigc::mem_fun(*this, &SpellCheck::onStop));
    tree_view.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &SpellCheck::onTreeSelectionChange));
    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &SpellCheck::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children ();

    // run it at once
    onStart ();
}

SpellCheck::~SpellCheck(void)
{
    clearRects();
    disconnect();

    desktopChangeConn.disconnect();
    deskTrack.disconnect();
}

void SpellCheck::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void SpellCheck::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        this->desktop = desktop;
        if (_working) {
            // Stop and start on the new desktop
            finished();
            onStart();
        }
    }
}

void SpellCheck::clearRects()
{
    for (GSList *it = _rects; it; it = it->next) {
        sp_canvas_item_hide(SP_CANVAS_ITEM(it->data));
        sp_canvas_item_destroy(SP_CANVAS_ITEM(it->data));
    }
    g_slist_free(_rects);
    _rects = NULL;
}

void SpellCheck::disconnect()
{
    if (_release_connection) {
        _release_connection.disconnect();
    }
    if (_modified_connection) {
        _modified_connection.disconnect();
    }
}

GSList *SpellCheck::allTextItems (SPObject *r, GSList *l, bool hidden, bool locked)
{
    if (!desktop)
        return l; // no desktop to check

    if (SP_IS_DEFS(r))
        return l; // we're not interested in items in defs

    if (!strcmp(r->getRepr()->name(), "svg:metadata")) {
        return l; // we're not interested in metadata
    }

    for (SPObject *child = r->firstChild(); child; child = child->next) {
        if (SP_IS_ITEM (child) && !child->cloned && !desktop->isLayer(SP_ITEM(child))) {
                if ((hidden || !desktop->itemIsHidden(SP_ITEM(child))) && (locked || !SP_ITEM(child)->isLocked())) {
                    if (SP_IS_TEXT(child) || SP_IS_FLOWTEXT(child))
                        l = g_slist_prepend (l, child);
                }
        }
        l = allTextItems (child, l, hidden, locked);
    }
    return l;
}

bool
SpellCheck::textIsValid (SPObject *root, SPItem *text)
{
    GSList *l = NULL;
    l = allTextItems (root, l, false, true);
    for (GSList *i = l; i; i = i->next) {
        SPItem *item = static_cast<SPItem *>(i->data);
        if (item == text) {
            g_slist_free (l);
            return true;
        }
    }
    g_slist_free (l);
    return false;
}

gint SpellCheck::compareTextBboxes (gconstpointer a, gconstpointer b)
{
    SPItem *i1 = SP_ITEM(a);
    SPItem *i2 = SP_ITEM(b);

    Geom::OptRect bbox1 = i1->desktopVisualBounds();
    Geom::OptRect bbox2 = i2->desktopVisualBounds();
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

// We regenerate and resort the list every time, because user could have changed it while the
// dialog was waiting
SPItem *SpellCheck::getText (SPObject *root)
{
    GSList *l = NULL;
    l = allTextItems (root, l, false, true);
    l = g_slist_sort(l, (GCompareFunc)SpellCheck::compareTextBboxes);

    for (GSList *i = l; i; i = i->next) {
        SPItem *item = static_cast<SPItem *>(i->data);
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
SpellCheck::nextText()
{
    disconnect();

    _text = getText(_root);
    if (_text) {

        _modified_connection = (SP_OBJECT(_text))->connectModified(sigc::mem_fun(*this, &SpellCheck::onObjModified));
        _release_connection = (SP_OBJECT(_text))->connectRelease(sigc::mem_fun(*this, &SpellCheck::onObjReleased));

        _layout = te_get_layout (_text);
        _begin_w = _layout->begin();
    }
    _end_w = _begin_w;
    _word.clear();
}

bool
SpellCheck::init(SPDesktop *d)
{
    desktop = d;

    tree_view.set_sensitive(false);
    accept_button.set_sensitive(false);
    ignore_button.set_sensitive(false);
    ignoreonce_button.set_sensitive(false);
    add_button.set_sensitive(false);
    gtk_widget_set_sensitive(dictionary_combo, false);
    start_button.set_sensitive(false);

#ifdef WIN32
    // on windows, dictionaries are in a lib/aspell-0.60 subdir off inkscape's executable dir;
    // this is some black magick to find out the executable path to give it to aspell
    char exeName[MAX_PATH+1];
    GetModuleFileName(NULL, exeName, MAX_PATH);
    char *slashPos = strrchr(exeName, '\\');
    if (slashPos)
        *slashPos = '\0';
    //g_print ("Aspell prefix path: %s\n", exeName);
#endif

    _stops = 0;
    _adds = 0;
    clearRects();

#ifdef HAVE_ASPELL
    {
        AspellConfig *config = new_aspell_config();
#ifdef WIN32
        aspell_config_replace(config, "prefix", exeName);
#endif
        aspell_config_replace(config, "lang", _lang.c_str());
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

    if (_lang2 != "") {
        AspellConfig *config = new_aspell_config();
#ifdef WIN32
        aspell_config_replace(config, "prefix", exeName);
#endif
        aspell_config_replace(config, "lang", _lang2.c_str());
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

    if (_lang3 != "") {
        AspellConfig *config = new_aspell_config();
#ifdef WIN32
        aspell_config_replace(config, "prefix", exeName);
#endif
        aspell_config_replace(config, "lang", _lang3.c_str());
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
#endif  /* HAVE_ASPELL */

    _root = desktop->getDocument()->getRoot();

    // empty the list of objects we've checked
    g_slist_free (_seen_objects);
    _seen_objects = NULL;

    // grab first text
    nextText();

    _working = true;

    return true;
}

void
SpellCheck::finished ()
{
#ifdef HAVE_ASPELL
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
#endif  /* HAVE_ASPELL */

    clearRects();
    disconnect();

    //desktop->clearWaitingCursor();

    tree_view.set_sensitive(false);
    accept_button.set_sensitive(false);
    ignore_button.set_sensitive(false);
    ignoreonce_button.set_sensitive(false);
    gtk_widget_set_sensitive(dictionary_combo, false);
    add_button.set_sensitive(false);
    stop_button.set_sensitive(false);
    start_button.set_sensitive(true);

    {
        gchar *label;
        if (_stops)
            label = g_strdup_printf(_("<b>Finished</b>, <b>%d</b> words added to dictionary"), _adds);
        else
            label = g_strdup_printf("%s", _("<b>Finished</b>, nothing suspicious found"));
        banner_label.set_markup(label);
        g_free(label);
    }

    g_slist_free(_seen_objects);
    _seen_objects = NULL;

    desktop = NULL;
    _root = NULL;

    _working = false;
}

bool
SpellCheck::nextWord()
{
    if (!_working)
        return false;

    if (!_text) {
        finished();
        return false;
    }
    _word.clear();

    while (_word.size() == 0) {
        _begin_w = _end_w;

        if (!_layout || _begin_w == _layout->end()) {
            nextText();
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
    if (_prefs->getInt(_prefs_path + "ignorenumbers") != 0) {
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
    if (_prefs->getInt(_prefs_path + "ignoreallcaps") != 0) {
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

    int have = 0;

#ifdef HAVE_ASPELL
    // run it by all active spellers
    have = aspell_speller_check(_speller, _word.c_str(), -1);
    if (_speller2)
        have += aspell_speller_check(_speller2, _word.c_str(), -1);
    if (_speller3)
        have += aspell_speller_check(_speller3, _word.c_str(), -1);
#endif  /* HAVE_ASPELL */

    if (have == 0) { // not found in any!
        _stops ++;

        //desktop->clearWaitingCursor();

        // display it in window
        {
            Glib::ustring langs = _lang;
            if (_lang2 != "")
                langs = langs + ", " + _lang2;
            if (_lang3 != "")
                langs = langs + ", " + _lang3;
            gchar *label = g_strdup_printf(_("Not in dictionary (%s): <b>%s</b>"), langs.c_str(), _word.c_str());
            banner_label.set_markup(label);
            g_free(label);
        }

        tree_view.set_sensitive(true);
        ignore_button.set_sensitive(true);
        ignoreonce_button.set_sensitive(true);
        gtk_widget_set_sensitive(dictionary_combo, true);
        add_button.set_sensitive(true);
        stop_button.set_sensitive(true);

        // draw rect
        std::vector<Geom::Point> points =
            _layout->createSelectionShape(_begin_w, _end_w, _text->i2dt_affine());
        if (points.size() >= 4) { // we may not have a single quad if this is a clipped part of text on path; in that case skip drawing the rect
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
            SPCanvasItem *rect = sp_canvas_bpath_new(desktop->getSketch(), NULL);
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
            Geom::Point const center = desktop->get_display_area().midpoint();
            area.expandBy(0.5 * mindim);
            Geom::Point scrollto;
            double dist = 0;
            for (unsigned corner = 0; corner < 4; corner ++) {
                if (Geom::L2(area.corner(corner) - center) > dist) {
                    dist = Geom::L2(area.corner(corner) - center);
                    scrollto = area.corner(corner);
                }
            }
            desktop->scroll_to_point (scrollto, 1.0);
        }

        // select text; if in Text tool, position cursor to the beginning of word
        // unless it is already in the word
        if (desktop->selection->singleItem() != _text)
            desktop->selection->set (_text);
        if (tools_isactive(desktop, TOOLS_TEXT)) {
            Inkscape::Text::Layout::iterator *cursor =
                sp_text_context_get_cursor_position(SP_TEXT_CONTEXT(desktop->event_context), _text);
            if (!cursor) // some other text is selected there
                desktop->selection->set (_text);
            else if (*cursor <= _begin_w || *cursor >= _end_w)
                sp_text_context_place_cursor (SP_TEXT_CONTEXT(desktop->event_context), _text, _begin_w);
        } 

#ifdef HAVE_ASPELL

        // get suggestions
        {
            model = Gtk::ListStore::create(tree_columns);
            tree_view.set_model(model);

            {
            const AspellWordList *wl = aspell_speller_suggest(_speller, _word.c_str(), -1);
            AspellStringEnumeration * els = aspell_word_list_elements(wl);
            const char *sugg;
            Gtk::TreeModel::iterator iter;

            while ((sugg = aspell_string_enumeration_next(els)) != 0) {
                iter = model->append();
                Gtk::TreeModel::Row row = *iter;
                row[tree_columns.suggestions] = sugg;
            }
            delete_aspell_string_enumeration(els);
            }

            if (_speller2) {
            const AspellWordList *wl = aspell_speller_suggest(_speller2, _word.c_str(), -1);
            AspellStringEnumeration * els = aspell_word_list_elements(wl);
            const char *sugg;
            Gtk::TreeModel::iterator iter;
            while ((sugg = aspell_string_enumeration_next(els)) != 0) {
                iter = model->append();
                Gtk::TreeModel::Row row = *iter;
                row[tree_columns.suggestions] = sugg;
            }
            delete_aspell_string_enumeration(els);
            }

            if (_speller3) {
            const AspellWordList *wl = aspell_speller_suggest(_speller3, _word.c_str(), -1);
            AspellStringEnumeration * els = aspell_word_list_elements(wl);
            const char *sugg;
            Gtk::TreeModel::iterator iter;
            while ((sugg = aspell_string_enumeration_next(els)) != 0) {
                iter = model->append();
                Gtk::TreeModel::Row row = *iter;
                row[tree_columns.suggestions] = sugg;
            }
            delete_aspell_string_enumeration(els);
            }

            accept_button.set_sensitive(false);  // gray it out until something is chosen
        }

#endif  /* HAVE_ASPELL */

        return true;

    }
    return false;
}



void
SpellCheck::deleteLastRect ()
{
    if (_rects) {
        sp_canvas_item_hide(SP_CANVAS_ITEM(_rects->data));
        sp_canvas_item_destroy(SP_CANVAS_ITEM(_rects->data));
        _rects = _rects->next; // pop latest-prepended rect
    }
}

void SpellCheck::doSpellcheck ()
{
    banner_label.set_markup(_("<i>Checking...</i>"));

    //desktop->setWaitingCursor();

    while (_working)
        if (nextWord())
            break;
}

void SpellCheck::onTreeSelectionChange()
{
    accept_button.set_sensitive(true);
}

void SpellCheck::onObjModified (SPObject* /* blah */, unsigned int /* bleh */)
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
            deleteLastRect ();
            doSpellcheck (); // recheck this word and go ahead if it's ok
        }
    }
}

void SpellCheck::onObjReleased (SPObject* /* blah */)
{
    if (_working && _root) {
        // the text object was deleted
        deleteLastRect ();
        nextText();
        doSpellcheck (); // get next text and continue
    }
}

void SpellCheck::onAccept ()
{
    // insert chosen suggestion

    Glib::RefPtr<Gtk::TreeSelection> selection = tree_view.get_selection();
    Gtk::TreeModel::iterator iter = selection->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring sugg = row[tree_columns.suggestions];

        if (sugg.length() > 0) {
            //g_print("chosen: %s\n", sugg);
            _local_change = true;
            sp_te_replace(_text, _begin_w, _end_w, sugg.c_str());
            // find the end of the word anew
            _end_w = _begin_w;
            _end_w.nextEndOfWord();
            DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_TEXT,
                               _("Fix spelling"));
        }
    }

    deleteLastRect();
    doSpellcheck();
}

void
SpellCheck::onIgnore ()
{
#ifdef HAVE_ASPELL
    aspell_speller_add_to_session(_speller, _word.c_str(), -1);
    if (_speller2)
        aspell_speller_add_to_session(_speller2, _word.c_str(), -1);
    if (_speller3)
        aspell_speller_add_to_session(_speller3, _word.c_str(), -1);
#endif  /* HAVE_ASPELL */

    deleteLastRect();
    doSpellcheck();
}

void
SpellCheck::onIgnoreOnce ()
{
    deleteLastRect();
    doSpellcheck();
}

void
SpellCheck::onAdd ()
{
    _adds++;

#ifdef HAVE_ASPELL
    gint num = gtk_combo_box_get_active((GtkComboBox *)dictionary_combo);
    switch (num) {
        case 0:
            aspell_speller_add_to_personal(_speller, _word.c_str(), -1);
            break;
        case 1:
            if (_speller2)
                aspell_speller_add_to_personal(_speller2, _word.c_str(), -1);
            break;
        case 2:
            if (_speller3)
                aspell_speller_add_to_personal(_speller3, _word.c_str(), -1);
            break;
        default:
            break;
    }
#endif  /* HAVE_ASPELL */

    deleteLastRect();
    doSpellcheck();
}

void
SpellCheck::onStop ()
{
    finished();
}

void
SpellCheck::onStart ()
{
    if (init (SP_ACTIVE_DESKTOP))
        doSpellcheck();
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

#ifndef __INKSCAPE_H__
#define __INKSCAPE_H__

/*
 * Interface to main application
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Liam P. White <inkscapebrony@gmail.com>
 *
 * Copyright (C) 1999-2014 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <vector>
#include <glib.h>
#include <glib-object.h>
#include <sigc++/signal.h>
#include "layer-model.h"
#include "selection.h"

class SPDesktop;
class SPDocument;
struct SPColor;

namespace Inkscape {

class Application;
namespace UI {
namespace Tools {

class ToolBase;

} // namespace Tools
} // namespace UI

class ActionContext;

namespace XML {
class Node;
struct Document;
} // namespace XML

} // namespace Inkscape

void inkscape_ref  (Inkscape::Application & in);
void inkscape_unref(Inkscape::Application & in);

#define INKSCAPE (Inkscape::Application::instance())
#define SP_ACTIVE_EVENTCONTEXT (INKSCAPE.active_event_context())
#define SP_ACTIVE_DOCUMENT (INKSCAPE.active_document())
#define SP_ACTIVE_DESKTOP (INKSCAPE.active_desktop())

class AppSelectionModel {
    Inkscape::LayerModel _layer_model;
    Inkscape::Selection *_selection;

public:
    AppSelectionModel(SPDocument *doc) {
        _layer_model.setDocument(doc);
        // TODO: is this really how we should manage the lifetime of the selection?
        // I just copied this from the initialization of the Selection in SPDesktop.
        // When and how is it actually released?
        _selection = Inkscape::GC::release(new Inkscape::Selection(&_layer_model, NULL));
    }

    Inkscape::Selection *getSelection() const { return _selection; }
};

namespace Inkscape {

class Application {
public:
    static Application& instance();
    static bool exists();
    static void create(const char* argv0, bool use_gui);
    
    // returns the mask of the keyboard modifier to map to Alt, zero if no mapping
    // Needs to be a guint because gdktypes.h does not define a 'no-modifier' value
    guint mapalt() const { return _mapalt; }
    
    // Sets the keyboard modifer to map to Alt. Zero switches off mapping, as does '1', which is the default 
    void mapalt(guint maskvalue);
    
    guint trackalt() const { return _trackalt; }
    void trackalt(guint trackvalue) { _trackalt = trackvalue; }

    bool use_gui() const { return _use_gui; }
    void use_gui(gboolean guival) { _use_gui = guival; }

    char const* argv0() const { return _argv0; }
    void argv0(char const *);

    // no setter for this -- only we can control this variable
    static bool isCrashing() { return _crashIsHappening; }

    // useful functions
    void autosave_init();
    void application_init (const gchar *argv0, gboolean use_gui);
    void load_config (const gchar *filename, Inkscape::XML::Document *config, const gchar *skeleton, 
                      unsigned int skel_size, const gchar *e_notreg, const gchar *e_notxml, 
                      const gchar *e_notsp, const gchar *warn);

    bool load_menus();
    bool save_menus();
    Inkscape::XML::Node * get_menus();
    
    Inkscape::UI::Tools::ToolBase * active_event_context();
    SPDocument * active_document();
    SPDesktop * active_desktop();
    
    // Use this function to get selection model etc for a document
    Inkscape::ActionContext action_context_for_document(SPDocument *doc);
    Inkscape::ActionContext active_action_context();
    
    bool sole_desktop_for_document(SPDesktop const &desktop);
    
    // Inkscape desktop stuff
    void add_desktop(SPDesktop * desktop);
    void remove_desktop(SPDesktop* desktop);
    void activate_desktop (SPDesktop * desktop);
    void switch_desktops_next ();
    void switch_desktops_prev ();
    void get_all_desktops (std::list< SPDesktop* >& listbuf);
    void reactivate_desktop (SPDesktop * desktop);
    SPDesktop * find_desktop_by_dkey (unsigned int dkey);
    unsigned int maximum_dkey();
    SPDesktop * next_desktop ();
    SPDesktop * prev_desktop ();
    
    void dialogs_hide ();
    void dialogs_unhide ();
    void dialogs_toggle ();
    
    void external_change ();
    void selection_modified (Inkscape::Selection *selection, guint flags);
    void selection_changed (Inkscape::Selection * selection);
    void subselection_changed (SPDesktop *desktop);
    void selection_set (Inkscape::Selection * selection);
    
    void eventcontext_set (Inkscape::UI::Tools::ToolBase * eventcontext);
    
    // Moved document add/remove functions into public inkscape.h as they are used
    // (rightly or wrongly) by console-mode functions
    void add_document (SPDocument *document);
    bool remove_document (SPDocument *document);
    
    static char *homedir_path(const char *filename);
    static char *profile_path(const char *filename);
    
    // fixme: This has to be rethought
    void refresh_display ();
    
    // fixme: This also
    void exit ();
    
    static void crash_handler(int signum);

    int autosave();

    // nobody should be accessing our reference count, so it's made private.
    friend void ::inkscape_ref  (Application & in);
    friend void ::inkscape_unref(Application & in);

    // signals
    
    // one of selections changed
    sigc::signal<void, Inkscape::Selection *> signal_selection_changed;
    // one of subselections (text selection, gradient handle, etc) changed
    sigc::signal<void, SPDesktop *> signal_subselection_changed;
    // one of selections modified
    sigc::signal<void, Inkscape::Selection *, guint /*flags*/> signal_selection_modified;
    // one of selections set
    sigc::signal<void, Inkscape::Selection *> signal_selection_set;
    // tool switched
    sigc::signal<void, Inkscape::UI::Tools::ToolBase * /*eventcontext*/> signal_eventcontext_set;
    // some desktop got focus
    sigc::signal<void, SPDesktop *> signal_activate_desktop;
    // some desktop lost focus
    sigc::signal<void, SPDesktop *> signal_deactivate_desktop;
    
    // these are orphaned signals (nothing emits them and nothing connects to them)
    sigc::signal<void, SPDocument *> signal_destroy_document;
    sigc::signal<void, SPColor *, double /*opacity*/> signal_color_set;
    
    // inkscape is quitting
    sigc::signal<void> signal_shut_down;
    // user pressed F12
    sigc::signal<void> signal_dialogs_hide;
    // user pressed F12
    sigc::signal<void> signal_dialogs_unhide;
    // a document was changed by some external means (undo or XML editor); this
    // may not be reflected by a selection change and thus needs a separate signal
    sigc::signal<void> signal_external_change;

private:
    static Inkscape::Application * _S_inst;

    Application(const char* argv0, bool use_gui);
    ~Application();

    Application(Application const&); // no copy
    Application& operator=(Application const&); // no assign
    Application* operator&() const; // no pointer access

    Inkscape::XML::Document * _menus;
    std::map<SPDocument *, int> _document_set;
    std::map<SPDocument *, AppSelectionModel *> _selection_models;
    std::vector<SPDesktop *> * _desktops;

    unsigned refCount;
    bool _dialogs_toggle;
    guint _mapalt;
    guint _trackalt;
    char * _argv0;
    static bool _crashIsHappening;
    bool _use_gui;
};

} // namespace Inkscape

#endif

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

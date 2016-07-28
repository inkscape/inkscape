/**
 * @file
 * Interface to main application.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Liam P. White <inkscapebrony@gmail.com>
 *
 * Copyright (C) 1999-2014 authors
 * c++ port Copyright (C) 2003 Nathan Hurst 
 * c++ification Copyright (C) 2014 Liam P. White
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>

#include <map>

#include <gtkmm/messagedialog.h>
#include "debug/simple-event.h"
#include "debug/event-tracker.h"

#ifndef WIN32
# define HAS_PROC_SELF_EXE  //to get path of executable
#else

#if !defined(_WIN32_IE) || (_WIN32_IE < 0x0400)
# undef _WIN32_IE 
# define _WIN32_IE 0x0400
#endif
//#define HAS_SHGetSpecialFolderPath
#define HAS_SHGetSpecialFolderLocation
#define HAS_GetModuleFileName
# include <shlobj.h>
#endif

#include <cstring>
#include <glib/gstdio.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <glibmm/convert.h>
#include <gtk/gtk.h>
#include <signal.h>
#include <string>

#include "desktop.h"

#include "device-manager.h"
#include "document.h"
#include "extension/db.h"
#include "extension/init.h"
#include "extension/output.h"
#include "extension/system.h"
#include "helper/action-context.h"
#include "helper/sp-marshal.h"
#include "inkscape.h"
#include "io/sys.h"
#include "layer-model.h"
#include "message-stack.h"
#include "preferences.h"
#include "resource-manager.h"
#include "selection.h"
#include "ui/tools/tool-base.h"
#include "ui/dialog/debug.h"
#include "xml/repr.h"

/* Backbones of configuration xml data */
#include "menus-skeleton.h"

// Inkscape::Application static members
Inkscape::Application * Inkscape::Application::_S_inst = NULL;
bool Inkscape::Application::_crashIsHappening = false;

#define DESKTOP_IS_ACTIVE(d) (!INKSCAPE._desktops->empty() && ((d) == INKSCAPE._desktops->front()))

static void (* segv_handler) (int) = SIG_DFL;
static void (* abrt_handler) (int) = SIG_DFL;
static void (* fpe_handler)  (int) = SIG_DFL;
static void (* ill_handler)  (int) = SIG_DFL;
#ifndef WIN32
static void (* bus_handler)  (int) = SIG_DFL;
#endif

#define INKSCAPE_PROFILE_DIR "inkscape"
#define INKSCAPE_PROFILE_DIR_047DEV "Inkscape"
#define INKSCAPE_LEGACY_PROFILE_DIR ".inkscape"
#define MENUS_FILE "menus.xml"

#define SP_INDENT 8

#ifdef WIN32
typedef int uid_t;
#define getuid() 0
#endif

/**  C++ification TODO list
 * - _S_inst should NOT need to be assigned inside the constructor, but if it isn't the Filters+Extensions menus break. 
 * - Application::_deskops has to be a pointer because of a signal bug somewhere else. Basically, it will attempt to access a deleted object in sp_ui_close_all(),
 *   but if it's a pointer we can stop and return NULL in Application::active_desktop()
 * - These functions are calling Application::create for no good reason I can determine:
 *
 *   Inkscape::UI::Dialog::SVGPreview::SVGPreview()
 *       src/ui/dialog/filedialogimpl-gtkmm.cpp:542:9
 *   void Inkscape::UI::Widget::ImageIcon::init()
 *       src/ui/widget/imageicon.cpp:93:9
 */


class InkErrorHandler : public Inkscape::ErrorReporter {
public:
    InkErrorHandler(bool useGui) : Inkscape::ErrorReporter(),
                                   _useGui(useGui)
    {}
    virtual ~InkErrorHandler() {}

    virtual void handleError( Glib::ustring const& primary, Glib::ustring const& secondary ) const
    {
        if (_useGui) {
            Gtk::MessageDialog err(primary, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
            err.set_secondary_text(secondary);
            err.run();
        } else {
            g_message("%s", primary.data());
            g_message("%s", secondary.data());
        }
    }

private:
    bool _useGui;
};

void inkscape_ref(Inkscape::Application & in)
{
    in.refCount++;
}

void inkscape_unref(Inkscape::Application & in)
{
    in.refCount--;

    if (&in == Inkscape::Application::_S_inst) {
        if (in.refCount <= 0) {
            delete Inkscape::Application::_S_inst;
        }
    } else {
        g_error("Attempt to unref an Application (=%p) not the current instance (=%p) (maybe it's already been destroyed?)", 
                &in, Inkscape::Application::_S_inst);
    }
}

// Callback passed to g_timeout_add_seconds()
// gets the current instance and calls autosave()
int inkscape_autosave(gpointer) {
    g_assert(Inkscape::Application::exists());
    return INKSCAPE.autosave();
}

namespace Inkscape {

/**
 * Defined only for debugging purposes. If we are certain the bugs are gone we can remove this
 * and the references in inkscape_ref and inkscape_unref.
 */
Application*
Application::operator &() const
{
    return const_cast<Application*>(this);
}
/**
 *  Creates a new Inkscape::Application global object.
 */
void
Application::create(const char *argv0, bool use_gui)
{
   if (!Application::exists()) {
        new Application(argv0, use_gui);
    } else {
        g_assert_not_reached();
    }
}


/**
 *  Checks whether the current Inkscape::Application global object exists.
 */
bool
Application::exists()
{
    return Application::_S_inst != NULL;
}

/**
 *  Returns the current Inkscape::Application global object.
 *  \pre Application::_S_inst != NULL
 */
Application&
Application::instance()
{
    if (!exists()) {
         g_error("Inkscape::Application does not yet exist.");
    }
    return *Application::_S_inst;
}

/**
 * static gint inkscape_autosave(gpointer);
 *
 * Responsible for autosaving all open documents
 */
int Application::autosave()
{
    if (_document_set.empty()) { // nothing to autosave
        return TRUE;
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // Use UID for separating autosave-documents between users if directory is multiuser
    uid_t uid = getuid();

    Glib::ustring autosave_dir;
    {
        Glib::ustring tmp = prefs->getString("/options/autosave/path");
        if (!tmp.empty()) {
            autosave_dir = tmp;
        } else {
            autosave_dir = Glib::get_tmp_dir();
        }
    }

    GDir *autosave_dir_ptr = g_dir_open(autosave_dir.c_str(), 0, NULL);
    if (!autosave_dir_ptr) {
        // Try to create the autosave directory if it doesn't exist
        if (g_mkdir(autosave_dir.c_str(), 0755)) {
            // the creation failed
            Glib::ustring msg = Glib::ustring::compose(
                    _("Autosave failed! Cannot create directory %1."), Glib::filename_to_utf8(autosave_dir));
            g_warning("%s", msg.c_str());
            SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg.c_str());
            return TRUE;
        }
        // Try to read dir again
        autosave_dir_ptr = g_dir_open(autosave_dir.c_str(), 0, NULL);
        if( !autosave_dir_ptr ){
            Glib::ustring msg = Glib::ustring::compose(
                    _("Autosave failed! Cannot open directory %1."), Glib::filename_to_utf8(autosave_dir));
            g_warning("%s", msg.c_str());
            SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg.c_str());
            return TRUE;
        }
    }

    time_t sptime = time(NULL);
    struct tm *sptm = localtime(&sptime);
    gchar sptstr[256];
    strftime(sptstr, 256, "%Y_%m_%d_%H_%M_%S", sptm);

    gint autosave_max = prefs->getInt("/options/autosave/max", 10);

    gint docnum = 0;

    SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Autosaving documents..."));
    for (std::map<SPDocument*,int>::iterator iter = _document_set.begin();
          iter != _document_set.end();
          ++iter) {

        SPDocument *doc = iter->first;

        ++docnum;

        Inkscape::XML::Node *repr = doc->getReprRoot();

        if (doc->isModifiedSinceSave()) {
            gchar *oldest_autosave = 0;
            const gchar  *filename = 0;
            struct stat sb;
            time_t min_time = 0;
            gint count = 0;

            // Look for previous autosaves
            gchar* baseName = g_strdup_printf( "inkscape-autosave-%d", uid );
            g_dir_rewind(autosave_dir_ptr);
            while( (filename = g_dir_read_name(autosave_dir_ptr)) != NULL ){
                if ( strncmp(filename, baseName, strlen(baseName)) == 0 ){
                    gchar* full_path = g_build_filename( autosave_dir.c_str(), filename, NULL );
                    if (g_file_test (full_path, G_FILE_TEST_EXISTS)){ 
                        if ( g_stat(full_path, &sb) != -1 ) {
                            if ( difftime(sb.st_ctime, min_time) < 0 || min_time == 0 ){
                                min_time = sb.st_ctime;
                                if ( oldest_autosave ) {
                                    g_free(oldest_autosave);
                                }
                                oldest_autosave = g_strdup(full_path);
                            }
                            count ++;
                        }
                    }
                    g_free(full_path);
                }
            }

            // Have we reached the limit for number of autosaves?
            if ( count >= autosave_max ){
                // Remove the oldest file
                if ( oldest_autosave ) {
                    unlink(oldest_autosave);
                }
            }

            if ( oldest_autosave ) {
                g_free(oldest_autosave);
                oldest_autosave = 0;
            }


            // Set the filename we will actually save to
            g_free(baseName);
            baseName = g_strdup_printf("inkscape-autosave-%d-%s-%03d.svg", uid, sptstr, docnum);
            gchar* full_path = g_build_filename(autosave_dir.c_str(), baseName, NULL);
            g_free(baseName);
            baseName = 0;

            // Try to save the file
            FILE *file = Inkscape::IO::fopen_utf8name(full_path, "w");
            gchar *errortext = 0;
            if (file) {
                try{
                    sp_repr_save_stream(repr->document(), file, SP_SVG_NS_URI);
                } catch (Inkscape::Extension::Output::no_extension_found &e) {
                    errortext = g_strdup(_("Autosave failed! Could not find inkscape extension to save document."));
                } catch (Inkscape::Extension::Output::save_failed &e) {
                    gchar *safeUri = Inkscape::IO::sanitizeString(full_path);
                    errortext = g_strdup_printf(_("Autosave failed! File %s could not be saved."), safeUri);
                    g_free(safeUri);
                }
                fclose(file);
            }
            else {
                gchar *safeUri = Inkscape::IO::sanitizeString(full_path);
                errortext = g_strdup_printf(_("Autosave failed! File %s could not be saved."), safeUri);
                g_free(safeUri);
            }

            if (errortext) {
                SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, errortext);
                g_warning("%s", errortext);
                g_free(errortext);
            }

            g_free(full_path);
        }
    }
    g_dir_close(autosave_dir_ptr);

    SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Autosave complete."));

    return TRUE;
}

void Application::autosave_init()
{
    static guint32 autosave_timeout_id = 0;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // Turn off any previously initiated timeouts
    if ( autosave_timeout_id ) {
        g_source_remove(autosave_timeout_id);
        autosave_timeout_id = 0;
    }

    // Is autosave enabled?
    if (!prefs->getBool("/options/autosave/enable", true)){
        autosave_timeout_id = 0;
    } else {
        // Turn on autosave
        guint32 timeout = prefs->getInt("/options/autosave/interval", 10) * 60;
        autosave_timeout_id = g_timeout_add_seconds(timeout, inkscape_autosave, NULL);
    }
}

void Application::argv0(char const* argv)
{
    _argv0 = g_strdup(argv);
}

/* \brief Constructor for the application.
 *  Creates a new Inkscape::Application.
 *
 *  \pre Application::_S_inst == NULL
 */

Application::Application(const char* argv, bool use_gui) :
    _menus(NULL),
    _desktops(NULL),
    refCount(1),
    _dialogs_toggle(TRUE),
    _mapalt(GDK_MOD1_MASK),
    _trackalt(FALSE),
    _use_gui(use_gui)
{
    /* fixme: load application defaults */

    segv_handler = signal (SIGSEGV, Application::crash_handler);
    abrt_handler = signal (SIGABRT, Application::crash_handler);
    fpe_handler  = signal (SIGFPE,  Application::crash_handler);
    ill_handler  = signal (SIGILL,  Application::crash_handler);
#ifndef WIN32
    bus_handler  = signal (SIGBUS,  Application::crash_handler);
#endif

    _argv0 = g_strdup(argv);

    // \TODO: this belongs to Application::init but if it isn't here
    // then the Filters and Extensions menus don't work.
    _S_inst = this;

    /* Load the preferences and menus */
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    InkErrorHandler* handler = new InkErrorHandler(use_gui);
    prefs->setErrorHandler(handler);
    {
        Glib::ustring msg;
        Glib::ustring secondary;
        if (prefs->getLastError( msg, secondary )) {
            handler->handleError(msg, secondary);
        }
    }

    if (use_gui) {
        load_menus();
        Inkscape::DeviceManager::getManager().loadConfig();
    }
    Inkscape::ResourceManager::getManager();

    /* set language for user interface according setting in preferences */
    Glib::ustring ui_language = prefs->getString("/ui/language");
    if(!ui_language.empty())
    {
        setenv("LANGUAGE", ui_language, 1);
    }

    /* DebugDialog redirection.  On Linux, default to OFF, on Win32, default to ON.
     * Use only if use_gui is enabled
     */
#ifdef WIN32
#define DEFAULT_LOG_REDIRECT true
#else
#define DEFAULT_LOG_REDIRECT false
#endif

    if (use_gui == TRUE && prefs->getBool("/dialogs/debug/redirect", DEFAULT_LOG_REDIRECT))
    {
        Inkscape::UI::Dialog::DebugDialog::getInstance()->captureLogMessages();
    }

    /* Check for global remapping of Alt key */
    if (use_gui)
    {
        mapalt(guint(prefs->getInt("/options/mapalt/value", 0)));
        trackalt(guint(prefs->getInt("/options/trackalt/value", 0)));
    }

    /* Initialize the extensions */
    Inkscape::Extension::init();

    autosave_init();
}

Application::~Application()
{
    if (_desktops) {
        g_error("FATAL: desktops still in list on application destruction!");
    }

    Inkscape::Preferences::unload();

    if (_menus) {
        Inkscape::GC::release(_menus);
        _menus = NULL;
    }

    if (_argv0) {
        g_free(_argv0);
        _argv0 = NULL;
    }

    _S_inst = NULL; // this will probably break things

    refCount = 0;
    gtk_main_quit ();
}

/** Sets the keyboard modifer to map to Alt.
 *
 * Zero switches off mapping, as does '1', which is the default.
 */
void Application::mapalt(guint maskvalue)
{
    if ( maskvalue < 2 || maskvalue > 5 ) {  // MOD5 is the highest defined in gdktypes.h
        _mapalt = 0;
    } else {
        _mapalt = (GDK_MOD1_MASK << (maskvalue-1));
    }
}

void
Application::crash_handler (int /*signum*/)
{
    using Inkscape::Debug::SimpleEvent;
    using Inkscape::Debug::EventTracker;
    using Inkscape::Debug::Logger;

    static bool recursion = false;

    /*
     * reset all signal handlers: any further crashes should just be allowed
     * to crash normally.
     * */
    signal (SIGSEGV, segv_handler );
    signal (SIGABRT, abrt_handler );
    signal (SIGFPE,  fpe_handler  );
    signal (SIGILL,  ill_handler  );
#ifndef WIN32
    signal (SIGBUS,  bus_handler  );
#endif

    /* Stop bizarre loops */
    if (recursion) {
        abort ();
    }
    recursion = true;

    _crashIsHappening = true;

    EventTracker<SimpleEvent<Inkscape::Debug::Event::CORE> > tracker("crash");
    tracker.set<SimpleEvent<> >("emergency-save");

    fprintf(stderr, "\nEmergency save activated!\n");

    time_t sptime = time (NULL);
    struct tm *sptm = localtime (&sptime);
    gchar sptstr[256];
    strftime (sptstr, 256, "%Y_%m_%d_%H_%M_%S", sptm);

    gint count = 0;
    gchar *curdir = g_get_current_dir(); // This one needs to be freed explicitly
    gchar *inkscapedir = g_path_get_dirname(INKSCAPE._argv0); // Needs to be freed
    GSList *savednames = NULL;
    GSList *failednames = NULL;
    for (std::map<SPDocument*,int>::iterator iter = INKSCAPE._document_set.begin(), e = INKSCAPE._document_set.end();
          iter != e;
          ++iter) {
        SPDocument *doc = iter->first;
        Inkscape::XML::Node *repr;
        repr = doc->getReprRoot();
        if (doc->isModifiedSinceSave()) {
            const gchar *docname;

            /* originally, the document name was retrieved from
             * the sodipod:docname attribute */
            docname = doc->getName();
            if (docname) {
                /* Removes an emergency save suffix if present: /(.*)\.[0-9_]*\.[0-9_]*\.[~\.]*$/\1/ */
                const char* d0 = strrchr ((char*)docname, '.');
                if (d0 && (d0 > docname)) {
                    const char* d = d0;
                    unsigned int dots = 0;
                    while ((isdigit (*d) || *d=='_' || *d=='.') && d>docname && dots<2) {
                        d -= 1;
                        if (*d=='.') dots++;
                    }
                    if (*d=='.' && d>docname && dots==2) {
                        char n[64];
                        size_t len = MIN (d - docname, 63);
                        memcpy (n, docname, len);
                        n[len] = '\0';
                        docname = n;
                    }
                }
            }
            if (!docname || !*docname) docname = "emergency";

            // Emergency filename
            char c[1024];
            g_snprintf (c, 1024, "%.256s.%s.%d.svg", docname, sptstr, count);

            // Find a location
            const char* locations[] = {
                doc->getBase(),
                g_get_home_dir(),
                g_get_tmp_dir(),
                curdir,
                inkscapedir
            };
            FILE *file = 0;
            for(size_t i=0; i<sizeof(locations)/sizeof(*locations); i++) {
                if (!locations[i]) continue; // It seems to be okay, but just in case
                gchar * filename = g_build_filename(locations[i], c, NULL);
                Inkscape::IO::dump_fopen_call(filename, "E");
                file = Inkscape::IO::fopen_utf8name(filename, "w");
                if (file) {
                    g_snprintf (c, 1024, "%s", filename); // we want the complete path to be stored in c (for reporting purposes)
                    break;
                }
            }

            // Save
            if (file) {
                sp_repr_save_stream (repr->document(), file, SP_SVG_NS_URI);
                savednames = g_slist_prepend (savednames, g_strdup (c));
                fclose (file);
            } else {
                failednames = g_slist_prepend (failednames, (doc->getName()) ? g_strdup(doc->getName()) : g_strdup (_("Untitled document")));
            }
            count++;
        }
    }
    g_free(curdir);
    g_free(inkscapedir);

    savednames = g_slist_reverse (savednames);
    failednames = g_slist_reverse (failednames);
    if (savednames) {
        fprintf (stderr, "\nEmergency save document locations:\n");
        for (GSList *l = savednames; l != NULL; l = l->next) {
            fprintf (stderr, "  %s\n", (gchar *) l->data);
        }
    }
    if (failednames) {
        fprintf (stderr, "\nFailed to do emergency save for documents:\n");
        for (GSList *l = failednames; l != NULL; l = l->next) {
            fprintf (stderr, "  %s\n", (gchar *) l->data);
        }
    }

    // do not save the preferences since they can be in a corrupted state
    Inkscape::Preferences::unload(false);

    fprintf (stderr, "Emergency save completed. Inkscape will close now.\n");
    fprintf (stderr, "If you can reproduce this crash, please file a bug at www.inkscape.org\n");
    fprintf (stderr, "with a detailed description of the steps leading to the crash, so we can fix it.\n");

    /* Show nice dialog box */

    char const *istr = _("Inkscape encountered an internal error and will close now.\n");
    char const *sstr = _("Automatic backups of unsaved documents were done to the following locations:\n");
    char const *fstr = _("Automatic backup of the following documents failed:\n");
    gint nllen = strlen ("\n");
    gint len = strlen (istr) + strlen (sstr) + strlen (fstr);
    for (GSList *l = savednames; l != NULL; l = l->next) {
        len = len + SP_INDENT + strlen ((gchar *) l->data) + nllen;
    }
    for (GSList *l = failednames; l != NULL; l = l->next) {
        len = len + SP_INDENT + strlen ((gchar *) l->data) + nllen;
    }
    len += 1;
    gchar *b = g_new (gchar, len);
    gint pos = 0;
    len = strlen (istr);
    memcpy (b + pos, istr, len);
    pos += len;
    if (savednames) {
        len = strlen (sstr);
        memcpy (b + pos, sstr, len);
        pos += len;
        for (GSList *l = savednames; l != NULL; l = l->next) {
            memset (b + pos, ' ', SP_INDENT);
            pos += SP_INDENT;
            len = strlen ((gchar *) l->data);
            memcpy (b + pos, l->data, len);
            pos += len;
            memcpy (b + pos, "\n", nllen);
            pos += nllen;
        }
    }
    if (failednames) {
        len = strlen (fstr);
        memcpy (b + pos, fstr, len);
        pos += len;
        for (GSList *l = failednames; l != NULL; l = l->next) {
            memset (b + pos, ' ', SP_INDENT);
            pos += SP_INDENT;
            len = strlen ((gchar *) l->data);
            memcpy (b + pos, l->data, len);
            pos += len;
            memcpy (b + pos, "\n", nllen);
            pos += nllen;
        }
    }
    *(b + pos) = '\0';

    if ( exists() && instance().use_gui() ) {
        GtkWidget *msgbox = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", b);
        gtk_dialog_run (GTK_DIALOG (msgbox));
        gtk_widget_destroy (msgbox);
    }
    else
    {
        g_message( "Error: %s", b );
    }
    g_free (b);

    tracker.clear();
    Logger::shutdown();

    /* on exit, allow restored signal handler to take over and crash us */
}

/**
 *  Menus management
 *
 */
bool Application::load_menus()
{
    gchar *fn = profile_path(MENUS_FILE);
    gchar *menus_xml = 0;
    gsize len = 0;

    if ( g_file_get_contents(fn, &menus_xml, &len, NULL) ) {
        // load the menus_xml file
        _menus = sp_repr_read_mem(menus_xml, len, NULL);

        g_free(menus_xml);
        menus_xml = 0;
    }
    g_free(fn);
    fn = 0;

    if ( !_menus ) {
        _menus = sp_repr_read_mem(menus_skeleton, MENUS_SKELETON_SIZE, NULL);
    }

    return (_menus != 0);
}


void
Application::selection_modified (Inkscape::Selection *selection, guint flags)
{
    g_return_if_fail (selection != NULL);

    if (DESKTOP_IS_ACTIVE (selection->desktop())) {
        signal_selection_modified.emit(selection, flags);
    }
}


void
Application::selection_changed (Inkscape::Selection * selection)
{
    g_return_if_fail (selection != NULL);
    
    if (DESKTOP_IS_ACTIVE (selection->desktop())) {
        signal_selection_changed.emit(selection);
    }
}

void
Application::subselection_changed (SPDesktop *desktop)
{
    g_return_if_fail (desktop != NULL);

    if (DESKTOP_IS_ACTIVE (desktop)) {
        signal_subselection_changed.emit(desktop);
    }
}


void
Application::selection_set (Inkscape::Selection * selection)
{
    g_return_if_fail (selection != NULL);

    if (DESKTOP_IS_ACTIVE (selection->desktop())) {
        signal_selection_set.emit(selection);
        signal_selection_changed.emit(selection);
    }
}


void
Application::eventcontext_set (Inkscape::UI::Tools::ToolBase * eventcontext)
{
    g_return_if_fail (eventcontext != NULL);
    g_return_if_fail (SP_IS_EVENT_CONTEXT (eventcontext));

    if (DESKTOP_IS_ACTIVE (eventcontext->desktop)) {
        signal_eventcontext_set.emit(eventcontext);
    }
}


void
Application::add_desktop (SPDesktop * desktop)
{
    g_return_if_fail (desktop != NULL);
    if (_desktops == NULL) {
        _desktops = new std::vector<SPDesktop*>;
    }

    if (std::find(_desktops->begin(), _desktops->end(), desktop) != _desktops->end()) {
        g_error("Attempted to add desktop already in list.");
    }

    _desktops->insert(_desktops->begin(), desktop);

    signal_activate_desktop.emit(desktop);
    signal_eventcontext_set.emit(desktop->getEventContext());
    signal_selection_set.emit(desktop->getSelection());
    signal_selection_changed.emit(desktop->getSelection());
}



void
Application::remove_desktop (SPDesktop * desktop)
{
    g_return_if_fail (desktop != NULL);

    if (std::find (_desktops->begin(), _desktops->end(), desktop) == _desktops->end() ) {
        g_error("Attempted to remove desktop not in list.");
    }

    if (DESKTOP_IS_ACTIVE (desktop)) {
        signal_deactivate_desktop.emit(desktop);
        if (_desktops->size() > 1) {
            SPDesktop * new_desktop = *(++_desktops->begin());
            _desktops->erase(std::find(_desktops->begin(), _desktops->end(), new_desktop));
            _desktops->insert(_desktops->begin(), new_desktop);
            
            signal_activate_desktop.emit(new_desktop);
            signal_eventcontext_set.emit(new_desktop->getEventContext());
            signal_selection_set.emit(new_desktop->getSelection());
            signal_selection_changed.emit(new_desktop->getSelection());
        } else {
            signal_eventcontext_set.emit(NULL);
            if (desktop->getSelection())
                desktop->getSelection()->clear();
        }
    }

    _desktops->erase(std::find(_desktops->begin(), _desktops->end(), desktop));

    // if this was the last desktop, shut down the program
    if (_desktops->empty()) {
        this->exit();
        delete _desktops;
        _desktops = NULL;
    }
}



void
Application::activate_desktop (SPDesktop * desktop)
{
    g_return_if_fail (desktop != NULL);

    if (DESKTOP_IS_ACTIVE (desktop)) {
        return;
    }

    std::vector<SPDesktop*>::iterator i;
    
    if ((i = std::find (_desktops->begin(), _desktops->end(), desktop)) == _desktops->end()) {
        g_error("Tried to activate desktop not added to list.");
    }

    SPDesktop *current = _desktops->front();

    signal_deactivate_desktop.emit(current);

    _desktops->erase (i);
    _desktops->insert (_desktops->begin(), desktop);

    signal_activate_desktop.emit(desktop);
    signal_eventcontext_set.emit(desktop->getEventContext());
    signal_selection_set(desktop->getSelection());
    signal_selection_changed(desktop->getSelection());
}


/**
 *  Resends ACTIVATE_DESKTOP for current desktop; needed when a new desktop has got its window that dialogs will transientize to
 */
void
Application::reactivate_desktop (SPDesktop * desktop)
{
    g_return_if_fail (desktop != NULL);

    if (DESKTOP_IS_ACTIVE (desktop)) {
        signal_activate_desktop.emit(desktop);
    }
}



SPDesktop *
Application::find_desktop_by_dkey (unsigned int dkey)
{
    for (std::vector<SPDesktop*>::iterator r = _desktops->begin(), e = _desktops->end(); r != e; ++r) {
        if ((*r)->dkey == dkey){
            return *r;
        }
    }
    return NULL;
}


unsigned int
Application::maximum_dkey()
{
    unsigned int dkey = 0;

    for (std::vector<SPDesktop*>::iterator r = _desktops->begin(), e = _desktops->end(); r != e; ++r) {
        if ((*r)->dkey > dkey){
            dkey = (*r)->dkey;
        }
    }
    return dkey;
}



SPDesktop *
Application::next_desktop ()
{
    SPDesktop *d = NULL;
    unsigned int dkey_current = (_desktops->front())->dkey;

    if (dkey_current < maximum_dkey()) {
        // find next existing
        for (unsigned int i = dkey_current + 1; i <= maximum_dkey(); ++i) {
            d = find_desktop_by_dkey (i);
            if (d) {
                break;
            }
        }
    } else {
        // find first existing
        for (unsigned int i = 0; i <= maximum_dkey(); ++i) {
            d = find_desktop_by_dkey (i);
            if (d) {
                break;
            }
        }
    }

    g_assert (d);
    return d;
}



SPDesktop *
Application::prev_desktop ()
{
    SPDesktop *d = NULL;
    unsigned int dkey_current = (_desktops->front())->dkey;

    if (dkey_current > 0) {
        // find prev existing
        for (signed int i = dkey_current - 1; i >= 0; --i) {
            d = find_desktop_by_dkey (i);
            if (d) {
                break;
            }
        }
    }
    if (!d) {
        // find last existing
        d = find_desktop_by_dkey (maximum_dkey());
    }

    g_assert (d);
    return d;
}



void
Application::switch_desktops_next ()
{
    next_desktop()->presentWindow();
}

void
Application::switch_desktops_prev()
{
    prev_desktop()->presentWindow();
}

void
Application::dialogs_hide()
{
    signal_dialogs_hide.emit();
    _dialogs_toggle = false;
}



void
Application::dialogs_unhide()
{
    signal_dialogs_unhide.emit();
    _dialogs_toggle = true;
}



void
Application::dialogs_toggle()
{
    if (_dialogs_toggle) {
        dialogs_hide();
    } else {
        dialogs_unhide();
    }
}

void
Application::external_change()
{
    signal_external_change.emit();
}

/**
 * fixme: These need probably signals too
 */
void
Application::add_document (SPDocument *document)
{
    g_return_if_fail (document != NULL);

    // try to insert the pair into the list
    if (!(_document_set.insert(std::make_pair(document, 1)).second)) {
        //insert failed, this key (document) is already in the list
        for (std::map<SPDocument*,int>::iterator iter = _document_set.begin();
               iter != _document_set.end();
               ++iter) {
            if (iter->first == document) {
                // found this document in list, increase its count
                iter->second ++;
            }
       }
    } else {
        // insert succeeded, this document is new. Do we need to create a
        // selection model for it, i.e. are we running without a desktop?
        if (!_use_gui) {
            // Create layer model and selection model so we can run some verbs without a GUI
            g_assert(_selection_models.find(document) == _selection_models.end());
            _selection_models[document] = new AppSelectionModel(document);
        }
    }
}


// returns true if this was last reference to this document, so you can delete it
bool
Application::remove_document (SPDocument *document)
{
    g_return_val_if_fail (document != NULL, false);

    for (std::map<SPDocument *,int>::iterator iter = _document_set.begin();
              iter != _document_set.end();
              ++iter) {
        if (iter->first == document) {
            // found this document in list, decrease its count
            iter->second --;
            if (iter->second < 1) {
                // this was the last one, remove the pair from list
                _document_set.erase (iter);

                // also remove the selection model
                std::map<SPDocument *, AppSelectionModel *>::iterator sel_iter = _selection_models.find(document);
                if (sel_iter != _selection_models.end()) {
                    _selection_models.erase(sel_iter);
                }

                return true;
            } else {
                return false;
            }
        }
    }

    return false;
}

SPDesktop *
Application::active_desktop()
{
    if (!_desktops || _desktops->empty()) {
        return NULL;
    }

    return _desktops->front();
}

SPDocument *
Application::active_document()
{
    if (SP_ACTIVE_DESKTOP) {
        return SP_ACTIVE_DESKTOP->getDocument();
    } else if (!_document_set.empty()) {
        // If called from the command line there will be no desktop
        // So 'fall back' to take the first listed document in the Inkscape instance
        return _document_set.begin()->first;
    }

    return NULL;
}

bool
Application::sole_desktop_for_document(SPDesktop const &desktop) {
    SPDocument const* document = desktop.doc();
    if (!document) {
        return false;
    }
    for ( std::vector<SPDesktop*>::iterator iter = _desktops->begin(), e = _desktops->end() ; iter != e; ++iter ) {
        SPDesktop *other_desktop = *iter;
        SPDocument *other_document = other_desktop->doc();
        if ( other_document == document && other_desktop != &desktop ) {
            return false;
        }
    }
    return true;
}

Inkscape::UI::Tools::ToolBase *
Application::active_event_context (void)
{
    if (SP_ACTIVE_DESKTOP) {
        return SP_ACTIVE_DESKTOP->getEventContext();
    }

    return NULL;
}

Inkscape::ActionContext
Application::active_action_context()
{
    if (SP_ACTIVE_DESKTOP) {
        return Inkscape::ActionContext(SP_ACTIVE_DESKTOP);
    }

    SPDocument *doc = active_document();
    if (!doc) {
        return Inkscape::ActionContext();
    }

    return action_context_for_document(doc);
}

Inkscape::ActionContext
Application::action_context_for_document(SPDocument *doc)
{
    // If there are desktops, check them first to see if the document is bound to one of them
    if (_desktops != NULL) {
        for (std::vector<SPDesktop*>::iterator iter = _desktops->begin(), e = _desktops->end() ; iter != e ; ++iter) {
            SPDesktop *desktop = *iter;
            if (desktop->doc() == doc) {
                return Inkscape::ActionContext(desktop);
            }
        }
    }

    // Document is not associated with any desktops - maybe we're in command-line mode
    std::map<SPDocument *, AppSelectionModel *>::iterator sel_iter = _selection_models.find(doc);
    if (sel_iter == _selection_models.end()) {
        return Inkscape::ActionContext();
    }
    return Inkscape::ActionContext(sel_iter->second->getSelection());
}


/*#####################
# HELPERS
#####################*/

void
Application::refresh_display ()
{
    for (std::vector<SPDesktop*>::iterator l = _desktops->begin(), e = _desktops->end(); l != e; ++l) {
        (*l)->requestRedraw();
    }
}


/**
 *  Handler for Inkscape's Exit verb.  This emits the shutdown signal,
 *  saves the preferences if appropriate, and quits.
 */
void
Application::exit ()
{
    //emit shutdown signal so that dialogs could remember layout
    signal_shut_down.emit();

    Inkscape::Preferences::unload();
    gtk_main_quit ();
}

char *
Application::homedir_path(const char *filename)
{
    static const gchar *homedir = NULL;
    if (!homedir) {
        homedir = g_get_home_dir();
    }
    if (!homedir) {
        if (Application::exists()) {
            homedir = g_path_get_dirname(Application::instance()._argv0);
        }
    }
    return g_build_filename(homedir, filename, NULL);
}


/**
 * Get, or guess, or decide the location where the preferences.xml
 * file should be located.
 */
gchar *
Application::profile_path(const char *filename)
{
    static const gchar *prefdir = NULL;


    if (!prefdir) {
        // First check for a custom environment variable for a "portable app"
        gchar const *val = g_getenv("INKSCAPE_PORTABLE_PROFILE_DIR");
        if (val) {
            prefdir = g_strdup(val);
        }
        // Then check for a custom user environment variable
        gchar const *userenv = g_getenv("INKSCAPE_PROFILE_DIR");
        if (userenv) {
            prefdir = g_strdup(userenv);
        }

#ifdef HAS_SHGetSpecialFolderLocation
        // prefer c:\Documents and Settings\UserName\Application Data\ to
        // c:\Documents and Settings\userName\;
        if (!prefdir) {
            ITEMIDLIST *pidl = 0;
            if ( SHGetSpecialFolderLocation( NULL, CSIDL_APPDATA, &pidl ) == NOERROR ) {
                gchar * utf8Path = NULL;

                {
                    wchar_t pathBuf[MAX_PATH+1];
                    g_assert(sizeof(wchar_t) == sizeof(gunichar2));

                    if ( SHGetPathFromIDListW( pidl, pathBuf ) ) {
                        utf8Path = g_utf16_to_utf8( (gunichar2*)(&pathBuf[0]), -1, NULL, NULL, NULL );
                    }
                }

                if ( utf8Path ) {
                    if (!g_utf8_validate(utf8Path, -1, NULL)) {
                        g_warning( "SHGetPathFromIDListW() resulted in invalid UTF-8");
                        g_free( utf8Path );
                        utf8Path = 0;
                    } else {
                        prefdir = utf8Path;
                    }
                }


                /* not compiling yet...

                // Remember to free the list pointer
                IMalloc * imalloc = 0;
                if ( SHGetMalloc(&imalloc) == NOERROR) {
                    imalloc->lpVtbl->Free( imalloc, pidl );
                    imalloc->lpVtbl->Release( imalloc );
                }
                */
            }

            if (prefdir) {
                const char *prefdir_profile = g_build_filename(prefdir, INKSCAPE_PROFILE_DIR, NULL);
                g_free((void *)prefdir);
                prefdir = prefdir_profile;
            }
        }
#endif
        if (!prefdir) {
            prefdir = g_build_filename(g_get_user_config_dir(), INKSCAPE_PROFILE_DIR, NULL);
            gchar * legacyDir = homedir_path(INKSCAPE_LEGACY_PROFILE_DIR);
            gchar * dev47Dir = g_build_filename(g_get_user_config_dir(), INKSCAPE_PROFILE_DIR_047DEV, NULL);

            bool needsMigration = ( !Inkscape::IO::file_test( prefdir, G_FILE_TEST_EXISTS ) && Inkscape::IO::file_test( legacyDir, G_FILE_TEST_EXISTS ) );
            if (needsMigration) {
                // TODO here is a point to hook in preference migration
                g_warning("Preferences need to be migrated from 0.46 or older %s to %s", legacyDir, prefdir);
                Inkscape::Preferences::migrate( legacyDir, prefdir );
            }

            bool needsRenameWarning = ( !Inkscape::IO::file_test( prefdir, G_FILE_TEST_EXISTS ) && Inkscape::IO::file_test( dev47Dir, G_FILE_TEST_EXISTS ) );
            if (needsRenameWarning) {
                g_warning("Preferences need to be copied from  %s to %s", legacyDir, prefdir);
            }

            g_free(legacyDir);
            legacyDir = 0;
            g_free(dev47Dir);
            dev47Dir = 0;
            // In case the XDG user config dir of the moment does not yet exist...
            int mode = S_IRWXU;
#ifdef S_IRGRP
            mode |= S_IRGRP;
#endif
#ifdef S_IXGRP
            mode |= S_IXGRP;
#endif
#ifdef S_IXOTH
            mode |= S_IXOTH;
#endif
            if ( g_mkdir_with_parents(prefdir, mode) == -1 ) {
                int problem = errno;
                g_warning("Unable to create profile directory (%s) (%d)", g_strerror(problem), problem);
            } else {
                gchar const *userDirs[] = {"keys", "templates", "icons", "extensions", "palettes", NULL};
                for (gchar const** name = userDirs; *name; ++name) {
                    gchar *dir = g_build_filename(prefdir, *name, NULL);
                    g_mkdir_with_parents(dir, mode);
                    g_free(dir);
                }
            }
        }
    }
    return g_build_filename(prefdir, filename, NULL);
}

Inkscape::XML::Node *
Application::get_menus()
{
    Inkscape::XML::Node *repr = _menus->root();
    g_assert (!(strcmp (repr->name(), "inkscape")));
    return repr->firstChild();
}

void
Application::get_all_desktops(std::list< SPDesktop* >& listbuf)
{
    listbuf.insert(listbuf.end(), _desktops->begin(), _desktops->end());
}

} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

/*
 * Inkscape - an ambitious vector drawing program
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Davide Puricelli <evo@debian.org>
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Masatake YAMATO  <jet@gyve.org>
 *   F.J.Franklin <F.J.Franklin@sheffield.ac.uk>
 *   Michael Meeks <michael@helixcode.com>
 *   Chema Celorio <chema@celorio.com>
 *   Pawel Palucha
 * ... and various people who have worked with various projects
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Inkscape authors:
 *   Johan Ceuppens
 *
 * Copyright (C) 2004 Inkscape authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <sys/stat.h>
#include <locale.h>

#include <gtkmm/applicationwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/main.h>

// #include <stropts.h>

#include <libxml/tree.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "inkgc/gc-core.h"
#include "preferences.h"

#include <glibmm/i18n.h>
#include "document.h"
#include "svg-view.h"
#include "svg-view-widget.h"
#include "util/units.h"

#ifdef WITH_INKJAR
#include "io/inkjar.h"
#endif

#include "inkscape.h"

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif

#include "ui/icon-names.h"

extern char *optarg;
extern int  optind, opterr;

/**
 * The main application window for the slideshow
 */
class SPSlideShow : public Gtk::ApplicationWindow {
public:
    std::vector<std::string>  slides;  ///< List of filenames for each slide
    int                       current; ///< Index of the currently displayed slide
    SPDocument               *doc;     ///< The currently displayed slide
    GtkWidget                *view;
    int                       timer;
    
    /// Current state of application (full-screen or windowed)
    bool is_fullscreen;

    SPSlideShow()
        :
            slides(),
            current(0),
            doc(NULL),
            view(NULL),
            is_fullscreen(false)
    {}

    void control_show();
    void show_next();
    void show_prev();
    void goto_first();
    void goto_last();

protected:
    void waiting_cursor();
    void normal_cursor();
    void set_document(SPDocument *doc,
                      int         current);
};

#ifdef WITH_INKJAR
static bool is_jar(char const *filename);
#endif
static void usage();

static GtkWidget *ctrlwin = NULL;

// Dummy functions to keep linker happy
int sp_main_gui (int, char const**) { return 0; }
int sp_main_console (int, char const**) { return 0; }

static int sp_svgview_main_delete (GtkWidget */*widget*/,
                                   GdkEvent */*event*/,
                                   struct SPSlideShow */*ss*/)
{
    gtk_main_quit ();
    return FALSE;
}

static int sp_svgview_main_key_press (GtkWidget */*widget*/, 
                                      GdkEventKey *event,
                                      struct SPSlideShow *ss)
{
    switch (event->keyval) {
        case GDK_KEY_Up:
        case GDK_KEY_Home:
            ss->goto_first();
            break;
        case GDK_KEY_Down:
        case GDK_KEY_End:
            ss->goto_last();
            break;
        case GDK_KEY_F11:
            if (ss->is_fullscreen) {
                ss->unfullscreen();
                ss->is_fullscreen = false;
            } else {
                ss->fullscreen();
                ss->is_fullscreen = true;
            }
            break;
        case GDK_KEY_Return:
            ss->control_show();
        break;
        case GDK_KEY_KP_Page_Down:
        case GDK_KEY_Page_Down:
        case GDK_KEY_Right:
        case GDK_KEY_space:
            ss->show_next();
        break;
        case GDK_KEY_KP_Page_Up:
        case GDK_KEY_Page_Up:
        case GDK_KEY_Left:
        case GDK_KEY_BackSpace:
            ss->show_prev();
            break;
        case GDK_KEY_Escape:
        case GDK_KEY_q:
        case GDK_KEY_Q:
            gtk_main_quit();
            break;
        default:
            break;
    }

    ss->set_title(ss->doc->getName());
    return TRUE;
}

int main (int argc, const char **argv)
{
    if (argc == 1) {
        usage();
    }

    // Prevents errors like "Unable to wrap GdkPixbuf..." (in nr-filter-image.cpp for example)
    Gtk::Main::init_gtkmm_internals();

    Gtk::Main main_instance (&argc, const_cast<char ***>(&argv));

    int num_parsed_options = 0;
    SPSlideShow ss;

    // the list of arguments is in the net line
    for (int i = 1; i < argc; i++) {
        if ((argv[i][0] == '-')) {
            if (!strcmp(argv[i], "--")) {
                break;
            }
            else if ((!strcmp(argv[i], "-t"))) {
                if (i + 1 >= argc) {
                    usage();
                }
                ss.timer = atoi(argv[i+1]);
                num_parsed_options = i+1;
                i++;
            }
            else {
                usage();
            }
        }
    }

    int i;

    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    LIBXML_TEST_VERSION

    Inkscape::GC::init();
    Inkscape::Preferences::get(); // ensure preferences are initialized

#ifdef lalaWITH_MODULES
    g_warning ("Have to autoinit modules (lauris)");
    sp_modulesys_init();
#endif /* WITH_MODULES */

    /* We must set LC_NUMERIC to default, or otherwise */
    /* we'll end with localised SVG files :-( */

    setlocale (LC_NUMERIC, "C");

    Inkscape::Application::create(argv[0], true);
    //Inkscape::Application &inkscape = Inkscape::Application::instance();

    // starting at where the commandline options stopped parsing because
    // we want all the files to be in the list
    for (i = num_parsed_options + 1 ; i < argc; i++) {
        struct stat st;
        if (stat (argv[i], &st)
              || !S_ISREG (st.st_mode)
              || (st.st_size < 64)) {
            fprintf(stderr, "could not open file %s\n", argv[i]);
        } else {

    #ifdef WITH_INKJAR
            if (is_jar(argv[i])) {
                Inkjar::JarFileReader jar_file_reader(argv[i]);
                for (;;) {
                    GByteArray *gba = jar_file_reader.get_next_file();
                    if (gba == NULL) {
                        char *c_ptr;
                        gchar *last_filename = jar_file_reader.get_last_filename();
                        if (last_filename == NULL)
                            break;
                        if ((c_ptr = std::strrchr(last_filename, '/')) != NULL) {
                            if (*(++c_ptr) == '\0') {
                                g_free(last_filename);
                                continue;
                            }
                        }
                    } else if (gba->len > 0) {
                        ss.doc = SPDocument::createNewDocFromMem ((const gchar *)gba->data,
                                           gba->len,
                                           TRUE);
                        gchar *last_filename = jar_file_reader.get_last_filename();
                        if (ss.doc) {
                            ss.slides.push_back(strdup(last_filename));
                            (ss.doc)->setUri(last_filename);
                        }
                        g_byte_array_free(gba, TRUE);
                        g_free(last_filename);
                    } else {
                        break;
                    }
                }
            } else {
    #endif /* WITH_INKJAR */
            /* Append to list */
            ss.slides.push_back(strdup (argv[i]));

            if (!ss.doc) {
                ss.doc = SPDocument::createNewDoc((ss.slides[ss.current]).c_str(), TRUE, false);
                if (!ss.doc) {
                    ++ss.current;
                }
            }
    #ifdef WITH_INKJAR
            }
    #endif
        }
    }

    if(!ss.doc) {
       return 1; /* none of the slides loadable */
    }
    
    ss.set_title(ss.doc->getName() );
    ss.set_default_size(MIN ((int)(ss.doc)->getWidth().value("px"), (int)gdk_screen_width() - 64),
                        MIN ((int)(ss.doc)->getHeight().value("px"), (int)gdk_screen_height() - 64));

    g_signal_connect (G_OBJECT (ss.gobj()), "delete_event", (GCallback) sp_svgview_main_delete, &ss);
    g_signal_connect (G_OBJECT (ss.gobj()), "key_press_event", (GCallback) sp_svgview_main_key_press, &ss);

    (ss.doc)->ensureUpToDate();
    ss.view = sp_svg_view_widget_new (ss.doc);
    (ss.doc)->doUnref ();
    SP_SVG_VIEW_WIDGET(ss.view)->setResize( false, ss.doc->getWidth().value("px"), ss.doc->getHeight().value("px") );
    gtk_widget_show (ss.view);
    ss.add(*Glib::wrap(ss.view));

    ss.show();

    gtk_main ();

    return 0;
}

static int sp_svgview_ctrlwin_delete (GtkWidget */*widget*/,
                                      GdkEvent */*event*/,
                                      void */*data*/)
{
    ctrlwin = NULL;
    return FALSE;
}

/**
 * @brief Show the control buttons (next, previous etc) for the application
 */
void SPSlideShow::control_show()
{
    if (!ctrlwin) {
        ctrlwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_resizable(GTK_WINDOW(ctrlwin), FALSE);
        gtk_window_set_transient_for(GTK_WINDOW(ctrlwin), GTK_WINDOW(this->gobj()));
        g_signal_connect(G_OBJECT (ctrlwin), "key_press_event", (GCallback) sp_svgview_main_key_press, this);
        g_signal_connect(G_OBJECT (ctrlwin), "delete_event", (GCallback) sp_svgview_ctrlwin_delete, NULL);
        auto t = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_container_add(GTK_CONTAINER(ctrlwin), t);

        auto btn_go_first = Gtk::manage(new Gtk::Button());
        auto img_go_first = Gtk::manage(new Gtk::Image());
        img_go_first->set_from_icon_name(INKSCAPE_ICON("go-first"), Gtk::ICON_SIZE_BUTTON);
        btn_go_first->set_image(*img_go_first);
        gtk_container_add(GTK_CONTAINER(t), GTK_WIDGET(btn_go_first->gobj()));
        btn_go_first->signal_clicked().connect(sigc::mem_fun(*this, &SPSlideShow::goto_first));
        
        auto btn_go_prev = Gtk::manage(new Gtk::Button());
        auto img_go_prev = Gtk::manage(new Gtk::Image());
        img_go_prev->set_from_icon_name(INKSCAPE_ICON("go-previous"), Gtk::ICON_SIZE_BUTTON);
        btn_go_prev->set_image(*img_go_prev);
        gtk_container_add(GTK_CONTAINER(t), GTK_WIDGET(btn_go_prev->gobj()));
        btn_go_prev->signal_clicked().connect(sigc::mem_fun(*this, &SPSlideShow::show_prev));
        
        auto btn_go_next = Gtk::manage(new Gtk::Button());
        auto img_go_next = Gtk::manage(new Gtk::Image());
        img_go_next->set_from_icon_name(INKSCAPE_ICON("go-next"), Gtk::ICON_SIZE_BUTTON);
        btn_go_next->set_image(*img_go_next);
        gtk_container_add(GTK_CONTAINER(t), GTK_WIDGET(btn_go_next->gobj()));
        btn_go_next->signal_clicked().connect(sigc::mem_fun(*this, &SPSlideShow::show_next));
        
        auto btn_go_last = Gtk::manage(new Gtk::Button());
        auto img_go_last = Gtk::manage(new Gtk::Image());
        img_go_last->set_from_icon_name(INKSCAPE_ICON("go-last"), Gtk::ICON_SIZE_BUTTON);
        btn_go_last->set_image(*img_go_last);
        gtk_container_add(GTK_CONTAINER(t), GTK_WIDGET(btn_go_last->gobj()));
        btn_go_last->signal_clicked().connect(sigc::mem_fun(*this, &SPSlideShow::goto_last));

        gtk_widget_show_all(ctrlwin);
    } else {
        gtk_window_present(GTK_WINDOW(ctrlwin));
    }
}

void SPSlideShow::waiting_cursor()
{
    auto display = Gdk::Display::get_default();
    auto waiting = Gdk::Cursor::create(display, Gdk::WATCH);
    get_window()->set_cursor(waiting);
    
    if (ctrlwin) {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(ctrlwin)), waiting->gobj());
    }
    while(gtk_events_pending()) {
       gtk_main_iteration();
    }
}

void SPSlideShow::normal_cursor()
{
    get_window()->set_cursor();
    if (ctrlwin) {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(ctrlwin)), NULL);
    }
}

void SPSlideShow::set_document(SPDocument *doc,
                               int         current)
{
    if (doc && doc != this->doc) {
        doc->ensureUpToDate();
        reinterpret_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (view))->setDocument (doc);
        this->doc = doc;
        this->current = current;
    }
}

/**
 * @brief Show the next file in the slideshow
 */
void SPSlideShow::show_next()
{
    waiting_cursor();

    SPDocument *doc = NULL;
    while (!doc && (current < slides.size() - 1)) {
        doc = SPDocument::createNewDoc ((slides[++current]).c_str(), TRUE, false);
    }

    set_document(doc, current);
    normal_cursor();
}

/**
 * @brief Show the previous file in the slideshow
 */
void SPSlideShow::show_prev()
{
    waiting_cursor();

    SPDocument *doc = NULL;
    while (!doc && (current > 0)) {
        doc = SPDocument::createNewDoc ((slides[--current]).c_str(), TRUE, false);
    }

    set_document(doc, current);
    normal_cursor();
}

/**
 * @brief Switch to first slide in slideshow
 */
void SPSlideShow::goto_first()
{
    waiting_cursor();

    SPDocument *doc = NULL;
    int current = 0;
    while ( !doc && (current < slides.size() - 1)) {
        doc = SPDocument::createNewDoc((slides[current++]).c_str(), TRUE, false);
    }

    set_document(doc, current - 1);

    normal_cursor();
}

/**
 * @brief Switch to last slide in slideshow
 */
void SPSlideShow::goto_last()
{
    waiting_cursor();

    SPDocument *doc = NULL;
    int current = slides.size() - 1;
    while (!doc && (current >= 0)) {
        doc = SPDocument::createNewDoc((slides[current--]).c_str(), TRUE, false);
    }

    set_document(doc, current + 1);

    normal_cursor();
}

#ifdef WITH_INKJAR
static bool is_jar(char const *filename)
{
    /* fixme: Check MIME type or something.  /usr/share/misc/file/magic suggests that checking for
       initial string "PK\003\004" in content should suffice. */
    size_t const filename_len = strlen(filename);
    if (filename_len < 5) {
        return false;
    }
    char const *extension = filename + filename_len - 4;
    return ((memcmp(extension, ".jar", 4) == 0) ||
            (memcmp(extension, ".sxw", 4) == 0)   );
}
#endif /* WITH_INKJAR */

static void usage()
{
    fprintf(stderr,
        "Usage: inkview [OPTIONS...] [FILES ...]\n"
        "\twhere FILES are SVG (.svg or .svgz)"
#ifdef WITH_INKJAR
        " or archives of SVGs (.sxw, .jar)"
#endif
        "\n");
    exit(1);
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

#define __SPSVGVIEW_C__

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

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <string.h>
#include <sys/stat.h>
#include <locale.h>

#include <glib/gmem.h>
#include <libnr/nr-macros.h>

// #include <stropts.h>

#include <libxml/tree.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbutton.h>

#include <gtkmm/main.h>

#include "gc-core.h"
#include "preferences.h"

#include <glibmm/i18n.h>
#include "document.h"
#include "svg-view.h"
#include "svg-view-widget.h"

#ifdef WITH_INKJAR
#include "io/inkjar.h"
#endif

#include "inkscape-private.h"

Inkscape::Application *inkscape;

#include <iostream>

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif

extern char *optarg;
extern int  optind, opterr;

struct SPSlideShow {
    char **slides;
    int size;
    int length;
    int current;
    SPDocument *doc;
    GtkWidget *view;
    GtkWidget *window;
    bool fullscreen;
    int timer;
};

static GtkWidget *sp_svgview_control_show (struct SPSlideShow *ss);
static void sp_svgview_show_next (struct SPSlideShow *ss);
static void sp_svgview_show_prev (struct SPSlideShow *ss);
static void sp_svgview_goto_first (struct SPSlideShow *ss);
static void sp_svgview_goto_last (struct SPSlideShow *ss);

static int sp_svgview_show_next_cb (GtkWidget *widget, void *data);
static int sp_svgview_show_prev_cb (GtkWidget *widget, void *data);
static int sp_svgview_goto_first_cb (GtkWidget *widget, void *data);
static int sp_svgview_goto_last_cb (GtkWidget *widget, void *data);
#ifdef WITH_INKJAR
static bool is_jar(char const *filename);
#endif
static void usage();

static GtkWidget *ctrlwin = NULL;

/// Dummy functions to keep linker happy
int sp_main_gui (int, char const**) { return 0; }
int sp_main_console (int, char const**) { return 0; }

static int
sp_svgview_main_delete (GtkWidget */*widget*/, GdkEvent */*event*/, struct SPSlideShow */*ss*/)
{
    gtk_main_quit ();
    return FALSE;
}

static int
sp_svgview_main_key_press (GtkWidget */*widget*/, GdkEventKey *event, struct SPSlideShow *ss)
{
    switch (event->keyval) {
    case GDK_Up:
    case GDK_Home:
	sp_svgview_goto_first(ss);
	break;
    case GDK_Down:
    case GDK_End:
	sp_svgview_goto_last(ss);
	break;
    case GDK_F11:
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
	if (ss->fullscreen) {
	    gtk_window_unfullscreen (GTK_WINDOW(ss->window));
	    ss->fullscreen = false;
	} else {
	    gtk_window_fullscreen (GTK_WINDOW(ss->window));
	    ss->fullscreen = true;
	}
#else
	std::cout<<"Your GTK+ does not support fullscreen mode. Upgrade to 2.2."<<std::endl;
#endif
	break;
    case GDK_Return:
	sp_svgview_control_show (ss);
	break;
    case GDK_KP_Page_Down:
    case GDK_Page_Down:
    case GDK_Right:
    case GDK_space:
	sp_svgview_show_next (ss);
	break;
    case GDK_KP_Page_Up:
    case GDK_Page_Up:
    case GDK_Left:
    case GDK_BackSpace:
	sp_svgview_show_prev (ss);
	break;
    case GDK_Escape:
    case GDK_q:
    case GDK_Q:
	gtk_main_quit();
	break;
    default:
	break;
    }
    gtk_window_set_title(GTK_WINDOW(ss->window), SP_DOCUMENT_NAME(ss->doc));
    return TRUE;
}

int
main (int argc, const char **argv)
{
    if (argc == 1) {
	usage();
    }

    // Prevents errors like "Unable to wrap GdkPixbuf..." (in nr-filter-image.cpp for example)
    Gtk::Main::init_gtkmm_internals();

    Gtk::Main main_instance (&argc, const_cast<char ***>(&argv));

    struct SPSlideShow ss;

    int option,
        num_parsed_options = 0;

    // the list of arguments is in the net line
    while ((option = getopt(argc, (char* const* )argv, "t:")) != -1)
    {
        switch(option) {
	    case 't': // for timer
                // fprintf(stderr, "set timer arg %s\n", optarg );
	        ss.timer = atoi(optarg);
	        num_parsed_options += 2; // 2 because of flag + option
                break;
            case '?':
            default:
		usage();
        }
    }

    GtkWidget *w;
    int i;

    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    LIBXML_TEST_VERSION

    Inkscape::GC::init();
    Inkscape::Preferences::get(); // ensure preferences are initialized

    gtk_init (&argc, (char ***) &argv);

#ifdef lalaWITH_MODULES
    g_warning ("Have to autoinit modules (lauris)");
    sp_modulesys_init();
#endif /* WITH_MODULES */

    /* We must set LC_NUMERIC to default, or otherwise */
    /* we'll end with localised SVG files :-( */

    setlocale (LC_NUMERIC, "C");

    ss.size = 32;
    ss.length = 0;
    ss.current = 0;
    ss.slides = g_new (char *, ss.size);
    ss.current = 0;
    ss.doc = NULL;
    ss.view = NULL;
    ss.fullscreen = false;

    inkscape = (Inkscape::Application *)g_object_new (SP_TYPE_INKSCAPE, NULL);

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
			//::write(1, gba->data, gba->len);
			/* Append to list */
			if (ss.length >= ss.size) {
			    /* Expand */
			    ss.size <<= 1;
			    ss.slides = g_renew (char *, ss.slides, ss.size);
			}

			ss.doc = sp_document_new_from_mem ((const gchar *)gba->data,
							   gba->len,
							   TRUE);
			gchar *last_filename = jar_file_reader.get_last_filename();
			if (ss.doc) {
			    ss.slides[ss.length++] = strdup (last_filename);
			    sp_document_set_uri (ss.doc, strdup(last_filename));
			}
			g_byte_array_free(gba, TRUE);
			g_free(last_filename);
		    } else
			break;
		}
	    } else {
#endif /* WITH_INKJAR */
		/* Append to list */
		if (ss.length >= ss.size) {
		    /* Expand */
		    ss.size <<= 1;
		    ss.slides = g_renew (char *, ss.slides, ss.size);

		}

		ss.slides[ss.length++] = strdup (argv[i]);

                if (!ss.doc) {
                    ss.doc = sp_document_new (ss.slides[ss.current], TRUE, false);
                    if (!ss.doc)
                        ++ss.current;
		}
#ifdef WITH_INKJAR
	    }
#endif
	}
    }

    if(!ss.doc)
       return 1; /* none of the slides loadable */

    w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (w), SP_DOCUMENT_NAME (ss.doc));
    gtk_window_set_default_size (GTK_WINDOW (w),
				 MIN ((int)sp_document_width (ss.doc), (int)gdk_screen_width () - 64),
				 MIN ((int)sp_document_height (ss.doc), (int)gdk_screen_height () - 64));
    gtk_window_set_policy (GTK_WINDOW (w), TRUE, TRUE, FALSE);
    ss.window = w;

    g_signal_connect (G_OBJECT (w), "delete_event", (GCallback) sp_svgview_main_delete, &ss);
    g_signal_connect (G_OBJECT (w), "key_press_event", (GCallback) sp_svgview_main_key_press, &ss);

    sp_document_ensure_up_to_date (ss.doc);
    ss.view = sp_svg_view_widget_new (ss.doc);
    sp_document_unref (ss.doc);
    sp_svg_view_widget_set_resize (SP_SVG_VIEW_WIDGET (ss.view), FALSE,
                                   sp_document_width (ss.doc), sp_document_height (ss.doc));
    gtk_widget_show (ss.view);
    gtk_container_add (GTK_CONTAINER (w), ss.view);

    gtk_widget_show (w);

    gtk_main ();

    return 0;
}

static int
sp_svgview_ctrlwin_delete (GtkWidget */*widget*/, GdkEvent */*event*/, void */*data*/)
{
    ctrlwin = NULL;
    return FALSE;
}

static GtkWidget *
sp_svgview_control_show (struct SPSlideShow *ss)
{
    if (!ctrlwin) {
	GtkWidget *t, *b;
	ctrlwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for (GTK_WINDOW(ctrlwin), GTK_WINDOW(ss->window));
    g_signal_connect (G_OBJECT (ctrlwin), "key_press_event", (GCallback) sp_svgview_main_key_press, ss);
	g_signal_connect (G_OBJECT (ctrlwin), "delete_event", (GCallback) sp_svgview_ctrlwin_delete, NULL);
	t = gtk_table_new (1, 4, TRUE);
	gtk_container_add ((GtkContainer *) ctrlwin, t);
	b = gtk_button_new_from_stock (GTK_STOCK_GOTO_FIRST);
	gtk_table_attach ((GtkTable *) t, b, 0, 1, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  0, 0);
	g_signal_connect ((GObject *) b, "clicked", (GCallback) sp_svgview_goto_first_cb, ss);
	b = gtk_button_new_from_stock (GTK_STOCK_GO_BACK);
	gtk_table_attach ((GtkTable *) t, b, 1, 2, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  0, 0);
	g_signal_connect (G_OBJECT(b), "clicked", (GCallback) sp_svgview_show_prev_cb, ss);
	b = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	gtk_table_attach ((GtkTable *) t, b, 2, 3, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  0, 0);
	g_signal_connect (G_OBJECT(b), "clicked", (GCallback) sp_svgview_show_next_cb, ss);
	b = gtk_button_new_from_stock (GTK_STOCK_GOTO_LAST);
	gtk_table_attach ((GtkTable *) t, b, 3, 4, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  0, 0);
	g_signal_connect (G_OBJECT(b), "clicked", (GCallback) sp_svgview_goto_last_cb, ss);
	gtk_widget_show_all (ctrlwin);
    } else {
	gtk_window_present ((GtkWindow *) ctrlwin);
    }

    return NULL;
}

static int
sp_svgview_show_next_cb (GtkWidget */*widget*/, void *data)
{
    sp_svgview_show_next(static_cast<struct SPSlideShow *>(data));
    return FALSE;
}

static int
sp_svgview_show_prev_cb (GtkWidget */*widget*/, void *data)
{
    sp_svgview_show_prev(static_cast<struct SPSlideShow *>(data));
    return FALSE;
}

static int
sp_svgview_goto_first_cb (GtkWidget */*widget*/, void *data)
{
    sp_svgview_goto_first(static_cast<struct SPSlideShow *>(data));
    return FALSE;
}

static int
sp_svgview_goto_last_cb (GtkWidget */*widget*/, void *data)
{
    sp_svgview_goto_last(static_cast<struct SPSlideShow *>(data));
    return FALSE;
}

static void
sp_svgview_waiting_cursor(struct SPSlideShow *ss)
{
    GdkCursor *waiting = gdk_cursor_new(GDK_WATCH);
    gdk_window_set_cursor(GTK_WIDGET(ss->window)->window, waiting);
    gdk_cursor_unref(waiting);
    if (ctrlwin) {
        GdkCursor *waiting = gdk_cursor_new(GDK_WATCH);
        gdk_window_set_cursor(GTK_WIDGET(ctrlwin)->window, waiting);
        gdk_cursor_unref(waiting);
    }
    while(gtk_events_pending())
       gtk_main_iteration();
}

static void
sp_svgview_normal_cursor(struct SPSlideShow *ss)
{
   gdk_window_set_cursor(GTK_WIDGET(ss->window)->window, NULL);
    if (ctrlwin) {
        gdk_window_set_cursor(GTK_WIDGET(ctrlwin)->window, NULL);
    }
}

static void
sp_svgview_set_document(struct SPSlideShow *ss, SPDocument *doc, int current)
{
    if (doc && doc != ss->doc) {
        sp_document_ensure_up_to_date (doc);
        reinterpret_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (ss->view))->setDocument (doc);
        ss->doc = doc;
        ss->current = current;
    }
}

static void
sp_svgview_show_next (struct SPSlideShow *ss)
{
    sp_svgview_waiting_cursor(ss);

    SPDocument *doc = NULL;
    int current = ss->current;
    while (!doc && (current < ss->length - 1)) {
        doc = sp_document_new (ss->slides[++current], TRUE, false);
    }

    sp_svgview_set_document(ss, doc, current);

    sp_svgview_normal_cursor(ss);
}

static void
sp_svgview_show_prev (struct SPSlideShow *ss)
{
    sp_svgview_waiting_cursor(ss);

    SPDocument *doc = NULL;
    int current = ss->current;
    while (!doc && (current > 0)) {
        doc = sp_document_new (ss->slides[--current], TRUE, false);
    }

    sp_svgview_set_document(ss, doc, current);

    sp_svgview_normal_cursor(ss);
}

static void
sp_svgview_goto_first (struct SPSlideShow *ss)
{
    sp_svgview_waiting_cursor(ss);

    SPDocument *doc = NULL;
    int current = 0;
    while ( !doc && (current < ss->length - 1)) {
        if (current == ss->current)
            break;
        doc = sp_document_new (ss->slides[current++], TRUE, false);
    }

    sp_svgview_set_document(ss, doc, current - 1);

    sp_svgview_normal_cursor(ss);
}

static void
sp_svgview_goto_last (struct SPSlideShow *ss)
{
    sp_svgview_waiting_cursor(ss);

    SPDocument *doc = NULL;
    int current = ss->length - 1;
    while (!doc && (current >= 0)) {
        if (current == ss->current)
            break;
        doc = sp_document_new (ss->slides[current--], TRUE, false);
    }

    sp_svgview_set_document(ss, doc, current + 1);

    sp_svgview_normal_cursor(ss);
}

#ifdef WITH_INKJAR
static bool
is_jar(char const *filename)
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

#ifdef XXX
/* TODO !!! make this temporary stub unnecessary */
Inkscape::Application *inkscape_get_instance() { return NULL; }
void inkscape_ref (void) {}
void inkscape_unref (void) {}
void inkscape_add_document (SPDocument *document) {}
void inkscape_remove_document (SPDocument *document) {}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

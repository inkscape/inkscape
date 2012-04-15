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

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <cstring>
#include <sys/stat.h>
#include <locale.h>

#include <glib.h>

// #include <stropts.h>

#include <libxml/tree.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

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

#if !GTK_CHECK_VERSION(2,22,0)
#define GDK_KEY_VoidSymbol 0xffffff
#define GDK_KEY_Up 0xff52
#define GDK_KEY_KP_Up 0xff97
#define GDK_KEY_Down 0xff54
#define GDK_KEY_KP_Down 0xff99
#define GDK_KEY_Left 0xff51
#define GDK_KEY_KP_Left 0xff96
#define GDK_KEY_Right 0xff53
#define GDK_KEY_KP_Right 0xff98
#define GDK_KEY_Page_Up 0xff55
#define GDK_KEY_KP_Page_Up 0xff9a
#define GDK_KEY_Page_Down 0xff56
#define GDK_KEY_KP_Page_Down 0xff9b
#define GDK_KEY_Home 0xff50
#define GDK_KEY_KP_Home 0xff95
#define GDK_KEY_End 0xff57
#define GDK_KEY_KP_End 0xff9c
#define GDK_KEY_a 0x061
#define GDK_KEY_A 0x041
#define GDK_KEY_b 0x062
#define GDK_KEY_B 0x042
#define GDK_KEY_c 0x063
#define GDK_KEY_C 0x043
#define GDK_KEY_d 0x064
#define GDK_KEY_D 0x044
#define GDK_KEY_g 0x067
#define GDK_KEY_G 0x047
#define GDK_KEY_h 0x068
#define GDK_KEY_H 0x048
#define GDK_KEY_i 0x069
#define GDK_KEY_I 0x049
#define GDK_KEY_j 0x06a
#define GDK_KEY_J 0x04a
#define GDK_KEY_k 0x06b
#define GDK_KEY_K 0x04b
#define GDK_KEY_l 0x06c
#define GDK_KEY_L 0x04c
#define GDK_KEY_q 0x071
#define GDK_KEY_Q 0x051
#define GDK_KEY_r 0x072
#define GDK_KEY_R 0x052
#define GDK_KEY_s 0x073
#define GDK_KEY_S 0x053
#define GDK_KEY_u 0x075
#define GDK_KEY_U 0x055
#define GDK_KEY_v 0x076
#define GDK_KEY_V 0x056
#define GDK_KEY_w 0x077
#define GDK_KEY_W 0x057
#define GDK_KEY_x 0x078
#define GDK_KEY_X 0x058
#define GDK_KEY_y 0x079
#define GDK_KEY_Y 0x059
#define GDK_KEY_z 0x07a
#define GDK_KEY_Z 0x05a
#define GDK_KEY_Escape 0xff1b
#define GDK_KEY_Control_L 0xffe3
#define GDK_KEY_Control_R 0xffe4
#define GDK_KEY_Alt_L 0xffe9
#define GDK_KEY_Alt_R 0xffea
#define GDK_KEY_Shift_L 0xffe1
#define GDK_KEY_Shift_R 0xffe2
#define GDK_KEY_Meta_L 0xffe7
#define GDK_KEY_Meta_R 0xffe8
#define GDK_KEY_KP_0 0xffb0
#define GDK_KEY_KP_1 0xffb1
#define GDK_KEY_KP_2 0xffb2
#define GDK_KEY_KP_3 0xffb3
#define GDK_KEY_KP_4 0xffb4
#define GDK_KEY_KP_5 0xffb5
#define GDK_KEY_KP_6 0xffb6
#define GDK_KEY_KP_7 0xffb7
#define GDK_KEY_KP_8 0xffb8
#define GDK_KEY_KP_9 0xffb9
#define GDK_KEY_F1 0xffbe
#define GDK_KEY_F2 0xffbf
#define GDK_KEY_F3 0xffc0
#define GDK_KEY_F4 0xffc1
#define GDK_KEY_F5 0xffc2
#define GDK_KEY_F6 0xffc3
#define GDK_KEY_F7 0xffc4
#define GDK_KEY_F8 0xffc5
#define GDK_KEY_F9 0xffc6
#define GDK_KEY_F10 0xffc7
#define GDK_KEY_F11 0xffc8
#define GDK_KEY_Insert 0xff63
#define GDK_KEY_KP_Insert 0xff9e
#define GDK_KEY_Delete 0xffff
#define GDK_KEY_KP_Delete 0xff9f
#define GDK_KEY_BackSpace 0xff08
#define GDK_KEY_Return 0xff0d
#define GDK_KEY_KP_Enter 0xff8d
#define GDK_KEY_space 0x020
#define GDK_KEY_KP_Space 0xff80
#define GDK_KEY_Tab 0xff09
#define GDK_KEY_ISO_Left_Tab 0xfe20
#define GDK_KEY_bracketleft 0x05b
#define GDK_KEY_bracketright 0x05d
#define GDK_KEY_braceleft 0x07b
#define GDK_KEY_braceright 0x07d
#define GDK_KEY_less 0x03c
#define GDK_KEY_greater 0x03e
#define GDK_KEY_comma 0x02c
#define GDK_KEY_period 0x02e
#define GDK_KEY_KP_Add 0xffab
#define GDK_KEY_KP_Subtract 0xffad
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

// Dummy functions to keep linker happy
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
    case GDK_KEY_Up:
    case GDK_KEY_Home:
	sp_svgview_goto_first(ss);
	break;
    case GDK_KEY_Down:
    case GDK_KEY_End:
	sp_svgview_goto_last(ss);
	break;
    case GDK_KEY_F11:
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
    case GDK_KEY_Return:
	sp_svgview_control_show (ss);
	break;
    case GDK_KEY_KP_Page_Down:
    case GDK_KEY_Page_Down:
    case GDK_KEY_Right:
    case GDK_KEY_space:
	sp_svgview_show_next (ss);
	break;
    case GDK_KEY_KP_Page_Up:
    case GDK_KEY_Page_Up:
    case GDK_KEY_Left:
    case GDK_KEY_BackSpace:
	sp_svgview_show_prev (ss);
	break;
    case GDK_KEY_Escape:
    case GDK_KEY_q:
    case GDK_KEY_Q:
	gtk_main_quit();
	break;
    default:
	break;
    }
    gtk_window_set_title(GTK_WINDOW(ss->window), ss->doc->getName());
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

			ss.doc = SPDocument::createNewDocFromMem ((const gchar *)gba->data,
							   gba->len,
							   TRUE);
			gchar *last_filename = jar_file_reader.get_last_filename();
			if (ss.doc) {
			    ss.slides[ss.length++] = strdup (last_filename);
			    (ss.doc)->setUri (strdup(last_filename));
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
                    ss.doc = SPDocument::createNewDoc (ss.slides[ss.current], TRUE, false);
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
    gtk_window_set_title( GTK_WINDOW(w), ss.doc->getName() );
    gtk_window_set_default_size (GTK_WINDOW (w),
				 MIN ((int)(ss.doc)->getWidth (), (int)gdk_screen_width () - 64),
				 MIN ((int)(ss.doc)->getHeight (), (int)gdk_screen_height () - 64));
    ss.window = w;

    g_signal_connect (G_OBJECT (w), "delete_event", (GCallback) sp_svgview_main_delete, &ss);
    g_signal_connect (G_OBJECT (w), "key_press_event", (GCallback) sp_svgview_main_key_press, &ss);

    (ss.doc)->ensureUpToDate();
    ss.view = sp_svg_view_widget_new (ss.doc);
    (ss.doc)->doUnref ();
    SP_SVG_VIEW_WIDGET(ss.view)->setResize( false, ss.doc->getWidth(), ss.doc->getHeight() );
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
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(ss->window)), waiting);
#if GTK_CHECK_VERSION(3,0,0)
    g_object_unref(waiting);
#else
    gdk_cursor_unref(waiting);
#endif
    if (ctrlwin) {
        GdkCursor *waiting = gdk_cursor_new(GDK_WATCH);
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(ctrlwin)), waiting);
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(waiting);
#else
        gdk_cursor_unref(waiting);
#endif
    }
    while(gtk_events_pending())
       gtk_main_iteration();
}

static void
sp_svgview_normal_cursor(struct SPSlideShow *ss)
{
   gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(ss->window)), NULL);
    if (ctrlwin) {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(ctrlwin)), NULL);
    }
}

static void
sp_svgview_set_document(struct SPSlideShow *ss, SPDocument *doc, int current)
{
    if (doc && doc != ss->doc) {
        doc->ensureUpToDate();
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
        doc = SPDocument::createNewDoc (ss->slides[++current], TRUE, false);
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
        doc = SPDocument::createNewDoc (ss->slides[--current], TRUE, false);
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
        doc = SPDocument::createNewDoc (ss->slides[current++], TRUE, false);
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
        doc = SPDocument::createNewDoc (ss->slides[current--], TRUE, false);
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

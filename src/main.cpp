#define __MAIN_C__

/** \file
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
 *   Bryce Harrington <bryce@bryceharrington.com>
 * ... and various people who have worked with various projects
 *
 * Copyright (C) 1999-2004 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "path-prefix.h"

#include <gtk/gtkmessagedialog.h>

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#include <cstring>
#include <string>
#include <locale.h>

#include <popt.h>
#ifndef POPT_TABLEEND
#define POPT_TABLEEND { NULL, '\0', 0, 0, 0, NULL, NULL }
#endif /* Not def: POPT_TABLEEND */

#include <libxml/tree.h>
#include <glib-object.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkbox.h>

#include "gc-core.h"

#include "macros.h"
#include "file.h"
#include "document.h"
#include "sp-object.h"
#include "interface.h"
#include "print.h"
#include "color.h"
#include "sp-item.h"
#include "sp-root.h"
#include "unit-constants.h"

#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/stringstream.h"

#include "inkscape-private.h"
#include "inkscape-stock.h"
#include "inkscape_version.h"

#include "sp-namedview.h"
#include "sp-guide.h"
#include "sp-object-repr.h"
#include "xml/repr.h"

#include "io/sys.h"

#include "debug/logger.h"
#include "debug/log-display-config.h"

#include "helper/png-write.h"

#include <extension/extension.h>
#include <extension/system.h>
#include <extension/db.h>
#include <extension/output.h>

#ifdef WIN32
//#define REPLACEARGS_ANSI
//#define REPLACEARGS_DEBUG

#include "registrytool.h"

#include "extension/internal/win32.h"
using Inkscape::Extension::Internal::PrintWin32;

#endif // WIN32

#include "extension/init.h"

#include <glibmm/i18n.h>
#include <gtkmm/main.h>

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif

#include "application/application.h"

#include "main-cmdlineact.h"

#include <png.h>
#include <errno.h>

enum {
    SP_ARG_NONE,
    SP_ARG_NOGUI,
    SP_ARG_GUI,
    SP_ARG_FILE,
    SP_ARG_PRINT,
    SP_ARG_EXPORT_PNG,
    SP_ARG_EXPORT_DPI,
    SP_ARG_EXPORT_AREA,
    SP_ARG_EXPORT_AREA_DRAWING,
    SP_ARG_EXPORT_AREA_CANVAS,
    SP_ARG_EXPORT_AREA_SNAP,
    SP_ARG_EXPORT_WIDTH,
    SP_ARG_EXPORT_HEIGHT,
    SP_ARG_EXPORT_ID,
    SP_ARG_EXPORT_ID_ONLY,
    SP_ARG_EXPORT_USE_HINTS,
    SP_ARG_EXPORT_BACKGROUND,
    SP_ARG_EXPORT_BACKGROUND_OPACITY,
    SP_ARG_EXPORT_SVG,
    SP_ARG_EXPORT_PS,
    SP_ARG_EXPORT_EPS,
    SP_ARG_EXPORT_PDF,
#ifdef WIN32
    SP_ARG_EXPORT_EMF,
#endif //WIN32
    SP_ARG_EXPORT_TEXT_TO_PATH,
    SP_ARG_EXPORT_FONT,
    SP_ARG_EXPORT_BBOX_PAGE,
    SP_ARG_EXTENSIONDIR,
    SP_ARG_FIT_PAGE_TO_DRAWING,
    SP_ARG_QUERY_X,
    SP_ARG_QUERY_Y,
    SP_ARG_QUERY_WIDTH,
    SP_ARG_QUERY_HEIGHT,
    SP_ARG_QUERY_ALL,
    SP_ARG_QUERY_ID,
    SP_ARG_VERSION,
    SP_ARG_VACUUM_DEFS,
    SP_ARG_VERB_LIST,
    SP_ARG_VERB,
    SP_ARG_SELECT,
    SP_ARG_LAST
};

int sp_main_gui(int argc, char const **argv);
int sp_main_console(int argc, char const **argv);
static void sp_do_export_png(SPDocument *doc);
static void do_export_ps(SPDocument* doc, gchar const* uri, char const *mime);
static void do_export_pdf(SPDocument* doc, gchar const* uri, char const *mime);
#ifdef WIN32
static void do_export_emf(SPDocument* doc, gchar const* uri, char const *mime);
#endif //WIN32
static void do_query_dimension (SPDocument *doc, bool extent, NR::Dim2 const axis, const gchar *id);
static void do_query_all (SPDocument *doc);
static void do_query_all_recurse (SPObject *o);

static gchar *sp_global_printer = NULL;
static gchar *sp_export_png = NULL;
static gchar *sp_export_dpi = NULL;
static gchar *sp_export_area = NULL;
static gboolean sp_export_area_drawing = FALSE;
static gboolean sp_export_area_canvas = FALSE;
static gchar *sp_export_width = NULL;
static gchar *sp_export_height = NULL;
static gchar *sp_export_id = NULL;
static gchar *sp_export_background = NULL;
static gchar *sp_export_background_opacity = NULL;
static gboolean sp_export_area_snap = FALSE;
static gboolean sp_export_use_hints = FALSE;
static gboolean sp_export_id_only = FALSE;
static gchar *sp_export_svg = NULL;
static gchar *sp_export_ps = NULL;
static gchar *sp_export_eps = NULL;
static gchar *sp_export_pdf = NULL;
#ifdef WIN32
static gchar *sp_export_emf = NULL;
#endif //WIN32
static gboolean sp_export_text_to_path = FALSE;
static gboolean sp_export_font = FALSE;
static gboolean sp_export_bbox_page = FALSE;
static gboolean sp_query_x = FALSE;
static gboolean sp_query_y = FALSE;
static gboolean sp_query_width = FALSE;
static gboolean sp_query_height = FALSE;
static gboolean sp_query_all = FALSE;
static gchar *sp_query_id = NULL;
static int sp_new_gui = FALSE;
static gboolean sp_vacuum_defs = FALSE;

static gchar *sp_export_png_utf8 = NULL;
static gchar *sp_export_svg_utf8 = NULL;
static gchar *sp_global_printer_utf8 = NULL;

#ifdef WIN32
static bool replaceArgs( int& argc, char**& argv );
#endif
static GSList *sp_process_args(poptContext ctx);
struct poptOption options[] = {
    {"version", 'V',
     POPT_ARG_NONE, NULL, SP_ARG_VERSION,
     N_("Print the Inkscape version number"),
     NULL},

    {"without-gui", 'z',
     POPT_ARG_NONE, NULL, SP_ARG_NOGUI,
     N_("Do not use X server (only process files from console)"),
     NULL},

    {"with-gui", 'g',
     POPT_ARG_NONE, NULL, SP_ARG_GUI,
     N_("Try to use X server (even if $DISPLAY is not set)"),
     NULL},

    {"file", 'f',
     POPT_ARG_STRING, NULL, SP_ARG_FILE,
     N_("Open specified document(s) (option string may be excluded)"),
     N_("FILENAME")},

    {"print", 'p',
     POPT_ARG_STRING, &sp_global_printer, SP_ARG_PRINT,
     N_("Print document(s) to specified output file (use '| program' for pipe)"),
     N_("FILENAME")},

    {"export-png", 'e',
     POPT_ARG_STRING, &sp_export_png, SP_ARG_EXPORT_PNG,
     N_("Export document to a PNG file"),
     N_("FILENAME")},

    {"export-dpi", 'd',
     POPT_ARG_STRING, &sp_export_dpi, SP_ARG_EXPORT_DPI,
     N_("The resolution used for exporting SVG into bitmap (default 90)"),
     N_("DPI")},

    {"export-area", 'a',
     POPT_ARG_STRING, &sp_export_area, SP_ARG_EXPORT_AREA,
     N_("Exported area in SVG user units (default is the canvas; 0,0 is lower-left corner)"),
     N_("x0:y0:x1:y1")},

    {"export-area-drawing", 'D',
     POPT_ARG_NONE, &sp_export_area_drawing, SP_ARG_EXPORT_AREA_DRAWING,
     N_("Exported area is the entire drawing (not canvas)"),
     NULL},

    {"export-area-canvas", 'C',
     POPT_ARG_NONE, &sp_export_area_canvas, SP_ARG_EXPORT_AREA_CANVAS,
     N_("Exported area is the entire canvas"),
     NULL},

    {"export-area-snap", 0,
     POPT_ARG_NONE, &sp_export_area_snap, SP_ARG_EXPORT_AREA_SNAP,
     N_("Snap the bitmap export area outwards to the nearest integer values (in SVG user units)"),
     NULL},

    {"export-width", 'w',
     POPT_ARG_STRING, &sp_export_width, SP_ARG_EXPORT_WIDTH,
     N_("The width of exported bitmap in pixels (overrides export-dpi)"),
     N_("WIDTH")},

    {"export-height", 'h',
     POPT_ARG_STRING, &sp_export_height, SP_ARG_EXPORT_HEIGHT,
     N_("The height of exported bitmap in pixels (overrides export-dpi)"),
     N_("HEIGHT")},

    {"export-id", 'i',
     POPT_ARG_STRING, &sp_export_id, SP_ARG_EXPORT_ID,
     N_("The ID of the object to export"),
     N_("ID")},

    {"export-id-only", 'j',
     POPT_ARG_NONE, &sp_export_id_only, SP_ARG_EXPORT_ID_ONLY,
     // TRANSLATORS: this means: "Only export the object whose id is given in --export-id".
     //  See "man inkscape" for details.
     N_("Export just the object with export-id, hide all others (only with export-id)"),
     NULL},

    {"export-use-hints", 't',
     POPT_ARG_NONE, &sp_export_use_hints, SP_ARG_EXPORT_USE_HINTS,
     N_("Use stored filename and DPI hints when exporting (only with export-id)"),
     NULL},

    {"export-background", 'b',
     POPT_ARG_STRING, &sp_export_background, SP_ARG_EXPORT_BACKGROUND,
     N_("Background color of exported bitmap (any SVG-supported color string)"),
     N_("COLOR")},

    {"export-background-opacity", 'y',
     POPT_ARG_STRING, &sp_export_background_opacity, SP_ARG_EXPORT_BACKGROUND_OPACITY,
     N_("Background opacity of exported bitmap (either 0.0 to 1.0, or 1 to 255)"),
     N_("VALUE")},

    {"export-plain-svg", 'l',
     POPT_ARG_STRING, &sp_export_svg, SP_ARG_EXPORT_SVG,
     N_("Export document to plain SVG file (no sodipodi or inkscape namespaces)"),
     N_("FILENAME")},

    {"export-ps", 'P',
     POPT_ARG_STRING, &sp_export_ps, SP_ARG_EXPORT_PS,
     N_("Export document to a PS file"),
     N_("FILENAME")},

    {"export-eps", 'E',
     POPT_ARG_STRING, &sp_export_eps, SP_ARG_EXPORT_EPS,
     N_("Export document to an EPS file"),
     N_("FILENAME")},

    {"export-pdf", 'A',
     POPT_ARG_STRING, &sp_export_pdf, SP_ARG_EXPORT_PDF,
     N_("Export document to a PDF file"),
     N_("FILENAME")},

#ifdef WIN32
    {"export-emf", 'M',
     POPT_ARG_STRING, &sp_export_emf, SP_ARG_EXPORT_EMF,
     N_("Export document to an Enhanced Metafile (EMF) File"),
     N_("FILENAME")},
#endif //WIN32

    {"export-text-to-path", 'T',
     POPT_ARG_NONE, &sp_export_text_to_path, SP_ARG_EXPORT_TEXT_TO_PATH,
     N_("Convert text object to paths on export (EPS)"),
     NULL},

    {"export-embed-fonts", 'F',
     POPT_ARG_NONE, &sp_export_font, SP_ARG_EXPORT_FONT,
     N_("Embed fonts on export (Type 1 only) (EPS)"),
     NULL},

    {"export-bbox-page", 'B',
     POPT_ARG_NONE, &sp_export_bbox_page, SP_ARG_EXPORT_BBOX_PAGE,
     N_("Export files with the bounding box set to the page size (EPS)"),
     NULL},

    {"query-x", 'X',
     POPT_ARG_NONE, &sp_query_x, SP_ARG_QUERY_X,
     // TRANSLATORS: "--query-id" is an Inkscape command line option; see "inkscape --help"
     N_("Query the X coordinate of the drawing or, if specified, of the object with --query-id"),
     NULL},

    {"query-y", 'Y',
     POPT_ARG_NONE, &sp_query_y, SP_ARG_QUERY_Y,
     // TRANSLATORS: "--query-id" is an Inkscape command line option; see "inkscape --help"
     N_("Query the Y coordinate of the drawing or, if specified, of the object with --query-id"),
     NULL},

    {"query-width", 'W',
     POPT_ARG_NONE, &sp_query_width, SP_ARG_QUERY_WIDTH,
     // TRANSLATORS: "--query-id" is an Inkscape command line option; see "inkscape --help"
     N_("Query the width of the drawing or, if specified, of the object with --query-id"),
     NULL},

    {"query-height", 'H',
     POPT_ARG_NONE, &sp_query_height, SP_ARG_QUERY_HEIGHT,
     // TRANSLATORS: "--query-id" is an Inkscape command line option; see "inkscape --help"
     N_("Query the height of the drawing or, if specified, of the object with --query-id"),
     NULL},

    {"query-all", 'S',
     POPT_ARG_NONE, &sp_query_all, SP_ARG_QUERY_ALL,
     N_("List id,x,y,w,h for all objects"),
     NULL},

    {"query-id", 'I',
     POPT_ARG_STRING, &sp_query_id, SP_ARG_QUERY_ID,
     N_("The ID of the object whose dimensions are queried"),
     N_("ID")},

    {"extension-directory", 'x',
     POPT_ARG_NONE, NULL, SP_ARG_EXTENSIONDIR,
     // TRANSLATORS: this option makes Inkscape print the name (path) of the extension directory
     N_("Print out the extension directory and exit"),
     NULL},

    {"vacuum-defs", 0,
     POPT_ARG_NONE, &sp_vacuum_defs, SP_ARG_VACUUM_DEFS,
     N_("Remove unused definitions from the defs section(s) of the document"),
     NULL},

    {"verb-list", 0,
     POPT_ARG_NONE, NULL, SP_ARG_VERB_LIST,
     N_("List the IDs of all the verbs in Inkscape"),
     NULL},

    {"verb", 0,
     POPT_ARG_STRING, NULL, SP_ARG_VERB,
     N_("Verb to call when Inkscape opens."),
     N_("VERB-ID")},

    {"select", 0,
     POPT_ARG_STRING, NULL, SP_ARG_SELECT,
     N_("Object ID to select when Inkscape opens."),
     N_("OBJECT-ID")},

    POPT_AUTOHELP POPT_TABLEEND
};

static bool needToRecodeParams = true;
gchar* blankParam = "";

#ifdef WIN32
static int _win32_set_inkscape_env(char *argv0)
{
    CHAR szFullPath[_MAX_PATH];

    CHAR szDrive[_MAX_DRIVE];
    CHAR szDir[_MAX_DIR];
    CHAR szFile[_MAX_FNAME];
    CHAR szExt[_MAX_EXT];

    CHAR tmp[_MAX_EXT];

    if (GetModuleFileName(NULL, szFullPath, sizeof(szFullPath)) == 0) {
        strcpy(szFullPath, argv0);
    }

    _splitpath(szFullPath, szDrive, szDir, szFile, szExt);
    strcpy(szFullPath, szDrive);
    strcat(szFullPath, szDir);

    char *oldenv = getenv("PATH");
	strcpy(tmp, "PATH=");
    strcat(tmp, szFullPath);
    strcat(tmp, ";");
    strcat(tmp, szFullPath);
    strcat(tmp, "python;");
    strcat(tmp, szFullPath);
    strcat(tmp, "perl;");
    if(oldenv != NULL) {
		strcat(tmp, ";");
		strcat(tmp, oldenv);
	}
	_putenv(tmp);

    oldenv = getenv("PYTHONPATH");
    strcpy(tmp, "PYTHONPATH=");
    strcat(tmp, szFullPath);
    strcat(tmp, "python;");
    strcat(tmp, szFullPath);
    strcat(tmp, "python\\Lib;");
    strcat(tmp, szFullPath);
    strcat(tmp, "python\\DLLs");
    if(oldenv != NULL) {
		strcat(tmp, ";");
		strcat(tmp, oldenv);
	}
	_putenv(tmp);

    return 0;
}
#endif

int
main(int argc, char **argv)
{
#ifdef HAVE_FPSETMASK
    /* This is inherited from Sodipodi code, where it was in #ifdef __FreeBSD__.  It's probably
       safe to remove: the default mask is already 0 in C99, and in current FreeBSD according to
       the fenv man page on www.freebsd.org, and in glibc according to (libc)FP Exceptions. */
    fpsetmask(fpgetmask() & ~(FP_X_DZ | FP_X_INV));
#endif

#ifdef ENABLE_NLS
#ifdef WIN32
	_win32_set_inkscape_env(argv[0]);
    RegistryTool rt;
    rt.setPathInfo();
    gchar *pathBuf = g_strconcat(g_path_get_dirname(argv[0]), "\\", PACKAGE_LOCALE_DIR, NULL);
    bindtextdomain(GETTEXT_PACKAGE, pathBuf);
    g_free(pathBuf);
#else
#ifdef ENABLE_BINRELOC
    bindtextdomain(GETTEXT_PACKAGE, BR_LOCALEDIR(""));
#else
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#endif
#endif
    // Allow the user to override the locale directory by setting
    // the environment variable INKSCAPE_LOCALEDIR.
    char *inkscape_localedir = getenv("INKSCAPE_LOCALEDIR");
    if (inkscape_localedir != NULL) {
        bindtextdomain(GETTEXT_PACKAGE, inkscape_localedir);
    }
#endif

    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

#ifdef ENABLE_NLS
    textdomain(GETTEXT_PACKAGE);
#endif

    LIBXML_TEST_VERSION

    Inkscape::GC::init();

    Inkscape::Debug::Logger::init();

    gboolean use_gui;
#ifndef WIN32
    use_gui = (getenv("DISPLAY") != NULL);
#else
    /*
      Set the current directory to the directory of the
      executable.  This seems redundant, but is needed for
      when inkscape.exe is executed from another directory.
      We use relative paths on win32.
      HKCR\svgfile\shell\open\command is a good example
    */
    /// \todo FIXME BROKEN - non-UTF-8 sneaks in here.
    char *homedir = g_path_get_dirname(argv[0]);
    SetCurrentDirectory(homedir);
    g_free(homedir);

    use_gui = TRUE;
#endif
    /* Test whether with/without GUI is forced */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-z")
            || !strcmp(argv[i], "--without-gui")
            || !strcmp(argv[i], "-p")
            || !strncmp(argv[i], "--print", 7)
            || !strcmp(argv[i], "-e")
            || !strncmp(argv[i], "--export-png", 12)
            || !strcmp(argv[i], "-l")
            || !strncmp(argv[i], "--export-plain-svg", 12)
            || !strcmp(argv[i], "-i")
            || !strncmp(argv[i], "--export-area-drawing", 21)
            || !strcmp(argv[i], "-D")
            || !strncmp(argv[i], "--export-area-canvas", 20)
            || !strcmp(argv[i], "-C")
            || !strncmp(argv[i], "--export-id", 12)
            || !strcmp(argv[i], "-P")
            || !strncmp(argv[i], "--export-ps", 11)
            || !strcmp(argv[i], "-E")
            || !strncmp(argv[i], "--export-eps", 12)
            || !strcmp(argv[i], "-A")
            || !strncmp(argv[i], "--export-pdf", 12)
#ifdef WIN32
            || !strcmp(argv[i], "-M")
            || !strncmp(argv[i], "--export-emf", 12)
#endif //WIN32
            || !strcmp(argv[i], "-W")
            || !strncmp(argv[i], "--query-width", 13)
            || !strcmp(argv[i], "-H")
            || !strncmp(argv[i], "--query-height", 14)
            || !strcmp(argv[i], "-S")
            || !strncmp(argv[i], "--query-all", 11)
            || !strcmp(argv[i], "-X")
            || !strncmp(argv[i], "--query-x", 13)
            || !strcmp(argv[i], "-Y")
            || !strncmp(argv[i], "--query-y", 14)
            || !strcmp(argv[i], "--vacuum-defs")
           )
        {
            /* main_console handles any exports -- not the gui */
            use_gui = FALSE;
            break;
        } else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--with-gui")) {
            use_gui = TRUE;
            break;
        }
    }

#ifdef WIN32
#ifndef REPLACEARGS_ANSI
    if ( PrintWin32::is_os_wide() )
#endif // REPLACEARGS_ANSI
    {
        // If the call fails, we'll need to convert charsets
        needToRecodeParams = !replaceArgs( argc, argv );
    }
#endif // WIN32

    /// \todo  Should this be a static object (see inkscape.cpp)?
    Inkscape::NSApplication::Application app(argc, argv, use_gui, sp_new_gui);

    return app.run();
}

void fixupSingleFilename( gchar **orig, gchar **spare )
{
    if ( orig && *orig && **orig ) {
        GError *error = NULL;
        gchar *newFileName = Inkscape::IO::locale_to_utf8_fallback(*orig, -1, NULL, NULL, &error);
        if ( newFileName )
        {
            *orig = newFileName;
            if ( spare ) {
                *spare = newFileName;
            }
//             g_message("Set a replacement fixup");
        }
    }
}

GSList *fixupFilenameEncoding( GSList* fl )
{
    GSList *newFl = NULL;
    while ( fl ) {
        gchar *fn = static_cast<gchar*>(fl->data);
        fl = g_slist_remove( fl, fl->data );
        gchar *newFileName = Inkscape::IO::locale_to_utf8_fallback(fn, -1, NULL, NULL, NULL);
        if ( newFileName ) {

            if ( 0 )
            {
                gchar *safeFn = Inkscape::IO::sanitizeString(fn);
                gchar *safeNewFn = Inkscape::IO::sanitizeString(newFileName);
                GtkWidget *w = gtk_message_dialog_new( NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                                       "Note: Converted '%s' to '%s'", safeFn, safeNewFn );
                gtk_dialog_run (GTK_DIALOG (w));
                gtk_widget_destroy (w);
                g_free(safeNewFn);
                g_free(safeFn);
            }

            g_free( fn );
            fn = newFileName;
            newFileName = 0;
        }
        else
            if ( 0 )
        {
            gchar *safeFn = Inkscape::IO::sanitizeString(fn);
            GtkWidget *w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Error: Unable to convert '%s'", safeFn );
            gtk_dialog_run (GTK_DIALOG (w));
            gtk_widget_destroy (w);
            g_free(safeFn);
        }
        newFl = g_slist_append( newFl, fn );
    }
    return newFl;
}

int sp_common_main( int argc, char const **argv, GSList **flDest )
{
    /// \todo fixme: Move these to some centralized location (Lauris)
    sp_object_type_register("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
    sp_object_type_register("sodipodi:guide", SP_TYPE_GUIDE);


    // temporarily switch gettext encoding to locale, so that help messages can be output properly
    gchar const *charset;
    g_get_charset(&charset);

    bind_textdomain_codeset(GETTEXT_PACKAGE, charset);

    poptContext ctx = poptGetContext(NULL, argc, argv, options, 0);
    poptSetOtherOptionHelp(ctx, _("[OPTIONS...] [FILE...]\n\nAvailable options:"));
    g_return_val_if_fail(ctx != NULL, 1);

    /* Collect own arguments */
    GSList *fl = sp_process_args(ctx);
    poptFreeContext(ctx);

    // now switch gettext back to UTF-8 (for GUI)
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    // Now let's see if the file list still holds up
    if ( needToRecodeParams )
    {
        fl = fixupFilenameEncoding( fl );
    }

    // Check the globals for filename-fixup
    if ( needToRecodeParams )
    {
        fixupSingleFilename( &sp_export_png, &sp_export_png_utf8 );
        fixupSingleFilename( &sp_export_svg, &sp_export_svg_utf8 );
        fixupSingleFilename( &sp_global_printer, &sp_global_printer_utf8 );
    }
    else
    {
        if ( sp_export_png )
            sp_export_png_utf8 = g_strdup( sp_export_png );
        if ( sp_export_svg )
            sp_export_svg_utf8 = g_strdup( sp_export_svg );
        if ( sp_global_printer )
            sp_global_printer_utf8 = g_strdup( sp_global_printer );
    }

    // Return the list if wanted, else free it up.
    if ( flDest ) {
        *flDest = fl;
        fl = 0;
    } else {
        while ( fl ) {
            g_free( fl->data );
            fl = g_slist_remove( fl, fl->data );
        }
    }
    return 0;
}

static void
snooper(GdkEvent *event, gpointer /*data*/) {
    if(inkscape_mapalt())  /* returns the map of the keyboard modifier to map to Alt, zero if no mapping */
    {
        GdkModifierType mapping=(GdkModifierType)inkscape_mapalt();
        switch (event->type) {
            case GDK_MOTION_NOTIFY:
                if(event->motion.state & mapping) {
                    event->motion.state|=GDK_MOD1_MASK;
                }
                break;
            case GDK_BUTTON_PRESS:
                if(event->button.state & mapping) {
                    event->button.state|=GDK_MOD1_MASK;
                }
                break;
             case GDK_KEY_PRESS:
                 if(event->key.state & mapping) {
                     event->key.state|=GDK_MOD1_MASK;
                 }
                 break;
        default:
            break;
        }
    }
    gtk_main_do_event (event);
}

int
sp_main_gui(int argc, char const **argv)
{
    Gtk::Main main_instance (&argc, const_cast<char ***>(&argv));

    GSList *fl = NULL;
    int retVal = sp_common_main( argc, argv, &fl );
    g_return_val_if_fail(retVal == 0, 1);

    inkscape_gtk_stock_init();

    gdk_event_handler_set((GdkEventFunc)snooper, NULL, NULL);

    Inkscape::Debug::log_display_config();

    /* Set default icon */
    gchar *filename = (gchar *) g_build_filename (INKSCAPE_APPICONDIR, "inkscape.png", NULL);
    if (Inkscape::IO::file_test(filename, (GFileTest)(G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK))) {
        gtk_window_set_default_icon_from_file(filename, NULL);
    }
    g_free (filename);
    filename = 0;

    gboolean create_new = TRUE;

    /// \todo FIXME BROKEN - non-UTF-8 sneaks in here.
    inkscape_application_init(argv[0], true);

    while (fl) {
        if (sp_file_open((gchar *)fl->data,NULL)) {
            create_new=FALSE;
        }
        fl = g_slist_remove(fl, fl->data);
    }
    if (create_new) {
        sp_file_new_default();
    }

    Glib::signal_idle().connect(sigc::ptr_fun(&Inkscape::CmdLineAction::idle));
    main_instance.run();

#ifdef WIN32
    //We might not need anything here
    //sp_win32_finish(); <-- this is a NOP func
#endif

    return 0;
}

int
sp_main_console(int argc, char const **argv)
{
    /* We are started in text mode */

    /* Do this g_type_init(), so that we can use Xft/Freetype2 (Pango)
     * in a non-Gtk environment.  Used in libnrtype's
     * FontInstance.cpp and FontFactory.cpp.
     * http://mail.gnome.org/archives/gtk-list/2003-December/msg00063.html
     */
    g_type_init();
    char **argv2 = const_cast<char **>(argv);
    gtk_init_check( &argc, &argv2 );
    //setlocale(LC_ALL, "");

    GSList *fl = NULL;
    int retVal = sp_common_main( argc, argv, &fl );
    g_return_val_if_fail(retVal == 0, 1);

    if (fl == NULL) {
        g_print("Nothing to do!\n");
        exit(0);
    }

    inkscape_application_init(argv[0], false);

    while (fl) {
        SPDocument *doc;

        doc = Inkscape::Extension::open(NULL, (gchar *)fl->data);
        if (doc == NULL) {
            doc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), (gchar *)fl->data);
        }
        if (doc == NULL) {
            g_warning("Specified document %s cannot be opened (is it valid SVG file?)", (gchar *) fl->data);
        } else {
            if (sp_vacuum_defs) {
                vacuum_document(doc);
            }
            if (sp_vacuum_defs && !sp_export_svg) {
                // save under the name given in the command line
                sp_repr_save_file(doc->rdoc, (gchar *)fl->data, SP_SVG_NS_URI);
            }
            if (sp_global_printer) {
                sp_print_document_to_file(doc, sp_global_printer);
            }
            if (sp_export_png) {
                sp_do_export_png(doc);
            }
            if (sp_export_svg) {
                Inkscape::XML::Document *rdoc;
                Inkscape::XML::Node *repr;
                rdoc = sp_repr_document_new("svg:svg");
                repr = rdoc->root();
                repr = sp_document_root(doc)->updateRepr(repr, SP_OBJECT_WRITE_BUILD);
                sp_repr_save_file(repr->document(), sp_export_svg, SP_SVG_NS_URI);
            }
            if (sp_export_ps) {
                do_export_ps(doc, sp_export_ps, "image/x-postscript");
            }
            if (sp_export_eps) {
                do_export_ps(doc, sp_export_eps, "image/x-e-postscript");
            }
            if (sp_export_pdf) {
                do_export_pdf(doc, sp_export_pdf, "application/pdf");
            }
#ifdef WIN32
            if (sp_export_emf) {
                do_export_emf(doc, sp_export_emf, "image/x-emf");
            }
#endif //WIN32
            if (sp_query_all) {
                do_query_all (doc);
            } else if (sp_query_width || sp_query_height) {
                do_query_dimension (doc, true, sp_query_width? NR::X : NR::Y, sp_query_id);
            } else if (sp_query_x || sp_query_y) {
                do_query_dimension (doc, false, sp_query_x? NR::X : NR::Y, sp_query_id);
            }
        }

        fl = g_slist_remove(fl, fl->data);
    }

    inkscape_unref();

    return 0;
}

static void
do_query_dimension (SPDocument *doc, bool extent, NR::Dim2 const axis, const gchar *id)
{
    SPObject *o = NULL;

    if (id) {
        o = doc->getObjectById(id);
        if (o) {
            if (!SP_IS_ITEM (o)) {
                g_warning("Object with id=\"%s\" is not a visible item. Cannot query dimensions.", id);
                return;
            }
        } else {
            g_warning("Object with id=\"%s\" is not found. Cannot query dimensions.", id);
            return;
        }
    } else {
        o = SP_DOCUMENT_ROOT(doc);
    }

    if (o) {
        sp_document_ensure_up_to_date (doc);
        SPItem *item = ((SPItem *) o);

        // "true" SVG bbox for scripting
        NR::Maybe<NR::Rect> area = item->getBounds(sp_item_i2doc_affine(item));
        if (area) {
            Inkscape::SVGOStringStream os;
            if (extent) {
                os << area->extent(axis);
            } else {
                os << area->min()[axis];
            }
            g_print ("%s", os.str().c_str());
        } else {
            g_print("0");
        }
    }
}

static void
do_query_all (SPDocument *doc)
{
    SPObject *o = NULL;

    o = SP_DOCUMENT_ROOT(doc);

    if (o) {
        sp_document_ensure_up_to_date (doc);
        do_query_all_recurse(o);
    }
}

static void
do_query_all_recurse (SPObject *o)
{
    SPItem *item = ((SPItem *) o);
    if (o->id && SP_IS_ITEM(item)) {
        NR::Maybe<NR::Rect> area = item->getBounds(sp_item_i2doc_affine(item));
        if (area) {
            Inkscape::SVGOStringStream os;
            os << o->id;
            os << "," << area->min()[NR::X];
            os << "," << area->min()[NR::Y];
            os << "," << area->extent(NR::X);
            os << "," << area->extent(NR::Y);
            g_print ("%s\n", os.str().c_str());
        }
    }

    SPObject *child = o->children;
    while (child) {
        do_query_all_recurse (child);
        child = child->next;
    }
}


static void
sp_do_export_png(SPDocument *doc)
{
    const gchar *filename = NULL;
    gdouble dpi = 0.0;

    if (sp_export_use_hints && (!sp_export_id && !sp_export_area_drawing)) {
        g_warning ("--export-use-hints can only be used with --export-id or --export-area-drawing; ignored.");
    }

    GSList *items = NULL;

    NRRect area;
    if (sp_export_id || sp_export_area_drawing) {

        SPObject *o = NULL;
        SPObject *o_area = NULL;
        if (sp_export_id && sp_export_area_drawing) {
            o = doc->getObjectById(sp_export_id);
            o_area = SP_DOCUMENT_ROOT (doc);
        } else if (sp_export_id) {
            o = doc->getObjectById(sp_export_id);
            o_area = o;
        } else if (sp_export_area_drawing) {
            o = SP_DOCUMENT_ROOT (doc);
            o_area = o;
        }

        if (o) {
            if (!SP_IS_ITEM (o)) {
                g_warning("Object with id=\"%s\" is not a visible item. Nothing exported.", sp_export_id);
                return;
            }

            items = g_slist_prepend (items, SP_ITEM(o));

            if (sp_export_id_only) {
                g_print("Exporting only object with id=\"%s\"; all other objects hidden\n", sp_export_id);
            }

            if (sp_export_use_hints) {

                // retrieve export filename hint
                const gchar *fn_hint = SP_OBJECT_REPR(o)->attribute("inkscape:export-filename");
                if (fn_hint) {
                    if (sp_export_png) {
                        g_warning ("Using export filename from the command line (--export-png). Filename hint %s is ignored.", fn_hint);
                        filename = sp_export_png;
                    } else {
                        filename = fn_hint;
                    }
                } else {
                    g_warning ("Export filename hint not found for the object.");
                    filename = sp_export_png;
                }

                // retrieve export dpi hints
                const gchar *dpi_hint = SP_OBJECT_REPR(o)->attribute("inkscape:export-xdpi"); // only xdpi, ydpi is always the same now
                if (dpi_hint) {
                    if (sp_export_dpi || sp_export_width || sp_export_height) {
                        g_warning ("Using bitmap dimensions from the command line (--export-dpi, --export-width, or --export-height). DPI hint %s is ignored.", dpi_hint);
                    } else {
                        dpi = atof(dpi_hint);
                    }
                } else {
                    g_warning ("Export DPI hint not found for the object.");
                }

            }

            // write object bbox to area
            sp_document_ensure_up_to_date (doc);
            sp_item_invoke_bbox((SPItem *) o_area, &area, sp_item_i2r_affine((SPItem *) o_area), TRUE);
        } else {
            g_warning("Object with id=\"%s\" was not found in the document. Nothing exported.", sp_export_id);
            return;
        }
    }

    if (sp_export_area) {
        /* Try to parse area (given in SVG pixels) */
        if (!sscanf(sp_export_area, "%lg:%lg:%lg:%lg", &area.x0, &area.y0, &area.x1, &area.y1) == 4) {
            g_warning("Cannot parse export area '%s'; use 'x0:y0:x1:y1'. Nothing exported.", sp_export_area);
            return;
        }
        if ((area.x0 >= area.x1) || (area.y0 >= area.y1)) {
            g_warning("Export area '%s' has negative width or height. Nothing exported.", sp_export_area);
            return;
        }
    } else if (sp_export_area_canvas || !(sp_export_id || sp_export_area_drawing)) {
        /* Export the whole canvas */
        sp_document_ensure_up_to_date (doc);
        area.x0 = SP_ROOT(doc->root)->x.computed;
        area.y0 = SP_ROOT(doc->root)->y.computed;
        area.x1 = area.x0 + sp_document_width (doc);
        area.y1 = area.y0 + sp_document_height (doc);
    }

    // set filename and dpi from options, if not yet set from the hints
    if (!filename) {
        if (!sp_export_png) {
            g_warning ("No export filename given and no filename hint. Nothing exported.");
            return;
        }
        filename = sp_export_png;
    }

    if (sp_export_dpi && dpi == 0.0) {
        dpi = atof(sp_export_dpi);
        if ((dpi < 0.1) || (dpi > 10000.0)) {
            g_warning("DPI value %s out of range [0.1 - 10000.0]. Nothing exported.", sp_export_dpi);
            return;
        }
        g_print("DPI: %g\n", dpi);
    }

    if (sp_export_area_snap) {
        area.x0 = std::floor (area.x0);
        area.y0 = std::floor (area.y0);
        area.x1 = std::ceil (area.x1);
        area.y1 = std::ceil (area.y1);
    }

    // default dpi
    if (dpi == 0.0)
        dpi = PX_PER_IN;

    unsigned long int width = 0;
    unsigned long int height = 0;

    if (sp_export_width) {
        width = strtoul(sp_export_width, NULL, 0);
        if ((width < 1) || (width > PNG_UINT_31_MAX) || (errno == ERANGE) ) {
            g_warning("Export width %lu out of range (1 - %lu). Nothing exported.", width, (unsigned long int)PNG_UINT_31_MAX);
            return;
        }
        dpi = (gdouble) width * PX_PER_IN / (area.x1 - area.x0);
    }

    if (sp_export_height) {
        height = strtoul(sp_export_height, NULL, 0);
        if ((height < 1) || (height > PNG_UINT_31_MAX)) {
            g_warning("Export height %lu out of range (1 - %lu). Nothing exported.", height, (unsigned long int)PNG_UINT_31_MAX);
            return;
        }
        dpi = (gdouble) height * PX_PER_IN / (area.y1 - area.y0);
    }

    if (!sp_export_width) {
        width = (unsigned long int) ((area.x1 - area.x0) * dpi / PX_PER_IN + 0.5);
    }

    if (!sp_export_height) {
        height = (unsigned long int) ((area.y1 - area.y0) * dpi / PX_PER_IN + 0.5);
    }

    guint32 bgcolor = 0x00000000;
    if (sp_export_background) {
        // override the page color
        bgcolor = sp_svg_read_color(sp_export_background, 0xffffff00);
        bgcolor |= 0xff; // default is no opacity
    } else {
        // read from namedview
        Inkscape::XML::Node *nv = sp_repr_lookup_name (doc->rroot, "sodipodi:namedview");
        if (nv && nv->attribute("pagecolor"))
            bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
        if (nv && nv->attribute("inkscape:pageopacity"))
            bgcolor |= SP_COLOR_F_TO_U(sp_repr_get_double_attribute (nv, "inkscape:pageopacity", 1.0));
    }

    if (sp_export_background_opacity) {
        // override opacity
        gfloat value;
        if (sp_svg_number_read_f (sp_export_background_opacity, &value)) {
            if (value > 1.0) {
                value = CLAMP (value, 1.0f, 255.0f);
                bgcolor &= (guint32) 0xffffff00;
                bgcolor |= (guint32) floor(value);
            } else {
                value = CLAMP (value, 0.0f, 1.0f);
                bgcolor &= (guint32) 0xffffff00;
                bgcolor |= SP_COLOR_F_TO_U(value);
            }
        }
    }

    g_print("Background RRGGBBAA: %08x\n", bgcolor);

    g_print("Area %g:%g:%g:%g exported to %lu x %lu pixels (%g dpi)\n", area.x0, area.y0, area.x1, area.y1, width, height, dpi);

    g_print("Bitmap saved as: %s\n", filename);

    if ((width >= 1) && (height >= 1) && (width <= PNG_UINT_31_MAX) && (height <= PNG_UINT_31_MAX)) {
        sp_export_png_file(doc, filename, area.x0, area.y0, area.x1, area.y1, width, height, dpi, dpi, bgcolor, NULL, NULL, true, sp_export_id_only ? items : NULL);
    } else {
        g_warning("Calculated bitmap dimensions %lu %lu are out of range (1 - %lu). Nothing exported.", width, height, (unsigned long int)PNG_UINT_31_MAX);
    }

    g_slist_free (items);
}


/**
 *  Perform an export of either PS or EPS.
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as.
 */

static void do_export_ps(SPDocument* doc, gchar const* uri, char const* mime)
{
    Inkscape::Extension::DB::OutputList o;
    Inkscape::Extension::db.get_output_list(o);
    Inkscape::Extension::DB::OutputList::const_iterator i = o.begin();
    while (i != o.end() && strcmp( (*i)->get_mimetype(), mime ) != 0) {
        i++;
    }

    if (i == o.end())
    {
        g_warning ("Could not find an extension to export to MIME type %s.", mime);
        return;
    }

    bool old_text_to_path = false;
    bool old_font_embedded = false;
    bool old_bbox_page = false;

    try {
        old_text_to_path = (*i)->get_param_bool("textToPath");
        (*i)->set_param_bool("textToPath", sp_export_text_to_path);
    }
    catch (...) {
        g_warning ("Could not set export-text-to-path option for this export.");
    }

    try {
        old_font_embedded = (*i)->get_param_bool("fontEmbedded");
        (*i)->set_param_bool("fontEmbedded", sp_export_font);
    }
    catch (...) {
        g_warning ("Could not set export-font option for this export.");
    }

    try {
        old_bbox_page = (*i)->get_param_bool("pageBoundingBox");
        (*i)->set_param_bool("pageBoundingBox", sp_export_bbox_page);
    }
    catch (...) {
        g_warning ("Could not set export-bbox-page option for this export.");
    }

    (*i)->save(doc, uri);

    try {
        (*i)->set_param_bool("textToPath", old_text_to_path);
        (*i)->set_param_bool("fontEmbedded", old_font_embedded);
        (*i)->set_param_bool("pageBoundingBox", old_bbox_page);
    }
    catch (...) {

    }
}

/**
 *  Perform a PDF export
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as.
 */

static void do_export_pdf(SPDocument* doc, gchar const* uri, char const* mime)
{
    Inkscape::Extension::DB::OutputList o;
    Inkscape::Extension::db.get_output_list(o);
    Inkscape::Extension::DB::OutputList::const_iterator i = o.begin();
    while (i != o.end() && strcmp( (*i)->get_mimetype(), mime ) != 0) {
        i++;
    }

    if (i == o.end())
    {
        g_warning ("Could not find an extension to export to MIME type %s.", mime);
        return;
    }

    if (sp_export_id) {
        SPObject *o = doc->getObjectById(sp_export_id);
        if (o == NULL) {
            g_warning("Object with id=\"%s\" was not found in the document. Nothing exported.", sp_export_id);
            return;
        }
        (*i)->set_param_string ("exportId", sp_export_id);
    } else {
        (*i)->set_param_string ("exportId", "");
    }

    if (sp_export_area_drawing) {
        (*i)->set_param_bool ("exportDrawing", TRUE);
    } else {
        (*i)->set_param_bool ("exportDrawing", FALSE);
    }

    (*i)->save(doc, uri);
}

#ifdef WIN32
/**
 *  Export a document to EMF
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as (should be "image/x-emf")
 */

static void do_export_emf(SPDocument* doc, gchar const* uri, char const* mime)
{
    Inkscape::Extension::DB::OutputList o;
    Inkscape::Extension::db.get_output_list(o);
    Inkscape::Extension::DB::OutputList::const_iterator i = o.begin();
    while (i != o.end() && strcmp( (*i)->get_mimetype(), mime ) != 0) {
        i++;
    }

    if (i == o.end())
    {
        g_warning ("Could not find an extension to export to MIME type %s.", mime);
        return;
    }

    (*i)->save(doc, uri);
}
#endif //WIN32

#ifdef WIN32
bool replaceArgs( int& argc, char**& argv )
{
    bool worked = false;

#ifdef REPLACEARGS_DEBUG
    MessageBoxA( NULL, "GetCommandLineW() getting called", "GetCommandLineW", MB_OK | MB_ICONINFORMATION );
#endif // REPLACEARGS_DEBUG

    wchar_t* line = GetCommandLineW();
    if ( line )
    {
#ifdef REPLACEARGS_DEBUG
        {
            gchar* utf8Line = g_utf16_to_utf8( (gunichar2*)line, -1, NULL, NULL, NULL );
            if ( utf8Line )
            {
                gchar *safe = Inkscape::IO::sanitizeString(utf8Line);
                {
                    char tmp[strlen(safe) + 32];
                    snprintf( tmp, sizeof(tmp), "GetCommandLineW() = '%s'", safe );
                    MessageBoxA( NULL, tmp, "GetCommandLineW", MB_OK | MB_ICONINFORMATION );
                }
            }
        }
#endif // REPLACEARGS_DEBUG

        int numArgs = 0;
        wchar_t** parsed = CommandLineToArgvW( line, &numArgs );

#ifdef REPLACEARGS_ANSI
// test code for trying things on Win95/98/ME
        if ( !parsed )
        {
#ifdef REPLACEARGS_DEBUG
            MessageBoxA( NULL, "Unable to process command-line. Faking it", "CommandLineToArgvW", MB_OK | MB_ICONINFORMATION );
#endif // REPLACEARGS_DEBUG
            int lineLen = wcslen(line) + 1;
            wchar_t* lineDup = new wchar_t[lineLen];
            wcsncpy( lineDup, line, lineLen );

            int pos = 0;
            bool inQuotes = false;
            bool inWhitespace = true;
            std::vector<int> places;
            while ( lineDup[pos] )
            {
                if ( inQuotes )
                {
                    if ( lineDup[pos] == L'"' )
                    {
                        inQuotes = false;
                    }
                }
                else if ( lineDup[pos] == L'"' )
                {
                    inQuotes = true;
                    inWhitespace = false;
                    places.push_back(pos);
                }
                else if ( lineDup[pos] == L' ' || lineDup[pos] == L'\t' )
                {
                    if ( !inWhitespace )
                    {
                        inWhitespace = true;
                        lineDup[pos] = 0;
                    }
                }
                else if ( inWhitespace && (lineDup[pos] != L' ' && lineDup[pos] != L'\t') )
                {
                    inWhitespace = false;
                    places.push_back(pos);
                }
                else
                {
                    // consume
                }
                pos++;
            }
#ifdef REPLACEARGS_DEBUG
            {
                char tmp[256];
                snprintf( tmp, sizeof(tmp), "Counted %d args", places.size() );
                MessageBoxA( NULL, tmp, "CommandLineToArgvW", MB_OK | MB_ICONINFORMATION );
            }
#endif // REPLACEARGS_DEBUG

            wchar_t** block = new wchar_t*[places.size()];
            int i = 0;
            for ( std::vector<int>::iterator it = places.begin(); it != places.end(); it++ )
            {
                block[i++] = &lineDup[*it];
            }
            parsed = block;
            numArgs = places.size();
        }
#endif // REPLACEARGS_ANSI

        if ( parsed )
        {
            std::vector<wchar_t*>expandedArgs;
            if ( numArgs > 0 )
            {
                expandedArgs.push_back( parsed[0] );
            }

            for ( int i1 = 1; i1 < numArgs; i1++ )
            {
                bool wildcarded = (wcschr(parsed[i1], L'?') != NULL) || (wcschr(parsed[i1], L'*') != NULL);
                wildcarded &= parsed[i1][0] != L'"';
                wildcarded &= parsed[i1][0] != L'-';
                if ( wildcarded )
                {
#ifdef REPLACEARGS_ANSI
                    WIN32_FIND_DATAA data = {0};
#else
                    WIN32_FIND_DATAW data = {0};
#endif // REPLACEARGS_ANSI

                    int baseLen = wcslen(parsed[i1]) + 2;
                    wchar_t* base = new wchar_t[baseLen];
                    wcsncpy( base, parsed[i1], baseLen );
                    wchar_t* last = wcsrchr( base, L'\\' );
                    if ( last )
                    {
                        last[1] = 0;
                    }
                    else
                    {
                        base[0] = 0;
                    }
                    baseLen = wcslen( base );

#ifdef REPLACEARGS_ANSI
                    char target[MAX_PATH];
                    if ( WideCharToMultiByte( CP_ACP, 0, parsed[i1], -1, target, sizeof(target), NULL, NULL) )
                    {
                        HANDLE hf = FindFirstFileA( target, &data );
#else
                        HANDLE hf = FindFirstFileW( parsed[i1], &data );
#endif // REPLACEARGS_ANSI
                        if ( hf != INVALID_HANDLE_VALUE )
                        {
                            BOOL found = TRUE;
                            do
                            {
#ifdef REPLACEARGS_ANSI
                                int howMany = MultiByteToWideChar( CP_ACP, 0, data.cFileName, -1, NULL, 0 );
                                if ( howMany > 0 )
                                {
                                    howMany += baseLen;
                                    wchar_t* tmp = new wchar_t[howMany + 1];
                                    wcsncpy( tmp, base, howMany + 1 );
                                    MultiByteToWideChar( CP_ACP, 0, data.cFileName, -1, tmp + baseLen, howMany + 1 - baseLen );
                                    expandedArgs.push_back( tmp );
                                    found = FindNextFileA( hf, &data );
                                }
#else
                                int howMany = wcslen(data.cFileName) + baseLen;
                                wchar_t* tmp = new wchar_t[howMany + 1];
                                wcsncpy( tmp, base, howMany + 1 );
                                wcsncat( tmp, data.cFileName, howMany + 1 );
                                expandedArgs.push_back( tmp );
                                found = FindNextFileW( hf, &data );
#endif // REPLACEARGS_ANSI
                            } while ( found );

                            FindClose( hf );
                        }
                        else
                        {
                            expandedArgs.push_back( parsed[i1] );
                        }
#ifdef REPLACEARGS_ANSI
                    }
#endif // REPLACEARGS_ANSI

                    delete[] base;
                }
                else
                {
                    expandedArgs.push_back( parsed[i1] );
                }
            }

            {
                wchar_t** block = new wchar_t*[expandedArgs.size()];
                int iz = 0;
                for ( std::vector<wchar_t*>::iterator it = expandedArgs.begin(); it != expandedArgs.end(); it++ )
                {
                    block[iz++] = *it;
                }
                parsed = block;
                numArgs = expandedArgs.size();
            }

            std::vector<gchar*> newArgs;
            for ( int i = 0; i < numArgs; i++ )
            {
                gchar* replacement = g_utf16_to_utf8( (gunichar2*)parsed[i], -1, NULL, NULL, NULL );
                if ( replacement )
                {
#ifdef REPLACEARGS_DEBUG
                    gchar *safe2 = Inkscape::IO::sanitizeString(replacement);

                    if ( safe2 )
                    {
                        {
                            char tmp[1024];
                            snprintf( tmp, sizeof(tmp), "    [%2d] = '%s'", i, safe2 );
                            MessageBoxA( NULL, tmp, "GetCommandLineW", MB_OK | MB_ICONINFORMATION );
                        }
                        g_free( safe2 );
                    }
#endif // REPLACEARGS_DEBUG

                    newArgs.push_back( replacement );
                }
                else
                {
                    newArgs.push_back( blankParam );
                }
            }

            // Now push our munged params to be the new argv and argc
            {
                char** block = new char*[newArgs.size()];
                int iz = 0;
                for ( std::vector<char*>::iterator it = newArgs.begin(); it != newArgs.end(); it++ )
                {
                    block[iz++] = *it;
                }
                argv = block;
                argc = newArgs.size();
                worked = true;
            }
        }
#ifdef REPLACEARGS_DEBUG
        else
        {
            MessageBoxA( NULL, "Unable to process command-line", "CommandLineToArgvW", MB_OK | MB_ICONINFORMATION );
        }
#endif // REPLACEARGS_DEBUG
    }
#ifdef REPLACEARGS_DEBUG
    else
    {
        {
            MessageBoxA( NULL,  "Unable to fetch result from GetCommandLineW()", "GetCommandLineW", MB_OK | MB_ICONINFORMATION );
        }

        char* line2 = GetCommandLineA();
        if ( line2 )
        {
            gchar *safe = Inkscape::IO::sanitizeString(line2);
            {
                {
                    char tmp[strlen(safe) + 32];
                    snprintf( tmp, sizeof(tmp), "GetCommandLineA() = '%s'", safe );
                    MessageBoxA( NULL, tmp, "GetCommandLineA", MB_OK | MB_ICONINFORMATION );
                }
            }
        }
        else
        {
            MessageBoxA( NULL, "Unable to fetch result from GetCommandLineA()", "GetCommandLineA", MB_OK | MB_ICONINFORMATION );
        }
    }
#endif // REPLACEARGS_DEBUG

    return worked;
}
#endif // WIN32

static GSList *
sp_process_args(poptContext ctx)
{
    GSList *fl = NULL;

    gint a;
    while ((a = poptGetNextOpt(ctx)) >= 0) {
        switch (a) {
            case SP_ARG_FILE: {
                gchar const *fn = poptGetOptArg(ctx);
                if (fn != NULL) {
                    fl = g_slist_append(fl, g_strdup(fn));
                }
                break;
            }
            case SP_ARG_VERSION: {
                printf("Inkscape %s (%s)\n", INKSCAPE_VERSION, __DATE__);
                exit(0);
                break;
            }
            case SP_ARG_EXTENSIONDIR: {
                printf("%s\n", INKSCAPE_EXTENSIONDIR);
                exit(0);
                break;
            }
            case SP_ARG_VERB_LIST: {
                // This really shouldn't go here, we should init the app.
                // But, since we're just exiting in this path, there is
                // no harm, and this is really a better place to put
                // everything else.
                Inkscape::Extension::init();
                Inkscape::Verb::list();
                exit(0);
                break;
            }
            case SP_ARG_VERB:
            case SP_ARG_SELECT: {
                gchar const *arg = poptGetOptArg(ctx);
                if (arg != NULL) {
                    // printf("Adding in: %s\n", arg);
                    new Inkscape::CmdLineAction((a == SP_ARG_VERB), arg);
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    gchar const ** const args = poptGetArgs(ctx);
    if (args != NULL) {
        for (unsigned i = 0; args[i] != NULL; i++) {
            fl = g_slist_append(fl, g_strdup(args[i]));
        }
    }

    return fl;
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

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
 *   Bryce Harrington <bryce@bryceharrington.org>
 * ... and various people who have worked with various projects
 *   Jon A. Cruz <jon@oncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2004 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

// This has to be included prior to anything that includes setjmp.h, it croaks otherwise
#include <png.h>

#include "ui/widget/panel.h" // This has to be the first to include <glib.h> because of Glibmm's dependence on a deprecated feature...

#include "path-prefix.h"

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#include <cstring>
#include <string>
#include <locale.h>
#include <stdlib.h>

#include <popt.h>
#ifndef POPT_TABLEEND
#define POPT_TABLEEND { NULL, '\0', 0, 0, 0, NULL, NULL }
#endif /* Not def: POPT_TABLEEND */

#include <libxml/tree.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "gc-core.h"

#ifdef AND
#undef AND
#endif

#include "macros.h"
#include "file.h"
#include "document.h"
#include "layer-model.h"
#include "selection.h"
#include "sp-object.h"
#include "interface.h"
#include "print.h"
#include "color.h"
#include "sp-item.h"
#include "sp-root.h"

#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/stringstream.h"

#include "inkscape-private.h"
#include "inkscape-version.h"

#include "sp-namedview.h"
#include "sp-guide.h"
#include "xml/repr.h"

#include "io/sys.h"

#include "debug/logger.h"
#include "debug/log-display-config.h"

#include "helper/action-context.h"
#include "helper/png-write.h"
#include "helper/geom.h"

#include <extension/extension.h>
#include <extension/system.h>
#include <extension/db.h>
#include <extension/output.h>
#include <extension/input.h>

#ifdef WIN32
#include <windows.h>
#include "registrytool.h"
#endif // WIN32

#include "extension/init.h"
// Not ideal, but there doesn't appear to be a nicer system in place for
// passing command-line parameters to extensions before initialization...
#ifdef WITH_DBUS
#include "extension/dbus/dbus-init.h"
#endif // WITH_DBUS

#include <glibmm/i18n.h>
#include <glibmm/main.h>
#include <glibmm/miscutils.h>

#include <gtkmm/main.h>

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif

#include "main-cmdlineact.h"
#include "widgets/icon.h"

#include <errno.h>
#include "verbs.h"

#include <gdk/gdkkeysyms.h>

#include "path-chemistry.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"

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
    SP_ARG_EXPORT_AREA_PAGE,
    SP_ARG_EXPORT_MARGIN,
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
    SP_ARG_EXPORT_PS_LEVEL,
    SP_ARG_EXPORT_PDF,
    SP_ARG_EXPORT_PDF_VERSION,
    SP_ARG_EXPORT_LATEX,
    SP_ARG_EXPORT_EMF,
    SP_ARG_EXPORT_WMF,
    SP_ARG_EXPORT_TEXT_TO_PATH,
    SP_ARG_EXPORT_IGNORE_FILTERS,
    SP_ARG_EXTENSIONDIR,
    SP_ARG_QUERY_X,
    SP_ARG_QUERY_Y,
    SP_ARG_QUERY_WIDTH,
    SP_ARG_QUERY_HEIGHT,
    SP_ARG_QUERY_ALL,
    SP_ARG_QUERY_ID,
    SP_ARG_SHELL,
    SP_ARG_VERSION,
    SP_ARG_VACUUM_DEFS,
#ifdef WITH_DBUS
    SP_ARG_DBUS_LISTEN,
    SP_ARG_DBUS_NAME,
#endif // WITH_DBUS
    SP_ARG_VERB_LIST,
    SP_ARG_VERB,
    SP_ARG_SELECT,
    SP_ARG_LAST
};

int sp_main_gui(int argc, char const **argv);
int sp_main_console(int argc, char const **argv);
static int sp_do_export_png(SPDocument *doc);
static int do_export_ps_pdf(SPDocument* doc, gchar const* uri, char const *mime);
static int do_export_emf(SPDocument* doc, gchar const* uri, char const *mime);
static int do_export_wmf(SPDocument* doc, gchar const* uri, char const *mime);
static int do_export_win_metafile_common(SPDocument* doc, gchar const* uri, char const *mime);
static void do_query_dimension (SPDocument *doc, bool extent, Geom::Dim2 const axis, const gchar *id);
static void do_query_all (SPDocument *doc);
static void do_query_all_recurse (SPObject *o);

static gchar *sp_global_printer = NULL;
static gchar *sp_export_png = NULL;
static gchar *sp_export_dpi = NULL;
static gchar *sp_export_area = NULL;
static gboolean sp_export_area_drawing = FALSE;
static gboolean sp_export_area_page = FALSE;
static gchar *sp_export_margin = NULL;
static gboolean sp_export_latex = FALSE;
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
static gint sp_export_ps_level = 2;
static gchar *sp_export_pdf = NULL;
static gchar *sp_export_pdf_version = NULL;
static gchar *sp_export_emf = NULL;
static gchar *sp_export_wmf = NULL;
static gboolean sp_export_text_to_path = FALSE;
static gboolean sp_export_ignore_filters = FALSE;
static gboolean sp_export_font = FALSE;
static gboolean sp_query_x = FALSE;
static gboolean sp_query_y = FALSE;
static gboolean sp_query_width = FALSE;
static gboolean sp_query_height = FALSE;
static gboolean sp_query_all = FALSE;
static gchar *sp_query_id = NULL;
static gboolean sp_shell = FALSE;
static gboolean sp_vacuum_defs = FALSE;
#ifdef WITH_DBUS
static gboolean sp_dbus_listen = FALSE;
static gchar *sp_dbus_name = NULL;
#endif // WITH_DBUS
static gchar *sp_export_png_utf8 = NULL;
static gchar *sp_export_svg_utf8 = NULL;
static gchar *sp_global_printer_utf8 = NULL;


/**
 *  Reset variables to default values.
 */
static void resetCommandlineGlobals() {
        sp_global_printer = NULL;
        sp_export_png = NULL;
        sp_export_dpi = NULL;
        sp_export_area = NULL;
        sp_export_area_drawing = FALSE;
        sp_export_area_page = FALSE;
        sp_export_margin = NULL;
        sp_export_latex = FALSE;
        sp_export_width = NULL;
        sp_export_height = NULL;
        sp_export_id = NULL;
        sp_export_background = NULL;
        sp_export_background_opacity = NULL;
        sp_export_area_snap = FALSE;
        sp_export_use_hints = FALSE;
        sp_export_id_only = FALSE;
        sp_export_svg = NULL;
        sp_export_ps = NULL;
        sp_export_eps = NULL;
        sp_export_ps_level = 2;
        sp_export_pdf = NULL;
        sp_export_pdf_version = NULL;
        sp_export_emf = NULL;
        sp_export_wmf = NULL;
        sp_export_text_to_path = FALSE;
        sp_export_ignore_filters = FALSE;
        sp_export_font = FALSE;
        sp_query_x = FALSE;
        sp_query_y = FALSE;
        sp_query_width = FALSE;
        sp_query_height = FALSE;
        sp_query_all = FALSE;
        sp_query_id = NULL;
        sp_vacuum_defs = FALSE;
#ifdef WITH_DBUS
        sp_dbus_listen = FALSE;
        sp_dbus_name = NULL;
#endif // WITH_DBUS

        sp_export_png_utf8 = NULL;
        sp_export_svg_utf8 = NULL;
        sp_global_printer_utf8 = NULL;
}

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
     N_("Resolution for exporting to bitmap and for rasterization of filters in PS/EPS/PDF (default 90)"),
     N_("DPI")},

    {"export-area", 'a',
     POPT_ARG_STRING, &sp_export_area, SP_ARG_EXPORT_AREA,
     N_("Exported area in SVG user units (default is the page; 0,0 is lower-left corner)"),
     N_("x0:y0:x1:y1")},

    {"export-area-drawing", 'D',
     POPT_ARG_NONE, &sp_export_area_drawing, SP_ARG_EXPORT_AREA_DRAWING,
     N_("Exported area is the entire drawing (not page)"),
     NULL},

    {"export-area-page", 'C',
     POPT_ARG_NONE, &sp_export_area_page, SP_ARG_EXPORT_AREA_PAGE,
     N_("Exported area is the entire page"),
     NULL},

    {"export-margin", 0,
     POPT_ARG_STRING, &sp_export_margin, SP_ARG_EXPORT_MARGIN,
     N_("Only for PS/EPS/PDF, sets margin in mm around exported area (default 0)"),
     N_("VALUE")},

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

    {"export-ps-level", 0,
     POPT_ARG_INT, &sp_export_ps_level, SP_ARG_EXPORT_PS_LEVEL,
     N_("Choose the PostScript Level used to export. Possible choices are"
        " 2 (the default) and 3"),
     N_("PS Level")},

    {"export-pdf", 'A',
     POPT_ARG_STRING, &sp_export_pdf, SP_ARG_EXPORT_PDF,
     N_("Export document to a PDF file"),
     N_("FILENAME")},

    {"export-pdf-version", 0,
     POPT_ARG_STRING, &sp_export_pdf_version, SP_ARG_EXPORT_PDF_VERSION,
     // TRANSLATORS: "--export-pdf-version" is an Inkscape command line option; see "inkscape --help"
     N_("Export PDF to given version. (hint: make sure to input the exact string found in the PDF export dialog, e.g. \"PDF 1.4\" which is PDF-a conformant)"),
     N_("PDF_VERSION")},

    {"export-latex", 0,
     POPT_ARG_NONE, &sp_export_latex, SP_ARG_EXPORT_LATEX,
     N_("Export PDF/PS/EPS without text. Besides the PDF/PS/EPS, a LaTeX file is exported, putting the text on top of the PDF/PS/EPS file. Include the result in LaTeX like: \\input{latexfile.tex}"),
     NULL},

    {"export-emf", 'M',
     POPT_ARG_STRING, &sp_export_emf, SP_ARG_EXPORT_EMF,
     N_("Export document to an Enhanced Metafile (EMF) File"),
     N_("FILENAME")},

    {"export-wmf", 'm',
     POPT_ARG_STRING, &sp_export_wmf, SP_ARG_EXPORT_WMF,
     N_("Export document to a Windows Metafile (WMF) File"),
     N_("FILENAME")},

    {"export-text-to-path", 'T',
     POPT_ARG_NONE, &sp_export_text_to_path, SP_ARG_EXPORT_TEXT_TO_PATH,
     N_("Convert text object to paths on export (PS, EPS, PDF, SVG)"),
     NULL},

    {"export-ignore-filters", 0,
     POPT_ARG_NONE, &sp_export_ignore_filters, SP_ARG_EXPORT_IGNORE_FILTERS,
     N_("Render filtered objects without filters, instead of rasterizing (PS, EPS, PDF)"),
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
     
#ifdef WITH_DBUS
    {"dbus-listen", 0,
     POPT_ARG_NONE, &sp_dbus_listen, SP_ARG_DBUS_LISTEN,
     N_("Enter a listening loop for D-Bus messages in console mode"),
     NULL},

    {"dbus-name", 0,
     POPT_ARG_STRING, &sp_dbus_name, SP_ARG_DBUS_NAME,
     N_("Specify the D-Bus bus name to listen for messages on (default is org.inkscape)"),
     N_("BUS-NAME")},
#endif // WITH_DBUS

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

    {"shell", 0,
     POPT_ARG_NONE, &sp_shell, SP_ARG_SHELL,
     N_("Start Inkscape in interactive shell mode."),
     NULL},

    POPT_AUTOHELP POPT_TABLEEND
};

static bool needToRecodeParams = true;
gchar * blankParam = g_strdup("");



#ifdef WIN32

/**
 * Set up the PATH, INKSCAPE_LOCALEDIR and PYTHONPATH environment
 * variables on Windows
 * @param exe Inkscape executable directory in UTF-8
 */
static void _win32_set_inkscape_env(gchar const *exe)
{
    gchar const *path = g_getenv("PATH");
    gchar const *pythonpath = g_getenv("PYTHONPATH");

    gchar *python = g_build_filename(exe, "python", NULL);
    gchar *scripts = g_build_filename(exe, "python", "Scripts", NULL);
    gchar *perl = g_build_filename(exe, "python", NULL);
    gchar *pythonlib = g_build_filename(exe, "python", "Lib", NULL);
    gchar *pythondll = g_build_filename(exe, "python", "DLLs", NULL);
    
    // Python 2.x needs short paths in PYTHONPATH.
    // Otherwise it doesn't work when Inkscape is installed in Unicode directories.
    // g_win32_locale_filename_from_utf8 is the GLib wrapper for GetShortPathName.
    // Remove this once we move to Python 3.0.
    gchar *python_s = g_win32_locale_filename_from_utf8(python);
    gchar *pythonlib_s = g_win32_locale_filename_from_utf8(pythonlib);
    gchar *pythondll_s = g_win32_locale_filename_from_utf8(pythondll);

    gchar *new_path;
    gchar *new_pythonpath;
    if (path) {
        new_path = g_strdup_printf("%s;%s;%s;%s;%s", exe, python, scripts, perl, path);
    } else {
        new_path = g_strdup_printf("%s;%s;%s;%s", exe, python, scripts, perl);
    }
    if (pythonpath) {
        new_pythonpath = g_strdup_printf("%s;%s;%s;%s",
             python_s, pythonlib_s, pythondll_s, pythonpath);
    } else {
        new_pythonpath = g_strdup_printf("%s;%s;%s",
            python_s, pythonlib_s, pythondll_s);
    }

    g_setenv("PATH", new_path, TRUE);
    g_setenv("PYTHONPATH", new_pythonpath, TRUE);

    /*
    printf("PATH = %s\n\n", g_getenv("PATH"));
    printf("PYTHONPATH = %s\n\n", g_getenv("PYTHONPATH"));

    gchar *p = g_find_program_in_path("python");
    if (p) {
        printf("python in %s\n\n", p);
        g_free(p);
    } else {
        printf("python not found\n\n");
    }*/

    // INKSCAPE_LOCALEDIR is needed by Python/Gettext
    gchar *localepath = g_build_filename(exe, PACKAGE_LOCALE_DIR, NULL);
    g_setenv("INKSCAPE_LOCALEDIR", localepath, TRUE);

    // prevent "please insert disk" messages. fixes bug #950781
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

    g_free(python);
    g_free(scripts);
    g_free(perl);
    g_free(pythonlib);
    g_free(pythondll);
    
    g_free(python_s);
    g_free(pythonlib_s);
    g_free(pythondll_s);

    g_free(new_path);
    g_free(new_pythonpath);
    
    g_free(localepath);
}
#endif

static void set_extensions_env()
{
    gchar const *pythonpath = g_getenv("PYTHONPATH");
    gchar *extdir;
    gchar *new_pythonpath;
    
#ifdef WIN32
    extdir = g_win32_locale_filename_from_utf8(INKSCAPE_EXTENSIONDIR);
#else
    extdir = g_strdup(INKSCAPE_EXTENSIONDIR);
#endif

    // On some platforms, INKSCAPE_EXTENSIONDIR is not absolute,
    // but relative to the directory that contains the Inkscape executable.
    // Since we spawn Python chdir'ed into the script's directory,
    // we need to obtain the absolute path here.
    if (!g_path_is_absolute(extdir)) {
        gchar *curdir = g_get_current_dir();
        gchar *extdir_new = g_build_filename(curdir, extdir, NULL);
        g_free(extdir);
        g_free(curdir);
        extdir = extdir_new;
    }

    if (pythonpath) {
        new_pythonpath = g_strdup_printf("%s" G_SEARCHPATH_SEPARATOR_S "%s",
                                         extdir, pythonpath);
        g_free(extdir);
    } else {
        new_pythonpath = extdir;
    }

    g_setenv("PYTHONPATH", new_pythonpath, TRUE);
    g_free(new_pythonpath);
    //printf("PYTHONPATH = %s\n", g_getenv("PYTHONPATH"));
}

/**
 * This is the classic main() entry point of the program, though on some
 * architectures it might be called by something else.
 */
int
main(int argc, char **argv)
{
#ifdef HAVE_FPSETMASK
    /* This is inherited from Sodipodi code, where it was in #ifdef __FreeBSD__.  It's probably
       safe to remove: the default mask is already 0 in C99, and in current FreeBSD according to
       the fenv man page on www.freebsd.org, and in glibc according to (libc)FP Exceptions. */
    fpsetmask(fpgetmask() & ~(FP_X_DZ | FP_X_INV));
#endif

#ifdef WIN32
    gchar *exedir = g_strdup(win32_getExePath().data());
    _win32_set_inkscape_env(exedir);

# ifdef ENABLE_NLS
    // obtain short path to executable dir and pass it
    // to bindtextdomain (it doesn't understand UTF-8)
    gchar *shortexedir = g_win32_locale_filename_from_utf8(exedir);
    gchar *localepath = g_build_filename(shortexedir, PACKAGE_LOCALE_DIR, NULL);
    bindtextdomain(GETTEXT_PACKAGE, localepath);
    g_free(shortexedir);
    g_free(localepath);
# endif
    g_free(exedir);

    // Don't touch the registry (works fine without it) for Inkscape Portable
    gchar const *val = g_getenv("INKSCAPE_PORTABLE_PROFILE_DIR");
    if (!val) {
        RegistryTool rt;
        rt.setPathInfo();
    }
#elif defined(ENABLE_NLS)
# ifdef ENABLE_BINRELOC
    bindtextdomain(GETTEXT_PACKAGE, BR_LOCALEDIR(""));
# else
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    // needed by Python/Gettext
    g_setenv("PACKAGE_LOCALE_DIR", PACKAGE_LOCALE_DIR, TRUE);
# endif
#endif

    // the bit below compiles regardless of platform
#ifdef ENABLE_NLS
    // Allow the user to override the locale directory by setting
    // the environment variable INKSCAPE_LOCALEDIR.
    char const *inkscape_localedir = g_getenv("INKSCAPE_LOCALEDIR");
    if (inkscape_localedir != NULL) {
        bindtextdomain(GETTEXT_PACKAGE, inkscape_localedir);
    }

    // common setup
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    set_extensions_env();

    // Prevents errors like "Unable to wrap GdkPixbuf..." (in nr-filter-image.cpp for example)
    Gtk::Main::init_gtkmm_internals();

    LIBXML_TEST_VERSION

    Inkscape::GC::init();

    Inkscape::Debug::Logger::init();

    gboolean use_gui;

#if !defined(WIN32) && !defined(GDK_WINDOWING_QUARTZ)
    use_gui = (g_getenv("DISPLAY") != NULL);
#else
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
            || !strncmp(argv[i], "--export-plain-svg", 18)
            || !strcmp(argv[i], "-i")
            || !strncmp(argv[i], "--export-area-drawing", 21)
            || !strcmp(argv[i], "-D")
            || !strncmp(argv[i], "--export-area-page", 18)
            || !strcmp(argv[i], "-C")
            || !strncmp(argv[i], "--export-id", 11)
            || !strcmp(argv[i], "-P")
            || !strncmp(argv[i], "--export-ps", 11)
            || !strcmp(argv[i], "-E")
            || !strncmp(argv[i], "--export-eps", 12)
            || !strcmp(argv[i], "-A")
            || !strncmp(argv[i], "--export-pdf", 12)
            || !strncmp(argv[i], "--export-latex", 14)
            || !strcmp(argv[i], "-M")
            || !strncmp(argv[i], "--export-emf", 12)
            || !strcmp(argv[i], "-m")
            || !strncmp(argv[i], "--export-wmf", 12)
            || !strcmp(argv[i], "-W")
            || !strncmp(argv[i], "--query-width", 13)
            || !strcmp(argv[i], "-H")
            || !strncmp(argv[i], "--query-height", 14)
            || !strcmp(argv[i], "-S")
            || !strncmp(argv[i], "--query-all", 11)
            || !strcmp(argv[i], "-X")
            || !strncmp(argv[i], "--query-x", 9)
            || !strcmp(argv[i], "-Y")
            || !strncmp(argv[i], "--query-y", 9)
            || !strcmp(argv[i], "--vacuum-defs")
#ifdef WITH_DBUS
            || !strcmp(argv[i], "--dbus-listen")
#endif // WITH_DBUS
            || !strcmp(argv[i], "--shell")
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
    {
        // If the call fails, we'll need to convert charsets
        needToRecodeParams = !replaceArgs( argc, argv );
    }
#endif // WIN32

    int retcode;

    if (use_gui) {
        retcode = sp_main_gui(argc, (const char **) argv);
    } else {
        retcode = sp_main_console(argc, (const char **) argv);
    }

    return retcode;
}




static void fixupSingleFilename( gchar **orig, gchar **spare )
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



static GSList *fixupFilenameEncoding( GSList* fl )
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

static int sp_common_main( int argc, char const **argv, GSList **flDest )
{
    /// \todo fixme: Move these to some centralized location (Lauris)
    //sp_object_type_register("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
    //sp_object_type_register("sodipodi:guide", SP_TYPE_GUIDE);


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
    
#ifdef WITH_DBUS
    // Before initializing extensions, we must set the DBus bus name if required
    if (sp_dbus_name != NULL) {
        Inkscape::Extension::Dbus::dbus_set_bus_name(sp_dbus_name);
    }
#endif

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

namespace Inkscape {
namespace UI {
namespace Tools {

guint get_group0_keyval(GdkEventKey const* event);

}
}
}

static void
snooper(GdkEvent *event, gpointer /*data*/) {
    if (inkscape_mapalt())  /* returns the map of the keyboard modifier to map to Alt, zero if no mapping */
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

    if (inkscape_trackalt()) {
        // MacOS X with X11 has some problem with the default
        // xmodmapping.  A ~/.xmodmap solution does not work reliably due
        // to the way we package our executable in a .app that can launch
        // X11 or use an already-running X11.  The same problem has been
        // reported on Linux but there is no .app/X11 to get in the way
        // of ~/.xmodmap fixes.  So we make this a preference.
        //
        // For some reason, Gdk senses changes in Alt (Mod1) state for
        // many message types, but not for keystrokes!  So this ugly hack
        // tracks what the state of Alt-pressing is, and ensures
        // GDK_MOD1_MASK is in the event->key.state as appropriate.
        //
        static gboolean altL_pressed = FALSE;
        static gboolean altR_pressed = FALSE;
        static gboolean alt_pressed = FALSE;

        guint keyval = 0;
        switch (event->type) {
        case GDK_MOTION_NOTIFY:
            alt_pressed = TRUE && (event->motion.state & GDK_MOD1_MASK);
            break;
        case GDK_BUTTON_PRESS:
            alt_pressed = TRUE && (event->button.state & GDK_MOD1_MASK);
            break;
        case GDK_KEY_PRESS:
            keyval = Inkscape::UI::Tools::get_group0_keyval(&event->key);
            if (keyval == GDK_KEY_Alt_L) altL_pressed = TRUE;
            if (keyval == GDK_KEY_Alt_R) altR_pressed = TRUE;
            alt_pressed = alt_pressed || altL_pressed || altR_pressed;
            alt_pressed = alt_pressed || (event->button.state & GDK_MOD1_MASK);
            if (alt_pressed)
                event->key.state |= GDK_MOD1_MASK;
            else
                event->key.state &= ~GDK_MOD1_MASK;
            break;
        case GDK_KEY_RELEASE:
            keyval = Inkscape::UI::Tools::get_group0_keyval(&event->key);
            if (keyval == GDK_KEY_Alt_L) altL_pressed = FALSE;
            if (keyval == GDK_KEY_Alt_R) altR_pressed = FALSE;
            if (!altL_pressed && !altR_pressed)
                alt_pressed = FALSE;
            break;
        default:
            break;
        }
        //printf("alt_pressed: %s\n", alt_pressed? "+" : "-");
    }

    gtk_main_do_event (event);
}

static std::vector<Glib::ustring> getDirectorySet(const gchar* userDir, const gchar* const * systemDirs) {
    std::vector<Glib::ustring> listing;
    listing.push_back(userDir);
    for ( const char* const* cur = systemDirs; *cur; cur++ )
    {
        listing.push_back(*cur);
    }
    return listing;
}

int
sp_main_gui(int argc, char const **argv)
{
    Gtk::Main main_instance (&argc, const_cast<char ***>(&argv));

    GSList *fl = NULL;
    int retVal = sp_common_main( argc, argv, &fl );
    g_return_val_if_fail(retVal == 0, 1);

    // Add possible icon entry directories
    std::vector<Glib::ustring> dataDirs = getDirectorySet( g_get_user_data_dir(),
                                                           g_get_system_data_dirs() );
    for (std::vector<Glib::ustring>::iterator it = dataDirs.begin(); it != dataDirs.end(); ++it)
    {
        std::vector<Glib::ustring> listing;
        listing.push_back(*it);
        listing.push_back("inkscape");
        listing.push_back("icons");
        Glib::ustring dir = Glib::build_filename(listing);
        gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), dir.c_str());
    }

    // Add our icon directory to the search path for icon theme lookups.
    gchar *usericondir = profile_path("icons");
    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), usericondir);
    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), INKSCAPE_PIXMAPDIR);
    g_free(usericondir);

    gdk_event_handler_set((GdkEventFunc)snooper, NULL, NULL);
    Inkscape::Debug::log_display_config();

    // Set default window icon. Obeys the theme.
    gtk_window_set_default_icon_name("inkscape");
    // Do things that were previously in inkscape_gtk_stock_init().
    sp_icon_get_phys_size(GTK_ICON_SIZE_MENU);
    Inkscape::UI::Widget::Panel::prep();

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

/**
 * Process file list
 */
static int sp_process_file_list(GSList *fl)
{
    int retVal = 0;
#ifdef WITH_DBUS
    if (!fl) {
        // If we've been asked to listen for D-Bus messages, enter a main loop here
        // The main loop may be exited by calling "exit" on the D-Bus application interface.
        if (sp_dbus_listen) {
            Gtk::Main main_dbus_loop(0, NULL);
            main_dbus_loop.run();
        }
    }
#endif // WITH_DBUS

    while (fl) {
        const gchar *filename = (gchar *)fl->data;

        SPDocument *doc = NULL;
        try {
            doc = Inkscape::Extension::open(NULL, filename);
        } catch (Inkscape::Extension::Input::no_extension_found &e) {
            doc = NULL;
        } catch (Inkscape::Extension::Input::open_failed &e) {
            doc = NULL;
        }

        if (doc == NULL) {
            try {
                doc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), filename);
            } catch (Inkscape::Extension::Input::no_extension_found &e) {
                doc = NULL;
            } catch (Inkscape::Extension::Input::open_failed &e) {
                doc = NULL;
            }
        }
        if (doc == NULL) {
            g_warning("Specified document %s cannot be opened (does not exist or not a valid SVG file)", filename);
            retVal++;
        } else {

            inkscape_add_document(doc);

            if (sp_vacuum_defs) {
                doc->vacuumDocument();
            }
            
            // Execute command-line actions (selections and verbs) using our local models
            bool has_performed_actions = Inkscape::CmdLineAction::doList(inkscape_active_action_context());

#ifdef WITH_DBUS
            // If we've been asked to listen for D-Bus messages, enter a main loop here
            // The main loop may be exited by calling "exit" on the D-Bus application interface.
            if (sp_dbus_listen) {
                Gtk::Main main_dbus_loop(0, NULL);
                main_dbus_loop.run();
            }
#endif // WITH_DBUS

            if (!sp_export_svg && (sp_vacuum_defs || has_performed_actions)) {
                // save under the name given in the command line
                Inkscape::Extension::save(Inkscape::Extension::db.get("org.inkscape.output.svg.inkscape"), doc, filename, false,
                            false, false, Inkscape::Extension::FILE_SAVE_METHOD_INKSCAPE_SVG);
            }
            if (sp_global_printer) {
                sp_print_document_to_file(doc, sp_global_printer);
            }
            if (sp_export_png || (sp_export_id && sp_export_use_hints)) {
                retVal |= sp_do_export_png(doc);
            }
            if (sp_export_svg) {
                if (sp_export_text_to_path) {
                    GSList *items = NULL;
                    SPRoot *root = doc->getRoot();
                    for ( SPObject *iter = root->firstChild(); iter ; iter = iter->getNext()) {
                        SPItem* item = (SPItem*) iter;
                        if (! (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item) || SP_IS_GROUP(item))) {
                            continue;
                        }

                        te_update_layout_now_recursive(item);
                        items = g_slist_append(items, item);
                    }

                    GSList *selected = NULL;
                    GSList *to_select = NULL;

                    sp_item_list_to_curves(items, &selected, &to_select);

                    g_slist_free (items);
                    g_slist_free (selected);
                    g_slist_free (to_select);
                }
                if(sp_export_id) {
                    doc->ensureUpToDate();

                    // "crop" the document to the specified object, cleaning as we go.
                    SPObject *obj = doc->getObjectById(sp_export_id);
                    Geom::OptRect const bbox(SP_ITEM(obj)->visualBounds());

                    if (bbox) {
                        doc->fitToRect(*bbox, false);
                    }

                    if (sp_export_id_only) {
                        // If -j then remove all other objects to complete the "crop"
                        doc->getRoot()->cropToObject(obj);
                    }
                }

                Inkscape::Extension::save(Inkscape::Extension::db.get("org.inkscape.output.svg.plain"), doc, sp_export_svg, false,
                            false, false, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY);
            }
            if (sp_export_ps) {
                retVal |= do_export_ps_pdf(doc, sp_export_ps, "image/x-postscript");
            }
            if (sp_export_eps) {
                retVal |= do_export_ps_pdf(doc, sp_export_eps, "image/x-e-postscript");
            }
            if (sp_export_pdf) {
                retVal |= do_export_ps_pdf(doc, sp_export_pdf, "application/pdf");
            }
            if (sp_export_emf) {
                retVal |= do_export_emf(doc, sp_export_emf, "image/x-emf");
            }
            if (sp_export_wmf) {
                retVal |= do_export_wmf(doc, sp_export_wmf, "image/x-wmf");
            }
            if (sp_query_all) {
                do_query_all (doc);
            } else if (sp_query_width || sp_query_height) {
                do_query_dimension (doc, true, sp_query_width? Geom::X : Geom::Y, sp_query_id);
            } else if (sp_query_x || sp_query_y) {
                do_query_dimension (doc, false, sp_query_x? Geom::X : Geom::Y, sp_query_id);
            }

            inkscape_remove_document(doc);

            delete doc;
        }
        fl = g_slist_remove(fl, fl->data);
    }
    return retVal;
}

/**
 * Run the application as an interactive shell, parsing command lines from stdin
 * Returns -1 on error.
 */
static int sp_main_shell(char const* command_name)
{
    int retval = 0;

    const unsigned int buffer_size = 4096;
    gchar *command_line = g_strnfill(buffer_size, 0);
    g_strlcpy(command_line, command_name, buffer_size);
    gsize offset = g_strlcat(command_line, " ", buffer_size);
    gsize sizeLeft = buffer_size - offset;
    gchar *useme = command_line + offset;

    fprintf(stdout, "Inkscape %s interactive shell mode. Type 'quit' to quit.\n", Inkscape::version_string);
    fflush(stdout);
    char* linedata = 0;
    do {
        fprintf(stdout, ">");
        fflush(stdout);
        if ((linedata = fgets(useme, sizeLeft, stdin))) {
            size_t len = strlen(useme);
            if ( (len >= sizeLeft - 1) || (useme[len - 1] != '\n') ) {
                fprintf(stdout, "ERROR: Command line too long\n");
                // Consume rest of line
                retval = -1; // If the while loop completes, this remains -1
                while (fgets(useme, sizeLeft, stdin) && retval) {
                    len = strlen(command_line);
                    if ( (len < buffer_size) && (command_line[len-1] == '\n') ) {
                        retval = 0;
                    }
                }
            } else {
                useme[--len] = '\0';  // Strip newline
                if (useme[len - 1] == '\r') {
                    useme[--len] = '\0';
                }
                if ( strcmp(useme, "quit") == 0 ) {
                    // Time to quit
                    fflush(stdout);
                    linedata = 0; // mark for exit
                } else if ( len < 1 ) {
                    // blank string. Do nothing.
                } else {
                    GError* parseError = 0;
                    gchar** argv = 0;
                    gint argc = 0;
                    if ( g_shell_parse_argv(command_line, &argc, &argv, &parseError) ) {
                        poptContext ctx = poptGetContext(NULL, argc, const_cast<const gchar**>(argv), options, 0);
                        poptSetOtherOptionHelp(ctx, _("[OPTIONS...] [FILE...]\n\nAvailable options:"));
                        if ( ctx ) {
                            GSList *fl = sp_process_args(ctx);
                            if (sp_process_file_list(fl)) {
                                retval = -1;
                            }
                            poptFreeContext(ctx);
                        } else {
                            retval = 1; // not sure why. But this was the previous return value
                        }
                        resetCommandlineGlobals();
                        g_strfreev(argv);
                    } else {
                        g_warning("Cannot parse commandline: %s", useme);
                        retval = -1;
                    }
                }
            }
        } // if (linedata...
    } while (linedata && (retval == 0));

    g_free(command_line);
    return retval;
}

int sp_main_console(int argc, char const **argv)
{
    /* We are started in text mode */

#if !GLIB_CHECK_VERSION(2,36,0)
    /* Do this g_type_init(), so that we can use Xft/Freetype2 (Pango)
     * in a non-Gtk environment.  Used in libnrtype's
     * FontInstance.cpp and FontFactory.cpp.
     * http://mail.gnome.org/archives/gtk-list/2003-December/msg00063.html
     */
    g_type_init();
#endif

    char **argv2 = const_cast<char **>(argv);
    gtk_init_check( &argc, &argv2 );
    //setlocale(LC_ALL, "");

    GSList *fl = NULL;
    int retVal = sp_common_main( argc, argv, &fl );
    g_return_val_if_fail(retVal == 0, 1);

    if (fl == NULL && !sp_shell
#ifdef WITH_DBUS
        && !sp_dbus_listen
#endif // WITH_DBUS
        ) {
        g_print("Nothing to do!\n");
        exit(0);
    }

    inkscape_application_init(argv[0], false);

    if (sp_shell) {
        int retVal = sp_main_shell(argv[0]); // Run as interactive shell
        exit((retVal < 0) ? 1 : 0);
    } else {
        int retVal = sp_process_file_list(fl); // Normal command line invokation
        if (retVal){
            exit(1);
        }
    }

    return 0;
}

static void
do_query_dimension (SPDocument *doc, bool extent, Geom::Dim2 const axis, const gchar *id)
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
        o = doc->getRoot();
    }

    if (o) {
        doc->ensureUpToDate();
        SPItem *item = ((SPItem *) o);

        // visual bbox in document coords for scripting
        Geom::OptRect area = item->documentVisualBounds();
        if (area) {
            Inkscape::SVGOStringStream os;
            if (extent) {
                os << area->dimensions()[axis];
            } else {
                os << area->min()[axis];
            }
            g_print ("%s", os.str().c_str());
        } else {
            g_print("0");
        }
    }
}

static void do_query_all(SPDocument *doc)
{
    SPObject *o = doc->getRoot();

    if (o) {
        doc->ensureUpToDate();
        do_query_all_recurse(o);
    }
}

static void
do_query_all_recurse (SPObject *o)
{
    SPItem *item = ((SPItem *) o);
    if (o->getId() && SP_IS_ITEM(item)) {
        Geom::OptRect area = item->documentVisualBounds();
        if (area) {
            Inkscape::SVGOStringStream os;
            os << o->getId();
            os << "," << area->min()[Geom::X];
            os << "," << area->min()[Geom::Y];
            os << "," << area->dimensions()[Geom::X];
            os << "," << area->dimensions()[Geom::Y];
            g_print ("%s\n", os.str().c_str());
        }
    }

    SPObject *child = o->children;
    while (child) {
        do_query_all_recurse (child);
        child = child->next;
    }
}


static int sp_do_export_png(SPDocument *doc)
{
    Glib::ustring filename;
    bool filename_from_hint = false;
    gdouble dpi = 0.0;

    if (sp_export_use_hints && (!sp_export_id && !sp_export_area_drawing)) {
        g_warning ("--export-use-hints can only be used with --export-id or --export-area-drawing; ignored.");
    }

    GSList *items = NULL;

    Geom::Rect area;
    if (sp_export_id || sp_export_area_drawing) {

        SPObject *o = NULL;
        SPObject *o_area = NULL;
        if (sp_export_id && sp_export_area_drawing) {
            o = doc->getObjectById(sp_export_id);
            o_area = doc->getRoot();
        } else if (sp_export_id) {
            o = doc->getObjectById(sp_export_id);
            o_area = o;
        } else if (sp_export_area_drawing) {
            o = doc->getRoot();
            o_area = o;
        }

        if (o) {
            if (!SP_IS_ITEM (o)) {
                g_warning("Object with id=\"%s\" is not a visible item. Nothing exported.", sp_export_id);
                return 1;
            }

            items = g_slist_prepend (items, SP_ITEM(o));

            if (sp_export_id_only) {
                g_print("Exporting only object with id=\"%s\"; all other objects hidden\n", sp_export_id);
            }

            if (sp_export_use_hints) {

                // retrieve export filename hint
                const gchar *fn_hint = o->getRepr()->attribute("inkscape:export-filename");
                if (fn_hint) {
                    if (sp_export_png) {
                        g_warning ("Using export filename from the command line (--export-png). Filename hint %s is ignored.", fn_hint);
                        filename = sp_export_png;
                    } else {
                        filename = fn_hint;
                        filename_from_hint = true;
                    }
                } else {
                    g_warning ("Export filename hint not found for the object.");
                    filename = sp_export_png;
                }

                // retrieve export dpi hints
                const gchar *dpi_hint = o->getRepr()->attribute("inkscape:export-xdpi"); // only xdpi, ydpi is always the same now
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
            doc->ensureUpToDate();
            Geom::OptRect areaMaybe = static_cast<SPItem *>(o_area)->desktopVisualBounds();
            if (areaMaybe) {
                area = *areaMaybe;
            } else {
                g_warning("Unable to determine a valid bounding box. Nothing exported.");
                return 1;
            }
        } else {
            g_warning("Object with id=\"%s\" was not found in the document. Nothing exported.", sp_export_id);
            return 1;
        }
    }

    if (sp_export_area) {
        /* Try to parse area (given in SVG pixels) */
        gdouble x0,y0,x1,y1;
        if (sscanf(sp_export_area, "%lg:%lg:%lg:%lg", &x0, &y0, &x1, &y1) != 4) {
            g_warning("Cannot parse export area '%s'; use 'x0:y0:x1:y1'. Nothing exported.", sp_export_area);
            return 1;
        }
        area = Geom::Rect(Geom::Interval(x0,x1), Geom::Interval(y0,y1));
    } else if (sp_export_area_page || !(sp_export_id || sp_export_area_drawing)) {
        /* Export the whole page: note: Inkscape uses 'page' in all menus and dialogs, not 'canvas' */
        doc->ensureUpToDate();
        Geom::Point origin(doc->getRoot()->x.computed, doc->getRoot()->y.computed);
        area = Geom::Rect(origin, origin + doc->getDimensions());
    }

    // set filename and dpi from options, if not yet set from the hints
    if (filename.empty()) {
        if (!sp_export_png || sp_export_png[0] == '\0') {
            g_warning ("No export filename given and no filename hint. Nothing exported.");
            return 1;
        }
        filename = sp_export_png;
    }

    if (sp_export_dpi && dpi == 0.0) {
        dpi = atof(sp_export_dpi);
        if ((dpi < 0.1) || (dpi > 10000.0)) {
            g_warning("DPI value %s out of range [0.1 - 10000.0]. Nothing exported.", sp_export_dpi);
            return 1;
        }
        g_print("DPI: %g\n", dpi);
    }

    if (sp_export_area_snap) {
        round_rectangle_outwards(area);
    }

    // default dpi
    if (dpi == 0.0) {
        dpi = Inkscape::Util::Quantity::convert(1, "in", "px");
    }

    unsigned long int width = 0;
    unsigned long int height = 0;

    if (sp_export_width) {
        errno=0;
        width = strtoul(sp_export_width, NULL, 0);
        if ((width < 1) || (width > PNG_UINT_31_MAX) || (errno == ERANGE) ) {
            g_warning("Export width %lu out of range (1 - %lu). Nothing exported.", width, (unsigned long int)PNG_UINT_31_MAX);
            return 1;
        }
        dpi = (gdouble) Inkscape::Util::Quantity::convert(width, "in", "px") / area.width();
    }

    if (sp_export_height) {
        errno=0;
        height = strtoul(sp_export_height, NULL, 0);
        if ((height < 1) || (height > PNG_UINT_31_MAX)) {
            g_warning("Export height %lu out of range (1 - %lu). Nothing exported.", height, (unsigned long int)PNG_UINT_31_MAX);
            return 1;
        }
        dpi = (gdouble) Inkscape::Util::Quantity::convert(height, "in", "px") / area.height();
    }

    if (!sp_export_width) {
        width = (unsigned long int) (Inkscape::Util::Quantity::convert(area.width(), "px", "in") * dpi + 0.5);
    }

    if (!sp_export_height) {
        height = (unsigned long int) (Inkscape::Util::Quantity::convert(area.height(), "px", "in") * dpi + 0.5);
    }

    guint32 bgcolor = 0x00000000;
    if (sp_export_background) {
        // override the page color
        bgcolor = sp_svg_read_color(sp_export_background, 0xffffff00);
        bgcolor |= 0xff; // default is no opacity
    } else {
        // read from namedview
        Inkscape::XML::Node *nv = sp_repr_lookup_name (doc->rroot, "sodipodi:namedview");
        if (nv && nv->attribute("pagecolor")){
            bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
        }
        if (nv && nv->attribute("inkscape:pageopacity")){
            double opacity = 1.0;
            sp_repr_get_double (nv, "inkscape:pageopacity", &opacity);
            bgcolor |= SP_COLOR_F_TO_U(opacity);
        }
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

    Glib::ustring path;
    if (filename_from_hint) {
        //Make relative paths go from the document location, if possible:
        if (!Glib::path_is_absolute(filename) && doc->getURI()) {
            Glib::ustring dirname = Glib::path_get_dirname(doc->getURI());
            if (!dirname.empty()) {
                path = Glib::build_filename(dirname, filename);
            }
        }
        if (path.empty()) {
            path = filename;
        }
    } else {
        path = filename;
    }

    int retcode = 0;
    //check if specified directory exists

    if (!Inkscape::IO::file_directory_exists(filename.c_str())) {
        g_warning("File path \"%s\" includes directory that doesn't exist.\n", filename.c_str());
        retcode = 1;
    } else {
        g_print("Background RRGGBBAA: %08x\n", bgcolor);

        g_print("Area %g:%g:%g:%g exported to %lu x %lu pixels (%g dpi)\n", area[Geom::X][0], area[Geom::Y][0], area[Geom::X][1], area[Geom::Y][1], width, height, dpi);

        if ((width >= 1) && (height >= 1) && (width <= PNG_UINT_31_MAX) && (height <= PNG_UINT_31_MAX)) {
            if( sp_export_png_file(doc, path.c_str(), area, width, height, dpi,
              dpi, bgcolor, NULL, NULL, true, sp_export_id_only ? items : NULL) == 1 ) {
                g_print("Bitmap saved as: %s\n", filename.c_str());
            } else {
                g_warning("Bitmap failed to save to: %s", filename.c_str());
            }
        } else {
            g_warning("Calculated bitmap dimensions %lu %lu are out of range (1 - %lu). Nothing exported.", width, height, (unsigned long int)PNG_UINT_31_MAX);
        }
    }

    g_slist_free (items);
    return retcode;
}


/**
 *  Perform a PDF/PS/EPS export
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as.
 */

static int do_export_ps_pdf(SPDocument* doc, gchar const* uri, char const* mime)
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
        return 1;
    }

    if (sp_export_id) {
        SPObject *o = doc->getObjectById(sp_export_id);
        if (o == NULL) {
            g_warning("Object with id=\"%s\" was not found in the document. Nothing exported.", sp_export_id);
            return 1;
        }
        (*i)->set_param_string ("exportId", sp_export_id);
    } else {
        (*i)->set_param_string ("exportId", "");
    }

    if (sp_export_area_page && sp_export_area_drawing) {
        g_warning ("You cannot use --export-area-page and --export-area-drawing at the same time; only the former will take effect.");
        sp_export_area_drawing = false;
    }

    if (sp_export_area_drawing) {
        (*i)->set_param_optiongroup ("area", "drawing");
    }

    if (sp_export_area_page) {
        if (sp_export_eps) {
            g_warning ("EPS cannot have its bounding box extend beyond its content, so if your drawing is smaller than the page, --export-area-page will clip it to drawing.");
        }
        (*i)->set_param_optiongroup ("area", "page");
    }

    if (!sp_export_area_drawing && !sp_export_area_page && !sp_export_id) {
        // neither is set, set page as default for ps/pdf and drawing for eps
        if (sp_export_eps) {
            try {
               (*i)->set_param_optiongroup("area", "drawing");
            } catch (...) {}
        }
    }

    if (sp_export_text_to_path) {
        (*i)->set_param_bool("textToPath", TRUE);
    } else {
        (*i)->set_param_bool("textToPath", FALSE);
    }

    if (sp_export_latex) {
        (*i)->set_param_bool("textToLaTeX", TRUE);
    } else {
        (*i)->set_param_bool("textToLaTeX", FALSE);
    }

    if (sp_export_ignore_filters) {
        (*i)->set_param_bool("blurToBitmap", FALSE);
    } else {
        (*i)->set_param_bool("blurToBitmap", TRUE);

        gdouble dpi = 90.0;
        if (sp_export_dpi) {
            dpi = atof(sp_export_dpi);
            if ((dpi < 1) || (dpi > 10000.0)) {
                g_warning("DPI value %s out of range [1 - 10000]. Using 90 dpi instead.", sp_export_dpi);
                dpi = 90;
            }
        }

        (*i)->set_param_int("resolution", (int) dpi);
    }

    // if no bleed/margin is given, set to 0  (otherwise it takes the value last used from the UI)
    float margin = 0.;
    if (sp_export_margin) {
        margin = g_ascii_strtod(sp_export_margin, NULL);
    }
    (*i)->set_param_float("bleed", margin);

    // handle --export-pdf-version
    if (g_strcmp0(mime, "application/pdf") == 0) {
        bool set_export_pdf_version_fail=true;
        const gchar *pdfver_param_name="PDFversion";
        if(sp_export_pdf_version) {
            // combine "PDF " and the given command line
            std::string version_gui_string=std::string("PDF ")+sp_export_pdf_version;
            try{
                // first, check if the given pdf version is selectable in the ComboBox
                if((*i)->get_param_enum_contains("PDFversion", version_gui_string.c_str())) {
                    (*i)->set_param_enum(pdfver_param_name, version_gui_string.c_str());
                    set_export_pdf_version_fail=false;
                } else {
                    g_warning("Desired PDF export version \"%s\" not supported! Hint: input one of the versions found in the pdf export dialog e.g. \"1.4\".",
                              sp_export_pdf_version);
                }
            } catch (...) {
                // can be thrown along the way:
                // throw Extension::param_not_exist();
                // throw Extension::param_not_enum_param();
                g_warning("Parameter or Enum \"%s\" might not exist",pdfver_param_name);
            }
        }

        // set default pdf export version to 1.4, also if something went wrong
        if(set_export_pdf_version_fail) {
            (*i)->set_param_enum(pdfver_param_name, "PDF 1.4");
        }
    }

    if(!uri || uri[0] == '\0') {
        g_warning ("No export filename given. Nothing exported.");
        return 0;
    }

    //check if specified directory exists
    if (!Inkscape::IO::file_directory_exists(uri)) {
        g_warning("File path \"%s\" includes directory that doesn't exist.\n", uri);
        return 1;
    }

    if ( g_strcmp0(mime, "image/x-postscript") == 0
         || g_strcmp0(mime, "image/x-e-postscript") == 0 ) {
        if ( sp_export_ps_level < 2 || sp_export_ps_level > 3 ) {
            g_warning("Only supported PostScript levels are 2 and 3."
                      " Defaulting to 2.");
            sp_export_ps_level = 2;
        }

        (*i)->set_param_enum("PSlevel", (sp_export_ps_level == 3)
                             ? "PostScript level 3" : "PostScript level 2");
    }

    try {
        (*i)->save(doc, uri);
    } catch(...) {
        g_warning("Failed to save pdf to: %s", uri);
    }
    return 0;
}

/**
 *  Export a document to EMF or WMF 
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as (should be "image/x-emf" or "image/x-wmf")
 */

static int do_export_win_metafile_common(SPDocument* doc, gchar const* uri, char const* mime)
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
        return 1;
    }

    //check if specified directory exists
    if (!Inkscape::IO::file_directory_exists(uri)){
        g_warning("File path \"%s\" includes directory that doesn't exist.\n",
        uri);
        return 1;
    }

    (*i)->save(doc, uri);
    return 0;
}

/**
 *  Export a document to EMF
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as (should be "image/x-emf")
 */

static int do_export_emf(SPDocument* doc, gchar const* uri, char const* mime)
{
    if(!uri || uri[0] == '\0') {
        g_warning("No filename provided for emf export.");
        return 0;
    }
    return do_export_win_metafile_common(doc, uri, mime);
}

/**
 *  Export a document to WMF
 *
 *  \param doc Document to export.
 *  \param uri URI to export to.
 *  \param mime MIME type to export as (should be "image/x-wmf")
 */

static int do_export_wmf(SPDocument* doc, gchar const* uri, char const* mime)
{
    if(!uri || uri[0] == '\0') {
        g_warning("No filename provided for wmf export.");
        return 0;
    }
    return do_export_win_metafile_common(doc, uri, mime);
}

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
                    WIN32_FIND_DATAA data;
#else
                    WIN32_FIND_DATAW data;
#endif // REPLACEARGS_ANSI

                    memset((void *)&data, 0, sizeof(data));

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
    while ((a = poptGetNextOpt(ctx)) != -1) {
        switch (a) {
            case SP_ARG_FILE: {
                gchar const *fn = poptGetOptArg(ctx);
                if (fn != NULL) {
                    fl = g_slist_append(fl, g_strdup(fn));
                }
                break;
            }
            case SP_ARG_VERSION: {
                printf("Inkscape %s (%s)\n", Inkscape::version_string, __DATE__);
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
            case POPT_ERROR_BADOPT: {
                g_warning ("Invalid option %s", poptBadOption(ctx, 0));
                exit(1);
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

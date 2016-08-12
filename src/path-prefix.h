/*
 * Separate the inkscape paths from the prefix code, as that is kind of
 * a separate package (binreloc)
 * 	http://autopackage.org/downloads.html
 *
 * Since the directories set up by autoconf end up in config.h, we can't
 * _change_ them, since config.h isn't protected by a set of
 * one-time-include directives and is repeatedly re-included by some
 * chains of .h files.  As a result, nothing should refer to those
 * define'd directories, and instead should use only the paths defined here.
 *
 */
#ifndef SEEN_PATH_PREFIX_H
#define SEEN_PATH_PREFIX_H

#include "require-config.h"  // INKSCAPE_DATADIR
#include "prefix.h"

//#ifdef __cplusplus
//extern "C" {
//#endif /* __cplusplus */

#ifdef ENABLE_BINRELOC
/* The way that we're building now is with a shared library between Inkscape
   and Inkview, and the code will find the path to the library then. But we
   don't really want that. This prefix then pulls things out of the lib directory
   and back into the root install dir. */
#  define INKSCAPE_LIBPREFIX      "/../.."
#  define INKSCAPE_APPICONDIR     BR_DATADIR( INKSCAPE_LIBPREFIX "/share/pixmaps" )
#  define INKSCAPE_ATTRRELDIR     BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/attributes" )
#  define INKSCAPE_BINDDIR        BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/bind" )
#  define INKSCAPE_EXAMPLESDIR    BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/examples" )
#  define INKSCAPE_EXTENSIONDIR   BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/extensions" )
#  define INKSCAPE_FILTERDIR      BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/filters" )
#  define INKSCAPE_GRADIENTSDIR   BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/gradients" )
#  define INKSCAPE_KEYSDIR        BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/keys" )
#  define INKSCAPE_PIXMAPDIR      BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/icons" )
#  define INKSCAPE_MARKERSDIR     BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/markers" )
#  define INKSCAPE_PALETTESDIR    BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/palettes" )
#  define INKSCAPE_PATTERNSDIR    BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/patterns" )
#  define INKSCAPE_SCREENSDIR     BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/screens" )
#  define INKSCAPE_SYMBOLSDIR     BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/symbols" )
#  define INKSCAPE_THEMEDIR       BR_DATADIR( INKSCAPE_LIBPREFIX "/share/icons" )
#  define INKSCAPE_TUTORIALSDIR   BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/tutorials" )
#  define INKSCAPE_TEMPLATESDIR   BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/templates" )
#  define INKSCAPE_UIDIR          BR_DATADIR( INKSCAPE_LIBPREFIX "/share/inkscape/ui" )
//CREATE V0.1 support
#    define CREATE_GRADIENTSDIR   BR_DATADIR( INKSCAPE_LIBPREFIX "/share/create/gradients/gimp" )
#    define CREATE_PALETTESDIR    BR_DATADIR( INKSCAPE_LIBPREFIX "/share/create/swatches" )
#    define CREATE_PATTERNSDIR    BR_DATADIR( INKSCAPE_LIBPREFIX "/share/create/patterns/vector" )
#else
#  ifdef WIN32
#    define INKSCAPE_APPICONDIR   WIN32_DATADIR("pixmaps")
#    define INKSCAPE_ATTRRELDIR   WIN32_DATADIR("share\\attributes")
#    define INKSCAPE_BINDDIR      WIN32_DATADIR("share\\bind")
#    define INKSCAPE_EXAMPLESDIR  WIN32_DATADIR("share\\examples")
#    define INKSCAPE_EXTENSIONDIR WIN32_DATADIR("share\\extensions")
#    define INKSCAPE_FILTERDIR    WIN32_DATADIR("share\\filters")
#    define INKSCAPE_GRADIENTSDIR WIN32_DATADIR("share\\gradients")
#    define INKSCAPE_KEYSDIR      WIN32_DATADIR("share\\keys")
#    define INKSCAPE_PIXMAPDIR    WIN32_DATADIR("share\\icons")
#    define INKSCAPE_MARKERSDIR   WIN32_DATADIR("share\\markers")
#    define INKSCAPE_PALETTESDIR  WIN32_DATADIR("share\\palettes")
#    define INKSCAPE_PATTERNSDIR  WIN32_DATADIR("share\\patterns")
#    define INKSCAPE_SCREENSDIR   WIN32_DATADIR("share\\screens")
#    define INKSCAPE_SYMBOLSDIR   WIN32_DATADIR("share\\symbols")
#    define INKSCAPE_TUTORIALSDIR WIN32_DATADIR("share\\tutorials")
#    define INKSCAPE_TEMPLATESDIR WIN32_DATADIR("share\\templates")
#    define INKSCAPE_UIDIR        WIN32_DATADIR("share\\ui")
//CREATE V0.1  WIN32 support
#    define CREATE_GRADIENTSDIR   WIN32_DATADIR("create\\gradients\\gimp")
#    define CREATE_PALETTESDIR    WIN32_DATADIR("create\\swatches")
#    define CREATE_PATTERNSDIR    WIN32_DATADIR("create\\patterns\\vector")
#  elif defined ENABLE_OSX_APP_LOCATIONS
#    define INKSCAPE_APPICONDIR   "Contents/Resources/share/pixmaps"
#    define INKSCAPE_ATTRRELDIR   "Contents/Resources/share/inkscape/attributes"
#    define INKSCAPE_BINDDIR      "Contents/Resources/share/inkscape/bind"
#    define INKSCAPE_EXAMPLESDIR  "Contents/Resources/share/inkscape/examples"
#    define INKSCAPE_EXTENSIONDIR "Contents/Resources/share/inkscape/extensions"
#    define INKSCAPE_FILTERDIR    "Contents/Resources/share/inkscape/filters"
#    define INKSCAPE_GRADIENTSDIR "Contents/Resources/share/inkscape/gradients"
#    define INKSCAPE_KEYSDIR      "Contents/Resources/share/inkscape/keys"
#    define INKSCAPE_PIXMAPDIR    "Contents/Resources/share/inkscape/icons"
#    define INKSCAPE_MARKERSDIR   "Contents/Resources/share/inkscape/markers"
#    define INKSCAPE_PALETTESDIR  "Contents/Resources/share/inkscape/palettes"
#    define INKSCAPE_PATTERNSDIR  "Contents/Resources/share/inkscape/patterns"
#    define INKSCAPE_SCREENSDIR   "Contents/Resources/share/inkscape/screens"
#    define INKSCAPE_SYMBOLSDIR   "Contents/Resources/share/inkscape/symbols"
#    define INKSCAPE_TUTORIALSDIR "Contents/Resources/share/inkscape/tutorials"
#    define INKSCAPE_TEMPLATESDIR "Contents/Resources/share/inkscape/templates"
#    define INKSCAPE_UIDIR        "Contents/Resources/share/inkscape/ui"
//CREATE V0.1 support
#    define CREATE_GRADIENTSDIR  "/Library/Application Support/create/gradients/gimp"
#    define CREATE_PALETTESDIR   "/Library/Application Support/create/swatches"
#    define CREATE_PATTERNSDIR   "/Library/Application Support/create/patterns/vector"
#  else
#    define INKSCAPE_APPICONDIR   INKSCAPE_DATADIR "/pixmaps"
#    define INKSCAPE_ATTRRELDIR   INKSCAPE_DATADIR "/inkscape/attributes"
#    define INKSCAPE_BINDDIR      INKSCAPE_DATADIR "/inkscape/bind"
#    define INKSCAPE_EXAMPLESDIR  INKSCAPE_DATADIR "/inkscape/examples"
#    define INKSCAPE_EXTENSIONDIR INKSCAPE_DATADIR "/inkscape/extensions"
#    define INKSCAPE_FILTERDIR    INKSCAPE_DATADIR "/inkscape/filters"
#    define INKSCAPE_GRADIENTSDIR INKSCAPE_DATADIR "/inkscape/gradients"
#    define INKSCAPE_KEYSDIR      INKSCAPE_DATADIR "/inkscape/keys"
#    define INKSCAPE_PIXMAPDIR    INKSCAPE_DATADIR "/inkscape/icons"
#    define INKSCAPE_MARKERSDIR   INKSCAPE_DATADIR "/inkscape/markers"
#    define INKSCAPE_PALETTESDIR  INKSCAPE_DATADIR "/inkscape/palettes"
#    define INKSCAPE_PATTERNSDIR  INKSCAPE_DATADIR "/inkscape/patterns"
#    define INKSCAPE_SCREENSDIR   INKSCAPE_DATADIR "/inkscape/screens"
#    define INKSCAPE_SYMBOLSDIR   INKSCAPE_DATADIR "/inkscape/symbols"
#    define INKSCAPE_THEMEDIR     INKSCAPE_DATADIR "/icons"
#    define INKSCAPE_TUTORIALSDIR INKSCAPE_DATADIR "/inkscape/tutorials"
#    define INKSCAPE_TEMPLATESDIR INKSCAPE_DATADIR "/inkscape/templates"
#    define INKSCAPE_UIDIR        INKSCAPE_DATADIR "/inkscape/ui"
//CREATE V0.1 support
#    define CREATE_GRADIENTSDIR INKSCAPE_DATADIR "/create/gradients/gimp"
#    define CREATE_PALETTESDIR  INKSCAPE_DATADIR "/create/swatches"
#    define CREATE_PATTERNSDIR  INKSCAPE_DATADIR "/create/patterns/vector"
#	 endif
#endif

//#ifdef __cplusplus
//}
//#endif /* __cplusplus */

#endif /* _PATH_PREFIX_H_ */

/*
 * This is what gets executed to initialize all of the modules.  For
 * the internal modules this involves executing their initialization
 * functions, for external ones it involves reading their .spmodule
 * files and bringing them into Sodipodi.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_POPPLER
# include "internal/pdfinput/pdf-input.h"
#endif

#include "path-prefix.h"

#include "inkscape.h"

#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>
#include <glibmm/ustring.h>

#include "system.h"
#include "db.h"
#include "internal/svgz.h"
# include "internal/emf-inout.h"
# include "internal/emf-print.h"
# include "internal/wmf-inout.h"
# include "internal/wmf-print.h"
#ifdef HAVE_CAIRO_PDF
# include "internal/cairo-renderer-pdf-out.h"
# include "internal/cairo-png-out.h"
# include "internal/cairo-ps-out.h"
#endif
#include "internal/pov-out.h"
#include "internal/javafx-out.h"
#include "internal/odf.h"
#include "internal/latex-pstricks-out.h"
#include "internal/latex-pstricks.h"
#include "internal/gdkpixbuf-input.h"
#include "internal/bluredge.h"
#include "internal/gimpgrad.h"
#include "internal/grid.h"
#ifdef WITH_LIBWPG
#include "internal/wpg-input.h"
#endif
#ifdef WITH_LIBVISIO
#include "internal/vsd-input.h"
#endif
#ifdef WITH_LIBCDR
#include "internal/cdr-input.h"
#endif
#include "preferences.h"
#include "io/sys.h"
#ifdef WITH_DBUS
#include "dbus/dbus-init.h"
#endif

#ifdef WITH_IMAGE_MAGICK
#include "internal/bitmap/adaptiveThreshold.h"
#include "internal/bitmap/addNoise.h"
#include "internal/bitmap/blur.h"
#include "internal/bitmap/channel.h"
#include "internal/bitmap/charcoal.h"
#include "internal/bitmap/colorize.h"
#include "internal/bitmap/contrast.h"
#include "internal/bitmap/crop.h"
#include "internal/bitmap/cycleColormap.h"
#include "internal/bitmap/despeckle.h"
#include "internal/bitmap/edge.h"
#include "internal/bitmap/emboss.h"
#include "internal/bitmap/enhance.h"
#include "internal/bitmap/equalize.h"
#include "internal/bitmap/gaussianBlur.h"
#include "internal/bitmap/implode.h"
#include "internal/bitmap/level.h"
#include "internal/bitmap/levelChannel.h"
#include "internal/bitmap/medianFilter.h"
#include "internal/bitmap/modulate.h"
#include "internal/bitmap/negate.h"
#include "internal/bitmap/normalize.h"
#include "internal/bitmap/oilPaint.h"
#include "internal/bitmap/opacity.h"
#include "internal/bitmap/raise.h"
#include "internal/bitmap/reduceNoise.h"
#include "internal/bitmap/sample.h"
#include "internal/bitmap/shade.h"
#include "internal/bitmap/sharpen.h"
#include "internal/bitmap/solarize.h"
#include "internal/bitmap/spread.h"
#include "internal/bitmap/swirl.h"
//#include "internal/bitmap/threshold.h"
#include "internal/bitmap/unsharpmask.h"
#include "internal/bitmap/wave.h"
#endif /* WITH_IMAGE_MAGICK */

#include "internal/filter/filter.h"

#include "init.h"

namespace Inkscape {
namespace Extension {

/** This is the extention that all files are that are pulled from
    the extension directory and parsed */
#define SP_MODULE_EXTENSION  "inx"

static void build_module_from_dir(gchar const *dirname);
static void check_extensions();

/**
 * \return    none
 * \brief     Examines the given string preference and checks to see
 *            that at least one of the registered extensions matches
 *            it.  If not, a default is assigned.
 * \param     pref_path        Preference path to update
 * \param     pref_default     Default string to set
 * \param     extension_family List of extensions to search
 */
static void
update_pref(Glib::ustring const &pref_path,
            gchar const *pref_default) // , GSList *extension_family)
{
    Glib::ustring pref = Inkscape::Preferences::get()->getString(pref_path);
    /*
    gboolean missing=TRUE;
    for (GSList *list = extension_family; list; list = g_slist_next(list)) {
        g_assert( list->data );

        Inkscape::Extension *extension;
        extension = reinterpret_cast<Inkscape::Extension *>(list->data);

        if (!strcmp(extension->get_id(),pref)) missing=FALSE;
    }
    */
    if (!Inkscape::Extension::db.get( pref.data() ) /*missing*/) {
        Inkscape::Preferences::get()->setString(pref_path, pref_default);
    }
}

/**
 * Invokes the init routines for internal modules.
 *
 * This should be a list of all the internal modules that need to initialized.  This is just a
 * convinent place to put them.  Also, this function calls build_module_from_dir to parse the
 * Inkscape extensions directory.
 */
void
init()
{
    /* TODO: Change to Internal */
    Internal::Svg::init();
    Internal::Svgz::init();

#ifdef HAVE_CAIRO_PDF
    //g_print ("Using CairoRendererPdfOutput: new pdf exporter\n");
    Internal::CairoRendererPdfOutput::init();
    Internal::CairoRendererOutput::init();

    Internal::CairoPsOutput::init();
    Internal::CairoEpsOutput::init();
#endif
#ifdef HAVE_POPPLER
    Internal::PdfInput::init();
#endif
    Internal::PrintEmf::init();
    Internal::Emf::init();
    Internal::PrintWmf::init();
    Internal::Wmf::init();
    Internal::PovOutput::init();
    Internal::JavaFXOutput::init();
    Internal::OdfOutput::init();
    Internal::PrintLatex::init();
    Internal::LatexOutput::init();
#ifdef WITH_LIBWPG
    Internal::WpgInput::init();
#endif
#ifdef WITH_LIBVISIO
    Internal::VsdInput::init();
#endif
#ifdef WITH_LIBCDR
    Internal::CdrInput::init();
#endif

    /* Effects */
    Internal::BlurEdge::init();
    Internal::GimpGrad::init();
    Internal::Grid::init();

#ifdef WITH_DBUS
    Dbus::init();
#endif

    /* Raster Effects */
#ifdef WITH_IMAGE_MAGICK
    Internal::Bitmap::AdaptiveThreshold::init();
    Internal::Bitmap::AddNoise::init();
    Internal::Bitmap::Blur::init();
    Internal::Bitmap::Channel::init();
    Internal::Bitmap::Charcoal::init();
    Internal::Bitmap::Colorize::init();
    Internal::Bitmap::Contrast::init();
    Internal::Bitmap::Crop::init();
    Internal::Bitmap::CycleColormap::init();
    Internal::Bitmap::Edge::init();
    Internal::Bitmap::Despeckle::init();
    Internal::Bitmap::Emboss::init();
    Internal::Bitmap::Enhance::init();
    Internal::Bitmap::Equalize::init();
    Internal::Bitmap::GaussianBlur::init();
    Internal::Bitmap::Implode::init();
    Internal::Bitmap::Level::init();
    Internal::Bitmap::LevelChannel::init();
    Internal::Bitmap::MedianFilter::init();
    Internal::Bitmap::Modulate::init();
    Internal::Bitmap::Negate::init();
    Internal::Bitmap::Normalize::init();
    Internal::Bitmap::OilPaint::init();
    Internal::Bitmap::Opacity::init();
    Internal::Bitmap::Raise::init();
    Internal::Bitmap::ReduceNoise::init();
    Internal::Bitmap::Sample::init();
    Internal::Bitmap::Shade::init();
    Internal::Bitmap::Sharpen::init();
    Internal::Bitmap::Solarize::init();
    Internal::Bitmap::Spread::init();
    Internal::Bitmap::Swirl::init();
    //Internal::Bitmap::Threshold::init();
    Internal::Bitmap::Unsharpmask::init();
    Internal::Bitmap::Wave::init();
#endif /* WITH_IMAGE_MAGICK */

    Internal::Filter::Filter::filters_all();

    /* Load search path for extensions */
    if (Inkscape::Extension::Extension::search_path.size() == 0)
    {
        Inkscape::Extension::Extension::search_path.push_back(Inkscape::Application::profile_path("extensions"));

        Inkscape::Extension::Extension::search_path.push_back(g_strdup(INKSCAPE_EXTENSIONDIR));

    }

    for (unsigned int i=0; i<Inkscape::Extension::Extension::search_path.size(); i++) {
        build_module_from_dir(Inkscape::Extension::Extension::search_path[i]);
    }

    /* this is at the very end because it has several catch-alls
     * that are possibly over-ridden by other extensions (such as
     * svgz)
     */
    Internal::GdkpixbufInput::init();

    /* now we need to check and make sure everyone is happy */
    check_extensions();

    /* This is a hack to deal with updating saved outdated module
     * names in the prefs...
     */
    update_pref("/dialogs/save_as/default",
                SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE
                // Inkscape::Extension::db.get_output_list()
        );
}

/**
 * \return    none
 * \brief     This function parses a directory for files of SP_MODULE_EXTENSION
 *            type and loads them.
 * \param     dirname  The directory that should be searched for modules
 *
 * Here is just a basic function that moves through a directory.  It looks at every entry, and
 * compares its filename with SP_MODULE_EXTENSION.  Of those that pass, build_from_file is called
 * with their filenames.
 */
static void
build_module_from_dir(gchar const *dirname)
{
    if (!dirname) {
        g_warning("%s", _("Null external module directory name.  Modules will not be loaded."));
        return;
    }

    if (!Glib::file_test(std::string(dirname), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR)) {
        return;
    }

    //# Hopefully doing this the Glib way is portable

    GError *err;
    GDir *directory = g_dir_open(dirname, 0, &err);
    if (!directory) {
        gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
        g_warning(_("Modules directory (%s) is unavailable.  External modules in that directory will not be loaded."), safeDir);
        g_free(safeDir);
        return;
    }

    gchar *filename;
    while ((filename = (gchar *)g_dir_read_name(directory)) != NULL) {
        if (strlen(filename) < strlen(SP_MODULE_EXTENSION)) {
            continue;
        }

        if (strcmp(SP_MODULE_EXTENSION, filename + (strlen(filename) - strlen(SP_MODULE_EXTENSION)))) {
            continue;
        }

        gchar *pathname = g_build_filename(dirname, filename, (char *) NULL);
        build_from_file(pathname);
        g_free(pathname);
    }

    g_dir_close(directory);

	return;
}


static void
check_extensions_internal(Extension *in_plug, gpointer in_data)
{
    int *count = (int *)in_data;

    if (in_plug == NULL) return;
    if (!in_plug->deactivated() && !in_plug->check()) {
         in_plug->deactivate();
        (*count)++;
    }
}

static void check_extensions()
{
    int count = 1;

    Inkscape::Extension::Extension::error_file_open();
    while (count != 0) {
        count = 0;
        db.foreach(check_extensions_internal, (gpointer)&count);
    }
    Inkscape::Extension::Extension::error_file_close();
}

} } /* namespace Inkscape::Extension */


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

/**
 * @file
 * Print dialog.
 */
/* Authors:
 *   Kees Cook <kees@outflux.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Kees Cook
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef WIN32
#include <io.h>
#include <windows.h>
#endif

#include <gtkmm.h>

#include "preferences.h"
#include "print.h"

#include "extension/internal/cairo-render-context.h"
#include "extension/internal/cairo-renderer.h"
#include "ui/widget/rendering-options.h"
#include "document.h"

#include "util/units.h"
#include "helper/png-write.h"
#include "svg/svg-color.h"
#include "io/sys.h"

#include <glibmm/i18n.h>


static void draw_page(
#ifdef WIN32
                      GtkPrintOperation *operation,
#else
                      GtkPrintOperation *,
#endif
                      GtkPrintContext   *context,
                      gint               /*page_nr*/,
                      gpointer           user_data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    struct workaround_gtkmm *junk = (struct workaround_gtkmm*)user_data;
    //printf("%s %d\n",__FUNCTION__, page_nr);

    if (junk->_tab->as_bitmap()) {
        // Render as exported PNG
        prefs->setBool("/dialogs/printing/asbitmap", true);
        gdouble width = (junk->_doc)->getWidth().value("px");
        gdouble height = (junk->_doc)->getHeight().value("px");
        gdouble dpi = junk->_tab->bitmap_dpi();
        prefs->setDouble("/dialogs/printing/dpi", dpi);
        
        std::string tmp_png;
        std::string tmp_base = "inkscape-print-png-XXXXXX";

        int tmp_fd;
        if ( (tmp_fd = Inkscape::IO::file_open_tmp (tmp_png, tmp_base)) >= 0) {
            close(tmp_fd);

            guint32 bgcolor = 0x00000000;
            Inkscape::XML::Node *nv = sp_repr_lookup_name (junk->_doc->rroot, "sodipodi:namedview");
            if (nv && nv->attribute("pagecolor")){
                bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
            }
            if (nv && nv->attribute("inkscape:pageopacity")){
                double opacity = 1.0;
                sp_repr_get_double (nv, "inkscape:pageopacity", &opacity);
                bgcolor |= SP_COLOR_F_TO_U(opacity);
            }

            sp_export_png_file(junk->_doc, tmp_png.c_str(), 0.0, 0.0,
                width, height,
                (unsigned long)(Inkscape::Util::Quantity::convert(width, "px", "in") * dpi),
                (unsigned long)(Inkscape::Util::Quantity::convert(height, "px", "in") * dpi),
                dpi, dpi, bgcolor, NULL, NULL, true, NULL);

            // This doesn't seem to work:
            //context->set_cairo_context ( Cairo::Context::create (Cairo::ImageSurface::create_from_png (tmp_png) ), dpi, dpi );
            //
            // so we'll use a surface pattern blat instead...
            //
            // but the C++ interface isn't implemented in cairomm:
            //context->get_cairo_context ()->set_source_surface(Cairo::ImageSurface::create_from_png (tmp_png) );
            //
            // so do it in C:
            {
                Cairo::RefPtr<Cairo::ImageSurface> png = Cairo::ImageSurface::create_from_png (tmp_png);
                cairo_t *cr = gtk_print_context_get_cairo_context (context);
                cairo_matrix_t m;
                cairo_get_matrix(cr, &m);
                cairo_scale(cr, Inkscape::Util::Quantity::convert(1, "in", "pt") / dpi, Inkscape::Util::Quantity::convert(1, "in", "pt") / dpi);
                // FIXME: why is the origin offset??
                cairo_set_source_surface(cr, png->cobj(), 0, 0);
                cairo_paint(cr);
                cairo_set_matrix(cr, &m);
            }

            // Clean up
            unlink (tmp_png.c_str());
        }
        else {
            g_warning("%s", _("Could not open temporary PNG for bitmap printing"));
        }
    }
    else {
        // Render as vectors
        prefs->setBool("/dialogs/printing/asbitmap", false);
        Inkscape::Extension::Internal::CairoRenderer renderer;
        Inkscape::Extension::Internal::CairoRenderContext *ctx = renderer.createContext();

        // ctx->setPSLevel(CAIRO_PS_LEVEL_3);
        ctx->setTextToPath(false);
        ctx->setFilterToBitmap(true);
        ctx->setBitmapResolution(72);

        cairo_t *cr = gtk_print_context_get_cairo_context (context);
        cairo_surface_t *surface = cairo_get_target(cr);
        cairo_matrix_t ctm;
        cairo_get_matrix(cr, &ctm);
#ifdef WIN32
        //Gtk+ does not take the non printable area into account
        //http://bugzilla.gnome.org/show_bug.cgi?id=381371
        //
        // This workaround translates the origin from the top left of the
        // printable area to the top left of the page.
        GtkPrintSettings *settings = gtk_print_operation_get_print_settings(operation);
        const gchar *printerName = gtk_print_settings_get_printer(settings);
        HDC hdc = CreateDC("WINSPOOL", printerName, NULL, NULL);
        if (hdc) {
            cairo_matrix_t mat;
            int x_off = GetDeviceCaps (hdc, PHYSICALOFFSETX);
            int y_off = GetDeviceCaps (hdc, PHYSICALOFFSETY);
            cairo_matrix_init_translate(&mat, -x_off, -y_off);
            cairo_matrix_multiply (&ctm, &ctm, &mat);
            DeleteDC(hdc);
        }
#endif             
        bool ret = ctx->setSurfaceTarget (surface, true, &ctm);
        if (ret) {
            ret = renderer.setupDocument (ctx, junk->_doc, TRUE, 0., NULL);
            if (ret) {
                renderer.renderItem(ctx, junk->_base);
                ret = ctx->finish();
            }
            else {
                g_warning("%s", _("Could not set up Document"));
            }
        }
        else {
            g_warning("%s", _("Failed to set CairoRenderContext"));
        }

        // Clean up
        renderer.destroyContext(ctx);
    }

}

static GObject* create_custom_widget (GtkPrintOperation */*operation*/,
                      gpointer           user_data)
{
    //printf("%s\n",__FUNCTION__);
    return G_OBJECT(user_data);
}

static void begin_print (GtkPrintOperation *operation,
             GtkPrintContext   */*context*/,
             gpointer           /*user_data*/)
{
    //printf("%s\n",__FUNCTION__);
    gtk_print_operation_set_n_pages (operation, 1);
}

namespace Inkscape {
namespace UI {
namespace Dialog {

Print::Print(SPDocument *doc, SPItem *base) :
    _doc (doc),
    _base (base)
{
    g_assert (_doc);
    g_assert (_base);

    _printop = gtk_print_operation_new ();

    // set up dialog title, based on document name
    gchar const *jobname = _doc->getName() ? _doc->getName() : _("SVG Document");
    Glib::ustring title = _("Print");
    title += " ";
    title += jobname;
    gtk_print_operation_set_job_name (_printop, title.c_str());

    // set up paper size to match the document size
    gtk_print_operation_set_unit (_printop, GTK_UNIT_POINTS);
    GtkPageSetup *page_setup = gtk_page_setup_new();
    gdouble doc_width = _doc->getWidth().value("pt");
    gdouble doc_height = _doc->getHeight().value("pt");
    GtkPaperSize *paper_size;
    if (doc_width > doc_height) {
        gtk_page_setup_set_orientation (page_setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
        paper_size = gtk_paper_size_new_custom("custom", "custom",
                                               doc_height, doc_width, GTK_UNIT_POINTS);
    } else {
        gtk_page_setup_set_orientation (page_setup, GTK_PAGE_ORIENTATION_PORTRAIT);
        paper_size = gtk_paper_size_new_custom("custom", "custom",
                                               doc_width, doc_height, GTK_UNIT_POINTS);
    }

    gtk_page_setup_set_paper_size (page_setup, paper_size);
    gtk_print_operation_set_default_page_setup (_printop, page_setup);
    gtk_print_operation_set_use_full_page (_printop, TRUE);

    // set up signals
    _workaround._doc = _doc;
    _workaround._base = _base;
    _workaround._tab = &_tab;
    g_signal_connect (_printop, "create-custom-widget", G_CALLBACK (create_custom_widget), _tab.gobj());
    g_signal_connect (_printop, "begin-print", G_CALLBACK (begin_print), NULL);
    g_signal_connect (_printop, "draw-page", G_CALLBACK (draw_page), &_workaround);

    // build custom preferences tab
    gtk_print_operation_set_custom_tab_label (_printop, _("Rendering"));
}

Gtk::PrintOperationResult Print::run(Gtk::PrintOperationAction, Gtk::Window &parent_window)
{
    gtk_print_operation_run (_printop, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
	    parent_window.gobj(), NULL);
    return Gtk::PRINT_OPERATION_RESULT_APPLY;
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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

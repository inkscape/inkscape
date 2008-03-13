/**
 * \brief Print dialog
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2007 Kees Cook
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef WIN32
#include <io.h>
#endif

#include <gtkmm/stock.h>
#include "print.h"

#include "extension/internal/cairo-render-context.h"
#include "extension/internal/cairo-renderer.h"
#include "ui/widget/rendering-options.h"

#include "unit-constants.h"
#include "helper/png-write.h"
#include "svg/svg-color.h"
#include "io/sys.h"


static void
draw_page (GtkPrintOperation */*operation*/,
           GtkPrintContext   *context,
           gint               /*page_nr*/,
           gpointer           user_data)
{
    struct workaround_gtkmm *junk = (struct workaround_gtkmm*)user_data;
    //printf("%s %d\n",__FUNCTION__, page_nr);

    if (junk->_tab->as_bitmap()) {
        // Render as exported PNG
        gdouble width = sp_document_width(junk->_doc);
        gdouble height = sp_document_height(junk->_doc);
        gdouble dpi = junk->_tab->bitmap_dpi();
        std::string tmp_png;
        std::string tmp_base = "inkscape-print-png-XXXXXX";

        int tmp_fd;
        if ( (tmp_fd = Inkscape::IO::file_open_tmp (tmp_png, tmp_base)) >= 0) {
            close(tmp_fd);

            guint32 bgcolor = 0x00000000;
            Inkscape::XML::Node *nv = sp_repr_lookup_name (junk->_doc->rroot, "sodipodi:namedview");
            if (nv && nv->attribute("pagecolor"))
                bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
            if (nv && nv->attribute("inkscape:pageopacity"))
                bgcolor |= SP_COLOR_F_TO_U(sp_repr_get_double_attribute (nv, "inkscape:pageopacity", 1.0));

            sp_export_png_file(junk->_doc, tmp_png.c_str(), 0.0, 0.0,
                width, height,
                (unsigned long)(width * PT_PER_IN / PX_PER_IN),
                (unsigned long)(height * PT_PER_IN / PX_PER_IN),
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
                // FIXME: why is the origin offset??
                cairo_set_source_surface(cr, png->cobj(), -16.0, -16.0);
            }
            cairo_paint(gtk_print_context_get_cairo_context (context));

            // Clean up
            unlink (tmp_png.c_str());
        }
        else {
            g_warning(_("Could not open temporary PNG for bitmap printing"));
        }
    }
    else {
        // Render as vectors
        Inkscape::Extension::Internal::CairoRenderer renderer;
        Inkscape::Extension::Internal::CairoRenderContext *ctx = renderer.createContext();
        bool ret = ctx->setSurfaceTarget (cairo_get_target (gtk_print_context_get_cairo_context (context)), true);
        if (ret) {
            ret = renderer.setupDocument (ctx, junk->_doc);
            if (ret) {
                renderer.renderItem(ctx, junk->_base);
                ret = ctx->finish();
            }
            else {
                g_warning(_("Could not set up Document"));
            }
        }
        else {
            g_warning(_("Failed to set CairoRenderContext"));
        }

        // Clean up
        renderer.destroyContext(ctx);
    }

}

static GObject*
create_custom_widget (GtkPrintOperation */*operation*/,
                      gpointer           user_data)
{
    //printf("%s\n",__FUNCTION__);
    return G_OBJECT(user_data);
}

static void
begin_print (GtkPrintOperation *operation,
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
    gchar *jobname = _doc->name ? _doc->name : _("SVG Document");
    Glib::ustring title = _("Print");
    title += " ";
    title += jobname;
    gtk_print_operation_set_job_name (_printop, title.c_str());

    // set up paper size to match the document size
    GtkPageSetup *page_setup = gtk_page_setup_new();
    gdouble doc_width = sp_document_width(_doc) * PT_PER_PX;
    gdouble doc_height = sp_document_height(_doc) * PT_PER_PX;
    GtkPaperSize *paper_size = gtk_paper_size_new_custom("custom", "custom",
                                doc_width, doc_height, GTK_UNIT_POINTS);
    gtk_page_setup_set_paper_size (page_setup, paper_size);
    gtk_print_operation_set_default_page_setup (_printop, page_setup);

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

Gtk::PrintOperationResult Print::run(Gtk::PrintOperationAction, Gtk::Window&)
{
    gtk_print_operation_run (_printop, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);
    return Gtk::PRINT_OPERATION_RESULT_APPLY;
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape


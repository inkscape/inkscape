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

namespace Inkscape {
namespace UI {
namespace Dialog {

void
Print::_draw_page (const Glib::RefPtr<Gtk::PrintContext> &context, int /*page_nr*/)
{
    if (_tab.as_bitmap()) {
        // Render as exported PNG
        gdouble width = sp_document_width(_doc);
        gdouble height = sp_document_height(_doc);
        gdouble dpi = _tab.bitmap_dpi();
        std::string tmp_png;
        std::string tmp_base = "inkscape-print-png-XXXXXX";

        int tmp_fd;
        if ( (tmp_fd = Glib::file_open_tmp (tmp_png, tmp_base)) >= 0) {
            close(tmp_fd);

            guint32 bgcolor = 0x00000000;
            Inkscape::XML::Node *nv = sp_repr_lookup_name (_doc->rroot, "sodipodi:namedview");
            if (nv && nv->attribute("pagecolor"))
                bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
            if (nv && nv->attribute("inkscape:pageopacity"))
                bgcolor |= SP_COLOR_F_TO_U(sp_repr_get_double_attribute (nv, "inkscape:pageopacity", 1.0));

            sp_export_png_file(_doc, tmp_png.c_str(), 0.0, 0.0,
                width, height,
                (unsigned long)width * dpi / PX_PER_IN,
                (unsigned long)height * dpi / PX_PER_IN,
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
                cairo_t *cr = context->get_cairo_context ()->cobj();
                // FIXME: why is the origin offset??
                cairo_set_source_surface(cr, png->cobj(), -20.0, -20.0);
            }
            context->get_cairo_context ()->paint ();

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
        bool ret = ctx->setSurfaceTarget (context->get_cairo_context ()->get_target ()->cobj(), true);
        if (ret) {
            ret = renderer.setupDocument (ctx, _doc);
            if (ret) {
                renderer.renderItem(ctx, _base);
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

Gtk::Widget *
Print::_create_custom_widget ()
{
    return &_tab;
}

void
Print::_custom_widget_apply (Gtk::Widget */*widget*/)
{
    g_warning (_("custom widget apply"));
}

Print::Print(SPDocument *doc, SPItem *base) :
    _doc (doc),
    _base (base),
    _tab ()
{
    g_assert (_doc);
    g_assert (_base);

    _printop = Gtk::PrintOperation::create();

    // set up dialog title, based on document name
    gchar *jobname = _doc->name ? _doc->name : _("SVG Document");
    Glib::ustring title = _("Print");
    title += " ";
    title += jobname;
    _printop->set_job_name (title);
    _printop->set_n_pages (1);

    // set up paper size to match the document size
    Glib::RefPtr<Gtk::PageSetup> page_setup = Gtk::PageSetup::create();
    gdouble doc_width = sp_document_width(_doc) * PT_PER_PX;
    gdouble doc_height = sp_document_height(_doc) * PT_PER_PX;
    Gtk::PaperSize paper_size(Glib::ustring("custom"), Glib::ustring("custom"),
                              doc_width, doc_height, Gtk::UNIT_POINTS);
    page_setup->set_paper_size (paper_size);
    _printop->set_default_page_setup (page_setup);

    // build custom preferences tab
    _printop->set_custom_tab_label (Glib::ustring(_("Rendering")));
    _printop->signal_create_custom_widget().connect(sigc::mem_fun(*this, &Print::_create_custom_widget));
//    _printop->signal_custom_widget_apply().connect(sigc::mem_fun(*this, &Print::_custom_widget_apply));

    // register actual page surface drawing callback
    _printop->signal_draw_page().connect(sigc::mem_fun(*this, &Print::_draw_page));

}

Gtk::PrintOperationResult
Print::run(Gtk::PrintOperationAction action)
{
    Gtk::PrintOperationResult res;
    res = _printop->run (action);
    return res;
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape


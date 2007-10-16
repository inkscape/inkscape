#define __SP_PRINT_C__

/** \file
 * Frontend to printing
 */
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif



#include "sp-item.h"
#include "extension/print.h"
#include "extension/system.h"
#include "print.h"

#include <unit-constants.h>

#ifdef HAVE_GTK_UNIX_PRINT
# include <gtk/gtk.h>
# include <glibmm/i18n.h>
# include <gtk/gtkprintunixdialog.h>
# include <unistd.h>   // close, unlink
# include <cstdio>
using std::fprintf;
#endif

#if 0
# include <extension/internal/ps.h>

# ifdef WIN32
#  include <extension/internal/win32.h>
# endif
#endif

/* Identity typedef */

unsigned int sp_print_bind(SPPrintContext *ctx, NR::Matrix const &transform, float opacity)
{
    NRMatrix const ntransform(transform);
    return sp_print_bind(ctx, &ntransform, opacity);
}

unsigned int
sp_print_bind(SPPrintContext *ctx, NRMatrix const *transform, float opacity)
{
    return ctx->module->bind(transform, opacity);
}

unsigned int
sp_print_release(SPPrintContext *ctx)
{
    return ctx->module->release();
}

unsigned int
sp_print_comment(SPPrintContext *ctx, char const *comment)
{
    return ctx->module->comment(comment);
}

unsigned int
sp_print_fill(SPPrintContext *ctx, NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *style,
              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return ctx->module->fill(bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_stroke(SPPrintContext *ctx, NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *style,
                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return ctx->module->stroke(bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_image_R8G8B8A8_N(SPPrintContext *ctx,
                          guchar *px, unsigned int w, unsigned int h, unsigned int rs,
                          NRMatrix const *transform, SPStyle const *style)
{
    return ctx->module->image(px, w, h, rs, transform, style);
}

unsigned int sp_print_text(SPPrintContext *ctx, char const *text, NR::Point p,
                           SPStyle const *style)
{
    return ctx->module->text(text, p, style);
}

#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

/* UI */

void
sp_print_preview_document(SPDocument *doc)
{
    Inkscape::Extension::Print *mod;
    unsigned int ret;

    sp_document_ensure_up_to_date(doc);

    mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_DEFAULT);

    ret = mod->set_preview();

    if (ret) {
        SPPrintContext context;
        context.module = mod;

        /* fixme: This has to go into module constructor somehow */
        /* Create new arena */
        mod->base = SP_ITEM(sp_document_root(doc));
        mod->arena = NRArena::create();
        mod->dkey = sp_item_display_key_new(1);
        mod->root = sp_item_invoke_show(mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);
        /* Print document */
        ret = mod->begin(doc);
        sp_item_invoke_print(mod->base, &context);
        ret = mod->finish();
        /* Release arena */
        sp_item_invoke_hide(mod->base, mod->dkey);
        mod->base = NULL;
        nr_arena_item_unref(mod->root);
        mod->root = NULL;
        nr_object_unref((NRObject *) mod->arena);
        mod->arena = NULL;
    }

    return;
}

#ifdef HAVE_GTK_UNIX_PRINT
static void
unix_print_complete (GtkPrintJob *print_job,
                     gpointer user_data,
                     GError *error)
{
    fprintf(stderr,"print job finished: %s\n",error ? error->message : "no error");
}

static void
unix_print_dialog (const gchar * ps_file, const gchar * jobname, gdouble doc_width, gdouble doc_height)
{
    Glib::ustring title = _("Print");
    title += " ";
    title += jobname;
    GtkWidget* dlg = gtk_print_unix_dialog_new(title.c_str(), NULL);

    // force output system to only handle our pre-generated PS output
    gtk_print_unix_dialog_set_manual_capabilities (GTK_PRINT_UNIX_DIALOG(dlg),
                                                   GTK_PRINT_CAPABILITY_GENERATE_PS);

/*
 * It would be nice to merge the PrintPS::setup routine with a custom
 * configuration dialog:
   
    gtk_print_unix_dialog_add_custom_tab (GtkPrintUnixDialog *dialog,
                                          GtkWidget *child,
                                          GtkWidget *tab_label);
*/

    int const response = gtk_dialog_run(GTK_DIALOG(dlg));

    if (response == GTK_RESPONSE_OK) {
        GtkPrinter* printer = gtk_print_unix_dialog_get_selected_printer(GTK_PRINT_UNIX_DIALOG(dlg));

        if (gtk_printer_accepts_ps (printer)) {
			GtkPageSetup *page_setup = gtk_print_unix_dialog_get_page_setup(GTK_PRINT_UNIX_DIALOG(dlg));

			//It's important to set the right paper size here, otherwise it will 
			//default to letter; if for example an A4 is printed as a letter, then
			//part of it will be truncated even when printing on A4 paper
			
			//TODO: let the user decide upon the paper size, by enabling
			//the drop down widget in the printing dialog. For now, we'll simply
			//take the document's dimensions and communicate these to the printer
			//driver 
			
			GtkPaperSize *page_size = gtk_paper_size_new_custom("custom", "custom", doc_width, doc_height, GTK_UNIT_POINTS);

			gtk_page_setup_set_paper_size (page_setup, page_size);								   
			
            GtkPrintJob* job = gtk_print_job_new  (jobname, printer,
              gtk_print_unix_dialog_get_settings(GTK_PRINT_UNIX_DIALOG(dlg)),
              page_setup);
			
			GtkPaperSize* tmp = gtk_page_setup_get_paper_size(gtk_print_unix_dialog_get_page_setup(GTK_PRINT_UNIX_DIALOG(dlg)));
			
            GError * error = NULL;
            if ( gtk_print_job_set_source_file (job, ps_file, &error)) {
                gtk_print_job_send (job, unix_print_complete, NULL, NULL);
            }
            else {
                g_warning(_("Could not set print source: %s"),error ? error->message : _("unknown error"));
            }
            if (error) g_error_free(error);
        }
        else {
            g_warning(_("Printer '%s' does not support PS output"), gtk_printer_get_name (printer));
        }
    }
    else if (response == GTK_RESPONSE_APPLY) {
        // since we didn't include the Preview capability,
        // this should never happen.
        g_warning(_("Print Preview not available"));
    }

    gtk_widget_destroy(dlg);
}
#endif // HAVE_GTK_UNIX_PRINT


void
sp_print_document(SPDocument *doc, unsigned int direct)
{
    Inkscape::Extension::Print *mod;
    unsigned int ret;
#ifdef HAVE_GTK_UNIX_PRINT
    Glib::ustring tmpfile = "";
#endif

    sp_document_ensure_up_to_date(doc);

    if (direct) {
        mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_PS);
    } else {
        mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_DEFAULT);

#ifdef HAVE_GTK_UNIX_PRINT
        // unix print dialog reads from the exported tempfile
        gchar  *filename = NULL;
        GError *error    = NULL;
        gint tmpfd = g_file_open_tmp("inkscape-ps-XXXXXX",
                                     &filename,
                                     &error);
        if (tmpfd<0) {
            g_warning(_("Failed to create tempfile for printing: %s"),
                      error ? error->message : _("unknown error"));
            if (error) g_error_free(error);
            return;
        }
        tmpfile = filename;
        g_free(filename);
        close(tmpfd);

        Glib::ustring destination = ">" + tmpfile;
        mod->set_param_string("destination", destination.c_str());
#endif
    }

    ret = mod->setup();

    if (ret) {
        SPPrintContext context;
        context.module = mod;

        /* fixme: This has to go into module constructor somehow */
        /* Create new arena */
        mod->base = SP_ITEM(sp_document_root(doc));
        mod->arena = NRArena::create();
        mod->dkey = sp_item_display_key_new(1);
        mod->root = sp_item_invoke_show(mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);
        /* Print document */
        ret = mod->begin(doc);
        sp_item_invoke_print(mod->base, &context);
        ret = mod->finish();
        /* Release arena */
        sp_item_invoke_hide(mod->base, mod->dkey);
        mod->base = NULL;
        nr_arena_item_unref(mod->root);
        mod->root = NULL;
        nr_object_unref((NRObject *) mod->arena);
        mod->arena = NULL;

#ifdef HAVE_GTK_UNIX_PRINT
        // redirect output to new print dialog
        
        // width and height in pt
    	gdouble width = sp_document_width(doc) * PT_PER_PX;
    	gdouble height = sp_document_height(doc) * PT_PER_PX;
        
        unix_print_dialog(tmpfile.c_str(),doc->name ? doc->name : _("SVG Document"), width, height);
        unlink(tmpfile.c_str());
        // end redirected new print dialog
#endif
    }

    return;
}

void
sp_print_document_to_file(SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Print *mod;
    SPPrintContext context;
    gchar const *oldconst;
    gchar *oldoutput;
    unsigned int ret;

    sp_document_ensure_up_to_date(doc);

    mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_PS);
    oldconst = mod->get_param_string("destination");
    oldoutput = g_strdup(oldconst);
    mod->set_param_string("destination", (gchar *)filename);

/* Start */
    context.module = mod;
    /* fixme: This has to go into module constructor somehow */
    /* Create new arena */
    mod->base = SP_ITEM(sp_document_root(doc));
    mod->arena = NRArena::create();
    mod->dkey = sp_item_display_key_new(1);
    mod->root = sp_item_invoke_show(mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);
    /* Print document */
    ret = mod->begin(doc);
    sp_item_invoke_print(mod->base, &context);
    ret = mod->finish();
    /* Release arena */
    sp_item_invoke_hide(mod->base, mod->dkey);
    mod->base = NULL;
    nr_arena_item_unref(mod->root);
    mod->root = NULL;
    nr_object_unref((NRObject *) mod->arena);
    mod->arena = NULL;
/* end */

    mod->set_param_string("destination", oldoutput);
    g_free(oldoutput);

    return;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

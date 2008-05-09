#define __SP_PRINT_C__

/** \file
 * Frontend to printing
 */
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Kees Cook <kees@outflux.net>
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "inkscape.h"
#include "desktop.h"
#include "sp-item.h"
#include "extension/print.h"
#include "extension/system.h"
#include "print.h"

#include "ui/dialog/print.h"


/* Identity typedef */

unsigned int sp_print_bind(SPPrintContext *ctx, NR::Matrix const &transform, float opacity)
{
    NR::Matrix const ntransform(transform);
    return sp_print_bind(ctx, &ntransform, opacity);
}

unsigned int
sp_print_bind(SPPrintContext *ctx, NR::Matrix const *transform, float opacity)
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
sp_print_fill(SPPrintContext *ctx, const_NRBPath const *bpath, NR::Matrix const *ctm, SPStyle const *style,
              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return ctx->module->fill(bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_stroke(SPPrintContext *ctx, const_NRBPath const *bpath, NR::Matrix const *ctm, SPStyle const *style,
                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return ctx->module->stroke(bpath, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_image_R8G8B8A8_N(SPPrintContext *ctx,
                          guchar *px, unsigned int w, unsigned int h, unsigned int rs,
                          NR::Matrix const *transform, SPStyle const *style)
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

void
sp_print_document(Gtk::Window& parentWindow, SPDocument *doc)
{
    sp_document_ensure_up_to_date(doc);

    // Build arena
    SPItem      *base = SP_ITEM(sp_document_root(doc));
    NRArena    *arena = NRArena::create();
    unsigned int dkey = sp_item_display_key_new(1);
    NRArenaItem *root = sp_item_invoke_show(base, arena, dkey, SP_ITEM_SHOW_DISPLAY);

    // Run print dialog
    Inkscape::UI::Dialog::Print printop(doc,base);
    Gtk::PrintOperationResult res = printop.run(Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG, parentWindow);
    (void)res; // TODO handle this

    // Release arena
    sp_item_invoke_hide(base, dkey);
    nr_arena_item_unref(root);
    nr_object_unref((NRObject *) arena);
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

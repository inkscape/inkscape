/** \file
 * Frontend to printing
 */
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Kees Cook <kees@outflux.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
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
#include "sp-root.h"

#include "ui/dialog/print.h"


/* Identity typedef */

unsigned int sp_print_bind(SPPrintContext *ctx, Geom::Affine const &transform, float opacity)
{
    Geom::Affine const ntransform(transform);
    return sp_print_bind(ctx, &ntransform, opacity);
}

unsigned int
sp_print_bind(SPPrintContext *ctx, Geom::Affine const *transform, float opacity)
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
sp_print_fill(SPPrintContext *ctx, Geom::PathVector const &pathv, Geom::Affine const *ctm, SPStyle const *style,
              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return ctx->module->fill(pathv, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_stroke(SPPrintContext *ctx, Geom::PathVector const &pathv, Geom::Affine const *ctm, SPStyle const *style,
                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return ctx->module->stroke(pathv, ctm, style, pbox, dbox, bbox);
}

unsigned int
sp_print_image_R8G8B8A8_N(SPPrintContext *ctx,
                          guchar *px, unsigned int w, unsigned int h, unsigned int rs,
                          Geom::Affine const *transform, SPStyle const *style)
{
    return ctx->module->image(px, w, h, rs, transform, style);
}

unsigned int sp_print_text(SPPrintContext *ctx, char const *text, Geom::Point p,
                           SPStyle const *style)
{
    return ctx->module->text(text, p, style);
}

#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

/* UI */

void
sp_print_document(Gtk::Window& parentWindow, SPDocument *doc)
{
    doc->ensureUpToDate();

    // Build arena
    SPItem      *base = doc->getRoot();
    NRArena    *arena = NRArena::create();
    unsigned int dkey = SPItem::display_key_new(1);
    // TODO investigate why we are grabbing root and then ignoring it.
    NRArenaItem *root = base->invoke_show(arena, dkey, SP_ITEM_SHOW_DISPLAY);

    // Run print dialog
    Inkscape::UI::Dialog::Print printop(doc,base);
    Gtk::PrintOperationResult res = printop.run(Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG, parentWindow);
    (void)res; // TODO handle this

    // Release arena
    base->invoke_hide(dkey);
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

    doc->ensureUpToDate();

    mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_PS);
    oldconst = mod->get_param_string("destination");
    oldoutput = g_strdup(oldconst);
    mod->set_param_string("destination", (gchar *)filename);

/* Start */
    context.module = mod;
    /* fixme: This has to go into module constructor somehow */
    /* Create new arena */
    mod->base = doc->getRoot();
    mod->arena = NRArena::create();
    mod->dkey = SPItem::display_key_new(1);
    mod->root = (mod->base)->invoke_show(mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);
    /* Print document */
    ret = mod->begin(doc);
    (mod->base)->invoke_print(&context);
    ret = mod->finish();
    /* Release arena */
    (mod->base)->invoke_hide(mod->dkey);
    mod->base = NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

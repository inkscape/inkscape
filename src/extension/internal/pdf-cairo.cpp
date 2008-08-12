#define __SP_PDF_CAIRO_C__

/** \file
 * PDF printing with Cairo.
 */
/*
 * Authors:
 *   Miklós Erdélyi <erdelyim@gmail.com>
 *
 * Based on pdf.cpp
 *
 * Licensed under GNU GPL
 */

/* Plain Print */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_CAIRO_PDF

#ifndef PANGO_ENABLE_BACKEND
#define PANGO_ENABLE_BACKEND
#endif

#ifndef PANGO_ENABLE_ENGINE
#define PANGO_ENABLE_ENGINE
#endif


#include <signal.h>
#include <errno.h>

#include <glib/gmem.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktooltips.h>

#include <glibmm/i18n.h>
#include "display/nr-arena-item.h"
#include "display/canvas-bpath.h"
#include "display/inkscape-cairo.h"
#include "sp-item.h"
#include "style.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"

#include "libnrtype/font-instance.h"
#include "libnrtype/font-style-to-pos.h"

#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"

#include <unit-constants.h>

#include "pdf-cairo.h"
#include "extension/system.h"
#include "extension/print.h"

#include "io/sys.h"

#include <cairo.h>
#include <cairo-pdf.h>

#include <pango/pango.h>
#include <pango/pangofc-fontmap.h>

#ifdef RENDER_WITH_PANGO_CAIRO
#include <pango/pangocairo.h>
#else
#include <cairo-ft.h>
#endif
namespace Inkscape {
namespace Extension {
namespace Internal {

static cairo_status_t _write_callback(void *closure, const unsigned char *data, unsigned int length);
static void _concat_transform(cairo_t *cr, double xx, double yx, double xy, double yy, double x0, double y0);

PrintCairoPDF::PrintCairoPDF() :
    cr(NULL),
    pdf_surface(NULL),
    _layout(NULL),
    _dpi(72),
    _bitmap(false)
{
}

PrintCairoPDF::~PrintCairoPDF(void)
{
    if (cr) cairo_destroy(cr);
    if (pdf_surface) cairo_surface_destroy(pdf_surface);
    if (_layout) g_object_unref(_layout);

    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif

    return;
}

unsigned int
PrintCairoPDF::setup(Inkscape::Extension::Print * mod)
{
    static gchar const *const pdr[] = {"72", "75", "100", "144", "150", "200", "300", "360", "600", "1200", "2400", NULL};

#ifdef TED
    Inkscape::XML::Node *repr = ((SPModule *) mod)->repr;
#endif

    unsigned int ret = FALSE;

    /* Create dialog */
    GtkTooltips *tt = gtk_tooltips_new();
    g_object_ref((GObject *) tt);
    gtk_object_sink((GtkObject *) tt);

    GtkWidget *dlg = gtk_dialog_new_with_buttons(_("Print Destination"),
//            SP_DT_WIDGET(SP_ACTIVE_DESKTOP)->window,
            NULL,
            (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT),
            GTK_STOCK_CANCEL,
            GTK_RESPONSE_CANCEL,
            GTK_STOCK_PRINT,
            GTK_RESPONSE_OK,
            NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dlg), GTK_RESPONSE_OK);

    GtkWidget *vbox = GTK_DIALOG(dlg)->vbox;
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    /* Print properties frame */
    GtkWidget *f = gtk_frame_new(_("Print properties"));
    gtk_box_pack_start(GTK_BOX(vbox), f, FALSE, FALSE, 4);
    GtkWidget *vb = gtk_vbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(f), vb);
    gtk_container_set_border_width(GTK_CONTAINER(vb), 4);
    /* Print type */
    bool const p2bm = mod->get_param_bool("bitmap");
    GtkWidget *rb = gtk_radio_button_new_with_label(NULL, _("Print using PDF operators"));
    gtk_tooltips_set_tip((GtkTooltips *) tt, rb,
                         _("Use PDF vector operators. The resulting image is usually smaller "
                           "in file size and can be arbitrarily scaled, but "
                           "patterns will be lost."), NULL);
    if (!p2bm) gtk_toggle_button_set_active((GtkToggleButton *) rb, TRUE);
    gtk_box_pack_start(GTK_BOX(vb), rb, FALSE, FALSE, 0);
    rb = gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) rb), _("Print as bitmap"));
    gtk_tooltips_set_tip((GtkTooltips *) tt, rb,
                         _("Print everything as bitmap. The resulting image is usually larger "
                           "in file size and cannot be arbitrarily scaled without quality loss, "
                           "but all objects will be rendered exactly as displayed."), NULL);
    if (p2bm) gtk_toggle_button_set_active((GtkToggleButton *) rb, TRUE);
    gtk_box_pack_start(GTK_BOX(vb), rb, FALSE, FALSE, 0);
    /* Resolution */
    GtkWidget *hb = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);
    GtkWidget *combo = gtk_combo_new();
    gtk_combo_set_value_in_list(GTK_COMBO(combo), FALSE, FALSE);
    gtk_combo_set_use_arrows(GTK_COMBO(combo), TRUE);
    gtk_combo_set_use_arrows_always(GTK_COMBO(combo), TRUE);
    gtk_widget_set_size_request(combo, 64, -1);
    gtk_tooltips_set_tip((GtkTooltips *) tt, GTK_COMBO(combo)->entry,
                         _("Preferred resolution (dots per inch) of bitmap"), NULL);
    /* Setup strings */
    GList *sl = NULL;
    for (unsigned i = 0; pdr[i] != NULL; i++) {
        sl = g_list_prepend(sl, (gpointer) pdr[i]);
    }
    sl = g_list_reverse(sl);
    gtk_combo_set_popdown_strings(GTK_COMBO(combo), sl);
    g_list_free(sl);
    if (1) {
        gchar const *val = mod->get_param_string("resolution");
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), val);
    }
    gtk_box_pack_end(GTK_BOX(hb), combo, FALSE, FALSE, 0);
    GtkWidget *l = gtk_label_new(_("Resolution:"));
    gtk_box_pack_end(GTK_BOX(hb), l, FALSE, FALSE, 0);

    /* Print destination frame */
    f = gtk_frame_new(_("Print destination"));
    gtk_box_pack_start(GTK_BOX(vbox), f, FALSE, FALSE, 4);
    vb = gtk_vbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(f), vb);
    gtk_container_set_border_width(GTK_CONTAINER(vb), 4);

    l = gtk_label_new(_("Printer name (as given by lpstat -p);\n"
                        "leave empty to use the system default printer.\n"
                        "Use '> filename' to print to file.\n"
                        "Use '| prog arg...' to pipe to a program."));
    gtk_box_pack_start(GTK_BOX(vb), l, FALSE, FALSE, 0);

    GtkWidget *e = gtk_entry_new();
    if (1) {
        gchar const *val = mod->get_param_string("destination");
        gtk_entry_set_text(GTK_ENTRY(e), ( val != NULL
                                           ? val
                                           : "" ));
    }
    gtk_box_pack_start(GTK_BOX(vb), e, FALSE, FALSE, 0);

    // pressing enter in the destination field is the same as clicking Print:
    gtk_entry_set_activates_default(GTK_ENTRY(e), TRUE);

    gtk_widget_show_all(vbox);

    int const response = gtk_dialog_run(GTK_DIALOG(dlg));

    g_object_unref((GObject *) tt);

    if (response == GTK_RESPONSE_OK) {
        gchar const *fn;
        char const *sstr;

        _bitmap = gtk_toggle_button_get_active((GtkToggleButton *) rb);
        sstr = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
        _dpi = (unsigned int) MAX((int)(atof(sstr)), 1);
        /* Arrgh, have to do something */
        fn = gtk_entry_get_text(GTK_ENTRY(e));
        /* skip leading whitespace, bug #1068483 */
        while (fn && *fn==' ') { fn++; }
        /* g_print("Printing to %s\n", fn); */

        mod->set_param_bool("bitmap", _bitmap);
        mod->set_param_string("resolution", (gchar *)sstr);
        mod->set_param_string("destination", (gchar *)fn);
        ret = TRUE;
    }

    gtk_widget_destroy(dlg);

    return ret;
}

unsigned int
PrintCairoPDF::begin(Inkscape::Extension::Print *mod, SPDocument *doc)
{
    FILE *osf = NULL;
    FILE *osp = NULL;

    _alpha_stack.clear();
    _alpha_stack.push_back(1.0);

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    gchar const *utf8_fn = mod->get_param_string("destination");
    gchar *local_fn = g_filename_from_utf8( utf8_fn,
                                            -1,  &bytesRead,  &bytesWritten, &error);
    gchar const *fn = local_fn;

    /* TODO: Replace the below fprintf's with something that does the right thing whether in
     * gui or batch mode (e.g. --print=blah).  Consider throwing an exception: currently one of
     * the callers (sp_print_document_to_file, "ret = mod->begin(doc)") wrongly ignores the
     * return code.
     */
    if (fn != NULL) {
        if (*fn == '|') {
            fn += 1;
            while (isspace(*fn)) fn += 1;
#ifndef WIN32
            osp = popen(fn, "w");
#else
            osp = _popen(fn, "w");
#endif
            if (!osp) {
                fprintf(stderr, "inkscape: popen(%s): %s\n",
                        fn, strerror(errno));
                return 0;
            }
            _stream = osp;
        } else if (*fn == '>') {
            fn += 1;
            while (isspace(*fn)) fn += 1;
            Inkscape::IO::dump_fopen_call(fn, "K");
            osf = Inkscape::IO::fopen_utf8name(fn, "w+");
            if (!osf) {
                fprintf(stderr, "inkscape: fopen(%s): %s\n",
                        fn, strerror(errno));
                return 0;
            }
            _stream = osf;
        } else {
            /* put cwd stuff in here */
            gchar *qn = ( *fn
                          ? g_strdup_printf("lpr -P %s", fn)  /* FIXME: quote fn */
                          : g_strdup("lpr") );
#ifndef WIN32
            osp = popen(qn, "w");
#else
            osp = _popen(qn, "w");
#endif
            if (!osp) {
                fprintf(stderr, "inkscape: popen(%s): %s\n",
                        qn, strerror(errno));
                return 0;
            }
            g_free(qn);
            _stream = osp;
        }
    }

    g_free(local_fn);

    if (_stream) {
        /* fixme: this is kinda icky */
#if !defined(_WIN32) && !defined(__WIN32__)
        (void) signal(SIGPIPE, SIG_IGN);
#endif
    }

    // test output stream?

    // width and height in pt
    _width = sp_document_width(doc) * PT_PER_PX;
    _height = sp_document_height(doc) * PT_PER_PX;

    NRRect d;
    bool   pageBoundingBox = mod->get_param_bool("pageBoundingBox");
    SPItem* doc_item = SP_ITEM(mod->base);
    // printf("Page Bounding Box: %s\n", pageBoundingBox ? "TRUE" : "FALSE");
    Geom::Matrix t (Geom::identity());
    if (pageBoundingBox) {
        d.x0 = d.y0 = 0;
        d.x1 = _width;
        d.y1 = _height;
    } else {
        // if not page, use our base, which is either root or the item we want to export
        sp_item_invoke_bbox(doc_item, &d, sp_item_i2doc_affine (doc_item), TRUE);
        // convert from px to pt
        d.x0 *= PT_PER_PX;
        d.x1 *= PT_PER_PX;
        d.y0 *= PT_PER_PX;
        d.y1 *= PT_PER_PX;
    }

    // When rendering a standalone object, we must set cairo's transform to the accumulated
    // ancestor transform of that item - e.g. it may be rotated or skewed by its parent group, and
    // we must reproduce that in the export even though we start traversing the tree from the
    // object itself, ignoring its ancestors

    // complete transform, including doc_item's own transform
    t = sp_item_i2doc_affine (doc_item);
    // subreact doc_item's transform (comes first) from it
    t = Geom::Matrix(doc_item->transform).inverse() * t;

    // create cairo context
    pdf_surface = cairo_pdf_surface_create_for_stream(Inkscape::Extension::Internal::_write_callback, _stream, d.x1-d.x0, d.y1-d.y0);
    cr = cairo_create(pdf_surface);

    // move to the origin
    cairo_translate (cr, -d.x0, -d.y0);

    // set cairo transform;  we must scale the translation values to pt
    _concat_transform (cr, t[0], t[1], t[2], t[3], t[4]*PT_PER_PX, t[5]*PT_PER_PX);

    if (!_bitmap) {
        cairo_scale(cr, PT_PER_PX, PT_PER_PX);

        // from now on we can output px, but they will be treated as pt
        // note that the we do not have to flip the y axis
        // because Cairo's coordinate system is identical to Inkscape's
    }

    return 1;
}

unsigned int
PrintCairoPDF::finish(Inkscape::Extension::Print *mod)
{
    if (!_stream) return 0;
    if (_bitmap) return 0;

    cairo_show_page(cr);

    cairo_destroy(cr);
    cairo_surface_finish(pdf_surface);
    cairo_status_t status = cairo_surface_status(pdf_surface);
    cairo_surface_destroy(pdf_surface);
    cr = NULL;
    pdf_surface = NULL;

    /* Flush stream to be sure. */
    (void) fflush(_stream);

    /* fixme: should really use pclose for popen'd streams */
    fclose(_stream);
    _stream = 0;

    if (status == CAIRO_STATUS_SUCCESS)
        return true;
    else
        return false;
}

unsigned int
PrintCairoPDF::bind(Inkscape::Extension::Print *mod, Geom::Matrix const *transform, float opacity)
{
    if (!_stream) return 0;  // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if (opacity < 1.0) {
        cairo_push_group(cr);
    } else {
        cairo_save(cr);
    }
    _concat_transform(cr, (*transform)[0], (*transform)[1], (*transform)[2], (*transform)[3], (*transform)[4], (*transform)[5]);
    // printf("bind: (%f) %f %f %f %f %f %f\n", opacity, transform->c[0], transform->c[1], transform->c[2], transform->c[3], transform->c[4], transform->c[5]);
    // remember these to be able to undo them when outputting text
    _last_tx = (*transform)[4];
    _last_ty = (*transform)[5];

    _alpha_stack.push_back(opacity);

    return 1;
}

unsigned int
PrintCairoPDF::release(Inkscape::Extension::Print *mod)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    float opacity = _alpha_stack.back();

    if (opacity < 1.0) {
        cairo_pop_group_to_source(cr);
        cairo_paint_with_alpha(cr, opacity);
    } else {
        cairo_restore(cr);
    }
//    g_printf("release\n");
    _alpha_stack.pop_back();
    g_assert(_alpha_stack.size() > 0);

    return 1;
}

unsigned int
PrintCairoPDF::comment(Inkscape::Extension::Print *mod, char const *comment)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    return 1;
}

cairo_pattern_t*
PrintCairoPDF::create_pattern_for_paint(SPPaintServer const *const paintserver, NRRect const *pbox, float alpha)
{
    cairo_pattern_t *pattern = NULL;
    bool apply_bbox2user = false;

    if (SP_IS_LINEARGRADIENT (paintserver)) {

            SPLinearGradient *lg=SP_LINEARGRADIENT(paintserver);

            sp_gradient_ensure_vector(SP_GRADIENT(lg)); // when exporting from commandline, vector is not built

            Geom::Point p1 (lg->x1.computed, lg->y1.computed);
            Geom::Point p2 (lg->x2.computed, lg->y2.computed);
            if (pbox && SP_GRADIENT(lg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                Geom::Matrix bbox2user(pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
                p1 *= bbox2user;
                p2 *= bbox2user;
            }

            // create linear gradient pattern
            pattern = cairo_pattern_create_linear(p1[Geom::X], p1[Geom::Y], p2[Geom::X], p2[Geom::Y]);

            // add stops
            for (gint i = 0; unsigned(i) < lg->vector.stops.size(); i++) {
                float rgb[3];
                sp_color_get_rgb_floatv(&lg->vector.stops[i].color, rgb);
                cairo_pattern_add_color_stop_rgba(pattern, lg->vector.stops[i].offset, rgb[0], rgb[1], rgb[2], lg->vector.stops[i].opacity * alpha);
            }
    } else if (SP_IS_RADIALGRADIENT (paintserver)) {

        SPRadialGradient *rg=SP_RADIALGRADIENT(paintserver);

        sp_gradient_ensure_vector(SP_GRADIENT(rg)); // when exporting from commandline, vector is not built

        Geom::Point c (rg->cx.computed, rg->cy.computed);
        Geom::Point f (rg->fx.computed, rg->fy.computed);
        double r = rg->r.computed;

        Geom::Coord const df = hypot(f[Geom::X] - c[Geom::X], f[Geom::Y] - c[Geom::Y]);
        if (df >= r) {
            f[Geom::X] = c[Geom::X] + (f[Geom::X] - c[Geom::X] ) * r / (float) df;
            f[Geom::Y] = c[Geom::Y] + (f[Geom::Y] - c[Geom::Y] ) * r / (float) df;
        }

        if (pbox && SP_GRADIENT(rg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX)
            apply_bbox2user = true;

        // create radial gradient pattern
        pattern = cairo_pattern_create_radial(f[Geom::X], f[Geom::Y], 0, c[Geom::X], c[Geom::Y], r);

        // add stops
        for (gint i = 0; unsigned(i) < rg->vector.stops.size(); i++) {
            float rgb[3];
            sp_color_get_rgb_floatv(&rg->vector.stops[i].color, rgb);
            cairo_pattern_add_color_stop_rgba(pattern, rg->vector.stops[i].offset, rgb[0], rgb[1], rgb[2], rg->vector.stops[i].opacity * alpha);
        }
    }

    if (pattern) {
        SPGradient *g = SP_GRADIENT(paintserver);

        // set extend type
        SPGradientSpread spread = sp_gradient_get_spread(g);
      	switch (spread) {
       		case SP_GRADIENT_SPREAD_REPEAT:
       			cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
       			break;
       		case SP_GRADIENT_SPREAD_REFLECT:
       			cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
       			break;
       		case SP_GRADIENT_SPREAD_PAD:
            default:
       			cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
       			break;
       	}

        cairo_matrix_t pattern_matrix;
        if (g->gradientTransform_set) {
	        // apply gradient transformation
	        cairo_matrix_init(&pattern_matrix,
	            g->gradientTransform[0], g->gradientTransform[1],
	            g->gradientTransform[2], g->gradientTransform[3],
	            g->gradientTransform[4], g->gradientTransform[5]);
        } else {
            cairo_matrix_init_identity (&pattern_matrix);
        }

        if (apply_bbox2user) {
            // convert to userspace
            cairo_matrix_t bbox2user;
            cairo_matrix_init (&bbox2user, pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
            cairo_matrix_multiply (&pattern_matrix, &bbox2user, &pattern_matrix);
        }
        cairo_matrix_invert(&pattern_matrix);   // because Cairo expects a userspace->patternspace matrix
        cairo_pattern_set_matrix(pattern, &pattern_matrix);
    }

    return pattern;
}

void
PrintCairoPDF::print_fill_style(cairo_t *cr, SPStyle const *const style, NRRect const *pbox)
{
    g_return_if_fail( style->fill.isColor()
                      || ( style->fill.isPaintserver()
                           && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) );

    if (style->fill.isColor()) {
        float rgb[3];
        sp_color_get_rgb_floatv(&style->fill.value.color, rgb);

        float alpha = SP_SCALE24_TO_FLOAT(style->fill_opacity.value);

        cairo_set_source_rgba(cr, rgb[0], rgb[1], rgb[2], alpha);
    } else {
        g_assert( style->fill.isPaintserver()
                  && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );

        cairo_pattern_t *pattern = create_pattern_for_paint(SP_STYLE_FILL_SERVER(style), pbox, 1.0);

        if (pattern) {
	        cairo_set_source(cr, pattern);
            cairo_pattern_destroy(pattern);
        }
    }
}

unsigned int
PrintCairoPDF::fill(Inkscape::Extension::Print *mod, Geom::PathVector const &pathv, Geom::Matrix const *ctm, SPStyle const *const style,
              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if ( style->fill.isColor()
         || ( style->fill.isPaintserver()
              && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) ) {

        float alpha = SP_SCALE24_TO_FLOAT(style->fill_opacity.value);

        cairo_save(cr);

        print_fill_style(cr, style, pbox);

        cairo_new_path(cr);
        feed_pathvector_to_cairo(cr, pathv);

        if (style->fill_rule.computed == SP_WIND_RULE_EVENODD) {
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
        } else {
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
        }
        if (alpha != 1.0 &&
            !style->fill.isColor()) {

            cairo_clip (cr);
            cairo_paint_with_alpha (cr, alpha);
        } else {
            cairo_fill(cr);
        }

        cairo_restore(cr);
    }

    return 0;
}

void
PrintCairoPDF::print_stroke_style(cairo_t *cr, SPStyle const *style, NRRect const *pbox)
{
    float alpha = SP_SCALE24_TO_FLOAT(style->stroke_opacity.value);

    if ( style->stroke.isColor() ) {
        float rgb[3];
        sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);

        cairo_set_source_rgba(cr, rgb[0], rgb[1], rgb[2], alpha);
    } else if ( style->stroke.isPaintserver()
                  && SP_IS_GRADIENT(SP_STYLE_STROKE_SERVER(style)) ) {

        cairo_pattern_t *pattern = create_pattern_for_paint(SP_STYLE_STROKE_SERVER(style), pbox, alpha);

        if (pattern) {
        	cairo_set_source(cr, pattern);
            cairo_pattern_destroy(pattern);
        }
    }

    if (style->stroke_dash.n_dash   &&
        style->stroke_dash.dash       )
    {
        cairo_set_dash(cr, style->stroke_dash.dash, style->stroke_dash.n_dash, style->stroke_dash.offset);
    } else {
    	cairo_set_dash(cr, NULL, 0, 0.0);	// disable dashing
    }

    cairo_set_line_width(cr, style->stroke_width.computed);

    // set line join type
    cairo_line_join_t join = CAIRO_LINE_JOIN_MITER;
    switch (style->stroke_linejoin.computed) {
    	case SP_STROKE_LINEJOIN_MITER:
    	    join = CAIRO_LINE_JOIN_MITER;
    	    break;
    	case SP_STROKE_LINEJOIN_ROUND:
    	    join = CAIRO_LINE_JOIN_ROUND;
    	    break;
    	case SP_STROKE_LINEJOIN_BEVEL:
    	    join = CAIRO_LINE_JOIN_BEVEL;
    	    break;
    }
    cairo_set_line_join(cr, join);

    // set line cap type
    cairo_line_cap_t cap = CAIRO_LINE_CAP_BUTT;
    switch (style->stroke_linecap.computed) {
    	case SP_STROKE_LINECAP_BUTT:
    	    cap = CAIRO_LINE_CAP_BUTT;
    	    break;
    	case SP_STROKE_LINECAP_ROUND:
    	    cap = CAIRO_LINE_CAP_ROUND;
    	    break;
    	case SP_STROKE_LINECAP_SQUARE:
    	    cap = CAIRO_LINE_CAP_SQUARE;
    	    break;
    }
    cairo_set_line_cap(cr, cap);
    cairo_set_miter_limit(cr, MAX(1, style->stroke_miterlimit.value));
}

unsigned int
PrintCairoPDF::stroke(Inkscape::Extension::Print *mod, Geom::PathVector const &pathv, Geom::Matrix const *ctm, SPStyle const *style,
                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if ( style->stroke.isColor() ||
         ( style->stroke.isPaintserver()
              && SP_IS_GRADIENT(SP_STYLE_STROKE_SERVER(style)) ) ) {

        cairo_save(cr);

        cairo_new_path(cr);
        print_stroke_style(cr, style, pbox);

        feed_pathvector_to_cairo(cr, pathv);
        cairo_stroke(cr);

        cairo_restore(cr);
    }

    return 0;
}

unsigned int
PrintCairoPDF::image(Inkscape::Extension::Print *mod, guchar *px, unsigned int w, unsigned int h, unsigned int rs,
               Geom::Matrix const *transform, SPStyle const *style)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    guchar* px_rgba = (guchar*)g_malloc(4 * w * h);
    if (!px_rgba) return 0;

    float alpha = _alpha_stack.back();

    // make a copy of the original pixbuf with premultiplied alpha
    // if we pass the original pixbuf it will get messed up
    for (unsigned i = 0; i < h; i++) {
    	for (unsigned j = 0; j < w; j++) {
            guchar const *src = px + i * rs + j * 4;
            guint32 *dst = (guint32 *)(px_rgba + i * rs + j * 4);
            guchar r, g, b, alpha_dst;

            // calculate opacity-modified alpha
            alpha_dst = src[3];
            if (alpha != 1.0)
                alpha_dst = (guchar)ceil((float)alpha_dst * alpha);

            // premul alpha (needed because this will be undone by cairo-pdf)
            r = src[0]*alpha_dst/255;
            g = src[1]*alpha_dst/255;
            b = src[2]*alpha_dst/255;

            *dst = (((alpha_dst) << 24) | (((r)) << 16) | (((g)) << 8) | (b));
    	}
    }

    cairo_surface_t *image_surface = cairo_image_surface_create_for_data(px_rgba, CAIRO_FORMAT_ARGB32, w, h, w * 4);
    if (cairo_surface_status(image_surface)) {
        g_printf("Error: %s\n", cairo_status_to_string(cairo_surface_status(image_surface)));
    	return 0;
    }

    cairo_save(cr);

	// scaling by width & height is not needed because it will be done by cairo-pdf
	// we have to translate down by height also in order to eliminate the last translation done by sp_image_print
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix,
	            (*transform)[0]/w, (*transform)[1],
	            (*transform)[2], -(*transform)[3]/h,
	            (*transform)[4], (*transform)[5] + (*transform)[3]);
    cairo_transform(cr, &matrix);

    cairo_pattern_t *pattern = cairo_pattern_create_for_surface(image_surface);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);

    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    // set clip region so that the pattern will not be repeated (bug in Cairo prior 1.2.1)
    cairo_new_path(cr);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_clip(cr);

    cairo_paint(cr);

    cairo_restore(cr);

    cairo_surface_destroy(image_surface);
    g_free(px_rgba);

    return 0;
}

#define GLYPH_ARRAY_SIZE 64

#ifndef RENDER_WITH_PANGO_CAIRO

Geom::Point
PrintCairoPDF::draw_glyphs(cairo_t *cr, Geom::Point p, PangoFont *font, PangoGlyphString *glyph_string,
               bool vertical, bool stroke)
{
    cairo_glyph_t glyph_array[GLYPH_ARRAY_SIZE];
    cairo_glyph_t *glyphs = glyph_array;
    if (glyph_string->num_glyphs > GLYPH_ARRAY_SIZE)
        glyphs = (cairo_glyph_t*)g_malloc(sizeof(cairo_glyph_t) * glyph_string->num_glyphs);

    PangoGlyphUnit x_offset = 0, y_offset = 0;
    PangoGlyphInfo *info;
    int num_invalid_glyphs = 0;
    for (gint i = 0; i < glyph_string->num_glyphs; i++) {
        info = &glyph_string->glyphs[i];
        // skip glyphs which are PANGO_GLYPH_EMPTY (0x0FFFFFFF) or have
        // the PANGO_GLYPH_UNKNOWN_FLAG (0x10000000) set
        if (info->glyph == 0x0FFFFFFF || info->glyph & 0x10000000) {
            num_invalid_glyphs++;
            continue;
        }

        glyphs[i - num_invalid_glyphs].index = info->glyph;
        glyphs[i - num_invalid_glyphs].x = p[Geom::X] + (x_offset + info->geometry.x_offset)/PANGO_SCALE;
        glyphs[i - num_invalid_glyphs].y = p[Geom::Y] + (y_offset + info->geometry.y_offset)/PANGO_SCALE;

        if (vertical) {
            cairo_text_extents_t extents;
            cairo_glyph_extents(cr, &glyphs[i - num_invalid_glyphs], 1, &extents);
            y_offset += (PangoGlyphUnit)(extents.y_advance * PANGO_SCALE);
        }
        else
            x_offset += info->geometry.width;
    }

    if (stroke)
        cairo_glyph_path(cr, glyphs, glyph_string->num_glyphs - num_invalid_glyphs);
    else
        cairo_show_glyphs(cr, glyphs, glyph_string->num_glyphs - num_invalid_glyphs);

    if (glyph_string->num_glyphs > GLYPH_ARRAY_SIZE)
        g_free(glyphs);

    return Geom::Point(x_offset, y_offset);
}
#endif



unsigned int
PrintCairoPDF::text(Inkscape::Extension::Print *mod, char const *text, Geom::Point p,
              SPStyle const *const style)
{
    bool dirty_pattern = false;

    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    cairo_save(cr);

    // this needs to be there to encounter Cairo's userspace locking semantics for set_source
    // otherwise gradients won't be shown correctly
    // printf("\n _last_tx:%f _last_ty:%f", _last_tx, _last_ty);
    cairo_translate(cr, -_last_tx, -_last_ty);

    // create pango layout and context if needed
    if (_layout == NULL) {
#ifdef RENDER_WITH_PANGO_CAIRO
        //_context = pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(pango_cairo_font_map_get_default()));
        _layout = pango_cairo_create_layout(cr);
#else
        _layout = pango_layout_new((font_factory::Default())->fontContext);
#endif
    }

    // create font instance from style and use it
    font_instance *tf = font_factory::Default()->FaceFromStyle(style);
    if (tf == NULL) {   // do something
        g_printf("Warning: trouble getting font_instance\n");
        tf = (font_factory::Default())->Face("sans", font_style_to_pos(*style));
    }
    PangoFontDescription *adjusted = pango_font_description_copy(tf->descr);

    pango_font_description_set_absolute_size(adjusted, (gint)(style->font_size.computed * PANGO_SCALE));
    pango_layout_set_font_description(_layout, adjusted);
    pango_layout_set_text(_layout, text, -1);

    pango_font_description_free(adjusted);
    tf->Unref();

#ifdef RENDER_WITH_PANGO_CAIRO
    pango_cairo_update_layout(cr, _layout);
#else
    pango_layout_context_changed(_layout);   // is this needed?
#endif

    PangoLayoutLine *line = pango_layout_get_line(_layout, 0);
    if (line == NULL)
        return 0;

#ifndef RENDER_WITH_PANGO_CAIRO
    // apply the selected font
    double size;
    PangoLayoutRun *first_run = (PangoLayoutRun *)line->runs->data;
    PangoFcFont *fc_font = PANGO_FC_FONT(first_run->item->analysis.font);
    FcPattern *fc_pattern = fc_font->font_pattern;

    if ( style->writing_mode.set &&
       (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB_RL ||
        style->writing_mode.computed == SP_CSS_WRITING_MODE_TB_LR) )
    {
        FcPatternDel(fc_pattern, FC_VERTICAL_LAYOUT);
        FcPatternAddBool(fc_pattern, FC_VERTICAL_LAYOUT, FcTrue);
        dirty_pattern = true;
    }

    cairo_font_face_t *font_face = cairo_ft_font_face_create_for_pattern(fc_pattern);
    cairo_set_font_face(cr, font_face);

    if (FcPatternGetDouble(fc_pattern, FC_PIXEL_SIZE, 0, &size) != FcResultMatch)
        size = 12.0;

    // adjust size and horizontal flip
    cairo_matrix_t matrix;
    cairo_get_font_matrix(cr, &matrix);
    matrix.xx = size;
    matrix.yy = -matrix.xx;
    cairo_set_font_matrix(cr, &matrix);
#endif

    if ( style->fill.isColor()
         || ( style->fill.isPaintserver()
              && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) )
    {
        // set fill style
        print_fill_style(cr, style, NULL);

#ifndef RENDER_WITH_PANGO_CAIRO
        Geom::Point cursor(_last_tx, _last_ty);
        for (GSList *tmpList = line->runs; tmpList && tmpList->data; tmpList = tmpList->next) {
            PangoLayoutRun *run = (PangoLayoutRun *)tmpList->data;
            cursor += draw_glyphs(cr, cursor, run->item->analysis.font, run->glyphs, dirty_pattern, false) / PANGO_SCALE;
        }
#else
        cairo_new_path(cr);
        // as pango has inverted origin we simulate the moveto and apply the vertical flip
        //cairo_move_to(cr, _last_tx, _last_ty);
        cairo_translate(cr, _last_tx, _last_ty);
        cairo_scale(cr, 1, -1);
        cairo_move_to(cr, 0, 0);
        pango_cairo_show_layout_line(cr, line);
        cairo_scale(cr, 1, -1);
        cairo_translate(cr, -_last_tx, -_last_ty);

#endif
    }

    if (style->stroke.isColor()
        || ( style->stroke.isPaintserver()
              && SP_IS_GRADIENT(SP_STYLE_STROKE_SERVER(style)) ) )
    {
        // set stroke style
        print_stroke_style(cr, style, NULL);

        // paint stroke
#ifndef RENDER_WITH_PANGO_CAIRO
        Geom::Point cursor(_last_tx, _last_ty);
        for (GSList *tmpList = line->runs; tmpList && tmpList->data; tmpList = tmpList->next) {
            PangoLayoutRun *run = (PangoLayoutRun *)tmpList->data;
            cursor += draw_glyphs(cr, cursor, run->item->analysis.font, run->glyphs, dirty_pattern, true) / PANGO_SCALE;
        }
#else
        cairo_new_path(cr);
        // as pango has inverted origin we simulate the moveto and apply the vertical flip
        //cairo_move_to(cr, _last_tx, _last_ty);
        cairo_translate(cr, _last_tx, _last_ty);
        cairo_scale(cr, 1, -1);
        cairo_move_to(cr, 0, 0);
        pango_cairo_layout_line_path(cr, line);
        cairo_scale(cr, 1, -1);
        cairo_translate(cr, -_last_tx, -_last_ty);
#endif
        cairo_stroke(cr);
    }


    cairo_restore(cr);

#ifndef RENDER_WITH_PANGO_CAIRO
    cairo_font_face_destroy(font_face);

    if (dirty_pattern) {
        FcPatternDel(fc_pattern, FC_VERTICAL_LAYOUT);
        FcPatternAddBool(fc_pattern, FC_VERTICAL_LAYOUT, FcFalse);
    }
#endif

    return 0;
}

/* Helper functions */

static void
_concat_transform(cairo_t *cr, double xx, double yx, double xy, double yy, double x0, double y0)
{
    cairo_matrix_t matrix;

    cairo_matrix_init(&matrix, xx, yx, xy, yy, x0, y0);
    cairo_transform(cr, &matrix);
}

static cairo_status_t
_write_callback(void *closure, const unsigned char *data, unsigned int length)
{
    size_t written;
    FILE *file = (FILE*)closure;

    written = fwrite (data, 1, length, file);

    if (written == length)
	return CAIRO_STATUS_SUCCESS;
    else
	return CAIRO_STATUS_WRITE_ERROR;
}

bool
PrintCairoPDF::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

#include "clear-n_.h"

void
PrintCairoPDF::init(void)
{
    /* PDF out */
    (void) Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
        "<name>" N_("PDF Print") "</name>\n"
        "<id>" SP_MODULE_KEY_PRINT_CAIRO_PDF "</id>\n"
        "<param name=\"bitmap\" type=\"boolean\">false</param>\n"
        "<param name=\"resolution\" type=\"string\">72</param>\n"
        "<param name=\"destination\" type=\"string\">| lp</param>\n"
        "<param name=\"pageBoundingBox\" type=\"boolean\">true</param>\n"
        "<param name=\"textToPath\" type=\"boolean\">true</param>\n"
        "<param name=\"exportDrawing\" type=\"boolean\">false</param>\n"
        "<param name=\"exportCanvas\" type=\"boolean\">false</param>\n"
        "<param name=\"exportId\" type=\"string\"></param>\n"
        "<print/>\n"
        "</inkscape-extension>", new PrintCairoPDF());
}


}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

/* End of GNU GPL code */

#endif /* HAVE_CAIRO_PDF */


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

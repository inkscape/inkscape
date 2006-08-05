#define __SP_PDF_C__

/** \file
 * PDF printing.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Basic printing code, EXCEPT image and
 * ascii85 filter is in public domain
 *
 * Image printing and Ascii85 filter:
 *
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997-98 Peter Kirchgessner
 * George White <aa056@chebucto.ns.ca>
 * Austin Donnelly <austin@gimp.org>
 *
 * Licensed under GNU GPL
 */

/* Plain Print */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <signal.h>
#include <errno.h>

#include <glib/gmem.h>
#include <libnr/n-art-bpath.h>
#include <libnr/nr-point-matrix-ops.h>

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
#include "sp-item.h"
#include "style.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"

#include "libnrtype/font-instance.h"
#include "libnrtype/font-style-to-pos.h"

#include <unit-constants.h>

#include "pdf.h"
#include "extension/system.h"
#include "extension/print.h"

#include "io/sys.h"

#include "pdf-mini.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

PrintPDF::PrintPDF() :
    _stream(NULL),
    _dpi(72),
    _bitmap(false)
{
    _num_alphas = 10;
    _pushed_alphas = (float*) malloc(_num_alphas*sizeof(float));
}

PrintPDF::~PrintPDF(void)
{
    free(_pushed_alphas);

    /* fixme: should really use pclose for popen'd streams */
    if (_stream) fclose(_stream);

    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif

    return;
}

unsigned int
PrintPDF::setup(Inkscape::Extension::Print * mod)
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
PrintPDF::begin(Inkscape::Extension::Print *mod, SPDocument *doc)
{
    gboolean epsexport = false;

    _latin1_encoded_fonts.clear();
    _newlatin1font_proc_defined = false;

    FILE *osf = NULL;
    FILE *osp = NULL;

    _curr_alpha = 0;
    _pushed_alphas[_curr_alpha] = 1.0;

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
            osp = popen(fn, "wb");
#else
            osp = _popen(fn, "wb");
#endif
            if (!osp) {
                fprintf(stderr, "inkscape: popen(%s): %s\n",
                        fn, strerror(errno));
                return 0;
            }
            _stream = osp;
        } else if (*fn == '>') {
            fn += 1;
            epsexport = g_str_has_suffix(fn,".eps");
            while (isspace(*fn)) fn += 1;
            Inkscape::IO::dump_fopen_call(fn, "K");
            osf = Inkscape::IO::fopen_utf8name(fn, "wb+");
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
            osp = popen(qn, "wb");
#else
            osp = _popen(qn, "wb");
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

    pdf_file = new PdfFile(_stream);
    doc_info = pdf_file->begin_document(1.4);
    *doc_info << "  /Title(" << doc->name << ")\n";
    *doc_info << "  /Author(" << g_get_real_name()  << ")\n";
//    *doc_info << "  /Subject(" << "?" << ")\n";
//    *doc_info << "  /Keywords(" << "?" << ")\n";
    *doc_info << "  /Creator(" << "www.inkscape.org" << ")\n";
    *doc_info << "  /Producer(" << "Inkscape " << PACKAGE_STRING << ")\n";
	//the date should be in ISO/IEC 8824
	GDate date;
	GTimeVal ltime;
	glong time_hh, time_mm, time_ss;
#if GLIB_CHECK_VERSION(2,9,0)
	g_date_set_time_t (&date, time (NULL));
#else
	g_date_set_time(&date, time (NULL));
#endif
	gchar date_str[100], time_str[100];
	g_date_strftime(date_str, 99, "%Y%m%d", &date);
	g_get_current_time(&ltime);
	time_hh=(ltime.tv_sec/3600)%24;
	time_mm=(ltime.tv_sec/60)%60;
	time_ss=(ltime.tv_sec)%60;
	g_snprintf(time_str, 99, "%02ld%02ld%02ld", time_hh, time_mm, time_ss); 
	*doc_info << "  /CreationDate(D:" << date_str << time_str << "Z)\n";
//	*doc_info << "  /CreationDate(D:" << date_str << time_str << "OHH'mm')\n";

    /* flush this to test output stream as early as possible */
    if (fflush(_stream)) {
        /*g_print("caught error in sp_module_print_plain_begin\n");*/
        if (ferror(_stream)) {
            g_print("Error %d on output stream: %s\n", errno,
                    g_strerror(errno));
        }
        g_print("Printing failed\n");
        /* fixme: should use pclose() for pipes */
        fclose(_stream);
        _stream = NULL;
        fflush(stdout);
        return 0;
    }

    // width and height in pt
    _width = sp_document_width(doc) * PT_PER_PX;
    _height = sp_document_height(doc) * PT_PER_PX;

    NRRect d;
    bool   pageBoundingBox;
    pageBoundingBox = mod->get_param_bool("pageBoundingBox");
    // printf("Page Bounding Box: %s\n", pageBoundingBox ? "TRUE" : "FALSE");
    if (pageBoundingBox) {
        d.x0 = d.y0 = 0;
        d.x1 = _width;
        d.y1 = _height;
    } else {
        SPItem* doc_item = SP_ITEM(sp_document_root(doc));
        sp_item_invoke_bbox(doc_item, &d, sp_item_i2r_affine(doc_item), TRUE);
        // convert from px to pt
        d.x0 *= PT_PER_PX;
        d.x1 *= PT_PER_PX;
        d.y0 *= PT_PER_PX;
        d.y1 *= PT_PER_PX;
    }

    page_stream = pdf_file->begin_page( d.x0, d.y0, d.x1, d.y1 );

    if (!_bitmap) {
        Inkscape::SVGOStringStream os;
        os.setf(std::ios::fixed);
        os << PT_PER_PX << " 0 0 "
           << -PT_PER_PX << " 0 " << (int) ceil(_height)
           << " cm\n";
        // from now on we can output px, but they will be treated as pt
        pdf_file->puts(os);
    }
    
    return 1;
}

unsigned int
PrintPDF::finish(Inkscape::Extension::Print *mod)
{
    if (!_stream) return 0;

    if (_bitmap) {
        double const dots_per_pt = _dpi / PT_PER_IN;

        double const x0 = 0.0;
        double const y0 = 0.0;
        double const x1 = x0 + _width;
        double const y1 = y0 + _height;

        /* Bitmap width/height in bitmap dots. */
        int const width = (int) (_width * dots_per_pt + 0.5);
        int const height = (int) (_height * dots_per_pt + 0.5);

        NRMatrix affine;
        affine.c[0] = width / ((x1 - x0) * PX_PER_PT);
        affine.c[1] = 0.0;
        affine.c[2] = 0.0;
        affine.c[3] = height / ((y1 - y0) * PX_PER_PT);
        affine.c[4] = -affine.c[0] * x0;
        affine.c[5] = -affine.c[3] * y0;

        nr_arena_item_set_transform(mod->root, &affine);

        guchar *const px = g_new(guchar, 4 * width * 64);

        for (int y = 0; y < height; y += 64) {
            /* Set area of interest. */
            NRRectL bbox;
            bbox.x0 = 0;
            bbox.y0 = y;
            bbox.x1 = width;
            bbox.y1 = MIN(height, y + 64);

            /* Update to renderable state. */
            NRGC gc(NULL);
            nr_matrix_set_identity(&gc.transform);
            nr_arena_item_invoke_update(mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);
            /* Render */
            /* This should take guchar* instead of unsigned char*) */
            NRPixBlock pb;
            nr_pixblock_setup_extern(&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                                     bbox.x0, bbox.y0, bbox.x1, bbox.y1,
                                     (guchar*)px, 4 * width, FALSE, FALSE);
            memset(px, 0xff, 4 * width * 64);
            nr_arena_item_invoke_render(mod->root, &bbox, &pb, 0);
            /* Blitter goes here */
            NRMatrix imgt;
            imgt.c[0] = (bbox.x1 - bbox.x0) / dots_per_pt;
            imgt.c[1] = 0.0;
            imgt.c[2] = 0.0;
            imgt.c[3] = (bbox.y1 - bbox.y0) / dots_per_pt;
            imgt.c[4] = 0.0;
            imgt.c[5] = _height - y / dots_per_pt - (bbox.y1 - bbox.y0) / dots_per_pt;

            print_image(_stream, px, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0, 4 * width, &imgt);
        }

        g_free(px);
    }

    pdf_file->end_page(page_stream);
    pdf_file->end_document(doc_info);

    delete pdf_file;

    /* Flush stream to be sure. */
    (void) fflush(_stream);

    /* fixme: should really use pclose for popen'd streams */
    fclose(_stream);
    _stream = 0;
    _latin1_encoded_fonts.clear();

    return 0;
}

unsigned int
PrintPDF::bind(Inkscape::Extension::Print *mod, NRMatrix const *transform, float opacity)
{
    if (!_stream) return 0;  // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed);
    
    os << "q\n";
    os << transform->c[0] << " "
       << transform->c[1] << " "
       << transform->c[2] << " "
       << transform->c[3] << " "
       << transform->c[4] << " "
       << transform->c[5] << " cm\n";

    float alpha = opacity * _pushed_alphas[_curr_alpha];
        
    _curr_alpha++;
    if (_curr_alpha >= _num_alphas) {
        _num_alphas = _num_alphas*2;
        _pushed_alphas = (float *) realloc(_pushed_alphas, _num_alphas*sizeof(float));
    }
    _pushed_alphas[_curr_alpha] = alpha;

    pdf_file->puts(os);
    return 1;
}

unsigned int
PrintPDF::release(Inkscape::Extension::Print *mod)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if (_curr_alpha > 0)
        _curr_alpha--;

    pdf_file->puts("Q\n");

    return 1;
}

unsigned int
PrintPDF::comment(Inkscape::Extension::Print *mod, char const *comment)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;
 
    return 1;
}

void
PrintPDF::print_fill_alpha(SVGOStringStream &os, SPStyle const *const style, NRRect const *pbox)
{
    g_return_if_fail( style->fill.type == SP_PAINT_TYPE_COLOR
                      || ( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
                           && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) );
    
    if (style->fill.type == SP_PAINT_TYPE_COLOR) {
        float alpha = 1.0;
        alpha *= SP_SCALE24_TO_FLOAT(style->fill_opacity.value);
        alpha *= _pushed_alphas[_curr_alpha];

        if (alpha != 1.0) {
            PdfObject *pdf_alpha = pdf_file->begin_resource(pdf_extgstate);
            *pdf_alpha << "<< /Type /ExtGState\n";
            *pdf_alpha << "   /ca " << alpha << "\n";
            *pdf_alpha << "   /AIS false\n";
            *pdf_alpha << ">>\n";
            
            os << pdf_alpha->get_name()
               << " gs\n";

            pdf_file->end_resource(pdf_alpha);
        }
    } else {
        g_assert( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
                  && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );
        
        if (SP_IS_LINEARGRADIENT (SP_STYLE_FILL_SERVER (style))) {
            
            SPLinearGradient *lg=SP_LINEARGRADIENT(SP_STYLE_FILL_SERVER (style));

            sp_gradient_ensure_vector(SP_GRADIENT(lg)); // when exporting from commandline, vector is not built

            NR::Point p1 (lg->x1.computed, lg->y1.computed);
            NR::Point p2 (lg->x2.computed, lg->y2.computed);
            if (pbox && SP_GRADIENT(lg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                NR::Matrix bbox2user(pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
                p1 *= bbox2user;
                p2 *= bbox2user;
            }
            
            double alpha = 1.0;
            for (gint i = 0; unsigned(i) < lg->vector.stops.size(); i++) {
                alpha *= lg->vector.stops[i].opacity;
            }
            
            if (alpha != 1.0 || _pushed_alphas[_curr_alpha] != 1.0) {
                PdfObject *pdf_gstate = pdf_file->begin_resource(pdf_extgstate);
                *pdf_gstate << "<< /Type /ExtGState\n";
                
                if (_pushed_alphas[_curr_alpha] != 1.0) {
                    *pdf_gstate << "   /ca " << _pushed_alphas[_curr_alpha] << "\n";
                    *pdf_gstate << "   /AIS false\n";
                }
                
                if (alpha != 1.0) {
                    PdfObject *pdf_alpha = pdf_file->begin_resource(pdf_shading);
                    PdfObject *pdf_smask = pdf_file->begin_resource(pdf_none);
                    PdfObject *pdf_xobj = pdf_file->begin_resource(pdf_none);


                    *pdf_gstate << "   /SMask " << pdf_smask->get_id() << " 0 R\n";

            
                    *pdf_alpha << "<<\n/ShadingType 2\n/ColorSpace /DeviceGray\n";
                    *pdf_alpha << "/Coords [" << p1[NR::X] << " " << p1[NR::Y] << " " << p2[NR::X] << " " << p2[NR::Y] <<"]\n";
                    *pdf_alpha << "/Extend [true true]\n";
                    *pdf_alpha << "/Domain [0 1]\n";
                    *pdf_alpha << "/Function <<\n/FunctionType 3\n/Functions\n[\n";

                    for (gint i = 0; unsigned(i) < lg->vector.stops.size() - 1; i++) {
                        *pdf_alpha << "<<\n/FunctionType 2\n/Domain [0 1]\n";
                        *pdf_alpha << "/C0 [" << lg->vector.stops[i].opacity << "]\n";
                        *pdf_alpha << "/C1 [" << lg->vector.stops[i+1].opacity << "]\n";
                        *pdf_alpha << "/N 1\n>>\n";
                    }
                    *pdf_alpha << "]\n/Domain [0 1]\n";
                    *pdf_alpha << "/Bounds [ ";
                    for (gint i=0;unsigned(i)<lg->vector.stops.size()-2;i++) {
                        *pdf_alpha << lg->vector.stops[i+1].offset <<" ";
                    }
                    *pdf_alpha << "]\n";
                    *pdf_alpha << "/Encode [ ";
                    for (gint i=0;unsigned(i)<lg->vector.stops.size()-1;i++) {
                        *pdf_alpha << "0 1 ";
                    }
                    *pdf_alpha << "]\n";
                    *pdf_alpha << ">>\n>>\n";

                    
                    *pdf_smask << "<< /Type /Mask\n";
                    *pdf_smask << "   /S /Alpha\n";
                    *pdf_smask << "   /BC [0.0]\n";
                    *pdf_smask << "   /G " << pdf_xobj->get_id() << " 0 R\n";
                    *pdf_smask << ">>\n";

                    
                    *pdf_xobj << "<< /Type /XObject\n";
                    *pdf_xobj << "   /Subtype /Form\n";
                    *pdf_xobj << "   /FormType 1\n";
                    *pdf_xobj << "   /BBox ["
                              << pbox->x0 << " "
                              << pbox->y0 << " "
                              << pbox->x1 << " "
                              << pbox->y1 << "]\n";
                    *pdf_xobj << "   /Group\n";
                    *pdf_xobj << "   << /Type /Group\n";
                    *pdf_xobj << "      /S /Transparency\n";
                    *pdf_xobj << "      /CS /DeviceGray \n";
                    *pdf_xobj << "   >>\n";
                    
                    Inkscape::SVGOStringStream os;
                    os.setf(std::ios::fixed);
                    
                    os << "q\n"
                       << pbox->x0 << " " << pbox->y0 << " m\n"
                       << pbox->x1 << " " << pbox->y0 << " l\n"
                       << pbox->x1 << " " << pbox->y1 << " l\n"
                       << pbox->x0 << " " << pbox->y1 << " l\n"
                       << pbox->x0 << " " << pbox->y0 << " l\n"
                       << "h\n"
                       << "W* n\n";
                    
                    SPGradient const *g = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
                    if (g->gradientTransform_set) {
                        os << g->gradientTransform[0] << " "
                           << g->gradientTransform[1] << " "
                           << g->gradientTransform[2] << " "
                           << g->gradientTransform[3] << " "
                           << g->gradientTransform[4] << " "
                           << g->gradientTransform[5] << " cm\n"; 
                    }
                    
                    os << pdf_alpha->get_name() << " sh\n";
                    os << "Q\n";
                    *pdf_xobj << "   /Length " << strlen(os.str().c_str()) << "\n";
                    *pdf_xobj << ">>\n";
                    *pdf_xobj << "stream\n";
                    *pdf_xobj << os.str().c_str();
                    *pdf_xobj << "endstream\n";


                    pdf_file->end_resource(pdf_alpha);
                    pdf_file->end_resource(pdf_smask);
                    pdf_file->end_resource(pdf_xobj);
                }
                
                *pdf_gstate << ">>\n";

                os << pdf_gstate->get_name() << " gs\n";
                    
                pdf_file->end_resource(pdf_gstate);
            }
        } else if (SP_IS_RADIALGRADIENT (SP_STYLE_FILL_SERVER (style))) {

            SPRadialGradient *rg=SP_RADIALGRADIENT(SP_STYLE_FILL_SERVER (style));

            sp_gradient_ensure_vector(SP_GRADIENT(rg)); // when exporting from commandline, vector is not built

            NR::Point c (rg->cx.computed, rg->cy.computed);
            NR::Point f (rg->fx.computed, rg->fy.computed);
            double r = rg->r.computed;

            NR::Coord const df = hypot(f[NR::X] - c[NR::X], f[NR::Y] - c[NR::Y]);
            if (df >= r) {
                f[NR::X] = c[NR::X] + (f[NR::X] - c[NR::X] ) * r / (float) df;
                f[NR::Y] = c[NR::Y] + (f[NR::Y] - c[NR::Y] ) * r / (float) df;
            }

            if (pbox && SP_GRADIENT(rg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                NR::Matrix bbox2user(pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
                c *= bbox2user;
                f *= bbox2user;
                r *= bbox2user.expansion();
            }
            
            double alpha = 1.0;
            for (gint i = 0; unsigned(i) < rg->vector.stops.size(); i++) {
                alpha *= rg->vector.stops[i].opacity;
            }

            if (alpha != 1.0 || _pushed_alphas[_curr_alpha] != 1.0) {
                PdfObject *pdf_gstate = pdf_file->begin_resource(pdf_extgstate);
                *pdf_gstate << "<< /Type /ExtGState\n";
                
                if (_pushed_alphas[_curr_alpha] != 1.0) {
                    *pdf_gstate << "   /ca " << _pushed_alphas[_curr_alpha] << "\n";
                    *pdf_gstate << "   /AIS false\n";
                }
                
                if (alpha != 1.0) {
                    PdfObject *pdf_alpha = pdf_file->begin_resource(pdf_shading);
                    PdfObject *pdf_smask = pdf_file->begin_resource(pdf_none);
                    PdfObject *pdf_xobj = pdf_file->begin_resource(pdf_none);


                    *pdf_gstate << "   /SMask " << pdf_smask->get_id() << " 0 R\n";


                    *pdf_alpha << "<<\n/ShadingType 3\n/ColorSpace /DeviceGray\n";
                    *pdf_alpha << "/Coords ["<< f[NR::X] <<" "<< f[NR::Y] <<" 0 "<< c[NR::X] <<" "<< c[NR::Y] <<" "<< r <<"]\n";
                    *pdf_alpha << "/Extend [true true]\n";
                    *pdf_alpha << "/Domain [0 1]\n";
                    *pdf_alpha << "/Function <<\n/FunctionType 3\n/Functions\n[\n";

                    for (gint i = 0; unsigned(i) < rg->vector.stops.size() - 1; i++) {
                        *pdf_alpha << "<<\n/FunctionType 2\n/Domain [0 1]\n";
                        *pdf_alpha << "/C0 [" << rg->vector.stops[i].opacity << "]\n";
                        *pdf_alpha << "/C1 [" << rg->vector.stops[i+1].opacity << "]\n";
                        *pdf_alpha << "/N 1\n>>\n";
                    }
                    *pdf_alpha << "]\n/Domain [0 1]\n";
                    *pdf_alpha << "/Bounds [ ";

                    for (gint i=0;unsigned(i)<rg->vector.stops.size()-2;i++) {
                        *pdf_alpha << rg->vector.stops[i+1].offset <<" ";
                    }
                    *pdf_alpha << "]\n";
                    *pdf_alpha << "/Encode [ ";
                    for (gint i=0;unsigned(i)<rg->vector.stops.size()-1;i++) {
                        *pdf_alpha << "0 1 ";
                    }
                    *pdf_alpha << "]\n";
                    *pdf_alpha << ">>\n>>\n";
                    
                    
                    *pdf_smask << "<< /Type /Mask\n";
                    *pdf_smask << "   /S /Alpha\n";
                    *pdf_smask << "   /BC [0.0]\n";
                    *pdf_smask << "   /G " << pdf_xobj->get_id() << " 0 R\n";
                    *pdf_smask << ">>\n";

                    
                    *pdf_xobj << "<< /Type /XObject\n";
                    *pdf_xobj << "   /Subtype /Form\n";
                    *pdf_xobj << "   /FormType 1\n";
                    *pdf_xobj << "   /BBox ["
                              << pbox->x0 << " "
                              << pbox->y0 << " "
                              << pbox->x1 << " "
                              << pbox->y1 << "]\n";
                    *pdf_xobj << "   /Group\n";
                    *pdf_xobj << "   << /Type /Group\n";
                    *pdf_xobj << "      /S /Transparency\n";
                    *pdf_xobj << "      /CS /DeviceGray \n";
                    *pdf_xobj << "   >>\n";
                    
                    Inkscape::SVGOStringStream os;
                    os.setf(std::ios::fixed);
                    
                    os << "q\n"
                       << pbox->x0 << " " << pbox->y0 << " m\n"
                       << pbox->x1 << " " << pbox->y0 << " l\n"
                       << pbox->x1 << " " << pbox->y1 << " l\n"
                       << pbox->x0 << " " << pbox->y1 << " l\n"
                       << pbox->x0 << " " << pbox->y0 << " l\n"
                       << "h\n"
                       << "W* n\n";
                    
                    SPGradient const *g = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
                    if (g->gradientTransform_set) {
                        os << g->gradientTransform[0] << " "
                           << g->gradientTransform[1] << " "
                           << g->gradientTransform[2] << " "
                           << g->gradientTransform[3] << " "
                           << g->gradientTransform[4] << " "
                           << g->gradientTransform[5] << " cm\n"; 
                    }
                    
                    os << pdf_alpha->get_name() << " sh\n";
                    os << "Q\n";
                    *pdf_xobj << "   /Length " << strlen(os.str().c_str()) << "\n";
                    *pdf_xobj << ">>\n";
                    *pdf_xobj << "stream\n";
                    *pdf_xobj << os.str().c_str();
                    *pdf_xobj << "endstream\n";


                    pdf_file->end_resource(pdf_alpha);
                    pdf_file->end_resource(pdf_smask);
                    pdf_file->end_resource(pdf_xobj);
                }
                
                *pdf_gstate << ">>\n";
                
                os << pdf_gstate->get_name() << " gs\n";
                
                pdf_file->end_resource(pdf_gstate);
            }
        }
    }
}

void
PrintPDF::print_fill_style(SVGOStringStream &os, SPStyle const *const style, NRRect const *pbox)
{
    g_return_if_fail( style->fill.type == SP_PAINT_TYPE_COLOR
                      || ( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
                           && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) );
    
    if (style->fill.type == SP_PAINT_TYPE_COLOR) {
        float rgb[3];
        sp_color_get_rgb_floatv(&style->fill.value.color, rgb);

        os << rgb[0] << " " << rgb[1] << " " << rgb[2] << " rg\n";
    } else {
        g_assert( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
                  && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );

        if (SP_IS_LINEARGRADIENT (SP_STYLE_FILL_SERVER (style))) {

            SPLinearGradient *lg=SP_LINEARGRADIENT(SP_STYLE_FILL_SERVER (style));
            NR::Point p1 (lg->x1.computed, lg->y1.computed);
            NR::Point p2 (lg->x2.computed, lg->y2.computed);
            if (pbox && SP_GRADIENT(lg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                NR::Matrix bbox2user(pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
                p1 *= bbox2user;
                p2 *= bbox2user;
            }

            PdfObject *pdf_shade = pdf_file->begin_resource(pdf_shading);

            *pdf_shade << "<<\n/ShadingType 2\n/ColorSpace /DeviceRGB\n";
            *pdf_shade << "/Coords [" << p1[NR::X] << " " << p1[NR::Y] << " " << p2[NR::X] << " " << p2[NR::Y] <<"]\n";
            *pdf_shade << "/Extend [true true]\n";
            *pdf_shade << "/Domain [0 1]\n";
            *pdf_shade << "/Function <<\n/FunctionType 3\n/Functions\n[\n";

            sp_gradient_ensure_vector(SP_GRADIENT(lg)); // when exporting from commandline, vector is not built
            for (gint i = 0; unsigned(i) < lg->vector.stops.size() - 1; i++) {
                float rgb[3];
                sp_color_get_rgb_floatv(&lg->vector.stops[i].color, rgb);
                *pdf_shade << "<<\n/FunctionType 2\n/Domain [0 1]\n";
                *pdf_shade << "/C0 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                sp_color_get_rgb_floatv(&lg->vector.stops[i+1].color, rgb);
                *pdf_shade << "/C1 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                *pdf_shade << "/N 1\n>>\n";
            }
            *pdf_shade << "]\n/Domain [0 1]\n";
            *pdf_shade << "/Bounds [ ";
            for (gint i=0;unsigned(i)<lg->vector.stops.size()-2;i++) {
                *pdf_shade << lg->vector.stops[i+1].offset <<" ";
            }
            *pdf_shade << "]\n";
            *pdf_shade << "/Encode [ ";
            for (gint i=0;unsigned(i)<lg->vector.stops.size()-1;i++) {
                *pdf_shade << "0 1 ";
            }
            *pdf_shade << "]\n";
            *pdf_shade << ">>\n>>\n";
            os << pdf_shade->get_name() << " ";

            pdf_file->end_resource(pdf_shade);
        } else if (SP_IS_RADIALGRADIENT (SP_STYLE_FILL_SERVER (style))) {

            SPRadialGradient *rg=SP_RADIALGRADIENT(SP_STYLE_FILL_SERVER (style));
            NR::Point c (rg->cx.computed, rg->cy.computed);
            NR::Point f (rg->fx.computed, rg->fy.computed);
            double r = rg->r.computed;

            NR::Coord const df = hypot(f[NR::X] - c[NR::X], f[NR::Y] - c[NR::Y]);
            if (df >= r) {
                f[NR::X] = c[NR::X] + (f[NR::X] - c[NR::X] ) * r / (float) df;
                f[NR::Y] = c[NR::Y] + (f[NR::Y] - c[NR::Y] ) * r / (float) df;
            }

            if (pbox && SP_GRADIENT(rg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                NR::Matrix bbox2user(pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
                c *= bbox2user;
                f *= bbox2user;
                r *= bbox2user.expansion();
            }

            PdfObject *pdf_shade = pdf_file->begin_resource(pdf_shading);

            *pdf_shade << "<<\n/ShadingType 3\n/ColorSpace /DeviceRGB\n";
            *pdf_shade << "/Coords ["<< f[NR::X] <<" "<< f[NR::Y] <<" 0 "<< c[NR::X] <<" "<< c[NR::Y] <<" "<< r <<"]\n";
            *pdf_shade << "/Extend [true true]\n";
            *pdf_shade << "/Domain [0 1]\n";
            *pdf_shade << "/Function <<\n/FunctionType 3\n/Functions\n[\n";

            sp_gradient_ensure_vector(SP_GRADIENT(rg)); // when exporting from commandline, vector is not built
            for (gint i = 0; unsigned(i) < rg->vector.stops.size() - 1; i++) {
                float rgb[3];
                sp_color_get_rgb_floatv(&rg->vector.stops[i].color, rgb);
                *pdf_shade << "<<\n/FunctionType 2\n/Domain [0 1]\n";
                *pdf_shade << "/C0 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                sp_color_get_rgb_floatv(&rg->vector.stops[i+1].color, rgb);
                *pdf_shade << "/C1 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                *pdf_shade << "/N 1\n>>\n";
            }
            *pdf_shade << "]\n/Domain [0 1]\n";
            *pdf_shade << "/Bounds [ ";

            for (gint i=0;unsigned(i)<rg->vector.stops.size()-2;i++) {
                *pdf_shade << rg->vector.stops[i+1].offset <<" ";
            }
            *pdf_shade << "]\n";
            *pdf_shade << "/Encode [ ";
            for (gint i=0;unsigned(i)<rg->vector.stops.size()-1;i++) {
                *pdf_shade << "0 1 ";
            }
            *pdf_shade << "]\n";
            *pdf_shade << ">>\n>>\n";

            os << pdf_shade->get_name() << " ";

            pdf_file->end_resource(pdf_shade);
        }
    }
}

void
PrintPDF::print_stroke_style(SVGOStringStream &os, SPStyle const *style)
{
    float rgb[3];
    
    sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);
    os << rgb[0] << " " << rgb[1] << " " << rgb[2] << " RG\n";
        
    float alpha = 1.0;
    alpha *= SP_SCALE24_TO_FLOAT(style->stroke_opacity.value);
    alpha *= _pushed_alphas[_curr_alpha];

    if (alpha != 1.0) {
        PdfObject *pdf_alpha = pdf_file->begin_resource(pdf_extgstate);
        *pdf_alpha << "<< /Type /ExtGState\n";
        *pdf_alpha << "   /CA " << alpha << "\n";
        *pdf_alpha << "   /AIS false\n";
        *pdf_alpha << ">>\n";
        
        os << pdf_alpha->get_name() << " gs\n";
        
        pdf_file->end_resource(pdf_alpha);
    }

    // There are rare cases in which for a solid line stroke_dasharray_set is true. To avoid
    // invalid PS-lines such as "[0.0000000 0.0000000] 0.0000000 setdash", which should be "[] 0 setdash",
    // we first check if all components of stroke_dash.dash are 0.
    bool LineSolid = true;
    if (style->stroke_dash.n_dash   &&
        style->stroke_dash.dash       )
    {
        int i = 0;
        while (LineSolid && (i < style->stroke_dash.n_dash)) {
                if (style->stroke_dash.dash[i] > 0.00000001)
                    LineSolid = false;
                i++;
        }
        if (!LineSolid) {
            os << "[";
            for (i = 0; i < style->stroke_dash.n_dash; i++) {
                if (i > 0) {
                    os << " ";
                }
                os << style->stroke_dash.dash[i];
            }
            os << "] " << style->stroke_dash.offset << " d\n";
        } else {
            os << "[] 0 d\n";
        }
    } else {
        os << "[] 0 d\n";
    }

    os << style->stroke_width.computed << " w\n";
    os << style->stroke_linejoin.computed << " j\n";
    os << style->stroke_linecap.computed << " J\n";
    os <<
        ( style->stroke_miterlimit.value > 1 ?
          style->stroke_miterlimit.value : 1 ) << " M\n";
}


unsigned int
PrintPDF::fill(Inkscape::Extension::Print *mod, NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *const style,
              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed);

    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if ( style->fill.type == SP_PAINT_TYPE_COLOR ) {
        os << "q\n";
        print_fill_style(os, style, pbox);
        print_fill_alpha(os, style, pbox);
        print_bpath(os, bpath->path);
        if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
            os << "f*\n";
        } else {
            os << "f\n";
        }
        os << "Q\n";
    }
    else if ( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
              && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) )
    {
        os << "q\n";
        print_bpath(os, bpath->path);

        if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
            g_assert( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
                      && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );
            SPGradient const *g = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
            os << "W* n\n";
            print_fill_alpha(os, style, pbox);
            if (g->gradientTransform_set) {
                os << "q\n";
                os << g->gradientTransform[0] << " "
                   << g->gradientTransform[1] << " "
                   << g->gradientTransform[2] << " "
                   << g->gradientTransform[3] << " "
                   << g->gradientTransform[4] << " "
                   << g->gradientTransform[5] << " cm\n";
            }
            print_fill_style(os, style, pbox);
            os << "sh\n";
            if (g->gradientTransform_set) {
                os << "Q\n";
            }
        } else {
            g_assert( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
                      && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );
            SPGradient const *g = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
            os << "W n\n";
            print_fill_alpha(os, style, pbox);
            if (g->gradientTransform_set) {
                os << "q\n";
                os << g->gradientTransform[0] << " "
                   << g->gradientTransform[1] << " "
                   << g->gradientTransform[2] << " "
                   << g->gradientTransform[3] << " "
                   << g->gradientTransform[4] << " "
                   << g->gradientTransform[5] << " cm\n"; 
            }
            print_fill_style(os, style, pbox);
            os << "sh\n";
            if (g->gradientTransform_set) {
                os << "Q\n";
            }
        }

        os << "Q\n";
    }        

    pdf_file->puts(os);
    return 0;
}


unsigned int
PrintPDF::stroke(Inkscape::Extension::Print *mod, NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *style,
                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if (style->stroke.type == SP_PAINT_TYPE_COLOR) {
        Inkscape::SVGOStringStream os;
        os.setf(std::ios::fixed);

        os << "q\n";

        print_stroke_style(os, style);
        print_bpath(os, bpath->path);
        os << "S\n";

        os << "Q\n";

        pdf_file->puts(os);
    }

    return 0;
}

unsigned int
PrintPDF::image(Inkscape::Extension::Print *mod, guchar *px, unsigned int w, unsigned int h, unsigned int rs,
               NRMatrix const *transform, SPStyle const *style)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    return 0;
    
    return print_image(_stream, px, w, h, rs, transform);
#if 0
    fprintf(_stream, "gsave\n");
    fprintf(_stream, "/rowdata %d string def\n", 3 * w);
    fprintf(_stream, "[%g %g %g %g %g %g] concat\n",
            transform->c[0],
            transform->c[1],
            transform->c[2],
            transform->c[3],
            transform->c[4],
            transform->c[5]);
    fprintf(_stream, "%d %d 8 [%d 0 0 -%d 0 %d]\n", w, h, w, h, h);
    fprintf(_stream, "{currentfile rowdata readhexstring pop}\n");
    fprintf(_stream, "false 3 colorimage\n");

    for (unsigned int r = 0; r < h; r++) {
        guchar *s;
        unsigned int c0, c1, c;
        s = px + r * rs;
        for (c0 = 0; c0 < w; c0 += 24) {
            c1 = MIN(w, c0 + 24);
            for (c = c0; c < c1; c++) {
                static char const xtab[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
                fputc(xtab[s[0] >> 4], _stream);
                fputc(xtab[s[0] & 0xf], _stream);
                fputc(xtab[s[1] >> 4], _stream);
                fputc(xtab[s[1] & 0xf], _stream);
                fputc(xtab[s[2] >> 4], _stream);
                fputc(xtab[s[2] & 0xf], _stream);
                s += 4;
            }
            fputs("\n", _stream);
        }
    }

    fprintf(_stream, "grestore\n");

    return 0;
#endif
}

char const *
PrintPDF::PSFontName(SPStyle const *style)
{
    font_instance *tf = (font_factory::Default())->Face(style->text->font_family.value, font_style_to_pos(*style));

    char const *n;
    char name_buf[256];

    if (tf) {
        tf->PSName(name_buf, sizeof(name_buf));
        n = name_buf;
        tf->Unref();
    } else {
        // this system does not have this font, so just use the name from SVG in the hope that PS interpreter will make sense of it
        bool i = (style->font_style.value == SP_CSS_FONT_STYLE_ITALIC);
        bool o = (style->font_style.value == SP_CSS_FONT_STYLE_OBLIQUE);
        bool b = (style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLD) ||
            (style->font_weight.value >= SP_CSS_FONT_WEIGHT_500 && style->font_weight.value <= SP_CSS_FONT_WEIGHT_900);

        n = g_strdup_printf("%s%s%s%s",
                            g_strdelimit(style->text->font_family.value, " ", '-'),
                            (b || i || o) ? "-" : "",
                            (b) ? "Bold" : "",
                            (i) ? "Italic" : ((o) ? "Oblique" : "") );
    }

    return g_strdup(n);
}


unsigned int
PrintPDF::text(Inkscape::Extension::Print *mod, char const *text, NR::Point p,
              SPStyle const *const style)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed);

    return 0;
    
    // Escape chars
    Inkscape::SVGOStringStream escaped_text;
    escaped_text.setf(std::ios::fixed);
    
    escaped_text << std::oct;
    for (gchar const *p_text = text ; *p_text ; p_text = g_utf8_next_char(p_text)) {
        gunichar const c = g_utf8_get_char(p_text);
        if (c == '\\' || c == ')' || c == '(')
            escaped_text << '\\' << static_cast<char>(c);
        else if (c >= 0x80)
            escaped_text << '\\' << c;
        else
            escaped_text << static_cast<char>(c);
    }

    os << "gsave\n";

    // set font
    char const *fn = PSFontName(style);
    if (_latin1_encoded_fonts.find(fn) == _latin1_encoded_fonts.end()) {
        if (!_newlatin1font_proc_defined) {
            // input: newfontname, existingfontname
            // output: new font object, also defined to newfontname
            os << "/newlatin1font "         // name of the proc
                  "{findfont dup length dict copy "     // load the font and create a copy of it
                  "dup /Encoding ISOLatin1Encoding put "     // change the encoding in the copy
                  "definefont} def\n";      // create the new font and leave it on the stack, define the proc
            _newlatin1font_proc_defined = true;
        }
        os << "/" << fn << "-ISOLatin1 /" << fn << " newlatin1font\n";
        _latin1_encoded_fonts.insert(fn);
    } else
        os << "/" << fn << "-ISOLatin1 findfont\n";
    os << style->font_size.computed << " scalefont\n";
    os << "setfont\n";
    g_free((void *) fn);

    if ( style->fill.type == SP_PAINT_TYPE_COLOR
         || ( style->fill.type == SP_PAINT_TYPE_PAINTSERVER
              && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) )
    {
        // set fill style
        print_fill_style(os, style, NULL);
        // FIXME: we don't know the pbox of text, so have to pass NULL. This means gradients with
        // bbox units won't work with text. However userspace gradients don't work with text either
        // (text is black) for some reason.

        os << "newpath\n";
        os << p[NR::X] << " " << p[NR::Y] << " moveto\n";
        os << "(" << escaped_text.str() << ") show\n";
    }

    if (style->stroke.type == SP_PAINT_TYPE_COLOR) {

        // set stroke style
        print_stroke_style(os, style);

        // paint stroke
        os << "newpath\n";
        os << p[NR::X] << " " << p[NR::Y] << " moveto\n";
        os << "(" << escaped_text.str() << ") false charpath stroke\n";
    }

    os << "grestore\n";

    fprintf(_stream, "%s", os.str().c_str());

    return 0;
}



/* PDF helpers */

void
PrintPDF::print_bpath(SVGOStringStream &os, NArtBpath const *bp)
{
    bool closed = false;
    while (bp->code != NR_END) {
        switch (bp->code) {
            case NR_MOVETO:
                if (closed) {
                    os << "h\n";
                }
                closed = true;
                os << bp->x3 << " " << bp->y3 << " m\n";
                break;
            case NR_MOVETO_OPEN:
                if (closed) {
                    os << "h\n";
                }
                closed = false;
                os << bp->x3 << " " << bp->y3 << " m\n";
                break;
            case NR_LINETO:
                os << bp->x3 << " " << bp->y3 << " l\n";
                break;
            case NR_CURVETO:
                os << bp->x1 << " " << bp->y1 << " "
                   << bp->x2 << " " << bp->y2 << " "
                   << bp->x3 << " " << bp->y3 << " c\n";
                break;
            default:
                break;
        }
        bp += 1;
    }
    if (closed) {
        os << "h\n";
    }
}

/* The following code is licensed under GNU GPL.
** The packbits, ascii85 and imaging printing code
** is from the gimp's postscript.c.
*/

/**
* \param nin Number of bytes of source data.
* \param src Source data.
* \param nout Number of output bytes.
* \param dst Buffer for output.
*/
void
PrintPDF::compress_packbits(int nin,
                           guchar *src,
                           int *nout,
                           guchar *dst)

{
    register guchar c;
    int nrepeat, nliteral;
    guchar *run_start;
    guchar *start_dst = dst;
    guchar *last_literal = NULL;

    for (;;) {
        if (nin <= 0) break;

        run_start = src;
        c = *run_start;

        /* Search repeat bytes */
        if ((nin > 1) && (c == src[1])) {
            nrepeat = 1;
            nin -= 2;
            src += 2;
            while ((nin > 0) && (c == *src)) {
                nrepeat++;
                src++;
                nin--;
                if (nrepeat == 127) break; /* Maximum repeat */
            }

            /* Add two-byte repeat to last literal run ? */
            if ( (nrepeat == 1)
                 && (last_literal != NULL) && (((*last_literal)+1)+2 <= 128) )
            {
                *last_literal += 2;
                *(dst++) = c;
                *(dst++) = c;
                continue;
            }

            /* Add repeat run */
            *(dst++) = (guchar)((-nrepeat) & 0xff);
            *(dst++) = c;
            last_literal = NULL;
            continue;
        }
        /* Search literal bytes */
        nliteral = 1;
        nin--;
        src++;

        for (;;) {
            if (nin <= 0) break;

            if ((nin >= 2) && (src[0] == src[1])) /* A two byte repeat ? */
                break;

            nliteral++;
            nin--;
            src++;
            if (nliteral == 128) break; /* Maximum literal run */
        }

        /* Could be added to last literal run ? */
        if ((last_literal != NULL) && (((*last_literal)+1)+nliteral <= 128)) {
            *last_literal += nliteral;
        } else {
            last_literal = dst;
            *(dst++) = (guchar)(nliteral-1);
        }
        while (nliteral-- > 0) *(dst++) = *(run_start++);
    }
    *nout = dst - start_dst;
}

void
PrintPDF::ascii85_init(void)
{
    ascii85_len = 0;
    ascii85_linewidth = 0;
}

void
PrintPDF::ascii85_flush(SVGOStringStream &os)
{
    char c[5];
    bool const zero_case = (ascii85_buf == 0);
    static int const max_linewidth = 75;

    for (int i = 4; i >= 0; i--) {
        c[i] = (ascii85_buf % 85) + '!';
        ascii85_buf /= 85;
    }
    /* check for special case: "!!!!!" becomes "z", but only if not
     * at end of data. */
    if (zero_case && (ascii85_len == 4)) {
        if (ascii85_linewidth >= max_linewidth) {
            os << '\n';
            ascii85_linewidth = 0;
        }
        os << 'z';
        ascii85_linewidth++;
    } else {
        for (int i = 0; i < ascii85_len+1; i++) {
            if ((ascii85_linewidth >= max_linewidth) && (c[i] != '%')) {
                os << '\n';
                ascii85_linewidth = 0;
            }
            os << c[i];
            ascii85_linewidth++;
        }
    }

    ascii85_len = 0;
    ascii85_buf = 0;
}

inline void
PrintPDF::ascii85_out(guchar byte, SVGOStringStream &os)
{
    if (ascii85_len == 4)
        ascii85_flush(os);

    ascii85_buf <<= 8;
    ascii85_buf |= byte;
    ascii85_len++;
}

void
PrintPDF::ascii85_nout(int n, guchar *uptr, SVGOStringStream &os)
{
    while (n-- > 0) {
        ascii85_out(*uptr, os);
        uptr++;
    }
}

void
PrintPDF::ascii85_done(SVGOStringStream &os)
{
    if (ascii85_len) {
        /* zero any unfilled buffer portion, then flush */
        ascii85_buf <<= (8 * (4-ascii85_len));
        ascii85_flush(os);
    }

    os << "~>\n";
}

unsigned int
PrintPDF::print_image(FILE *ofp, guchar *px, unsigned int width, unsigned int height, unsigned int rs,
                     NRMatrix const *transform)
{
    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed);

    return 0;
    
    os << "gsave\n";
    os << "[" << transform->c[0] << " "
       << transform->c[1] << " "
       << transform->c[2] << " "
       << transform->c[3] << " "
       << transform->c[4] << " "
       << transform->c[5] << "] concat\n";
    os << width << " " << height << " 8 ["
       << width << " 0 0 -" << height << " 0 " << height << "]\n";


    /* Write read image procedure */
    os << "% Strings to hold RGB-samples per scanline\n";
    os << "/rstr " << width << " string def\n";
    os << "/gstr " << width << " string def\n";
    os << "/bstr " << width << " string def\n";
    os << "{currentfile /ASCII85Decode filter /RunLengthDecode filter rstr readstring pop}\n";
    os << "{currentfile /ASCII85Decode filter /RunLengthDecode filter gstr readstring pop}\n";
    os << "{currentfile /ASCII85Decode filter /RunLengthDecode filter bstr readstring pop}\n";
    os << "true 3\n";

    /* Allocate buffer for packbits data. Worst case: Less than 1% increase */
    guchar *const packb = (guchar *)g_malloc((width * 105)/100+2);
    guchar *const plane = (guchar *)g_malloc(width);

    /* ps_begin_data(ofp); */
    os << "colorimage\n";

/*#define GET_RGB_TILE(begin)                   \
 *  {int scan_lines;                                                    \
 *    scan_lines = (i+tile_height-1 < height) ? tile_height : (height-i); \
 *    gimp_pixel_rgn_get_rect(&pixel_rgn, begin, 0, i, width, scan_lines); \
 *    src = begin; }
 */

    for (unsigned i = 0; i < height; i++) {
        /* if ((i % tile_height) == 0) GET_RGB_TILE(data); */ /* Get more data */
        guchar const *const src = px + i * rs;

        /* Iterate over RGB */
        for (int rgb = 0; rgb < 3; rgb++) {
            guchar const *src_ptr = src + rgb;
            guchar *plane_ptr = plane;
            for (unsigned j = 0; j < width; j++) {
                *(plane_ptr++) = *src_ptr;
                src_ptr += 4;
            }

            int nout;
            compress_packbits(width, plane, &nout, packb);

            ascii85_init();
            ascii85_nout(nout, packb, os);
            ascii85_out(128, os); /* Write EOD of RunLengthDecode filter */
            ascii85_done(os);
        }
    }
    /* ps_end_data(ofp); */

#if 0
    fprintf(ofp, "showpage\n");
    g_free(data);
#endif

    g_free(packb);
    g_free(plane);

#if 0
    if (ferror(ofp)) {
        g_message(_("write error occurred"));
        return (FALSE);
    }
#endif

    os << "grestore\n";

    fprintf(ofp, "%s", os.str().c_str());

    return 0;
//#undef GET_RGB_TILE
}

bool
PrintPDF::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

#include "clear-n_.h"

void
PrintPDF::init(void)
{
    /* SVG in */
    (void) Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
        "<name>" N_("PDF Print") "</name>\n"
        "<id>" SP_MODULE_KEY_PRINT_PDF "</id>\n"
        "<param name=\"bitmap\" type=\"boolean\">FALSE</param>\n"
        "<param name=\"resolution\" type=\"string\">72</param>\n"
        "<param name=\"destination\" type=\"string\">| lp</param>\n"
        "<param name=\"pageBoundingBox\" type=\"boolean\">TRUE</param>\n"
        "<param name=\"textToPath\" type=\"boolean\">TRUE</param>\n"
        "<print/>\n"
        "</inkscape-extension>", new PrintPDF());
}


}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

/* End of GNU GPL code */


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

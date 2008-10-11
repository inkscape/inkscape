#define __SP_PS_C__

/** \file
 * PostScript printing.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Basic printing code, EXCEPT image and
 * ascii85 filter is in public domain
 *
 * Image printing and Ascii85 filter:
 *
 * Copyright (C) 2006 Johan Engelen
 * Copyright (C) 1997-98 Peter Kirchgessner
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <libnr/nr-matrix-fns.h>

#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
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

#include "ps.h"
#include "extension/system.h"
#include "extension/print.h"

#include "io/sys.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_XFREE86_H
#include <pango/pangoft2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <2geom/sbasis-to-bezier.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include "helper/geom-curves.h"

/*
using std::atof;
using std::ceil;
using std::fclose;
using std::ferror;
using std::fflush;
using std::fgetc;
using std::fprintf;  --> this line will result  'std::libintl_fprintf' has not been declared
using std::fputc;
using std::fseek;
using std::ifstream;
using std::ios;
using std::memset;
using std::strchr;
using std::strcmp;
using std::strerror;
using std::tmpfile;
*/
using namespace std;

namespace Inkscape {
namespace Extension {
namespace Internal {

PrintPS::PrintPS() :
    _stream(NULL),
    _dpi(72),
    _bitmap(false)
{
    //map font types
    _fontTypesMap["Type 1"] = FONT_TYPE1;
    _fontTypesMap["TrueType"] = FONT_TRUETYPE;
    //TODO: support other font types (cf. embed_font())
}

PrintPS::~PrintPS(void)
{
    /* fixme: should really use pclose for popen'd streams */
    if (_stream) fclose(_stream);
    if(_begin_stream) fclose(_begin_stream);
    if(_fonts) g_tree_destroy(_fonts);

    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif

    return;
}

unsigned int
PrintPS::setup(Inkscape::Extension::Print * mod)
{
    static gchar const *const pdr[] = {"72", "75", "100", "144", "150", "200", "300", "360", "600", "1200", "2400", NULL};

#ifdef TED
    Inkscape::XML::Node *repr = ((SPModule *) mod)->repr;
#endif

    unsigned int ret = FALSE;
    gchar const *destination = mod->get_param_string("destination");

    /* Create dialog */
    GtkTooltips *tt = gtk_tooltips_new();
    g_object_ref((GObject *) tt);
    gtk_object_sink((GtkObject *) tt);

    GtkWidget *dlg = gtk_dialog_new_with_buttons(
            destination ?  _("Print Configuration") : _("Print Destination"),
//            SP_DT_WIDGET(SP_ACTIVE_DESKTOP)->window,
            NULL,
            (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT),
            GTK_STOCK_CANCEL,
            GTK_RESPONSE_CANCEL,
            destination ? GTK_STOCK_GO_FORWARD : GTK_STOCK_PRINT,
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
    GtkWidget *rb = gtk_radio_button_new_with_label(NULL, _("Print using PostScript operators"));
    gtk_tooltips_set_tip((GtkTooltips *) tt, rb,
                         _("Use PostScript vector operators. The resulting image is usually smaller "
                           "in file size and can be arbitrarily scaled, but alpha transparency "
                           "and patterns will be lost."), NULL);
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

    GtkWidget *e = NULL;
    /* if the destination isn't already set, we must prompt for it */
    if (!destination) {
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

        e = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(e), ( destination != NULL
                                           ? destination
                                           : "" ));
        gtk_box_pack_start(GTK_BOX(vb), e, FALSE, FALSE, 0);

        // pressing enter in the destination field is the same as clicking Print:
        gtk_entry_set_activates_default(GTK_ENTRY(e), TRUE);
    }

    gtk_widget_show_all(vbox);

    int const response = gtk_dialog_run(GTK_DIALOG(dlg));

    g_object_unref((GObject *) tt);

    if (response == GTK_RESPONSE_OK) {
        gchar const *fn;
        char const *sstr;

        _bitmap = gtk_toggle_button_get_active((GtkToggleButton *) rb);
        sstr = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
        _dpi = (unsigned int) MAX((int)(atof(sstr)), 1);

        /* if the destination was prompted for, record the new value */
        if (e) {
            /* Arrgh, have to do something */
            fn = gtk_entry_get_text(GTK_ENTRY(e));
            /* skip leading whitespace, bug #1068483 */
            while (fn && *fn==' ') { fn++; }
            /* g_print("Printing to %s\n", fn); */

            mod->set_param_string("destination", (gchar *)fn);
        }
        mod->set_param_bool("bitmap", _bitmap);
        mod->set_param_string("resolution", (gchar *)sstr);
        ret = TRUE;
    }

    gtk_widget_destroy(dlg);

    return ret;
}

unsigned int
PrintPS::begin(Inkscape::Extension::Print *mod, SPDocument *doc)
{
    gboolean epsexport = false;


    _latin1_encoded_fonts.clear();
    _newlatin1font_proc_defined = false;

    FILE *osf = NULL;
    FILE *osp = NULL;
    FILE *osf_tmp = NULL;

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError *error = NULL;
    //check whether fonts have to be embedded in the (EPS only) output
    bool font_embedded = mod->fontEmbedded();
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
            while (g_ascii_isspace(*fn)) fn += 1;
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
            _stream = _begin_stream = osp;
        } else if (*fn == '>') {
            fn += 1;
            epsexport = g_str_has_suffix(fn,".eps");
            while (isspace(*fn)) fn += 1;
            Inkscape::IO::dump_fopen_call(fn, "K");
            osf = Inkscape::IO::fopen_utf8name(fn, "w+");
            if (!osf) {
                fprintf(stderr, "inkscape: fopen(%s): %s\n",
                        fn, strerror(errno));
                return 0;
            }
            _begin_stream = osf;
             /* if font embedding is requested for EPS export...
             * TODO:could be extended to PS export if texttopath=FALSE possible
             */
             if(font_embedded && epsexport)
             {
               /**
               * Create temporary file where to print the main "script" part of the EPS document.
               * Use an extra stream (_begin_stream) to print the prolog and document setup sections.
               * Thus, once all the (main) script part printed, all the fonts used are known and can be embedded
               * just after the prolog section, in a Begin(End)Setup section (document setup),
               * one Begin(End)Resource (DSC comment) section for each font embedded.
               * Then, append the final script part from the temporary file (_stream in this case).
               * Reference: Adobe Technical note 5001, "PostScript Document Struturing Conventions Specifications"
               * page 19
               */
               osf_tmp = tmpfile();
               if(!osf_tmp)
               {
                 g_warning("Could not create a temporary file for font embedding. Font embedding canceled.");
                 mod->set_param_bool("fontEmbedded", false);
                 font_embedded = false;
                 _stream = osf;
               } else _stream = osf_tmp;
             } else _stream = osf;
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
            _stream = _begin_stream = osp;
        }
    }

    g_free(local_fn);

    if (_stream) {
        /* fixme: this is kinda icky */
#if !defined(_WIN32) && !defined(__WIN32__)
        (void) signal(SIGPIPE, SIG_IGN);
#endif
    }

    int const res = fprintf(_begin_stream, ( epsexport
                                       ? "%%!PS-Adobe-3.0 EPSF-3.0\n"
                                       : "%%!PS-Adobe-3.0\n" ));
    /* flush this to test output stream as early as possible */
    if (fflush(_begin_stream)) {
        /*g_print("caught error in sp_module_print_plain_begin\n");*/
        if (ferror(_begin_stream)) {
            g_print("Error %d on output stream: %s\n", errno,
                    g_strerror(errno));
        }
        g_print("Printing failed\n");
        /* fixme: should use pclose() for pipes */
        fclose(_begin_stream);
        _begin_stream = NULL;
        fflush(stdout);
        return 0;
    }
    //TODO: do this same test on _stream

    // width and height in pt
    _width = sp_document_width(doc) * PT_PER_PX;
    _height = sp_document_height(doc) * PT_PER_PX;

    NRRect d;
    bool   pageBoundingBox;
    bool   pageLandscape;
    pageBoundingBox = mod->get_param_bool("pageBoundingBox");
    // printf("Page Bounding Box: %s\n", pageBoundingBox ? "TRUE" : "FALSE");
    if (pageBoundingBox) {
        d.x0 = d.y0 = 0;
        d.x1 = ceil(_width);
        d.y1 = ceil(_height);
    } else {
        SPItem* doc_item = SP_ITEM(sp_document_root(doc));
        sp_item_invoke_bbox(doc_item, &d, sp_item_i2r_affine(doc_item), TRUE);
        // convert from px to pt
        d.x0 *= PT_PER_PX;
        d.x1 *= PT_PER_PX;
        d.y0 *= PT_PER_PX;
        d.y1 *= PT_PER_PX;
    }

    Inkscape::SVGOStringStream os;
    if (res >= 0) {

        os << "%%Creator: " << PACKAGE_STRING << "\n";
        // This will become problematic if inkscape gains the
        // ability to handle multi paged documents. If this is
        // the case the %%Orientation: comments should be
        // renamed to %%PageOrientation: and moved to the
        // respective pages.
        os << "%%Pages: 1\n";

        // 2004 Dec 10, BFC:
        // The point of the following code is (1) to do the thing that's expected by users
        // who have done File>New>A4_landscape or ...letter_landscape (i.e., rotate
        // the output), while (2) not messing up users who simply want their output wider
        // than it is tall (e.g., small figures for inclusion in LaTeX).
        // The original patch by WQ only had the w>h condition.
        {
             double w = (d.x1 - d.x0); // width and height of bounding box, in pt
             double h = (d.y1 - d.y0);
             pageLandscape = (
                 (w > 0. && h > 0.) // empty documents fail this sanity check, have w<0, h<0
                 && (w > h)   // implies, but does not prove, the user wanted landscape
                 && (w > 600) // approximate maximum printable width of an A4
                 && (!epsexport) // eps should always be portrait
             )
             ? true : false;
        }

        if (pageLandscape) {
            os << "%%Orientation: Landscape\n";
            os << "%%BoundingBox: " << (int) (_height - d.y1) << " "
               << (int) d.x0 << " "
               << (int) ceil(_height - d.y0) << " "
               << (int) ceil(d.x1) << "\n";
            // According to Mike Sweet (the author of CUPS)
            // HiResBoundingBox is only appropriate
            // for EPS files. This means that we should
            // distinguish if we export to ps or eps here.
            // FIXME: I couldn't find HiResBoundingBox in the PS
            // reference manual, so I guess we should skip
            // it.
            os << "%%HiResBoundingBox: " << (_height - d.y1) << " "
               << d.x0 << " "
               << (_height - d.y0) << " "
               << d.x1 << "\n";
            if (!epsexport) {
                os << "%%DocumentMedia: plain "
                   << (int) ceil(_height) << " "
                   << (int) ceil(_width) << " "
                   << "0 () ()\n";
            }
        } else {
            os << "%%Orientation: Portrait\n";
            os << "%%BoundingBox: " << (int) d.x0 << " "
               << (int) d.y0 << " "
               << (int) ceil(d.x1) << " "
               << (int) ceil(d.y1) << "\n";
            os << "%%HiResBoundingBox: " << d.x0 << " "
               << d.y0 << " "
               << d.x1 << " "
               << d.y1 << "\n";
            if (!epsexport) {
				os << "%%DocumentMedia: plain "
				   << (int) ceil(_width) << " "
                  << (int) ceil(_height) << " "
                  << "0 () ()\n";
			}
        }

        os << "%%EndComments\n";
         /* If font embedding requested, begin document setup section where to include font resources */
         if(font_embedded) os << "%%BeginSetup\n";/* Resume it later with Begin(End)Resource sections for font embedding. So, for now, we are done with the prolog/setup part. */
         gint ret = fprintf(_begin_stream, "%s", os.str().c_str());
         if(ret < 0) return ret;

         /* Main Script part (after document setup) begins */
         /* Empty os from all previous printing */
         std::string clrstr = "";
         os.str(clrstr);
        // This will become problematic if we print multi paged documents:
        os << "%%Page: 1 1\n";

        if (pageLandscape) {
            os << "90 rotate\n";
            if (_bitmap) {
                os << "0 " << (int) -ceil(_height) << " translate\n";
            }
        } else {
            if (!_bitmap) {
                os << "0 " << (int) ceil(_height) << " translate\n";
            }
        }

        if (!_bitmap) {
            os << PT_PER_PX << " " << -PT_PER_PX << " scale\n";
            // from now on we can output px, but they will be treated as pt
        }

 	/* As a new PS document is created, _fontlist has to be reinitialized (unref fonts from possible former PS docs) */
        _fonts = g_tree_new_full((GCompareDataFunc)strcmp, NULL, (GDestroyNotify)g_free, (GDestroyNotify)g_free);
    }

    os << "0 0 0 setrgbcolor\n"
       << "[] 0 setdash\n"
       << "1 setlinewidth\n"
       << "0 setlinejoin\n"
       << "0 setlinecap\n";

    /* FIXME: This function is declared to return unsigned, whereas fprintf returns a signed int *
     * that can be zero if the first fprintf failed (os is empty) or "negative" (i.e. very positive
     * in unsigned int interpretation) if the first fprintf failed but this one succeeds, or
     * positive if both succeed. */
    return fprintf(_stream, "%s", os.str().c_str());
}

unsigned int
PrintPS::finish(Inkscape::Extension::Print *mod)
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

        Geom::Matrix affine;
        affine[0] = width / ((x1 - x0) * PX_PER_PT);
        affine[1] = 0.0;
        affine[2] = 0.0;
        affine[3] = height / ((y1 - y0) * PX_PER_PT);
        affine[4] = -affine[0] * x0;
        affine[5] = -affine[3] * y0;

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
            gc.transform.setIdentity();
            nr_arena_item_invoke_update(mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);
            /* Render */
            /* This should take guchar* instead of unsigned char*) */
            NRPixBlock pb;
            nr_pixblock_setup_extern(&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                                     bbox.x0, bbox.y0, bbox.x1, bbox.y1,
                                     (guchar*)px, 4 * width, FALSE, FALSE);
            memset(px, 0xff, 4 * width * 64);
            nr_arena_item_invoke_render(NULL, mod->root, &bbox, &pb, 0);
            /* Blitter goes here */
            Geom::Matrix imgt;
            imgt[0] = (bbox.x1 - bbox.x0) / dots_per_pt;
            imgt[1] = 0.0;
            imgt[2] = 0.0;
            imgt[3] = (bbox.y1 - bbox.y0) / dots_per_pt;
            imgt[4] = 0.0;
            imgt[5] = _height - y / dots_per_pt - (bbox.y1 - bbox.y0) / dots_per_pt;

            print_image(_stream, px, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0, 4 * width, &imgt);
        }

        g_free(px);
    }

    fprintf(_stream, "showpage\n");
    int const res = fprintf(_stream, "%%%%EOF\n");

    /* Flush stream to be sure. */
    (void) fflush(_stream);

    char c;
    /* If font embedding... */
    if(mod->get_param_bool("fontEmbedded"))
    {
        /* Close the document setup section that had been started (because all the needed resources are supposed to be included now) */
       /*res = */fprintf(_begin_stream, "%s", "%%EndSetup\n");
       /* If font embedding requested, the following PS script part was printed to a different file from the prolog/setup, so script part (current _stream) needs to be copied to prolog/setup file to get the complete (E)PS document */
       if(fseek(_stream, 0, SEEK_SET) == 0)
       {
           while((c = fgetc(_stream))!=EOF) fputc(c, _begin_stream);
       }
       fclose(_stream);
       _stream = _begin_stream;
    }

    /* fixme: should really use pclose for popen'd streams */
    fclose(_stream);
    _stream = NULL;

    _latin1_encoded_fonts.clear();

    g_tree_destroy(_fonts);

    return res;
}

unsigned int
PrintPS::bind(Inkscape::Extension::Print */*mod*/, Geom::Matrix const *transform, float /*opacity*/)
{
    if (!_stream) return 0;  // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    Inkscape::SVGOStringStream os;
    os << "gsave [" << (*transform)[0] << " "
       << (*transform)[1] << " "
       << (*transform)[2] << " "
       << (*transform)[3] << " "
       << (*transform)[4] << " "
       << (*transform)[5] << "] concat\n";

    return fprintf(_stream, "%s", os.str().c_str());
}

unsigned int
PrintPS::release(Inkscape::Extension::Print */*mod*/)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    return fprintf(_stream, "grestore\n");
}

unsigned int
PrintPS::comment(Inkscape::Extension::Print */*mod*/, char const *comment)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    return fprintf(_stream, "%%! %s\n",comment);
}

void
PrintPS::print_fill_style(SVGOStringStream &os, SPStyle const *const style, NRRect const *pbox)
{
    g_return_if_fail( style->fill.isColor()
                      || ( style->fill.isPaintserver()
                           && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) );

    if (style->fill.isColor()) {
        float rgb[3];
        sp_color_get_rgb_floatv(&style->fill.value.color, rgb);

        os << rgb[0] << " " << rgb[1] << " " << rgb[2] << " setrgbcolor\n";

    } else {
        g_assert( style->fill.isPaintserver()
                  && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );

        if (SP_IS_LINEARGRADIENT(SP_STYLE_FILL_SERVER(style))) {

            SPLinearGradient *lg = SP_LINEARGRADIENT(SP_STYLE_FILL_SERVER(style));
            Geom::Point p1 (lg->x1.computed, lg->y1.computed);
            Geom::Point p2 (lg->x2.computed, lg->y2.computed);
            if (pbox && SP_GRADIENT(lg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                Geom::Matrix bbox2user(pbox->x1 - pbox->x0, 0, 0, pbox->y1 - pbox->y0, pbox->x0, pbox->y0);
                p1 *= bbox2user;
                p2 *= bbox2user;
            }

            os << "<<\n/ShadingType 2\n/ColorSpace /DeviceRGB\n";
            os << "/Coords [" << p1[Geom::X] << " " << p1[Geom::Y] << " " << p2[Geom::X] << " " << p2[Geom::Y] <<"]\n";
            os << "/Extend [true true]\n";
            os << "/Domain [0 1]\n";
            os << "/Function <<\n/FunctionType 3\n/Functions\n[\n";

            sp_gradient_ensure_vector(SP_GRADIENT(lg)); // when exporting from commandline, vector is not built
            for (unsigned i = 0; i + 1 < lg->vector.stops.size(); i++) {
                float rgb[3];
                sp_color_get_rgb_floatv(&lg->vector.stops[i].color, rgb);
                os << "<<\n/FunctionType 2\n/Domain [0 1]\n";
                os << "/C0 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                sp_color_get_rgb_floatv(&lg->vector.stops[i+1].color, rgb);
                os << "/C1 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                os << "/N 1\n>>\n";
            }
            os << "]\n/Domain [0 1]\n";
            os << "/Bounds [ ";
            for (unsigned i = 0; i + 2 < lg->vector.stops.size(); i++) {
                os << lg->vector.stops[i+1].offset <<" ";
            }
            os << "]\n";
            os << "/Encode [ ";
            for (unsigned i = 0; i + 1 < lg->vector.stops.size(); i++) {
                os << "0 1 ";
            }
            os << "]\n";
            os << ">>\n>>\n";

        } else if (SP_IS_RADIALGRADIENT(SP_STYLE_FILL_SERVER(style))) {

            SPRadialGradient *rg = SP_RADIALGRADIENT(SP_STYLE_FILL_SERVER(style));
            Geom::Point c(rg->cx.computed, rg->cy.computed);
            Geom::Point f(rg->fx.computed, rg->fy.computed);
            double r = rg->r.computed;
            if (pbox && SP_GRADIENT(rg)->units == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
                // convert to userspace
                Geom::Matrix const bbox2user(pbox->x1 - pbox->x0, 0,
                                           0, pbox->y1 - pbox->y0,
                                           pbox->x0, pbox->y0);
                c *= bbox2user;
                f *= bbox2user;
                r *= bbox2user.descrim();
            }

            os << "<<\n/ShadingType 3\n/ColorSpace /DeviceRGB\n";
            os << "/Coords ["<< f[Geom::X] <<" "<< f[Geom::Y] <<" 0 "<< c[Geom::X] <<" "<< c[Geom::Y] <<" "<< r <<"]\n";
            os << "/Extend [true true]\n";
            os << "/Domain [0 1]\n";
            os << "/Function <<\n/FunctionType 3\n/Functions\n[\n";

            sp_gradient_ensure_vector(SP_GRADIENT(rg)); // when exporting from commandline, vector is not built
            for (unsigned i = 0; i + 1 < rg->vector.stops.size(); i++) {
                float rgb[3];
                sp_color_get_rgb_floatv(&rg->vector.stops[i].color, rgb);
                os << "<<\n/FunctionType 2\n/Domain [0 1]\n";
                os << "/C0 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                sp_color_get_rgb_floatv(&rg->vector.stops[i+1].color, rgb);
                os << "/C1 [" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "]\n";
                os << "/N 1\n>>\n";
            }
            os << "]\n/Domain [0 1]\n";
            os << "/Bounds [ ";
            for (unsigned i = 0; i + 2 < rg->vector.stops.size(); i++) {
                os << rg->vector.stops[i+1].offset << " ";
            }
            os << "]\n";
            os << "/Encode [ ";
            for (unsigned i = 0; i + 1 < rg->vector.stops.size(); i++) {
                os << "0 1 ";
            }
            os << "]\n";
            os << ">>\n>>\n";
        }
    }
}

void
PrintPS::print_stroke_style(SVGOStringStream &os, SPStyle const *style)
{
    float rgb[3];
    sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);

    os << rgb[0] << " " << rgb[1] << " " << rgb[2] << " setrgbcolor\n";

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
            os << "] " << style->stroke_dash.offset << " setdash\n";
        } else {
            os << "[] 0 setdash\n";
        }
    } else {
        os << "[] 0 setdash\n";
    }

    os << style->stroke_width.computed << " setlinewidth\n";
    os << style->stroke_linejoin.computed << " setlinejoin\n";
    os << style->stroke_linecap.computed << " setlinecap\n";
}


unsigned int
PrintPS::fill(Inkscape::Extension::Print *mod, Geom::PathVector const &pathv, Geom::Matrix const *ctm, SPStyle const *const style,
              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if ( style->fill.isColor()
         || ( style->fill.isPaintserver()
              && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) )
    {
        Inkscape::SVGOStringStream os;

        os << "gsave\n";

        print_fill_style(os, style, pbox);

        print_pathvector(os, pathv);

        if (style->fill_rule.computed == SP_WIND_RULE_EVENODD) {
            if (style->fill.isColor()) {
                os << "eofill\n";
            } else {
                g_assert( style->fill.isPaintserver()
                          && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );
                SPGradient const *g = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
                os << "eoclip\n";
                if (g->gradientTransform_set) {
                    os << "gsave [" << g->gradientTransform[0] << " " << g->gradientTransform[1]
                        << " " << g->gradientTransform[2] << " " << g->gradientTransform[3]
                        << " " << g->gradientTransform[4] << " " << g->gradientTransform[5] << "] concat\n";
                }
                os << "shfill\n";
                if (g->gradientTransform_set) {
                    os << "grestore\n";
                }
            }
        } else {
            if (style->fill.isColor()) {
                os << "fill\n";
            } else {
                g_assert( style->fill.isPaintserver()
                          && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) );
                SPGradient const *g = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
                os << "clip\n";
                if (g->gradientTransform_set) {
                    os << "gsave [" << g->gradientTransform[0] << " " << g->gradientTransform[1]
                        << " " << g->gradientTransform[2] << " " << g->gradientTransform[3]
                        << " " << g->gradientTransform[4] << " " << g->gradientTransform[5] << "] concat\n";
                }
                os << "shfill\n";
                if (g->gradientTransform_set) {
                    os << "grestore\n";
                }
            }
        }

        os << "grestore\n";

        fprintf(_stream, "%s", os.str().c_str());
    }

    return 0;
}


unsigned int
PrintPS::stroke(Inkscape::Extension::Print *mod, Geom::PathVector const &pathv, Geom::Matrix const *ctm, SPStyle const *style,
                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    if (style->stroke.isColor()) {
        Inkscape::SVGOStringStream os;

        print_stroke_style(os, style);

        print_pathvector(os, pathv);

        os << "stroke\n";

        fprintf(_stream, "%s", os.str().c_str());
    }

    return 0;
}

unsigned int
PrintPS::image(Inkscape::Extension::Print *mod, guchar *px, unsigned int w, unsigned int h, unsigned int rs,
               Geom::Matrix const *transform, SPStyle const *style)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    return print_image(_stream, px, w, h, rs, transform);
}

/* PSFontName is now useless (cf. text() method code) */
char const *
PrintPS::PSFontName(SPStyle const *style)
{
    font_instance *tf = font_factory::Default()->FaceFromStyle(style);

    char const *n;
    char name_buf[256];

    // PS does not like spaces in fontnames, replace them with the usual dashes.

    if (tf) {
        tf->PSName(name_buf, sizeof(name_buf));
        n = g_strdelimit(name_buf, " ", '-');
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

//LSB = Least Significant Byte
//converts 4-byte array to "LSB first" to "LSB last"
/**
* (Used by PrintPS::embed_t1 (from libgnomeprint/gnome-font-face.c),
* to get the length of data segment (bytes 3-6 in IBM PC (PostScript) font file format.
* Reference: Adobe technical note 5040, "Supporting Downloadable PostScript
* Language Fonts", page 9)
*/

#define INT32_LSB_2_5(q) ((q)[2] + ((q)[3] << 8) + ((q)[4] << 16) + ((q)[5] << 24))

/**
* \brief For "Type 1" font only, print font data in output stream, to embed font data in PS output.
* \param os Stream of output.
* \param font Font whose data to embed.
* \return FALSE if font embedding canceled (due to error or not supported font type), TRUE otherwise
* TODO: enable font embedding for True Type
*/
//adapted more/less from libgnomeprint/gnome_font_face_ps_embed_t1()
bool
PrintPS::embed_t1 (SVGOStringStream &os, font_instance* font)
{
        //check font type
        FT_Face font_face = pango_ft2_font_get_face(font->pFont);
        const FT_String* font_type = FT_Get_X11_Font_Format(font_face);
        g_return_val_if_fail (_fontTypesMap[font_type] == FONT_TYPE1, false);
        //get font filename, stream to font file and size
        FT_Stream font_stream = font_face->stream;
        const char* font_filename = (char*) font_stream->pathname.pointer;
        unsigned long font_stream_size = font_stream->size;
        //first detect if font file is in IBM PC format
        /**
        * if first byte is 0x80, font file is pfb, do the same as a pfb to pfa converter
        * Reference: Adobe technical note 5040, "Supporting Downloadable PostScript
        * Language Fonts", page 9
	* else: include all the ASCII data in the font pfa file
        **/
        char* buf = new char[7];
        unsigned char* buffer = new unsigned char[7];//for the 6 header bytes (data segment length is unknown at this point) and final '\0'
        std::string ascii_data;//for data segment "type 1" in IBM PC Format
        //read the 6 header bytes
        //for debug: g_warning("Reading from font file %s...", font_filename);
        font_stream->close(font_stream);
        ifstream font_file (font_filename, ios::in|ios::binary);
        if (!font_file.is_open()) {
                g_warning ("file %s: line %d: Cannot open font file %s", __FILE__, __LINE__, font_filename);
                return false;
        }
        font_file.read(buf, 6);
        buffer = (unsigned char*) buf;

	//If font file is pfb, do the same as pfb to pfa converter
        //check byte 1
        if (buffer[0] == 0x80) {
                const char hextab[17] = "0123456789abcdef";
                unsigned long offset = 0;

                while (offset < font_stream_size) {
                        gint length, i;
                        if (buffer[0] != 0x80) {
                                g_warning ("file %s: line %d: Corrupt %s", __FILE__, __LINE__, font_filename);
                                //TODO: print some default font data anyway like libgnomeprint/gnome_font_face_ps_embed_empty
                                return false;
                        }
                        switch (buffer[1]) {
                        case 1:
                                //get data segment length from bytes 3-6
                                //(Note: byte 1 is first byte in comments to match Adobe technical note 5040, but index 0 in code)
                                length = INT32_LSB_2_5 (buffer);
                                offset += 6;
                                //resize the buffer to fit the data segment length
                                delete [] buf;
                                buf = new char[length + 1];
                                buffer = new unsigned char[length + 1];
                                //read and print all the data segment length
                                font_file.read(buf, length);
        			buffer = (unsigned char*) buf;
                                /**
                                * Assigning a part from the buffer of length "length" ensures
                                * that no incorrect extra character will be printed and make the PS output invalid
                                * That was the case with the code:
                                * os << buffer;
                                * (A substring method could have been used as well.)
                                */
                                ascii_data.assign(buf, 0, length);
                                os << ascii_data;
                                offset += length;
                                //read next 6 header bytes
                                font_file.read(buf, 6);
                                break;
                        case 2:
                                length = INT32_LSB_2_5 (buffer);
                                offset += 6;
                                //resize the buffer to fit the data segment length
                                delete [] buf;
                                buf = new char[length + 1];
                                buffer = new unsigned char[length + 1];
                                //read and print all the data segment length
                                font_file.read(buf, length);
                                buffer = (unsigned char*) buf;
                                for (i = 0; i < length; i++) {
                                        os << hextab[buffer[i] >> 4];
                                        os << hextab[buffer[i] & 15];
                                        offset += 1;
                                        if ((i & 31) == 31 || i == length - 1)
                                                os << "\n";
                                }
                                //read next 6 header bytes
                                font_file.read(buf, 6);
                                break;
                        case 3:
                                /* Finished */
                                os << "\n";
                                offset = font_stream_size;
                                break;
                        default:
                                os << "%%%ERROR: Font file corrupted at byte " << offset << "\n";
                                //TODO: print some default font data anyway like libgnomeprint/gnome_font_face_ps_embed_empty
                                return false;
                        }
                }
        }
	//else: font file is pfa, include all directly
	else {
                //font is not in IBM PC format, all the file content can be directly printed
                //resize buffer
                delete [] buf;
                buf = new char[font_stream_size + 1];
                delete [] buffer;
                font_file.seekg (0, ios::beg);
                font_file.read(buf, font_stream_size);
		/**
		 * Assigning a part from the buffer of length "length" ensures
                 * that no incorrect extra character will be printed and make the PS output invalid
                 * That was the case with the code:
                 * os << buffer;
                 * (A substring method could have been used as well.)
                 */
		ascii_data.assign(buf, 0, font_stream_size);
		os << ascii_data;
        }
        font_file.close();
        delete [] buf;
        buf = NULL;
        buffer = NULL;// Clear buffer to prevent using invalid memory reference.

        char font_psname[256];
        font->PSName(font_psname, sizeof(font_psname));
        FT_Long font_num_glyphs = font_face->num_glyphs;
        if (font_num_glyphs < 256) {
                gint glyph;
                /* 8-bit vector */
                os << "(" << font_psname << ") cvn findfont dup length dict begin\n";
                os << "{1 index /FID ne {def} {pop pop} ifelse} forall\n";
                os << "/Encoding [\n";
                for (glyph = 0; glyph < 256; glyph++) {
                        guint g;
                        gchar c[256];
                        FT_Error status;
                        g = (glyph < font_num_glyphs) ? glyph : 0;
                        status = FT_Get_Glyph_Name (font_face, g, c, 256);

			if (status != FT_Err_Ok) {
                                g_warning ("file %s: line %d: Glyph %d has no name in %s", __FILE__, __LINE__, g, font_filename);
                                g_snprintf (c, 256, ".notdef");
                        }

                        os << "/" << c << ( ((glyph & 0xf) == 0xf)?"\n":" " );
                }
                os << "] def currentdict end\n";
                //TODO: manage several font instances for same ps name like in libgnomeprint/gnome_print_ps2_set_font_real()
                //gf_pso_sprintf (pso, "(%s) cvn exch definefont pop\n", pso->encodedname);
                os << "(" << font_psname << ") cvn exch definefont pop\n";
        } else {
                gint nfonts, i, j;
                /* 16-bit vector */
                nfonts = (font_num_glyphs + 255) >> 8;

                os << "32 dict begin\n";
                /* Common entries */
                os << "/FontType 0 def\n";
                os << "/FontMatrix [1 0 0 1 0 0] def\n";
                os << "/FontName (" << font_psname << "-Glyph-Composite) cvn def\n";
                os << "/LanguageLevel 2 def\n";

                /* Type 0 entries */
                os << "/FMapType 2 def\n";

                /* Bitch 'o' bitches */
                os << "/FDepVector [\n";

                for (i = 0; i < nfonts; i++) {
                        os << "(" << font_psname << ") cvn findfont dup length dict begin\n";
                        os << "{1 index /FID ne {def} {pop pop} ifelse} forall\n";
                        os << "/Encoding [\n";
                        for (j = 0; j < 256; j++) {
                                gint glyph;
                                gchar c[256];
                                FT_Error status;
                                glyph = 256 * i + j;
                                if (glyph >= font_num_glyphs)
                                        glyph = 0;
                                status = FT_Get_Glyph_Name (font_face, glyph, c, 256);
                                if (status != FT_Err_Ok) {
                                        g_warning ("file %s: line %d: Glyph %d has no name in %s", __FILE__, __LINE__, glyph, font_filename);
                                        g_snprintf (c, 256, ".notdef");
                                }
                                os << "/" << c << ( ((j & 0xf) == 0xf)?"\n":" " );
                        }
                        os << "] def\n";
                        os << "currentdict end (" << font_psname << "-Glyph-Page-";
                        os << std::dec << i;
                        os << ") cvn exch definefont\n";
                }
                os << "] def\n";
                os << "/Encoding [\n";
                for (i = 0; i < 256; i++) {
                        gint fn;
                        fn = (i < nfonts) ? i : 0;
                        os << std::dec << fn;
                        os << ( ((i & 0xf) == 0xf) ? "\n" : " " );
                }
                os << "] def\n";
                os << "currentdict end\n";
                //TODO: manage several font instances for same ps name like in libgnomeprint/gnome_print_ps2_set_font_real()
                //gf_pso_sprintf (pso, "(%s) cvn exch definefont pop\n", pso->encodedname);
                os << "(" << font_psname << ") cvn exch definefont pop\n";
        }
	//font embedding completed
	return true;
}



/**
* \brief Print font data in output stream, to embed font data in PS output.
* \param os Stream of output.
* \param font Font whose data to embed.
* \return FALSE if font embedding canceled (due to error or not supported font type), TRUE otherwise
*/
//adapted from libgnomeprint/gnome_font_face_ps_embed()
bool PrintPS::embed_font(SVGOStringStream &os, font_instance* font)
{
  //Hinted at by a comment in libgnomeprint/fcpattern_to_gp_font_entry()
  //Determining the font type in the "Pango way"
  FT_Face font_face = pango_ft2_font_get_face(font->pFont);
  const FT_String* font_type = FT_Get_X11_Font_Format(font_face);

  /**
  * Possible values of "font_type": Type 1, TrueType, etc.
  * Embedding available only for Type 1 fonts so far.
  */
  //TODO: provide support for other font types (TrueType is a priority)
  switch(_fontTypesMap[font_type])
  {
    case FONT_TYPE1:
      return embed_t1 (os, font);
    //TODO: implement TT font embedding
    /*case FONT_TRUETYPE:
      embed_tt (os, font);
      break;*/
    default:
      g_warning("Unknown (not supported) font type for embedding: %s", font_type);
      //TODO: embed something like in libgnomeprint/gnome_font_face_ps_embed_empty();
      return false;
  }
}


/**
* \brief Converts UTF-8 string to sequence of glyph numbers for PostScript string (cf. "show" commands.).
* \param os Stream of output.
* \param font Font used for unicode->glyph mapping.
* \param unistring UTF-8 encoded string to convert.
*/
void PrintPS::print_glyphlist(SVGOStringStream &os, font_instance* font, Glib::ustring unistring)
{
  //iterate through unicode chars in unistring
  Glib::ustring::iterator unistring_iter;
  gunichar unichar;
  gint glyph_index, glyph_page;
  
  FT_Face font_face = pango_ft2_font_get_face(font->pFont);
  FT_Long font_num_glyphs = font_face->num_glyphs;
  //whether font has more than one glyph pages (16-bit encoding)
  bool two_bytes_encoded = (font_num_glyphs > 255);

  for (unistring_iter = unistring.begin();   unistring_iter!=unistring.end();  unistring_iter++)
  {
    //get unicode char
    unichar = *unistring_iter;
    //get matching glyph index in current font for unicode char
    //default glyph index is 0 for undefined font character (glyph unavailable)
    //TODO: if glyph unavailable for current font, use a default font among the most Unicode-compliant - e.g. Bitstream Cyberbit - I guess
    glyph_index = font->MapUnicodeChar(unichar);
    //if more than one glyph pages for current font (16-bit encoding),
    if(two_bytes_encoded)
    {
      //add glyph page before glyph index.
      glyph_page = (glyph_index >> 8) & 0xff;
      os << "\\";
      //convert in octal code before printing
      os << std::oct << glyph_page;
    }
    //(If one page - 8-bit encoding -, nothing to add.)
    //TODO: explain the following line inspired from libgnomeprint/gnome_print_ps2_glyphlist()
    glyph_index = glyph_index & 0xff;
    //TODO: mark glyph as used for current font, if Inkscape has to embed minimal font data in PS
    os << "\\";
    //convert in octal code before printing
    os << std::oct << glyph_index;
  }
}

unsigned int
PrintPS::text(Inkscape::Extension::Print *mod, char const *text, Geom::Point p,
              SPStyle const *const style)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.
    if (_bitmap) return 0;

    //check whether fonts have to be embedded in the PS output
    //if not, use the former way of Inkscape to print text
    gboolean font_embedded = mod->fontEmbedded();

    Inkscape::SVGOStringStream os;
    //find font
    /**
    * A font_instance object is necessary for the next steps,
    * that's why using PSFontName() method just to get the PS fontname
    * is not enough and not appropriate
    */
    font_instance *tf = font_factory::Default()->FaceFromStyle(style);
    
    const gchar *fn = NULL;
    char name_buf[256];

    //check whether font was found
    /**
    * This check is not strictly reliable
    * since Inkscape returns a default font if font not found.
    * This is just to be consistent with the method PSFontName().
    */
    if (tf) {
        //get font PS name
        tf->PSName(name_buf, sizeof(name_buf));
        fn = name_buf;
    } else {
    	// this system does not have this font, so cancel font embedding...
        font_embedded = FALSE;
        //this case seems to never happen since Inkscape uses a default font instead (like BitstreamVeraSans on Windows)
        g_warning("Font %s not found.", fn);
        //...and just use the name from SVG in the hope that PS interpreter will make sense of it
        bool i = (style->font_style.value == SP_CSS_FONT_STYLE_ITALIC);
        bool o = (style->font_style.value == SP_CSS_FONT_STYLE_OBLIQUE);
        bool b = (style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLD) ||
            (style->font_weight.value >= SP_CSS_FONT_WEIGHT_500 && style->font_weight.value <= SP_CSS_FONT_WEIGHT_900);

        fn = g_strdup_printf("%s%s%s%s",
                            g_strdelimit(style->text->font_family.value, " ", '-'),
                            (b || i || o) ? "-" : "",
                            (b) ? "Bold" : "",
                            (i) ? "Italic" : ((o) ? "Oblique" : "") );
    }

    /**
    * If font embedding is requested, tempt to embed the font the first time it is used, once and for all.
    * There is no selection of the glyph descriptions to embed, based on the characters used effectively in the document.
    * (TODO?)
    * Else, back to the former way of printing.
    */
    gpointer  is_embedded;
    //if not first time the font is used and if font embedding requested, check whether the font has been embedded (successfully the first time).
    if(g_tree_lookup_extended(_fonts, fn, NULL, &is_embedded)) font_embedded = font_embedded && (strcmp((char *)is_embedded, "TRUE") == 0);
    else
    {
      //first time the font is used
      if(font_embedded)
      {
        //embed font in PS output
        //adapted from libgnomeprint/gnome_print_ps2_close()
        os << "%%BeginResource: font " << fn << "\n";
        font_embedded = embed_font(os, tf);
        os << "%%EndResource: font " << fn << "\n";
        if(!font_embedded) g_warning("Font embedding canceled for font: %s", fn);
        else fprintf(_begin_stream, "%s", os.str().c_str());
        //empty os before resume printing to the script stream
        std::string clrstr = "";
        os.str(clrstr);

      }
      //add to the list
      g_tree_insert(_fonts, g_strdup(fn), g_strdup((font_embedded)?"TRUE":"FALSE"));
    }
    
    Glib::ustring s;
    // Escape chars
    Inkscape::SVGOStringStream escaped_text;
    //if font embedding, all characters will be converted to glyph indices (cf. PrintPS::print_glyphlist()),
    //so no need to escape characters
    //else back to the old way, i.e. escape chars: '\',')','(' and UTF-8 ones
    if(font_embedded) s = text;
    else {
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
    }

    os << "gsave\n";

    // set font
    if(font_embedded) os << "/" << fn << " findfont\n";
    else {
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
		if(strchr(fn, ' ') == NULL)
			os << "/" << fn << "-ISOLatin1 /" << fn << " newlatin1font\n";
		else
			os << "(/" << fn << "-ISOLatin1) (/" << fn << ") newlatin1font\n";
		_latin1_encoded_fonts.insert(fn);
	} else
		if(strchr(fn, ' ') == NULL)
			os << "/" << fn << "-ISOLatin1 findfont\n";
		else
			os << "(/" << fn << "-ISOLatin1) findfont\n";
    }
    os << style->font_size.computed << " scalefont\n";
    os << "setfont\n";
   //The commented line beneath causes Inkscape to crash under Linux but not under Windows
    //g_free((void*) fn);

    if ( style->fill.isColor()
         || ( style->fill.isPaintserver()
              && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style)) ) )
    {
        // set fill style
        print_fill_style(os, style, NULL);
        // FIXME: we don't know the pbox of text, so have to pass NULL. This means gradients with
        // bbox units won't work with text. However userspace gradients don't work with text either
        // (text is black) for some reason.

        os << "newpath\n";
        os << p[Geom::X] << " " << p[Geom::Y] << " moveto\n";
        os << "(";
        if(font_embedded) print_glyphlist(os, tf, s);
        else os << escaped_text.str();
        os << ") show\n";
    }

    if (style->stroke.isColor()) {

        // set stroke style
        print_stroke_style(os, style);

        // paint stroke
        os << "newpath\n";
        os << p[Geom::X] << " " << p[Geom::Y] << " moveto\n";
        os << "(";
        if(font_embedded) print_glyphlist(os, tf, s);
        else os << escaped_text.str();
        os << ") false charpath stroke\n";
    }

    if(tf) tf->Unref();

    os << "grestore\n";

    fprintf(_stream, "%s", os.str().c_str());

    return 0;
}



/* PostScript helpers */

void
PrintPS::print_pathvector(SVGOStringStream &os, Geom::PathVector const &pathv)
{
    if (pathv.empty())
        return;

    os << "newpath\n";

    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {

        os << it->initialPoint()[Geom::X] << " " << it->initialPoint()[Geom::Y] << " moveto\n";

        for(Geom::Path::const_iterator cit = it->begin(); cit != it->end_open(); ++cit) {
            print_2geomcurve(os, *cit);
        }

        if (it->closed()) {
            os << "closepath\n";
        }

    }
}

void
PrintPS::print_2geomcurve(SVGOStringStream &os, Geom::Curve const & c )
{
    using Geom::X;
    using Geom::Y;

    if( is_straight_curve(c) )
    {
        os << c.finalPoint()[X] << " " << c.finalPoint()[Y] << " lineto\n";
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const*>(&c)) {
        std::vector<Geom::Point> points = cubic_bezier->points();
        os << points[1][X] << " " << points[1][Y] << " "
           << points[2][X] << " " << points[2][Y] << " "
           << points[3][X] << " " << points[3][Y] << " curveto\n";
    }
    else {
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            print_2geomcurve(os, *iter);
        }
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
PrintPS::compress_packbits(int nin,
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
PrintPS::ascii85_init(void)
{
    ascii85_len = 0;
    ascii85_linewidth = 0;
}

void
PrintPS::ascii85_flush(SVGOStringStream &os)
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
PrintPS::ascii85_out(guchar byte, SVGOStringStream &os)
{
    if (ascii85_len == 4)
        ascii85_flush(os);

    ascii85_buf <<= 8;
    ascii85_buf |= byte;
    ascii85_len++;
}

void
PrintPS::ascii85_nout(int n, guchar *uptr, SVGOStringStream &os)
{
    while (n-- > 0) {
        ascii85_out(*uptr, os);
        uptr++;
    }
}

void
PrintPS::ascii85_done(SVGOStringStream &os)
{
    if (ascii85_len) {
        /* zero any unfilled buffer portion, then flush */
        ascii85_buf <<= (8 * (4-ascii85_len));
        ascii85_flush(os);
    }

    os << "~>\n";
}

unsigned int
PrintPS::print_image(FILE *ofp, guchar *px, unsigned int width, unsigned int height, unsigned int rs,
                     Geom::Matrix const *transform)
{
    Inkscape::SVGOStringStream os;

    os << "gsave\n";

    os << "[" << (*transform)[0] << " "
       << (*transform)[1] << " "
       << (*transform)[2] << " "
       << (*transform)[3] << " "
       << (*transform)[4] << " "
       << (*transform)[5] << "] concat\n";

    /* Write read image procedure */
    os << "<<\n";
    os << "  /ImageType 3\n";
    os << "  /InterleaveType 1\n";

    os << "  /MaskDict\n";
    os << "  <<\n";
    os << "    /ImageType 1\n";
    os << "    /Width " << width << "\n";
    os << "    /Height " << height << "\n";
    os << "    /ImageMatrix "
       << "[" << width << " "
       << 0 << " "
       << 0 << " "
       << -((long) height) << " "
       << 0 << " "
       << height << "]\n";
    os << "    /BitsPerComponent 8\n";
    os << "    /Decode [1 0]\n";
    os << "  >>\n";

    os << "  /DataDict\n";
    os << "  <<\n";
    os << "    /ImageType 1\n";
    os << "    /Width " << width << "\n";
    os << "    /Height " << height << "\n";
    os << "    /ImageMatrix "
       << "[" << width << " "
       << 0 << " "
       << 0 << " "
       << -((long )height) << " "
       << 0 << " "
       << height << "]\n";
    os << "    /DataSource currentfile /ASCII85Decode filter\n";
    os << "    /BitsPerComponent 8\n";
    os << "    /Decode [0 1 0 1 0 1]\n";
    os << "  >>\n";

    os << ">>\n";

    /* Allocate buffer for packbits data. Worst case: Less than 1% increase */
    guchar *const packb = (guchar *)g_malloc((4*width * 105)/100+2);
    guchar *const plane = (guchar *)g_malloc(4*width);

    os << "image\n";

    ascii85_init();
    
    for (unsigned i = 0; i < height; i++) {
        guchar const *const src = px + i * rs;

        guchar const *src_ptr = src;
        guchar *plane_ptr = plane;
        for (unsigned j = 0; j < width; j++) {
            *(plane_ptr++) = *(src_ptr+3);
            *(plane_ptr++) = *(src_ptr+0);
            *(plane_ptr++) = *(src_ptr+1);
            *(plane_ptr++) = *(src_ptr+2);
            src_ptr += 4;
        }
        
        ascii85_nout(4*width, plane, os);
    }
    ascii85_done(os);

    g_free(packb);
    g_free(plane);

    os << "grestore\n";

    fprintf(ofp, "%s", os.str().c_str());

    return 0;
}

bool
PrintPS::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

/**
* \brief Get "fontEmbedded" param
* \retval TRUE Fonts have to be embedded in the output so that the user might not need to install fonts to have the interpreter read the document correctly
* \retval FALSE No font embedding
*
* Only available for Adobe Type 1 fonts in EPS output till now
*/
bool
PrintPS::fontEmbedded(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("fontEmbedded");
}

#include "clear-n_.h"

void
PrintPS::init(void)
{
    /* SVG in */
    (void) Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
        "<name>" N_("Postscript Print") "</name>\n"
        "<id>" SP_MODULE_KEY_PRINT_PS "</id>\n"
        "<param name=\"bitmap\" type=\"boolean\">false</param>\n"
        "<param name=\"resolution\" type=\"string\">72</param>\n"
        "<param name=\"destination\" type=\"string\">| lp</param>\n"
        "<param name=\"pageBoundingBox\" type=\"boolean\">true</param>\n"
        "<param name=\"textToPath\" type=\"boolean\">true</param>\n"
        "<param name=\"fontEmbedded\" type=\"boolean\">false</param>\n"
        "<print/>\n"
        "</inkscape-extension>", new PrintPS());
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

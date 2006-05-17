#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libnr/nr-blit.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/raster-glyph.h>
#include <libnrtype/RasterFont.h>
#include <libnrtype/TextWrapper.h>
#include <libnrtype/one-glyph.h>

#include <glibmm.h>
#include <gtkmm.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>

#include "font-lister.h"

namespace Inkscape
{
                FontLister::FontLister ()
                {
                    font_list_store = Gtk::ListStore::create (FontList);
                    
                    if (font_factory::Default()->Families(&families))
                    {
                        for (unsigned int i = 0; i < families.length; ++i)
                        {
                            Gtk::TreeModel::iterator iter = font_list_store->append();
                            (*iter)[FontList.font] = reinterpret_cast<const char*>(families.names[i]);

                            NRStyleList styles;
                            if (font_factory::Default()->Styles (reinterpret_cast<const char*>(families.names[i]), &styles));

                            GList *Styles=0;
                            for (unsigned int n = 0; n < styles.length; ++n)
                            {
                                    NRStyleRecord style_record = styles.records[n];
                                    Styles = g_list_append (Styles, strdup(style_record.name));
                            }

                            (*iter)[FontList.styles] = Styles;

                            font_list_store_iter_map.insert (std::make_pair (reinterpret_cast<const char*>(families.names[i]), Gtk::TreePath (iter)));
                        }
                    }
                    
                }

                FontLister::~FontLister ()
                {
                };

                const Glib::RefPtr<Gtk::ListStore>
                FontLister::get_font_list () const
                {
                    return font_list_store;
                }
}


#if 0
#define __SP_FONT_SELECTOR_C__

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libnr/nr-blit.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/raster-glyph.h>
#include <libnrtype/RasterFont.h>
#include <libnrtype/TextWrapper.h>
#include <libnrtype/one-glyph.h>

#include <gtk/gtkframe.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkclist.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkdrawingarea.h>

#include "../display/nr-plain-stuff-gdk.h"
#include <glibmm/i18n.h>

#include "font-selector.h"

/* SPFontSelector */

struct SPFontSelector
{
    GtkHBox hbox;
    
    unsigned int block_emit : 1;
    
    GtkWidget *family;
    GtkWidget *style;
    GtkWidget *size;
    
    NRNameList families;
    NRStyleList styles;
    int familyidx;
    int styleidx;
    gfloat fontsize;
    bool fontsize_dirty;
    font_instance *font;
};


struct SPFontSelectorClass
{
    GtkHBoxClass parent_class;
	
    void (* font_set) (SPFontSelector *fsel, font_instance *font);
};

enum {
    FONT_SET,
    LAST_SIGNAL
};

static void sp_font_selector_class_init(SPFontSelectorClass *c);
static void sp_font_selector_init(SPFontSelector *fsel);
static void sp_font_selector_destroy(GtkObject *object);

static void sp_font_selector_family_select_row(GtkCList *clist, gint row, gint column,
                                               GdkEvent *event, SPFontSelector *fsel);
static void sp_font_selector_style_select_row(GtkCList *clist, gint row, gint column,
                                              GdkEvent *event, SPFontSelector *fsel);
static void sp_font_selector_size_changed(GtkEditable *editable, SPFontSelector *fsel);

static void sp_font_selector_emit_set(SPFontSelector *fsel);

static const gchar *sizes[] = {
	"4", "6", "8", "9", "10", "11", "12", "13", "14",
	"16", "18", "20", "22", "24", "28",
	"32", "36", "40", "48", "56", "64", "72", "144",
	NULL
};

static GtkHBoxClass *fs_parent_class = NULL;
static guint fs_signals[LAST_SIGNAL] = { 0 };

GtkType sp_font_selector_get_type()
{
    static GtkType type = 0;
    if (!type) {
        static const GtkTypeInfo info = {
            "SPFontSelector",
            sizeof(SPFontSelector),
            sizeof(SPFontSelectorClass),
            (GtkClassInitFunc) sp_font_selector_class_init,
            (GtkObjectInitFunc) sp_font_selector_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique(GTK_TYPE_HBOX, &info);
    }
    return type;
}

static void sp_font_selector_class_init(SPFontSelectorClass *c)
{
    GtkObjectClass *object_class = (GtkObjectClass *) c;
  
    fs_parent_class = (GtkHBoxClass* )gtk_type_class(GTK_TYPE_HBOX);
	
    fs_signals[FONT_SET] = gtk_signal_new ("font_set",
                                           GTK_RUN_FIRST,
                                           GTK_CLASS_TYPE(object_class),
                                           GTK_SIGNAL_OFFSET(SPFontSelectorClass, font_set),
                                           gtk_marshal_NONE__POINTER,
                                           GTK_TYPE_NONE,
                                           1, GTK_TYPE_POINTER);
	
	object_class->destroy = sp_font_selector_destroy;
}

static void sp_font_selector_init(SPFontSelector *fsel)
{
	gtk_box_set_homogeneous(GTK_BOX(fsel), TRUE);
	gtk_box_set_spacing(GTK_BOX(fsel), 4);
	
	/* Family frame */
	GtkWidget *f = gtk_frame_new(_("Font family"));
	gtk_widget_show(f);
	gtk_box_pack_start(GTK_BOX(fsel), f, TRUE, TRUE, 0);
	
	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(sw);
	gtk_container_set_border_width(GTK_CONTAINER (sw), 4);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(f), sw);
	
	fsel->family = gtk_clist_new (1);
	gtk_widget_show (fsel->family);
	gtk_clist_set_selection_mode(GTK_CLIST(fsel->family), GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_hide(GTK_CLIST(fsel->family));
	gtk_signal_connect(GTK_OBJECT(fsel->family), "select_row", GTK_SIGNAL_FUNC(sp_font_selector_family_select_row), fsel);
	gtk_container_add(GTK_CONTAINER(sw), fsel->family);
	
	if ((font_factory::Default())->Families(&fsel->families)) {
		gtk_clist_freeze(GTK_CLIST(fsel->family));
		for (guint i = 0; i < fsel->families.length; i++) {
			gtk_clist_append(GTK_CLIST(fsel->family), (gchar **) fsel->families.names + i);
			gtk_clist_set_row_data(GTK_CLIST(fsel->family), i, GUINT_TO_POINTER(i));
		}
		gtk_clist_thaw(GTK_CLIST(fsel->family));
	}
	
	/* Style frame */
	f = gtk_frame_new(_("Style"));
	gtk_widget_show(f);
	gtk_box_pack_start(GTK_BOX (fsel), f, TRUE, TRUE, 0);
	
	GtkWidget *vb = gtk_vbox_new(FALSE, 4);
	gtk_widget_show(vb);
	gtk_container_set_border_width(GTK_CONTAINER (vb), 4);
	gtk_container_add(GTK_CONTAINER(f), vb);
	
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(sw);
	gtk_container_set_border_width(GTK_CONTAINER (sw), 4);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX (vb), sw, TRUE, TRUE, 0);
	
	fsel->style = gtk_clist_new (1);
	gtk_widget_show (fsel->style);
	gtk_clist_set_selection_mode(GTK_CLIST (fsel->style), GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_hide(GTK_CLIST (fsel->style));
	gtk_signal_connect(GTK_OBJECT(fsel->style), "select_row", GTK_SIGNAL_FUNC (sp_font_selector_style_select_row), fsel);
	gtk_container_add(GTK_CONTAINER(sw), fsel->style);
	
	GtkWidget *hb = gtk_hbox_new(FALSE, 4);
	gtk_widget_show(hb);
	gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);
	
	fsel->size = gtk_combo_new();
	gtk_widget_show (fsel->size);
	gtk_combo_set_value_in_list(GTK_COMBO (fsel->size), FALSE, FALSE);
	gtk_combo_set_use_arrows(GTK_COMBO (fsel->size), TRUE);
	gtk_combo_set_use_arrows_always(GTK_COMBO (fsel->size), TRUE);
	gtk_widget_set_size_request(fsel->size, 90, -1);
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO(fsel->size)->entry), "changed", GTK_SIGNAL_FUNC(sp_font_selector_size_changed), fsel);
	gtk_box_pack_end(GTK_BOX(hb), fsel->size, FALSE, FALSE, 0);
	
	GtkWidget *l = gtk_label_new(_("Font size:"));
	gtk_widget_show(l);
	gtk_box_pack_end(GTK_BOX (hb), l, FALSE, FALSE, 0);
	
	/* Setup strings */
	GList *sl = NULL;
	for (int i = 0; sizes[i] != NULL; i++) {
		sl = g_list_prepend (sl, (gpointer) sizes[i]);
	}
	sl = g_list_reverse (sl);
	gtk_combo_set_popdown_strings(GTK_COMBO(fsel->size), sl);
	g_list_free (sl);
	
	fsel->familyidx = 0;
	fsel->styleidx = 0;
	fsel->fontsize = 10.0;
	fsel->fontsize_dirty = false;
	fsel->font = NULL;
}

static void sp_font_selector_destroy(GtkObject *object)
{
    SPFontSelector *fsel = SP_FONT_SELECTOR (object);
    
    if (fsel->font) {
        fsel->font->Unref();
        fsel->font = NULL;
    }
    
    if (fsel->families.length > 0) {
        nr_name_list_release(&fsel->families);
        fsel->families.length = 0;
    }
    
    if (fsel->styles.length > 0) {
        nr_style_list_release(&fsel->styles);
        fsel->styles.length = 0;
    }
    
    if (GTK_OBJECT_CLASS(fs_parent_class)->destroy) {
        GTK_OBJECT_CLASS(fs_parent_class)->destroy(object);
    }
}

static void sp_font_selector_family_select_row(GtkCList *clist, gint row, gint column,
                                               GdkEvent *event, SPFontSelector *fsel)
{
    fsel->familyidx = GPOINTER_TO_UINT (gtk_clist_get_row_data (clist, row));
	
    if (fsel->styles.length > 0) {
        nr_style_list_release (&fsel->styles);
        fsel->styles.length = 0;
        fsel->styleidx = 0;
    }
    gtk_clist_clear (GTK_CLIST (fsel->style));
    
    if ( static_cast<unsigned int> (fsel->familyidx) < fsel->families.length ) {
        
        const gchar *family = (const gchar *) fsel->families.names[fsel->familyidx];
        
        if ((font_factory::Default())->Styles(family, &fsel->styles)) {

            gtk_clist_freeze(GTK_CLIST(fsel->style));
            for (unsigned int i = 0; i < fsel->styles.length; i++) {
                
                const gchar *p = (const gchar *) ((fsel->styles.records)[i].name);
				
                gtk_clist_append(GTK_CLIST(fsel->style), (gchar **) &p);
                gtk_clist_set_row_data(GTK_CLIST(fsel->style), static_cast<gint> (i), GUINT_TO_POINTER (i));
            }
            gtk_clist_thaw(GTK_CLIST(fsel->style));
            gtk_clist_select_row(GTK_CLIST(fsel->style), 0, 0);
        }
    }
}

static void sp_font_selector_style_select_row(GtkCList *clist, gint row, gint column,
                                              GdkEvent *event, SPFontSelector *fsel)
{
    fsel->styleidx = GPOINTER_TO_UINT(gtk_clist_get_row_data(clist, row));
	
    if (!fsel->block_emit) {
        sp_font_selector_emit_set (fsel);
    }
}

static void sp_font_selector_size_changed(GtkEditable *editable, SPFontSelector *fsel)
{
    const gchar *sstr = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(fsel->size)->entry));
    gfloat old_size = fsel->fontsize;
    fsel->fontsize = MAX(atof(sstr), 0.1);
    if ( fabs(fsel->fontsize-old_size) > 0.001) {
        fsel->fontsize_dirty = true;
    }
	
    sp_font_selector_emit_set(fsel);
}

static void sp_font_selector_emit_set(SPFontSelector *fsel)
{
    font_instance *font;
    
    if (static_cast<unsigned int>(fsel->styleidx) < fsel->styles.length
        && static_cast<unsigned int>(fsel->familyidx) < fsel->families.length)
    {
        font = (font_factory::Default())->FaceFromDescr ((gchar *) ((fsel->families.names)[fsel->familyidx]),
                                                         (gchar *) ((fsel->styles.records)[fsel->styleidx].name));
    } else {
        font = NULL;
    }
    
    // FIXME: when a text object uses non-available font, font==NULL and we can't set size
    // (and the size shown in the widget is invalid). To fix, here we must always get some
    // default font, exactly the same as sptext uses for on-canvas display, so that
    // font!=NULL ever.
    if (font != fsel->font || ( font && fsel->fontsize_dirty ) ) {
        if ( font ) {
            font->Ref();
        }
        if ( fsel->font ) {
            fsel->font->Unref();
        }
        fsel->font = font;
        gtk_signal_emit(GTK_OBJECT(fsel), fs_signals[FONT_SET], fsel->font);
    }
    fsel->fontsize_dirty = false;
    if (font) {
        font->Unref();
    }
    font = NULL;
}

GtkWidget *sp_font_selector_new()
{
    SPFontSelector *fsel = (SPFontSelector*) gtk_type_new(SP_TYPE_FONT_SELECTOR);
  
    return (GtkWidget *) fsel;
}

void sp_font_selector_set_font(SPFontSelector *fsel, font_instance *font, double size)
{
    GtkCList *fcl = GTK_CLIST(fsel->family);
    GtkCList *scl = GTK_CLIST(fsel->style);
	
    if (font && (fsel->font != font || size != fsel->fontsize)) {
        { // select family in the list
            gchar family[256];
            font->Family (family, 256);

            unsigned int i;
            for (i = 0; i < fsel->families.length; i++) {
                if (!strcasecmp (family, (gchar *)fsel->families.names[i])) {
                    break;
                }
            }
            
            if (i >= fsel->families.length) {
                return;
            }
			
            fsel->block_emit = TRUE;
            gtk_clist_select_row(fcl, i, 0);
            gtk_clist_moveto(fcl, i, 0, 0.66, 0.0);
            fsel->block_emit = FALSE;
        }
		
        { // select best-matching style in the list
            gchar descr[256];
            font->Name(descr, 256);
            PangoFontDescription *descr_ = pango_font_description_from_string(descr);
            PangoFontDescription *best_ = pango_font_description_from_string((fsel->styles.records)[0].descr);
            guint best_i = 0;

            for (guint i = 0; i < fsel->styles.length; i++) {
                PangoFontDescription *try_ = pango_font_description_from_string((fsel->styles.records)[i].descr);
                if (pango_font_description_better_match(descr_, best_, try_)) {
                    pango_font_description_free(best_);
                    best_ = pango_font_description_from_string((fsel->styles.records)[i].descr);
                    best_i = i;
                }
                pango_font_description_free(try_);
            }
            
            gtk_clist_select_row(scl, best_i, 0);
            gtk_clist_moveto(scl, best_i, 0, 0.66, 0.0);
        }
        
        if (size != fsel->fontsize) {
            gchar s[8];
            g_snprintf (s, 8, "%.5g", size); // UI, so printf is ok
            gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (fsel->size)->entry), s);
            fsel->fontsize = size;
        }
    }
}

font_instance* sp_font_selector_get_font(SPFontSelector *fsel)
{
    if (fsel->font) {
        fsel->font->Ref();
    }
    
    return fsel->font;
}

double sp_font_selector_get_size(SPFontSelector *fsel)
{
    return fsel->fontsize;
}

/* SPFontPreview */

struct SPFontPreview
{
    GtkDrawingArea darea;
    
    font_instance *font;
    raster_font *rfont;
    gchar *phrase;
    unsigned long rgba;
};

struct SPFontPreviewClass
{
    GtkDrawingAreaClass parent_class;
};

static void sp_font_preview_class_init(SPFontPreviewClass *c);
static void sp_font_preview_init(SPFontPreview *fsel);
static void sp_font_preview_destroy(GtkObject *object);

void sp_font_preview_size_request(GtkWidget *widget, GtkRequisition *req);
static gint sp_font_preview_expose(GtkWidget *widget, GdkEventExpose *event);

static GtkDrawingAreaClass *fp_parent_class = NULL;

GtkType sp_font_preview_get_type()
{
    static GtkType type = 0;
    if (!type) {
        static const GtkTypeInfo info = {
            "SPFontPreview",
            sizeof (SPFontPreview),
            sizeof (SPFontPreviewClass),
            (GtkClassInitFunc) sp_font_preview_class_init,
            (GtkObjectInitFunc) sp_font_preview_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique (GTK_TYPE_DRAWING_AREA, &info);
    }
    return type;
}

static void sp_font_preview_class_init (SPFontPreviewClass *c)
{
    GtkObjectClass *object_class = (GtkObjectClass *) c;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) c;
    
    fp_parent_class = (GtkDrawingAreaClass*) gtk_type_class(GTK_TYPE_DRAWING_AREA);
    
    object_class->destroy = sp_font_preview_destroy;
	
    widget_class->size_request = sp_font_preview_size_request;
    widget_class->expose_event = sp_font_preview_expose;
}

static void sp_font_preview_init(SPFontPreview *fprev)
{
    fprev->rgba = 0x000000ff;
}

static void sp_font_preview_destroy(GtkObject *object)
{
    SPFontPreview *fprev = SP_FONT_PREVIEW (object);
	
    if (fprev->rfont) {
        fprev->rfont->Unref();
        fprev->rfont = NULL;
    }
	
    if (fprev->font) {
        fprev->font->Unref();
        fprev->font = NULL;
    }
	
    g_free(fprev->phrase);
    fprev->phrase = NULL;
	
    if (GTK_OBJECT_CLASS (fp_parent_class)->destroy) {
        GTK_OBJECT_CLASS (fp_parent_class)->destroy(object);
    }
}

void sp_font_preview_size_request(GtkWidget *widget, GtkRequisition *req)
{
    req->width = 256;
    req->height = 32;
}

#define SPFP_MAX_LEN 64

static gint sp_font_preview_expose(GtkWidget *widget, GdkEventExpose *event)
{
    SPFontPreview *fprev = SP_FONT_PREVIEW(widget);
	
    if (GTK_WIDGET_DRAWABLE (widget)) {
        if (fprev->rfont) {
            
            int glyphs[SPFP_MAX_LEN];
            double hpos[SPFP_MAX_LEN];
            
            font_instance *tface = fprev->rfont->daddy;
            
            double theSize = NR_MATRIX_DF_EXPANSION (&fprev->rfont->style.transform);
            
            gchar const *p;
            if (fprev->phrase) {
                p = fprev->phrase;
            } else {
                /* TRANSLATORS: Test string used in text and font dialog (when no
                 * text has been entered) to get a preview of the font.  Choose
                 * some representative characters that users of your locale will be
                 * interested in. */
		p = _("AaBbCcIiPpQq12369$\342\202\254\302\242?.;/()");
            }
            int len = 0;
            
            NRRect bbox;
            bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
            
            text_wrapper* str_text=new text_wrapper;
            str_text->SetDefaultFont(tface);
            str_text->AppendUTF8(p,-1);
            if ( str_text->uni32_length > 0 ) {
                str_text->DoLayout();
                if ( str_text->glyph_length > 0 ) {
                    PangoFont *curPF = NULL;
                    font_instance *curF = NULL;
                    for (int i = 0; i < str_text->glyph_length && i < SPFP_MAX_LEN; i++) {
                        if ( str_text->glyph_text[i].font != curPF ) {
                            curPF = str_text->glyph_text[i].font;
                            if (curF) {
                                curF->Unref();
                            }
                            curF = NULL;
                            if ( curPF ) {
                                PangoFontDescription* pfd = pango_font_describe(curPF);
                                curF = (font_factory::Default())->Face(pfd);
                                pango_font_description_free(pfd);
                            }
                        }
                        NR::Point base_pt(str_text->glyph_text[i].x, str_text->glyph_text[i].y);
                        base_pt *= theSize;
						
                        glyphs[len] = str_text->glyph_text[i].gl;
                        hpos[len] = base_pt[0];
                        len++;
                        if ( curF ) {
                            NR::Rect nbbox = curF->BBox(str_text->glyph_text[i].gl);
                            bbox.x0 = MIN(bbox.x0, base_pt[NR::X] + theSize * (nbbox.min())[0]);
                            bbox.y0 = MIN(bbox.y0, base_pt[NR::Y] - theSize * (nbbox.max())[1]);
                            bbox.x1 = MAX(bbox.x1, base_pt[NR::X] + theSize * (nbbox.max())[0]);
                            bbox.y1 = MAX(bbox.y1, base_pt[NR::Y] - theSize * (nbbox.min())[1]);
                        }
                    }
                    if ( curF ) {
                        curF->Unref();
                    }
                }
            }
            
            // XXX: FIXME: why does this code ignore adv.y
            /*			while (p && *p && (len < SPFP_MAX_LEN)) {
				unsigned int unival;
                                NRRect gbox;
                                unival = g_utf8_get_char (p);
                                glyphs[len] =  tface->MapUnicodeChar( unival);
                                hpos[len] = (int)px;
                                NR::Point adv = fprev->rfont->Advance(glyphs[len]);
                                fprev->rfont->BBox( glyphs[len], &gbox);
                                bbox.x0 = MIN (px + gbox.x0, bbox.x0);
                                bbox.y0 = MIN (py + gbox.y0, bbox.y0);
                                bbox.x1 = MAX (px + gbox.x1, bbox.x1);
                                bbox.y1 = MAX (py + gbox.y1, bbox.y1);
                                px += adv[NR::X];
                                len += 1;
                                p = g_utf8_next_char (p);
                                }*/
            
            float startx = (widget->allocation.width - (bbox.x1 - bbox.x0)) / 2;
            float starty = widget->allocation.height - (widget->allocation.height - (bbox.y1 - bbox.y0)) / 2 - bbox.y1;
			
            for (int y = event->area.y; y < event->area.y + event->area.height; y += 64) {
                for (int x = event->area.x; x < event->area.x + event->area.width; x += 64) {
                    NRPixBlock pb, m;
                    int x0 = x;
                    int y0 = y;
                    int x1 = MIN(x0 + 64, event->area.x + event->area.width);
                    int y1 = MIN(y0 + 64, event->area.y + event->area.height);
                    guchar *ps = nr_pixelstore_16K_new (TRUE, 0xff);
                    nr_pixblock_setup_extern(&pb, NR_PIXBLOCK_MODE_R8G8B8, x0, y0, x1, y1, ps, 3 * (x1 - x0), FALSE, FALSE);
                    nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, x0, y0, x1, y1, TRUE);
                    pb.empty = FALSE;
                    
                    PangoFont *curPF = NULL;
                    font_instance *curF = NULL;
                    raster_font *curRF = NULL;
                    for (int i=0; i < len; i++) {
                        if ( str_text->glyph_text[i].font != curPF ) {
                            curPF=str_text->glyph_text[i].font;
                            if ( curF ) {
                                curF->Unref();
                            }
                            curF = NULL;
                            if ( curPF ) {
                                PangoFontDescription* pfd = pango_font_describe(curPF);
                                curF=(font_factory::Default())->Face(pfd);
                                pango_font_description_free(pfd);
                            }
                            if ( curF ) {
                                if ( curRF ) {
                                    curRF->Unref();
                                }
                                curRF = NULL;
                                curRF = curF->RasterFont(fprev->rfont->style);
                            }
                        }
                        raster_glyph *g = (curRF) ? curRF->GetGlyph(glyphs[i]) : NULL;
                        if ( g ) {
                            g->Blit(NR::Point(hpos[i] + startx, starty), m);
                        }
                    }
                    if (curRF) {
                        curRF->Unref();
                    }
                    if (curF) {
                        curF->Unref();
                    }
                    
                    nr_blit_pixblock_mask_rgba32(&pb, &m, fprev->rgba);
                    gdk_draw_rgb_image(widget->window, widget->style->black_gc,
                                       x0, y0, x1 - x0, y1 - y0,
                                       GDK_RGB_DITHER_NONE, NR_PIXBLOCK_PX (&pb), pb.rs);
                    nr_pixblock_release(&m);
                    nr_pixblock_release(&pb);
                    nr_pixelstore_16K_free(ps);
                }
            }
            
            delete str_text;
            
        } else {
            nr_gdk_draw_gray_garbage(widget->window, widget->style->black_gc,
                                     event->area.x, event->area.y,
                                     event->area.width, event->area.height);
        }
    }
    
    return TRUE;
}

GtkWidget * sp_font_preview_new()
{
    GtkWidget *w = (GtkWidget*) gtk_type_new(SP_TYPE_FONT_PREVIEW);
    
    return w;
}

void sp_font_preview_set_font(SPFontPreview *fprev, font_instance *font, SPFontSelector *fsel)
{
    if (font != fprev->font) {
        if (font) {
            font->Ref();
        }
        if (fprev->font) {
            fprev->font->Unref();
        }
        fprev->font = font;
	
        if (fprev->rfont) {
            fprev->rfont->Unref();
            fprev->rfont=NULL;
        }
        if (fprev->font) {
            NRMatrix flip;
            nr_matrix_set_scale (&flip, fsel->fontsize, -fsel->fontsize);
            fprev->rfont = fprev->font->RasterFont(flip, 0);
        }
        if (GTK_WIDGET_DRAWABLE (fprev)) gtk_widget_queue_draw (GTK_WIDGET (fprev));
    }
}

void sp_font_preview_set_rgba32(SPFontPreview *fprev, guint32 rgba)
{
    fprev->rgba = rgba;
    if (GTK_WIDGET_DRAWABLE (fprev)) {
        gtk_widget_queue_draw (GTK_WIDGET (fprev));
    }
}

void sp_font_preview_set_phrase(SPFontPreview *fprev, const gchar *phrase)
{
    g_free (fprev->phrase);
    if (phrase) {
        fprev->phrase = g_strdup (phrase);
    } else {
        fprev->phrase = NULL;
    }
    if (GTK_WIDGET_DRAWABLE(fprev)) {
        gtk_widget_queue_draw (GTK_WIDGET (fprev));
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
#endif

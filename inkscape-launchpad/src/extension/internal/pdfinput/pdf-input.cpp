/*
 * Native PDF import using libpoppler.
 *
 * Authors:
 *   miklos erdelyi
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pdf-input.h"

#ifdef HAVE_POPPLER
#include <poppler/goo/GooString.h>
#include <poppler/ErrorCodes.h>
#include <poppler/GlobalParams.h>
#include <poppler/PDFDoc.h>
#include <poppler/Page.h>
#include <poppler/Catalog.h>

#ifdef HAVE_POPPLER_CAIRO
#include <poppler/glib/poppler.h>
#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>
#endif

#include <gtkmm/alignment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>

#if WITH_GTKMM_3_0
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#endif

#include "extension/system.h"
#include "extension/input.h"
#include "svg-builder.h"
#include "pdf-parser.h"

#include "document-private.h"
#include "document-undo.h"
#include "inkscape.h"
#include "util/units.h"

#include "ui/dialog-events.h"
#include <gtk/gtk.h>
#include "ui/widget/spinbutton.h"
#include "ui/widget/frame.h"
#include <glibmm/i18n.h>

#include <gdkmm/general.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
 * \brief The PDF import dialog
 * FIXME: Probably this should be placed into src/ui/dialog
 */

static const gchar * crop_setting_choices[] = {
	//TRANSLATORS: The following are document crop settings for PDF import
	// more info: http://www.acrobatusers.com/tech_corners/javascript_corner/tips/2006/page_bounds/
    N_("media box"),
    N_("crop box"),
    N_("trim box"),
    N_("bleed box"),
    N_("art box")
};

PdfImportDialog::PdfImportDialog(PDFDoc *doc, const gchar */*uri*/)
{
#ifdef HAVE_POPPLER_CAIRO
    _poppler_doc = NULL;
#endif // HAVE_POPPLER_CAIRO
    _pdf_doc = doc;
    cancelbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-cancel")));
    okbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-ok")));
    _labelSelect = Gtk::manage(new class Gtk::Label(_("Select page:")));

    // Page number
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> _pageNumberSpin_adj = Gtk::Adjustment::create(1, 1, _pdf_doc->getNumPages(), 1, 10, 0);
    _pageNumberSpin = Gtk::manage(new Inkscape::UI::Widget::SpinButton(_pageNumberSpin_adj, 1, 1));
#else
    Gtk::Adjustment *_pageNumberSpin_adj = Gtk::manage(
            new class Gtk::Adjustment(1, 1, _pdf_doc->getNumPages(), 1, 10, 0));
    _pageNumberSpin = Gtk::manage(new class Inkscape::UI::Widget::SpinButton(*_pageNumberSpin_adj, 1, 1));
#endif
    _labelTotalPages = Gtk::manage(new class Gtk::Label());
    hbox2 = Gtk::manage(new class Gtk::HBox(false, 0));
    // Disable the page selector when there's only one page
    int num_pages = _pdf_doc->getCatalog()->getNumPages();
    if ( num_pages == 1 ) {
        _pageNumberSpin->set_sensitive(false);
    } else {
        // Display total number of pages
        gchar *label_text = g_strdup_printf(_("out of %i"), num_pages);
        _labelTotalPages->set_label(label_text);
        g_free(label_text);
    }

    // Crop settings
    _cropCheck = Gtk::manage(new class Gtk::CheckButton(_("Clip to:")));
    _cropTypeCombo = Gtk::manage(new class Gtk::ComboBoxText());
    int num_crop_choices = sizeof(crop_setting_choices) / sizeof(crop_setting_choices[0]);
    for ( int i = 0 ; i < num_crop_choices ; i++ ) {
        _cropTypeCombo->append(_(crop_setting_choices[i]));
    }
    _cropTypeCombo->set_active_text(_(crop_setting_choices[0]));
    _cropTypeCombo->set_sensitive(false);

    hbox3 = Gtk::manage(new class Gtk::HBox(false, 4));
    vbox2 = Gtk::manage(new class Gtk::VBox(false, 4));
    _pageSettingsFrame = Gtk::manage(new class Inkscape::UI::Widget::Frame(_("Page settings")));
    _labelPrecision = Gtk::manage(new class Gtk::Label(_("Precision of approximating gradient meshes:")));
    _labelPrecisionWarning = Gtk::manage(new class Gtk::Label(_("<b>Note</b>: setting the precision too high may result in a large SVG file and slow performance.")));

#ifdef HAVE_POPPLER_CAIRO
    Gtk::RadioButton::Group group;
    _importViaPoppler  = Gtk::manage(new class Gtk::RadioButton(group,_("Poppler/Cairo import")));
    _labelViaPoppler = Gtk::manage(new class Gtk::Label(_("Import via external library. Text consists of groups containing cloned glyphs where each glyph is a path. Images are stored internally. Meshes cause entire document to be rendered as a raster image.")));
    _importViaInternal = Gtk::manage(new class Gtk::RadioButton(group,_("Internal import")));
    _labelViaInternal = Gtk::manage(new class Gtk::Label(_("Import via internal (Poppler derived) library. Text is stored as text but white space is missing. Meshes are converted to tiles, the number depends on the precision set below.")));
#endif

#if WITH_GTKMM_3_0
    _fallbackPrecisionSlider_adj = Gtk::Adjustment::create(2, 1, 256, 1, 10, 10);
    _fallbackPrecisionSlider = Gtk::manage(new class Gtk::Scale(_fallbackPrecisionSlider_adj));
#else
    _fallbackPrecisionSlider_adj = Gtk::manage(new class Gtk::Adjustment(2, 1, 256, 1, 10, 10));
    _fallbackPrecisionSlider = Gtk::manage(new class Gtk::HScale(*_fallbackPrecisionSlider_adj));
#endif
    _fallbackPrecisionSlider->set_value(2.0);
    _labelPrecisionComment = Gtk::manage(new class Gtk::Label(_("rough")));
    hbox6 = Gtk::manage(new class Gtk::HBox(false, 4));

    // Text options
    // _labelText = Gtk::manage(new class Gtk::Label(_("Text handling:")));
    // _textHandlingCombo = Gtk::manage(new class Gtk::ComboBoxText());
    // _textHandlingCombo->append(_("Import text as text"));
    // _textHandlingCombo->set_active_text(_("Import text as text"));
    // hbox5 = Gtk::manage(new class Gtk::HBox(false, 4));

    // Font option
    _localFontsCheck = Gtk::manage(new class Gtk::CheckButton(_("Replace PDF fonts by closest-named installed fonts")));

    _embedImagesCheck = Gtk::manage(new class Gtk::CheckButton(_("Embed images")));
    vbox3 = Gtk::manage(new class Gtk::VBox(false, 4));
    _importSettingsFrame = Gtk::manage(new class Inkscape::UI::Widget::Frame(_("Import settings")));
    vbox1 = Gtk::manage(new class Gtk::VBox(false, 4));
    _previewArea = Gtk::manage(new class Gtk::DrawingArea());
    hbox1 = Gtk::manage(new class Gtk::HBox(false, 4));
    cancelbutton->set_can_focus();
    cancelbutton->set_can_default();
    cancelbutton->set_relief(Gtk::RELIEF_NORMAL);
    okbutton->set_can_focus();
    okbutton->set_can_default();
    okbutton->set_relief(Gtk::RELIEF_NORMAL);
    this->get_action_area()->property_layout_style().set_value(Gtk::BUTTONBOX_END);
    _labelSelect->set_alignment(0.5,0.5);
    _labelSelect->set_padding(4,0);
    _labelSelect->set_justify(Gtk::JUSTIFY_LEFT);
    _labelSelect->set_line_wrap(false);
    _labelSelect->set_use_markup(false);
    _labelSelect->set_selectable(false);
    _pageNumberSpin->set_can_focus();
    _pageNumberSpin->set_update_policy(Gtk::UPDATE_ALWAYS);
    _pageNumberSpin->set_numeric(true);
    _pageNumberSpin->set_digits(0);
    _pageNumberSpin->set_wrap(false);
    _labelTotalPages->set_alignment(0.5,0.5);
    _labelTotalPages->set_padding(4,0);
    _labelTotalPages->set_justify(Gtk::JUSTIFY_LEFT);
    _labelTotalPages->set_line_wrap(false);
    _labelTotalPages->set_use_markup(false);
    _labelTotalPages->set_selectable(false);
    hbox2->pack_start(*_labelSelect, Gtk::PACK_SHRINK, 4);
    hbox2->pack_start(*_pageNumberSpin, Gtk::PACK_SHRINK, 4);
    hbox2->pack_start(*_labelTotalPages, Gtk::PACK_SHRINK, 4);
    _cropCheck->set_can_focus();
    _cropCheck->set_relief(Gtk::RELIEF_NORMAL);
    _cropCheck->set_mode(true);
    _cropCheck->set_active(false);
    _cropTypeCombo->set_border_width(1);
    hbox3->pack_start(*_cropCheck, Gtk::PACK_SHRINK, 4);
    hbox3->pack_start(*_cropTypeCombo, Gtk::PACK_SHRINK, 0);
    vbox2->pack_start(*hbox2);
    vbox2->pack_start(*hbox3);
    _pageSettingsFrame->add(*vbox2);
    _pageSettingsFrame->set_border_width(4);
    _labelPrecision->set_alignment(0,0.5);
    _labelPrecision->set_padding(4,0);
    _labelPrecision->set_justify(Gtk::JUSTIFY_LEFT);
    _labelPrecision->set_line_wrap(true);
    _labelPrecision->set_use_markup(false);
    _labelPrecision->set_selectable(false);
    _labelPrecisionWarning->set_alignment(0,0.5);
    _labelPrecisionWarning->set_padding(4,0);
    _labelPrecisionWarning->set_justify(Gtk::JUSTIFY_LEFT);
    _labelPrecisionWarning->set_line_wrap(true);
    _labelPrecisionWarning->set_use_markup(true);
    _labelPrecisionWarning->set_selectable(false);

#ifdef HAVE_POPPLER_CAIRO
    _importViaPoppler->set_can_focus();
    _importViaPoppler->set_relief(Gtk::RELIEF_NORMAL);
    _importViaPoppler->set_mode(true);
    _importViaPoppler->set_active(false);
    _importViaInternal->set_can_focus();
    _importViaInternal->set_relief(Gtk::RELIEF_NORMAL);
    _importViaInternal->set_mode(true);
    _importViaInternal->set_active(true);
    _labelViaPoppler->set_line_wrap(true);
    _labelViaInternal->set_line_wrap(true);
#endif

    _fallbackPrecisionSlider->set_size_request(180,-1);
    _fallbackPrecisionSlider->set_can_focus();
    _fallbackPrecisionSlider->set_inverted(false);
    _fallbackPrecisionSlider->set_digits(1);
    _fallbackPrecisionSlider->set_draw_value(true);
    _fallbackPrecisionSlider->set_value_pos(Gtk::POS_TOP);
    _labelPrecisionComment->set_size_request(90,-1);
    _labelPrecisionComment->set_alignment(0.5,0.5);
    _labelPrecisionComment->set_padding(4,0);
    _labelPrecisionComment->set_justify(Gtk::JUSTIFY_LEFT);
    _labelPrecisionComment->set_line_wrap(false);
    _labelPrecisionComment->set_use_markup(false);
    _labelPrecisionComment->set_selectable(false);
    hbox6->pack_start(*_fallbackPrecisionSlider, Gtk::PACK_SHRINK, 4);
    hbox6->pack_start(*_labelPrecisionComment, Gtk::PACK_SHRINK, 0);
    // _labelText->set_alignment(0.5,0.5);
    // _labelText->set_padding(4,0);
    // _labelText->set_justify(Gtk::JUSTIFY_LEFT);
    // _labelText->set_line_wrap(false);
    // _labelText->set_use_markup(false);
    // _labelText->set_selectable(false);
    // hbox5->pack_start(*_labelText, Gtk::PACK_SHRINK, 0);
    // hbox5->pack_start(*_textHandlingCombo, Gtk::PACK_SHRINK, 0);
    _localFontsCheck->set_can_focus();
    _localFontsCheck->set_relief(Gtk::RELIEF_NORMAL);
    _localFontsCheck->set_mode(true);
    _localFontsCheck->set_active(true);
    _embedImagesCheck->set_can_focus();
    _embedImagesCheck->set_relief(Gtk::RELIEF_NORMAL);
    _embedImagesCheck->set_mode(true);
    _embedImagesCheck->set_active(true);
#ifdef HAVE_POPPLER_CAIRO
    vbox3->pack_start(*_importViaPoppler,  Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_labelViaPoppler,  Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_importViaInternal, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_labelViaInternal, Gtk::PACK_SHRINK, 0);
#endif    
    vbox3->pack_start(*_localFontsCheck, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_embedImagesCheck, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_labelPrecision, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*hbox6, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_labelPrecisionWarning, Gtk::PACK_SHRINK, 0);
    // vbox3->pack_start(*hbox5, Gtk::PACK_SHRINK, 4);
    _importSettingsFrame->add(*vbox3);
    _importSettingsFrame->set_border_width(4);
    vbox1->pack_start(*_pageSettingsFrame, Gtk::PACK_EXPAND_PADDING, 0);
    vbox1->pack_start(*_importSettingsFrame, Gtk::PACK_EXPAND_PADDING, 0);
    hbox1->pack_start(*vbox1);
    hbox1->pack_start(*_previewArea, Gtk::PACK_EXPAND_WIDGET, 4);

#if WITH_GTKMM_3_0
    get_content_area()->set_homogeneous(false);
    get_content_area()->set_spacing(0);
    get_content_area()->pack_start(*hbox1);
#else
    this->get_vbox()->set_homogeneous(false);
    this->get_vbox()->set_spacing(0);
    this->get_vbox()->pack_start(*hbox1);
#endif

    this->set_title(_("PDF Import Settings"));
    this->set_modal(true);
    sp_transientize(GTK_WIDGET(this->gobj()));  //Make transient
    this->property_window_position().set_value(Gtk::WIN_POS_NONE);
    this->set_resizable(true);
    this->property_destroy_with_parent().set_value(false);
    this->add_action_widget(*cancelbutton, -6);
    this->add_action_widget(*okbutton, -5);

    this->show_all();
    
    // Connect signals
#if WITH_GTKMM_3_0
    _previewArea->signal_draw().connect(sigc::mem_fun(*this, &PdfImportDialog::_onDraw));
#else
    _previewArea->signal_expose_event().connect(sigc::mem_fun(*this, &PdfImportDialog::_onExposePreview));
#endif

    _pageNumberSpin_adj->signal_value_changed().connect(sigc::mem_fun(*this, &PdfImportDialog::_onPageNumberChanged));
    _cropCheck->signal_toggled().connect(sigc::mem_fun(*this, &PdfImportDialog::_onToggleCropping));
    _fallbackPrecisionSlider_adj->signal_value_changed().connect(sigc::mem_fun(*this, &PdfImportDialog::_onPrecisionChanged));
#ifdef HAVE_POPPLER_CAIRO
    _importViaPoppler->signal_toggled().connect(sigc::mem_fun(*this, &PdfImportDialog::_onToggleImport));
#endif

    _render_thumb = false;
#ifdef HAVE_POPPLER_CAIRO
    _cairo_surface = NULL;
    _render_thumb = true;

    // Create PopplerDocument
    Glib::ustring filename = _pdf_doc->getFileName()->getCString();
    if (!Glib::path_is_absolute(filename)) {
        filename = Glib::build_filename(Glib::get_current_dir(),filename);
    }
    Glib::ustring full_uri = Glib::filename_to_uri(filename);
    
    if (!full_uri.empty()) {
        _poppler_doc = poppler_document_new_from_file(full_uri.c_str(), NULL, NULL);
    }

    // Set sensitivity of some widgets based on selected import type.
    _onToggleImport();
#endif

    // Set default preview size
    _preview_width = 200;
    _preview_height = 300;

    // Init preview
    _thumb_data = NULL;
    _pageNumberSpin_adj->set_value(1.0);
    _current_page = 1;
    _setPreviewPage(_current_page);

    set_default (*okbutton);
    set_focus (*okbutton);
}

PdfImportDialog::~PdfImportDialog() {
#ifdef HAVE_POPPLER_CAIRO
    if (_cairo_surface) {
        cairo_surface_destroy(_cairo_surface);
    }
    if (_poppler_doc) {
        g_object_unref(G_OBJECT(_poppler_doc));
    }
#endif
    if (_thumb_data) {
        if (_render_thumb) {
            delete _thumb_data;
        } else {
            gfree(_thumb_data);
        }
    }
}

bool PdfImportDialog::showDialog() {
    show();
    gint b = run();
    hide();
    if ( b == Gtk::RESPONSE_OK ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int PdfImportDialog::getSelectedPage() {
    return _current_page;
}

bool PdfImportDialog::getImportMethod() {
#ifdef HAVE_POPPLER_CAIRO
    return _importViaPoppler->get_active();
#else
    return false;
#endif
}

/**
 * \brief Retrieves the current settings into a repr which SvgBuilder will use
 *        for determining the behaviour desired by the user
 */
void PdfImportDialog::getImportSettings(Inkscape::XML::Node *prefs) {
    sp_repr_set_svg_double(prefs, "selectedPage", (double)_current_page);
    if (_cropCheck->get_active()) {
        Glib::ustring current_choice = _cropTypeCombo->get_active_text();
        int num_crop_choices = sizeof(crop_setting_choices) / sizeof(crop_setting_choices[0]);
        int i = 0;
        for ( ; i < num_crop_choices ; i++ ) {
            if ( current_choice == _(crop_setting_choices[i]) ) {
                break;
            }
        }
        sp_repr_set_svg_double(prefs, "cropTo", (double)i);
    } else {
        sp_repr_set_svg_double(prefs, "cropTo", -1.0);
    }
    sp_repr_set_svg_double(prefs, "approximationPrecision",
                           _fallbackPrecisionSlider->get_value());
    if (_localFontsCheck->get_active()) {
        prefs->setAttribute("localFonts", "1");
    } else {
        prefs->setAttribute("localFonts", "0");
    }
    if (_embedImagesCheck->get_active()) {
        prefs->setAttribute("embedImages", "1");
    } else {
        prefs->setAttribute("embedImages", "0");
    }
#ifdef HAVE_POPPLER_CAIRO
    if (_importViaPoppler->get_active()) {
        prefs->setAttribute("importviapoppler", "1");
    } else {
        prefs->setAttribute("importviapoppler", "0");
    }
#endif
}

/**
 * \brief Redisplay the comment on the current approximation precision setting
 * Evenly divides the interval of possible values between the available labels.
 */
void PdfImportDialog::_onPrecisionChanged() {

    static Glib::ustring precision_comments[] = {
        Glib::ustring(C_("PDF input precision", "rough")),
        Glib::ustring(C_("PDF input precision", "medium")),
        Glib::ustring(C_("PDF input precision", "fine")),
        Glib::ustring(C_("PDF input precision", "very fine"))
    };

    double min = _fallbackPrecisionSlider_adj->get_lower();
    double max = _fallbackPrecisionSlider_adj->get_upper();
    int num_intervals = sizeof(precision_comments) / sizeof(precision_comments[0]);
    double interval_len = ( max - min ) / (double)num_intervals;
    double value = _fallbackPrecisionSlider_adj->get_value();
    int comment_idx = (int)floor( ( value - min ) / interval_len );
    _labelPrecisionComment->set_label(precision_comments[comment_idx]);
}

void PdfImportDialog::_onToggleCropping() {
    _cropTypeCombo->set_sensitive(_cropCheck->get_active());
}

void PdfImportDialog::_onPageNumberChanged() {
    int page = _pageNumberSpin->get_value_as_int();
    _current_page = CLAMP(page, 1, _pdf_doc->getCatalog()->getNumPages());
    _setPreviewPage(_current_page);
}

#ifdef HAVE_POPPLER_CAIRO
void PdfImportDialog::_onToggleImport() {
    if( _importViaPoppler->get_active() ) {
        hbox3->set_sensitive(false);
        _localFontsCheck->set_sensitive(false);
        _embedImagesCheck->set_sensitive(false);
        hbox6->set_sensitive(false);
    } else {
        hbox3->set_sensitive();
        _localFontsCheck->set_sensitive();
        _embedImagesCheck->set_sensitive();
        hbox6->set_sensitive();
    }
}
#endif


#ifdef HAVE_POPPLER_CAIRO
/**
 * \brief Copies image data from a Cairo surface to a pixbuf
 *
 * Borrowed from libpoppler, from the file poppler-page.cc
 * Copyright (C) 2005, Red Hat, Inc.
 *
 */
static void copy_cairo_surface_to_pixbuf (cairo_surface_t *surface,
                                          unsigned char   *data,
                                          GdkPixbuf       *pixbuf)
{
    int cairo_width, cairo_height, cairo_rowstride;
    unsigned char *pixbuf_data, *dst, *cairo_data;
    int pixbuf_rowstride, pixbuf_n_channels;
    unsigned int *src;
    int x, y;

    cairo_width = cairo_image_surface_get_width (surface);
    cairo_height = cairo_image_surface_get_height (surface);
    cairo_rowstride = cairo_width * 4;
    cairo_data = data;

    pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
    pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

    if (cairo_width > gdk_pixbuf_get_width (pixbuf))
        cairo_width = gdk_pixbuf_get_width (pixbuf);
    if (cairo_height > gdk_pixbuf_get_height (pixbuf))
        cairo_height = gdk_pixbuf_get_height (pixbuf);
    for (y = 0; y < cairo_height; y++)
    {
        src = reinterpret_cast<unsigned int *>(cairo_data + y * cairo_rowstride);
        dst = pixbuf_data + y * pixbuf_rowstride;
        for (x = 0; x < cairo_width; x++)
        {
            dst[0] = (*src >> 16) & 0xff;
            dst[1] = (*src >> 8) & 0xff;
            dst[2] = (*src >> 0) & 0xff;
            if (pixbuf_n_channels == 4)
                dst[3] = (*src >> 24) & 0xff;
            dst += pixbuf_n_channels;
            src++;
        }
    }
}

#endif

/**
 * \brief Updates the preview area with the previously rendered thumbnail
 */
#if !WITH_GTKMM_3_0
bool PdfImportDialog::_onExposePreview(GdkEventExpose * /*event*/) {
    Cairo::RefPtr<Cairo::Context> cr = _previewArea->get_window()->create_cairo_context();
    return _onDraw(cr);
}
#endif

bool PdfImportDialog::_onDraw(const Cairo::RefPtr<Cairo::Context>& cr) {
    // Check if we have a thumbnail at all
    if (!_thumb_data) {
        return true;
    }

    // Create the pixbuf for the thumbnail
    Glib::RefPtr<Gdk::Pixbuf> thumb;

    if (_render_thumb) {
        thumb = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true,
                                    8, _thumb_width, _thumb_height);
    } else {
        thumb = Gdk::Pixbuf::create_from_data(_thumb_data, Gdk::COLORSPACE_RGB,
            false, 8, _thumb_width, _thumb_height, _thumb_rowstride);
    }
    if (!thumb) {
        return true;
    }

    // Set background to white
    if (_render_thumb) {
        thumb->fill(0xffffffff);
	Gdk::Cairo::set_source_pixbuf(cr, thumb, 0, 0);
	cr->paint();
    }
#ifdef HAVE_POPPLER_CAIRO
    // Copy the thumbnail image from the Cairo surface
    if (_render_thumb) {
        copy_cairo_surface_to_pixbuf(_cairo_surface, _thumb_data, thumb->gobj());
    }
#endif

    Gdk::Cairo::set_source_pixbuf(cr, thumb, 0, _render_thumb ? 0 : 20);
    cr->paint();
    return true;
}

/**
 * \brief Renders the given page's thumbnail using Cairo
 */
void PdfImportDialog::_setPreviewPage(int page) {

    _previewed_page = _pdf_doc->getCatalog()->getPage(page);
    // Try to get a thumbnail from the PDF if possible
    if (!_render_thumb) {
        if (_thumb_data) {
            gfree(_thumb_data);
            _thumb_data = NULL;
        }
        if (!_previewed_page->loadThumb(&_thumb_data,
             &_thumb_width, &_thumb_height, &_thumb_rowstride)) {
            return;
        }
        // Redraw preview area
        _previewArea->set_size_request(_thumb_width, _thumb_height + 20);
        _previewArea->queue_draw();
        return;
    }
#ifdef HAVE_POPPLER_CAIRO
    // Get page size by accounting for rotation
    double width, height;
    int rotate = _previewed_page->getRotate();
    if ( rotate == 90 || rotate == 270 ) {
        height = _previewed_page->getCropWidth();
        width = _previewed_page->getCropHeight();
    } else {
        width = _previewed_page->getCropWidth();
        height = _previewed_page->getCropHeight();
    }
    // Calculate the needed scaling for the page
    double scale_x = (double)_preview_width / width;
    double scale_y = (double)_preview_height / height;
    double scale_factor = ( scale_x > scale_y ) ? scale_y : scale_x;
    // Create new Cairo surface
    _thumb_width = (int)ceil( width * scale_factor );
    _thumb_height = (int)ceil( height * scale_factor );
    _thumb_rowstride = _thumb_width * 4;
    if (_thumb_data) {
        delete _thumb_data;
    }
    _thumb_data = new unsigned char[ _thumb_rowstride * _thumb_height ];
    if (_cairo_surface) {
        cairo_surface_destroy(_cairo_surface);
    }
    _cairo_surface = cairo_image_surface_create_for_data(_thumb_data,
            CAIRO_FORMAT_ARGB32, _thumb_width, _thumb_height, _thumb_rowstride);
    cairo_t *cr = cairo_create(_cairo_surface);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);  // Set fill color to white
    cairo_paint(cr);    // Clear it
    cairo_scale(cr, scale_factor, scale_factor);    // Use Cairo for resizing the image
    // Render page
    if (_poppler_doc != NULL) {
        PopplerPage *poppler_page = poppler_document_get_page(_poppler_doc, page-1);
        poppler_page_render(poppler_page, cr);
        g_object_unref(G_OBJECT(poppler_page));
    }
    // Clean up
    cairo_destroy(cr);
    // Redraw preview area
    _previewArea->set_size_request(_preview_width, _preview_height);
    _previewArea->queue_draw();
#endif
}

////////////////////////////////////////////////////////////////////////////////

bool
PdfInput::wasCancelled () {
    return _cancelled;
}

#ifdef HAVE_POPPLER_CAIRO
/// helper method
static cairo_status_t
        _write_ustring_cb(void *closure, const unsigned char *data, unsigned int length)
{
    Glib::ustring* stream = static_cast<Glib::ustring*>(closure);
    stream->append(reinterpret_cast<const char*>(data), length);

    return CAIRO_STATUS_SUCCESS;
}
#endif

/**
 * Parses the selected page of the given PDF document using PdfParser.
 */
SPDocument *
PdfInput::open(::Inkscape::Extension::Input * /*mod*/, const gchar * uri) {

    _cancelled = false;

    // Initialize the globalParams variable for poppler
    if (!globalParams) {
#ifdef ENABLE_OSX_APP_LOCATIONS
        //
        // data files for poppler are not relocatable (loaded from 
        // path defined at build time). This fails to work with relocatable
        // application bundles for OS X. 
        //
        // Workaround: 
        // 1. define $POPPLER_DATADIR env variable in app launcher script
        // 2. pass custom $POPPLER_DATADIR via poppler's GlobalParams()
        //
        // relevant poppler commit:
        // <http://cgit.freedesktop.org/poppler/poppler/commit/?id=869584a84eed507775ff1c3183fe484c14b6f77b>
        //
        // FIXES: Inkscape bug #956282, #1264793
        // TODO: report RFE upstream (full relocation support for OS X packaging)
        //
        gchar const *poppler_datadir = g_getenv("POPPLER_DATADIR");
        if (poppler_datadir != NULL) {
            globalParams = new GlobalParams(poppler_datadir);
        } else {
            globalParams = new GlobalParams();
        }
#else
        globalParams = new GlobalParams();
#endif // ENABLE_OSX_APP_LOCATIONS
    }


    // PDFDoc is from poppler. PDFDoc is used for preview and for native import.

#ifndef WIN32
    // poppler does not use glib g_open. So on win32 we must use unicode call. code was copied from
    // glib gstdio.c
    GooString *filename_goo = new GooString(uri);
    PDFDoc *pdf_doc = new PDFDoc(filename_goo, NULL, NULL, NULL);   // TODO: Could ask for password
    //delete filename_goo;
#else
    wchar_t *wfilename = reinterpret_cast<wchar_t*>(g_utf8_to_utf16 (uri, -1, NULL, NULL, NULL));

    if (wfilename == NULL) {
      return NULL;
    }

    PDFDoc *pdf_doc = new PDFDoc(wfilename, wcslen(wfilename), NULL, NULL, NULL);   // TODO: Could ask for password
    g_free (wfilename);
#endif

    if (!pdf_doc->isOk()) {
        int error = pdf_doc->getErrorCode();
        delete pdf_doc;
        if (error == errEncrypted) {
            g_message("Document is encrypted.");
        } else if (error == errOpenFile) {
            g_message("couldn't open the PDF file.");
        } else if (error == errBadCatalog) {
            g_message("couldn't read the page catalog.");
        } else if (error == errDamaged) {
            g_message("PDF file was damaged and couldn't be repaired.");
        } else if (error == errHighlightFile) {
            g_message("nonexistent or invalid highlight file.");
        } else if (error == errBadPrinter) {
            g_message("invalid printer.");
        } else if (error == errPrinting) {
            g_message("Error during printing.");
        } else if (error == errPermission) {
            g_message("PDF file does not allow that operation.");
        } else if (error == errBadPageNum) {
            g_message("invalid page number.");
        } else if (error == errFileIO) {
            g_message("file IO error.");
        } else {
            g_message("Failed to load document from data (error %d)", error);
        }

        return NULL;
    }

    PdfImportDialog *dlg = NULL;
    if (INKSCAPE.use_gui()) {
        dlg = new PdfImportDialog(pdf_doc, uri);
        if (!dlg->showDialog()) {
            _cancelled = true;
            delete dlg;
            delete pdf_doc;
            return NULL;
        }
    }

    // Get options
    int page_num = 1;
    bool is_importvia_poppler = false;
    if (dlg) {
        page_num = dlg->getSelectedPage();
#ifdef HAVE_POPPLER_CAIRO
        is_importvia_poppler = dlg->getImportMethod();
        // printf("PDF import via %s.\n", is_importvia_poppler ? "poppler" : "native");
#endif
    }

    SPDocument *doc = NULL;
    bool saved = false;
    if(!is_importvia_poppler)
    {
        // native importer
        doc = SPDocument::createNewDoc(NULL, TRUE, TRUE);
        saved = DocumentUndo::getUndoSensitive(doc);
        DocumentUndo::setUndoSensitive(doc, false); // No need to undo in this temporary document

        // Create builder
        gchar *docname = g_path_get_basename(uri);
        gchar *dot = g_strrstr(docname, ".");
        if (dot) {
            *dot = 0;
        }
        SvgBuilder *builder = new SvgBuilder(doc, docname, pdf_doc->getXRef());

        // Get preferences
        Inkscape::XML::Node *prefs = builder->getPreferences();
        if (dlg)
            dlg->getImportSettings(prefs);

        // Apply crop settings
        PDFRectangle *clipToBox = NULL;
        double crop_setting;
        sp_repr_get_double(prefs, "cropTo", &crop_setting);

        Catalog *catalog = pdf_doc->getCatalog();
        Page *page = catalog->getPage(page_num);

        if ( crop_setting >= 0.0 ) {    // Do page clipping
            int crop_choice = (int)crop_setting;
            switch (crop_choice) {
                case 0: // Media box
                    clipToBox = page->getMediaBox();
                    break;
                case 1: // Crop box
                    clipToBox = page->getCropBox();
                    break;
                case 2: // Bleed box
                    clipToBox = page->getBleedBox();
                    break;
                case 3: // Trim box
                    clipToBox = page->getTrimBox();
                    break;
                case 4: // Art box
                    clipToBox = page->getArtBox();
                    break;
                default:
                    break;
            }
        }

        // Create parser  (extension/internal/pdfinput/pdf-parser.h)
        PdfParser *pdf_parser = new PdfParser(pdf_doc->getXRef(), builder, page_num-1, page->getRotate(),
                                              page->getResourceDict(), page->getCropBox(), clipToBox);

        // Set up approximation precision for parser. Used for convering Mesh Gradients into tiles.
        double color_delta;
        sp_repr_get_double(prefs, "approximationPrecision", &color_delta);
        if ( color_delta <= 0.0 ) {
            color_delta = 1.0 / 2.0;
        } else {
            color_delta = 1.0 / color_delta;
        }
        for ( int i = 1 ; i <= pdfNumShadingTypes ; i++ ) {
            pdf_parser->setApproximationPrecision(i, color_delta, 6);
        }

        // Parse the document structure
        Object obj;
        page->getContents(&obj);
        if (!obj.isNull()) {
            pdf_parser->parse(&obj);
        }

        // Cleanup
        obj.free();
        delete pdf_parser;
        delete builder;
        g_free(docname);
    }
    else
    {
#ifdef HAVE_POPPLER_CAIRO
        // the poppler import

        Glib::ustring full_path = uri;
        if (!Glib::path_is_absolute(uri)) {
            full_path = Glib::build_filename(Glib::get_current_dir(),uri);
        }
        Glib::ustring full_uri = Glib::filename_to_uri(full_path);

        GError *error = NULL;
        /// @todo handle password
        /// @todo check if win32 unicode needs special attention
        PopplerDocument* document = poppler_document_new_from_file(full_uri.c_str(), NULL, &error);

        if(error != NULL) {
            std::cerr << "PDFInput::open: error opening document: " << full_uri << std::endl;
            g_error_free (error);
        }

        if (document != NULL)
        {
            double width, height;
            PopplerPage* page = poppler_document_get_page(document, page_num - 1);
            poppler_page_get_size(page, &width, &height);

            Glib::ustring output;
            cairo_surface_t* surface = cairo_svg_surface_create_for_stream(Inkscape::Extension::Internal::_write_ustring_cb,
                                                                           &output, width, height);

            // This magical function results in more fine-grain fallbacks. In particular, a mesh
            // gradient won't necessarily result in the whole PDF being rasterized. Of course, SVG
            // 1.2 never made it as a standard, but hey, we'll take what we can get. This trick was
            // found by examining the 'pdftocairo' code.
            cairo_svg_surface_restrict_to_version( surface, CAIRO_SVG_VERSION_1_2 );

            cairo_t* cr = cairo_create(surface);

            poppler_page_render_for_printing(page, cr);
            cairo_show_page(cr);

            cairo_destroy(cr);
            cairo_surface_destroy(surface);

            doc = SPDocument::createNewDocFromMem(output.c_str(), output.length(), TRUE);
            
            // Cleanup
            // delete output;
            g_object_unref(G_OBJECT(page));
            g_object_unref(G_OBJECT(document));
        }
        else
        {
            doc = SPDocument::createNewDoc(NULL, TRUE, TRUE);   // fallback create empty document
        }
        saved = DocumentUndo::getUndoSensitive(doc);
        DocumentUndo::setUndoSensitive(doc, false); // No need to undo in this temporary document
#endif
    }

    // Cleanup
    delete pdf_doc;
    delete dlg;

    // Set viewBox if it doesn't exist
    if (!doc->getRoot()->viewBox_set) {
        doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDisplayUnit()), doc->getHeight().value(doc->getDisplayUnit())));
    }

    // Restore undo
    DocumentUndo::setUndoSensitive(doc, saved);

    return doc;
}

#include "../clear-n_.h"

void PdfInput::init(void) {
    /* PDF in */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("PDF Input") "</name>\n"
            "<id>org.inkscape.input.pdf</id>\n"
            "<input>\n"
                "<extension>.pdf</extension>\n"
                "<mimetype>application/pdf</mimetype>\n"
                "<filetypename>" N_("Adobe PDF (*.pdf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Adobe Portable Document Format") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new PdfInput());

    /* AI in */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("AI Input") "</name>\n"
            "<id>org.inkscape.input.ai</id>\n"
            "<input>\n"
                "<extension>.ai</extension>\n"
                "<mimetype>image/x-adobe-illustrator</mimetype>\n"
                "<filetypename>" N_("Adobe Illustrator 9.0 and above (*.ai)") "</filetypename>\n"
                "<filetypetooltip>" N_("Open files saved in Adobe Illustrator 9.0 and newer versions") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new PdfInput());
} // init

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

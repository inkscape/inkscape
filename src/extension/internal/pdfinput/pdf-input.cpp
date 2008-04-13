 /** \file
 * Native PDF import using libpoppler.
 *
 * Authors:
 *   miklos erdelyi
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER

#include "goo/GooString.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "Page.h"
#include "Catalog.h"

#ifdef HAVE_POPPLER_CAIRO
#include <poppler/glib/poppler.h>
#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>
#endif

#include "pdf-input.h"
#include "extension/system.h"
#include "extension/input.h"
#include "svg-builder.h"
#include "pdf-parser.h"

#include "document-private.h"

#include "dialogs/dialog-events.h"
#include <gtk/gtkdialog.h>

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

PdfImportDialog::PdfImportDialog(PDFDoc *doc, const gchar *uri)
{
#ifdef HAVE_POPPLER_CAIRO
    _poppler_doc = NULL;
#endif // HAVE_POPPLER_CAIRO
    _pdf_doc = doc;

    cancelbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-cancel")));
    okbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-ok")));
    _labelSelect = Gtk::manage(new class Gtk::Label(_("Select page:")));

    // Page number
    Gtk::Adjustment *_pageNumberSpin_adj = Gtk::manage(
            new class Gtk::Adjustment(1, 1, _pdf_doc->getNumPages(), 1, 10, 0));
    _pageNumberSpin = Gtk::manage(new class Gtk::SpinButton(*_pageNumberSpin_adj, 1, 1));
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
        _cropTypeCombo->append_text(_(crop_setting_choices[i]));
    }
    _cropTypeCombo->set_active_text(_(crop_setting_choices[0]));
    _cropTypeCombo->set_sensitive(false);

    hbox3 = Gtk::manage(new class Gtk::HBox(false, 4));
    vbox2 = Gtk::manage(new class Gtk::VBox(false, 4));
    alignment3 = Gtk::manage(new class Gtk::Alignment(0.5, 0.5, 1, 1));
    _labelPageSettings = Gtk::manage(new class Gtk::Label(_("Page settings")));
    _pageSettingsFrame = Gtk::manage(new class Gtk::Frame());
    _labelPrecision = Gtk::manage(new class Gtk::Label(_("Precision of approximating gradient meshes:")));
    _labelPrecisionWarning = Gtk::manage(new class Gtk::Label(_("<b>Note</b>: setting the precision too high may result in a large SVG file and slow performance.")));

    _fallbackPrecisionSlider_adj = Gtk::manage(new class Gtk::Adjustment(2, 1, 256, 1, 10, 10));
    _fallbackPrecisionSlider = Gtk::manage(new class Gtk::HScale(*_fallbackPrecisionSlider_adj));
    _fallbackPrecisionSlider->set_value(2.0);
    _labelPrecisionComment = Gtk::manage(new class Gtk::Label(_("rough")));
    hbox6 = Gtk::manage(new class Gtk::HBox(false, 4));

    // Text options
    _labelText = Gtk::manage(new class Gtk::Label(_("Text handling:")));
    _textHandlingCombo = Gtk::manage(new class Gtk::ComboBoxText());
    _textHandlingCombo->append_text(_("Import text as text"));
    _textHandlingCombo->set_active_text(_("Import text as text"));

    hbox5 = Gtk::manage(new class Gtk::HBox(false, 4));
    _embedImagesCheck = Gtk::manage(new class Gtk::CheckButton(_("Embed images")));
    vbox3 = Gtk::manage(new class Gtk::VBox(false, 4));
    alignment4 = Gtk::manage(new class Gtk::Alignment(0.5, 0.5, 1, 1));
    _labelImportSettings = Gtk::manage(new class Gtk::Label(_("Import settings")));
    _importSettingsFrame = Gtk::manage(new class Gtk::Frame());
    vbox1 = Gtk::manage(new class Gtk::VBox(false, 4));
    _previewArea = Gtk::manage(new class Gtk::DrawingArea());
    hbox1 = Gtk::manage(new class Gtk::HBox(false, 4));
    cancelbutton->set_flags(Gtk::CAN_FOCUS);
    cancelbutton->set_flags(Gtk::CAN_DEFAULT);
    cancelbutton->set_relief(Gtk::RELIEF_NORMAL);
    okbutton->set_flags(Gtk::CAN_FOCUS);
    okbutton->set_flags(Gtk::CAN_DEFAULT);
    okbutton->set_relief(Gtk::RELIEF_NORMAL);
    this->get_action_area()->property_layout_style().set_value(Gtk::BUTTONBOX_END);
    _labelSelect->set_alignment(0.5,0.5);
    _labelSelect->set_padding(4,0);
    _labelSelect->set_justify(Gtk::JUSTIFY_LEFT);
    _labelSelect->set_line_wrap(false);
    _labelSelect->set_use_markup(false);
    _labelSelect->set_selectable(false);
    _pageNumberSpin->set_flags(Gtk::CAN_FOCUS);
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
    _cropCheck->set_flags(Gtk::CAN_FOCUS);
    _cropCheck->set_relief(Gtk::RELIEF_NORMAL);
    _cropCheck->set_mode(true);
    _cropCheck->set_active(false);
    _cropTypeCombo->set_border_width(1);
    hbox3->pack_start(*_cropCheck, Gtk::PACK_SHRINK, 4);
    hbox3->pack_start(*_cropTypeCombo, Gtk::PACK_SHRINK, 0);
    vbox2->pack_start(*hbox2);
    vbox2->pack_start(*hbox3);
    alignment3->add(*vbox2);
    _labelPageSettings->set_alignment(0.5,0.5);
    _labelPageSettings->set_padding(4,0);
    _labelPageSettings->set_justify(Gtk::JUSTIFY_LEFT);
    _labelPageSettings->set_line_wrap(false);
    _labelPageSettings->set_use_markup(true);
    _labelPageSettings->set_selectable(false);
    _pageSettingsFrame->set_border_width(4);
    _pageSettingsFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    _pageSettingsFrame->set_label_align(0,0.5);
    _pageSettingsFrame->add(*alignment3);
    _pageSettingsFrame->set_label_widget(*_labelPageSettings);
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
    _fallbackPrecisionSlider->set_size_request(180,-1);
    _fallbackPrecisionSlider->set_flags(Gtk::CAN_FOCUS);
    _fallbackPrecisionSlider->set_update_policy(Gtk::UPDATE_CONTINUOUS);
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
    _labelText->set_alignment(0.5,0.5);
    _labelText->set_padding(4,0);
    _labelText->set_justify(Gtk::JUSTIFY_LEFT);
    _labelText->set_line_wrap(false);
    _labelText->set_use_markup(false);
    _labelText->set_selectable(false);
    hbox5->pack_start(*_labelText, Gtk::PACK_SHRINK, 0);
    hbox5->pack_start(*_textHandlingCombo, Gtk::PACK_SHRINK, 0);
    _embedImagesCheck->set_flags(Gtk::CAN_FOCUS);
    _embedImagesCheck->set_relief(Gtk::RELIEF_NORMAL);
    _embedImagesCheck->set_mode(true);
    _embedImagesCheck->set_active(true);
    vbox3->pack_start(*_labelPrecision, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*hbox6, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_labelPrecisionWarning, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*hbox5, Gtk::PACK_SHRINK, 4);
    vbox3->pack_start(*_embedImagesCheck, Gtk::PACK_SHRINK, 0);
    alignment4->add(*vbox3);
    _labelImportSettings->set_alignment(0.5,0.5);
    _labelImportSettings->set_padding(4,0);
    _labelImportSettings->set_justify(Gtk::JUSTIFY_LEFT);
    _labelImportSettings->set_line_wrap(false);
    _labelImportSettings->set_use_markup(true);
    _labelImportSettings->set_selectable(false);
    _importSettingsFrame->set_border_width(4);
    _importSettingsFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    _importSettingsFrame->set_label_align(0,0.5);
    _importSettingsFrame->add(*alignment4);
    _importSettingsFrame->set_label_widget(*_labelImportSettings);
    vbox1->pack_start(*_pageSettingsFrame, Gtk::PACK_EXPAND_PADDING, 0);
    vbox1->pack_start(*_importSettingsFrame, Gtk::PACK_EXPAND_PADDING, 0);
    hbox1->pack_start(*vbox1);
    hbox1->pack_start(*_previewArea, Gtk::PACK_EXPAND_WIDGET, 4);
    this->get_vbox()->set_homogeneous(false);
    this->get_vbox()->set_spacing(0);
    this->get_vbox()->pack_start(*hbox1);
    this->set_title(_("PDF Import Settings"));
    this->set_modal(true);
    sp_transientize((GtkWidget *)this->gobj());  //Make transient
    this->property_window_position().set_value(Gtk::WIN_POS_NONE);
    this->set_resizable(true);
    this->property_destroy_with_parent().set_value(false);
    this->set_has_separator(true);
    this->add_action_widget(*cancelbutton, -6);
    this->add_action_widget(*okbutton, -5);
    cancelbutton->show();
    okbutton->show();
    _labelSelect->show();
    _pageNumberSpin->show();
    _labelTotalPages->show();
    hbox2->show();
    _cropCheck->show();
    _cropTypeCombo->show();
    hbox3->show();
    vbox2->show();
    alignment3->show();
    _labelPageSettings->show();
    _pageSettingsFrame->show();
    _labelPrecision->show();
    _labelPrecisionWarning->show();
    _fallbackPrecisionSlider->show();
    _labelPrecisionComment->show();
    hbox6->show();
    _labelText->show();
    _textHandlingCombo->show();
    hbox5->show();
    _embedImagesCheck->show();
    vbox3->show();
    alignment4->show();
    _labelImportSettings->show();
    _importSettingsFrame->show();
    vbox1->show();
    _previewArea->show();
    hbox1->show();

    // Connect signals
    _previewArea->signal_expose_event().connect(sigc::mem_fun(*this, &PdfImportDialog::_onExposePreview));
    _pageNumberSpin_adj->signal_value_changed().connect(sigc::mem_fun(*this, &PdfImportDialog::_onPageNumberChanged));
    _cropCheck->signal_toggled().connect(sigc::mem_fun(*this, &PdfImportDialog::_onToggleCropping));
    _fallbackPrecisionSlider_adj->signal_value_changed().connect(sigc::mem_fun(*this, &PdfImportDialog::_onPrecisionChanged));

    _render_thumb = false;
#ifdef HAVE_POPPLER_CAIRO
    _cairo_surface = NULL;
    _render_thumb = true;
    // Create PopplerDocument
    gchar *doc_uri = g_filename_to_uri(uri, NULL, NULL);
    if (doc_uri) {
        _poppler_doc = poppler_document_new_from_file(doc_uri, NULL, NULL);
        g_free(doc_uri);
    }
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
    if (_embedImagesCheck->get_active()) {
        prefs->setAttribute("embedImages", "1");
    } else {
        prefs->setAttribute("embedImages", "0");
    }
}

/**
 * \brief Redisplay the comment on the current approximation precision setting
 * Evenly divides the interval of possible values between the available labels.
 */
void PdfImportDialog::_onPrecisionChanged() {

    static Glib::ustring precision_comments[] = {
        Glib::ustring(_("rough")),
        Glib::ustring(Q_("pdfinput|medium")),
        Glib::ustring(_("fine")),
        Glib::ustring(_("very fine"))
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
        src = (unsigned int *) (cairo_data + y * cairo_rowstride);
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
bool PdfImportDialog::_onExposePreview(GdkEventExpose */*event*/) {

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
        Glib::RefPtr<Gdk::Pixmap> back_pixmap = Gdk::Pixmap::create(
                _previewArea->get_window(), _thumb_width, _thumb_height, -1);
        if (!back_pixmap) {
            return true;
        }
        back_pixmap->draw_pixbuf(Glib::RefPtr<Gdk::GC>(), thumb, 0, 0, 0, 0,
                                 _thumb_width, _thumb_height,
                                 Gdk::RGB_DITHER_NONE, 0, 0);
        _previewArea->get_window()->set_back_pixmap(back_pixmap, false);
        _previewArea->get_window()->clear();
    }
#ifdef HAVE_POPPLER_CAIRO
    // Copy the thumbnail image from the Cairo surface
    if (_render_thumb) {
        copy_cairo_surface_to_pixbuf(_cairo_surface, _thumb_data, thumb->gobj());
    }
#endif
    _previewArea->get_window()->draw_pixbuf(Glib::RefPtr<Gdk::GC>(), thumb,
                                            0, 0, 0, _render_thumb ? 0 : 20,
                                            -1, -1, Gdk::RGB_DITHER_NONE, 0, 0);

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

/**
 * Parses the selected page of the given PDF document using PdfParser.
 */
SPDocument *
PdfInput::open(::Inkscape::Extension::Input * /*mod*/, const gchar * uri) {

    // Initialize the globalParams variable for poppler
    if (!globalParams) {
        globalParams = new GlobalParams();
    }
    // poppler does not use glib g_open. So on win32 we must use unicode call. code was copied from glib gstdio.c
#ifndef WIN32
    GooString *filename_goo = new GooString(uri);
    PDFDoc *pdf_doc = new PDFDoc(filename_goo, NULL, NULL, NULL);   // TODO: Could ask for password
    //delete filename_goo;
#else
    wchar_t *wfilename = (wchar_t*)g_utf8_to_utf16 (uri, -1, NULL, NULL, NULL);

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
    PdfImportDialog *dlg = new PdfImportDialog(pdf_doc, uri);
    if (!dlg->showDialog()) {
        delete dlg;
        delete pdf_doc;

        return NULL;
    }

    // Get needed page
    int page_num = dlg->getSelectedPage();
    Catalog *catalog = pdf_doc->getCatalog();
    Page *page = catalog->getPage(page_num);

    SPDocument *doc = sp_document_new(NULL, TRUE, TRUE);
    bool saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, false); // No need to undo in this temporary document

    // Create builder
    gchar *docname = g_path_get_basename(uri);
    gchar *dot = g_strrstr(docname, ".");
    if (dot) {
        *dot = 0;
    }
    SvgBuilder *builder = new SvgBuilder(doc, docname, pdf_doc->getXRef());

    // Get preferences
    Inkscape::XML::Node *prefs = builder->getPreferences();
    dlg->getImportSettings(prefs);

    // Apply crop settings
    PDFRectangle *clipToBox = NULL;
    double crop_setting;
    sp_repr_get_double(prefs, "cropTo", &crop_setting);
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

    // Create parser
    PdfParser *pdf_parser = new PdfParser(pdf_doc->getXRef(), builder, page_num-1, page->getRotate(),
                                          page->getResourceDict(), page->getCropBox(), clipToBox);

    // Set up approximation precision for parser
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
    delete pdf_doc;

    // Restore undo
    sp_document_set_undo_sensitive(doc, saved);

    return doc;
}

#include "../clear-n_.h"

void
PdfInput::init(void) {
    Inkscape::Extension::Extension * ext;

    /* PDF in */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
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
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
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

 /*
 * Simple PDF import extension using libpoppler and Cairo's SVG surface.
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

#ifdef HAVE_POPPLER_GLIB
#ifdef HAVE_POPPLER_CAIRO

#include "pdf-input-cairo.h"
#include "extension/system.h"
#include "extension/input.h"
#include "dialogs/dialog-events.h"
#include "document.h"
#include "sp-root.h"
#include "util/units.h"

#include <2geom/rect.h>

#include "inkscape.h"

#include <cairo-svg.h>
#include <poppler/glib/poppler.h>
#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>

#include "ui/widget/spinbutton.h"
#include "ui/widget/frame.h"

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

PdfImportCairoDialog::PdfImportCairoDialog(PopplerDocument *doc)
{
    if(doc == NULL) {
        // if there is no document, throw exception here
        throw;
    }

    _poppler_doc = doc;

    cancelbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-cancel")));
    okbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-ok")));
    _labelSelect = Gtk::manage(new class Gtk::Label(_("Select page:")));

    // Page number
    int num_pages = poppler_document_get_n_pages(_poppler_doc);
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> _pageNumberSpin_adj( Gtk::Adjustment::create(1, 1, num_pages, 1, 10, 0) );
    _pageNumberSpin = Gtk::manage(new class Inkscape::UI::Widget::SpinButton(_pageNumberSpin_adj, 1, 1));
#else
    Gtk::Adjustment *_pageNumberSpin_adj = Gtk::manage(new class Gtk::Adjustment(1, 1, num_pages, 1, 10, 0));
    _pageNumberSpin = Gtk::manage(new class Inkscape::UI::Widget::SpinButton(*_pageNumberSpin_adj, 1, 1));
#endif
    _labelTotalPages = Gtk::manage(new class Gtk::Label());
    hbox2 = Gtk::manage(new class Gtk::HBox(false, 0));
    // Disable the page selector when there's only one page
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

#if WITH_GTKMM_3_0
    _fallbackPrecisionSlider_adj = Gtk::Adjustment::create(2, 1, 256, 1, 10, 10);
    _fallbackPrecisionSlider = Gtk::manage(new Gtk::Scale(_fallbackPrecisionSlider_adj));
#else
    _fallbackPrecisionSlider_adj = Gtk::manage(new class Gtk::Adjustment(2, 1, 256, 1, 10, 10));
    _fallbackPrecisionSlider = Gtk::manage(new class Gtk::HScale(*_fallbackPrecisionSlider_adj));
#endif
    _fallbackPrecisionSlider->set_value(2.0);
    _labelPrecisionComment = Gtk::manage(new class Gtk::Label(_("rough")));
    hbox6 = Gtk::manage(new class Gtk::HBox(false, 4));

    // Text options
    _labelText = Gtk::manage(new class Gtk::Label(_("Text handling:")));
    _textHandlingCombo = Gtk::manage(new class Gtk::ComboBoxText());
    _textHandlingCombo->append(_("Import text as text"));
    _textHandlingCombo->set_active_text(_("Import text as text"));
    _localFontsCheck = Gtk::manage(new class Gtk::CheckButton(_("Replace PDF fonts by closest-named installed fonts")));

    hbox5 = Gtk::manage(new class Gtk::HBox(false, 4));
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
    _labelText->set_alignment(0.5,0.5);
    _labelText->set_padding(4,0);
    _labelText->set_justify(Gtk::JUSTIFY_LEFT);
    _labelText->set_line_wrap(false);
    _labelText->set_use_markup(false);
    _labelText->set_selectable(false);
    hbox5->pack_start(*_labelText, Gtk::PACK_SHRINK, 0);
    hbox5->pack_start(*_textHandlingCombo, Gtk::PACK_SHRINK, 0);
    _localFontsCheck->set_can_focus();
    _localFontsCheck->set_relief(Gtk::RELIEF_NORMAL);
    _localFontsCheck->set_mode(true);
    _localFontsCheck->set_active(true);
    _embedImagesCheck->set_can_focus();
    _embedImagesCheck->set_relief(Gtk::RELIEF_NORMAL);
    _embedImagesCheck->set_mode(true);
    _embedImagesCheck->set_active(true);
    vbox3->pack_start(*_labelPrecision, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*hbox6, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_labelPrecisionWarning, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*hbox5, Gtk::PACK_SHRINK, 4);
    vbox3->pack_start(*_localFontsCheck, Gtk::PACK_SHRINK, 0);
    vbox3->pack_start(*_embedImagesCheck, Gtk::PACK_SHRINK, 0);
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
    _pageSettingsFrame->show();
    _labelPrecision->show();
    _labelPrecisionWarning->show();
    _fallbackPrecisionSlider->show();
    _labelPrecisionComment->show();
    hbox6->show();
    _labelText->show();
    _textHandlingCombo->show();
    hbox5->show();
    _localFontsCheck->show();
    _embedImagesCheck->show();
    vbox3->show();
    _importSettingsFrame->show();
    vbox1->show();
    _previewArea->show();
    hbox1->show();

    // Connect signals
#if WITH_GTKMM_3_0
    _previewArea->signal_draw().connect(sigc::mem_fun(*this, &PdfImportCairoDialog::_onDraw));
#else
    _previewArea->signal_expose_event().connect(sigc::mem_fun(*this, &PdfImportCairoDialog::_onExposePreview));
#endif
    _pageNumberSpin_adj->signal_value_changed().connect(sigc::mem_fun(*this, &PdfImportCairoDialog::_onPageNumberChanged));
    _cropCheck->signal_toggled().connect(sigc::mem_fun(*this, &PdfImportCairoDialog::_onToggleCropping));
    _fallbackPrecisionSlider_adj->signal_value_changed().connect(sigc::mem_fun(*this, &PdfImportCairoDialog::_onPrecisionChanged));

    _render_thumb = false;
    _cairo_surface = NULL;
    _render_thumb = true;

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

PdfImportCairoDialog::~PdfImportCairoDialog() {
    if (_cairo_surface) {
        cairo_surface_destroy(_cairo_surface);
    }
    if (_thumb_data) {
        if (_render_thumb) {
            delete _thumb_data;
        } else {
            // -->gfree(_thumb_data);
            delete _thumb_data;
        }
    }
}

bool PdfImportCairoDialog::showDialog() {
    show();
    gint b = run();
    hide();
    if ( b == Gtk::RESPONSE_OK ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int PdfImportCairoDialog::getSelectedPage() {
    return _current_page;
}

/**
 * \brief Retrieves the current settings into a repr which SvgBuilder will use
 *        for determining the behaviour desired by the user
 */
void PdfImportCairoDialog::getImportSettings(Inkscape::XML::Node *prefs) {
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
}

/**
 * \brief Redisplay the comment on the current approximation precision setting
 * Evenly divides the interval of possible values between the available labels.
 */
void PdfImportCairoDialog::_onPrecisionChanged() {

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

void PdfImportCairoDialog::_onToggleCropping() {
    _cropTypeCombo->set_sensitive(_cropCheck->get_active());
}

void PdfImportCairoDialog::_onPageNumberChanged() {
    int page = _pageNumberSpin->get_value_as_int();
    _current_page = CLAMP(page, 1, poppler_document_get_n_pages(_poppler_doc));
    _setPreviewPage(_current_page);
}

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

/**
 * \brief Updates the preview area with the previously rendered thumbnail
 */
#if !WITH_GTKMM_3_0
bool PdfImportCairoDialog::_onExposePreview(GdkEventExpose * /*event*/) {
    Cairo::RefPtr<Cairo::Context> cr = _previewArea->get_window()->create_cairo_context();
    return _onDraw(cr);
}
#endif


bool PdfImportCairoDialog::_onDraw(const Cairo::RefPtr<Cairo::Context>& cr) {	
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

    // Copy the thumbnail image from the Cairo surface
    if (_render_thumb) {
        copy_cairo_surface_to_pixbuf(_cairo_surface, _thumb_data, thumb->gobj());
    }
    Gdk::Cairo::set_source_pixbuf(cr, thumb, 0, _render_thumb ? 0 : 20);
    cr->paint();

    return true;
}

/**
 * \brief Renders the given page's thumbnail using Cairo
 */
void PdfImportCairoDialog::_setPreviewPage(int page) {

    PopplerPage *_previewed_page = poppler_document_get_page(_poppler_doc, page-1);

    // Try to get a thumbnail from the PDF if possible
    if (!_render_thumb) {
        if (_thumb_data) {
            // --> gfree(_thumb_data);
            free(_thumb_data);
            _thumb_data = NULL;
        }

/*
-->        if (!_previewed_page->loadThumb(&_thumb_data,
             &_thumb_width, &_thumb_height, &_thumb_rowstride)) {
            return;
        }
*/
        // Redraw preview area
        _previewArea->set_size_request(_thumb_width, _thumb_height + 20);
        _previewArea->queue_draw();
        return;
    }

    // Get page size by accounting for rotation
    double width, height;
    // --> int rotate = _previewed_page->getRotate();
    int rotate = 0;
    if ( rotate == 90 || rotate == 270 ) {
// -->        height = _previewed_page->getCropWidth();
// -->        width = _previewed_page->getCropHeight();
    } else {
        poppler_page_get_size (_previewed_page, &width, &height);
// -->       width = _previewed_page->getCropWidth();
// -->        height = _previewed_page->getCropHeight();
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
        PopplerPage *poppler_page = poppler_document_get_page(_poppler_doc, page - 1);
        poppler_page_render(poppler_page, cr);
        g_object_unref(G_OBJECT(poppler_page));
    }
    // Clean up
    cairo_destroy(cr);
    // Redraw preview area
    _previewArea->set_size_request(_preview_width, _preview_height);
    _previewArea->queue_draw();

}


static cairo_status_t _write_ustring_cb(void *closure, const unsigned char *data, unsigned int length);

SPDocument *
PdfInputCairo::open(Inkscape::Extension::Input * /*mod*/, const gchar * uri) {

    g_message("Attempting to open using PdfInputCairo\n");

    gchar* filename_uri = g_filename_to_uri(uri, NULL, NULL);

    GError *error = NULL;
    /// @todo handle passwort
    /// @todo check if win32 unicode needs special attention
    PopplerDocument* document = poppler_document_new_from_file(filename_uri, NULL, &error);

    if(error != NULL) {
        g_message("Unable to read file: %s\n", error->message);
        g_error_free (error);
    }

    if (document == NULL) {
        return NULL;
    }

    // create and show the import dialog
    PdfImportCairoDialog *dlg = NULL;
    if (inkscape_use_gui()) {
        dlg = new PdfImportCairoDialog(document);
        if (!dlg->showDialog()) {
            delete dlg;
            return NULL;
        }
    }

    // Get needed page
    int page_num;
    if (dlg) {
        page_num = dlg->getSelectedPage();
        delete dlg;
    }
    else
        page_num = 1;

    double width, height;
    PopplerPage* page = poppler_document_get_page(document, page_num - 1);
    poppler_page_get_size(page, &width, &height);

    Glib::ustring* output = new Glib::ustring("");
    cairo_surface_t* surface = cairo_svg_surface_create_for_stream(Inkscape::Extension::Internal::_write_ustring_cb,
                                                                   output, width, height);
    cairo_t* cr = cairo_create(surface);

    poppler_page_render_for_printing(page, cr);
    cairo_show_page(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    SPDocument * doc = SPDocument::createNewDocFromMem(output->c_str(), output->length(), TRUE);

    // Set viewBox if it doesn't exist
    if (doc && !doc->getRoot()->viewBox_set) {
        doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDefaultUnit()), doc->getHeight().value(doc->getDefaultUnit())));
    }

    delete output;
    g_object_unref(page);
    g_object_unref(document);

    return doc;
}

static cairo_status_t
        _write_ustring_cb(void *closure, const unsigned char *data, unsigned int length)
{
    Glib::ustring* stream = static_cast<Glib::ustring*>(closure);
    stream->append(reinterpret_cast<const char*>(data), length);

    return CAIRO_STATUS_SUCCESS;
}


#include "clear-n_.h"

void
PdfInputCairo::init(void) {
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
        "<name>" N_("PDF Input") "</name>\n"
            "<id>org.inkscape.input.cairo-pdf</id>\n"
            "<input>\n"
                "<extension>.pdf</extension>\n"
                "<mimetype>application/pdf</mimetype>\n"
                "<filetypename>" N_("Adobe PDF via poppler-cairo (*.pdf)") "</filetypename>\n"
                "<filetypetooltip>" N_("PDF Document") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new PdfInputCairo());
} // init

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER_CAIRO */
#endif /* HAVE_POPPLER_GLIB */

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

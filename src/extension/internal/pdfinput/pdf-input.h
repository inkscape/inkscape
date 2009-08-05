#ifndef __EXTENSION_INTERNAL_PDFINPUT_H__
#define __EXTENSION_INTERNAL_PDFINPUT_H__

 /** \file
 * PDF import using libpoppler.
 *
 * Authors:
 *   miklos erdelyi
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER

#include "../../implementation/implementation.h"

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>
#include <gtkmm/scale.h>
#include <glibmm/i18n.h>
#include <gdk/gdk.h>

#include "PDFDoc.h"
#ifdef HAVE_POPPLER_CAIRO
#include <poppler/glib/poppler-document.h>
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

class PdfImportDialog : public Gtk::Dialog
{
public:
    PdfImportDialog(PDFDoc *doc, const gchar *uri);
    virtual ~PdfImportDialog();

    bool showDialog();
    int getSelectedPage();
    void getImportSettings(Inkscape::XML::Node *prefs);

private:
    void _setPreviewPage(int page);

    // Signal handlers
    bool _onExposePreview(GdkEventExpose *event);
    void _onPageNumberChanged();
    void _onToggleCropping();
    void _onPrecisionChanged();

    class Gtk::Button * cancelbutton;
    class Gtk::Button * okbutton;
    class Gtk::Label * _labelSelect;
    class Gtk::SpinButton * _pageNumberSpin;
    class Gtk::Label * _labelTotalPages;
    class Gtk::HBox * hbox2;
    class Gtk::CheckButton * _cropCheck;
    class Gtk::ComboBoxText * _cropTypeCombo;
    class Gtk::HBox * hbox3;
    class Gtk::VBox * vbox2;
    class Gtk::Alignment * alignment3;
    class Gtk::Label * _labelPageSettings;
    class Gtk::Frame * _pageSettingsFrame;
    class Gtk::Label * _labelPrecision;
    class Gtk::Label * _labelPrecisionWarning;
    class Gtk::HScale * _fallbackPrecisionSlider;
    class Gtk::Adjustment *_fallbackPrecisionSlider_adj;
    class Gtk::Label * _labelPrecisionComment;
    class Gtk::HBox * hbox6;
    class Gtk::Label * _labelText;
    class Gtk::ComboBoxText * _textHandlingCombo;
    class Gtk::HBox * hbox5;
    class Gtk::CheckButton * _localFontsCheck;
    class Gtk::CheckButton * _embedImagesCheck;
    class Gtk::VBox * vbox3;
    class Gtk::Alignment * alignment4;
    class Gtk::Label * _labelImportSettings;
    class Gtk::Frame * _importSettingsFrame;
    class Gtk::VBox * vbox1;
    class Gtk::DrawingArea * _previewArea;
    class Gtk::HBox * hbox1;

    PDFDoc *_pdf_doc;   // Document to be imported
    int _current_page;  // Current selected page
    Page *_previewed_page;    // Currently previewed page
    unsigned char *_thumb_data; // Thumbnail image data
    int _thumb_width, _thumb_height;    // Thumbnail size
    int _thumb_rowstride;
    int _preview_width, _preview_height;    // Size of the preview area
    bool _render_thumb;     // Whether we can/shall render thumbnails
#ifdef HAVE_POPPLER_CAIRO
    cairo_surface_t *_cairo_surface;
    PopplerDocument *_poppler_doc;
#endif
};

    
class PdfInput: public Inkscape::Extension::Implementation::Implementation {
    PdfInput () { };
public:
    Document *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );
    static void         init( void );

};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER */

#endif /* __EXTENSION_INTERNAL_PDFINPUT_H__ */

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

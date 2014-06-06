#ifndef SEEN_EXTENSION_INTERNAL_PDFINPUT_H
#define SEEN_EXTENSION_INTERNAL_PDFINPUT_H

/*
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

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/dialog.h>

#include "../../implementation/implementation.h"

#ifdef HAVE_POPPLER_CAIRO
struct _PopplerDocument;
typedef struct _PopplerDocument            PopplerDocument;
#endif

struct _GdkEventExpose;
typedef _GdkEventExpose GdkEventExpose;

class Page;
class PDFDoc;

namespace Gtk {
  class Alignment;
  class Button;
  class CheckButton;
  class ComboBoxText;
  class DrawingArea;
  class Frame;
  class HBox;
#if WITH_GTKMM_3_0
  class Scale;
#else
  class HScale;
#endif
  class VBox;
  class Label;
}

namespace Inkscape {

namespace UI {
namespace Widget {
  class SpinButton;
  class Frame;
}
}

namespace Extension {
namespace Internal {

/**
 * PDF import using libpoppler.
 */
class PdfImportDialog : public Gtk::Dialog
{
public:
    PdfImportDialog(PDFDoc *doc, const gchar *uri);
    virtual ~PdfImportDialog();

    bool showDialog();
    int getSelectedPage();
    int getImportMethod();
    void getImportSettings(Inkscape::XML::Node *prefs);

private:
    void _setPreviewPage(int page);

    // Signal handlers
#if !WITH_GTKMM_3_0
    bool _onExposePreview(GdkEventExpose *event);
#endif

    bool _onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
    void _onPageNumberChanged();
    void _onToggleCropping();
    void _onPrecisionChanged();

    class Gtk::Button * cancelbutton;
    class Gtk::Button * okbutton;
    class Gtk::Label * _labelSelect;
    class Inkscape::UI::Widget::SpinButton * _pageNumberSpin;
    class Gtk::Label * _labelTotalPages;
    class Gtk::HBox * hbox2;
    class Gtk::CheckButton * _cropCheck;
    class Gtk::ComboBoxText * _cropTypeCombo;
    class Gtk::HBox * hbox3;
    class Gtk::VBox * vbox2;
    class Inkscape::UI::Widget::Frame * _pageSettingsFrame;
    class Gtk::Label * _labelPrecision;
    class Gtk::Label * _labelPrecisionWarning;
#ifdef HAVE_POPPLER_CAIRO
    class Gtk::CheckButton * _importviaPopplerCheck; // using poppler_cairo for importing
#endif
#if WITH_GTKMM_3_0
    class Gtk::Scale * _fallbackPrecisionSlider;
    Glib::RefPtr<Gtk::Adjustment> _fallbackPrecisionSlider_adj;
#else
    class Gtk::HScale * _fallbackPrecisionSlider;
    class Gtk::Adjustment *_fallbackPrecisionSlider_adj;
#endif
    class Gtk::Label * _labelPrecisionComment;
    class Gtk::HBox * hbox6;
    class Gtk::Label * _labelText;
    class Gtk::ComboBoxText * _textHandlingCombo;
    class Gtk::HBox * hbox5;
    class Gtk::CheckButton * _localFontsCheck;
    class Gtk::CheckButton * _embedImagesCheck;
    class Gtk::VBox * vbox3;
    class Inkscape::UI::Widget::Frame * _importSettingsFrame;
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
    SPDocument *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );
    static void         init( void );
    virtual bool wasCancelled();
private:
    bool _cancelled;
};

} // namespace Implementation
} // namespace Extension
} // namespace Inkscape

#endif // HAVE_POPPLER

#endif // SEEN_EXTENSION_INTERNAL_PDFINPUT_H

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

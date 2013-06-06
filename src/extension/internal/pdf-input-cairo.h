#ifndef __EXTENSION_INTERNAL_PDFINPUTCAIRO_H__
#define __EXTENSION_INTERNAL_PDFINPUTCAIRO_H__

/*
 * PDF input using libpoppler and Cairo's SVG surface.
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

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>
#include <gtkmm/scale.h>
#include <glibmm/i18n.h>
#include <gdk/gdk.h>

#ifdef HAVE_POPPLER_GLIB
#ifdef HAVE_POPPLER_CAIRO

#include <poppler/glib/poppler.h>

#include "../implementation/implementation.h"

namespace Gtk {
#if WITH_GTKMM_3_0
    class Scale;
#else
    class HScale;
#endif
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

class PdfImportCairoDialog : public Gtk::Dialog
{
public:
    PdfImportCairoDialog(PopplerDocument* doc);
    virtual ~PdfImportCairoDialog();

    bool showDialog();
    int getSelectedPage();
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

    PopplerDocument *_poppler_doc;
    // PopplerPage *_previewed_page;
    int _current_page;  // Current selected page
    unsigned char *_thumb_data; // Thumbnail image data
    int _thumb_width, _thumb_height;    // Thumbnail size
    int _thumb_rowstride;
    int _preview_width, _preview_height;    // Size of the preview area
    bool _render_thumb;     // Whether we can/shall render thumbnails
    cairo_surface_t *_cairo_surface;    // this cairo surface is used for preview
};


class PdfInputCairo: public Inkscape::Extension::Implementation::Implementation {
    PdfInputCairo () { };
public:
    SPDocument *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );
    static void         init( void );

};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER_CAIRO */
#endif /* HAVE_POPPLER_GLIB */

#endif /* __EXTENSION_INTERNAL_PDFINPUTCAIRO_H__ */

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

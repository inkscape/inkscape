/*
 *  This file came from libwpg as a source, their utility wpg2svg
 *  specifically.  It has been modified to work as an Inkscape extension.
 *  The Inkscape extension code is covered by this copyright, but the
 *  rest is covered by the one bellow.
 *
 * Authors:
 *   Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include <stdio.h>
#include "config.h"

#include "cdr-input.h"

#ifdef WITH_LIBCDR

#include <string>
#include <cstring>

#include <libcdr/libcdr.h>

// TODO: Drop this check when librevenge is widespread.
#if WITH_LIBCDR01
  #include <librevenge-stream/librevenge-stream.h>

  using librevenge::RVNGString;
  using librevenge::RVNGFileStream;
  using librevenge::RVNGStringVector;
#else
  #include <libwpd-stream/libwpd-stream.h>

  typedef WPXString               RVNGString;
  typedef WPXFileStream           RVNGFileStream;
  typedef libcdr::CDRStringVector RVNGStringVector;
#endif

#include <gtkmm/alignment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/frame.h>
#include <gtkmm/scale.h>

#include "extension/system.h"
#include "extension/input.h"
#include "document.h"

#include "document-private.h"
#include "document-undo.h"
#include "inkscape.h"

#include "ui/dialog-events.h"
#include <gtk/gtk.h>
#include "ui/widget/spinbutton.h"
#include "ui/widget/frame.h"
#include <glibmm/i18n.h>

#include <gdkmm/general.h>

#include "svg-view.h"
#include "svg-view-widget.h"

#include "util/units.h"

namespace Inkscape {
namespace Extension {
namespace Internal {


class CdrImportDialog : public Gtk::Dialog {
public:
     CdrImportDialog(const std::vector<RVNGString> &vec);
     virtual ~CdrImportDialog();

     bool showDialog();
     unsigned getSelectedPage();
     void getImportSettings(Inkscape::XML::Node *prefs);

private:
     void _setPreviewPage(unsigned page);

     // Signal handlers
#if !WITH_GTKMM_3_0
     bool _onExposePreview(GdkEventExpose *event);
#endif

     void _onPageNumberChanged();

     class Gtk::Button * cancelbutton;
     class Gtk::Button * okbutton;
     class Gtk::Label * _labelSelect;
     class Inkscape::UI::Widget::SpinButton * _pageNumberSpin;
     class Gtk::Label * _labelTotalPages;
     class Gtk::VBox * vbox1;
     class Gtk::VBox * vbox2;
     class Gtk::Widget * _previewArea;

     const std::vector<RVNGString> &_vec;   // Document to be imported
     unsigned _current_page;  // Current selected page
     int _preview_width, _preview_height;    // Size of the preview area
};

CdrImportDialog::CdrImportDialog(const std::vector<RVNGString> &vec)
     : _vec(vec), _current_page(1)
{
     int num_pages = _vec.size();
     if ( num_pages <= 1 )
          return;
     cancelbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-cancel")));
     okbutton = Gtk::manage(new class Gtk::Button(Gtk::StockID("gtk-ok")));
     _labelSelect = Gtk::manage(new class Gtk::Label(_("Select page:")));

     // Page number
#if WITH_GTKMM_3_0
     Glib::RefPtr<Gtk::Adjustment> _pageNumberSpin_adj = Gtk::Adjustment::create(1, 1, _vec.size(), 1, 10, 0);
     _pageNumberSpin = Gtk::manage(new Inkscape::UI::Widget::SpinButton(_pageNumberSpin_adj, 1, 1));
#else
     Gtk::Adjustment *_pageNumberSpin_adj = Gtk::manage(
               new class Gtk::Adjustment(1, 1, _vec.size(), 1, 10, 0));
     _pageNumberSpin = Gtk::manage(new class Inkscape::UI::Widget::SpinButton(*_pageNumberSpin_adj, 1, 1));
#endif
     _labelTotalPages = Gtk::manage(new class Gtk::Label());
     gchar *label_text = g_strdup_printf(_("out of %i"), num_pages);
     _labelTotalPages->set_label(label_text);
     g_free(label_text);

     vbox1 = Gtk::manage(new class Gtk::VBox(false, 4));
     SPDocument *doc = SPDocument::createNewDocFromMem(_vec[0].cstr(), strlen(_vec[0].cstr()), 0);
     _previewArea = Glib::wrap(sp_svg_view_widget_new(doc));

     vbox2 = Gtk::manage(new class Gtk::VBox(false, 4));
     cancelbutton->set_can_focus();
     cancelbutton->set_can_default();
     cancelbutton->set_relief(Gtk::RELIEF_NORMAL);
     okbutton->set_can_focus();
     okbutton->set_can_default();
     okbutton->set_relief(Gtk::RELIEF_NORMAL);
     this->get_action_area()->property_layout_style().set_value(Gtk::BUTTONBOX_END);
     _labelSelect->set_line_wrap(false);
     _labelSelect->set_use_markup(false);
     _labelSelect->set_selectable(false);
     _pageNumberSpin->set_can_focus();
     _pageNumberSpin->set_update_policy(Gtk::UPDATE_ALWAYS);
     _pageNumberSpin->set_numeric(true);
     _pageNumberSpin->set_digits(0);
     _pageNumberSpin->set_wrap(false);
     _labelTotalPages->set_line_wrap(false);
     _labelTotalPages->set_use_markup(false);
     _labelTotalPages->set_selectable(false);
     vbox2->pack_start(*_previewArea, Gtk::PACK_SHRINK, 0);
#if WITH_GTKMM_3_0
     this->get_content_area()->set_homogeneous(false);
     this->get_content_area()->set_spacing(0);
     this->get_content_area()->pack_start(*vbox2);
#else
     this->get_vbox()->set_homogeneous(false);
     this->get_vbox()->set_spacing(0);
     this->get_vbox()->pack_start(*vbox2);
#endif
     this->set_title(_("Page Selector"));
     this->set_modal(true);
     sp_transientize(GTK_WIDGET(this->gobj()));  //Make transient
     this->property_window_position().set_value(Gtk::WIN_POS_NONE);
     this->set_resizable(true);
     this->property_destroy_with_parent().set_value(false);
     this->get_action_area()->add(*_labelSelect);
     this->add_action_widget(*_pageNumberSpin, -7);
     this->get_action_area()->add(*_labelTotalPages);
     this->add_action_widget(*cancelbutton, -6);
     this->add_action_widget(*okbutton, -5);
     cancelbutton->show();
     okbutton->show();
     _labelSelect->show();
     _pageNumberSpin->show();
     _labelTotalPages->show();
     vbox1->show();
     _previewArea->show();
     vbox2->show();

     // Connect signals
     _pageNumberSpin_adj->signal_value_changed().connect(sigc::mem_fun(*this, &CdrImportDialog::_onPageNumberChanged));
}

CdrImportDialog::~CdrImportDialog() {}

bool CdrImportDialog::showDialog()
{
     show();
     gint b = run();
     hide();
     if ( b == Gtk::RESPONSE_OK ) {
          return TRUE;
     } else {
          return FALSE;
     }
}

unsigned CdrImportDialog::getSelectedPage()
{
     return _current_page;
}

void CdrImportDialog::_onPageNumberChanged()
{
     unsigned page = static_cast<unsigned>(_pageNumberSpin->get_value_as_int());
     _current_page = CLAMP(page, 1U, _vec.size());
     _setPreviewPage(_current_page);
}

/**
 * \brief Renders the given page's thumbnail
 */
void CdrImportDialog::_setPreviewPage(unsigned page)
{
     SPDocument *doc = SPDocument::createNewDocFromMem(_vec[page-1].cstr(), strlen(_vec[page-1].cstr()), 0);
     Gtk::Widget * tmpPreviewArea = Glib::wrap(sp_svg_view_widget_new(doc));
     std::swap(_previewArea, tmpPreviewArea);
     if (tmpPreviewArea) {
          _previewArea->set_size_request( tmpPreviewArea->get_width(), tmpPreviewArea->get_height() );
          delete tmpPreviewArea;
     }
     vbox2->pack_start(*_previewArea, Gtk::PACK_SHRINK, 0);
     _previewArea->show_now();
}

SPDocument *CdrInput::open(Inkscape::Extension::Input * /*mod*/, const gchar * uri)
{
     RVNGFileStream input(uri);

     if (!libcdr::CDRDocument::isSupported(&input)) {
          return NULL;
     }

     RVNGStringVector output;
#if WITH_LIBCDR01
     librevenge::RVNGSVGDrawingGenerator generator(output, "svg");

     if (!libcdr::CDRDocument::parse(&input, &generator)) {
#else
     if (!libcdr::CDRDocument::generateSVG(&input, output)) {
#endif
          return NULL;
     }

     if (output.empty()) {
          return NULL;
     }

     std::vector<RVNGString> tmpSVGOutput;
     for (unsigned i=0; i<output.size(); ++i) {
          RVNGString tmpString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
          tmpString.append(output[i]);
          tmpSVGOutput.push_back(tmpString);
     }

     unsigned page_num = 1;

     // If only one page is present, import that one without bothering user
     if (tmpSVGOutput.size() > 1) {
          CdrImportDialog *dlg = 0;
          if (INKSCAPE.use_gui()) {
               dlg = new CdrImportDialog(tmpSVGOutput);
               if (!dlg->showDialog()) {
                    delete dlg;
                    return NULL;
               }
          }

          // Get needed page
          if (dlg) {
               page_num = dlg->getSelectedPage();
               if (page_num < 1)
                    page_num = 1;
               if (page_num > tmpSVGOutput.size())
                    page_num = tmpSVGOutput.size();
          }
     }

     SPDocument * doc = SPDocument::createNewDocFromMem(tmpSVGOutput[page_num-1].cstr(), strlen(tmpSVGOutput[page_num-1].cstr()), TRUE);

     // Set viewBox if it doesn't exist
     if (doc && !doc->getRoot()->viewBox_set) {
         doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDisplayUnit()), doc->getHeight().value(doc->getDisplayUnit())));
     }
     return doc;
}

#include "clear-n_.h"

void CdrInput::init(void)
{
    /* CDR */
     Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Corel DRAW Input") "</name>\n"
            "<id>org.inkscape.input.cdr</id>\n"
            "<input>\n"
                "<extension>.cdr</extension>\n"
                "<mimetype>image/x-xcdr</mimetype>\n"
                "<filetypename>" N_("Corel DRAW 7-X4 files (*.cdr)") "</filetypename>\n"
                "<filetypetooltip>" N_("Open files saved in Corel DRAW 7-X4") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new CdrInput());

    /* CDT */
     Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Corel DRAW templates input") "</name>\n"
            "<id>org.inkscape.input.cdt</id>\n"
            "<input>\n"
                "<extension>.cdt</extension>\n"
                "<mimetype>application/x-xcdt</mimetype>\n"
                "<filetypename>" N_("Corel DRAW 7-13 template files (*.cdt)") "</filetypename>\n"
                "<filetypetooltip>" N_("Open files saved in Corel DRAW 7-13") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new CdrInput());

    /* CCX */
     Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Corel DRAW Compressed Exchange files input") "</name>\n"
            "<id>org.inkscape.input.ccx</id>\n"
            "<input>\n"
                "<extension>.ccx</extension>\n"
                "<mimetype>application/x-xccx</mimetype>\n"
                "<filetypename>" N_("Corel DRAW Compressed Exchange files (*.ccx)") "</filetypename>\n"
                "<filetypetooltip>" N_("Open compressed exchange files saved in Corel DRAW") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new CdrInput());

    /* CMX */
     Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Corel DRAW Presentation Exchange files input") "</name>\n"
            "<id>org.inkscape.input.cmx</id>\n"
            "<input>\n"
                "<extension>.cmx</extension>\n"
                "<mimetype>application/x-xcmx</mimetype>\n"
                "<filetypename>" N_("Corel DRAW Presentation Exchange files (*.cmx)") "</filetypename>\n"
                "<filetypetooltip>" N_("Open presentation exchange files saved in Corel DRAW") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new CdrInput());

     return;

} // init

} } }  /* namespace Inkscape, Extension, Implementation */
#endif /* WITH_LIBCDR */

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

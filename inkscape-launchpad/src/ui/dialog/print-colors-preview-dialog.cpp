/**
 * @file
 * Print Colors Preview dialog - implementation.
 */
/* Authors:
 *   Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPLv2 (or later).  Read the file 'COPYING' for more information.
 */
/*
#include "desktop.h"
#include "print-colors-preview-dialog.h"
#include "preferences.h"
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

//Yes, I know we shouldn't hardcode CMYK. This class needs to be refactored
// in order to accomodate spot colors and color components defined using 
// ICC colors. --Juca

void PrintColorsPreviewDialog::toggle_cyan(){
  Inkscape::Preferences *prefs = Inkscape::Preferences::get();
  prefs->setBool("/options/printcolorspreview/cyan", cyan->get_active());

  SPDesktop *desktop = getDesktop();
  desktop->setDisplayModePrintColorsPreview();
}

void PrintColorsPreviewDialog::toggle_magenta(){
  Inkscape::Preferences *prefs = Inkscape::Preferences::get();
  prefs->setBool("/options/printcolorspreview/magenta", magenta->get_active());

  SPDesktop *desktop = getDesktop();
  desktop->setDisplayModePrintColorsPreview();
}

void PrintColorsPreviewDialog::toggle_yellow(){
  Inkscape::Preferences *prefs = Inkscape::Preferences::get();
  prefs->setBool("/options/printcolorspreview/yellow", yellow->get_active());

  SPDesktop *desktop = getDesktop();
  desktop->setDisplayModePrintColorsPreview();
}

void PrintColorsPreviewDialog::toggle_black(){
  Inkscape::Preferences *prefs = Inkscape::Preferences::get();
  prefs->setBool("/options/printcolorspreview/black", black->get_active());

  SPDesktop *desktop = getDesktop();
  desktop->setDisplayModePrintColorsPreview();
}

PrintColorsPreviewDialog::PrintColorsPreviewDialog()
 : UI::Widget::Panel("", "/dialogs/printcolorspreview", SP_VERB_DIALOG_PRINT_COLORS_PREVIEW)
{
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());

    cyan = new Gtk::ToggleButton(_("Cyan"));
    vbox->pack_start( *cyan, false, false );
//    tips.set_tip((*cyan), _("Render cyan separation"));
    cyan->signal_clicked().connect( sigc::mem_fun(*this, &PrintColorsPreviewDialog::toggle_cyan) );

    magenta = new Gtk::ToggleButton(_("Magenta"));
    vbox->pack_start( *magenta, false, false );
//    tips.set_tip((*magenta), _("Render magenta separation"));
    magenta->signal_clicked().connect( sigc::mem_fun(*this, &PrintColorsPreviewDialog::toggle_magenta) );

    yellow = new Gtk::ToggleButton(_("Yellow"));
    vbox->pack_start( *yellow, false, false );
//    tips.set_tip((*yellow), _("Render yellow separation"));
    yellow->signal_clicked().connect( sigc::mem_fun(*this, &PrintColorsPreviewDialog::toggle_yellow) );

    black = new Gtk::ToggleButton(_("Black"));
    vbox->pack_start( *black, false, false );
//    tips.set_tip((*black), _("Render black separation"));
    black->signal_clicked().connect( sigc::mem_fun(*this, &PrintColorsPreviewDialog::toggle_black) );

    gint val;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    val = prefs->getBool("/options/printcolorspreview/cyan");
    cyan->set_active( val != 0 );
    val = prefs->getBool("/options/printcolorspreview/magenta");
    magenta->set_active( val != 0 );
    val = prefs->getBool("/options/printcolorspreview/yellow");
    yellow->set_active( val != 0 );
    val = prefs->getBool("/options/printcolorspreview/black");
    black->set_active( val != 0 );

    _getContents()->add(*vbox);
    _getContents()->show_all();
}

PrintColorsPreviewDialog::~PrintColorsPreviewDialog(){}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape
*/

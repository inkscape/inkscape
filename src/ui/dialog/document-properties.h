/** \file
 * \brief  Document Properties dialog
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

#include <list>
#include <sigc++/sigc++.h>
#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "ui/widget/notebook-page.h"
#include "ui/widget/page-sizer.h"
#include "ui/widget/registered-widget.h"
#include "ui/widget/registry.h"
#include "ui/widget/tolerance-slider.h"
#include "ui/widget/panel.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
    namespace XML {
        class Node;
    }
    namespace UI {
        namespace Dialog {

class DocumentProperties : public UI::Widget::Panel {
public:
    void  update();
    static DocumentProperties &getInstance();
    static void destroy();
    sigc::connection _doc_replaced_connection;

    void  update_gridspage();
    
protected:
    void  build_page();
    void  build_grid();
    void  build_guides();
    void  build_snap();
    void  build_snap_dtls();
    void  build_gridspage();
    void  init();
    virtual void  on_response (int);

    Gtk::Tooltips _tt;
    Gtk::Notebook  _notebook;

    NotebookPage   _page_page, _page_guides;
    NotebookPage   _page_snap, _page_snap_dtls;
    NotebookPage   _page_grids;

    //---------------------------------------------------------------
    RegisteredCheckButton _rcb_canb, _rcb_bord, _rcb_shad;
    RegisteredColorPicker _rcp_bg, _rcp_bord;
    RegisteredUnitMenu    _rum_deflt;
    PageSizer             _page_sizer;
    //---------------------------------------------------------------
    RegisteredCheckButton _rcbgrid;
    RegisteredUnitMenu    _rums;
    //---------------------------------------------------------------
    RegisteredCheckButton _rcb_sgui;
    RegisteredUnitMenu    _rum_gusn;
    RegisteredColorPicker _rcp_gui, _rcp_hgui;
    //---------------------------------------------------------------
    RegisteredCheckButton _rcbsnbb, _rcbsnn, _rcbsnop;
    RegisteredCheckButton _rcbsnon, _rcbsnbbp, _rcbsnbbn;
    RegisteredUnitMenu    _rumso;
    ToleranceSlider       _rsu_sno, _rsu_sn, _rsu_gusn;
    RegisteredRadioButtonPair _rrb_pix;
    //---------------------------------------------------------------
    RegisteredCheckButton _rcbic, _rcbsng;
    RegisteredCheckButton _rcbsigg, _rcbsils;
	//---------------------------------------------------------------
    Gtk::Notebook   _grids_notebook;
    Gtk::Button     _grids_button_new;
    Gtk::Button     _grids_button_remove;
    Gtk::ComboBoxText _grids_combo_gridtype;
    //---------------------------------------------------------------

    gchar const *_prefs_path;
    Registry _wr;

private:
    DocumentProperties();
    virtual ~DocumentProperties();

    // callback methods for buttons on grids page.
    void onNewGrid();
    void onRemoveGrid();    
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

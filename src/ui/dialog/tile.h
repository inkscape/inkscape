/** @file
 * @brief Dialog for creating grid type arrangements of selected objects
 */
/* Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_DIALOG_TILE_H
#define SEEN_UI_DIALOG_TILE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/threads.h>

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>

#include "ui/widget/panel.h"
#include "ui/widget/spinbutton.h"
#include "ui/widget/scalar-unit.h"

namespace Gtk {
class Button;

#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * Dialog for tiling an object
 */
class TileDialog : public UI::Widget::Panel {
public:
    TileDialog() ;
    virtual ~TileDialog() {};

    /**
     * Do the actual work
     */
    void Grid_Arrange();

    /**
     * Respond to selection change
     */
    void updateSelection();

    /**
     * Callback from Apply
     */
    virtual void _apply();

    // Callbacks from spinbuttons
    void on_row_spinbutton_changed();
    void on_col_spinbutton_changed();
    void on_xpad_spinbutton_changed();
    void on_ypad_spinbutton_changed();
    void on_RowSize_checkbutton_changed();
    void on_ColSize_checkbutton_changed();
    void on_rowSize_spinbutton_changed();
    void on_colSize_spinbutton_changed();
    void Spacing_button_changed();
    void VertAlign_changed();
    void HorizAlign_changed();

    static TileDialog& getInstance() { return *new TileDialog(); }

private:
    TileDialog(TileDialog const &d); // no copy
    void operator=(TileDialog const &d); // no assign

    bool userHidden;
    bool updating;

    Gtk::Notebook   notebook;

    Gtk::VBox             TileBox;
    Gtk::Button           *TileOkButton;
    Gtk::Button           *TileCancelButton;

    // Number selected label
    Gtk::Label            SelectionContentsLabel;


    Gtk::HBox             AlignHBox;
    Gtk::HBox             SpinsHBox;

    // Number per Row
    Gtk::VBox             NoOfColsBox;
    Gtk::Label            NoOfColsLabel;
    Inkscape::UI::Widget::SpinButton NoOfColsSpinner;
    bool AutoRowSize;
    Gtk::CheckButton      RowHeightButton;

    Gtk::VBox             XByYLabelVBox;
    Gtk::Label            padXByYLabel;
    Gtk::Label            XByYLabel;

    // Number per Column
    Gtk::VBox             NoOfRowsBox;
    Gtk::Label            NoOfRowsLabel;
    Inkscape::UI::Widget::SpinButton NoOfRowsSpinner;
    bool AutoColSize;
    Gtk::CheckButton      ColumnWidthButton;

    // Vertical align
    Gtk::Label            VertAlignLabel;
    Gtk::HBox             VertAlignHBox;
    Gtk::VBox             VertAlignVBox;
    Gtk::RadioButtonGroup VertAlignGroup;
    Gtk::RadioButton      VertCentreRadioButton;
    Gtk::RadioButton      VertTopRadioButton;
    Gtk::RadioButton      VertBotRadioButton;
    double VertAlign;

    // Horizontal align
    Gtk::Label            HorizAlignLabel;
    Gtk::VBox             HorizAlignVBox;
    Gtk::HBox             HorizAlignHBox;
    Gtk::RadioButtonGroup HorizAlignGroup;
    Gtk::RadioButton      HorizCentreRadioButton;
    Gtk::RadioButton      HorizLeftRadioButton;
    Gtk::RadioButton      HorizRightRadioButton;
    double HorizAlign;

    Inkscape::UI::Widget::UnitMenu      PaddingUnitMenu;
    Inkscape::UI::Widget::ScalarUnit    XPadding;
    Inkscape::UI::Widget::ScalarUnit    YPadding;

#if WITH_GTKMM_3_0
    Gtk::Grid                          *PaddingTable;
#else
    Gtk::Table                         *PaddingTable;
#endif

    // BBox or manual spacing
    Gtk::VBox             SpacingVBox;
    Gtk::RadioButtonGroup SpacingGroup;
    Gtk::RadioButton      SpaceByBBoxRadioButton;
    Gtk::RadioButton      SpaceManualRadioButton;
    bool ManualSpacing;

    // Row height
    Gtk::VBox             RowHeightVBox;
    Gtk::HBox             RowHeightBox;
    Gtk::Label            RowHeightLabel;
    Inkscape::UI::Widget::SpinButton RowHeightSpinner;

    // Column width
    Gtk::VBox             ColumnWidthVBox;
    Gtk::HBox             ColumnWidthBox;
    Gtk::Label            ColumnWidthLabel;
    Inkscape::UI::Widget::SpinButton ColumnWidthSpinner;
};


} //namespace Dialog
} //namespace UI
} //namespace Inkscape


#endif /* __TILEDIALOG_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

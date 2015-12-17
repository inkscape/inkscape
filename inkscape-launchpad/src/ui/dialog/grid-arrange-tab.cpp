/*
 * A simple dialog for creating grid type arrangements of selected objects
 *
 * Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *   Declara Denis
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
//#define DEBUG_GRID_ARRANGE 1

#include "ui/dialog/grid-arrange-tab.h"
#include <gtk/gtk.h> //for GTK_RESPONSE* types
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <2geom/transforms.h>

#include "verbs.h"
#include "preferences.h"
#include "inkscape.h"

#include "selection.h"
#include "document.h"
#include "document-undo.h"
#include "sp-item.h"
#include "widgets/icon.h"
#include "desktop.h"
//#include "sp-item-transform.h" FIXME
#include "ui/dialog/tile.h" // for Inkscape::UI::Dialog::ArrangeDialog

/*
 *    Sort items by their x co-ordinates, taking account of y (keeps rows intact)
 *
 *    <0 *elem1 goes before *elem2
 *    0  *elem1 == *elem2
 *    >0  *elem1 goes after *elem2
 */
static bool sp_compare_x_position(SPItem *first, SPItem *second)
{
    using Geom::X;
    using Geom::Y;

    Geom::OptRect a = first->documentVisualBounds();
    Geom::OptRect b = second->documentVisualBounds();

    if ( !a || !b ) {
        // FIXME?
        return false;
    }

    double const a_height = a->dimensions()[Y];
    double const b_height = b->dimensions()[Y];

    bool a_in_b_vert = false;
    if ((a->min()[Y] < b->min()[Y] + 0.1) && (a->min()[Y] > b->min()[Y] - b_height)) {
        a_in_b_vert = true;
    } else if ((b->min()[Y] < a->min()[Y] + 0.1) && (b->min()[Y] > a->min()[Y] - a_height)) {
        a_in_b_vert = true;
    } else if (b->min()[Y] == a->min()[Y]) {
        a_in_b_vert = true;
    } else {
        a_in_b_vert = false;
    }

    if (!a_in_b_vert) { // a and b are not in the same row
        return (a->min()[Y] < b->min()[Y]);
    }
    return (a->min()[X] < b->min()[X]);
}

/*
 *    Sort items by their y co-ordinates.
 */
static bool sp_compare_y_position(SPItem *first, SPItem *second)
{
    Geom::OptRect a = first->documentVisualBounds();
    Geom::OptRect b = second->documentVisualBounds();

    if ( !a || !b ) {
        // FIXME?
        return false;
    }

    if (a->min()[Geom::Y] > b->min()[Geom::Y]) {
        return false;
    }
    if (a->min()[Geom::Y] < b->min()[Geom::Y]) {
        return true;
    }

    return false;
}


namespace Inkscape {
namespace UI {
namespace Dialog {


//#########################################################################
//## E V E N T S
//#########################################################################

/*
 *
 * This arranges the selection in a grid pattern.
 *
 */

void GridArrangeTab::arrange()
{

    int cnt,row_cnt,col_cnt,a,row,col;
    double grid_left,grid_top,col_width,row_height,paddingx,paddingy,width, height, new_x, new_y;
    double total_col_width,total_row_height;
    col_width = 0;
    row_height = 0;
    total_col_width=0;
    total_row_height=0;

    // check for correct numbers in the row- and col-spinners
    on_col_spinbutton_changed();
    on_row_spinbutton_changed();

    // set padding to manual values
    paddingx = XPadding.getValue("px");
    paddingy = YPadding.getValue("px");

    std::vector<double> row_heights;
    std::vector<double> col_widths;
    std::vector<double> row_ys;
    std::vector<double> col_xs;

    int NoOfCols = NoOfColsSpinner.get_value_as_int();
    int NoOfRows = NoOfRowsSpinner.get_value_as_int();

    width = 0;
    for (a=0;a<NoOfCols; a++){
        col_widths.push_back(width);
    }

    height = 0;
    for (a=0;a<NoOfRows; a++){
        row_heights.push_back(height);
    }
    grid_left = 99999;
    grid_top = 99999;

    SPDesktop *desktop = Parent->getDesktop();
    desktop->getDocument()->ensureUpToDate();

    Inkscape::Selection *selection = desktop->getSelection();
    const std::vector<SPItem*> items = selection ? selection->itemList() : std::vector<SPItem*>();
    for(std::vector<SPItem*>::const_iterator i = items.begin();i!=items.end(); ++i){
        SPItem *item = *i;
        Geom::OptRect b = item->documentVisualBounds();
        if (!b) {
            continue;
        }

        width = b->dimensions()[Geom::X];
        height = b->dimensions()[Geom::Y];

        if (b->min()[Geom::X] < grid_left) {
            grid_left = b->min()[Geom::X];
        }
        if (b->min()[Geom::Y] < grid_top) {
            grid_top = b->min()[Geom::Y];
        }
        if (width > col_width) {
            col_width = width;
        }
        if (height > row_height) {
            row_height = height;
        }
    }


    // require the sorting done before we can calculate row heights etc.

    g_return_if_fail(selection);
    std::vector<SPItem*> sorted(selection->itemList());
    sort(sorted.begin(),sorted.end(),sp_compare_y_position);
    sort(sorted.begin(),sorted.end(),sp_compare_x_position);


    // Calculate individual Row and Column sizes if necessary


        cnt=0;
        const std::vector<SPItem*> sizes(sorted);
        for (std::vector<SPItem*>::const_iterator i = sizes.begin();i!=sizes.end(); ++i) {
            SPItem *item = *i;
            Geom::OptRect b = item->documentVisualBounds();
            if (b) {
                width = b->dimensions()[Geom::X];
                height = b->dimensions()[Geom::Y];
                if (width > col_widths[(cnt % NoOfCols)]) {
                    col_widths[(cnt % NoOfCols)] = width;
                }
                if (height > row_heights[(cnt / NoOfCols)]) {
                    row_heights[(cnt / NoOfCols)] = height;
                }
            }

            cnt++;
        }


    /// Make sure the top and left of the grid dont move by compensating for align values.
    if (RowHeightButton.get_active()){
        grid_top = grid_top - (((row_height - row_heights[0]) / 2)*(VertAlign));
    }
    if (ColumnWidthButton.get_active()){
        grid_left = grid_left - (((col_width - col_widths[0]) /2)*(HorizAlign));
    }

    #ifdef DEBUG_GRID_ARRANGE
     g_print("\n cx = %f cy= %f gridleft=%f",cx,cy,grid_left);
    #endif

    // Calculate total widths and heights, allowing for columns and rows non uniformly sized.

    if (ColumnWidthButton.get_active()){
        total_col_width = col_width * NoOfCols;
        col_widths.clear();
        for (a=0;a<NoOfCols; a++){
            col_widths.push_back(col_width);
        }
    } else {
        for (a = 0; a < (int)col_widths.size(); a++)
        {
          total_col_width += col_widths[a] ;
        }
    }

    if (RowHeightButton.get_active()){
        total_row_height = row_height * NoOfRows;
        row_heights.clear();
        for (a=0;a<NoOfRows; a++){
            row_heights.push_back(row_height);
        }
    } else {
        for (a = 0; a < (int)row_heights.size(); a++)
        {
          total_row_height += row_heights[a] ;
        }
    }


    Geom::OptRect sel_bbox = selection->visualBounds();
    // Fit to bbox, calculate padding between rows accordingly.
    if ( sel_bbox && !SpaceManualRadioButton.get_active() ){
#ifdef DEBUG_GRID_ARRANGE
g_print("\n row = %f     col = %f selection x= %f selection y = %f", total_row_height,total_col_width, b.extent(Geom::X), b.extent(Geom::Y));
#endif
        paddingx = (sel_bbox->width() - total_col_width) / (NoOfCols -1);
        paddingy = (sel_bbox->height() - total_row_height) / (NoOfRows -1);
    }

/*
    Horizontal align  - Left    = 0
                        Centre  = 1
                        Right   = 2

    Vertical align    - Top     = 0
                        Middle  = 1
                        Bottom  = 2

    X position is calculated by taking the grids left co-ord, adding the distance to the column,
   then adding 1/2 the spacing multiplied by the align variable above,
   Y position likewise, takes the top of the grid, adds the y to the current row then adds the padding in to align it.

*/

    // Calculate row and column x and y coords required to allow for columns and rows which are non uniformly sized.

    for (a=0;a<NoOfCols; a++){
        if (a<1) col_xs.push_back(0);
        else col_xs.push_back(col_widths[a-1]+paddingx+col_xs[a-1]);
    }


    for (a=0;a<NoOfRows; a++){
        if (a<1) row_ys.push_back(0);
        else row_ys.push_back(row_heights[a-1]+paddingy+row_ys[a-1]);
    }

    cnt=0;
    std::vector<SPItem*>::iterator it = sorted.begin();
    for (row_cnt=0; ((it != sorted.end()) && (row_cnt<NoOfRows)); ++row_cnt) {

             GSList *current_row = NULL;
             col_cnt = 0;
             for(;it!=sorted.end()&&col_cnt<NoOfCols;++it) {
                 current_row = g_slist_append (current_row, *it);
                 col_cnt++;
             }

             for (; current_row != NULL; current_row = current_row->next) {
                 SPItem *item=SP_ITEM(current_row->data);
                 Inkscape::XML::Node *repr = item->getRepr();
                 Geom::OptRect b = item->documentVisualBounds();
                 Geom::Point min;
                 if (b) {
                     width = b->dimensions()[Geom::X];
                     height = b->dimensions()[Geom::Y];
                     min = b->min();
                 } else {
                     width = height = 0;
                     min = Geom::Point(0, 0);
                 }

                 row = cnt / NoOfCols;
                 col = cnt % NoOfCols;

                 new_x = grid_left + (((col_widths[col] - width)/2)*HorizAlign) + col_xs[col];
                 new_y = grid_top + (((row_heights[row] - height)/2)*VertAlign) + row_ys[row];

                 // signs are inverted between x and y due to y inversion
                 Geom::Point move = Geom::Point(new_x - min[Geom::X], min[Geom::Y] - new_y);
                 Geom::Affine const affine = Geom::Affine(Geom::Translate(move));
                 item->set_i2d_affine(item->i2dt_affine() * affine);
                 item->doWriteTransform(repr, item->transform,  NULL);
                 SP_OBJECT (current_row->data)->updateRepr();
                 cnt +=1;
             }
             g_slist_free (current_row);
    }

    DocumentUndo::done(desktop->getDocument(), SP_VERB_SELECTION_ARRANGE,
                       _("Arrange in a grid"));

}


//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * changed value in # of columns spinbox.
 */
void GridArrangeTab::on_row_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = Parent->getDesktop();

    Inkscape::Selection *selection = desktop ? desktop->selection : 0;
    g_return_if_fail( selection );

    std::vector<SPItem*> const items = selection->itemList();
    int selcount = items.size();

    double PerCol = ceil(selcount / NoOfColsSpinner.get_value());
    NoOfRowsSpinner.set_value(PerCol);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/gridtiler/NoOfCols", NoOfColsSpinner.get_value());
    updating=false;
}

/**
 * changed value in # of rows spinbox.
 */
void GridArrangeTab::on_col_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = Parent->getDesktop();
    Inkscape::Selection *selection = desktop ? desktop->selection : 0;
    g_return_if_fail(selection);

    int selcount = selection->itemList().size();

    double PerRow = ceil(selcount / NoOfRowsSpinner.get_value());
    NoOfColsSpinner.set_value(PerRow);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/gridtiler/NoOfCols", PerRow);

    updating=false;
}

/**
 * changed value in x padding spinbox.
 */
void GridArrangeTab::on_xpad_spinbutton_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/gridtiler/XPad", XPadding.getValue("px"));

}

/**
 * changed value in y padding spinbox.
 */
void GridArrangeTab::on_ypad_spinbutton_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/gridtiler/YPad", YPadding.getValue("px"));
}


/**
 * checked/unchecked autosize Rows button.
 */
void GridArrangeTab::on_RowSize_checkbutton_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (RowHeightButton.get_active()) {
        prefs->setDouble("/dialogs/gridtiler/AutoRowSize", 20);
    } else {
        prefs->setDouble("/dialogs/gridtiler/AutoRowSize", -20);
    }
    RowHeightBox.set_sensitive ( !RowHeightButton.get_active());
}

/**
 * checked/unchecked autosize Rows button.
 */
void GridArrangeTab::on_ColSize_checkbutton_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (ColumnWidthButton.get_active()) {
        prefs->setDouble("/dialogs/gridtiler/AutoColSize", 20);
    } else {
        prefs->setDouble("/dialogs/gridtiler/AutoColSize", -20);
    }
    ColumnWidthBox.set_sensitive ( !ColumnWidthButton.get_active());
}

/**
 * changed value in columns spinbox.
 */
void GridArrangeTab::on_rowSize_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/gridtiler/RowHeight", RowHeightSpinner.get_value());
    updating=false;

}

/**
 * changed value in rows spinbox.
 */
void GridArrangeTab::on_colSize_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/gridtiler/ColWidth", ColumnWidthSpinner.get_value());
    updating=false;

}

/**
 * changed Radio button in Spacing group.
 */
void GridArrangeTab::Spacing_button_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (SpaceManualRadioButton.get_active()) {
        prefs->setDouble("/dialogs/gridtiler/SpacingType", 20);
    } else {
        prefs->setDouble("/dialogs/gridtiler/SpacingType", -20);
    }

    XPadding.set_sensitive ( SpaceManualRadioButton.get_active());
    YPadding.set_sensitive ( SpaceManualRadioButton.get_active());
}

/**
 * changed Anchor selection widget.
 */
void GridArrangeTab::Align_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    VertAlign = AlignmentSelector.getVerticalAlignment();
    prefs->setInt("/dialogs/gridtiler/VertAlign", VertAlign);
    HorizAlign = AlignmentSelector.getHorizontalAlignment();
    prefs->setInt("/dialogs/gridtiler/HorizAlign", HorizAlign);
}

/**
 * Desktop selection changed
 */
void GridArrangeTab::updateSelection()
{
    // quit if run by the attr_changed listener
    if (updating) {
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = Parent->getDesktop();
    Inkscape::Selection *selection = desktop ? desktop->selection : 0;
    std::vector<SPItem*> const items = selection ? selection->itemList() : std::vector<SPItem*>();

    if (!items.empty()) {
        int selcount = items.size();

        if (NoOfColsSpinner.get_value() > 1 && NoOfRowsSpinner.get_value() > 1){
            // Update the number of rows assuming number of columns wanted remains same.
            double NoOfRows = ceil(selcount / NoOfColsSpinner.get_value());
            NoOfRowsSpinner.set_value(NoOfRows);

            // if the selection has less than the number set for one row, reduce it appropriately
            if (selcount < NoOfColsSpinner.get_value()) {
                double NoOfCols = ceil(selcount / NoOfRowsSpinner.get_value());
                NoOfColsSpinner.set_value(NoOfCols);
                prefs->setInt("/dialogs/gridtiler/NoOfCols", NoOfCols);
            }
        } else {
            double PerRow = ceil(sqrt(selcount));
            double PerCol = ceil(sqrt(selcount));
            NoOfRowsSpinner.set_value(PerRow);
            NoOfColsSpinner.set_value(PerCol);
            prefs->setInt("/dialogs/gridtiler/NoOfCols", static_cast<int>(PerCol));
        }
    }

    updating = false;
}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
GridArrangeTab::GridArrangeTab(ArrangeDialog *parent)
    : Parent(parent),
      XPadding(_("X:"), _("Horizontal spacing between columns."), UNIT_TYPE_LINEAR, "", "object-columns", &PaddingUnitMenu),
      YPadding(_("Y:"), _("Vertical spacing between rows."), XPadding, "", "object-rows", &PaddingUnitMenu),
#if WITH_GTKMM_3_0
      PaddingTable(Gtk::manage(new Gtk::Grid()))
#else
      PaddingTable(Gtk::manage(new Gtk::Table(2, 2, false)))
#endif
{
     // bool used by spin button callbacks to stop loops where they change each other.
    updating = false;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // could not do this in gtkmm - there's no Gtk::SizeGroup public constructor (!)
    GtkSizeGroup *_col1 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    GtkSizeGroup *_col2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    GtkSizeGroup *_col3 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    {
        // Selection Change signal
        INKSCAPE.signal_selection_changed.connect(sigc::hide<0>(sigc::mem_fun(*this, &GridArrangeTab::updateSelection)));
    }

    Gtk::Box *contents = this;

#define MARGIN 2

    //##Set up the panel

    SPDesktop *desktop = Parent->getDesktop();

    Inkscape::Selection *selection = desktop ? desktop->selection : 0;
    g_return_if_fail( selection );
    int selcount = 1;
    if (!selection->isEmpty()) {
        selcount = selection->itemList().size();
    }


    /*#### Number of Rows ####*/

    double PerRow = ceil(sqrt(selcount));
    double PerCol = ceil(sqrt(selcount));

    #ifdef DEBUG_GRID_ARRANGE
        g_print("/n PerRox = %f PerCol = %f selcount = %d",PerRow,PerCol,selcount);
    #endif

    NoOfRowsLabel.set_text_with_mnemonic(_("_Rows:"));
    NoOfRowsLabel.set_mnemonic_widget(NoOfRowsSpinner);
    NoOfRowsBox.pack_start(NoOfRowsLabel, false, false, MARGIN);

    NoOfRowsSpinner.set_digits(0);
    NoOfRowsSpinner.set_increments(1, 0);
    NoOfRowsSpinner.set_range(1.0, 10000.0);
    NoOfRowsSpinner.set_value(PerCol);
    NoOfRowsSpinner.signal_changed().connect(sigc::mem_fun(*this, &GridArrangeTab::on_col_spinbutton_changed));
    NoOfRowsSpinner.set_tooltip_text(_("Number of rows"));
    NoOfRowsBox.pack_start(NoOfRowsSpinner, false, false, MARGIN);
    gtk_size_group_add_widget(_col1, (GtkWidget *) NoOfRowsBox.gobj());

    RowHeightButton.set_label(_("Equal _height"));
    RowHeightButton.set_use_underline(true);
    double AutoRow = prefs->getDouble("/dialogs/gridtiler/AutoRowSize", 15);
    if (AutoRow>0)
         AutoRowSize=true;
    else
         AutoRowSize=false;
    RowHeightButton.set_active(AutoRowSize);

    NoOfRowsBox.pack_start(RowHeightButton, false, false, MARGIN);

    RowHeightButton.set_tooltip_text(_("If not set, each row has the height of the tallest object in it"));
    RowHeightButton.signal_toggled().connect(sigc::mem_fun(*this, &GridArrangeTab::on_RowSize_checkbutton_changed));

    SpinsHBox.pack_start(NoOfRowsBox, false, false, MARGIN);


    /*#### Label for X ####*/
    padXByYLabel.set_label(" ");
    XByYLabelVBox.pack_start(padXByYLabel, false, false, MARGIN);
    XByYLabel.set_markup(" &#215; ");
    XByYLabelVBox.pack_start(XByYLabel, false, false, MARGIN);
    SpinsHBox.pack_start(XByYLabelVBox, false, false, MARGIN);
    gtk_size_group_add_widget(_col2, GTK_WIDGET(XByYLabelVBox.gobj()));

    /*#### Number of columns ####*/

    NoOfColsLabel.set_text_with_mnemonic(_("_Columns:"));
    NoOfColsLabel.set_mnemonic_widget(NoOfColsSpinner);
    NoOfColsBox.pack_start(NoOfColsLabel, false, false, MARGIN);

    NoOfColsSpinner.set_digits(0);
    NoOfColsSpinner.set_increments(1, 0);
    NoOfColsSpinner.set_range(1.0, 10000.0);
    NoOfColsSpinner.set_value(PerRow);
    NoOfColsSpinner.signal_changed().connect(sigc::mem_fun(*this, &GridArrangeTab::on_row_spinbutton_changed));
    NoOfColsSpinner.set_tooltip_text(_("Number of columns"));
    NoOfColsBox.pack_start(NoOfColsSpinner, false, false, MARGIN);
    gtk_size_group_add_widget(_col3, GTK_WIDGET(NoOfColsBox.gobj()));

    ColumnWidthButton.set_label(_("Equal _width"));
    ColumnWidthButton.set_use_underline(true);
    double AutoCol = prefs->getDouble("/dialogs/gridtiler/AutoColSize", 15);
    if (AutoCol>0)
         AutoColSize=true;
    else
         AutoColSize=false;
    ColumnWidthButton.set_active(AutoColSize);
    NoOfColsBox.pack_start(ColumnWidthButton, false, false, MARGIN);

    ColumnWidthButton.set_tooltip_text(_("If not set, each column has the width of the widest object in it"));
    ColumnWidthButton.signal_toggled().connect(sigc::mem_fun(*this, &GridArrangeTab::on_ColSize_checkbutton_changed));

    SpinsHBox.pack_start(NoOfColsBox, false, false, MARGIN);

    TileBox.pack_start(SpinsHBox, false, false, MARGIN);

    VertAlign = prefs->getInt("/dialogs/gridtiler/VertAlign", 1);
    HorizAlign = prefs->getInt("/dialogs/gridtiler/HorizAlign", 1);

    // Anchor selection widget
    AlignLabel.set_label(_("Alignment:"));
    AlignLabel.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
    AlignmentSelector.setAlignment(HorizAlign, VertAlign);
    AlignmentSelector.on_selectionChanged().connect(sigc::mem_fun(*this, &GridArrangeTab::Align_changed));
    TileBox.pack_start(AlignLabel, false, false, MARGIN);
    TileBox.pack_start(AlignmentSelector, true, false, MARGIN);

    {
        /*#### Radio buttons to control spacing manually or to fit selection bbox ####*/
        SpaceByBBoxRadioButton.set_label(_("_Fit into selection box"));
        SpaceByBBoxRadioButton.set_use_underline (true);
        SpaceByBBoxRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &GridArrangeTab::Spacing_button_changed));
        SpacingGroup = SpaceByBBoxRadioButton.get_group();

        SpacingVBox.pack_start(SpaceByBBoxRadioButton, false, false, MARGIN);

        SpaceManualRadioButton.set_label(_("_Set spacing:"));
        SpaceManualRadioButton.set_use_underline (true);
        SpaceManualRadioButton.set_group(SpacingGroup);
        SpaceManualRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &GridArrangeTab::Spacing_button_changed));
        SpacingVBox.pack_start(SpaceManualRadioButton, false, false, MARGIN);

        TileBox.pack_start(SpacingVBox, false, false, MARGIN);
    }

    {
        /*#### Padding ####*/
        PaddingUnitMenu.setUnitType(UNIT_TYPE_LINEAR);
        PaddingUnitMenu.setUnit("px");

        YPadding.setDigits(5);
        YPadding.setIncrements(0.2, 0);
        YPadding.setRange(-10000, 10000);
        double yPad = prefs->getDouble("/dialogs/gridtiler/YPad", 15);
        YPadding.setValue(yPad, "px");
        YPadding.signal_value_changed().connect(sigc::mem_fun(*this, &GridArrangeTab::on_ypad_spinbutton_changed));

        XPadding.setDigits(5);
        XPadding.setIncrements(0.2, 0);
        XPadding.setRange(-10000, 10000);
        double xPad = prefs->getDouble("/dialogs/gridtiler/XPad", 15);
        XPadding.setValue(xPad, "px");

        XPadding.signal_value_changed().connect(sigc::mem_fun(*this, &GridArrangeTab::on_xpad_spinbutton_changed));
    }

    PaddingTable->set_border_width(MARGIN);

#if WITH_GTKMM_3_0
    PaddingTable->set_row_spacing(MARGIN);
    PaddingTable->set_column_spacing(MARGIN);
    PaddingTable->attach(XPadding,        0, 0, 1, 1);
    PaddingTable->attach(PaddingUnitMenu, 1, 0, 1, 1);
    PaddingTable->attach(YPadding,        0, 1, 1, 1);
#else
    PaddingTable->set_row_spacings(MARGIN);
    PaddingTable->set_col_spacings(MARGIN);
    PaddingTable->attach(XPadding, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    PaddingTable->attach(PaddingUnitMenu, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    PaddingTable->attach(YPadding, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
#endif

    TileBox.pack_start(*PaddingTable, false, false, MARGIN);

    contents->pack_start(TileBox);

    double SpacingType = prefs->getDouble("/dialogs/gridtiler/SpacingType", 15);
    if (SpacingType>0) {
        ManualSpacing=true;
    } else {
        ManualSpacing=false;
    }
    SpaceManualRadioButton.set_active(ManualSpacing);
    SpaceByBBoxRadioButton.set_active(!ManualSpacing);
    XPadding.set_sensitive (ManualSpacing);
    YPadding.set_sensitive (ManualSpacing);

    //## The OK button FIXME
    /*TileOkButton = addResponseButton(C_("Rows and columns dialog","_Arrange"), GTK_RESPONSE_APPLY);
    TileOkButton->set_use_underline(true);
    TileOkButton->set_tooltip_text(_("Arrange selected objects"));*/

    show_all_children();
}

} //namespace Dialog
} //namespace UI
} //namespace Inkscape

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

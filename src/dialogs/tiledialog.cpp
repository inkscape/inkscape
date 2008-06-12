/*
 * A simple dialog for creating grid type arrangements of selected objects
 *
 * Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
//#define DEBUG_GRID_ARRANGE 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <gtk/gtksizegroup.h>
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>

#include "libnr/nr-matrix-ops.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "selection.h"
#include "document.h"
#include "sp-item.h"
#include "widgets/icon.h"
#include "tiledialog.h"



/*
 *    Sort items by their x co-ordinates, taking account of y (keeps rows intact)
 *
 *    <0 *elem1 goes before *elem2
 *    0  *elem1 == *elem2
 *    >0  *elem1 goes after *elem2
 */
int
sp_compare_x_position(SPItem *first, SPItem *second)
{
    using NR::X;
    using NR::Y;

    NR::Maybe<NR::Rect> a = first->getBounds(from_2geom(sp_item_i2doc_affine(first)));
    NR::Maybe<NR::Rect> b = second->getBounds(from_2geom(sp_item_i2doc_affine(second)));

    if ( !a || !b ) {
        // FIXME?
        return 0;
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

    if (!a_in_b_vert) {
        return -1;
    }
    if (a_in_b_vert && a->min()[X] > b->min()[X]) {
        return 1;
    }
    if (a_in_b_vert && a->min()[X] < b->min()[X]) {
        return -1;
    }
    return 0;
}

/*
 *    Sort items by their y co-ordinates.
 */
int
sp_compare_y_position(SPItem *first, SPItem *second)
{
    NR::Maybe<NR::Rect> a = first->getBounds(from_2geom(sp_item_i2doc_affine(first)));
    NR::Maybe<NR::Rect> b = second->getBounds(from_2geom(sp_item_i2doc_affine(second)));

    if ( !a || !b ) {
        // FIXME?
        return 0;
    }

    if (a->min()[NR::Y] > b->min()[NR::Y]) {
        return 1;
    }
    if (a->min()[NR::Y] < b->min()[NR::Y]) {
        return -1;
    }
    
    return 0;
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

void TileDialog::Grid_Arrange ()
{

    int cnt,row_cnt,col_cnt,a,row,col;
    double grid_left,grid_top,col_width,row_height,paddingx,paddingy,width, height, new_x, new_y,cx,cy;
    double total_col_width,total_row_height;
    col_width = 0;
    row_height = 0;
    total_col_width=0;
    total_row_height=0;

    // check for correct numbers in the row- and col-spinners
    on_col_spinbutton_changed();
    on_row_spinbutton_changed();

    // set padding to manual values
    paddingx = XPadSpinner.get_value();
    paddingy = YPadSpinner.get_value();

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

    SPDesktop *desktop = getDesktop();
    sp_document_ensure_up_to_date(sp_desktop_document(desktop));

    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    const GSList *items = selection->itemList();
    cnt=0;
    for (; items != NULL; items = items->next) {
        SPItem *item = SP_ITEM(items->data);
        NR::Maybe<NR::Rect> b = item->getBounds(from_2geom(sp_item_i2doc_affine(item)));
        if (!b) {
            continue;
        }

        width = b->dimensions()[NR::X];
        height = b->dimensions()[NR::Y];

        cx = b->midpoint()[NR::X];
        cy = b->midpoint()[NR::Y];

        if (b->min()[NR::X] < grid_left) {
            grid_left = b->min()[NR::X];
        }
        if (b->min()[NR::Y] < grid_top) {
            grid_top = b->min()[NR::Y];
        }
        if (width > col_width) {
            col_width = width;
        }
        if (height > row_height) {
            row_height = height;
        }
    }


    // require the sorting done before we can calculate row heights etc.

    const GSList *items2 = selection->itemList();
    GSList *rev = g_slist_copy((GSList *) items2);
    GSList *sorted = NULL;
    rev = g_slist_sort(rev, (GCompareFunc) sp_compare_y_position);
    sorted = g_slist_sort(rev, (GCompareFunc) sp_compare_x_position);


    // Calculate individual Row and Column sizes if necessary


        cnt=0;
        const GSList *sizes = sorted;
        for (; sizes != NULL; sizes = sizes->next) {
            SPItem *item = SP_ITEM(sizes->data);
            NR::Maybe<NR::Rect> b = item->getBounds(from_2geom(sp_item_i2doc_affine(item)));
            if (b) {
                width = b->dimensions()[NR::X];
                height = b->dimensions()[NR::Y];
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


    NR::Maybe<NR::Rect> sel_bbox = selection->bounds();
    // Fit to bbox, calculate padding between rows accordingly.
    if ( sel_bbox && !SpaceManualRadioButton.get_active() ){
#ifdef DEBUG_GRID_ARRANGE
g_print("\n row = %f     col = %f selection x= %f selection y = %f", total_row_height,total_col_width, b.extent(NR::X), b.extent(NR::Y));
#endif
        paddingx = (sel_bbox->extent(NR::X) - total_col_width) / (NoOfCols -1);
        paddingy = (sel_bbox->extent(NR::Y) - total_row_height) / (NoOfRows -1);
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
  for (row_cnt=0; ((sorted != NULL) && (row_cnt<NoOfRows)); row_cnt++) {

             GSList *current_row = NULL;
             for (col_cnt = 0; ((sorted != NULL) && (col_cnt<NoOfCols)); col_cnt++) {
                 current_row = g_slist_append (current_row, sorted->data);
                 sorted = sorted->next;
             }

             for (; current_row != NULL; current_row = current_row->next) {
                 SPItem *item=SP_ITEM(current_row->data);
                 Inkscape::XML::Node *repr = SP_OBJECT_REPR(item);
                 NR::Maybe<NR::Rect> b = item->getBounds(from_2geom(sp_item_i2doc_affine(item)));
                 NR::Point min;
                 if (b) {
                     width = b->dimensions()[NR::X];
                     height = b->dimensions()[NR::Y];
                     min = b->min();
                 } else {
                     width = height = 0;
                     min = NR::Point(0, 0);
                 }

                 row = cnt / NoOfCols;
                 col = cnt % NoOfCols;

                 new_x = grid_left + (((col_widths[col] - width)/2)*HorizAlign) + col_xs[col];
                 new_y = grid_top + (((row_heights[row] - height)/2)*VertAlign) + row_ys[row];

                 // signs are inverted between x and y due to y inversion
                 NR::Point move = NR::Point(new_x - min[NR::X], min[NR::Y] - new_y);
                 NR::Matrix const &affine = NR::Matrix(NR::translate(move));
                 sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * to_2geom(affine));
                 sp_item_write_transform(item, repr, item->transform,  NULL);
                 SP_OBJECT (current_row->data)->updateRepr();
                 cnt +=1;
             }
             g_slist_free (current_row);
    }

    NRRect b;
            selection->bounds(&b);

            sp_document_done (sp_desktop_document (desktop), SP_VERB_SELECTION_GRIDTILE, 
                              _("Arrange in a grid"));

}


//#########################################################################
//## E V E N T S
//#########################################################################


void TileDialog::_apply()
{
	Grid_Arrange();
}


/**
 * changed value in # of columns spinbox.
 */
void TileDialog::on_row_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = getDesktop();

    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    GSList const *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);

    double PerCol = ceil(selcount / NoOfColsSpinner.get_value());
    NoOfRowsSpinner.set_value(PerCol);
    prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", NoOfColsSpinner.get_value());
    updating=false;
}

/**
 * changed value in # of rows spinbox.
 */
void TileDialog::on_col_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = getDesktop();
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    GSList const *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);

    double PerRow = ceil(selcount / NoOfRowsSpinner.get_value());
    NoOfColsSpinner.set_value(PerRow);
    prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", PerRow);

    updating=false;
}

/**
 * changed value in x padding spinbox.
 */
void TileDialog::on_xpad_spinbutton_changed()
{

    prefs_set_double_attribute ("dialogs.gridtiler", "XPad", XPadSpinner.get_value());

}

/**
 * changed value in y padding spinbox.
 */
void TileDialog::on_ypad_spinbutton_changed()
{

    prefs_set_double_attribute ("dialogs.gridtiler", "YPad", YPadSpinner.get_value());

}


/**
 * checked/unchecked autosize Rows button.
 */
void TileDialog::on_RowSize_checkbutton_changed()
{

   if (RowHeightButton.get_active()) {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoRowSize", 20);
   } else {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoRowSize", -20);
   }
   RowHeightBox.set_sensitive ( !RowHeightButton.get_active());
}

/**
 * checked/unchecked autosize Rows button.
 */
void TileDialog::on_ColSize_checkbutton_changed()
{

   if (ColumnWidthButton.get_active()) {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoColSize", 20);
   } else {
       prefs_set_double_attribute ("dialogs.gridtiler", "AutoColSize", -20);
   }
   ColumnWidthBox.set_sensitive ( !ColumnWidthButton.get_active());

}

/**
 * changed value in columns spinbox.
 */
void TileDialog::on_rowSize_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    prefs_set_double_attribute ("dialogs.gridtiler", "RowHeight", RowHeightSpinner.get_value());
    updating=false;

}

/**
 * changed value in rows spinbox.
 */
void TileDialog::on_colSize_spinbutton_changed()
{
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    // in turn, prevent listener from responding
    updating = true;
    prefs_set_double_attribute ("dialogs.gridtiler", "ColWidth", ColumnWidthSpinner.get_value());
    updating=false;

}

/**
 * changed Radio button in Spacing group.
 */
void TileDialog::Spacing_button_changed()
{
   if (SpaceManualRadioButton.get_active()) {
       prefs_set_double_attribute ("dialogs.gridtiler", "SpacingType", 20);
   } else {
       prefs_set_double_attribute ("dialogs.gridtiler", "SpacingType", -20);
   }

   SizesHBox.set_sensitive ( SpaceManualRadioButton.get_active());
}

/**
 * changed Radio button in Vertical Align group.
 */
void TileDialog::VertAlign_changed()
{
   if (VertTopRadioButton.get_active()) {
       VertAlign = 0;
       prefs_set_double_attribute ("dialogs.gridtiler", "VertAlign", 0);
   } else if (VertCentreRadioButton.get_active()){
       VertAlign = 1;
       prefs_set_double_attribute ("dialogs.gridtiler", "VertAlign", 1);
   } else if (VertBotRadioButton.get_active()){
       VertAlign = 2;
       prefs_set_double_attribute ("dialogs.gridtiler", "VertAlign", 2);
   }

}

/**
 * changed Radio button in Vertical Align group.
 */
void TileDialog::HorizAlign_changed()
{
   if (HorizLeftRadioButton.get_active()) {
       HorizAlign = 0;
       prefs_set_double_attribute ("dialogs.gridtiler", "HorizAlign", 0);
   } else if (HorizCentreRadioButton.get_active()){
       HorizAlign = 1;
       prefs_set_double_attribute ("dialogs.gridtiler", "HorizAlign", 1);
   } else if (HorizRightRadioButton.get_active()){
       HorizAlign = 2;
       prefs_set_double_attribute ("dialogs.gridtiler", "HorizAlign", 2);
   }

}

/**
 * Desktop selection changed
 */
void TileDialog::updateSelection()
{
    double col_width, row_height;
    // quit if run by the attr_changed listener
    if (updating) {
            return;
        }

    col_width=0;
    row_height=0;
    // in turn, prevent listener from responding
    updating = true;
    SPDesktop *desktop = getDesktop();
    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    const GSList *items = selection->itemList();
    int selcount = g_slist_length((GSList *)items);

    if (NoOfColsSpinner.get_value()>1){
        // Update the number of rows assuming number of columns wanted remains same.
        double NoOfRows = ceil(selcount / NoOfColsSpinner.get_value());
        NoOfRowsSpinner.set_value(NoOfRows);

        // if the selection has less than the number set for one row, reduce it appropriately
        if (selcount<NoOfColsSpinner.get_value()) {
            double NoOfCols = ceil(selcount / NoOfRowsSpinner.get_value());
            NoOfColsSpinner.set_value(NoOfCols);
            prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", NoOfCols);
        }
    } else {
        NoOfColsSpinner.set_value(selcount);
        prefs_set_double_attribute ("dialogs.gridtiler", "NoOfCols", selcount);

    }

    updating=false;

}



/*##########################
## Experimental
##########################*/

static void updateSelectionCallback(Inkscape::Application */*inkscape*/, Inkscape::Selection */*selection*/, TileDialog *dlg)
{
    TileDialog *tledlg = (TileDialog *) dlg;
    tledlg->updateSelection();
}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
TileDialog::TileDialog()
    : UI::Widget::Panel("", "dialogs.gridtiler", SP_VERB_SELECTION_GRIDTILE)
{
     // bool used by spin button callbacks to stop loops where they change each other.
    updating = false;

    // could not do this in gtkmm - there's no Gtk::SizeGroup public constructor (!)
    GtkSizeGroup *_col1 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    GtkSizeGroup *_col2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    GtkSizeGroup *_col3 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    {
        // Selection Change signal
        g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (updateSelectionCallback), this);
    }

    Gtk::Box *contents = _getContents();

#define MARGIN 2

    //##Set up the panel

    SPDesktop *desktop = getDesktop();

    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    int selcount = 1;
    if (!selection->isEmpty()) {
        GSList const *items = selection->itemList();
        selcount = g_slist_length((GSList *)items);
    }


    /*#### Number of Rows ####*/

    double PerRow = selcount;
    double PerCol = 1;

    #ifdef DEBUG_GRID_ARRANGE
        g_print("/n PerRox = %f PerCol = %f selcount = %d",PerRow,PerCol,selcount);
    #endif

    NoOfRowsLabel.set_label(_("Rows:"));
    NoOfRowsBox.pack_start(NoOfRowsLabel, false, false, MARGIN);

    NoOfRowsSpinner.set_digits(0);
    NoOfRowsSpinner.set_increments(1, 5);
    NoOfRowsSpinner.set_range(1.0, 100.0);
    NoOfRowsSpinner.set_value(PerCol);
    NoOfRowsSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialog::on_col_spinbutton_changed));
    tips.set_tip(NoOfRowsSpinner, _("Number of rows"));
    NoOfRowsBox.pack_start(NoOfRowsSpinner, false, false, MARGIN);
    gtk_size_group_add_widget(_col1, (GtkWidget *) NoOfRowsBox.gobj());

    RowHeightButton.set_label(_("Equal height"));
    double AutoRow = prefs_get_double_attribute ("dialogs.gridtiler", "AutoRowSize", 15);
    if (AutoRow>0)
         AutoRowSize=true;
    else
         AutoRowSize=false;
    RowHeightButton.set_active(AutoRowSize);

    NoOfRowsBox.pack_start(RowHeightButton, false, false, MARGIN);

    tips.set_tip(RowHeightButton, _("If not set, each row has the height of the tallest object in it"));
    RowHeightButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::on_RowSize_checkbutton_changed));

 {
        /*#### Radio buttons to control vertical alignment ####*/

        VertAlignLabel.set_label(_("Align:"));
        VertAlignHBox.pack_start(VertAlignLabel, false, false, MARGIN);

        VertTopRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::VertAlign_changed));
        VertAlignGroup = VertTopRadioButton.get_group();
        VertAlignVBox.pack_start(VertTopRadioButton, false, false, 0);

        VertCentreRadioButton.set_group(VertAlignGroup);
        VertCentreRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::VertAlign_changed));
        VertAlignVBox.pack_start(VertCentreRadioButton, false, false, 0);

        VertBotRadioButton.set_group(VertAlignGroup);
        VertBotRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::VertAlign_changed));
        VertAlignVBox.pack_start(VertBotRadioButton, false, false, 0);

        VertAlign = prefs_get_double_attribute ("dialogs.gridtiler", "VertAlign", 1);
        if (VertAlign == 0) {
            VertTopRadioButton.set_active(TRUE);
        }
        else if (VertAlign == 1) {
            VertCentreRadioButton.set_active(TRUE);
        }
        else if (VertAlign == 2){
            VertBotRadioButton.set_active(TRUE);
        }
        VertAlignHBox.pack_start(VertAlignVBox, false, false, MARGIN);
        NoOfRowsBox.pack_start(VertAlignHBox, false, false, MARGIN);
    }

    SpinsHBox.pack_start(NoOfRowsBox, false, false, MARGIN);


    /*#### Label for X ####*/
    padXByYLabel.set_label(" ");
    XByYLabelVBox.pack_start(padXByYLabel, false, false, MARGIN);
    XByYLabel.set_markup(" &#215; ");
    XByYLabelVBox.pack_start(XByYLabel, false, false, MARGIN);
    SpinsHBox.pack_start(XByYLabelVBox, false, false, MARGIN);
    gtk_size_group_add_widget(_col2, (GtkWidget *) XByYLabelVBox.gobj());

    /*#### Number of columns ####*/

    NoOfColsLabel.set_label(_("Columns:"));
    NoOfColsBox.pack_start(NoOfColsLabel, false, false, MARGIN);

    NoOfColsSpinner.set_digits(0);
    NoOfColsSpinner.set_increments(1, 5);
    NoOfColsSpinner.set_range(1.0, 100.0);
    NoOfColsSpinner.set_value(PerRow);
    NoOfColsSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialog::on_row_spinbutton_changed));
    tips.set_tip(NoOfColsSpinner, _("Number of columns"));
    NoOfColsBox.pack_start(NoOfColsSpinner, false, false, MARGIN);
    gtk_size_group_add_widget(_col3, (GtkWidget *) NoOfColsBox.gobj());

    ColumnWidthButton.set_label(_("Equal width"));
    double AutoCol = prefs_get_double_attribute ("dialogs.gridtiler", "AutoColSize", 15);
    if (AutoCol>0)
         AutoColSize=true;
    else
         AutoColSize=false;
    ColumnWidthButton.set_active(AutoColSize);
    NoOfColsBox.pack_start(ColumnWidthButton, false, false, MARGIN);

    tips.set_tip(ColumnWidthButton, _("If not set, each column has the width of the widest object in it"));
    ColumnWidthButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::on_ColSize_checkbutton_changed));


    {
        /*#### Radio buttons to control horizontal alignment ####*/

        HorizAlignLabel.set_label(_("Align:"));
        HorizAlignVBox.pack_start(HorizAlignLabel, false, false, MARGIN);

        HorizAlignHBox.pack_start(*(new Gtk::HBox()), true, true, 0); // centering strut

        HorizLeftRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::HorizAlign_changed));
        HorizAlignGroup = HorizLeftRadioButton.get_group();
        HorizAlignHBox.pack_start(HorizLeftRadioButton, false, false, 0);

        HorizCentreRadioButton.set_group(HorizAlignGroup);
        HorizCentreRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::HorizAlign_changed));
        HorizAlignHBox.pack_start(HorizCentreRadioButton, false, false, 0);

        HorizRightRadioButton.set_group(HorizAlignGroup);
        HorizRightRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::HorizAlign_changed));
        HorizAlignHBox.pack_start(HorizRightRadioButton, false, false, 0);

        HorizAlignHBox.pack_start(*(new Gtk::HBox()), true, true, 0); // centering strut

        HorizAlign = prefs_get_double_attribute ("dialogs.gridtiler", "HorizAlign", 1);
        if (HorizAlign == 0) {
            HorizLeftRadioButton.set_active(TRUE);
        }
        else if (HorizAlign == 1) {
            HorizCentreRadioButton.set_active(TRUE);
        }
        else if (HorizAlign == 2) {
            HorizRightRadioButton.set_active(TRUE);
        }
        HorizAlignVBox.pack_start(HorizAlignHBox, false, false, MARGIN);
        NoOfColsBox.pack_start(HorizAlignVBox, false, false, MARGIN);
    }

    SpinsHBox.pack_start(NoOfColsBox, false, false, MARGIN);

    TileBox.pack_start(SpinsHBox, false, false, MARGIN);

    {
        /*#### Radio buttons to control spacing manually or to fit selection bbox ####*/
        SpaceByBBoxRadioButton.set_label(_("Fit into selection box"));
        SpaceByBBoxRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::Spacing_button_changed));
        SpacingGroup = SpaceByBBoxRadioButton.get_group();

        SpacingVBox.pack_start(SpaceByBBoxRadioButton, false, false, MARGIN);

        SpaceManualRadioButton.set_label(_("Set spacing:"));
        SpaceManualRadioButton.set_group(SpacingGroup);
        SpaceManualRadioButton.signal_toggled().connect(sigc::mem_fun(*this, &TileDialog::Spacing_button_changed));
        SpacingVBox.pack_start(SpaceManualRadioButton, false, false, MARGIN);

        TileBox.pack_start(SpacingVBox, false, false, MARGIN);
    }

    {
        /*#### Y Padding ####*/

        GtkWidget *i = sp_icon_new (Inkscape::ICON_SIZE_MENU, "clonetiler_per_row");
        YPadBox.pack_start (*(Glib::wrap(i)), false, false, MARGIN);

        YPadSpinner.set_digits(1);
        YPadSpinner.set_increments(0.2, 2);
        YPadSpinner.set_range(-10000, 10000);
        double YPad = prefs_get_double_attribute ("dialogs.gridtiler", "YPad", 15);
        YPadSpinner.set_value(YPad);
        YPadBox.pack_start(YPadSpinner, true, true, MARGIN);
        tips.set_tip(YPadSpinner, _("Vertical spacing between rows (px units)"));
        YPadSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialog::on_ypad_spinbutton_changed));
        gtk_size_group_add_widget(_col1, (GtkWidget *) YPadBox.gobj());

        SizesHBox.pack_start(YPadBox, false, false, MARGIN);
    }

    {
    Gtk::HBox *spacer = new Gtk::HBox;
    SizesHBox.pack_start(*spacer, false, false, 0);
    gtk_size_group_add_widget(_col2, (GtkWidget *) spacer->gobj());
    }

    {
        /*#### X padding ####*/

        GtkWidget *i = sp_icon_new (Inkscape::ICON_SIZE_MENU, "clonetiler_per_column");
        XPadBox.pack_start (*(Glib::wrap(i)), false, false, MARGIN);

        XPadSpinner.set_digits(1);
        XPadSpinner.set_increments(0.2, 2);
        XPadSpinner.set_range(-10000, 10000);
        double XPad = prefs_get_double_attribute ("dialogs.gridtiler", "XPad", 15);
        XPadSpinner.set_value(XPad);
        XPadBox.pack_start(XPadSpinner, true, true, MARGIN);
        tips.set_tip(XPadSpinner, _("Horizontal spacing between columns (px units)"));
        XPadSpinner.signal_changed().connect(sigc::mem_fun(*this, &TileDialog::on_xpad_spinbutton_changed));
        gtk_size_group_add_widget(_col3, (GtkWidget *) XPadBox.gobj());

        SizesHBox.pack_start(XPadBox, false, false, MARGIN);
    }


    TileBox.pack_start(SizesHBox, false, false, MARGIN);

    contents->pack_start(TileBox);

    double SpacingType = prefs_get_double_attribute ("dialogs.gridtiler", "SpacingType", 15);
    if (SpacingType>0) {
        ManualSpacing=true;
    } else {
        ManualSpacing=false;
    }
    SpaceManualRadioButton.set_active(ManualSpacing);
    SpaceByBBoxRadioButton.set_active(!ManualSpacing);
    SizesHBox.set_sensitive (ManualSpacing);

    //## The OK button
    TileOkButton = addResponseButton(_("Arrange"), GTK_RESPONSE_APPLY);
    tips.set_tip((*TileOkButton), _("Arrange selected objects"));

    show_all_children();
}






} //namespace Dialog
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :




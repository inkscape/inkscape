/**
 * \brief Object Transformation dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/stock.h>
#include <gtkmm/dialog.h>

#include "document.h"
#include "desktop-handles.h"
#include "transformation.h"
#include "align-and-distribute.h"
#include "libnr/nr-matrix-ops.h"
#include "inkscape.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "sp-item-transform.h"
#include "macros.h"
#include "sp-item.h"
#include "util/glib-list-iterators.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

void on_selection_changed(Inkscape::Application */*inkscape*/, Inkscape::Selection *selection, Transformation *daad)
{
    int page = daad->getCurrentPage();
    daad->updateSelection((Inkscape::UI::Dialog::Transformation::PageType)page, selection);
}

void on_selection_modified( Inkscape::Application */*inkscape*/,
                            Inkscape::Selection *selection,
                            guint /*flags*/,
                            Transformation *daad )
{
    int page = daad->getCurrentPage();
    daad->updateSelection((Inkscape::UI::Dialog::Transformation::PageType)page, selection);
}

/*########################################################################
# C O N S T R U C T O R
########################################################################*/

/**
 * Constructor for Transformation.  This does the initialization
 * and layout of the dialog used for transforming SVG objects.  It
 * consists of 5 pages for the 5 operations it handles:
 * 'Move' allows x,y translation of SVG objects
 * 'Scale' allows linear resizing of SVG objects
 * 'Rotate' allows rotating SVG objects by a degree
 * 'Skew' allows skewing SVG objects
 * 'Matrix' allows applying a generic affine transform on SVG objects,
 *     with the user specifying the 6 degrees of freedom manually.
 *
 * The dialog is implemented as a Gtk::Notebook with five pages.
 * The pages are implemented using Inkscape's NotebookPage which
 * is used to help make sure all of Inkscape's notebooks follow
 * the same style.  We then populate the pages with our widgets,
 * we use the ScalarUnit class for this.
 *
 */
Transformation::Transformation()
    : UI::Widget::Panel ("", "dialogs.transformation", SP_VERB_DIALOG_TRANSFORM),
      _page_move              (4, 2),
      _page_scale             (4, 2),
      _page_rotate            (4, 2),
      _page_skew              (4, 2),
      _page_transform         (3, 3),
      _scalar_move_horizontal (_("_Horizontal"), _("Horizontal displacement (relative) or position (absolute)"), UNIT_TYPE_LINEAR,
                               "", "arrows_hor", &_units_move),
      _scalar_move_vertical   (_("_Vertical"),  _("Vertical displacement (relative) or position (absolute)"), UNIT_TYPE_LINEAR,
                               "", "arrows_ver", &_units_move),
      _scalar_scale_horizontal(_("_Width"), _("Horizontal size (absolute or percentage of current)"), UNIT_TYPE_DIMENSIONLESS,
                               "", "transform_scale_hor", &_units_scale),
      _scalar_scale_vertical  (_("_Height"),  _("Vertical size (absolute or percentage of current)"), UNIT_TYPE_DIMENSIONLESS,
                               "", "transform_scale_ver", &_units_scale),
      _scalar_rotate          (_("A_ngle"), _("Rotation angle (positive = counterclockwise)"), UNIT_TYPE_RADIAL,
                               "", "transform_rotate", &_units_rotate),
      _scalar_skew_horizontal (_("_Horizontal"), _("Horizontal skew angle (positive = counterclockwise), or absolute displacement, or percentage displacement"), UNIT_TYPE_LINEAR,
                               "", "transform_scew_hor", &_units_skew),
      _scalar_skew_vertical   (_("_Vertical"),  _("Vertical skew angle (positive = counterclockwise), or absolute displacement, or percentage displacement"),  UNIT_TYPE_LINEAR,
                               "", "transform_scew_ver", &_units_skew),

      _scalar_transform_a     ("_A", _("Transformation matrix element A")),
      _scalar_transform_b     ("_B", _("Transformation matrix element B")),
      _scalar_transform_c     ("_C", _("Transformation matrix element C")),
      _scalar_transform_d     ("_D", _("Transformation matrix element D")),
      _scalar_transform_e     ("_E", _("Transformation matrix element E")),
      _scalar_transform_f     ("_F", _("Transformation matrix element F")),

      _check_move_relative    (_("Rela_tive move"), _("Add the specified relative displacement to the current position; otherwise, edit the current absolute position directly")),
      _check_scale_proportional (_("Scale proportionally"), _("Preserve the width/height ratio of the scaled objects")),
      _check_apply_separately    (_("Apply to each _object separately"), _("Apply the scale/rotate/skew to each selected object separately; otherwise, transform the selection as a whole")),
      _check_replace_matrix    (_("Edit c_urrent matrix"), _("Edit the current transform= matrix; otherwise, post-multiply transform= by this matrix"))

{
    Gtk::Box *contents = _getContents();

    contents->set_spacing(0);

    // Notebook for individual transformations
    contents->pack_start(_notebook, true, true);

    _notebook.append_page(_page_move, _("_Move"), true);
    layoutPageMove();

    _notebook.append_page(_page_scale, _("_Scale"), true);
    layoutPageScale();

    _notebook.append_page(_page_rotate, _("_Rotate"), true);
    layoutPageRotate();

    _notebook.append_page(_page_skew, _("Ske_w"), true);
    layoutPageSkew();

    _notebook.append_page(_page_transform, _("Matri_x"), true);
    layoutPageTransform();

    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Transformation::onSwitchPage));

    // Apply separately
    contents->pack_start(_check_apply_separately, true, true);
    _check_apply_separately.set_active(prefs_get_int_attribute_limited ("dialogs.transformation", "applyseparately", 0, 0, 1));
    _check_apply_separately.signal_toggled().connect(sigc::mem_fun(*this, &Transformation::onApplySeparatelyToggled));

    // make sure all spinbuttons activate Apply on pressing Enter
      ((Gtk::Entry *) (_scalar_move_horizontal.getWidget()))->set_activates_default(true);
      ((Gtk::Entry *) (_scalar_move_vertical.getWidget()))->set_activates_default(true);
      ((Gtk::Entry *) (_scalar_scale_horizontal.getWidget()))->set_activates_default(true);
      ((Gtk::Entry *) (_scalar_scale_vertical.getWidget()))->set_activates_default(true);
      ((Gtk::Entry *) (_scalar_rotate.getWidget()))->set_activates_default(true);
      ((Gtk::Entry *) (_scalar_skew_horizontal.getWidget()))->set_activates_default(true);
      ((Gtk::Entry *) (_scalar_skew_vertical.getWidget()))->set_activates_default(true);

    updateSelection(PAGE_MOVE, _getSelection());

    resetButton = addResponseButton(Gtk::Stock::CLEAR, 0);
    if (resetButton) {
        _tooltips.set_tip((*resetButton), _("Reset the values on the current tab to defaults"));
        resetButton->set_sensitive(true);
        resetButton->signal_clicked().connect(sigc::mem_fun(*this, &Transformation::onClear));
    }

    applyButton = addResponseButton(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);
    if (applyButton) {
        _tooltips.set_tip((*applyButton), _("Apply transformation to selection"));
        applyButton->set_sensitive(false);
    }

    // Connect to the global selection changed & modified signals
    g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (on_selection_changed), this);
    g_signal_connect (G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (on_selection_modified), this);

    show_all_children();
}

Transformation::~Transformation()
{
    sp_signal_disconnect_by_data (G_OBJECT (INKSCAPE), this);
}


/*########################################################################
# U T I L I T Y
########################################################################*/

void
Transformation::presentPage(Transformation::PageType page)
{
    _notebook.set_current_page(page);
    show();
    present();
}




/*########################################################################
# S E T U P   L A Y O U T
########################################################################*/


void
Transformation::layoutPageMove()
{
    _units_move.setUnitType(UNIT_TYPE_LINEAR);
    _scalar_move_horizontal.initScalar(-1e6, 1e6);
    _scalar_move_horizontal.setDigits(3);
    _scalar_move_horizontal.setIncrements(0.1, 1.0);

    _scalar_move_vertical.initScalar(-1e6, 1e6);
    _scalar_move_vertical.setDigits(3);
    _scalar_move_vertical.setIncrements(0.1, 1.0);

    //_scalar_move_vertical.set_label_image( INKSCAPE_STOCK_ARROWS_HOR );
    _page_move.table()
        .attach(_scalar_move_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);

    _page_move.table()
        .attach(_units_move, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _scalar_move_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveValueChanged));

    //_scalar_move_vertical.set_label_image( INKSCAPE_STOCK_ARROWS_VER );
    _page_move.table()
        .attach(_scalar_move_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);

    _scalar_move_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveValueChanged));

    // Relative moves
    _page_move.table()
        .attach(_check_move_relative, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
    _check_move_relative.set_active(true);
    _check_move_relative.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveRelativeToggled));
}

void
Transformation::layoutPageScale()
{
    _units_scale.setUnitType(UNIT_TYPE_DIMENSIONLESS);
    _units_scale.setUnitType(UNIT_TYPE_LINEAR);

    _scalar_scale_horizontal.initScalar(-1e6, 1e6);
    _scalar_scale_horizontal.setValue(100.0, "%");
    _scalar_scale_horizontal.setDigits(3);
    _scalar_scale_horizontal.setIncrements(0.1, 1.0);
    _scalar_scale_horizontal.setAbsoluteIsIncrement(true);
    _scalar_scale_horizontal.setPercentageIsIncrement(true);

    _scalar_scale_vertical.initScalar(-1e6, 1e6);
    _scalar_scale_vertical.setValue(100.0, "%");
    _scalar_scale_vertical.setDigits(3);
    _scalar_scale_vertical.setIncrements(0.1, 1.0);
    _scalar_scale_vertical.setAbsoluteIsIncrement(true);
    _scalar_scale_vertical.setPercentageIsIncrement(true);

    _page_scale.table()
        .attach(_scalar_scale_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    _scalar_scale_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleXValueChanged));

    _page_scale.table()
        .attach(_units_scale, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_scale.table()
        .attach(_scalar_scale_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
    _scalar_scale_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleYValueChanged));

    _page_scale.table()
        .attach(_check_scale_proportional, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
    _check_scale_proportional.set_active(false);
    _check_scale_proportional.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleProportionalToggled));

    //TODO: add a widget for selecting the fixed point in scaling, or honour rotation center?
}

void
Transformation::layoutPageRotate()
{
    _units_rotate.setUnitType(UNIT_TYPE_RADIAL);

    _scalar_rotate.initScalar(-360.0, 360.0);
    _scalar_rotate.setDigits(3);
    _scalar_rotate.setIncrements(0.1, 1.0);

    _page_rotate.table()
        .attach(_scalar_rotate, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);

    _page_rotate.table()
        .attach(_units_rotate, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _scalar_rotate.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onRotateValueChanged));

    //TODO: honour rotation center?
}

void
Transformation::layoutPageSkew()
{
    _units_skew.setUnitType(UNIT_TYPE_LINEAR);
    _units_skew.setUnitType(UNIT_TYPE_DIMENSIONLESS);
    _units_skew.setUnitType(UNIT_TYPE_RADIAL);

    _scalar_skew_horizontal.initScalar(-1e6, 1e6);
    _scalar_skew_horizontal.setDigits(3);
    _scalar_skew_horizontal.setIncrements(0.1, 1.0);

    _scalar_skew_vertical.initScalar(-1e6, 1e6);
    _scalar_skew_vertical.setDigits(3);
    _scalar_skew_vertical.setIncrements(0.1, 1.0);

    _page_skew.table()
        .attach(_scalar_skew_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    _scalar_skew_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onSkewValueChanged));

    _page_skew.table()
        .attach(_units_skew, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_skew.table()
        .attach(_scalar_skew_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
    _scalar_skew_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onSkewValueChanged));

    //TODO: honour rotation center?
}



void
Transformation::layoutPageTransform()
{
    _scalar_transform_a.setWidgetSizeRequest(65, -1);
    _scalar_transform_a.setRange(-1e10, 1e10);
    _scalar_transform_a.setDigits(3);
    _scalar_transform_a.setIncrements(0.1, 1.0);
    _scalar_transform_a.setValue(1.0);
    _page_transform.table()
        .attach(_scalar_transform_a, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_a.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_b.setWidgetSizeRequest(65, -1);
    _scalar_transform_b.setRange(-1e10, 1e10);
    _scalar_transform_b.setDigits(3);
    _scalar_transform_b.setIncrements(0.1, 1.0);
    _scalar_transform_b.setValue(0.0);
    _page_transform.table()
        .attach(_scalar_transform_b, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_b.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_c.setWidgetSizeRequest(65, -1);
    _scalar_transform_c.setRange(-1e10, 1e10);
    _scalar_transform_c.setDigits(3);
    _scalar_transform_c.setIncrements(0.1, 1.0);
    _scalar_transform_c.setValue(0.0);
    _page_transform.table()
        .attach(_scalar_transform_c, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_c.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_d.setWidgetSizeRequest(65, -1);
    _scalar_transform_d.setRange(-1e10, 1e10);
    _scalar_transform_d.setDigits(3);
    _scalar_transform_d.setIncrements(0.1, 1.0);
    _scalar_transform_d.setValue(1.0);
    _page_transform.table()
        .attach(_scalar_transform_d, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_d.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_e.setWidgetSizeRequest(65, -1);
    _scalar_transform_e.setRange(-1e10, 1e10);
    _scalar_transform_e.setDigits(3);
    _scalar_transform_e.setIncrements(0.1, 1.0);
    _scalar_transform_e.setValue(0.0);
    _page_transform.table()
        .attach(_scalar_transform_e, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_e.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_f.setWidgetSizeRequest(65, -1);
    _scalar_transform_f.setRange(-1e10, 1e10);
    _scalar_transform_f.setDigits(3);
    _scalar_transform_f.setIncrements(0.1, 1.0);
    _scalar_transform_f.setValue(0.0);
    _page_transform.table()
        .attach(_scalar_transform_f, 2, 3, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _scalar_transform_f.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));

    // Edit existing matrix
    _page_transform.table()
        .attach(_check_replace_matrix, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
    _check_replace_matrix.set_active(false);
    _check_replace_matrix.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onReplaceMatrixToggled));
}


/*########################################################################
# U P D A T E
########################################################################*/

void
Transformation::updateSelection(PageType page, Inkscape::Selection *selection)
{
    if (!selection || selection->isEmpty())
        return;

    switch (page) {
        case PAGE_MOVE: {
            updatePageMove(selection);
            break;
        }
        case PAGE_SCALE: {
            updatePageScale(selection);
            break;
        }
        case PAGE_ROTATE: {
            updatePageRotate(selection);
            break;
        }
        case PAGE_SKEW: {
            updatePageSkew(selection);
            break;
        }
        case PAGE_TRANSFORM: {
            updatePageTransform(selection);
            break;
        }
        case PAGE_QTY: {
            break;
        }
    }

    setResponseSensitive(Gtk::RESPONSE_APPLY,
                         selection && !selection->isEmpty());
}

void
Transformation::onSwitchPage(GtkNotebookPage */*page*/,
                                   guint pagenum)
{
    updateSelection((PageType)pagenum, sp_desktop_selection(getDesktop()));
}


void
Transformation::updatePageMove(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        if (!_check_move_relative.get_active()) {
            boost::optional<NR::Rect> bbox = selection->bounds();
            if (bbox) {
                double x = bbox->min()[Geom::X];
                double y = bbox->min()[Geom::Y];

                _scalar_move_horizontal.setValue(x, "px");
                _scalar_move_vertical.setValue(y, "px");
            }
        } else {
            // do nothing, so you can apply the same relative move to many objects in turn
        }
        _page_move.set_sensitive(true);
    } else {
        _page_move.set_sensitive(false);
    }
}

void
Transformation::updatePageScale(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        boost::optional<NR::Rect> bbox = selection->bounds();
        if (bbox) {
            double w = bbox->extent(Geom::X);
            double h = bbox->extent(Geom::Y);
            _scalar_scale_horizontal.setHundredPercent(w);
            _scalar_scale_vertical.setHundredPercent(h);
            onScaleXValueChanged(); // to update x/y proportionality if switch is on
            _page_scale.set_sensitive(true);
        } else {
            _page_scale.set_sensitive(false);
        }
    } else {
        _page_scale.set_sensitive(false);
    }
}

void
Transformation::updatePageRotate(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        _page_rotate.set_sensitive(true);
    } else {
        _page_rotate.set_sensitive(false);
    }
}

void
Transformation::updatePageSkew(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        boost::optional<NR::Rect> bbox = selection->bounds();
        if (bbox) {
            double w = bbox->extent(Geom::X);
            double h = bbox->extent(Geom::Y);
            _scalar_skew_vertical.setHundredPercent(w);
            _scalar_skew_horizontal.setHundredPercent(h);
            _page_skew.set_sensitive(true);
        } else {
            _page_skew.set_sensitive(false);
        }
    } else {
        _page_skew.set_sensitive(false);
    }
}

void
Transformation::updatePageTransform(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        if (_check_replace_matrix.get_active()) {
            Geom::Matrix current (SP_ITEM(selection->itemList()->data)->transform); // take from the first item in selection

            Geom::Matrix new_displayed = current;

            _scalar_transform_a.setValue(new_displayed[0]);
            _scalar_transform_b.setValue(new_displayed[1]);
            _scalar_transform_c.setValue(new_displayed[2]);
            _scalar_transform_d.setValue(new_displayed[3]);
            _scalar_transform_e.setValue(new_displayed[4]);
            _scalar_transform_f.setValue(new_displayed[5]);
        } else {
            // do nothing, so you can apply the same matrix to many objects in turn
        }
        _page_transform.set_sensitive(true);
    } else {
        _page_transform.set_sensitive(false);
    }
}





/*########################################################################
# A P P L Y
########################################################################*/



void
Transformation::_apply()
{
    Inkscape::Selection * const selection = _getSelection();
    if (!selection || selection->isEmpty())
        return;

    int const page = _notebook.get_current_page();

    switch (page) {
        case PAGE_MOVE: {
            applyPageMove(selection);
            break;
        }
        case PAGE_ROTATE: {
            applyPageRotate(selection);
            break;
        }
        case PAGE_SCALE: {
            applyPageScale(selection);
            break;
        }
        case PAGE_SKEW: {
            applyPageSkew(selection);
            break;
        }
        case PAGE_TRANSFORM: {
            applyPageTransform(selection);
            break;
        }
    }

    //Let's play with never turning this off
    //setResponseSensitive(Gtk::RESPONSE_APPLY, false);
}

void
Transformation::applyPageMove(Inkscape::Selection *selection)
{
    double x = _scalar_move_horizontal.getValue("px");
    double y = _scalar_move_vertical.getValue("px");

    if (prefs_get_int_attribute_limited ("dialogs.transformation", "applyseparately", 0, 0, 1) == 0) {
        // move selection as a whole
        if (_check_move_relative.get_active()) {
            sp_selection_move_relative(selection, x, y);
        } else {
            boost::optional<NR::Rect> bbox = selection->bounds();
            if (bbox) {
                sp_selection_move_relative(selection,
                                           x - bbox->min()[Geom::X], y - bbox->min()[Geom::Y]);
            }
        }
    } else {

        if (_check_move_relative.get_active()) {
            // shift each object relatively to the previous one
            using Inkscape::Util::GSListConstIterator;
            std::list<SPItem *> selected;
            selected.insert<GSListConstIterator<SPItem *> >(selected.end(), selection->itemList(), NULL);
            if (selected.empty()) return;

            if (fabs(x) > 1e-6) {
                std::vector< BBoxSort  > sorted;
                for (std::list<SPItem *>::iterator it(selected.begin());
                     it != selected.end();
                     ++it)
                {
                    boost::optional<NR::Rect> bbox = sp_item_bbox_desktop(*it);
                    if (bbox) {
                        sorted.push_back(BBoxSort(*it, to_2geom(*bbox), Geom::X, x > 0? 1. : 0., x > 0? 0. : 1.));
                    }
                }
                //sort bbox by anchors
                std::sort(sorted.begin(), sorted.end());

                double move = x;
                for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
                      it < sorted.end();
                      it ++ )
                {
                    sp_item_move_rel(it->item, Geom::Translate(move, 0));
                    // move each next object by x relative to previous
                    move += x;
                }
            }
            if (fabs(y) > 1e-6) {
                std::vector< BBoxSort  > sorted;
                for (std::list<SPItem *>::iterator it(selected.begin());
                     it != selected.end();
                     ++it)
                {
                    boost::optional<NR::Rect> bbox = sp_item_bbox_desktop(*it);
                    if (bbox) {
                        sorted.push_back(BBoxSort(*it, to_2geom(*bbox), Geom::Y, y > 0? 1. : 0., y > 0? 0. : 1.));
                    }
                }
                //sort bbox by anchors
                std::sort(sorted.begin(), sorted.end());

                double move = y;
                for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
                      it < sorted.end();
                      it ++ )
                {
                    sp_item_move_rel(it->item, Geom::Translate(0, move));
                    // move each next object by x relative to previous
                    move += y;
                }
            }
        } else {
            boost::optional<NR::Rect> bbox = selection->bounds();
            if (bbox) {
                sp_selection_move_relative(selection,
                                           x - bbox->min()[Geom::X], y - bbox->min()[Geom::Y]);
            }
        }
    }

    sp_document_done ( sp_desktop_document (selection->desktop()) , SP_VERB_DIALOG_TRANSFORM,
                       _("Move"));
}

void
Transformation::applyPageScale(Inkscape::Selection *selection)
{
    double scaleX = _scalar_scale_horizontal.getValue("px");
    double scaleY = _scalar_scale_vertical.getValue("px");

    if (prefs_get_int_attribute_limited ("dialogs.transformation", "applyseparately", 0, 0, 1) == 1) {
        for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
            SPItem *item = SP_ITEM(l->data);
            Geom::Scale scale (0,0);
            // the values are increments!
            if (_units_scale.isAbsolute()) {
                boost::optional<NR::Rect> bbox(sp_item_bbox_desktop(item));
                if (bbox) {
                    double new_width = scaleX;
                    if (fabs(new_width) < 1e-6) new_width = 1e-6; // not 0, as this would result in a nasty no-bbox object
                    double new_height = scaleY;
                    if (fabs(new_height) < 1e-6) new_height = 1e-6;
                    scale = Geom::Scale(new_width / bbox->extent(Geom::X), new_height / bbox->extent(Geom::Y));
                }
            } else {
                double new_width = scaleX;
                if (fabs(new_width) < 1e-6) new_width = 1e-6;
                double new_height = scaleY;
                if (fabs(new_height) < 1e-6) new_height = 1e-6;
                scale = Geom::Scale(new_width / 100.0, new_height / 100.0);
            }
            sp_item_scale_rel (item, scale);
        }
    } else {
        boost::optional<NR::Rect> bbox(selection->bounds());
        if (bbox) {
            Geom::Point center(bbox->midpoint()); // use rotation center?
            Geom::Scale scale (0,0);
            // the values are increments!
            if (_units_scale.isAbsolute()) {
                double new_width = scaleX;
                if (fabs(new_width) < 1e-6) new_width = 1e-6;
                double new_height = scaleY;
                if (fabs(new_height) < 1e-6) new_height = 1e-6;
                scale = Geom::Scale(new_width / bbox->extent(Geom::X), new_height / bbox->extent(Geom::Y));
            } else {
                double new_width = scaleX;
                if (fabs(new_width) < 1e-6) new_width = 1e-6;
                double new_height = scaleY;
                if (fabs(new_height) < 1e-6) new_height = 1e-6;
                scale = Geom::Scale(new_width / 100.0, new_height / 100.0);
            }
            sp_selection_scale_relative(selection, center, scale);
        }
    }

    sp_document_done(sp_desktop_document(selection->desktop()), SP_VERB_DIALOG_TRANSFORM,
                     _("Scale"));
}

void
Transformation::applyPageRotate(Inkscape::Selection *selection)
{
    double angle = _scalar_rotate.getValue("deg");

    if (prefs_get_int_attribute_limited ("dialogs.transformation", "applyseparately", 0, 0, 1) == 1) {
        for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
            SPItem *item = SP_ITEM(l->data);
            sp_item_rotate_rel(item, Geom::Rotate (angle*M_PI/180.0));
        }
    } else {
        boost::optional<Geom::Point> center = selection->center();
        if (center) {
            sp_selection_rotate_relative(selection, *center, angle);
        }
    }

    sp_document_done(sp_desktop_document(selection->desktop()), SP_VERB_DIALOG_TRANSFORM,
                     _("Rotate"));
}

void
Transformation::applyPageSkew(Inkscape::Selection *selection)
{
    if (prefs_get_int_attribute_limited ("dialogs.transformation", "applyseparately", 0, 0, 1) == 1) {
        for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
            SPItem *item = SP_ITEM(l->data);

            if (!_units_skew.isAbsolute()) { // percentage
                double skewX = _scalar_skew_horizontal.getValue("%");
                double skewY = _scalar_skew_vertical.getValue("%");
                sp_item_skew_rel (item, 0.01*skewX, 0.01*skewY);
            } else if (_units_skew.isRadial()) { //deg or rad
                double angleX = _scalar_skew_horizontal.getValue("rad");
                double angleY = _scalar_skew_vertical.getValue("rad");
                double skewX = tan(-angleX);
                double skewY = tan(angleY);
                sp_item_skew_rel (item, skewX, skewY);
            } else { // absolute displacement
                double skewX = _scalar_skew_horizontal.getValue("px");
                double skewY = _scalar_skew_vertical.getValue("px");
                boost::optional<NR::Rect> bbox(sp_item_bbox_desktop(item));
                if (bbox) {
                    double width = bbox->extent(Geom::X);
                    double height = bbox->extent(Geom::Y);
                    sp_item_skew_rel (item, skewX/height, skewY/width);
                }
            }
        }
    } else { // transform whole selection
        boost::optional<NR::Rect> bbox = selection->bounds();
        boost::optional<Geom::Point> center = selection->center();

        if ( bbox && center ) {
            double width  = bbox->extent(Geom::X);
            double height = bbox->extent(Geom::Y);

            if (!_units_skew.isAbsolute()) { // percentage
                double skewX = _scalar_skew_horizontal.getValue("%");
                double skewY = _scalar_skew_vertical.getValue("%");
                sp_selection_skew_relative(selection, *center, 0.01*skewX, 0.01*skewY);
            } else if (_units_skew.isRadial()) { //deg or rad
                double angleX = _scalar_skew_horizontal.getValue("rad");
                double angleY = _scalar_skew_vertical.getValue("rad");
                double skewX = tan(-angleX);
                double skewY = tan(angleY);
                sp_selection_skew_relative(selection, *center, skewX, skewY);
            } else { // absolute displacement
                double skewX = _scalar_skew_horizontal.getValue("px");
                double skewY = _scalar_skew_vertical.getValue("px");
                sp_selection_skew_relative(selection, *center, skewX/height, skewY/width);
            }
        }
    }

    sp_document_done(sp_desktop_document(selection->desktop()), SP_VERB_DIALOG_TRANSFORM,
                     _("Skew"));
}


void
Transformation::applyPageTransform(Inkscape::Selection *selection)
{
    double a = _scalar_transform_a.getValue();
    double b = _scalar_transform_b.getValue();
    double c = _scalar_transform_c.getValue();
    double d = _scalar_transform_d.getValue();
    double e = _scalar_transform_e.getValue();
    double f = _scalar_transform_f.getValue();

    NR::Matrix displayed(a, b, c, d, e, f);

    if (_check_replace_matrix.get_active()) {
        for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
            SPItem *item = SP_ITEM(l->data);
            sp_item_set_item_transform(item, displayed);
            SP_OBJECT(item)->updateRepr();
        }
    } else {
        sp_selection_apply_affine(selection, displayed); // post-multiply each object's transform
    }

    sp_document_done(sp_desktop_document(selection->desktop()), SP_VERB_DIALOG_TRANSFORM,
                     _("Edit transformation matrix"));
}





/*########################################################################
# V A L U E - C H A N G E D    C A L L B A C K S
########################################################################*/

void
Transformation::onMoveValueChanged()
{
    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onMoveRelativeToggled()
{
    Inkscape::Selection *selection = _getSelection();

    if (!selection || selection->isEmpty())
        return;

    double x = _scalar_move_horizontal.getValue("px");
    double y = _scalar_move_vertical.getValue("px");

    //g_message("onMoveRelativeToggled: %f, %f px\n", x, y);

    boost::optional<NR::Rect> bbox = selection->bounds();

    if (bbox) {
        if (_check_move_relative.get_active()) {
            // From absolute to relative
            _scalar_move_horizontal.setValue(x - bbox->min()[Geom::X], "px");
            _scalar_move_vertical.setValue(  y - bbox->min()[Geom::Y], "px");
        } else {
            // From relative to absolute
            _scalar_move_horizontal.setValue(bbox->min()[Geom::X] + x, "px");
            _scalar_move_vertical.setValue(  bbox->min()[Geom::Y] + y, "px");
        }
    }

    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onScaleXValueChanged()
{
    if (_scalar_scale_horizontal.setProgrammatically) {
        _scalar_scale_horizontal.setProgrammatically = false;
        return;
    }

    setResponseSensitive(Gtk::RESPONSE_APPLY, true);

    if (_check_scale_proportional.get_active()) {
        if (!_units_scale.isAbsolute()) { // percentage, just copy over
            _scalar_scale_vertical.setValue(_scalar_scale_horizontal.getValue("%"));
        } else {
            double scaleXPercentage = _scalar_scale_horizontal.getAsPercentage();
            _scalar_scale_vertical.setFromPercentage (scaleXPercentage);
        }
    }
}

void
Transformation::onScaleYValueChanged()
{
    if (_scalar_scale_vertical.setProgrammatically) {
        _scalar_scale_vertical.setProgrammatically = false;
        return;
    }

    setResponseSensitive(Gtk::RESPONSE_APPLY, true);

    if (_check_scale_proportional.get_active()) {
        if (!_units_scale.isAbsolute()) { // percentage, just copy over
            _scalar_scale_horizontal.setValue(_scalar_scale_vertical.getValue("%"));
        } else {
            double scaleYPercentage = _scalar_scale_vertical.getAsPercentage();
            _scalar_scale_horizontal.setFromPercentage (scaleYPercentage);
        }
    }
}

void
Transformation::onRotateValueChanged()
{
    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onSkewValueChanged()
{
    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onTransformValueChanged()
{

    /*
    double a = _scalar_transform_a.getValue();
    double b = _scalar_transform_b.getValue();
    double c = _scalar_transform_c.getValue();
    double d = _scalar_transform_d.getValue();
    double e = _scalar_transform_e.getValue();
    double f = _scalar_transform_f.getValue();

    //g_message("onTransformValueChanged: (%f, %f, %f, %f, %f, %f)\n",
    //          a, b, c, d, e ,f);
    */

    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void
Transformation::onReplaceMatrixToggled()
{
    Inkscape::Selection *selection = _getSelection();

    if (!selection || selection->isEmpty())
        return;

    double a = _scalar_transform_a.getValue();
    double b = _scalar_transform_b.getValue();
    double c = _scalar_transform_c.getValue();
    double d = _scalar_transform_d.getValue();
    double e = _scalar_transform_e.getValue();
    double f = _scalar_transform_f.getValue();

    Geom::Matrix displayed (a, b, c, d, e, f);
    Geom::Matrix current = SP_ITEM(selection->itemList()->data)->transform; // take from the first item in selection

    Geom::Matrix new_displayed;
    if (_check_replace_matrix.get_active()) {
        new_displayed = current;
    } else {
        new_displayed = current.inverse() * displayed;
    }

    _scalar_transform_a.setValue(new_displayed[0]);
    _scalar_transform_b.setValue(new_displayed[1]);
    _scalar_transform_c.setValue(new_displayed[2]);
    _scalar_transform_d.setValue(new_displayed[3]);
    _scalar_transform_e.setValue(new_displayed[4]);
    _scalar_transform_f.setValue(new_displayed[5]);
}

void
Transformation::onScaleProportionalToggled()
{
    onScaleXValueChanged();
}


void
Transformation::onClear()
{
    int const page = _notebook.get_current_page();

    switch (page) {
    case PAGE_MOVE: {
        Inkscape::Selection *selection = _getSelection();
        if (!selection || selection->isEmpty() || _check_move_relative.get_active()) {
            _scalar_move_horizontal.setValue(0);
            _scalar_move_vertical.setValue(0);
        } else {
            boost::optional<NR::Rect> bbox = selection->bounds();
            if (bbox) {
                _scalar_move_horizontal.setValue(bbox->min()[Geom::X], "px");
                _scalar_move_vertical.setValue(bbox->min()[Geom::Y], "px");
            }
        }
        break;
    }
    case PAGE_ROTATE: {
        _scalar_rotate.setValue(0);
        break;
    }
    case PAGE_SCALE: {
        _scalar_scale_horizontal.setValue(100, "%");
        _scalar_scale_vertical.setValue(100, "%");
        break;
    }
    case PAGE_SKEW: {
        _scalar_skew_horizontal.setValue(0);
        _scalar_skew_vertical.setValue(0);
        break;
    }
    case PAGE_TRANSFORM: {
        _scalar_transform_a.setValue(1);
        _scalar_transform_b.setValue(0);
        _scalar_transform_c.setValue(0);
        _scalar_transform_d.setValue(1);
        _scalar_transform_e.setValue(0);
        _scalar_transform_f.setValue(0);
        break;
    }
    }
}

void
Transformation::onApplySeparatelyToggled()
{
    prefs_set_int_attribute ("dialogs.transformation", "applyseparately", _check_apply_separately.get_active()? 1 : 0);
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

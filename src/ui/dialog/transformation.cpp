/**
 * @file
 * Transform dialog - implementation.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   buliabyak@gmail.com
 *   Abhishek Sharma
 *
 * Copyright (C) 2004, 2005 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <2geom/transforms.h>

#include "document.h"
#include "document-undo.h"
#include "desktop.h"

#include "transformation.h"
#include "align-and-distribute.h"
#include "inkscape.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "message-stack.h"
#include "verbs.h"
#include "preferences.h"
#include "sp-namedview.h"
#include "sp-item-transform.h"
#include "macros.h"
#include "sp-item.h"
#include "ui/icon-names.h"
#include "widgets/icon.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

static void on_selection_changed(Inkscape::Selection *selection, Transformation *daad)
{
    int page = daad->getCurrentPage();
    daad->updateSelection((Inkscape::UI::Dialog::Transformation::PageType)page, selection);
}

static void on_selection_modified(Inkscape::Selection *selection, Transformation *daad)
{
    int page = daad->getCurrentPage();
    daad->updateSelection((Inkscape::UI::Dialog::Transformation::PageType)page, selection);
}

/*########################################################################
# C O N S T R U C T O R
########################################################################*/

Transformation::Transformation()
    : UI::Widget::Panel ("", "/dialogs/transformation", SP_VERB_DIALOG_TRANSFORM),
      _page_move              (4, 2),
      _page_scale             (4, 2),
      _page_rotate            (4, 2),
      _page_skew              (4, 2),
      _page_transform         (3, 3),
      _scalar_move_horizontal (_("_Horizontal:"), _("Horizontal displacement (relative) or position (absolute)"), UNIT_TYPE_LINEAR,
                               "", "transform-move-horizontal", &_units_move),
      _scalar_move_vertical   (_("_Vertical:"),  _("Vertical displacement (relative) or position (absolute)"), UNIT_TYPE_LINEAR,
                               "", "transform-move-vertical", &_units_move),
      _scalar_scale_horizontal(_("_Width:"), _("Horizontal size (absolute or percentage of current)"), UNIT_TYPE_DIMENSIONLESS,
                               "", "transform-scale-horizontal", &_units_scale),
      _scalar_scale_vertical  (_("_Height:"),  _("Vertical size (absolute or percentage of current)"), UNIT_TYPE_DIMENSIONLESS,
                               "", "transform-scale-vertical", &_units_scale),
      _scalar_rotate          (_("A_ngle:"), _("Rotation angle (positive = counterclockwise)"), UNIT_TYPE_RADIAL,
                               "", "transform-rotate", &_units_rotate),
      _scalar_skew_horizontal (_("_Horizontal:"), _("Horizontal skew angle (positive = counterclockwise), or absolute displacement, or percentage displacement"), UNIT_TYPE_LINEAR,
                               "", "transform-skew-horizontal", &_units_skew),
      _scalar_skew_vertical   (_("_Vertical:"),  _("Vertical skew angle (positive = counterclockwise), or absolute displacement, or percentage displacement"),  UNIT_TYPE_LINEAR,
                               "", "transform-skew-vertical", &_units_skew),

      _scalar_transform_a     ("_A:", _("Transformation matrix element A")),
      _scalar_transform_b     ("_B:", _("Transformation matrix element B")),
      _scalar_transform_c     ("_C:", _("Transformation matrix element C")),
      _scalar_transform_d     ("_D:", _("Transformation matrix element D")),
      _scalar_transform_e     ("_E:", _("Transformation matrix element E")),
      _scalar_transform_f     ("_F:", _("Transformation matrix element F")),

      _counterclockwise_rotate (),
      _clockwise_rotate (),

      _check_move_relative    (_("Rela_tive move"), _("Add the specified relative displacement to the current position; otherwise, edit the current absolute position directly")),
      _check_scale_proportional (_("_Scale proportionally"), _("Preserve the width/height ratio of the scaled objects")),
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
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _check_apply_separately.set_active(prefs->getBool("/dialogs/transformation/applyseparately"));
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
        resetButton->set_tooltip_text(_("Reset the values on the current tab to defaults"));
        resetButton->set_sensitive(true);
        resetButton->signal_clicked().connect(sigc::mem_fun(*this, &Transformation::onClear));
    }

    applyButton = addResponseButton(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);
    if (applyButton) {
        applyButton->set_tooltip_text(_("Apply transformation to selection"));
        applyButton->set_sensitive(false);
    }

    // Connect to the global selection changed & modified signals
    _selChangeConn = INKSCAPE.signal_selection_changed.connect(sigc::bind(sigc::ptr_fun(&on_selection_changed), this));
    _selModifyConn = INKSCAPE.signal_selection_modified.connect(sigc::hide<1>(sigc::bind(sigc::ptr_fun(&on_selection_modified), this)));

    _desktopChangeConn = _deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &Transformation::setDesktop) );
    _deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children();
}

Transformation::~Transformation()
{
    _selModifyConn.disconnect();
    _selChangeConn.disconnect();   
    _desktopChangeConn.disconnect();
    _deskTrack.disconnect();
}

void Transformation::setTargetDesktop(SPDesktop *desktop)
{
    if (_desktop != desktop) {
        _desktop = desktop;
    }
}

/*########################################################################
# U T I L I T Y
########################################################################*/

void Transformation::presentPage(Transformation::PageType page)
{
    _notebook.set_current_page(page);
    show();
    present();
}




/*########################################################################
# S E T U P   L A Y O U T
########################################################################*/


void Transformation::layoutPageMove()
{
    _units_move.setUnitType(UNIT_TYPE_LINEAR);
    
    // Setting default unit to document unit
    SPDesktop *dt = getDesktop();
    SPNamedView *nv = dt->getNamedView();
    if (nv->display_units) {
        _units_move.setUnit(nv->display_units->abbr);
    }
    
    _scalar_move_horizontal.initScalar(-1e6, 1e6);
    _scalar_move_horizontal.setDigits(3);
    _scalar_move_horizontal.setIncrements(0.1, 1.0);

    _scalar_move_vertical.initScalar(-1e6, 1e6);
    _scalar_move_vertical.setDigits(3);
    _scalar_move_vertical.setIncrements(0.1, 1.0);

    //_scalar_move_vertical.set_label_image( INKSCAPE_STOCK_ARROWS_HOR );
    
#if WITH_GTKMM_3_0
    _page_move.table().attach(_scalar_move_horizontal, 0, 0, 2, 1);
    _page_move.table().attach(_units_move,             2, 0, 1, 1);
#else
    _page_move.table()
        .attach(_scalar_move_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);

    _page_move.table()
        .attach(_units_move, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_move_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveValueChanged));

    //_scalar_move_vertical.set_label_image( INKSCAPE_STOCK_ARROWS_VER );
#if WITH_GTKMM_3_0
    _page_move.table().attach(_scalar_move_vertical, 0, 1, 2, 1);
#else
    _page_move.table()
        .attach(_scalar_move_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
#endif

    _scalar_move_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveValueChanged));

    // Relative moves
#if WITH_GTKMM_3_0
    _page_move.table().attach(_check_move_relative, 0, 2, 2, 1);
#else
    _page_move.table()
        .attach(_check_move_relative, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
#endif

    _check_move_relative.set_active(true);
    _check_move_relative.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onMoveRelativeToggled));
}

void Transformation::layoutPageScale()
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

#if WITH_GTKMM_3_0
    _page_scale.table().attach(_scalar_scale_horizontal, 0, 0, 2, 1);
#else
    _page_scale.table()
        .attach(_scalar_scale_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
#endif

    _scalar_scale_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleXValueChanged));

#if WITH_GTKMM_3_0
    _page_scale.table().attach(_units_scale,           2, 0, 1, 1);
    _page_scale.table().attach(_scalar_scale_vertical, 0, 1, 2, 1);
#else
    _page_scale.table()
        .attach(_units_scale, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_scale.table()
        .attach(_scalar_scale_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
#endif

    _scalar_scale_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleYValueChanged));

#if WITH_GTKMM_3_0
    _page_scale.table().attach(_check_scale_proportional, 0, 2, 2, 1);
#else
    _page_scale.table()
        .attach(_check_scale_proportional, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
#endif

    _check_scale_proportional.set_active(false);
    _check_scale_proportional.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onScaleProportionalToggled));

    //TODO: add a widget for selecting the fixed point in scaling, or honour rotation center?
}

void Transformation::layoutPageRotate()
{
    _units_rotate.setUnitType(UNIT_TYPE_RADIAL);

    _scalar_rotate.initScalar(-360.0, 360.0);
    _scalar_rotate.setDigits(3);
    _scalar_rotate.setIncrements(0.1, 1.0);

    _counterclockwise_rotate.add(*manage( Glib::wrap(
            sp_icon_new(Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("object-rotate-left")))));
    _counterclockwise_rotate.set_mode(false);
    _counterclockwise_rotate.set_relief(Gtk::RELIEF_NONE);
    _counterclockwise_rotate.set_tooltip_text(_("Rotate in a counterclockwise direction"));

    _clockwise_rotate.add(*manage( Glib::wrap(
            sp_icon_new(Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("object-rotate-right")))));
    _clockwise_rotate.set_mode(false);
    _clockwise_rotate.set_relief(Gtk::RELIEF_NONE);
    _clockwise_rotate.set_tooltip_text(_("Rotate in a clockwise direction"));

    Gtk::RadioButton::Group group = _counterclockwise_rotate.get_group();
    _clockwise_rotate.set_group(group);

#if WITH_GTKMM_3_0
    _page_rotate.table().attach(_scalar_rotate,           0, 0, 2, 1);
    _page_rotate.table().attach(_units_rotate,            2, 0, 1, 1);
    _page_rotate.table().attach(_counterclockwise_rotate, 3, 0, 1, 1);
    _page_rotate.table().attach(_clockwise_rotate,        4, 0, 1, 1);
#else
    _page_rotate.table()
        .attach(_scalar_rotate, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);

    _page_rotate.table()
        .attach(_units_rotate, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_rotate.table()
        .attach(_counterclockwise_rotate, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_rotate.table()
        .attach(_clockwise_rotate, 4, 5, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
#endif

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/dialogs/transformation/rotateCounterClockwise", TRUE)) {
        _counterclockwise_rotate.set_active();
        onRotateCounterclockwiseClicked();
    } else {
        _clockwise_rotate.set_active();
        onRotateClockwiseClicked();
    }

    _scalar_rotate.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onRotateValueChanged));

    _counterclockwise_rotate.signal_clicked().connect(sigc::mem_fun(*this, &Transformation::onRotateCounterclockwiseClicked));
    _clockwise_rotate.signal_clicked().connect(sigc::mem_fun(*this, &Transformation::onRotateClockwiseClicked));

    //TODO: honour rotation center?
}

void Transformation::layoutPageSkew()
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

#if WITH_GTKMM_3_0
    _page_skew.table().attach(_scalar_skew_horizontal, 0, 0, 2, 1);
#else
    _page_skew.table()
        .attach(_scalar_skew_horizontal, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
#endif

    _scalar_skew_horizontal.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onSkewValueChanged));

#if WITH_GTKMM_3_0
    _page_skew.table().attach(_units_skew,           2, 0, 1, 1);
    _page_skew.table().attach(_scalar_skew_vertical, 0, 1, 2, 1);
#else
    _page_skew.table()
        .attach(_units_skew, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _page_skew.table()
        .attach(_scalar_skew_vertical, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
#endif

    _scalar_skew_vertical.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onSkewValueChanged));

    //TODO: honour rotation center?
}



void Transformation::layoutPageTransform()
{
    _scalar_transform_a.setWidgetSizeRequest(65, -1);
    _scalar_transform_a.setRange(-1e10, 1e10);
    _scalar_transform_a.setDigits(3);
    _scalar_transform_a.setIncrements(0.1, 1.0);
    _scalar_transform_a.setValue(1.0);

#if WITH_GTKMM_3_0
    _page_transform.table().attach(_scalar_transform_a, 0, 0, 1, 1);
#else
    _page_transform.table()
        .attach(_scalar_transform_a, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_transform_a.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));

    _scalar_transform_b.setWidgetSizeRequest(65, -1);
    _scalar_transform_b.setRange(-1e10, 1e10);
    _scalar_transform_b.setDigits(3);
    _scalar_transform_b.setIncrements(0.1, 1.0);
    _scalar_transform_b.setValue(0.0);

#if WITH_GTKMM_3_0
    _page_transform.table().attach(_scalar_transform_b, 0, 1, 1, 1);
#else
    _page_transform.table()
        .attach(_scalar_transform_b, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_transform_b.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));

    _scalar_transform_c.setWidgetSizeRequest(65, -1);
    _scalar_transform_c.setRange(-1e10, 1e10);
    _scalar_transform_c.setDigits(3);
    _scalar_transform_c.setIncrements(0.1, 1.0);
    _scalar_transform_c.setValue(0.0);

#if WITH_GTKMM_3_0
    _page_transform.table().attach(_scalar_transform_c, 1, 0, 1, 1);
#else
    _page_transform.table()
        .attach(_scalar_transform_c, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_transform_c.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_d.setWidgetSizeRequest(65, -1);
    _scalar_transform_d.setRange(-1e10, 1e10);
    _scalar_transform_d.setDigits(3);
    _scalar_transform_d.setIncrements(0.1, 1.0);
    _scalar_transform_d.setValue(1.0);

#if WITH_GTKMM_3_0
    _page_transform.table().attach(_scalar_transform_d, 1, 1, 1, 1);
#else
    _page_transform.table()
        .attach(_scalar_transform_d, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_transform_d.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_e.setWidgetSizeRequest(65, -1);
    _scalar_transform_e.setRange(-1e10, 1e10);
    _scalar_transform_e.setDigits(3);
    _scalar_transform_e.setIncrements(0.1, 1.0);
    _scalar_transform_e.setValue(0.0);

#if WITH_GTKMM_3_0
    _page_transform.table().attach(_scalar_transform_e, 2, 0, 1, 1);
#else
    _page_transform.table()
        .attach(_scalar_transform_e, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_transform_e.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));


    _scalar_transform_f.setWidgetSizeRequest(65, -1);
    _scalar_transform_f.setRange(-1e10, 1e10);
    _scalar_transform_f.setDigits(3);
    _scalar_transform_f.setIncrements(0.1, 1.0);
    _scalar_transform_f.setValue(0.0);

#if WITH_GTKMM_3_0
    _page_transform.table().attach(_scalar_transform_f, 2, 1, 1, 1);
#else
    _page_transform.table()
        .attach(_scalar_transform_f, 2, 3, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
#endif

    _scalar_transform_f.signal_value_changed()
        .connect(sigc::mem_fun(*this, &Transformation::onTransformValueChanged));

    // Edit existing matrix
#if WITH_GTKMM_3_0
    _page_transform.table().attach(_check_replace_matrix, 0, 2, 2, 1);
#else
    _page_transform.table()
        .attach(_check_replace_matrix, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
#endif

    _check_replace_matrix.set_active(false);
    _check_replace_matrix.signal_toggled()
        .connect(sigc::mem_fun(*this, &Transformation::onReplaceMatrixToggled));
}


/*########################################################################
# U P D A T E
########################################################################*/

void Transformation::updateSelection(PageType page, Inkscape::Selection *selection)
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

#if WITH_GTKMM_3_0
void Transformation::onSwitchPage(Gtk::Widget * /*page*/, guint pagenum)
#else
void Transformation::onSwitchPage(GtkNotebookPage * /*page*/, guint pagenum)
#endif
{
    updateSelection((PageType)pagenum, getDesktop()->getSelection());
}


void Transformation::updatePageMove(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        if (!_check_move_relative.get_active()) {
            Geom::OptRect bbox = selection->preferredBounds();
            if (bbox) {
                double x = bbox->min()[Geom::X];
                double y = bbox->min()[Geom::Y];

                double conversion = _units_move.getConversion("px");
                _scalar_move_horizontal.setValue(x / conversion);
                _scalar_move_vertical.setValue(y / conversion);
            }
        } else {
            // do nothing, so you can apply the same relative move to many objects in turn
        }
        _page_move.set_sensitive(true);
    } else {
        _page_move.set_sensitive(false);
    }
}

void Transformation::updatePageScale(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        Geom::OptRect bbox = selection->preferredBounds();
        if (bbox) {
            double w = bbox->dimensions()[Geom::X];
            double h = bbox->dimensions()[Geom::Y];
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

void Transformation::updatePageRotate(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        _page_rotate.set_sensitive(true);
    } else {
        _page_rotate.set_sensitive(false);
    }
}

void Transformation::updatePageSkew(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        Geom::OptRect bbox = selection->preferredBounds();
        if (bbox) {
            double w = bbox->dimensions()[Geom::X];
            double h = bbox->dimensions()[Geom::Y];
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

void Transformation::updatePageTransform(Inkscape::Selection *selection)
{
    if (selection && !selection->isEmpty()) {
        if (_check_replace_matrix.get_active()) {
            Geom::Affine current (selection->itemList()[0]->transform); // take from the first item in selection

            Geom::Affine new_displayed = current;

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



void Transformation::_apply()
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

void Transformation::applyPageMove(Inkscape::Selection *selection)
{
    double x = _scalar_move_horizontal.getValue("px");
    double y = _scalar_move_vertical.getValue("px");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/dialogs/transformation/applyseparately")) {
        // move selection as a whole
        if (_check_move_relative.get_active()) {
            sp_selection_move_relative(selection, x, y);
        } else {
            Geom::OptRect bbox = selection->preferredBounds();
            if (bbox) {
                sp_selection_move_relative(selection,
                                           x - bbox->min()[Geom::X], y - bbox->min()[Geom::Y]);
            }
        }
    } else {

        if (_check_move_relative.get_active()) {
            // shift each object relatively to the previous one
            std::vector<SPItem*> selected(selection->itemList());
            if (selected.empty()) return;

            if (fabs(x) > 1e-6) {
                std::vector< BBoxSort  > sorted;
                for (std::vector<SPItem*>::iterator it(selected.begin());
                     it != selected.end();
                     ++it)
                {
                	SPItem* item = *it;
                    Geom::OptRect bbox = item->desktopPreferredBounds();
                    if (bbox) {
                        sorted.push_back(BBoxSort(item, *bbox, Geom::X, x > 0? 1. : 0., x > 0? 0. : 1.));
                    }
                }
                //sort bbox by anchors
                std::sort(sorted.begin(), sorted.end());

                double move = x;
                for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
                      it < sorted.end();
                      ++it )
                {
                    sp_item_move_rel(it->item, Geom::Translate(move, 0));
                    // move each next object by x relative to previous
                    move += x;
                }
            }
            if (fabs(y) > 1e-6) {
                std::vector< BBoxSort  > sorted;
                for (std::vector<SPItem*>::iterator it(selected.begin());
                     it != selected.end();
                     ++it)
                {
                	SPItem* item = *it;
                	Geom::OptRect bbox = item->desktopPreferredBounds();
                    if (bbox) {
                        sorted.push_back(BBoxSort(item, *bbox, Geom::Y, y > 0? 1. : 0., y > 0? 0. : 1.));
                    }
                }
                //sort bbox by anchors
                std::sort(sorted.begin(), sorted.end());

                double move = y;
                for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
                      it < sorted.end();
                      ++it )
                {
                    sp_item_move_rel(it->item, Geom::Translate(0, move));
                    // move each next object by x relative to previous
                    move += y;
                }
            }
        } else {
            Geom::OptRect bbox = selection->preferredBounds();
            if (bbox) {
                sp_selection_move_relative(selection,
                                           x - bbox->min()[Geom::X], y - bbox->min()[Geom::Y]);
            }
        }
    }

    DocumentUndo::done( selection->desktop()->getDocument() , SP_VERB_DIALOG_TRANSFORM,
                        _("Move"));
}

void Transformation::applyPageScale(Inkscape::Selection *selection)
{
    double scaleX = _scalar_scale_horizontal.getValue("px");
    double scaleY = _scalar_scale_vertical.getValue("px");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool transform_stroke = prefs->getBool("/options/transform/stroke", true);
    bool preserve = prefs->getBool("/options/preservetransform/value", false);
    if (prefs->getBool("/dialogs/transformation/applyseparately")) {
    	std::vector<SPItem*> tmp=selection->itemList();
    	for(std::vector<SPItem*>::const_iterator i=tmp.begin();i!=tmp.end();++i){
            SPItem *item = *i;
            Geom::OptRect bbox_pref = item->desktopPreferredBounds();
            Geom::OptRect bbox_geom = item->desktopGeometricBounds();
            if (bbox_pref && bbox_geom) {
                double new_width = scaleX;
                double new_height = scaleY;
                // the values are increments!
                if (!_units_scale.isAbsolute()) { // Relative scaling, i.e in percent
                    new_width = scaleX/100 * bbox_pref->width();
                    new_height = scaleY/100  * bbox_pref->height();
                }
                if (fabs(new_width) < 1e-6) new_width = 1e-6; // not 0, as this would result in a nasty no-bbox object
                if (fabs(new_height) < 1e-6) new_height = 1e-6;

                double x0 = bbox_pref->midpoint()[Geom::X] - new_width/2;
                double y0 = bbox_pref->midpoint()[Geom::Y] - new_height/2;
                double x1 = bbox_pref->midpoint()[Geom::X] + new_width/2;
                double y1 = bbox_pref->midpoint()[Geom::Y] + new_height/2;

                Geom::Affine scaler = get_scale_transform_for_variable_stroke (*bbox_pref, *bbox_geom, transform_stroke, preserve, x0, y0, x1, y1);
                item->set_i2d_affine(item->i2dt_affine() * scaler);
                item->doWriteTransform(item->getRepr(), item->transform);
            }
        }
    } else {
        Geom::OptRect bbox_pref = selection->preferredBounds();
        Geom::OptRect bbox_geom = selection->geometricBounds();
        if (bbox_pref && bbox_geom) {
            // the values are increments!
            double new_width = scaleX;
            double new_height = scaleY;
            if (!_units_scale.isAbsolute()) { // Relative scaling, i.e in percent
                new_width = scaleX/100 * bbox_pref->width();
                new_height = scaleY/100 * bbox_pref->height();
            }
            if (fabs(new_width) < 1e-6) new_width = 1e-6;
            if (fabs(new_height) < 1e-6) new_height = 1e-6;

            double x0 = bbox_pref->midpoint()[Geom::X] - new_width/2;
            double y0 = bbox_pref->midpoint()[Geom::Y] - new_height/2;
            double x1 = bbox_pref->midpoint()[Geom::X] + new_width/2;
            double y1 = bbox_pref->midpoint()[Geom::Y] + new_height/2;
            Geom::Affine scaler = get_scale_transform_for_variable_stroke (*bbox_pref, *bbox_geom, transform_stroke, preserve, x0, y0, x1, y1);

            sp_selection_apply_affine(selection, scaler);
        }
    }

    DocumentUndo::done(selection->desktop()->getDocument(), SP_VERB_DIALOG_TRANSFORM,
                       _("Scale"));
}

void Transformation::applyPageRotate(Inkscape::Selection *selection)
{
    double angle = _scalar_rotate.getValue(DEG);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/dialogs/transformation/rotateCounterClockwise", TRUE)) {
        angle *= -1;
    }

    if (prefs->getBool("/dialogs/transformation/applyseparately")) {
    	std::vector<SPItem*> tmp=selection->itemList();
    	for(std::vector<SPItem*>::const_iterator i=tmp.begin();i!=tmp.end();++i){
            SPItem *item = *i;
            sp_item_rotate_rel(item, Geom::Rotate (angle*M_PI/180.0));
        }
    } else {
        boost::optional<Geom::Point> center = selection->center();
        if (center) {
            sp_selection_rotate_relative(selection, *center, angle);
        }
    }

    DocumentUndo::done(selection->desktop()->getDocument(), SP_VERB_DIALOG_TRANSFORM,
                       _("Rotate"));
}

void Transformation::applyPageSkew(Inkscape::Selection *selection)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/dialogs/transformation/applyseparately")) {
    	std::vector<SPItem*> items=selection->itemList();
    	for(std::vector<SPItem*>::const_iterator i = items.begin();i!=items.end();++i){
            SPItem *item = *i;

            if (!_units_skew.isAbsolute()) { // percentage
                double skewX = _scalar_skew_horizontal.getValue("%");
                double skewY = _scalar_skew_vertical.getValue("%");
                if (fabs(0.01*skewX*0.01*skewY - 1.0) < Geom::EPSILON) {
                    getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
                    return;
                }
                sp_item_skew_rel (item, 0.01*skewX, 0.01*skewY);
            } else if (_units_skew.isRadial()) { //deg or rad
                double angleX = _scalar_skew_horizontal.getValue("rad");
                double angleY = _scalar_skew_vertical.getValue("rad");
                if ((fabs(angleX - angleY + M_PI/2) < Geom::EPSILON)
                ||  (fabs(angleX - angleY - M_PI/2) < Geom::EPSILON)
                ||  (fabs((angleX - angleY)/3 + M_PI/2) < Geom::EPSILON)
                ||  (fabs((angleX - angleY)/3 - M_PI/2) < Geom::EPSILON)) {
                    getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
                    return;
                }
                double skewX = tan(-angleX);
                double skewY = tan(angleY);
                sp_item_skew_rel (item, skewX, skewY);
            } else { // absolute displacement
                double skewX = _scalar_skew_horizontal.getValue("px");
                double skewY = _scalar_skew_vertical.getValue("px");
                Geom::OptRect bbox = item->desktopPreferredBounds();
                if (bbox) {
                    double width = bbox->dimensions()[Geom::X];
                    double height = bbox->dimensions()[Geom::Y];
                    if (fabs(skewX*skewY - width*height) < Geom::EPSILON) {
                        getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
                        return;
                    }
                    sp_item_skew_rel (item, skewX/height, skewY/width);
                }
            }
        }
    } else { // transform whole selection
        Geom::OptRect bbox = selection->preferredBounds();
        boost::optional<Geom::Point> center = selection->center();

        if ( bbox && center ) {
            double width  = bbox->dimensions()[Geom::X];
            double height = bbox->dimensions()[Geom::Y];

            if (!_units_skew.isAbsolute()) { // percentage
                double skewX = _scalar_skew_horizontal.getValue("%");
                double skewY = _scalar_skew_vertical.getValue("%");
                if (fabs(0.01*skewX*0.01*skewY - 1.0) < Geom::EPSILON) {
                    getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
                    return;
                }
                sp_selection_skew_relative(selection, *center, 0.01*skewX, 0.01*skewY);
            } else if (_units_skew.isRadial()) { //deg or rad
                double angleX = _scalar_skew_horizontal.getValue("rad");
                double angleY = _scalar_skew_vertical.getValue("rad");
                if ((fabs(angleX - angleY + M_PI/2) < Geom::EPSILON)
                ||  (fabs(angleX - angleY - M_PI/2) < Geom::EPSILON)
                ||  (fabs((angleX - angleY)/3 + M_PI/2) < Geom::EPSILON)
                ||  (fabs((angleX - angleY)/3 - M_PI/2) < Geom::EPSILON)) {
                    getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
                    return;
                }
                double skewX = tan(-angleX);
                double skewY = tan(angleY);
                sp_selection_skew_relative(selection, *center, skewX, skewY);
            } else { // absolute displacement
                double skewX = _scalar_skew_horizontal.getValue("px");
                double skewY = _scalar_skew_vertical.getValue("px");
                if (fabs(skewX*skewY - width*height) < Geom::EPSILON) {
                    getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
                    return;
                }
                sp_selection_skew_relative(selection, *center, skewX/height, skewY/width);
            }
        }
    }

    DocumentUndo::done(selection->desktop()->getDocument(), SP_VERB_DIALOG_TRANSFORM,
                       _("Skew"));
}


void Transformation::applyPageTransform(Inkscape::Selection *selection)
{
    double a = _scalar_transform_a.getValue();
    double b = _scalar_transform_b.getValue();
    double c = _scalar_transform_c.getValue();
    double d = _scalar_transform_d.getValue();
    double e = _scalar_transform_e.getValue();
    double f = _scalar_transform_f.getValue();

    Geom::Affine displayed(a, b, c, d, e, f);
    if (displayed.isSingular()) {
        getDesktop()->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Transform matrix is singular, <b>not used</b>."));
        return;
    }

    if (_check_replace_matrix.get_active()) {
    	std::vector<SPItem*> tmp=selection->itemList();
    	for(std::vector<SPItem*>::const_iterator i=tmp.begin();i!=tmp.end();++i){
            SPItem *item = *i;
            item->set_item_transform(displayed);
            item->updateRepr();
        }
    } else {
        sp_selection_apply_affine(selection, displayed); // post-multiply each object's transform
    }

    DocumentUndo::done(selection->desktop()->getDocument(), SP_VERB_DIALOG_TRANSFORM,
                       _("Edit transformation matrix"));
}





/*########################################################################
# V A L U E - C H A N G E D    C A L L B A C K S
########################################################################*/

void Transformation::onMoveValueChanged()
{
    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void Transformation::onMoveRelativeToggled()
{
    Inkscape::Selection *selection = _getSelection();

    if (!selection || selection->isEmpty())
        return;

    double x = _scalar_move_horizontal.getValue("px");
    double y = _scalar_move_vertical.getValue("px");

    double conversion = _units_move.getConversion("px");

    //g_message("onMoveRelativeToggled: %f, %f px\n", x, y);

    Geom::OptRect bbox = selection->preferredBounds();

    if (bbox) {
        if (_check_move_relative.get_active()) {
            // From absolute to relative
            _scalar_move_horizontal.setValue((x - bbox->min()[Geom::X]) / conversion);
            _scalar_move_vertical.setValue((  y - bbox->min()[Geom::Y]) / conversion);
        } else {
            // From relative to absolute
            _scalar_move_horizontal.setValue((bbox->min()[Geom::X] + x) / conversion);
            _scalar_move_vertical.setValue((  bbox->min()[Geom::Y] + y) / conversion);
        }
    }

    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void Transformation::onScaleXValueChanged()
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

void Transformation::onScaleYValueChanged()
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

void Transformation::onRotateValueChanged()
{
    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void Transformation::onRotateCounterclockwiseClicked()
{
    _scalar_rotate.setTooltipText(_("Rotation angle (positive = counterclockwise)"));
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/dialogs/transformation/rotateCounterClockwise", TRUE);
}

void Transformation::onRotateClockwiseClicked()
{
    _scalar_rotate.setTooltipText(_("Rotation angle (positive = clockwise)"));
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/dialogs/transformation/rotateCounterClockwise", FALSE);
}

void Transformation::onSkewValueChanged()
{
    setResponseSensitive(Gtk::RESPONSE_APPLY, true);
}

void Transformation::onTransformValueChanged()
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

void Transformation::onReplaceMatrixToggled()
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

    Geom::Affine displayed (a, b, c, d, e, f);
    Geom::Affine current = selection->itemList()[0]->transform; // take from the first item in selection

    Geom::Affine new_displayed;
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

void Transformation::onScaleProportionalToggled()
{
    onScaleXValueChanged();
    if (_scalar_scale_vertical.setProgrammatically) {
        _scalar_scale_vertical.setProgrammatically = false;
    }
}


void Transformation::onClear()
{
    int const page = _notebook.get_current_page();

    switch (page) {
    case PAGE_MOVE: {
        Inkscape::Selection *selection = _getSelection();
        if (!selection || selection->isEmpty() || _check_move_relative.get_active()) {
            _scalar_move_horizontal.setValue(0);
            _scalar_move_vertical.setValue(0);
        } else {
            Geom::OptRect bbox = selection->preferredBounds();
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

void Transformation::onApplySeparatelyToggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/dialogs/transformation/applyseparately", _check_apply_separately.get_active());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

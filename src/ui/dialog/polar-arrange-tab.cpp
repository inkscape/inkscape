/**
 * @brief Arranges Objects into a Circle/Ellipse
 */
/* Authors:
 *   Declara Denis
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/dialog/polar-arrange-tab.h"
#include "ui/dialog/tile.h"

#include <2geom/transforms.h>
#include <glibmm/i18n.h>

#include "verbs.h"
#include "preferences.h"
#include "inkscape.h"

#include "selection.h"
#include "document.h"
#include "document-undo.h"
#include "sp-item.h"
#include "widgets/icon.h"
#include "desktop.h"
#include "sp-ellipse.h"
#include "sp-item-transform.h"
#include <gtkmm/messagedialog.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

PolarArrangeTab::PolarArrangeTab(ArrangeDialog *parent_)
	: parent(parent_),
#if WITH_GTKMM_3_0
	  parametersTable(),
#else
	  parametersTable(3, 3, false),
#endif
	  centerY("", C_("Polar arrange tab", "Y coordinate of the center"), UNIT_TYPE_LINEAR),
	  centerX("", C_("Polar arrange tab", "X coordinate of the center"), centerY),
	  radiusY("", C_("Polar arrange tab", "Y coordinate of the radius"), UNIT_TYPE_LINEAR),
	  radiusX("", C_("Polar arrange tab", "X coordinate of the radius"), radiusY),
	  angleY("", C_("Polar arrange tab", "Starting angle"), UNIT_TYPE_RADIAL),
	  angleX("", C_("Polar arrange tab", "End angle"), angleY)
{
	anchorPointLabel.set_text(C_("Polar arrange tab", "Anchor point:"));
	anchorPointLabel.set_alignment(Gtk::ALIGN_START);
	pack_start(anchorPointLabel, false, false);

	anchorBoundingBoxRadio.set_label(C_("Polar arrange tab", "Object's bounding box:"));
	anchorRadioGroup = anchorBoundingBoxRadio.get_group();
	anchorBoundingBoxRadio.signal_toggled().connect(sigc::mem_fun(*this, &PolarArrangeTab::on_anchor_radio_changed));
	pack_start(anchorBoundingBoxRadio, false, false);

	pack_start(anchorSelector, false, false);

	anchorObjectPivotRadio.set_label(C_("Polar arrange tab", "Object's rotational center"));
	anchorObjectPivotRadio.set_group(anchorRadioGroup);
	anchorObjectPivotRadio.signal_toggled().connect(sigc::mem_fun(*this, &PolarArrangeTab::on_anchor_radio_changed));
	pack_start(anchorObjectPivotRadio, false, false);

	arrangeOnLabel.set_text(C_("Polar arrange tab", "Arrange on:"));
	arrangeOnLabel.set_alignment(Gtk::ALIGN_START);
	pack_start(arrangeOnLabel, false, false);

	arrangeOnFirstCircleRadio.set_label(C_("Polar arrange tab", "First selected circle/ellipse/arc"));
	arrangeRadioGroup = arrangeOnFirstCircleRadio.get_group();
	arrangeOnFirstCircleRadio.signal_toggled().connect(sigc::mem_fun(*this, &PolarArrangeTab::on_arrange_radio_changed));
	pack_start(arrangeOnFirstCircleRadio, false, false);

	arrangeOnLastCircleRadio.set_label(C_("Polar arrange tab", "Last selected circle/ellipse/arc"));
	arrangeOnLastCircleRadio.set_group(arrangeRadioGroup);
	arrangeOnLastCircleRadio.signal_toggled().connect(sigc::mem_fun(*this, &PolarArrangeTab::on_arrange_radio_changed));
	pack_start(arrangeOnLastCircleRadio, false, false);

	arrangeOnParametersRadio.set_label(C_("Polar arrange tab", "Parameterized:"));
	arrangeOnParametersRadio.set_group(arrangeRadioGroup);
	arrangeOnParametersRadio.signal_toggled().connect(sigc::mem_fun(*this, &PolarArrangeTab::on_arrange_radio_changed));
	pack_start(arrangeOnParametersRadio, false, false);

	centerLabel.set_text(C_("Polar arrange tab", "Center X/Y:"));
#if WITH_GTKMM_3_0
	parametersTable.attach(centerLabel, 0, 0, 1, 1);
#else
	parametersTable.attach(centerLabel, 0, 1, 0, 1, Gtk::FILL);
#endif
	centerX.setDigits(2);
	centerX.setIncrements(0.2, 0);
	centerX.setRange(-10000, 10000);
	centerX.setValue(0, "px");
	centerY.setDigits(2);
	centerY.setIncrements(0.2, 0);
	centerY.setRange(-10000, 10000);
	centerY.setValue(0, "px");
#if WITH_GTKMM_3_0
	parametersTable.attach(centerX, 1, 0, 1, 1);
	parametersTable.attach(centerY, 2, 0, 1, 1);
#else
	parametersTable.attach(centerX, 1, 2, 0, 1, Gtk::FILL);
	parametersTable.attach(centerY, 2, 3, 0, 1, Gtk::FILL);
#endif

	radiusLabel.set_text(C_("Polar arrange tab", "Radius X/Y:"));
#if WITH_GTKMM_3_0
	parametersTable.attach(radiusLabel, 0, 1, 1, 1);
#else
	parametersTable.attach(radiusLabel, 0, 1, 1, 2, Gtk::FILL);
#endif
	radiusX.setDigits(2);
	radiusX.setIncrements(0.2, 0);
	radiusX.setRange(0.001, 10000);
	radiusX.setValue(100, "px");
	radiusY.setDigits(2);
	radiusY.setIncrements(0.2, 0);
	radiusY.setRange(0.001, 10000);
	radiusY.setValue(100, "px");
#if WITH_GTKMM_3_0
	parametersTable.attach(radiusX, 1, 1, 1, 1);
	parametersTable.attach(radiusY, 2, 1, 1, 1);
#else
	parametersTable.attach(radiusX, 1, 2, 1, 2, Gtk::FILL);
	parametersTable.attach(radiusY, 2, 3, 1, 2, Gtk::FILL);
#endif

	angleLabel.set_text(_("Angle X/Y:"));
#if WITH_GTKMM_3_0
	parametersTable.attach(angleLabel, 0, 2, 1, 1);
#else
	parametersTable.attach(angleLabel, 0, 1, 2, 3, Gtk::FILL);
#endif
	angleX.setDigits(2);
	angleX.setIncrements(0.2, 0);
	angleX.setRange(-10000, 10000);
	angleX.setValue(0, "°");
	angleY.setDigits(2);
	angleY.setIncrements(0.2, 0);
	angleY.setRange(-10000, 10000);
	angleY.setValue(180, "°");
#if WITH_GTKMM_3_0
	parametersTable.attach(angleX, 1, 2, 1, 1);
	parametersTable.attach(angleY, 2, 2, 1, 1);
#else
	parametersTable.attach(angleX, 1, 2, 2, 3, Gtk::FILL);
	parametersTable.attach(angleY, 2, 3, 2, 3, Gtk::FILL);
#endif
	pack_start(parametersTable, false, false);

	rotateObjectsCheckBox.set_label(_("Rotate objects"));
	rotateObjectsCheckBox.set_active(true);
	pack_start(rotateObjectsCheckBox, false, false);

	centerX.set_sensitive(false);
	centerY.set_sensitive(false);
	angleX.set_sensitive(false);
	angleY.set_sensitive(false);
	radiusX.set_sensitive(false);
	radiusY.set_sensitive(false);
}

/**
 * This function rotates an item around a given point by a given amount
 * @param item item to rotate
 * @param center center of the rotation to perform
 * @param rotation amount to rotate the object by
 */
static void rotateAround(SPItem *item, Geom::Point center, Geom::Rotate const &rotation)
{
	Geom::Translate const s(center);
	Geom::Affine affine = Geom::Affine(s).inverse() * Geom::Affine(rotation) * Geom::Affine(s);

	// Save old center
    center = item->getCenter();

	item->set_i2d_affine(item->i2dt_affine() * affine);
	item->doWriteTransform(item->getRepr(), item->transform);

	if(item->isCenterSet())
	{
		item->setCenter(center * affine);
		item->updateRepr();
	}
}

/**
 * Calculates the angle at which to put an object given the total amount
 * of objects, the index of the objects as well as the arc start and end
 * points
 * @param arcBegin angle at which the arc begins
 * @param arcEnd angle at which the arc ends
 * @param count number of objects in the selection
 * @param n index of the object in the selection
 */
static float calcAngle(float arcBegin, float arcEnd, int count, int n)
{
	float arcLength = arcEnd - arcBegin;
	float delta = std::abs(std::abs(arcLength) - 2*M_PI);
	if(delta > 0.01) count--; // If not a complete circle, put an object also at the extremes of the arc;

	float angle = n / (float)count;
	// Normalize for arcLength:
	angle = angle * arcLength;
	angle += arcBegin;

	return angle;
}

/**
 * Calculates the point at which an object needs to be, given the center of the ellipse,
 * it's radius (x and y), as well as the angle
 */
static Geom::Point calcPoint(float cx, float cy, float rx, float ry, float angle)
{
	return Geom::Point(cx + cos(angle) * rx, cy + sin(angle) * ry);
}

/**
 * Returns the selected anchor point in document coordinates. If anchor
 * is 0 to 8, then a bounding box point has been choosen. If it is 9 however
 * the rotational center is chosen.
 * @todo still using a hack to get the real coordinate space (subtracting document height
 * 		 and inverting axes)
 */
static Geom::Point getAnchorPoint(int anchor, SPItem *item)
{
	Geom::Point source;

	Geom::OptRect bbox = item->documentVisualBounds();

	switch(anchor)
	{
		case 0: // Top    - Left
		case 3: // Middle - Left
		case 6: // Bottom - Left
			source[0] = bbox->min()[Geom::X];
			break;
		case 1: // Top    - Middle
		case 4: // Middle - Middle
		case 7: // Bottom - Middle
			source[0] = (bbox->min()[Geom::X] + bbox->max()[Geom::X]) / 2.0f;
			break;
		case 2: // Top    - Right
		case 5: // Middle - Right
		case 8: // Bottom - Right
			source[0] = bbox->max()[Geom::X];
			break;
	};

	switch(anchor)
	{
		case 0: // Top    - Left
		case 1: // Top    - Middle
		case 2: // Top    - Right
			source[1] = bbox->min()[Geom::Y];
			break;
		case 3: // Middle - Left
		case 4: // Middle - Middle
		case 5: // Middle - Right
			source[1] = (bbox->min()[Geom::Y] + bbox->max()[Geom::Y]) / 2.0f;
			break;
		case 6: // Bottom - Left
		case 7: // Bottom - Middle
		case 8: // Bottom - Right
			source[1] = bbox->max()[Geom::Y];
			break;
	};

	// If using center
	if(anchor == 9)
		source = item->getCenter();
	else
	{
		// FIXME:
		source[1] -= item->document->getHeight().value("px");
		source[1] *= -1;
	}

	return source;
}

/**
 * Moves an SPItem to a given location, the location is based on the given anchor point.
 * @param anchor 0 to 8 are the various bounding box points like follows:
 *               0  1  2
 *               3  4  5
 *               6  7  8
 *               Anchor mode 9 is the rotational center of the object
 * @param item Item to move
 * @param p point at which to move the object
 */
static void moveToPoint(int anchor, SPItem *item, Geom::Point p)
{
	sp_item_move_rel(item, Geom::Translate(p - getAnchorPoint(anchor, item)));
}

void PolarArrangeTab::arrange()
{
	Inkscape::Selection *selection = parent->getDesktop()->getSelection();
	const GSList *items, *tmp;
	tmp = items = selection->itemList();
	SPGenericEllipse *referenceEllipse = NULL; // Last ellipse in selection

	bool arrangeOnEllipse = !arrangeOnParametersRadio.get_active();
	bool arrangeOnFirstEllipse = arrangeOnEllipse && arrangeOnFirstCircleRadio.get_active();

	int count = 0;
	while(tmp)
	{
		if(arrangeOnEllipse)
		{
			SPItem *item = SP_ITEM(tmp->data);

			if(arrangeOnFirstEllipse)
			{
				// The first selected ellipse is actually the last one in the list
				if(SP_IS_GENERICELLIPSE(item))
					referenceEllipse = SP_GENERICELLIPSE(item);
			} else {
				// The last selected ellipse is actually the first in list
				if(SP_IS_GENERICELLIPSE(item) && referenceEllipse == NULL)
					referenceEllipse = SP_GENERICELLIPSE(item);
			}
		}
		tmp = tmp->next;
		++count;
	}

	float cx, cy; // Center of the ellipse
	float rx, ry; // Radiuses of the ellipse in x and y direction
	float arcBeg, arcEnd; // begin and end angles for arcs
	Geom::Affine transformation; // Any additional transformation to apply to the objects

	if(arrangeOnEllipse)
	{
		if(referenceEllipse == NULL)
		{
			Gtk::MessageDialog dialog(_("Couldn't find an ellipse in selection"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
			dialog.run();
			return;
		} else {
			cx = referenceEllipse->cx.value;
			cy = referenceEllipse->cy.value;
			rx = referenceEllipse->rx.value;
			ry = referenceEllipse->ry.value;
			arcBeg = referenceEllipse->start;
			arcEnd = referenceEllipse->end;

			transformation = referenceEllipse->i2dt_affine();

			// We decrement the count by 1 as we are not going to lay
			// out the reference ellipse
			--count;
		}

	} else 	{
		// Read options from UI
		cx = centerX.getValue("px");
		cy = centerY.getValue("px");
		rx = radiusX.getValue("px");
		ry = radiusY.getValue("px");
		arcBeg = angleX.getValue("rad");
		arcEnd = angleY.getValue("rad");
		transformation.setIdentity();
		referenceEllipse = NULL;
	}

	int anchor = 9;
	if(anchorBoundingBoxRadio.get_active())
	{
		anchor = anchorSelector.getHorizontalAlignment() +
				anchorSelector.getVerticalAlignment() * 3;
	}

	Geom::Point realCenter = Geom::Point(cx, cy) * transformation;

	tmp = items;
	int i = 0;
	while(tmp)
	{
		SPItem *item = SP_ITEM(tmp->data);

		// Ignore the reference ellipse if any
		if(item != referenceEllipse)
		{
			float angle = calcAngle(arcBeg, arcEnd, count, i);
			Geom::Point newLocation = calcPoint(cx, cy, rx, ry, angle) * transformation;

			moveToPoint(anchor, item, newLocation);

			if(rotateObjectsCheckBox.get_active()) {
				// Calculate the angle by which to rotate each object
				angle = -atan2f(newLocation.x() - realCenter.x(), newLocation.y() - realCenter.y());
				rotateAround(item, newLocation, Geom::Rotate(angle));
			}

			++i;
		}
		tmp = tmp->next;
	}

    DocumentUndo::done(parent->getDesktop()->getDocument(), SP_VERB_SELECTION_ARRANGE,
                       _("Arrange on ellipse"));
}

void PolarArrangeTab::updateSelection()
{
}

void PolarArrangeTab::on_arrange_radio_changed()
{
	bool arrangeParametric = arrangeOnParametersRadio.get_active();

	centerX.set_sensitive(arrangeParametric);
	centerY.set_sensitive(arrangeParametric);

	angleX.set_sensitive(arrangeParametric);
	angleY.set_sensitive(arrangeParametric);

	radiusX.set_sensitive(arrangeParametric);
	radiusY.set_sensitive(arrangeParametric);

	parametersTable.set_visible(arrangeParametric);
}

void PolarArrangeTab::on_anchor_radio_changed()
{
	bool anchorBoundingBox = anchorBoundingBoxRadio.get_active();

	anchorSelector.set_sensitive(anchorBoundingBox);
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

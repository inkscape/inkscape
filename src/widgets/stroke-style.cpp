/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <brycehar@bryceharrington.org>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Josh Andler <scislac@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 * Copyright (C) 2008 Maximilian Albert (gtkmm-ification)
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_SS_VERBOSE

#include "stroke-style.h"
#include "gradient-chemistry.h"
#include "sp-gradient.h"
#include "sp-stop.h"
#include "svg/svg-color.h"
#include "util/units.h"
#include "ui/widget/unit-menu.h"
#include "desktop-widget.h"

using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

/**
 * Creates a new widget for the line stroke paint.
 */
Gtk::Widget *sp_stroke_style_paint_widget_new(void)
{
    return Inkscape::Widgets::createStyleWidget( STROKE );
}

/**
 * Creates a new widget for the line stroke style.
 */
Gtk::Widget *sp_stroke_style_line_widget_new(void)
{
    return Inkscape::Widgets::createStrokeStyleWidget();
}

void sp_stroke_style_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop)
{
    Inkscape::StrokeStyle *ss = dynamic_cast<Inkscape::StrokeStyle*>(widget);
    if (ss) {
        ss->setDesktop(desktop);
    }
}


/**
 * Extract the actual name of the link
 * e.g. get mTriangle from url(#mTriangle).
 * \return Buffer containing the actual name, allocated from GLib;
 * the caller should free the buffer when they no longer need it.
 */
SPObject* getMarkerObj(gchar const *n, SPDocument *doc)
{
    gchar const *p = n;
    while (*p != '\0' && *p != '#') {
        p++;
    }

    if (*p == '\0' || p[1] == '\0') {
        return NULL;
    }

    p++;
    int c = 0;
    while (p[c] != '\0' && p[c] != ')') {
        c++;
    }

    if (p[c] == '\0') {
        return NULL;
    }

    gchar* b = g_strdup(p);
    b[c] = '\0';

    // FIXME: get the document from the object and let the caller pass it in
    SPObject *marker = doc->getObjectById(b);

    g_free(b);
    return marker;
}

namespace Inkscape {


/**
 * Construct a stroke-style radio button with a given icon
 *
 * \param[in] grp          The Gtk::RadioButtonGroup to which to add the new button
 * \param[in] icon         The icon to use for the button
 * \param[in] button_type  The type of stroke-style radio button (join/cap)
 * \param[in] stroke_style The style attribute to associate with the button
 */
StrokeStyle::StrokeStyleButton::StrokeStyleButton(Gtk::RadioButtonGroup &grp,
                                                  char const            *icon,
                                                  StrokeStyleButtonType  button_type,
                                                  gchar const           *stroke_style)
    : 
        Gtk::RadioButton(grp),
        button_type(button_type),
        stroke_style(stroke_style)
{
    show();
    set_mode(false);
        
    Gtk::Widget *px = Gtk::manage(Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_LARGE_TOOLBAR, icon)));
    g_assert(px != NULL);
    px->show();
    add(*px);
}

/**
 * Create the fill or stroke style widget, and hook up all the signals.
 */
Gtk::Widget *Inkscape::Widgets::createStrokeStyleWidget( )
{
    StrokeStyle *strokeStyle = new StrokeStyle();

    return strokeStyle;
}

StrokeStyle::StrokeStyle() :
    Gtk::VBox(),
    miterLimitSpin(),
    widthSpin(),
    unitSelector(),
    joinMiter(),
    joinRound(),
    joinBevel(),
    capButt(),
    capRound(),
    capSquare(),
    dashSelector(),
    update(false),
    desktop(0),
    selectChangedConn(),
    selectModifiedConn(),
    startMarkerConn(),
    midMarkerConn(),
    endMarkerConn(),
    _old_unit(NULL)
{
    Gtk::HBox *hb;
    Gtk::HBox *f = new Gtk::HBox(false, 0);
    f->show();
    add(*f);

#if WITH_GTKMM_3_0
    table = new Gtk::Grid();
    table->set_border_width(4);
    table->set_row_spacing(4);
#else
    table = new Gtk::Table(3, 6, false);
    table->set_border_width(4);
    table->set_row_spacings(4);
#endif
    
    table->show();
    f->add(*table);

    gint i = 0;

    //spw_label(t, C_("Stroke width", "_Width:"), 0, i);

    hb = spw_hbox(table, 3, 1, i);

// TODO: when this is gtkmmified, use an Inkscape::UI::Widget::ScalarUnit instead of the separate
// spinbutton and unit selector for stroke width. In sp_stroke_style_line_update, use
// setHundredPercent to remember the averaged width corresponding to 100%. Then the
// stroke_width_set_unit will be removed (because ScalarUnit takes care of conversions itself), and
// with it, the two remaining calls of stroke_average_width, allowing us to get rid of that
// function in desktop-style.

#if WITH_GTKMM_3_0
    widthAdj = new Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(1.0, 0.0, 1000.0, 0.1, 10.0, 0.0));
#else
    widthAdj = new Gtk::Adjustment(1.0, 0.0, 1000.0, 0.1, 10.0, 0.0);
#endif

    widthSpin = new Inkscape::UI::Widget::SpinButton(*widthAdj, 0.1, 3);
    widthSpin->set_tooltip_text(_("Stroke width"));
    widthSpin->show();
    spw_label(table, C_("Stroke width", "_Width:"), 0, i, widthSpin);

    sp_dialog_defocus_on_enter_cpp(widthSpin);

    hb->pack_start(*widthSpin, false, false, 0);
    unitSelector = new Inkscape::UI::Widget::UnitMenu();
    unitSelector->setUnitType(Inkscape::Util::UNIT_TYPE_LINEAR);
    Gtk::Widget *us = Gtk::manage(unitSelector);
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    unitSelector->addUnit(*unit_table.getUnit("%"));
    _old_unit = unitSelector->getUnit();
    if (desktop) {
        unitSelector->setUnit(desktop->getNamedView()->display_units->abbr);
        _old_unit = desktop->getNamedView()->display_units;
    }
    widthSpin->setUnitMenu(unitSelector);
    unitChangedConn = unitSelector->signal_changed().connect(sigc::mem_fun(*this, &StrokeStyle::unitChangedCB));
    
    us->show();

    hb->pack_start(*us, FALSE, FALSE, 0);

#if WITH_GTKMM_3_0
    (*widthAdj)->signal_value_changed().connect(sigc::mem_fun(*this, &StrokeStyle::widthChangedCB));
#else
    widthAdj->signal_value_changed().connect(sigc::mem_fun(*this, &StrokeStyle::widthChangedCB));
#endif
    i++;

    /* Dash */
    spw_label(table, _("Dashes:"), 0, i, NULL); //no mnemonic for now
                                            //decide what to do:
                                            //   implement a set_mnemonic_source function in the
                                            //   SPDashSelector class, so that we do not have to
                                            //   expose any of the underlying widgets?
    dashSelector = Gtk::manage(new SPDashSelector);

    dashSelector->show();

#if WITH_GTKMM_3_0
    dashSelector->set_hexpand();
    dashSelector->set_halign(Gtk::ALIGN_FILL);
    dashSelector->set_valign(Gtk::ALIGN_CENTER);
    table->attach(*dashSelector, 1, i, 3, 1);
#else
    table->attach(*dashSelector, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
#endif

    dashSelector->changed_signal.connect(sigc::mem_fun(*this, &StrokeStyle::lineDashChangedCB));

    i++;

    /* Drop down marker selectors*/
    // TRANSLATORS: Path markers are an SVG feature that allows you to attach arbitrary shapes
    // (arrowheads, bullets, faces, whatever) to the start, end, or middle nodes of a path.

    spw_label(table, _("Markers:"), 0, i, NULL);

    hb = spw_hbox(table, 1, 1, i);
    i++;

    startMarkerCombo = Gtk::manage(new MarkerComboBox("marker-start", SP_MARKER_LOC_START));
    startMarkerCombo->set_tooltip_text(_("Start Markers are drawn on the first node of a path or shape"));
    startMarkerConn = startMarkerCombo->signal_changed().connect(
            sigc::bind<MarkerComboBox *, StrokeStyle *, SPMarkerLoc>(
                sigc::ptr_fun(&StrokeStyle::markerSelectCB), startMarkerCombo, this, SP_MARKER_LOC_START));
    startMarkerCombo->show();

    hb->pack_start(*startMarkerCombo, true, true, 0);

    midMarkerCombo = Gtk::manage(new MarkerComboBox("marker-mid", SP_MARKER_LOC_MID));
    midMarkerCombo->set_tooltip_text(_("Mid Markers are drawn on every node of a path or shape except the first and last nodes"));
    midMarkerConn = midMarkerCombo->signal_changed().connect(
        sigc::bind<MarkerComboBox *, StrokeStyle *, SPMarkerLoc>(
            sigc::ptr_fun(&StrokeStyle::markerSelectCB), midMarkerCombo, this, SP_MARKER_LOC_MID));
    midMarkerCombo->show();

    hb->pack_start(*midMarkerCombo, true, true, 0);

    endMarkerCombo = Gtk::manage(new MarkerComboBox("marker-end", SP_MARKER_LOC_END));
    endMarkerCombo->set_tooltip_text(_("End Markers are drawn on the last node of a path or shape"));
    endMarkerConn = endMarkerCombo->signal_changed().connect(
        sigc::bind<MarkerComboBox *, StrokeStyle *, SPMarkerLoc>(
            sigc::ptr_fun(&StrokeStyle::markerSelectCB), endMarkerCombo, this, SP_MARKER_LOC_END));
    endMarkerCombo->show();

    hb->pack_start(*endMarkerCombo, true, true, 0);

    i++;

    /* Join type */
    // TRANSLATORS: The line join style specifies the shape to be used at the
    //  corners of paths. It can be "miter", "round" or "bevel".
    spw_label(table, _("Join:"), 0, i, NULL);

    hb = spw_hbox(table, 3, 1, i);

    Gtk::RadioButtonGroup joinGrp;

    joinRound = makeRadioButton(joinGrp, INKSCAPE_ICON("stroke-join-round"),
                                hb, STROKE_STYLE_BUTTON_JOIN, "round");

    // TRANSLATORS: Round join: joining lines with a rounded corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    joinRound->set_tooltip_text(_("Round join"));

    joinBevel = makeRadioButton(joinGrp, INKSCAPE_ICON("stroke-join-bevel"),
                                hb, STROKE_STYLE_BUTTON_JOIN, "bevel");

    // TRANSLATORS: Bevel join: joining lines with a blunted (flattened) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    joinBevel->set_tooltip_text(_("Bevel join"));

    joinMiter = makeRadioButton(joinGrp, INKSCAPE_ICON("stroke-join-miter"),
                                hb, STROKE_STYLE_BUTTON_JOIN, "miter");

    // TRANSLATORS: Miter join: joining lines with a sharp (pointed) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    joinMiter->set_tooltip_text(_("Miter join"));

    /* Miterlimit  */
    // TRANSLATORS: Miter limit: only for "miter join", this limits the length
    //  of the sharp "spike" when the lines connect at too sharp an angle.
    // When two line segments meet at a sharp angle, a miter join results in a
    //  spike that extends well beyond the connection point. The purpose of the
    //  miter limit is to cut off such spikes (i.e. convert them into bevels)
    //  when they become too long.
    //spw_label(t, _("Miter _limit:"), 0, i);

#if WITH_GTKMM_3_0
    miterLimitAdj = new Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(4.0, 0.0, 100.0, 0.1, 10.0, 0.0));
    miterLimitSpin = new Inkscape::UI::Widget::SpinButton(*miterLimitAdj, 0.1, 2);
#else
    miterLimitAdj = new Gtk::Adjustment(4.0, 0.0, 100.0, 0.1, 10.0, 0.0);
    miterLimitSpin = new Inkscape::UI::Widget::SpinButton(*miterLimitAdj, 0.1, 2);
#endif

    miterLimitSpin->set_tooltip_text(_("Maximum length of the miter (in units of stroke width)"));
    miterLimitSpin->show();
    sp_dialog_defocus_on_enter_cpp(miterLimitSpin);

    hb->pack_start(*miterLimitSpin, false, false, 0);

#if WITH_GTKMM_3_0
    (*miterLimitAdj)->signal_value_changed().connect(sigc::mem_fun(*this, &StrokeStyle::miterLimitChangedCB));

#else
    miterLimitAdj->signal_value_changed().connect(sigc::mem_fun(*this, &StrokeStyle::miterLimitChangedCB));
#endif

    i++;

    /* Cap type */
    // TRANSLATORS: cap type specifies the shape for the ends of lines
    //spw_label(t, _("_Cap:"), 0, i);
    spw_label(table, _("Cap:"), 0, i, NULL);

    hb = spw_hbox(table, 3, 1, i);

    Gtk::RadioButtonGroup capGrp;

    capButt = makeRadioButton(capGrp, INKSCAPE_ICON("stroke-cap-butt"),
                                hb, STROKE_STYLE_BUTTON_CAP, "butt");

    // TRANSLATORS: Butt cap: the line shape does not extend beyond the end point
    //  of the line; the ends of the line are square
    capButt->set_tooltip_text(_("Butt cap"));

    capRound = makeRadioButton(capGrp, INKSCAPE_ICON("stroke-cap-round"),
                                hb, STROKE_STYLE_BUTTON_CAP, "round");

    // TRANSLATORS: Round cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are rounded
    capRound->set_tooltip_text(_("Round cap"));

    capSquare = makeRadioButton(capGrp, INKSCAPE_ICON("stroke-cap-square"),
                                hb, STROKE_STYLE_BUTTON_CAP, "square");

    // TRANSLATORS: Square cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are square
    capSquare->set_tooltip_text(_("Square cap"));

    i++;

    /* Paint order */
    // TRANSLATORS: Paint order determines the order the 'fill', 'stroke', and 'markers are painted.
    spw_label(table, _("Order:"), 0, i, NULL);

    hb = spw_hbox(table, 4, 1, i);

    Gtk::RadioButtonGroup paintOrderGrp;

    paintOrderFSM = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-fsm"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "normal");
    paintOrderFSM->set_tooltip_text(_("Fill, Stroke, Markers")); 

    paintOrderSFM = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-sfm"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "stroke fill markers");
    paintOrderSFM->set_tooltip_text(_("Stroke, Fill, Markers")); 

    paintOrderFMS = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-fms"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "fill markers stroke");
    paintOrderFMS->set_tooltip_text(_("Fill, Markers, Stroke")); 

    i++;

    hb = spw_hbox(table, 4, 1, i);

    paintOrderMFS = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-mfs"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "markers fill stroke");
    paintOrderMFS->set_tooltip_text(_("Markers, Fill, Stroke")); 

    paintOrderSMF = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-smf"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "stroke markers fill");
    paintOrderSMF->set_tooltip_text(_("Stroke, Markers, Fill")); 

    paintOrderMSF = makeRadioButton(paintOrderGrp, INKSCAPE_ICON("paint-order-msf"),
                                    hb, STROKE_STYLE_BUTTON_ORDER, "markers stroke fill");
    paintOrderMSF->set_tooltip_text(_("Markers, Stroke, Fill")); 

    i++;

    setDesktop(desktop);
    updateLine();

}

StrokeStyle::~StrokeStyle()
{
    selectModifiedConn.disconnect();
    selectChangedConn.disconnect();
}

void StrokeStyle::setDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {

        if (this->desktop) {
            selectModifiedConn.disconnect();
            selectChangedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &StrokeStyle::selectionChangedCB)));
            selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &StrokeStyle::selectionModifiedCB)));
        }
        updateLine();
    }
}


/**
 * Helper function for creating stroke-style radio buttons.
 *
 * \param[in] grp           The Gtk::RadioButtonGroup in which to add the button
 * \param[in] icon          The icon for the button
 * \param[in] hb            The Gtk::Box container in which to add the button
 * \param[in] button_type   The type (join/cap) for the button
 * \param[in] stroke_style  The style attribute to associate with the button
 *
 * \details After instantiating the button, it is added to a container box and
 *          a handler for the toggle event is connected.
 */
StrokeStyle::StrokeStyleButton *
StrokeStyle::makeRadioButton(Gtk::RadioButtonGroup &grp,
                             char const            *icon,
                             Gtk::HBox             *hb,
                             StrokeStyleButtonType  button_type,
                             gchar const           *stroke_style)
{
    g_assert(icon != NULL);
    g_assert(hb  != NULL);

    StrokeStyleButton *tb = new StrokeStyleButton(grp, icon, button_type, stroke_style);

    hb->pack_start(*tb, false, false, 0);
    set_data(icon, tb);

    tb->signal_toggled().connect(sigc::bind<StrokeStyleButton *, StrokeStyle *>(
                                     sigc::ptr_fun(&StrokeStyle::buttonToggledCB), tb, this));

    return tb;
}

/**
 * Handles when user selects one of the markers from the marker combobox.
 * Gets the marker uri string and applies it to all selected
 * items in the current desktop.
 */
void StrokeStyle::markerSelectCB(MarkerComboBox *marker_combo, StrokeStyle *spw, SPMarkerLoc const /*which*/)
{
    bool markers_update = spw->startMarkerCombo->update() ||
                          spw->midMarkerCombo->update() ||
                          spw->endMarkerCombo->update();

    if (spw->update || markers_update) {
        return;
    }

    spw->update = true;

    SPDocument *document = spw->desktop->getDocument();
    if (!document) {
        return;
    }

    /* Get Marker */
    gchar const *marker = marker_combo->get_active_marker_uri();


    SPCSSAttr *css = sp_repr_css_attr_new();
    gchar const *combo_id = marker_combo->get_id();
    sp_repr_css_set_property(css, combo_id, marker);

    // Also update the marker combobox, so the document's markers
    // show up at the top of the combobox
//    sp_stroke_style_line_update( SP_WIDGET(spw), desktop ? desktop->getSelection() : NULL);
    //spw->updateMarkerHist(which);

    Inkscape::Selection *selection = spw->desktop->getSelection();
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (!SP_IS_SHAPE(item) || SP_IS_RECT(item)) { // can't set marker to rect, until it's converted to using <path>
            continue;
        }
        Inkscape::XML::Node *selrepr = item->getRepr();
        if (selrepr) {
            sp_repr_css_change_recursive(selrepr, css, "style");
            SPObject *markerObj = getMarkerObj(marker, document);
            spw->setMarkerColor(markerObj, marker_combo->get_loc(), item);
        }

        item->requestModified(SP_OBJECT_MODIFIED_FLAG);
        item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

        DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE, _("Set markers"));
    }

    sp_repr_css_attr_unref(css);
    css = 0;

    spw->update = false;
};

void StrokeStyle::updateMarkerHist(SPMarkerLoc const which)
{
    switch (which) {
        case SP_MARKER_LOC_START:
            startMarkerConn.block();
            startMarkerCombo->set_active_history();
            startMarkerConn.unblock();
            break;

        case SP_MARKER_LOC_MID:
            midMarkerConn.block();
            midMarkerCombo->set_active_history();
            midMarkerConn.unblock();
            break;

        case SP_MARKER_LOC_END:
            endMarkerConn.block();
            endMarkerCombo->set_active_history();
            endMarkerConn.unblock();
            break;
        default:
            g_assert_not_reached();
    }
}

/**
 * Callback for when UnitMenu widget is modified.
 * Triggers update action.
 */
void StrokeStyle::unitChangedCB()
{
    Inkscape::Util::Unit const *new_unit = unitSelector->getUnit();
    if (new_unit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) {
        widthSpin->set_value(100);
    }
    widthSpin->set_value(Inkscape::Util::Quantity::convert(widthSpin->get_value(), _old_unit, new_unit));
    _old_unit = new_unit;
}

/**
 * Callback for when stroke style widget is modified.
 * Triggers update action.
 */
void
StrokeStyle::selectionModifiedCB(guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        updateLine();
    }
}

/**
 * Callback for when stroke style widget is changed.
 * Triggers update action.
 */
void
StrokeStyle::selectionChangedCB()
{
    updateLine();
}

/*
 * Fork marker if necessary and set the referencing items url to the new marker
 * Return the new marker
 */
SPObject *
StrokeStyle::forkMarker(SPObject *marker, int loc, SPItem *item)
{
    if (!item || !marker) {
        return NULL;
    }

    gchar const *marker_id = SPMarkerNames[loc].key;

    /*
     * Optimization when all the references to this marker are from this item
     * then we can reuse it and dont need to fork
     */
    Glib::ustring urlId = Glib::ustring::format("url(#", marker->getRepr()->attribute("id"), ")");
    unsigned int refs = 0;
    for (int i = SP_MARKER_LOC_START; i < SP_MARKER_LOC_QTY; i++) {
        if (item->style->marker_ptrs[i]->set &&
            !strcmp(urlId.c_str(), item->style->marker_ptrs[i]->value)) {
            refs++;
        }
    }
    if (marker->hrefcount <= refs) {
        return marker;
    }

    marker = sp_marker_fork_if_necessary(marker);

    // Update the items url to new marker
    Inkscape::XML::Node *mark_repr = marker->getRepr();
    SPCSSAttr *css_item = sp_repr_css_attr_new();
    sp_repr_css_set_property(css_item, marker_id, g_strconcat("url(#", mark_repr->attribute("id"), ")", NULL));
    sp_repr_css_change_recursive(item->getRepr(), css_item, "style");

    sp_repr_css_attr_unref(css_item);
    css_item = 0;

    return marker;
}

/**
 * Change the color of the marker to match the color of the item.
 * Marker stroke color is set to item stroke color.
 * Fill color :
 * 1. If the item has fill, use that for the marker fill,
 * 2. If the marker has same fill and stroke assume its solid, use item stroke for both fill and stroke the line stroke
 * 3. If the marker has fill color, use the marker fill color
 *
 */
void
StrokeStyle::setMarkerColor(SPObject *marker, int loc, SPItem *item)
{

    if (!item || !marker) {
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean colorStock = prefs->getBool("/options/markers/colorStockMarkers", true);
    gboolean colorCustom = prefs->getBool("/options/markers/colorCustomMarkers", false);
    const gchar *stock = marker->getRepr()->attribute("inkscape:isstock");
    gboolean isStock = (stock && !strcmp(stock,"true"));

    if (isStock ? !colorStock : !colorCustom) {
        return;
    }

    // Check if we need to fork this marker
    marker = forkMarker(marker, loc, item);

    Inkscape::XML::Node *repr = marker->getRepr()->firstChild();
    if (!repr) {
        return;
    };

    // Current line style
    SPCSSAttr *css_item = sp_css_attr_from_object(item, SP_STYLE_FLAG_ALWAYS);
    const char *lstroke = getItemColorForMarker(item, FOR_STROKE, loc);
    const char *lstroke_opacity = sp_repr_css_property(css_item, "stroke-opacity", "1");
    const char *lfill = getItemColorForMarker(item, FOR_FILL, loc);
    const char *lfill_opacity = sp_repr_css_property(css_item, "fill-opacity", "1");

    // Current marker style
    SPCSSAttr *css_marker = sp_css_attr_from_object(marker->firstChild(), SP_STYLE_FLAG_ALWAYS);
    const char *mfill = sp_repr_css_property(css_marker, "fill", "none");
    const char *mstroke = sp_repr_css_property(css_marker, "fill", "none");

    // Create new marker style with the lines stroke
    SPCSSAttr *css = sp_repr_css_attr_new();

    sp_repr_css_set_property(css, "stroke", lstroke);
    sp_repr_css_set_property(css, "stroke-opacity", lstroke_opacity);

    if (strcmp(lfill, "none") ) {
        // 1. If the line has fill, use that for the marker fill
        sp_repr_css_set_property(css, "fill", lfill);
        sp_repr_css_set_property(css, "fill-opacity", lfill_opacity);
    }
    else if (mfill && mstroke && !strcmp(mfill, mstroke) && mfill[0] == '#' && strcmp(mfill, "#ffffff")) {
        // 2. If the marker has same fill and stroke assume its solid. use line stroke for both fill and stroke the line stroke
        sp_repr_css_set_property(css, "fill", lstroke);
        sp_repr_css_set_property(css, "fill-opacity", lstroke_opacity);
    }
    else if (mfill && mfill[0] == '#' && strcmp(mfill, "#000000")) {
        // 3. If the marker has fill color, use the marker fill color
        sp_repr_css_set_property(css, "fill", mfill);
        //sp_repr_css_set_property(css, "fill-opacity", mfill_opacity);
    }

    sp_repr_css_change_recursive(marker->firstChild()->getRepr(), css, "style");

    // Tell the combos to update its image cache of this marker
    gchar const *mid = marker->getRepr()->attribute("id");
    startMarkerCombo->update_marker_image(mid);
    midMarkerCombo->update_marker_image(mid);
    endMarkerCombo->update_marker_image(mid);

    sp_repr_css_attr_unref(css);
    css = 0;


}

/*
 * Get the fill or stroke color of the item
 * If its a gradient, then return first or last stop color
 */
const char *
StrokeStyle::getItemColorForMarker(SPItem *item, Inkscape::PaintTarget fill_or_stroke, int loc)
{
    SPCSSAttr *css_item = sp_css_attr_from_object(item, SP_STYLE_FLAG_ALWAYS);
    const char *color;
    if (fill_or_stroke == FOR_FILL)
        color = sp_repr_css_property(css_item, "fill", "none");
    else
        color = sp_repr_css_property(css_item, "stroke", "none");

    if (!strncmp (color, "url(", 4)) {
        // If the item has a gradient use the first stop color for the marker

        SPGradient *grad = getGradient(item, fill_or_stroke);
        if (grad) {
            SPGradient *vector = grad->getVector(FALSE);
            SPStop *stop = vector->getFirstStop();
            if (loc == SP_MARKER_LOC_END) {
                stop = sp_last_stop(vector);
            }
            if (stop) {
                guint32 const c1 = stop->get_rgba32();
                gchar c[64];
                sp_svg_write_color(c, sizeof(c), c1);
                color = g_strdup(c);
                //lstroke_opacity = Glib::ustring::format(stop->opacity).c_str();
            }
        }
    }
    return color;
}
/**
 * Sets selector widgets' dash style from an SPStyle object.
 */
void
StrokeStyle::setDashSelectorFromStyle(SPDashSelector *dsel, SPStyle *style)
{
    if (!style->stroke_dasharray.values.empty()) {
        double d[64];
        size_t len = MIN(style->stroke_dasharray.values.size(), 64);
        for (unsigned i = 0; i < len; i++) {
            if (style->stroke_width.computed != 0)
                d[i] = style->stroke_dasharray.values[i] / style->stroke_width.computed;
            else
                d[i] = style->stroke_dasharray.values[i]; // is there a better thing to do for stroke_width==0?
        }
        dsel->set_dash(len, d, style->stroke_width.computed != 0 ?
                       style->stroke_dashoffset.value / style->stroke_width.computed  :
                       style->stroke_dashoffset.value);
    } else {
        dsel->set_dash(0, NULL, 0.0);
    }
}

/**
 * Sets the join type for a line, and updates the stroke style widget's buttons
 */
void
StrokeStyle::setJoinType (unsigned const jointype)
{
    Gtk::RadioButton *tb = NULL;
    switch (jointype) {
        case SP_STROKE_LINEJOIN_MITER:
            tb = joinMiter;
            break;
        case SP_STROKE_LINEJOIN_ROUND:
            tb = joinRound;
            break;
        case SP_STROKE_LINEJOIN_BEVEL:
            tb = joinBevel;
            break;
        default:
            // Should not happen
            std::cerr << "StrokeStyle::setJoinType(): Invalid value: " << jointype << std::endl;
            tb = joinMiter;
            break;
    }
    setJoinButtons(tb);
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
void
StrokeStyle::setCapType (unsigned const captype)
{
    Gtk::RadioButton *tb = NULL;
    switch (captype) {
        case SP_STROKE_LINECAP_BUTT:
            tb = capButt;
            break;
        case SP_STROKE_LINECAP_ROUND:
            tb = capRound;
            break;
        case SP_STROKE_LINECAP_SQUARE:
            tb = capSquare;
            break;
        default:
            // Should not happen
            std::cerr << "StrokeStyle::setCapType(): Invalid value: " << captype << std::endl;
            tb = capButt;
            break;
    }
    setCapButtons(tb);
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
void
StrokeStyle::setPaintOrder (gchar const *paint_order)
{
    Gtk::RadioButton *tb = paintOrderFSM;

    SPIPaintOrder temp;
    temp.read( paint_order );

    if (temp.layer[0] != SP_CSS_PAINT_ORDER_NORMAL) {

        if (temp.layer[0] == SP_CSS_PAINT_ORDER_FILL) {
            if (temp.layer[1] == SP_CSS_PAINT_ORDER_STROKE) {
                tb = paintOrderFSM;
            } else {
                tb = paintOrderFMS;
            }
        } else if (temp.layer[0] == SP_CSS_PAINT_ORDER_STROKE) {
            if (temp.layer[1] == SP_CSS_PAINT_ORDER_FILL) {
                tb = paintOrderSFM;
            } else {
                tb = paintOrderSMF;
            }
        } else {
            if (temp.layer[1] == SP_CSS_PAINT_ORDER_STROKE) {
                tb = paintOrderMSF;
            } else {
                tb = paintOrderMFS;
            }
        }

    }
    setPaintOrderButtons(tb);
}

/**
 * Callback for when stroke style widget is updated, including markers, cap type,
 * join type, etc.
 */
void
StrokeStyle::updateLine()
{
    if (update) {
        return;
    }

    update = true;

    Inkscape::Selection *sel = desktop ? desktop->getSelection() : NULL;

    FillOrStroke kind = GPOINTER_TO_INT(get_data("kind")) ? FILL : STROKE;

    // create temporary style
    SPStyle query(SP_ACTIVE_DOCUMENT);
    // query into it
    int result_sw = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
    int result_ml = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_STROKEMITERLIMIT);
    int result_cap = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_STROKECAP);
    int result_join = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_STROKEJOIN);

    int result_order = sp_desktop_query_style (SP_ACTIVE_DESKTOP, &query, QUERY_STYLE_PROPERTY_PAINTORDER);

    SPIPaint &targPaint = (kind == FILL) ? query.fill : query.stroke;

    if (!sel || sel->isEmpty()) {
        // Nothing selected, grey-out all controls in the stroke-style dialog
        table->set_sensitive(false);

        update = false;

        return;
    } else {
        table->set_sensitive(true);

        if (result_sw == QUERY_STYLE_MULTIPLE_AVERAGED) {
            unitSelector->setUnit("%");
        } else {
            // same width, or only one object; no sense to keep percent, switch to absolute
            Inkscape::Util::Unit const *tempunit = unitSelector->getUnit();
            if (tempunit->type != Inkscape::Util::UNIT_TYPE_LINEAR) {
                unitSelector->setUnit(SP_ACTIVE_DESKTOP->getNamedView()->display_units->abbr);
            }
        }

        Inkscape::Util::Unit const *unit = unitSelector->getUnit();

        if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
            double avgwidth = Inkscape::Util::Quantity::convert(query.stroke_width.computed, "px", unit);
#if WITH_GTKMM_3_0
            (*widthAdj)->set_value(avgwidth);
#else
            widthAdj->set_value(avgwidth);
#endif
        } else {
#if WITH_GTKMM_3_0
            (*widthAdj)->set_value(100);
#else
            widthAdj->set_value(100);
#endif
        }

        // if none of the selected objects has a stroke, than quite some controls should be disabled
        // The markers might still be shown though, so these will not be disabled
        bool enabled = (result_sw != QUERY_STYLE_NOTHING) && !targPaint.isNoneSet();
        /* No objects stroked, set insensitive */
        joinMiter->set_sensitive(enabled);
        joinRound->set_sensitive(enabled);
        joinBevel->set_sensitive(enabled);

        miterLimitSpin->set_sensitive(enabled);

        capButt->set_sensitive(enabled);
        capRound->set_sensitive(enabled);
        capSquare->set_sensitive(enabled);

        dashSelector->set_sensitive(enabled);
    }

    if (result_ml != QUERY_STYLE_NOTHING)
#if WITH_GTKMM_3_0
        (*miterLimitAdj)->set_value(query.stroke_miterlimit.value); // TODO: reflect averagedness?
#else
        miterLimitAdj->set_value(query.stroke_miterlimit.value); // TODO: reflect averagedness?
#endif

    if (result_join != QUERY_STYLE_MULTIPLE_DIFFERENT &&
        result_join != QUERY_STYLE_NOTHING ) {
        setJoinType(query.stroke_linejoin.value);
    } else {
        setJoinButtons(NULL);
    }

    if (result_cap != QUERY_STYLE_MULTIPLE_DIFFERENT &&
        result_cap != QUERY_STYLE_NOTHING ) {
        setCapType (query.stroke_linecap.value);
    } else {
        setCapButtons(NULL);
    }

    if (result_order != QUERY_STYLE_MULTIPLE_DIFFERENT &&
        result_order != QUERY_STYLE_NOTHING ) {
        setPaintOrder (query.paint_order.value);
    } else {
        setPaintOrder (NULL);
    }

    if (!sel || sel->isEmpty())
        return;

    std::vector<SPItem*> const objects = sel->itemList();
    SPObject * const object = objects[0];
    SPStyle * const style = object->style;

    /* Markers */
    updateAllMarkers(objects); // FIXME: make this desktop query too

    /* Dash */
    setDashSelectorFromStyle(dashSelector, style); // FIXME: make this desktop query too

    table->set_sensitive(true);

    update = false;
}

/**
 * Sets a line's dash properties in a CSS style object.
 */
void
StrokeStyle::setScaledDash(SPCSSAttr *css,
                                int ndash, double *dash, double offset,
                                double scale)
{
    if (ndash > 0) {
        Inkscape::CSSOStringStream osarray;
        for (int i = 0; i < ndash; i++) {
            osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
                osarray << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", osarray.str().c_str());

        Inkscape::CSSOStringStream osoffset;
        osoffset << offset * scale;
        sp_repr_css_set_property(css, "stroke-dashoffset", osoffset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", NULL);
    }
}

/**
 * Sets line properties like width, dashes, markers, etc. on all currently selected items.
 */
void
StrokeStyle::scaleLine()
{
    if (update) {
        return;
    }

    update = true;
    
    SPDocument *document = desktop->getDocument();
    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> items=selection->itemList();

    /* TODO: Create some standardized method */
    SPCSSAttr *css = sp_repr_css_attr_new();

    if (!items.empty()) {
#if WITH_GTKMM_3_0
        double width_typed = (*widthAdj)->get_value();
        double const miterlimit = (*miterLimitAdj)->get_value();
#else
        double width_typed = widthAdj->get_value();
        double const miterlimit = miterLimitAdj->get_value();
#endif

        Inkscape::Util::Unit const *const unit = unitSelector->getUnit();

        double *dash, offset;
        int ndash;
        dashSelector->get_dash(&ndash, &dash, &offset);

        for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();++i){
            /* Set stroke width */
            double width;
            if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
                width = Inkscape::Util::Quantity::convert(width_typed, unit, "px");
            } else { // percentage
                gdouble old_w = (*i)->style->stroke_width.computed;
                width = old_w * width_typed / 100;
            }

            {
                Inkscape::CSSOStringStream os_width;
                os_width << width;
                sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());
            }

            {
                Inkscape::CSSOStringStream os_ml;
                os_ml << miterlimit;
                sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());
            }

            /* Set dash */
            setScaledDash(css, ndash, dash, offset, width);

            sp_desktop_apply_css_recursive ((*i), css, true);
        }

        g_free(dash);

        if (unit->type != Inkscape::Util::UNIT_TYPE_LINEAR) {
            // reset to 100 percent
#if WITH_GTKMM_3_0
            (*widthAdj)->set_value(100.0);
#else
            widthAdj->set_value(100.0);
#endif
        }

    }

    // we have already changed the items, so set style without changing selection
    // FIXME: move the above stroke-setting stuff, including percentages, to desktop-style
    sp_desktop_set_style (desktop, css, false);

    sp_repr_css_attr_unref(css);
    css = 0;

    DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                       _("Set stroke style"));

    update = false;
}

/**
 * Callback for when the stroke style's width changes.
 * Causes all line styles to be applied to all selected items.
 */
void
StrokeStyle::widthChangedCB()
{
    if (update) {
        return;
    }

    scaleLine();
}

/**
 * Callback for when the stroke style's miterlimit changes.
 * Causes all line styles to be applied to all selected items.
 */
void
StrokeStyle::miterLimitChangedCB()
{
    if (update) {
        return;
    }

    scaleLine();
}

/**
 * Callback for when the stroke style's dash changes.
 * Causes all line styles to be applied to all selected items.
 */

void
StrokeStyle::lineDashChangedCB()
{
    if (update) {
        return;
    }

    scaleLine();
}

/**
 * This routine handles toggle events for buttons in the stroke style dialog.
 *
 * When activated, this routine gets the data for the various widgets, and then
 * calls the respective routines to update css properties, etc.
 *
 */
void StrokeStyle::buttonToggledCB(StrokeStyleButton *tb, StrokeStyle *spw)
{
    if (spw->update) {
        return;
    }

    if (tb->get_active()) {
        if (tb->get_button_type() == STROKE_STYLE_BUTTON_JOIN) {
            spw->miterLimitSpin->set_sensitive(!strcmp(tb->get_stroke_style(), "miter"));
        }

        /* TODO: Create some standardized method */
        SPCSSAttr *css = sp_repr_css_attr_new();

        switch (tb->get_button_type()) {
            case STROKE_STYLE_BUTTON_JOIN: 
                sp_repr_css_set_property(css, "stroke-linejoin", tb->get_stroke_style());
                sp_desktop_set_style (spw->desktop, css);
                spw->setJoinButtons(tb);
                break;
            case STROKE_STYLE_BUTTON_CAP:
                sp_repr_css_set_property(css, "stroke-linecap", tb->get_stroke_style());
                sp_desktop_set_style (spw->desktop, css);
                spw->setCapButtons(tb);
                break;
            case STROKE_STYLE_BUTTON_ORDER:
                sp_repr_css_set_property(css, "paint-order", tb->get_stroke_style());
                sp_desktop_set_style (spw->desktop, css);
                //spw->setPaintButtons(tb);
        }

        sp_repr_css_attr_unref(css);
        css = 0;

        DocumentUndo::done(spw->desktop->getDocument(), SP_VERB_DIALOG_FILL_STROKE, _("Set stroke style"));
    }
}

/**
 * Updates the join style toggle buttons
 */
void
StrokeStyle::setJoinButtons(Gtk::ToggleButton *active)
{
    joinMiter->set_active(active == joinMiter);
    miterLimitSpin->set_sensitive(active == joinMiter);
    joinRound->set_active(active == joinRound);
    joinBevel->set_active(active == joinBevel);
}

/**
 * Updates the cap style toggle buttons
 */
void
StrokeStyle::setCapButtons(Gtk::ToggleButton *active)
{
    capButt->set_active(active == capButt);
    capRound->set_active(active == capRound);
    capSquare->set_active(active == capSquare);
}


/**
 * Updates the paint order style toggle buttons
 */
void
StrokeStyle::setPaintOrderButtons(Gtk::ToggleButton *active)
{
    paintOrderFSM->set_active(active == paintOrderFSM);
    paintOrderSFM->set_active(active == paintOrderSFM);
    paintOrderFMS->set_active(active == paintOrderFMS);
    paintOrderMFS->set_active(active == paintOrderMFS);
    paintOrderSMF->set_active(active == paintOrderSMF);
    paintOrderMSF->set_active(active == paintOrderMSF);
}


/**
 * Updates the marker combobox to highlight the appropriate marker and scroll to
 * that marker.
 */
void
StrokeStyle::updateAllMarkers(std::vector<SPItem*> const &objects)
{
    struct { MarkerComboBox *key; int loc; } const keyloc[] = {
            { startMarkerCombo, SP_MARKER_LOC_START },
            { midMarkerCombo, SP_MARKER_LOC_MID },
            { endMarkerCombo, SP_MARKER_LOC_END }
    };

    bool all_texts = true;
    for(std::vector<SPItem*>::const_iterator i=objects.begin();i!=objects.end();++i){
        if (!SP_IS_TEXT (*i)) {
            all_texts = false;
            break;
        }
    }

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        MarkerComboBox *combo = static_cast<MarkerComboBox *>(keyloc[i].key);
        // Per SVG spec, text objects cannot have markers; disable combobox if only texts are selected
        combo->set_sensitive(!all_texts);
    }

    // We show markers of the first object in the list only
    // FIXME: use the first in the list that has the marker of each type, if any
    SPObject *object = objects[0];

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        // For all three marker types,

        // find the corresponding combobox item
        MarkerComboBox *combo = static_cast<MarkerComboBox *>(keyloc[i].key);

        // Quit if we're in update state
        if (combo->update()) {
            return;
        }

        combo->setDesktop(desktop);

        if (object->style->marker_ptrs[keyloc[i].loc]->value != NULL && !all_texts) {
            // If the object has this type of markers,

            // Extract the name of the marker that the object uses
            SPObject *marker = getMarkerObj(object->style->marker_ptrs[keyloc[i].loc]->value, object->document);
            // Scroll the combobox to that marker
            combo->set_current(marker);

            // Set the marker color
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            gboolean update = prefs->getBool("/options/markers/colorUpdateMarkers", true);

            if (update) {
                setMarkerColor(marker, combo->get_loc(), SP_ITEM(object));

                SPDocument *document = desktop->getDocument();
                DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                                   _("Set marker color"));
            }

        } else {
            combo->set_current(NULL);
        }
    }

}



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

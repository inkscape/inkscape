/**
 * @file
 * Inkscape toolbar definitions and general utility functions.
 * Each tool should have its own xxx-toolbar implementation file
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/action.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/toolitem.h>
#include <glibmm/i18n.h>

#include "../desktop.h"
#include "../desktop-handles.h"
#include "../desktop-style.h"
#include "document-undo.h"
#include "../ege-adjustment-action.h"
#include "../ege-output-action.h"
#include "../ege-select-one-action.h"
#include "../graphlayout.h"
#include "../helper/action.h"
#include "../helper/action-context.h"
#include "icon.h"
#include "../ink-action.h"
#include "../ink-comboboxentry-action.h"
#include "../inkscape.h"
#include "../interface.h"
#include "../shortcuts.h"
#include "../sp-namedview.h"
#include "../tools-switch.h"
#include "../ui/icon-names.h"
#include "../ui/widget/style-swatch.h"
#include "../verbs.h"
#include "../widgets/button.h"
#include "../widgets/spinbutton-events.h"
#include "ui/widget/spinbutton.h"
#include "../widgets/spw-utilities.h"
#include "../widgets/widget-sizes.h"
#include "../xml/attribute-record.h"
#include "../xml/node-event-vector.h"
#include "../xml/repr.h"
#include "ui/uxmanager.h"


#include "arc-toolbar.h"
#include "box3d-toolbar.h"
#include "calligraphy-toolbar.h"
#include "connector-toolbar.h"
#include "dropper-toolbar.h"
#include "eraser-toolbar.h"
#include "gradient-toolbar.h"
#include "lpe-toolbar.h"
#include "mesh-toolbar.h"
#include "measure-toolbar.h"
#include "node-toolbar.h"
#include "rect-toolbar.h"
#include "paintbucket-toolbar.h"
#include "pencil-toolbar.h"
#include "select-toolbar.h"
#include "spray-toolbar.h"
#include "spiral-toolbar.h"
#include "star-toolbar.h"
#include "tweak-toolbar.h"
#include "text-toolbar.h"
#include "zoom-toolbar.h"

#include "toolbox.h"
#include <gtk/gtk.h>

#include "ui/tools/tool-base.h"

//#define DEBUG_TEXT

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::PrefPusher;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::Tools::ToolBase;

typedef void (*SetupFunction)(GtkWidget *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

enum BarId {
    BAR_TOOL = 0,
    BAR_AUX,
    BAR_COMMANDS,
    BAR_SNAP,
};

#define BAR_ID_KEY "BarIdValue"
#define HANDLE_POS_MARK "x-inkscape-pos"

static GtkWidget *sp_empty_toolbox_new(SPDesktop *desktop);


Inkscape::IconSize ToolboxFactory::prefToSize( Glib::ustring const &path, int base ) {
    static Inkscape::IconSize sizeChoices[] = {
        Inkscape::ICON_SIZE_LARGE_TOOLBAR,
        Inkscape::ICON_SIZE_SMALL_TOOLBAR,
        Inkscape::ICON_SIZE_MENU
    };
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int index = prefs->getIntLimited( path, base, 0, G_N_ELEMENTS(sizeChoices) );
    return sizeChoices[index];
}

static struct {
    gchar const *type_name;
    gchar const *data_name;
    sp_verb_t verb;
    sp_verb_t doubleclick_verb;
} const tools[] = {
	{ "/tools/select",   "select_tool",    SP_VERB_CONTEXT_SELECT,  SP_VERB_CONTEXT_SELECT_PREFS},
	{ "/tools/nodes",     "node_tool",      SP_VERB_CONTEXT_NODE, SP_VERB_CONTEXT_NODE_PREFS },
	{ "/tools/tweak",    "tweak_tool",     SP_VERB_CONTEXT_TWEAK, SP_VERB_CONTEXT_TWEAK_PREFS },
	{ "/tools/spray",    "spray_tool",     SP_VERB_CONTEXT_SPRAY, SP_VERB_CONTEXT_SPRAY_PREFS },
	{ "/tools/zoom",     "zoom_tool",      SP_VERB_CONTEXT_ZOOM, SP_VERB_CONTEXT_ZOOM_PREFS },
	{ "/tools/measure",  "measure_tool",   SP_VERB_CONTEXT_MEASURE, SP_VERB_CONTEXT_MEASURE_PREFS },
	{ "/tools/shapes/rect",     "rect_tool",      SP_VERB_CONTEXT_RECT, SP_VERB_CONTEXT_RECT_PREFS },
	{ "/tools/shapes/3dbox",      "3dbox_tool",     SP_VERB_CONTEXT_3DBOX, SP_VERB_CONTEXT_3DBOX_PREFS },
	{ "/tools/shapes/arc",      "arc_tool",       SP_VERB_CONTEXT_ARC, SP_VERB_CONTEXT_ARC_PREFS },
	{ "/tools/shapes/star",     "star_tool",      SP_VERB_CONTEXT_STAR, SP_VERB_CONTEXT_STAR_PREFS },
	{ "/tools/shapes/spiral",   "spiral_tool",    SP_VERB_CONTEXT_SPIRAL, SP_VERB_CONTEXT_SPIRAL_PREFS },
	{ "/tools/freehand/pencil",   "pencil_tool",    SP_VERB_CONTEXT_PENCIL, SP_VERB_CONTEXT_PENCIL_PREFS },
	{ "/tools/freehand/pen",      "pen_tool",       SP_VERB_CONTEXT_PEN, SP_VERB_CONTEXT_PEN_PREFS },
	{ "/tools/calligraphic", "dyna_draw_tool", SP_VERB_CONTEXT_CALLIGRAPHIC, SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS },
	{ "/tools/lpetool",  "lpetool_tool",   SP_VERB_CONTEXT_LPETOOL, SP_VERB_CONTEXT_LPETOOL_PREFS },
	{ "/tools/eraser",   "eraser_tool",    SP_VERB_CONTEXT_ERASER, SP_VERB_CONTEXT_ERASER_PREFS },
	{ "/tools/paintbucket",    "paintbucket_tool",     SP_VERB_CONTEXT_PAINTBUCKET, SP_VERB_CONTEXT_PAINTBUCKET_PREFS },
	{ "/tools/text",     "text_tool",      SP_VERB_CONTEXT_TEXT, SP_VERB_CONTEXT_TEXT_PREFS },
	{ "/tools/connector","connector_tool", SP_VERB_CONTEXT_CONNECTOR, SP_VERB_CONTEXT_CONNECTOR_PREFS },
	{ "/tools/gradient", "gradient_tool",  SP_VERB_CONTEXT_GRADIENT, SP_VERB_CONTEXT_GRADIENT_PREFS },
	{ "/tools/mesh",     "mesh_tool",      SP_VERB_CONTEXT_MESH, SP_VERB_CONTEXT_MESH_PREFS },
	{ "/tools/dropper",  "dropper_tool",   SP_VERB_CONTEXT_DROPPER, SP_VERB_CONTEXT_DROPPER_PREFS },
	{ NULL, NULL, 0, 0 }
};

static struct {
    gchar const *type_name;
    gchar const *data_name;
    GtkWidget *(*create_func)(SPDesktop *desktop);
    void (*prep_func)(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
    gchar const *ui_name;
    gint swatch_verb_id;
    gchar const *swatch_tool;
    gchar const *swatch_tip;
} const aux_toolboxes[] = {
    { "/tools/select", "select_toolbox", 0, sp_select_toolbox_prep,            "SelectToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/nodes",   "node_toolbox",   0, sp_node_toolbox_prep,              "NodeToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/tweak",   "tweak_toolbox",   0, sp_tweak_toolbox_prep,              "TweakToolbar",
      SP_VERB_CONTEXT_TWEAK_PREFS, "/tools/tweak", N_("Color/opacity used for color tweaking")},
    { "/tools/spray",   "spray_toolbox",   0, sp_spray_toolbox_prep,              "SprayToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/zoom",   "zoom_toolbox",   0, sp_zoom_toolbox_prep,              "ZoomToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/measure",   "measure_toolbox",   0, sp_measure_toolbox_prep,              "MeasureToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/shapes/star",   "star_toolbox",   0, sp_star_toolbox_prep,              "StarToolbar",
      SP_VERB_CONTEXT_STAR_PREFS,   "/tools/shapes/star",     N_("Style of new stars")},
    { "/tools/shapes/rect",   "rect_toolbox",   0, sp_rect_toolbox_prep,              "RectToolbar",
      SP_VERB_CONTEXT_RECT_PREFS,   "/tools/shapes/rect",     N_("Style of new rectangles")},
    { "/tools/shapes/3dbox",  "3dbox_toolbox",  0, box3d_toolbox_prep,             "3DBoxToolbar",
      SP_VERB_CONTEXT_3DBOX_PREFS,  "/tools/shapes/3dbox",    N_("Style of new 3D boxes")},
    { "/tools/shapes/arc",    "arc_toolbox",    0, sp_arc_toolbox_prep,               "ArcToolbar",
      SP_VERB_CONTEXT_ARC_PREFS,    "/tools/shapes/arc",      N_("Style of new ellipses")},
    { "/tools/shapes/spiral", "spiral_toolbox", 0, sp_spiral_toolbox_prep,            "SpiralToolbar",
      SP_VERB_CONTEXT_SPIRAL_PREFS, "/tools/shapes/spiral",   N_("Style of new spirals")},
    { "/tools/freehand/pencil", "pencil_toolbox", 0, sp_pencil_toolbox_prep,            "PencilToolbar",
      SP_VERB_CONTEXT_PENCIL_PREFS, "/tools/freehand/pencil", N_("Style of new paths created by Pencil")},
    { "/tools/freehand/pen", "pen_toolbox", 0, sp_pen_toolbox_prep,                     "PenToolbar",
      SP_VERB_CONTEXT_PEN_PREFS,    "/tools/freehand/pen",    N_("Style of new paths created by Pen")},
    { "/tools/calligraphic", "calligraphy_toolbox", 0, sp_calligraphy_toolbox_prep,"CalligraphyToolbar",
      SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS, "/tools/calligraphic", N_("Style of new calligraphic strokes")},
    { "/tools/eraser", "eraser_toolbox", 0, sp_eraser_toolbox_prep,"EraserToolbar",
      SP_VERB_CONTEXT_ERASER_PREFS, "/tools/eraser", _("TBD")},
    { "/tools/lpetool", "lpetool_toolbox", 0, sp_lpetool_toolbox_prep, "LPEToolToolbar",
      SP_VERB_CONTEXT_LPETOOL_PREFS, "/tools/lpetool", _("TBD")},
    { "/tools/text",   "text_toolbox",   0, sp_text_toolbox_prep, "TextToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/dropper", "dropper_toolbox", 0, sp_dropper_toolbox_prep,         "DropperToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/connector", "connector_toolbox", 0, sp_connector_toolbox_prep,   "ConnectorToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/gradient", "gradient_toolbox", 0, sp_gradient_toolbox_prep, "GradientToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/mesh", "mesh_toolbox", 0, sp_mesh_toolbox_prep, "MeshToolbar",
      SP_VERB_INVALID, 0, 0},
    { "/tools/paintbucket",  "paintbucket_toolbox",  0, sp_paintbucket_toolbox_prep, "PaintbucketToolbar",
      SP_VERB_CONTEXT_PAINTBUCKET_PREFS, "/tools/paintbucket", N_("Style of Paint Bucket fill objects")},
    { NULL, NULL, NULL, NULL, NULL, SP_VERB_INVALID, NULL, NULL }
};

static gchar const * ui_descr =
        "<ui>"
        "  <toolbar name='SelectToolbar'>"
        "    <toolitem action='EditSelectAll' />"
        "    <toolitem action='EditSelectAllInAllLayers' />"
        "    <toolitem action='EditDeselect' />"
        "    <separator />"
        "    <toolitem action='ObjectRotate90CCW' />"
        "    <toolitem action='ObjectRotate90' />"
        "    <toolitem action='ObjectFlipHorizontally' />"
        "    <toolitem action='ObjectFlipVertically' />"
        "    <separator />"
        "    <toolitem action='SelectionToBack' />"
        "    <toolitem action='SelectionLower' />"
        "    <toolitem action='SelectionRaise' />"
        "    <toolitem action='SelectionToFront' />"
        "    <separator />"
        "    <toolitem action='XAction' />"
        "    <toolitem action='YAction' />"
        "    <toolitem action='WidthAction' />"
        "    <toolitem action='LockAction' />"
        "    <toolitem action='HeightAction' />"
        "    <toolitem action='UnitsAction' />"
        "    <separator />"
        "    <toolitem action='transform_stroke' />"
        "    <toolitem action='transform_corners' />"
        "    <toolitem action='transform_gradient' />"
        "    <toolitem action='transform_pattern' />"
        "  </toolbar>"

        "  <toolbar name='NodeToolbar'>"
        "    <separator />"
        "    <toolitem action='NodeInsertAction'>"
        "      <menu action='NodeInsertActionMenu'>"
        "        <menuitem action='NodeInsertActionMinX' />"
        "        <menuitem action='NodeInsertActionMaxX' />"
        "        <menuitem action='NodeInsertActionMinY' />"
        "        <menuitem action='NodeInsertActionMaxY' />"
        "      </menu>"
        "    </toolitem>"
        "    <toolitem action='NodeDeleteAction' />"
        "    <separator />"
        "    <toolitem action='NodeJoinAction' />"
        "    <toolitem action='NodeBreakAction' />"
        "    <separator />"
        "    <toolitem action='NodeJoinSegmentAction' />"
        "    <toolitem action='NodeDeleteSegmentAction' />"
        "    <separator />"
        "    <toolitem action='NodeCuspAction' />"
        "    <toolitem action='NodeSmoothAction' />"
        "    <toolitem action='NodeSymmetricAction' />"
        "    <toolitem action='NodeAutoAction' />"
        "    <separator />"
        "    <toolitem action='NodeLineAction' />"
        "    <toolitem action='NodeCurveAction' />"
        "    <separator />"
        "    <toolitem action='ObjectToPath' />"
        "    <toolitem action='StrokeToPath' />"
        "    <separator />"
        "    <toolitem action='NodeXAction' />"
        "    <toolitem action='NodeYAction' />"
        "    <toolitem action='NodeUnitsAction' />"
        "    <separator />"
        "    <toolitem action='ObjectEditClipPathAction' />"
        "    <toolitem action='ObjectEditMaskPathAction' />"
        "    <toolitem action='EditNextPathEffectParameter' />"
        "    <separator />"
        "    <toolitem action='NodesShowTransformHandlesAction' />"
        "    <toolitem action='NodesShowHandlesAction' />"
        "    <toolitem action='NodesShowHelperpath' />"
        "  </toolbar>"

        "  <toolbar name='TweakToolbar'>"
        "    <toolitem action='TweakWidthAction' />"
        "    <separator />"
        "    <toolitem action='TweakForceAction' />"
        "    <toolitem action='TweakPressureAction' />"
        "    <separator />"
        "    <toolitem action='TweakModeAction' />"
        "    <separator />"
        "    <toolitem action='TweakFidelityAction' />"
        "    <separator />"
        "    <toolitem action='TweakChannelsLabel' />"
        "    <toolitem action='TweakDoH' />"
        "    <toolitem action='TweakDoS' />"
        "    <toolitem action='TweakDoL' />"
        "    <toolitem action='TweakDoO' />"
        "  </toolbar>"

        "  <toolbar name='SprayToolbar'>"
        "    <toolitem action='SprayModeAction' />"
        "    <separator />"
        "    <toolitem action='SprayWidthAction' />"
        "    <toolitem action='SprayPressureAction' />"
        "    <toolitem action='SprayPopulationAction' />"
        "    <separator />"
        "    <toolitem action='SprayRotationAction' />"
        "    <toolitem action='SprayScaleAction' />"
        "    <separator />"
        "    <toolitem action='SprayStandard_deviationAction' />"
        "    <toolitem action='SprayMeanAction' />"
        "  </toolbar>"

        "  <toolbar name='ZoomToolbar'>"
        "    <toolitem action='ZoomIn' />"
        "    <toolitem action='ZoomOut' />"
        "    <separator />"
        "    <toolitem action='Zoom1:0' />"
        "    <toolitem action='Zoom1:2' />"
        "    <toolitem action='Zoom2:1' />"
        "    <separator />"
        "    <toolitem action='ZoomSelection' />"
        "    <toolitem action='ZoomDrawing' />"
        "    <toolitem action='ZoomPage' />"
        "    <toolitem action='ZoomPageWidth' />"
        "    <separator />"
        "    <toolitem action='ZoomPrev' />"
        "    <toolitem action='ZoomNext' />"
        "  </toolbar>"

        "  <toolbar name='MeasureToolbar'>"
        "    <toolitem action='MeasureFontSizeAction' />"
        "    <separator />"
        "    <toolitem action='measure_units_label' />"
        "    <toolitem action='MeasureUnitsAction' />"
        "  </toolbar>"

        "  <toolbar name='StarToolbar'>"
        "    <separator />"
        "    <toolitem action='StarStateAction' />"
        "    <separator />"
        "    <toolitem action='FlatAction' />"
        "    <separator />"
        "    <toolitem action='MagnitudeAction' />"
        "    <toolitem action='SpokeAction' />"
        "    <toolitem action='RoundednessAction' />"
        "    <toolitem action='RandomizationAction' />"
        "    <separator />"
        "    <toolitem action='StarResetAction' />"
        "  </toolbar>"

        "  <toolbar name='RectToolbar'>"
        "    <toolitem action='RectStateAction' />"
        "    <toolitem action='RectWidthAction' />"
        "    <toolitem action='RectHeightAction' />"
        "    <toolitem action='RadiusXAction' />"
        "    <toolitem action='RadiusYAction' />"
        "    <toolitem action='RectUnitsAction' />"
        "    <separator />"
        "    <toolitem action='RectResetAction' />"
        "  </toolbar>"

        "  <toolbar name='3DBoxToolbar'>"
        "    <toolitem action='3DBoxAngleXAction' />"
        "    <toolitem action='3DBoxVPXStateAction' />"
        "    <separator />"
        "    <toolitem action='3DBoxAngleYAction' />"
        "    <toolitem action='3DBoxVPYStateAction' />"
        "    <separator />"
        "    <toolitem action='3DBoxAngleZAction' />"
        "    <toolitem action='3DBoxVPZStateAction' />"
        "  </toolbar>"

        "  <toolbar name='SpiralToolbar'>"
        "    <toolitem action='SpiralStateAction' />"
        "    <toolitem action='SpiralRevolutionAction' />"
        "    <toolitem action='SpiralExpansionAction' />"
        "    <toolitem action='SpiralT0Action' />"
        "    <separator />"
        "    <toolitem action='SpiralResetAction' />"
        "  </toolbar>"

        "  <toolbar name='PenToolbar'>"
        "    <toolitem action='FreehandModeActionPen' />"
        "    <separator />"
        "    <toolitem action='SetPenShapeAction'/>"
        "  </toolbar>"

        "  <toolbar name='PencilToolbar'>"
        "    <toolitem action='FreehandModeActionPencil' />"
        "    <separator />"
        "    <toolitem action='PencilToleranceAction' />"
        "    <separator />"
        "    <toolitem action='PencilResetAction' />"
        "    <separator />"
        "    <toolitem action='SetPencilShapeAction'/>"
        "  </toolbar>"

        "  <toolbar name='CalligraphyToolbar'>"
        "    <separator />"
        "    <toolitem action='SetProfileAction'/>"
        "    <toolitem action='ProfileEditAction'/>"
        "    <separator />"
        "    <toolitem action='CalligraphyWidthAction' />"
        "    <toolitem action='PressureAction' />"
        "    <toolitem action='TraceAction' />"
        "    <toolitem action='ThinningAction' />"
        "    <separator />"
        "    <toolitem action='AngleAction' />"
        "    <toolitem action='TiltAction' />"
        "    <toolitem action='FixationAction' />"
        "    <separator />"
        "    <toolitem action='CapRoundingAction' />"
        "    <separator />"
        "    <toolitem action='TremorAction' />"
        "    <toolitem action='WiggleAction' />"
        "    <toolitem action='MassAction' />"
        "    <separator />"
        "  </toolbar>"

        "  <toolbar name='ArcToolbar'>"
        "    <toolitem action='ArcStateAction' />"
        "    <separator />"
        "    <toolitem action='ArcStartAction' />"
        "    <toolitem action='ArcEndAction' />"
        "    <separator />"
        "    <toolitem action='ArcOpenAction' />"
        "    <separator />"
        "    <toolitem action='ArcResetAction' />"
        "    <separator />"
        "  </toolbar>"

        "  <toolbar name='PaintbucketToolbar'>"
        "    <toolitem action='ChannelsAction' />"
        "    <separator />"
        "    <toolitem action='ThresholdAction' />"
        "    <separator />"
        "    <toolitem action='OffsetAction' />"
        "    <toolitem action='PaintbucketUnitsAction' />"
        "    <separator />"
        "    <toolitem action='AutoGapAction' />"
        "    <separator />"
        "    <toolitem action='PaintbucketResetAction' />"
        "  </toolbar>"

        "  <toolbar name='EraserToolbar'>"
        "    <toolitem action='EraserModeAction' />"
        "    <separator />"
        "    <toolitem action='EraserWidthAction' />"
        "  </toolbar>"

        "  <toolbar name='TextToolbar'>"
        "    <toolitem action='TextFontFamilyAction' />"
        "    <toolitem action='TextFontSizeAction' />"
        "    <toolitem action='TextFontStyleAction' />"
//        "    <toolitem action='TextBoldAction' />"
//        "    <toolitem action='TextItalicAction' />"
        "    <separator />"
        "    <toolitem action='TextAlignAction' />"
        "    <separator />"
        "    <toolitem action='TextSuperscriptAction' />"
        "    <toolitem action='TextSubscriptAction' />"
        "    <separator />"
        "    <toolitem action='TextLineHeightAction' />"
        "    <toolitem action='TextLetterSpacingAction' />"
        "    <toolitem action='TextWordSpacingAction' />"
        "    <toolitem action='TextDxAction' />"
        "    <toolitem action='TextDyAction' />"
        "    <toolitem action='TextRotationAction' />"
        "    <separator />"
        "    <toolitem action='TextOrientationAction' />"
        "  </toolbar>"

        "  <toolbar name='LPEToolToolbar'>"
        "    <toolitem action='LPEToolModeAction' />"
        "    <separator />"
        "    <toolitem action='LPEShowBBoxAction' />"
        "    <toolitem action='LPEBBoxFromSelectionAction' />"
        "    <separator />"
        "    <toolitem action='LPELineSegmentAction' />"
        "    <separator />"
        "    <toolitem action='LPEMeasuringAction' />"
        "    <toolitem action='LPEToolUnitsAction' />"
        "    <separator />"
        "    <toolitem action='LPEOpenLPEDialogAction' />"
        "  </toolbar>"

        "  <toolbar name='GradientToolbar'>"
        "    <toolitem action='GradientNewTypeAction' />"
        "    <toolitem action='GradientNewFillStrokeAction' />"
        "    <separator />"
        "    <toolitem action='GradientSelectGradientAction' />"
        "    <toolitem action='GradientEditLinkAction' />"
        "    <toolitem action='GradientEditReverseAction' />"
        "    <toolitem action='GradientSelectRepeatAction' />"
        "    <separator />"
        "    <toolitem action='GradientEditStopsAction' />"
        "    <toolitem action='GradientEditOffsetAction' />"
        "    <toolitem action='GradientEditAddAction' />"
        "    <toolitem action='GradientEditDeleteAction' />"
        "  </toolbar>"

        "  <toolbar name='MeshToolbar'>"
        "    <toolitem action='MeshNewTypeAction' />"
        "    <toolitem action='MeshNewFillStrokeAction' />"
        "    <toolitem action='MeshRowAction' />"
        "    <toolitem action='MeshColumnAction' />"
        "    <separator />"
//        "    <toolitem action='MeshEditFillAction' />"
//        "    <toolitem action='MeshEditStrokeAction' />"
//        "    <toolitem action='MeshShowHandlesAction' />"
        "    <separator />"
        "  </toolbar>"

        "  <toolbar name='DropperToolbar'>"
        "    <toolitem action='DropperOpacityAction' />"
        "    <toolitem action='DropperPickAlphaAction' />"
        "    <toolitem action='DropperSetAlphaAction' />"
        "  </toolbar>"

        "  <toolbar name='ConnectorToolbar'>"
        "    <toolitem action='ConnectorAvoidAction' />"
        "    <toolitem action='ConnectorIgnoreAction' />"
        "    <toolitem action='ConnectorOrthogonalAction' />"
        "    <toolitem action='ConnectorCurvatureAction' />"
        "    <toolitem action='ConnectorSpacingAction' />"
        "    <toolitem action='ConnectorGraphAction' />"
        "    <toolitem action='ConnectorLengthAction' />"
        "    <toolitem action='ConnectorDirectedAction' />"
        "    <toolitem action='ConnectorOverlapAction' />"
        "  </toolbar>"

        "</ui>"
;

static Glib::RefPtr<Gtk::ActionGroup> create_or_fetch_actions( SPDesktop* desktop );

static void setup_snap_toolbox(GtkWidget *toolbox, SPDesktop *desktop);

static void setup_tool_toolbox(GtkWidget *toolbox, SPDesktop *desktop);
static void update_tool_toolbox(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

static void setup_aux_toolbox(GtkWidget *toolbox, SPDesktop *desktop);
static void update_aux_toolbox(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

static void setup_commands_toolbox(GtkWidget *toolbox, SPDesktop *desktop);
static void update_commands_toolbox(SPDesktop *desktop, ToolBase *eventcontext, GtkWidget *toolbox);

static GtkToolItem * sp_toolbox_button_item_new_from_verb_with_doubleclick( GtkWidget *t, Inkscape::IconSize size, SPButtonType type,
                                                                     Inkscape::Verb *verb, Inkscape::Verb *doubleclick_verb,
                                                                     Inkscape::UI::View::View *view);

class VerbAction : public Gtk::Action {
public:
    static Glib::RefPtr<VerbAction> create(Inkscape::Verb* verb, Inkscape::Verb* verb2, Inkscape::UI::View::View *view);

    virtual ~VerbAction();
    virtual void set_active(bool active = true);

protected:
    virtual Gtk::Widget* create_menu_item_vfunc();
    virtual Gtk::Widget* create_tool_item_vfunc();

    virtual void connect_proxy_vfunc(Gtk::Widget* proxy);
    virtual void disconnect_proxy_vfunc(Gtk::Widget* proxy);

    virtual void on_activate();

private:
    Inkscape::Verb* verb;
    Inkscape::Verb* verb2;
    Inkscape::UI::View::View *view;
    bool active;

    VerbAction(Inkscape::Verb* verb, Inkscape::Verb* verb2, Inkscape::UI::View::View *view);
};


Glib::RefPtr<VerbAction> VerbAction::create(Inkscape::Verb* verb, Inkscape::Verb* verb2, Inkscape::UI::View::View *view)
{
    Glib::RefPtr<VerbAction> result;
    SPAction *action = verb->get_action(Inkscape::ActionContext(view));
    if ( action ) {
        //SPAction* action2 = verb2 ? verb2->get_action(Inkscape::ActionContext(view)) : 0;
        result = Glib::RefPtr<VerbAction>(new VerbAction(verb, verb2, view));
    }

    return result;
}

VerbAction::VerbAction(Inkscape::Verb* verb, Inkscape::Verb* verb2, Inkscape::UI::View::View *view) :
    Gtk::Action(Glib::ustring(verb->get_id()), Gtk::StockID(verb->get_image()), Glib::ustring(g_dpgettext2(NULL, "ContextVerb", verb->get_name())), Glib::ustring(_(verb->get_tip()))),
    verb(verb),
    verb2(verb2),
    view(view),
    active(false)
{
}

VerbAction::~VerbAction()
{
}

Gtk::Widget* VerbAction::create_menu_item_vfunc()
{
// First call in to get the icon rendered if present in SVG
    Gtk::Widget *widget = sp_icon_get_icon( property_stock_id().get_value().get_string(), Inkscape::ICON_SIZE_MENU );
    delete widget;
    widget = 0;

    Gtk::Widget* widg = Gtk::Action::create_menu_item_vfunc();
//     g_message("create_menu_item_vfunc() = %p  for '%s'", widg, verb->get_id());
    return widg;
}

Gtk::Widget* VerbAction::create_tool_item_vfunc()
{
//     Gtk::Widget* widg = Gtk::Action::create_tool_item_vfunc();
    Inkscape::IconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/tools/small");
    GtkWidget* toolbox = 0;
    GtkToolItem *button_toolitem = sp_toolbox_button_item_new_from_verb_with_doubleclick( toolbox, toolboxSize,
                                                                                      SP_BUTTON_TYPE_TOGGLE,
                                                                                      verb,
                                                                                      verb2,
                                                                                      view );

    GtkWidget* button_widget = gtk_bin_get_child(GTK_BIN(button_toolitem));

    if ( active ) {
        sp_button_toggle_set_down( SP_BUTTON(button_widget), active);
    }
    gtk_widget_show_all( button_widget );
    Gtk::ToolItem* holder = Glib::wrap(button_toolitem);

//     g_message("create_tool_item_vfunc() = %p  for '%s'", holder, verb->get_id());
    return holder;
}

void VerbAction::connect_proxy_vfunc(Gtk::Widget* proxy)
{
//     g_message("connect_proxy_vfunc(%p)  for '%s'", proxy, verb->get_id());
    Gtk::Action::connect_proxy_vfunc(proxy);
}

void VerbAction::disconnect_proxy_vfunc(Gtk::Widget* proxy)
{
//     g_message("disconnect_proxy_vfunc(%p)  for '%s'", proxy, verb->get_id());
    Gtk::Action::disconnect_proxy_vfunc(proxy);
}

void VerbAction::set_active(bool active)
{
    this->active = active;
    Glib::SListHandle<Gtk::Widget*> proxies = get_proxies();
    for ( Glib::SListHandle<Gtk::Widget*>::iterator it = proxies.begin(); it != proxies.end(); ++it ) {
        Gtk::ToolItem* ti = dynamic_cast<Gtk::ToolItem*>(*it);
        if (ti) {
            // *should* have one child that is the SPButton
            Gtk::Widget* child = ti->get_child();
            if ( child && SP_IS_BUTTON(child->gobj()) ) {
                SPButton* button = SP_BUTTON(child->gobj());
                sp_button_toggle_set_down( button, active );
            }
        }
    }
}

void VerbAction::on_activate()
{
    if ( verb ) {
        SPAction *action = verb->get_action(Inkscape::ActionContext(view));
        if ( action ) {
            sp_action_perform(action, 0);
        }
    }
}

/* Global text entry widgets necessary for update */
/* GtkWidget *dropper_rgb_entry,
          *dropper_opacity_entry ; */
// should be made a private member once this is converted to class

void delete_connection(GObject * /*obj*/, sigc::connection *connection)
{
    connection->disconnect();
    delete connection;
}

void purge_repr_listener( GObject* /*obj*/, GObject* tbl )
{
    Inkscape::XML::Node* oldrepr = reinterpret_cast<Inkscape::XML::Node *>( g_object_get_data( tbl, "repr" ) );
    if (oldrepr) { // remove old listener
        sp_repr_remove_listener_by_data(oldrepr, tbl);
        Inkscape::GC::release(oldrepr);
        oldrepr = 0;
        g_object_set_data( tbl, "repr", NULL );
    }
}

PrefPusher::PrefPusher( GtkToggleAction *act, Glib::ustring const &path, void (*callback)(GObject*), GObject *cbData ) :
    Observer(path),
    act(act),
    callback(callback),
    cbData(cbData),
    freeze(false)
{
    g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggleCB), this);
    freeze = true;
    gtk_toggle_action_set_active( act, Inkscape::Preferences::get()->getBool(observed_path) );
    freeze = false;

    Inkscape::Preferences::get()->addObserver(*this);
}

PrefPusher::~PrefPusher()
{
    Inkscape::Preferences::get()->removeObserver(*this);
}

void PrefPusher::toggleCB( GtkToggleAction * /*act*/, PrefPusher *self )
{
    if (self) {
        self->handleToggled();
    }
}

void PrefPusher::handleToggled()
{
    if (!freeze) {
        freeze = true;
        Inkscape::Preferences::get()->setBool(observed_path, gtk_toggle_action_get_active( act ));
        if (callback) {
            (*callback)(cbData);
        }
        freeze = false;
    }
}

void PrefPusher::notify(Inkscape::Preferences::Entry const &newVal)
{
    bool newBool = newVal.getBool();
    bool oldBool = gtk_toggle_action_get_active(act);

    if (!freeze && (newBool != oldBool)) {
        gtk_toggle_action_set_active( act, newBool );
    }
}

void delete_prefspusher(GObject * /*obj*/, PrefPusher *watcher )
{
    delete watcher;
}

// ------------------------------------------------------


GtkToolItem * sp_toolbox_button_item_new_from_verb_with_doubleclick(GtkWidget *t, Inkscape::IconSize size, SPButtonType type,
                                                             Inkscape::Verb *verb, Inkscape::Verb *doubleclick_verb,
                                                             Inkscape::UI::View::View *view)
{
    SPAction *action = verb->get_action(Inkscape::ActionContext(view));
    if (!action) {
        return NULL;
    }

    SPAction *doubleclick_action;
    if (doubleclick_verb) {
        doubleclick_action = doubleclick_verb->get_action(Inkscape::ActionContext(view));
    } else {
        doubleclick_action = NULL;
    }

    /* fixme: Handle sensitive/unsensitive */
    /* fixme: Implement sp_button_new_from_action */
    GtkWidget *b = sp_button_new(size, type, action, doubleclick_action);
    gtk_widget_show(b);
    GtkToolItem *b_toolitem = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(b_toolitem), b);

    unsigned int shortcut = sp_shortcut_get_primary(verb);
    if (shortcut != GDK_KEY_VoidSymbol) {
        gchar *key = sp_shortcut_get_label(shortcut);
        gchar *tip = g_strdup_printf ("%s (%s)", action->tip, key);
        if ( t ) {
           gtk_toolbar_insert(GTK_TOOLBAR(t), b_toolitem, -1);
           gtk_widget_set_tooltip_text(b, tip);
        }
        g_free(tip);
        g_free(key);
    } else {
        if ( t ) {
            gtk_toolbar_insert(GTK_TOOLBAR(t), b_toolitem, -1);
            gtk_widget_set_tooltip_text(b, action->tip);
        }
    }

    return b_toolitem;
}


static void trigger_sp_action( GtkAction* /*act*/, gpointer user_data )
{
    SPAction* targetAction = SP_ACTION(user_data);
    if ( targetAction ) {
        sp_action_perform( targetAction, NULL );
    }
}

static GtkAction* create_action_for_verb( Inkscape::Verb* verb, Inkscape::UI::View::View* view, Inkscape::IconSize size )
{
    GtkAction* act = 0;

    SPAction* targetAction = verb->get_action(Inkscape::ActionContext(view));
    InkAction* inky = ink_action_new( verb->get_id(), _(verb->get_name()), verb->get_tip(), verb->get_image(), size  );
    act = GTK_ACTION(inky);
    gtk_action_set_sensitive( act, targetAction->sensitive );

    g_signal_connect( G_OBJECT(inky), "activate", G_CALLBACK(trigger_sp_action), targetAction );

    // FIXME: memory leak: this is not unrefed anywhere
    g_object_ref(G_OBJECT(targetAction));
    g_object_set_data_full(G_OBJECT(inky), "SPAction", (void*) targetAction, (GDestroyNotify) &g_object_unref);
    targetAction->signal_set_sensitive.connect(
        sigc::bind<0>(
            sigc::ptr_fun(&gtk_action_set_sensitive),
            GTK_ACTION(inky)));

    return act;
}

static std::map<SPDesktop*, Glib::RefPtr<Gtk::ActionGroup> > groups;

static void desktopDestructHandler(SPDesktop *desktop)
{
    std::map<SPDesktop*, Glib::RefPtr<Gtk::ActionGroup> >::iterator it = groups.find(desktop);
    if (it != groups.end())
    {
        groups.erase(it);
    }
}

static Glib::RefPtr<Gtk::ActionGroup> create_or_fetch_actions( SPDesktop* desktop )
{
    Inkscape::UI::View::View *view = desktop;
    gint verbsToUse[] = {
        // disabled until we have icons for them:
        //find
        //SP_VERB_EDIT_TILE,
        //SP_VERB_EDIT_UNTILE,
        SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
        SP_VERB_DIALOG_DISPLAY,
        SP_VERB_DIALOG_FILL_STROKE,
        SP_VERB_DIALOG_NAMEDVIEW,
        SP_VERB_DIALOG_TEXT,
        SP_VERB_DIALOG_XML_EDITOR,
        SP_VERB_DIALOG_LAYERS,
        SP_VERB_EDIT_CLONE,
        SP_VERB_EDIT_COPY,
        SP_VERB_EDIT_CUT,
        SP_VERB_EDIT_DUPLICATE,
        SP_VERB_EDIT_PASTE,
        SP_VERB_EDIT_REDO,
        SP_VERB_EDIT_UNDO,
        SP_VERB_EDIT_UNLINK_CLONE,
        //SP_VERB_FILE_EXPORT,
        SP_VERB_DIALOG_EXPORT,
        SP_VERB_FILE_IMPORT,
        SP_VERB_FILE_NEW,
        SP_VERB_FILE_OPEN,
        SP_VERB_FILE_PRINT,
        SP_VERB_FILE_SAVE,
        SP_VERB_OBJECT_TO_CURVE,
        SP_VERB_SELECTION_GROUP,
        SP_VERB_SELECTION_OUTLINE,
        SP_VERB_SELECTION_UNGROUP,
        SP_VERB_ZOOM_1_1,
        SP_VERB_ZOOM_1_2,
        SP_VERB_ZOOM_2_1,
        SP_VERB_ZOOM_DRAWING,
        SP_VERB_ZOOM_IN,
        SP_VERB_ZOOM_NEXT,
        SP_VERB_ZOOM_OUT,
        SP_VERB_ZOOM_PAGE,
        SP_VERB_ZOOM_PAGE_WIDTH,
        SP_VERB_ZOOM_PREV,
        SP_VERB_ZOOM_SELECTION
    };

    Inkscape::IconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/small");

    Glib::RefPtr<Gtk::ActionGroup> mainActions;
    if ( groups.find(desktop) != groups.end() ) {
        mainActions = groups[desktop];
    }

    if ( !mainActions ) {
        mainActions = Gtk::ActionGroup::create("main");
        groups[desktop] = mainActions;
        if (desktop)
        {
            desktop->connectDestroy(&desktopDestructHandler);
        }
    }

    for ( guint i = 0; i < G_N_ELEMENTS(verbsToUse); i++ ) {
        Inkscape::Verb* verb = Inkscape::Verb::get(verbsToUse[i]);
        if ( verb ) {
            if (!mainActions->get_action(verb->get_id())) {
                GtkAction* act = create_action_for_verb( verb, view, toolboxSize );
                mainActions->add(Glib::wrap(act));
            }
        }
    }

    if ( !mainActions->get_action("ToolZoom") ) {
        for ( guint i = 0; i < G_N_ELEMENTS(tools) && tools[i].type_name; i++ ) {
            Glib::RefPtr<VerbAction> va = VerbAction::create(Inkscape::Verb::get(tools[i].verb), Inkscape::Verb::get(tools[i].doubleclick_verb), view);
            if ( va ) {
                mainActions->add(va);
                if ( i == 0 ) {
                    va->set_active(true);
                }
            }
        }
    }

    return mainActions;
}


static GtkWidget* toolboxNewCommon( GtkWidget* tb, BarId id, GtkPositionType /*handlePos*/ )
{
    g_object_set_data(G_OBJECT(tb), "desktop", NULL);

    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_event_box_new(); // A simple, neutral container.

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    gpointer val = GINT_TO_POINTER(id);
    g_object_set_data(G_OBJECT(hb), BAR_ID_KEY, val);

    return hb;
}

GtkWidget *ToolboxFactory::createToolToolbox()
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);
#else
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);
#endif

    return toolboxNewCommon( tb, BAR_TOOL, GTK_POS_TOP );
}

GtkWidget *ToolboxFactory::createAuxToolbox()
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);
#else
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);
#endif

    return toolboxNewCommon( tb, BAR_AUX, GTK_POS_LEFT );
}

//####################################
//# Commands Bar
//####################################

GtkWidget *ToolboxFactory::createCommandsToolbox()
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);
#else
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);
#endif

    return toolboxNewCommon( tb, BAR_COMMANDS, GTK_POS_LEFT );
}

GtkWidget *ToolboxFactory::createSnapToolbox()
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *tb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(tb), FALSE);
#else
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);
#endif

    return toolboxNewCommon( tb, BAR_SNAP, GTK_POS_LEFT );
}

static GtkWidget* createCustomSlider( GtkAdjustment *adjustment, gdouble climbRate, guint digits, Inkscape::UI::Widget::UnitTracker *unit_tracker)
{
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> adj = Glib::wrap(adjustment, true);
    Inkscape::UI::Widget::SpinButton *inkSpinner = new Inkscape::UI::Widget::SpinButton(adj, climbRate, digits);
#else
    Inkscape::UI::Widget::SpinButton *inkSpinner = new Inkscape::UI::Widget::SpinButton(*Glib::wrap(adjustment, true), climbRate, digits);
#endif
    inkSpinner->addUnitTracker(unit_tracker);
    inkSpinner = Gtk::manage( inkSpinner );
    GtkWidget *widget = GTK_WIDGET( inkSpinner->gobj() );
    return widget;
}

EgeAdjustmentAction * create_adjustment_action( gchar const *name,
                                                       gchar const *label, gchar const *shortLabel, gchar const *tooltip,
                                                       Glib::ustring const &path, gdouble def,
                                                       GtkWidget *focusTarget,
                                                       GObject *dataKludge,
                                                       gboolean altx, gchar const *altx_mark,
                                                       gdouble lower, gdouble upper, gdouble step, gdouble page,
                                                       gchar const** descrLabels, gdouble const* descrValues, guint descrCount,
                                                       void (*callback)(GtkAdjustment *, GObject *),
                                                       Inkscape::UI::Widget::UnitTracker *unit_tracker,
                                                       gdouble climb/* = 0.1*/, guint digits/* = 3*/, double factor/* = 1.0*/ )
{
    static bool init = false;
    if ( !init ) {
        init = true;
        ege_adjustment_action_set_compact_tool_factory( createCustomSlider );
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkAdjustment* adj = GTK_ADJUSTMENT( gtk_adjustment_new( prefs->getDouble(path, def) * factor,
                                                             lower, upper, step, page, 0 ) );

    g_signal_connect( G_OBJECT(adj), "value-changed", G_CALLBACK(callback), dataKludge );

    EgeAdjustmentAction* act = ege_adjustment_action_new( adj, name, label, tooltip, 0, climb, digits, unit_tracker );
    if ( shortLabel ) {
        g_object_set( act, "short_label", shortLabel, NULL );
    }

    if ( (descrCount > 0) && descrLabels && descrValues ) {
        ege_adjustment_action_set_descriptions( act, descrLabels, descrValues, descrCount );
    }

    if ( focusTarget ) {
        ege_adjustment_action_set_focuswidget( act, focusTarget );
    }

    if ( altx && altx_mark ) {
        g_object_set( G_OBJECT(act), "self-id", altx_mark, NULL );
    }

    if ( dataKludge ) {
        // Rather lame, but it's the only place where we need to get the entry name
        // but we don't have an Entry
        g_object_set_data( dataKludge, prefs->getEntry(path).getEntryName().data(), adj );
    }

    // Using a cast just to make sure we pass in the right kind of function pointer
    g_object_set( G_OBJECT(act), "tool-post", static_cast<EgeWidgetFixup>(sp_set_font_size_smaller), NULL );

    return act;
}


void ToolboxFactory::setToolboxDesktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    sigc::connection *conn = static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox),
                                                                              "event_context_connection"));

    BarId id = static_cast<BarId>( GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toolbox), BAR_ID_KEY)) );

    SetupFunction setup_func = 0;
    UpdateFunction update_func = 0;

    switch (id) {
        case BAR_TOOL:
            setup_func = setup_tool_toolbox;
            update_func = update_tool_toolbox;
            break;

        case BAR_AUX:
            toolbox = gtk_bin_get_child(GTK_BIN(toolbox));
            setup_func = setup_aux_toolbox;
            update_func = update_aux_toolbox;
            break;

        case BAR_COMMANDS:
            setup_func = setup_commands_toolbox;
            update_func = update_commands_toolbox;
            break;

        case BAR_SNAP:
            setup_func = setup_snap_toolbox;
            update_func = updateSnapToolbox;
            break;
        default:
            g_warning("Unexpected toolbox id encountered.");
    }

    gpointer ptr = g_object_get_data(G_OBJECT(toolbox), "desktop");
    SPDesktop *old_desktop = static_cast<SPDesktop*>(ptr);

    if (old_desktop) {
        GList *children, *iter;

        children = gtk_container_get_children(GTK_CONTAINER(toolbox));
        for ( iter = children ; iter ; iter = iter->next ) {
            gtk_container_remove( GTK_CONTAINER(toolbox), GTK_WIDGET(iter->data) );
        }
        g_list_free(children);
    }

    g_object_set_data(G_OBJECT(toolbox), "desktop", (gpointer)desktop);

    if (desktop && setup_func && update_func) {
        gtk_widget_set_sensitive(toolbox, TRUE);
        setup_func(toolbox, desktop);
        update_func(desktop, desktop->event_context, toolbox);
        *conn = desktop->connectEventContextChanged(sigc::bind (sigc::ptr_fun(update_func), toolbox));
    } else {
        gtk_widget_set_sensitive(toolbox, FALSE);
    }

} // end of sp_toolbox_set_desktop()


static void setupToolboxCommon( GtkWidget *toolbox,
                                SPDesktop *desktop,
                                gchar const *descr,
                                gchar const* toolbarName,
                                gchar const* sizePref )
{
    Glib::RefPtr<Gtk::ActionGroup> mainActions = create_or_fetch_actions( desktop );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    GtkUIManager* mgr = gtk_ui_manager_new();
    GError* errVal = 0;

    GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;

    gtk_ui_manager_insert_action_group( mgr, mainActions->gobj(), 0 );
    gtk_ui_manager_add_ui_from_string( mgr, descr, -1, &errVal );

    GtkWidget* toolBar = gtk_ui_manager_get_widget( mgr, toolbarName );
    if ( prefs->getBool("/toolbox/icononly", true) ) {
        gtk_toolbar_set_style( GTK_TOOLBAR(toolBar), GTK_TOOLBAR_ICONS );
    }

    Inkscape::IconSize toolboxSize = ToolboxFactory::prefToSize(sizePref);
    gtk_toolbar_set_icon_size( GTK_TOOLBAR(toolBar), static_cast<GtkIconSize>(toolboxSize) );

    GtkPositionType pos = static_cast<GtkPositionType>(GPOINTER_TO_INT(g_object_get_data( G_OBJECT(toolbox), HANDLE_POS_MARK )));
    orientation = ((pos == GTK_POS_LEFT) || (pos == GTK_POS_RIGHT)) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    gtk_orientable_set_orientation (GTK_ORIENTABLE(toolBar), orientation);
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolBar), TRUE);

    g_object_set_data(G_OBJECT(toolBar), "desktop", NULL);

    GtkWidget* child = gtk_bin_get_child(GTK_BIN(toolbox));
    if ( child ) {
        gtk_container_remove( GTK_CONTAINER(toolbox), child );
    }

    gtk_container_add( GTK_CONTAINER(toolbox), toolBar );
}

#define noDUMP_DETAILS 1

void ToolboxFactory::setOrientation(GtkWidget* toolbox, GtkOrientation orientation)
{
#if DUMP_DETAILS
    g_message("Set orientation for %p to be %d", toolbox, orientation);
    GType type = G_OBJECT_TYPE(toolbox);
    g_message("        [%s]", g_type_name(type));
    g_message("             %p", g_object_get_data(G_OBJECT(toolbox), BAR_ID_KEY));
#endif

    GtkPositionType pos = (orientation == GTK_ORIENTATION_HORIZONTAL) ? GTK_POS_LEFT : GTK_POS_TOP;

    if (GTK_IS_BIN(toolbox)) {
#if DUMP_DETAILS
        g_message("            is a BIN");
#endif // DUMP_DETAILS
        GtkWidget* child = gtk_bin_get_child(GTK_BIN(toolbox));
        if (child) {
#if DUMP_DETAILS
            GType type2 = G_OBJECT_TYPE(child);
            g_message("            child    [%s]", g_type_name(type2));
#endif // DUMP_DETAILS

            if (GTK_IS_BOX(child)) {
#if DUMP_DETAILS
                g_message("                is a BOX");
#endif // DUMP_DETAILS

                GList* children = gtk_container_get_children(GTK_CONTAINER(child));
                if (children) {
                    for (GList* curr = children; curr; curr = g_list_next(curr)) {
                        GtkWidget* child2 = GTK_WIDGET(curr->data);
#if DUMP_DETAILS
                        GType type3 = G_OBJECT_TYPE(child2);
                        g_message("                child2   [%s]", g_type_name(type3));
#endif // DUMP_DETAILS

                        if (GTK_IS_CONTAINER(child2)) {
                            GList* children2 = gtk_container_get_children(GTK_CONTAINER(child2));
                            if (children2) {
                                for (GList* curr2 = children2; curr2; curr2 = g_list_next(curr2)) {
                                    GtkWidget* child3 = GTK_WIDGET(curr2->data);
#if DUMP_DETAILS
                                    GType type4 = G_OBJECT_TYPE(child3);
                                    g_message("                    child3   [%s]", g_type_name(type4));
#endif // DUMP_DETAILS
                                    if (GTK_IS_TOOLBAR(child3)) {
                                        GtkToolbar* childBar = GTK_TOOLBAR(child3);
                                        gtk_orientable_set_orientation(GTK_ORIENTABLE(childBar), orientation);
                                    }
                                }
                                g_list_free(children2);
                            }
                        }


                        if (GTK_IS_TOOLBAR(child2)) {
                            GtkToolbar* childBar = GTK_TOOLBAR(child2);
                            gtk_orientable_set_orientation(GTK_ORIENTABLE(childBar), orientation);
                        } else {
                            g_message("need to add dynamic switch");
                        }
                    }
                    g_list_free(children);
                } else {
                    // The call is being made before the toolbox proper has been setup.
                    g_object_set_data(G_OBJECT(toolbox), HANDLE_POS_MARK, GINT_TO_POINTER(pos));
                }
            } else if (GTK_IS_TOOLBAR(child)) {
                GtkToolbar* toolbar = GTK_TOOLBAR(child);
                gtk_orientable_set_orientation( GTK_ORIENTABLE(toolbar), orientation );
            }
        }
    }
}

void setup_tool_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    gchar const * descr =
        "<ui>"
        "  <toolbar name='ToolToolbar'>"

        "   <!-- Basics -->"
        "    <toolitem action='ToolSelector' />"
        "    <toolitem action='ToolNode' />"
        "    <toolitem action='ToolTweak' />"
        "    <toolitem action='ToolZoom' />"
        "    <toolitem action='ToolMeasure' />"

        "   <!-- Shapes -->"
        "    <toolitem action='ToolRect' />"
        "    <toolitem action='Tool3DBox' />"
        "    <toolitem action='ToolArc' />"
        "    <toolitem action='ToolStar' />"
        "    <toolitem action='ToolSpiral' />"

        "   <!-- Paths -->"
        "    <toolitem action='ToolPencil' />"
        "    <toolitem action='ToolPen' />"
        "    <toolitem action='ToolCalligraphic' />"

        "   <!-- Text -->"
        "    <toolitem action='ToolText' />"

        "   <!-- Paint large areas -->"
        "    <toolitem action='ToolSpray' />"
        "    <toolitem action='ToolEraser' />"

        "   <!-- Fill -->"
        "    <toolitem action='ToolPaintBucket' />"
        "    <toolitem action='ToolGradient' />"
#ifdef WITH_MESH
        "    <toolitem action='ToolMesh' />"
#endif
        "    <toolitem action='ToolDropper' />"

        "    <toolitem action='ToolConnector' />"
#ifdef WITH_LPETOOL
        "    <toolitem action='ToolLPETool' />"
#endif
        "  </toolbar>"
        "</ui>";

    setupToolboxCommon( toolbox, desktop, descr,
                        "/ui/ToolToolbar",
                        "/toolbox/tools/small");
}

void update_tool_toolbox( SPDesktop *desktop, ToolBase *eventcontext, GtkWidget * /*toolbox*/ )
{
    gchar const *const tname = ( eventcontext
                                 ? eventcontext->getPrefsPath().c_str() //g_type_name(G_OBJECT_TYPE(eventcontext))
                                 : NULL );
    Glib::RefPtr<Gtk::ActionGroup> mainActions = create_or_fetch_actions( desktop );

    for (int i = 0 ; tools[i].type_name ; i++ ) {
        Glib::RefPtr<Gtk::Action> act = mainActions->get_action( Inkscape::Verb::get(tools[i].verb)->get_id() );
        if ( act ) {
            bool setActive = tname && !strcmp(tname, tools[i].type_name);
            Glib::RefPtr<VerbAction> verbAct = Glib::RefPtr<VerbAction>::cast_dynamic(act);
            if ( verbAct ) {
                verbAct->set_active(setActive);
            }
        }
    }
}

void setup_aux_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkSizeGroup* grouper = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
    Glib::RefPtr<Gtk::ActionGroup> mainActions = create_or_fetch_actions( desktop );
    GtkUIManager* mgr = gtk_ui_manager_new();
    GError* errVal = 0;
    gtk_ui_manager_insert_action_group( mgr, mainActions->gobj(), 0 );
    gtk_ui_manager_add_ui_from_string( mgr, ui_descr, -1, &errVal );

    std::map<std::string, GtkWidget*> dataHolders;

    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        if ( aux_toolboxes[i].prep_func ) {
            // converted to GtkActions and UIManager

            GtkWidget* kludge = gtk_toolbar_new();
            g_object_set_data( G_OBJECT(kludge), "dtw", desktop->canvas);
            g_object_set_data( G_OBJECT(kludge), "desktop", desktop);
            dataHolders[aux_toolboxes[i].type_name] = kludge;
            aux_toolboxes[i].prep_func( desktop, mainActions->gobj(), G_OBJECT(kludge) );
        } else {

            GtkWidget *sub_toolbox = 0;
            if (aux_toolboxes[i].create_func == NULL) {
                sub_toolbox = sp_empty_toolbox_new(desktop);
            } else {
                sub_toolbox = aux_toolboxes[i].create_func(desktop);
            }

            gtk_size_group_add_widget( grouper, sub_toolbox );

            gtk_container_add(GTK_CONTAINER(toolbox), sub_toolbox);
            g_object_set_data(G_OBJECT(toolbox), aux_toolboxes[i].data_name, sub_toolbox);

        }
    }

    // Second pass to create toolbars *after* all GtkActions are created
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        if ( aux_toolboxes[i].prep_func ) {
            // converted to GtkActions and UIManager

            GtkWidget* kludge = dataHolders[aux_toolboxes[i].type_name];

#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget* holder = gtk_grid_new();
            gtk_grid_attach( GTK_GRID(holder), kludge, 2, 0, 1, 1);
#else
            GtkWidget* holder = gtk_table_new( 1, 3, FALSE );
            gtk_table_attach( GTK_TABLE(holder), kludge, 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0 );
#endif

            gchar* tmp = g_strdup_printf( "/ui/%s", aux_toolboxes[i].ui_name );
            GtkWidget* toolBar = gtk_ui_manager_get_widget( mgr, tmp );
            g_free( tmp );
            tmp = 0;

            if ( prefs->getBool( "/toolbox/icononly", true) ) {
                gtk_toolbar_set_style( GTK_TOOLBAR(toolBar), GTK_TOOLBAR_ICONS );
            }

            Inkscape::IconSize toolboxSize = ToolboxFactory::prefToSize("/toolbox/small");
            gtk_toolbar_set_icon_size( GTK_TOOLBAR(toolBar), static_cast<GtkIconSize>(toolboxSize) );

#if GTK_CHECK_VERSION(3,0,0)
            gtk_widget_set_hexpand(toolBar, TRUE);
            gtk_grid_attach( GTK_GRID(holder), toolBar, 0, 0, 1, 1);
#else
            gtk_table_attach( GTK_TABLE(holder), toolBar, 0, 1, 0, 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0 );
#endif

            if ( aux_toolboxes[i].swatch_verb_id != SP_VERB_INVALID ) {
                Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch( NULL, _(aux_toolboxes[i].swatch_tip) );
                swatch->setDesktop( desktop );
                swatch->setClickVerb( aux_toolboxes[i].swatch_verb_id );
                swatch->setWatchedTool( aux_toolboxes[i].swatch_tool, true );
                GtkWidget *swatch_ = GTK_WIDGET( swatch->gobj() );

#if GTK_CHECK_VERSION(3,0,0)
                gtk_widget_set_margin_left(swatch_, AUX_BETWEEN_BUTTON_GROUPS);
                gtk_widget_set_margin_right(swatch_, AUX_BETWEEN_BUTTON_GROUPS);
                gtk_widget_set_margin_top(swatch_, AUX_SPACING);
                gtk_widget_set_margin_bottom(swatch_, AUX_SPACING);
                gtk_grid_attach( GTK_GRID(holder), swatch_, 1, 0, 1, 1);
#else
                gtk_table_attach( GTK_TABLE(holder), swatch_, 1, 2, 0, 1, (GtkAttachOptions)(GTK_SHRINK | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), AUX_BETWEEN_BUTTON_GROUPS, AUX_SPACING );
#endif
            }

            gtk_widget_show_all( holder );
            sp_set_font_size_smaller( holder );

            gtk_size_group_add_widget( grouper, holder );

            gtk_container_add( GTK_CONTAINER(toolbox), holder );
            g_object_set_data( G_OBJECT(toolbox), aux_toolboxes[i].data_name, holder );
        }
    }

    g_object_unref( G_OBJECT(grouper) );
}

void update_aux_toolbox(SPDesktop * /*desktop*/, ToolBase *eventcontext, GtkWidget *toolbox)
{
    gchar const *tname = ( eventcontext
                           ? eventcontext->getPrefsPath().c_str() //g_type_name(G_OBJECT_TYPE(eventcontext))
                           : NULL );
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        GtkWidget *sub_toolbox = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), aux_toolboxes[i].data_name));
        if (tname && !strcmp(tname, aux_toolboxes[i].type_name)) {
            gtk_widget_show_all(sub_toolbox);
            g_object_set_data(G_OBJECT(toolbox), "shows", sub_toolbox);
        } else {
            gtk_widget_hide(sub_toolbox);
        }
    }
}

void setup_commands_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    gchar const * descr =
        "<ui>"
        "  <toolbar name='CommandsToolbar'>"
        "    <toolitem action='FileNew' />"
        "    <toolitem action='FileOpen' />"
        "    <toolitem action='FileSave' />"
        "    <toolitem action='FilePrint' />"
        "    <separator />"
        "    <toolitem action='FileImport' />"
        "    <toolitem action='DialogExport' />"
        "    <separator />"
        "    <toolitem action='EditUndo' />"
        "    <toolitem action='EditRedo' />"
        "    <separator />"
        "    <toolitem action='EditCopy' />"
        "    <toolitem action='EditCut' />"
        "    <toolitem action='EditPaste' />"
        "    <separator />"
        "    <toolitem action='ZoomSelection' />"
        "    <toolitem action='ZoomDrawing' />"
        "    <toolitem action='ZoomPage' />"
        "    <separator />"
        "    <toolitem action='EditDuplicate' />"
        "    <toolitem action='EditClone' />"
        "    <toolitem action='EditUnlinkClone' />"
        "    <separator />"
        "    <toolitem action='SelectionGroup' />"
        "    <toolitem action='SelectionUnGroup' />"
        "    <separator />"
        "    <toolitem action='DialogFillStroke' />"
        "    <toolitem action='DialogText' />"
        "    <toolitem action='DialogLayers' />"
        "    <toolitem action='DialogXMLEditor' />"
        "    <toolitem action='DialogAlignDistribute' />"
        "    <separator />"
        "    <toolitem action='DialogDocumentProperties' />"
        "    <toolitem action='DialogPreferences' />"
        "  </toolbar>"
        "</ui>";

    setupToolboxCommon( toolbox, desktop, descr,
                        "/ui/CommandsToolbar",
                        "/toolbox/small" );
}

void update_commands_toolbox(SPDesktop * /*desktop*/, ToolBase * /*eventcontext*/, GtkWidget * /*toolbox*/)
{
}

static void toggle_snap_callback(GtkToggleAction *act, gpointer data) //data points to the toolbox
{
    if (g_object_get_data(G_OBJECT(data), "freeze" )) {
        return;
    }

    gpointer ptr = g_object_get_data(G_OBJECT(data), "desktop");
    g_assert(ptr != NULL);

    SPDesktop *dt = reinterpret_cast<SPDesktop*>(ptr);
    SPNamedView *nv = sp_desktop_namedview(dt);
    SPDocument *doc = nv->document;

    if (dt == NULL || nv == NULL) {
        g_warning("No desktop or namedview specified (in toggle_snap_callback)!");
        return;
    }

    Inkscape::XML::Node *repr = nv->getRepr();

    if (repr == NULL) {
        g_warning("This namedview doesn't have a xml representation attached!");
        return;
    }

    bool saved = DocumentUndo::getUndoSensitive(doc);
    DocumentUndo::setUndoSensitive(doc, false);

    bool v = false;
    SPAttributeEnum attr = (SPAttributeEnum) GPOINTER_TO_INT(g_object_get_data(G_OBJECT(act), "SP_ATTR_INKSCAPE"));

    switch (attr) {
        case SP_ATTR_INKSCAPE_SNAP_GLOBAL:
            dt->toggleSnapGlobal();
            break;
        case SP_ATTR_INKSCAPE_SNAP_BBOX:
            v = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_BBOX_CATEGORY);
            sp_repr_set_boolean(repr, "inkscape:snap-bbox", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_EDGE);
            sp_repr_set_boolean(repr, "inkscape:bbox-paths", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_BBOX_CORNER:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_CORNER);
            sp_repr_set_boolean(repr, "inkscape:bbox-nodes", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_NODE:
            v = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_NODE_CATEGORY);
            sp_repr_set_boolean(repr, "inkscape:snap-nodes", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_PATH:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH);
            sp_repr_set_boolean(repr, "inkscape:object-paths", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_PATH_CLIP:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_CLIP);
            sp_repr_set_boolean(repr, "inkscape:snap-path-clip", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_PATH_MASK:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_MASK);
            sp_repr_set_boolean(repr, "inkscape:snap-path-mask", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_NODE_CUSP:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_CUSP);
            sp_repr_set_boolean(repr, "inkscape:object-nodes", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_NODE_SMOOTH:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_SMOOTH);
            sp_repr_set_boolean(repr, "inkscape:snap-smooth-nodes", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_PATH_INTERSECTION:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_INTERSECTION);
            sp_repr_set_boolean(repr, "inkscape:snap-intersection-paths", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_OTHERS:
            v = nv->snap_manager.snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_OTHERS_CATEGORY);
            sp_repr_set_boolean(repr, "inkscape:snap-others", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_ROTATION_CENTER:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_ROTATION_CENTER);
            sp_repr_set_boolean(repr, "inkscape:snap-center", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_GRID:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GRID);
            sp_repr_set_boolean(repr, "inkscape:snap-grids", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_GUIDE:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GUIDE);
            sp_repr_set_boolean(repr, "inkscape:snap-to-guides", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_PAGE_BORDER:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PAGE_BORDER);
            sp_repr_set_boolean(repr, "inkscape:snap-page", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_LINE_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_LINE_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-midpoints", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_OBJECT_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_OBJECT_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-object-midpoints", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_TEXT_BASELINE:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_TEXT_BASELINE);
            sp_repr_set_boolean(repr, "inkscape:snap-text-baseline", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_EDGE_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-bbox-edge-midpoints", !v);
            break;
        case SP_ATTR_INKSCAPE_SNAP_BBOX_MIDPOINT:
            v = nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_BBOX_MIDPOINT);
            sp_repr_set_boolean(repr, "inkscape:snap-bbox-midpoints", !v);
            break;
        default:
            g_warning("toggle_snap_callback has been called with an ID for which no action has been defined");
            break;
    }

    // The snapping preferences are stored in the document, and therefore toggling makes the document dirty
    doc->setModifiedSinceSave();

    DocumentUndo::setUndoSensitive(doc, saved);
}

void setup_snap_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    Glib::RefPtr<Gtk::ActionGroup> mainActions = create_or_fetch_actions(desktop);

    gchar const * descr =
        "<ui>"
        "  <toolbar name='SnapToolbar'>"
        "    <toolitem action='ToggleSnapGlobal' />"
        "    <separator />"
        "    <toolitem action='ToggleSnapFromBBoxCorner' />"
        "    <toolitem action='ToggleSnapToBBoxPath' />"
        "    <toolitem action='ToggleSnapToBBoxNode' />"
        "    <toolitem action='ToggleSnapToFromBBoxEdgeMidpoints' />"
        "    <toolitem action='ToggleSnapToFromBBoxCenters' />"
        "    <separator />"
        "    <toolitem action='ToggleSnapFromNode' />"
        "    <toolitem action='ToggleSnapToItemPath' />"
        "    <toolitem action='ToggleSnapToPathIntersections' />"
        "    <toolitem action='ToggleSnapToItemNode' />"
        "    <toolitem action='ToggleSnapToSmoothNodes' />"
        "    <toolitem action='ToggleSnapToFromLineMidpoints' />"
        "    <separator />"
        "    <toolitem action='ToggleSnapFromOthers' />"
        "    <toolitem action='ToggleSnapToFromObjectCenters' />"
        "    <toolitem action='ToggleSnapToFromRotationCenter' />"
        "    <toolitem action='ToggleSnapToFromTextBaseline' />"
        "    <separator />"
        "    <toolitem action='ToggleSnapToPageBorder' />"
        "    <toolitem action='ToggleSnapToGrids' />"
        "    <toolitem action='ToggleSnapToGuides' />"
        //"    <toolitem action='ToggleSnapToGridGuideIntersections' />"
        "  </toolbar>"
        "</ui>";

    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    {
        // TODO: This is a cludge. On the one hand we have verbs+actions,
        //       on the other we have all these explicit callbacks specified here.
        //       We should really unify these (should save some lines of code as well).
        //       For example, this action could be based on the verb(+action) + PrefsPusher.
        Inkscape::Verb* verb = Inkscape::Verb::get(SP_VERB_TOGGLE_SNAPPING);
        InkToggleAction* act = ink_toggle_action_new(verb->get_id(),
                                                     verb->get_name(), verb->get_tip(), INKSCAPE_ICON("snap"), secondarySize,
                                                     SP_ATTR_INKSCAPE_SNAP_GLOBAL);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapFromBBoxCorner",
                                                     _("Bounding box"), _("Snap bounding boxes"), INKSCAPE_ICON("snap"),
                                                     secondarySize, SP_ATTR_INKSCAPE_SNAP_BBOX);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToBBoxPath",
                                                     _("Bounding box edges"), _("Snap to edges of a bounding box"),
                                                     INKSCAPE_ICON("snap-bounding-box-edges"), secondarySize, SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToBBoxNode",
                                                     _("Bounding box corners"), _("Snap bounding box corners"),
                                                     INKSCAPE_ICON("snap-bounding-box-corners"), secondarySize, SP_ATTR_INKSCAPE_SNAP_BBOX_CORNER);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToFromBBoxEdgeMidpoints",
                                                     _("BBox Edge Midpoints"), _("Snap midpoints of bounding box edges"),
                                                     INKSCAPE_ICON("snap-bounding-box-midpoints"), secondarySize,
                                                     SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE_MIDPOINT);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToFromBBoxCenters",
                                                     _("BBox Centers"), _("Snapping centers of bounding boxes"),
                                                     INKSCAPE_ICON("snap-bounding-box-center"), secondarySize, SP_ATTR_INKSCAPE_SNAP_BBOX_MIDPOINT);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapFromNode",
                                                     _("Nodes"), _("Snap nodes, paths, and handles"), INKSCAPE_ICON("snap"), secondarySize, SP_ATTR_INKSCAPE_SNAP_NODE);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToItemPath",
                                                     _("Paths"), _("Snap to paths"), INKSCAPE_ICON("snap-nodes-path"), secondarySize,
                                                     SP_ATTR_INKSCAPE_SNAP_PATH);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToPathIntersections",
                                                     _("Path intersections"), _("Snap to path intersections"),
                                                     INKSCAPE_ICON("snap-nodes-intersection"), secondarySize, SP_ATTR_INKSCAPE_SNAP_PATH_INTERSECTION);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToItemNode",
                                                     _("To nodes"), _("Snap cusp nodes, incl. rectangle corners"), INKSCAPE_ICON("snap-nodes-cusp"), secondarySize,
                                                     SP_ATTR_INKSCAPE_SNAP_NODE_CUSP);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToSmoothNodes",
                                                     _("Smooth nodes"), _("Snap smooth nodes, incl. quadrant points of ellipses"), INKSCAPE_ICON("snap-nodes-smooth"),
                                                     secondarySize, SP_ATTR_INKSCAPE_SNAP_NODE_SMOOTH);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToFromLineMidpoints",
                                                     _("Line Midpoints"), _("Snap midpoints of line segments"),
                                                     INKSCAPE_ICON("snap-nodes-midpoint"), secondarySize, SP_ATTR_INKSCAPE_SNAP_LINE_MIDPOINT);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapFromOthers",
                                                     _("Others"), _("Snap other points (centers, guide origins, gradient handles, etc.)"), INKSCAPE_ICON("snap"), secondarySize, SP_ATTR_INKSCAPE_SNAP_OTHERS);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToFromObjectCenters",
                                                     _("Object Centers"), _("Snap centers of objects"),
                                                     INKSCAPE_ICON("snap-nodes-center"), secondarySize, SP_ATTR_INKSCAPE_SNAP_OBJECT_MIDPOINT);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToFromRotationCenter",
                                                     _("Rotation Centers"), _("Snap an item's rotation center"),
                                                     INKSCAPE_ICON("snap-nodes-rotation-center"), secondarySize, SP_ATTR_INKSCAPE_SNAP_ROTATION_CENTER);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToFromTextBaseline",
                                                     _("Text baseline"), _("Snap text anchors and baselines"),
                                                     INKSCAPE_ICON("snap-text-baseline"), secondarySize, SP_ATTR_INKSCAPE_SNAP_TEXT_BASELINE);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }


    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToPageBorder",
                                                     _("Page border"), _("Snap to the page border"), INKSCAPE_ICON("snap-page"),
                                                     secondarySize, SP_ATTR_INKSCAPE_SNAP_PAGE_BORDER);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToGrids",
                                                     _("Grids"), _("Snap to grids"), INKSCAPE_ICON("grid-rectangular"), secondarySize,
                                                     SP_ATTR_INKSCAPE_SNAP_GRID);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    {
        InkToggleAction* act = ink_toggle_action_new("ToggleSnapToGuides",
                                                     _("Guides"), _("Snap guides"), INKSCAPE_ICON("guides"), secondarySize,
                                                     SP_ATTR_INKSCAPE_SNAP_GUIDE);

        gtk_action_group_add_action( mainActions->gobj(), GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_snap_callback), toolbox );
    }

    setupToolboxCommon( toolbox, desktop, descr,
                        "/ui/SnapToolbar",
                        "/toolbox/secondary" );
}

Glib::ustring ToolboxFactory::getToolboxName(GtkWidget* toolbox)
{
    Glib::ustring name;
    BarId id = static_cast<BarId>( GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toolbox), BAR_ID_KEY)) );
    switch(id) {
        case BAR_TOOL:
            name = "ToolToolbar";
            break;
        case BAR_AUX:
            name = "AuxToolbar";
            break;
        case BAR_COMMANDS:
            name = "CommandsToolbar";
            break;
        case BAR_SNAP:
            name = "SnapToolbar";
            break;
    }

    return name;
}

void ToolboxFactory::updateSnapToolbox(SPDesktop *desktop, ToolBase * /*eventcontext*/, GtkWidget *toolbox)
{
    g_assert(desktop != NULL);
    g_assert(toolbox != NULL);

    SPNamedView *nv = sp_desktop_namedview(desktop);
    if (nv == NULL) {
        g_warning("Namedview cannot be retrieved (in updateSnapToolbox)!");
        return;
    }

    Glib::RefPtr<Gtk::ActionGroup> mainActions = create_or_fetch_actions(desktop);

    Glib::RefPtr<Gtk::Action> act1 = mainActions->get_action("ToggleSnapGlobal");
    Glib::RefPtr<Gtk::Action> act2 = mainActions->get_action("ToggleSnapFromBBoxCorner");
    Glib::RefPtr<Gtk::Action> act3 = mainActions->get_action("ToggleSnapToBBoxPath");
    Glib::RefPtr<Gtk::Action> act4 = mainActions->get_action("ToggleSnapToBBoxNode");
    Glib::RefPtr<Gtk::Action> act4b = mainActions->get_action("ToggleSnapToFromBBoxEdgeMidpoints");
    Glib::RefPtr<Gtk::Action> act4c = mainActions->get_action("ToggleSnapToFromBBoxCenters");
    Glib::RefPtr<Gtk::Action> act5 = mainActions->get_action("ToggleSnapFromNode");
    Glib::RefPtr<Gtk::Action> act6 = mainActions->get_action("ToggleSnapToItemPath");
    Glib::RefPtr<Gtk::Action> act6b = mainActions->get_action("ToggleSnapToPathIntersections");
    Glib::RefPtr<Gtk::Action> act7 = mainActions->get_action("ToggleSnapToItemNode");
    Glib::RefPtr<Gtk::Action> act8 = mainActions->get_action("ToggleSnapToSmoothNodes");
    Glib::RefPtr<Gtk::Action> act9 = mainActions->get_action("ToggleSnapToFromLineMidpoints");
    Glib::RefPtr<Gtk::Action> act10 = mainActions->get_action("ToggleSnapFromOthers");
    Glib::RefPtr<Gtk::Action> act10b = mainActions->get_action("ToggleSnapToFromObjectCenters");
    Glib::RefPtr<Gtk::Action> act11 = mainActions->get_action("ToggleSnapToFromRotationCenter");
    Glib::RefPtr<Gtk::Action> act11b = mainActions->get_action("ToggleSnapToFromTextBaseline");
    Glib::RefPtr<Gtk::Action> act12 = mainActions->get_action("ToggleSnapToPageBorder");
    Glib::RefPtr<Gtk::Action> act14 = mainActions->get_action("ToggleSnapToGrids");
    Glib::RefPtr<Gtk::Action> act15 = mainActions->get_action("ToggleSnapToGuides");


    if (!act1) {
        return; // The snap actions haven't been defined yet (might be the case during startup)
    }

    // The ..._set_active calls below will toggle the buttons, but this shouldn't lead to
    // changes in our document because we're only updating the UI;
    // Setting the "freeze" parameter to true will block the code in toggle_snap_callback()
    g_object_set_data(G_OBJECT(toolbox), "freeze", GINT_TO_POINTER(TRUE));

    bool const c1 = nv->snap_manager.snapprefs.getSnapEnabledGlobally();
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act1->gobj()), c1);

    bool const c2 = nv->snap_manager.snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CATEGORY);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act2->gobj()), c2);
    gtk_action_set_sensitive(GTK_ACTION(act2->gobj()), c1);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act3->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_EDGE));
    gtk_action_set_sensitive(GTK_ACTION(act3->gobj()), c1 && c2);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act4->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_CORNER));
    gtk_action_set_sensitive(GTK_ACTION(act4->gobj()), c1 && c2);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act4b->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_EDGE_MIDPOINT));
    gtk_action_set_sensitive(GTK_ACTION(act4b->gobj()), c1 && c2);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act4c->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(SNAPTARGET_BBOX_MIDPOINT));
    gtk_action_set_sensitive(GTK_ACTION(act4c->gobj()), c1 && c2);

    bool const c3 = nv->snap_manager.snapprefs.isTargetSnappable(SNAPTARGET_NODE_CATEGORY);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act5->gobj()), c3);
    gtk_action_set_sensitive(GTK_ACTION(act5->gobj()), c1);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act6->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH));
    gtk_action_set_sensitive(GTK_ACTION(act6->gobj()), c1 && c3);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act6b->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_INTERSECTION));
    gtk_action_set_sensitive(GTK_ACTION(act6b->gobj()), c1 && c3);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act7->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_CUSP));
    gtk_action_set_sensitive(GTK_ACTION(act7->gobj()), c1 && c3);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act8->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_NODE_SMOOTH));
    gtk_action_set_sensitive(GTK_ACTION(act8->gobj()), c1 && c3);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act9->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_LINE_MIDPOINT));
    gtk_action_set_sensitive(GTK_ACTION(act9->gobj()), c1 && c3);

    bool const c5 = nv->snap_manager.snapprefs.isTargetSnappable(SNAPTARGET_OTHERS_CATEGORY);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act10->gobj()), c5);
    gtk_action_set_sensitive(GTK_ACTION(act10->gobj()), c1);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act10b->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_OBJECT_MIDPOINT));
    gtk_action_set_sensitive(GTK_ACTION(act10b->gobj()), c1 && c5);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act11->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_ROTATION_CENTER));
    gtk_action_set_sensitive(GTK_ACTION(act11->gobj()), c1 && c5);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act11b->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_TEXT_BASELINE));
    gtk_action_set_sensitive(GTK_ACTION(act11b->gobj()), c1 && c5);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act12->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PAGE_BORDER));
    gtk_action_set_sensitive(GTK_ACTION(act12->gobj()), c1);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act14->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GRID));
    gtk_action_set_sensitive(GTK_ACTION(act14->gobj()), c1);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act15->gobj()), nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_GUIDE));
    gtk_action_set_sensitive(GTK_ACTION(act15->gobj()), c1);


    g_object_set_data(G_OBJECT(toolbox), "freeze", GINT_TO_POINTER(FALSE)); // unfreeze (see above)
}

void ToolboxFactory::showAuxToolbox(GtkWidget *toolbox_toplevel)
{
    gtk_widget_show(toolbox_toplevel);
    GtkWidget *toolbox = gtk_bin_get_child(GTK_BIN(toolbox_toplevel));

    GtkWidget *shown_toolbox = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), "shows"));
    if (!shown_toolbox) {
        return;
    }
    gtk_widget_show(toolbox);

    gtk_widget_show_all(shown_toolbox);
}

static GtkWidget *sp_empty_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_toolbar_new();
    g_object_set_data(G_OBJECT(tbl), "dtw", desktop->canvas);
    g_object_set_data(G_OBJECT(tbl), "desktop", desktop);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}

#define MODE_LABEL_WIDTH 70


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

/** \file
 * Controls bars for some of Inkscape's tools
 * (for some tools, they are in their own files)
 */

/*
*
* Authors:
*   MenTaLguY <mental@rydia.net>
*   Lauris Kaplinski <lauris@kaplinski.com>
*   bulia byak <buliabyak@users.sf.net>
*   Frank Felfe <innerspace@iname.com>
*   John Cliff <simarilius@yahoo.com>
*   David Turner <novalis@gnu.org>
*   Josh Andler <scislac@scislac.com>
*   Jon A. Cruz <jon@joncruz.org>
*
* Copyright (C) 2004 David Turner
* Copyright (C) 2003 MenTaLguY
* Copyright (C) 1999-2006 authors
* Copyright (C) 2001-2002 Ximian, Inc.
*
* Released under GNU GPL, read the file 'COPYING' for more information
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include <gtkmm.h>
#include <gtk/gtk.h>
#include <iostream>
#include <sstream>

#include "widgets/button.h"
#include "widgets/widget-sizes.h"
#include "widgets/spw-utilities.h"
#include "widgets/spinbutton-events.h"
#include "dialogs/text-edit.h"
#include "dialogs/dialog-events.h"

#include "ui/widget/style-swatch.h"

#include "prefs-utils.h"
#include "verbs.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "live_effects/effect.h"

#include "inkscape.h"
#include "conn-avoid-ref.h"


#include "select-toolbar.h"
#include "gradient-toolbar.h"

#include "connector-context.h"
#include "node-context.h"
#include "draw-context.h"
#include "shape-editor.h"
#include "tweak-context.h"
#include "sp-rect.h"
#include "box3d.h"
#include "box3d-context.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-ellipse.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "style.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "document-private.h"
#include "desktop-style.h"
#include "../libnrtype/font-lister.h"
#include "../libnrtype/font-instance.h"
#include "../connection-pool.h"
#include "../prefs-utils.h"
#include "../inkscape-stock.h"
#include "icon.h"
#include "graphlayout/graphlayout.h"
#include "interface.h"
#include "shortcuts.h"

#include "mod360.h"

#include "toolbox.h"

#include "flood-context.h"

#include "ink-action.h"
#include "ege-adjustment-action.h"
#include "ege-output-action.h"
#include "ege-select-one-action.h"
#include "helper/unit-tracker.h"

#include "svg/css-ostringstream.h"

#include "widgets/calligraphic-profile-rename.h"

using Inkscape::UnitTracker;

typedef void (*SetupFunction)(GtkWidget *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static void       sp_node_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_tweak_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_zoom_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_star_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_arc_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_rect_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       box3d_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_spiral_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_pencil_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_pen_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_calligraphy_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_dropper_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static GtkWidget *sp_empty_toolbox_new(SPDesktop *desktop);
static void       sp_connector_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_paintbucket_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);
static void       sp_eraser_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder);

namespace { GtkWidget *sp_text_toolbox_new (SPDesktop *desktop); }


Inkscape::IconSize prefToSize( gchar const *path, gchar const *attr, int base ) {
    static Inkscape::IconSize sizeChoices[] = {
        Inkscape::ICON_SIZE_LARGE_TOOLBAR,
        Inkscape::ICON_SIZE_SMALL_TOOLBAR,
        Inkscape::ICON_SIZE_MENU
    };
    int index = prefs_get_int_attribute_limited( path, attr, base, 0, G_N_ELEMENTS(sizeChoices) );
    return sizeChoices[index];
}

static struct {
    gchar const *type_name;
    gchar const *data_name;
    sp_verb_t verb;
    sp_verb_t doubleclick_verb;
} const tools[] = {
    { "SPSelectContext",   "select_tool",    SP_VERB_CONTEXT_SELECT,  SP_VERB_CONTEXT_SELECT_PREFS},
    { "SPNodeContext",     "node_tool",      SP_VERB_CONTEXT_NODE, SP_VERB_CONTEXT_NODE_PREFS },
    { "SPTweakContext",    "tweak_tool",     SP_VERB_CONTEXT_TWEAK, SP_VERB_CONTEXT_TWEAK_PREFS },
    { "SPZoomContext",     "zoom_tool",      SP_VERB_CONTEXT_ZOOM, SP_VERB_CONTEXT_ZOOM_PREFS },
    { "SPRectContext",     "rect_tool",      SP_VERB_CONTEXT_RECT, SP_VERB_CONTEXT_RECT_PREFS },
    { "Box3DContext",      "3dbox_tool",     SP_VERB_CONTEXT_3DBOX, SP_VERB_CONTEXT_3DBOX_PREFS },
    { "SPArcContext",      "arc_tool",       SP_VERB_CONTEXT_ARC, SP_VERB_CONTEXT_ARC_PREFS },
    { "SPStarContext",     "star_tool",      SP_VERB_CONTEXT_STAR, SP_VERB_CONTEXT_STAR_PREFS },
    { "SPSpiralContext",   "spiral_tool",    SP_VERB_CONTEXT_SPIRAL, SP_VERB_CONTEXT_SPIRAL_PREFS },
    { "SPPencilContext",   "pencil_tool",    SP_VERB_CONTEXT_PENCIL, SP_VERB_CONTEXT_PENCIL_PREFS },
    { "SPPenContext",      "pen_tool",       SP_VERB_CONTEXT_PEN, SP_VERB_CONTEXT_PEN_PREFS },
    { "SPDynaDrawContext", "dyna_draw_tool", SP_VERB_CONTEXT_CALLIGRAPHIC, SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS },
    { "SPEraserContext",   "eraser_tool",    SP_VERB_CONTEXT_ERASER, SP_VERB_CONTEXT_ERASER_PREFS },
    { "SPFloodContext",    "paintbucket_tool",     SP_VERB_CONTEXT_PAINTBUCKET, SP_VERB_CONTEXT_PAINTBUCKET_PREFS },
    { "SPTextContext",     "text_tool",      SP_VERB_CONTEXT_TEXT, SP_VERB_CONTEXT_TEXT_PREFS },
    { "SPConnectorContext","connector_tool", SP_VERB_CONTEXT_CONNECTOR, SP_VERB_CONTEXT_CONNECTOR_PREFS },
    { "SPGradientContext", "gradient_tool",  SP_VERB_CONTEXT_GRADIENT, SP_VERB_CONTEXT_GRADIENT_PREFS },
    { "SPDropperContext",  "dropper_tool",   SP_VERB_CONTEXT_DROPPER, SP_VERB_CONTEXT_DROPPER_PREFS },
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
    { "SPSelectContext", "select_toolbox", 0, sp_select_toolbox_prep,            "SelectToolbar",
      SP_VERB_INVALID, 0, 0},
    { "SPNodeContext",   "node_toolbox",   0, sp_node_toolbox_prep,              "NodeToolbar",
      SP_VERB_INVALID, 0, 0},
    { "SPTweakContext",   "tweak_toolbox",   0, sp_tweak_toolbox_prep,              "TweakToolbar",
      SP_VERB_CONTEXT_TWEAK_PREFS, "tools.tweak", N_("Color/opacity used for color tweaking")},
    { "SPZoomContext",   "zoom_toolbox",   0, sp_zoom_toolbox_prep,              "ZoomToolbar",
      SP_VERB_INVALID, 0, 0},
    { "SPStarContext",   "star_toolbox",   0, sp_star_toolbox_prep,              "StarToolbar",
      SP_VERB_CONTEXT_STAR_PREFS,   "tools.shapes.star",     N_("Style of new stars")},
    { "SPRectContext",   "rect_toolbox",   0, sp_rect_toolbox_prep,              "RectToolbar",
      SP_VERB_CONTEXT_RECT_PREFS,   "tools.shapes.rect",     N_("Style of new rectangles")},
    { "Box3DContext",  "3dbox_toolbox",  0, box3d_toolbox_prep,             "3DBoxToolbar",
      SP_VERB_CONTEXT_3DBOX_PREFS,  "tools.shapes.3dbox",    N_("Style of new 3D boxes")},
    { "SPArcContext",    "arc_toolbox",    0, sp_arc_toolbox_prep,               "ArcToolbar",
      SP_VERB_CONTEXT_ARC_PREFS,    "tools.shapes.arc",      N_("Style of new ellipses")},
    { "SPSpiralContext", "spiral_toolbox", 0, sp_spiral_toolbox_prep,            "SpiralToolbar",
      SP_VERB_CONTEXT_SPIRAL_PREFS, "tools.shapes.spiral",   N_("Style of new spirals")},
    { "SPPencilContext", "pencil_toolbox", 0, sp_pencil_toolbox_prep,            "PencilToolbar",
      SP_VERB_CONTEXT_PENCIL_PREFS, "tools.freehand.pencil", N_("Style of new paths created by Pencil")},
    { "SPPenContext", "pen_toolbox", 0, sp_pen_toolbox_prep,                     "PenToolbar",
      SP_VERB_CONTEXT_PEN_PREFS,    "tools.freehand.pen",    N_("Style of new paths created by Pen")},
    { "SPDynaDrawContext", "calligraphy_toolbox", 0, sp_calligraphy_toolbox_prep,"CalligraphyToolbar",
      SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS, "tools.calligraphic", N_("Style of new calligraphic strokes")},
    { "SPEraserContext", "eraser_toolbox", 0, sp_eraser_toolbox_prep,"EraserToolbar",
      SP_VERB_CONTEXT_ERASER_PREFS, "tools.eraser", _("TBD")},
    { "SPTextContext",   "text_toolbox",   sp_text_toolbox_new, 0,               0,
      SP_VERB_INVALID, 0, 0},
    { "SPDropperContext", "dropper_toolbox", 0, sp_dropper_toolbox_prep,         "DropperToolbar",
      SP_VERB_INVALID, 0, 0},
    { "SPGradientContext", "gradient_toolbox", sp_gradient_toolbox_new, 0,       0,
      SP_VERB_INVALID, 0, 0},
    { "SPConnectorContext", "connector_toolbox", 0, sp_connector_toolbox_prep,   "ConnectorToolbar",
      SP_VERB_INVALID, 0, 0},
    { "SPFloodContext",  "paintbucket_toolbox",  0, sp_paintbucket_toolbox_prep, "PaintbucketToolbar",
      SP_VERB_CONTEXT_PAINTBUCKET_PREFS, "tools.paintbucket", N_("Style of Paint Bucket fill objects")},
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
        "    <toolitem action='transform_affect_label' />"
        "    <toolitem action='transform_stroke' />"
        "    <toolitem action='transform_corners' />"
        "    <toolitem action='transform_gradient' />"
        "    <toolitem action='transform_pattern' />"
        "  </toolbar>"

        "  <toolbar name='NodeToolbar'>"
        "    <toolitem action='NodeInsertAction' />"
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
        "    <toolitem action='EditNextLPEParameterAction' />"
        "    <separator />"
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
        "    <toolitem action='FreehandModeActionPenTemp' />"
        "    <toolitem action='FreehandModeActionPen' />"
        "  </toolbar>"

        "  <toolbar name='PencilToolbar'>"
        "    <toolitem action='FreehandModeActionPencilTemp' />"
        "    <toolitem action='FreehandModeActionPencil' />"
        "    <separator />"
        "    <toolitem action='PencilToleranceAction' />"    
        "    <separator />"
        "    <toolitem action='PencilResetAction' />"
        "  </toolbar>"

        "  <toolbar name='CalligraphyToolbar'>"
        "    <separator />"
        "    <toolitem action='SetProfileAction'/>"
        "    <toolitem action='SaveDeleteProfileAction'/>"
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
        "    <toolitem action='EraserWidthAction' />"
        "    <separator />"
        "    <toolitem action='EraserModeAction' />"
        "  </toolbar>"

        "  <toolbar name='DropperToolbar'>"
        "    <toolitem action='DropperOpacityAction' />"
        "    <toolitem action='DropperPickAlphaAction' />"
        "    <toolitem action='DropperSetAlphaAction' />"
        "  </toolbar>"

        "  <toolbar name='ConnectorToolbar'>"
        "    <toolitem action='ConnectorAvoidAction' />"
        "    <toolitem action='ConnectorIgnoreAction' />"
        "    <toolitem action='ConnectorSpacingAction' />"
        "    <toolitem action='ConnectorGraphAction' />"
        "    <toolitem action='ConnectorLengthAction' />"
        "    <toolitem action='ConnectorDirectedAction' />"
        "    <toolitem action='ConnectorOverlapAction' />"
        "  </toolbar>"

        "</ui>"
;

static GtkActionGroup* create_or_fetch_actions( SPDesktop* desktop );

static void toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func, sigc::connection*);

static void setup_tool_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static void setup_aux_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static void setup_commands_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_commands_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

/* Global text entry widgets necessary for update */
/* GtkWidget *dropper_rgb_entry,
          *dropper_opacity_entry ; */
// should be made a private member once this is converted to class

static void delete_connection(GObject */*obj*/, sigc::connection *connection) {
    connection->disconnect();
    delete connection;
}

static void purge_repr_listener( GObject* obj, GObject* tbl )
{
    (void)obj;
    Inkscape::XML::Node* oldrepr = reinterpret_cast<Inkscape::XML::Node *>( g_object_get_data( tbl, "repr" ) );
    if (oldrepr) { // remove old listener
        sp_repr_remove_listener_by_data(oldrepr, tbl);
        Inkscape::GC::release(oldrepr);
        oldrepr = 0;
        g_object_set_data( tbl, "repr", NULL );
    }
}

GtkWidget *
sp_toolbox_button_new_from_verb_with_doubleclick(GtkWidget *t, Inkscape::IconSize size, SPButtonType type,
                                                 Inkscape::Verb *verb, Inkscape::Verb *doubleclick_verb,
                                                 Inkscape::UI::View::View *view, GtkTooltips *tt)
{
    SPAction *action = verb->get_action(view);
    if (!action) return NULL;

    SPAction *doubleclick_action;
    if (doubleclick_verb)
        doubleclick_action = doubleclick_verb->get_action(view);
    else
        doubleclick_action = NULL;

    /* fixme: Handle sensitive/unsensitive */
    /* fixme: Implement sp_button_new_from_action */
    GtkWidget *b = sp_button_new(size, type, action, doubleclick_action, tt);
    gtk_widget_show(b);


    unsigned int shortcut = sp_shortcut_get_primary(verb);
    if (shortcut) {
        gchar key[256];
        sp_ui_shortcut_string(shortcut, key);
        gchar *tip = g_strdup_printf ("%s (%s)", action->tip, key);
        gtk_toolbar_append_widget( GTK_TOOLBAR(t), b, tip, 0 );
        g_free(tip);
    } else {
        gtk_toolbar_append_widget( GTK_TOOLBAR(t), b, action->tip, 0 );
    }

    return b;
}


static void trigger_sp_action( GtkAction* /*act*/, gpointer user_data )
{
    SPAction* targetAction = SP_ACTION(user_data);
    if ( targetAction ) {
        sp_action_perform( targetAction, NULL );
    }
}

static void sp_action_action_set_sensitive (SPAction */*action*/, unsigned int sensitive, void *data)
{
    if ( data ) {
        GtkAction* act = GTK_ACTION(data);
        gtk_action_set_sensitive( act, sensitive );
    }
}

static SPActionEventVector action_event_vector = {
    {NULL},
    NULL,
    NULL,
    sp_action_action_set_sensitive,
    NULL,
    NULL
};

static GtkAction* create_action_for_verb( Inkscape::Verb* verb, Inkscape::UI::View::View* view, Inkscape::IconSize size )
{
    GtkAction* act = 0;

    SPAction* targetAction = verb->get_action(view);
    InkAction* inky = ink_action_new( verb->get_id(), _(verb->get_name()), verb->get_tip(), verb->get_image(), size  );
    act = GTK_ACTION(inky);
    gtk_action_set_sensitive( act, targetAction->sensitive );

    g_signal_connect( G_OBJECT(inky), "activate", GTK_SIGNAL_FUNC(trigger_sp_action), targetAction );

    SPAction*rebound = dynamic_cast<SPAction *>( nr_object_ref( dynamic_cast<NRObject *>(targetAction) ) );
    nr_active_object_add_listener( (NRActiveObject *)rebound, (NRObjectEventVector *)&action_event_vector, sizeof(SPActionEventVector), inky );

    return act;
}

GtkActionGroup* create_or_fetch_actions( SPDesktop* desktop )
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
        SP_VERB_EDIT_CLONE,
        SP_VERB_EDIT_COPY,
        SP_VERB_EDIT_CUT,
        SP_VERB_EDIT_DUPLICATE,
        SP_VERB_EDIT_PASTE,
        SP_VERB_EDIT_REDO,
        SP_VERB_EDIT_UNDO,
        SP_VERB_EDIT_UNLINK_CLONE,
        SP_VERB_FILE_EXPORT,
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
        SP_VERB_ZOOM_SELECTION,
    };

    Inkscape::IconSize toolboxSize = prefToSize("toolbox", "small");

    static std::map<SPDesktop*, GtkActionGroup*> groups;
    GtkActionGroup* mainActions = 0;
    if ( groups.find(desktop) != groups.end() ) {
        mainActions = groups[desktop];
    }

    if ( !mainActions ) {
        mainActions = gtk_action_group_new("main");
        groups[desktop] = mainActions;
    }

    for ( guint i = 0; i < G_N_ELEMENTS(verbsToUse); i++ ) {
        Inkscape::Verb* verb = Inkscape::Verb::get(verbsToUse[i]);
        if ( verb ) {
            if ( !gtk_action_group_get_action( mainActions, verb->get_id() ) ) {
                GtkAction* act = create_action_for_verb( verb, view, toolboxSize );
                gtk_action_group_add_action( mainActions, act );
            }
        }
    }

    return mainActions;
}


void handlebox_detached(GtkHandleBox* /*handlebox*/, GtkWidget* widget, gpointer /*userData*/)
{
    gtk_widget_set_size_request( widget,
                                 widget->allocation.width,
                                 widget->allocation.height );
}

void handlebox_attached(GtkHandleBox* /*handlebox*/, GtkWidget* widget, gpointer /*userData*/)
{
    gtk_widget_set_size_request( widget, -1, -1 );
}



GtkWidget *
sp_tool_toolbox_new()
{
    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget* tb = gtk_toolbar_new();
    gtk_toolbar_set_orientation(GTK_TOOLBAR(tb), GTK_ORIENTATION_VERTICAL);
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(tb), TRUE);

    g_object_set_data(G_OBJECT(tb), "desktop", NULL);
    g_object_set_data(G_OBJECT(tb), "tooltips", tt);

    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(hb), GTK_POS_TOP);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(hb), GTK_SHADOW_OUT);
    gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);

     gtk_container_add(GTK_CONTAINER(hb), tb);
     gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
     g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    g_signal_connect(G_OBJECT(hb), "child_detached", G_CALLBACK(handlebox_detached), static_cast<gpointer>(0));
    g_signal_connect(G_OBJECT(hb), "child_attached", G_CALLBACK(handlebox_attached), static_cast<gpointer>(0));

    return hb;
}

GtkWidget *
sp_aux_toolbox_new()
{
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);

    gtk_box_set_spacing(GTK_BOX(tb), AUX_SPACING);

    g_object_set_data(G_OBJECT(tb), "desktop", NULL);

    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(hb), GTK_SHADOW_OUT);
    gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    g_signal_connect(G_OBJECT(hb), "child_detached", G_CALLBACK(handlebox_detached), static_cast<gpointer>(0));
    g_signal_connect(G_OBJECT(hb), "child_attached", G_CALLBACK(handlebox_attached), static_cast<gpointer>(0));

    return hb;
}

//####################################
//# Commands Bar
//####################################

GtkWidget *
sp_commands_toolbox_new()
{
    GtkWidget *tb = gtk_toolbar_new();

    g_object_set_data(G_OBJECT(tb), "desktop", NULL);
    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(hb), GTK_SHADOW_OUT);
    gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    g_signal_connect(G_OBJECT(hb), "child_detached", G_CALLBACK(handlebox_detached), static_cast<gpointer>(0));
    g_signal_connect(G_OBJECT(hb), "child_attached", G_CALLBACK(handlebox_attached), static_cast<gpointer>(0));

    return hb;
}


static EgeAdjustmentAction * create_adjustment_action( gchar const *name,
                                                       gchar const *label, gchar const *shortLabel, gchar const *tooltip,
                                                       gchar const *path, gchar const *data, gdouble def,
                                                       GtkWidget *focusTarget,
                                                       GtkWidget *us,
                                                       GObject *dataKludge,
                                                       gboolean altx, gchar const *altx_mark,
                                                       gdouble lower, gdouble upper, gdouble step, gdouble page,
                                                       gchar const** descrLabels, gdouble const* descrValues, guint descrCount,
                                                       void (*callback)(GtkAdjustment *, GObject *),
                                                       gdouble climb = 0.1, guint digits = 3, double factor = 1.0 )
{
    GtkAdjustment* adj = GTK_ADJUSTMENT( gtk_adjustment_new( prefs_get_double_attribute(path, data, def) * factor,
                                                             lower, upper, step, page, page ) );
    if (us) {
        sp_unit_selector_add_adjustment( SP_UNIT_SELECTOR(us), adj );
    }

    gtk_signal_connect( GTK_OBJECT(adj), "value-changed", GTK_SIGNAL_FUNC(callback), dataKludge );

    EgeAdjustmentAction* act = ege_adjustment_action_new( adj, name, label, tooltip, 0, climb, digits );
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
        g_object_set_data( dataKludge, data, adj );
    }

    // Using a cast just to make sure we pass in the right kind of function pointer
    g_object_set( G_OBJECT(act), "tool-post", static_cast<EgeWidgetFixup>(sp_set_font_size_smaller), NULL );

    return act;
}


//####################################
//# node editing callbacks
//####################################

/**
 * FIXME: Returns current shape_editor in context. // later eliminate this function at all!
 */
static ShapeEditor *get_current_shape_editor()
{
    if (!SP_ACTIVE_DESKTOP) {
        return NULL;
    }

    SPEventContext *event_context = (SP_ACTIVE_DESKTOP)->event_context;

    if (!SP_IS_NODE_CONTEXT(event_context)) {
        return NULL;
    }

    return SP_NODE_CONTEXT(event_context)->shape_editor;
}


void
sp_node_path_edit_add(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->add_node();
}

void
sp_node_path_edit_delete(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->delete_nodes_preserving_shape();
}

void
sp_node_path_edit_delete_segment(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->delete_segment();
}

void
sp_node_path_edit_break(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->break_at_nodes();
}

void
sp_node_path_edit_join(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->join_nodes();
}

void
sp_node_path_edit_join_segment(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->join_segments();
}

void
sp_node_path_edit_toline(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->set_type_of_segments(NR_LINETO);
}

void
sp_node_path_edit_tocurve(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->set_type_of_segments(NR_CURVETO);
}

void
sp_node_path_edit_cusp(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->set_node_type(Inkscape::NodePath::NODE_CUSP);
}

void
sp_node_path_edit_smooth(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->set_node_type(Inkscape::NodePath::NODE_SMOOTH);
}

void
sp_node_path_edit_symmetrical(void)
{
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->set_node_type(Inkscape::NodePath::NODE_SYMM);
}

static void toggle_show_handles (GtkToggleAction *act, gpointer /*data*/) {
    bool show = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute ("tools.nodes", "show_handles",  show ? 1 : 0);
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->show_handles(show);
}

static void toggle_show_helperpath (GtkToggleAction *act, gpointer /*data*/) {
    bool show = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute ("tools.nodes", "show_helperpath",  show ? 1 : 0);
    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor) shape_editor->show_helperpath(show);
}

void sp_node_path_edit_nextLPEparam (GtkAction */*act*/, gpointer data) {
    sp_selection_next_patheffect_param( reinterpret_cast<SPDesktop*>(data) );
}

void sp_node_path_edit_clippath (GtkAction */*act*/, gpointer data) {
    sp_selection_edit_clip_or_mask( reinterpret_cast<SPDesktop*>(data), true);
}

void sp_node_path_edit_maskpath (GtkAction */*act*/, gpointer data) {
    sp_selection_edit_clip_or_mask( reinterpret_cast<SPDesktop*>(data), false);
}

/* is called when the node selection is modified */
static void
sp_node_toolbox_coord_changed(gpointer /*shape_editor*/, GObject *tbl)
{
    GtkAction* xact = GTK_ACTION( g_object_get_data( tbl, "nodes_x_action" ) );
    GtkAction* yact = GTK_ACTION( g_object_get_data( tbl, "nodes_y_action" ) );
    GtkAdjustment *xadj = ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION(xact));
    GtkAdjustment *yadj = ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION(yact));

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>( g_object_get_data( tbl, "tracker" ) );
    SPUnit const *unit = tracker->getActiveUnit();

    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor && shape_editor->has_nodepath()) {
        Inkscape::NodePath::Path *nodepath = shape_editor->get_nodepath();
        int n_selected = 0;
        if (nodepath) {
            n_selected = nodepath->numSelected();
        }

        if (n_selected == 0) {
            gtk_action_set_sensitive(xact, FALSE);
            gtk_action_set_sensitive(yact, FALSE);
        } else {
            gtk_action_set_sensitive(xact, TRUE);
            gtk_action_set_sensitive(yact, TRUE);
            NR::Coord oldx = sp_units_get_pixels(gtk_adjustment_get_value(xadj), *unit);
            NR::Coord oldy = sp_units_get_pixels(gtk_adjustment_get_value(xadj), *unit);

            if (n_selected == 1) {
                NR::Point sel_node = nodepath->singleSelectedCoords();
                if (oldx != sel_node[NR::X] || oldy != sel_node[NR::Y]) {
                    gtk_adjustment_set_value(xadj, sp_pixels_get_units(sel_node[NR::X], *unit));
                    gtk_adjustment_set_value(yadj, sp_pixels_get_units(sel_node[NR::Y], *unit));
                }
            } else {
                NR::Maybe<NR::Coord> x = sp_node_selected_common_coord(nodepath, NR::X);
                NR::Maybe<NR::Coord> y = sp_node_selected_common_coord(nodepath, NR::Y);
                if ((x && ((*x) != oldx)) || (y && ((*y) != oldy))) {
                    /* Note: Currently x and y will always have a value, even if the coordinates of the
                       selected nodes don't coincide (in this case we use the coordinates of the center
                       of the bounding box). So the entries are never set to zero. */
                    // FIXME: Maybe we should clear the entry if several nodes are selected
                    //        instead of providing a kind of average value
                    gtk_adjustment_set_value(xadj, sp_pixels_get_units(x ? (*x) : 0.0, *unit));
                    gtk_adjustment_set_value(yadj, sp_pixels_get_units(y ? (*y) : 0.0, *unit));
                }
            }
        }
    } else {
        // no shape-editor or nodepath yet (when we just switched to the tool); coord entries must be inactive
        gtk_action_set_sensitive(xact, FALSE);
        gtk_action_set_sensitive(yact, FALSE);
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void
sp_node_path_value_changed(GtkAdjustment *adj, GObject *tbl, gchar const *value_name)
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    SPUnit const *unit = tracker->getActiveUnit();

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.nodes", value_name, sp_units_get_pixels(adj->value, *unit));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    ShapeEditor *shape_editor = get_current_shape_editor();
    if (shape_editor && shape_editor->has_nodepath()) {
        double val = sp_units_get_pixels(gtk_adjustment_get_value(adj), *unit);
        if (!strcmp(value_name, "x")) {
            sp_node_selected_move_absolute(shape_editor->get_nodepath(), val, NR::X);
        }
        if (!strcmp(value_name, "y")) {
            sp_node_selected_move_absolute(shape_editor->get_nodepath(), val, NR::Y);
        }
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void
sp_node_path_x_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_node_path_value_changed(adj, tbl, "x");
}

static void
sp_node_path_y_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_node_path_value_changed(adj, tbl, "y");
}

void
sp_node_toolbox_sel_changed (Inkscape::Selection *selection, GObject *tbl)
{
    {
    GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "nodes_lpeedit" ) );
    SPItem *item = selection->singleItem();
    if (item && SP_IS_LPE_ITEM(item)) {
       if (sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {
           gtk_action_set_sensitive(w, TRUE);
       } else {
           gtk_action_set_sensitive(w, FALSE);
       }
    } else {
       gtk_action_set_sensitive(w, FALSE);
    }
    }

    {
    GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "nodes_clippathedit" ) );
    SPItem *item = selection->singleItem();
    if (item && item->clip_ref && item->clip_ref->getObject()) {
       gtk_action_set_sensitive(w, TRUE);
    } else {
       gtk_action_set_sensitive(w, FALSE);
    }
    }

    {
    GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "nodes_maskedit" ) );
    SPItem *item = selection->singleItem();
    if (item && item->mask_ref && item->mask_ref->getObject()) {
       gtk_action_set_sensitive(w, TRUE);
    } else {
       gtk_action_set_sensitive(w, FALSE);
    }
    }
}

void
sp_node_toolbox_sel_modified (Inkscape::Selection *selection, guint /*flags*/, GObject *tbl)
{
    sp_node_toolbox_sel_changed (selection, tbl);
}



//################################
//##    Node Editing Toolbox    ##
//################################

static void sp_node_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    UnitTracker* tracker = new UnitTracker( SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE );
    tracker->setActiveUnit( sp_desktop_namedview(desktop)->doc_units );
    g_object_set_data( holder, "tracker", tracker );

    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

    {
        InkAction* inky = ink_action_new( "NodeInsertAction",
                                          _("Insert node"),
                                          _("Insert new nodes into selected segments"),
                                          "node_insert",
                                          secondarySize );
        g_object_set( inky, "short_label", _("Insert"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_add), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeDeleteAction",
                                          _("Delete node"),
                                          _("Delete selected nodes"),
                                          "node_delete",
                                          secondarySize );
        g_object_set( inky, "short_label", _("Delete"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_delete), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeJoinAction",
                                          _("Join endnodes"),
                                          _("Join selected endnodes"),
                                          "node_join",
                                          secondarySize );
        g_object_set( inky, "short_label", _("Join"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_join), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeBreakAction",
                                          _("Break nodes"),
                                          _("Break path at selected nodes"),
                                          "node_break",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_break), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }


    {
        InkAction* inky = ink_action_new( "NodeJoinSegmentAction",
                                          _("Join with segment"),
                                          _("Join selected endnodes with a new segment"),
                                          "node_join_segment",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_join_segment), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeDeleteSegmentAction",
                                          _("Delete segment"),
                                          _("Delete segment between two non-endpoint nodes"),
                                          "node_delete_segment",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_delete_segment), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeCuspAction",
                                          _("Node Cusp"),
                                          _("Make selected nodes corner"),
                                          "node_cusp",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_cusp), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeSmoothAction",
                                          _("Node Smooth"),
                                          _("Make selected nodes smooth"),
                                          "node_smooth",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_smooth), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeSymmetricAction",
                                          _("Node Symmetric"),
                                          _("Make selected nodes symmetric"),
                                          "node_symmetric",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_symmetrical), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeLineAction",
                                          _("Node Line"),
                                          _("Make selected segments lines"),
                                          "node_line",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_toline), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeCurveAction",
                                          _("Node Curve"),
                                          _("Make selected segments curves"),
                                          "node_curve",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_tocurve), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowHandlesAction",
                                                      _("Show Handles"),
                                                      _("Show the Bezier handles of selected nodes"),
                                                      "nodes_show_handles",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_show_handles), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.nodes", "show_handles", 1 ) );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowHelperpath",
                                                      _("Show Outline"),
                                                      _("Show the outline of the path"),
                                                      "nodes_show_helperpath",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_show_helperpath), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.nodes", "show_helperpath", 0 ) );
    }

    {
        InkAction* inky = ink_action_new( "EditNextLPEParameterAction",
                                          _("Next path effect parameter"),
                                          _("Show next path effect parameter for editing"),
                                          "edit_next_parameter",
                                          Inkscape::ICON_SIZE_DECORATION );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_nextLPEparam), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        g_object_set_data( holder, "nodes_lpeedit", inky);
    }

    {
        InkAction* inky = ink_action_new( "ObjectEditClipPathAction",
                                          _("Edit clipping path"),
                                          _("Edit the clipping path of the object"),
                                          "nodeedit-clippath",
                                          Inkscape::ICON_SIZE_DECORATION );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_clippath), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        g_object_set_data( holder, "nodes_clippathedit", inky);
    }

    {
        InkAction* inky = ink_action_new( "ObjectEditMaskPathAction",
                                          _("Edit mask path"),
                                          _("Edit the mask of the object"),
                                          "nodeedit-mask",
                                          Inkscape::ICON_SIZE_DECORATION );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_maskpath), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        g_object_set_data( holder, "nodes_maskedit", inky);
    }

    /* X coord of selected node(s) */
    {
        EgeAdjustmentAction* eact = 0;
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "NodeXAction",
                                         _("X coordinate:"), _("X:"), _("X coordinate of selected node(s)"),
                                         "tools.nodes", "Xcoord", 0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, TRUE, "altx-nodes",
                                         -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_node_path_x_value_changed );
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "nodes_x_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Y coord of selected node(s) */
    {
        EgeAdjustmentAction* eact = 0;
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "NodeYAction",
                                         _("Y coordinate:"), _("Y:"), _("Y coordinate of selected node(s)"),
                                         "tools.nodes", "Ycoord", 0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, FALSE, NULL,
                                         -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_node_path_y_value_changed );
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "nodes_y_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // add the units menu
    {
        GtkAction* act = tracker->createAction( "NodeUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act );
    }


    sp_node_toolbox_sel_changed(sp_desktop_selection(desktop), holder);

    //watch selection
    Inkscape::ConnectionPool* pool = Inkscape::ConnectionPool::new_connection_pool ("ISNodeToolbox");

    sigc::connection *c_selection_changed =
        new sigc::connection (sp_desktop_selection (desktop)->connectChanged
                              (sigc::bind (sigc::ptr_fun (sp_node_toolbox_sel_changed), (GObject*)holder)));
    pool->add_connection ("selection-changed", c_selection_changed);

    sigc::connection *c_selection_modified =
        new sigc::connection (sp_desktop_selection (desktop)->connectModified
                              (sigc::bind (sigc::ptr_fun (sp_node_toolbox_sel_modified), (GObject*)holder)));
    pool->add_connection ("selection-modified", c_selection_modified);

    sigc::connection *c_subselection_changed =
        new sigc::connection (desktop->connectToolSubselectionChanged
                              (sigc::bind (sigc::ptr_fun (sp_node_toolbox_coord_changed), (GObject*)holder)));
    pool->add_connection ("tool-subselection-changed", c_subselection_changed);

    Inkscape::ConnectionPool::connect_destroy (G_OBJECT (holder), pool);

    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
} // end of sp_node_toolbox_prep()


//########################
//##    Zoom Toolbox    ##
//########################

static void sp_zoom_toolbox_prep(SPDesktop */*desktop*/, GtkActionGroup* /*mainActions*/, GObject* /*holder*/)
{
    // no custom GtkAction setup needed
} // end of sp_zoom_toolbox_prep()

void
sp_tool_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    if ( GTK_IS_BIN(toolbox) ) {
        toolbox_set_desktop(gtk_bin_get_child(GTK_BIN(toolbox)), desktop, setup_tool_toolbox, update_tool_toolbox, static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox), "event_context_connection")));
    } else if (GTK_IS_TOOLBAR(toolbox) ) {
        toolbox_set_desktop(toolbox,
                            desktop,
                            setup_tool_toolbox,
                            update_tool_toolbox,
                            static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox),
                                                                             "event_context_connection")));
    } else {
        g_warning("Unexpected toolbox type");
    }
}


void
sp_aux_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    toolbox_set_desktop(gtk_bin_get_child(GTK_BIN(toolbox)),
                        desktop,
                        setup_aux_toolbox,
                        update_aux_toolbox,
                        static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox),
                                                                         "event_context_connection")));
}

void
sp_commands_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    toolbox_set_desktop(toolbox,
                        desktop,
                        setup_commands_toolbox,
                        update_commands_toolbox,
                        static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox),
                                                                         "event_context_connection")));
}

static void
toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func, sigc::connection *conn)
{
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

    if (desktop) {
        gtk_widget_set_sensitive(toolbox, TRUE);
        setup_func(toolbox, desktop);
        update_func(desktop, desktop->event_context, toolbox);
        *conn = desktop->connectEventContextChanged
            (sigc::bind (sigc::ptr_fun(update_func), toolbox));
    } else {
        gtk_widget_set_sensitive(toolbox, FALSE);
    }

} // end of toolbox_set_desktop()


static void
setup_tool_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    GtkTooltips *tooltips=GTK_TOOLTIPS(g_object_get_data(G_OBJECT(toolbox), "tooltips"));
    Inkscape::IconSize toolboxSize = prefToSize("toolbox.tools", "small");

    for (int i = 0 ; tools[i].type_name ; i++ ) {
        GtkWidget *button =
            sp_toolbox_button_new_from_verb_with_doubleclick( toolbox, toolboxSize,
                                                              SP_BUTTON_TYPE_TOGGLE,
                                                              Inkscape::Verb::get(tools[i].verb),
                                                              Inkscape::Verb::get(tools[i].doubleclick_verb),
                                                              desktop,
                                                              tooltips );

        g_object_set_data( G_OBJECT(toolbox), tools[i].data_name,
                           (gpointer)button );
    }
}


static void
update_tool_toolbox( SPDesktop */*desktop*/, SPEventContext *eventcontext, GtkWidget *toolbox )
{
    gchar const *const tname = ( eventcontext
                                 ? gtk_type_name(GTK_OBJECT_TYPE(eventcontext))
                                 : NULL );
    for (int i = 0 ; tools[i].type_name ; i++ ) {
        SPButton *button = SP_BUTTON(g_object_get_data(G_OBJECT(toolbox), tools[i].data_name));
        sp_button_toggle_set_down(button, tname && !strcmp(tname, tools[i].type_name));
    }
}

static void
setup_aux_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    GtkSizeGroup* grouper = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
    GtkActionGroup* mainActions = create_or_fetch_actions( desktop );
    GtkUIManager* mgr = gtk_ui_manager_new();
    GError* errVal = 0;
    gtk_ui_manager_insert_action_group( mgr, mainActions, 0 );
    gtk_ui_manager_add_ui_from_string( mgr, ui_descr, -1, &errVal );

    std::map<std::string, GtkWidget*> dataHolders;

    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        if ( aux_toolboxes[i].prep_func ) {
            // converted to GtkActions and UIManager

            GtkWidget* kludge = gtk_toolbar_new();
            g_object_set_data( G_OBJECT(kludge), "dtw", desktop->canvas);
            g_object_set_data( G_OBJECT(kludge), "desktop", desktop);
            dataHolders[aux_toolboxes[i].type_name] = kludge;
            aux_toolboxes[i].prep_func( desktop, mainActions, G_OBJECT(kludge) );
        } else {

            GtkWidget *sub_toolbox = 0;
            if (aux_toolboxes[i].create_func == NULL)
                sub_toolbox = sp_empty_toolbox_new(desktop);
            else {
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

            GtkWidget* holder = gtk_table_new( 1, 3, FALSE );
            gtk_table_attach( GTK_TABLE(holder), kludge, 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0 );

            gchar* tmp = g_strdup_printf( "/ui/%s", aux_toolboxes[i].ui_name );
            GtkWidget* toolBar = gtk_ui_manager_get_widget( mgr, tmp );
            g_free( tmp );
            tmp = 0;

            Inkscape::IconSize toolboxSize = prefToSize("toolbox", "small");
            if ( prefs_get_int_attribute_limited( "toolbox", "icononly", 1, 0, 1 ) ) {
                gtk_toolbar_set_style( GTK_TOOLBAR(toolBar), GTK_TOOLBAR_ICONS );
            }
            gtk_toolbar_set_icon_size( GTK_TOOLBAR(toolBar), static_cast<GtkIconSize>(toolboxSize) );


            gtk_table_attach( GTK_TABLE(holder), toolBar, 0, 1, 0, 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0 );

            if ( aux_toolboxes[i].swatch_verb_id != SP_VERB_INVALID ) {
                Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch( NULL, _(aux_toolboxes[i].swatch_tip) );
                swatch->setDesktop( desktop );
                swatch->setClickVerb( aux_toolboxes[i].swatch_verb_id );
                swatch->setWatchedTool( aux_toolboxes[i].swatch_tool, true );
                GtkWidget *swatch_ = GTK_WIDGET( swatch->gobj() );
                gtk_table_attach( GTK_TABLE(holder), swatch_, 1, 2, 0, 1, (GtkAttachOptions)(GTK_SHRINK | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), AUX_BETWEEN_BUTTON_GROUPS, 0 );
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

static void
update_aux_toolbox(SPDesktop */*desktop*/, SPEventContext *eventcontext, GtkWidget *toolbox)
{
    gchar const *tname = ( eventcontext
                           ? gtk_type_name(GTK_OBJECT_TYPE(eventcontext))
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

static void
setup_commands_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
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
        "    <toolitem action='FileExport' />"
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
        "    <toolitem action='DialogXMLEditor' />"
        "    <toolitem action='DialogAlignDistribute' />"
        "    <separator />"
        "    <toolitem action='DialogPreferences' />"
        "    <toolitem action='DialogDocumentProperties' />"
        "  </toolbar>"
        "</ui>";
    GtkActionGroup* mainActions = create_or_fetch_actions( desktop );


    GtkUIManager* mgr = gtk_ui_manager_new();
    GError* errVal = 0;

    gtk_ui_manager_insert_action_group( mgr, mainActions, 0 );
    gtk_ui_manager_add_ui_from_string( mgr, descr, -1, &errVal );

    GtkWidget* toolBar = gtk_ui_manager_get_widget( mgr, "/ui/CommandsToolbar" );
    if ( prefs_get_int_attribute_limited( "toolbox", "icononly", 1, 0, 1 ) ) {
        gtk_toolbar_set_style( GTK_TOOLBAR(toolBar), GTK_TOOLBAR_ICONS );
    }

    Inkscape::IconSize toolboxSize = prefToSize("toolbox", "small");
    gtk_toolbar_set_icon_size( GTK_TOOLBAR(toolBar), (GtkIconSize)toolboxSize );

    gtk_toolbar_set_orientation(GTK_TOOLBAR(toolBar), GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolBar), TRUE);


    g_object_set_data(G_OBJECT(toolBar), "desktop", NULL);

    GtkWidget* child = gtk_bin_get_child(GTK_BIN(toolbox));
    if ( child ) {
        gtk_container_remove( GTK_CONTAINER(toolbox), child );
    }

    gtk_container_add( GTK_CONTAINER(toolbox), toolBar );
}

static void
update_commands_toolbox(SPDesktop */*desktop*/, SPEventContext */*eventcontext*/, GtkWidget */*toolbox*/)
{
}

void show_aux_toolbox(GtkWidget *toolbox_toplevel)
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

static GtkWidget *
sp_empty_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_toolbar_new();
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}

// helper UI functions

GtkWidget *
sp_tb_spinbutton(
    gchar *label, gchar const *tooltip,
    gchar const *path, gchar const *data, gdouble def,
    GtkWidget *us,
    GtkWidget *tbl,
    gboolean altx, gchar const *altx_mark,
    gdouble lower, gdouble upper, gdouble step, gdouble page,
    void (*callback)(GtkAdjustment *, GtkWidget *),
    gdouble climb = 0.1, guint digits = 3, double factor = 1.0)
{
    GtkTooltips *tt = gtk_tooltips_new();

    GtkWidget *hb = gtk_hbox_new(FALSE, 1);

    GtkWidget *l = gtk_label_new(label);
    gtk_widget_show(l);
    gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
    gtk_container_add(GTK_CONTAINER(hb), l);

    GtkObject *a = gtk_adjustment_new(prefs_get_double_attribute(path, data, def) * factor,
                                      lower, upper, step, page, page);
    gtk_object_set_data(GTK_OBJECT(tbl), data, a);
    if (us)
        sp_unit_selector_add_adjustment(SP_UNIT_SELECTOR(us), GTK_ADJUSTMENT(a));

    GtkWidget *sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), climb, digits);
    gtk_tooltips_set_tip(tt, sb, tooltip, NULL);
    if (altx)
        gtk_object_set_data(GTK_OBJECT(sb), altx_mark, sb);
    gtk_widget_set_size_request(sb,
                                (upper <= 1.0 || digits == 0)? AUX_SPINBUTTON_WIDTH_SMALL - 10: AUX_SPINBUTTON_WIDTH_SMALL,
                                AUX_SPINBUTTON_HEIGHT);
    gtk_widget_show(sb);
    gtk_signal_connect(GTK_OBJECT(sb), "focus-in-event", GTK_SIGNAL_FUNC(spinbutton_focus_in), tbl);
    gtk_signal_connect(GTK_OBJECT(sb), "key-press-event", GTK_SIGNAL_FUNC(spinbutton_keypress), tbl);
    gtk_container_add(GTK_CONTAINER(hb), sb);
    gtk_signal_connect(GTK_OBJECT(a), "value_changed", GTK_SIGNAL_FUNC(callback), tbl);

    return hb;
}

#define MODE_LABEL_WIDTH 70

//########################
//##       Star         ##
//########################

static void sp_stb_magnitude_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        // do not remember prefs if this call is initiated by an undo change, because undoing object
        // creation sets bogus values to its attributes before it is deleted
        prefs_set_int_attribute("tools.shapes.star", "magnitude", (gint)adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_int(repr,"sodipodi:sides",(gint)adj->value);
            sp_repr_set_svg_double(repr, "sodipodi:arg2",
                                   (sp_repr_get_double_attribute(repr, "sodipodi:arg1", 0.5)
                                    + M_PI / (gint)adj->value));
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }
    if (modmade)  sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                                   _("Star: Change number of corners"));

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_proportion_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.star", "proportion", adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);

            gdouble r1 = sp_repr_get_double_attribute(repr, "sodipodi:r1", 1.0);
            gdouble r2 = sp_repr_get_double_attribute(repr, "sodipodi:r2", 1.0);
            if (r2 < r1) {
                sp_repr_set_svg_double(repr, "sodipodi:r2", r1*adj->value);
            } else {
                sp_repr_set_svg_double(repr, "sodipodi:r1", r2*adj->value);
            }

            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }

    if (modmade) sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                                   _("Star: Change spoke ratio"));

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_sides_flat_state_changed( EgeSelectOneAction *act, GObject *dataKludge )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );
    bool flat = ege_select_one_action_get_active( act ) == 0;

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_string_attribute( "tools.shapes.star", "isflatsided",
                                    flat ? "true" : "false" );
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    GtkAction* prop_action = GTK_ACTION( g_object_get_data( dataKludge, "prop_action" ) );
    bool modmade = false;

    if ( prop_action ) {
        gtk_action_set_sensitive( prop_action, !flat );
    }

    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            repr->setAttribute("inkscape:flatsided", flat ? "true" : "false" );
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                         flat ? _("Make polygon") : _("Make star"));
    }

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_rounded_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.star", "rounded", (gdouble) adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double(repr, "inkscape:rounded", (gdouble) adj->value);
            SP_OBJECT(items->data)->updateRepr();
            modmade = true;
        }
    }
    if (modmade)  sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                                   _("Star: Change rounding"));

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_randomized_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.star", "randomized", (gdouble) adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double(repr, "inkscape:randomized", (gdouble) adj->value);
            SP_OBJECT(items->data)->updateRepr();
            modmade = true;
        }
    }
    if (modmade)  sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                                   _("Star: Change randomization"));

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}


static void star_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const */*old_value*/, gchar const */*new_value*/,
                                       bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    GtkAdjustment *adj = 0;

    gchar const *flatsidedstr = prefs_get_string_attribute( "tools.shapes.star", "isflatsided" );
    bool isFlatSided = flatsidedstr ? (strcmp(flatsidedstr, "false") != 0) : true;

    if (!strcmp(name, "inkscape:randomized")) {
        adj = GTK_ADJUSTMENT( gtk_object_get_data(GTK_OBJECT(tbl), "randomized") );
        gtk_adjustment_set_value(adj, sp_repr_get_double_attribute(repr, "inkscape:randomized", 0.0));
    } else if (!strcmp(name, "inkscape:rounded")) {
        adj = GTK_ADJUSTMENT( gtk_object_get_data(GTK_OBJECT(tbl), "rounded") );
        gtk_adjustment_set_value(adj, sp_repr_get_double_attribute(repr, "inkscape:rounded", 0.0));
    } else if (!strcmp(name, "inkscape:flatsided")) {
        GtkAction* prop_action = GTK_ACTION( g_object_get_data(G_OBJECT(tbl), "prop_action") );
        char const *flatsides = repr->attribute("inkscape:flatsided");
        EgeSelectOneAction* flat_action = EGE_SELECT_ONE_ACTION( g_object_get_data( G_OBJECT(tbl), "flat_action" ) );
        if ( flatsides && !strcmp(flatsides,"false") ) {
            ege_select_one_action_set_active( flat_action, 1 );
            gtk_action_set_sensitive( prop_action, TRUE );
        } else {
            ege_select_one_action_set_active( flat_action, 0 );
            gtk_action_set_sensitive( prop_action, FALSE );
        }
    } else if ((!strcmp(name, "sodipodi:r1") || !strcmp(name, "sodipodi:r2")) && (!isFlatSided) ) {
        adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "proportion");
        gdouble r1 = sp_repr_get_double_attribute(repr, "sodipodi:r1", 1.0);
        gdouble r2 = sp_repr_get_double_attribute(repr, "sodipodi:r2", 1.0);
        if (r2 < r1) {
            gtk_adjustment_set_value(adj, r2/r1);
        } else {
            gtk_adjustment_set_value(adj, r1/r2);
        }
    } else if (!strcmp(name, "sodipodi:sides")) {
        adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "magnitude");
        gtk_adjustment_set_value(adj, sp_repr_get_int_attribute(repr, "sodipodi:sides", 0));
    }

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}


static Inkscape::XML::NodeEventVector star_tb_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    star_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


/**
 *  \param selection Should not be NULL.
 */
static void
sp_star_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;

    purge_repr_listener( tbl, tbl );

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_STAR((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );
    } else if (n_selected == 1) {
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &star_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &star_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected stars
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
    }
}


static void sp_stb_defaults( GtkWidget */*widget*/, GObject *dataKludge )
{
    // FIXME: in this and all other _default functions, set some flag telling the value_changed
    // callbacks to lump all the changes for all selected objects in one undo step

    GtkAdjustment *adj = 0;

    // fixme: make settable in prefs!
    gint mag = 5;
    gdouble prop = 0.5;
    gboolean flat = FALSE;
    gdouble randomized = 0;
    gdouble rounded = 0;

    EgeSelectOneAction* flat_action = EGE_SELECT_ONE_ACTION( g_object_get_data( dataKludge, "flat_action" ) );
    ege_select_one_action_set_active( flat_action, flat ? 0 : 1 );

    GtkAction* sb2 = GTK_ACTION( g_object_get_data( dataKludge, "prop_action" ) );
    gtk_action_set_sensitive( sb2, !flat );

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "magnitude" ) );
    gtk_adjustment_set_value(adj, mag);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "proportion" ) );
    gtk_adjustment_set_value(adj, prop);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "rounded" ) );
    gtk_adjustment_set_value(adj, rounded);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "randomized" ) );
    gtk_adjustment_set_value(adj, randomized);
    gtk_adjustment_value_changed(adj);
}


void
sp_toolbox_add_label(GtkWidget *tbl, gchar const *title, bool wide)
{
    GtkWidget *boxl = gtk_hbox_new(FALSE, 0);
    if (wide) gtk_widget_set_size_request(boxl, MODE_LABEL_WIDTH, -1);
    GtkWidget *l = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(l), title);
    gtk_box_pack_end(GTK_BOX(boxl), l, FALSE, FALSE, 0);
    if ( GTK_IS_TOOLBAR(tbl) ) {
        gtk_toolbar_append_widget( GTK_TOOLBAR(tbl), boxl, "", "" );
    } else {
        gtk_box_pack_start(GTK_BOX(tbl), boxl, FALSE, FALSE, 0);
    }
    gtk_object_set_data(GTK_OBJECT(tbl), "mode_label", l);
}


static void sp_star_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

    {
        EgeOutputAction* act = ege_output_action_new( "StarStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    {
        EgeAdjustmentAction* eact = 0;
        gchar const *flatsidedstr = prefs_get_string_attribute( "tools.shapes.star", "isflatsided" );
        bool isFlatSided = flatsidedstr ? (strcmp(flatsidedstr, "false") != 0) : true;

        /* Flatsided checkbox */
        {
            GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Polygon"),
                                1, _("Regular polygon (with one handle) instead of a star"),
                                2, "star_flat",
                                -1 );

            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Star"),
                                1, _("Star instead of a regular polygon (with one handle)"),
                                2, "star_angled",
                                -1 );

            EgeSelectOneAction* act = ege_select_one_action_new( "FlatAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
            gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
            g_object_set_data( holder, "flat_action", act );

            ege_select_one_action_set_appearance( act, "full" );
            ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
            g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
            ege_select_one_action_set_icon_column( act, 2 );
            ege_select_one_action_set_icon_size( act, secondarySize );
            ege_select_one_action_set_tooltip_column( act, 1  );

            ege_select_one_action_set_active( act, isFlatSided ? 0 : 1 );
            g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_stb_sides_flat_state_changed), holder);
        }

        /* Magnitude */
        {
        gchar const* labels[] = {_("triangle/tri-star"), _("square/quad-star"), _("pentagon/five-pointed star"), _("hexagon/six-pointed star"), 0, 0, 0, 0, 0};
        gdouble values[] = {3, 4, 5, 6, 7, 8, 10, 12, 20};
        eact = create_adjustment_action( "MagnitudeAction",
                                         _("Corners"), _("Corners:"), _("Number of corners of a polygon or star"),
                                         "tools.shapes.star", "magnitude", 3,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         3, 1024, 1, 5,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_magnitude_value_changed,
                                         1.0, 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        /* Spoke ratio */
        {
        gchar const* labels[] = {_("thin-ray star"), 0, _("pentagram"), _("hexagram"), _("heptagram"), _("octagram"), _("regular polygon")};
        gdouble values[] = {0.01, 0.2, 0.382, 0.577, 0.692, 0.765, 1};
        eact = create_adjustment_action( "SpokeAction",
                                         _("Spoke ratio"), _("Spoke ratio:"),
                                         // TRANSLATORS: Tip radius of a star is the distance from the center to the farthest handle.
                                         // Base radius is the same for the closest handle.
                                         _("Base radius to tip radius ratio"),
                                         "tools.shapes.star", "proportion", 0.5,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         0.01, 1.0, 0.01, 0.1,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_proportion_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "prop_action", eact );
        }

        if ( !isFlatSided ) {
            gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        } else {
            gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        }

        /* Roundedness */
        {
        gchar const* labels[] = {_("stretched"), _("twisted"), _("slightly pinched"), _("NOT rounded"), _("slightly rounded"), _("visibly rounded"), _("well rounded"), _("amply rounded"), 0, _("stretched"), _("blown up")};
        gdouble values[] = {-1, -0.2, -0.03, 0, 0.05, 0.1, 0.2, 0.3, 0.5, 1, 10};
        eact = create_adjustment_action( "RoundednessAction",
                                         _("Rounded"), _("Rounded:"), _("How much rounded are the corners (0 for sharp)"),
                                         "tools.shapes.star", "rounded", 0.0,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         -10.0, 10.0, 0.01, 0.1,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_rounded_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        /* Randomization */
        {
        gchar const* labels[] = {_("NOT randomized"), _("slightly irregular"), _("visibly randomized"), _("strongly randomized"), _("blown up")};
        gdouble values[] = {0, 0.01, 0.1, 0.5, 10};
        eact = create_adjustment_action( "RandomizationAction",
                                         _("Randomized"), _("Randomized:"), _("Scatter randomly the corners and angles"),
                                         "tools.shapes.star", "randomized", 0.0,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         -10.0, 10.0, 0.001, 0.01,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_randomized_value_changed, 0.1, 3 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }
    }

    {
        /* Reset */
        {
            GtkAction* act = gtk_action_new( "StarResetAction",
                                             _("Defaults"),
                                             _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                             GTK_STOCK_CLEAR );
            g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(sp_stb_defaults), holder );
            gtk_action_group_add_action( mainActions, act );
            gtk_action_set_sensitive( act, TRUE );
        }
    }

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_star_toolbox_selection_changed), (GObject *)holder))
        );
    g_signal_connect( holder, "destroy", G_CALLBACK(delete_connection), connection );
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
}


//########################
//##       Rect         ##
//########################

static void sp_rtb_sensitivize( GObject *tbl )
{
    GtkAdjustment *adj1 = GTK_ADJUSTMENT( g_object_get_data(tbl, "rx") );
    GtkAdjustment *adj2 = GTK_ADJUSTMENT( g_object_get_data(tbl, "ry") );
    GtkAction* not_rounded = GTK_ACTION( g_object_get_data(tbl, "not_rounded") );

    if (adj1->value == 0 && adj2->value == 0 && g_object_get_data(tbl, "single")) { // only for a single selected rect (for now)
        gtk_action_set_sensitive( not_rounded, FALSE );
    } else {
        gtk_action_set_sensitive( not_rounded, TRUE );
    }
}


static void
sp_rtb_value_changed(GtkAdjustment *adj, GObject *tbl, gchar const *value_name,
                          void (*setter)(SPRect *, gdouble))
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    SPUnit const *unit = tracker->getActiveUnit();

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.rect", value_name, sp_units_get_pixels(adj->value, *unit));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    for (GSList const *items = selection->itemList(); items != NULL; items = items->next) {
        if (SP_IS_RECT(items->data)) {
            if (adj->value != 0) {
                setter(SP_RECT(items->data), sp_units_get_pixels(adj->value, *unit));
            } else {
                SP_OBJECT_REPR(items->data)->setAttribute(value_name, NULL);
            }
            modmade = true;
        }
    }

    sp_rtb_sensitivize( tbl );

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_RECT,
                                   _("Change rectangle"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void
sp_rtb_rx_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "rx", sp_rect_set_visible_rx);
}

static void
sp_rtb_ry_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "ry", sp_rect_set_visible_ry);
}

static void
sp_rtb_width_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "width", sp_rect_set_visible_width);
}

static void
sp_rtb_height_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "height", sp_rect_set_visible_height);
}



static void
sp_rtb_defaults( GtkWidget */*widget*/, GObject *obj)
{
    GtkAdjustment *adj = 0;

    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "rx") );
    gtk_adjustment_set_value(adj, 0.0);
    // this is necessary if the previous value was 0, but we still need to run the callback to change all selected objects
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "ry") );
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    sp_rtb_sensitivize( obj );
}

static void rect_tb_event_attr_changed(Inkscape::XML::Node */*repr*/, gchar const */*name*/,
                                       gchar const */*old_value*/, gchar const */*new_value*/,
                                       bool /*is_interactive*/, gpointer data)
{
    GObject *tbl = G_OBJECT(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>( g_object_get_data( tbl, "tracker" ) );
    SPUnit const *unit = tracker->getActiveUnit();

    gpointer item = g_object_get_data( tbl, "item" );
    if (item && SP_IS_RECT(item)) {
        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "rx" ) );
            gdouble rx = sp_rect_get_visible_rx(SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(rx, *unit));
        }

        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "ry" ) );
            gdouble ry = sp_rect_get_visible_ry(SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(ry, *unit));
        }

        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "width" ) );
            gdouble width = sp_rect_get_visible_width (SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(width, *unit));
        }

        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "height" ) );
            gdouble height = sp_rect_get_visible_height (SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(height, *unit));
        }
    }

    sp_rtb_sensitivize( tbl );

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}


static Inkscape::XML::NodeEventVector rect_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    rect_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
 *  \param selection should not be NULL.
 */
static void
sp_rect_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    SPItem *item = NULL;

    if ( g_object_get_data( tbl, "repr" ) ) {
        g_object_set_data( tbl, "item", NULL );
    }
    purge_repr_listener( tbl, tbl );

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_RECT((SPItem *) items->data)) {
            n_selected++;
            item = (SPItem *) items->data;
            repr = SP_OBJECT_REPR(item);
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    g_object_set_data( tbl, "single", GINT_TO_POINTER(FALSE) );

    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );

        GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "width_action" ) );
        gtk_action_set_sensitive(w, FALSE);
        GtkAction* h = GTK_ACTION( g_object_get_data( tbl, "height_action" ) );
        gtk_action_set_sensitive(h, FALSE);

    } else if (n_selected == 1) {
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
        g_object_set_data( tbl, "single", GINT_TO_POINTER(TRUE) );

        GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "width_action" ) );
        gtk_action_set_sensitive(w, TRUE);
        GtkAction* h = GTK_ACTION( g_object_get_data( tbl, "height_action" ) );
        gtk_action_set_sensitive(h, TRUE);

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            g_object_set_data( tbl, "item", item );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &rect_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &rect_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
        sp_rtb_sensitivize( tbl );
    }
}


static void sp_rect_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

    {
        EgeOutputAction* act = ege_output_action_new( "RectStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    // rx/ry units menu: create
    UnitTracker* tracker = new UnitTracker( SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE );
    //tracker->addUnit( SP_UNIT_PERCENT, 0 );
    // fixme: add % meaning per cent of the width/height
    tracker->setActiveUnit( sp_desktop_namedview(desktop)->doc_units );
    g_object_set_data( holder, "tracker", tracker );

    /* W */
    {
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "RectWidthAction",
                                         _("Width"), _("W:"), _("Width of rectangle"),
                                         "tools.shapes.rect", "width", 0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, TRUE, "altx-rect",
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_width_value_changed );
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "width_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* H */
    {
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "RectHeightAction",
                                         _("Height"), _("H:"), _("Height of rectangle"),
                                         "tools.shapes.rect", "height", 0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_height_value_changed );
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "height_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* rx */
    {
        gchar const* labels[] = {_("not rounded"), 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {0.5, 1, 2, 3, 5, 10, 20, 50, 100};
        eact = create_adjustment_action( "RadiusXAction",
                                         _("Horizontal radius"), _("Rx:"), _("Horizontal radius of rounded corners"),
                                         "tools.shapes.rect", "rx", 0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_rx_value_changed);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* ry */
    {
        gchar const* labels[] = {_("not rounded"), 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {0.5, 1, 2, 3, 5, 10, 20, 50, 100};
        eact = create_adjustment_action( "RadiusYAction",
                                         _("Vertical radius"), _("Ry:"), _("Vertical radius of rounded corners"),
                                         "tools.shapes.rect", "ry", 0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_ry_value_changed);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // add the units menu
    {
        GtkAction* act = tracker->createAction( "RectUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act );
    }

    /* Reset */
    {
        InkAction* inky = ink_action_new( "RectResetAction",
                                          _("Not rounded"),
                                          _("Make corners sharp"),
                                          "squared_corner",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_rtb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), TRUE );
        g_object_set_data( holder, "not_rounded", inky );
    }

    g_object_set_data( holder, "single", GINT_TO_POINTER(TRUE) );
    sp_rtb_sensitivize( holder );

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_rect_toolbox_selection_changed), (GObject *)holder))
        );
    g_signal_connect( holder, "destroy", G_CALLBACK(delete_connection), connection );
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
}

//########################
//##       3D Box       ##
//########################

// normalize angle so that it lies in the interval [0,360]
static double box3d_normalize_angle (double a) {
    double angle = a + ((int) (a/360.0))*360;
    if (angle < 0) {
        angle += 360.0;
    }
    return angle;
}

static void
box3d_set_button_and_adjustment(Persp3D *persp, Proj::Axis axis,
                                GtkAdjustment *adj, GtkAction *act, GtkToggleAction *tact) {
    // TODO: Take all selected perspectives into account but don't touch the state button if not all of them
    //       have the same state (otherwise a call to box3d_vp_z_state_changed() is triggered and the states
    //       are reset).
    bool is_infinite = !persp3d_VP_is_finite(persp, axis);

    if (is_infinite) {
        gtk_toggle_action_set_active(tact, TRUE);
        gtk_action_set_sensitive(act, TRUE);

        double angle = persp3d_get_infinite_angle(persp, axis);
        if (angle != NR_HUGE) { // FIXME: We should catch this error earlier (don't show the spinbutton at all)
            gtk_adjustment_set_value(adj, box3d_normalize_angle(angle));
        }
    } else {
        gtk_toggle_action_set_active(tact, FALSE);
        gtk_action_set_sensitive(act, FALSE);
    }
}

static void
box3d_resync_toolbar(Inkscape::XML::Node *persp_repr, GObject *data) {
    if (!persp_repr) {
        g_print ("No perspective given to box3d_resync_toolbar().\n");
        return;
    }

    GtkWidget *tbl = GTK_WIDGET(data);
    GtkAdjustment *adj = 0;
    GtkAction *act = 0;
    GtkToggleAction *tact = 0;
    Persp3D *persp = persp3d_get_from_repr(persp_repr);
    {
        adj = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "box3d_angle_x"));
        act = GTK_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_angle_x_action"));
        tact = &INK_TOGGLE_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_vp_x_state_action"))->action;

        box3d_set_button_and_adjustment(persp, Proj::X, adj, act, tact);
    }
    {
        adj = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "box3d_angle_y"));
        act = GTK_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_angle_y_action"));
        tact = &INK_TOGGLE_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_vp_y_state_action"))->action;

        box3d_set_button_and_adjustment(persp, Proj::Y, adj, act, tact);
    }
    {
        adj = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "box3d_angle_z"));
        act = GTK_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_angle_z_action"));
        tact = &INK_TOGGLE_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_vp_z_state_action"))->action;

        box3d_set_button_and_adjustment(persp, Proj::Z, adj, act, tact);
    }
}

static void box3d_persp_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const */*name*/,
                                                  gchar const */*old_value*/, gchar const */*new_value*/,
                                                  bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the attr_changed listener
    // note: it used to work without the differently called freeze_ attributes (here and in
    //       box3d_angle_value_changed()) but I now it doesn't so I'm leaving them in for now
    if (g_object_get_data(G_OBJECT(tbl), "freeze_angle")) {
        return;
    }

    // set freeze so that it can be caught in box3d_angle_z_value_changed() (to avoid calling
    // sp_document_maybe_done() when the document is undo insensitive)
    g_object_set_data(G_OBJECT(tbl), "freeze_attr", GINT_TO_POINTER(TRUE));

    // TODO: Only update the appropriate part of the toolbar
//    if (!strcmp(name, "inkscape:vp_z")) {
        box3d_resync_toolbar(repr, G_OBJECT(tbl));
//    }

    Persp3D *persp = persp3d_get_from_repr(repr);
    persp3d_update_box_reprs(persp);

    g_object_set_data(G_OBJECT(tbl), "freeze_attr", GINT_TO_POINTER(FALSE));
}

static Inkscape::XML::NodeEventVector box3d_persp_tb_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    box3d_persp_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
 *  \param selection Should not be NULL.
 */
// FIXME: This should rather be put into persp3d-reference.cpp or something similar so that it reacts upon each
//        Change of the perspective, and not of the current selection (but how to refer to the toolbar then?)
static void
box3d_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    // Here the following should be done: If all selected boxes have finite VPs in a certain direction,
    // disable the angle entry fields for this direction (otherwise entering a value in them should only
    // update the perspectives with infinite VPs and leave the other ones untouched).

    Inkscape::XML::Node *persp_repr = NULL;
    purge_repr_listener(tbl, tbl);

    SPItem *item = selection->singleItem();
    if (item && SP_IS_BOX3D(item)) {
        // FIXME: Also deal with multiple selected boxes
        SPBox3D *box = SP_BOX3D(item);
        Persp3D *persp = box3d_get_perspective(box);
        persp_repr = SP_OBJECT_REPR(persp);
        if (persp_repr) {
            g_object_set_data(tbl, "repr", persp_repr);
            Inkscape::GC::anchor(persp_repr);
            sp_repr_add_listener(persp_repr, &box3d_persp_tb_repr_events, tbl);
            sp_repr_synthesize_events(persp_repr, &box3d_persp_tb_repr_events, tbl);
        }

        inkscape_active_document()->current_persp3d = persp3d_get_from_repr(persp_repr);
        prefs_set_string_attribute("tools.shapes.3dbox", "persp", persp_repr->attribute("id"));

        box3d_resync_toolbar(persp_repr, tbl);
    }
}

static void
box3d_angle_value_changed(GtkAdjustment *adj, GObject *dataKludge, Proj::Axis axis)
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );
    SPDocument *document = sp_desktop_document(desktop);

    // quit if run by the attr_changed listener
    // note: it used to work without the differently called freeze_ attributes (here and in
    //       box3d_persp_tb_event_attr_changed()) but I now it doesn't so I'm leaving them in for now
    if (g_object_get_data( dataKludge, "freeze_attr" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(dataKludge, "freeze_angle", GINT_TO_POINTER(TRUE));

    //Persp3D *persp = document->current_persp3d;
    std::list<Persp3D *> sel_persps = sp_desktop_selection(desktop)->perspList();
    if (sel_persps.empty()) {
        // this can happen when the document is created; we silently ignore it
        return;
    }
    Persp3D *persp = sel_persps.front();

    persp->tmat.set_infinite_direction (axis, adj->value);
    SP_OBJECT(persp)->updateRepr();

    // TODO: use the correct axis here, too
    sp_document_maybe_done(document, "perspangle", SP_VERB_CONTEXT_3DBOX, _("3D Box: Change perspective (angle of infinite axis)"));

    g_object_set_data( dataKludge, "freeze_angle", GINT_TO_POINTER(FALSE) );
}


static void
box3d_angle_x_value_changed(GtkAdjustment *adj, GObject *dataKludge)
{
    box3d_angle_value_changed(adj, dataKludge, Proj::X);
}

static void
box3d_angle_y_value_changed(GtkAdjustment *adj, GObject *dataKludge)
{
    box3d_angle_value_changed(adj, dataKludge, Proj::Y);
}

static void
box3d_angle_z_value_changed(GtkAdjustment *adj, GObject *dataKludge)
{
    box3d_angle_value_changed(adj, dataKludge, Proj::Z);
}


static void box3d_vp_state_changed( GtkToggleAction *act, GtkAction */*box3d_angle*/, Proj::Axis axis )
{
    // TODO: Take all selected perspectives into account
    std::list<Persp3D *> sel_persps = sp_desktop_selection(inkscape_active_desktop())->perspList();
    if (sel_persps.empty()) {
        // this can happen when the document is created; we silently ignore it
        return;
    }
    Persp3D *persp = sel_persps.front();

    bool set_infinite = gtk_toggle_action_get_active(act);
    persp3d_set_VP_state (persp, axis, set_infinite ? Proj::VP_INFINITE : Proj::VP_FINITE);
}

static void box3d_vp_x_state_changed( GtkToggleAction *act, GtkAction *box3d_angle )
{
    box3d_vp_state_changed(act, box3d_angle, Proj::X);
}

static void box3d_vp_y_state_changed( GtkToggleAction *act, GtkAction *box3d_angle )
{
    box3d_vp_state_changed(act, box3d_angle, Proj::Y);
}

static void box3d_vp_z_state_changed( GtkToggleAction *act, GtkAction *box3d_angle )
{
    box3d_vp_state_changed(act, box3d_angle, Proj::Z);
}

static void box3d_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;
    SPDocument *document = sp_desktop_document (desktop);
    Persp3D *persp = document->current_persp3d;

    EgeAdjustmentAction* box3d_angle_x = 0;
    EgeAdjustmentAction* box3d_angle_y = 0;
    EgeAdjustmentAction* box3d_angle_z = 0;

    /* Angle X */
    {
        gchar const* labels[] = { 0, 0, 0, 0, 0, 0, 0 };
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        eact = create_adjustment_action( "3DBoxAngleXAction",
                                         _("Angle in X direction"), _("Angle X:"),
                                         // Translators: PL is short for 'perspective line'
                                         _("Angle of PLs in X direction"),
                                         "tools.shapes.3dbox", "box3d_angle_x", 30,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "altx-box3d",
                                         -360.0, 360.0, 1.0, 10.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         box3d_angle_x_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "box3d_angle_x_action", eact );
        box3d_angle_x = eact;
    }

    if (!persp3d_VP_is_finite(persp, Proj::X)) {
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    } else {
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
    }


    /* VP X state */
    {
        InkToggleAction* act = ink_toggle_action_new( "3DBoxVPXStateAction",
                                                      // Translators: VP is short for 'vanishing point'
                                                      _("State of VP in X direction"),
                                                      _("Toggle VP in X direction between 'finite' and 'infinite' (=parallel)"),
                                                      "toggle_vp_x",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "box3d_vp_x_state_action", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(box3d_vp_x_state_changed), box3d_angle_x );
        gtk_action_set_sensitive( GTK_ACTION(box3d_angle_x), !prefs_get_int_attribute( "tools.shapes.3dbox", "vp_x_state", 1 ) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.shapes.3dbox", "vp_x_state", 1 ) );
    }

    /* Angle Y */
    {
        gchar const* labels[] = { 0, 0, 0, 0, 0, 0, 0 };
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        eact = create_adjustment_action( "3DBoxAngleYAction",
                                         _("Angle in Y direction"), _("Angle Y:"),
                                         // Translators: PL is short for 'perspective line'
                                         _("Angle of PLs in Y direction"),
                                         "tools.shapes.3dbox", "box3d_angle_y", 30,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         box3d_angle_y_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "box3d_angle_y_action", eact );
        box3d_angle_y = eact;
    }

    if (!persp3d_VP_is_finite(persp, Proj::Y)) {
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    } else {
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
    }

    /* VP Y state */
    {
        InkToggleAction* act = ink_toggle_action_new( "3DBoxVPYStateAction",
                                                      // Translators: VP is short for 'vanishing point'
                                                      _("State of VP in Y direction"),
                                                      _("Toggle VP in Y direction between 'finite' and 'infinite' (=parallel)"),
                                                      "toggle_vp_y",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "box3d_vp_y_state_action", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(box3d_vp_y_state_changed), box3d_angle_y );
        gtk_action_set_sensitive( GTK_ACTION(box3d_angle_y), !prefs_get_int_attribute( "tools.shapes.3dbox", "vp_y_state", 1 ) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.shapes.3dbox", "vp_y_state", 1 ) );
    }

    /* Angle Z */
    {
        gchar const* labels[] = { 0, 0, 0, 0, 0, 0, 0 };
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        eact = create_adjustment_action( "3DBoxAngleZAction",
                                         _("Angle in Z direction"), _("Angle Z:"),
                                         // Translators: PL is short for 'perspective line'
                                         _("Angle of PLs in Z direction"),
                                         "tools.shapes.3dbox", "box3d_angle_z", 30,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         box3d_angle_z_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "box3d_angle_z_action", eact );
        box3d_angle_z = eact;
    }

    if (!persp3d_VP_is_finite(persp, Proj::Z)) {
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    } else {
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
    }

    /* VP Z state */
    {
        InkToggleAction* act = ink_toggle_action_new( "3DBoxVPZStateAction",
                                                      // Translators: VP is short for 'vanishing point'
                                                      _("State of VP in Z direction"),
                                                      _("Toggle VP in Z direction between 'finite' and 'infinite' (=parallel)"),
                                                      "toggle_vp_z",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "box3d_vp_z_state_action", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(box3d_vp_z_state_changed), box3d_angle_z );
        gtk_action_set_sensitive( GTK_ACTION(box3d_angle_z), !prefs_get_int_attribute( "tools.shapes.3dbox", "vp_z_state", 1 ) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.shapes.3dbox", "vp_z_state", 1 ) );
    }

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(box3d_toolbox_selection_changed), (GObject *)holder))
       );
    g_signal_connect(holder, "destroy", G_CALLBACK(delete_connection), connection);
    g_signal_connect(holder, "destroy", G_CALLBACK(purge_repr_listener), holder);
}

//########################
//##       Spiral       ##
//########################

static void
sp_spl_tb_value_changed(GtkAdjustment *adj, GObject *tbl, gchar const *value_name)
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.spiral", value_name, adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);

    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double( repr, namespaced_name, adj->value );
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }

    g_free(namespaced_name);

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_SPIRAL,
                                   _("Change spiral"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void
sp_spl_tb_revolution_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "revolution");
}

static void
sp_spl_tb_expansion_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "expansion");
}

static void
sp_spl_tb_t0_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "t0");
}

static void
sp_spl_tb_defaults(GtkWidget */*widget*/, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);

    GtkAdjustment *adj;

    // fixme: make settable
    gdouble rev = 5;
    gdouble exp = 1.0;
    gdouble t0 = 0.0;

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "revolution");
    gtk_adjustment_set_value(adj, rev);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "expansion");
    gtk_adjustment_set_value(adj, exp);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "t0");
    gtk_adjustment_set_value(adj, t0);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(GTK_OBJECT(tbl));
}


static void spiral_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const */*name*/,
                                         gchar const */*old_value*/, gchar const */*new_value*/,
                                         bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    GtkAdjustment *adj;
    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "revolution");
    gtk_adjustment_set_value(adj, (sp_repr_get_double_attribute(repr, "sodipodi:revolution", 3.0)));

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "expansion");
    gtk_adjustment_set_value(adj, (sp_repr_get_double_attribute(repr, "sodipodi:expansion", 1.0)));

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "t0");
    gtk_adjustment_set_value(adj, (sp_repr_get_double_attribute(repr, "sodipodi:t0", 0.0)));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}


static Inkscape::XML::NodeEventVector spiral_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    spiral_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

static void
sp_spiral_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;

    purge_repr_listener( tbl, tbl );

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );
    } else if (n_selected == 1) {
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &spiral_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &spiral_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
    }
}


static void sp_spiral_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

    {
        EgeOutputAction* act = ege_output_action_new( "SpiralStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    /* Revolution */
    {
        gchar const* labels[] = {_("just a curve"), 0, _("one full revolution"), 0, 0, 0, 0, 0, 0};
        gdouble values[] = {0.01, 0.5, 1, 2, 3, 5, 10, 20, 50, 100};
        eact = create_adjustment_action( "SpiralRevolutionAction",
                                         _("Number of turns"), _("Turns:"), _("Number of revolutions"),
                                         "tools.shapes.spiral", "revolution", 3.0,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "altx-spiral",
                                         0.01, 1024.0, 0.1, 1.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_spl_tb_revolution_value_changed, 1, 2);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Expansion */
    {
        gchar const* labels[] = {_("circle"), _("edge is much denser"), _("edge is denser"), _("even"), _("center is denser"), _("center is much denser"), 0};
        gdouble values[] = {0, 0.1, 0.5, 1, 1.5, 5, 20};
        eact = create_adjustment_action( "SpiralExpansionAction",
                                         _("Divergence"), _("Divergence:"), _("How much denser/sparser are outer revolutions; 1 = uniform"),
                                         "tools.shapes.spiral", "expansion", 1.0,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         0.0, 1000.0, 0.01, 1.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_spl_tb_expansion_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* T0 */
    {
        gchar const* labels[] = {_("starts from center"), _("starts mid-way"), _("starts near edge")};
        gdouble values[] = {0, 0.5, 0.9};
        eact = create_adjustment_action( "SpiralT0Action",
                                         _("Inner radius"), _("Inner radius:"), _("Radius of the innermost revolution (relative to the spiral size)"),
                                         "tools.shapes.spiral", "t0", 0.0,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         0.0, 0.999, 0.01, 1.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_spl_tb_t0_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Reset */
    {
        InkAction* inky = ink_action_new( "SpiralResetAction",
                                          _("Defaults"),
                                          _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                          GTK_STOCK_CLEAR,
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_spl_tb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }


    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_spiral_toolbox_selection_changed), (GObject *)holder))
        );
    g_signal_connect( holder, "destroy", G_CALLBACK(delete_connection), connection );
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
}

//########################
//##     Pen/Pencil    ##
//########################

static void sp_pc_spiro_spline_mode_changed(EgeSelectOneAction* act, GObject* /*tbl*/)
{
    prefs_set_int_attribute("tools.freehand", "spiro-spline-mode", ege_select_one_action_get_active(act));
}

static void sp_add_spiro_toggle(GtkActionGroup* mainActions, GObject* holder, bool tool_is_pencil)
{
    // FIXME: No action is needed, we only want a simple label. But sp_toolbox_add_label() always
    //        adds the label at the end of the toolbar, whence this workarund. How to avoid this?
    {
        EgeOutputAction* act = ege_output_action_new(
            tool_is_pencil ?
            "FreehandModeActionPencilTemp" :
            "FreehandModeActionPenTemp",
            _("<b>Mode:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "freehand_mode_action", act );
    }

    /* Freehand mode toggle buttons */
    {
        //gchar const *flatsidedstr = prefs_get_string_attribute( "tools.shapes.star", "isflatsided" );
        //bool isSpiroMode = flatsidedstr ? (strcmp(flatsidedstr, "false") != 0) : true;
        guint spiroMode = prefs_get_int_attribute("tools.freehand", "spiro-spline-mode", 0);
        Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

        {
            GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Bézier"),
                                1, _("Regular Bézier mode"),
                                2, "bezier_mode",
                                -1 );

            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Spiro"),
                                1, _("Spiro splines mode"),
                                2, "spiro_splines_mode",
                                -1 );

            EgeSelectOneAction* act = ege_select_one_action_new(tool_is_pencil ?
                                                                "FreehandModeActionPencil" :
                                                                "FreehandModeActionPen",
                                                                (""), (""), NULL, GTK_TREE_MODEL(model) );
            gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
            g_object_set_data( holder, "freehande_mode_action", act );

            ege_select_one_action_set_appearance( act, "full" );
            ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
            g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
            ege_select_one_action_set_icon_column( act, 2 );
            ege_select_one_action_set_icon_size( act, secondarySize );
            ege_select_one_action_set_tooltip_column( act, 1  );

            ege_select_one_action_set_active( act, spiroMode);
            g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_pc_spiro_spline_mode_changed), holder);
        }
    }
}

static void sp_pen_toolbox_prep(SPDesktop */*desktop*/, GtkActionGroup* mainActions, GObject* holder)
{
    sp_add_spiro_toggle(mainActions, holder, false);
}


static void
sp_pencil_tb_defaults(GtkWidget */*widget*/, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);

    GtkAdjustment *adj;

    // fixme: make settable
    gdouble tolerance = 4;

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "tolerance");
    gtk_adjustment_set_value(adj, tolerance);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_pencil_tb_tolerance_value_changed(GtkAdjustment *adj, GObject *tbl)
{

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }
    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );
    prefs_set_double_attribute("tools.freehand.pencil", 
                               "tolerance", adj->value);
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
    
}



static void
sp_pencil_tb_tolerance_value_changed_external(Inkscape::XML::Node *repr, 
                                              const gchar *key, 
                                              const gchar *oldval, 
                                              const gchar *newval, 
                                              bool is_interactive, 
                                              void * data)
{
    GObject* tbl = G_OBJECT(data);
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }    

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    GtkAdjustment * adj = (GtkAdjustment*)g_object_get_data(tbl,
                                                            "tolerance");
    
    double v = prefs_get_double_attribute("tools.freehand.pencil", 
                                            "tolerance", adj->value);
    gtk_adjustment_set_value(adj, v);
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
    
}

static Inkscape::XML::NodeEventVector pencil_node_events = 
{
    NULL, 
    NULL, 
    sp_pencil_tb_tolerance_value_changed_external,
    NULL, 
    NULL,
};


static void sp_pencil_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    sp_add_spiro_toggle(mainActions, holder, true);

    EgeAdjustmentAction* eact = 0;

    /* Tolerance */
    {

        eact = create_adjustment_action( "PencilToleranceAction",
                 _("Number of pixels allowed in interpolating"), 
                                         _("Tolerance:"), _("Tolerance"),
                                         "tools.freehand.pencil", "tolerance", 
                                         3.0,
                                         GTK_WIDGET(desktop->canvas), NULL, 
                                         holder, TRUE, "altx-pencil",
                                         0.5, 100.0, 0.5, 1.0, 
                                         NULL, NULL, 0,
                                         sp_pencil_tb_tolerance_value_changed,
                                         1, 2);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );

        Inkscape::XML::Node *repr = inkscape_get_repr(INKSCAPE, 
                                                      "tools.freehand.pencil");
        repr->addListener(&pencil_node_events, G_OBJECT(holder));
        g_object_set_data(G_OBJECT(holder), "repr", repr);

    }
    /* Reset */
    {
        InkAction* inky = ink_action_new( "PencilResetAction",
                                          _("Defaults"),
                                          _("Reset pencil parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                          GTK_STOCK_CLEAR,
                                          Inkscape::ICON_SIZE_SMALL_TOOLBAR );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_pencil_tb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }
    
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );

}


//########################
//##       Tweak        ##
//########################

static void sp_tweak_width_value_changed( GtkAdjustment *adj, GObject */*tbl*/ )
{
    prefs_set_double_attribute( "tools.tweak", "width", adj->value * 0.01 );
}

static void sp_tweak_force_value_changed( GtkAdjustment *adj, GObject */*tbl*/ )
{
    prefs_set_double_attribute( "tools.tweak", "force", adj->value * 0.01 );
}

static void sp_tweak_pressure_state_changed( GtkToggleAction *act, gpointer /*data*/ )
{
    prefs_set_int_attribute( "tools.tweak", "usepressure", gtk_toggle_action_get_active( act ) ? 1 : 0);
}

static void sp_tweak_mode_changed( EgeSelectOneAction *act, GObject *tbl )
{
    int mode = ege_select_one_action_get_active( act );
    prefs_set_int_attribute("tools.tweak", "mode", mode);

    GtkAction *doh = GTK_ACTION(g_object_get_data( tbl, "tweak_doh"));
    GtkAction *dos = GTK_ACTION(g_object_get_data( tbl, "tweak_dos"));
    GtkAction *dol = GTK_ACTION(g_object_get_data( tbl, "tweak_dol"));
    GtkAction *doo = GTK_ACTION(g_object_get_data( tbl, "tweak_doo"));
    GtkAction *fid = GTK_ACTION(g_object_get_data( tbl, "tweak_fidelity"));
    GtkAction *dolabel = GTK_ACTION(g_object_get_data( tbl, "tweak_channels_label"));
    if (mode == TWEAK_MODE_COLORPAINT || mode == TWEAK_MODE_COLORJITTER) {
        if (doh) gtk_action_set_sensitive (doh, TRUE);
        if (dos) gtk_action_set_sensitive (dos, TRUE);
        if (dol) gtk_action_set_sensitive (dol, TRUE);
        if (doo) gtk_action_set_sensitive (doo, TRUE);
        if (dolabel) gtk_action_set_sensitive (dolabel, TRUE);
        if (fid) gtk_action_set_sensitive (fid, FALSE);
    } else {
        if (doh) gtk_action_set_sensitive (doh, FALSE);
        if (dos) gtk_action_set_sensitive (dos, FALSE);
        if (dol) gtk_action_set_sensitive (dol, FALSE);
        if (doo) gtk_action_set_sensitive (doo, FALSE);
        if (dolabel) gtk_action_set_sensitive (dolabel, FALSE);
        if (fid) gtk_action_set_sensitive (fid, TRUE);
    }
}

static void sp_tweak_fidelity_value_changed( GtkAdjustment *adj, GObject */*tbl*/ )
{
    prefs_set_double_attribute( "tools.tweak", "fidelity", adj->value * 0.01 );
}

static void tweak_toggle_doh (GtkToggleAction *act, gpointer /*data*/) {
    bool show = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute ("tools.tweak", "doh",  show ? 1 : 0);
}
static void tweak_toggle_dos (GtkToggleAction *act, gpointer /*data*/) {
    bool show = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute ("tools.tweak", "dos",  show ? 1 : 0);
}
static void tweak_toggle_dol (GtkToggleAction *act, gpointer /*data*/) {
    bool show = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute ("tools.tweak", "dol",  show ? 1 : 0);
}
static void tweak_toggle_doo (GtkToggleAction *act, gpointer /*data*/) {
    bool show = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute ("tools.tweak", "doo",  show ? 1 : 0);
}

static void sp_tweak_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

    {
        /* Width */
        gchar const* labels[] = {_("(pinch tweak)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad tweak)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "TweakWidthAction",
                                                              _("Width"), _("Width:"), _("The width of the tweak area (relative to the visible canvas area)"),
                                                              "tools.tweak", "width", 15,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "altx-tweak",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_tweak_width_value_changed,  0.01, 0, 100 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }


    {
        /* Force */
        gchar const* labels[] = {_("(minimum force)"), 0, 0, _("(default)"), 0, 0, 0, _("(maximum force)")};
        gdouble values[] = {1, 5, 10, 20, 30, 50, 70, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "TweakForceAction",
                                                              _("Force"), _("Force:"), _("The force of the tweak action"),
                                                              "tools.tweak", "force", 20,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "tweak-force",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_tweak_force_value_changed,  0.01, 0, 100 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    /* Mode */
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Push mode"),
                            1, _("Push parts of paths in any direction"),
                            2, "tweak_push_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Shrink mode"),
                            1, _("Shrink (inset) parts of paths"),
                            2, "tweak_shrink_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Grow mode"),
                            1, _("Grow (outset) parts of paths"),
                            2, "tweak_grow_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Attract mode"),
                            1, _("Attract parts of paths towards cursor"),
                            2, "tweak_attract_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Repel mode"),
                            1, _("Repel parts of paths from cursor"),
                            2, "tweak_repel_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Roughen mode"),
                            1, _("Roughen parts of paths"),
                            2, "tweak_roughen_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Color paint mode"),
                            1, _("Paint the tool's color upon selected objects"),
                            2, "tweak_colorpaint_mode",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Color jitter mode"),
                            1, _("Jitter the colors of selected objects"),
                            2, "tweak_colorjitter_mode",
                            -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "TweakModeAction", _("Mode"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("Mode:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "mode_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_icon_size( act, secondarySize );
        ege_select_one_action_set_tooltip_column( act, 1  );

        gint mode = prefs_get_int_attribute("tools.tweak", "mode", 0);
        ege_select_one_action_set_active( act, mode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_tweak_mode_changed), holder );

        g_object_set_data( G_OBJECT(holder), "tweak_tool_mode", act);
    }

    guint mode = prefs_get_int_attribute("tools.tweak", "mode", 0);

    {
        EgeOutputAction* act = ege_output_action_new( "TweakChannelsLabel", _("Channels:"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        if (mode != TWEAK_MODE_COLORPAINT && mode != TWEAK_MODE_COLORJITTER)
            gtk_action_set_sensitive (GTK_ACTION(act), FALSE);
        g_object_set_data( holder, "tweak_channels_label", act);
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoH",
                                                      _("Hue"),
                                                      _("In color mode, act on objects' hue"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS:  "H" here stands for hue
        g_object_set( act, "short_label", _("H"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_doh), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.tweak", "doh", 1 ) );
        if (mode != TWEAK_MODE_COLORPAINT && mode != TWEAK_MODE_COLORJITTER)
            gtk_action_set_sensitive (GTK_ACTION(act), FALSE);
        g_object_set_data( holder, "tweak_doh", act);
    }
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoS",
                                                      _("Saturation"),
                                                      _("In color mode, act on objects' saturation"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS: "S" here stands for Saturation
        g_object_set( act, "short_label", _("S"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_dos), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.tweak", "dos", 1 ) );
        if (mode != TWEAK_MODE_COLORPAINT && mode != TWEAK_MODE_COLORJITTER)
            gtk_action_set_sensitive (GTK_ACTION(act), FALSE);
        g_object_set_data( holder, "tweak_dos", act );
    }
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoL",
                                                      _("Lightness"),
                                                      _("In color mode, act on objects' lightness"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS: "L" here stands for Lightness
        g_object_set( act, "short_label", _("L"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_dol), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.tweak", "dol", 1 ) );
        if (mode != TWEAK_MODE_COLORPAINT && mode != TWEAK_MODE_COLORJITTER)
            gtk_action_set_sensitive (GTK_ACTION(act), FALSE);
        g_object_set_data( holder, "tweak_dol", act );
    }
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoO",
                                                      _("Opacity"),
                                                      _("In color mode, act on objects' opacity"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS: "O" here stands for Opacity
        g_object_set( act, "short_label", _("O"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_doo), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.tweak", "doo", 1 ) );
        if (mode != TWEAK_MODE_COLORPAINT && mode != TWEAK_MODE_COLORJITTER)
            gtk_action_set_sensitive (GTK_ACTION(act), FALSE);
        g_object_set_data( holder, "tweak_doo", act );
    }

    {   /* Fidelity */
        gchar const* labels[] = {_("(rough, simplified)"), 0, 0, _("(default)"), 0, 0, _("(fine, but many nodes)")};
        gdouble values[] = {10, 25, 35, 50, 60, 80, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "TweakFidelityAction",
                                                              _("Fidelity"), _("Fidelity:"),
                                                              _("Low fidelity simplifies paths; high fidelity preserves path features but may generate a lot of new nodes"),
                                                              "tools.tweak", "fidelity", 50,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "tweak-fidelity",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_tweak_fidelity_value_changed,  0.01, 0, 100 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        if (mode == TWEAK_MODE_COLORPAINT || mode == TWEAK_MODE_COLORJITTER)
            gtk_action_set_sensitive (GTK_ACTION(eact), FALSE);
        g_object_set_data( holder, "tweak_fidelity", eact );
    }


    /* Use Pressure button */
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakPressureAction",
                                                      _("Pressure"),
                                                      _("Use the pressure of the input device to alter the force of tweak action"),
                                                      "use_pressure",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_tweak_pressure_state_changed), NULL);
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.tweak", "usepressure", 1 ) );
    }

}


//########################
//##     Calligraphy    ##
//########################
static void update_presets_list(GObject *dataKludge ){
    EgeSelectOneAction *sel = static_cast<EgeSelectOneAction *>(g_object_get_data(dataKludge, "profile_selector"));
    if (sel) {
        ege_select_one_action_set_active(sel, 0 );
    }
}

static void sp_ddc_mass_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "mass", adj->value );
    update_presets_list(tbl);
}

static void sp_ddc_wiggle_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "wiggle", adj->value );
    update_presets_list(tbl);
}

static void sp_ddc_angle_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "angle", adj->value );
    update_presets_list(tbl);
}

static void sp_ddc_width_value_changed( GtkAdjustment *adj, GObject *tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "width", adj->value * 0.01 );
    update_presets_list(tbl);
}

static void sp_ddc_velthin_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute("tools.calligraphic", "thinning", adj->value);
    update_presets_list(tbl);
}

static void sp_ddc_flatness_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "flatness", adj->value );
    update_presets_list(tbl);
}

static void sp_ddc_tremor_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "tremor", adj->value );
    update_presets_list(tbl);
}

static void sp_ddc_cap_rounding_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    prefs_set_double_attribute( "tools.calligraphic", "cap_rounding", adj->value );
    update_presets_list(tbl);
}

static void sp_ddc_pressure_state_changed( GtkToggleAction *act, GObject*  tbl )
{
    prefs_set_int_attribute( "tools.calligraphic", "usepressure", gtk_toggle_action_get_active( act ) ? 1 : 0);
    update_presets_list(tbl);
}

static void sp_ddc_trace_background_changed( GtkToggleAction *act, GObject*  tbl )
{
    prefs_set_int_attribute( "tools.calligraphic", "tracebackground", gtk_toggle_action_get_active( act ) ? 1 : 0);
    update_presets_list(tbl);
}

static void sp_ddc_tilt_state_changed( GtkToggleAction *act, GObject*  tbl )
{
    GtkAction * calligraphy_angle = static_cast<GtkAction *> (g_object_get_data(tbl,"angle"));
    prefs_set_int_attribute( "tools.calligraphic", "usetilt", gtk_toggle_action_get_active( act ) ? 1 : 0 );
    update_presets_list(tbl);
    if (calligraphy_angle )
        gtk_action_set_sensitive( calligraphy_angle, !gtk_toggle_action_get_active( act ) );
}


#define PROFILE_FLOAT_SIZE 7
#define PROFILE_INT_SIZE 4
struct ProfileFloatElement {
    char const *name;
    double def;
    double min;
    double max;
};
struct ProfileIntElement {
    char const *name;
    int def;
    int min;
    int max;
};



static ProfileFloatElement f_profile[PROFILE_FLOAT_SIZE] = {
    {"mass",0.02, 0.0, 1.0},
    {"wiggle",0.0, 0.0, 1.0},
    {"angle",30.0, -90.0, 90.0},
    {"thinning",0.1, -1.0, 1.0},
    {"tremor",0.0, 0.0, 1.0},
    {"flatness",0.9, 0.0, 1.0},
    {"cap_rounding",0.0, 0.0, 5.0}
};
static ProfileIntElement i_profile[PROFILE_INT_SIZE] = {
    {"width",15, 1, 100},
    {"usepressure",1,0,1},
    {"tracebackground",0,0,1},
    {"usetilt",1,0,1},
};



static void sp_dcc_save_profile( GtkWidget */*widget*/, GObject *dataKludge ){
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( dataKludge, "desktop" );
    if (! desktop) return;

    Inkscape::UI::Dialogs::CalligraphicProfileDialog::show(desktop);
    if ( ! Inkscape::UI::Dialogs::CalligraphicProfileDialog::applied()) return;
    Glib::ustring profile_name = Inkscape::UI::Dialogs::CalligraphicProfileDialog::getProfileName();

    unsigned int new_index = pref_path_number_of_children("tools.calligraphic.preset") +1;
    gchar *profile_id = g_strdup_printf("dcc%d", new_index);
    gchar *pref_path = create_pref("tools.calligraphic.preset",profile_id);    
    
    for (unsigned i = 0; i < PROFILE_FLOAT_SIZE; ++i) {
        ProfileFloatElement const &pe = f_profile[i];
        double v = prefs_get_double_attribute_limited("tools.calligraphic",pe.name, pe.def, pe.min, pe.max);
        prefs_set_double_attribute(pref_path,pe.name,v);
    }    
    for (unsigned i = 0; i < PROFILE_INT_SIZE; ++i) {
        ProfileIntElement const &pe = i_profile[i];
        int v = prefs_get_int_attribute_limited("tools.calligraphic",pe.name, pe.def,pe.min, pe.max);
        prefs_set_int_attribute(pref_path,pe.name,v);
    }
    prefs_set_string_attribute(pref_path,"name",profile_name.c_str());

    EgeSelectOneAction* selector = static_cast<EgeSelectOneAction *>(g_object_get_data(dataKludge, "profile_selector"));
    GtkListStore* model = GTK_LIST_STORE(ege_select_one_action_get_model(selector));
    GtkTreeIter iter;
    gtk_list_store_append( model, &iter );
    gtk_list_store_set( model, &iter, 0, profile_name.c_str(), 1, new_index, -1 );

    free(profile_id);
    free(pref_path);

    ege_select_one_action_set_active(selector, new_index);
}


static void sp_ddc_change_profile(EgeSelectOneAction* act, GObject *dataKludge) {
    
    gint preset_index = ege_select_one_action_get_active( act ); 
    gchar *profile_name = get_pref_nth_child("tools.calligraphic.preset", preset_index);
       
    if ( profile_name) {        
        g_object_set_data(dataKludge, "profile_selector",NULL); //temporary hides the selector so no one will updadte it
        for (unsigned i = 0; i < PROFILE_FLOAT_SIZE; ++i) {
            ProfileFloatElement const &pe = f_profile[i];
            double v = prefs_get_double_attribute_limited(profile_name, pe.name, pe.def, pe.min, pe.max);
            GtkAdjustment* adj = static_cast<GtkAdjustment *>(g_object_get_data(dataKludge, pe.name));
            if ( adj ) {
                gtk_adjustment_set_value(adj, v);
            }
        } 
        for (unsigned i = 0; i < PROFILE_INT_SIZE; ++i) {
            ProfileIntElement const &pe = i_profile[i];
            int v = prefs_get_int_attribute_limited(profile_name, pe.name, pe.def, pe.min, pe.max);
            GtkToggleAction* toggle = static_cast<GtkToggleAction *>(g_object_get_data(dataKludge, pe.name));
            if ( toggle ) {
                gtk_toggle_action_set_active(toggle, v);
            } else printf("No toggle");
        }
        free(profile_name);
        g_object_set_data(dataKludge, "profile_selector",act); //restore selector visibility
    }

}


static void sp_calligraphy_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    {
        EgeAdjustmentAction* calligraphy_angle = 0;

        {
        /* Width */
        gchar const* labels[] = {_("(hairline)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad stroke)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "CalligraphyWidthAction",
                                                              _("Pen Width"), _("Width:"),
                                                              _("The width of the calligraphic pen (relative to the visible canvas area)"),
                                                              "tools.calligraphic", "width", 15,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "altx-calligraphy",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_width_value_changed,  0.01, 0, 100 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Thinning */
            gchar const* labels[] = {_("(speed blows up stroke)"), 0, 0, _("(slight widening)"), _("(constant width)"), _("(slight thinning, default)"), 0, 0, _("(speed deflates stroke)")};
            gdouble values[] = {-1, -0.4, -0.2, -0.1, 0, 0.1, 0.2, 0.4, 1};
        EgeAdjustmentAction* eact = create_adjustment_action( "ThinningAction",
                                                              _("Stroke Thinning"), _("Thinning:"),
                                                              _("How much velocity thins the stroke (> 0 makes fast strokes thinner, < 0 makes them broader, 0 makes width independent of velocity)"),
                                                              "tools.calligraphic", "thinning", 0.1,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                                              -1.0, 1.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_velthin_value_changed, 0.01, 2);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Angle */
        gchar const* labels[] = {_("(left edge up)"), 0, 0, _("(horizontal)"), _("(default)"), 0, _("(right edge up)")};
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        EgeAdjustmentAction* eact = create_adjustment_action( "AngleAction",
                                                              _("Pen Angle"), _("Angle:"),
                                                              _("The angle of the pen's nib (in degrees; 0 = horizontal; has no effect if fixation = 0)"),
                                                              "tools.calligraphic", "angle", 30,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "calligraphy-angle",
                                                              -90.0, 90.0, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_angle_value_changed, 1, 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        g_object_set_data( holder, "angle", eact );
        calligraphy_angle = eact;
        }

        {
        /* Fixation */
            gchar const* labels[] = {_("(perpendicular to stroke, \"brush\")"), 0, 0, 0, _("(almost fixed, default)"), _("(fixed by Angle, \"pen\")")};
        gdouble values[] = {0, 0.2, 0.4, 0.6, 0.9, 1.0};
        EgeAdjustmentAction* eact = create_adjustment_action( "FixationAction",
                                                              _("Fixation"), _("Fixation:"),
                                                              _("Angle behavior (0 = nib always perpendicular to stroke direction, 1 = fixed angle)"),
                                                              "tools.calligraphic", "flatness", 0.9,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                                              0.0, 1.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_flatness_value_changed, 0.01, 2 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Cap Rounding */
            gchar const* labels[] = {_("(blunt caps, default)"), _("(slightly bulging)"), 0, 0, _("(approximately round)"), _("(long protruding caps)")};
        gdouble values[] = {0, 0.3, 0.5, 1.0, 1.4, 5.0};
        // TRANSLATORS: "cap" means "end" (both start and finish) here
        EgeAdjustmentAction* eact = create_adjustment_action( "CapRoundingAction",
                                                              _("Cap rounding"), _("Caps:"),
                                                              _("Increase to make caps at the ends of strokes protrude more (0 = no caps, 1 = round caps)"),
                                                              "tools.calligraphic", "cap_rounding", 0.0,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                                              0.0, 5.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_cap_rounding_value_changed, 0.01, 2 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Tremor */
            gchar const* labels[] = {_("(smooth line)"), _("(slight tremor)"), _("(noticeable tremor)"), 0, 0, _("(maximum tremor)")};
        gdouble values[] = {0, 0.1, 0.2, 0.4, 0.6, 1.0};
        EgeAdjustmentAction* eact = create_adjustment_action( "TremorAction",
                                                              _("Stroke Tremor"), _("Tremor:"),
                                                              _("Increase to make strokes rugged and trembling"),
                                                              "tools.calligraphic", "tremor", 0.0,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                                              0.0, 1.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_tremor_value_changed, 0.01, 2 );

        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Wiggle */
        gchar const* labels[] = {_("(no wiggle)"), _("(slight deviation)"), 0, 0, _("(wild waves and curls)")};
        gdouble values[] = {0, 0.2, 0.4, 0.6, 1.0};
        EgeAdjustmentAction* eact = create_adjustment_action( "WiggleAction",
                                                              _("Pen Wiggle"), _("Wiggle:"),
                                                              _("Increase to make the pen waver and wiggle"),
                                                              "tools.calligraphic", "wiggle", 0.0,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                                              0.0, 1.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_wiggle_value_changed, 0.01, 2 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Mass */
            gchar const* labels[] = {_("(no inertia)"), _("(slight smoothing, default)"), _("(noticeable lagging)"), 0, 0, _("(maximum inertia)")};
        gdouble values[] = {0.0, 0.02, 0.1, 0.2, 0.5, 1.0};
        EgeAdjustmentAction* eact = create_adjustment_action( "MassAction",
                                                              _("Pen Mass"), _("Mass:"),
                                                              _("Increase to make the pen drag behind, as if slowed by inertia"),
                                                              "tools.calligraphic", "mass", 0.02,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                                              0.0, 1.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_mass_value_changed, 0.01, 2 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }


        /* Trace Background button */
        {
            InkToggleAction* act = ink_toggle_action_new( "TraceAction",
                                                          _("Trace Background"),
                                                          _("Trace the lightness of the background by the width of the pen (white - minimum width, black - maximum width)"),
                                                          "trace_background",
                                                          Inkscape::ICON_SIZE_DECORATION );
            gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
            g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_ddc_trace_background_changed), holder);
            gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.calligraphic", "tracebackground", 0 ) );
            g_object_set_data( holder, "tracebackground", act );
        }

        /* Use Pressure button */
        {
            InkToggleAction* act = ink_toggle_action_new( "PressureAction",
                                                          _("Pressure"),
                                                          _("Use the pressure of the input device to alter the width of the pen"),
                                                          "use_pressure",
                                                          Inkscape::ICON_SIZE_DECORATION );
            gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
            g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_ddc_pressure_state_changed), holder);
            gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.calligraphic", "usepressure", 1 ) );
            g_object_set_data( holder, "usepressure", act );
        }

        /* Use Tilt button */
        {
            InkToggleAction* act = ink_toggle_action_new( "TiltAction",
                                                          _("Tilt"),
                                                          _("Use the tilt of the input device to alter the angle of the pen's nib"),
                                                          "use_tilt",
                                                          Inkscape::ICON_SIZE_DECORATION );
            gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
            g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_ddc_tilt_state_changed), holder );
            gtk_action_set_sensitive( GTK_ACTION(calligraphy_angle), !prefs_get_int_attribute( "tools.calligraphic", "usetilt", 1 ) );
            gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.calligraphic", "usetilt", 1 ) );
            g_object_set_data( holder, "usetilt", act );
        }

        /*calligraphic profile */
        {
            GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );            
            gchar *pref_path;


            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, _("No preset"), 1, 0, -1 );

            //TODO: switch back to prefs API
            Inkscape::XML::Node *repr = inkscape_get_repr(INKSCAPE, "tools.calligraphic.preset" );
            Inkscape::XML::Node *child_repr = sp_repr_children(repr);
            int ii=1;
            while (child_repr) {
                GtkTreeIter iter;
                char *preset_name = (char *) child_repr->attribute("name");
                gtk_list_store_append( model, &iter );                
                gtk_list_store_set( model, &iter, 0, preset_name, 1, ++ii, -1 );                
                child_repr = sp_repr_next(child_repr); 
            }

            pref_path = NULL;
            EgeSelectOneAction* act1 = ege_select_one_action_new( "SetProfileAction", "" , (_("Change calligraphic profile")), NULL, GTK_TREE_MODEL(model) );
            ege_select_one_action_set_appearance( act1, "compact" );
            g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(sp_ddc_change_profile), holder );
            gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
            g_object_set_data( holder, "profile_selector", act1 );
        
        }

        /*Save or delete calligraphic profile */
        {
            GtkAction* act = gtk_action_new( "SaveDeleteProfileAction",
                                             _("Defaults"),
                                             _("Save current settings as new profile"),
                                             GTK_STOCK_SAVE );
            g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(sp_dcc_save_profile), holder );


            gtk_action_group_add_action( mainActions, act );
            gtk_action_set_sensitive( act, TRUE );
            g_object_set_data( holder, "profile_save_delete", act );
        }
    }
}


//########################
//##    Circle / Arc    ##
//########################

static void sp_arctb_sensitivize( GObject *tbl, double v1, double v2 )
{
    GtkAction *ocb = GTK_ACTION( g_object_get_data( tbl, "open_action" ) );
    GtkAction *make_whole = GTK_ACTION( g_object_get_data( tbl, "make_whole" ) );

    if (v1 == 0 && v2 == 0) {
        if (g_object_get_data( tbl, "single" )) { // only for a single selected ellipse (for now)
            gtk_action_set_sensitive( ocb, FALSE );
            gtk_action_set_sensitive( make_whole, FALSE );
        }
    } else {
        gtk_action_set_sensitive( ocb, TRUE );
        gtk_action_set_sensitive( make_whole, TRUE );
    }
}

static void
sp_arctb_startend_value_changed(GtkAdjustment *adj, GObject *tbl, gchar const *value_name, gchar const *other_name)
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.arc", value_name, (adj->value * M_PI)/ 180);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);

    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        SPItem *item = SP_ITEM(items->data);

        if (SP_IS_ARC(item) && SP_IS_GENERICELLIPSE(item)) {

            SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
            SPArc *arc = SP_ARC(item);

            if (!strcmp(value_name, "start"))
                ge->start = (adj->value * M_PI)/ 180;
            else
                ge->end = (adj->value * M_PI)/ 180;

            sp_genericellipse_normalize(ge);
            ((SPObject *)arc)->updateRepr();
            ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

            modmade = true;
        }
    }

    g_free(namespaced_name);

    GtkAdjustment *other = GTK_ADJUSTMENT( g_object_get_data( tbl, other_name ) );

    sp_arctb_sensitivize( tbl, adj->value, other->value );

    if (modmade) {
        sp_document_maybe_done(sp_desktop_document(desktop), value_name, SP_VERB_CONTEXT_ARC,
                                   _("Arc: Change start/end"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}


static void sp_arctb_start_value_changed(GtkAdjustment *adj,  GObject *tbl)
{
    sp_arctb_startend_value_changed(adj,  tbl, "start", "end");
}

static void sp_arctb_end_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_arctb_startend_value_changed(adj,  tbl, "end", "start");
}


static void sp_erasertb_mode_changed( EgeSelectOneAction *act, GObject *tbl )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );
    gint eraserMode = (ege_select_one_action_get_active( act ) != 0) ? 1 : 0;
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_int_attribute( "tools.eraser", "mode", eraserMode );
    }

    // only take action if run by the attr_changed listener
    if (!g_object_get_data( tbl, "freeze" )) {
        // in turn, prevent listener from responding
        g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

        if ( eraserMode != 0 ) {
        } else {
        }
        // TODO finish implementation

        g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
    }
}

static void sp_arctb_open_state_changed( EgeSelectOneAction *act, GObject *tbl )
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        if ( ege_select_one_action_get_active( act ) != 0 ) {
            prefs_set_string_attribute("tools.shapes.arc", "open", "true");
        } else {
            prefs_set_string_attribute("tools.shapes.arc", "open", NULL);
        }
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    if ( ege_select_one_action_get_active(act) != 0 ) {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC((SPItem *) items->data)) {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("sodipodi:open", "true");
                SP_OBJECT((SPItem *) items->data)->updateRepr();
                modmade = true;
            }
        }
    } else {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC((SPItem *) items->data))    {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("sodipodi:open", NULL);
                SP_OBJECT((SPItem *) items->data)->updateRepr();
                modmade = true;
            }
        }
    }

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_ARC,
                                   _("Arc: Change open/closed"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_arctb_defaults(GtkWidget *, GObject *obj)
{
    GtkAdjustment *adj;
    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "start") );
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "end") );
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus( GTK_OBJECT(obj) );
}

static void arc_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const */*name*/,
                                      gchar const */*old_value*/, gchar const */*new_value*/,
                                      bool /*is_interactive*/, gpointer data)
{
    GObject *tbl = G_OBJECT(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    gdouble start = sp_repr_get_double_attribute(repr, "sodipodi:start", 0.0);
    gdouble end = sp_repr_get_double_attribute(repr, "sodipodi:end", 0.0);

    GtkAdjustment *adj1,*adj2;
    adj1 = GTK_ADJUSTMENT( g_object_get_data( tbl, "start" ) );
    gtk_adjustment_set_value(adj1, mod360((start * 180)/M_PI));
    adj2 = GTK_ADJUSTMENT( g_object_get_data( tbl, "end" ) );
    gtk_adjustment_set_value(adj2, mod360((end * 180)/M_PI));

    sp_arctb_sensitivize( tbl, adj1->value, adj2->value );

    char const *openstr = NULL;
    openstr = repr->attribute("sodipodi:open");
    EgeSelectOneAction *ocb = EGE_SELECT_ONE_ACTION( g_object_get_data( tbl, "open_action" ) );

    if (openstr) {
        ege_select_one_action_set_active( ocb, 1 );
    } else {
        ege_select_one_action_set_active( ocb, 0 );
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static Inkscape::XML::NodeEventVector arc_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    arc_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


static void sp_arc_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;

    purge_repr_listener( tbl, tbl );

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_ARC((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    g_object_set_data( tbl, "single", GINT_TO_POINTER(FALSE) );
    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );
    } else if (n_selected == 1) {
        g_object_set_data( tbl, "single", GINT_TO_POINTER(TRUE) );
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &arc_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &arc_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
        sp_arctb_sensitivize( tbl, 1, 0 );
    }
}


static void sp_arc_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);


    {
        EgeOutputAction* act = ege_output_action_new( "ArcStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    /* Start */
    {
        eact = create_adjustment_action( "ArcStartAction",
                                         _("Start"), _("Start:"),
                                         _("The angle (in degrees) from the horizontal to the arc's start point"),
                                         "tools.shapes.arc", "start", 0.0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, TRUE, "altx-arc",
                                         -360.0, 360.0, 1.0, 10.0,
                                         0, 0, 0,
                                         sp_arctb_start_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* End */
    {
        eact = create_adjustment_action( "ArcEndAction",
                                         _("End"), _("End:"),
                                         _("The angle (in degrees) from the horizontal to the arc's end point"),
                                         "tools.shapes.arc", "end", 0.0,
                                         GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         0, 0, 0,
                                         sp_arctb_end_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Segments / Pie checkbox */
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Closed arc"),
                            1, _("Switch to segment (closed shape with two radii)"),
                            2, "circle_closed_arc",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Open Arc"),
                            1, _("Switch to arc (unclosed shape)"),
                            2, "circle_open_arc",
                            -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "ArcOpenAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "open_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_icon_size( act, secondarySize );
        ege_select_one_action_set_tooltip_column( act, 1  );

        gchar const *openstr = prefs_get_string_attribute("tools.shapes.arc", "open");
        bool isClosed = (!openstr || (openstr && !strcmp(openstr, "false")));
        ege_select_one_action_set_active( act, isClosed ? 0 : 1 );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_arctb_open_state_changed), holder );
    }

    /* Make Whole */
    {
        InkAction* inky = ink_action_new( "ArcResetAction",
                                          _("Make whole"),
                                          _("Make the shape a whole ellipse, not arc or segment"),
                                          "reset_circle",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_arctb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), TRUE );
        g_object_set_data( holder, "make_whole", inky );
    }

    g_object_set_data( G_OBJECT(holder), "single", GINT_TO_POINTER(TRUE) );
    // sensitivize make whole and open checkbox
    {
        GtkAdjustment *adj1 = GTK_ADJUSTMENT( g_object_get_data( holder, "start" ) );
        GtkAdjustment *adj2 = GTK_ADJUSTMENT( g_object_get_data( holder, "end" ) );
        sp_arctb_sensitivize( holder, adj1->value, adj2->value );
    }


    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_arc_toolbox_selection_changed), (GObject *)holder))
        );
    g_signal_connect( holder, "destroy", G_CALLBACK(delete_connection), connection );
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
}




// toggle button callbacks and updaters

//########################
//##      Dropper       ##
//########################

static void toggle_dropper_pick_alpha( GtkToggleAction* act, gpointer tbl ) {
    prefs_set_int_attribute( "tools.dropper", "pick", gtk_toggle_action_get_active( act ) );
    GtkAction* set_action = GTK_ACTION( g_object_get_data(G_OBJECT(tbl), "set_action") );
    if ( set_action ) {
        if ( gtk_toggle_action_get_active( act ) ) {
            gtk_action_set_sensitive( set_action, TRUE );
        } else {
            gtk_action_set_sensitive( set_action, FALSE );
        }
    }

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void toggle_dropper_set_alpha( GtkToggleAction* act, gpointer tbl ) {
    prefs_set_int_attribute( "tools.dropper", "setalpha", gtk_toggle_action_get_active( act ) ? 1 : 0 );
    spinbutton_defocus(GTK_OBJECT(tbl));
}


/**
 * Dropper auxiliary toolbar construction and setup.
 *
 * TODO: Would like to add swatch of current color.
 * TODO: Add queue of last 5 or so colors selected with new swatches so that
 *       can drag and drop places. Will provide a nice mixing palette.
 */
static void sp_dropper_toolbox_prep(SPDesktop */*desktop*/, GtkActionGroup* mainActions, GObject* holder)
{
    gint pickAlpha = prefs_get_int_attribute( "tools.dropper", "pick", 1 );

    {
        EgeOutputAction* act = ege_output_action_new( "DropperOpacityAction", _("Opacity:"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "DropperPickAlphaAction",
                                                      _("Pick opacity"),
                                                      _("Pick both the color and the alpha (transparency) under cursor; otherwise, pick only the visible color premultiplied by alpha"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        g_object_set( act, "short_label", _("Pick"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "pick_action", act );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), pickAlpha );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_dropper_pick_alpha), holder );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "DropperSetAlphaAction",
                                                      _("Assign opacity"),
                                                      _("If alpha was picked, assign it to selection as fill or stroke transparency"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        g_object_set( act, "short_label", _("Assign"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "set_action", act );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs_get_int_attribute( "tools.dropper", "setalpha", 1 ) );
        // make sure it's disabled if we're not picking alpha
        gtk_action_set_sensitive( GTK_ACTION(act), pickAlpha );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_dropper_set_alpha), holder );
    }
}



static void sp_eraser_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    {
        /* Width */
        gchar const* labels[] = {_("(hairline)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad stroke)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "EraserWidthAction",
                                                              _("Pen Width"), _("Width:"),
                                                              _("The width of the eraser pen (relative to the visible canvas area)"),
                                                              "tools.eraser", "width", 15,
                                                              GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "altx-eraser",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_width_value_changed,  0.01, 0, 100 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Delete"),
                            1, _("Delete objects touched by the eraser"),
                            2, "delete_object",
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Cut"),
                            1, _("Cut out from objects"),
                            2, "difference",
                            -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "EraserModeAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "eraser_mode_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_tooltip_column( act, 1  );

        gint eraserMode = (prefs_get_int_attribute("tools.eraser", "mode", 0) != 0) ? 1 : 0;
        ege_select_one_action_set_active( act, eraserMode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_erasertb_mode_changed), holder );
    }
    
}

//########################
//##    Text Toolbox    ##
//########################
/*
static void
sp_text_letter_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for letter sizing spinbutton
}

static void
sp_text_line_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for line height spinbutton
}

static void
sp_text_horiz_kern_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for horizontal kerning spinbutton
}

static void
sp_text_vert_kern_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for vertical kerning spinbutton
}

static void
sp_text_letter_rotation_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for letter rotation spinbutton
}*/

namespace {

bool popdown_visible = false;
bool popdown_hasfocus = false;

void
sp_text_toolbox_selection_changed (Inkscape::Selection */*selection*/, GObject *tbl)
{
    SPStyle *query =
        sp_style_new (SP_ACTIVE_DOCUMENT);

    int result_fontspec =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);

    int result_family =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTFAMILY);

    int result_style =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTSTYLE);

    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

    gtk_widget_hide (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_family == QUERY_STYLE_NOTHING || result_style == QUERY_STYLE_NOTHING || result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::XML::Node *repr = inkscape_get_repr (INKSCAPE, "tools.text");

        if (repr)
        {
            sp_style_read_from_repr (query, repr);
        }
        else
        {
            return;
        }
    }

    if (query->text)
    {
        if (result_family == QUERY_STYLE_MULTIPLE_DIFFERENT) {
            GtkWidget *entry = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "family-entry"));
            gtk_entry_set_text (GTK_ENTRY (entry), "");

        } else if (query->text->font_specification.value || query->text->font_family.value) {

            GtkWidget *entry = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "family-entry"));

            // Get the font that corresponds
            Glib::ustring familyName;

            font_instance * font = font_factory::Default()->FaceFromStyle(query);
            if (font) {
                familyName = font_factory::Default()->GetUIFamilyString(font->descr);
                font->Unref();
                font = NULL;
            }

            gtk_entry_set_text (GTK_ENTRY (entry), familyName.c_str());

            Gtk::TreePath path;
            try {
                path = Inkscape::FontLister::get_instance()->get_row_for_font (familyName);
            } catch (...) {
                g_warning("Family name %s does not have an entry in the font lister.", familyName.c_str());
                return;
            }

            GtkTreeSelection *tselection = GTK_TREE_SELECTION (g_object_get_data (G_OBJECT(tbl), "family-tree-selection"));
            GtkTreeView *treeview = GTK_TREE_VIEW (g_object_get_data (G_OBJECT(tbl), "family-tree-view"));

            g_object_set_data (G_OBJECT (tselection), "block", gpointer(1));

            gtk_tree_selection_select_path (tselection, path.gobj());
            gtk_tree_view_scroll_to_cell (treeview, path.gobj(), NULL, TRUE, 0.5, 0.0);

            g_object_set_data (G_OBJECT (tselection), "block", gpointer(0));
        }

        //Size
        GtkWidget *cbox = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "combo-box-size"));
        char *str = g_strdup_printf ("%.5g", query->font_size.computed);
        g_object_set_data (tbl, "size-block", gpointer(1));
        gtk_entry_set_text (GTK_ENTRY(GTK_BIN (cbox)->child), str);
        g_object_set_data (tbl, "size-block", gpointer(0));
        free (str);

        //Anchor
        if (query->text_align.computed == SP_CSS_TEXT_ALIGN_JUSTIFY)
        {
            GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-fill"));
            g_object_set_data (G_OBJECT (button), "block", gpointer(1));
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
            g_object_set_data (G_OBJECT (button), "block", gpointer(0));
        }
        else
        {
            if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_START)
            {
                GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-start"));
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
            else if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_MIDDLE)
            {
                GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-middle"));
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
            else if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_END)
            {
                GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-end"));
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
        }

        //Style
        {
            GtkToggleButton *button = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (tbl), "style-bold"));

            gboolean active = gtk_toggle_button_get_active (button);
            gboolean check  = (query->font_weight.computed >= SP_CSS_FONT_WEIGHT_700);

            if (active != check)
            {
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), check);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
        }

        {
            GtkToggleButton *button = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (tbl), "style-italic"));

            gboolean active = gtk_toggle_button_get_active (button);
            gboolean check  = (query->font_style.computed != SP_CSS_FONT_STYLE_NORMAL);

            if (active != check)
            {
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), check);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
        }

        //Orientation
        //locking both buttons, changing one affect all group (both)
        GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "orientation-horizontal"));
        g_object_set_data (G_OBJECT (button), "block", gpointer(1));

        GtkWidget *button1 = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "orientation-vertical"));
        g_object_set_data (G_OBJECT (button1), "block", gpointer(1));

        if (query->writing_mode.computed == SP_CSS_WRITING_MODE_LR_TB)
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
        }
        else
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button1), TRUE);
        }
        g_object_set_data (G_OBJECT (button), "block", gpointer(0));
        g_object_set_data (G_OBJECT (button1), "block", gpointer(0));
    }

    sp_style_unref(query);
}

void
sp_text_toolbox_selection_modified (Inkscape::Selection *selection, guint /*flags*/, GObject *tbl)
{
    sp_text_toolbox_selection_changed (selection, tbl);
}

void
sp_text_toolbox_subselection_changed (gpointer /*dragger*/, GObject *tbl)
{
    sp_text_toolbox_selection_changed (NULL, tbl);
}

void
sp_text_toolbox_family_changed (GtkTreeSelection    *selection,
                                GObject             *tbl)
{
    SPDesktop    *desktop = SP_ACTIVE_DESKTOP;
    GtkTreeModel *model = 0;
    GtkWidget    *entry = GTK_WIDGET (g_object_get_data (tbl, "family-entry"));
    GtkTreeIter   iter;
    char         *family = 0;

    gdk_pointer_ungrab (GDK_CURRENT_TIME);
    gdk_keyboard_ungrab (GDK_CURRENT_TIME);

    if ( !gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
        return;
    }

    gtk_tree_model_get (model, &iter, 0, &family, -1);

    if (g_object_get_data (G_OBJECT (selection), "block"))
    {
        gtk_entry_set_text (GTK_ENTRY (entry), family);
        return;
    }

    gtk_entry_set_text (GTK_ENTRY (entry), family);

    SPStyle *query =
        sp_style_new (SP_ACTIVE_DOCUMENT);

    int result_fontspec =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);

    font_instance * fontFromStyle = font_factory::Default()->FaceFromStyle(query);

    SPCSSAttr *css = sp_repr_css_attr_new ();


    // First try to get the font spec from the stored value
    Glib::ustring fontSpec = query->text->font_specification.set ?  query->text->font_specification.value : "";

    if (fontSpec.empty()) {
        // Construct a new font specification if it does not yet exist
        font_instance * fontFromStyle = font_factory::Default()->FaceFromStyle(query);
        fontSpec = font_factory::Default()->ConstructFontSpecification(fontFromStyle);
        fontFromStyle->Unref();
    }

    if (!fontSpec.empty()) {
        Glib::ustring newFontSpec = font_factory::Default()->ReplaceFontSpecificationFamily(fontSpec, family);
        if (!newFontSpec.empty() && fontSpec != newFontSpec) {
            font_instance *font = font_factory::Default()->FaceFromFontSpecification(newFontSpec.c_str());
            if (font) {
                sp_repr_css_set_property (css, "-inkscape-font-specification", newFontSpec.c_str());

                // Set all the these just in case they were altered when finding the best
                // match for the new family and old style...

                gchar c[256];

                font->Family(c, 256);
                sp_repr_css_set_property (css, "font-family", c);

                font->Attribute( "weight", c, 256);
                sp_repr_css_set_property (css, "font-weight", c);

                font->Attribute("style", c, 256);
                sp_repr_css_set_property (css, "font-style", c);

                font->Attribute("stretch", c, 256);
                sp_repr_css_set_property (css, "font-stretch", c);

                font->Attribute("variant", c, 256);
                sp_repr_css_set_property (css, "font-variant", c);

                font->Unref();
            }
        }
    }

    // If querying returned nothing, set the default style of the tool (for new texts)
    if (result_fontspec == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
        sp_text_edit_dialog_default_set_insensitive (); //FIXME: Replace trough a verb
    }
    else
    {
        sp_desktop_set_style (desktop, css, true, true);
    }

    sp_style_unref(query);

    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                                   _("Text: Change font family"));
    sp_repr_css_attr_unref (css);
    free (family);
    gtk_widget_hide (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_family_entry_activate (GtkEntry     *entry,
                                       GObject      *tbl)
{
    const char *family = gtk_entry_get_text (entry);

    try {
        Gtk::TreePath path = Inkscape::FontLister::get_instance()->get_row_for_font (family);
        GtkTreeSelection *selection = GTK_TREE_SELECTION (g_object_get_data (G_OBJECT(tbl), "family-tree-selection"));
        GtkTreeView *treeview = GTK_TREE_VIEW (g_object_get_data (G_OBJECT(tbl), "family-tree-view"));
        gtk_tree_selection_select_path (selection, path.gobj());
        gtk_tree_view_scroll_to_cell (treeview, path.gobj(), NULL, TRUE, 0.5, 0.0);
        gtk_widget_hide (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));
    } catch (...) {
        if (family && strlen (family))
        {
            gtk_widget_show_all (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));
        }
    }
}

void
sp_text_toolbox_anchoring_toggled (GtkRadioButton   *button,
                                   gpointer          data)
{
    if (g_object_get_data (G_OBJECT (button), "block")) return;
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) return;
    int prop = GPOINTER_TO_INT(data);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPCSSAttr *css = sp_repr_css_attr_new ();

    switch (prop)
    {
        case 0:
        {
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "start");
            break;
        }
        case 1:
        {
            sp_repr_css_set_property (css, "text-anchor", "middle");
            sp_repr_css_set_property (css, "text-align", "center");
            break;
        }

        case 2:
        {
            sp_repr_css_set_property (css, "text-anchor", "end");
            sp_repr_css_set_property (css, "text-align", "end");
            break;
        }

        case 3:
        {
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "justify");
            break;
        }
    }

    SPStyle *query =
        sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_style_unref(query);

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                                   _("Text: Change alignment"));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_style_toggled (GtkToggleButton  *button,
                               gpointer          data)
{
    if (g_object_get_data (G_OBJECT (button), "block")) return;

    SPDesktop   *desktop    = SP_ACTIVE_DESKTOP;
    SPCSSAttr   *css        = sp_repr_css_attr_new ();
    int          prop       = GPOINTER_TO_INT(data);
    bool         active     = gtk_toggle_button_get_active (button);

    SPStyle *query =
        sp_style_new (SP_ACTIVE_DOCUMENT);

    int result_fontspec =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);

    int result_family =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTFAMILY);

    int result_style =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTSTYLE);

    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

    Glib::ustring fontSpec = query->text->font_specification.set ?  query->text->font_specification.value : "";
    Glib::ustring newFontSpec = "";

    if (fontSpec.empty()) {
        // Construct a new font specification if it does not yet exist
        font_instance * fontFromStyle = font_factory::Default()->FaceFromStyle(query);
        fontSpec = font_factory::Default()->ConstructFontSpecification(fontFromStyle);
        fontFromStyle->Unref();
    }

    switch (prop)
    {
        case 0:
        {
            if (!fontSpec.empty()) {
                newFontSpec = font_factory::Default()->FontSpecificationSetBold(fontSpec, active);
            }
            if (fontSpec != newFontSpec) {
                // Don't even set the bold if the font didn't exist on the system
                sp_repr_css_set_property (css, "font-weight", active ? "bold" : "normal" );
            }
            break;
        }

        case 1:
        {
            if (!fontSpec.empty()) {
                newFontSpec = font_factory::Default()->FontSpecificationSetItalic(fontSpec, active);
            }
            if (fontSpec != newFontSpec) {
                // Don't even set the italic if the font didn't exist on the system
                sp_repr_css_set_property (css, "font-style", active ? "italic" : "normal");
            }
            break;
        }
    }

    if (!newFontSpec.empty()) {
        sp_repr_css_set_property (css, "-inkscape-font-specification", newFontSpec.c_str());
    }

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_fontspec == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_style_unref(query);

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                                   _("Text: Change font style"));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_orientation_toggled (GtkRadioButton  *button,
                                     gpointer         data)
{
    if (g_object_get_data (G_OBJECT (button), "block")) {
        g_object_set_data (G_OBJECT (button), "block", gpointer(0));
        return;
    }

    SPDesktop   *desktop    = SP_ACTIVE_DESKTOP;
    SPCSSAttr   *css        = sp_repr_css_attr_new ();
    int          prop       = GPOINTER_TO_INT(data);

    switch (prop)
    {
        case 0:
        {
            sp_repr_css_set_property (css, "writing-mode", "lr");
            break;
        }

        case 1:
        {
            sp_repr_css_set_property (css, "writing-mode", "tb");
            break;
        }
    }

    SPStyle *query =
        sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                                   _("Text: Change orientation"));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

gboolean
sp_text_toolbox_family_keypress (GtkWidget */*w*/, GdkEventKey *event, GObject *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    switch (get_group0_keyval (event)) {
        case GDK_Escape: // defocus
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            sp_text_toolbox_selection_changed (NULL, tbl); // update
            return TRUE; // I consumed the event
            break;
    }
    return FALSE;
}

gboolean
sp_text_toolbox_family_list_keypress (GtkWidget *w, GdkEventKey *event, GObject */*tbl*/)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    switch (get_group0_keyval (event)) {
        case GDK_KP_Enter:
        case GDK_Return:
        case GDK_Escape: // defocus
            gtk_widget_hide (w);
            popdown_visible = false;
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            return TRUE; // I consumed the event
            break;
        case GDK_w:
        case GDK_W:
            if (event->state & GDK_CONTROL_MASK) {
                gtk_widget_hide (w);
                popdown_visible = false;
                gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
                return TRUE; // I consumed the event
            }
            break;
    }
    return FALSE;
}


void
sp_text_toolbox_size_changed  (GtkComboBox *cbox,
                               GObject     *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (g_object_get_data (tbl, "size-block")) return;

    // If this is not from selecting a size in the list (in which case get_active will give the
    // index of the selected item, otherwise -1) and not from user pressing Enter/Return, do not
    // process this event. This fixes GTK's stupid insistence on sending an activate change every
    // time any character gets typed or deleted, which made this control nearly unusable in 0.45.
    if (gtk_combo_box_get_active (cbox) < 0 && !g_object_get_data (tbl, "enter-pressed"))
        return;

    gchar *endptr;
    gdouble value = -1;
    char *text = gtk_combo_box_get_active_text (cbox);
    if (text) {
        value = g_strtod (text, &endptr);
        if (endptr == text) // conversion failed, non-numeric input
            value = -1;
        free (text);
    }
    if (value <= 0) {
        return; // could not parse value
    }

    SPCSSAttr *css = sp_repr_css_attr_new ();
    Inkscape::CSSOStringStream osfs;
    osfs << value;
    sp_repr_css_set_property (css, "font-size", osfs.str().c_str());

    SPStyle *query =
        sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_style_unref(query);

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "ttb:size", SP_VERB_NONE,
                                   _("Text: Change font size"));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

gboolean
sp_text_toolbox_size_focusout (GtkWidget */*w*/, GdkEventFocus */*event*/, GObject *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    if (!g_object_get_data (tbl, "esc-pressed")) {
        g_object_set_data (tbl, "enter-pressed", gpointer(1));
        GtkComboBox *cbox = GTK_COMBO_BOX(g_object_get_data (G_OBJECT (tbl), "combo-box-size"));
        sp_text_toolbox_size_changed (cbox, tbl);
        g_object_set_data (tbl, "enter-pressed", gpointer(0));
    }
    return FALSE; // I consumed the event
}


gboolean
sp_text_toolbox_size_keypress (GtkWidget */*w*/, GdkEventKey *event, GObject *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    switch (get_group0_keyval (event)) {
        case GDK_Escape: // defocus
            g_object_set_data (tbl, "esc-pressed", gpointer(1));
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            g_object_set_data (tbl, "esc-pressed", gpointer(0));
            return TRUE; // I consumed the event
            break;
        case GDK_Return: // defocus
        case GDK_KP_Enter:
            g_object_set_data (tbl, "enter-pressed", gpointer(1));
            GtkComboBox *cbox = GTK_COMBO_BOX(g_object_get_data (G_OBJECT (tbl), "combo-box-size"));
            sp_text_toolbox_size_changed (cbox, tbl);
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            g_object_set_data (tbl, "enter-pressed", gpointer(0));
            return TRUE; // I consumed the event
            break;
    }
    return FALSE;
}

void
sp_text_toolbox_text_popdown_clicked    (GtkButton          */*button*/,
                                         GObject            *tbl)
{
    GtkWidget *popdown = GTK_WIDGET (g_object_get_data (tbl, "family-popdown-window"));
    GtkWidget *widget = GTK_WIDGET (g_object_get_data (tbl, "family-entry"));
    int x, y;

    if (!popdown_visible)
    {
        gdk_window_get_origin (widget->window, &x, &y);
        gtk_window_move (GTK_WINDOW (popdown), x, y + widget->allocation.height + 2); //2px of grace space
        gtk_widget_show_all (popdown);
        //sp_transientize (popdown);

        gdk_pointer_grab (widget->window, TRUE,
                          GdkEventMask (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                                        GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
                                        GDK_POINTER_MOTION_MASK),
                          NULL, NULL, GDK_CURRENT_TIME);

        gdk_keyboard_grab (widget->window, TRUE, GDK_CURRENT_TIME);

        popdown_visible = true;
    }
    else
    {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gdk_keyboard_ungrab (GDK_CURRENT_TIME);
        gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
        gtk_widget_hide (popdown);
        popdown_visible = false;
    }
}

gboolean
sp_text_toolbox_entry_focus_in  (GtkWidget        *entry,
                                 GdkEventFocus    */*event*/,
                                 GObject          */*tbl*/)
{
    gtk_entry_select_region (GTK_ENTRY (entry), 0, -1);
    return FALSE;
}

gboolean
sp_text_toolbox_popdown_focus_out (GtkWidget        *popdown,
                                   GdkEventFocus    */*event*/,
                                   GObject          */*tbl*/)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (popdown_hasfocus) {
        gtk_widget_hide (popdown);
        popdown_hasfocus = false;
        popdown_visible = false;
        gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
        return TRUE;
    }
    return FALSE;
}

gboolean
sp_text_toolbox_popdown_focus_in (GtkWidget        */*popdown*/,
                                   GdkEventFocus    */*event*/,
                                   GObject          */*tbl*/)
{
    popdown_hasfocus = true;
    return TRUE;
}


void
cell_data_func  (GtkTreeViewColumn */*column*/,
                 GtkCellRenderer   *cell,
                 GtkTreeModel      *tree_model,
                 GtkTreeIter       *iter,
                 gpointer           /*data*/)
{
    char        *family,
        *family_escaped,
        *sample_escaped;

    static const char *sample = _("AaBbCcIiPpQq12369$\342\202\254\302\242?.;/()");

    gtk_tree_model_get (tree_model, iter, 0, &family, -1);

    family_escaped = g_markup_escape_text (family, -1);
    sample_escaped = g_markup_escape_text (sample, -1);

    std::stringstream markup;
    markup << family_escaped << "  <span foreground='darkgray' font_family='" << family_escaped << "'>" << sample_escaped << "</span>";
    g_object_set (G_OBJECT (cell), "markup", markup.str().c_str(), NULL);

    free (family);
    free (family_escaped);
    free (sample_escaped);
}

static void delete_completion(GObject */*obj*/, GtkWidget *entry) {
    GObject *completion = (GObject *) gtk_object_get_data(GTK_OBJECT(entry), "completion");
    if (completion) {
        gtk_entry_set_completion (GTK_ENTRY(entry), NULL);
        g_object_unref (completion);
    }
}

GtkWidget*
sp_text_toolbox_new (SPDesktop *desktop)
{
    GtkToolbar   *tbl = GTK_TOOLBAR(gtk_toolbar_new());
    GtkIconSize secondarySize = static_cast<GtkIconSize>(prefToSize("toolbox", "secondary", 1));

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();
    Glib::RefPtr<Gtk::ListStore> store = Inkscape::FontLister::get_instance()->get_font_list();

    ////////////Family
    //Window
    GtkWidget   *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);

    //Entry
    GtkWidget           *entry = gtk_entry_new ();
    gtk_object_set_data(GTK_OBJECT(entry), "altx-text", entry);
    GtkEntryCompletion  *completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (Glib::unwrap(store)));
    gtk_entry_completion_set_text_column (completion, 0);
    gtk_entry_completion_set_minimum_key_length (completion, 1);
    g_object_set (G_OBJECT(completion), "inline-completion", TRUE, "popup-completion", TRUE, NULL);
    gtk_entry_set_completion (GTK_ENTRY(entry), completion);
    gtk_object_set_data(GTK_OBJECT(entry), "completion", completion);
    gtk_toolbar_append_widget( tbl, entry, "", "" );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_completion), entry);

    //Button
    GtkWidget   *button = gtk_button_new ();
    gtk_container_add       (GTK_CONTAINER (button), gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE));
    gtk_toolbar_append_widget( tbl, button, "", "");

    //Popdown
    GtkWidget           *sw = gtk_scrolled_window_new (NULL, NULL);
    GtkWidget           *treeview = gtk_tree_view_new ();

    GtkCellRenderer     *cell = gtk_cell_renderer_text_new ();
    GtkTreeViewColumn   *column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_cell_data_func (column, cell, GtkTreeCellDataFunc (cell_data_func), NULL, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (Glib::unwrap(store)));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);

    //gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), TRUE);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_container_add (GTK_CONTAINER (sw), treeview);

    gtk_container_add (GTK_CONTAINER (window), sw);
    gtk_widget_set_size_request (window, 300, 450);

    g_signal_connect (G_OBJECT (entry),  "activate", G_CALLBACK (sp_text_toolbox_family_entry_activate), tbl);
    g_signal_connect (G_OBJECT (entry),  "focus-in-event", G_CALLBACK (sp_text_toolbox_entry_focus_in), tbl);
    g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK(sp_text_toolbox_family_keypress), tbl);

    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (sp_text_toolbox_text_popdown_clicked), tbl);

    g_signal_connect (G_OBJECT (window), "focus-out-event", G_CALLBACK (sp_text_toolbox_popdown_focus_out), tbl);
    g_signal_connect (G_OBJECT (window), "focus-in-event", G_CALLBACK (sp_text_toolbox_popdown_focus_in), tbl);
    g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK(sp_text_toolbox_family_list_keypress), tbl);

    GtkTreeSelection *tselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (tselection), "changed", G_CALLBACK (sp_text_toolbox_family_changed), tbl);

    g_object_set_data (G_OBJECT (tbl), "family-entry", entry);
    g_object_set_data (G_OBJECT (tbl), "family-popdown-button", button);
    g_object_set_data (G_OBJECT (tbl), "family-popdown-window", window);
    g_object_set_data (G_OBJECT (tbl), "family-tree-selection", tselection);
    g_object_set_data (G_OBJECT (tbl), "family-tree-view", treeview);

    GtkWidget *image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, secondarySize);
    GtkWidget *box = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (box), image);
    gtk_toolbar_append_widget( tbl, box, "", "");
    g_object_set_data (G_OBJECT (tbl), "warning-image", box);
    GtkTooltips *tooltips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (tooltips, box, _("This font is currently not installed on your system. Inkscape will use the default font instead."), "");
    gtk_widget_hide (GTK_WIDGET (box));
    g_signal_connect_swapped (G_OBJECT (tbl), "show", G_CALLBACK (gtk_widget_hide), box);

    ////////////Size
    const char *sizes[] = {
        "4", "6", "8", "9", "10", "11", "12", "13", "14",
        "16", "18", "20", "22", "24", "28",
        "32", "36", "40", "48", "56", "64", "72", "144"
    };

    GtkWidget *cbox = gtk_combo_box_entry_new_text ();
    for (unsigned int n = 0; n < G_N_ELEMENTS (sizes); gtk_combo_box_append_text (GTK_COMBO_BOX(cbox), sizes[n++]));
    gtk_widget_set_size_request (cbox, 80, -1);
    gtk_toolbar_append_widget( tbl, cbox, "", "");
    g_object_set_data (G_OBJECT (tbl), "combo-box-size", cbox);
    g_signal_connect (G_OBJECT (cbox), "changed", G_CALLBACK (sp_text_toolbox_size_changed), tbl);
    gtk_signal_connect(GTK_OBJECT(gtk_bin_get_child(GTK_BIN(cbox))), "key-press-event", GTK_SIGNAL_FUNC(sp_text_toolbox_size_keypress), tbl);
    gtk_signal_connect(GTK_OBJECT(gtk_bin_get_child(GTK_BIN(cbox))), "focus-out-event", GTK_SIGNAL_FUNC(sp_text_toolbox_size_focusout), tbl);

    ////////////Text anchor
    GtkWidget *group   = gtk_radio_button_new (NULL);
    GtkWidget *row     = gtk_hbox_new (FALSE, 4);
    g_object_set_data (G_OBJECT (tbl), "anchor-group", group);

    // left
    GtkWidget *rbutton = group;
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT, secondarySize));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-start", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer(0));
    gtk_tooltips_set_tip(tt, rbutton, _("Align left"), NULL);

    // center
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_CENTER, secondarySize));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-middle", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer (1));
    gtk_tooltips_set_tip(tt, rbutton, _("Center"), NULL);

    // right
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_RIGHT, secondarySize));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-end", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer(2));
    gtk_tooltips_set_tip(tt, rbutton, _("Align right"), NULL);

    // fill
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, secondarySize));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-fill", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer(3));
    gtk_tooltips_set_tip(tt, rbutton, _("Justify"), NULL);

    gtk_toolbar_append_widget( tbl, row, "", "");

    //spacer
    gtk_toolbar_append_widget( tbl, gtk_vseparator_new(), "", "" );

    ////////////Text style
    row = gtk_hbox_new (FALSE, 4);

    // bold
    rbutton = gtk_toggle_button_new ();
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_BOLD, secondarySize));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Bold"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "style-bold", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_style_toggled), gpointer(0));

    // italic
    rbutton = gtk_toggle_button_new ();
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_ITALIC, secondarySize));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Italic"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "style-italic", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_style_toggled), gpointer (1));

    gtk_toolbar_append_widget( tbl, row, "", "");

    //spacer
    gtk_toolbar_append_widget( tbl, gtk_vseparator_new(), "", "" );

    ////////////Text orientation
    group   = gtk_radio_button_new (NULL);
    row     = gtk_hbox_new (FALSE, 4);
    g_object_set_data (G_OBJECT (tbl), "orientation-group", group);

    // horizontal
    rbutton = group;
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton),
                                 sp_icon_new (static_cast<Inkscape::IconSize>(secondarySize), INKSCAPE_STOCK_WRITING_MODE_LR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Horizontal text"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "orientation-horizontal", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_orientation_toggled), gpointer(0));

    // vertical
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton),
                                 sp_icon_new (static_cast<Inkscape::IconSize>(secondarySize), INKSCAPE_STOCK_WRITING_MODE_TB));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Vertical text"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "orientation-vertical", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_orientation_toggled), gpointer (1));
    gtk_toolbar_append_widget( tbl, row, "", "" );


    //watch selection
    Inkscape::ConnectionPool* pool = Inkscape::ConnectionPool::new_connection_pool ("ISTextToolbox");

    sigc::connection *c_selection_changed =
        new sigc::connection (sp_desktop_selection (desktop)->connectChanged
                              (sigc::bind (sigc::ptr_fun (sp_text_toolbox_selection_changed), (GObject*)tbl)));
    pool->add_connection ("selection-changed", c_selection_changed);

    sigc::connection *c_selection_modified =
        new sigc::connection (sp_desktop_selection (desktop)->connectModified
                              (sigc::bind (sigc::ptr_fun (sp_text_toolbox_selection_modified), (GObject*)tbl)));
    pool->add_connection ("selection-modified", c_selection_modified);

    sigc::connection *c_subselection_changed =
        new sigc::connection (desktop->connectToolSubselectionChanged
                              (sigc::bind (sigc::ptr_fun (sp_text_toolbox_subselection_changed), (GObject*)tbl)));
    pool->add_connection ("tool-subselection-changed", c_subselection_changed);

    Inkscape::ConnectionPool::connect_destroy (G_OBJECT (tbl), pool);


    gtk_widget_show_all( GTK_WIDGET(tbl) );

    return GTK_WIDGET(tbl);
} // end of sp_text_toolbox_new()

}//<unnamed> namespace


//#########################
//##      Connector      ##
//#########################

static void sp_connector_path_set_avoid(void)
{
    cc_selection_set_avoid(true);
}


static void sp_connector_path_set_ignore(void)
{
    cc_selection_set_avoid(false);
}



static void connector_spacing_changed(GtkAdjustment *adj, GObject* tbl)
{
    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );
    SPDocument *doc = sp_desktop_document(desktop);

    if (!sp_document_get_undo_sensitive(doc))
    {
        return;
    }

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);

    if ( repr->attribute("inkscape:connector-spacing") ) {
        gdouble priorValue = gtk_adjustment_get_value(adj);
        sp_repr_get_double( repr, "inkscape:connector-spacing", &priorValue );
        if ( priorValue == gtk_adjustment_get_value(adj) ) {
            return;
        }
    } else if ( adj->value == defaultConnSpacing ) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    sp_repr_set_css_double(repr, "inkscape:connector-spacing", adj->value);
    SP_OBJECT(desktop->namedview)->updateRepr();

    GSList *items = get_avoided_items(NULL, desktop->currentRoot(), desktop);
    for ( GSList const *iter = items ; iter != NULL ; iter = iter->next ) {
        SPItem *item = reinterpret_cast<SPItem *>(iter->data);
        NR::Matrix m = NR::identity();
        avoid_item_move(&m, item);
    }

    if (items) {
        g_slist_free(items);
    }

    sp_document_done(doc, SP_VERB_CONTEXT_CONNECTOR,
            _("Change connector spacing"));

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void sp_connector_graph_layout(void)
{
    if (!SP_ACTIVE_DESKTOP) return;

    // hack for clones, see comment in align-and-distribute.cpp
    int saved_compensation = prefs_get_int_attribute("options.clonecompensation", "value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs_set_int_attribute("options.clonecompensation", "value", SP_CLONE_COMPENSATION_UNMOVED);

    graphlayout(sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList());

    prefs_set_int_attribute("options.clonecompensation", "value", saved_compensation);

    sp_document_done(sp_desktop_document(SP_ACTIVE_DESKTOP), SP_VERB_DIALOG_ALIGN_DISTRIBUTE, _("Arrange connector network"));
}

static void sp_directed_graph_layout_toggled( GtkToggleAction* act, GtkObject */*tbl*/ )
{
    if ( gtk_toggle_action_get_active( act ) ) {
        prefs_set_string_attribute("tools.connector", "directedlayout",
                "true");
    } else {
        prefs_set_string_attribute("tools.connector", "directedlayout",
                "false");
    }
}

static void sp_nooverlaps_graph_layout_toggled( GtkToggleAction* act, GtkObject */*tbl*/ )
{
    if ( gtk_toggle_action_get_active( act ) ) {
        prefs_set_string_attribute("tools.connector", "avoidoverlaplayout",
                "true");
    } else {
        prefs_set_string_attribute("tools.connector", "avoidoverlaplayout",
                "false");
    }
}


static void connector_length_changed(GtkAdjustment *adj, GObject* tbl)
{
    prefs_set_double_attribute("tools.connector", "length", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void connector_tb_event_attr_changed(Inkscape::XML::Node *repr,
                                            gchar const *name, gchar const */*old_value*/, gchar const */*new_value*/,
                                            bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }
    if (strcmp(name, "inkscape:connector-spacing") != 0) {
        return;
    }

    GtkAdjustment *adj = (GtkAdjustment*)
            gtk_object_get_data(GTK_OBJECT(tbl), "spacing");
    gdouble spacing = defaultConnSpacing;
    sp_repr_get_double(repr, "inkscape:connector-spacing", &spacing);

    gtk_adjustment_set_value(adj, spacing);
}


static Inkscape::XML::NodeEventVector connector_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    connector_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


static void sp_connector_toolbox_prep( SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder )
{
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

    {
        InkAction* inky = ink_action_new( "ConnectorAvoidAction",
                                          _("Avoid"),
                                          _("Make connectors avoid selected objects"),
                                          "connector_avoid",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_path_set_avoid), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "ConnectorIgnoreAction",
                                          _("Ignore"),
                                          _("Make connectors ignore selected objects"),
                                          "connector_ignore",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_path_set_ignore), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    EgeAdjustmentAction* eact = 0;

    // Spacing spinbox
    eact = create_adjustment_action( "ConnectorSpacingAction",
                                     _("Connector Spacing"), _("Spacing:"),
                                     _("The amount of space left around objects by auto-routing connectors"),
                                     "tools.connector", "spacing", defaultConnSpacing,
                                     GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "inkscape:connector-spacing",
                                     0, 100, 1.0, 10.0,
                                     0, 0, 0,
                                     connector_spacing_changed, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );

    // Graph (connector network) layout
    {
        InkAction* inky = ink_action_new( "ConnectorGraphAction",
                                          _("Graph"),
                                          _("Nicely arrange selected connector network"),
                                          "graph_layout",
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_graph_layout), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    // Default connector length spinbox
    eact = create_adjustment_action( "ConnectorLengthAction",
                                     _("Connector Length"), _("Length:"),
                                     _("Ideal length for connectors when layout is applied"),
                                     "tools.connector", "length", 100,
                                     GTK_WIDGET(desktop->canvas), NULL, holder, TRUE, "inkscape:connector-length",
                                     10, 1000, 10.0, 100.0,
                                     0, 0, 0,
                                     connector_length_changed, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );


    // Directed edges toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorDirectedAction",
                                                      _("Downwards"),
                                                      _("Make connectors with end-markers (arrows) point downwards"),
                                                      "directed_graph",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        gchar const* tbuttonstate = prefs_get_string_attribute( "tools.connector", "directedlayout" );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act),
                (tbuttonstate && !strcmp(tbuttonstate, "true")) ? TRUE:FALSE );

        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_directed_graph_layout_toggled), holder );
    }

    // Avoid overlaps toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorOverlapAction",
                                                      _("Remove overlaps"),
                                                      _("Do not allow overlapping shapes"),
                                                      "remove_overlaps",
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        gchar const* tbuttonstate = prefs_get_string_attribute( "tools.connector", "avoidoverlaplayout" );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act),
                (tbuttonstate && !strcmp(tbuttonstate, "true")) ? TRUE:FALSE );

        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_nooverlaps_graph_layout_toggled), holder );
    }

    // Code to watch for changes to the connector-spacing attribute in
    // the XML.
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);
    g_assert(repr != NULL);

    purge_repr_listener( holder, holder );

    if (repr) {
        g_object_set_data( holder, "repr", repr );
        Inkscape::GC::anchor(repr);
        sp_repr_add_listener( repr, &connector_tb_repr_events, holder );
        sp_repr_synthesize_events( repr, &connector_tb_repr_events, holder );
    }
} // end of sp_connector_toolbox_prep()


//#########################
//##     Paintbucket     ##
//#########################

static void paintbucket_channels_changed(EgeSelectOneAction* act, GObject* /*tbl*/)
{
    gint channels = ege_select_one_action_get_active( act );
    flood_channels_set_channels( channels );
}

static void paintbucket_threshold_changed(GtkAdjustment *adj, GObject */*tbl*/)
{
    prefs_set_int_attribute("tools.paintbucket", "threshold", (gint)adj->value);
}

static void paintbucket_autogap_changed(EgeSelectOneAction* act, GObject */*tbl*/)
{
    prefs_set_int_attribute("tools.paintbucket", "autogap", ege_select_one_action_get_active( act ));
}

static void paintbucket_offset_changed(GtkAdjustment *adj, GObject *tbl)
{
    UnitTracker* tracker = static_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    SPUnit const *unit = tracker->getActiveUnit();

    prefs_set_double_attribute("tools.paintbucket", "offset", (gdouble)sp_units_get_pixels(adj->value, *unit));

    prefs_set_string_attribute("tools.paintbucket", "offsetunits", sp_unit_get_abbreviation(unit));
}

static void paintbucket_defaults(GtkWidget *, GObject *dataKludge)
{
    // FIXME: make defaults settable via Inkscape Options
    struct KeyValue {
        char const *key;
        double value;
    } const key_values[] = {
        {"threshold", 15},
        {"offset", 0.0}
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(key_values); ++i) {
        KeyValue const &kv = key_values[i];
        GtkAdjustment* adj = static_cast<GtkAdjustment *>(g_object_get_data(dataKludge, kv.key));
        if ( adj ) {
            gtk_adjustment_set_value(adj, kv.value);
        }
    }

    EgeSelectOneAction* channels_action = EGE_SELECT_ONE_ACTION( g_object_get_data( dataKludge, "channels_action" ) );
    ege_select_one_action_set_active( channels_action, FLOOD_CHANNELS_RGB );
    EgeSelectOneAction* autogap_action = EGE_SELECT_ONE_ACTION( g_object_get_data( dataKludge, "autogap_action" ) );
    ege_select_one_action_set_active( autogap_action, 0 );
}

static void sp_paintbucket_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;

    {
        GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );

        GList* items = 0;
        gint count = 0;
        for ( items = flood_channels_dropdown_items_list(); items ; items = g_list_next(items) )
        {
            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, reinterpret_cast<gchar*>(items->data), 1, count, -1 );
            count++;
        }
        g_list_free( items );
        items = 0;
        EgeSelectOneAction* act1 = ege_select_one_action_new( "ChannelsAction", _("Fill by"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act1, "short_label", _("Fill by:"), NULL );
        ege_select_one_action_set_appearance( act1, "compact" );
        ege_select_one_action_set_active( act1, prefs_get_int_attribute("tools.paintbucket", "channels", 0) );
        g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(paintbucket_channels_changed), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
        g_object_set_data( holder, "channels_action", act1 );
    }

    // Spacing spinbox
    {
        eact = create_adjustment_action(
            "ThresholdAction",
            _("Fill Threshold"), _("Threshold:"),
            _("The maximum allowed difference between the clicked pixel and the neighboring pixels to be counted in the fill"),
            "tools.paintbucket", "threshold", 5, GTK_WIDGET(desktop->canvas), NULL, holder, TRUE,
            "inkscape:paintbucket-threshold", 0, 100.0, 1.0, 10.0,
            0, 0, 0,
            paintbucket_threshold_changed, 1, 0 );

        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // Create the units menu.
    UnitTracker* tracker = new UnitTracker( SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE );
    const gchar *stored_unit = prefs_get_string_attribute("tools.paintbucket", "offsetunits");
    if (stored_unit)
        tracker->setActiveUnit(sp_unit_get_by_abbreviation(stored_unit));
    g_object_set_data( holder, "tracker", tracker );
    {
        GtkAction* act = tracker->createAction( "PaintbucketUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act );
    }

    // Offset spinbox
    {
        eact = create_adjustment_action(
            "OffsetAction",
            _("Grow/shrink by"), _("Grow/shrink by:"),
            _("The amount to grow (positive) or shrink (negative) the created fill path"),
            "tools.paintbucket", "offset", 0, GTK_WIDGET(desktop->canvas), NULL/*us*/, holder, TRUE,
            "inkscape:paintbucket-offset", -1e6, 1e6, 0.1, 0.5,
            0, 0, 0,
            paintbucket_offset_changed, 1, 2);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );

        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Auto Gap */
    {
        GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );

        GList* items = 0;
        gint count = 0;
        for ( items = flood_autogap_dropdown_items_list(); items ; items = g_list_next(items) )
        {
            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, reinterpret_cast<gchar*>(items->data), 1, count, -1 );
            count++;
        }
        g_list_free( items );
        items = 0;
        EgeSelectOneAction* act2 = ege_select_one_action_new( "AutoGapAction", _("Close gaps"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act2, "short_label", _("Close gaps:"), NULL );
        ege_select_one_action_set_appearance( act2, "compact" );
        ege_select_one_action_set_active( act2, prefs_get_int_attribute("tools.paintbucket", "autogap", 0) );
        g_signal_connect( G_OBJECT(act2), "changed", G_CALLBACK(paintbucket_autogap_changed), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act2) );
        g_object_set_data( holder, "autogap_action", act2 );
    }

    /* Reset */
    {
        GtkAction* act = gtk_action_new( "PaintbucketResetAction",
                                          _("Defaults"),
                                          _("Reset paint bucket parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                          GTK_STOCK_CLEAR );
        g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(paintbucket_defaults), holder );
        gtk_action_group_add_action( mainActions, act );
        gtk_action_set_sensitive( act, TRUE );
    }

}

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





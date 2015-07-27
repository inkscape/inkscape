/**
 * @file
 * Pencil aux toolbar
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

#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <list>

#include "pencil-toolbar.h"
#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "ui/tools-switch.h"
#include "ui/icon-names.h"
#include "ui/tools/pen-tool.h"
#include "ui/uxmanager.h"
#include "widgets/spinbutton-events.h"
#include <selection.h>
#include "live_effects/effect.h"
#include "live_effects/lpe-simplify.h"
#include "live_effects/effect-enum.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "sp-lpe-item.h"
#include "util/glib-list-iterators.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//########################
//##     Pen/Pencil     ##
//########################

/* This is used in generic functions below to share large portions of code between pen and pencil tool */
static Glib::ustring const freehand_tool_name(GObject *dataKludge)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(dataKludge, "desktop"));
    return ( tools_isactive(desktop, TOOLS_FREEHAND_PEN)
             ? "/tools/freehand/pen"
             : "/tools/freehand/pencil" );
}

static void freehand_mode_changed(EgeSelectOneAction* act, GObject* tbl)
{
    gint mode = ege_select_one_action_get_active(act);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(freehand_tool_name(tbl) + "/freehand-mode", mode);

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(tbl, "desktop"));

    // in pen tool we have more options than in pencil tool; if one of them was chosen, we do any
    // preparatory work here
    if (SP_IS_PEN_CONTEXT(desktop->event_context)) {
        Inkscape::UI::Tools::PenTool *pc = SP_PEN_CONTEXT(desktop->event_context);
        pc->setPolylineMode();
    }
}

static void sp_add_freehand_mode_toggle(GtkActionGroup* mainActions, GObject* holder, bool tool_is_pencil)
{
    /* Freehand mode toggle buttons */
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        guint freehandMode = prefs->getInt(( tool_is_pencil ? "/tools/freehand/pencil/freehand-mode" : "/tools/freehand/pen/freehand-mode" ), 0);
        Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

        {
            GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Bezier"),
                                1, _("Create regular Bezier path"),
                                2, INKSCAPE_ICON("path-mode-bezier"),
                                -1 );

            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Spiro"),
                                1, _("Create Spiro path"),
                                2, INKSCAPE_ICON("path-mode-spiro"),
                                -1 );
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("BSpline"),
                                1, _("Create BSpline path"),
                                2, INKSCAPE_ICON("path-mode-bspline"),
                                -1 );
            if (!tool_is_pencil) {
                gtk_list_store_append( model, &iter );
                gtk_list_store_set( model, &iter,
                                    0, _("Zigzag"),
                                    1, _("Create a sequence of straight line segments"),
                                    2, INKSCAPE_ICON("path-mode-polyline"),
                                    -1 );

                gtk_list_store_append( model, &iter );
                gtk_list_store_set( model, &iter,
                                    0, _("Paraxial"),
                                    1, _("Create a sequence of paraxial line segments"),
                                    2, INKSCAPE_ICON("path-mode-polyline-paraxial"),
                                    -1 );
            }

            EgeSelectOneAction* act = ege_select_one_action_new(tool_is_pencil ?
                                                                "FreehandModeActionPencil" :
                                                                "FreehandModeActionPen",
                                                                (_("Mode:")), (_("Mode of new lines drawn by this tool")), NULL, GTK_TREE_MODEL(model) );
            gtk_action_group_add_action( mainActions, GTK_ACTION(act) );

            ege_select_one_action_set_appearance( act, "full" );
            ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
            g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
            ege_select_one_action_set_icon_column( act, 2 );
            ege_select_one_action_set_icon_size( act, secondarySize );
            ege_select_one_action_set_tooltip_column( act, 1  );

            ege_select_one_action_set_active( act, freehandMode);
            g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(freehand_mode_changed), holder);
        }
    }
}

static void freehand_change_shape(EgeSelectOneAction* act, GObject *dataKludge) {
    gint shape = ege_select_one_action_get_active( act );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(freehand_tool_name(dataKludge) + "/shape", shape);
}

static void freehand_simplify_lpe(InkToggleAction* itact, GObject *dataKludge) {
    gint simplify = gtk_toggle_action_get_active( GTK_TOGGLE_ACTION(itact) );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(freehand_tool_name(dataKludge) + "/simplify", simplify);
}

/**
 * Generate the list of freehand advanced shape option entries.
 */
static GList * freehand_shape_dropdown_items_list() {
    GList *glist = NULL;

    glist = g_list_append (glist, const_cast<gchar *>(C_("Freehand shape", "None")));
    glist = g_list_append (glist, _("Triangle in"));
    glist = g_list_append (glist, _("Triangle out"));
    glist = g_list_append (glist, _("Ellipse"));
    glist = g_list_append (glist, _("From clipboard"));
    glist = g_list_append (glist, _("Last applied"));

    return glist;
}

static void freehand_add_advanced_shape_options(GtkActionGroup* mainActions, GObject* holder, bool tool_is_pencil)
{
    /*advanced shape options */
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );

        GList* items = 0;
        gint count = 0;
        for ( items = freehand_shape_dropdown_items_list(); items ; items = g_list_next(items) )
        {
            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, reinterpret_cast<gchar*>(items->data), 1, count, -1 );
            count++;
        }
        g_list_free( items );
        items = 0;
        EgeSelectOneAction* act1 = ege_select_one_action_new(
            tool_is_pencil ? "SetPencilShapeAction" : "SetPenShapeAction",
            _("Shape:"), (_("Shape of new paths drawn by this tool")), NULL, GTK_TREE_MODEL(model));
        g_object_set( act1, "short_label", _("Shape:"), NULL );
        ege_select_one_action_set_appearance( act1, "compact" );
        ege_select_one_action_set_active( act1, prefs->getInt(( tool_is_pencil ? "/tools/freehand/pencil/shape" : "/tools/freehand/pen/shape" ), 0) );
        g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(freehand_change_shape), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
        g_object_set_data( holder, "shape_action", act1 );
    }
}

void sp_pen_toolbox_prep(SPDesktop * /*desktop*/, GtkActionGroup* mainActions, GObject* holder)
{
    sp_add_freehand_mode_toggle(mainActions, holder, false);
    freehand_add_advanced_shape_options(mainActions, holder, false);
}


static void sp_pencil_tb_defaults(GtkWidget * /*widget*/, GObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);

    GtkAdjustment *adj;

    // fixme: make settable
    gdouble tolerance = 4;

    adj = GTK_ADJUSTMENT(g_object_get_data(obj, "tolerance"));
    gtk_adjustment_set_value(adj, tolerance);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(tbl);
}

static void sp_simplify_flatten(GtkWidget * /*widget*/, GObject *obj)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(obj, "desktop"));
    std::vector<SPItem *> selected = desktop->getSelection()->itemList();
    for (std::vector<SPItem *>::iterator it(selected.begin()); it != selected.end(); ++it){
        SPLPEItem* lpeitem = dynamic_cast<SPLPEItem*>(*it);
        if (lpeitem && lpeitem->hasPathEffect()){
            PathEffectList lpelist = lpeitem->getEffectList();
            std::list<Inkscape::LivePathEffect::LPEObjectReference *>::iterator i;
            for (i = lpelist.begin(); i != lpelist.end(); ++i) {
                LivePathEffectObject *lpeobj = (*i)->lpeobject;
                if (lpeobj) {
                    Inkscape::LivePathEffect::Effect *lpe = lpeobj->get_lpe();
                    if (dynamic_cast<Inkscape::LivePathEffect::LPESimplify *>(lpe)) {
                        SPShape * shape = dynamic_cast<SPShape *>(lpeitem);
                        if(shape){
                            SPCurve * c = shape->getCurveBeforeLPE();
                            lpe->doEffect(c);
                            lpeitem->setCurrentPathEffect(*i);
                            if (lpelist.size() > 1){
                                lpeitem->removeCurrentPathEffect(true);
                                shape->setCurveBeforeLPE(c);
                            } else {
                                lpeitem->removeCurrentPathEffect(false);
                                shape->setCurve(c,0);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

static void sp_pencil_tb_tolerance_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }
    // in turn, prevent listener from responding
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );
    prefs->setDouble("/tools/freehand/pencil/tolerance",
            gtk_adjustment_get_value(adj));
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(tbl, "desktop"));
    std::vector<SPItem *> selected = desktop->getSelection()->itemList();
    for (std::vector<SPItem *>::iterator it(selected.begin()); it != selected.end(); ++it){
        SPLPEItem* lpeitem = dynamic_cast<SPLPEItem*>(*it);
        if (lpeitem && lpeitem->hasPathEffect()){
            Inkscape::LivePathEffect::Effect* thisEffect = lpeitem->getPathEffectOfType(Inkscape::LivePathEffect::SIMPLIFY);
            if(thisEffect){
                Inkscape::LivePathEffect::LPESimplify *lpe = dynamic_cast<Inkscape::LivePathEffect::LPESimplify*>(thisEffect->getLPEObj()->get_lpe());
                if (lpe) {
                    double tol = prefs->getDoubleLimited("/tools/freehand/pencil/tolerance", 10.0, 1.0, 100.0);
                    tol = tol/(100.0*(102.0-tol));
                    std::ostringstream ss;
                    ss << tol;
                    lpe->getRepr()->setAttribute("threshold", ss.str());
                }
            }
        }
    }
}

/*
class PencilToleranceObserver : public Inkscape::Preferences::Observer {
public:
    PencilToleranceObserver(Glib::ustring const &path, GObject *x) : Observer(path), _obj(x)
    {
        g_object_set_data(_obj, "prefobserver", this);
    }
    virtual ~PencilToleranceObserver() {
        if (g_object_get_data(_obj, "prefobserver") == this) {
            g_object_set_data(_obj, "prefobserver", NULL);
        }
    }
    virtual void notify(Inkscape::Preferences::Entry const &val) {
        GObject* tbl = _obj;
        if (g_object_get_data( tbl, "freeze" )) {
            return;
        }
        g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

        GtkAdjustment * adj = GTK_ADJUSTMENT(g_object_get_data(tbl, "tolerance"));

        double v = val.getDouble(adj->value);
        gtk_adjustment_set_value(adj, v);
        g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
    }
private:
    GObject *_obj;
};
*/

void sp_pencil_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    sp_add_freehand_mode_toggle(mainActions, holder, true);

    EgeAdjustmentAction* eact = 0;

    /* Tolerance */
    {
        gchar const* labels[] = {_("(many nodes, rough)"), _("(default)"), 0, 0, 0, 0, _("(few nodes, smooth)")};
        gdouble values[] = {1, 10, 20, 30, 50, 75, 100};
        eact = create_adjustment_action( "PencilToleranceAction",
                                         _("Smoothing:"), _("Smoothing: "),
                 _("How much smoothing (simplifying) is applied to the line"),
                                         "/tools/freehand/pencil/tolerance",
                                         3.0,
                                         GTK_WIDGET(desktop->canvas),
                                         holder, TRUE, "altx-pencil",
                                         1, 100.0, 0.5, 1.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_pencil_tb_tolerance_value_changed,
                                         NULL /*unit tracker*/,
                                         1, 2);
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* advanced shape options */
    freehand_add_advanced_shape_options(mainActions, holder, true);

    /* Reset */
    {
        InkAction* inky = ink_action_new( "PencilResetAction",
                                          _("Defaults"),
                                          _("Reset pencil parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                          INKSCAPE_ICON("edit-clear"),
                                          Inkscape::ICON_SIZE_SMALL_TOOLBAR );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_pencil_tb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }
    /* LPE simplify based tolerance */
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        InkToggleAction* itact = ink_toggle_action_new( "PencilLpeSimplify",
                                                        _("LPE based interactive simplify"),
                                                        _("LPE based interactive simplify"),
                                                        INKSCAPE_ICON("interactive_simplify"),
                                                        Inkscape::ICON_SIZE_SMALL_TOOLBAR );
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(itact), prefs->getInt("/tools/freehand/pencil/simplify", 0) );
        g_signal_connect_after(  G_OBJECT(itact), "toggled", G_CALLBACK(freehand_simplify_lpe), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }
    /* LPE simplify flatten */
    {
        InkAction* inky = ink_action_new( "PencilLpeSimplifyFlatten",
                                          _("LPE simplify flatten"),
                                          _("LPE simplify flatten"),
                                          INKSCAPE_ICON("flatten_simplify"),
                                          Inkscape::ICON_SIZE_SMALL_TOOLBAR );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_simplify_flatten), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

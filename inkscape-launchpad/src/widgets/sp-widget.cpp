/*
 * Abstract base class for dynamic control widgets
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "macros.h"
#include "document.h"
#include "inkscape.h"
#include "sp-widget.h"
#include "helper/sp-marshal.h"

using Inkscape::SPWidgetImpl;

enum {
    CONSTRUCT,
    MODIFY_SELECTION,
    CHANGE_SELECTION,
    SET_SELECTION,
    LAST_SIGNAL
};

namespace Inkscape {

class SPWidgetImpl
{
public:
    SPWidgetImpl(SPWidget &target);
    ~SPWidgetImpl();

    static void dispose(GObject *object);
    static void show(GtkWidget *widget);
    static void hide(GtkWidget *widget);

#if GTK_CHECK_VERSION(3,0,0)
    static void getPreferredWidth(GtkWidget *widget,
                                  gint *minimal_width,
                                  gint *natural_width);

    static void getPreferredHeight(GtkWidget *widget,
                                   gint *minimal_height,
                                   gint *natural_height);
    static gboolean draw(GtkWidget *widget, cairo_t *cr);
#else
    static void sizeRequest(GtkWidget *widget, GtkRequisition *requisition);
    static gboolean expose(GtkWidget *widget, GdkEventExpose *event);
#endif

    static void sizeAllocate(GtkWidget *widget, GtkAllocation *allocation);
    static void modifySelectionCB(Selection *selection, guint flags, SPWidget *spw);
    static void changeSelectionCB(Selection *selection, SPWidget *spw);
    static void setSelectionCB(Selection *selection, SPWidget *spw);

    static GtkWidget *constructGlobal(SPWidget *spw);

    void modifySelection(Selection *selection, guint flags);
    void changeSelection(Selection *selection);
    void setSelection(Selection *selection);

private:
    SPWidget &_target;
};
} // namespace Inkscape

G_DEFINE_TYPE(SPWidget, sp_widget, GTK_TYPE_BIN);

static guint signals[LAST_SIGNAL] = {0};

static void
sp_widget_class_init(SPWidgetClass *klass)
{
    GObjectClass *object_class = reinterpret_cast<GObjectClass *>(klass);
    GtkWidgetClass *widget_class = reinterpret_cast<GtkWidgetClass *>(klass);

    object_class->dispose = SPWidgetImpl::dispose;

    signals[CONSTRUCT] =        g_signal_new ("construct",
                                              G_TYPE_FROM_CLASS(object_class),
                                              G_SIGNAL_RUN_FIRST,
                                              G_STRUCT_OFFSET(SPWidgetClass, construct),
                                              NULL, NULL,
                                              g_cclosure_marshal_VOID__VOID,
                                              G_TYPE_NONE, 0);

    signals[CHANGE_SELECTION] = g_signal_new ("change_selection",
                                              G_TYPE_FROM_CLASS(object_class),
                                              G_SIGNAL_RUN_FIRST,
                                              G_STRUCT_OFFSET(SPWidgetClass, change_selection),
                                              NULL, NULL,
                                              g_cclosure_marshal_VOID__POINTER,
                                              G_TYPE_NONE, 1,
                                              G_TYPE_POINTER);

    signals[MODIFY_SELECTION] = g_signal_new ("modify_selection",
                                              G_TYPE_FROM_CLASS(object_class),
                                              G_SIGNAL_RUN_FIRST,
                                              G_STRUCT_OFFSET(SPWidgetClass, modify_selection),
                                              NULL, NULL,
                                              sp_marshal_VOID__POINTER_UINT,
                                              G_TYPE_NONE, 2,
                                              G_TYPE_POINTER, G_TYPE_UINT);

    signals[SET_SELECTION] =    g_signal_new ("set_selection",
                                              G_TYPE_FROM_CLASS(object_class),
                                              G_SIGNAL_RUN_FIRST,
                                              G_STRUCT_OFFSET(SPWidgetClass, set_selection),
                                              NULL, NULL,
                                              g_cclosure_marshal_VOID__POINTER,
                                              G_TYPE_NONE, 1,
                                              G_TYPE_POINTER);

    widget_class->show = SPWidgetImpl::show;
    widget_class->hide = SPWidgetImpl::hide;
#if GTK_CHECK_VERSION(3,0,0)
    widget_class->get_preferred_width = SPWidgetImpl::getPreferredWidth;
    widget_class->get_preferred_height = SPWidgetImpl::getPreferredHeight;
    widget_class->draw = SPWidgetImpl::draw;
#else
    widget_class->size_request = SPWidgetImpl::sizeRequest;
    widget_class->expose_event = SPWidgetImpl::expose;
#endif
    widget_class->size_allocate = SPWidgetImpl::sizeAllocate;
}

static void sp_widget_init(SPWidget *spw)
{
    spw->_impl = new SPWidgetImpl(*spw); // ctor invoked after all other init
}

namespace Inkscape {

SPWidgetImpl::SPWidgetImpl(SPWidget &target) :
    _target(target)
{
}

SPWidgetImpl::~SPWidgetImpl()
{
}

void SPWidgetImpl::dispose(GObject *object)
{
    SPWidget *spw = reinterpret_cast<SPWidget *>(object);

    // Disconnect signals
    if (Application::exists()) {
        spw->selModified.disconnect();
        spw->selChanged.disconnect();
        spw->selSet.disconnect();
    }            

    delete spw->_impl;
    spw->_impl = 0;

    if (G_OBJECT_CLASS(sp_widget_parent_class)->dispose) {
        G_OBJECT_CLASS(sp_widget_parent_class)->dispose(object);
    }
}

void SPWidgetImpl::show(GtkWidget *widget)
{
    SPWidget *spw = SP_WIDGET(widget);

    if (Application::exists()) {
        // Connect signals
        spw->selModified = INKSCAPE.signal_selection_modified.connect(
                sigc::bind(
                sigc::ptr_fun(SPWidgetImpl::modifySelectionCB), spw)
        );
        spw->selChanged = INKSCAPE.signal_selection_changed.connect(
                sigc::bind(
                sigc::ptr_fun(SPWidgetImpl::changeSelectionCB), spw)
        );
        spw->selSet = INKSCAPE.signal_selection_set.connect(
                sigc::bind(
                sigc::ptr_fun(SPWidgetImpl::setSelectionCB), spw)
        );
    }

    if (GTK_WIDGET_CLASS(sp_widget_parent_class)->show) {
        GTK_WIDGET_CLASS(sp_widget_parent_class)->show(widget);
    }
}

void SPWidgetImpl::hide(GtkWidget *widget)
{
    SPWidget *spw = SP_WIDGET (widget);

    if (Application::exists()) {
        // Disconnect signals
        spw->selModified.disconnect();
        spw->selChanged.disconnect();
        spw->selSet.disconnect();
    }

    if (GTK_WIDGET_CLASS(sp_widget_parent_class)->hide) {
        GTK_WIDGET_CLASS(sp_widget_parent_class)->hide(widget);
    }
}

#if GTK_CHECK_VERSION(3,0,0)
gboolean SPWidgetImpl::draw(GtkWidget *widget, cairo_t *cr)
#else
gboolean SPWidgetImpl::expose(GtkWidget *widget, GdkEventExpose *event)
#endif
{
    GtkBin    *bin = GTK_BIN(widget);
    GtkWidget *child = gtk_bin_get_child(bin);

    if (child) {
#if GTK_CHECK_VERSION(3,0,0)
        gtk_container_propagate_draw(GTK_CONTAINER(widget), child, cr);
#else
        gtk_container_propagate_expose(GTK_CONTAINER(widget), child, event);
#endif
    }

    return FALSE;
}

#if GTK_CHECK_VERSION(3,0,0)
void SPWidgetImpl::getPreferredWidth(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
    GtkBin    *bin   = GTK_BIN(widget);
    GtkWidget *child = gtk_bin_get_child(bin);

    if(child) {
        gtk_widget_get_preferred_width(child, minimal_width, natural_width);
    }
}

void SPWidgetImpl::getPreferredHeight(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    GtkBin    *bin   = GTK_BIN(widget);
    GtkWidget *child = gtk_bin_get_child(bin);

    if(child) {
        gtk_widget_get_preferred_height(child, minimal_height, natural_height);
    }
}
#else
void SPWidgetImpl::sizeRequest(GtkWidget *widget, GtkRequisition *requisition)
{
    GtkBin    *bin   = GTK_BIN(widget);
    GtkWidget *child = gtk_bin_get_child(bin);

    if (child) {
        gtk_widget_size_request(child, requisition);
    }
}
#endif

void SPWidgetImpl::sizeAllocate(GtkWidget *widget, GtkAllocation *allocation)
{
    gtk_widget_set_allocation(widget, allocation);

    GtkBin        *bin   = GTK_BIN(widget);
    GtkWidget     *child = gtk_bin_get_child(bin);

    if (child) {
        gtk_widget_size_allocate(child, allocation);
    }
}

GtkWidget *SPWidgetImpl::constructGlobal(SPWidget *spw)
{
    if (gtk_widget_get_visible(GTK_WIDGET(spw))) {
        spw->selModified = INKSCAPE.signal_selection_modified.connect(
                sigc::bind(
                sigc::ptr_fun(SPWidgetImpl::modifySelectionCB), spw)
        );
        spw->selChanged = INKSCAPE.signal_selection_changed.connect(
                sigc::bind(
                sigc::ptr_fun(SPWidgetImpl::changeSelectionCB), spw)
        );
        spw->selSet = INKSCAPE.signal_selection_set.connect(
                sigc::bind(
                sigc::ptr_fun(SPWidgetImpl::setSelectionCB), spw)
        );
    }

    g_signal_emit(spw, signals[CONSTRUCT], 0);

    return GTK_WIDGET(spw);
}

void SPWidgetImpl::modifySelectionCB(Selection *selection, guint flags, SPWidget *spw)
{
    spw->_impl->modifySelection(selection, flags);
}

void SPWidgetImpl::changeSelectionCB(Selection *selection, SPWidget *spw)
{
    spw->_impl->changeSelection(selection);
}

void SPWidgetImpl::setSelectionCB(Selection *selection, SPWidget *spw)
{
    spw->_impl->setSelection(selection);
}

void SPWidgetImpl::modifySelection(Selection *selection, guint flags)
{
    g_signal_emit(&_target, signals[MODIFY_SELECTION], 0, selection, flags);
}

void SPWidgetImpl::changeSelection(Selection *selection)
{
    g_signal_emit(&_target, signals[CHANGE_SELECTION], 0, selection);
}

void SPWidgetImpl::setSelection(Selection *selection)
{
    // Emit "set_selection" signal
    g_signal_emit(&_target, signals[SET_SELECTION], 0, selection);
    // Inkscape will force "change_selection" anyways
}

} // namespace Inkscape

// Methods

GtkWidget *sp_widget_new_global()
{
    SPWidget *spw = reinterpret_cast<SPWidget*>(g_object_new(SP_TYPE_WIDGET, NULL));

    if (!SPWidgetImpl::constructGlobal(spw)) {
        g_object_unref(spw);
        spw = 0;
    }

    return reinterpret_cast<GtkWidget *>(spw);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

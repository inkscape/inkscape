/**
 * \brief A panel holding dialog
 *
 * Authors:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_PANEL_DIALOG_H
#define INKSCAPE_PANEL_DIALOG_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/stock.h>

#include "verbs.h"
#include "dialog.h"
#include "dialogs/swatches.h"
#include "ui/dialog/floating-behavior.h"
#include "ui/dialog/dock-behavior.h"
#include "prefs-utils.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/* local desktop event handlers */
static void handle_activate_desktop(Inkscape::Application *, SPDesktop *, void *);
static void handle_deactivate_desktop(Inkscape::Application *, SPDesktop *, void *);

struct PanelDialogBase {
    virtual void present() =0;
    virtual Panel &getPanel() =0;
    virtual ~PanelDialogBase() {}

private:
    virtual void _propagateDocumentReplaced(SPDesktop* desktop, SPDocument *document) =0;
    virtual void _propagateDesktopActivated(Inkscape::Application *, SPDesktop *) =0;
    virtual void _propagateDesktopDeactivated(Inkscape::Application *, SPDesktop *) =0;

    friend void handle_activate_desktop(Inkscape::Application *, SPDesktop *, void *);
    friend void handle_deactivate_desktop(Inkscape::Application *, SPDesktop *, void *);
};

template <typename Behavior>
class PanelDialog : public PanelDialogBase, public Inkscape::UI::Dialog::Dialog {

public:
    PanelDialog(Panel &contents, char const *prefs_path, int const verb_num, 
                Glib::ustring const &apply_label);

    virtual ~PanelDialog() {}

    template <typename T>
    static PanelDialog<Behavior> *create();

    virtual void present();

    Panel &getPanel() { return _panel; }

private:
    void _propagateDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    void _propagateDesktopActivated(Inkscape::Application *, SPDesktop *);
    void _propagateDesktopDeactivated(Inkscape::Application *, SPDesktop *);

    Panel &_panel;
    sigc::connection _document_replaced_connection;

    PanelDialog();  // no constructor without params
    PanelDialog(PanelDialog<Behavior> const &d);                      // no copy
    PanelDialog<Behavior>& operator=(PanelDialog<Behavior> const &d); // no assign

};

template <typename B>
PanelDialog<B>::PanelDialog(Panel &panel, char const *prefs_path, int const verb_num, Glib::ustring const &apply_label) :
    Dialog(&B::create, prefs_path, verb_num, apply_label),
    _panel (panel)
{
    Gtk::VBox *vbox = get_vbox();
    _panel.signalResponse().connect(sigc::mem_fun(*this, &PanelDialog::_handleResponse));
    _panel.signalPresent().connect(sigc::mem_fun(*this, &PanelDialog::present));

    vbox->pack_start(_panel, true, true, 0);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    _propagateDesktopActivated(INKSCAPE, desktop);

    _document_replaced_connection = 
        desktop->connectDocumentReplaced(sigc::mem_fun(*this, &PanelDialog::_propagateDocumentReplaced));

    if (prefs_get_int_attribute ("dialogs", "showclose", 0) || !apply_label.empty()) {
        // TODO: make the order of buttons obey the global preference
        if (!apply_label.empty()) {
            panel.addResponseButton(apply_label, Gtk::RESPONSE_APPLY);
            panel.setDefaultResponse(Gtk::RESPONSE_APPLY);
        }
        panel.addResponseButton(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    }

    show_all_children();
}

template <typename B> template <typename P>
PanelDialog<B> *
PanelDialog<B>::create()
{
    Panel &panel = P::getInstance();
    return new PanelDialog<B>(panel, panel.getPrefsPath(), panel.getVerb(), panel.getApplyLabel());
}

/** 
 * Specialize factory method for panel dialogs with floating behavior in order to make them work as
 * singletons, i.e. allow them track the current active desktop.
 */
template <> template <typename P>
PanelDialog<Behavior::FloatingBehavior> *
PanelDialog<Behavior::FloatingBehavior>::create()
{
    Panel &panel = P::getInstance();
    PanelDialog<Behavior::FloatingBehavior> *instance = 
        new PanelDialog<Behavior::FloatingBehavior>(panel, panel.getPrefsPath(), 
                                                    panel.getVerb(), panel.getApplyLabel());

    g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK(handle_activate_desktop), instance);
    g_signal_connect(G_OBJECT(INKSCAPE), "deactivate_desktop", G_CALLBACK(handle_deactivate_desktop), instance);

    return instance;
}

template <typename B>
void
PanelDialog<B>::present()
{
    Dialog::present();
}

template <typename B>
void
PanelDialog<B>::_propagateDocumentReplaced(SPDesktop *desktop, SPDocument *document)
{
    _panel.signalDocumentReplaced().emit(desktop, document);
}

template <typename B>
void
PanelDialog<B>::_propagateDesktopActivated(Inkscape::Application *application, SPDesktop *desktop)
{
    _document_replaced_connection = 
        desktop->connectDocumentReplaced(sigc::mem_fun(*this, &PanelDialog::_propagateDocumentReplaced));
    _panel.signalActivateDesktop().emit(application, desktop);
}

template <typename B>
void
PanelDialog<B>::_propagateDesktopDeactivated(Inkscape::Application *application, SPDesktop *desktop)
{
    _document_replaced_connection.disconnect();
    _panel.signalDeactiveDesktop().emit(application, desktop);
}


static void
handle_activate_desktop(Inkscape::Application *application, SPDesktop *desktop, void *data)
{
    g_return_if_fail(data != NULL);
    static_cast<PanelDialogBase *>(data)->_propagateDesktopActivated(application, desktop);
}

static void
handle_deactivate_desktop(Inkscape::Application *application, SPDesktop *desktop, void *data)
{
    g_return_if_fail(data != NULL);
    static_cast<PanelDialogBase *>(data)->_propagateDesktopDeactivated(application, desktop);
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_PANEL_DIALOG_H

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

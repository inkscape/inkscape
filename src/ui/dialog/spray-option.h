
/*Julien LERAY (julien.leray@ecl2010.ec-lyon.fr), interface for the spray tool*/

#ifndef INKSCAPE_UI_DIALOG_SPRAY_OPTION_H
#define INKSCAPE_UI_DIALOG_SPRAY_OPTION_H

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include <list>
#include <gtkmm/frame.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/table.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include "libnr/nr-dim2.h"
#include "libnr/nr-rect.h"


#include "ui/widget/panel.h"
#include "ui/widget/notebook-page.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/spinbutton.h>
#include "desktop-handles.h"
#include "unclump.h"
#include "document.h"
#include "enums.h"
#include "graphlayout/graphlayout.h"
#include "inkscape.h"
#include "macros.h"
#include "node-context.h"
#include "preferences.h"
#include "removeoverlap/removeoverlap.h"
#include "selection.h"
#include "shape-editor.h"
#include "sp-flowtext.h"
#include "sp-item-transform.h"
#include "sp-text.h"
#include "text-editing.h"
#include "tools-switch.h"
#include "ui/icon-names.h"
#include "util/glib-list-iterators.h"
#include "verbs.h"
#include "widgets/icon.h"

#include "spray-context.h"
#include "verbs.h"

#include <iostream>
using namespace std;

using namespace Inkscape::UI::Widget;

class SPItem;


namespace Inkscape {
namespace UI {
namespace Dialog {

class Action;

class SprayOptionClass : public Widget::Panel {

private:

    SprayOptionClass(SprayOptionClass const &d);
    SprayOptionClass& operator=(SprayOptionClass const &d);

public:
    SprayOptionClass();
    virtual ~SprayOptionClass();
    void test() {    cout<<"appel de test !!"<<endl;  }
    static SprayOptionClass &getInstance() { return *new SprayOptionClass(); }


    Gtk::Table &_Table(){return _ETable;}
    Gtk::Table &F_Table(){return _FTable;}
    Gtk::Tooltips &tooltips(){return _tooltips;}
    void action();
    void combo_action();
    Geom::OptRect randomize_bbox;

    SprayOptionClass &get_SprayOptionClass();

protected:

    void addGaussianButton(guint row, guint col);
    void addEButton(const Glib::ustring &id, const Glib::ustring &tiptext, guint row, guint column,
                guint min, guint max, const Glib::ustring &pref_path);
    void addFButton(const Glib::ustring &id, const Glib::ustring &tiptext, guint row, guint column,
                const Glib::ustring &pref1_path, const Glib::ustring &pref2_path);

    std::list<Action *> _actionList;
    Gtk::Frame _distributionFrame;
    Gtk::Frame _Frame;
    Gtk::Frame _FFrame;
    Gtk::Table _distributionTable;
    Gtk::Table _gaussianTable;
    Gtk::Table _ETable;
    Gtk::Table _FTable;
    Gtk::HBox _anchorBox;
    Gtk::HBox _unifBox;
    Gtk::HBox _gaussianBox;
    Gtk::HBox _HBox;
    Gtk::HBox _FHBox;
    Gtk::HBox _BoutonBox;
    Gtk::VBox _distributionBox;
    Gtk::VBox  _VBox;
    Gtk::VBox _FVBox;
    Gtk::VBox _ActionBox;
    Gtk::Label _anchorLabel;
    Gtk::Label _unifLabel;
    Gtk::Label _gaussLabel;
    Gtk::Label _Label;
    Gtk::Label _FLabel;
    Gtk::CheckButton _unif;
    Gtk::CheckButton _gauss;
    Gtk::ComboBoxText _combo;
    Gtk::Tooltips _tooltips;
};


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H

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


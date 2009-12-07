/*Julien LERAY (julien.leray@ecl2010.ec-lyon.fr), interface for the spray tool*/


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

#include "spray-option.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


//Classes qui permettent de créer les environnements Gaussienne, Witdh...



class Action {
public:
    Action(const Glib::ustring &id,
           const Glib::ustring &tiptext,
           guint row, guint column,
           Gtk::Table &parent,
           Gtk::Tooltips &tooltips,
           SprayOptionClass &dialog):
        _dialog(dialog),
        _id(id),
        _parent(parent) {}
    
    virtual ~Action(){}
    virtual void on_button_click(){}
    SprayOptionClass &_dialog;
    
private :
    
    Glib::ustring _id;
    Gtk::Table &_parent;
};

class ActionE : public Action {
private:
    Gtk::Label _Label;
    Gtk::SpinButton _Gap;
    guint _min, _max;
    Glib::ustring _pref_path;

public:
    ActionE(const Glib::ustring &id,
                const Glib::ustring &tiptext,
                guint row, guint column,
                SprayOptionClass &dialog,
                guint min, guint max,
                Glib::ustring const &pref_path ):
        Action(id, tiptext, row, column,
               dialog._Table(), dialog.tooltips(), dialog),
        _min(min),
        _max(max),
        _pref_path(pref_path)
        {
            dialog._Table().set_col_spacings(3);

            double increm = ((double)_max - (double)_min)/10;
            double val_ini = ((double)_max + (double)_min)/2;            
            _Gap.set_digits(1);
            _Gap.set_size_request(60, -1);
            _Gap.set_increments(increm , 0);
            _Gap.set_range(_min, _max);
            _Gap.set_value(val_ini);
            dialog.tooltips().set_tip(_Gap,
                                  tiptext);
            _Gap.signal_changed().connect(sigc::mem_fun(*this, &ActionE::on_button_click));  //rajout douteux
            _Label.set_label(id);

            dialog._Table().attach(_Label, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
            dialog._Table().attach(_Gap, column+1, column+2, row, row+1, Gtk::EXPAND, Gtk::EXPAND);
        }

    virtual void on_button_click(){
        if (!_dialog.getDesktop()) return;
              
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        
        prefs->setDouble(_pref_path, SP_VERB_CONTEXT_SPRAY);
       
        double const Gap = _Gap.get_value();
        
         
        prefs->setDouble(_pref_path, Gap);

        sp_document_done(sp_desktop_document(_dialog.getDesktop()), SP_VERB_CONTEXT_SPRAY,
                         _("Remove overlaps"));
    }


};    

class ActionF : public Action {
private:
    Gtk::Label _Label;
    Gtk::Label _Label1;
    Gtk::Label _Label2;
    Gtk::SpinButton _Gap1;
    Gtk::SpinButton _Gap2;
    Glib::ustring _pref1_path;
    Glib::ustring _pref2_path;

public:
    ActionF(const Glib::ustring &id,
                const Glib::ustring &tiptext,
                guint row, guint column,
                SprayOptionClass &dialog,
                Glib::ustring const &pref1_path,
                Glib::ustring const &pref2_path ):
        Action(id, tiptext, row, column,
               dialog._Table(), dialog.tooltips(), dialog),
        _pref1_path(pref1_path),
        _pref2_path(pref2_path)
        {
            dialog.F_Table().set_col_spacings(3);

            _Label.set_label(id);            

            _Gap1.set_digits(1);
            _Gap1.set_size_request(60, -1);
            _Gap1.set_increments(0.1, 0);
            _Gap1.set_range(0, 10);
            _Gap1.set_value(1);
            dialog.tooltips().set_tip(_Gap1,
                                  _("Minimum"));
        
            _Label1.set_label(Q_("Min"));

            _Gap2.set_digits(1);
            _Gap2.set_size_request(60, -1);
            _Gap2.set_increments(0.1, 0);
            _Gap2.set_range(0, 10);
            _Gap2.set_value(1);
            dialog.tooltips().set_tip(_Gap2,
                                  _("Maximum"));
        
            _Label2.set_label(_("Max:"));
            
            _Gap1.signal_changed().connect(sigc::mem_fun(*this, &ActionF::on_button_click));
            _Gap2.signal_changed().connect(sigc::mem_fun(*this, &ActionF::on_button_click)); 

            dialog.F_Table().attach(_Label, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
            dialog.F_Table().attach(_Label1, column+1, column+2, row, row+1, Gtk::FILL, Gtk::FILL);
            dialog.F_Table().attach(_Gap1, column+2, column+3, row, row+1, Gtk::EXPAND, Gtk::EXPAND);
            dialog.F_Table().attach(_Label2, column+3, column+4, row, row+1, Gtk::FILL, Gtk::FILL);
            dialog.F_Table().attach(_Gap2, column+4, column+5, row, row+1, Gtk::EXPAND, Gtk::EXPAND);

        }

    virtual void on_button_click(){
        if (!_dialog.getDesktop()) return;
              
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        
        prefs->setDouble(_pref1_path, SP_VERB_CONTEXT_SPRAY);
        prefs->setDouble(_pref2_path, SP_VERB_CONTEXT_SPRAY);
       
        double const Gap1 = _Gap1.get_value();
        double const Gap2 = _Gap2.get_value();
                 
        prefs->setDouble(_pref1_path, Gap1);
        prefs->setDouble(_pref2_path, Gap2);

        sp_document_done(sp_desktop_document(_dialog.getDesktop()), SP_VERB_CONTEXT_SPRAY,
                         _("Remove overlaps"));
    }


};    



void SprayOptionClass::combo_action() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    cout<<"combo.get_active_row_number = "<<_combo.get_active_row_number()<<endl;
        
    int const distrib = _combo.get_active_row_number();
                 
    prefs->setInt("/tools/spray/distribution", distrib);
       

    sp_document_done(sp_desktop_document(this->getDesktop()), SP_VERB_CONTEXT_SPRAY,
                         _("Remove overlaps"));    

}




void SprayOptionClass::action() {
    int r=1;    
    for (list<Action *>::iterator it = _actionList.begin();
         it != _actionList.end();
         it ++)
        (*it)->on_button_click();
    combo_action();
}






void on_selection_changed(Inkscape::Application */*inkscape*/, Inkscape::Selection */*selection*/, SprayOptionClass *daad)
{
    daad->randomize_bbox = Geom::OptRect();
}

/////////////////////////////////////////////////////////
//Construction de l'interface
/////////////////////////////////////////////////////////


SprayOptionClass::SprayOptionClass()
    : UI::Widget::Panel ("", "/dialogs/spray", SP_VERB_DIALOG_SPRAY_OPTION),
      _distributionFrame(Q_("sprayOptions|Distribution")),
      _Frame(Q_("sprayOptions|Cursor Options")),
      _FFrame(Q_("sprayOptions|Random Options")),
      _gaussianTable(1, 5, false),
      _ETable(3,2,false),
      _FTable(2,5,false),
      _unifLabel(Q_("sprayOptions|Uniform")),
      _gaussLabel(Q_("sprayOptions|Gaussian")),
      _anchorLabel(Q_("sprayOptions|Distribution:"))
      
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    //ComboBoxText

    _combo.append_text(Q_("sprayOptions|Uniform"));
    _combo.append_text(Q_("sprayOptions|Gaussian"));
    
    _combo.set_active(prefs->getInt("/tools/spray/distribution", 1));
    _combo.signal_changed().connect(sigc::mem_fun(*this, &SprayOptionClass::combo_action));

    _anchorBox.pack_start(_anchorLabel);
    _anchorBox.pack_start(_combo);

    _gaussianBox.pack_start(_anchorBox);


    _distributionBox.pack_start(_gaussianBox);
    _distributionFrame.add(_distributionBox);


    //Hbox Random
    addFButton(Q_("sprayOptions|Scale:") ,_("Apply a scale factor"), 0, 0, "/tools/spray/scale_min","/tools/spray/scale_max");
    addFButton(Q_("sprayOptions|Rotation:") ,_("Apply rotation"), 1, 0, "/tools/spray/rot_min","/tools/spray/rot_max");
    _FHBox.pack_start(_FLabel);
    _FHBox.pack_start(_FTable);

    //Implementation dans la Vbox Cursor
    _FVBox.pack_start(_FHBox);
    _FFrame.add(_FVBox);    

    //Hbox Cursor
    addEButton(Q_("sprayOptions|Ratio:") ,_("Eccentricity of the ellipse"), 0, 0, 0, 1,"/tools/spray/ratio");
    addEButton(Q_("sprayOptions|Angle:") ,_("Angle of the ellipse"), 1, 0, 0, 5,"/tools/spray/tilt");
    addEButton(Q_("sprayOptions|Width:") ,_("Size of the ellipse"), 2, 0, 0, 1,"/tools/spray/width");
    _HBox.pack_start(_Label);
    _HBox.pack_start(_ETable);

    //Implementation dans la Vbox Cursor
    _VBox.pack_start(_HBox);
    _Frame.add(_VBox);

    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);

    
    

    

    // Crée dans l'ordre suivant les différentes Frames (cadres de réglages)

    contents->pack_start(_distributionFrame, true, true);
    contents->pack_start(_FFrame, true, true);    
    contents->pack_start(_Frame, true, true);
    
    

    // Connect to the global selection change, to invalidate cached randomize_bbox
    g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (on_selection_changed), this);
    randomize_bbox = Geom::OptRect();

    show_all_children();
    
    

}

SprayOptionClass::~SprayOptionClass()
{
    sp_signal_disconnect_by_data (G_OBJECT (INKSCAPE), this);

    for (std::list<Action *>::iterator it = _actionList.begin();
         it != _actionList.end();
         it ++)
        delete *it;
}







//Fonctions qui lient la demande d'ajout d'une interface graphique à l'action correspondante

void SprayOptionClass::addEButton(const Glib::ustring &id,
                const Glib::ustring &tiptext,
                guint row, guint column,
                guint min, guint max,
                Glib::ustring const &pref_path) 
{
        _actionList.push_back( new ActionE(id, tiptext,row, column,*this,min ,max, pref_path ));
}

void SprayOptionClass::addFButton(const Glib::ustring &id,
                const Glib::ustring &tiptext,
                guint row, guint column,
                Glib::ustring const &pref1_path,
                Glib::ustring const &pref2_path) 
{
        _actionList.push_back( new ActionF(id, tiptext,row, column,*this,pref1_path, pref2_path ));
}





SprayOptionClass &SprayOptionClass::get_SprayOptionClass()
{
    return *this;
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

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifndef INKSCAPE_EXTENSION_EFFECT_H__
#define INKSCAPE_EXTENSION_EFFECT_H__

#include <config.h>
#include <glibmm/i18n.h>
#include "verbs.h"
#include "extension.h"

namespace Gtk {
	class VBox;
}

class SPDocument;

namespace Inkscape {


namespace Extension {
class PrefDialog;

/** \brief  Effects are extensions that take a document and do something
            to it in place.  This class adds the extra functions required
            to make extensions effects.
*/
class Effect : public Extension {
    /** \brief  This is the last effect that was used.  This is used in
                a menu item to rapidly recall the same effect. */
    static Effect * _last_effect;
    /** \brief  The location of the Extensions and Filters menus on the menu structure
                XML file.  This is saved so it only has to be discovered
                once. */
    static Inkscape::XML::Node * _effects_list;
    static Inkscape::XML::Node * _filters_list;
    Inkscape::XML::Node *find_menu (Inkscape::XML::Node * menustruct, const gchar *name);
    void merge_menu (Inkscape::XML::Node * base, Inkscape::XML::Node * start, Inkscape::XML::Node * patern, Inkscape::XML::Node * mergee);

    /** \brief  This is the verb type that is used for all effect's verbs.
                It provides convience functions and maintains a pointer
                back to the effect that created it.  */
    class EffectVerb : public Inkscape::Verb {
        private:
            static void perform (SPAction * action, void * mydata);

            /** \brief  The effect that this verb represents. */
            Effect * _effect;
            /** \brief  Whether or not to show preferences on display */
            bool _showPrefs;
            /** \brief  Name with elipses if that makes sense */
            gchar * _elip_name;
        protected:
            virtual SPAction * make_action (Inkscape::ActionContext const & context);
        public:
            /** \brief Use the Verb initializer with the same parameters. */
            EffectVerb(gchar const * id,
                       gchar const * name,
                       gchar const * tip,
                       gchar const * image,
                       Effect *      effect,
                       bool          showPrefs) :
                    Verb(id, _(name), _(tip), image, _("Extensions")),
                    _effect(effect), 
                    _showPrefs(showPrefs),
                    _elip_name(NULL) {
                /* No clue why, but this is required */
                this->set_default_sensitive(true);
                if (_showPrefs && effect != NULL && effect->param_visible_count() != 0) {
                    _elip_name = g_strdup_printf("%s...", _(name));
                    set_name(_elip_name);
                }
            }
            
            /** \brief  Destructor */
            ~EffectVerb() {
                if (_elip_name != NULL) {
                    g_free(_elip_name);
                }
            }
    };

    /** \brief  ID used for the verb without preferences */
    Glib::ustring _id_noprefs;
    /** \brief  Name used for the verb without preferences */
    Glib::ustring _name_noprefs;

    /** \brief  The verb representing this effect. */
    EffectVerb _verb;
    /** \brief  The verb representing this effect.  Without preferences. */
    EffectVerb _verb_nopref;
    /** \brief  Menu node created for this effect */
    Inkscape::XML::Node * _menu_node;
    /** \brief  Whehter a working dialog should be shown */
    bool _workingDialog;

    /** \brief  The preference dialog if it is shown */
    PrefDialog * _prefDialog;
public:
                 Effect  (Inkscape::XML::Node * in_repr,
                          Implementation::Implementation * in_imp);
    virtual     ~Effect  (void);
    virtual bool check                (void);
    bool         prefs   (Inkscape::UI::View::View * doc);
    void         effect  (Inkscape::UI::View::View * doc);
    /** \brief  Accessor function for a pointer to the verb */
    Inkscape::Verb * get_verb (void) { return &_verb; };

    /** \brief  Static function to get the last effect used */
    static Effect *  get_last_effect (void) { return _last_effect; };
    static void      set_last_effect (Effect * in_effect);

    static void      place_menus (void);
    void             place_menu (Inkscape::XML::Node * menus);

    Gtk::VBox *    get_info_widget(void);

    bool no_doc; // if true, the effect does not process SVG document at all, so no need to save, read, and watch for errors
    bool no_live_preview; // if true, the effect does not need "live preview" checkbox in its dialog

    void        set_pref_dialog (PrefDialog * prefdialog);
private:
    static gchar *   remove_ (gchar * instr);
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_EFFECT_H__ */

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

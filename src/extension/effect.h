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
#include <gtk/gtkdialog.h>
#include "verbs.h"

#include "extension.h"

struct SPDocument;

namespace Inkscape {
namespace UI {
namespace View {
typedef View View;
};
};

namespace Extension {

/** \brief  Effects are extensions that take a document and do something
            to it in place.  This class adds the extra functions required
            to make extensions effects.
*/
class Effect : public Extension {
    /** \brief  This is the last effect that was used.  This is used in
                a menu item to rapidly recall the same effect. */
    static Effect * _last_effect;
    /** \brief  The location of the effects menu on the menu structure
                XML file.  This is saved so it only has to be discovered
                once. */
    static Inkscape::XML::Node * _effects_list;
    bool find_effects_list (Inkscape::XML::Node * menustruct);
    void merge_menu (Inkscape::XML::Node * base, Inkscape::XML::Node * start, Inkscape::XML::Node * patern, Inkscape::XML::Node * mergee);

    /** \brief  This is the verb type that is used for all effect's verbs.
                It provides convience functions and maintains a pointer
                back to the effect that created it.  */
    class EffectVerb : public Inkscape::Verb {
        private:
            static void perform (SPAction * action, void * mydata, void * otherdata);
            /** \brief  Function to call for specific actions */
            static SPActionEventVector vector;

            /** \brief  The effect that this verb represents. */
            Effect * _effect;
        protected:
            virtual SPAction * make_action (Inkscape::UI::View::View * view);
        public:
            /** \brief Use the Verb initializer with the same parameters. */
            EffectVerb(gchar const * id,
                       gchar const * name,
                       gchar const * tip,
                       gchar const * image,
                       Effect *      effect) :
                    Verb(id, _(name), _(tip), image), _effect(effect) {
                /* No clue why, but this is required */
                this->set_default_sensitive(true);
            }
    };

    /** \brief  The verb representing this effect. */
    EffectVerb _verb;
    /** \brief  Menu node created for this effect */
    Inkscape::XML::Node * _menu_node;
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

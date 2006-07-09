/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "inkscape-private.h"
#include "helper/action.h"
#include "document.h"
#include "prefdialog.h"
#include "implementation/implementation.h"
#include "effect.h"
#include "ui/view/view.h"

/* Inkscape::Extension::Effect */

namespace Inkscape {
namespace Extension {

Effect * Effect::_last_effect = NULL;
Inkscape::XML::Node * Effect::_effects_list = NULL;

Effect::Effect (Inkscape::XML::Node * in_repr, Implementation::Implementation * in_imp)
    : Extension(in_repr, in_imp), _verb(get_id(), get_name(), NULL, NULL, this), _menu_node(NULL)
{
    Inkscape::XML::Node * local_effects_menu = NULL;

    // This is a weird hack
    if (!strcmp(this->get_id(), "org.inkscape.filter.dropshadow"))
        return;

    if (repr != NULL) {
        Inkscape::XML::Node * child_repr;

        for (child_repr = sp_repr_children(repr); child_repr != NULL; child_repr = child_repr->next()) {
            if (!strcmp(child_repr->name(), "effect")) {
                for (child_repr = sp_repr_children(child_repr); child_repr != NULL; child_repr = child_repr->next()) {
                    if (!strcmp(child_repr->name(), "effects-menu")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        local_effects_menu = sp_repr_children(child_repr);
                    }
                    if (!strcmp(child_repr->name(), "menu-name") ||
                            !strcmp(child_repr->name(), "_menu-name")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        _verb.set_name(sp_repr_children(child_repr)->content());
                    }
                    if (!strcmp(child_repr->name(), "menu-tip") ||
                            !strcmp(child_repr->name(), "_menu-tip")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        _verb.set_tip(sp_repr_children(child_repr)->content());
                    }
                } // children of "effect"
                break; // there can only be one effect
            } // find "effect"
        } // children of "inkscape-extension"
    } // if we have an XML file

    if (_effects_list == NULL)
        find_effects_list(inkscape_get_menus(INKSCAPE));

    if (_effects_list != NULL) {
        _menu_node = sp_repr_new("verb");
        _menu_node->setAttribute("verb-id", this->get_id(), false);

        merge_menu(_effects_list->parent(), _effects_list, local_effects_menu, _menu_node);
    }

    return;
}

void
Effect::merge_menu (Inkscape::XML::Node * base,
                    Inkscape::XML::Node * start,
                    Inkscape::XML::Node * patern,
                    Inkscape::XML::Node * mergee) {
    Glib::ustring mergename;
    Inkscape::XML::Node * tomerge = NULL;
    Inkscape::XML::Node * submenu = NULL;

    /* printf("Merge menu with '%s' '%s' '%s'\n",
            base != NULL ? base->name() : "NULL",
            patern != NULL ? patern->name() : "NULL",
            mergee != NULL ? mergee->name() : "NULL"); */

    if (patern == NULL) {
        // Merge the verb name
        tomerge = mergee;
        mergename = _(this->get_name());
    } else {
        gchar const * menuname = patern->attribute("name");
        if (menuname == NULL) menuname = patern->attribute("_name");
        if (menuname == NULL) return;

        tomerge = sp_repr_new("submenu");
        tomerge->setAttribute("name", menuname, false);

        mergename = _(menuname);
    }

    int position = -1;

    if (start != NULL) {
        Inkscape::XML::Node * menupass;
        for (menupass = start->next(); menupass != NULL; menupass = menupass->next()) {
            gchar const * compare_char = NULL;
            if (!strcmp(menupass->name(), "verb")) {
                gchar const * verbid = menupass->attribute("verb-id");
                Inkscape::Verb * verb = Inkscape::Verb::getbyid(verbid);
                if (verb == NULL) {
                    continue;
                }
                compare_char = verb->get_name();
            } else if (!strcmp(menupass->name(), "submenu")) {
                compare_char = menupass->attribute("name");
                if (compare_char == NULL)
                    compare_char = menupass->attribute("_name");
            }

            /* This will cause us to skip tags we don't understand */
            if (compare_char == NULL) {
                continue;
            }

            Glib::ustring compare(_(compare_char));

            if (mergename == compare) {
                Inkscape::GC::release(tomerge);
                tomerge = NULL;
                submenu = menupass;
                break;
            }

            if (mergename < compare) {
                position = menupass->position();
                break;
            }
        } // for menu items
    } // start != NULL

    if (tomerge != NULL) {
        base->appendChild(tomerge);
        Inkscape::GC::release(tomerge);
        if (position != -1)
            tomerge->setPosition(position);
    }

    if (patern != NULL) {
        if (submenu == NULL)
            submenu = tomerge;
        merge_menu(submenu, submenu->firstChild(), patern->firstChild(), mergee);
    }

    return;
}

Effect::~Effect (void)
{
    if (get_last_effect() == this)
        set_last_effect(NULL);
    return;
}

bool
Effect::check (void)
{
    if (!Extension::check()) {
        /** \todo  Check to see if parent has this as its only child,
                   if so, delete it too */
        if (_menu_node != NULL)
            sp_repr_unparent(_menu_node);
        _menu_node = NULL;
        return false;
    }
    return true;
}

bool
Effect::prefs (Inkscape::UI::View::View * doc)
{
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return false;

    Gtk::Widget * controls;
    controls = imp->prefs_effect(this, doc);
    if (controls == NULL) {
        // std::cout << "No preferences for Effect" << std::endl;
        return true;
    }

    PrefDialog * dialog = new PrefDialog(this->get_name(), this->get_help(), controls);
    int response = dialog->run();
    dialog->hide();

    delete dialog;

    if (response == Gtk::RESPONSE_OK) return true;

    return false;
}

/**
    \brief  The function that 'does' the effect itself
    \param  doc  The Inkscape::UI::View::View to do the effect on

    This function first insures that the extension is loaded, and if not,
    loads it.  It then calls the implemention to do the actual work.  It
    also resets the last effect pointer to be this effect.  Finally, it
    executes a \c sp_document_done to commit the changes to the undo
    stack.
*/
void
Effect::effect (Inkscape::UI::View::View * doc)
{
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return;

    set_last_effect(this);
    imp->effect(this, doc);

    sp_document_done(doc->doc(), SP_VERB_NONE, 
                     /* TODO: annotate */ "effect.cpp:226");

    return;
}

void
Effect::set_last_effect (Effect * in_effect)
{
    if (in_effect == NULL) {
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST)->sensitive(NULL, false);
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST_PREF)->sensitive(NULL, false);
    } else if (_last_effect == NULL) {
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST)->sensitive(NULL, true);
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST_PREF)->sensitive(NULL, true);
    }

    _last_effect = in_effect;
    return;
}

#define  EFFECTS_LIST  "effects-list"

bool
Effect::find_effects_list (Inkscape::XML::Node * menustruct)
{
    if (menustruct == NULL) return false;
    for (Inkscape::XML::Node * child = menustruct;
            child != NULL;
            child = child->next()) {
        if (!strcmp(child->name(), EFFECTS_LIST)) {
            _effects_list = child;
            return true;
        }
        Inkscape::XML::Node * firstchild = child->firstChild();
        if (firstchild != NULL)
            if (find_effects_list(firstchild))
                return true;
    }
    return false;
}

Gtk::VBox *
Effect::get_info_widget(void)
{
    return Extension::get_info_widget();
}

/** \brief  Create an action for a \c EffectVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
Effect::EffectVerb::make_action (Inkscape::UI::View::View * view)
{
    return make_action_helper(view, &vector, static_cast<void *>(_effect));
}

/** \brief  Decode the verb code and take appropriate action */
void
Effect::EffectVerb::perform (SPAction *action, void * data, void *pdata)
{
    Inkscape::UI::View::View * current_view = sp_action_get_view(action);
//  SPDocument * current_document = current_view->doc;
    Effect * effect = reinterpret_cast<Effect *>(data);

    if (effect == NULL) return;
    if (current_view == NULL) return;

    // std::cout << "Executing: " << effect->get_name() << std::endl;
    if (effect->prefs(current_view))
        effect->effect(current_view);

    return;
}

/**
 * Action vector to define functions called if a staticly defined file verb
 * is called.
 */
SPActionEventVector Effect::EffectVerb::vector =
            {{NULL}, Effect::EffectVerb::perform, NULL, NULL, NULL, NULL};


} }  /* namespace Inkscape, Extension */

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

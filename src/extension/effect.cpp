/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "prefdialog.h"
#include "inkscape.h"
#include "helper/action.h"
#include "ui/view/view.h"
#include "desktop-handles.h"
#include "selection.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "implementation/implementation.h"
#include "effect.h"
#include "execution-env.h"
#include "timer.h"



/* Inkscape::Extension::Effect */

namespace Inkscape {
namespace Extension {

Effect * Effect::_last_effect = NULL;
Inkscape::XML::Node * Effect::_effects_list = NULL;
Inkscape::XML::Node * Effect::_filters_list = NULL;

#define  EFFECTS_LIST  "effects-list"
#define  FILTERS_LIST  "filters-list"

Effect::Effect (Inkscape::XML::Node * in_repr, Implementation::Implementation * in_imp)
    : Extension(in_repr, in_imp),
      _id_noprefs(Glib::ustring(get_id()) + ".noprefs"),
      _name_noprefs(Glib::ustring(_(get_name())) + _(" (No preferences)")),
      _verb(get_id(), get_name(), NULL, NULL, this, true),
      _verb_nopref(_id_noprefs.c_str(), _name_noprefs.c_str(), NULL, NULL, this, false),
      _menu_node(NULL), _workingDialog(true),
      _prefDialog(NULL)
{
    Inkscape::XML::Node * local_effects_menu = NULL;

    // This is a weird hack
    if (!strcmp(this->get_id(), "org.inkscape.filter.dropshadow"))
        return;

    bool hidden = false;

    no_doc = false;
    no_live_preview = false;

    if (repr != NULL) {

        for (Inkscape::XML::Node *child = repr->firstChild(); child != NULL; child = child->next()) {
            if (!strcmp(child->name(), INKSCAPE_EXTENSION_NS "effect")) {
                if (child->attribute("needs-document") && !strcmp(child->attribute("needs-document"), "false")) {
                  no_doc = true;
                }
                if (child->attribute("needs-live-preview") && !strcmp(child->attribute("needs-live-preview"), "false")) {
                  no_live_preview = true;
                }
                for (Inkscape::XML::Node *effect_child = child->firstChild(); effect_child != NULL; effect_child = effect_child->next()) {
                    if (!strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "effects-menu")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        local_effects_menu = effect_child->firstChild();
                        if (effect_child->attribute("hidden") && !strcmp(effect_child->attribute("hidden"), "true")) {
                            hidden = true;
                        }
                    }
                    if (!strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "menu-name") ||
                            !strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "_menu-name")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        _verb.set_name(effect_child->firstChild()->content());
                    }
                    if (!strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "menu-tip") ||
                            !strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "_menu-tip")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        _verb.set_tip(effect_child->firstChild()->content());
                    }
                } // children of "effect"
                break; // there can only be one effect
            } // find "effect"
        } // children of "inkscape-extension"
    } // if we have an XML file

    // \TODO this gets called from the Inkscape::Application constructor, where it initializes the menus.
    // But in the constructor, our object isn't quite there yet!
    if (Inkscape::Application::exists() && INKSCAPE.use_gui()) {
        if (_effects_list == NULL)
            _effects_list = find_menu(INKSCAPE.get_menus(), EFFECTS_LIST);
        if (_filters_list == NULL)
            _filters_list = find_menu(INKSCAPE.get_menus(), FILTERS_LIST);
    }

    if ((_effects_list != NULL || _filters_list != NULL)) {
        Inkscape::XML::Document *xml_doc;
        xml_doc = _effects_list->document();
        _menu_node = xml_doc->createElement("verb");
        _menu_node->setAttribute("verb-id", this->get_id(), false);

        if (!hidden) {
            if (_filters_list &&
                local_effects_menu && 
                local_effects_menu->attribute("name") && 
                !strcmp(local_effects_menu->attribute("name"), ("Filters"))) {
                merge_menu(_filters_list->parent(), _filters_list, local_effects_menu->firstChild(), _menu_node);
            } else if (_effects_list) {
                merge_menu(_effects_list->parent(), _effects_list, local_effects_menu, _menu_node);
            }
        }
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
        
        Inkscape::XML::Document *xml_doc;
        xml_doc = base->document();
        tomerge = xml_doc->createElement("submenu");
        tomerge->setAttribute("name", menuname, false);

        mergename = _(menuname);
    }

    int position = -1;

    if (start != NULL) {
        Inkscape::XML::Node * menupass;
        for (menupass = start; menupass != NULL && strcmp(menupass->name(), "separator"); menupass = menupass->next()) {
            gchar const * compare_char = NULL;
            if (!strcmp(menupass->name(), "verb")) {
                gchar const * verbid = menupass->attribute("verb-id");
                Inkscape::Verb * verb = Inkscape::Verb::getbyid(verbid);
                if (verb == NULL) {
					g_warning("Unable to find verb '%s' which is referred to in the menus.", verbid);
                    continue;
                }
                compare_char = verb->get_name();
            } else if (!strcmp(menupass->name(), "submenu")) {
                compare_char = menupass->attribute("name");
                if (compare_char == NULL)
                    compare_char = menupass->attribute("_name");
            }

            position = menupass->position() + 1;

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
    if (_menu_node)
        Inkscape::GC::release(_menu_node);
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
    if (_prefDialog != NULL) {
        _prefDialog->raise();
        return true;
    }

    if (param_visible_count() == 0) {
        effect(doc);
        return true;
    }

    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return false;

    _prefDialog = new PrefDialog(this->get_name(), this->get_help(), NULL, this);
    _prefDialog->show();

    return true;
}

/**
    \brief  The function that 'does' the effect itself
    \param  doc  The Inkscape::UI::View::View to do the effect on

    This function first insures that the extension is loaded, and if not,
    loads it.  It then calls the implemention to do the actual work.  It
    also resets the last effect pointer to be this effect.  Finally, it
    executes a \c SPDocumentUndo::done to commit the changes to the undo
    stack.
*/
void
Effect::effect (Inkscape::UI::View::View * doc)
{
    //printf("Execute effect\n");
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return;


    ExecutionEnv executionEnv(this, doc);
    timer->lock();
    executionEnv.run();
    if (executionEnv.wait()) {
        executionEnv.commit();
    } else {
        executionEnv.cancel();
    }
    timer->unlock();

    return;
}

/** \brief  Sets which effect was called last
    \param in_effect  The effect that has been called
    
    This function sets the static variable \c _last_effect and it
    ensures that the last effect verb is sensitive.

    If the \c in_effect variable is \c NULL then the last effect
    verb is made insesitive.
*/
void
Effect::set_last_effect (Effect * in_effect)
{
    if (in_effect) {
        gchar const * verb_id = in_effect->get_verb()->get_id();
        gchar const * help_id_prefix = "org.inkscape.help.";

        // We don't want these "effects" to register as the last effect,
        // this wouldn't be helpful to the user who selects a real effect,
        // then goes to the help file (implemented as an effect), then goes
        // back to the effect, only to see it written over by the help file
        // selection.

        // This snippet should fix this bug:
        // https://bugs.launchpad.net/inkscape/+bug/600671
        if (strncmp(verb_id, help_id_prefix, strlen(help_id_prefix)) == 0) return;
    }

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

Inkscape::XML::Node *
Effect::find_menu (Inkscape::XML::Node * menustruct, const gchar *name)
{
    if (menustruct == NULL) return NULL;
    for (Inkscape::XML::Node * child = menustruct;
            child != NULL;
            child = child->next()) {
        if (!strcmp(child->name(), name)) {
            return child;
        }
        Inkscape::XML::Node * firstchild = child->firstChild();
        if (firstchild != NULL) {
            Inkscape::XML::Node *found = find_menu (firstchild, name);
            if (found)
                return found;
        }
    }
    return NULL;
}


Gtk::VBox *
Effect::get_info_widget(void)
{
    return Extension::get_info_widget();
}

void
Effect::set_pref_dialog (PrefDialog * prefdialog)
{
    _prefDialog = prefdialog;
    return;
}

SPAction *
Effect::EffectVerb::make_action (Inkscape::ActionContext const & context)
{
    return make_action_helper(context, &perform, static_cast<void *>(this));
}

/** \brief  Decode the verb code and take appropriate action */
void
Effect::EffectVerb::perform( SPAction *action, void * data )
{
    g_return_if_fail(ensure_desktop_valid(action));
    Inkscape::UI::View::View * current_view = sp_action_get_view(action);

    Effect::EffectVerb * ev = reinterpret_cast<Effect::EffectVerb *>(data);
    Effect * effect = ev->_effect;

    if (effect == NULL) return;

    if (ev->_showPrefs) {
        effect->prefs(current_view);
    } else {
        effect->effect(current_view);
    }

    return;
}

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

/** \file
 * Notebook and NotebookPage parameters for extensions.
 */

/*
 * Authors:
 *   Johan Engelen <johan@shouraizou.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2006 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/notebook.h>

#include <glibmm/i18n.h>

#include <xml/node.h>

#include <extension/extension.h>
#include "preferences.h"
#include "document-private.h"
#include "sp-object.h"

#include "notebook.h"

/**
 * The root directory in the preferences database for extension
 * related parameters.
 */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {


/**
 * A class to represent the pages of a notebookparameter of an extension.
 */
class ParamNotebookPage : public Parameter {
private:
    GSList * parameters; /**< A table to store the parameters for this page.
                              This only gets created if there are parameters on this
                              page */

public:
    static ParamNotebookPage * makepage (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext);

    ParamNotebookPage(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    ~ParamNotebookPage(void);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void paramString (std::list <std::string> &list);
    gchar * get_guitext (void) {return _text;};
    Parameter * get_param (const gchar * name);
}; /* class ParamNotebookPage */


ParamNotebookPage::ParamNotebookPage (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext)
{
    parameters = NULL;

    // Read XML to build page
    if (xml != NULL) {
        Inkscape::XML::Node *child_repr = xml->firstChild();
        while (child_repr != NULL) {
            char const * chname = child_repr->name();
            if (!strncmp(chname, INKSCAPE_EXTENSION_NS_NC, strlen(INKSCAPE_EXTENSION_NS_NC))) {
                chname += strlen(INKSCAPE_EXTENSION_NS);
            }
            if (chname[0] == '_') // Allow _ for translation of tags
                chname++;
            if (!strcmp(chname, "param") || !strcmp(chname, "_param")) {
                Parameter * param;
                param = Parameter::make(child_repr, ext);
                if (param != NULL) parameters = g_slist_append(parameters, param);
            }
            child_repr = child_repr->next();
        }
    }
}

ParamNotebookPage::~ParamNotebookPage (void)
{
    //destroy parameters
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        delete param;
    }
    g_slist_free(parameters);
}

/** Return the value as a string. */
void ParamNotebookPage::paramString(std::list <std::string> &list)
{
    for (GSList * plist = parameters; plist != NULL; plist = g_slist_next(plist)) {
        Parameter * param = reinterpret_cast<Parameter *>(plist->data);
        param->string(list);
    }
}


/**
    \return None
    \brief  This function creates a page that can be used later.  This
            is typically done in the creation of the notebook and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  in_repr  The XML describing the page
    \todo   the 'gui-hidden' attribute is read but not used!

    This function first grabs all of the data out of the Repr and puts
    it into local variables.  Actually, these are just pointers, and the
    data is not duplicated so we need to be careful with it.  If there
    isn't a name in the XML, then no page is created as
    the function just returns.

    From this point on, we're pretty committed as we've allocated an
    object and we're starting to fill it.  The name is set first, and
    is created with a strdup to actually allocate memory for it.  Then
    there is a case statement (roughly because strcmp requires 'ifs')
    based on what type of parameter this is.  Depending which type it
    is, the value is interpreted differently, but they are relatively
    straight forward.  In all cases the value is set to the default
    value from the XML and the type is set to the interpreted type.
*/
ParamNotebookPage *
ParamNotebookPage::makepage (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext)
{
    const char * name;
    const char * guitext;
    const char * desc;
    const char * scope_str;
    Parameter::_scope_t scope = Parameter::SCOPE_USER;
    bool gui_hidden = false;
    const char * gui_hide;
    const char * gui_tip;

    name = in_repr->attribute("name");
    guitext = in_repr->attribute("gui-text");
    if (guitext == NULL)
        guitext = in_repr->attribute("_gui-text");
    gui_tip = in_repr->attribute("gui-tip");
    if (gui_tip == NULL)
        gui_tip = in_repr->attribute("_gui-tip");
    desc = in_repr->attribute("gui-description");
    if (desc == NULL)
        desc = in_repr->attribute("_gui-description");
    scope_str = in_repr->attribute("scope");
    gui_hide = in_repr->attribute("gui-hidden");
    if (gui_hide != NULL) {
        if (strcmp(gui_hide, "1") == 0 ||
            strcmp(gui_hide, "true") == 0) {
                gui_hidden = true;
        }
        /* else stays false */
    }

    /* In this case we just don't have enough information */
    if (name == NULL) {
        return NULL;
    }

    if (scope_str != NULL) {
        if (!strcmp(scope_str, "user")) {
            scope = Parameter::SCOPE_USER;
        } else if (!strcmp(scope_str, "document")) {
            scope = Parameter::SCOPE_DOCUMENT;
        } else if (!strcmp(scope_str, "node")) {
            scope = Parameter::SCOPE_NODE;
        }
    }

    ParamNotebookPage * page = new ParamNotebookPage(name, guitext, desc, scope, gui_hidden, gui_tip, in_ext, in_repr);

    /* Note: page could equal NULL */
    return page;
}



/**
 * Creates a notebookpage widget for a notebook.
 *
 * Builds a notebook page (a vbox) and puts parameters on it.
 */
Gtk::Widget * ParamNotebookPage::get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    if (_gui_hidden) {
        return NULL;
    }

    Gtk::VBox * vbox = Gtk::manage(new Gtk::VBox);
    vbox->set_border_width(5);

    // add parameters onto page (if any)
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        Gtk::Widget * widg = param->get_widget(doc, node, changeSignal);
        if (widg) {
            gchar const * tip = param->get_tooltip();
    //        printf("Tip: '%s'\n", tip);
            vbox->pack_start(*widg, false, false, 2);
            if (tip) {
                widg->set_tooltip_text(_(tip));
            } else {
                widg->set_tooltip_text("");
                widg->set_has_tooltip(false);
            }
        }
    }

    vbox->show();

    return dynamic_cast<Gtk::Widget *>(vbox);
}


ParamNotebook::ParamNotebook (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext)
{
    pages = NULL;

    // Read XML tree to add pages:
    if (xml != NULL) {
        Inkscape::XML::Node *child_repr = xml->firstChild();
        while (child_repr != NULL) {
            char const * chname = child_repr->name();
            if (!strncmp(chname, INKSCAPE_EXTENSION_NS_NC, strlen(INKSCAPE_EXTENSION_NS_NC))) {
                chname += strlen(INKSCAPE_EXTENSION_NS);
            }
            if (chname[0] == '_') // Allow _ for translation of tags
                chname++;
            if (!strcmp(chname, "page")) {
                ParamNotebookPage * page;
                page = ParamNotebookPage::makepage(child_repr, ext);
                if (page != NULL) pages = g_slist_append(pages, page);
            }
            child_repr = child_repr->next();
        }
    }

    // Initialize _value with the current page
    const char * defaultval = NULL;
    // set first page as default
    if (pages != NULL) {
        ParamNotebookPage * defpage = reinterpret_cast<ParamNotebookPage *>(pages->data);
        defaultval = defpage->name();
    }

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring paramval = prefs->getString(extension_pref_root + pref_name);
    g_free(pref_name);

    if (!paramval.empty())
        defaultval = paramval.data();
    if (defaultval != NULL)
        _value = g_strdup(defaultval);  // allocate space for _value
}

ParamNotebook::~ParamNotebook (void)
{
    //destroy pages
    for (GSList * list = pages; list != NULL; list = g_slist_next(list)) {
        ParamNotebookPage * page = reinterpret_cast<ParamNotebookPage *>(list->data);
        delete page;
    }
    g_slist_free(pages);

    g_free(_value);
}


/**
 * A function to set the \c _value.
 *
 * This function sets the internal value, but it also sets the value
 * in the preferences structure.  To put it in the right place, \c PREF_DIR
 * and \c pref_name() are used.
 *
 * To copy the data into _value the old memory must be free'd first.
 * It is important to note that \c g_free handles \c NULL just fine.  Then
 * the passed in value is duplicated using \c g_strdup().
 *
 * @param  in   The number of the page which value must be set.
 * @param  doc  A document that should be used to set the value.
 * @param  node The node where the value may be placed.
 */
const gchar *ParamNotebook::set(const int in, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/)
{
    ParamNotebookPage * page = NULL;
    int i = 0;
    for (GSList * list = pages; (list != NULL) && (i <= in); list = g_slist_next(list)) {
        page = reinterpret_cast<ParamNotebookPage *>(list->data);
        i++;
    }

    if (page == NULL) return _value;

    if (_value != NULL) g_free(_value);
    _value = g_strdup(page->name());

    gchar * prefname = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString(extension_pref_root + prefname, _value);
    g_free(prefname);

    return _value;
}

void ParamNotebook::string(std::list <std::string> &list) const
{
    std::string param_string;
    param_string += "--";
    param_string += name();
    param_string += "=";

    param_string += "\"";
    param_string += _value;  // the name of the current page
    param_string += "\"";
    list.insert(list.end(), param_string);

    for (GSList * pglist = pages; pglist != NULL; pglist = g_slist_next(pglist)) {
        ParamNotebookPage * page = reinterpret_cast<ParamNotebookPage *>(pglist->data);
        page->paramString(list);
    }
}

/** A special category of Gtk::Notebook to handle notebook parameters. */
class ParamNotebookWdg : public Gtk::Notebook {
private:
    ParamNotebook * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
public:
    /**
     * Build a notebookpage preference for the given parameter.
     * @param  pref  Where to get the string (pagename) from, and where to put it
     *               when it changes.
     */
    ParamNotebookWdg (ParamNotebook * pref, SPDocument * doc, Inkscape::XML::Node * node) :
        Gtk::Notebook(), _pref(pref), _doc(doc), _node(node), activated(false) {
        // don't have to set the correct page: this is done in ParamNotebook::get_widget.
        // hook function
        this->signal_switch_page().connect(sigc::mem_fun(this, &ParamNotebookWdg::changed_page));
    };
#if WITH_GTKMM_3_0
    void changed_page(Gtk::Widget *page, guint pagenum);
#else
    void changed_page(GtkNotebookPage *page, guint pagenum);
#endif
    bool activated;
};

/**
 * Respond to the selected page of notebook changing.
 * This function responds to the changing by reporting it to
 * ParamNotebook. The change is only reported when the notebook
 * is actually visible. This to exclude 'fake' changes when the
 * notebookpages are added or removed.
 */
#if WITH_GTKMM_3_0
void ParamNotebookWdg::changed_page(Gtk::Widget * /*page*/, guint pagenum)
#else
void ParamNotebookWdg::changed_page(GtkNotebookPage * /*page*/, guint pagenum)
#endif
{
    if (get_visible()) {
        _pref->set((int)pagenum, _doc, _node);
    }
}

/** Search the parameter's name in the notebook content. */
Parameter *ParamNotebook::get_param(const gchar * name)
{
    if (name == NULL) {
        throw Extension::param_not_exist();
    }
    for (GSList * pglist = pages; pglist != NULL; pglist = g_slist_next(pglist)) {
        ParamNotebookPage * page = reinterpret_cast<ParamNotebookPage *>(pglist->data);
        Parameter * subparam = page->get_param(name);
        if (subparam) {
            return subparam;
        }
    }

    return NULL;
}

/** Search the parameter's name in the page content. */
Parameter *ParamNotebookPage::get_param(const gchar * name)
{
    if (name == NULL) {
        throw Extension::param_not_exist();
    }
    if (this->parameters == NULL) {
        // the list of parameters is empty
        throw Extension::param_not_exist();
    }

    for (GSList * list = this->parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = static_cast<Parameter*>(list->data);
        if (!strcmp(param->name(), name)) {
            return param;
        }
    }

    return NULL;
}

/**
 * Creates a Notebook widget for a notebook parameter.
 *
 * Builds a notebook and puts pages in it.
 */
Gtk::Widget * ParamNotebook::get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    if (_gui_hidden) {
        return NULL;
    }

    ParamNotebookWdg * nb = Gtk::manage(new ParamNotebookWdg(this, doc, node));

    // add pages (if any)
    int i = -1;
    int pagenr = i;
    for (GSList * list = pages; list != NULL; list = g_slist_next(list)) {
        i++;
        ParamNotebookPage * page = reinterpret_cast<ParamNotebookPage *>(list->data);
        Gtk::Widget * widg = page->get_widget(doc, node, changeSignal);
        nb->append_page(*widg, _(page->get_guitext()));
        if (!strcmp(_value, page->name())) {
            pagenr = i; // this is the page to be displayed?
        }
    }

    nb->show();

    if (pagenr >= 0) nb->set_current_page(pagenr);

    return dynamic_cast<Gtk::Widget *>(nb);
}


}  // namespace Extension
}  // namespace Inkscape

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

/** \file
 * Notebook and NotebookPage parameters for extensions.
 */

/*
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/notebook.h>
#include <gtkmm/tooltips.h>

#include <glibmm/i18n.h>

#include <xml/node.h>

#include <extension/extension.h>
#include "preferences.h"
#include "document-private.h"
#include "sp-object.h"

#include "notebook.h"

/** \brief  The root directory in the preferences database for extension
            related parameters. */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {


// \brief  A class to represent the pages of a notebookparameter of an extension
class ParamNotebookPage : public Parameter {
private:
    GSList * parameters; /**< A table to store the parameters for this page.
                              This only gets created if there are parameters on this
                              page */
    Gtk::Tooltips * _tooltips;

public:
    static ParamNotebookPage * makepage (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext);

    ParamNotebookPage(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    ~ParamNotebookPage(void);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void paramString (std::list <std::string> &list);
    gchar * get_guitext (void) {return _text;};

}; /* class ParamNotebookPage */


ParamNotebookPage::ParamNotebookPage (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext)
{
    parameters = NULL;

    // Read XML to build page
    if (xml != NULL) {
        Inkscape::XML::Node *child_repr = sp_repr_children(xml);
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
            child_repr = sp_repr_next(child_repr);
        }
    }

    return;
}

ParamNotebookPage::~ParamNotebookPage (void)
{
    if (_tooltips) delete _tooltips;
    //destroy parameters
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        delete param;
    }
    g_slist_free(parameters);
}

/** \brief  Return the value as a string */
void
ParamNotebookPage::paramString (std::list <std::string> &list)
{
    for (GSList * plist = parameters; plist != NULL; plist = g_slist_next(plist)) {
        Parameter * param = reinterpret_cast<Parameter *>(plist->data);
        param->string(list);
    }

    return;
}


/**
    \return None
    \brief  This function creates a page that can be used later.  This
            is typically done in the creation of the notebook and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  in_repr  The XML describing the page

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

    ParamNotebookPage * page = new ParamNotebookPage(name, guitext, desc, scope, gui_hide, gui_tip, in_ext, in_repr);

    /* Note: page could equal NULL */
    return page;
}



/**
    \brief  Creates a notebookpage widget for a notebook

    Builds a notebook page (a vbox) and puts parameters on it.
*/
Gtk::Widget *
ParamNotebookPage::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
	if (_gui_hidden) return NULL;

    if (!_tooltips) _tooltips = new Gtk::Tooltips();

    Gtk::VBox * vbox = Gtk::manage(new Gtk::VBox);
    vbox->set_border_width(5);

    // add parameters onto page (if any)
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        Gtk::Widget * widg = param->get_widget(doc, node, changeSignal);
        gchar const * tip = param->get_tooltip();

        vbox->pack_start(*widg, true, true, 2);
        if (tip != NULL) {
            _tooltips->set_tip(*widg, Glib::ustring(tip));
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
        Inkscape::XML::Node *child_repr = sp_repr_children(xml);
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
            child_repr = sp_repr_next(child_repr);
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
    Glib::ustring paramval = prefs->getString(PREF_DIR, pref_name);
    g_free(pref_name);

    if (!paramval.empty())
        defaultval = paramval.data();
    if (defaultval != NULL)
        _value = g_strdup(defaultval);  // allocate space for _value

    return;
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


/** \brief  A function to set the \c _value
    \param  in   The number of the page which value must be set
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.

    To copy the data into _value the old memory must be free'd first.
    It is important to note that \c g_free handles \c NULL just fine.  Then
    the passed in value is duplicated using \c g_strdup().
*/
const gchar *
ParamNotebook::set (const int in, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/)
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
    prefs_set_string_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}


/**
    \brief  A function to get the currentpage and the parameters in a string form
    \return A string with the 'value' and all the parameters on all pages as command line arguments
*/
void
ParamNotebook::string (std::list <std::string> &list)
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

    return;
}

/** \brief  A special category of Gtk::Notebook to handle notebook parameters */
class ParamNotebookWdg : public Gtk::Notebook {
private:
    ParamNotebook * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
public:
    /** \brief  Build a notebookpage preference for the given parameter
        \param  pref  Where to get the string (pagename) from, and where to put it
                      when it changes.
    */
    ParamNotebookWdg (ParamNotebook * pref, SPDocument * doc, Inkscape::XML::Node * node) :
        Gtk::Notebook(), _pref(pref), _doc(doc), _node(node), activated(false) {
        // don't have to set the correct page: this is done in ParamNotebook::get_widget.
        // hook function
        this->signal_switch_page().connect(sigc::mem_fun(this, &ParamNotebookWdg::changed_page));
        return;
    };
    void changed_page(GtkNotebookPage *page, guint pagenum);
    bool activated;
};

/** \brief  Respond to the selected page of notebook changing
    This function responds to the changing by reporting it to
    ParamNotebook. The change is only reported when the notebook
    is actually visible. This to exclude 'fake' changes when the
    notebookpages are added or removed.
*/
void
ParamNotebookWdg::changed_page(GtkNotebookPage */*page*/,
                                   guint pagenum)
{
    if (is_visible()) {
        _pref->set((int)pagenum, _doc, _node);
    }
    return;
}



/**
    \brief  Creates a Notebook widget for a notebook parameter

    Builds a notebook and puts pages in it.
*/
Gtk::Widget *
ParamNotebook::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
	if (_gui_hidden) return NULL;

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


}  /* namespace Extension */
}  /* namespace Inkscape */

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

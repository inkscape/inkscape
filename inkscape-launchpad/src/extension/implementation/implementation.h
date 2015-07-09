/*
    Author:  Ted Gould <ted@gould.cx>
    Copyright (c) 2003-2005,2007

    This code is licensed under the GNU GPL.  See COPYING for details.
 
    This file is the backend to the extensions system.  These are
    the parts of the system that most users will never see, but are
    important for implementing the extensions themselves.  This file
    contains the base class for all of that.
*/
#ifndef SEEN_INKSCAPE_EXTENSION_IMPLEMENTATION_H
#define SEEN_INKSCAPE_EXTENSION_IMPLEMENTATION_H

#include <vector>
#include <sigc++/signal.h>
#include <glibmm/value.h>
#include <2geom/forward.h>

namespace Gtk {
	class Widget;
}

class SPDocument;
class SPStyle;

namespace Inkscape {

namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI

namespace XML {
	class Node;
} // namespace XML

namespace Extension {

class Effect;
class Extension;
class Input;
class Output;
class Print;

namespace Implementation {

/**
 * A cache for the document and this implementation.
 */
class ImplementationDocumentCache {

	/**
         * The document that this instance is working on.
         */
	Inkscape::UI::View::View * _view;
public:
	ImplementationDocumentCache (Inkscape::UI::View::View * view) :
			_view(view)
	{
		return;
	};
	virtual ~ImplementationDocumentCache ( ) { return; };
	Inkscape::UI::View::View const * view ( ) { return _view; };
};

/**
 * Base class for all implementations of modules.  This is whether they are done systematically by
 * having something like the scripting system, or they are implemented internally they all derive
 * from this class.
 */
class Implementation {
public:
    // ----- Constructor / destructor -----
    Implementation() {}
    
    virtual ~Implementation() {}

    // ----- Basic functions for all Extension -----
    virtual bool load(Inkscape::Extension::Extension * /*module*/) { return true; }

    virtual void unload(Inkscape::Extension::Extension * /*module*/) {}

    /**
     * Create a new document cache object.
     * This function just returns \c NULL.  Subclasses are likely
     * to reimplement it to do something useful.
     * @param  ext  The extension that is referencing us
     * @param  doc  The document to create the cache of
     * @return A new document cache that is valid as long as the document
     *         is not changed.
     */
    virtual ImplementationDocumentCache * newDocCache (Inkscape::Extension::Extension * /*ext*/, Inkscape::UI::View::View * /*doc*/) { return NULL; }

    /** Verify any dependencies. */
    virtual bool check(Inkscape::Extension::Extension * /*module*/) { return true; }

    virtual bool cancelProcessing () { return true; }
    virtual bool wasCancelled () { return false; }
    virtual void commitDocument () {}

    // ----- Input functions -----
    /** Find out information about the file. */
    virtual Gtk::Widget *prefs_input(Inkscape::Extension::Input *module,
                             gchar const *filename);

    virtual SPDocument *open(Inkscape::Extension::Input * /*module*/,
                             gchar const * /*filename*/) { return NULL; }

    // ----- Output functions -----
    /** Find out information about the file. */
    virtual Gtk::Widget *prefs_output(Inkscape::Extension::Output *module);
    virtual void save(Inkscape::Extension::Output * /*module*/, SPDocument * /*doc*/, gchar const * /*filename*/) {}

    // ----- Effect functions -----
    /** Find out information about the file. */
    virtual Gtk::Widget * prefs_effect(Inkscape::Extension::Effect *module,
	                               Inkscape::UI::View::View *view,
                                       sigc::signal<void> *changeSignal,
                                       ImplementationDocumentCache *docCache);
    virtual void effect(Inkscape::Extension::Effect * /*module*/,
                        Inkscape::UI::View::View * /*document*/,
                        ImplementationDocumentCache * /*docCache*/) {}

    // ----- Print functions -----
    virtual unsigned setup(Inkscape::Extension::Print * /*module*/) { return 0; }
    virtual unsigned set_preview(Inkscape::Extension::Print * /*module*/) { return 0; }

    virtual unsigned begin(Inkscape::Extension::Print * /*module*/,
                           SPDocument * /*doc*/) { return 0; }
    virtual unsigned finish(Inkscape::Extension::Print * /*module*/) { return 0; }

    /**
     * Tell the printing engine whether text should be text or path.
     * Default value is false because most printing engines will support
     * paths more than they'll support text.  (at least they do today)
     * \retval true  Render the text as a path
     * \retval false Render text using the text function (above)
     */
    virtual bool     textToPath(Inkscape::Extension::Print * /*ext*/) { return false; }

    /**
     * Get "fontEmbedded" param, i.e. tell the printing engine whether fonts should be embedded.
     * Only available for Adobe Type 1 fonts in EPS output as of now
     * \retval true Fonts have to be embedded in the output so that the user might not need
     *              to install fonts to have the interpreter read the document correctly
     * \retval false Do not embed fonts
     */
    virtual bool     fontEmbedded(Inkscape::Extension::Print * /*ext*/) { return false; }

    // ----- Rendering methods -----
    virtual unsigned bind(Inkscape::Extension::Print * /*module*/,
                          Geom::Affine const & /*transform*/,
                          float /*opacity*/) { return 0; }
    virtual unsigned release(Inkscape::Extension::Print * /*module*/) { return 0; }
    virtual unsigned comment(Inkscape::Extension::Print * /*module*/, char const * /*comment*/) { return 0; }
    virtual unsigned fill(Inkscape::Extension::Print * /*module*/,
                          Geom::PathVector const & /*pathv*/,
                          Geom::Affine const & /*ctm*/,
                          SPStyle const * /*style*/,
                          Geom::OptRect const & /*pbox*/,
                          Geom::OptRect const & /*dbox*/,
                          Geom::OptRect const & /*bbox*/) { return 0; }
    virtual unsigned stroke(Inkscape::Extension::Print * /*module*/,
                            Geom::PathVector const & /*pathv*/,
                            Geom::Affine const & /*transform*/,
                            SPStyle const * /*style*/,
                            Geom::OptRect const & /*pbox*/,
                            Geom::OptRect const & /*dbox*/,
                            Geom::OptRect const & /*bbox*/) { return 0; }
    virtual unsigned image(Inkscape::Extension::Print * /*module*/,
                           unsigned char * /*px*/,
                           unsigned int /*w*/,
                           unsigned int /*h*/,
                           unsigned int /*rs*/,
                           Geom::Affine const & /*transform*/,
                           SPStyle const * /*style*/) { return 0; }
    virtual unsigned text(Inkscape::Extension::Print * /*module*/,
                          char const * /*text*/,
                          Geom::Point const & /*p*/,
                          SPStyle const * /*style*/) { return 0; }
    virtual void     processPath(Inkscape::XML::Node * /*node*/) {}
};


}  // namespace Implementation
}  // namespace Extension
}  // namespace Inkscape

#endif // __INKSCAPE_EXTENSION_IMPLEMENTATION_H__

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

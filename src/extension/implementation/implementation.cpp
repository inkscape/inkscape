/*
    Author:  Ted Gould <ted@gould.cx>
    Copyright (c) 2003-2005,2007

    This code is licensed under the GNU GPL.  See COPYING for details.

    This file is the backend to the extensions system.  These are
    the parts of the system that most users will never see, but are
    important for implementing the extensions themselves.  This file
    contains the base class for all of that.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "implementation.h"

#include <extension/output.h>
#include <extension/input.h>
#include <extension/effect.h>

#include "selection.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "ui/view/view.h"
#include "util/glib-list-iterators.h"

namespace Inkscape {
namespace Extension {
namespace Implementation {

/**
 * \return   Was the load sucessful?
 * \brief    This function is the stub load.  It just returns success.
 * \param    module   The Extension that should be loaded.
 */
bool
Implementation::load(Inkscape::Extension::Extension */*module*/) {
    return TRUE;
} /* Implementation::load */

void
Implementation::unload(Inkscape::Extension::Extension */*module*/) {
    return;
} /* Implementation::unload */

/** \brief  Create a new document cache object
    \param  ext  The extension that is referencing us
	\param  doc  The document to create the cache of
	\return A new document cache that is valid as long as the document
	        is not changed.

	This function just returns \c NULL.  Subclasses are likely
	to reimplement it to do something useful.
*/
ImplementationDocumentCache *
Implementation::newDocCache( Inkscape::Extension::Extension * /*ext*/, Inkscape::UI::View::View * /*view*/ ) {
    return NULL;
}

bool
Implementation::check(Inkscape::Extension::Extension */*module*/) {
    /* If there are no checks, they all pass */
    return TRUE;
} /* Implemenation::check */

bool
Implementation::cancelProcessing (void) {
    return true;
}

void
Implementation::commitDocument (void) {
    return;
}

Gtk::Widget *
Implementation::prefs_input(Inkscape::Extension::Input *module, gchar const */*filename*/) {
    return module->autogui(NULL, NULL);
} /* Implementation::prefs_input */

SPDocument *
Implementation::open(Inkscape::Extension::Input */*module*/, gchar const */*filename*/) {
    /* throw open_failed(); */
    return NULL;
} /* Implementation::open */

Gtk::Widget *
Implementation::prefs_output(Inkscape::Extension::Output *module) {
    return module->autogui(NULL, NULL);
} /* Implementation::prefs_output */

void
Implementation::save(Inkscape::Extension::Output */*module*/, SPDocument */*doc*/, gchar const */*filename*/) {
    /* throw save_fail */
    return;
} /* Implementation::save */

Gtk::Widget *
Implementation::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal, ImplementationDocumentCache * docCache) {
    if (module->param_visible_count() == 0) return NULL;

    SPDocument * current_document = view->doc();

    using Inkscape::Util::GSListConstIterator;
    GSListConstIterator<SPItem *> selected =
           sp_desktop_selection((SPDesktop *)view)->itemList();
    Inkscape::XML::Node * first_select = NULL;
    if (selected != NULL) {
        const SPItem * item = *selected;
        first_select = SP_OBJECT_REPR(item);
    }

    return module->autogui(current_document, first_select, changeSignal);
} /* Implementation::prefs_effect */

void
Implementation::effect(Inkscape::Extension::Effect */*module*/, Inkscape::UI::View::View */*document*/, ImplementationDocumentCache * /*docCache*/) {
    /* throw filter_fail */
    return;
} /* Implementation::filter */

unsigned int
Implementation::setup(Inkscape::Extension::Print */*module*/)
{
    return 0;
}

unsigned int
Implementation::set_preview(Inkscape::Extension::Print */*module*/)
{
    return 0;
}


unsigned int
Implementation::begin(Inkscape::Extension::Print */*module*/, SPDocument */*doc*/)
{
    return 0;
}

unsigned int
Implementation::finish(Inkscape::Extension::Print */*module*/)
{
    return 0;
}


/* Rendering methods */
unsigned int
Implementation::bind(Inkscape::Extension::Print */*module*/, NR::Matrix const */*transform*/, float /*opacity*/)
{
    return 0;
}

unsigned int
Implementation::release(Inkscape::Extension::Print */*module*/)
{
    return 0;
}

unsigned int
Implementation::comment(Inkscape::Extension::Print */*module*/, char const */*comment*/)
{
    return 0;
}

unsigned int
Implementation::fill(Inkscape::Extension::Print */*module*/, NRBPath const */*bpath*/, NR::Matrix const */*ctm*/, SPStyle const */*style*/,
                     NRRect const */*pbox*/, NRRect const */*dbox*/, NRRect const */*bbox*/)
{
    return 0;
}

unsigned int
Implementation::stroke(Inkscape::Extension::Print */*module*/, NRBPath const */*bpath*/, NR::Matrix const */*transform*/, SPStyle const */*style*/,
                       NRRect const */*pbox*/, NRRect const */*dbox*/, NRRect const */*bbox*/)
{
    return 0;
}

unsigned int
Implementation::image(Inkscape::Extension::Print */*module*/, unsigned char */*px*/, unsigned int /*w*/, unsigned int /*h*/, unsigned int /*rs*/,
                      NR::Matrix const */*transform*/, SPStyle const */*style*/)
{
    return 0;
}

unsigned int
Implementation::text(Inkscape::Extension::Print */*module*/, char const */*text*/,
                     NR::Point /*p*/, SPStyle const */*style*/)
{
    return 0;
}

void
Implementation::processPath(Inkscape::XML::Node * /*node*/)
{
    return;
}

/**
   \brief  Tell the printing engine whether text should be text or path
   \retval true  Render the text as a path
   \retval false Render text using the text function (above)

    Default value is false because most printing engines will support
    paths more than they'll support text.  (at least they do today)
*/
bool
Implementation::textToPath(Inkscape::Extension::Print */*ext*/)
{
    return false;
}

/**
   \brief Get "fontEmbedded" param, i.e. tell the printing engine whether fonts should be embedded
   \retval TRUE Fonts have to be embedded in the output so that the user might not need to install fonts to have the interpreter read the document correctly
   \retval FALSE Not embed fonts

   Only available for Adobe Type 1 fonts in EPS output as of now
*/

bool
Implementation::fontEmbedded(Inkscape::Extension::Print * /*ext*/)
{
    return false;
}

}  /* namespace Implementation */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

/** \file
 * Inkscape UI action context implementation
 *//*
 * Author:
 *   Eric Greveson <eric@greveson.co.uk>
 *
 * Copyright (C) 2013 Eric Greveson
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_ACTION_CONTEXT_H
#define SEEN_INKSCAPE_ACTION_CONTEXT_H

class SPDesktop;
class SPDocument;

namespace Inkscape {

class Selection;

namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI

/** This structure contains all the document/view context required
  for an action. Some actions may be executed on a document without
  requiring a GUI, hence not providing the info directly through
  Inkscape::UI::View::View. Actions that do require GUI objects should
  check to see if the relevant pointers are NULL before attempting to
  use them.

  TODO: we store a UI::View::View* because that's what the actions and verbs
  used to take as parameters in their methods. Why is this? They almost
  always seemed to cast straight to an SPDesktop* - so shouldn't we actually
  be storing an SPDesktop*? Is there a case where a non-SPDesktop
  UI::View::View is used by the actions?
  
  ActionContext is designed to be copyable, so it may be used with stack
  storage if required. */
class ActionContext {
    // NB: Only one of these is typically set - selection model if in console mode, view if in GUI mode
    Selection *_selection;  /**< The selection model to which this action applies, if running in console mode. May be NULL. */
    UI::View::View *_view;  /**< The view to which this action applies. May be NULL (e.g. if running in console mode). */

public:
    /** Construct without any document or GUI */
    ActionContext();

    /** Construct an action context for when the app is being run without
      any GUI, i.e. in console mode */
    ActionContext(Selection *selection);
    
    /** Construct an action context for when the app is being run in GUI mode */
    ActionContext(UI::View::View *view);

    /** Get the document for the action context. May be NULL. Prefer this
      function to getView()->doc() if the action doesn't require a GUI. */
    SPDocument *getDocument() const;

    /** Get the selection for the action context. May be NULL. Should be 
      non-NULL if getDocument() is non-NULL. */
    Selection *getSelection() const;

    /** Get the view for the action context. May be NULL. Guaranteed to be
      NULL if running in console mode. */
    UI::View::View *getView() const;

    /** Get the desktop for the action context. May be NULL. Guaranteed to be
      NULL if running in console mode. */
    SPDesktop *getDesktop() const;
};

} // namespace Inkscape

#endif

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

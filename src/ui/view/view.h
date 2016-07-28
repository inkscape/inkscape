#ifndef INKSCAPE_UI_VIEW_VIEW_H
#define INKSCAPE_UI_VIEW_VIEW_H
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include <stddef.h>
#include <sigc++/connection.h>
#include "message.h"
#include "inkgc/gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include <2geom/forward.h>

/**
 * Iterates until true or returns false.
 * When used as signal accumulator, stops emission if one slot returns true.
 */
struct StopOnTrue {
  typedef bool result_type;

  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const{
      for (; first != last; ++first)
          if (*first) return true;
      return false;
  }
};

/**
 * Iterates until nonzero or returns 0.
 * When used as signal accumulator, stops emission if one slot returns nonzero.
 */
struct StopOnNonZero {
  typedef int result_type;

  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const{
      for (; first != last; ++first)
          if (*first) return *first;
      return 0;
  }
};

class SPDocument;

namespace Inkscape {
    class MessageContext;
    class MessageStack;
    namespace UI {
        namespace View {

/**
 * View is an abstract base class of all UI document views.  This
 * includes both the editing window and the SVG preview, but does not
 * include the non-UI RGBA buffer-based Inkscape::Drawing nor the XML editor or
 * similar views.  The View base class has very little functionality of
 * its own.
 */
class View : public GC::Managed<>,
             public GC::Finalized,
             public GC::Anchored
{
public:

    View();

    /**
     * Deletes and nulls all View message stacks and disconnects it from signals.
     */
    virtual ~View();

    void close() { _close(); }

    /// Returns a pointer to the view's document.
    SPDocument *doc() const
      { return _doc; }
    /// Returns a pointer to the view's message stack.
    Inkscape::MessageStack *messageStack() const
      { return _message_stack; }
    /// Returns a pointer to the view's tipsMessageContext.
    Inkscape::MessageContext *tipsMessageContext() const
      { return _tips_message_context; }

    void emitResized(gdouble width, gdouble height);
    void requestRedraw();

    // view subclasses must give implementations of these methods

    virtual bool shutdown() = 0;
    virtual void mouseover() = 0;
    virtual void mouseout() = 0;

    virtual void onResized (double, double) = 0;
    virtual void onRedrawRequested() = 0;
    virtual void onStatusMessage (Inkscape::MessageType type, gchar const *message) = 0;
    virtual void onDocumentURISet (gchar const* uri) = 0;
    virtual void onDocumentResized (double, double) = 0;

protected:
    SPDocument *_doc;
    Inkscape::MessageStack *_message_stack;
    Inkscape::MessageContext *_tips_message_context;

    virtual void _close();

    /**
     * Disconnects the view from the document signals, connects the view 
     * to a new one, and emits the _document_set_signal on the view.
     *
     * This is code comon to all subclasses and called from their
     * setDocument() methods after they are done.
     * 
     * @param doc The new document to connect the view to.
     */
    virtual void setDocument(SPDocument *doc);

    sigc::signal<void,double,double>   _resized_signal;
    sigc::signal<void,gchar const*>    _document_uri_set_signal;
    sigc::signal<void>                 _redraw_requested_signal;

private:
    sigc::connection _resized_connection;
    sigc::connection _redraw_requested_connection;
    sigc::connection _message_changed_connection;  // foreign
    sigc::connection _document_uri_set_connection; // foreign
    sigc::connection _document_resized_connection; // foreign
};

}}}

#endif  // INKSCAPE_UI_VIEW_VIEW_H

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

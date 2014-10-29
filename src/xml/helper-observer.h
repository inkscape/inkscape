#ifndef SEEN_XML_HELPER_OBSERVER
#define SEEN_XML_HELPER_OBSERVER

#include <cstddef>
#include <sigc++/sigc++.h>

#include "node-observer.h"
#include "node.h"
#include "sp-object.h"

namespace Inkscape {
namespace XML {

class Node;

// Very simple observer that just emits a signal if anything happens to a node
class SignalObserver : public NodeObserver {
public:
    SignalObserver();
    ~SignalObserver();

    // Add this observer to the SPObject and remove it from any previous object
    void set(SPObject* o);
    void notifyChildAdded(Node&, Node&, Node*);
    void notifyChildRemoved(Node&, Node&, Node*);
    void notifyChildOrderChanged(Node&, Node&, Node*, Node*);
    void notifyContentChanged(Node&, Util::ptr_shared<char>, Util::ptr_shared<char>);
    void notifyAttributeChanged(Node&, GQuark, Util::ptr_shared<char>, Util::ptr_shared<char>);
    sigc::signal<void>& signal_changed();
private:
    sigc::signal<void> _signal_changed;
    SPObject* _oldsel;
};

}
}

#endif //#ifndef __XML_HELPER_OBSERVER__

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

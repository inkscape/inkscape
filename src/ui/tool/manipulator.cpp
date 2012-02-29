/** @file
 * Manipulator base class and manipulator group - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/tool/node.h"
#include "ui/tool/manipulator.h"

namespace Inkscape {
namespace UI {

/*
void Manipulator::_grabEvents()
{
    if (_group) _group->_grabEvents(boost::shared_ptr<Manipulator>(this));
}
void Manipulator::_ungrabEvents()
{
    if (_group) _group->_ungrabEvents(boost::shared_ptr<Manipulator>(this));
}

ManipulatorGroup::ManipulatorGroup(SPDesktop *d) :
    _desktop(d)
{
}
ManipulatorGroup::~ManipulatorGroup()
{
}

void ManipulatorGroup::_grabEvents(boost::shared_ptr<Manipulator> m)
{
    if (!_grab) _grab = m;
}
void ManipulatorGroup::_ungrabEvents(boost::shared_ptr<Manipulator> m)
{
    if (_grab == m) _grab.reset();
}

void ManipulatorGroup::add(boost::shared_ptr<Manipulator> m)
{
    m->_group = this;
    push_back(m);
}
void ManipulatorGroup::remove(boost::shared_ptr<Manipulator> m)
{
    for (std::list<boost::shared_ptr<Manipulator> >::iterator i = begin(); i != end(); ++i) {
        if ((*i) == m) {
            erase(i);
            break;
        }
    }
    m->_group = 0;
}

void ManipulatorGroup::clear()
{
    std::list<boost::shared_ptr<Manipulator> >::clear();
}

bool ManipulatorGroup::event(GdkEvent *event)
{
    if (_grab) {
        return _grab->event(event);
    }
    
    for (std::list<boost::shared_ptr<Manipulator> >::iterator i = begin(); i != end(); ++i) {
        if ((*i)->event(event) || _grab) return true;
    }
    return false;
}*/

} // namespace UI
} // namespace Inkscape

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

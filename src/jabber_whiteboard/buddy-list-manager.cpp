/**
 * Whiteboard session manager
 * Buddy list management facility
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <sigc++/sigc++.h>
#include <set>

#include "jabber_whiteboard/typedefs.h"
#include "jabber_whiteboard/buddy-list-manager.h"

namespace Inkscape {

namespace Whiteboard {

BuddyListManager::BuddyListManager() { }
BuddyListManager::~BuddyListManager() { }

void
BuddyListManager::addInsertListener(BuddyListListener listener)
{
	this->_sig_insert.connect(listener);
}

void
BuddyListManager::addEraseListener(BuddyListListener listener)
{
	this->_sig_erase.connect(listener);
}

void
BuddyListManager::insert(std::string& jid)
{
	this->_bl.insert(jid);
	this->_sig_insert.emit(jid);
}

void
BuddyListManager::erase(std::string& jid)
{
	this->_bl.erase(jid);
	this->_sig_erase.emit(jid);
}

BuddyList::iterator
BuddyListManager::begin()
{
	return this->_bl.begin();
}

BuddyList::iterator
BuddyListManager::end()
{
	return this->_bl.end();
}

BuddyList&
BuddyListManager::getList()
{
	return this->_bl;
}

}

}

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

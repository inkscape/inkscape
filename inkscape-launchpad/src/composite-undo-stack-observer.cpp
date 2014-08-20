/*
 * Heavily inspired by Inkscape::XML::CompositeNodeObserver.
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>

#include "composite-undo-stack-observer.h"
#include "xml/event.h"

namespace Inkscape {

CompositeUndoStackObserver::CompositeUndoStackObserver() : _iterating(0) { }
CompositeUndoStackObserver::~CompositeUndoStackObserver() { }

void
CompositeUndoStackObserver::add(UndoStackObserver& observer)
{
	if (!this->_iterating) {
		this->_active.push_back(UndoStackObserverRecord(observer));
	} else {
		this->_pending.push_back(UndoStackObserverRecord(observer));
	}
}

void
CompositeUndoStackObserver::remove(UndoStackObserver& observer)
{
	if (!this->_iterating) {
		// logical-or operator short-circuits
		this->_remove_one(this->_active, observer) || this->_remove_one(this->_pending, observer);
	} else {
		this->_mark_one(this->_active, observer) || this->_mark_one(this->_pending, observer);
	}
}

void
CompositeUndoStackObserver::notifyUndoEvent(Event* log)
{
	this->_lock();
	for(UndoObserverRecordList::iterator i = this->_active.begin(); i != _active.end(); ++i) {
		if (!i->to_remove) {
			i->issueUndo(log);
		}
	}
	this->_unlock();
}

void
CompositeUndoStackObserver::notifyRedoEvent(Event* log)
{

	this->_lock();
	for(UndoObserverRecordList::iterator i = this->_active.begin(); i != _active.end(); ++i) {
		if (!i->to_remove) {
			i->issueRedo(log);
		}
	}
	this->_unlock();
}

void
CompositeUndoStackObserver::notifyUndoCommitEvent(Event* log)
{
	this->_lock();
	for(UndoObserverRecordList::iterator i = this->_active.begin(); i != _active.end(); ++i) {
		if (!i->to_remove) {
			i->issueUndoCommit(log);
		}
	}
	this->_unlock();
}

void
CompositeUndoStackObserver::notifyClearUndoEvent()
{
	this->_lock();
	for(UndoObserverRecordList::iterator i = this->_active.begin(); i != _active.end(); ++i) {
		if (!i->to_remove) {
			i->issueClearUndo();
		}
	}
	this->_unlock();
}

void
CompositeUndoStackObserver::notifyClearRedoEvent()
{
	this->_lock();
	for(UndoObserverRecordList::iterator i = this->_active.begin(); i != _active.end(); ++i) {
		if (!i->to_remove) {
			i->issueClearRedo();
		}
	}
	this->_unlock();
}

bool
CompositeUndoStackObserver::_remove_one(UndoObserverRecordList& list, UndoStackObserver& o)
{
	UndoStackObserverRecord eq_comp(o);

	UndoObserverRecordList::iterator i = std::find_if(list.begin(), list.end(), std::bind1st(std::equal_to< UndoStackObserverRecord >(), eq_comp));

	if (i != list.end()) {
		list.erase(i);
		return true;
	} else {
		return false;
	}
}

bool
CompositeUndoStackObserver::_mark_one(UndoObserverRecordList& list, UndoStackObserver& o)
{
	UndoStackObserverRecord eq_comp(o);

	UndoObserverRecordList::iterator i = std::find_if(list.begin(), list.end(), std::bind1st(std::equal_to< UndoStackObserverRecord >(), eq_comp));

	if (i != list.end()) {
		(*i).to_remove = true;
		return true;
	} else {
		return false;
	}
}

void
CompositeUndoStackObserver::_unlock()
{
	if (!--this->_iterating) {
		// Remove marked observers
		UndoObserverRecordList::iterator i = this->_active.begin();
		for(; i != this->_active.begin(); ) {
			if (i->to_remove) {
				i = this->_active.erase(i);
			}
			else{
				++i;
			}
		}

		i = this->_pending.begin();
		for(; i != this->_pending.begin(); ) {
			if (i->to_remove) {
				i = this->_active.erase(i);
			}
			else {
				++i;
			}
		}

		// Merge pending and active
		this->_active.insert(this->_active.end(), this->_pending.begin(), this->_pending.end());
		this->_pending.clear();
	}
}

}

/**
 * Aggregates individual serialized XML::Events into larger packages 
 * for more efficient delivery
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __WHITEBOARD_MESSAGE_AGGREGATOR_H__
#define __WHITEBOARD_MESSAGE_AGGREGATOR_H__

#include <glibmm.h>

namespace Inkscape {

namespace Whiteboard {

/**
 * Aggregates individual serialized XML::Events into larger messages for increased
 * efficiency.
 *
 * \see Inkscape::Whiteboard::Serializer
 */
class MessageAggregator {
public:
	// TODO: This should be user-configurable; perhaps an option in Inkscape Preferences...
	/// Maximum size of aggregates in kilobytes; ULONG_MAX = no limit.
	static unsigned int const MAX_SIZE = 16384;

	MessageAggregator() { }
    virtual ~MessageAggregator() { }

	/**
	 * Return the instance of this class.
	 *
	 * \return MessageAggregator instance.
	 */
	static MessageAggregator& instance()
	{
		static MessageAggregator singleton;
		return singleton;
	}

	/**
	 * Adds one message to the aggregate 
	 * using a user-provided buffer.  Returns true if more messages can be
	 * added to the buffer; false otherwise.
	 *
	 * \param msg The message to add to the aggregate.
	 * \param buf The aggregate buffer.
	 * \return Whether or not more messages can be added to the buffer.
	 */
	bool addOne(Glib::ustring const& msg, Glib::ustring& buf);

	/**
	 * Adds one message to the aggregate using the internal buffer.  
	 * Note that since this class is designed to be a singleton class, usage of the internal
	 * buffer is not thread-safe.  Use the above method if this matters to you
	 * (it currently shouldn't matter, but in future...)
	 *
	 * Also note that usage of the internal buffer means that you will have to manually
	 * clear the internal buffer; use reset() for that.
	 *
	 * \param msg The message to add to the aggregate.
	 * \return Whether or not more messages can be added to the buffer.
	 */
	bool addOne(Glib::ustring const& msg);

	/**
	 * Return the aggregate message.
	 *
	 * Because this method returns a reference to a string, it is not safe to assume
         * that its contents will remain untouched across two calls to this MessageAggregator.
         * If you require that guarantee, make a copy.
	 *
	 * \return A reference to the aggregate message.
	 */
	Glib::ustring const& getAggregate()
	{
		return this->_buf;
	}

	/**
	 * Return the aggregate message.
	 *
	 * \return The aggregate message.
	 */
	Glib::ustring const getAggregateCopy()
	{
		return this->_buf;
	}

	/**
	 * Return the aggregate message and clear the internal buffer.
	 *
	 * \return The aggregate message.
	 */
	Glib::ustring const detachAggregate()
	{
		Glib::ustring ret = this->_buf;
		this->_buf.clear();
		return ret;
	}

	/**
	 * Clear the internal buffer.
	 */
	void reset()
	{
		this->_buf.clear();
	}

private:
	Glib::ustring _buf;
};

}

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

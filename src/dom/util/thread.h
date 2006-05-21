#ifndef __DOM_THREAD_H__
#define __DOM_THREAD_H__
/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * Thread wrapper.  This provides a platform-independent thread
 * class for IO and testing.
 *
 */


namespace org
{
namespace w3c
{
namespace dom
{
namespace util
{


/**
 * This is the interface for a delegate class which can
 * be run by a Thread.
 * Thread thread(runnable);
 * thread.start();
 */
class Runnable
{
public:

    Runnable()
        {}
    virtual ~Runnable()
        {}

    /**
     * The method of a delegate class which can
     * be run by a Thread.  Thread is completed when this
     * method is done.
     */
    virtual void run() = 0;

};



/**
 *  A simple wrapper of native threads in a portable class.
 *  It can be used either to execute its own run() method, or
 *  delegate to a Runnable class's run() method.
 */
class Thread
{
public:

    /**
     *  Create a thread which will execute its own run() method.
     */
    Thread()
        { runnable = (Runnable *)0 ; started = false; }

    /**
     * Create a thread which will run a Runnable class's run() method.
     */
    Thread(const Runnable &runner)
        { runnable = (Runnable *)&runner; started = false; }

    /**
     *  This does not kill a spawned thread.
     */
    virtual ~Thread()
        {}

    /**
     *  Static method to pause the current thread for a given
     *  number of milliseconds.
     */
    static void sleep(unsigned long millis);

    /**
     *  This method will be executed if the Thread was created with
     *  no delegated Runnable class.  The thread is completed when
     *  the method is done.
     */
    virtual void run()
        {}

    /**
     *  Starts the thread.
     */
    virtual void start();

    /**
     *  Calls either this class's run() method, or that of a Runnable.
     *  A user would normally not call this directly.
     */
    virtual void execute()
        {
        started = true;
        if (runnable)
            runnable->run();
        else
            run();
        }

private:

    Runnable *runnable;

    bool started;

};


}  //namespace util
}  //namespace dom
}  //namespace w3c
}  //namespace org



#endif /* __DOM_THREAD_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################



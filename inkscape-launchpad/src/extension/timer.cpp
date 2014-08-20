/*
 * Here is where the extensions can get timed on when they load and
 * unload.  All of the timing is done in here.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/main.h>

#include "extension.h"
#include "timer.h"

namespace Inkscape {
namespace Extension {

#define TIMER_SCALE_VALUE  20

ExpirationTimer * ExpirationTimer::timer_list = NULL;
ExpirationTimer * ExpirationTimer::idle_start = NULL;
long ExpirationTimer::timeout = 240;
bool ExpirationTimer::timer_started = false;

/** \brief  Create a new expiration timer
    \param  in_extension  Which extension this timer is related to

    This function creates the timer, and sets the time to the current
    time, plus what ever the current timeout is.  Also, if this is
    the first timer extension, the timer is kicked off.  This function
    also sets up the circularly linked list of all the timers.
*/
ExpirationTimer::ExpirationTimer (Extension * in_extension):
    locked(0),
    extension(in_extension)
{
    /* Fix Me! */
    if (timer_list == NULL) {
        next = this;
        timer_list = this;
    } else {
        next = timer_list->next;
        timer_list->next = this;
    }

    expiration.assign_current_time();
    expiration += timeout;
    
    if (!timer_started) {
        Glib::signal_timeout().connect(sigc::ptr_fun(&timer_func), timeout * 1000 / TIMER_SCALE_VALUE);
        timer_started = true;
    }

    return;
}

/** \brief  Deletes a \c ExpirationTimer
    
    The most complex thing that this function does is remove the timer
    from the circularly linked list.  If this is the only entry in the
    list that is easy, otherwise all the entries must be found, and this
    one removed from the list.
*/
ExpirationTimer::~ExpirationTimer(void)
{
    if (this != next) {
        /* This will remove this entry from the circularly linked
           list. */
        ExpirationTimer * prev;
        for (prev = timer_list;
                prev->next != this;
                prev = prev->next){};
        prev->next = next;

        if (idle_start == this)
            idle_start = next;

        /* This may occur more than you think, just because the guy
           doing most of the deletions is the idle function, who tracks
           where it is looking using the \c timer_list variable. */
        if (timer_list == this)
            timer_list = next;
    } else {
        /* If we're the only entry in the list, the list needs to go
           to NULL */
        timer_list = NULL;
        idle_start = NULL;
    }

    return;
}

/** \brief  Touches the timer to extend the length before it expires

    Basically it adds more time to the timer.  One thing that is kinda
    tricky is that it adds half the time remaining back into the timer.
    This allows for some extensions that are used regularly to having
    extended expiration times.  So, in the end, they stay loaded longer.
    Extensions that are only used once will expire at a standard rate
    set by \c timeout.
*/
void
ExpirationTimer::touch (void)
{
    Glib::TimeVal current;
    current.assign_current_time();

    long time_left = (long)(expiration.as_double() - current.as_double());
    if (time_left < 0) time_left = 0;
    time_left /= 2;

    expiration = current + timeout + time_left;
    return;
}

/** \brief  Check to see if the timer has expired

    Checks the time against the current time.
*/
bool
ExpirationTimer::expired (void) const
{
    if (locked > 0) return false;

    Glib::TimeVal current;
    current.assign_current_time();
    return expiration < current;
}

// int idle_cnt = 0;

/** \brief  This function goes in the idle loop to find expired extensions
    \return Whether the function should be requeued or not

    This function first insures that there is a timer list, and then checks
    to see if the one on the top of the list has expired.  If it has
    expired it unloads the module.  By unloading the module, the timer
    gets deleted (happens in the unload function).  If the list is
    no empty, the function returns that it should be dequeued and sets
    the \c timer_started variable so that the timer will be reissued when
    a timer is added.  If there is entries left, but the next one is
    where this function started, then the timer is set up.  The timer
    will then re-add the idle loop function when it runs.
*/ 
bool
ExpirationTimer::idle_func (void)
{
    // std::cout << "Idle func pass: " << idle_cnt++ << "  timer list: " << timer_list << std::endl;

    /* see if this is the last */
    if (timer_list == NULL) {
        timer_started = false;
        return false;
    }

    /* evalutate current */
    if (timer_list->expired()) {
        timer_list->extension->set_state(Extension::STATE_UNLOADED);
    }

    /* see if this is the last */
    if (timer_list == NULL) {
        timer_started = false;
        return false;
    }

    if (timer_list->next == idle_start) {
        /* if so, set up the timer and return FALSE */
        /* Note: This may cause one to be missed on the evaluation if
                 the one before it expires and it is last in the list.
                 While this could be taken care of, it isn't worth the
                 complexity for this lazy removal that we're doing.  It
                 should get picked up next time */
        Glib::signal_timeout().connect(sigc::ptr_fun(&timer_func), timeout * 1000 / TIMER_SCALE_VALUE);
        return false;
    }

    /* If nothing else, continue on */
    timer_list = timer_list->next;
    return true;
}

/** \brief  A timer function to set up the idle function
    \return Always false -- to disable the timer

    This function sets up the idle loop when it runs.  The idle loop is
    the one that unloads all the extensions.
*/
bool
ExpirationTimer::timer_func (void)
{
    // std::cout << "Timer func" << std::endl;
    idle_start = timer_list;
    // idle_cnt = 0;
    Glib::signal_idle().connect(sigc::ptr_fun(&idle_func));
    return false;
}

}; }; /* namespace Inkscape, Extension */

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

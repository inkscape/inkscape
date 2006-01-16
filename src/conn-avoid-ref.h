#ifndef SEEN_CONN_AVOID_REF
#define SEEN_CONN_AVOID_REF

/*
 * A class for handling shape interaction with libavoid.
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <sigc++/connection.h>

struct SPItem;
namespace Avoid {
    class ShapeRef;
}

class SPAvoidRef {
public:
    SPAvoidRef(SPItem *spitem);
    ~SPAvoidRef();

    // libavoid's internal representation of the item.
    Avoid::ShapeRef *shapeRef;

    void setAvoid(char const *value);
    void handleSettingChange(void);

private:
    SPItem *item;

    // true if avoiding, false if not.
    bool setting;
    bool new_setting;

    // A sigc connection for transformed signal.
    sigc::connection _transformed_connection;
};


#endif /* !SEEN_CONN_AVOID_REF */

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

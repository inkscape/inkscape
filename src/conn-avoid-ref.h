#ifndef SEEN_CONN_AVOID_REF
#define SEEN_CONN_AVOID_REF

/** \file
 * A class for handling shape interaction with libavoid.
 */
/*
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/point.h>
#include <stddef.h>
#include <sigc++/connection.h>

class  SPDesktop;
class SPObject;
class  SPItem;
namespace Avoid { class ShapeRef; }

class SPAvoidRef {
public:
    SPAvoidRef(SPItem *spitem);
    virtual ~SPAvoidRef();

    // libavoid's internal representation of the item.
    Avoid::ShapeRef *shapeRef;

    void setAvoid(char const *value);
    void handleSettingChange(void);

    Geom::Point getConnectionPointPos(void);

    // Returns a list of SPItems of all connectors/shapes attached to
    // this object.  Pass one of the following for 'type':
    //     Avoid::runningTo
    //     Avoid::runningFrom
    //     Avoid::runningToAndFrom
    std::vector<SPItem *> getAttachedShapes(const unsigned int type);
    std::vector<SPItem *> getAttachedConnectors(const unsigned int type);

private:
    SPItem *item;

    // true if avoiding, false if not.
    bool setting;
    bool new_setting;

    // A sigc connection for transformed signal.
    sigc::connection _transformed_connection;
};

extern std::vector<SPItem *> get_avoided_items(std::vector<SPItem *> &list, SPObject *from,
        SPDesktop *desktop, bool initialised = true);
extern void avoid_item_move(Geom::Affine const *mp, SPItem *moved_item);
extern void init_avoided_shape_geometry(SPDesktop *desktop);

static const double defaultConnSpacing = 3.0;
static const double defaultConnCurvature = 3.0;

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

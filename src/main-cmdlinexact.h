
#ifndef __INK_MAIN_CMD_LINE_XACTIONS_H__
#define __INK_MAIN_CMD_LINE_XACTIONS_H__

#ifdef WITH_YAML

/** \file
 * Extended actions that can be queued at the yaml file
 */

/*
 * Authors:
 *   Dmitry Zhulanov <dmitry.zhulanov@gmail.com>
 *
 * Copyright (C) 2016 Authors
 *
 * Released under GNU GPL v2.x, read the file 'COPYING' for more information
 */

#include "main-cmdlineact.h"
#include <string>

namespace Inkscape {

typedef std::map<std::string, std::string > xaction_args_values_map_t;

class CmdLineXAction : public CmdLineAction {
    std::string arg;
    xaction_args_values_map_t _values_map;
public:
    CmdLineXAction (gchar const * arg, xaction_args_values_map_t &values_map);

    virtual void doItX (ActionContext const & context);
    virtual bool isExtended();

    static void createActionsFromYAML( gchar const *filename );
};

} // Inkscape


#endif // WITH_YAML
#endif /* __INK_MAIN_CMD_LINE_XACTIONS_H__ */

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

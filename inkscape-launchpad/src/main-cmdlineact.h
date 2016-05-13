
#ifndef __INK_MAIN_CMD_LINE_ACTIONS_H__
#define __INK_MAIN_CMD_LINE_ACTIONS_H__

/** \file
 * Small actions that can be queued at the command line
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information
 */

namespace Inkscape {

class ActionContext;

class CmdLineAction {
    bool _isVerb;
    char * _arg;

    static std::list <CmdLineAction *> _list;

public:
    CmdLineAction (bool isVerb, char const * arg);
    virtual ~CmdLineAction ();

    void doIt (ActionContext const & context);
    /** Return true if any actions were performed */
    static bool doList (ActionContext const & context);
    static bool idle (void);
};

} // Inkscape



#endif /* __INK_MAIN_CMD_LINE_ACTIONS_H__ */

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

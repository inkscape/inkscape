/*
 * Implementation the Pedro mini-XMPP client
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2007 Bob Jamison
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

#include <stdio.h>




//#######################################################################
//# G E C K O (xulrunner)
//#######################################################################

#ifdef GECKO_EMBED


#include "geckoembed.h"

int main(int argc, char *argv[])
{
    GeckoEmbed embedder;
    
    embedder.run();

    return 0;
}


//#######################################################################
//# G T K M M  (pedrogui)
//#######################################################################
#else /* NOT GECKO_EMBED */



#include "pedrogui.h"

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);

    Pedro::PedroGui window;

    kit.run(window);

    return 0;
}



#endif /* GECKO_EMBED */





#ifdef __WIN32__
#include <windows.h>

extern "C" int __export WINAPI
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
         char *lpszCmdLine, int nCmdShow)
{
    int ret = main (__argc, __argv);
    return ret;
}

#endif



//########################################################################
//# E N D    O F     F I L E
//########################################################################


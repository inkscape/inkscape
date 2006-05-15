#include <stdio.h>

#include "pedrogui.h"

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);

    Pedro::PedroGui window;

    kit.run(window);

    return 0;
}




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


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <glib.h>

#ifdef G_OS_WIN32
#undef DATADIR

#include <stdio.h>
#include <io.h>
#include <conio.h>

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
# undef _WIN32_WINNT 
# define _WIN32_WINNT 0x0501
#endif

#include <windows.h>

extern int main (int argc, char **argv);

/* In case we build this as a windowed application */

#ifdef __GNUC__
#  ifndef _stdcall
#    define _stdcall  __attribute__((stdcall))
#  endif
#endif

int _stdcall
WinMain (struct HINSTANCE__ */*hInstance*/,
	 struct HINSTANCE__ */*hPrevInstance*/,
	 char               */*lpszCmdLine*/,
	 int                 /*nCmdShow*/)
{
    if (fileno (stdout) != -1 &&
 	  _get_osfhandle (fileno (stdout)) != -1)
	{
	  /* stdout is fine, presumably redirected to a file or pipe */
	}
    else
    {
	  typedef BOOL (* WINAPI AttachConsole_t) (DWORD);

	  AttachConsole_t p_AttachConsole =
	    (AttachConsole_t) GetProcAddress (GetModuleHandle ("kernel32.dll"), "AttachConsole");

	  if (p_AttachConsole != NULL && p_AttachConsole (ATTACH_PARENT_PROCESS))
      {
	      freopen ("CONOUT$", "w", stdout);
	      dup2 (fileno (stdout), 1);
	      freopen ("CONOUT$", "w", stderr);
	      dup2 (fileno (stderr), 2);

      }
	}

	int ret;
	ret = main (__argc, __argv);
	return ret;
}

#endif

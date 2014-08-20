/**
 * \file
 * Command-line wrapper for Windows.
 *
 * Windows has two types of executables: GUI and console.
 * The GUI executables detach immediately when run from the command
 * prompt (cmd.exe), and whatever you write to standard output
 * disappears into a black hole. Console executables
 * do display standard output and take standard input from the console,
 * but when you run them from the GUI, an extra console window appears.
 * It's possible to hide it, but it still flashes for a fraction
 * of a second.
 *
 * To provide an Unix-like experience, where the application will behave
 * correctly in command line mode and at the same time won't create
 * the ugly console window when run from the GUI, we have to have two
 * executables. The first one, inkscape.exe, is the GUI application.
 * Its entry points are in main.cpp and winmain.cpp. The second one,
 * called inkscape.com, is a small helper application contained in
 * this file. It spawns the GUI application and redirects its output
 * to the console.
 *
 * Note that inkscape.com has nothing to do with "compact executables"
 * from DOS. It's a normal PE executable renamed to .com. The trick
 * is that cmd.exe picks .com over .exe when both are present in PATH,
 * so when you type "inkscape" into the command prompt, inkscape.com
 * gets run. The Windows program loader does not inspect the extension,
 * just like an Unix program loader; it determines the binary format
 * based on the contents of the file.
 *
 *//*
 * Authors:
 *   Jos Hirth <jh@kaioa.com>
 *   Krzysztof Kosinski <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2008-2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef WIN32
#undef DATADIR
#include <windows.h>

struct echo_thread_info {
    HANDLE echo_read;
    HANDLE echo_write;
    unsigned buffer_size;
};

// thread function for echoing from one file handle to another
DWORD WINAPI echo_thread(void *info_void)
{
    echo_thread_info *info = static_cast<echo_thread_info*>(info_void);
    char *buffer = reinterpret_cast<char *>(LocalAlloc(LMEM_FIXED, info->buffer_size));
    DWORD bytes_read, bytes_written;

    while(true){
        if (!ReadFile(info->echo_read, buffer, info->buffer_size, &bytes_read, NULL) || bytes_read == 0)
            if (GetLastError() == ERROR_BROKEN_PIPE)
                break;

        if (!WriteFile(info->echo_write, buffer, bytes_read, &bytes_written, NULL)) {
            if (GetLastError() == ERROR_NO_DATA)
                break;
        }
    }

    LocalFree(reinterpret_cast<HLOCAL>(buffer));
    CloseHandle(info->echo_read);
    CloseHandle(info->echo_write);

    return 1;
}

int main()
{
    // structs that will store information for our I/O threads
    echo_thread_info stdin = {NULL, NULL, 4096};
    echo_thread_info stdout = {NULL, NULL, 4096};
    echo_thread_info stderr = {NULL, NULL, 4096};
    // handles we'll pass to inkscape.exe
    HANDLE inkscape_stdin, inkscape_stdout, inkscape_stderr;
    HANDLE stdin_thread, stdout_thread, stderr_thread;

    SECURITY_ATTRIBUTES sa;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;

    // Determine the path to the Inkscape executable.
    // Do this by looking up the name of this one and redacting the extension to ".exe"
    const int pathbuf = 2048;
    WCHAR *inkscape = reinterpret_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, pathbuf * sizeof(WCHAR)));
    GetModuleFileNameW(NULL, inkscape, pathbuf);
    WCHAR *dot_index = wcsrchr(inkscape, L'.');
    wcsncpy(dot_index, L".exe", 4);

    // we simply reuse our own command line for inkscape.exe
    // it guarantees perfect behavior w.r.t. quoting
    WCHAR *cmd = GetCommandLineW();

    // set up the pipes and handles
    stdin.echo_read = GetStdHandle(STD_INPUT_HANDLE);
    stdout.echo_write = GetStdHandle(STD_OUTPUT_HANDLE);
    stderr.echo_write = GetStdHandle(STD_ERROR_HANDLE);
    CreatePipe(&inkscape_stdin, &stdin.echo_write, &sa, 0);
    CreatePipe(&stdout.echo_read, &inkscape_stdout, &sa, 0);
    CreatePipe(&stderr.echo_read, &inkscape_stderr, &sa, 0);

    // fill in standard IO handles to be used by the process
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;

    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = inkscape_stdin;
    si.hStdOutput = inkscape_stdout;
    si.hStdError = inkscape_stderr;

    // spawn inkscape.exe
    CreateProcessW(inkscape, // path to inkscape.exe
                   cmd, // command line as a single string
                   NULL, // process security attributes - unused
                   NULL, // thread security attributes - unused
                   TRUE, // inherit handles
                   0, // flags
                   NULL, // environment - NULL = inherit from us
                   NULL, // working directory - NULL = inherit ours
                   &si, // startup info - see above
                   &pi); // information about the created process - unused

    // clean up a bit
    LocalFree(reinterpret_cast<HLOCAL>(inkscape));
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(inkscape_stdin);
    CloseHandle(inkscape_stdout);
    CloseHandle(inkscape_stderr);

    // create IO echo threads
    DWORD unused;
    stdin_thread = CreateThread(NULL, 0, echo_thread, (void*) &stdin, 0, &unused);
    stdout_thread = CreateThread(NULL, 0, echo_thread, (void*) &stdout, 0, &unused);
    stderr_thread = CreateThread(NULL, 0, echo_thread, (void*) &stderr, 0, &unused);

    // wait until the standard output thread terminates
    WaitForSingleObject(stdout_thread, INFINITE);

    return 0;
}

#endif

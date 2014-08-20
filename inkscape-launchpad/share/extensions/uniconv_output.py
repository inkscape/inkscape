#!/usr/bin/env python

"""
uniconv_output.py
Module for running UniConverter exporting commands in Inkscape extensions

Copyright (C) 2009 Nicolas Dufour (jazzynico)
Based on unicon-ext.py and run_command.py by Stephen Silver

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
"""

# Run a command that generates an UniConvertor export file from a SVG file.
# On success, outputs the contents of the UniConverter convertion to stdout, 
# and exits with a return code of 0.
# On failure, outputs an error message to stderr, and exits with a return
# code of 1.

# standard library
import os
import sys
import tempfile
# local library
import inkex
inkex.localize()

def run(command_format, prog_name, uniconv_format):
    outfile = tempfile.mktemp(uniconv_format)
    command = command_format + outfile
    msg = None
    # In order to get a return code from the process, we use subprocess.Popen
    # if it's available (Python 2.4 onwards) and otherwise use popen2.Popen3
    # (Unix only).  As the Inkscape package for Windows includes Python 2.5,
    # this should cover all supported platforms.
    try:
        try:
            from subprocess import Popen, PIPE
            p = Popen(command, shell=True, stdout=PIPE, stderr=PIPE)
            rc = p.wait()
            out = p.stdout.read()
            err = p.stderr.read()
        except ImportError:
            try:
                from popen2 import Popen3
                p = Popen3(command, True)
                p.wait()
                rc = p.poll()
                out = p.fromchild.read()
                err = p.childerr.read()
            except ImportError:
                # shouldn't happen...
                msg = "Neither subprocess.Popen nor popen2.Popen3 is available"
        if rc and msg is None:
            msg = "%s failed:\n%s\n%s\n" % (prog_name, out, err)
    except Exception, inst:
        msg = "Error attempting to run %s: %s" % (prog_name, str(inst))

    # If successful, copy the output file to stdout.
    if msg is None:
        if os.name == 'nt':  # make stdout work in binary on Windows
            import msvcrt
            msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)
        try:
            f = open(outfile, "rb")
            data = f.read()
            sys.stdout.write(data)
            f.close()
        except IOError, inst:
            msg = "Error reading temporary file: %s" % str(inst)

    # Clean up.
    try:
        os.remove(outfile)
    except Exception:
        pass

    # Ouput error message (if any) and exit.
    if msg is not None:
        sys.stderr.write(msg + "\n")
        sys.exit(1)
    else:
        sys.exit(0)

def get_command():
    cmd = None

    try:
        from subprocess import Popen, PIPE
        p = Popen('uniconvertor', shell=True, stdout=PIPE, stderr=PIPE).wait()
        if p==0 :
            cmd = 'uniconvertor'
        else:
            p = Popen('uniconv', shell=True, stdout=PIPE, stderr=PIPE).wait()
            if p==0 :
                cmd = 'uniconv'
    except ImportError:
        from popen2 import Popen3
        p = Popen3('uniconv', True).wait()
        if p!=32512 : cmd = 'uniconv'
        p = Popen3('uniconvertor', True).wait()
        if p!=32512 : cmd = 'uniconvertor'

    if cmd == None:
        # there's no succeffully-returning uniconv command; try to get the module directly (on Windows)
        try:
            # cannot simply import uniconvertor, it aborts if you import it without parameters
            import imp
            imp.find_module("uniconvertor")
        except ImportError:
            sys.stderr.write(_('You need to install the UniConvertor software.\n'+\
                         'For GNU/Linux: install the package python-uniconvertor.\n'+\
                         'For Windows: download it from\n'+\
                         'http://sk1project.org/modules.php?name=Products&product=uniconvertor\n'+\
                         'and install into your Inkscape\'s Python location\n'))
            sys.exit(1)
        cmd = 'python -c "import uniconvertor; uniconvertor.uniconv_run();"'

    return cmd

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99

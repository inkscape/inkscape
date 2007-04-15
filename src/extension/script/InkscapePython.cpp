/**
 * Python Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <ishmalius@gmail.com>
 *
 * Copyright (C) 2004-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <Python.h>

#include "InkscapePython.h"


#include <stdio.h>
#include <CXX/Extensions.hxx>
#include <CXX/Objects.hxx>


#include <inkscape.h>
#include <desktop.h>
#include <document.h>
#include <ui/dialog/dialog-manager.h>

//only for sp_help_about()
#include "help.h"

namespace Inkscape
{
namespace Extension
{
namespace Script
{




//########################################################################
//# D I A L O G    M A N A G E R
//########################################################################

class PyDialogManager : public Py::PythonExtension<PyDialogManager>
{
public:

    PyDialogManager(Inkscape::UI::Dialog::DialogManager *dm) :
                            dialogManager(dm)
        {
        }

    virtual ~PyDialogManager()
        {
        }

    virtual Py::Object getattr(const char *name)
        {
        /*
        if (strcmp(name, "activeDesktop")==0)
            {
            PyDesktop obj(SP_ACTIVE_DESKTOP);
            return obj;
            }
        */
        return getattr_methods(name);
        }
        
    virtual Py::Object showAbout(const Py::Tuple &args)
        {
        sp_help_about();
        return Py::Nothing();
        }


    static void init_type()
        {
        behaviors().name("DialogManager");
        behaviors().doc("dialogManager objects: ");
        behaviors().supportRepr();
        behaviors().supportGetattr();

        add_varargs_method("showAbout", &PyDialogManager::showAbout,
             "Shows a silly dialog");
        }

private:

    Inkscape::UI::Dialog::DialogManager *dialogManager;
};



//########################################################################
//# D E S K T O P
//########################################################################

class PyDesktop : public Py::PythonExtension<PyDesktop>
{
public:

    PyDesktop(SPDesktop *dt) : desktop(dt)
        {
        }

    virtual ~PyDesktop()
        {
        }

    virtual Py::Object getattr(const char *name)
        {
        if (strcmp(name, "dialogManager")==0)
            {
            Py::Object obj(Py::asObject(
                  new PyDialogManager(desktop->_dlg_mgr)));
            return obj;
            }
        return getattr_methods(name);
        }
        
    static void init_type()
        {
        behaviors().name("Desktop");
        behaviors().doc("desktop objects: dialogManager");
        behaviors().supportRepr();
        behaviors().supportGetattr();
        }



private:

    SPDesktop *desktop;

};



//########################################################################
//# D O C U M E N T
//########################################################################


class PyDocument : public Py::PythonExtension<PyDocument>
{
public:

    PyDocument(SPDocument *doc) : document(doc)
        {
        }

    virtual ~PyDocument()
        {
        }

    virtual Py::Object getattr(const char *name)
        {
        return getattr_methods(name);
        }
        

    static void init_type()
        {
        behaviors().name("Document");
        behaviors().doc("document objects: ");
        behaviors().supportRepr();
        behaviors().supportGetattr();
        }

private:

    SPDocument *document;
};







//########################################################################
//# I N K S C A P E    and siblings
//#
//# The following are children of PyInkscapeModule and are spawned
//# by its methods.  The classes above are spawned by PyInkscape and
//# it descendants.
//#
//########################################################################


class PyInkscape : public Py::PythonExtension<PyInkscape>
{
public:

    PyInkscape(InkscapePython &par) : parent(par)
        {
        inkscape = INKSCAPE;
        }
        
    virtual ~PyInkscape()
        {
        }


    virtual Py::Object getattr(const char *name)
        {
        if (strcmp(name, "activeDesktop")==0)
            {
            Py::Object obj(Py::asObject(
                  new PyDesktop(SP_ACTIVE_DESKTOP)));
            return obj;
            }
        else if (strcmp(name, "activeDocument")==0)
            {
            Py::Object obj(Py::asObject(
                  new PyDocument(SP_ACTIVE_DOCUMENT)));
            return obj;
            }
        return getattr_methods(name);
        }
        
    virtual Py::Object exit(const Py::Tuple &args)
        {
        //exit();
        return Py::Nothing(); //like a void
        }

    virtual Py::Object hello(const Py::Tuple &args)
        {
        //exit();
        //throw Py::RuntimeError("some error message");        
        return Py::String("Hello, world!");
        }

    static void init_type()
        {
        behaviors().name("Inkscape");
        behaviors().doc("inkscape objects: activeDesktop activeDocument");
        behaviors().supportRepr();
        behaviors().supportGetattr();

        add_varargs_method("hello", &PyInkscape::hello,
             "Does a hello, world");
        add_varargs_method("exit", &PyInkscape::exit,
             "exit from the current application");
        }

private:

    InkscapePython &parent;

    Inkscape::Application *inkscape;

};




//########################################################################
//# O U T P U T S
//########################################################################



class PyStdOut : public Py::PythonExtension<PyStdOut>
{
public:

    PyStdOut(InkscapePython &par) : parent(par)
        {
        }
        
    virtual ~PyStdOut()
        {
        }

        
    virtual Py::Object write(const Py::Tuple &args)
        {
        for(unsigned int i=0 ; i<args.length() ; i++)
            {
            Py::String str(args[i]);
            parent.writeStdOut(str.as_std_string());
            }
        return Py::Nothing();
        }

    static void init_type()
        {
        behaviors().name("PyStdOut");

        add_varargs_method("write", &PyStdOut::write,
             "redirects stdout");
        }

private:

    InkscapePython &parent;

};




class PyStdErr : public Py::PythonExtension<PyStdErr>
{
public:

    PyStdErr(InkscapePython &par) : parent(par)
        {
        }
        
    virtual ~PyStdErr()
        {
        }

        
    virtual Py::Object write(const Py::Tuple &args)
        {
        for(unsigned int i=0 ; i<args.length() ; i++)
            {
            Py::String str(args[i]);
            parent.writeStdErr(str.as_std_string());
            }
        return Py::Nothing();
        }

    static void init_type()
        {
        behaviors().name("PyStdErr");

        add_varargs_method("write", &PyStdErr::write,
             "redirects stderr");
        }

private:

    InkscapePython &parent;


};



//########################################################################
//# M O D U L E
//########################################################################



class PyInkscapeModule : public Py::ExtensionModule<PyInkscapeModule>
{
public:
    PyInkscapeModule(InkscapePython &par)
        : Py::ExtensionModule<PyInkscapeModule>( "PyInkscapeModule" ),
          parent(par)
        {
        //# Init our module's classes
        PyInkscape::init_type();
        PyDocument::init_type();
        PyDesktop::init_type();
        PyDialogManager::init_type();
        PyStdOut::init_type();
        PyStdErr::init_type();

        add_varargs_method("getInkscape", 
            &PyInkscapeModule::getInkscape, "returns global inkscape app");
        add_varargs_method("getStdOut", 
            &PyInkscapeModule::getStdOut, "gets redirected output");
        add_varargs_method("getStdErr", 
            &PyInkscapeModule::getStdErr, "gets redirected output");

        initialize( "main Inkscape module" );
        }

    virtual ~PyInkscapeModule()
        {
        }

    virtual Py::Object getInkscape(const Py::Tuple &args)
        {
        Py::Object obj(Py::asObject(new PyInkscape(parent)));
        return obj;
        }
    virtual Py::Object getStdOut(const Py::Tuple &args)
        {
        Py::Object obj(Py::asObject(new PyStdOut(parent)));
        return obj;
        }
    virtual Py::Object getStdErr(const Py::Tuple &args)
        {
        Py::Object obj(Py::asObject(new PyStdErr(parent)));
        return obj;
        }

private:

    InkscapePython &parent;


};



//########################################################################
//# M A I N
//########################################################################




/**
 *  Interpret an in-memory string
 */
bool InkscapePython::interpretScript(const Glib::ustring &script,
          Glib::ustring &output, Glib::ustring &error)
{

    stdOut.clear();
    stdErr.clear();

    //## First bind our classes
    Py_Initialize();
    

    //# Init our custom objects
    PyInkscapeModule inkscapeModule(*this);

    PyObject *globalMod  = PyImport_AddModule("__main__");
    PyObject *globalDict = PyModule_GetDict(globalMod);
    PyObject *localDict  = inkscapeModule.moduleDictionary().ptr();

    Glib::ustring buf =
    "import sys\n"
    "sys.stdout = getStdOut()\n"
    "sys.stderr = getStdErr()\n"
    "\n"
    "inkscape = getInkscape()\n"
    "\n";
    buf.append(script);


    char *codeStr = (char *)buf.c_str();
    PyRun_String(codeStr, Py_file_input, globalDict, localDict);

    output = stdOut;
    //output = "hello, world\n";
    error  = stdErr;


    //## Check for errors
    if (PyErr_Occurred())
        {
        PyObject *errtype      = NULL;
        PyObject *errval       = NULL;
        PyObject *errtraceback = NULL;

        PyErr_Fetch(&errtype, &errval, &errtraceback);
        //PyErr_Clear();

        if (errval && PyString_Check(errval))
            {
            PyObject *pystring = PyObject_Str(errval);
            char *errStr = PyString_AsString(pystring);
            int line = ((PyTracebackObject*)errtraceback)->tb_lineno;
            //error = "Line ";
            //error.append(line);
            //error.append(" : ");
            error.append(errStr);
            Py_XDECREF(pystring);
            }
        else
            {
            error = "Error occurred";
            }
        Py_XDECREF(errtype);
        Py_XDECREF(errval);
        Py_XDECREF(errtraceback);
        Py_Finalize();
        return false;
        }


    Py_Finalize();

    return true;
}







}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

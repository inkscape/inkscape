/**
 * Python Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
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

namespace Inkscape
{
namespace Extension
{
namespace Script
{


//########################################################################
//# I N K S C A P E
//########################################################################

class PyDialogManager : public Py::PythonExtension<PyDialogManager>
{
public:



private:

};



class PyDesktop : public Py::PythonExtension<PyDesktop>
{
public:



private:

};



class PyDocument : public Py::PythonExtension<PyDocument>
{
public:



private:

};



class PyInkscape : public Py::PythonExtension<PyInkscape>
{
public:

    PyInkscape()
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
            }
        else if (strcmp(name, "activeDocument")==0)
            {
            }
        else if (strcmp(name, "dialogManager")==0)
            {
            }
        return getattr_methods(name);
        }
        
    virtual Py::Object exit(const Py::Tuple &args)
        {
        //exit();
        }

    static void init_type()
        {
        behaviors().name("inkscape");
        behaviors().doc("inkscape objects: activeDesktop activeDocument dialogManager");
        behaviors().supportRepr();
        behaviors().supportGetattr();

        add_varargs_method("exit", &PyInkscape::exit,
             "exit from the current application");
        }

private:

    Inkscape::Application *inkscape;

};




//########################################################################
//# M A I N
//########################################################################


bool InkscapePython::initialize()
{
    if (initialized)
        return true;

    Py_Initialize();
    
    


    initialized = true;
    return true;
}



/*
 *  Interpret an in-memory string
 */
bool InkscapePython::interpretScript(const Glib::ustring &script,
          Glib::ustring &output, Glib::ustring &error)
{
    if (!initialize())
        return false;

    char *codeStr = (char *)script.raw().c_str();
    //PyRun_SimpleString(inkscape_module_script);
    //PyRun_SimpleString("inkscape = _inkscape_py.getInkscape()\n");
    //PyRun_SimpleString(codeStr);

    //## Check for errors
    if (PyErr_Occurred())
        {
        PyObject *errobj       = NULL;
        PyObject *errdata      = NULL;
        PyObject *errtraceback = NULL;

        PyErr_Fetch(&errobj, &errdata, &errtraceback);
        //PyErr_Clear();

        if (errobj && PyString_Check(errobj))
            {
            PyObject *pystring = PyObject_Str(errobj);
            char *errStr =  PyString_AsString(pystring);
            error = errStr;
            Py_XDECREF(pystring);
            }
        else
            {
            error = "Error occurred";
            }
        Py_XDECREF(errobj);
        Py_XDECREF(errdata);
        Py_XDECREF(errtraceback);
        return false;
        }
    //Py_Finalize();
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

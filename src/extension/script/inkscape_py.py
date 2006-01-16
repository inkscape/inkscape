# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _inkscape_py

def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name) or (name == "thisown"):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



getInkscape = _inkscape_py.getInkscape
class Inkscape(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Inkscape, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Inkscape, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<%s.%s; proxy of C++ Inkscape::Extension::Script::Inkscape instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __del__(self, destroy=_inkscape_py.delete_Inkscape):
        try:
            if self.thisown: destroy(self)
        except: pass

    def getDesktop(*args): return _inkscape_py.Inkscape_getDesktop(*args)
    def getDialogManager(*args): return _inkscape_py.Inkscape_getDialogManager(*args)

class InkscapePtr(Inkscape):
    def __init__(self, this):
        _swig_setattr(self, Inkscape, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Inkscape, 'thisown', 0)
        _swig_setattr(self, Inkscape,self.__class__,Inkscape)
_inkscape_py.Inkscape_swigregister(InkscapePtr)

class DialogManager(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, DialogManager, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, DialogManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<%s.%s; proxy of C++ Inkscape::Extension::Script::DialogManager instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __del__(self, destroy=_inkscape_py.delete_DialogManager):
        try:
            if self.thisown: destroy(self)
        except: pass

    def showAbout(*args): return _inkscape_py.DialogManager_showAbout(*args)

class DialogManagerPtr(DialogManager):
    def __init__(self, this):
        _swig_setattr(self, DialogManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, DialogManager, 'thisown', 0)
        _swig_setattr(self, DialogManager,self.__class__,DialogManager)
_inkscape_py.DialogManager_swigregister(DialogManagerPtr)

class Desktop(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Desktop, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Desktop, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<%s.%s; proxy of C++ Inkscape::Extension::Script::Desktop instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __del__(self, destroy=_inkscape_py.delete_Desktop):
        try:
            if self.thisown: destroy(self)
        except: pass

    def getDocument(*args): return _inkscape_py.Desktop_getDocument(*args)

class DesktopPtr(Desktop):
    def __init__(self, this):
        _swig_setattr(self, Desktop, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Desktop, 'thisown', 0)
        _swig_setattr(self, Desktop,self.__class__,Desktop)
_inkscape_py.Desktop_swigregister(DesktopPtr)

class Document(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Document, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Document, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<%s.%s; proxy of C++ Inkscape::Extension::Script::Document instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __del__(self, destroy=_inkscape_py.delete_Document):
        try:
            if self.thisown: destroy(self)
        except: pass

    def hello(*args): return _inkscape_py.Document_hello(*args)

class DocumentPtr(Document):
    def __init__(self, this):
        _swig_setattr(self, Document, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Document, 'thisown', 0)
        _swig_setattr(self, Document,self.__class__,Document)
_inkscape_py.Document_swigregister(DocumentPtr)



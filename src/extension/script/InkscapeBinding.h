#ifndef __INKSCAPE_BINDING_H__
#define __INKSCAPE_BINDING_H__
/**
 * This file is an attempt to provide a hierarchical design
 * to wrap Inkscape in an OO model.  This file is parsed by Swig
 * to produce scripting extension modules for such interpreters
 * as Python or Perl
 *
 * Authors:
 *   Bob Jamison <ishmalius@gmail.com>
 *
 * Copyright (C) 2004-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
/*
 * Note: we are doc-ing this file instead of the .cpp since
 * the .cpp file has impl's, not def's of these classes.
 * Also note that you need to understand Swig before you edit this file.
 */


namespace Inkscape {
namespace Extension {
namespace Script {

class Inkscape;
class DialogManager;
class Desktop;
class Document;



/**
 * Get the root Inkscape object.  The module wrapper should
 * always call this first so that there will be an 'inkscape'
 * object available to the user.
 */
Inkscape *getInkscape();


/**
 * Root inkscape object.  Owner of everything
 */
class Inkscape
{
public:

    /**
     *
     */
    Inkscape(){}
    
    /**
     *
     */
    virtual ~Inkscape(){};
    
    /**
     *
     */
    virtual Desktop *getDesktop() = 0;

    /**
     *
     */
    virtual DialogManager *getDialogManager() = 0;

};

/**
 * Controller for the various Inkscape dialogs
 */
class DialogManager
{
public:

    /**
     *
     */
    DialogManager(){}
    
    /**
     *
     */
    virtual ~DialogManager(){};
    
    /**
     *
     */
    virtual void showAbout() = 0;

};


/**
 *
 */
class Desktop
{

public:

    /**
     *
     */
    Desktop() {}
    
    /**
     *
     */
    virtual ~Desktop(){};
    
    /**
     *
     */
    virtual Document *getDocument() = 0;

};



/**
 *
 */
class Document
{

public:

    /**
     *
     */
    Document() {};
    
    /**
     *
     */
    virtual ~Document(){};
    
    /**
     *
     */
    virtual void hello() = 0;


};


}//namespace Script
}//namespace Extension
}//namespace Inkscape



#endif  /*__INKSCAPE_BINDING_H__*/

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

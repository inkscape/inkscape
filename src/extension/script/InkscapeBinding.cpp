

#include "InkscapeBinding.h"

#include "help.h"

namespace Inkscape {
namespace Extension {
namespace Script {


class InkscapeImpl;
class DesktopImpl;
class DocumentImpl;


//#########################################################################
//# D O C U M E N T
//#########################################################################
class DocumentImpl : public Document
{
public:
    DocumentImpl();
    
    virtual ~DocumentImpl();
    
    virtual void hello();
    
private:


};


DocumentImpl::DocumentImpl()
{


}
    
DocumentImpl::~DocumentImpl()
{


}
    
void DocumentImpl::hello()
{
    printf("######## HELLO, WORLD! #######\n");
}



//#########################################################################
//# D E S K T O P
//#########################################################################
class DesktopImpl : public Desktop
{
public:
    DesktopImpl();
    
    virtual ~DesktopImpl();
    
    virtual Document *getDocument();
    
private:

    DocumentImpl document;

};


DesktopImpl::DesktopImpl()
{


}
    
DesktopImpl::~DesktopImpl()
{


}
    

Document *DesktopImpl::getDocument()
{
    return &document;
}



//#########################################################################
//# D I A L O G    M A N A G E R
//#########################################################################

class DialogManagerImpl : public DialogManager
{
public:
    DialogManagerImpl();
    
    virtual ~DialogManagerImpl();
    
    virtual void showAbout();
    
private:


};

DialogManagerImpl::DialogManagerImpl()
{

}

    
DialogManagerImpl::~DialogManagerImpl()
{

}
    

void DialogManagerImpl::showAbout()
{
    sp_help_about();

}



//#########################################################################
//# I N K S C A P E
//#########################################################################

class InkscapeImpl : public Inkscape
{
public:
    InkscapeImpl();
    
    virtual ~InkscapeImpl();
    
    virtual Desktop *getDesktop();
    
    virtual DialogManager *getDialogManager();
    
private:

    DesktopImpl desktop;

    DialogManagerImpl dialogManager;

};

Inkscape *getInkscape()
{
    Inkscape *inkscape = new InkscapeImpl();
    return inkscape;
}


InkscapeImpl::InkscapeImpl()
{

}

    
InkscapeImpl::~InkscapeImpl()
{

}
    

Desktop *InkscapeImpl::getDesktop()
{
    return &desktop;
}

DialogManager *InkscapeImpl::getDialogManager()
{
    return &dialogManager;
}



}//namespace Script
}//namespace Extension
}//namespace Inkscape


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

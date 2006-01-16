
#include <stdio.h>

#include "InkscapeBinding.h"

void doTest()
{
    Inkscape::Extension::Script::Inkscape *inkscape =
         Inkscape::Extension::Script::getInkscape();
    Inkscape::Extension::Script::Desktop  *desktop  = inkscape->getDesktop();
    Inkscape::Extension::Script::Document *document = desktop->getDocument();
    document->hello();
}

int main(int argc, char **argv)
{

    doTest();
    
}



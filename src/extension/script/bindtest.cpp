
#include <stdio.h>

#include "InkscapeScript.h"

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static char *pythonCodeStr =
//"\n"
//"inkscape = _inkscape_py.getInkscape()\n"
"desktop  = inkscape.getDesktop()\n"
"document = desktop.getDocument()\n"
"document.hello()\n"
"";

int testPython()
{
    Inkscape::Extension::Script::InkscapeScript scriptEngine;
    printf("##### Python Test #####\n");
    printf("===== CODE ====\n%s\n==============\n", pythonCodeStr);
    scriptEngine.interpretScript(pythonCodeStr, 
          Inkscape::Extension::Script::InkscapeScript::PYTHON);
    printf("##### End Python #####\n\n");
    return TRUE;
}

static char *perlCodeStr =
//"\n"
//"$inkscape = inkscape_perlc::getInkscape();\n"
"print \"inkscape: '$inkscape'\\n\"; \n"
"$desktop  = $inkscape->getDesktop();\n"
"$document = $desktop->getDocument();\n"
"$document->hello()\n"
//"reverse 'rekcaH lreP rehtonA tsuJ'\n"
"";

int testPerl()
{
    Inkscape::Extension::Script::InkscapeScript scriptEngine;
    printf("##### Perl Test #####\n");
    printf("===== CODE ====\n%s\n==============\n", perlCodeStr);
    scriptEngine.interpretScript(perlCodeStr,
         Inkscape::Extension::Script::InkscapeScript::PERL);
    printf("##### End Perl #####\n\n");
    return TRUE;
}



int doTest()
{
    if (!testPython())
        {
        printf("Failed Python test\n");
        return FALSE;
        }
    if (!testPerl())
        {
        printf("Failed Perl test\n");
        return FALSE;
        }
    return TRUE;
}



int main(int argc, char **argv)
{

    if (doTest())
        printf("Tests succeeded\n");
    else
        printf("Tests failed\n");
    return 0;
}







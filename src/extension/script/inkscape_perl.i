%module inkscape_perl
%{
#include "InkscapeBinding.h"


static void xs_init _((pTHX));
static PerlInterpreter *my_perl;

int perl_eval(char *string) {
  char *argv[2];
  argv[0] = string;
  argv[1] = (char *) 0;
  return perl_call_argv("eval",0,argv);
}

extern "C" int
InkscapePerlParseBuf(char *startupCodeBuf, char *codeBuf)
{
    STRLEN n_a;
    int  exitstatus;
    char *embedding[] = { "", "-e", "0" };
        
    my_perl = perl_alloc();
    if (!my_perl)
       return 0;
    perl_construct( my_perl );

    exitstatus = perl_parse( my_perl, xs_init, 3,
                           embedding, (char **) NULL );
    if (exitstatus)
	return 0;

    /* Initialize all of the module variables */

    exitstatus = perl_run( my_perl );

    SV *retSV = eval_pv(startupCodeBuf, TRUE);
    char *ret = SvPV(retSV, n_a);
    //printf("## module ret:%s\n", ret);

    retSV = eval_pv("$inkscape = inkscape_perlc::getInkscape();\n", TRUE);
    ret = SvPV(retSV, n_a);
    //printf("## inkscape ret:%s\n", ret);

    retSV = eval_pv(codeBuf, TRUE);
    ret = SvPV(retSV, n_a);
    //printf("## code ret:%s\n", ret);

    perl_destruct( my_perl );
    perl_free( my_perl );

    return 1;
}

/* Register any extra external extensions */

/* Do not delete this line--writemain depends on it */
/* EXTERN_C void boot_DynaLoader _((CV* cv)); */

static void
xs_init(pTHX)
{
/*  dXSUB_SYS; */
    char *file = __FILE__;
    {
      /*        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file); */
	newXS(SWIG_name, SWIG_init, file);
#ifdef SWIGMODINIT
	SWIGMODINIT
#endif
    }
}


%}

%include "InkscapeBinding.h"

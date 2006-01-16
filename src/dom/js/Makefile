# #################################
# Unix/Linux makefile for libjs.a
# #################################


CC=gcc
CFLAGS= -O2 -DXP_UNIX

all: js

INC += -Ifdlibm

OBJ = \
	fdlibm/e_acos.o \
	fdlibm/e_acosh.o \
	fdlibm/e_asin.o \
	fdlibm/e_atan2.o \
	fdlibm/e_atanh.o \
	fdlibm/e_cosh.o \
	fdlibm/e_exp.o \
	fdlibm/e_fmod.o \
	fdlibm/e_gamma.o \
	fdlibm/e_gamma_r.o \
	fdlibm/e_hypot.o \
	fdlibm/e_j0.o \
	fdlibm/e_j1.o \
	fdlibm/e_jn.o \
	fdlibm/e_lgamma.o \
	fdlibm/e_lgamma_r.o \
	fdlibm/e_log.o \
	fdlibm/e_log10.o \
	fdlibm/e_pow.o \
	fdlibm/e_rem_pio2.o \
	fdlibm/e_remainder.o \
	fdlibm/e_scalb.o \
	fdlibm/e_sinh.o \
	fdlibm/e_sqrt.o \
	fdlibm/k_cos.o \
	fdlibm/k_rem_pio2.o \
	fdlibm/k_sin.o \
	fdlibm/k_standard.o \
	fdlibm/k_tan.o \
	fdlibm/s_asinh.o \
	fdlibm/s_atan.o \
	fdlibm/s_cbrt.o \
	fdlibm/s_ceil.o \
	fdlibm/s_copysign.o \
	fdlibm/s_cos.o \
	fdlibm/s_erf.o \
	fdlibm/s_expm1.o \
	fdlibm/s_fabs.o \
	fdlibm/s_finite.o \
	fdlibm/s_floor.o \
	fdlibm/s_frexp.o \
	fdlibm/s_ilogb.o \
	fdlibm/s_isnan.o \
	fdlibm/s_ldexp.o \
	fdlibm/s_lib_version.o \
	fdlibm/s_log1p.o \
	fdlibm/s_logb.o \
	fdlibm/s_matherr.o \
	fdlibm/s_modf.o \
	fdlibm/s_nextafter.o \
	fdlibm/s_rint.o \
	fdlibm/s_scalbn.o \
	fdlibm/s_signgam.o \
	fdlibm/s_significand.o \
	fdlibm/s_sin.o \
	fdlibm/s_tan.o \
	fdlibm/s_tanh.o \
	fdlibm/w_acos.o \
	fdlibm/w_acosh.o \
	fdlibm/w_asin.o \
	fdlibm/w_atan2.o \
	fdlibm/w_atanh.o \
	fdlibm/w_cosh.o \
	fdlibm/w_exp.o \
	fdlibm/w_fmod.o \
	fdlibm/w_gamma.o \
	fdlibm/w_gamma_r.o \
	fdlibm/w_hypot.o \
	fdlibm/w_j0.o \
	fdlibm/w_j1.o \
	fdlibm/w_jn.o \
	fdlibm/w_lgamma.o \
	fdlibm/w_lgamma_r.o \
	fdlibm/w_log.o \
	fdlibm/w_log10.o \
	fdlibm/w_pow.o \
	fdlibm/w_remainder.o \
	fdlibm/w_scalb.o \
	fdlibm/w_sinh.o \
	fdlibm/w_sqrt.o \
	jsapi.o \
	jsarena.o \
	jsarray.o \
	jsatom.o \
	jsbool.o \
	jscntxt.o \
	jscpucfg.o \
	jsdate.o \
	jsdbgapi.o \
	jsdhash.o \
	jsdtoa.o \
	jsemit.o \
	jsexn.o \
	jsfile.o \
	jsfun.o \
	jsgc.o \
	jshash.o \
	jsinterp.o \
	jslock.o \
	jslog2.o \
	jslong.o \
	jsmath.o \
	jsnum.o \
	jsobj.o \
	jsopcode.o \
	jsparse.o \
	jsprf.o \
	jsregexp.o \
	jsscan.o \
	jsscope.o \
	jsscript.o \
	jsstr.o \
	jsutil.o \
	jsxdrapi.o \
	prmjtime.o

js: js.o libjs.a
	$(CC) -o js js.o libjs.a -lm

libjs.a: $(OBJ)
	ar crv libjs.a $(OBJ)
	ranlib libjs.a

clean:
	$(RM) js libjs.a $(OBJ)

# ###########################
# DEPENDENCIES
# ###########################


fdlibm/e_acos.o: \
	fdlibm/e_acos.c \
	fdlibm/fdlibm.h

fdlibm/e_acosh.o: \
	fdlibm/e_acosh.c \
	fdlibm/fdlibm.h

fdlibm/e_asin.o: \
	fdlibm/e_asin.c \
	fdlibm/fdlibm.h

fdlibm/e_atan2.o: \
	fdlibm/e_atan2.c \
	fdlibm/fdlibm.h

fdlibm/e_atanh.o: \
	fdlibm/e_atanh.c \
	fdlibm/fdlibm.h

fdlibm/e_cosh.o: \
	fdlibm/e_cosh.c \
	fdlibm/fdlibm.h

fdlibm/e_exp.o: \
	fdlibm/e_exp.c \
	fdlibm/fdlibm.h

fdlibm/e_fmod.o: \
	fdlibm/e_fmod.c \
	fdlibm/fdlibm.h

fdlibm/e_gamma.o: \
	fdlibm/e_gamma.c \
	fdlibm/fdlibm.h

fdlibm/e_gamma_r.o: \
	fdlibm/e_gamma_r.c \
	fdlibm/fdlibm.h

fdlibm/e_hypot.o: \
	fdlibm/e_hypot.c \
	fdlibm/fdlibm.h

fdlibm/e_j0.o: \
	fdlibm/e_j0.c \
	fdlibm/fdlibm.h

fdlibm/e_j1.o: \
	fdlibm/e_j1.c \
	fdlibm/fdlibm.h

fdlibm/e_jn.o: \
	fdlibm/e_jn.c \
	fdlibm/fdlibm.h

fdlibm/e_lgamma.o: \
	fdlibm/e_lgamma.c \
	fdlibm/fdlibm.h

fdlibm/e_lgamma_r.o: \
	fdlibm/e_lgamma_r.c \
	fdlibm/fdlibm.h

fdlibm/e_log.o: \
	fdlibm/e_log.c \
	fdlibm/fdlibm.h

fdlibm/e_log10.o: \
	fdlibm/e_log10.c \
	fdlibm/fdlibm.h

fdlibm/e_pow.o: \
	fdlibm/e_pow.c \
	fdlibm/fdlibm.h

fdlibm/e_rem_pio2.o: \
	fdlibm/e_rem_pio2.c \
	fdlibm/fdlibm.h

fdlibm/e_remainder.o: \
	fdlibm/e_remainder.c \
	fdlibm/fdlibm.h

fdlibm/e_scalb.o: \
	fdlibm/e_scalb.c \
	fdlibm/fdlibm.h

fdlibm/e_sinh.o: \
	fdlibm/e_sinh.c \
	fdlibm/fdlibm.h

fdlibm/e_sqrt.o: \
	fdlibm/e_sqrt.c \
	fdlibm/fdlibm.h

fdlibm/k_cos.o: \
	fdlibm/k_cos.c \
	fdlibm/fdlibm.h

fdlibm/k_rem_pio2.o: \
	fdlibm/k_rem_pio2.c \
	fdlibm/fdlibm.h

fdlibm/k_sin.o: \
	fdlibm/k_sin.c \
	fdlibm/fdlibm.h

fdlibm/k_standard.o: \
	fdlibm/k_standard.c \
	fdlibm/fdlibm.h

fdlibm/k_tan.o: \
	fdlibm/k_tan.c \
	fdlibm/fdlibm.h

fdlibm/s_asinh.o: \
	fdlibm/s_asinh.c \
	fdlibm/fdlibm.h

fdlibm/s_atan.o: \
	fdlibm/s_atan.c \
	fdlibm/fdlibm.h

fdlibm/s_cbrt.o: \
	fdlibm/s_cbrt.c \
	fdlibm/fdlibm.h

fdlibm/s_ceil.o: \
	fdlibm/s_ceil.c \
	fdlibm/fdlibm.h

fdlibm/s_copysign.o: \
	fdlibm/s_copysign.c \
	fdlibm/fdlibm.h

fdlibm/s_cos.o: \
	fdlibm/s_cos.c \
	fdlibm/fdlibm.h

fdlibm/s_erf.o: \
	fdlibm/s_erf.c \
	fdlibm/fdlibm.h

fdlibm/s_expm1.o: \
	fdlibm/s_expm1.c \
	fdlibm/fdlibm.h

fdlibm/s_fabs.o: \
	fdlibm/s_fabs.c \
	fdlibm/fdlibm.h

fdlibm/s_finite.o: \
	fdlibm/s_finite.c \
	fdlibm/fdlibm.h

fdlibm/s_floor.o: \
	fdlibm/s_floor.c \
	fdlibm/fdlibm.h

fdlibm/s_frexp.o: \
	fdlibm/s_frexp.c \
	fdlibm/fdlibm.h

fdlibm/s_ilogb.o: \
	fdlibm/s_ilogb.c \
	fdlibm/fdlibm.h

fdlibm/s_isnan.o: \
	fdlibm/s_isnan.c \
	fdlibm/fdlibm.h

fdlibm/s_ldexp.o: \
	fdlibm/s_ldexp.c \
	fdlibm/fdlibm.h

fdlibm/s_lib_version.o: \
	fdlibm/s_lib_version.c \
	fdlibm/fdlibm.h

fdlibm/s_log1p.o: \
	fdlibm/s_log1p.c \
	fdlibm/fdlibm.h

fdlibm/s_logb.o: \
	fdlibm/s_logb.c \
	fdlibm/fdlibm.h

fdlibm/s_matherr.o: \
	fdlibm/s_matherr.c \
	fdlibm/fdlibm.h

fdlibm/s_modf.o: \
	fdlibm/s_modf.c \
	fdlibm/fdlibm.h

fdlibm/s_nextafter.o: \
	fdlibm/s_nextafter.c \
	fdlibm/fdlibm.h

fdlibm/s_rint.o: \
	fdlibm/s_rint.c \
	fdlibm/fdlibm.h

fdlibm/s_scalbn.o: \
	fdlibm/s_scalbn.c \
	fdlibm/fdlibm.h

fdlibm/s_signgam.o: \
	fdlibm/s_signgam.c \
	fdlibm/fdlibm.h

fdlibm/s_significand.o: \
	fdlibm/s_significand.c \
	fdlibm/fdlibm.h

fdlibm/s_sin.o: \
	fdlibm/s_sin.c \
	fdlibm/fdlibm.h

fdlibm/s_tan.o: \
	fdlibm/s_tan.c \
	fdlibm/fdlibm.h

fdlibm/s_tanh.o: \
	fdlibm/s_tanh.c \
	fdlibm/fdlibm.h

fdlibm/w_acos.o: \
	fdlibm/w_acos.c \
	fdlibm/fdlibm.h

fdlibm/w_acosh.o: \
	fdlibm/w_acosh.c \
	fdlibm/fdlibm.h

fdlibm/w_asin.o: \
	fdlibm/w_asin.c \
	fdlibm/fdlibm.h

fdlibm/w_atan2.o: \
	fdlibm/w_atan2.c \
	fdlibm/fdlibm.h

fdlibm/w_atanh.o: \
	fdlibm/w_atanh.c \
	fdlibm/fdlibm.h

fdlibm/w_cosh.o: \
	fdlibm/w_cosh.c \
	fdlibm/fdlibm.h

fdlibm/w_exp.o: \
	fdlibm/w_exp.c \
	fdlibm/fdlibm.h

fdlibm/w_fmod.o: \
	fdlibm/w_fmod.c \
	fdlibm/fdlibm.h

fdlibm/w_gamma.o: \
	fdlibm/w_gamma.c \
	fdlibm/fdlibm.h

fdlibm/w_gamma_r.o: \
	fdlibm/w_gamma_r.c \
	fdlibm/fdlibm.h

fdlibm/w_hypot.o: \
	fdlibm/w_hypot.c \
	fdlibm/fdlibm.h

fdlibm/w_j0.o: \
	fdlibm/w_j0.c \
	fdlibm/fdlibm.h

fdlibm/w_j1.o: \
	fdlibm/w_j1.c \
	fdlibm/fdlibm.h

fdlibm/w_jn.o: \
	fdlibm/w_jn.c \
	fdlibm/fdlibm.h

fdlibm/w_lgamma.o: \
	fdlibm/w_lgamma.c \
	fdlibm/fdlibm.h

fdlibm/w_lgamma_r.o: \
	fdlibm/w_lgamma_r.c \
	fdlibm/fdlibm.h

fdlibm/w_log.o: \
	fdlibm/w_log.c \
	fdlibm/fdlibm.h

fdlibm/w_log10.o: \
	fdlibm/w_log10.c \
	fdlibm/fdlibm.h

fdlibm/w_pow.o: \
	fdlibm/w_pow.c \
	fdlibm/fdlibm.h

fdlibm/w_remainder.o: \
	fdlibm/w_remainder.c \
	fdlibm/fdlibm.h

fdlibm/w_scalb.o: \
	fdlibm/w_scalb.c \
	fdlibm/fdlibm.h

fdlibm/w_sinh.o: \
	fdlibm/w_sinh.c \
	fdlibm/fdlibm.h

fdlibm/w_sqrt.o: \
	fdlibm/w_sqrt.c \
	fdlibm/fdlibm.h

js.o: \
	js.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsparse.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsapi.o: \
	jsapi.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsbool.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdate.h \
	jsdhash.h \
	jsdtoa.h \
	jsemit.h \
	jsexn.h \
	jsfile.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsmath.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsparse.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	prmjtime.h

jsarena.o: \
	jsarena.c \
	jsarena.h \
	jsbit.h \
	jscompat.h \
	jscpucfg.h \
	jshash.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsscope.h \
	jsstddef.h \
	jstypes.h \
	jsutil.h

jsarray.o: \
	jsarray.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsatom.o: \
	jsatom.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsbool.o: \
	jsbool.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsbool.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jscntxt.o: \
	jscntxt.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsexn.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jscpucfg.o: \
	jscpucfg.c

jsdate.o: \
	jsdate.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdate.h \
	jsdhash.h \
	jsdtoa.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	prmjtime.h

jsdbgapi.o: \
	jsdbgapi.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsdhash.o: \
	jsdhash.c \
	jsbit.h \
	jscpucfg.h \
	jsdhash.h \
	jsosdep.h \
	jsotypes.h \
	jstypes.h \
	jsutil.h

jsdtoa.o: \
	jsdtoa.c \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdtoa.h \
	jslibmath.h \
	jslong.h \
	jsnum.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jspubtd.h \
	jsstddef.h \
	jstypes.h \
	jsutil.h

jsemit.o: \
	jsemit.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsbit.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsparse.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsexn.o: \
	jsexn.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsbit.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsexn.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsfile.o: \
	jsfile.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdate.h \
	jsdbgapi.h \
	jsdhash.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsparse.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsfun.o: \
	jsfun.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsbit.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsexn.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsparse.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	jsxdrapi.h

jsgc.o: \
	jsgc.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jshash.o: \
	jshash.c \
	jsbit.h \
	jscompat.h \
	jscpucfg.h \
	jshash.h \
	jslong.h \
	jsosdep.h \
	jsotypes.h \
	jsstddef.h \
	jstypes.h \
	jsutil.h

jsinterp.o: \
	jsinterp.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsbool.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jslock.o: \
	jslock.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsbit.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsdtoa.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jslog2.o: \
	jslog2.c \
	jsbit.h \
	jscpucfg.h \
	jsosdep.h \
	jsotypes.h \
	jsstddef.h \
	jstypes.h

jslong.o: \
	jslong.c \
	jscpucfg.h \
	jslong.h \
	jsosdep.h \
	jsotypes.h \
	jsstddef.h \
	jstypes.h

jsmath.o: \
	jsmath.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslibmath.h \
	jslock.h \
	jslong.h \
	jsmath.h \
	jsnum.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	prmjtime.h

jsnum.o: \
	jsnum.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsdtoa.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsobj.o: \
	jsobj.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsbool.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	jsxdrapi.h

jsopcode.o: \
	jsopcode.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsdtoa.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsparse.o: \
	jsparse.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsparse.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsprf.o: \
	jsprf.c \
	jscpucfg.h \
	jslong.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsstddef.h \
	jstypes.h \
	jsutil.h

jsregexp.o: \
	jsregexp.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	jsxdrapi.h

jsscan.o: \
	jsscan.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsdtoa.h \
	jsemit.h \
	jsexn.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscan.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsscope.o: \
	jsscope.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsbit.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsscript.o: \
	jsscript.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdbgapi.h \
	jsdhash.h \
	jsemit.h \
	jsfun.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	jsxdrapi.h

jsstr.o: \
	jsstr.c \
	jsapi.h \
	jsarena.h \
	jsarray.h \
	jsatom.h \
	jsbool.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsnum.h \
	jsobj.h \
	jsopcode.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h

jsutil.o: \
	jsutil.c \
	jscpucfg.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsstddef.h \
	jstypes.h \
	jsutil.h

jsxdrapi.o: \
	jsxdrapi.c \
	jsapi.h \
	jsarena.h \
	jsatom.h \
	jsclist.h \
	jscntxt.h \
	jscompat.h \
	jsconfig.h \
	jscpucfg.h \
	jsdhash.h \
	jsgc.h \
	jshash.h \
	jsinterp.h \
	jslock.h \
	jslong.h \
	jsobj.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsprvtd.h \
	jspubtd.h \
	jsregexp.h \
	jsscope.h \
	jsscript.h \
	jsstddef.h \
	jsstr.h \
	jstypes.h \
	jsutil.h \
	jsxdrapi.h

prmjtime.o: \
	prmjtime.c \
	jscompat.h \
	jscpucfg.h \
	jslong.h \
	jsosdep.h \
	jsotypes.h \
	jsprf.h \
	jsstddef.h \
	jstypes.h \
	jsutil.h \
	prmjtime.h


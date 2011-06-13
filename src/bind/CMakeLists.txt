
set(bind_SRC
	dobinding.cpp
	javabind.cpp


	# -------
	# Headers
	javabind-private.h
	javabind.h
	javainc/jni.h
	javainc/linux/jni_md.h
	javainc/solaris/jni_md.h
	javainc/win32/jni_md.h
)

# add_inkscape_lib(bind_LIB "${bind_SRC}")
add_inkscape_source("${bind_SRC}")

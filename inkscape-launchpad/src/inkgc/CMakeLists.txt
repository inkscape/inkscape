set(libgc_SRC
	gc.cpp

	# -------
	# Headers
	gc-alloc.h
	../gc-anchored.h
	gc-core.h
	gc-managed.h
	gc-soft-ptr.h
)

add_inkscape_lib(gc_LIB "${libgc_SRC}")
# add_inkscape_source("${libgc_SRC}")

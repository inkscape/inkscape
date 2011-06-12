set(cxxtest_SRC
	#add stuff here
)

# Add New folders in src folder here
set(cxxtestfolders
	cxxtest
	docs
	sample
)

foreach(cxxtestdirlist ${cxxtestfolders})
	add_subdirectory(${cxxtestdirlist})
endforeach()

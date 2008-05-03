SET(cxxtest_SRC
#add stuff here
)

# Add New folders in src folder here
SET(cxxtestfolders
cxxtest
docs
sample
)

FOREACH(cxxtestdirlist ${cxxtestfolders})
ADD_SUBDIRECTORY(${cxxtestdirlist})
ENDFOREACH(cxxtestdirlist)

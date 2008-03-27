SET(BIND_SRC
dobinding.cpp
javabind.cpp
#javabind.h
#javabind-private.h
)

# Add New folders in src folder here
SET(bindfolders
java
javainc
)

FOREACH(binddirlist ${bindfolders})
ADD_SUBDIRECTORY(${binddirlist})
ENDFOREACH(binddirlist)

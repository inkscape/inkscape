#  POTRACE_FOUND - system has Potrace
#  POTRACE_INCLUDE_DIRS - the Potrace include directory
#  POTRACE_LIBRARIES - The libraries needed to use Potrace

IF (POTRACE_LIBRARIES AND POTRACE_INCLUDE_DIRS)
   # in cache already
   SET(POTRACE_FOUND TRUE)
ELSE (POTRACE_LIBRARIES AND POTRACE_INCLUDE_DIRS)
   FIND_PATH (POTRACE_INCLUDE_DIR
      NAMES
         potracelib.h
      PATHS
         /usr/include
         /usr/local/include
	 $ENV{DEVLIBS_PATH}/include
      PATH_SUFFIXES
         potrace
   )

   FIND_LIBRARY (POTRACE_LIBRARY
      NAMES
        potrace
        libpotrace
      PATHS
         /usr/lib
         /usr/local/lib
         $ENV{DEVLIBS_PATH}/lib
   )

   if (POTRACE_LIBRARY)
      set (POTRACE_FOUND TRUE)
   endif (POTRACE_LIBRARY)

   set (POTRACE_INCLUDE_DIRS
      ${POTRACE_INCLUDE_DIR}
   )

   if (POTRACE_FOUND)
      set(POTRACE_LIBRARIES
        ${POTRACE_LIBRARIES}
        ${POTRACE_LIBRARY}
      )
   endif (POTRACE_FOUND)

   if (POTRACE_INCLUDE_DIRS AND POTRACE_LIBRARIES)
      set(POTRACE_FOUND TRUE)
   endif (POTRACE_INCLUDE_DIRS AND POTRACE_LIBRARIES)

   if (POTRACE_FOUND)
      if (NOT Potrace_FIND_QUIETLY)
         message(STATUS "Found Potrace: ${POTRACE_LIBRARIES}")
      endif (NOT Potrace_FIND_QUIETLY)
   else (POTRACE_FOUND)
      if (Potrace_FIND_REQUIRED)
	      message(FATAL_ERROR "Could not find potrace")
      endif (Potrace_FIND_REQUIRED)
   endif (POTRACE_FOUND)

   # show the POTRACE_INCLUDE_DIRS and POTRACE_LIBRARIES variables only in the advanced view
   MARK_AS_ADVANCED(POTRACE_INCLUDE_DIRS POTRACE_LIBRARIES)

endif (POTRACE_LIBRARIES AND POTRACE_INCLUDE_DIRS)

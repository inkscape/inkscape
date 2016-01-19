set(_generated
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake  
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake  
    ${CMAKE_CURRENT_BINARY_DIR}/po/cmake_install.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles
)

message("${_generated}")

foreach(file ${_generated})
    if(EXISTS ${file})
	message("Removing ${file}")
	file(REMOVE_RECURSE ${file})
    endif()
endforeach(file)

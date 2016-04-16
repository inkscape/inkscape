set(grid_PART_SRCS grid.cpp)                                                                      

include_directories( ${CMAKE_BINARY_DIR}/src )

add_library(grid2 SHARED ${grid_PART_SRCS})
                                                                                                              
target_link_libraries(grid2 inkscape_base)                                                  
                                                                                                              

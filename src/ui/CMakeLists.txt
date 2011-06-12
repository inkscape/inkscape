set(uifolders
	cache
	dialog
	view
	widget
)
foreach(uilistsrc ${uifolders})
	add_subdirectory(${uilistsrc})
endforeach()

set(ui_SRC
	clipboard.cpp
	context-menu.cpp
	previewholder.cpp
	uxmanager.cpp

	tool/control-point-selection.cpp
	tool/control-point.cpp
	tool/curve-drag-point.cpp
	tool/event-utils.cpp
	tool/manipulator.cpp
	tool/modifier-tracker.cpp
	tool/multi-path-manipulator.cpp
	tool/node-tool.cpp
	tool/node.cpp
	tool/path-manipulator.cpp
	tool/selectable-control-point.cpp
	tool/selector.cpp
	tool/transform-handle-set.cpp
	
	${ui_cache_SRC}
	${ui_dialog_SRC}
	${ui_view_SRC}
	${ui_widget_SRC}
)

include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/bind/javainc"
	"${CMAKE_SOURCE_DIR}/bind/javainc/linux"
	"${CMAKE_SOURCE_DIR}/extension/dbus"
)

add_library(ui_LIB STATIC ${ui_SRC})
target_link_libraries(ui_LIB 2geom_LIB ${INKSCAPE_LIBS})

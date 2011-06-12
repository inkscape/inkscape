add_subdirectory(dialog)

set(jabber_whiteboard_SRC
	defines.cpp
	empty.cpp
	inkboard-document.cpp
	inkboard-node.cpp
	invitation-confirm-dialog.cpp
	keynode.cpp
	message-aggregator.cpp
	message-queue.cpp
	message-tags.cpp
	message-utilities.cpp
	#node-tracker.cpp
	#node-utilities.cpp
	pedrogui.cpp
	session-file-selector.cpp
	session-manager.cpp
	${jabber_whiteboard_dialog_SRC}
)

add_library(jabber_whiteboard_LIB STATIC ${jabber_whiteboard_SRC})
target_link_libraries(jabber_whiteboard_LIB 2geom_LIB ${INKSCAPE_LIBS})

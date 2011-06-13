
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

	dialog/choose-desktop.cpp
)

# add_library(jabber_whiteboard_LIB ${jabber_whiteboard_SRC})
add_inkscape_source("${jabber_whiteboard_SRC}")

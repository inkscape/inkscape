
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


	# -------
	# Headers
	defines.h
	dialog/choose-desktop.h
	inkboard-document.h
	invitation-confirm-dialog.h
	keynode.h
	message-aggregator.h
	message-node.h
	message-queue.h
	message-tags.h
	message-utilities.h
	message-verifier.h
	node-tracker.h
	node-utilities.h
	pedrogui.h
	session-file-selector.h
	session-manager.h
	tracker-node.h
)

# add_inkscape_lib(jabber_whiteboard_LIB "${jabber_whiteboard_SRC}")
add_inkscape_source("${jabber_whiteboard_SRC}")

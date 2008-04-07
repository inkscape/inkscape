SET(jabber_whiteboard_SRC
defines.cpp
defines.h
empty.cpp
inkboard-document.cpp
inkboard-document.h
inkboard-node.cpp
invitation-confirm-dialog.cpp
invitation-confirm-dialog.h
keynode.cpp
keynode.h
message-aggregator.cpp
message-aggregator.h
message-node.h
message-queue.cpp
message-queue.h
message-tags.cpp
message-tags.h
message-utilities.cpp
message-utilities.h
message-verifier.h
node-tracker.cpp
node-tracker.h
node-utilities.cpp
node-utilities.h
pedrogui.cpp
pedrogui.h
session-file-selector.cpp
session-file-selector.h
session-manager.cpp
session-manager.h
tracker-node.h
${jabber_whiteboard_dialog_SRC}
)

# Add New folders in src folder here
SET(jabber_whiteboardfolders
dialog
)

FOREACH(jabber_whiteboarddirlist ${jabber_whiteboardfolders})
ADD_SUBDIRECTORY(${jabber_whiteboarddirlist})
ENDFOREACH(jabber_whiteboarddirlist)
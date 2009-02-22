ADD_SUBDIRECTORY(dialog)

SET(jabber_whiteboard_SRC
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
ADD_LIBRARY(jabber_whiteboard STATIC ${jabber_whiteboard_SRC})
TARGET_LINK_LIBRARIES(jabber_whiteboard
2geom ${INKSCAPE_LIBS})
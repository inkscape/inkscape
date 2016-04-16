if(WITH_DBUS)
include_directories(${CMAKE_BINARY_DIR}/src/extension/dbus)
set(dbus_SRC "")
       list(APPEND dbus_SRC
                application-interface.cpp
                dbus-init.cpp
                document-interface.cpp
        )
        add_custom_target(inkscape_dbus 
            DEPENDS ${CMAKE_BINARY_DIR}/src/extension/dbus/application-server-glue.h ${CMAKE_BINARY_DIR}/src/extension/dbus/document-server-glue.h ${CMAKE_BINARY_DIR}/src/extension/dbus/document-client-glue.h 
            )
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/src/extension/dbus/application-server-glue.h ${CMAKE_BINARY_DIR}/src/extension/dbus/document-server-glue.h ${CMAKE_BINARY_DIR}/src/extension/dbus/document-client-glue.h
            DEPENDS ${CMAKE_SOURCE_DIR}/src/extension/dbus/application-interface.xml ${CMAKE_SOURCE_DIR}/src/extension/dbus/document-interface.xml
            COMMAND dbus-binding-tool --mode=glib-server --output=${CMAKE_BINARY_DIR}/src/extension/dbus/application-server-glue.h --prefix=application_interface ${CMAKE_SOURCE_DIR}/src/extension/dbus/application-interface.xml
            COMMAND dbus-binding-tool --mode=glib-server --output=${CMAKE_BINARY_DIR}/src/extension/dbus/document-server-glue.h --prefix=document_interface ${CMAKE_SOURCE_DIR}/src/extension/dbus/document-interface.xml
            COMMAND dbus-binding-tool --mode=glib-client --output=${CMAKE_BINARY_DIR}/src/extension/dbus/document-client-glue.h --prefix=document_interface ${CMAKE_SOURCE_DIR}/src/extension/dbus/document-interface.xml
        )
        set_source_files_properties(
                ${CMAKE_BINARY_DIR}/src/extension/dbus/application-server-glue.h
                PROPERTIES GENERATED TRUE)
        set_source_files_properties(
                ${CMAKE_BINARY_DIR}/src/extension/dbus/document-server-glue.h
                PROPERTIES GENERATED TRUE)
        set_source_files_properties(
                ${CMAKE_BINARY_DIR}/src/extension/dbus/document-client-glue.h
                PROPERTIES GENERATED TRUE)
add_inkscape_source("${dbus_SRC}")
endif()

if (NOT "${WITH_EXT_GDL}")

    set(libgdl_SRC
        gdl-dock-bar.c
        gdl-dock-item-button-image.c
        gdl-dock-item-grip.c
        gdl-dock-item.c
        gdl-dock-master.c
        gdl-dock-notebook.c
        gdl-dock-object.c
        gdl-dock-paned.c
        gdl-dock-placeholder.c
        gdl-dock-tablabel.c
        gdl-dock.c
        gdl-i18n.c
        gdl-switcher.c
        libgdlmarshal.c
        libgdltypebuiltins.c


        # -------
        # Headers
        gdl-dock-bar.h
        gdl-dock-item-button-image.h
        gdl-dock-item-grip.h
        gdl-dock-item.h
        gdl-dock-master.h
        gdl-dock-notebook.h
        gdl-dock-object.h
        gdl-dock-paned.h
        gdl-dock-placeholder.h
        gdl-dock-tablabel.h
        gdl-dock.h
        gdl-i18n.h
        gdl-switcher.h
        gdl.h
        libgdlmarshal.h
        libgdltypebuiltins.h
    )

    if(WIN32)
        list(APPEND libgdl_SRC
            gdl-win32.c
            gdl-win32.h
        )
    endif()

    add_inkscape_lib(gdl_LIB "${libgdl_SRC}")

endif()

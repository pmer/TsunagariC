if (WIN32)
elseif(APPLE)
    set(PACK_SOURCES ${PACK_SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/unix.cpp
    )
    set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/unix.cpp
    )
else()
    set(PACK_SOURCES ${PACK_SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/unix.cpp
    )
    set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/unix.cpp
    )
endif(WIN32)

if(APPLE)
    set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/mac-gui.mm
    )

    set(GUI_TYPE MACOSX_BUNDLE)
endif(APPLE)

set(PACK_EXE_NAME pack)

set(PACK_SOURCES ${PACK_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/file-type.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pack-file.cpp
)

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/file-type.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pack-file.cpp
)

add_executable(${PACK_EXE_NAME} ${PACK_SOURCES})

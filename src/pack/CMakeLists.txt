set(PACK_EXE_NAME pack-tool)

find_package(Threads)

set(PACK_SOURCES ${PACK_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/file-type.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pack-file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui-log.cpp
)

set(PACK_LIBRARIES
    Threads::Threads
)

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/file-type.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pack-file.cpp
)

add_executable(${PACK_EXE_NAME} ${PACK_SOURCES})
target_link_libraries(${PACK_EXE_NAME} ${PACK_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBACKEND_GOSU")

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/gosu-cbuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-images.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-music.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-sounds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-window.cpp
)

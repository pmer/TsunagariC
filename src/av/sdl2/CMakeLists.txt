pkg_search_module(SDL2 REQUIRED SDL2)
pkg_search_module(SDL2_image REQUIRED SDL2_image)
pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${SDL2_mixer_LIBRARIES}
)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_image_INCLUDE_DIRS}
    ${SDL2_mixer_INCLUDE_DIRS}
)

link_directories(
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_image_LIBRARY_DIRS}
    ${SDL2_mixer_LIBRARY_DIRS}
)

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/error.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images.cpp
    ${CMAKE_CURRENT_LIST_DIR}/music.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sounds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/window.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBACKEND_SDL2")

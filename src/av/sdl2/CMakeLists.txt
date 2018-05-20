pkg_search_module(sdl2 sdl2)  # Fedora
pkg_search_module(SDL2 SDL2)  # macOS
pkg_search_module(SDL2_image REQUIRED SDL2_image)
pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${sdl2_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${SDL2_mixer_LIBRARIES}
)

include_directories(
    ${sdl2_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_image_INCLUDE_DIRS}
    ${SDL2_mixer_INCLUDE_DIRS}
)

link_directories(
    ${sdl2_LIBRARY_DIRS}
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

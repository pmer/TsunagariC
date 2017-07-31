include_directories(${CMAKE_CURRENT_LIST_DIR})

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/animation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/area.cpp
    ${CMAKE_CURRENT_LIST_DIR}/area-json.cpp
    ${CMAKE_CURRENT_LIST_DIR}/character.cpp
    ${CMAKE_CURRENT_LIST_DIR}/client-conf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cooldown.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dtds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/entity.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formatter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images.cpp
    ${CMAKE_CURRENT_LIST_DIR}/jsons.cpp
    ${CMAKE_CURRENT_LIST_DIR}/log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/measure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/music-impl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/music-worker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/npc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/overlay.cpp
    ${CMAKE_CURRENT_LIST_DIR}/player.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resources.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sounds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/viewport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/world.cpp
)

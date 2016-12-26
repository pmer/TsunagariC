include_directories(${CMAKE_CURRENT_LIST_DIR})

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/nbcl/nbcl.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/av/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/core/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/data/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/os/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/pack/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/resources/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/util/CMakeLists.txt)

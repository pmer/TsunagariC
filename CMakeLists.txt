cmake_minimum_required(VERSION 3.6)

find_package(PkgConfig REQUIRED)
find_package(gosu REQUIRED)
find_package(libxml2 REQUIRED)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${GOSU_LIBRARIES}
    ${GOSU_DEPENDENCIES}
    ${LIBXML2_LIBRARIES}
    physfs
)

include_directories(
    ${GOSU_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
)

link_directories(
    ${GOSU_LIBRARY_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")

include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)

#all: depends link zip client
#depends: Makefile.depends
#compile: $(OBJS)
#link: bin/$(EXE)
#zip: bin/$(ZIP)
#client: bin/client.ini
#clean:
#rm -rf bin Makefile.depends
#find . -name \*.o -delete
#.PHONY: all compile link zip client clean

#Makefile.depends:
#$(CXX) $(CXXFLAGS) -MM $(filter %.cpp,$(SRCS)) | $(TSU_DIR)/scripts/filter-depend.rb > Makefile.depends
#ifneq (,$(filter %.m,$(SRCS)))
#$(CXX) -MM $(filter %.m,$(SRCS)) | $(TSU_DIR)/scripts/filter-depend.rb >> Makefile.depends
#endif

#bin/$(EXE): $(OBJS)
#@mkdir -p bin
#$(LINK.cc) -o bin/$(EXE) $(OBJS)

#bin/$(ZIP): $(DATA)
#@mkdir -p bin
#(cd data; zip --symlinks -0 ../bin/$(ZIP) $(?:data/%=%))

#bin/client.ini: $(TSU_DIR)/data/client.ini
#@mkdir -p bin
#cp $< $@

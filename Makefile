TSU_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

CXXFLAGS_XML2 := $(shell xml2-config --cflags)
CXXFLAGS_SDL2 := $(shell pkg-config --cflags sdl2)

CXXFLAGS += -g
CXXFLAGS += -pipe
CXXFLAGS += -pedantic
CXXFLAGS += -std=c++14
CXXFLAGS += -I/usr/local/include
CXXFLAGS += $(CXXFLAGS_XML2)
CXXFLAGS += $(CXXFLAGS_SDL2)

CXXFLAGS += -Wall
CXXFLAGS += -Wextra
CXXFLAGS += -Wconversion
CXXFLAGS += -Wdeprecated

LDFLAGS += -L/usr/local/lib
LDFLAGS += -lboost_program_options
LDFLAGS += -lphysfs
LDFLAGS += -liconv
LDFLAGS += $(shell xml2-config --libs)
LDFLAGS += $(shell pkg-config --libs sdl2)
LDFLAGS += $(shell pkg-config --libs gosu)

# Recursive wildcard that emulates /usr/bin/find
# Usage: FILES := $(call rwildcard,path/,*.ext)
rwildcard = $(patsubst ./%,%,$(call rwildcard_,$1,$2))
rwildcard_ = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard_,$d/,$2))

SRCS := $(call rwildcard,./,*.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
DATA := $(call rwildcard,data/,*)

OS := $(shell uname -s)

ifeq ($(OS),Darwin)
	SRCS += $(TSU_DIR)/src/os-mac.m
	OBJS += $(TSU_DIR)/src/os-mac.o
	LDFLAGS += -framework Foundation
	LDFLAGS += -framework AppKit
	LDFLAGS += -framework OpenAL
	LDFLAGS += -framework AudioToolbox
	LDFLAGS += -framework OpenGL
endif

all: depends link zip client
depends: Makefile.depends
compile: $(OBJS)
link: bin/$(EXE)
zip: bin/$(ZIP)
client: bin/client.ini
clean:
	rm -rf bin Makefile.depends
	find . -name \*.o -delete
.PHONY: all compile link zip client clean

Makefile.depends:
	$(CXX) $(CXXFLAGS) -MM $(filter %.cpp,$(SRCS)) | $(TSU_DIR)/scripts/filter-depend.rb > Makefile.depends
ifneq (,$(filter %.m,$(SRCS)))
	$(CXX) -MM $(filter %.m,$(SRCS)) | $(TSU_DIR)/scripts/filter-depend.rb >> Makefile.depends
endif

bin/$(EXE): $(OBJS)
	@mkdir -p bin
	$(LINK.cc) -o bin/$(EXE) $(OBJS)

bin/$(ZIP): $(DATA)
	@mkdir -p bin
	(cd data; zip --symlinks -0 ../bin/$(ZIP) $(?:data/%=%))

bin/client.ini: $(TSU_DIR)/data/client.ini
	@mkdir -p bin
	cp $< $@

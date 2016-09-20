DEBUG ?= 0
DIST_DIR := dist
BUILD_DIR := build

CXX := ccache clang++-3.8
CXXFLAGS := -std=c++14
LDFLAGS := 
LDLIBS := -lpthread -lm

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -Wl,-export-dynamic -pedantic
	LDFLAGS += ../mf/dist/libmf_debug.so
else
	CXXFLAGS += -O3 -DNDEBUG
	LDFLAGS += ../mf/dist/libmf.so
endif

CXXFLAGS += -I../mf/external/include -I../mf/dist/include

PACKAGES := opencv eigen3 yaml-cpp
CXXFLAGS += $(shell pkg-config --cflags $(PACKAGES))
LDFLAGS += $(shell pkg-config --libs-only-L $(PACKAGES))
LDLIBS += $(shell pkg-config --libs-only-l --libs-only-other $(PACKAGES))

EXEC := $(DIST_DIR)/view_syn
EXEC_SRC := $(shell find src -name '*.cc')
EXEC_OBJ := $(patsubst src/%.cc,$(BUILD_DIR)/src/%.o,$(EXEC_SRC))
DEP += $(patsubst %.cc,$(BUILD_DIR)/%.d,$(EXEC_SRC))

export DEBUG


all : $(EXEC)
	

clean :
	rm -rf ./build/ ./dist/


mf :
	$(MAKE) -C ../mf library

$(EXEC) : mf $(EXEC_OBJ)
	mkdir -p $(dir $@) && \
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(EXEC_OBJ) $(LIB_OBJ) $(LDLIBS)
	

$(BUILD_DIR)/src/%.o : src/%.cc
	mkdir -p $(dir $@) && \
	$(CXX) $(CXXFLAGS) -c -o $@ $< -MMD


.PHONY: clean mf


-include $(DEP)

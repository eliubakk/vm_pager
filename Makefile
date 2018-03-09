CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -Werror
DEBUGFLAGS = -DDEBUG

APP_LIBS = libvm_app.o -ldl 
PAGER_LIBS = libvm_pager.o -ldl

EXECUTABLE = pager

PROJECTFILE = pager.cc

TESTSOURCES = $(wildcard test*.cc)
TESTS = $(TESTSOURCES:%.cc=%)

SOURCES = $(wildcard *.cc)
SOURCES := $(filter-out $(TESTSOURCES), $(SOURCES))
OBJECTS = $(SOURCES:%.cc=%.o)

all: $(EXECUTABLE)

release: CXXFLAGS += -DNDEBUG
release: all

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: clean all

$(EXECUTABLE): $(OBJECTS) #$(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(PAGER_LIBS) -o $(EXECUTABLE)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $*.cc

define make_tests
    SRCS = $$(filter-out $$(PROJECTFILE), $$(SOURCES))
    OBJS = $$(SRCS:%.cc=%.o)
    HDRS = $$(wildcard *.h)
    $(1): CXXFLAGS += $$(DEBUGFLAGS)
    $(1): $$(HDRS) $$(OBJS) $(1).cc
	$$(CXX) $$(CXXFLAGS) $$(APP_LIBS) $$(OBJS) $(1).cc -o $(1)
endef
$(foreach test, $(TESTS), $(eval $(call make_tests, $(test))))

alltests: clean $(TESTS)

clean: 
	rm -f $(OBJECTS) $(EXECUTABLE) $(TESTS)

.PHONY: all clean alltests debug release
.SUFFIXES:
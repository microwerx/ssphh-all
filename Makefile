# requires the following packages
# libczmq-dev libzmq3-dev libcurl4-gnutls-dev libsodium-dev zlib1g-dev python3-dev freeglut3-dev libglew-dev openexr-dev libopenimageio-dev
# development packages
# global

#INCLUDE_PROJECTS = hatchetfish fluxions-deps fluxions-gte damselfish
#INCLUDES = $(patsubst %,-I../%/include,$(INCLUDE_PROJECTS))
$(PROJECTDIR)
$(GITHUBDIR)
PROJECTDIR=ssphh/
GITHUBDIR=../
INCLUDES = -I$(GITHUBDIR)imgui -I$(GITHUBDIR)fluxions/include
SRCDIR = $(PROJECTDIR)src
INCDIR = $(PROJECTDIR)include
OBJDIR = $(PROJECTDIR)build
CXXSOURCES = $(wildcard $(SRCDIR)/*.cpp)
CXXHEADERS = $(wildcard $(INCDIR)/*.hpp)
CSOURCES = $(wildcard $(SRCDIR)/*.c)
CHEADERS = $(wildcard $(INCDIR)/*.h)
SOURCES = $(CXXSOURCES) $(CSOURCES)
HEADERS = $(CXXHEADERS) $(CHEADERS)
SRCOBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CXXSOURCES)) $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CSOURCES))
OBJECTS = $(SRCOBJECTS)
TARGET = $(PROJECTDIR)build/ssphh-debug
GCH = $(SRCDIR)/pch.h.gch
GCH_SRC = $(SRCDIR)/pch.hpp

MACINCDIRS = -I/usr/local/opt/mesa/include

CC = gcc
CCFLAGS = -Wall -I$(INCDIR) $(MACINCDIRS) `python3-config --includes`
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -I$(INCDIR) $(INCLUDES) $(MACINCDIRS) `python3-config --includes`
LDFLAGS = -LGLEW -LGL -LGLU -Lglut -lIlmImf
LDFLAGS = -L../imgui -L../fluxions/build -lfluxions -lGL -lpthread -lGLEW -lczmq -lSDL2 -lSDL2_image ../imgui/imgui.a -lcurl -lglfw

.PHONY: all clean precompiled

all: GTAGS $(GCH) $(TARGET)

precompiled: $(GCH)
	echo $(SRCOBJECTS)
	echo DEPOBJECTS =======================
	echo $(DEPOBJECTS)

cobjects: $(DEPCOBJECTS)
	echo $(DEPCOBJECTS)
	echo
	echo $(DEPOBJECTS)

$(TARGET): $(OBJECTS)
#	$(AR) cr $@ $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

$(GCH): $(GCH_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# $(OBJDIR)/%.o: $(CXXSOURCES)/%.cpp $(GCH)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(GCH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(DEP_SRCDIR)/%.cpp $(GCH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(GCH)
	$(CC) $(CCFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(DEP_SRCDIR)/%.c $(GCH)
	$(CC) $(CCFLAGS) -c $< -o $@

GTAGS: $(SOURCES) $(HEADERS)
	gtags

clean:
	$(RM) -f $(GCH)
	$(RM) -f $(OBJDIR)/*.o
	$(RM) -f $(TARGET)
	$(RM) -f GPATH GRTAGS GTAGS

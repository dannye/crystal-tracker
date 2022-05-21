DESTDIR =
PREFIX = /usr/local

crystaltracker = crystaltracker
crystaltrackerd = crystaltrackerd

CXX ?= g++
LD = $(CXX)
RM = rm -rf

srcdir = src
resdir = res
tmpdir = tmp
debugdir = tmp/debug
bindir = bin

fltk-config = $(bindir)/fltk-config

CXXFLAGS = -std=c++17 -I$(srcdir) -I$(resdir) $(shell $(fltk-config) --use-images --cxxflags)
LDFLAGS = $(shell $(fltk-config) --use-images --ldflags) $(shell pkg-config --libs libpng xpm)

RELEASEFLAGS = -DNDEBUG -O3 -flto -march=native
DEBUGFLAGS = -DDEBUG -D_DEBUG -O0 -g -ggdb3 -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter

COMMON = $(wildcard $(srcdir)/*.h) $(wildcard $(resdir)/*.xpm)
SOURCES = $(wildcard $(srcdir)/*.cpp)
OBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(tmpdir)/%.o)
DEBUGOBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(debugdir)/%.o)
TARGET = $(bindir)/$(crystaltracker)
DEBUGTARGET = $(bindir)/$(crystaltrackerd)
DESKTOP = "$(DESTDIR)$(PREFIX)/share/applications/Crystal Tracker.desktop"

.PHONY: all $(crystaltracker) $(crystaltrackerd) release debug clean install uninstall

.SUFFIXES: .o .cpp

all: $(crystaltracker)

$(crystaltracker): release
$(crystaltrackerd): debug

release: CXXFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(DEBUGTARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(LDFLAGS)

$(DEBUGTARGET): $(DEBUGOBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(LDFLAGS)

$(tmpdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(debugdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET) $(DEBUGTARGET) $(OBJECTS) $(DEBUGOBJECTS)

install: release
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(crystaltracker)
	mkdir -p $(DESTDIR)$(PREFIX)/share/pixmaps
	cp $(resdir)/app.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/crystaltracker48.xpm
	cp $(resdir)/app-icon.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/crystaltracker16.xpm
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	echo "[Desktop Entry]" > $(DESKTOP)
	echo "Name=Crystal Tracker" >> $(DESKTOP)
	echo "Comment=Edit pokecrystal music and sound effects" >> $(DESKTOP)
	echo "Icon=$(PREFIX)/share/pixmaps/crystaltracker48.xpm" >> $(DESKTOP)
	echo "Exec=$(PREFIX)/bin/$(crystaltracker)" >> $(DESKTOP)
	echo "Type=Application" >> $(DESKTOP)
	echo "Terminal=false" >> $(DESKTOP)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(crystaltracker)
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/crystaltracker48.xpm
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/crystaltracker16.xpm
	rm -f $(DESKTOP)

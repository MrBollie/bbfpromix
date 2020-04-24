#!/usr/bin/make -f

CFLAGS ?= `pkg-config alsa gtk+-3.0 --cflags`
LDFLAGS ?= `pkg-config alsa gtk+-3.0 --libs`

include Makefile.mk

# --------------------------------------------------------------

PREFIX  ?= /usr/local
DESTDIR ?= 
BUILDDIR ?= build

# --------------------------------------------------------------
# Default target is to build all plugins

all: bbfpromix

# --------------------------------------------------------------
# bolliedelay build rules

bbfpromix: $(BUILDDIR) $(BUILDDIR)/bbfpromix $(BUILDDIR)/bbfpromix.desktop

$(BUILDDIR)/channel.o: src/channel.c
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm -o $@ -c

$(BUILDDIR)/main.o: src/main.c
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm -o $@ -c

$(BUILDDIR)/bbfpromix: $(BUILDDIR)/channel.o $(BUILDDIR)/main.o
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm $(SHARED) -o $@

$(BUILDDIR)/bbfpromix.desktop: misc/bbfpromix.desktop
	sed -s "s#@bindir@#$(DESTDIR)$(PREFIX)/bin#g" $^ > $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# --------------------------------------------------------------

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/bbfpromix*

# --------------------------------------------------------------

install: bbfpromix
	echo "Install"
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/applications
	install -d $(DESTDIR)$(PREFIX)/share/pixmaps

	install -m 755 $(BUILDDIR)/bbfpromix  $(DESTDIR)$(PREFIX)/bin/bbfpromix
	install -m 644 ./misc/icon.png  $(DESTDIR)$(PREFIX)/share/pixmaps/bbfpromix.png
	install -m 644 $(BUILDDIR)/bbfpromix.desktop  $(DESTDIR)$(PREFIX)/share/applications/bbfpromix.desktop

# --------------------------------------------------------------
uninstall:
	echo "Uninstall"
	rm -f $(DESTDIR)$(PREFIX)/bin/bbfpromix
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/bbfpromix.png
	rm -f $(DESTDIR)$(PREFIX)/share/applications/bbfpromix.desktop
	
# --------------------------------------------------------------

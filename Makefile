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

bbfpromix: $(BUILDDIR) $(BUILDDIR)/bbfpromix

$(BUILDDIR)/channel.o: src/channel.c
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm -o $@ -c

$(BUILDDIR)/main.o: src/main.c
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm -o $@ -c

$(BUILDDIR)/bbfpromix: $(BUILDDIR)/channel.o $(BUILDDIR)/main.o
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm $(SHARED) -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# --------------------------------------------------------------

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/bbfpromix

# --------------------------------------------------------------

install: bbfpromix
	echo "Install"
	install -d $(DESTDIR)$(PREFIX)/bin

	install -m 644 $(BUILDDIR)/bbfpromix  $(DESTDIR)$(PREFIX)/bin/bbfpromix

# --------------------------------------------------------------
uninstall:
	echo "Uninstall"
	rm -fr $(DESTDIR)$(PREFIX)/bin/bbfpromix
	
# --------------------------------------------------------------

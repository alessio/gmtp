# gMTP Sync tool

PKG_NAME = gMTP
PREFIX = /usr/local
VER = 0.6
# Note: If you update above, please update the config.h and pkginfo file as well.

PKG = gMTP
ARCH = i386
PKGFILE = $(PKG)-$(VER)-$(ARCH).pkg
TARFILE = $(PKG_NAME)-$(VER)-$(ARCH).tar

UNAME = $(shell uname)

# See what OS we are, and set things for Solaris, otherwise use a default
# that should work.
ifeq ($(UNAME), SunOS)
CC = cc
INSTALL = /usr/ucb/install -c
LD_FLAGS += -L/usr/sfw/lib -R/usr/sfw/lib
else
CC = gcc
INSTALL = install -c
endif

GCONFTOOL = gconftool-2
TAR = tar

CFLAGS += -c -O 
LD_FLAGS += 
LIBS +=

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gconf-2.0 libmtp id3tag`
GTK_LDFLAGS = `pkg-config --libs gtk+-2.0 gconf-2.0 libmtp id3tag`

objects = main.o mtp.o interface.o callbacks.o prefs.o dnd.o
headers = main.h mtp.h interface.h callbacks.h prefs.h dnd.h config.h

gMTP:	$(objects)
	@echo "B: gMTP"
	@$(CC) -o gMTP $(LD_FLAGS) $(LIBS) $(GTK_LDFLAGS) $(objects)


.c.o: $(headers)
	@echo "C: "$<
	@$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $@  $<


install: gMTP
	$(INSTALL) -d $(PREFIX)
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -d $(PREFIX)/share
	$(INSTALL) -d $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -d $(PREFIX)/share/applications
	$(INSTALL) -d $(PREFIX)/share/pixmaps
	$(INSTALL) -m 755 gMTP $(PREFIX)/bin
	$(INSTALL) -m 644 icon.png $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 icon-16.png $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 stock-about-16.png $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 README $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 COPYING $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 ChangeLog $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 AUTHORS $(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 gMTP.desktop $(PREFIX)/share/applications
	$(INSTALL) -m 644 icon.png $(PREFIX)/share/pixmaps
	mv $(PREFIX)/share/pixmaps/icon.png $(PREFIX)/share/pixmaps/gMTPicon.png
	GCONF_CONFIG_SOURCE=`$(GCONFTOOL) --get-default-source` $(GCONFTOOL) --makefile-install-rule gMTP.schema

clean:
	@echo "CLEAN: core gMTP *.o *~ "$(objects)
	@-rm -f $(objects) core gMTP *.o *~ $(PKGFILE).gz

dist:
	-rm -f $(objects) core gMTP *.o *~
	cd .. && $(TAR) -cf $(TARFILE) gMTP && gzip $(TARFILE) && cd gMTP

pkg: gMTP
	pkgmk -o -d /tmp -a $(ARCH)
	touch $(PKGFILE)
	pkgtrans -s /tmp $(PKGFILE) $(PKG)
	rm -r /tmp/$(PKG)
	gzip $(PKGFILE)

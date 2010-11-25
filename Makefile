# gMTP Sync tool

PKG_NAME = gmtp
PREFIX = /usr/local
VER = 0.7
# Note: If you update above, please update the config.h and pkginfo file as well.

PKG = gmtp
ARCH = i386
PKGFILE = $(PKG)-$(VER)-$(ARCH).pkg
TARFILE = $(PKG_NAME)-$(VER)-$(ARCH).tar

UNAME = $(shell uname)

# See what OS we are, and set things for Solaris, otherwise use a default
# that should work.
ifeq ($(UNAME), SunOS)
CC = cc
INSTALL = /usr/ucb/install -c
LDFLAGS += -L/usr/sfw/lib -R/usr/sfw/lib
else
CC = gcc
INSTALL = install -c
endif

GCONFTOOL = gconftool-2
TAR = tar

CFLAGS += -c -g 
LDFLAGS += 
LIBS +=

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gconf-2.0 libmtp id3tag flac vorbisfile`
GTK_LDFLAGS = `pkg-config --libs gtk+-2.0 gconf-2.0 libmtp id3tag flac vorbisfile`

objects = main.o mtp.o interface.o callbacks.o prefs.o dnd.o metatag_info.o
headers = main.h mtp.h interface.h callbacks.h prefs.h dnd.h metatag_info.h config.h

gmtp:	$(objects)
	$(CC) -o gmtp $(LDFLAGS) $(GTK_LDFLAGS)  $(LIBS)  $(objects)


.c.o: $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  $<


install: gmtp
	$(INSTALL) -d $(DESTDIR)$(PREFIX)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/pixmaps
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/doc
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/gconf
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/gconf/schemas
	$(INSTALL) -m 755 gmtp $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 644 icon.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 icon-16.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 stock-about-16.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 README $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 COPYING $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 ChangeLog $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 AUTHORS $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 gMTP.desktop $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 644 icon.png $(DESTDIR)$(PREFIX)/share/pixmaps
	$(INSTALL) -m 644 gMTP.schema $(DESTDIR)$(PREFIX)/share/gconf/schemas
	mv $(DESTDIR)$(PREFIX)/share/pixmaps/icon.png $(DESTDIR)$(PREFIX)/share/pixmaps/gMTPicon.png
	GCONF_CONFIG_SOURCE=`$(GCONFTOOL) --get-default-source` $(GCONFTOOL) --makefile-install-rule gMTP.schema

clean:
	rm -f $(objects) core gmtp *.o *~ $(PKGFILE).gz

dist:
	rm -f $(objects) core gmtp *.o *~
	cd .. && $(TAR) -cf $(TARFILE) gMTP && gzip $(TARFILE) && cd gMTP

pkg: gmtp
	pkgmk -o -d /tmp -a $(ARCH)
	touch $(PKGFILE)
	pkgtrans -s /tmp $(PKGFILE) $(PKG)
	rm -r /tmp/$(PKG)
	gzip $(PKGFILE)

# gMTP Sync tool

PKG_NAME = gmtp
PREFIX ?= /usr/local
VER = 1.0.1
# Note: If you update above, please update the config.h and pkginfo file as well.

PKG = gmtp
ARCH = i386
PKGFILE = $(PKG)-$(VER)-$(ARCH).pkg
TARFILE = $(PKG_NAME)-$(VER)-$(ARCH).tar

UNAME = $(shell uname)

# See what OS we are, and set things for Solaris, otherwise use a default
# that should work.
ifeq ($(UNAME), SunOS)
CC ?= cc
INSTALL = /usr/ucb/install -c
MSGFMT = /usr/bin/msgfmt --strict
LDFLAGS += -L/usr/sfw/lib -R/usr/sfw/lib
else
CC ?= gcc
INSTALL = install -c
MSGFMT = msgfmt
endif

GCONFTOOL = gconftool-2
TAR = tar

CFLAGS += -c -g -O
LDFLAGS += 
LIBS +=

.SUFFIXES: .c .o .po .mo

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gconf-2.0 libmtp id3tag flac vorbisfile`
GTK_LDFLAGS = `pkg-config --libs gtk+-2.0 gconf-2.0 libmtp id3tag flac vorbisfile`

ifeq ($(MAKECMDGOALS),gtk3)
GTK_CFLAGS = `pkg-config --cflags gtk+-3.0 gio-2.0 libmtp id3tag flac vorbisfile`
GTK_LDFLAGS = `pkg-config --libs gtk+-3.0 gio-2.0 libmtp id3tag flac vorbisfile`
CFLAGS += -DGMTP_USE_GTK3
endif

objects = src/main.o src/mtp.o src/interface.o src/callbacks.o src/prefs.o src/dnd.o src/metatag_info.o
headers = src/main.h src/mtp.h src/interface.h src/callbacks.h src/prefs.h src/dnd.h src/metatag_info.h src/config.h

catalogues = po/es.mo po/it.mo po/fr.mo po/da.mo po/de.mo
POFILES = po/es.po po/it.po po/fr.po po/da.po po/de.po


all:	gmtp $(catalogues)

# GTK3 build

gtk3:	gmtp $(catalogues)

# GTK2 build

gtk2:	gmtp $(catalogues)

# Main executable

gmtp:	$(objects)
	$(CC) -o gmtp $(LDFLAGS) $(objects) $(GTK_LDFLAGS)  $(LIBS)

# Object Files

src/main.o: src/main.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/main.c

src/mtp.o: src/mtp.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/mtp.c

src/interface.o: src/interface.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/interface.c

src/callbacks.o: src/callbacks.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/callbacks.c

src/prefs.o: src/prefs.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/prefs.c

src/dnd.o: src/dnd.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/dnd.c

src/metatag_info.o: src/metatag_info.c $(headers)
	$(CC) $(GTK_CFLAGS) $(CFLAGS)  -o $@  src/metatag_info.c

# Language Files

po/es.mo: po/es.po
	$(MSGFMT) -o po/es.mo po/es.po

po/it.mo: po/it.po
	$(MSGFMT) -o po/it.mo po/it.po

po/de.mo: po/de.po
	$(MSGFMT) -o po/de.mo po/de.po

po/da.mo: po/da.po
	$(MSGFMT) -o po/da.mo po/da.po

po/fr.mo: po/fr.po
	$(MSGFMT) -o po/fr.mo po/fr.po

# Installation

install: gmtp $(catalogues)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/pixmaps
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/gconf
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/gconf/schemas
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/es
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/it
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/fr
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/da
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/de
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/es/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/it/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/da/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/de/LC_MESSAGES
	$(INSTALL) -m 755 gmtp $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 644 images/icon.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/logo.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/icon-16.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/stock-about-16.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/audio-x-mp3-playlist.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/audio-x-mpeg.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/folder.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/image-x-generic.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/media-cdrom-audio.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/text-plain.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/video-x-generic.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/empty.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/view-refresh.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 misc/gMTP.desktop $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 644 images/icon.png $(DESTDIR)$(PREFIX)/share/pixmaps
	$(INSTALL) -m 644 misc/gMTP.schemas $(DESTDIR)$(PREFIX)/share/gconf/schemas
	mv $(DESTDIR)$(PREFIX)/share/pixmaps/icon.png $(DESTDIR)$(PREFIX)/share/pixmaps/gMTPicon.png
	cp po/es.mo $(DESTDIR)$(PREFIX)/share/locale/es/LC_MESSAGES/gmtp.mo
	cp po/fr.mo $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES/gmtp.mo
	cp po/it.mo $(DESTDIR)$(PREFIX)/share/locale/it/LC_MESSAGES/gmtp.mo
	cp po/da.mo $(DESTDIR)$(PREFIX)/share/locale/da/LC_MESSAGES/gmtp.mo
	cp po/de.mo $(DESTDIR)$(PREFIX)/share/locale/de/LC_MESSAGES/gmtp.mo
	mv $(DESTDIR)$(PREFIX)/share/gconf/schemas/gMTP.schemas $(DESTDIR)$(PREFIX)/share/gconf/schemas/gmtp.schemas

register-gconf-schemas: install
	GCONF_CONFIG_SOURCE=`$(GCONFTOOL) --get-default-source` $(GCONFTOOL) --makefile-install-rule $(DESTDIR)$(PREFIX)/share/gconf/schemas/gmtp.schemas

install-gtk3: gmtp $(catalogues)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/pixmaps
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/glib-2.0
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/es
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/it
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/fr
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/da
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/de
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/es/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/it/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/da/LC_MESSAGES
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/locale/de/LC_MESSAGES
	$(INSTALL) -m 755 gmtp $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 644 images/icon.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/icon-16.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/logo.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/stock-about-16.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 misc/gMTP.desktop $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 644 images/icon.png $(DESTDIR)$(PREFIX)/share/pixmaps
	$(INSTALL) -m 644 images/audio-x-mp3-playlist.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/audio-x-mpeg.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/folder.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/image-x-generic.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/media-cdrom-audio.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/text-plain.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/video-x-generic.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/empty.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	$(INSTALL) -m 644 images/view-refresh.png $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)
	mv $(DESTDIR)$(PREFIX)/share/pixmaps/icon.png $(DESTDIR)$(PREFIX)/share/pixmaps/gMTPicon.png
	cp po/es.mo $(DESTDIR)$(PREFIX)/share/locale/es/LC_MESSAGES/gmtp.mo
	cp po/fr.mo $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES/gmtp.mo
	cp po/it.mo $(DESTDIR)$(PREFIX)/share/locale/it/LC_MESSAGES/gmtp.mo
	cp po/da.mo $(DESTDIR)$(PREFIX)/share/locale/da/LC_MESSAGES/gmtp.mo
	cp po/de.mo $(DESTDIR)$(PREFIX)/share/locale/de/LC_MESSAGES/gmtp.mo

register-gsettings-schemas: install-gtk3
	$(INSTALL) -m 644 misc/org.gnome.gmtp.gschema.xml $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas
	glib-compile-schemas $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas

install-doc:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/doc
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 README $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 COPYING $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 ChangeLog $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)
	$(INSTALL) -m 644 AUTHORS $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/gmtp
	rm -f  $(DESTDIR)$(PREFIX)/share/$(PKG_NAME)/*
	rm -f  $(DESTDIR)$(PREFIX)/share/applications/gMTP.desktop
	rm -f  $(DESTDIR)$(PREFIX)/share/pixmaps/gMTPicon.png
	rm -f  $(DESTDIR)$(PREFIX)/share/gconf/schemas/gmtp.schemas
	rm -f  $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas/org.gnome.gMTP.gschema.xml
	rm -f  $(DESTDIR)$(PREFIX)/share/locale/es/LC_MESSAGES/gmtp.mo
	rm -f  $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES/gmtp.mo
	rm -f  $(DESTDIR)$(PREFIX)/share/locale/it/LC_MESSAGES/gmtp.mo
	rm -f  $(DESTDIR)$(PREFIX)/share/locale/da/LC_MESSAGES/gmtp.mo
	rm -f  $(DESTDIR)$(PREFIX)/share/locale/de/LC_MESSAGES/gmtp.mo

uninstall-doc:
	rm $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)/README
	rm $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)/COPYING
	rm $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)/ChangeLog
	rm $(DESTDIR)$(PREFIX)/share/doc/$(PKG_NAME)/AUTHORS

clean:
	rm -f $(objects) core gmtp src/*.o src/*~ $(PKGFILE).gz po/*.mo po/*~

dist:
	rm -f $(objects) core gmtp src/*.o src/*~ po/*.mo po/*~
	cd .. && $(TAR) -cf $(TARFILE) gMTP && gzip $(TARFILE) && cd gMTP

pkg: gmtp $(catalogues)
	pkgmk -o -d /tmp -a $(ARCH)
	touch $(PKGFILE)
	pkgtrans -s /tmp $(PKGFILE) $(PKG)
	rm -r /tmp/$(PKG)
	gzip $(PKGFILE)

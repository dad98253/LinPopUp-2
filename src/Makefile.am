## Process this file with automake to produce Makefile.in

PROGNAME_LNK = LinPopUp
SHARE_DIR = $(DESTDIR)$(datadir)/linpopup
DATA_FILE = $(DESTDIR)$(localstatedir)/linpopup/messages.dat

INCLUDES = -I$(srcdir)/..

bin_PROGRAMS = linpopup

linpopup_SOURCES = \
	version.c \
	dialog.c \
	main.c \
	text.c \
	menu.c \
	toolbar.c \
	io.c \
	send.c \
	recept.c \
	custom.c \
	string.c \
	GTKmisc.c \
	docs.c \
	execute.c \
	$(srcdir)/../lib/netscape_remote.c \
	$(srcdir)/../lib/eggtrayicon.c \
	tray.c
linpopup_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -L/usr/X11R6/lib -lXmu
linpopup_CFLAGS = \
	-DDOC_DIR=\""$(DESTDIR)$(docdir)"\" \
	-DDATA_FILE=\""$(DATA_FILE)"\" \
	-DSHARE_DIR=\""$(SHARE_DIR)"\" \
	-DPACKAGE_DATA_DIR=\""$(DESTDIR)$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(DESTDIR)$(datadir)/locale"\" \
	@PACKAGE_CFLAGS@

noinst_HEADERS = protos.h version.h $(srcdir)/../lib/eggtrayicon.h

install-data-local:
	@$(NORMAL_INSTALL)
	ln -fs $(PACKAGE) $(DESTDIR)$(bindir)/$(PROGNAME_LNK)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -f $(DESTDIR)$(bindir)/$(PROGNAME_LNK)

## Process this file with automake to produce Makefile.in

SUBDIRS = \
	extra   \
	man     \
	pixmaps \
	po      \
	src

PROGNAME_LNK = LinPopUp
SHARE_DIR = $(DESTDIR)$(datadir)/linpopup
DATA_FILE = $(DESTDIR)$(localstatedir)/linpopup/messages.dat

EXTRA_DIST = mkinstalldirs

AUTOMAKE_OPTIONS = dist-bzip2

install-data-local:
	@$(NORMAL_INSTALL)
	if [ ! -f $(DATA_FILE) ]; then \
	   $(mkinstalldirs) $(DATA_FILE); \
	   rm -fR $(DATA_FILE); \
	   touch $(DATA_FILE); \
	   chgrp nobody $(DATA_FILE); \
	   chmod 0666 $(DATA_FILE); \
	fi

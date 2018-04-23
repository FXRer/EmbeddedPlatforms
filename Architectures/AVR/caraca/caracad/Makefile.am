## $Id: Makefile.am,v 1.4 2000/08/30 20:29:08 skr Exp $

## Process this file with automake to produce Makefile.in 
## Makefile.am, (c)1999 Tom Wheeley <tw104@york.ac.uk>

## we need version 1.4 of automake for DIST_SUBDIRS to work.
AUTOMAKE_OPTIONS = 1.4 check-news

EXTRA_DIST = ANNOUNCE TODO


sbin_PROGRAMS = caracad
caracad_SOURCES = caracad.c globals.c can_proto.c sock.c sock_proto.c log.c
caracad_LDADD = @daemon@
##caracad_DEPENDENCIES = @hw_module@
EXTRA_caracad_SOURCES = 

SUBDIRS = . client

# rules for building the perl module
perl: perl/Makefile
	cd perl && $(MAKE)

perl/Makefile: perl/Makefile.PL
	cd perl && $(PERL) Makefile.PL

# add the following to the all target
all-local:	@COMP_PERL@


install-exec-local: mkdev
uninstall-local: rmdev

install-exec-local: perl/Makefile
	cd perl && $(MAKE) install


install-data-local:
	test "$(lircd_conf)" = ""  || test -e $(sysconfdir)/lircd.conf  || ($(mkinstalldirs) $(sysconfdir) && $(INSTALL_DATA) $(top_srcdir)/remotes/$(lircd_conf) $(sysconfdir)/lircd.conf)

mkdev:
#	test -e @devdir@/lircd || @mknod@ @devdir@/lircd p
#create /dev/can if missing

rmdev:
	$(RM) @devdir@/can

CLEANFILES = *~
## From automake documentation:
## Note that EXTRA_DIST can only handle files in the current
## directory; files in other directories will cause make dist runtime
## failures.

dist-hook:
	for f in contrib test; do \
		cp -pr $(srcdir)/$$f $(distdir)/$$f; done
	rm -r `find $(distdir) -name "CVS"`
	-rcs2log >$(distdir)/ChangeLog

DISTCLEANFILES = configure.sh .setup.config

CLEANFILES = *~ contrib/*~

clean-local:
	cd perl && test -f Makefile && $(MAKE) clean || true

##END##

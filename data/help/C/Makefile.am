# We should take advantage of ${GLIB_COMPILE_RESOURCES} conditionally defined
# on configure.ac. However, if we have no such a tool we will not to be able
# of compiling rules.txt and the screenshots in a simple binary bundle file

rules.gresource: rules.xml
	glib-compile-resources $<

clean-local:
	rm -f rules.gresource

resdir = $(datadir)/chin-chon-lin/data/help/C
res_DATA = rules.gresource

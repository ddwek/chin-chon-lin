AC_INIT([chin-chon-lin], [0.3.0], [dwekdaniel@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_CHECK_PROG(GLIB_COMPILE_RESOURCES, 'glib-compile-resources', "yes", "no")
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.21)
AC_SUBST(CFLAGS, "-Wall")
AC_SUBST(CPPFLAGS, "")
AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/help/Makefile
data/help/C/Makefile
data/help/es_AR/Makefile
data/icons/Makefile
data/tiles/Makefile
data/ui/Makefile
po/Makefile.in
])
GLIB_GSETTINGS
AC_OUTPUT

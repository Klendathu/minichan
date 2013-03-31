AC_INIT([Minichan], [0.07], [klendathu@hushmail.com])
AM_INIT_AUTOMAKE([-Werror])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
PKG_CHECK_MODULES([DEPS], [gtkmm-3.0 >= 1.0 webkitgtk-3.0 >= 1.5 libcurl >= 3.0 gtkspell3-3.0 >= 3.0 libnotify >= 0.5])
AC_CONFIG_FILES([Makefile src/Makefile data/Makefile data/minichan.desktop])

#case "${host}" in
#        i[[3456789]]86-mingw32*) WIN32="yes" ;;
#        *cygwin*) WIN32="yes" ;;
#        *) WIN32="no" ;;
#esac
#AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")

AC_OUTPUT


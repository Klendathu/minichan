bin_PROGRAMS = minichan
minichan_SOURCES = main.cpp ModelColumns.h ModelThread.h ModelPost.h settings_dialog.h formatting.h json_funcs.h
#minichan_CPPFLAGS = -std=c++11 -pthread -I/usr/include/gtkmm-3.0 -I/usr/lib/x86_64-linux-gnu/gtkmm-3.0/include -I/usr/include/giomm-2.4 -I/usr/lib/x86_64-linux-gnu/giomm-2.4/include -I/usr/include/gtk-3.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/gtk-3.0/unix-print -I/usr/include/gdkmm-3.0 -I/usr/lib/x86_64-linux-gnu/gdkmm-3.0/include -I/usr/include/atk-1.0 -I/usr/include/glibmm-2.4 -I/usr/lib/x86_64-linux-gnu/glibmm-2.4/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/sigc++-2.0 -I/usr/lib/x86_64-linux-gnu/sigc++-2.0/include -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/gio-unix-2.0/ -I/usr/include/libsoup-2.4 -I/usr/include/libxml2 -I/usr/include/atkmm-1.6 -I/usr/include/pangomm-1.4 -I/usr/lib/pangomm-1.4/include -I/usr/include/cairomm-1.0 -I/usr/lib/cairomm-1.0/include -I/usr/include/jsoncpp -I/usr/include/webkitgtk-3.0  -lgtkmm-3.0 -latkmm-1.6 -lgdkmm-3.0 -lpangomm-1.4 -lcairomm-1.0 -lgiomm-2.4 -lglibmm-2.4 -lsigc-2.0 -ljsoncpp -lwebkitgtk-3.0 -lgtk-3 -lsoup-2.4 -ljavascriptcoregtk-3.0 -lgdk-3 -latk-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lpango-1.0 -lcairo -lgio-2.0 -lgobject-2.0 -lglib-2.0 -L/usr/lib/x86_64-linux-gnu -lcurl -L/usr/local/boost/boost_1_39_0/stage/lib -lboost_regex
minichan_CPPFLAGS = $(DEPS_CFLAGS) -DDATADIR=\"$(pkgdatadir)\" -I/usr/include/jsoncpp
minichan_LDFLAGS = -I/usr/include/boost -lboost_regex -I/usr/include/jsoncpp -ljsoncpp
minichan_LDADD = $(DEPS_LIBS)
AM_CPPFLAGS = -DDATADIR=\"$(pkgdatadir)\"

#if WIN32
#AM_CFLAGS = \
#        -mms-bitfields
#
#AM_LDFLAGS = \
#        -Wl,-subsystem,windows
#endif

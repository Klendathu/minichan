#Minichan
A small imageboard client

##Description
Minichan aims to be fully featured while not becoming overly bloated. Currently, it can only browse 4chan, but I hope to expand it to others, importantly Shrekchan, soon.

Minichan is written in C++, with the interface written using GTK+. It utilizes the 4chan JSON API to retrieve data, but it is fairly flexible.

##Installation
Minichan is cross-platform; it is compatible with any OS that supports GTK+ (GNU/Linux, *BSD[OpenBSD, FreeBDS, Mac OS X, etc.], Windows). To install, follow the standard GNU compilation process, namely:

    ./configure && make -j{num. cores +1} && sudo make install

The following libraries are required:

* gtkmm-3.0
* webkitgtk-3.0
* libcurl >= 3.0
* gtkspell3-3.0
* libnotify
* jsoncpp
* libboost-regex

Note that the configuration script will not check for jsoncpp or libboost-regex due to these libraries not supplying a .pc file for pkgconfig (At least, not on Arch GNU/Linux, anyway). So, if you receive a stream of errors when you compile the program complaining about missing libraries, check that these are installed. If that still doesn't solve it, I may have forgotten a dependency, so feel free to file a bug report.

##Licensing
This application is licensed under the GPLv3. Support Stallman.
[He fights for the users](http://i.imgur.com/ydmXM4f.jpg).

     ______________ 
    < MUH FREEDUMS >
     -------------- 
                \        @@@@@@ @
                 \     @@@@     @@
                  \   @@@@ =   =  @@ 
                   \ @@@ @ _   _   @@ 
                     @@@ @(0)|(0)  @@ 
                    @@@@   ~ | ~   @@
                    @@@ @  (o1o)    @@
                   @@@    #######    @
                   @@@   ##{+++}##   @@
                  @@@@@ ## ##### ## @@@@
                  @@@@@#############@@@@
                 @@@@@@@###########@@@@@@
                @@@@@@@#############@@@@@
                @@@@@@@### ## ### ###@@@@
                 @ @  @              @  @
                   @                    @

yukkipaste
==========

Yukkuri pastu shite itte ne!

A simple pasting utility for pocoo pastebin api.

To build it:

    make

To install it:
   
    sudo make instal # optional: DESTDIR=/usr/local

To use it:

    yukkipaste --help

Build for x86_64 and i686 GNU/Linux are available at

    http://yukkuri.eientei.org/yukkipaste/crossout/

Ebuild is located in directory:

    pkgs/gentoo/app-text/yukkipaste

Binary deb is avialable, thanks to madduck@RusNet

    http://yukkuri.eientei.org/yukkipaste/pkgs/deb/

Sample --help output:

    yukkipaste [OPTIONS...]
    
        --help,           -h            Prints help message
        --modules-dir,    -d DIR        Appends module path. Stackable.
        --list-modules                  Lists available modules
        --verbose,        -v            Increases verbosity level
        --author,         -a STRING     Your name
        --uri,            -u URI        Pastebin URI
        --language,       -l LANG       Paste language
        --list-languages                Lists available languages for selected module
        --remote-name,    -n NAME       Remote file name
        --mime-type          MIME       Paste mime type
        --module,         -m MODULE     Selects module to use
        --parent-id          ID         Parent paste id
        --private,        -p            Marks paste as private
        --run,            -r            Marks paste as runnable


Module compilation guide:

Assuming you have installed yukkipaste at DESTDIR=/usr,

    gcc `PKG_CONFIG_PATH=/usr/lib/pkgconfig/ pkg-config yukkipaste --cflags --libs` -shared -fPIC module.c
    


dependencies:
             * asio (if not present will be cloned locally)
             * ncurses

on mac: brew install ncurses asio(optionally)

BUILD AND INSTALL:
    git clone https://github.com/matiTechno/curchat.git
    cd curchat
    mkdir build
    cd build
    cmake .. (use ccmake to see options)
    make
    sudo make install

TEST RUN:
    curserver &
    curchat

curchat:
        * ^C - exit

command-line arguments:

    curserver <port>

    curchat <name>
    curchat <name> <host> <port>

TODO:
     * when running outside the terminal emulator (for example launched by d-menu),
       consumes 100% cpu
     * better command-line argument parsing
     * recent msgs colouring based on the session name and ip

HOT:
    * limit the number of sessions
    * validate session name
    * construct strings in ChatRoom not in ChatSession

dependencies:
             * asio (if not present will be cloned locally)
             * ncurses

on mac: brew install ncurses asio(optionally)

^C - exit
If the welcome message appeared you are connected to the server.

command line options:

    curserver <port>

    curchat <name>
    curchat <name> <host> <port>

TODO:
* reconnecting
* connection status
* when running outside the terminal emulator (for example launched by d-menu) consumes 100% cpu, why?
  fix or prevent it if possible

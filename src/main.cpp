#include <cstdlib>
#include <signal.h>


#include "ncurses_display.h"
#include "system.h"

  // For some reason ctl-c is being ignored (probably ncurses)
  void signal_handler(int signal_number) {
        std::exit(1); 
  }

int main() {
  signal(SIGINT, signal_handler);
  System system;
  NCursesDisplay::Display(system);
}
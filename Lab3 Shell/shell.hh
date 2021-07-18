#ifndef shell_hh
#define shell_hh

#include "command.hh"
//#include "pipe.h"
struct Shell {

  static void prompt();

  static Command _currentCommand;
};

#endif

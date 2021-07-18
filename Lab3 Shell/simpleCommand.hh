#ifndef simplecommand_hh
#define simplecommand_hh

#include <string>
#include <vector>

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void print();
  
  /* My Helper Functions */

  //Converts all the arguments from string pointers to char pointers and puts them all in an array
  char ** args_to_c();
  std::string * at(size_t index);
};

#endif

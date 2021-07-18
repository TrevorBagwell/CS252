#include <cstdio>
#include <cstdlib>








#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>


#ifndef MAXPATHLEN
#define MAXPATHLEN 2048
#endif


#include <iostream>

#include "simpleCommand.hh"


#define debugger true


SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  //std::cout << "Argument Count: " << _arguments.size() << std::endl;
  
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  
  //std::cout << "Here" << std::endl;
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}




//Because people love to so stupidly put thing in pointers, I can't use operators so now I have to 
//Make this functions so I can use an arrow to get an index
std::string * SimpleCommand::at(size_t index)
{
  return _arguments[index];
}



//Returns a char ** that points to an array of arguments, but in c_strings
char ** SimpleCommand::args_to_c()
{
  size_t args_size = _arguments.size();
  char ** c_arguments = new char*[args_size+1];
  uint index = 0;
  for(; index < args_size; index++)
  {
    c_arguments[index] = const_cast<char*> (_arguments[index]->c_str());
  }
  c_arguments[index] = NULL;
  return c_arguments;
}







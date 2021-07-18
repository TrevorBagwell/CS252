#ifndef command_hh
#define command_hh
#include "simpleCommand.hh"
#include "defines.h"
//#include "pipe.h"

// Command Data Structure

//enum { IN = 0, OUT = 1, ERR = 2 , T_IN = 3, T_OUT = 4, T_ERR = 5 };
//        in = 1, out = 2, err = 4, t_in = 8, t_out = 16, t_err = 32 };

/*int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}/**/

struct pipes
{
  public:
    //Holds the current active pipes
    int fds[3];
    //Holds a temporary set of pipes
    int temp[3];
    //Holds the amount of times print has been called this iteration
    int print_call;
  //TODO: Initilize this better  

    //Basic constructor and desctructor
    pipes() { reset(); print_call = 0; }
    ~pipes() { reset(); }
    //Resets all the pipes to -1
    void reset(); 
    /*{ 
      if(fd_is_valid(fds[IN])) { close(fds[IN]); }
      if(fd_is_valid(fds[OUT])) { close(fds[OUT]); }
      if(fd_is_valid(fds[ERR])) { close(fds[ERR]); }
      if(fd_is_valid(temp[IN])) { close(temp[IN]); }
      if(fd_is_valid(temp[OUT])) { close(temp[OUT]); }
      if(fd_is_valid(temp[ERR])) { close(temp[ERR]); }
      fds[IN] = 0; 
      fds[OUT] = 0; 
      fds[ERR] = 0; 
      temp[IN] = 0; 
      temp[OUT] = 0; 
      temp[ERR] = 0; 
    }*/
    void init(std::string * _inFile);
    int& operator[] (const size_t index);
    pipes& operator= (const pipes& p);
    int open_file(int type, bool _append, std::string * _file);
    void pipe_it_up(int i);
    void dup_2(int i);
    void print();
};






struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;

  //Dictates wether you should append to the file or truncate it
  bool _append;

  //Environment variable holders that assist in the project
  int status, last_command, last_process, returner;
  
  //TODO: Figure out what this does
  std::string * last_argument;
  
  //Meant to store the double pointer version of the simpleCommands string array
  //char ** c_arguments;

  //Simple constructor
  Command();
  //Inserts a simpleCommand into the simpleCommands list
  void insertSimpleCommand( SimpleCommand * simpleCommand );
  
  //Generic storage functions
  void clear();
  void print();
  void execute();

  static SimpleCommand *_currentSimpleCommand;
  
  pipes  p;
  
  //Prints the value of the pipes object p
  void print_p();
  
  //Prints all the files
  void print_files(int flags);
  
  bool printf_file_by_line(const char * file);

  //Gets the requested string* to a file with the respected enum index
  const char * get_file(size_t index);

  //Gets the simpleCommand at the specified index
  SimpleCommand* cmd_at(size_t index);

  //Gets the argument at index n of the specified command at index m
  std::string * get_arg(size_t m, size_t n);

  void shell_commands();

};

#endif

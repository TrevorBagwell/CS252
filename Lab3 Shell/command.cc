/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "command.hh"
#include "shell.hh"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>

#define FLAG_APPEND (O_WRONLY | O_CREAT | O_APPEND)
#define FLAG_TRUNC (O_WRONLY | O_CREAT | O_TRUNC)
#define FLAGS_INPUT (O_RDONLY)
#define EXTRA 0666

#define PRINTENV "printenv"
#define SETENV "setenv"
#define UNSETENV "unsetenv"
#define CD "cd"
#define HOME "HOME"
#define CD_ERR_MSG "cd: can't cd to "

#define UNALTERED_RETURNER -1

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();
    
    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _append = false;
    _background = false;

    //p = new pipes();
}

int gets_here_count;

void gets_here()
{
  printf("%i: Gets Here\n", gets_here_count++);
}


bool Command::printf_file_by_line(const char * file)
{
  printf("File Name: %s\n", file);
  std::string line;
  std::ifstream i_file (file);
  if(!i_file.is_open()) return false;
  while(getline(i_file,line)) { printf("%s> %s\n",file,line.c_str()); line.clear(); }
  i_file.close();
  return true;
}

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}/**/





void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}


void pipes::reset()
    { 
      //if(fd_is_valid(fds[IN])) { close(fds[IN]); }
      //if(fd_is_valid(fds[OUT])) { close(fds[OUT]); }
      //if(fd_is_valid(fds[ERR])) { close(fds[ERR]); }
      //if(fd_is_valid(temp[IN])) { close(temp[IN]); }
      //if(fd_is_valid(temp[OUT])) { close(temp[OUT]); }
      //if(fd_is_valid(temp[ERR])) { close(temp[ERR]); }
      fds[IN] = 0; 
      fds[OUT] = 0; 
      fds[ERR] = 0; 
      temp[IN] = 0; 
      temp[OUT] = 0; 
      temp[ERR] = 0; 
    }




void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) { delete simpleCommand; }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) delete _outFile;
    _outFile = NULL;

    if ( _inFile ) delete _inFile;
    _inFile = NULL;

    if ( _errFile ) delete _errFile;
    _errFile = NULL;
    
    
    _append = false;
    _background = false;
    
    p.reset();
}

void Command::print_p()
{
    printf("Pipe fds IN: %i\n",p.fds[IN]);
    printf("Pipe fds OUT: %i\n",p.fds[OUT]);
    printf("Pipe fds ERR: %i\n",p.fds[ERR]);
    printf("Pipe temp IN: %i\n",p.temp[IN]);
    printf("Pipe temp OUT: %i\n",p.temp[OUT]);
    printf("Pipe temp ERR: %i\n",p.temp[ERR]);
    printf("\n");
}

void Command::print_files(int flags)
{
    if(path_debugger) printf("Print files called with arguments (flags: %i)\r\n", flags);
    if ( flags & in ) {
      if(path_debugger) printf("Prints file in\r\n");
      if( _inFile )
      {
        printf("In ");
        printf_file_by_line(_inFile->c_str());
      }
    }
    if ( flags & out ) {
      if(path_debugger) printf("Prints file out\r\n");
      if( _outFile )
      {
        printf("Out ");
        printf_file_by_line(_outFile->c_str());
      }
    }
    if ( flags & err)
    {
      if(path_debugger) printf("Prints file err\r\n");
      if( _errFile )
      {
        printf("Err ");
        printf_file_by_line(_errFile->c_str());
      }/**/
    }
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    
    //if(debugger) gets_here();
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        //if(debugger) gets_here();
        simpleCommand->print();
        //if(debugger) gets_here();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
    
    //if(debugger) gets_here();
}


/***************** My implementations *******************/



/**
 * @brief Helper function that retrieves the simple command at a given index
      else { fds[OUT] = dup(tmp[OUT]); }
 *
 * @param size_t m: the index of the requested simpleCommand
 *
 * @return SimpleCommand * pointing to the simple command at the index of m in the _simpleCommands
 * vector, will return NULL if there is an out of bounds error
 */
SimpleCommand* Command::cmd_at(size_t m)
{
  //Check out of bounds for m
  if(m >= _simpleCommands.size()) return NULL;
  //Return the desired simpleCommand
  else return _simpleCommands[m];
}


/**
 * @brief Helper function that retrieves the simple command at a given index
 *
 * @param size_t m: the index of the requested simpleCommand
 *
 * @return SimpleCommand * pointing to the simple command at the index of m in the _simpleCommands
 * vecotr, will return NULL if there is an out of bounds error on either
 */
std::string * Command::get_arg(size_t m, size_t n)
{
  //Check for out of bounds in _simpleCommands
  if(m >= _simpleCommands.size()) return NULL;
  //Check for out of bound in the argument vector
  else if(n >= _simpleCommands[m]->_arguments.size()) return NULL;
  //Return the desired c_string
  else return _simpleCommands[m]->_arguments[n];
}



/**
 * @brief Operates as a leyway in order to set an int as a string based env variable
 *
 * @param:
 *      -const char * var_name: the environment variables name
 *      -int i: the integer to be set
 *
 * @return void
 */

inline void setenv_int(const char * var_name, int i)
{
  setenv(var_name,std::to_string(i).c_str(),1);
}



/**
 * @brief Helper function that executes all the shell commands if possible
 *
 * @param n/a
 *
 * @return void
 */
void Command::shell_commands()
{
  
  if(path_debugger) printf("Executes Shell commands\r\n");
  
  returner = UNALTERED_RETURNER;
  std::string * first_argument = get_arg(0,0);
  //If the set environment variable 
  if(!first_argument->compare(SETENV))
  {
    //Call set environment varaibles with the first and second arguments as the variable name and the
    //variable value to set with a flag of 1
    returner = setenv(get_arg(0,1)->c_str(), get_arg(0,2)->c_str(),1);
    if(returner) perror(SETENV);
  }
  else if (!first_argument->compare(UNSETENV))
  {
    //Call the unsetenv command with the first argument as the variable to unset
    returner = unsetenv(get_arg(0,1)->c_str());
    if(returner) perror(UNSETENV);
  }
  else if (!first_argument->compare(CD))
  {
    //If there is only one argument, the working path directory is the home
    if(cmd_at(0)->_arguments.size() == 1)
    {
      returner = chdir(getenv(HOME));
    }
    //Else the first argument is where the working path directory should be
    else
    {
      returner = chdir(get_arg(0,1)->c_str());
    }
    //Chdir returns -1 when there is no directory found with the given name
    if(returner == UNALTERED_RETURNER)
    {
      //Initialize the error message
      std::string em = CD_ERR_MSG;
      em.append(get_arg(0,1)->c_str());
      fprintf(stderr,"%s\n",em.c_str());
      returner = 1;
    }
  }
  else if(!first_argument->compare(SOURCE_ERROR))
  {
    if(path_debugger) printf("An error was found when executing source\n");
    fprintf(stdout, "File \"%s\" not able to be opened\n",get_arg(0,1)->c_str());
    returner = !UNALTERED_RETURNER;
  }

}



void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }
    // Print contents of Command data structure
    if(debugger) print();
    if(!get_arg(0,0)->compare(SOURCE_ERROR))
    {
      if(path_debugger) printf("An error was found when executing source\n");
    }
       

    //0. Do startup
      //a. Initialize the temporary pipes
      p.init(_inFile);
      
      //b. Open the input redirection file. If none exists, set the file descriptor for in to the
      //   default input
      
      p.open_file(IN,this->_append,this->_inFile);
      //c. Open the fds ERR and then dup2 it
      p.open_file(ERR,this->_append,this->_errFile);
      p.dup_2(err);
       
    //1. Check for shell commands
      //a. check if a shell command can be executed and if it is executable, execute it
      shell_commands();
      //b. if a shell command is executed, do the return cases and then return
      if(returner != UNALTERED_RETURNER)
      {
        if(path_debugger) printf("A shell command ran\r\n");
        clear();
        Shell::prompt();
        return;
      }

    //2. Iterate through all the simple commands currently in the 
      //a. Get the size of simpleCommands
      size_t scc = _simpleCommands.size();
      //a. Create the iterator for simple commands
      //std::vector<std::string>::iterator it = _simpleCommands.begin();
      //for(it; it != _simpleCommands.end(); it++)
      for(size_t i = 0; i < scc; i++)
      {
        //Redirect the imput because it is no longer necessary
        p.dup_2(in);
        //If it is the last simple command, do
        if(i == scc-1)
        {
          //Open the outfile so we can output to it
          p.open_file(OUT,this->_append,this->_outFile);
        }
        else
        {
          if(path_debugger) printf("Pipes\r\n");
          //Since it is not the last simple command, you should make a pipe it up for
          //the in and out
          p.pipe_it_up(in | out);
        }
        //We no longer need the output so just redirect it
        //Dup2 it
        p.dup_2(out);
        //We need to start the child
        //Call fork();
        returner = fork();
        //if(path_debugger) printf("Makes it past fork with returner value %i\r\n",returner); 
        
        if(debugger) print_files(in | out | err);
        
        //Prepare the c_arguments as this is the only place that it is viable
        char ** c_arguments = _simpleCommands[i]->args_to_c();
        //Same with the args_size
        size_t args_size = _simpleCommands[i]->_arguments.size();
        //If successfully completed, call execvp and use a successful exit
        if(returner == CHILD)
        {
          //if(path_debugger) printf("Returner from fork is 0\r\n");
          //If you get here then the last argument is in this string of arguments under simple command
          //Setting the last argument here is a trivial fix to a much more complex problem
          //Just make sure that there is an argument and set it
          if(args_size >= 1)
          {
            last_argument = get_arg(i,args_size-1);
          }
          //Setting the environment variable here in case of exit because of execvp 
          setenv("_",c_arguments[args_size-1],1); 
          //If the current indexed command is to print off the environment variables
          //print off all the environment variables
          if(!get_arg(i,0)->compare(PRINTENV))
          {
            //if(path_debugger) printf("Prints the Environment Variables\r\n");
            //temp holder for the environ variable
            char ** temp_environ = environ;
            //Iterate through the environment variables and print them off
            for(;*temp_environ; temp_environ++){ printf("%s\n",*temp_environ); }
            //Exit with a 0 argument
            
            //p.dup_2(tin | tout | terr);
            exit(EXIT_VAL);
          }
          else
          {
            //if(path_debugger) printf("Calls Execvp\r\n");
            execvp(get_arg(i,0)->c_str(),c_arguments);
            perror("execvp");
            _exit(SOFT_EXIT_VAL);
          }
        }
        //Failed exit
        else if (returner < CHILD)
        {
          if(path_debugger) printf("Returner from fork is less than 0\r\n");
          perror("fork");
          return;
        }
        //Get the last argument and store it just in case it never got to the _exit from the execvp
        setenv("_",c_arguments[args_size-1],1);
        //TODO: Free the c_arguments
        /*for(int a = 0; c_arguments[a]; a++) { delete c_arguments[a]; }*/
        delete c_arguments;
      }
      if(debugger) print_files(in | out | err);
      //Restore the in and out defaults
      p.dup_2(tin | tout | terr);

      //If the background character has not been put into the command line, you need to wait for the
      //background process to finish before continuing
      if(!_background)
      {
        //Finds the status pid of the child and puts it in status
        //Gives the child process the need to wait for returner
        //No tags should be included
        waitpid(returner, &status, 0);
        
        if(path_debugger) printf("Returner and status values after waitpid is called: %i,%i\r\n",returner,status);
        
        if(WIFEXITED(status))
        {
          //If it is here, the exit status is the latest commands exit status
          last_command = WEXITSTATUS(status);
          //Set the environment variable to make variable expansion easier
          setenv_int("?",last_command);
        }
      }
      //So if the background process is complete, the last process is the returner process
      else
      {
        last_process = returner;
        //Set the environment variable to make it easier
        setenv_int("!",last_process);
      }

    if(debugger) print_files(in | out | err);
    // Clear to prepare for next command
    clear();
    // Print new prompt
    if(isatty(0) != 0) { Shell::prompt(); }
    //Shell::prompt(); 
    return;
}


/* Pipe Struct methods */





/*
 * @brief: Initialization method
 *
 * @param:
 *    -std::string * _inFile- the infile to be initialized
 * 
 * @returns void
 */
void pipes::init(std::string * _inFile)
{
  if(path_debugger) printf("Gets to the beginning of init\r\n");
  //Set all the temp positions to a dup of the respective index
  temp[IN] = dup(IN);
  temp[OUT] = dup(OUT);
  temp[ERR] = dup(ERR);
  //Attempt to open the infile
  if(_inFile){}
  //if(_inFile) { fds[IN] = open(_inFile->c_str(), O_RDONLY); }
  //If not, just dup the temp[IN]
  //else { fds[IN] = dup(temp[IN]); }
}







int& pipes::operator[] (const size_t index)
{
  //if(index > 3) return -1;
  return fds[index];
}



/*
 * @brief: Operator that sets a pipe equal
 *
 * @param:
 *    -const pipes& p: the pipe to be copied
 * 
 * @return the pipe itself
 */
pipes& pipes::operator= (const pipes& p)
{
  this->fds[IN] = p.fds[IN];
  this->fds[OUT] = p.fds[OUT];
  this->fds[ERR] = p.fds[ERR];
  return *this;
}




/*
 * @brief: Helper function to open a file with respect to the type and flags
 *
 * @params:
 *    -int type: the type of file to be opened. Can be OUT or ERR
 *    -bool _append: the priveleges given to the new open file
 *    -std::string * _file: a non-null pointer to the name of the file to open
 *
 * @return bool:
 *    -true if a file is opened
 *    -false otherwise
 */
int pipes::open_file(int type, bool _append, std::string * _file)
{
  if(path_debugger) printf("Attempts to open file with arguments (type: %i, _append: %d, _file: %s)\r\n",type,_append,_file->c_str());
  //If the type is not 1 or 2, just return false
  if(type != IN && type != OUT && type != ERR) return 1;
  
  //Set it to Trunc by default
  int flags = FLAG_TRUNC;
  
  //If it wants to be appended, then set the flags to append
  if(_append) { flags = FLAG_APPEND; }
  
  if(type == IN)
  {
    flags = FLAGS_INPUT;
  }

  if(path_debugger) 
  {
    std::string out;
    if(flags == (O_WRONLY | O_CREAT | O_APPEND)) { out = "FLAG_APPEND"; }
    else { out = "FLAG_TRUNC"; }
    printf("Opening the file with flags: %s\r\n",out.c_str()); 
    out.clear();
  }
  
  //Check for Nullity, if the file isn't null, attempt to set the active file descriptor
  if(_file) 
  {
    if(path_debugger) printf("Opens said file\r\n"); 
    if(type == IN)
    {
      fds[type] = open(_file->c_str(), flags);
    }
    else
    {
      fds[type] = open(_file->c_str(), flags, EXTRA); 
    }
    return 2;
  }
  //Else dup the file with the same type
  else 
  { 
    if(path_debugger) printf("Fails to open the file\r\n");
    fds[type] = dup(temp[type]); 
    return 3; 
  }
  //Return true
  return 0;
}
//Calls pipe on a temp set of pipes and then sets the repscetive pipes that are asked to be set
//int i can be any number, but will only hit if the number is inclusively between 1 and 7
//If you want to hit the respected pipe index, one must use the following formula on i
//where n is the value of the desired index
//
// i = i & (1 << n)
//
//return void
void pipes::pipe_it_up(int i)
{
  int tmp[3];
  int i = pipe(tmp);
  if( i & in  ) { fds[IN] = tmp[IN]; }
  else { close(tmp[IN]); }
  if( i & out ) { fds[OUT] = tmp[OUT]; }
  else { close(tmp[OUT]); }
  if( i & err ) { fds[ERR] = tmp[ERR]; }
  else { close(tmp[ERR]); }

  //If you're looking here, took this out for shell
  //return i;
}



/*
 * @brief: Helper function that does all the dup2 method stuff on a given set of fds
 *
 * @param int i: all the possible powers of 2 of the respective fds' to dup2 on
 *
 * return void
 */
void pipes::dup_2(int i)
{
  if( i & in ) { dup2(fds[IN],IN); close(fds[IN]); }
  if( i & out ) { dup2(fds[OUT],OUT); close(fds[OUT]); }
  if( i & err ) { dup2(fds[ERR],ERR); close(fds[ERR]); }
  if( i & tin ) { dup2(temp[IN],IN); close(temp[IN]); }
  if( i & tout ) { dup2(temp[OUT],OUT); close(temp[OUT]); }
  if( i & terr ) { dup2(temp[ERR],ERR); close(temp[ERR]); }

}



/*
 * @brief: Helper function that prints out all the pipes and the info about the pipes
 *
 * @param none
 *
 * return void
 */
void pipes::print()
{
  print_call++;
  printf("Pipe fds IN: %i\n",fds[IN]);
  printf("Pipe fds OUT: %i\n",fds[OUT]);
  printf("Pipe fds ERR: %i\n",fds[ERR]);
  printf("Pipe temp IN: %i\n",temp[IN]);
  printf("Pipe temp OUT: %i\n",temp[OUT]);
  printf("Pipe temp ERR: %i\n",temp[ERR]);
}





SimpleCommand * Command::_currentSimpleCommand;

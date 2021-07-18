
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <cstring>
#include "y.tab.hh"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "unistd.h"
#include "shell.hh"
#include "defines.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 2048

#endif


//An external array that holds all the environment variables
extern char **environ;

//This is required to make a custom shell line
/*extern "C" char * read_line();


//Redefines mygetc. Has no real use until after part 1.
//TODO: Comment this method once you understand it
int mygetc(FILE * f) {
 static char *p;
 char ch;

 if (!isatty(0)) {
 // stdin is not a tty. Call real getc
   return getc(f);
 }

 // stdin is a tty. Call our read_line.
 if (p==NULL || *p == 0) {
   char * s = read_line();
   p = s;
 }
      
 ch = *p;
 p++;
    
 return ch;
}

//Redefine getc using the custom mygetc
#undef getc
#define getc(f) mygetc(f)
/**/

enum state { temp = 0 };

struct pipe_holder
{
  int p[2];
  pipe_holder() {clear();}
  ~pipe_holder() {clear();}
  
  void clear() { p[0] = -1; p[1] = -1; }
  int& operator[] (size_t index) { return p[index]; }
  void _pipe() { pipe(p); }
  void temp(int i) { p[i] = dup(i); }
  void _dup(int i) { p[i] = dup(i); }
  void _dup2(int i) { dup2(p[i],i); }
  void _close(int i) { close(p[i]); }
  void redirect(int i) { _dup2(i); _close(i); }
  void writer(std::string * str, size_t _length)
  {
    //0. Write the string out
    write(p[OUT], str->c_str(), _length);
    //1. Close the OUT
    close(p[OUT]);
    
    return;
  }
  std::string * reader()
  {
    //0. Initialize a buffer and a character holder
    char temp = 0;
    std::string * buff = new std::string();
    //1. Read the pipe into the buffer
    while(read(p[IN],&temp,1)) { *buff += temp; }
    //2. Close the IN
    _close(IN);
    //3. Return the buff
    return buff;
  }
};


//Hehehe, you use this like once
static  void yyunput (int c, char *buf_ptr);

//TODO: Create a string version of this method
void myunputc(int c) {
  unput(c);
}

/**
 * @brief Helper function that unputs an entire string instead of just one character/integer
 *
 * @param string s- character string to be unput
 *
 * @return void
 */
//I'm high key a c++ god. Keep the g lowercase tho.
void myunputs(std::string * s)
{
  //If the string is empty, return void
  if(s->empty()) return;
  //Else use a reverse iterator as you need to unput the string in reverse order.
  //This is what you get for unputting a string instead of just unputting the items as you go
  std::string::reverse_iterator rit;//Reverse iterator for the iteration loop
  //For loop from s.rbegin to s.rend
  for(rit = s->rbegin(); rit != s->rend(); rit++) { myunputc(*rit); }
  //Return void
  return;
}

/**
 * @brief Helper function designed to retrieve the requested evironment variable with the given name
 * 
 * @param std::string var_name is the name of the environment variable
 *
 * @return std::string - the environment variable's value being stored
 *
 */
std::string& mygetenv(std::string& var_name)
{
  //The value to be returned
  std::string value;

  //If the variable name is the "$", set value equal to a string conversion of the process id
  if(var_name.compare("$") == 0) { value = std::to_string(getpid()); }
  //Else if the var_name is "SHELL", set value to the real_path
  //TODO: Find the workaround to set this to it's actual value
  else if(var_name.compare("SHELL") == 0)
  {
    //Makes a real_path array
    char real_path[MAXPATHLEN];
    //Sets the value to the realpath
    value = realpath("/proc/self/exe",real_path);
  }
  //Else, lookup the environment variable by name
  else
  {
    //Get the enviroment variables value from the environ list 
    char * temp = getenv(var_name.c_str());
    //If the variable is null, just substitute the name in it's place
    if(!temp) { value = var_name.c_str(); }
    //Else set the value
    else { value = temp; }
  }
  var_name.clear();
  var_name = value;
  value.clear();
  //Return the value
  return var_name;
}
/**/



/**
 * @brief Helper function designed to split strings at an extremely efficient rate in order to properly  * expand the environment variables
 * 
 * @param string variable - the string to be parsed at any instance of "${" followed by a string and
 * goes till the last instance of "}"
 *
 * @return std::string - the expanded string
 *
 */
std::string& expand_variables(std::string& str) 
{
  //If an instance of "${" cannot be found, return the string how it was
  if(str.find("${") <= str.size())
  {
    //Get everything before "${"
    std::string left = str.substr(0,str.find("${"));
    //Get everything to the right of "}" and assign that value to right
    std::string right = str.substr(str.find("}")+strlen("}"));
    //Get everything between "${" and "}" and assign the value to middle
    std::string middle = str.substr(left.size()+strlen("${"),str.find("}")-str.find("${")-2);
    middle = mygetenv(middle);
    //Since the middle has already been expanded, we simply need to expand the rest of the variables
    //The left is everything before the first expansion is possible, so the left doesn't need to
    //be expanded. Therefore, expand the right and be done with it
    str = left + middle + expand_variables(right);
    //Clear the left, right and middle
    left.clear();
    right.clear();
    middle.clear();
    //Return the expanded string
  }
  return str;
}

/**/

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
  /* You can't tell me what to  *
   * do; you're not my real dad */
}
"|" {
  return PIPE;
}
">>&" {
  return GREATGREATAMPERSAND;
}
">>"  {
  return GREATGREAT;
}
">&" {
  return GREATAMPERSAND;
}
">" {
  return GREAT;
}
"<<" {
  return LESSLESS;
}
"<" {
  return LESS;
}
"&" {
  return AMPERSAND;
}
"2>" {
  return TWOGREAT;
}
exit {
  return EXIT;
}


$\([^\n\t]*\) {

  std::string _input = yytext;
  
  //INITIALIZE THE SUBSHELL VARIABLES
    //0. Set the temporary pipes
      pipe_holder tmp; tmp.temp(IN); tmp.temp(OUT);
    
    //1. Create a fds in set and an fds out set
      pipe_holder sub_in; sub_in._pipe();
      pipe_holder sub_out; sub_out._pipe();
    
    //2. Get the subshell text
      _input = _input.substr(2,_input.size()-3);
      _input += "\n";
  
  //START THE SUBSHELL
    //1. Write the command to the child
      sub_in.writer(&_input,_input.length());
      _input.clear();
    
    //2. Redirect the stdin to the _in reader
      sub_in.redirect(IN);
    
    //3. Redirect the stdout to the _out writer
      sub_out.redirect(OUT);
    
    //4. Get forked by shell 
      //a. Hold the value
      int ret = fork();
      
      //b. If you are in the child, recall the main
      if(ret == CHILD)
      {
        //Got tired of seeing an error so I just put the NULL value into a pointer
        char ** null_pntr = NULL;
        execvp("/proc/self/exe", null_pntr); 
        _exit(1);
      }
      else if( ret < CHILD)
      {
        perror("fork");
        _exit(1);
      }
    
    //5. Redirect the standards to the temporary holders
      tmp.redirect(IN);
      tmp.redirect(OUT);
  
  
  //GET THE OUTPUT OF THE SUBSHELL
    //0. Initialize a holder to read the output of _out
      std::string * _output = sub_out.reader();
    
    //1. Replace all endlines with a space in the string
      for(size_t a = 0; a < _output->size(); a++)
      {
        if(_output->at(a) == '\n') { _output->at(a) = ' '; }
      }
    
    //2. Unput the string
      myunputs(_output);
    
    //3. Delete the output container
      _output->clear();
      delete _output;
}

[\"][^\n\"]*[\"] {
  
  
  
  // 0. Handle quotations surrounding something
  std::string quote = yytext;
  
  // 1. Remove the quotes
  quote = quote.substr(1,quote.length()-2);
  
  // 2. Create a new cpp_string
  yylval.cpp_string = new std::string(quote);
  
  // R. Return as a word
  return WORD;

}

[~]([^ \n\t]*) {
  //TODO: Figure out what the fuck this does
  //It's Tilde
  
  //0. Temp holder for the yytext
    std::string yytemp(yytext);
  
  //1. Temp holder for the path
    std::string path;
  
  //2. If we a just doing a tilde, set the path to HOME
    if(yytemp.size() <= 1) { path = getenv("HOME"); }
  
  //3. Else if it is a file path, substitute the tilde with a HOME and append the path to the home path
    else if(yytemp[1] == '/') { path = getenv("HOME"); path += yytemp.substr(1); }
  
  //4. This only happens if you don't want to find a directory underneath your HOME
    else { path = "/homes/"+yytemp.substr(1); }
  
  //5. Set the yyval string
    yylval.cpp_string = new std::string(path.c_str());
  
  //6. Clear the temps
    yytemp.clear();
    path.clear();
  
  
  return WORD;
}


[^ \n\t]*([\$][\{][^ \n\t]*?[\}])[^ \n\t]*  {


  //0. Standardize the variable to a string so it can be handled without malloc
  std::string variable = yytext;
  
  //1. Get the string starting after \$ and before the next command
  std::string * value = new std::string();
  
  //2. Expand the environment variables if necessary
  *value = expand_variables(variable);
  
  //3. Set the yylval string to the value of the expanded thing 
  yylval.cpp_string = value;
  
  //4. Clear the temp holder for yytext
  variable.clear();
  
  return WORD;

}

source([\ ][^ \n\t]*[^\ \n\t]) {
  //HANDLES THE SOURCE CALL
  
  //1. Get the yytext and parse it for the thing you need the source for
  std::string _file = yytext;
  _file = _file.substr(7, _file.size()-7); 
  //2. Open the file in a read only form
  FILE * file = fopen(_file.c_str(), "r");
  
  //3. If the file exists, do the process for creating a buffer
  //if(access(_file.c_str(),F_OK))
  if(file != NULL)
  {
    //a. Create yy_buffer and hold it's yy_buffer_state
    YY_BUFFER_STATE state = yy_create_buffer(file, YY_BUF_SIZE);
    
    //4. Buffer it then parse using yyparse() the item
    yypush_buffer_state(state);
    yyparse();
    
    //5. Pop it from the state stack
    yypop_buffer_state();
  
    //6. Close the file
    fclose(file);

    //7. Memory Cleanup  
    _file.clear();
  }
  else 
  {
    std::string * temp = new std::string("SOURCE ERROR MATCH ");
    temp->append(_file);
    yylval.cpp_string = temp;
    _file.clear();
    return SOURCE_ERROR;
  }
}


([^ \t\n<>\|\&\"\(\)]|\\.)+ {
  //Do the escape characters
  
  //0.Create a temp string
  std::string line = "";
  
  //1. Iteratre through yytext and try till a null and try and find an escape character
  for(int index = 0; yytext[index] != '\0'; index++)
  {
    //a. If the current indexing isn't an escape character, add it to string
    if(!(yytext[index] == '\\' && yytext[index+1] != ' ')) line += yytext[index];
  }
  //2. Set the yylval to line
  yylval.cpp_string = new std::string(line);
  
  //3. Clear the line
  line.clear();
  
  return WORD;
}

. {
  return NOTOKEN;
}
        


/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token <cpp_string> SOURCE_ERROR
%token NOTOKEN NEWLINE PIPE GREAT GREATGREAT LESS LESSLESS
%token AMPERSAND GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT
%token EXIT

%{
//#define yylex yylex
#include <cstdio>
#include <iostream> 
#include <cstring> 
#include <string>
#include <vector>
#include "shell.hh"
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h> 
#include <signal.h>
#include <sys/stat.h>
#include <algorithm>
#include "defines.h"


std::vector<std::string *> * paths;


void yyerror(const char * s);
int yylex();

/* Gonna build some helper functions */
/*         Ahhhh Yeah                */

/**
 * @brief: Helper function designed solely to make my life harder probably.
 * JK. It just inserts the argument into the currentSimpleCommand list:
 *
 * @param: std::string * arg - the argument to be inserted into the currentSimpleCommand list
 *
 * @return void
 *
 */
inline void insert_arg(std::string * arg)
{
  Command::_currentSimpleCommand->insertArgument(arg);
}

/*
 * @brief: Helper function that also sorting of a vector of string pointers
 * 
 * @params: 
 *         0. std::string * s1 - the first string pointer to compare
 *         1. std::string * s2 - the second string 
 * pointer to compare
 *
 * @return bool - returns true if s2 is greater than s1, else returns false
 */

bool compare_str_ptr(std::string * s1, std::string * s2) { return *s1 < *s2; }
/**
 * @brief: Helper function that iterates through any given expression and checks if it needs expanding
 *
 * @param: std::string * expression - the expression that is in questionable expansion status
 *
 * @returns true or false based on the answer to the questions
 *
 */
inline bool needs_expanding(std::string * expression)
{
  for(int a = 0; a < expression->size(); a++)
  {
    if(expression->at(a) == '?' || expression->at(a) == '*') { return true; }
  }
  return false;
}

/*
 * @brief: Handles the case of a source_error
 * 
 * @param: std::string * str -
 *
 */ 
inline void source_error(std::string * str)
{
  std::string holder = *str;
  str->clear();
  *str = holder.substr(19,holder.size()-19);
  holder.clear();
  std::string * command = new std::string("SOURCE ERROR MATCH");
  insert_arg(command);
  insert_arg(str);
}

/*
template <typename T>
struct _holder
{
  T t;
  _holder() {}
  ~_holder() {}
  
  T& operator=(const T& other);
};
*/

struct regex_holder
{
  regex_t r;
  regmatch_t match;
  int expression_buffer;
  int mc;
  
  regex_holder(const char * str) { expression_buffer = regcomp(&r, str, REG_EXTENDED|REG_NOSUB); mc = 0; }
  ~regex_holder() {}
  
  void freereg() { regfree(&r); }
  int next_match(struct dirent * _entry)
  {
    //if(mc++) { regfree(&r); }
    return regexec(&r,_entry->d_name,0,&match,0);
  }
  
};


//Used to break down the entirety of a wildcard
//Yes, it was that horrible to code
struct path
{

  //Reference to the parent of the current path
  path * prev; path * next; bool _needs_expanding; std::string * current_directory;
  std::vector<std::string*> * paths; int successful_children;
  //Constructor for any descendants of the source parent
  //path * _parent should only refer to 
  path(path * _prev, std::string * pattern, std::vector<std::string*> * _paths) 
  {
    //0. Initialize variables
    paths = _paths;
    prev = _prev;
    int successful_children = 0;
    //1. Iteratre through the pattern
    size_t slash_index = pattern->find("/");
    if(slash_index != std::string::npos)
    {
      //a. Temp to hold the values of pattern
      std::string temp = *pattern;
      //b. Clear the pattern
      pattern->clear();
      //c. Set the pattern to everything after the slash_index
      *pattern = temp.substr(slash_index+1);
      //d. Instantiate the current directory path
      current_directory = new std::string(temp.begin(),temp.begin()+slash_index);
      //e. Delete the temp
      temp.clear();
      //f. Create the next node
      next = new path(this,pattern,_paths);
    }
    else
    {
      current_directory = pattern;
      next = NULL;
    }
    _needs_expanding = needs_expanding(current_directory);
  }
  
  //Destructor
  ~path() {}
  
  //Clears all existing pointers and clears all the descendants
  //DANGROUS: DO NOT CALL THIS ON ANY CHILDREN UNLESS YOU WANT TO DELETE THEM ALL
  void clear();
  
  //Prints all the nodes current directories out
  void print(int i);
  
  //Inserts the item into the paths vector
  void insert_to_paths(std::string * subpath);

};

/*
 * @breif: Recursive function that clears the entire path list
 *
 * @param: none
 *
 * @return: none
 */
void path::clear()
{
  //0. If there exists a current directory, delete it 
  if(current_directory != NULL)
  {
    current_directory->clear();
    delete current_directory;
    current_directory = NULL;
  }
  
  //1. If the exists a next, delete it
  if(next != NULL)
  {
    next->clear();
    delete next;
    next = NULL;
  }
  
  //2. Set paths to NULL
  paths = NULL;
  
  //3. Set needs expanding to false
  _needs_expanding = false;

}

/*
 * @breif: Function that prints off the entirety of the path list
 *
 * @param: iterator to signify how many times the path has been broken down
 *
 * @return: none
 */
void path::print(int i)
{
  printf("%d > %s does%s needs expanding.\n",i++,current_directory->c_str(), _needs_expanding?"":"n't");
  if(next != NULL) next->print(i);
}


/*
 * @breif: Function that inserts the given string into paths
 *
 * @param: iterator to signify how many times the path has been broken down
 *
 * @return: none
 */

void path::insert_to_paths(std::string * expression)
{
  successful_children++;
  
  int entry_start = 0;
  
  //1. Get the two front charaters
  char c0 = expression->at(0); char c1 = expression->at(1);
  if(c0 == '/' && ( c1 == '/' || c1 == '.' ) ) { entry_start = 1; } 
  else if(c0 == '.' && c1 == '/') { entry_start = 2; }
  
  //2. Erase the characters that need to be erased
  expression->erase(expression->begin(), expression->begin()+entry_start);
  
  //3. Insert to paths
  paths->push_back(expression);

  //4. Return
  return;
}



void expandWildcards(std::string * prefix, path * p)
{
  if(y_path_debugger)
  { printf("expand Called with prefix (%s) and p (%s)",prefix->c_str(),p->current_directory->c_str()); }
  std::string * recurrer;
  std::string * finalized_path;
  //0. If the current doesn't need expanding, recurr if possible
  if(!p->_needs_expanding)
  {
    
    recurrer = new std::string(*prefix + "/" + *(p->current_directory));
    if(p->next != NULL) expandWildcards(recurrer,p->next);
  }
  //1. Else we need to expand
  else
  {
    //a. If there is no prefix, set it to '.' for the current working directory
    if(prefix->empty()) { prefix->insert(0,"."); }
    
    //b. Create a temp that will be used for regex expansion
    std::string temp = "^";
    
    //c. Iterate through the current_directory of path to 
    for(size_t a = 0; a < p->current_directory->size(); a++)
    {
      //i. Replace * with .*, > with .? and . with \\.
      switch(p->current_directory->at(a))
      {
        case '*': temp += ".*"; break;
        case '?': temp += ".?"; break;
        case '.': temp += "\\."; break;
        default: temp += p->current_directory->at(a); break;
      }
    }
    
    //d. Need to append this to the temp
    temp += "$\0";
    
    //e. Open a regex holder
    regex_holder re(temp.c_str());
    
    //f. Clear temp as we no longer need it
    temp.clear();
    
    //g. Create a directory and an entry holder
    struct dirent * entry;
    DIR * dir = opendir(prefix->c_str());
    size_t valid_matches = 0;
    
    //h. Iterate through the directory till all matches are used up
    while((entry = readdir(dir)) != NULL)
    {
      if(y_path_debugger) printf("Try's to match entry %s\n",entry->d_name);
      
      //i. Check if the current entry if a regex match
      if(!re.next_match(entry))
      { 
        if(y_path_debugger) printf("Matches it\n");
        
        //0. Making it in here means there was a valid match so we need to iterate the match count
        valid_matches++;
        
        //1. If there is still more expansions to be made, recurr
        if(p->next != NULL)
        {
          //a. If the entry type is a directory, recurr through it
          if(entry->d_type == DT_DIR)
          {
            //a. Make a new string with "/" and the entry 
            recurrer = new std::string(entry->d_name);
            if(prefix->compare("/") == 0) { recurrer->insert(0,"/"); }
            else if (prefix->compare(".") != 0) 
            { 
              recurrer->insert(0,"/");  
              recurrer->insert(0,prefix->c_str()); 
            }
            //b. Free the regex
            //re.freereg();
            //c.Then recurr
            expandWildcards(recurrer,p->next);
          }
        }
        //2. If there are no more expansions
        else
        {
          //a. Instantiate the finalized path
          finalized_path = new std::string(*prefix + "/" + entry->d_name);
          //b. If it is a hidden file, check to make sure we are taking hidden files
          if(entry->d_name[0] == '.')
          {
            if(p->current_directory->at(0) == '.') { p->insert_to_paths(finalized_path); }
            else { finalized_path->clear(); delete finalized_path; }
          }
          //c. Else if it is a normal file, add it
          else { p->insert_to_paths(finalized_path); }
        }
        //re.freereg();
      }
      //re.freereg();
    }
    //i. Close the directory
    closedir(dir);
    //j. If there were no valid matches
    //if(valid_matches == 0)
    if(p->successful_children == 0 && valid_matches == 0)
    {
      finalized_path = new std::string(*prefix + "/" + *(p->current_directory));
      p->insert_to_paths(finalized_path);
    }
    re.freereg();
  }
  
  prefix->clear();
  delete prefix;
  return;
}


//This is wildcards
bool expandWildcardsIfNecessary(std::string * arg)
{
  //0. If there are not any wild cards in the string, simply insert the argument and return false
  if(arg->find("*") == std::string::npos && arg->find("?") == std::string::npos)
  {
    insert_arg(arg);
    return false;
  }
  //1. Instantiate the paths vector
  paths = new std::vector<std::string*>();
  
  //2. If not, create a new path object with the arg as the starting string
  path * p = new path(NULL, arg,paths);
  
  if(y_path_debugger) p->print(0);
  
  //3. Initialize the prime prefix
  std::string * prime_prefix = new std::string();
  
  
  //4. Expand the damn wildcards 
  expandWildcards(prime_prefix,p);
  
  bool has_paths = paths->size() > 0;
  //5. If the vector is not empty, insert all the paths as arguments into the simple command
  if(has_paths)
  {
    if(path_debugger) printf("Directory paths were found that match the wildcard\n");
    //a. Sort the vector using std::sort
    std::sort(paths->begin(),paths->end(), compare_str_ptr);
    //b. Insert all the strings from paths as arguments into a simple command
    std::vector<std::string *>::iterator it = paths->begin();
    for(it; it != paths->end(); it++) { insert_arg(*it); }
  }
  //6. Delete the path * p
  p->clear(); delete p;
  //7. Destroy the paths holder
  //   Do not delete paths as all the string pointers are to be inserted as argument into 
  paths->clear(); delete paths; paths = NULL;
  /**/
  //7. Return true
  return true;
}






%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
  pipe_list io_list background_optional NEWLINE {
    if(debugger)  printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | EXIT NEWLINE {
    printf("Keep the change you filthy animal!\n");
    exit(2);
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  | lex_error {
    Shell::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    if(debugger) printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    /* Command::_currentSimpleCommand->insertArgument( $1 ); */
    expandWildcardsIfNecessary( $1 ); 
  }
  ;

command_word:
  WORD {
    if(debugger) printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;
pipe_list:
  pipe_list PIPE command_and_args
  | command_and_args
  ;
io_list:
  io_list io_modifier_opt
  |
  ;
io_modifier_opt:
  TWOGREAT WORD {
    /* TODO: Comment this shit
    /* Something something err file*/
    if(debugger) printf("TWOGREAT WORD\n");
    if(Shell::_currentCommand._errFile)
    {
      if(debugger) printf("AMB ERROR\n");
      printf("Ambiguous output redirect.\n"); 
    }
    else
    {
      Shell::_currentCommand._errFile = $2;
    }
  }
  | GREAT WORD {
    /* printf("   Yacc: insert output \"%s\"\n", $2->c_str()); */
    if(debugger) printf("GREAT WORD\n");
    if(Shell::_currentCommand._outFile)
    {
      if(debugger) printf("AMB ERROR\n");
      printf("Ambiguous output redirect.\n");
    }
    else
    { 
      Shell::_currentCommand._outFile = $2;
    }
  }
  | GREATGREAT WORD {
    if(debugger) printf("GREATGREAT WORD\n");
    if(Shell::_currentCommand._outFile)
    {
      if(debugger) printf("AMB ERROR\n");
      printf("Ambiguous output redirect.\n");
    }
    else
    {
      /* if it doesn't, set it */
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._append = true;
    }

  }
  | GREATGREATAMPERSAND WORD {
    if(debugger) printf("GREATGREATAMPERSAND WORD\n");
    if(Shell::_currentCommand._outFile || Shell::_currentCommand._errFile)
    {
      if(debugger) printf("AMB ERROR\n");
      printf("Ambiguous output redirect.\n");
    }
    else
    {
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = $2;
      Shell::_currentCommand._append = true;
    }

  }
  | GREATAMPERSAND WORD {
    if(debugger) printf("GREATAMPERSAND WORD\n");
    if(Shell::_currentCommand._outFile || Shell::_currentCommand._errFile)
    { 
      if(debugger) printf("AMB ERROR\n");
      printf("Ambiguous output redirect.\n");
    }
    else
    {
      Shell::_currentCommand._outFile = $2;
      Shell::_currentCommand._errFile = $2;
    }

  }
  | LESS WORD {
    /* Check to see if an in file exists */
    if(Shell::_currentCommand._outFile || Shell::_currentCommand._errFile)
    { 
      if(debugger) printf("AMB ERROR\n");
      printf("Ambiguous output redirect.\n");
    }
    else
    {
      /* if it doesn't, set it */
      Shell::_currentCommand._inFile = $2;
    }

  }
  ;
background_optional:
  AMPERSAND {
    if(debugger) printf("Background value is set\r\n");
    Shell::_currentCommand._background = 1;
  }
  | /* empty */
  ;
lex_error:
  SOURCE_ERROR {
    Command::_currentSimpleCommand = new SimpleCommand();
    source_error( $1 );
  }
  | 
  ;
%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif

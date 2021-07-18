#include "tester.hh"
#include "defines.hh"



/*
 * brief> Inserts all strings made from splitting the string by the delim into the passed in vector 
 *
 * param> 0. std::string * - the string to split
 *        
 *        1. std::string * - the delimiter to split the string by
 *
 *        2. std::vector<std::string> * - the vector to insert the strings into
 *
 * return> void
 */
inline void delimsplitter(std::string * str, std::string * delim, std::vector<std::string> * vec)
{
  //0. Check the valdiity of the arguments
    //a. If anything is NULL, return
      if(str == NULL || delim == NULL || vec == NULL) { return; }

    //b. If the delim is empty or the str is empty, return
      if( str->empty() || delim->empty() ) { return; }

  //1. Make a temp string holder
      std::string temp_str = *str;
      std::string split_token;

  //2. Hold the delimiter location
    std::string::size_type _found = 0;

    while((_found = temp_str.find(*delim)) != std::string::npos )
    {
      //a. Set the token to the substring
        split_token = temp_str.substr(0, _found);
        if(VD) { std::cout << "var debugger> A string was found: " << split_token << std::endl; }

      //b. Insert the split_token into the vector
        vec->push_back(split_token);

      //c. Erase the string from to the end of the found delimiter
        temp_str.erase(0, _found + delim->length());

    }
  if(!temp_str.empty()) { vec->push_back(temp_str); }


  //3. Clear everything local
      temp_str.clear();
      split_token.clear();

  //R. Return
    return;
}

inline void validate_path(std::string * _path)
{
  //0. Validate the arguments
    //a. If the file_path is NULL, return 
      //i. check for nullity
        if(_path == NULL || _path->empty() ) { return; }

  //1. Make a delimitter string vector
    //a. Make the temp holders    
      std::vector<std::string> directories;
      std::string delimiter = FSLASH;

    //b. Call the splitter
      delimsplitter(_path, &delimiter, &directories);

      if(VD)
      {
        std::cout << "var debugger> Directories is as follows: \n";
        for(int a = 0; a < directories.size(); a++)
        { std::cout << "var debugger> directories[" << a << "]: " << directories[a] << std::endl; }
      }

    //c. The delimiter is useless so clear it
      delimiter.clear();

  //2. Iterate through directories and keep track of how many directories you are from the ROOT_DIR
    //a. Any positive level means we are currently still in subdirectory of the ROOT
      int level = 0;
      bool invalid = false;

    //b. Adjust the level for each item
    //   As soon as we invalidate the path, we no longer care about the count but we need to
    //   clear all the directory strings
      for(int a = 0; a < directories.size(); a++)
      {
        //0. Do nothing for empty 
          if(directories[a].compare("") == 0 || directories[a].compare("\r\n") == 0) {}

        //1. If the current directory's name is "..", the level will decrease  
          else if(directories[a].compare("..") == 0) { level--; }

        //2. If the current directory's name is equal to the ROOT_DIR_NAME, it's invalid
          else if(directories[a].compare(ROOT_DIR_NAME) == 0) { invalid = true; }

        //3. Else we a descending a directory  
          else { level++; }

        //4. If the level is less than 0, we have successfully traversed above the ROOT_DIR
        //   invalidating the path
          if(level < 0) { invalid = true; }

          directories[a].clear();
      }

    //c. Directories is no longer useful and has no non empty members
      directories.clear();

    //d. If the path is invalid, clear it, then set it to invalid
      if(invalid) { _path->clear(); *_path = INVALID_PATH; }

  //R.
    return;
}



/*
 * brief> This makes calling the main way easier, as argv is now a vector of strings
 *
 * param> std::vector<std::string> * argv, the pointer to the vector of string arguments
 *
 * return> int - the normal functionality return, typically 0
 */
int tester_main(std::vector<std::string> * argv);




int main(int argc, char ** argv)
{
  //0. Validate the argument count
    if(argc < 1) { std::cerr << "Invalid argument count\n" << std::endl; }
  
  //1. Make argument holder and a vector of argument strings
    std::vector<std::string> _argv;
    std::string argument = "";
  
  //2.Fill the arguments vector with arguments from argv
    for(int a = 0; a < argc; a++) 
    { 
      argument = argv[a]; 
      
      std::cout << "Reading in argument: " << argument << std::endl;
      
      _argv.push_back(argument); 
      
    }
    argument.clear();


  //3. Call the tester main function, getting it's return value
    int returner = tester_main(&_argv);

  //4. Clear _argv
    if(!_argv.empty())
    {
      for(int a = 0; a < _argv.size(); a++) { _argv[a].clear(); }
      _argv.clear();
    }


  //R. Return the returner
    return returner;
}


int tester_main(std::vector<std::string> * argv)
{
  //0. Make the iterator and a temp string
    std::vector<std::string>::iterator it = argv->begin();
    std::string temp = "";

  //1. Call the function on each item in the iterator
    for(it; it != argv->end(); it++) 
    { 
      temp = *it; 
      std::cout << "Validate path being called on: " << temp << std::endl;
      validate_path(&temp);
      std::cout << "Line after validate path was called: " << temp << std::endl << std::endl;
    }
  
  //R. Return 0
    return 0;
}







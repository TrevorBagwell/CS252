#include "pipe.h"


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
  //Set all the temp positions to a dup of the respective index
  temp[IN] = dup(IN);
  temp[OUT] = dup(OUT);
  temp[ERR] = dup(ERR);
  //Attempt to open the infile
  if(_inFile) fds[IN] = open(_inFile->c_str(), O_RDONLY);
  //If not, just dup the temp[IN]
  else fds[IN] = dup(temp[IN]);
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
  int r = 0;
  //If the type is not 1 or 2, just return false
  if(type != OUT && type != ERR) return r;
  r++;
  //Set it to Trunc by default
  int flags = FLAG_TRUNC;
  //If it wants to be appended, then set the flags to append
  if(_append) { flags = FLAG_APPEND; }
  //Check for Nullity, if the file isn't null, attempt to set the active file descriptor
  if(_file) { fds[type] = open(_file->c_str(), flags, EXTRA); return -3; }
  r++;
  //Else dup the file with the same type
  else { fds[type] = dup(temp[type]); return r; }
  r++;
  //Return true
  return r;
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
  pipe(tmp);
  if( i & in ) fds[IN] = tmp[IN];
  if( i & out ) fds[OUT] = tmp[OUT];
  if( i & err ) fds[ERR] = tmp[ERR];
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
  if( i & t_in ) { dup2(temp[IN],IN); close(temp[IN]); }
  if( i & t_out ) { dup2(temp[OUT],OUT); close(temp[OUT]); }
  if( i & t_err ) { dup2(temp[ERR],ERR); close(temp[ERR]); }

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
  printf("Pipe fds IN: %i",fds[IN]);
  printf("Pipe fds OUT: %i",fds[OUT]);
  printf("Pipe fds ERR: %i",fds[ERR]);
  printf("Pipe temp IN: %i",temp[IN]);
  printf("Pipe temp OUT: %i",temp[OUT]);
  printf("Pipe temp ERR: %i",temp[ERR]);
}







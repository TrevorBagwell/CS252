#include <iostream>
#include <fstream>
#include <sstream>


//Container includes
#include <string>
#include <vector>


//C method includes
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//My stuff
#include "defines.h"
//#include "socket.hh"
//#include "server.hh"

/*
 * brief> Prints a std::string as a list of ints instead of the character values
 *
 * param> std::string * - the string object to be printed out as ints
 *
 * return> void
 *
 */
inline void printAsInt(std::string * str)
{
  //0. If it is empty, return
    if(str->empty()) { return; }
  //1. Else iterate through the list of characters and print them off
    for(int a = 0; a < str->size(); a++) 
    { 
      if(str->at(a) == '\n') { std::cout << std::endl; }
      else { std::cout << (int)str->at(a) << " "; }
    }
  //R. Return
    return;
}
const char * usage =
"                                                               \n"
"daytime-server:                                                \n"
"                                                               \n"
"Simple server program that shows how to use socket calls       \n"
"in the server side.                                            \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   daytime-server <port>                                       \n"
"                                                               \n"
"Where 1024 < port < 65536.             \n"
"                                                               \n"
"In another window type:                                       \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where daytime-server  \n"
"is running. <port> is the port number you used when you run   \n"
"daytime-server.                                               \n"
"                                                               \n"
"Then type your name and return. You will get a greeting and   \n"
"the time of the day.                                          \n"
"                                                               \n";
/* LINE LOCATIONS: 50
 *
 * MY METHODS: 75
 *
 * DECLARATIONS AND COMMENTS:
 *   mutex> 140
 *   address> 160
 *   process> 180
 *   
 *   STRUCTURES: (COMMENT, DECLARATION)
 *     socket_o> (220, 320)
 *     server_o> (360, 435)
 *
 * CLASS IMPLEMENTATIONS: 
 *   
 *   socket_o> 465
 *   server_o> 615
 *
 * FUNCTION IMPLEMENTATIONS: 
 *
 *   mutex> 730
 *   address> 745
 *   process> 775
 *
 * MAIN: TDB
 *     
 */



/*************** MY METHODS ***************/



/*
 * brief> Helper functions that just reduces lines of code, calls perror then exits
 *
 * params> 0. std::string * message - pointer to the string object that cotains the err message
 *         
 *         1. int exit_type - conatins the status which you would like to exit
 * 
 * return> void
 */
inline void exit_err(const char * message, int exit_type)
{
  //0. Call perror with the requested message
    perror(message);
  //1. Exit the program
    exit(exit_type);
}
//@OVERRIDE: Change the const char * to a std::string *
inline void exit_err(std::string * message, int exit_type)
{
  //0. Call perror with the requested message
    perror(message->c_str());
  //1. Exit the program
    exit(exit_type);
}



/*
 * brief> Prints the character set defined end in the given socket
 *
 * params> 0. socket_t s - the socket_t to print out to
 *          
 * return> void
 */
inline void crlfrn (socket_t s) { write(s, crlf, 2); }



/*
 * brief> Helper function that finds what type of content would be in the given path
 *
 * params> 0. std::string * path - reference to the path you want to find's content type
 *
 * return> const char * - reference to the desired cstring
 */
inline const char * content_type(std::string * path)
{
  //0. If substring ".html" or ".html/" are found, it's an html text
    if(path->find(".html") != std::string::npos || path->find(".html/") != std::string::npos) 
    { return html_txt; }
  
  //1. Else if substrings ".gif" or ".gif/" are found in the string, it's a gif
    else if(path->find(".gif") != std::string::npos || path->find(".gif/") != std::string::npos) 
    { return gif; }
  
  //2. Otherwise it is just plain text
    else { return plain_txt; }
}
/******************* DATA STRUCTURE DECLARATIONS *********************/

struct socket_o;

struct server_o;



/******************* MUTEX DECLARATIONS *******************/

//The mutex thread and it's attributes
pthread_mutex_t mutex_thr;
pthread_mutexattr_t mutex_attr;



/*
 * brief> Helper function that initializes the mutex thread and mutex attributes of the program
 * 
 * param> N/A
 *
 * return> void
 */
inline void init_mutex();




/******************* ADDRESS DECLARATIONS *******************/

//Holder for the current working address of the last server to call the init_address
struct sockaddr_in * working_address;



/*
 * brief> Helper function that initializes an address for the given port
 * 
 * param> 0. struct sockaddr_in * address - a pointer to a place to initialize an address
 *        
 *        1. portno port - the port to have an address initialized for
 *
 * return> void
 */
inline void init_address(struct sockaddr_in * address, portno port);



/****************************** PROCESS DECLARATIONS ******************************/
 
/*
 * brief> Helper function that allows the pooling of slave sockets
 *
 * param> 0. socket_t puppet - socket to be pooled
 *
 * return> void
 */
inline void init_attr(pthread_attr_t * attr);





/****************************** PROCESS DECLARATIONS ******************************/

/*
 * brief> Helper function that allows the pooling of slave sockets
 *
 * param> 0. socket_t puppet - socket to be pooled
 *
 * return> void
 */
inline void pool_puppet(socket_t puppet);



/*
 * brief> Helper function that calls process_request and prints the output to the socket
 *
 * param> 0. socket_t puppet - socket with a request to process
 *
 * return> void
 */
inline void process_socket(socket_t puppet);



/*
 * brief> Helper function that processes a request
 *
 * param> 0. socket_o * puppet - reference to a socket object with a request process
 *        
 *        1. std::string * message - reference to the message that will print out to the
 *           puppet after the process request is returned
 *
 * return> void
 */
inline void process_request(socket_o * puppet, std::string * message);

/*
 * brief> Helper function that navigates 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */



/******************* NON-PRIMITIVE DATA TYPES **********************/

/*
 * 
 * name> socket_o
 *
 *
 * type> struct
 *
 *
 * brief> Handles the c code and initialization of a socket using C++
 *
 *
 * variables>
 *                0. socket_t val - the fds returned from calling 
 *             
 *                1. bool master - holds the type (true for master, false for slave)
 *
 *                2. socket_error err - holds the error return value of the functions
 *                                      called on the socket
 *             
 *                3. socket_option opt - holds the optval
 *
 *
 * constructor> NOTE: Each constructor has an init function to go with it.
 *
 *                0. socket_o():
 *                      brief> default constructor and intializer
 *                      
 *                      param> N/A
 *                      
 *                      return> N/A
 *
 *                1. socket_o(struct sockaddr_in * address):
 *                      brief> initializes a master socket at the given address
 *                      
 *                      param> 0. struct sockaddr_in * address - the address of the master socket
 *
 *                      return> N/A
 *
 *
 * destructor> NOTE: Each destructor has a terminate function to go with it.
 * 
 *                0. ~socket_o():
 *                      brief> default destructor and terminator, closes the socket val
 *                      
 *                      param> N/A
 *                      
 *                      return> N/A
 *
 *
 * operators>
 * 
 *                0. operator=(socket_t):
 *                      brief> makes a slave with value equal to the socket_t
 *
 *                      param> 0. socket_t _val - the value of the new socket
 *
 *                      return> socket_o& - a reference to the current object
 *
 *             
 * functions>
 *
 *                0. finish(int):
 *                      brief> shuts the socket down then calls terminate on it
 *
 *                      param> 0. int - the value to shutdown with 
 *
 *                      return> N/A
 *                      
 *
 *                1. out(std::string *, int):
 *                      brief> outputs the msg object to the current socket
 *
 *                      param> 0. std::string * - the message to write
 *
 *                             1. int - the amount of characters to write
 *                            
 *                      return> void
 *
 *
 *                2. in(std::string *, int):
 *                      brief> reads the current socket into the buffer
 *
 *                      param> 0. std::string * - the string to write to
 *                      
 *                             1. int - the amount of characters to read from the socket
 *
 *                      return> void
 *
 *
 *                3. output(int):
 *                      brief> outputs the object to stdout
 *
 *                      param> 0. int - order of it's outpt
 *
 *                      return> void
 *                      
 */

struct socket_o
{
  socket_t val;
  socket_error err;
  socket_option opt;
  bool master;
  
  
  //Constructor 0
  socket_o() { init(); }
  void init();
  //Constructor 1
  socket_o(struct sockaddr_in * address) { init(address); }
  void init(struct sockaddr_in * address);

  //Destructor 0
  ~socket_o() { terminate(); }
  void terminate();

  //Operator 0
  socket_o& operator=(socket_t _val);
  
  //Function 0
  void finish(int i);
  //Function 1
  void out(std::string * msg, int write_size);
  //Function 2
  void in(std::string * buffer, int read_size);
  //Function 3
  void output(int i);
  //Function 4

};

//Holds the current working socket incase of exit
socket_o * working_socket;



/*
 * 
 * name> server_o
 *
 *
 * type> struct
 *
 *
 * brief> Handler for a master socket, port number, address and mode of a webserver
 *
 *
 * variables>
 *                0. socket_o * master - the master socket handler pointer
 *             
 *                1. struct sockaddr * address - a pointer to the initialized address 
 *
 *                2. portno port - the port number
 *             
 *                3. mode_t mode - the mode in which the server is set (can be f, t , p or d)
 *
 *
 * constructors> NOTE: each constructor has an init function with the same args and return void
 *                
 *                0. server_o(): 
 *                       brief> default constructor
 *
 *                       param> N/A
 *
 *                       return> N/A
 *                
 *
 *                1. server_o(portno , struct sockaddr *, mode_t): 
 *                       brief> constructror for a generic server
 *
 *                       param> 
 *                              0. portno - the port number to open it with
 *
 *                              1. struct sockaddr * - an initialized address to open it at
 *                              
 *                              2. mode_t - the mode to open it with
 *
 *                       return> N/A
 *
 *
 * destructors> NOTE:each destructor has an terminate function with the same args and return void
 *                
 *                0. ~server_o():
 *                       brief> default destructor
 *
 *                       param> N/A
 *
 *                       return> N/A
 *
 *
 * functions>
 *           
 *                0. output(int):
 *                       brief> outputs the entirety of the server's values to 
 *
 *                       param> 0. int - The iteration it was output
 *
 *                       return> void
 *
 *
 *                1. socketval(socket_t *):
 *                       brief> retrieves the master socket's value
 *
 *                       param> 0. socket_t * - a reference to a place to put the socket val
 *
 *                       return> void
 *
 * 
 */


struct server_o
{
  socket_o * master;
  struct sockaddr_in * address;
  portno port;
  mode_t mode;
  
  //Constructor 0
    server_o() { init(); }
    void init();
  //Constructor 1
    server_o(struct sockaddr_in * _address, portno _port, mode_t _mode)
    { init(_address, _port, _mode); }
    void init(struct sockaddr_in * _address, portno _port, mode_t _mode);

  //Destructor 0
    ~server_o() { terminate(); }
    void terminate();

  //Function 0
    void output(int i);
  //Function 1
    socket_t socketval();
};

//The current running server
server_o server;



/******************** SOCKET IMPLEMENTATION *********************/

//Constructor 0
void socket_o::init() { val = -1; master = false; }

//Constructor 1
void socket_o::init(struct sockaddr_in * address)
{
  //PD. print a path debugger
  if(PD) 
  { std::cout << "path debugger> Initializes a master socket with address * of" << address << std::endl; }
  //0. If this is being called, this socket is the master socket
    master = true;

  //1. Initialize a new socket
    val = socket(PF_INET, SOCK_STREAM, 0);
    if(val < 0) exit_err(SOCKET_ERR,EXIT_VAL);

  //2. Set the sockets optval
    opt = 1;
    err = setsockopt(val, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(socket_option));

  //3. Bind the socket to the given address
    err = bind(val, (struct sockaddr *) address, sizeof(*address));
    if(err) exit_err(BIND_ERR, EXIT_VAL);

  //4. Listen in the master socket for something of length q
    err = listen(val, Q_LEN);
    if(err) exit_err(LISTEN_ERR, EXIT_VAL);

}



//Destructor 0
void socket_o::terminate() { close(val); val = -1; }



//Operator 0
socket_o& socket_o:: operator=(socket_t _val) { master = false; val = _val; return *this; }



//Function 0
void socket_o::finish(int i = 1) { shutdown(val, i); terminate(); }

//Function 1
void socket_o::out(std::string * msg, int write_size = MAX_FILE_TRANSFER)
{
  //PD. Print out the buffer pointer location and the specified len
    if(PD)
    {   
      std::cout << "path debugger> Writes the socket(" << val << ") with message(";
          
      if(msg == NULL) { std::cout << "NULL"; }
      else { std::cout << msg; }

      std::cout << ") and write_size(" << write_size <<")\n";
    }
  
  //0. Check the validity of the two arguments
    if(msg == NULL) { return; }
    else if(msg->size() == 0) { return; }
    else if( write_size <= 0) { return; }
  
  //1. Find the proper write length
    if(msg->size() < write_size) { write_size = msg->size(); }
  
  //2. Call the write function
    write(val, msg->c_str(), write_size);
  
  return;
}

//Function 2
void socket_o::in(std::string * buffer, int read_size = MAX_FILE_TRANSFER)
{ 
  //PD. Print out the buffer pointer location and the specified len
    if(PD)
    {
      std::cout << "path debugger> Reads the socket(" << val << ") into buffer(";
      
      if(buffer == NULL) { std::cout << "NULL"; }
      else { std::cout << buffer; }

      std::cout << ") and read_size(" << read_size <<")\n";

    }
  //0. Make sure we can read in a length and read in a buffer
    //a. Check the validity of buffer
      if(buffer == NULL) { return; }
    
    //b. Check the vailidity of len
      if(read_size <= 0) { return; } 
  
  //1. Read in the file breaking only when there is a "\r\n" or the read_size has been exceeded
    //a. Initialize the variables  
      int n = 0;
      char in = 0;
      int read_len = 0;
    
    //PD and VD. Indicate the start and the read_size
      if(PD) { std::cout << "path debugger> Starts the reading process.\n"; }
      if(VD) { std::cout << "var debugger> The maximum read size is "<<read_size<<std::endl; }

    //b. Start reading
      while((n = read(val, &in, 1)))
      {
        //i. Read in the line and increment blah blah blah ...  
          read_len++;
          buffer->insert(buffer->end(),in);
        //ii. Return if the read_size has been exceeded
          if(read_len >= read_size) { return; }
        //iii. Return if the end has been matched
          //a. If it is in DEBUG_MODE, make sure it is a telnet end and not a <clrf><clrf>
          if(DEBUG_MODE) { if(buffer->find(TELNET_END) != std::string::npos) {return;} }
          else { if(buffer->find(CHROME_END) != std::string::npos) { return; } }
      }
  //R. Return
    return;
}

//Function 3
void socket_o::output(int i)
{
  //0. Make the prefix
    //a. Declare it
      std::string prefix = "";
    
    //b. Add the double spaces
      for(int a = 0; a <= i; a++) { prefix.append("  ");  }
    
    //c. Add the name
      prefix.append("socket>");
  
  //1. Output whether it is the master or not
    //a. Initialize the status variable  
      std::string status = "";
      if(master) status = "master";
      else status = "slave";
    
    //b. cout it
      std::cout << prefix << " status: " << status << std::endl;
    
    //c. Clear the status temp
      status.clear();

  //2. Output the val
    std::cout << prefix << " socketno: " << val << std::endl; 

  //3. Clear the prefix as we have no use for it
    prefix.clear();
}



/******************** SERVER IMPLEMENTATION *********************/

//Constructor 0
void server_o::init()
{
  if(PD) std::cout << "path debugger> Calls the default initializer for a server.\n";
  
  port = -1;
  
  mode = 'd';
  
  address = NULL;
  
  master = NULL;

  working_socket = NULL;
}

//Constructor 1
void server_o::init(struct sockaddr_in * _address, portno _port, mode_t _mode)
{
  //PD. Print
  if(PD)
  {
    std::cout << "path debugger> Initializing a server with address(";
    
    if(_address == NULL) { std::cout << "NULL) "; }
    else { std::cout << _address << ") "; }
    
    std::cout << ", portno(" << _port << ") and mode(" << _mode << ")\n";
  }
  
  //0. Check the validity of the port and then set it if valid
    if(_port >= PORT_MIN && _port <= PORT_MAX) { port = _port; }
    else { port = 13137; }
  
  //1. Validate the mode and then set it
    if(_mode == 'f' || _mode == 't' || _mode == 'p') { mode = _mode; }
    else { mode = 'd'; }
  
  //2. Check for validity on the address or set the address
    if(_address == NULL) { return; }
    else { address = _address; }

  //3. Initialize a master
    master = new socket_o(_address);

  //4. Set the working_socket
    //working_socket = master;

  //VD. Print
  if(VD) { std::cout << "var debugger> Value of all the server objects: \n"; this->output(1); }
}



//Destructor 0
void server_o::terminate()
{
  if(PD) { std::cout << "path debugger> Default destructor called\n"; }
  
  if(master) { delete master; }
  master = NULL;

  address = NULL;
  port = 0;
  mode = 0;
}



//Function 0
void server_o::output(int i)
{
  //PD. Print
    if(PD) { std::cout << "Calls the output of server on iteration (" << i << ")\n"; }
  
  //0. Initialize the prefix
    std::string prefix = "";
    for(int a = 0; a <= i; a++) { prefix += "  "; }
    prefix += "server> ";

  //1. Print out the variables of server
    std::cout << prefix << "port: " << port << std::endl;
    std::cout << prefix << "mode: -" << mode << std::endl;
    
    std::cout << prefix << "address: ";
    if(address == NULL) { std::cout << "NULL\n"; }
    else { std::cout << address << std::endl; }

    std::cout << prefix << "master: ";
    if(master == NULL) { std::cout << "NULL\n"; }
    else { std::cout << std::endl; master->output(i+1); }

  //R. Return
    return;
}



//Function 1
socket_t server_o::socketval()
{
  //PD. Print
    if(PD) { std::cout << "path debugger> The master socket has been requested.\n"; }

  //0. Check the validity of the master socket_o
    if(master == NULL) 
    { 
      if(VD) { std::cout << "var debugger> master is NULL" << std::endl; }
      return -1; 
    }
  
    if(PD) { std::cout << "path debugger> manages to escape the master == NULL check\n"; }
  //1. Return the master sockets value if valid
    return master->val;
}

/*********************** GLOBAL VARIABLES **************************/

//Holds the current working socket incase of exit
//socket_o * working_socket;

//Server object to hold the server
//server_o server;




/*********************** MUTEX FUNCTIONS **************************/

//Initializer for the mutex attributes
inline void init_mutex()
{
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutex_init(&mutex_thr,&mutex_attr);
}







/******************** ADDRESS FUNCTIONS *************************/

//Initializer for an address/port
inline void init_address(struct sockaddr_in * address, portno port)
{
  //PD. Print
  if(PD) std::cout << "path debugger> Calls init on an address with port (" << port << ")\r\n";
  
  //0. Check for port address validity and set the current working address if it is valid
    if(address == NULL) { return; }
    else { working_address = address; }

  //1. Initialize the memory of address
    int len = sizeof(*address);
    memset(address, 0, len);
  
  //2. Set the sin_family to the symbolic constant AF_INET
    address->sin_family = AF_INET;
  
  //3. Set the sin_addr.s_addr to INADDR_ANY as to get the IP Address of the host
    address->sin_addr.s_addr = INADDR_ANY;
  
  //4. Set the port, call htons to convert it from host byte order to network byte order
    address->sin_port = htons((u_short)port);
  
}




/******************** PTHREAD FUNCTIONS *************************/

//FUNCTION 0
inline void init_attr(pthread_attr_t * attr)
{
  pthread_attr_init(attr);
  pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
}


/******************** PROCESS FUNCTIONS *************************/

//FUNCTION 0
inline void pool_puppet(socket_t puppet)
{
  //PD. Print
  if(PD) { std::cout << "path debugger> Pool puppet called on socket(" << puppet << ")\n"; }
  
  //0. Loop the process
    while(1)
    {
      //a. Lock a mutex thread
        pthread_mutex_lock(&mutex_thr);
        //i. Try and have a new address accept the socket
          struct sockaddr_in client;
          int len = sizeof(client);
          accept(puppet, (struct sockaddr*) &client, (socklen_t*) &len);
      
      //b. Unlock the mutex
        pthread_mutex_unlock(&mutex_thr);
      
      //c. Terminate the puppet
        shutdown(puppet, 1);
        close(puppet);
    }
}

//FUNCTION 1
inline void process_thread(socket_t puppet)
{
  //0. Make the socket holder for puppet
    socket_o puppet_holder;
    puppet_holder = puppet;
  
  //1. Make the string to print out to the puppet holder
    std::string message;
  
  //2. Call process_request
    process_request(&puppet_holder, &message);
 
    //VD. Print the returned message
       if(VD) {std::cout<<"var debugger> Recieved message: \n    "<<message<<"\n";}
  //3. Print to the puppet_holder
    puppet_holder.out(&message);

  //4. Terminate the puppet holder
    puppet_holder.terminate();
  
  //5. Clear the message
    message.clear();
}

//FUNCTION 2
inline void process_request(socket_o * puppet, std::string * message)
{
  //PD. Print
    if(PD) 
    { std::cout<<"path debugger> puppet process("<<puppet->val<<") has it's request processed.\n"; }
  
  //0. Initialize a string and a socket reader to read in from the puppet
    std::string line; 

  //1. Read the first line from the socket into the line
      //a. Call the puppet's in function on line
        puppet->in(&line);
        if(VD) { std::cout<<"var debugger> Read in line: \n\n"<<line<<"\n\n from the socket.\n"; }
      
      //b. Get the index of the first character after the first \r\n
        std::string::size_type end_found;
        //i. If it is in debug mode, find the first instance of END, else find a crlf
        if(DEBUG_MODE) { end_found= line.find(END); }
        else { end_found = line.find(crlf); }

      //c. Erase everything from the string after the first \r\n
        line.substr(0, end_found);
        if(VD) { std::cout<<"var debugger> Line after erasure: \n\n"<<line<<std::endl; }


  //2. Check for a get inside the line
    std::string::size_type get_found = line.find(GET);

  //3. If there was no location of "GET", return a 404 Error
    if(get_found == std::string::npos) { *message = NOT_FOUND_MSG; line.clear(); return; }

  //4. If it is however a valid request, process it
    //a. Format the line properly
      //i. Remove the "GET "
        line = line.substr(get_found+strlen(GET)+1);
      
      //ii. Remove the protocol at the end
        std::string::size_type protocol_found = line.find(PROTOCOL);
        if(protocol_found != std::string::npos) 
        { line.erase(line.begin() + protocol_found-1, line.end()); }

    //c. Get the file path from the line
      //PD. Print  
        if(PD) { std::cout<<"path debugger> Parsing the line \""<<line<<"\" for the file path.\n"; }
      
      //i. Make a file path temp and then parse the line for a the file path
        std::string file_path = "";

        //0. For icons and htdocs, add the root directory then the line to the file_path
          if((line.compare(ICONS) == 0) || (line.compare(HTDOCS) == 0))
          { file_path = ROOT_DIR + line; }

        //1. An instance of slash has it's own path, so set the file_path to that
          else if(line.compare(NOT_THE_SECRET_KEY) == 0) { file_path = SLASH_PATH; }

        //2. Else just add the default path to the front of line
          else { file_path = DEFAULT_PATH + line; }

      //ii. Retrieve the content type and store it in a temp
        std::string type = content_type(&line);

      //iii. Clear the line as it is now useless
        line.clear();

      
    //d. Open the file and read it into line
      //PD. Print
        if(PD) { std::cout<<"path debugger> Reading file\""<<file_path<<"\" into the message.\n"; }
      
      //i. Make an fstream and check if it opens the file
        //0. Open the file
          std::fstream file(file_path.c_str());
        
        //1. Clear the file_path as it is no longer useful
          file_path.clear();
        
        //2. If the file isn't opened, return a NOT_FOUND error message
          if(!file.is_open()) { *message = NOT_FOUND_MSG; type.clear(); return; }


      //ii. Open a stringstream to read the file into line
        //0. Initialize the strstream 
          std::stringstream buffer;

        //1. Put the read buffer of the file into the strstream
          buffer << file.rdbuf();
          
          file.close();

        //2. Read the contents into line
          line = buffer.str();
      
      //VD. Print the file contents
        if(VD) { std::cout << "var debugger> The line read in is: \n    "<<line<<"\n"; }
      

      //iii. Set the message to the file contents with the format
        //0. Set the value of message
          *message = HTTP_200 + type + crlf + crlf + line + crlf;

        //1. Clear the type and the line
          type.clear(); line.clear();

        //R. Return
          return;

}







/******************** MAIN ********************/

int main(int argc, char ** argv)
{
   
  //PD. Print
    if(PD) std::cout << "path debugger> main starts called with " << argc << " arguments.\n";
  //0. Initialize the variables
    portno port = 0;
    mode_t mode = 'd';

  //1. Check for valid argument size
    //a. If there is less that 2 string arguments, call exit_err with the usage string
      if (argc < 2) { exit_err(usage, EXIT_VAL); }
    
    //b. If there are only 2 arguments, the portno is argv[1]
      else if(argc == 2) { port = atoi(argv[1]); }

    //c. If there are 3 arguments, the portno is argv[2]
      else if(argc = 3) { port = atoi(argv[2]); }

    //d. If we have a valid portno and the first character of argv[1] is a dash, get the next char
    //   as that is the mode flag
      if((argc == 2 || argc == 3)&&(argv[1][0] == DASH)) { mode = argv[1][1]; }

  //PD. Print the port and the mode
    if(PD)
    { 
      std::cout << "path debugger> The desired portno (" << port <<") ";
      std::cout << "and the desired mode(" << mode << ")" << std::endl;
    }
   


  //2. Initialize an addresd and the server
    
    struct sockaddr_in address;
    init_address(&address,port);

    server.init(&address, port, mode);

  //PD. Returns from init 
    if(PD) { std::cout << "path debugger> Gets back from init" <<std::endl;}

  //3. While loop to handle all the modes 
    while(1)
    {

      //a. Handle the p mode first
      if(server.mode == 'p') 
      {
        //i. Do server mode p
          //0. Init the mutex
            init_mutex();
          
          //1. Make a q of pthreads and initialize the make attributes that can be initiliazed
            pthread_t qid[Q_LEN];
            pthread_attr_t attr;
            init_attr(&attr);
          
            socket_t _master = server.socketval();
            
          //2. For each pthread type in qid, create a pthread using pool slave and the master socket
            for(int q = 0; q < Q_LEN; q++)
            {
              pthread_create(&qid[q], &attr, (void * (*)(void *)) pool_puppet, (void *) _master);
            }

          //3. Join the threads
            for(int q = 0; q < Q_LEN; q++)
            { pthread_join(qid[q], NULL); }
        break;
      }

      //b. If the server is not in p mode, make a client address
        //i. Initialize then try and accept a client socket onto the current server's master socket
          struct sockaddr_in client;
          int len = sizeof(client);
          int sock = server.socketval(); 
  
          if(VD) { std::cout << "var debugger> sock is " << sock << ".\n"; }

          socket_t puppet;
          puppet = accept( sock , (struct sockaddr*) &client, (socklen_t*) &len);
          
          if(PD) { std::cout << "path debugger> Gets back from socketval and accept.\n"; }
        //ii. Check for failure on the accepting of the client
          if(puppet < 0) { exit_err(ACCEPT_ERR, EXIT_VAL); }

      //c. Handle the f mode
        if(server.mode == 'f')
        {
          //i. Make a returner and then fork
            pid_t returner = fork();
          
          //ii. If we are in the child process, process the thread then exit
            if(returner == 0) { process_thread(puppet); exit(1); }
          
          //iii. Close the socket if it's not the child
            close(puppet);
        }
      //d. Handle the t mode
        else if(server.mode == 't')
        {
          //0. Initialize a pthread type and a pthread attribute
            pthread_t qid;
            pthread_attr_t attr;
            init_attr(&attr);
          
          //1. Create a thread based around the slave_socket
            pthread_create(&qid, &attr, (void * (*)(void *)) process_thread, (void *) puppet);
            pthread_join(qid, NULL);
        }

      //e. Handle the default mode
        else { process_thread(puppet); }

    }
    
    server.terminate();
    return 0;
}

#ifndef DEFINES_H
#define DEFINES_H

//Debugger: Generalized debugger if you don't know what to put a debugger under
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

//Path debugger: prints everytime a new function is called
#ifndef PD
#define PD 1
#endif

//Variable debugger: used to check what values are at what place
#ifndef VD
#define VD 0
#endif

/*
#ifndef
#define 
#endif
*/





#ifndef INCLUDES_MYHTTPD
#define INCLUDES_MYHTTPD


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



#endif









//Max length of a name
#ifndef MAX_NAME_LEN 
#define MAX_NAME_LEN 1024
#endif

//Max length of a read or write to a file
#ifndef MAX_FILE_TRANSFER
#define MAX_FILE_TRANSFER 65535
#endif

//Min and Max port values
#ifndef PORT_MIN
#define PORT_MIN 1024
#endif

#ifndef PORT_MAX
#define PORT_MAX 65535
#endif



//TODO: Figure out what this does
#ifndef Q_LEN
#define Q_LEN 5
#endif

#ifndef EXIT_VAL
#define EXIT_VAL -1
#endif

// USEFUL STRINGS ARE CLOSE WITHIN THE BRACKETS {




//Used to print a new line to a socket
#ifndef CRLF
#define CRLF "\r\n"
#endif

#ifndef END
#define END "\n"
#endif

#ifndef TELNET_END
#define TELNET_END "\n"
#endif

#ifndef CHROME_END
#define CHROME_END "\r\n\r\n"
#endif

//I'm actually pulling a bamboozle here because this is actually the secret key
#ifndef NOT_THE_SECRET_KEY
#define NOT_THE_SECRET_KEY "/7141"
#endif

#ifndef SERVER_REEEEE
#define SERVER_REEEEE "Get off meeeee Server REEEEEEEEEE!!!!!!!!!\r\n"
#endif

#ifndef UNAUTH_ACCESS
#define UNAUTH_ACCESS "HTTP/1.1 401 Unauthorized Access\r\n"
#endif

#ifndef ROOT_DIR
#define ROOT_DIR "http-root-dir/"
#endif

#ifndef SLASH_PATH
#define SLASH_PATH "http-root-dir/htdocs/index.html"
#endif

#ifndef PROTOCOL
#define PROTOCOL "HTTP/1.1"
#endif

#ifndef DFAULT_PATH
#define DEFAULT_PATH "http-root-dir/htdocs"
#endif

#ifndef GET
#define GET "GET"
#endif

#ifndef ICONS
#define ICONS "/icons"
#endif

#ifndef HTDOCS
#define HTDOCS "/htdocs"
#endif

#ifndef _HTML
#define _HTML ".html"
#endif

#ifndef _HTML_
#define _HTML_ ".html/"
#endif

#ifndef _GIF
#define _GIF ".gif"
#endif

#ifndef _GIF_ 
#define _GIF_ ".gif/"
#endif

#ifndef HTTP_200
#define HTTP_200  "HTTP/1.1 200 Document follows\r\nServer: CS252 Lab5\r\nContent-type"
#endif

#ifndef NOT_FOUND_MSG
#define NOT_FOUND_MSG "HTTP/1.1 404 File not found\r\nServer: CS252 Lab5\r\nContent-type text/html\r\n\r\n"
#endif

#ifndef SOCKET_ERR
#define SOCKET_ERR "socket"
#endif

#ifndef BIND_ERR
#define BIND_ERR "bind"
#endif

#ifndef LISTEN_ERR
#define LISTEN_ERR "listen"
#endif

#ifndef ACCEPT_ERR
#define ACCEPT_ERR "accept"
#endif


#ifndef DASH
#define DASH '-'
#endif
/*
#ifndef
#define 
#endif
*/
// } USEFUL STRINGS ARE NOW CLOSED



//Indiced value for the content type followed by it's actual value in lowercase letters starts here
#ifndef HTML_TXT
#define HTML_TXT 0
#endif

#ifndef html_txt
#define html_txt "text/html"
#endif

#ifndef PLAIN_TXT
#define PLAIN_TXT 1
#endif

#ifndef plain_txt
#define plain_txt "text/plain"
#endif

#ifndef GIF
#define GIF 2
#endif

#ifndef gif
#define gif "image/gif"
#endif
//Ends here


//My own definitions for better readability
#ifndef llong
#define llong long long
#endif

#ifndef ullong
#define ullong unsigned long long
#endif

#ifndef portno
#define portno int
#endif


#ifndef socket_t
#define socket_t int
#endif

#ifndef socket_error
#define socket_error int
#endif

#ifndef socket_option
#define socket_option int
#endif

#ifndef mode_t
#define mode_t char
#endif

#endif

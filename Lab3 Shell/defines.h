#ifndef DEFINES_H
#define DEFINES_H

#ifndef indices 
#define indices
  //Use these enums for quick access
  enum { IN = 0, OUT = 1, ERR = 2, in = 1, out = 2, err = 4, stdall = 7, 
          tin = 8, tout = 16, terr = 32, tall = 56};
#endif

#ifndef debugger
#define debugger 0
#endif

#ifndef path_debugger 
#define path_debugger 0
#endif

#ifndef y_path_debugger 
#define y_path_debugger 0
#endif

#ifndef CHILD
#define CHILD 0
#endif

#ifndef EXIT_VAL
#define EXIT_VAL 0
#endif

#ifndef SOFT_EXIT_VAL
#define SOFT_EXIT_VAL 1
#endif

#ifndef SOURCE_ERROR
#define SOURCE_ERROR "SOURCE ERROR MATCH"
#endif

#endif

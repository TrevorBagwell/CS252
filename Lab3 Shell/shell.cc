#include <cstdio>

#include "shell.hh"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int yyparse(void);

void Shell::prompt() {
  if(isatty(0)){
    printf("mcgregorbash>");
    fflush(stdout);
  }
}

struct sigaction_wrapper
{
  struct sigaction action;
  int err;

  sigaction_wrapper(void (&f)(int))
  {
    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
  }
  void call_sigaction(int i)
  {
    err = sigaction(i, &action, NULL);
    if(err)
    {
      perror("sigaction");
      exit(-1);
    }
  }

};

extern "C" void ctrlc (int sig)
{
  Shell::_currentCommand.clear();
  printf("\n");
  Shell::prompt();
}
extern "C" void zombie (int sig)
{
  wait3(0,0, NULL);
  while(waitpid(-1, NULL, WNOHANG) > 0);
}



int main() {
  
  if(isatty(STDIN_FILENO))
  {
    //Check something, can't remember what
    system("/homes/tbagwel/cs252/lab3-src/.shellrc");
  }
  
  sigaction_wrapper ctrlc_act = sigaction_wrapper(ctrlc);
  sigaction_wrapper zombie_act = sigaction_wrapper(zombie);
  ctrlc_act.call_sigaction(SIGINT);
  zombie_act.call_sigaction(SIGCHLD);


  //Always belongs at the bottom
  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;

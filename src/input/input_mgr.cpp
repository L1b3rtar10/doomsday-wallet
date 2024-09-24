#include <iostream>
#include <termios.h>
#include "input_mgr.h"


InputMgr InputMgr::Make(int echo)
{
  return InputMgr(echo);
}

/* Initialize new terminal i/o settings */
void InputMgr::setConfig()
{
  tcgetattr(0, &oldconfig); /* grab old terminal i/o settings */
  newconfig = oldconfig; /* make new settings same as old settings */
  newconfig.c_lflag &= ~ICANON; /* disable buffered i/o */
  newconfig.c_lflag &= _echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newconfig); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void InputMgr::resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &oldconfig);
}

/* Read 1 character - echo defines echo mode */
char InputMgr::secureInput(char* input, size_t& len)
{
  char ch;
  setConfig();
  
  while ((ch = getchar()) != '\n') {
      input[len++] = ch;
      cout << '*';
  }
  resetTermios();
  return ch;
}

/*
 * Wrapper to getch() used to prevent typed input from being displayed to screen
 */
#ifndef INPUT_MGR_H
#define INPUT_MGR_H

#include <termios.h>
#include <optional>
#include <cstdint>

using namespace std;

class InputMgr
{    
    private:
        termios oldconfig;
        termios newconfig;
        int _echo;

        InputMgr(int echo) { _echo = echo; };
        
        void setConfig();
        void resetTermios();

    public:
        static std::optional<InputMgr> Make(int echo);
        char secureInput(char* input, size_t& len);
};

#endif

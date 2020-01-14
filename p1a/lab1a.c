#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

struct termios oldconfig;

void set_input_mode();
void reset_input_mode();

int main() {
    char ch;
    set_input_mode();
    while (1) {
        read(STDIN_FILENO, &ch, 1);
        if (ch == '\004')
            break;
        else
            write(STDOUT_FILENO, &ch, 1);
    }
    //reset_input_mode();
    
    return 0;
}

void reset_input_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldconfig);
}

void set_input_mode() {
    struct termios newconfig;
    tcgetattr(STDIN_FILENO, &oldconfig);
    atexit(reset_input_mode);
    tcgetattr(STDIN_FILENO, &newconfig);
    newconfig.c_iflag = ISTRIP;
    newconfig.c_oflag = 0;
    newconfig.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newconfig);
}
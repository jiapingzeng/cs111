#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>

struct termios saved;

void reset_input_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved);
}

void set_input_mode() {
    struct termios config;
    tcgetattr(STDIN_FILENO, &saved);
    tcgetattr(STDIN_FILENO, &config);
    config.c_iflag = ISTRIP;
    config.c_oflag = 0;
    config.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &config);
}

int main() {
    char buffer[8];
    set_input_mode();
    while (1) {
        read(STDIN_FILENO, &buffer, 8);
        if (buffer[0] == '\004')
            break;
        else
            write(STDOUT_FILENO, &buffer, 8);
    }
    reset_input_mode();
    
    return 0;
}
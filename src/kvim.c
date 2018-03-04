#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios original_termios;

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
        die("tcsetattr");
}

void EnableRawMode() {
    // Reads the termios sessings into a temp struct so that we can reset to
    // them later.
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);

    // Copy the current setting.
    struct termios raw = original_termios;

    // Stops Echoing, , Ctrl-C, Ctrl-Z, Ctrl-V
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    // Not auto adding carriage before newline.
    raw.c_oflag &= ~(OPOST);
    // character size to 8 bits per byte
    raw.c_cflag |= (CS8);
    // Making sure some other misc settings are off.
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // Timout reading a character.
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // Write our new settings.
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

int main() {
    EnableRawMode();

    while (1) {
        uint8_t c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }

        if (c == 'q')
            break;
    }

    return 0;
}

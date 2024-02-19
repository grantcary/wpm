#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

void enableRawMode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Turn off echoing and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

void printRandoms(int lower, int upper, int count) { 
    int i; 
    for (i = 0; i < count; i++) { 
        int num = (rand() % 
        (upper - lower + 1)) + lower; 
        printf("%d ", num); 
    } 
}

char **readlines() {
    char **arr = malloc(100 * sizeof(char *));

    FILE *file = fopen("100words.txt", "r");

    char word[10];
    int count;
    while (fgets(word, sizeof(word), file)) {
        word[strcspn(word, "\n")] = 0;
        arr[count] = malloc(strlen(word) + 1);
        strcpy(arr[count++], word);
    }

    fclose(file);

    return arr;
}

int main() {
    struct termios orig_termios;

    tcgetattr(STDIN_FILENO, &orig_termios);

    enableRawMode(&orig_termios);

    printf("\033[2J\033[H");
    printf("\033[33mPress ESC to quit.\033[0m\n");

    char *str = (char *) malloc(1);
    str[0] = '\0';
    size_t len = 0;

    char **words = readlines();
    char *word_buffer, *next_word;

    srand(time(0));
    int random = rand() % 100;
    word_buffer = words[random];

    random = rand() % 100;
    next_word = words[random];

    printf("\033[2K");
    printf("%s %s \033[33m0\033[0m", word_buffer, next_word);
    fflush(stdout);
    printf("\033[1G");

    char *incorrect_buffer = (char *) malloc(1);
    incorrect_buffer[0] = '\0';
    size_t incorrect_len = 0;
    bool incorrect_chars = false;


    struct timeval st, ct;
    int elapsed;
    gettimeofday(&st, NULL);
    int WPM;

    while (1) {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);

        if (c == 27) { // ESC was pressed
            break;
        }

        if (c == word_buffer[0] && strlen(incorrect_buffer) == 0) { // character correct
            word_buffer++; // remove character from front of word buffer
            incorrect_chars = false;
        } else if (c == ' ' && strlen(word_buffer) == 0 && strlen(incorrect_buffer) == 0) {
            incorrect_chars = false;
        } else {
            incorrect_chars = true;
        }

        if (c >= 32 && c < 127) { // Printable characters
            if (!incorrect_chars) {
                str = realloc(str, len + 2);
                str[len] = c;
                str[len + 1] = '\0';
                len++;
            } else {
                incorrect_buffer = realloc(incorrect_buffer, incorrect_len + 2);
                incorrect_buffer[incorrect_len] = c;
                incorrect_buffer[incorrect_len + 1] = '\0';
                incorrect_len++;
            }
        } else if (c == 127 && strlen(incorrect_buffer) > 0) { // Backspace was pressed
            incorrect_len--;
            incorrect_buffer[incorrect_len] = '\0';
            incorrect_buffer = realloc(incorrect_buffer, incorrect_len + 1);
        }

        if (c == ' ' && strlen(incorrect_buffer) == 0 && strlen(word_buffer) == 0) { // word correct, reset word buffer with new random word
            random = rand() % 100;
            word_buffer = next_word;
            next_word = words[random];
        }

        gettimeofday(&ct, NULL);
        elapsed = ct.tv_sec - st.tv_sec;
        WPM = 12 * (strlen(str) / elapsed);

        printf("\033[2K");
        printf("\033[32m%s\033[0m\033[31m%s\033[0m%s %s \033[33m%d\033[0m", str, incorrect_buffer, word_buffer, next_word, WPM);
        fflush(stdout);
        printf("\033[1G");
    }
    
    printf("\033[2K");
    fflush(stdout);
    printf("\033[1G");

    disableRawMode(&orig_termios);

    return 0;
}
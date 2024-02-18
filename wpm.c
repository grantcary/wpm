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
    printf("Press arrow keys or ESC to quit.\n");

    char *str = (char *) malloc(1); // Start with space for the terminating null
    str[0] = '\0'; // Initialize string as empty
    size_t len = 0;

    char **words = readlines();
    // while (*words != NULL) {
    //     printf("%s\n", *words++);
    // }

    char *word_buffer, *next_word;

    srand(time(0));
    int random = rand() % 100;
    word_buffer = words[random];

    random = rand() % 100;
    next_word = words[random];

    // while (!strcmp(word_buffer, next_word)) {
    //     random = rand() % 100;
    //     next_word = words[random];
    // }

    printf("\033[2K");
    printf("\033[32m%s %s\033[0m %lu", word_buffer, next_word, strlen(str));
    fflush(stdout);
    printf("\033[1G");

    char *incorrect_buffer = (char *) malloc(1);
    incorrect_buffer[0] = '\0';
    size_t incorrect_len = 0;

    int correct_chars = 0;
    int WPM;

    struct timeval st, ct;
    int elapsed;
    gettimeofday(&st, NULL);

    bool wrong = false;

    while (1) {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);

        if (c == '\033') {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;

            if (seq[0] == '[') {
                if (seq[1] == '3' && read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~') { // Delete key
                    if (len > 0) {
                        str[len-1] = '\0'; // Remove last character
                        len--;
                        str = realloc(str, len + 1); // Adjust memory allocation
                    }
                }
            }
            continue;
        }

        if (c == 27) {
            break;
        }

        if (c == word_buffer[0] && strlen(incorrect_buffer) == 0) {
            // character correct
            // remove character from front of word buffer
            correct_chars++;
            word_buffer++;
            wrong = false;
        } else if (strlen(word_buffer) != 0){
            wrong = true;
        }

        if (c >= 32 && c < 127) { // Printable characters
            if (!wrong) {
                str = realloc(str, len + 2); // Increase the allocation for new character + null terminator
                str[len] = c;
                str[len + 1] = '\0';
                len++;
            } else {
                incorrect_buffer = realloc(incorrect_buffer, incorrect_len + 2);
                incorrect_buffer[incorrect_len] = c;
                incorrect_buffer[incorrect_len + 1] = '\0';
                incorrect_len++;
            }
        } else if (c == 127) { // Backspace was pressed
            if (strlen(incorrect_buffer) != 0) {
                incorrect_len--;
                incorrect_buffer[incorrect_len] = '\0';
                incorrect_buffer = realloc(incorrect_buffer, incorrect_len + 1);
            }
        }

        if (c == ' ' && strlen(incorrect_buffer) == 0 && strlen(word_buffer) == 0) {
            if (strlen(word_buffer) == 0) {
                correct_chars++;
            }
            // word correct, reset word buffer with new random word
            random = rand() % 100;
            word_buffer = next_word;
            next_word = words[random];
        }

        gettimeofday(&ct, NULL);
        elapsed = ct.tv_sec - st.tv_sec;
        WPM = 12 * (correct_chars / elapsed);

        printf("\033[2K");
        printf("%s\033[31m%s\033[0m\033[32m%s %s\033[0m %d", str, incorrect_buffer, word_buffer, next_word, WPM);
        fflush(stdout);
        printf("\033[1G");
    }

    disableRawMode(&orig_termios);

    return 0;
}
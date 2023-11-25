#include "myshell.h"

int main() {
    rl_initialize();
    rl_bind_key('\t', rl_complete);
    using_history();

    signal(SIGTSTP, sigtstp_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);

    struct termios original_termios;
    tcgetattr(STDIN_FILENO, &original_termios);

    FILE* fp = fopen("info.txt", "r");
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        printf("%c", ch);
    }
    fclose(fp);

    printf("\n");
    while (1) {
        char* input = readline(prompt);

        if (!input) {
            printf("\n");
            break;
        }

        if (input && *input) {
            add_to_history(tokenize(input));
            char** tokens = tokenize(input);

            expand_wildcards(tokens);

            execute(tokens);

            for (int i = 0; tokens[i] != NULL; i++) {
                free(tokens[i]);
            }
            free(tokens);

            tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        }

        add_history(input);

        free(input);
    }

    clear_history();

    return 0;
}

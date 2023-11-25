#include "myshell.h"

void sigtstp_handler() {
    printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void sigint_handler() {
 printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void sigquit_handler() {
printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}


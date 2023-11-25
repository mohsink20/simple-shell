#include "myshell.h"

char prompt[MAX_INPUT_SIZE] = "meowsh😺$ ";

void change_prompt(const char* newprompt) {
    if (newprompt != NULL) {
        snprintf(prompt, sizeof(prompt), "%s", newprompt);
    }
}
